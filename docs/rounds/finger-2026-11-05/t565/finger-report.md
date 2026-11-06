# Finger suite report — 2026-11-06T01:47:08

First run 2026-11-05T18:57:49; runs marked (rerun) were run again by `--merge`, and the device-check below is the last run's.

Shells: enyo, mochi, mojo. Scripts: 3. 23192 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | addon-site | **pass** | 8 ok, 0 failed | WAIT ok 21-pref lightweightThemes.isThemeSelected = [false] | cavekit-addons-site.md R7 |
| enyo | addons-lifecycle | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/profile/extensions | cavekit-addons-extensions.md R2 |
| enyo | addons-list-pan | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/profile/extensions | cavekit-chrome-look.md R2 |
| enyo | addons-page | **pass** | 7 ok, 0 failed | CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill] | cavekit-addons-extensions.md R2 |
| enyo | addons-search-hide-key | **pass** | 8 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| enyo | address-select-all | **pass** | 4 ok, 0 failed | CHECK ok 16-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| enyo | appmenu-find (rerun) | **pass** | 4 ok, 0 failed | WAIT ok 16-log: [jihad-bs] find 'zebrafind' -> 1 | cavekit-ui-shell.md R7 |
| enyo | appmenu-print (rerun) | **pass** | 4 ok, 0 failed | WAIT ok 15-differs 0,200,756,960; frames: kept/enyo-appmenu-print-print-dialog_.jpg | cavekit-ui-shell.md R7 |
| enyo | appmenu-settings-keyboard-down (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 19-content: region 0,100-768,700: colours 499 hash 333a5bcc rows 600 uniform 0 light none blue none; page-title on the screen at 20,100-744,134, text holds [Settings] | cavekit-chrome-look.md R4 |
| enyo | audio-heard | **a person's** | 0 ok, 0 failed | hearing the speaker; the route and the mixer are measured by script (cavekit-addons-extensions R7) | cavekit-device-build.md R4 |
| enyo | busy-script-stop | **pass** | 4 ok, 0 failed | CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36; frames: kept/enyo-busy-script-stop-busy-dialog_.jpg | cavekit-modern-web.md R2 |
| enyo | card-dialog | **pass** | 7 ok, 0 failed | WAIT ok 18-text r = [c: false] | cavekit-browser-services.md R3 |
| enyo | card-select (rerun) | **pass** | 1 ok, 0 failed | WAIT ok 9-text picked = [picked: banana] | cavekit-device-build.md R4 |
| enyo | chrome-buttons | **pass** | 5 ok, 0 failed | WAIT ok 18-differs 0,100,756,700; frames: kept/enyo-chrome-buttons-new-card_.jpg | cavekit-device-build.md R4 |
| enyo | chrome-history | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/enyo-chrome-history-history-panel_.jpg | cavekit-device-build.md R4 |
| enyo | chrome-padlock | **pass** | 3 ok, 0 failed | WAIT ok 13-differs 0,100,756,960; frames: kept/enyo-chrome-padlock-padlock_.jpg | cavekit-ui-shell.md R8 |
| enyo | chrome-share | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/enyo-chrome-share-share-menu_.jpg | cavekit-device-build.md R4 |
| enyo | ddg-results | **pass** | 3 ok, 0 failed | CHECK ok 10-content: region 0,100-756,900: colours 1256 hash 2e4e0810 rows 800 uniform 188 light 100-895 blue 148-702; sel:body on the screen at 0,82-768,2616 | cavekit-modern-web.md R2 |
| enyo | dialog-slow-answer | **pass** | 4 ok, 0 failed | WAIT ok 14-text r = [c: true] | cavekit-finger-testing.md R4 |
| enyo | download-finishes | **pass** | 1 ok, 0 failed | WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-4 | cavekit-device-build.md R4 |
| enyo | drag-and-pinch-page | **pass** | 7 ok, 0 failed | CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15) | cavekit-input-bridging.md R3 |
| enyo | etsy-search | **pass** | 5 ok, 0 failed | CHECK ok 12-content: region 0,100-768,900: colours 4096 hash 2f575159 rows 800 uniform 138 light 100-882 blue none; sel:body on the screen at 0,82-768,8670 | cavekit-modern-web.md R2 |
| enyo | everyday-browsing | **pass** | 5 ok, 0 failed | WAIT ok 17-url: https://duckduckgo.com/ | cavekit-engine-update.md R8 |
| enyo | express-burst-wiki | **pass** | 5 ok, 0 failed | CHECK ok 17-content: region 0,100-756,700: colours 1051 hash 1f0b551c rows 600 uniform 0 light 101-699 blue 100-583; sel:body on the screen at -531,-1021-2065,2176 | cavekit-gpu-rendering.md R3 |
| enyo | flash-sound | **a person's** | 0 ok, 0 failed | hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route) | cavekit-engine-update.md R8 |
| enyo | fonts-pick | **pass** | 5 ok, 0 failed | WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.] | cavekit-preferences-ui.md R9 |
| enyo | forget-identities | **pass** | 3 ok, 0 failed | WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.] | cavekit-site-identity.md R1 |
| enyo | frame-field | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| enyo | keyboard-address | **pass** | 2 ok, 0 failed | CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| enyo | keyboard-arbitration | **not run** | 0 ok, 0 failed | hardware-blocked (no physical keyboard exists) |  |
| enyo | keyboard-fields (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 17-visible: field on the screen at 16,284-522,319 (the reach ends at 600, keyboard 1) | cavekit-input-bridging.md R2 |
| enyo | long-press-image (rerun) | **pass** | 2 ok, 0 failed | WAIT ok 12-differs 0,100,756,900; frames: kept/enyo-long-press-image-hold-image_.jpg | cavekit-ui-shell.md R7 |
| enyo | long-press-menu (rerun) | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| enyo | long-press-text-field (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html; frames: kept/enyo-long-press-text-field-hold-text_.jpg, kept/enyo-long-press-text-field-hold-field_.jpg | cavekit-ui-shell.md R7 |
| enyo | mobile-wiki-pinch | **pass** | 8 ok, 0 failed | CHECK ok 18-content: region 0,100-756,700: colours 4096 hash 752ea776 rows 600 uniform 8 light 100-699 blue none; sel:body on the screen at -348,-203-1687,2302 | cavekit-mojo-ui.md R6 |
| enyo | pan-long-drag | **pass** | 8 ok, 0 failed | CHECK ok 19-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| enyo | pan-long-flick | **known** | 2 ok, 1 failed | the open defect T-567: CHECK FAIL 17-stream-uniform: uniform 46 (want at most 0) | cavekit-gpu-rendering.md R3 |
| enyo | pan-speeds (rerun) | **known** | 3 ok, 2 failed | the open defect T-567, T-567: CHECK FAIL 19-stream-stale: stale 1 (want at most 0) / CHECK FAIL 20-stream-uniform: uniform 45 (want at most 0) | cavekit-finger-testing.md R4 |
| enyo | persona-cycle | **pass** | 12 ok, 0 failed | persona file: put | cavekit-personas-ui.md R6 |
| enyo | persona-wear | **pass** | 7 ok, 0 failed | WAIT ok 18-pref lightweightThemes.isThemeSelected = [false] | cavekit-personas-ui.md R6 |
| enyo | personas-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| enyo | personas-page-steady | **pass** | 3 ok, 0 failed | CHECK ok 15-stream-uniform: uniform 0 (at most 0) | cavekit-personas-ui.md R9 |
| enyo | proxy-login (rerun) | **pass** | 11 ok, 0 failed | auth proxy: running; frames: kept/enyo-proxy-login-auth-dialog_.jpg | cavekit-proxies-certs.md R5 |
| enyo | proxy-own | **pass** | 8 ok, 0 failed | auth proxy: running | cavekit-proxies-certs.md R1 |
| enyo | proxy-section-redraw | **pass** | 5 ok, 0 failed | WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input | cavekit-proxies-certs.md R1 |
| enyo | relay-choice | **pass** | 6 ok, 0 failed | CHECK ok 17-text-has: sel:p = [This domain is for use in documentation examples without needing permission. This is not a service; avoid relying on it for testing and monitoring purposes.] holds [documentation] | cavekit-proxies-certs.md R2 |
| enyo | rotate | **a person's** | 0 ok, 0 failed | the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side | cavekit-device-build.md R4 |
| enyo | script-hold-zoom | **pass** | 6 ok, 0 failed | CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2) | cavekit-gpu-rendering.md R3 |
| enyo | settings-every-control | **by a tool** | 0 ok, 0 failed | tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt) | cavekit-preferences-ui.md R6 |
| enyo | settings-field-saves | **pass** | 8 ok, 0 failed | CHECK ok 20-region-equal: region 416,644-735,704: colours 49 hash 34233dc3 rows 60 uniform 0 light 644-703 blue none = the sample's hash | cavekit-chrome-look.md R4 |
| enyo | settings-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| enyo | settings-int-emptied | **pass** | 5 ok, 0 failed | WAIT ok 16-pref browser.sessionhistory.max_entries = [20] | cavekit-preferences-ui.md R6 |
| enyo | settings-persist | **pass** | 5 ok, 0 failed | WAIT ok 16-pref general.smoothScroll = [true] | cavekit-preferences-ui.md R2 |
| enyo | settings-tabs | **pass** | 4 ok, 0 failed | CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content] | cavekit-preferences-ui.md R6 |
| enyo | settings-typing | **pass** | 5 ok, 0 failed | CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-preferences-ui.md R6 |
| enyo | ssl-confirm | **pass** | 2 ok, 0 failed | CHECK ok 13-dialog: [jihad-bs] dialog ssl-confirm -> cancel (answer=0); frames: kept/enyo-ssl-confirm-ssl-dialog_.jpg | cavekit-device-build.md R4 |
| enyo | tabbed-browsing | **not run** | 0 ok, 0 failed | not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word) | cavekit-tabbed-browsing.md R1 |
| enyo | touchpad-go | **not run** | 0 ok, 0 failed | owner-accepted without hardware (device-build R6) | cavekit-device-build.md R6 |
| enyo | typing-caret | **pass** | 4 ok, 0 failed | CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| enyo | video-controls | **pass** | 6 ok, 0 failed | CHECK ok 16-region-equal: region 16,125-517,405: colours 2717 hash 018a25e1 rows 280 uniform 0 light 392-397 blue 335-371 = the sample's hash | cavekit-gre-widgets.md R1 |
| enyo | webgl-cube | **pass** | 7 ok, 0 failed | WAIT ok 16-changed c | cavekit-gpu-rendering.md R8 |
| enyo | webone | **not run** | 0 ok, 0 failed | needs webone, keyboard-map (run with --with webone,keyboard-map) | cavekit-proxies-certs.md R3 |
| enyo | wikipedia-keyboard-zoom | **pass** | 10 ok, 0 failed | CHECK ok 20-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad] | cavekit-mochi-ui.md R6 |
| enyo | wikipedia-zoom | **pass** | 10 ok, 0 failed | CHECK ok 27-stream-uniform: uniform 0 (at most 0) | cavekit-gpu-rendering.md R6 |
| enyo | youtube-mobile | **pass** | 3 ok, 0 failed | WAIT ok 11-changed 0,130,756,560; frames: kept/enyo-youtube-mobile-playing_.jpg | cavekit-modern-web.md R2 |
| mochi | addon-site | **pass** | 8 ok, 0 failed | WAIT ok 21-pref lightweightThemes.isThemeSelected = [false] | cavekit-addons-site.md R7 |
| mochi | addons-lifecycle | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser-mochi/profile/extensions | cavekit-addons-extensions.md R2 |
| mochi | addons-list-pan | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser-mochi/profile/extensions | cavekit-chrome-look.md R2 |
| mochi | addons-page | **pass** | 7 ok, 0 failed | CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill] | cavekit-addons-extensions.md R2 |
| mochi | addons-search-hide-key | **pass** | 8 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mochi | address-select-all (rerun) | **pass** | 4 ok, 0 failed | CHECK ok 16-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mochi | appmenu-find (rerun) | **pass** | 4 ok, 0 failed | WAIT ok 16-log: [jihad-bs] find 'zebrafind' -> 1 | cavekit-ui-shell.md R7 |
| mochi | appmenu-print (rerun) | **pass** | 4 ok, 0 failed | WAIT ok 15-differs 0,200,756,960; frames: kept/mochi-appmenu-print-print-dialog_.jpg | cavekit-ui-shell.md R7 |
| mochi | appmenu-settings-keyboard-down (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 19-content: region 0,100-768,700: colours 787 hash 3b16afe0 rows 600 uniform 0 light none blue none; page-title on the screen at 20,96-744,130, text holds [Settings] | cavekit-chrome-look.md R4 |
| mochi | audio-heard | **a person's** | 0 ok, 0 failed | hearing the speaker; the route and the mixer are measured by script (cavekit-addons-extensions R7) | cavekit-device-build.md R4 |
| mochi | busy-script-stop | **pass** | 4 ok, 0 failed | CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36; frames: kept/mochi-busy-script-stop-busy-dialog_.jpg | cavekit-modern-web.md R2 |
| mochi | card-dialog | **pass** | 7 ok, 0 failed | WAIT ok 18-text r = [c: false] | cavekit-browser-services.md R3 |
| mochi | card-select (rerun) | **pass** | 1 ok, 0 failed | WAIT ok 9-text picked = [picked: banana] | cavekit-device-build.md R4 |
| mochi | chrome-buttons | **pass** | 5 ok, 0 failed | WAIT ok 18-differs 0,100,756,700; frames: kept/mochi-chrome-buttons-new-card_.jpg | cavekit-device-build.md R4 |
| mochi | chrome-history | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/mochi-chrome-history-history-panel_.jpg | cavekit-device-build.md R4 |
| mochi | chrome-padlock | **pass** | 3 ok, 0 failed | WAIT ok 13-differs 0,100,756,960; frames: kept/mochi-chrome-padlock-padlock_.jpg | cavekit-ui-shell.md R8 |
| mochi | chrome-share | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/mochi-chrome-share-share-menu_.jpg | cavekit-device-build.md R4 |
| mochi | ddg-results | **pass** | 3 ok, 0 failed | CHECK ok 10-content: region 0,100-756,900: colours 1246 hash 31a0caa8 rows 800 uniform 187 light 100-899 blue 143-698; sel:body on the screen at 0,78-768,2612 | cavekit-modern-web.md R2 |
| mochi | dialog-slow-answer | **pass** | 4 ok, 0 failed | WAIT ok 14-text r = [c: true] | cavekit-finger-testing.md R4 |
| mochi | download-finishes | **pass** | 1 ok, 0 failed | WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-5 | cavekit-device-build.md R4 |
| mochi | drag-and-pinch-page | **pass** | 7 ok, 0 failed | CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15) | cavekit-input-bridging.md R3 |
| mochi | etsy-search (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 12-content: region 0,100-768,900: colours 4096 hash 8f6c2115 rows 800 uniform 127 light 100-899 blue 330-392; sel:body on the screen at 0,78-768,8681 | cavekit-modern-web.md R2 |
| mochi | everyday-browsing | **pass** | 5 ok, 0 failed | WAIT ok 17-url: https://duckduckgo.com/ | cavekit-engine-update.md R8 |
| mochi | express-burst-wiki | **pass** | 5 ok, 0 failed | CHECK ok 17-content: region 0,100-756,700: colours 999 hash bd108dbc rows 600 uniform 0 light 100-699 blue 523-541; sel:body on the screen at -530,-1041-2066,2170 | cavekit-gpu-rendering.md R3 |
| mochi | flash-sound | **a person's** | 0 ok, 0 failed | hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route) | cavekit-engine-update.md R8 |
| mochi | fonts-pick | **pass** | 5 ok, 0 failed | WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.] | cavekit-preferences-ui.md R9 |
| mochi | forget-identities | **pass** | 3 ok, 0 failed | WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.] | cavekit-site-identity.md R1 |
| mochi | frame-field | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mochi | keyboard-address (rerun) | **pass** | 2 ok, 0 failed | CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| mochi | keyboard-arbitration | **not run** | 0 ok, 0 failed | hardware-blocked (no physical keyboard exists) |  |
| mochi | keyboard-fields (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 17-visible: field on the screen at 16,280-522,315 (the reach ends at 600, keyboard 1) | cavekit-input-bridging.md R2 |
| mochi | long-press-image | **pass** | 2 ok, 0 failed | WAIT ok 12-differs 0,100,756,900; frames: kept/mochi-long-press-image-hold-image_.jpg | cavekit-ui-shell.md R7 |
| mochi | long-press-menu (rerun) | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| mochi | long-press-text-field (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html; frames: kept/mochi-long-press-text-field-hold-text_.jpg, kept/mochi-long-press-text-field-hold-field_.jpg | cavekit-ui-shell.md R7 |
| mochi | mobile-wiki-pinch | **pass** | 8 ok, 0 failed | CHECK ok 18-content: region 0,100-756,700: colours 4096 hash 525844f9 rows 600 uniform 2 light 100-699 blue none; sel:body on the screen at -349,-212-1702,2324 | cavekit-mojo-ui.md R6 |
| mochi | mochi-paste (rerun) | **pass** | 4 ok, 0 failed | CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused); frames: kept/mochi-mochi-paste-mochi-field-sheet_.jpg | cavekit-mochi-ui.md R2 |
| mochi | pan-long-drag | **pass** | 8 ok, 0 failed | CHECK ok 19-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| mochi | pan-long-flick (rerun) | **known** | 1 ok, 2 failed | the open defect T-567, T-567: CHECK FAIL 16-stream-stale: stale 1 (want at most 0) / CHECK FAIL 17-stream-uniform: uniform 38 (want at most 0) | cavekit-gpu-rendering.md R3 |
| mochi | pan-speeds | **known** | 4 ok, 1 failed | the open defect T-567: CHECK FAIL 20-stream-uniform: uniform 47 (want at most 0) | cavekit-finger-testing.md R4 |
| mochi | persona-cycle | **pass** | 12 ok, 0 failed | persona file: put | cavekit-personas-ui.md R6 |
| mochi | persona-wear | **pass** | 7 ok, 0 failed | WAIT ok 18-pref lightweightThemes.isThemeSelected = [false] | cavekit-personas-ui.md R6 |
| mochi | personas-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| mochi | personas-page-steady | **pass** | 3 ok, 0 failed | CHECK ok 15-stream-uniform: uniform 0 (at most 0) | cavekit-personas-ui.md R9 |
| mochi | proxy-login (rerun) | **pass** | 11 ok, 0 failed | auth proxy: running; frames: kept/mochi-proxy-login-auth-dialog_.jpg | cavekit-proxies-certs.md R5 |
| mochi | proxy-own | **pass** | 8 ok, 0 failed | auth proxy: running | cavekit-proxies-certs.md R1 |
| mochi | proxy-section-redraw | **pass** | 5 ok, 0 failed | WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input | cavekit-proxies-certs.md R1 |
| mochi | relay-choice | **pass** | 6 ok, 0 failed | CHECK ok 17-text-has: sel:p = [This domain is for use in documentation examples without needing permission. This is not a service; avoid relying on it for testing and monitoring purposes.] holds [documentation] | cavekit-proxies-certs.md R2 |
| mochi | rotate | **a person's** | 0 ok, 0 failed | the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side | cavekit-device-build.md R4 |
| mochi | script-hold-zoom | **pass** | 6 ok, 0 failed | CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2) | cavekit-gpu-rendering.md R3 |
| mochi | settings-every-control | **by a tool** | 0 ok, 0 failed | tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt) | cavekit-preferences-ui.md R6 |
| mochi | settings-field-saves | **pass** | 8 ok, 0 failed | CHECK ok 20-region-equal: region 416,645-735,705: colours 44 hash 9db921ae rows 60 uniform 0 light 645-704 blue none = the sample's hash | cavekit-chrome-look.md R4 |
| mochi | settings-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| mochi | settings-int-emptied | **pass** | 5 ok, 0 failed | WAIT ok 16-pref browser.sessionhistory.max_entries = [20] | cavekit-preferences-ui.md R6 |
| mochi | settings-persist | **pass** | 5 ok, 0 failed | WAIT ok 16-pref general.smoothScroll = [true] | cavekit-preferences-ui.md R2 |
| mochi | settings-tabs | **pass** | 4 ok, 0 failed | CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content] | cavekit-preferences-ui.md R6 |
| mochi | settings-typing (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-preferences-ui.md R6 |
| mochi | ssl-confirm | **pass** | 2 ok, 0 failed | CHECK ok 13-dialog: [jihad-bs] dialog ssl-confirm -> cancel (answer=0); frames: kept/mochi-ssl-confirm-ssl-dialog_.jpg | cavekit-device-build.md R4 |
| mochi | tabbed-browsing | **not run** | 0 ok, 0 failed | not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word) | cavekit-tabbed-browsing.md R1 |
| mochi | touchpad-go | **not run** | 0 ok, 0 failed | owner-accepted without hardware (device-build R6) | cavekit-device-build.md R6 |
| mochi | typing-caret (rerun) | **pass** | 4 ok, 0 failed | CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| mochi | video-controls | **pass** | 6 ok, 0 failed | CHECK ok 16-region-equal: region 16,121-517,401: colours 2507 hash 33085829 rows 280 uniform 0 light 388-393 blue none = the sample's hash | cavekit-gre-widgets.md R1 |
| mochi | webgl-cube | **pass** | 7 ok, 0 failed | WAIT ok 16-changed c | cavekit-gpu-rendering.md R8 |
| mochi | webone | **not run** | 0 ok, 0 failed | needs webone, keyboard-map (run with --with webone,keyboard-map) | cavekit-proxies-certs.md R3 |
| mochi | wikipedia-keyboard-zoom | **pass** | 10 ok, 0 failed | CHECK ok 20-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad] | cavekit-mochi-ui.md R6 |
| mochi | wikipedia-zoom | **pass** | 10 ok, 0 failed | CHECK ok 27-stream-uniform: uniform 0 (at most 0) | cavekit-gpu-rendering.md R6 |
| mochi | youtube-mobile | **pass** | 3 ok, 0 failed | WAIT ok 11-changed 0,130,756,560; frames: kept/mochi-youtube-mobile-playing_.jpg | cavekit-modern-web.md R2 |
| mojo | addon-site (rerun) | **pass** | 8 ok, 0 failed | WAIT ok 21-pref lightweightThemes.isThemeSelected = [false] | cavekit-addons-site.md R7 |
| mojo | addons-lifecycle (rerun) | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser-mojo/profile/extensions | cavekit-addons-extensions.md R2 |
| mojo | addons-list-pan (rerun) | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser-mojo/profile/extensions | cavekit-chrome-look.md R2 |
| mojo | addons-page (rerun) | **pass** | 7 ok, 0 failed | CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill] | cavekit-addons-extensions.md R2 |
| mojo | addons-search-hide-key (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mojo | appmenu-find (rerun) | **pass** | 4 ok, 0 failed | WAIT ok 16-log: [jihad-bs] find 'Zebrafind' -> 1 | cavekit-ui-shell.md R7 |
| mojo | appmenu-print (rerun) | **pass** | 4 ok, 0 failed | WAIT ok 15-differs 0,200,756,960; frames: kept/mojo-appmenu-print-print-dialog_.jpg | cavekit-ui-shell.md R7 |
| mojo | appmenu-settings-keyboard-down (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 19-content: region 0,100-768,700: colours 448 hash 1c4c8669 rows 600 uniform 0 light none blue none; page-title on the screen at 20,91-744,125, text holds [Settings] | cavekit-chrome-look.md R4 |
| mojo | audio-heard (rerun) | **a person's** | 0 ok, 0 failed | hearing the speaker; the route and the mixer are measured by script (cavekit-addons-extensions R7) | cavekit-device-build.md R4 |
| mojo | busy-script-stop (rerun) | **pass** | 4 ok, 0 failed | CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36; frames: kept/mojo-busy-script-stop-busy-dialog_.jpg | cavekit-modern-web.md R2 |
| mojo | card-dialog (rerun) | **pass** | 7 ok, 0 failed | WAIT ok 18-text r = [c: false] | cavekit-browser-services.md R3 |
| mojo | card-select (rerun) | **pass** | 1 ok, 0 failed | WAIT ok 9-text picked = [picked: banana] | cavekit-device-build.md R4 |
| mojo | chrome-buttons (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 18-differs 0,100,756,700; frames: kept/mojo-chrome-buttons-new-card_.jpg | cavekit-device-build.md R4 |
| mojo | chrome-history (rerun) | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/mojo-chrome-history-history-panel_.jpg | cavekit-device-build.md R4 |
| mojo | chrome-padlock (rerun) | **pass** | 3 ok, 0 failed | WAIT ok 13-differs 0,100,756,960; frames: kept/mojo-chrome-padlock-padlock_.jpg | cavekit-ui-shell.md R8 |
| mojo | chrome-share (rerun) | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/mojo-chrome-share-share-menu_.jpg | cavekit-device-build.md R4 |
| mojo | ddg-results (rerun) | **pass** | 3 ok, 0 failed | CHECK ok 10-content: region 0,100-756,900: colours 1055 hash 336e0e09 rows 800 uniform 193 light 100-899 blue 138-693; sel:body on the screen at 0,73-768,2623 | cavekit-modern-web.md R2 |
| mojo | dialog-slow-answer (rerun) | **pass** | 4 ok, 0 failed | WAIT ok 14-text r = [c: true] | cavekit-finger-testing.md R4 |
| mojo | download-finishes (rerun) | **pass** | 1 ok, 0 failed | WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-7 | cavekit-device-build.md R4 |
| mojo | drag-and-pinch-page (rerun) | **pass** | 7 ok, 0 failed | CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15) | cavekit-input-bridging.md R3 |
| mojo | etsy-search (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 12-content: region 0,100-768,900: colours 4096 hash 65fedf66 rows 800 uniform 158 light 100-899 blue 502-537; sel:body on the screen at 0,73-768,8627 | cavekit-modern-web.md R2 |
| mojo | everyday-browsing (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 17-url: https://duckduckgo.com/ | cavekit-engine-update.md R8 |
| mojo | express-burst-wiki (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 17-content: region 0,100-756,700: colours 748 hash 1d49e678 rows 600 uniform 0 light 100-698 blue 477-699; sel:body on the screen at -531,-1090-2065,2291 | cavekit-gpu-rendering.md R3 |
| mojo | flash-sound (rerun) | **a person's** | 0 ok, 0 failed | hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route) | cavekit-engine-update.md R8 |
| mojo | fonts-pick (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.] | cavekit-preferences-ui.md R9 |
| mojo | forget-identities (rerun) | **pass** | 3 ok, 0 failed | WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.] | cavekit-site-identity.md R1 |
| mojo | frame-field (rerun) | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mojo | keyboard-address (rerun) | **pass** | 2 ok, 0 failed | CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| mojo | keyboard-arbitration (rerun) | **not run** | 0 ok, 0 failed | hardware-blocked (no physical keyboard exists) |  |
| mojo | keyboard-fields (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 17-visible: field on the screen at 16,275-522,310 (the reach ends at 600, keyboard 1) | cavekit-input-bridging.md R2 |
| mojo | long-press-image (rerun) | **pass** | 2 ok, 0 failed | WAIT ok 12-differs 0,100,756,900; frames: kept/mojo-long-press-image-hold-image_.jpg | cavekit-ui-shell.md R7 |
| mojo | long-press-menu (rerun) | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| mojo | long-press-text-field (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html; frames: kept/mojo-long-press-text-field-hold-text_.jpg, kept/mojo-long-press-text-field-hold-field_.jpg | cavekit-ui-shell.md R7 |
| mojo | mobile-wiki-pinch (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 18-content: region 0,100-756,700: colours 4096 hash efbcf2fb rows 600 uniform 0 light 100-699 blue none; sel:body on the screen at -346,-194-1680,2444 | cavekit-mojo-ui.md R6 |
| mojo | mojo-address-typing (rerun) | **pass** | 3 ok, 0 failed | CHECK ok 14-url: load done uri=https://lite.duckduckgo.com/lite/?q=webos | cavekit-mojo-ui.md R6 |
| mojo | mojo-launch-from-app (rerun) | **a person's** | 0 ok, 0 failed | a launch from another app is not emulated (a launch with a target from the command line showed the page with no keyboard, 2026-10-24) | cavekit-chrome-look.md R4 |
| mojo | pan-long-drag (rerun) | **pass** | 9 ok, 0 failed | CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| mojo | pan-long-flick (rerun) | **known** | 2 ok, 1 failed | the open defect T-567: CHECK FAIL 17-stream-uniform: uniform 10 (want at most 0) | cavekit-gpu-rendering.md R3 |
| mojo | pan-speeds (rerun) | **known** | 4 ok, 1 failed | the open defect T-567: CHECK FAIL 20-stream-uniform: uniform 14 (want at most 0) | cavekit-finger-testing.md R4 |
| mojo | persona-cycle (rerun) | **pass** | 12 ok, 0 failed | persona file: put | cavekit-personas-ui.md R6 |
| mojo | persona-wear (rerun) | **pass** | 7 ok, 0 failed | WAIT ok 18-pref lightweightThemes.isThemeSelected = [false] | cavekit-personas-ui.md R6 |
| mojo | personas-flick (rerun) | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| mojo | personas-page-steady (rerun) | **pass** | 3 ok, 0 failed | CHECK ok 15-stream-uniform: uniform 0 (at most 0) | cavekit-personas-ui.md R9 |
| mojo | proxy-login-mojo (rerun) | **pass** | 6 ok, 0 failed | auth proxy: running; frames: kept/mojo-proxy-login-mojo-auth-sheet_.jpg | cavekit-proxies-certs.md R5 |
| mojo | proxy-own (rerun) | **pass** | 8 ok, 0 failed | auth proxy: running | cavekit-proxies-certs.md R1 |
| mojo | proxy-section-redraw (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input | cavekit-proxies-certs.md R1 |
| mojo | relay-choice (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 17-text-has: sel:p = [This domain is for use in documentation examples without needing permission. This is not a service; avoid relying on it for testing and monitoring purposes.] holds [documentation] | cavekit-proxies-certs.md R2 |
| mojo | rotate (rerun) | **a person's** | 0 ok, 0 failed | the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side | cavekit-device-build.md R4 |
| mojo | script-hold-zoom (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2) | cavekit-gpu-rendering.md R3 |
| mojo | settings-every-control (rerun) | **by a tool** | 0 ok, 0 failed | tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt) | cavekit-preferences-ui.md R6 |
| mojo | settings-field-saves (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 20-region-equal: region 416,643-735,703: colours 38 hash c1c06359 rows 60 uniform 0 light 643-702 blue none = the sample's hash | cavekit-chrome-look.md R4 |
| mojo | settings-flick (rerun) | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| mojo | settings-int-emptied (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 16-pref browser.sessionhistory.max_entries = [20] | cavekit-preferences-ui.md R6 |
| mojo | settings-persist (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 16-pref general.smoothScroll = [true] | cavekit-preferences-ui.md R2 |
| mojo | settings-tabs (rerun) | **pass** | 4 ok, 0 failed | CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content] | cavekit-preferences-ui.md R6 |
| mojo | settings-typing (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-preferences-ui.md R6 |
| mojo | ssl-confirm (rerun) | **pass** | 2 ok, 0 failed | CHECK ok 13-dialog: [jihad-bs] dialog ssl-confirm -> cancel (answer=0); frames: kept/mojo-ssl-confirm-ssl-dialog_.jpg | cavekit-device-build.md R4 |
| mojo | tabbed-browsing (rerun) | **not run** | 0 ok, 0 failed | not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word) | cavekit-tabbed-browsing.md R1 |
| mojo | touchpad-go (rerun) | **not run** | 0 ok, 0 failed | owner-accepted without hardware (device-build R6) | cavekit-device-build.md R6 |
| mojo | typing-caret (rerun) | **pass** | 4 ok, 0 failed | CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| mojo | video-controls (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 16-region-equal: region 16,116-517,396: colours 2442 hash dd0c46e5 rows 280 uniform 0 light 383-388 blue none = the sample's hash | cavekit-gre-widgets.md R1 |
| mojo | webgl-cube (rerun) | **pass** | 7 ok, 0 failed | WAIT ok 16-changed c | cavekit-gpu-rendering.md R8 |
| mojo | webone (rerun) | **not run** | 0 ok, 0 failed | needs webone, keyboard-map (run with --with webone,keyboard-map) | cavekit-proxies-certs.md R3 |
| mojo | wikipedia-keyboard-zoom (rerun) | **pass** | 10 ok, 0 failed | CHECK ok 20-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad] | cavekit-mochi-ui.md R6 |
| mojo | wikipedia-zoom (rerun) | **known** | 9 ok, 1 failed | the open defect T-567: CHECK FAIL 27-stream-uniform: uniform 6 (want at most 0) | cavekit-gpu-rendering.md R6 |
| mojo | youtube-mobile (rerun) | **pass** | 3 ok, 0 failed | WAIT ok 11-changed 0,130,756,560; frames: kept/mojo-youtube-mobile-playing_.jpg | cavekit-modern-web.md R2 |

## The device at the end

```
DEVICE ok job-enyo: (1182901/753919845) jihad (start) running, process 1361
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (1182907/764124882) jihad-mochi (start) running, process 11483
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (1182915/361851251) jihad-mojo (start) running, process 21649
DEVICE ok daemons-mojo: one daemon
DEVICE ok hand-daemon-mojo: none
DEVICE ok hooks-mojo: none
DEVICE ok adapter-gates-mojo: none
DEVICE ok proxy-mojo: network.proxy.type unset
DEVICE ok history-mojo: browser.sessionhistory.max_entries the default
DEVICE ok cards-mojo: none
DEVICE ok look-mojo: {"returnValue":true,"theme":null}
DEVICE ok gate-files: none in /tmp
DEVICE ok device-scripts: none left in /tmp
DEVICE ok test-pads-enyo: none
DEVICE ok dialog-pipes-enyo: none
DEVICE ok test-pads-mochi: none
DEVICE ok dialog-pipes-mochi: none
DEVICE ok test-pads-mojo: none
DEVICE ok dialog-pipes-mojo: none
DEVICE ok tmp-dumps: none
DEVICE ok media-scratch: none
DEVICE ok overall: 39% of /tmp used
```

## Every test's lines

### enyo / addon-site — pass

    enyo card on https://addons.riverstonerelay.org/: connected at +1 s, load done at +13 s, 9 content frames at +13 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COM
    WAIT ok 8-rect sel:form.search input
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 114 landed on document y 32)
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 13-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 15-url: https://addons.riverstonerelay.org/personas/
    WAIT ok 16-rect sel:a.button[data-browsertheme]
    CHECK ok 18-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 20-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 21-pref lightweightThemes.isThemeSelected = [false]

### enyo / addons-lifecycle — pass

    enyo card on about:addons: connected at +2 s, load done at +11 s, 5 content frames at +11 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 258 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='pad01@jihad.test']
    WAIT ok 13-rect anon:richlistitem[value='pad01@jihad.test']|disable-btn
    WAIT ok 15-rect anon:richlistitem[value='pad01@jihad.test']|enable-btn
    WAIT ok 17-rect anon:richlistitem[value='pad01@jihad.test']|undo-btn
    WAIT ok 19-rect anon:richlistitem[value='pad01@jihad.test']|remove-btn

### enyo / addons-list-pan — pass

    enyo card on about:addons: connected at +2 s, load done at +11 s, 6 content frames at +11 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 258 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:richlistitem[value='pad12@jihad.test']
    WAIT ok 15-still 0,100,756,1000
    WAIT ok 17-rect sel:richlistitem[value='pad12@jihad.test'][selected='true']
    CHECK ok 18-hit: clickAt 843,1439 inside sel:richlistitem[value="pad12@jihad.test"] at 294,1412 656x99

### enyo / addons-page — pass

    enyo card on about:addons: connected at +2 s, load done at +10 s, 4 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#category-extension
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 258 landed on document y 224)
    WAIT ok 9-rect sel:#category-extension[selected='true']
    WAIT ok 10-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:#category-plugin[selected='true']
    WAIT ok 14-rect sel:#category-extension[selected='true']
    WAIT ok 16-rect sel:#detail-view:not([hidden])
    CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill]

### enyo / addons-search-hide-key — pass

    enyo card on about:addons: connected at +2 s, load done at +10 s, 10 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#header-search
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 119 landed on document y 47)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-still sel:#header-search
    CHECK ok 14-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 15-still sel:#header-search
    WAIT ok 17-still sel:#header-search
    CHECK ok 18-region-equal: region 470,103-746,134: colours 100 hash 1f1e36f6 rows 31 uniform 0 light 103-133 blue none = the sample's hash
    CHECK ok 20-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / address-select-all — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-changed 200,88,470,135
    WAIT ok 14-still 200,88,470,135
    CHECK ok 16-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / appmenu-find — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 10-differs 0,30,300,180
    WAIT ok 11-still 0,30,300,180
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 16-log: [jihad-bs] find 'zebrafind' -> 1

### enyo / appmenu-print — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +3 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 11-differs 0,30,300,180
    WAIT ok 12-still 0,30,300,180
    WAIT ok 14-line: [jihad-bs] luna: findPrinters (started)
    WAIT ok 15-differs 0,200,756,960

### enyo / appmenu-settings-keyboard-down — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    WAIT ok 15-load
    CHECK ok 16-url: load done uri=about:settings#chrome=%7B%22liteMigrated%22%3Atrue%2C%22homeLiteMigrated%22%3Atrue%2C%22searchEngine%22%3A%22google%22%2C%22homeUrl%22%3A%22https%3A%2F%2Flite.duckduckgo.com%2Flite%2F%22%2C%22startLinks%22%3A%5B%7B%22title%22%3A%22DuckDuckGo%22%2C%22url%22%3A%22https%3A%2F%2Fduckduckgo.com%2F%22%7D%
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 18-rect page-title
    CHECK ok 19-content: region 0,100-768,700: colours 499 hash 333a5bcc rows 600 uniform 0 light none blue none; page-title on the screen at 20,100-744,134, text holds [Settings]

### enyo / audio-heard — a person's

    hearing the speaker; the route and the mixer are measured by script (cavekit-addons-extensions R7)

### enyo / busy-script-stop — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 8 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 9-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 13-rect sel:h1
    CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36

### enyo / card-dialog — pass

    enyo card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect c
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 142 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 13-text r = [c: true]
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-2.fifo
    CHECK ok 17-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 18-text r = [c: false]

### enyo / card-select — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +9 s, 8 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 9-text picked = [picked: banana]

### enyo / chrome-buttons — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 9 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 9-load
    WAIT ok 11-url: https://lite.duckduckgo.com/lite/
    WAIT ok 13-url: http://192.168.1.245:8077/jihad-long-static.html
    WAIT ok 15-url: https://lite.duckduckgo.com/lite/
    WAIT ok 18-differs 0,100,756,700

### enyo / chrome-history — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### enyo / chrome-padlock — pass

    enyo card on https://example.com/: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:p
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 385 landed on document y 303)
    WAIT ok 12-line: [jihad-bs] luna: pageCertificate (sent)
    WAIT ok 13-differs 0,100,756,960

### enyo / chrome-share — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### enyo / ddg-results — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 8-launch: load done
    WAIT ok 9-line: [jihad-bs] titleAndUrl title=[webOS at DuckDuckGo] uri=https://duckduckgo.com/?q=webOS&ia=web back=1 fwd=0
    CHECK ok 10-content: region 0,100-756,900: colours 1256 hash 2e4e0810 rows 800 uniform 188 light 100-895 blue 148-702; sel:body on the screen at 0,82-768,2616

### enyo / dialog-slow-answer — pass

    enyo card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect c
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 142 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text r = [c: true]

### enyo / download-finishes — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +10 s, 8 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's 
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-4

### enyo / drag-and-pinch-page — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 8 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 9-card-scroll-min: the card's scroll y 650 (at least 400)
    WAIT ok 11-zoom
    CHECK ok 12-zoom-min: zoom 2.6742 (at least 1.5)
    CHECK ok 13-content: region 0,150-756,600: colours 809 hash 7f86e30e rows 450 uniform 196 light 150-599 blue none; sel:body on the screen at -240,-826-1802,53962
    WAIT ok 15-zoom
    WAIT ok 17-zoom
    CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15)

### enyo / etsy-search — pass

    enyo card on https://www.etsy.com/search?q=webos: connected at +2 s, load done at +53 s, 17 content frames at +53 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_CO
    WAIT ok 8-rect sel:body
    WAIT ok 9-url: https://www.etsy.com/search?q=webos
    CHECK ok 10-url: load done uri=https://www.etsy.com/search?q=webos
    WAIT ok 11-content 0,100,768,900
    CHECK ok 12-content: region 0,100-768,900: colours 4096 hash 2f575159 rows 800 uniform 138 light 100-882 blue none; sel:body on the screen at 0,82-768,8670

### enyo / everyday-browsing — pass

    enyo card on https://duckduckgo.com/: connected at +2 s, load done at +50 s, 104 content frames at +50 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:textarea[name='q']
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 440 landed on document y 358)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-launch: load done
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 158 landed on document y 110)
    CHECK ok 15-card-scroll-min: the card's scroll y 872 (at least 300)
    WAIT ok 17-url: https://duckduckgo.com/

### enyo / express-burst-wiki — pass

    enyo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +2 s, load done at +26 s, 15 content frames at +26 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 8-rect sel:#firstHeading
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 158 landed on document y 110)
    WAIT ok 14-zoom
    WAIT ok 15-settled: the card's scroll 532 1106
    CHECK ok 16-zoom-min: zoom 2.3273 (at least 1.0)
    CHECK ok 17-content: region 0,100-756,700: colours 1051 hash 1f0b551c rows 600 uniform 0 light 101-699 blue 100-583; sel:body on the screen at -531,-1021-2065,2176

