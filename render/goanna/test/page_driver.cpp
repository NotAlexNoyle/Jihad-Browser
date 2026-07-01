/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — driver exercising the GoannaRenderPage backend class:
 * init engine -> create page -> load URL -> render -> read ARGB32 -> shm.
 * This is the shape the daemon's BrowserPageGoanna will use per card.
 *
 * Usage: xvfb-run page_driver <greDir>   ($JIHAD_URL overrides the page)
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "../GoannaRenderPage.h"
#include "../EngineHost.h"

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  const char* url = getenv("JIHAD_URL");
  if (!url || !*url) url = "data:text/html,<title>Jihad</title>"
    "<body style='background:%23224488;color:white;font:48px sans-serif'>"
    "<h1>GoannaRenderPage OK</h1></body>";

  gtk_init(&argc, &argv);

  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[driver] FAIL engine init\n"); return 1; }
  printf("[driver] engine up\n");

  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(1024, 768)) { fprintf(stderr, "[driver] FAIL page create\n"); return 2; }
    printf("[driver] page created 1024x768\n");

    bool ok = page.LoadUrlAndWait(url, 20);
    printf("[driver] load %s: %s -> %s\n", url, ok ? "DONE" : "TIMEOUT", page.CurrentUri().c_str());
    page.PumpFor(1500);   // let it paint

    size_t bytes = (size_t)page.Width() * page.Height() * 4;
    unsigned char* buf = (unsigned char*)malloc(bytes);
    long nb = page.ReadPixels(buf, bytes);
    printf("[driver] ReadPixels: %ld non-white px into %zu-byte ARGB32 buffer\n", nb, bytes);

    if (nb > 100) {
      int shmid = shmget(IPC_PRIVATE, bytes, IPC_CREAT | 0600);
      if (shmid >= 0) {
        void* seg = shmat(shmid, nullptr, 0);
        if (seg != (void*)-1) { memcpy(seg, buf, bytes); shmdt(seg); }
        printf("[driver] wrote %zu bytes to shm id=%d (daemon would msgPainted here)\n", bytes, shmid);
        shmctl(shmid, IPC_RMID, nullptr);
      }
      printf("[driver] RENDER PASS\n");
    } else {
      printf("[driver] RENDER FAIL (blank)\n");
    }
    free(buf);
    // page destructor tears down browser/window before engine shutdown.
  }

  host.Shutdown();
  printf("[driver] done\n");
  return 0;
}
