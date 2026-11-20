Read only; change nothing. Adversarial code review of "Find on Page" in a web browser for the HP TouchPad (webOS 3): three user interfaces over one engine daemon (UXP/Goanna, a Firefox 52 fork). The uncommitted working-tree changes (`git diff` in the repository, excluding third_party/) implement the requirements in context/kits/cavekit-find-on-page.md. The files:

- Enyo 1 interface: app/source/FindBar.js, app/source/Browser.js (showFind, find, findAsk, findResult, findClosed), app/source/BrowserApp.js (the "find-result"/"find-open" notifications, listenCtrlF), app/css/browser.css
- Mochi (Enyo 2): app-mochi/source/JihadFindBar.js, app-mochi/source/JihadBrowser.js (find, findRequested, findClosed, findResult, showFindBar, listenCtrlF), app-mochi/source/JihadBrowser.css
- Mojo: app-mojo/app/assistants/main-assistant.js (showFind, hideFind, findAsk, findTyped, findStep, findKeyDown, setFindResult, bindFindHandlers, onPageScrolled), app-mojo/app/views/main/main-scene.html, app-mojo/stylesheets/jihad-browser.css
- NPAPI adapter: render/adapter/BrowserAdapter.cpp js_findInPage (a second argument, backwards; the IPC message FindString already carried a direction and must not change)
- Daemon: render/goanna/BrowserPageGoanna.cpp (findString, jihadJsonString, the Ctrl+F block in keyDown), render/goanna/GoannaRenderPage.h (ForgetFind)
- The page-side counter: render/goanna/components/jihadPageCommands.js (findCount, the "findCount" and "findClear" commands; it runs as chrome JavaScript in the daemon)

Notifications from the daemon are broadcast to every card of the variant; a card takes "find-result" only while its own search is out (a counter), and "find-open" only when it is in front.

Find real defects: wrong behaviour against the kit's criteria, races (a result for an earlier word, a result taken by the wrong card, counters that drift), a stale count or highlight left after Done or a navigation, a crash or exception path, escaping/injection in the JSON or HTML built from page text, a page that can spoof or block the find, focus or keyboard states that strand the bar, and memory or time costs of counting on a large page (the counter's limits: 1000 matches, 300 ms). For each: file:line, the scenario that breaks it, the consequence, and a fix. Rank by severity. Skip style. Under 900 words. Write the report to the file named by -o.
