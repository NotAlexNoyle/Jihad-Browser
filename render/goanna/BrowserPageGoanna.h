/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — Goanna rendering backend.
 *
 * BrowserPageGoanna provides the same public surface that the isis-project
 * BrowserServer dispatch (BrowserServerBase) expects from BrowserPage, but
 * implemented against the UXP / Goanna engine instead of QtWebKit. The YAP IPC
 * contract and the shared-memory framebuffer are unchanged; only the engine
 * behind this class differs.
 *
 * SCAFFOLD: declarations are the intended shape of the Phase-1 port, not a
 * compiling implementation. See PORT-MAP.md and ../../docs/IPC-CONTRACT.md.
 */
#ifndef JIHAD_BROWSERPAGEGOANNA_H
#define JIHAD_BROWSERPAGEGOANNA_H

#include <stdint.h>
#include <string>

// UXP / Goanna embedding interfaces (resolved by the engine build, see build/).
#include "nsCOMPtr.h"
#include "nsIWebBrowser.h"
#include "nsIWebNavigation.h"
#include "nsIBaseWindow.h"
#include "nsIDOMWindowUtils.h"

class BrowserServer;
class YapProxy;
class OffscreenWidget;   // custom nsIWidget, paints into shared framebuffer
class GoannaFrameSink;   // readback -> shmem -> msgPainted
class GoannaListeners;   // nsIWebProgressListener/chrome -> YAP msg…
class InputBridge;       // webOS input -> nsIDOMWindowUtils events

namespace jihad {

class BrowserPageGoanna
{
public:
  BrowserPageGoanna(BrowserServer* server, YapProxy* proxy);
  ~BrowserPageGoanna();

  // --- lifecycle / surface (YAP: connect/thaw/freeze/setWindowSize/...) ---
  bool init(uint32_t virtualW, uint32_t virtualH,
            int shmKey1, int shmKey2, int shmSize);
  bool attachToBuffer(uint32_t virtualW, uint32_t virtualH,
                      int shmKey1, int shmKey2, int shmSize);
  void bufferReturned(int32_t shmKey);
  bool freeze();
  bool thaw(int shmKey1, int shmKey2, int shmSize);
  void setWindowSize(uint32_t w, uint32_t h);
  void setVirtualWindowSize(uint32_t w, uint32_t h);
  void setScrollPosition(int cx, int cy, int cw, int ch);
  void setZoomAndScroll(double zoom, int cx, int cy);
  void setFocus(bool enable);

  // --- navigation / loading (YAP: openUrl/setHtml/back/forward/...) ---
  void openUrl(const char* url);
  void setHTML(const char* url, const char* body);
  void pageBackward();
  void pageForward();
  void pageReload();
  void pageStop();
  void clearHistory();
  bool canGoBackward() const;
  bool canGoForward() const;

  // --- input (YAP: clickAt/keyDown/mouseEvent/gestureEvent/touchEvent) ---
  bool clickAt(uint32_t x, uint32_t y, uint32_t numClicks);
  bool holdAt(uint32_t x, uint32_t y);
  void keyDown(int32_t key, int32_t modifiers, int32_t chr);
  void keyUp(int32_t key, int32_t modifiers, int32_t chr);
  void mouseEvent(int type, int x, int y, int detail);
  void gestureEvent(int type, int x, int y,
                    double scale, double rotation, int cx, int cy);
  void touchEvent(int type, int32_t count, int32_t modifiers,
                  const char* touchesJson);

  // --- find / selection / clipboard ---
  int  findString(const char* str, bool fwd);
  void selectAll();
  void cut();
  bool copy();
  void paste();
  void clearSelection();
  void insertStringAtCursor(const char* text);
  bool isEditing();

  // --- settings ---
  void setUserAgent(const char* ua);
  void setMinFontSize(int pt);
  void settingsJavaScriptEnabled(bool enable);
  void settingsPopupsEnabled(bool enable);
  void setAcceptCookies(bool enable);
  void clearCache();
  void clearCookies();

  // --- render-to-file (YAP sync command) ---
  int renderToFile(const char* filename, int viewX, int viewY, int viewW, int viewH);

  YapProxy* getProxy() { return m_proxy; }

private:
  // Engine handles.
  nsCOMPtr<nsIWebBrowser>    m_webBrowser;
  nsCOMPtr<nsIWebNavigation> m_webNav;
  nsCOMPtr<nsIBaseWindow>    m_baseWindow;
  nsCOMPtr<nsIDOMWindowUtils> m_windowUtils;

  // Backend collaborators (own the offscreen surface, listeners, input).
  OffscreenWidget* m_widget;
  GoannaFrameSink* m_frameSink;
  GoannaListeners* m_listeners;
  InputBridge*     m_input;

  // Server / IPC.
  BrowserServer* m_server;
  YapProxy*      m_proxy;

  // Shared framebuffer state.
  int m_shmKey1, m_shmKey2, m_shmSize;
  int m_windowWidth, m_windowHeight;
  int m_virtualWidth, m_virtualHeight;
  double m_zoomLevel;
  bool m_frozen;
  bool m_focused;
};

} // namespace jihad

#endif // JIHAD_BROWSERPAGEGOANNA_H
