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
  explicit JihadAdapterClient(const char* name)
    : YapClient(name), painted(0), verified(0), loadStopped(false),
      W(1024), H(768), sz((int)kHdr + 1024*768*4), key1(0x4a494831), key2(0x4a494832) {}
  int painted, verified;
  bool loadStopped;
  bool wroteImage = false;
  int W, H, sz, key1, key2;

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
    FILE* fp = fopen(path, "wb");
    if (fp) {
      fprintf(fp, "P6\n%d %d\n255\n", W, H);
      printf("[adapter] header says %dx%d zoom=%.3f rendered=%d,%d\n",
             oi->bufferWidth, oi->bufferHeight, oi->contentZoom, oi->renderedX, oi->renderedY);
      for (int i = 0; i < W * H; ++i) {
        unsigned char* p = px + (size_t)i * 4;    // B,G,R,A
        unsigned char rgb[3] = { p[2], p[1], p[0] };
        fwrite(rgb, 1, 3, fp);
      }
      fclose(fp);
      printf("[adapter] wrote %dx%d render to %s\n", W, H, path);
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
    long nb = 0;
    for (int i = 0; i < W * H; ++i) {
      unsigned char* p = b + kHdr + (size_t)i * 4;   // pixels AFTER the header; B,G,R,A
      if (!(p[0] > 240 && p[1] > 240 && p[2] > 240)) ++nb;
    }
    shmdt(b);
    return nb;
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
    if (!url || !*url) url = "data:text/html,<title>Jihad</title>"
      "<body style='background:%23224488;color:white;font:48px sans-serif'>"
      "<h1>Adapter round-trip OK</h1></body>";
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
        printf("[adapter] <- msgPainted(key=0x%x): %ld non-white px in shared buffer\n", (unsigned)k, nb);
        if (nb > 100) ++verified;
        if (nb > 100 && !wroteImage) { writeImage(k); wroteImage = true; }
        returnBuffer(k);   // hand the buffer back so rendering can continue
        if (loadStopped && verified > 0) {
          printf("[adapter] verified rendered frame after load; done\n");
          g_main_loop_quit(mainLoop());
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
  return ok ? 0 : 2;
}
