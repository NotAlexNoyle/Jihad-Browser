/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — minimal BrowserAdapter stand-in.
 * A YapClient that connects to jihad-browserserver, sends the real YAP commands
 * (Connect 0x1000, OpenUrl 0x1004) and prints the server->adapter messages
 * (LoadStarted/Progress/Stopped/LocationChanged, Painted). Proves the full IPC
 * round-trip against the running Goanna daemon, contract unchanged.
 */
#include <YapClient.h>
#include <YapPacket.h>
#include <BrowserOffscreenInfo.h>   // the isis shmem layout: [header][ARGB32 pixels]
#include <glib.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>

class JihadAdapterClient : public YapClient {
public:
  // HARNESS FIX (pre-existing, found while verifying T-057): the segment must hold
  // the BrowserOffscreenInfo HEADER *plus* the pixels. The real BrowserAdapter
  // allocates it that way and reads rasterBuffer() at base+sizeof(header); this
  // stand-in still allocated bare W*H*4, so the daemon's
  //   if (segSize < hdr + w*h*4) return;   // BrowserPageGoanna::paintToSharedBuffer
  // guard silently declined EVERY paint and the round-trip could only ever end in
  // the 30 s timeout (exit 124, never a msgPainted). The client dates from the
  // pre-header desktop PoC (36e8513) and was never updated when the header was
  // added to the layout.
  static constexpr size_t kHdr = sizeof(BrowserOffscreenInfo);
  // 4x the viewport, matching the real adapter (BrowserOffscreen::create,
  // kOffscreenSizeAsScreenSizeMultiplier = 4). This is what gives the daemon room to
  // paint viewport + overscan (scroll pan headroom); with a 1x segment the daemon
  // correctly degrades to viewport-exact paints and the overscan path is never tested.
  explicit JihadAdapterClient(const char* name)
    : YapClient(name), painted(0), verified(0), loadStopped(false),
      W(1024), H(768), sz((int)kHdr + 1024*768*4*4), key1(0x4a494831), key2(0x4a494832) {}
  int painted, verified;
  bool loadStopped;
  bool wroteImage = false;
  int W, H, sz, key1, key2;
  int lastRenderedH = 0, lastRenderedY = -1;   // from the last verified header (overscan check)
  long stripNonWhite = 0;                      // non-white px in rows BELOW the viewport
  long alphaBad = 0;                           // pixels with a != 0xff (adapter raw-blits words)
  // Scrolled-overscan phase (review 2026-08-02 F2: without a scroll every offset is 0 and
  // the bandRow/renderedY arithmetic is never exercised): after the first verified paint,
  // send SetScrollPosition(0,1200) and keep re-asserting until a frame arrives with
  // renderedY > 0 whose pixels place band 3 of the default page at the right buffer row.
  bool scrolled = false, scrollPass = false;
  int scrollRetries = 0;
  bool probeOK = false;                        // band-3 colour at (700, 2000-renderedY)

  void serverConnected() override {}      // YapClient never calls this; we drive from start()