### enyo / flash-sound — a person's

    hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route)

### enyo / fonts-pick — pass

    enyo card on about:settings: connected at +3 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    WAIT ok 11-rect fonts-reset
    WAIT ok 13-pref font.name.serif.x-western = [Dotum]
    WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.]

### enyo / forget-identities — pass

    enyo card on about:settings: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.]

### enyo / frame-field — pass

    enyo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +2 s, load done at +9 s, 9 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 7-rect f
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 151 landed on document y 88)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 11-visible: f on the screen at 13,322-759,529 (the reach ends at 600, keyboard 1)
    WAIT ok 14-differs f
    CHECK ok 15-region-changed: region 13,323-756,528: colours 203 hash 9f55f686 rows 205 uniform 0 light 324-526 blue none against the sample's hash 2a543c72
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 18-still f
    WAIT ok 20-still f
    CHECK ok 21-region-equal: region 13,323-756,528: colours 202 hash af7f74a8 rows 205 uniform 0 light 324-526 blue none = the sample's hash
    CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / keyboard-address — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / keyboard-arbitration — not run

    hardware-blocked (no physical keyboard exists)

### enyo / keyboard-fields — pass

    enyo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 301 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 10-visible: area on the screen at 9,160-484,200 (the reach ends at 600, keyboard 1)
    CHECK ok 12-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 13-launch: load done
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 17-visible: field on the screen at 16,284-522,319 (the reach ends at 600, keyboard 1)

