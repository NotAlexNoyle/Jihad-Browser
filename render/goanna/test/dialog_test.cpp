/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — JS dialog interception test (domain G / R3).
 * A page fires alert('hello-jihad') then confirm('go') on load. Our installed
 * prompter must capture both (bypassing the missing chrome dialog) and answer
 * confirm=true, which turns the blue background green. We assert the alert text
 * was captured, confirm was seen, and the page reacted to the reply.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"
#include "../DialogService.h"

using jihad::DialogKind;
using jihad::DialogReply;

struct RecSink : jihad::DialogSink {
  std::string lastAlert;
  int alerts = 0, confirms = 0, prompts = 0;
  void OnDialog(DialogKind k, const char* text, DialogReply* reply) override {
    if (k == DialogKind::Alert)   { lastAlert = text ? text : ""; ++alerts; }
    else if (k == DialogKind::Confirm) { ++confirms; reply->accept = true; }
    else { ++prompts; reply->accept = true; reply->promptValue = "x"; }
  }
};

static long countColor(unsigned char* buf, int n,
                       int rmn,int rmx,int gmn,int gmx,int bmn,int bmx) {
  long c = 0;
  for (int i = 0; i < n; ++i) { unsigned char* p = buf + (size_t)i*4;
    int b=p[0], g=p[1], r=p[2];
    if (r>=rmn&&r<=rmx && g>=gmn&&g<=gmx && b>=bmn&&b<=bmx) ++c; }
  return c;
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[dialog] engine init FAIL\n"); return 1; }
  if (!jihad::InstallDialogService()) { fprintf(stderr, "[dialog] install FAIL\n"); return 1; }
  RecSink sink;
  jihad::SetDialogSink(&sink);
  printf("[dialog] engine up + prompter installed\n");

  int rc = 3;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(1024, 768)) return 2;
    const char* url =
      "data:text/html,<body style='margin:0;width:100vw;height:100vh;background:%230000ff' "
      "onload=\"alert('hello-jihad'); if(confirm('go'))"
      "document.body.style.background='%2300ff00';\"></body>";
    page.LoadUrlAndWait(url, 20);
    page.PumpFor(2000);

    const int N = page.Width() * page.Height();
    size_t sz = (size_t)N * 4;
    unsigned char* buf = (unsigned char*)malloc(sz);
    page.ReadPixels(buf, sz);
    long green = countColor(buf, N, 0,80, 180,255, 0,80);
    free(buf);

    printf("[dialog] alerts=%d text='%s' confirms=%d green=%ld\n",
           sink.alerts, sink.lastAlert.c_str(), sink.confirms, green);
    bool ok = sink.alerts >= 1 && sink.lastAlert == "hello-jihad" &&
              sink.confirms >= 1 && green > 100000;
    printf("[dialog] %s\n", ok ? "DIALOG PASS" : "DIALOG FAIL");
    rc = ok ? 0 : 4;
  }
  jihad::SetDialogSink(nullptr);
  host.Shutdown();
  return rc;
}