  // Write the painted shared buffer to a PPM image (the real adapter blits it to
  // the card). Proves the desktop pipe produces a correct pixel image (R2/R3).
  void writeImage(int key) {
    int id = shmget(key, sz, 0);
    if (id < 0) return;
    unsigned char* b = (unsigned char*)shmat(id, nullptr, SHM_RDONLY);
    if (b == (unsigned char*)-1) return;
    const char* path = getenv("JIHAD_POC_IMAGE");
    if (!path || !*path) path = "/out/jihad-poc-render.ppm";
    const BrowserOffscreenInfo* oi = (const BrowserOffscreenInfo*)b;
    unsigned char* px = b + kHdr;                 // pixels live AFTER the header
    // Dump the REAL painted region (may be taller than the viewport — overscan).
    int iW = oi->renderedWidth > 0 ? oi->renderedWidth : W;
    int iH = oi->renderedHeight > 0 ? oi->renderedHeight : H;
    if ((size_t)iW * iH * 4 > (size_t)sz - kHdr) { iW = W; iH = H; }
    FILE* fp = fopen(path, "wb");
    if (fp) {
      fprintf(fp, "P6\n%d %d\n255\n", iW, iH);
      printf("[adapter] header says %dx%d zoom=%.3f rendered=%d,%d\n",
             oi->bufferWidth, oi->bufferHeight, oi->contentZoom, oi->renderedX, oi->renderedY);
      for (long i = 0; i < (long)iW * iH; ++i) {
        unsigned char* p = px + (size_t)i * 4;    // B,G,R,A
        unsigned char rgb[3] = { p[2], p[1], p[0] };
        fwrite(rgb, 1, 3, fp);
      }
      fclose(fp);
      printf("[adapter] wrote %dx%d render to %s\n", iW, iH, path);
    }
    shmdt(b);
  }

  // Return a painted buffer to the daemon per the contract (ReturnBuffer 0x150d).
  void returnBuffer(int key) {
    YapPacket* r = packetCommand();
    (*r) << (int16_t)0x150d; (*r) << (int32_t)key;
    sendAsyncCommand();
  }

  // Verify the daemon actually rendered content into the shared buffer it named
  // in msgPainted (the real adapter would blit it to the card). Returns
  // non-white pixel count, or -1.
  long verifyBuffer(int key) {
    int id = shmget(key, sz, 0);
    if (id < 0) return -1;
    unsigned char* b = (unsigned char*)shmat(id, nullptr, SHM_RDONLY);
    if (b == (unsigned char*)-1) return -1;
    const BrowserOffscreenInfo* oi = (const BrowserOffscreenInfo*)b;
    // The daemon reports the REAL painted geometry (may be taller than the viewport —
    // overscan). Count within it, bounded by the segment.
    int rW = oi->renderedWidth > 0 ? oi->renderedWidth : W;
    int rH = oi->renderedHeight > 0 ? oi->renderedHeight : H;
    if ((size_t)rW * rH * 4 > (size_t)sz - kHdr) { rW = W; rH = H; }
    lastRenderedH = rH; lastRenderedY = oi->renderedY;
    long nb = 0, strip = 0, abad = 0;
    for (long i = 0; i < (long)rW * rH; ++i) {
      unsigned char* p = b + kHdr + (size_t)i * 4;   // pixels AFTER the header; B,G,R,A
      if (!(p[0] > 240 && p[1] > 240 && p[2] > 240)) {
        ++nb;
        if (i >= (long)rW * H) ++strip;              // content beyond the first viewport
      }
      if (p[3] != 0xff) ++abad;                      // review F3: any a<255 is a hole in the card
    }
    if (strip > stripNonWhite) stripNonWhite = strip;
    alphaBad += abad;
    // Band-3 colour probe (#228844 → stored B=0x44 G=0x88 R=0x22) at DOCUMENT row 2000,
    // x=700 — validates that renderedY truly positions the pixels (review F2). Only
    // meaningful on the default banded page.
    probeOK = false;
    long pr = 2000 - (long)oi->renderedY;
    if (pr >= 0 && pr < rH) {
      unsigned char* p = b + kHdr + ((size_t)pr * rW + 700) * 4;
      int db = (int)p[0] - 0x44, dg = (int)p[1] - 0x88, dr = (int)p[2] - 0x22;
      probeOK = db > -12 && db < 12 && dg > -12 && dg < 12 && dr > -12 && dr < 12;
    }
    shmdt(b);
    return nb;
  }

  // SetScrollPosition (0x1500): cx, cy, cw, ch — pan the daemon like the real adapter does.
  void sendScroll(int x, int y) {
    YapPacket* c = packetCommand();
    (*c) << (int16_t)0x1500;
    (*c) << (int32_t)x; (*c) << (int32_t)y;
    (*c) << (int32_t)W; (*c) << (int32_t)H;
    sendAsyncCommand();
  }