### enyo / long-press-image — pass

    enyo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +2 s, load done at +25 s, 12 content frames at +25 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 8-rect sel:.infobox img
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 158 landed on document y 110)
    WAIT ok 12-differs 0,100,756,900

### enyo / long-press-menu — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### enyo / long-press-text-field — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 10-differs 0,100,756,600
    WAIT ok 13-still 0,100,756,600
    WAIT ok 15-line: [jihad-bs] hold at 270,219 editable=1 wasFocused=0
    WAIT ok 18-still 0,100,400,240
    WAIT ok 20-load
    CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### enyo / mobile-wiki-pinch — pass

    enyo card on https://en.m.wikipedia.org/wiki/HP_TouchPad: connected at +2 s, load done at +24 s, 14 content frames at +24 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 8-rect sel:#firstHeading
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 158 landed on document y 110)
    WAIT ok 11-still 0,100,756,700
    CHECK ok 12-zoom-min: zoom 0.6882 (at least 0.65)
    CHECK ok 13-content: region 0,100-756,700: colours 4096 hash 43b65a18 rows 600 uniform 69 light 100-699 blue 236-240; sel:body on the screen at 0,82-768,1028
    CHECK ok 14-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    WAIT ok 16-zoom
    CHECK ok 17-zoom-min: zoom 1.8243 (at least 1.05)
    CHECK ok 18-content: region 0,100-756,700: colours 4096 hash 752ea776 rows 600 uniform 8 light 100-699 blue none; sel:body on the screen at -348,-203-1687,2302

