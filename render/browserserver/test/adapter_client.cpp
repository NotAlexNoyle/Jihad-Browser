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
#include <glib.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

class JihadAdapterClient : public YapClient {
public:
  explicit JihadAdapterClient(const char* name)
    : YapClient(name), painted(0), loadStopped(false) {}
  int painted;
  bool loadStopped;

  void serverConnected() override {}      // YapClient never calls this; we drive from start()

  // Send the initial YAP commands after connect() succeeds.
  void start() {
    printf("[adapter] sending Connect + OpenUrl\n");
    const int W = 1024, H = 768, sz = W * H * 4;
    // Connect (0x1000): w, h, key1, key2, size, identifier
    YapPacket* c = packetCommand();   // YapPacket::operator<< returns void, so no chaining
    (*c) << (int16_t)0x1000;
    (*c) << (int32_t)W; (*c) << (int32_t)H;
    (*c) << (int32_t)0x4a494831; (*c) << (int32_t)0x4a494832;
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
      case 0x2000: { int32_t k = 0; (*msg) >> k;
        printf("[adapter] <- msgPainted(key=0x%x)\n", (unsigned)k); ++painted;
        if (loadStopped) { printf("[adapter] got rendered frame after load; done\n");
                           g_main_loop_quit(mainLoop()); }
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
  printf("[adapter] painted frames received: %d\n", client.painted);
  printf("[adapter] %s\n", (client.loadStopped && client.painted > 0) ? "ROUND-TRIP PASS" : "ROUND-TRIP FAIL");
  return (client.loadStopped && client.painted > 0) ? 0 : 2;
}