  // Send the initial YAP commands after connect() succeeds. The BrowserAdapter
  // OWNS the shared buffers: allocate them here, then connect() names their keys.
  void start() {
    shmget(key1, sz, IPC_CREAT | 0600);
    shmget(key2, sz, IPC_CREAT | 0600);
    printf("[adapter] allocated shared buffers; sending Connect + OpenUrl\n");
    // Connect (0x1000): w, h, key1, key2, size, identifier
    YapPacket* c = packetCommand();   // YapPacket::operator<< returns void, so no chaining
    (*c) << (int16_t)0x1000;
    (*c) << (int32_t)W; (*c) << (int32_t)H;
    (*c) << (int32_t)key1; (*c) << (int32_t)key2;
    (*c) << (int32_t)sz; (*c) << (int32_t)1;
    sendAsyncCommand();
    // OpenUrl (0x1004): url
    const char* url = getenv("JIHAD_URL");
    // Default page is TALLER than the viewport (4 distinct 768px bands) so the daemon's
    // overscan paint has real content below the first screen to render — a short page
    // legitimately clamps the painted region to the content height.
    if (!url || !*url) url = "data:text/html,<title>Jihad</title>"
      "<body style='margin:0;color:white;font:48px sans-serif'>"
      "<div style='height:768px;background:%23224488'><h1>Adapter round-trip OK</h1></div>"
      "<div style='height:768px;background:%23884422'>band 2</div>"
      "<div style='height:768px;background:%23228844'>band 3</div>"
      "<div style='height:768px;background:%23442288'>band 4</div></body>";
    YapPacket* o = packetCommand();
    (*o) << (int16_t)0x1004; (*o) << url;
    sendAsyncCommand();
  }
  void serverDisconnected() override { printf("[adapter] serverDisconnected\n"); }