### enyo / pan-long-drag — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 8 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 8-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 9-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 10-content: region 0,150-768,400: colours 321 hash 98b5debf rows 250 uniform 0 light 150-396 blue none; sel:#content p on the screen at 16,154-748,210, text holds [quick brown]
    CHECK ok 14-card-scroll: the card's scroll y 639 (want 650 within 80); the engine's 639
    CHECK ok 15-region-equal: region 0,83-756,137: colours 208 hash 34266017 rows 54 uniform 33 light none blue none = the sample's hash
    CHECK ok 16-region-changed: region 0,300-756,900: colours 967 hash faecb0c6 rows 600 uniform 212 light 300-899 blue none against the sample's hash 6a2b3a47
    CHECK ok 17-top: the page's top measured 82 (the tap at y 223 landed on document y 780), the model says 82 (header hidden: 1)
    CHECK ok 19-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### enyo / pan-long-flick — known

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 18-card-scroll-min: the card's scroll y 3300 (at least 300)
    STREAM fs-enyo-pan-long-flick.fbs: 127 samples (13.4/s asked 15, read median 2.25 ms max 81.41 ms, mmap stride 8, re-read 13, torn 0), 122 flips, cpu 7.9% of a core, polls 579/s | shown frames while moving: 117 gaps, median 27 ms, p90 43 ms, max 70 ms | shown scroll: 1532 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 46, still rows moved 0; the columns outside 0-756 changed in 38 samples | tracking (finger down, 24 samples): error median 30.0 px, max 188 px; lag median 190.0 ms, max 955 ms |   gesture at 1793925610670: the finger travelled 600 rows, the page sh
    CHECK ok 16-stream-stale: stale 0 (at most 0)
    CHECK FAIL 17-stream-uniform: uniform 46 (want at most 0)

### enyo / pan-speeds — known

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +10 s, 5 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's 
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 13-card-scroll-min: the card's scroll y 648 (at least 500)
    CHECK ok 22-card-scroll-min: the card's scroll y 4584 (at least 2500)
    STREAM fs-enyo-pan-speeds.fbs: 243 samples (14.8/s asked 15, read median 2.43 ms max 133.2 ms, mmap stride 8, re-read 34, torn 0), 358 flips, cpu 7.5% of a core, polls 521/s | shown frames while moving: 351 gaps, median 27 ms, p90 41 ms, max 260 ms | shown scroll: 3564 rows over the stream (0 samples with the shift unknown); stale bands 1, uniform bands 45, still rows moved 0; the columns outside 0-756 changed in 131 samples | tracking (finger down, 125 samples): error median -12 px, max 263 px; lag median 75 ms, max 792 ms |   gesture at 1793943603669: the finger travelled 650 rows, the page showed 
    CHECK FAIL 19-stream-stale: stale 1 (want at most 0)
    CHECK FAIL 20-stream-uniform: uniform 45 (want at most 0)
    CHECK ok 21-stream-error-max: error-max 263 (at most 500)

### enyo / persona-cycle — pass

    enyo card on about:personas: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#mine .card
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 280 landed on document y 198)
    WAIT ok 11-rect sel:#files button
    CHECK ok 13-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 15-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 16-pref lightweightThemes.isThemeSelected = [true]
    WAIT ok 17-rect sel:#mine .card:nth-child(2) button
    CHECK ok 18-text-has: sel:#mine .card:nth-child(2) = [Carbon Light 2.0Author unknownTake offRemove] holds [Carbon]
    WAIT ok 20-text status-text = [Persona taken off.]
    WAIT ok 22-text sel:#mine .card:nth-child(2) = [Carbon Light 2.0Author unknownWearRemove it?]
    CHECK ok 23-text-has: sel:#mine .card:nth-child(2) = [Carbon Light 2.0Author unknownWearRemove it?] holds [Remove it?]
    CHECK ok 25-text-has: sel:#mine .card:nth-child(2) = [Carbon Light 2.0Author unknownWearRemove it?] holds [Remove it?]
    WAIT ok 27-text status-text = [Removing Carbon Light…]

### enyo / persona-wear — pass

    enyo card on about:personas: connected at +2 s, load done at +10 s, 8 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#online .card button
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 280 landed on document y 198)
    CHECK ok 11-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text status-text = [Wearing Autumn Forest.]
    WAIT ok 15-pref lightweightThemes.isThemeSelected = [true]
    WAIT ok 17-text status-text = [Undone.]
    WAIT ok 18-pref lightweightThemes.isThemeSelected = [false]

### enyo / personas-flick — pass

    enyo card on about:personas: connected at +2 s, load done at +11 s, 15 content frames at +11 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#mine .card
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 280 landed on document y 198)
    CHECK ok 10-card-scroll-min: the card's scroll y 1644 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### enyo / personas-page-steady — pass

    enyo card on about:personas: connected at +1 s, load done at +10 s, 11 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-rect sel:#mine .card
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 280 landed on document y 198)
    CHECK ok 16-content: region 0,100-768,900: colours 4096 hash 92b57e71 rows 800 uniform 0 light 100-899 blue 281-755; sel:body on the screen at 0,-412-764,2157
    STREAM fs-enyo-personas-page-steady.fbs: 88 samples (15.0/s asked 15, read median 2.54 ms max 26.28 ms, mmap stride 8, re-read 12, torn 0), 142 flips, cpu 7.2% of a core, polls 530/s | shown frames while moving: 141 gaps, median 26 ms, p90 39 ms, max 49 ms | shown scroll: 496 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 0-756 changed in 51 samples | tracking (finger down, 53 samples): error median -10 px, max 24 px; lag median 71 ms, max 163 ms |   gesture at 1793925982023: the finger travelled 500 rows, the page show
    CHECK ok 15-stream-uniform: uniform 0 (at most 0)

### enyo / proxy-login — pass

    enyo card on about:settings: connected at +1 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 9-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    CHECK ok 12-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    WAIT ok 13-still 200,100,756,700
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 19-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 20-still 200,100,756,700
    CHECK ok 22-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 25-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 26-still 200,100,756,700
    CHECK ok 28-dialog: [jihad-bs] dialog auth -> ACCEPT (answer=1)
    WAIT ok 29-line: [jihad-bs] late load done uri=http://example.com/jihad-proxy-login

### enyo / proxy-own — pass

    enyo card on about:settings: connected at +2 s, load done at +9 s, 9 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 13-rect sel:.row[data-pref='network.proxy.http'] input
    CHECK ok 15-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 24-pref network.proxy.http = [192.168.1.245]
    CHECK ok 26-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 28-dialog: [jihad-bs] dialog auth -> cancel (answer=0)

### enyo / proxy-section-redraw — pass

    enyo card on about:settings: connected at +2 s, load done at +9 s, 4 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 11-rect sel:.row[data-pref='network.proxy.type'] select
    WAIT ok 13-rect sel:.row[data-pref='network.proxy.http'] input
    WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input

### enyo / relay-choice — pass

    enyo card on about:settings: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 12-rect sel:.row[data-pref='network.proxy.type'] select
    WAIT ok 14-pref network.proxy.http = [proxy.riverstonerelay.org]
    WAIT ok 15-launch: load done
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 385 landed on document y 303)
    CHECK ok 17-text-has: sel:p = [This domain is for use in documentation examples without needing permission. This is not a service; avoid relying on it for testing and monitoring purposes.] holds [documentation]

### enyo / rotate — a person's

    the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side

### enyo / script-hold-zoom — pass

    enyo card on http://192.168.1.245:8077/jihad-script-hold.html: connected at +2 s, load done at +9 s, 3 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 110 landed on document y 28)
    WAIT ok 9-zoom
    CHECK ok 10-zoom-min: zoom 1.9361 (at least 1.3)
    CHECK ok 12-card-scroll-min: the card's scroll y 460 (at least 200)
    WAIT ok 14-zoom
    WAIT ok 16-zoom
    CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2)

### enyo / settings-every-control — by a tool

    tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt)

### enyo / settings-field-saves — pass

    enyo card on about:settings: connected at +1 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 16-pref browser.display.foreground_color = [#000000x]
    WAIT ok 17-still sel:.row[data-pref='browser.display.foreground_color'] input
    WAIT ok 19-still sel:.row[data-pref='browser.display.foreground_color'] input
    CHECK ok 20-region-equal: region 416,644-735,704: colours 49 hash 34233dc3 rows 60 uniform 0 light 644-703 blue none = the sample's hash

### enyo / settings-flick — pass

    enyo card on about:settings: connected at +2 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    CHECK ok 10-card-scroll-min: the card's scroll y 720 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### enyo / settings-int-emptied — pass

    enyo card on about:settings: connected at +2 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 16-pref browser.sessionhistory.max_entries = [20]

### enyo / settings-persist — pass

    enyo card on about:settings: connected at +2 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-pref general.smoothScroll = [true]
    WAIT ok 14-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 16-pref general.smoothScroll = [true]

### enyo / settings-tabs — pass

    enyo card on about:settings: connected at +3 s, load done at +10 s, 7 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    CHECK ok 9-text: page-title = [Settings]
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content]

### enyo / settings-typing — pass

    enyo card on about:settings: connected at +2 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 15-pref browser.display.background_color = [#FFFFFFx]
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / ssl-confirm — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog ssl-confirm -> cancel (answer=0)

### enyo / tabbed-browsing — not run

    not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word)

### enyo / touchpad-go — not run

    owner-accepted without hardware (device-build R6)

### enyo / typing-caret — pass

    enyo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +3 s, load done at +9 s, 11 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 301 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 11-text echo = [plain n=6 c=0 last=[abcdef] value=[abcdef] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 13-text echo = [plain n=12 c=0 last=[] value=[] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / video-controls — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html: connected at +2 s, load done at +10 s, 37 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's 
    WAIT ok 7-rect status
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 10-changed sel:video
    WAIT ok 11-changed sel:video
    WAIT ok 13-still sel:video
    WAIT ok 15-still sel:video
    CHECK ok 16-region-equal: region 16,125-517,405: colours 2717 hash 018a25e1 rows 280 uniform 0 light 392-397 blue 335-371 = the sample's hash

### enyo / webgl-cube — pass

    enyo card on http://192.168.1.245:8077/jihad-webgl-cube.html: connected at +2 s, load done at +9 s, 20 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 7-rect c
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 124 landed on document y 57)
    CHECK ok 9-text-has: out = [webgl fps=22.1 shaders=ok texture=ok framebuffer=ok readPixels=ok maxtex=4096 ext=14 missing=ANGLE_instanced_arrays,WEBGL_compressed_texture_s3tc,EXT_blend_minmax,EXT_shader_texture_lod depthrb=ok renderer=Mozilla vendor=Mozilla] holds [webgl]
    CHECK ok 10-content: region 105,175-457,524: colours 976 hash dc607c95 rows 349 uniform 101 light none blue 234-435; c on the screen at 105,174-457,525
    WAIT ok 11-changed c
    WAIT ok 13-zoom
    CHECK ok 15-content: region 0,100-768,900: colours 1266 hash 5496b371 rows 800 uniform 0 light none blue 254-398; c on the screen at -23,103-527,654
    WAIT ok 16-changed c

### enyo / webone — not run

    needs webone, keyboard-map (run with --with webone,keyboard-map)

### enyo / wikipedia-keyboard-zoom — pass

    enyo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +2 s, load done at +22 s, 11 content frames at +22 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 8-rect sel:input[name='search']
    WAIT ok 9-still 0,100,756,700
    CHECK ok 10-zoom: zoom 0.6882 (want 0.69 within 0.06)
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 158 landed on document y 110)
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 14-still 0,100,756,500
    CHECK ok 15-zoom: zoom 0.6882 (want 0.69 within 0.06)
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 18-still 0,100,756,700
    CHECK ok 19-zoom: zoom 0.6882 (want 0.69 within 0.06)
    CHECK ok 20-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]

### enyo / wikipedia-zoom — pass

    enyo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +24 s, 9 content frames at +24 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 11-rect sel:.infobox
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 158 landed on document y 110)
    WAIT ok 15-zoom
    CHECK ok 16-zoom-min: zoom 2.8444 (at least 1.3)
    WAIT ok 18-zoom
    WAIT ok 20-zoom
    WAIT ok 22-still 0,100,756,700
    CHECK ok 24-zoom: zoom 0.6882 (want 0.69 within 0.12)
    CHECK ok 25-content: region 0,100-756,700: colours 4096 hash 1836ba1c rows 600 uniform 61 light 100-699 blue 236-240; sel:body on the screen at 0,82-768,1028
    CHECK ok 26-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    STREAM fs-enyo-wikipedia-zoom.fbs: 262 samples (15.0/s asked 15, read median 1.75 ms max 39.98 ms, mmap stride 8, re-read 9, torn 0), 201 flips, cpu 6.1% of a core, polls 701/s | shown frames while moving: 189 gaps, median 37 ms, p90 82 ms, max 253 ms | shown scroll: 82 rows over the stream (46 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 170-600 changed in 20 samples | tracking (finger down, 43 samples): error median 0 px, max 1 px; lag median 431 ms, max 898 ms |   gesture at 1793927314148: the finger travelled 0 rows, the page showed 0 by
    CHECK ok 27-stream-uniform: uniform 0 (at most 0)

### enyo / youtube-mobile — pass

    enyo card on https://m.youtube.com/watch?v=dQw4w9WgXcQ: connected at +3 s, load done at +28 s, 9 content frames at +28 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 8-rect sel:video
    WAIT ok 10-changed 0,130,756,560
    WAIT ok 11-changed 0,130,756,560

### mochi / addon-site — pass

    mochi card on https://addons.riverstonerelay.org/: connected at +3 s, load done at +14 s, 4 content frames at +14 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_CO
    WAIT ok 8-rect sel:form.search input
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 110 landed on document y 32)
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 13-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 15-url: https://addons.riverstonerelay.org/personas/
    WAIT ok 16-rect sel:a.button[data-browsertheme]
    CHECK ok 18-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 20-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 21-pref lightweightThemes.isThemeSelected = [false]

### mochi / addons-lifecycle — pass

    mochi card on about:addons: connected at +2 s, load done at +13 s, 4 content frames at +13 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 254 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='pad01@jihad.test']
    WAIT ok 13-rect anon:richlistitem[value='pad01@jihad.test']|disable-btn
    WAIT ok 15-rect anon:richlistitem[value='pad01@jihad.test']|enable-btn
    WAIT ok 17-rect anon:richlistitem[value='pad01@jihad.test']|undo-btn
    WAIT ok 19-rect anon:richlistitem[value='pad01@jihad.test']|remove-btn

### mochi / addons-list-pan — pass

    mochi card on about:addons: connected at +3 s, load done at +13 s, 4 content frames at +13 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 254 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:richlistitem[value='pad12@jihad.test']
    WAIT ok 15-still 0,100,756,1000
    WAIT ok 17-rect sel:richlistitem[value='pad12@jihad.test'][selected='true']
    CHECK ok 18-hit: clickAt 843,1439 inside sel:richlistitem[value="pad12@jihad.test"] at 294,1412 656x99

### mochi / addons-page — pass

    mochi card on about:addons: connected at +2 s, load done at +10 s, 5 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#category-extension
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 254 landed on document y 224)
    WAIT ok 9-rect sel:#category-extension[selected='true']
    WAIT ok 10-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:#category-plugin[selected='true']
    WAIT ok 14-rect sel:#category-extension[selected='true']
    WAIT ok 16-rect sel:#detail-view:not([hidden])
    CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill]

### mochi / addons-search-hide-key — pass

    mochi card on about:addons: connected at +1 s, load done at +9 s, 3 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#header-search
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 115 landed on document y 47)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-still sel:#header-search
    CHECK ok 14-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 15-still sel:#header-search
    WAIT ok 17-still sel:#header-search
    CHECK ok 18-region-equal: region 470,99-746,130: colours 103 hash 1b422781 rows 31 uniform 0 light 99-129 blue none = the sample's hash
    CHECK ok 20-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / address-select-all — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +3 s, load done at +10 s, 6 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-changed 200,88,470,135
    WAIT ok 14-still 200,88,470,135
    CHECK ok 16-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / appmenu-find — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +3 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 10-differs 0,30,300,180
    WAIT ok 11-still 0,30,300,180
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 16-log: [jihad-bs] find 'zebrafind' -> 1

### mochi / appmenu-print — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 11-differs 0,30,300,180
    WAIT ok 12-still 0,30,300,180
    WAIT ok 14-line: [jihad-bs] luna: findPrinters (started)
    WAIT ok 15-differs 0,200,756,960

### mochi / appmenu-settings-keyboard-down — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +10 s, 5 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    WAIT ok 15-load
    CHECK ok 16-url: load done uri=about:settings#chrome=%7B%22searchEngine%22%3A%22duckduckgo-lite%22%2C%22homeUrl%22%3A%22https%3A%2F%2Flite.duckduckgo.com%2Flite%2F%22%2C%22startLinks%22%3A%5B%7B%22title%22%3A%22DuckDuckGo%22%2C%22url%22%3A%22https%3A%2F%2Fduckduckgo.com%2F%22%7D%2C%7B%22title%22%3A%22Wikipedia%22%2C%22url%22%3A%2
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 18-rect page-title
    CHECK ok 19-content: region 0,100-768,700: colours 787 hash 3b16afe0 rows 600 uniform 0 light none blue none; page-title on the screen at 20,96-744,130, text holds [Settings]

### mochi / audio-heard — a person's

    hearing the speaker; the route and the mixer are measured by script (cavekit-addons-extensions R7)

### mochi / busy-script-stop — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 9-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 13-rect sel:h1
    CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36

### mochi / card-dialog — pass

    mochi card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +2 s, load done at +9 s, 4 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 7-rect c
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 138 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 13-text r = [c: true]
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-2.fifo
    CHECK ok 17-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 18-text r = [c: false]

### mochi / card-select — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 9-text picked = [picked: banana]

### mochi / chrome-buttons — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 6 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's 
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 9-load
    WAIT ok 11-url: https://lite.duckduckgo.com/lite/
    WAIT ok 13-url: http://192.168.1.245:8077/jihad-long-static.html
    WAIT ok 15-url: https://lite.duckduckgo.com/lite/
    WAIT ok 18-differs 0,100,756,700

### mochi / chrome-history — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### mochi / chrome-padlock — pass

    mochi card on https://example.com/: connected at +2 s, load done at +11 s, 8 content frames at +11 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:p
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 382 landed on document y 304)
    WAIT ok 12-line: [jihad-bs] luna: pageCertificate (sent)
    WAIT ok 13-differs 0,100,756,960

### mochi / chrome-share — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### mochi / ddg-results — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 8-launch: load done
    WAIT ok 9-line: [jihad-bs] titleAndUrl title=[webOS at DuckDuckGo] uri=https://duckduckgo.com/?q=webOS&ia=web back=1 fwd=0
    CHECK ok 10-content: region 0,100-756,900: colours 1246 hash 31a0caa8 rows 800 uniform 187 light 100-899 blue 143-698; sel:body on the screen at 0,78-768,2612

### mochi / dialog-slow-answer — pass

    mochi card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +1 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 7-rect c
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 138 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text r = [c: true]

### mochi / download-finishes — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +8 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-5

### mochi / drag-and-pinch-page — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 9-card-scroll-min: the card's scroll y 628 (at least 400)
    WAIT ok 11-zoom
    CHECK ok 12-zoom-min: zoom 2.6832 (at least 1.5)
    CHECK ok 13-content: region 0,150-756,600: colours 652 hash 68bc591b rows 450 uniform 203 light 170-599 blue 168-168; sel:body on the screen at -241,-811-1808,54161
    WAIT ok 15-zoom
    WAIT ok 17-zoom
    CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15)

### mochi / etsy-search — pass

    mochi card on https://www.etsy.com/search?q=webos: connected at +5 s, load done at +53 s, 20 content frames at +53 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_C
    WAIT ok 8-rect sel:body
    WAIT ok 9-url: https://www.etsy.com/search?q=webos
    CHECK ok 10-url: load done uri=https://www.etsy.com/search?q=webos
    WAIT ok 11-content 0,100,768,900
    CHECK ok 12-content: region 0,100-768,900: colours 4096 hash 8f6c2115 rows 800 uniform 127 light 100-899 blue 330-392; sel:body on the screen at 0,78-768,8681

### mochi / everyday-browsing — pass

    mochi card on https://duckduckgo.com/: connected at +2 s, load done at +38 s, 80 content frames at +38 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:textarea[name='q']
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 437 landed on document y 359)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-launch: load done
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 154 landed on document y 110)
    CHECK ok 15-card-scroll-min: the card's scroll y 872 (at least 300)
    WAIT ok 17-url: https://duckduckgo.com/

### mochi / express-burst-wiki — pass

    mochi card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +3 s, load done at +27 s, 14 content frames at +27 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 8-rect sel:#firstHeading
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 154 landed on document y 110)
    WAIT ok 14-zoom
    WAIT ok 15-settled: the card's scroll 531 1122
    CHECK ok 16-zoom-min: zoom 2.3273 (at least 1.0)
    CHECK ok 17-content: region 0,100-756,700: colours 999 hash bd108dbc rows 600 uniform 0 light 100-699 blue 523-541; sel:body on the screen at -530,-1041-2066,2170

### mochi / flash-sound — a person's

    hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route)

### mochi / fonts-pick — pass

    mochi card on about:settings: connected at +3 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    WAIT ok 11-rect fonts-reset
    WAIT ok 13-pref font.name.serif.x-western = [Dotum]
    WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.]

### mochi / forget-identities — pass

    mochi card on about:settings: connected at +2 s, load done at +8 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.]

### mochi / frame-field — pass

    mochi card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +2 s, load done at +9 s, 10 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's 
    WAIT ok 7-rect f
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 147 landed on document y 88)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 11-visible: f on the screen at 13,318-759,525 (the reach ends at 600, keyboard 1)
    WAIT ok 14-differs f
    CHECK ok 15-region-changed: region 13,319-756,524: colours 229 hash 4e051f6c rows 205 uniform 0 light 320-522 blue none against the sample's hash 47ef556c
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 18-still f
    WAIT ok 20-still f
    CHECK ok 21-region-equal: region 13,319-756,524: colours 228 hash ea9cee95 rows 205 uniform 0 light 320-522 blue none = the sample's hash
    CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / keyboard-address — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 3 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / keyboard-arbitration — not run

    hardware-blocked (no physical keyboard exists)