  void handleAsyncMessage(YapPacket* msg) override {
    int16_t id = 0;
    (*msg) >> id;
    switch ((uint16_t)id) {
      case 0x2000: { int32_t k = 0; (*msg) >> k; ++painted;
        long nb = verifyBuffer(k);
        printf("[adapter] <- msgPainted(key=0x%x): %ld non-white px, renderedY=%d renderedH=%d probe=%d\n",
               (unsigned)k, nb, lastRenderedY, lastRenderedH, (int)probeOK);
        if (nb > 100) ++verified;
        if (nb > 100 && !wroteImage) { writeImage(k); wroteImage = true; }
        returnBuffer(k);   // hand the buffer back so rendering can continue
        if (loadStopped && verified > 0) {
          const char* eo = getenv("JIHAD_EXPECT_OVERSCAN");
          // The scrolled phase only makes sense against the DEFAULT banded page (the
          // colour probe knows its layout) and only when overscan is expected at all.
          bool wantScroll = eo && *eo == '1' && !getenv("JIHAD_URL");
          if (!wantScroll) {
            // JIHAD_STAY=1: keep the connection (and therefore the daemon's page) alive so
            // the $JIHAD_INJECT channel can drive clicks into it — the popup/menu
            // experiments need a live page after the first verified paint. The harness
            // bounds the run with `timeout`.
            if (getenv("JIHAD_STAY")) {
              printf("[adapter] verified; STAYING for inject-driven testing\n");
            } else {
              printf("[adapter] verified rendered frame after load; done\n");
              g_main_loop_quit(mainLoop());
            }
          } else if (!scrolled) {
            printf("[adapter] -> SetScrollPosition(0,1200) — scrolled-overscan phase\n");
            sendScroll(0, 1200);
            scrolled = true;
          } else if (lastRenderedY > 0 && probeOK) {
            scrollPass = true;
            printf("[adapter] scrolled frame verified (renderedY=%d, band-3 probe hit); done\n",
                   lastRenderedY);
            g_main_loop_quit(mainLoop());
          } else if (++scrollRetries < 12) {
            // The engine scroll is async; nudge and wait for the next repaint.
            sendScroll(0, 1200);
          } else {
            printf("[adapter] scrolled-overscan phase exhausted retries\n");
            g_main_loop_quit(mainLoop());
          }
        }
        break; }
      case 0x2005: printf("[adapter] <- msgLoadStarted\n"); break;
      case 0x2006: printf("[adapter] <- msgLoadStopped\n"); loadStopped = true; break;
      case 0x2007: { int32_t p = 0; (*msg) >> p; printf("[adapter] <- msgLoadProgress(%d)\n", p); break; }
      case 0x2008: { char* uri = 0; bool b = false, f = false;
        (*msg) >> uri; (*msg) >> b; (*msg) >> f;
        printf("[adapter] <- msgLocationChanged(%s, back=%d fwd=%d)\n", uri ? uri : "", b, f);
        if (uri) free(uri); break; }
      default: printf("[adapter] <- msg 0x%x\n", (unsigned)(uint16_t)id); break;
    }
  }
};

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* name = getenv("JIHAD_BS_NAME");
  if (!name || !*name) name = "jihad-browserserver";
  JihadAdapterClient client(name);
  if (!client.connect()) { fprintf(stderr, "[adapter] connect('%s') FAILED\n", name); return 1; }
  printf("[adapter] connected to '%s'\n", name);
  client.start();          // send Connect + OpenUrl now that the socket is up
  client.run();
  // Adapter owns the buffers -> clean them up (Codex P1).
  { int id1 = shmget(client.key1, client.sz, 0); if (id1 >= 0) shmctl(id1, IPC_RMID, nullptr);
    int id2 = shmget(client.key2, client.sz, 0); if (id2 >= 0) shmctl(id2, IPC_RMID, nullptr); }
  printf("[adapter] painted=%d verified=%d\n", client.painted, client.verified);
  bool ok = client.loadStopped && client.verified > 0;
  printf("[adapter] %s\n", ok ? "ROUND-TRIP PASS" : "ROUND-TRIP FAIL");
  // Overscan (scroll pan headroom): with a 4x segment and a taller-than-viewport page the
  // daemon should paint viewport + overscan and report it. Diagnostic always; enforced
  // when JIHAD_EXPECT_OVERSCAN=1 (set by the round-trip scripts once libxul carries
  // jihad_offscreen_render_region — an old engine legitimately degrades to viewport-exact).
  printf("[adapter] overscan: renderedH=%d (viewport %d) renderedY=%d stripNonWhite=%ld alphaBad=%ld\n",
         client.lastRenderedH, client.H, client.lastRenderedY, client.stripNonWhite, client.alphaBad);
  // Review F3: the adapter blits raw words — a single a<255 pixel is a transparent hole
  // in the card. Enforced unconditionally (both the old and new paint paths must hold it).
  if (client.alphaBad > 0) {
    printf("[adapter] ALPHA FAIL (%ld px with a != 0xff)\n", client.alphaBad);
    return 4;
  }
  const char* eo = getenv("JIHAD_EXPECT_OVERSCAN");
  if (eo && *eo == '1') {
    bool over = client.lastRenderedH > client.H && client.stripNonWhite > 100;
    printf("[adapter] %s\n", over ? "OVERSCAN PASS" : "OVERSCAN FAIL");
    if (!over) return 3;
    if (!getenv("JIHAD_URL")) {
      // Review F2: the offset arithmetic is only proven by a frame whose renderedY > 0
      // placing known content at the right buffer row.
      printf("[adapter] %s\n", client.scrollPass ? "SCROLL-OVERSCAN PASS" : "SCROLL-OVERSCAN FAIL");
      if (!client.scrollPass) return 5;
    }
  }
  return ok ? 0 : 2;
}