### mochi / keyboard-fields — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +11 s, 6 content frames at +11 s; the GPU path, frames off the main thread (the express lane) (the job's 
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 297 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 10-visible: area on the screen at 9,156-484,196 (the reach ends at 600, keyboard 1)
    CHECK ok 12-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 13-launch: load done
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 17-visible: field on the screen at 16,280-522,315 (the reach ends at 600, keyboard 1)

### mochi / long-press-image — pass

    mochi card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +2 s, load done at +21 s, 13 content frames at +21 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 8-rect sel:.infobox img
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 154 landed on document y 110)
    WAIT ok 12-differs 0,100,756,900

### mochi / long-press-menu — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mochi / long-press-text-field — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +8 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 10-differs 0,100,756,600
    WAIT ok 13-still 0,100,756,600
    WAIT ok 15-line: [jihad-bs] hold at 270,219 editable=1 wasFocused=0
    WAIT ok 18-still 0,100,400,240
    WAIT ok 20-load
    CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mochi / mobile-wiki-pinch — pass

    mochi card on https://en.m.wikipedia.org/wiki/HP_TouchPad: connected at +3 s, load done at +28 s, 15 content frames at +28 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 8-rect sel:#firstHeading
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 154 landed on document y 110)
    WAIT ok 11-still 0,100,756,700
    CHECK ok 12-zoom-min: zoom 0.6882 (at least 0.65)
    CHECK ok 13-content: region 0,100-756,700: colours 4096 hash 7ca2a22e rows 600 uniform 69 light 100-699 blue 232-236; sel:body on the screen at 0,78-768,1028
    CHECK ok 14-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    WAIT ok 16-zoom
    CHECK ok 17-zoom-min: zoom 1.8385 (at least 1.05)
    CHECK ok 18-content: region 0,100-756,700: colours 4096 hash 525844f9 rows 600 uniform 2 light 100-699 blue none; sel:body on the screen at -349,-212-1702,2324

### mochi / mochi-paste — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +1 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 297 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-differs 100,100,668,420
    WAIT ok 15-still 100,100,668,420
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / pan-long-drag — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 8-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 9-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 10-content: region 0,150-768,400: colours 276 hash 0c6c964c rows 250 uniform 0 light 150-399 blue none; sel:#content p on the screen at 16,150-748,206, text holds [quick brown]
    CHECK ok 14-card-scroll: the card's scroll y 639 (want 650 within 80); the engine's 639
    CHECK ok 15-region-equal: region 0,79-756,133: colours 204 hash 5486acf3 rows 54 uniform 33 light none blue none = the sample's hash
    CHECK ok 16-region-changed: region 0,300-756,900: colours 840 hash 3f74d198 rows 600 uniform 214 light 307-899 blue none against the sample's hash 653a2a3a
    CHECK ok 17-top: the page's top measured 78 (the tap at y 219 landed on document y 780), the model says 78 (header hidden: 1)
    CHECK ok 19-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### mochi / pan-long-flick — known

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +8 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 18-card-scroll-min: the card's scroll y 3337 (at least 300)
    STREAM fs-mochi-pan-long-flick.fbs: 117 samples (14.6/s asked 15, read median 2.37 ms max 51.1 ms, mmap stride 8, re-read 10, torn 0), 142 flips, cpu 7.3% of a core, polls 560/s | shown frames while moving: 139 gaps, median 27 ms, p90 44 ms, max 279 ms | shown scroll: 1637 rows over the stream (0 samples with the shift unknown); stale bands 1, uniform bands 38, still rows moved 38; the columns outside 0-756 changed in 50 samples | tracking (finger down, 23 samples): error median 30 px, max 225 px; lag median 175 ms, max 914 ms |   gesture at 1793946162670: the finger travelled 600 rows, the page show
    CHECK FAIL 16-stream-stale: stale 1 (want at most 0)
    CHECK FAIL 17-stream-uniform: uniform 38 (want at most 0)

### mochi / pan-speeds — known

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 13-card-scroll-min: the card's scroll y 646 (at least 500)
    CHECK ok 22-card-scroll-min: the card's scroll y 4655 (at least 2500)
    STREAM fs-mochi-pan-speeds.fbs: 292 samples (14.5/s asked 15, read median 2.34 ms max 180.37 ms, mmap stride 8, re-read 22, torn 2), 338 flips, cpu 6.4% of a core, polls 573/s | shown frames while moving: 330 gaps, median 27.0 ms, p90 43 ms, max 166 ms | shown scroll: 2387 rows over the stream (3 samples with the shift unknown); stale bands 0, uniform bands 47, still rows moved 94; the columns outside 0-756 changed in 117 samples | tracking (finger down, 126 samples): error median -12.0 px, max 450 px; lag median 74.5 ms, max 792 ms |   gesture at 1793930384643: the finger travelled 650 rows, the pag
    CHECK ok 19-stream-stale: stale 0 (at most 0)
    CHECK FAIL 20-stream-uniform: uniform 47 (want at most 0)
    CHECK ok 21-stream-error-max: error-max 450 (at most 500)

### mochi / persona-cycle — pass

    mochi card on about:personas: connected at +2 s, load done at +10 s, 7 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#mine .card
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 276 landed on document y 198)
    WAIT ok 11-rect sel:#files button
    CHECK ok 13-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 15-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 16-pref lightweightThemes.isThemeSelected = [true]
    WAIT ok 17-rect sel:#mine .card:nth-child(2) button
    CHECK ok 18-text-has: sel:#mine .card:nth-child(2) = [Carbon Light 2.0Author unknownTake offRemove] holds [Carbon]
    WAIT ok 20-text status-text = [Persona taken off.]
    WAIT ok 22-text sel:#mine .card:nth-child(2) = [Carbon Light 2.0Author unknownWearRemove it?]
    CHECK ok 23-text-has: sel:#mine .card:nth-child(2) = [Carbon Light 2.0Author unknownWearRemove it?] holds [Remove it?]
    CHECK ok 25-text-has: sel:#mine .card:nth-child(2) = [Carbon Light 2.0Author unknownWearRemove it?] holds [Remove it?]
    WAIT ok 27-text status-text = [Removing Carbon Light…]

### mochi / persona-wear — pass

    mochi card on about:personas: connected at +2 s, load done at +11 s, 16 content frames at +11 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#online .card button
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 276 landed on document y 198)
    CHECK ok 11-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text status-text = [Wearing Autumn Forest.]
    WAIT ok 15-pref lightweightThemes.isThemeSelected = [true]
    WAIT ok 17-text status-text = [Undone.]
    WAIT ok 18-pref lightweightThemes.isThemeSelected = [false]

### mochi / personas-flick — pass

    mochi card on about:personas: connected at +2 s, load done at +11 s, 17 content frames at +11 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#mine .card
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 276 landed on document y 198)
    CHECK ok 10-card-scroll-min: the card's scroll y 1930 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### mochi / personas-page-steady — pass

    mochi card on about:personas: connected at +2 s, load done at +11 s, 12 content frames at +11 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-rect sel:#mine .card
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 276 landed on document y 198)
    CHECK ok 16-content: region 0,100-768,900: colours 4096 hash 17e8507e rows 800 uniform 0 light 100-645 blue 105-653; sel:body on the screen at 0,-420-764,2439
    STREAM fs-mochi-personas-page-steady.fbs: 97 samples (15.0/s asked 15, read median 2.46 ms max 20.25 ms, mmap stride 8, re-read 9, torn 0), 138 flips, cpu 6.5% of a core, polls 527/s | shown frames while moving: 137 gaps, median 27 ms, p90 41 ms, max 94 ms | shown scroll: 500 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 0-756 changed in 54 samples | tracking (finger down, 56 samples): error median -8.0 px, max 28 px; lag median 73.0 ms, max 236 ms |   gesture at 1793930692002: the finger travelled 500 rows, the page s
    CHECK ok 15-stream-uniform: uniform 0 (at most 0)

### mochi / proxy-login — pass

    mochi card on about:settings: connected at +3 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 9-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    CHECK ok 12-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    WAIT ok 13-still 200,100,756,700
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 19-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 20-still 200,100,756,700
    CHECK ok 22-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 25-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 26-still 200,100,756,700
    CHECK ok 28-dialog: [jihad-bs] dialog auth -> ACCEPT (answer=1)
    WAIT ok 29-line: [jihad-bs] late load done uri=http://example.com/jihad-proxy-login

### mochi / proxy-own — pass

    mochi card on about:settings: connected at +2 s, load done at +8 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 13-rect sel:.row[data-pref='network.proxy.http'] input
    CHECK ok 15-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 24-pref network.proxy.http = [192.168.1.245]
    CHECK ok 26-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 28-dialog: [jihad-bs] dialog auth -> cancel (answer=0)

### mochi / proxy-section-redraw — pass

    mochi card on about:settings: connected at +1 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 11-rect sel:.row[data-pref='network.proxy.type'] select
    WAIT ok 13-rect sel:.row[data-pref='network.proxy.http'] input
    WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input

### mochi / relay-choice — pass

    mochi card on about:settings: connected at +2 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 12-rect sel:.row[data-pref='network.proxy.type'] select
    WAIT ok 14-pref network.proxy.http = [proxy.riverstonerelay.org]
    WAIT ok 15-launch: load done
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 382 landed on document y 304)
    CHECK ok 17-text-has: sel:p = [This domain is for use in documentation examples without needing permission. This is not a service; avoid relying on it for testing and monitoring purposes.] holds [documentation]

### mochi / rotate — a person's

    the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side

### mochi / script-hold-zoom — pass

    mochi card on http://192.168.1.245:8077/jihad-script-hold.html: connected at +3 s, load done at +13 s, 5 content frames at +15 s; the GPU path, frames off the main thread (the express lane) (the job's
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 106 landed on document y 28)
    WAIT ok 9-zoom
    CHECK ok 10-zoom-min: zoom 1.9303 (at least 1.3)
    CHECK ok 12-card-scroll-min: the card's scroll y 463 (at least 200)
    WAIT ok 14-zoom
    WAIT ok 16-zoom
    CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2)

### mochi / settings-every-control — by a tool

    tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt)

### mochi / settings-field-saves — pass

    mochi card on about:settings: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 16-pref browser.display.foreground_color = [#000000x]
    WAIT ok 17-still sel:.row[data-pref='browser.display.foreground_color'] input
    WAIT ok 19-still sel:.row[data-pref='browser.display.foreground_color'] input
    CHECK ok 20-region-equal: region 416,645-735,705: colours 44 hash 9db921ae rows 60 uniform 0 light 645-704 blue none = the sample's hash

### mochi / settings-flick — pass

    mochi card on about:settings: connected at +2 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    CHECK ok 10-card-scroll-min: the card's scroll y 707 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### mochi / settings-int-emptied — pass

    mochi card on about:settings: connected at +2 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 16-pref browser.sessionhistory.max_entries = [20]

### mochi / settings-persist — pass

    mochi card on about:settings: connected at +2 s, load done at +9 s, 8 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-pref general.smoothScroll = [true]
    WAIT ok 14-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 16-pref general.smoothScroll = [true]

### mochi / settings-tabs — pass

    mochi card on about:settings: connected at +2 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    CHECK ok 9-text: page-title = [Settings]
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content]

### mochi / settings-typing — pass

    mochi card on about:settings: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 15-pref browser.display.background_color = [#FFFFFFx]
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / ssl-confirm — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog ssl-confirm -> cancel (answer=0)

### mochi / tabbed-browsing — not run

    not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word)

### mochi / touchpad-go — not run

    owner-accepted without hardware (device-build R6)

### mochi / typing-caret — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +10 s, 5 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's 
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 297 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 11-text echo = [plain n=6 c=0 last=[abcdef] value=[abcdef] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 13-text echo = [plain n=12 c=0 last=[] value=[] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / video-controls — pass

    mochi card on http://192.168.1.245:8077/jihad-media-test.html: connected at +2 s, load done at +10 s, 14 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's
    WAIT ok 7-rect status
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 98 landed on document y 25)
    WAIT ok 10-changed sel:video
    WAIT ok 11-changed sel:video
    WAIT ok 13-still sel:video
    WAIT ok 15-still sel:video
    CHECK ok 16-region-equal: region 16,121-517,401: colours 2507 hash 33085829 rows 280 uniform 0 light 388-393 blue none = the sample's hash

### mochi / webgl-cube — pass

    mochi card on http://192.168.1.245:8077/jihad-webgl-cube.html: connected at +2 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 7-rect c
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 120 landed on document y 57)
    CHECK ok 9-text-has: out = [webgl fps=25.6 shaders=ok texture=ok framebuffer=ok readPixels=ok maxtex=4096 ext=14 missing=ANGLE_instanced_arrays,WEBGL_compressed_texture_s3tc,EXT_blend_minmax,EXT_shader_texture_lod depthrb=ok renderer=Mozilla vendor=Mozilla] holds [webgl]
    CHECK ok 10-content: region 105,171-457,520: colours 788 hash fdd1f312 rows 349 uniform 161 light none blue 268-448; c on the screen at 105,170-457,521
    WAIT ok 11-changed c
    WAIT ok 13-zoom
    CHECK ok 15-content: region 0,100-768,900: colours 970 hash 81d4ee1d rows 800 uniform 267 light none blue none; c on the screen at -23,99-527,650
    WAIT ok 16-changed c

### mochi / webone — not run

    needs webone, keyboard-map (run with --with webone,keyboard-map)

### mochi / wikipedia-keyboard-zoom — pass

    mochi card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +2 s, load done at +24 s, 10 content frames at +24 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 8-rect sel:input[name='search']
    WAIT ok 9-still 0,100,756,700
    CHECK ok 10-zoom: zoom 0.6882 (want 0.69 within 0.06)
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 154 landed on document y 110)
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 14-still 0,100,756,500
    CHECK ok 15-zoom: zoom 0.6882 (want 0.69 within 0.06)
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 18-still 0,100,756,700
    CHECK ok 19-zoom: zoom 0.6882 (want 0.69 within 0.06)
    CHECK ok 20-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]

### mochi / wikipedia-zoom — pass

    mochi card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +2 s, load done at +21 s, 12 content frames at +21 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 11-rect sel:.infobox
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 154 landed on document y 110)
    WAIT ok 15-zoom
    CHECK ok 16-zoom-min: zoom 2.8444 (at least 1.3)
    WAIT ok 18-zoom
    WAIT ok 20-zoom
    WAIT ok 22-still 0,100,756,700
    CHECK ok 24-zoom: zoom 0.6882 (want 0.69 within 0.12)
    CHECK ok 25-content: region 0,100-756,700: colours 4096 hash 5c75943e rows 600 uniform 62 light 100-699 blue 232-236; sel:body on the screen at 0,78-768,1028
    CHECK ok 26-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    STREAM fs-mochi-wikipedia-zoom.fbs: 259 samples (15.0/s asked 15, read median 1.69 ms max 20.41 ms, mmap stride 8, re-read 4, torn 0), 186 flips, cpu 6.2% of a core, polls 716/s | shown frames while moving: 181 gaps, median 38 ms, p90 113 ms, max 285 ms | shown scroll: 114 rows over the stream (43 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 170-600 changed in 18 samples | tracking (finger down, 39 samples): error median 0 px, max 1 px; lag median 386 ms, max 851 ms |   gesture at 1793932009616: the finger travelled 0 rows, the page showed 0
    CHECK ok 27-stream-uniform: uniform 0 (at most 0)

### mochi / youtube-mobile — pass

    mochi card on https://m.youtube.com/watch?v=dQw4w9WgXcQ: connected at +2 s, load done at +27 s, 19 content frames at +27 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 8-rect sel:video
    WAIT ok 10-changed 0,130,756,560
    WAIT ok 11-changed 0,130,756,560

### mojo / addon-site — pass

    mojo card on https://addons.riverstonerelay.org/: connected at +2 s, load done at +12 s, 10 content frames at +12 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_CO
    WAIT ok 8-rect sel:form.search input
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 105 landed on document y 32)
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 13-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 15-url: https://addons.riverstonerelay.org/personas/
    WAIT ok 16-rect sel:a.button[data-browsertheme]
    CHECK ok 18-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 20-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 21-pref lightweightThemes.isThemeSelected = [false]

### mojo / addons-lifecycle — pass

    mojo card on about:addons: connected at +1 s, load done at +11 s, 5 content frames at +12 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 249 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='pad01@jihad.test']
    WAIT ok 13-rect anon:richlistitem[value='pad01@jihad.test']|disable-btn
    WAIT ok 15-rect anon:richlistitem[value='pad01@jihad.test']|enable-btn
    WAIT ok 17-rect anon:richlistitem[value='pad01@jihad.test']|undo-btn
    WAIT ok 19-rect anon:richlistitem[value='pad01@jihad.test']|remove-btn

### mojo / addons-list-pan — pass

    mojo card on about:addons: connected at +1 s, load done at +11 s, 6 content frames at +12 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 249 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:richlistitem[value='pad12@jihad.test']
    WAIT ok 15-still 0,100,756,1000
    WAIT ok 17-rect sel:richlistitem[value='pad12@jihad.test'][selected='true']
    CHECK ok 18-hit: clickAt 843,1439 inside sel:richlistitem[value="pad12@jihad.test"] at 294,1412 656x99

### mojo / addons-page — pass

    mojo card on about:addons: connected at +2 s, load done at +9 s, 7 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#category-extension
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 249 landed on document y 224)
    WAIT ok 9-rect sel:#category-extension[selected='true']
    WAIT ok 10-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:#category-plugin[selected='true']
    WAIT ok 14-rect sel:#category-extension[selected='true']
    WAIT ok 16-rect sel:#detail-view:not([hidden])
    CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill]

### mojo / addons-search-hide-key — pass

    mojo card on about:addons: connected at +1 s, load done at +8 s, 2 content frames at +38 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#header-search
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 110 landed on document y 47)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 12-still sel:#header-search
    CHECK ok 14-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 15-still sel:#header-search
    WAIT ok 17-still sel:#header-search
    CHECK ok 18-region-equal: region 470,94-746,125: colours 97 hash 15386459 rows 31 uniform 0 light 94-124 blue none = the sample's hash
    CHECK ok 20-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / appmenu-find — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 10-differs 0,30,300,180
    WAIT ok 11-still 0,30,300,180
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 16-log: [jihad-bs] find 'Zebrafind' -> 1

### mojo / appmenu-print — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 11-differs 0,30,300,180
    WAIT ok 12-still 0,30,300,180
    WAIT ok 14-line: [jihad-bs] luna: findPrinters (started)
    WAIT ok 15-differs 0,200,756,960

### mojo / appmenu-settings-keyboard-down — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    WAIT ok 15-load
    CHECK ok 16-url: load done uri=about:settings#chrome=%7B%22searchEngine%22%3A%22duckduckgo-lite%22%2C%22homeUrl%22%3A%22https%3A%2F%2Flite.duckduckgo.com%2Flite%2F%22%2C%22startLinks%22%3A%5B%7B%22title%22%3A%22DuckDuckGo%22%2C%22url%22%3A%22https%3A%2F%2Fduckduckgo.com%2F%22%7D%2C%7B%22title%22%3A%22Wikipedia%22%2C%22url%22%3A%2
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 18-rect page-title
    CHECK ok 19-content: region 0,100-768,700: colours 448 hash 1c4c8669 rows 600 uniform 0 light none blue none; page-title on the screen at 20,91-744,125, text holds [Settings]

### mojo / audio-heard — a person's

    hearing the speaker; the route and the mixer are measured by script (cavekit-addons-extensions R7)

### mojo / busy-script-stop — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +8 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 9-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 13-rect sel:h1
    CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36

### mojo / card-dialog — pass

    mojo card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +1 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect c
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 133 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 13-text r = [c: true]
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-2.fifo
    CHECK ok 17-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 18-text r = [c: false]

### mojo / card-select — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 9-text picked = [picked: banana]

### mojo / chrome-buttons — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 9-load
    WAIT ok 11-url: https://lite.duckduckgo.com/lite/
    WAIT ok 13-url: http://192.168.1.245:8077/jihad-long-static.html
    WAIT ok 15-url: https://lite.duckduckgo.com/lite/
    WAIT ok 18-differs 0,100,756,700

### mojo / chrome-history — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +7 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### mojo / chrome-padlock — pass

    mojo card on https://example.com/: connected at +2 s, load done at +12 s, 8 content frames at +12 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:p
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 389 landed on document y 316)
    WAIT ok 12-line: [jihad-bs] luna: pageCertificate (sent)
    WAIT ok 13-differs 0,100,756,960

### mojo / chrome-share — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### mojo / ddg-results — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 8-launch: load done
    WAIT ok 9-line: [jihad-bs] titleAndUrl title=[webOS at DuckDuckGo] uri=https://duckduckgo.com/?q=webOS&ia=web back=1 fwd=0
    CHECK ok 10-content: region 0,100-756,900: colours 1055 hash 336e0e09 rows 800 uniform 193 light 100-899 blue 138-693; sel:body on the screen at 0,73-768,2623

### mojo / dialog-slow-answer — pass

    mojo card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +1 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect c
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 133 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text r = [c: true]

### mojo / download-finishes — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-7

### mojo / drag-and-pinch-page — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 9-card-scroll-min: the card's scroll y 605 (at least 400)
    WAIT ok 11-zoom
    CHECK ok 12-zoom-min: zoom 2.6684 (at least 1.5)
    CHECK ok 13-content: region 0,150-756,600: colours 342 hash b7746bec rows 450 uniform 198 light 177-599 blue none; sel:body on the screen at -240,-868-1798,53801
    WAIT ok 15-zoom
    WAIT ok 17-zoom
    CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15)

### mojo / etsy-search — pass

    mojo card on https://www.etsy.com/search?q=webos: connected at +1 s, load done at +40 s, 16 content frames at +40 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_CO
    WAIT ok 8-rect sel:body
    WAIT ok 9-url: https://www.etsy.com/search?q=webos
    CHECK ok 10-url: load done uri=https://www.etsy.com/search?q=webos
    WAIT ok 11-content 0,100,768,900
    CHECK ok 12-content: region 0,100-768,900: colours 4096 hash 65fedf66 rows 800 uniform 158 light 100-899 blue 502-537; sel:body on the screen at 0,73-768,8627

### mojo / everyday-browsing — pass

    mojo card on https://duckduckgo.com/: connected at +1 s, load done at +38 s, 92 content frames at +38 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:textarea[name='q']
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 436 landed on document y 363)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 12-launch: load done
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 149 landed on document y 110)
    CHECK ok 15-card-scroll-min: the card's scroll y 777 (at least 300)
    WAIT ok 17-url: https://duckduckgo.com/

### mojo / express-burst-wiki — pass

    mojo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +22 s, 8 content frames at +22 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 8-rect sel:#firstHeading
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 149 landed on document y 110)
    WAIT ok 14-zoom
    WAIT ok 15-settled: the card's scroll 532 1121
    CHECK ok 16-zoom-min: zoom 2.3273 (at least 1.0)
    CHECK ok 17-content: region 0,100-756,700: colours 748 hash 1d49e678 rows 600 uniform 0 light 100-698 blue 477-699; sel:body on the screen at -531,-1090-2065,2291

### mojo / flash-sound — a person's

    hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route)

### mojo / fonts-pick — pass

    mojo card on about:settings: connected at +1 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    WAIT ok 11-rect fonts-reset
    WAIT ok 13-pref font.name.serif.x-western = [Dotum]
    WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.]

### mojo / forget-identities — pass

    mojo card on about:settings: connected at +1 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.]

### mojo / frame-field — pass

    mojo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +2 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 7-rect f
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 142 landed on document y 88)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 11-visible: f on the screen at 13,313-759,520 (the reach ends at 600, keyboard 1)
    WAIT ok 14-differs f
    CHECK ok 15-region-changed: region 13,314-756,519: colours 203 hash 3245e2d2 rows 205 uniform 0 light 315-517 blue none against the sample's hash 14b1cd9a
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 18-still f
    WAIT ok 20-still f
    CHECK ok 21-region-equal: region 13,314-756,519: colours 202 hash ba939f24 rows 205 uniform 0 light 315-517 blue none = the sample's hash
    CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / keyboard-address — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +7 s, 2 content frames at +37 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / keyboard-arbitration — not run

    hardware-blocked (no physical keyboard exists)

### mojo / keyboard-fields — pass

    mojo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +1 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 292 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 10-visible: area on the screen at 9,151-484,191 (the reach ends at 600, keyboard 1)
    CHECK ok 12-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 13-launch: load done
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 17-visible: field on the screen at 16,275-522,310 (the reach ends at 600, keyboard 1)

### mojo / long-press-image — pass

    mojo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +22 s, 10 content frames at +22 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 8-rect sel:.infobox img
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 149 landed on document y 110)
    WAIT ok 12-differs 0,100,756,900

### mojo / long-press-menu — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mojo / long-press-text-field — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 10-differs 0,100,756,600
    WAIT ok 13-still 0,100,756,600
    WAIT ok 15-line: [jihad-bs] hold at 270,219 editable=1 wasFocused=0
    WAIT ok 18-still 0,100,400,240
    WAIT ok 20-load
    CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mojo / mobile-wiki-pinch — pass

    mojo card on https://en.m.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +23 s, 12 content frames at +23 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 8-rect sel:#firstHeading
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 149 landed on document y 110)
    WAIT ok 11-still 0,100,756,700
    CHECK ok 12-zoom-min: zoom 0.6882 (at least 0.65)
    CHECK ok 13-content: region 0,100-756,700: colours 4096 hash b52107c3 rows 600 uniform 67 light 100-699 blue 227-231; sel:body on the screen at 0,73-768,1073
    CHECK ok 14-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    WAIT ok 16-zoom
    CHECK ok 17-zoom-min: zoom 1.8163 (at least 1.05)
    CHECK ok 18-content: region 0,100-756,700: colours 4096 hash efbcf2fb rows 600 uniform 0 light 100-699 blue none; sel:body on the screen at -346,-194-1680,2444

### mojo / mojo-address-typing — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 13-load
    CHECK ok 14-url: load done uri=https://lite.duckduckgo.com/lite/?q=webos

### mojo / mojo-launch-from-app — a person's

    a launch from another app is not emulated (a launch with a target from the command line showed the page with no keyboard, 2026-10-24)

### mojo / pan-long-drag — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 8-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 9-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 10-content: region 0,150-768,400: colours 293 hash 8b1a67cb rows 250 uniform 0 light 150-399 blue none; sel:#content p on the screen at 16,145-748,201, text holds [quick brown]
    CHECK ok 14-card-scroll: the card's scroll y 594 (want 650 within 80); the engine's 594
    WAIT ok 15-still header
    CHECK ok 16-region-equal: region 0,29-756,83: colours 185 hash eeaac7d6 rows 54 uniform 33 light none blue none = the sample's hash
    CHECK ok 17-region-changed: region 0,300-756,900: colours 488 hash 8a2d02d6 rows 600 uniform 219 light 302-899 blue none against the sample's hash 5ec8059e
    CHECK ok 18-top: the page's top measured 73 (the tap at y 214 landed on document y 780), the model says 73 (header hidden: 1)
    CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### mojo / pan-long-flick — known

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +8 s, 2 content frames at +38 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 18-card-scroll-min: the card's scroll y 3285 (at least 300)
    STREAM fs-mojo-pan-long-flick.fbs: 131 samples (14.1/s asked 15, read median 2.23 ms max 108.25 ms, mmap stride 8, re-read 14, torn 1), 102 flips, cpu 6.9% of a core, polls 578/s | shown frames while moving: 98 gaps, median 34.0 ms, p90 54 ms, max 275 ms | shown scroll: 909 rows over the stream (2 samples with the shift unknown); stale bands 0, uniform bands 10, still rows moved 0; the columns outside 0-756 changed in 33 samples | tracking (finger down, 23 samples): error median 20 px, max 450 px; lag median 232 ms, max 852 ms |   gesture at 1793947407374: the finger travelled 600 rows, the page show
    CHECK ok 16-stream-stale: stale 0 (at most 0)
    CHECK FAIL 17-stream-uniform: uniform 10 (want at most 0)

### mojo / pan-speeds — known

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +8 s, 2 content frames at +38 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 13-card-scroll-min: the card's scroll y 603 (at least 500)
    CHECK ok 22-card-scroll-min: the card's scroll y 4550 (at least 2500)
    STREAM fs-mojo-pan-speeds.fbs: 262 samples (14.2/s asked 15, read median 2.35 ms max 218.21 ms, mmap stride 8, re-read 24, torn 0), 286 flips, cpu 7.1% of a core, polls 539/s | shown frames while moving: 278 gaps, median 31.0 ms, p90 50 ms, max 288 ms | shown scroll: 3046 rows over the stream (3 samples with the shift unknown); stale bands 0, uniform bands 14, still rows moved 0; the columns outside 0-756 changed in 110 samples | tracking (finger down, 121 samples): error median -12 px, max 262 px; lag median 78 ms, max 1052 ms |   gesture at 1793941476547: the finger travelled 650 rows, the page sho
    CHECK ok 19-stream-stale: stale 0 (at most 0)
    CHECK FAIL 20-stream-uniform: uniform 14 (want at most 0)
    CHECK ok 21-stream-error-max: error-max 262 (at most 500)

### mojo / persona-cycle — pass

    mojo card on about:personas: connected at +1 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#mine .card
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 271 landed on document y 198)
    WAIT ok 11-rect sel:#files button
    CHECK ok 13-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 15-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 16-pref lightweightThemes.isThemeSelected = [true]
    WAIT ok 17-rect sel:#mine .card:nth-child(2) button
    CHECK ok 18-text-has: sel:#mine .card:nth-child(2) = [Carbon Light 2.0Author unknownTake offRemove] holds [Carbon]
    WAIT ok 20-text status-text = [Persona taken off.]
    WAIT ok 22-text sel:#mine .card:nth-child(2) = [Carbon Light 2.0Author unknownWearRemove it?]
    CHECK ok 23-text-has: sel:#mine .card:nth-child(2) = [Carbon Light 2.0Author unknownWearRemove it?] holds [Remove it?]
    CHECK ok 25-text-has: sel:#mine .card:nth-child(2) = [Carbon Light 2.0Author unknownWearRemove it?] holds [Remove it?]
    WAIT ok 27-text status-text = [Removing Carbon Light…]

### mojo / persona-wear — pass

    mojo card on about:personas: connected at +1 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#online .card button
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 271 landed on document y 198)
    CHECK ok 11-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text status-text = [Wearing Autumn Forest.]
    WAIT ok 15-pref lightweightThemes.isThemeSelected = [true]
    WAIT ok 17-text status-text = [Undone.]
    WAIT ok 18-pref lightweightThemes.isThemeSelected = [false]

### mojo / personas-flick — pass

    mojo card on about:personas: connected at +2 s, load done at +10 s, 13 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#mine .card
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 271 landed on document y 198)
    CHECK ok 10-card-scroll-min: the card's scroll y 1602 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### mojo / personas-page-steady — pass

    mojo card on about:personas: connected at +1 s, load done at +9 s, 12 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-rect sel:#mine .card
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 271 landed on document y 198)
    CHECK ok 16-content: region 0,100-768,900: colours 4096 hash 0eec4290 rows 800 uniform 0 light 182-737 blue 156-745; sel:body on the screen at 0,-425-764,2442
    STREAM fs-mojo-personas-page-steady.fbs: 89 samples (15.0/s asked 15, read median 2.33 ms max 57.06 ms, mmap stride 8, re-read 7, torn 0), 111 flips, cpu 7.1% of a core, polls 636/s | shown frames while moving: 110 gaps, median 27.0 ms, p90 38 ms, max 261 ms | shown scroll: 500 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 0-756 changed in 41 samples | tracking (finger down, 48 samples): error median -10.0 px, max 60 px; lag median 74.5 ms, max 339 ms |   gesture at 1793941789180: the finger travelled 500 rows, the pag
    CHECK ok 15-stream-uniform: uniform 0 (at most 0)

### mojo / proxy-login-mojo — pass

    mojo card on about:settings: connected at +1 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 9-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    CHECK ok 12-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 14-still 0,440,756,720
    CHECK ok 21-dialog: [jihad-bs] dialog auth -> ACCEPT (answer=1)
    WAIT ok 22-line: [jihad-bs] late load done uri=http://example.com/jihad-proxy-login

### mojo / proxy-own — pass

    mojo card on about:settings: connected at +1 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 13-rect sel:.row[data-pref='network.proxy.http'] input
    CHECK ok 15-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 24-pref network.proxy.http = [192.168.1.245]
    CHECK ok 26-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 28-dialog: [jihad-bs] dialog auth -> cancel (answer=0)

### mojo / proxy-section-redraw — pass

    mojo card on about:settings: connected at +1 s, load done at +8 s, 10 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 11-rect sel:.row[data-pref='network.proxy.type'] select
    WAIT ok 13-rect sel:.row[data-pref='network.proxy.http'] input
    WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input

### mojo / relay-choice — pass

    mojo card on about:settings: connected at +1 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 12-rect sel:.row[data-pref='network.proxy.type'] select
    WAIT ok 14-pref network.proxy.http = [proxy.riverstonerelay.org]
    WAIT ok 15-launch: load done
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 389 landed on document y 316)
    CHECK ok 17-text-has: sel:p = [This domain is for use in documentation examples without needing permission. This is not a service; avoid relying on it for testing and monitoring purposes.] holds [documentation]

### mojo / rotate — a person's

    the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side

### mojo / script-hold-zoom — pass

    mojo card on http://192.168.1.245:8077/jihad-script-hold.html: connected at +1 s, load done at +8 s, 3 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 101 landed on document y 28)
    WAIT ok 9-zoom
    CHECK ok 10-zoom-min: zoom 1.9071 (at least 1.3)
    CHECK ok 12-card-scroll-min: the card's scroll y 433 (at least 200)
    WAIT ok 14-zoom
    WAIT ok 16-zoom
    CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2)

### mojo / settings-every-control — by a tool

    tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt)

### mojo / settings-field-saves — pass

    mojo card on about:settings: connected at +1 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 16-pref browser.display.foreground_color = [#000000x]
    WAIT ok 17-still sel:.row[data-pref='browser.display.foreground_color'] input
    WAIT ok 19-still sel:.row[data-pref='browser.display.foreground_color'] input
    CHECK ok 20-region-equal: region 416,643-735,703: colours 38 hash c1c06359 rows 60 uniform 0 light 643-702 blue none = the sample's hash

### mojo / settings-flick — pass

    mojo card on about:settings: connected at +1 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    CHECK ok 10-card-scroll-min: the card's scroll y 675 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### mojo / settings-int-emptied — pass

    mojo card on about:settings: connected at +3 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 16-pref browser.sessionhistory.max_entries = [20]

### mojo / settings-persist — pass

    mojo card on about:settings: connected at +1 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-pref general.smoothScroll = [true]
    WAIT ok 14-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 16-pref general.smoothScroll = [true]

### mojo / settings-tabs — pass

    mojo card on about:settings: connected at +1 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    CHECK ok 9-text: page-title = [Settings]
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content]

### mojo / settings-typing — pass

    mojo card on about:settings: connected at +1 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 15-pref browser.display.background_color = [#FFFFFFx]
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / ssl-confirm — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog ssl-confirm -> cancel (answer=0)

### mojo / tabbed-browsing — not run

    not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word)

### mojo / touchpad-go — not run

    owner-accepted without hardware (device-build R6)

### mojo / typing-caret — pass

    mojo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +1 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 292 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 11-text echo = [plain n=6 c=0 last=[abcdef] value=[abcdef] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 13-text echo = [plain n=12 c=0 last=[] value=[] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / video-controls — pass

    mojo card on http://192.168.1.245:8077/jihad-media-test.html: connected at +1 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 7-rect status
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 93 landed on document y 25)
    WAIT ok 10-changed sel:video
    WAIT ok 11-changed sel:video
    WAIT ok 13-still sel:video
    WAIT ok 15-still sel:video
    CHECK ok 16-region-equal: region 16,116-517,396: colours 2442 hash dd0c46e5 rows 280 uniform 0 light 383-388 blue none = the sample's hash

### mojo / webgl-cube — pass

    mojo card on http://192.168.1.245:8077/jihad-webgl-cube.html: connected at +1 s, load done at +9 s, 21 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 7-rect c
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 115 landed on document y 57)
    CHECK ok 9-text-has: out = [webgl fps=24.0 shaders=ok texture=ok framebuffer=ok readPixels=ok maxtex=4096 ext=14 missing=ANGLE_instanced_arrays,WEBGL_compressed_texture_s3tc,EXT_blend_minmax,EXT_shader_texture_lod depthrb=ok renderer=Mozilla vendor=Mozilla] holds [webgl]
    CHECK ok 10-content: region 105,166-457,515: colours 716 hash 8ca1737a rows 349 uniform 109 light none blue 217-434; c on the screen at 105,165-457,516
    WAIT ok 11-changed c
    WAIT ok 13-zoom
    CHECK ok 15-content: region 0,100-768,900: colours 877 hash cb7d0379 rows 800 uniform 323 light none blue 262-333; c on the screen at -22,46-526,595
    WAIT ok 16-changed c

### mojo / webone — not run

    needs webone, keyboard-map (run with --with webone,keyboard-map)

### mojo / wikipedia-keyboard-zoom — pass

    mojo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +23 s, 18 content frames at +23 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 8-rect sel:input[name='search']
    WAIT ok 9-still 0,100,756,700
    CHECK ok 10-zoom: zoom 0.6882 (want 0.69 within 0.06)
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 149 landed on document y 110)
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 14-still 0,100,756,500
    CHECK ok 15-zoom: zoom 0.6882 (want 0.69 within 0.06)
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 18-still 0,100,756,700
    CHECK ok 19-zoom: zoom 0.6882 (want 0.69 within 0.06)
    CHECK ok 20-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]

### mojo / wikipedia-zoom — known

    mojo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +23 s, 13 content frames at +23 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 11-rect sel:.infobox
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 149 landed on document y 110)
    WAIT ok 15-zoom
    CHECK ok 16-zoom-min: zoom 2.4459 (at least 1.3)
    WAIT ok 18-zoom
    WAIT ok 20-zoom
    WAIT ok 22-still 0,100,756,700
    CHECK ok 24-zoom: zoom 0.6882 (want 0.69 within 0.12)
    CHECK ok 25-content: region 0,100-756,700: colours 4096 hash 4e113994 rows 600 uniform 64 light 100-699 blue 227-231; sel:body on the screen at 0,73-768,1073
    CHECK ok 26-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    STREAM fs-mojo-wikipedia-zoom.fbs: 256 samples (15.0/s asked 15, read median 1.69 ms max 50.23 ms, mmap stride 8, re-read 7, torn 0), 178 flips, cpu 5.7% of a core, polls 688/s | shown frames while moving: 170 gaps, median 43.0 ms, p90 150 ms, max 291 ms | shown scroll: 41 rows over the stream (44 samples with the shift unknown); stale bands 1, uniform bands 6, still rows moved 0; the columns outside 170-600 changed in 23 samples | tracking (finger down, 40 samples): error median 0.0 px, max 15 px; lag median 400.5 ms, max 852 ms |   gesture at 1793943291259: the finger travelled 0 rows, the page sho
    CHECK FAIL 27-stream-uniform: uniform 6 (want at most 0)

### mojo / youtube-mobile — pass

    mojo card on https://m.youtube.com/watch?v=dQw4w9WgXcQ: connected at +1 s, load done at +25 s, 22 content frames at +25 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 8-rect sel:video
    WAIT ok 10-changed 0,130,756,560
    WAIT ok 11-changed 0,130,756,560

