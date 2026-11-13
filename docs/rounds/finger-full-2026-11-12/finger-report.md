# Finger suite report — 2026-11-12T19:59:21

Shells: enyo, mochi, mojo. Scripts: 97. 16350 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | addon-site | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-addons-site.md R7 |
| enyo | addons-lifecycle | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/profile/extensions | cavekit-addons-extensions.md R2 |
| enyo | addons-list-pan | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/profile/extensions | cavekit-chrome-look.md R2 |
| enyo | addons-page | **pass** | 7 ok, 0 failed | CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill] | cavekit-addons-extensions.md R2 |
| enyo | addons-search-hide-key | **pass** | 8 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| enyo | address-select-all | **fail** | 3 ok, 1 failed | CHECK FAIL 12-wait-changed: the region 200,88,470,135 did not change within the bound; frames: kept/enyo-address-select-all-12-wait-changed_.jpg | cavekit-chrome-look.md R4 |
| enyo | appmenu-find | **pass** | 4 ok, 0 failed | WAIT ok 16-log: [jihad-bs] find 'zebrafind' -> 1 | cavekit-ui-shell.md R7 |
| enyo | appmenu-print | **fail** | 3 ok, 3 failed | CHECK FAIL 14-wait-line: no line luna: findPrinters within the bound / CHECK FAIL 15-wait-differs: 0,200,756,960 still shows the sample page after the bound / CHECK FAIL 16-wait-log: no line print: mdns: .* -> [0-9.]*:[0-9]* in the daemon log within the bound; frames: kept/enyo-appmenu-print-14-wait-line_.jpg, kept/enyo-appmenu-print-15-wait-differs_.jpg, kept/enyo-appmenu-print-16-wait-log_.jpg, kept/enyo-appmenu-print-print-dialog_.jpg | cavekit-ui-shell.md R7 |
| enyo | appmenu-settings-keyboard-down | **fail** | 5 ok, 3 failed | CHECK FAIL 16-url: load done uri=[about:reader?url=http%3A%2F%2F192.168.1.245%3A8077%2Fjihad-long-static.html] lacks about:settings / CHECK FAIL 18-wait-rect: no element page-title with a size within the bound / CHECK FAIL 19-content: region 0,100-768,700: colours 433 hash b3587c6b rows 600 uniform ; frames: kept/enyo-appmenu-settings-keyboard-down-16-url_.jpg, kept/enyo-appmenu-settings-keyboard-down-18-wait-rect_.jpg, kept/enyo-appmenu-settings-keyboard-down-19-content_.jpg | cavekit-chrome-look.md R4 |
| enyo | audio-heard | **a person's** | 0 ok, 0 failed | hearing the speaker; the route and the mixer are measured by script (cavekit-addons-extensions R7) | cavekit-device-build.md R4 |
| enyo | autoplay-sound | **pass** | 3 ok, 0 failed | WAIT ok 13-text status = [playing]; frames: kept/enyo-autoplay-sound-refused_.jpg | cavekit-gre-widgets.md R1 |
| enyo | autoplay-unmute | **pass** | 3 ok, 0 failed | WAIT ok 11-text status = [paused t=29.8]; frames: kept/enyo-autoplay-unmute-paused-on-unmute_.jpg | cavekit-gre-widgets.md R1 |
| enyo | busy-script-stop | **pass** | 4 ok, 0 failed | CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36; frames: kept/enyo-busy-script-stop-busy-dialog_.jpg | cavekit-modern-web.md R2 |
| enyo | card-dialog | **pass** | 7 ok, 0 failed | WAIT ok 18-text r = [c: false] | cavekit-browser-services.md R3 |
| enyo | card-select | **pass** | 1 ok, 0 failed | WAIT ok 9-text picked = [picked: banana] | cavekit-device-build.md R4 |
| enyo | chrome-buttons | **pass** | 5 ok, 0 failed | WAIT ok 18-differs 0,100,756,700; frames: kept/enyo-chrome-buttons-new-card_.jpg | cavekit-device-build.md R4 |
| enyo | chrome-history | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/enyo-chrome-history-history-panel_.jpg | cavekit-device-build.md R4 |
| enyo | chrome-padlock | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-ui-shell.md R8 |
| enyo | chrome-share | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/enyo-chrome-share-share-menu_.jpg | cavekit-device-build.md R4 |
| enyo | ddg-results | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-modern-web.md R2 |
| enyo | dialog-slow-answer | **pass** | 4 ok, 0 failed | WAIT ok 14-text r = [c: true] | cavekit-finger-testing.md R4 |
| enyo | download-finishes | **pass** | 1 ok, 0 failed | WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-32 | cavekit-device-build.md R4 |
| enyo | download-list | **pass** | 9 ok, 0 failed | WAIT ok 24-content 460,90,760,300; frames: kept/enyo-download-list-panel_.jpg, kept/enyo-download-list-downloads_.jpg | cavekit-device-build.md R4 |
| enyo | drag-and-pinch-page | **pass** | 7 ok, 0 failed | CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15) | cavekit-input-bridging.md R3 |
| enyo | etsy-search | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-modern-web.md R2 |
| enyo | everyday-browsing | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-engine-update.md R8 |
| enyo | express-burst-wiki | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-gpu-rendering.md R3 |
| enyo | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| enyo | flash-fullscreen | **fail** | 7 ok, 1 failed | CHECK FAIL 20-flash-behind: the colour 00ff00 in 73 of 192 cells (wanted yes); frames: kept/enyo-flash-fullscreen-fullscreen_.jpg, kept/enyo-flash-fullscreen-20-flash-behind_.jpg | cavekit-flash.md R4 |
| enyo | flash-input-cover | **fail** | 5 ok, 2 failed | CHECK FAIL 16-above-keyboard: m on the screen at 40,602-440,902, under the keyboard / CHECK FAIL 18-flash-at: the movie reported down 201,80 for stage 200,150 (more than 3 off); frames: kept/enyo-flash-input-cover-16-above-keyboard_.jpg, kept/enyo-flash-input-cover-18-flash-at_.jpg | cavekit-flash.md R5 |
| enyo | flash-input-hold | **pass** | 8 ok, 0 failed | CHECK ok 16-press-rate: 25.2 moves a second sent on (at least 20): ] a press of 1451 ms: 32 moves from the card in 1272 ms, 32 sent on, 586 ms in sending them | cavekit-flash.md R5 |
| enyo | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| enyo | flash-input-keyhold | **pass** | 7 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-flash.md R6 |
| enyo | flash-input-keys | **pass** | 13 ok, 0 failed | CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-flash.md R5 |
| enyo | flash-input-lock | **fail** | 14 ok, 1 failed | CHECK FAIL 22-number: n-move = [2] (wanted 5..400); frames: kept/enyo-flash-input-lock-22-number_.jpg | cavekit-flash.md R5 |
| enyo | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| enyo | flash-lane | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,287: 3.0 s, flips 43 (14.3/s), changes 0 (0.0/s), reads 44, distinct 1, spacing median 62 p90 85 max 197 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| enyo | flash-layout | **pass** | 17 ok, 0 failed | CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| enyo | flash-sound | **a person's** | 0 ok, 0 failed | hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route) | cavekit-engine-update.md R8 |
| enyo | flash-tags | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 55 (18.3/s), changes 40 (13.3/s), reads 56, distinct 41, spacing median 52 p90 82 max 126 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| enyo | flash-wmode | **pass** | 19 ok, 0 failed | CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| enyo | fonts-pick | **pass** | 5 ok, 0 failed | WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.] | cavekit-preferences-ui.md R9 |
| enyo | forget-identities | **pass** | 3 ok, 0 failed | WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.] | cavekit-site-identity.md R1 |
| enyo | frame-field | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| enyo | keyboard-address | **pass** | 2 ok, 0 failed | CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| enyo | keyboard-arbitration | **pass** | 8 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R7 |
| enyo | keyboard-fields | **pass** | 6 ok, 0 failed | CHECK ok 17-visible: field on the screen at 16,284-522,319 (the reach ends at 600, keyboard 1) | cavekit-input-bridging.md R2 |
| enyo | keyboard-keys | **pass** | 10 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R2 |
| enyo | long-press-image | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-ui-shell.md R7 |
| enyo | long-press-menu | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| enyo | long-press-text-field | **pass** | 6 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html; frames: kept/enyo-long-press-text-field-hold-text_.jpg, kept/enyo-long-press-text-field-hold-field_.jpg | cavekit-ui-shell.md R7 |
| enyo | media-controls | **pass** | 10 ok, 0 failed | CHECK ok 30-visible: anon:#v/controlBar on the screen at 16,581-586,603 (the reach ends at 1024, keyboard 0); frames: kept/enyo-media-controls-controls_.jpg, kept/enyo-media-controls-bar_.jpg | cavekit-gre-widgets.md R1 |
| enyo | media-scrub-drag | **pass** | 8 ok, 0 failed | WAIT ok 25-text seek = [60]; frames: kept/enyo-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |
| enyo | mobile-wiki-pinch | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-mojo-ui.md R6 |
| enyo | pan-long-drag | **pass** | 9 ok, 0 failed | CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| enyo | pan-long-flick | **known** | 1 ok, 2 failed | the open defect T-567, T-567: CHECK FAIL 16-stream-stale: stale 1 (want at most 0) / CHECK FAIL 17-stream-uniform: uniform 38 (want at most 0) | cavekit-gpu-rendering.md R3 |
| enyo | pan-speeds | **known** | 3 ok, 2 failed | the open defect T-567, T-567: CHECK FAIL 19-stream-stale: stale 3 (want at most 0) / CHECK FAIL 20-stream-uniform: uniform 31 (want at most 0) | cavekit-finger-testing.md R4 |
| enyo | persona-cycle | **pass** | 12 ok, 0 failed | persona file: put | cavekit-personas-ui.md R6 |
| enyo | persona-wear | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-personas-ui.md R6 |
| enyo | personas-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| enyo | personas-page-steady | **pass** | 3 ok, 0 failed | CHECK ok 15-stream-uniform: uniform 0 (at most 0) | cavekit-personas-ui.md R9 |
| enyo | proxy-login | **pass** | 11 ok, 0 failed | auth proxy: running; frames: kept/enyo-proxy-login-auth-dialog_.jpg | cavekit-proxies-certs.md R5 |
| enyo | proxy-own | **pass** | 8 ok, 0 failed | auth proxy: running | cavekit-proxies-certs.md R1 |
| enyo | proxy-section-redraw | **pass** | 5 ok, 0 failed | WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input | cavekit-proxies-certs.md R1 |
| enyo | relay-choice | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-proxies-certs.md R2 |
| enyo | rotate | **a person's** | 0 ok, 0 failed | the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side | cavekit-device-build.md R4 |
| enyo | screen-lock | **pass** | 5 ok, 0 failed | WAIT ok 34-text status = [paused t=29.2]; frames: kept/enyo-screen-lock-lock-button_.jpg, kept/enyo-screen-lock-locked_.jpg, kept/enyo-screen-lock-ask_.jpg, kept/enyo-screen-lock-unlocked_.jpg | cavekit-gre-widgets.md R1 |
| enyo | script-hold-zoom | **pass** | 6 ok, 0 failed | CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2) | cavekit-gpu-rendering.md R3 |
| enyo | settings-every-control | **by a tool** | 0 ok, 0 failed | tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt) | cavekit-preferences-ui.md R6 |
| enyo | settings-field-saves | **pass** | 8 ok, 0 failed | CHECK ok 20-region-equal: region 416,644-735,704: colours 49 hash 34233dc3 rows 60 uniform 0 light 644-703 blue none = the sample's hash | cavekit-chrome-look.md R4 |
| enyo | settings-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| enyo | settings-int-emptied | **pass** | 5 ok, 0 failed | WAIT ok 16-pref browser.sessionhistory.max_entries = [20] | cavekit-preferences-ui.md R6 |
| enyo | settings-persist | **pass** | 5 ok, 0 failed | WAIT ok 16-pref general.smoothScroll = [true] | cavekit-preferences-ui.md R2 |
| enyo | settings-tabs | **pass** | 4 ok, 0 failed | CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content] | cavekit-preferences-ui.md R6 |
| enyo | settings-typing | **pass** | 5 ok, 0 failed | CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-preferences-ui.md R6 |
| enyo | ssl-confirm | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-device-build.md R4 |
| enyo | tabbed-browsing | **not run** | 0 ok, 0 failed | not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word) | cavekit-tabbed-browsing.md R1 |
| enyo | touchpad-go | **not run** | 0 ok, 0 failed | owner-accepted without hardware (device-build R6) | cavekit-device-build.md R6 |
| enyo | typing-caret | **pass** | 4 ok, 0 failed | CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| enyo | video-controls | **pass** | 6 ok, 0 failed | CHECK ok 18-region-equal: region 16,176-517,456: colours 2439 hash e970f523 rows 280 uniform 0 light 444-448 blue none = the sample's hash | cavekit-gre-widgets.md R1 |
| enyo | webgl-cube | **pass** | 7 ok, 0 failed | WAIT ok 16-changed c | cavekit-gpu-rendering.md R8 |
| enyo | webone | **not run** | 0 ok, 0 failed | needs webone, keyboard-map (run with --with webone,keyboard-map) | cavekit-proxies-certs.md R3 |
| enyo | wikipedia-keyboard-zoom | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-mochi-ui.md R6 |
| enyo | wikipedia-zoom | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-gpu-rendering.md R6 |
| enyo | xpi-confirm | **pass** | 8 ok, 0 failed | xpi: 649 bytes served as jihad-effect.xpi; frames: kept/enyo-xpi-confirm-xpi-prompt_.jpg, kept/enyo-xpi-confirm-extensions-pane_.jpg | cavekit-addons-extensions.md R3 |
| enyo | youtube-feed-no-preview | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-device-headroom.md R4 |
| enyo | youtube-mobile | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-modern-web.md R2 |
| enyo | youtube-no-next | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-device-headroom.md R4 |
| enyo | zoom-dtap-column | **pass** | 6 ok, 0 failed | CHECK ok 14-content: region 0,100-756,700: colours 438 hash 2976668a rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-193-750,2000 | cavekit-input-bridging.md R5 |
| enyo | zoom-field-kept | **pass** | 6 ok, 0 failed | CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8) | cavekit-input-bridging.md R5 |
| enyo | zoom-locked-pinch-off | **pass** | 3 ok, 0 failed | CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05) | cavekit-input-bridging.md R5 |
| enyo | zoom-locked-pinch | **pass** | 4 ok, 0 failed | CHECK ok 13-zoom-min: zoom 1.9539 (at least 1.3) | cavekit-input-bridging.md R5 |
| enyo | zoom-new-page | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-input-bridging.md R3 |
| mochi | addon-site | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-addons-site.md R7 |
| mochi | addons-lifecycle | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser-mochi/profile/extensions | cavekit-addons-extensions.md R2 |
| mochi | addons-list-pan | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser-mochi/profile/extensions | cavekit-chrome-look.md R2 |
| mochi | addons-page | **pass** | 7 ok, 0 failed | CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill] | cavekit-addons-extensions.md R2 |
| mochi | addons-search-hide-key | **pass** | 8 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mochi | address-select-all | **pass** | 4 ok, 0 failed | CHECK ok 16-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mochi | appmenu-find | **pass** | 4 ok, 0 failed | WAIT ok 16-log: [jihad-bs] find 'zebrafind' -> 1 | cavekit-ui-shell.md R7 |
| mochi | appmenu-print | **fail** | 3 ok, 3 failed | CHECK FAIL 14-wait-line: no line luna: findPrinters within the bound / CHECK FAIL 15-wait-differs: 0,200,756,960 still shows the sample page after the bound / CHECK FAIL 16-wait-log: no line print: mdns: .* -> [0-9.]*:[0-9]* in the daemon log within the bound; frames: kept/mochi-appmenu-print-14-wait-line_.jpg, kept/mochi-appmenu-print-15-wait-differs_.jpg, kept/mochi-appmenu-print-16-wait-log_.jpg, kept/mochi-appmenu-print-print-dialog_.jpg | cavekit-ui-shell.md R7 |
| mochi | appmenu-settings-keyboard-down | **fail** | 5 ok, 3 failed | CHECK FAIL 16-url: load done uri=[about:reader?url=http%3A%2F%2F192.168.1.245%3A8077%2Fjihad-long-static.html] lacks about:settings / CHECK FAIL 18-wait-rect: no element page-title with a size within the bound / CHECK FAIL 19-content: region 0,100-768,700: colours 405 hash 83c241b8 rows 600 uniform ; frames: kept/mochi-appmenu-settings-keyboard-down-16-url_.jpg, kept/mochi-appmenu-settings-keyboard-down-18-wait-rect_.jpg, kept/mochi-appmenu-settings-keyboard-down-19-content_.jpg | cavekit-chrome-look.md R4 |
| mochi | audio-heard | **a person's** | 0 ok, 0 failed | hearing the speaker; the route and the mixer are measured by script (cavekit-addons-extensions R7) | cavekit-device-build.md R4 |
| mochi | busy-script-stop | **pass** | 4 ok, 0 failed | CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36; frames: kept/mochi-busy-script-stop-busy-dialog_.jpg | cavekit-modern-web.md R2 |
| mochi | card-dialog | **pass** | 7 ok, 0 failed | WAIT ok 18-text r = [c: false] | cavekit-browser-services.md R3 |
| mochi | card-select | **pass** | 1 ok, 0 failed | WAIT ok 9-text picked = [picked: banana] | cavekit-device-build.md R4 |
| mochi | chrome-buttons | **pass** | 5 ok, 0 failed | WAIT ok 18-differs 0,100,756,700; frames: kept/mochi-chrome-buttons-new-card_.jpg | cavekit-device-build.md R4 |
| mochi | chrome-history | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/mochi-chrome-history-history-panel_.jpg | cavekit-device-build.md R4 |
| mochi | chrome-padlock | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-ui-shell.md R8 |
| mochi | chrome-share | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/mochi-chrome-share-share-menu_.jpg | cavekit-device-build.md R4 |
| mochi | ddg-results | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-modern-web.md R2 |
| mochi | dialog-slow-answer | **pass** | 4 ok, 0 failed | WAIT ok 14-text r = [c: true] | cavekit-finger-testing.md R4 |
| mochi | download-finishes | **pass** | 1 ok, 0 failed | WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-34 | cavekit-device-build.md R4 |
| mochi | download-list | **pass** | 9 ok, 0 failed | WAIT ok 25-content 460,90,760,300; frames: kept/mochi-download-list-panel_.jpg, kept/mochi-download-list-downloads_.jpg | cavekit-device-build.md R4 |
| mochi | drag-and-pinch-page | **pass** | 7 ok, 0 failed | CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15) | cavekit-input-bridging.md R3 |
| mochi | etsy-search | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-modern-web.md R2 |
| mochi | everyday-browsing | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-engine-update.md R8 |
| mochi | express-burst-wiki | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-gpu-rendering.md R3 |
| mochi | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| mochi | flash-fullscreen | **pass** | 8 ok, 0 failed | CHECK ok 20-flash-behind: the colour 00ff00 in 180 of 192 cells (wanted yes); frames: kept/mochi-flash-fullscreen-fullscreen_.jpg | cavekit-flash.md R4 |
| mochi | flash-input-cover | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| mochi | flash-input-hold | **pass** | 8 ok, 0 failed | CHECK ok 16-press-rate: 24.8 moves a second sent on (at least 20): ] a press of 1439 ms: 33 moves from the card in 1328 ms, 33 sent on, 460 ms in sending them | cavekit-flash.md R5 |
| mochi | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| mochi | flash-input-keyhold | **pass** | 7 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-flash.md R6 |
| mochi | flash-input-keys | **pass** | 13 ok, 0 failed | CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-flash.md R5 |
| mochi | flash-input-lock | **pass** | 15 ok, 0 failed | CHECK ok 31-card-scroll-min: the card's scroll y 100 (at least 30) | cavekit-flash.md R5 |
| mochi | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| mochi | flash-lane | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,69-440,367: 3.0 s, flips 45 (15.0/s), changes 0 (0.0/s), reads 46, distinct 1, spacing median 66 p90 95 max 162 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| mochi | flash-layout | **pass** | 17 ok, 0 failed | CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mochi | flash-sound | **a person's** | 0 ok, 0 failed | hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route) | cavekit-engine-update.md R8 |
| mochi | flash-tags | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 42 (14.0/s), changes 42 (14.0/s), reads 43, distinct 43, spacing median 70 p90 80 max 96 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| mochi | flash-wmode | **pass** | 19 ok, 0 failed | CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mochi | fonts-pick | **pass** | 5 ok, 0 failed | WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.] | cavekit-preferences-ui.md R9 |
| mochi | forget-identities | **pass** | 3 ok, 0 failed | WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.] | cavekit-site-identity.md R1 |
| mochi | frame-field | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mochi | keyboard-address | **pass** | 2 ok, 0 failed | CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| mochi | keyboard-arbitration | **pass** | 8 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R7 |
| mochi | keyboard-fields | **pass** | 6 ok, 0 failed | CHECK ok 17-visible: field on the screen at 16,280-522,315 (the reach ends at 600, keyboard 1) | cavekit-input-bridging.md R2 |
| mochi | keyboard-keys | **pass** | 10 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R2 |
| mochi | long-press-image | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-ui-shell.md R7 |
| mochi | long-press-menu | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| mochi | long-press-text-field | **pass** | 6 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html; frames: kept/mochi-long-press-text-field-hold-text_.jpg, kept/mochi-long-press-text-field-hold-field_.jpg | cavekit-ui-shell.md R7 |
| mochi | media-controls | **pass** | 10 ok, 0 failed | CHECK ok 30-visible: anon:#v/controlBar on the screen at 16,577-586,599 (the reach ends at 1024, keyboard 0); frames: kept/mochi-media-controls-controls_.jpg, kept/mochi-media-controls-bar_.jpg | cavekit-gre-widgets.md R1 |
| mochi | media-scrub-drag | **pass** | 8 ok, 0 failed | WAIT ok 25-text seek = [60]; frames: kept/mochi-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |
| mochi | mobile-wiki-pinch | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-mojo-ui.md R6 |
| mochi | mochi-paste | **pass** | 4 ok, 0 failed | CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused); frames: kept/mochi-mochi-paste-mochi-field-sheet_.jpg | cavekit-mochi-ui.md R2 |
| mochi | pan-long-drag | **pass** | 9 ok, 0 failed | CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| mochi | pan-long-flick | **known** | 1 ok, 2 failed | the open defect T-567, T-567: CHECK FAIL 16-stream-stale: stale 6 (want at most 0) / CHECK FAIL 17-stream-uniform: uniform 17 (want at most 0) | cavekit-gpu-rendering.md R3 |
| mochi | pan-speeds | **known** | 4 ok, 1 failed | the open defect T-567: CHECK FAIL 20-stream-uniform: uniform 37 (want at most 0) | cavekit-finger-testing.md R4 |
| mochi | persona-cycle | **pass** | 12 ok, 0 failed | persona file: put | cavekit-personas-ui.md R6 |
| mochi | persona-wear | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-personas-ui.md R6 |
| mochi | personas-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| mochi | personas-page-steady | **pass** | 3 ok, 0 failed | CHECK ok 15-stream-uniform: uniform 0 (at most 0) | cavekit-personas-ui.md R9 |
| mochi | proxy-login | **pass** | 11 ok, 0 failed | auth proxy: running; frames: kept/mochi-proxy-login-auth-dialog_.jpg | cavekit-proxies-certs.md R5 |
| mochi | proxy-own | **pass** | 8 ok, 0 failed | auth proxy: running | cavekit-proxies-certs.md R1 |
| mochi | proxy-section-redraw | **pass** | 5 ok, 0 failed | WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input | cavekit-proxies-certs.md R1 |
| mochi | relay-choice | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-proxies-certs.md R2 |
| mochi | rotate | **a person's** | 0 ok, 0 failed | the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side | cavekit-device-build.md R4 |
| mochi | screen-lock | **fail** | 2 ok, 3 failed | CHECK FAIL 20-wait-text: status never read playing within the bound / CHECK FAIL 22-text: status = [paused t=24.9], want [playing] / CHECK FAIL 34-wait-text: status never read paused within the bound; frames: kept/mochi-screen-lock-lock-button_.jpg, kept/mochi-screen-lock-locked_.jpg, kept/mochi-screen-lock-20-wait-text_.jpg, kept/mochi-screen-lock-22-text_.jpg, kept/mochi-screen-lock-ask_.jpg, kept/mochi-screen-lock-unlocked_.jpg, kept/mochi-screen-lock-34-wait-text_.jpg | cavekit-gre-widgets.md R1 |
| mochi | script-hold-zoom | **pass** | 6 ok, 0 failed | CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2) | cavekit-gpu-rendering.md R3 |
| mochi | settings-every-control | **by a tool** | 0 ok, 0 failed | tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt) | cavekit-preferences-ui.md R6 |
| mochi | settings-field-saves | **pass** | 8 ok, 0 failed | CHECK ok 20-region-equal: region 416,645-735,705: colours 44 hash 9db921ae rows 60 uniform 0 light 645-704 blue none = the sample's hash | cavekit-chrome-look.md R4 |
| mochi | settings-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 97 | cavekit-chrome-look.md R3 |
| mochi | settings-int-emptied | **pass** | 5 ok, 0 failed | WAIT ok 16-pref browser.sessionhistory.max_entries = [20] | cavekit-preferences-ui.md R6 |
| mochi | settings-persist | **pass** | 5 ok, 0 failed | WAIT ok 16-pref general.smoothScroll = [true] | cavekit-preferences-ui.md R2 |
| mochi | settings-tabs | **pass** | 4 ok, 0 failed | CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content] | cavekit-preferences-ui.md R6 |
| mochi | settings-typing | **pass** | 5 ok, 0 failed | CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-preferences-ui.md R6 |
| mochi | ssl-confirm | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-device-build.md R4 |
| mochi | tabbed-browsing | **not run** | 0 ok, 0 failed | not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word) | cavekit-tabbed-browsing.md R1 |
| mochi | touchpad-go | **not run** | 0 ok, 0 failed | owner-accepted without hardware (device-build R6) | cavekit-device-build.md R6 |
| mochi | typing-caret | **pass** | 4 ok, 0 failed | CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| mochi | video-controls | **fail** | 4 ok, 2 failed | CHECK FAIL 12-wait-changed: the region sel:video did not change within the bound / CHECK FAIL 13-wait-changed: the region sel:video did not change within the bound; frames: kept/mochi-video-controls-12-wait-changed_.jpg, kept/mochi-video-controls-13-wait-changed_.jpg | cavekit-gre-widgets.md R1 |
| mochi | webgl-cube | **pass** | 7 ok, 0 failed | WAIT ok 16-changed c | cavekit-gpu-rendering.md R8 |
| mochi | webone | **not run** | 0 ok, 0 failed | needs webone, keyboard-map (run with --with webone,keyboard-map) | cavekit-proxies-certs.md R3 |
| mochi | wikipedia-keyboard-zoom | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-mochi-ui.md R6 |
| mochi | wikipedia-zoom | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-gpu-rendering.md R6 |
| mochi | xpi-confirm | **pass** | 8 ok, 0 failed | xpi: 649 bytes served as jihad-effect.xpi; frames: kept/mochi-xpi-confirm-xpi-prompt_.jpg, kept/mochi-xpi-confirm-extensions-pane_.jpg | cavekit-addons-extensions.md R3 |
| mochi | youtube-mobile | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-modern-web.md R2 |
| mochi | youtube-no-next | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-device-headroom.md R4 |
| mochi | zoom-dtap-column | **pass** | 6 ok, 0 failed | CHECK ok 14-content: region 0,100-756,700: colours 402 hash 6c6a1b91 rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-195-750,1998 | cavekit-input-bridging.md R5 |
| mochi | zoom-field-kept | **pass** | 6 ok, 0 failed | CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8) | cavekit-input-bridging.md R5 |
| mochi | zoom-locked-pinch-off | **pass** | 3 ok, 0 failed | CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05) | cavekit-input-bridging.md R5 |
| mochi | zoom-locked-pinch | **pass** | 4 ok, 0 failed | CHECK ok 13-zoom-min: zoom 1.9268 (at least 1.3) | cavekit-input-bridging.md R5 |
| mochi | zoom-new-page | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-input-bridging.md R3 |
| mojo | addon-site | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-addons-site.md R7 |
| mojo | addons-lifecycle | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser-mojo/profile/extensions | cavekit-addons-extensions.md R2 |
| mojo | addons-list-pan | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser-mojo/profile/extensions | cavekit-chrome-look.md R2 |
| mojo | addons-page | **pass** | 7 ok, 0 failed | CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill] | cavekit-addons-extensions.md R2 |
| mojo | addons-search-hide-key | **pass** | 8 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mojo | appmenu-find | **pass** | 4 ok, 0 failed | WAIT ok 16-log: [jihad-bs] find 'Zebrafind' -> 1 | cavekit-ui-shell.md R7 |
| mojo | appmenu-print | **fail** | 3 ok, 3 failed | CHECK FAIL 14-wait-line: no line luna: findPrinters within the bound / CHECK FAIL 15-wait-differs: 0,200,756,960 still shows the sample page after the bound / CHECK FAIL 16-wait-log: no line print: mdns: .* -> [0-9.]*:[0-9]* in the daemon log within the bound; frames: kept/mojo-appmenu-print-14-wait-line_.jpg, kept/mojo-appmenu-print-15-wait-differs_.jpg, kept/mojo-appmenu-print-16-wait-log_.jpg, kept/mojo-appmenu-print-print-dialog_.jpg | cavekit-ui-shell.md R7 |
| mojo | appmenu-settings-keyboard-down | **fail** | 4 ok, 4 failed | CHECK FAIL 16-url: load done uri=[about:reader?url=http%3A%2F%2F192.168.1.245%3A8077%2Fjihad-long-static.html] lacks about:settings / CHECK FAIL 17-keyboard: the keyboard is not down within 8 s (its last reading: focused 0 wanted, 1 read) / CHECK FAIL 18-wait-rect: no element page-title with a size ; frames: kept/mojo-appmenu-settings-keyboard-down-16-url_.jpg, kept/mojo-appmenu-settings-keyboard-down-17-keyboard_.jpg, kept/mojo-appmenu-settings-keyboard-down-18-wait-rect_.jpg, kept/mojo-appmenu-settings-keyboard-down-19-content_.jpg | cavekit-chrome-look.md R4 |
| mojo | audio-heard | **a person's** | 0 ok, 0 failed | hearing the speaker; the route and the mixer are measured by script (cavekit-addons-extensions R7) | cavekit-device-build.md R4 |
| mojo | busy-script-stop | **pass** | 4 ok, 0 failed | CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36; frames: kept/mojo-busy-script-stop-busy-dialog_.jpg | cavekit-modern-web.md R2 |
| mojo | card-dialog | **pass** | 7 ok, 0 failed | WAIT ok 18-text r = [c: false] | cavekit-browser-services.md R3 |
| mojo | card-select | **pass** | 1 ok, 0 failed | WAIT ok 9-text picked = [picked: banana] | cavekit-device-build.md R4 |
| mojo | chrome-buttons | **pass** | 5 ok, 0 failed | WAIT ok 18-differs 0,100,756,700; frames: kept/mojo-chrome-buttons-new-card_.jpg | cavekit-device-build.md R4 |
| mojo | chrome-history | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/mojo-chrome-history-history-panel_.jpg | cavekit-device-build.md R4 |
| mojo | chrome-padlock | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-ui-shell.md R8 |
| mojo | chrome-share | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/mojo-chrome-share-share-menu_.jpg | cavekit-device-build.md R4 |
| mojo | ddg-results | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-modern-web.md R2 |
| mojo | dialog-slow-answer | **pass** | 4 ok, 0 failed | WAIT ok 14-text r = [c: true] | cavekit-finger-testing.md R4 |
| mojo | download-finishes | **pass** | 1 ok, 0 failed | WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-36 | cavekit-device-build.md R4 |
| mojo | download-list | **pass** | 9 ok, 0 failed | WAIT ok 26-content 10,90,400,300; frames: kept/mojo-download-list-panel_.jpg, kept/mojo-download-list-downloads_.jpg | cavekit-device-build.md R4 |
| mojo | drag-and-pinch-page | **pass** | 7 ok, 0 failed | CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15) | cavekit-input-bridging.md R3 |
| mojo | etsy-search | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-modern-web.md R2 |
| mojo | everyday-browsing | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-engine-update.md R8 |
| mojo | express-burst-wiki | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-gpu-rendering.md R3 |
| mojo | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| mojo | flash-fullscreen | **pass** | 8 ok, 0 failed | CHECK ok 20-flash-behind: the colour 00ff00 in 192 of 192 cells (wanted yes); frames: kept/mojo-flash-fullscreen-fullscreen_.jpg | cavekit-flash.md R4 |
| mojo | flash-input-cover | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| mojo | flash-input-hold | **pass** | 8 ok, 0 failed | CHECK ok 16-press-rate: 22.5 moves a second sent on (at least 20): ] a press of 1491 ms: 34 moves from the card in 1380 ms, 31 sent on, 547 ms in sending them | cavekit-flash.md R5 |
| mojo | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| mojo | flash-input-keyhold | **pass** | 7 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-flash.md R6 |
| mojo | flash-input-keys | **pass** | 13 ok, 0 failed | CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-flash.md R5 |
| mojo | flash-input-lock | **pass** | 15 ok, 0 failed | CHECK ok 31-card-scroll-min: the card's scroll y 100 (at least 30) | cavekit-flash.md R5 |
| mojo | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| mojo | flash-lane | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,275: 3.0 s, flips 40 (13.3/s), changes 0 (0.0/s), reads 41, distinct 1, spacing median 68 p90 147 max 201 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| mojo | flash-layout | **pass** | 17 ok, 0 failed | CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mojo | flash-sound | **a person's** | 0 ok, 0 failed | hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route) | cavekit-engine-update.md R8 |
| mojo | flash-tags | **fail** | 14 ok, 1 failed | CHECK FAIL 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 8 (2.7/s), changes 4 (1.3/s), reads 20, distinct 5, spacing median 107 p90 319 max 319 ms (wanted 5..100/s); frames: kept/mojo-flash-tags-25-flash-rate_.jpg | cavekit-flash.md R4 |
| mojo | flash-wmode | **pass** | 19 ok, 0 failed | CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mojo | fonts-pick | **pass** | 5 ok, 0 failed | WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.] | cavekit-preferences-ui.md R9 |
| mojo | forget-identities | **pass** | 3 ok, 0 failed | WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.] | cavekit-site-identity.md R1 |
| mojo | frame-field | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mojo | keyboard-address | **pass** | 2 ok, 0 failed | CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| mojo | keyboard-arbitration | **pass** | 8 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R7 |
| mojo | keyboard-fields | **pass** | 6 ok, 0 failed | CHECK ok 17-visible: field on the screen at 16,275-522,310 (the reach ends at 600, keyboard 1) | cavekit-input-bridging.md R2 |
| mojo | keyboard-keys | **pass** | 10 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R2 |
| mojo | long-press-image | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-ui-shell.md R7 |
| mojo | long-press-menu | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| mojo | long-press-text-field | **pass** | 6 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html; frames: kept/mojo-long-press-text-field-hold-text_.jpg, kept/mojo-long-press-text-field-hold-field_.jpg | cavekit-ui-shell.md R7 |
| mojo | media-controls | **pass** | 10 ok, 0 failed | CHECK ok 30-visible: anon:#v/controlBar on the screen at 16,572-586,594 (the reach ends at 1024, keyboard 0); frames: kept/mojo-media-controls-controls_.jpg, kept/mojo-media-controls-bar_.jpg | cavekit-gre-widgets.md R1 |
| mojo | media-scrub-drag | **known** | 7 ok, 1 failed | the open defect T-598: CHECK FAIL 25-wait-text: seek never read 60 within the bound; frames: kept/mojo-media-scrub-drag-25-wait-text_.jpg, kept/mojo-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |
| mojo | mobile-wiki-pinch | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-mojo-ui.md R6 |
| mojo | mojo-address-typing | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-mojo-ui.md R6 |
| mojo | mojo-launch-from-app | **a person's** | 0 ok, 0 failed | a launch from another app is not emulated (a launch with a target from the command line showed the page with no keyboard, 2026-10-24) | cavekit-chrome-look.md R4 |
| mojo | pan-long-drag | **pass** | 9 ok, 0 failed | CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| mojo | pan-long-flick | **known** | 1 ok, 2 failed | the open defect T-567, T-567: CHECK FAIL 16-stream-stale: stale 1 (want at most 0) / CHECK FAIL 17-stream-uniform: uniform 16 (want at most 0) | cavekit-gpu-rendering.md R3 |
| mojo | pan-speeds | **known** | 3 ok, 2 failed | the open defect T-567, T-567: CHECK FAIL 19-stream-stale: stale 8 (want at most 0) / CHECK FAIL 20-stream-uniform: uniform 25 (want at most 0) | cavekit-finger-testing.md R4 |
| mojo | persona-cycle | **pass** | 12 ok, 0 failed | persona file: put | cavekit-personas-ui.md R6 |
| mojo | persona-wear | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-personas-ui.md R6 |
| mojo | personas-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| mojo | personas-page-steady | **pass** | 3 ok, 0 failed | CHECK ok 15-stream-uniform: uniform 0 (at most 0) | cavekit-personas-ui.md R9 |
| mojo | proxy-login-mojo | **pass** | 6 ok, 0 failed | auth proxy: running; frames: kept/mojo-proxy-login-mojo-auth-sheet_.jpg | cavekit-proxies-certs.md R5 |
| mojo | proxy-own | **pass** | 8 ok, 0 failed | auth proxy: running | cavekit-proxies-certs.md R1 |
| mojo | proxy-section-redraw | **pass** | 5 ok, 0 failed | WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input | cavekit-proxies-certs.md R1 |
| mojo | relay-choice | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-proxies-certs.md R2 |
| mojo | rotate | **a person's** | 0 ok, 0 failed | the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side | cavekit-device-build.md R4 |
| mojo | screen-lock | **pass** | 5 ok, 0 failed | WAIT ok 34-text status = [paused t=38.2]; frames: kept/mojo-screen-lock-lock-button_.jpg, kept/mojo-screen-lock-locked_.jpg, kept/mojo-screen-lock-ask_.jpg, kept/mojo-screen-lock-unlocked_.jpg | cavekit-gre-widgets.md R1 |
| mojo | script-hold-zoom | **pass** | 6 ok, 0 failed | CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2) | cavekit-gpu-rendering.md R3 |
| mojo | settings-every-control | **by a tool** | 0 ok, 0 failed | tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt) | cavekit-preferences-ui.md R6 |
| mojo | settings-field-saves | **pass** | 8 ok, 0 failed | CHECK ok 20-region-equal: region 416,643-735,703: colours 38 hash c1c06359 rows 60 uniform 0 light 643-702 blue none = the sample's hash | cavekit-chrome-look.md R4 |
| mojo | settings-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| mojo | settings-int-emptied | **pass** | 5 ok, 0 failed | WAIT ok 16-pref browser.sessionhistory.max_entries = [20] | cavekit-preferences-ui.md R6 |
| mojo | settings-persist | **pass** | 5 ok, 0 failed | WAIT ok 16-pref general.smoothScroll = [true] | cavekit-preferences-ui.md R2 |
| mojo | settings-tabs | **pass** | 4 ok, 0 failed | CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content] | cavekit-preferences-ui.md R6 |
| mojo | settings-typing | **pass** | 5 ok, 0 failed | CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-preferences-ui.md R6 |
| mojo | ssl-confirm | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-device-build.md R4 |
| mojo | tabbed-browsing | **not run** | 0 ok, 0 failed | not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word) | cavekit-tabbed-browsing.md R1 |
| mojo | touchpad-go | **not run** | 0 ok, 0 failed | owner-accepted without hardware (device-build R6) | cavekit-device-build.md R6 |
| mojo | typing-caret | **pass** | 4 ok, 0 failed | CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| mojo | video-controls | **pass** | 6 ok, 0 failed | CHECK ok 18-region-equal: region 16,167-517,447: colours 2206 hash bfcabf27 rows 280 uniform 0 light 435-439 blue none = the sample's hash | cavekit-gre-widgets.md R1 |
| mojo | webgl-cube | **pass** | 7 ok, 0 failed | WAIT ok 16-changed c | cavekit-gpu-rendering.md R8 |
| mojo | webone | **not run** | 0 ok, 0 failed | needs webone, keyboard-map (run with --with webone,keyboard-map) | cavekit-proxies-certs.md R3 |
| mojo | wikipedia-keyboard-zoom | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-mochi-ui.md R6 |
| mojo | wikipedia-zoom | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-gpu-rendering.md R6 |
| mojo | xpi-confirm | **pass** | 8 ok, 0 failed | xpi: 649 bytes served as jihad-effect.xpi; frames: kept/mojo-xpi-confirm-xpi-prompt_.jpg, kept/mojo-xpi-confirm-extensions-pane_.jpg | cavekit-addons-extensions.md R3 |
| mojo | youtube-mobile | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-modern-web.md R2 |
| mojo | youtube-no-next | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-device-headroom.md R4 |
| mojo | zoom-dtap-column | **pass** | 6 ok, 0 failed | CHECK ok 14-content: region 0,100-756,700: colours 325 hash 735e4dc6 rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-211-750,1982 | cavekit-input-bridging.md R5 |
| mojo | zoom-field-kept | **pass** | 6 ok, 0 failed | CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8) | cavekit-input-bridging.md R5 |
| mojo | zoom-locked-pinch-off | **pass** | 3 ok, 0 failed | CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05) | cavekit-input-bridging.md R5 |
| mojo | zoom-locked-pinch | **pass** | 4 ok, 0 failed | CHECK ok 13-zoom-min: zoom 1.9123 (at least 1.3) | cavekit-input-bridging.md R5 |
| mojo | zoom-new-page | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-input-bridging.md R3 |

## The device at the end

```
DEVICE ok job-enyo: (71916/415139252) jihad (start) running, process 24263
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (71928/525087546) jihad-mochi (start) running, process 2138
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (71938/269598951) jihad-mojo (start) running, process 7222
DEVICE ok daemons-mojo: one daemon
DEVICE ok hand-daemon-mojo: none
DEVICE ok hooks-mojo: none
DEVICE ok adapter-gates-mojo: none
DEVICE ok proxy-mojo: network.proxy.type unset
DEVICE ok history-mojo: browser.sessionhistory.max_entries the default
DEVICE ok cards-mojo: none
DEVICE ok audio-lib-mojo: the device's own
DEVICE ok look-mojo: {"returnValue":true,"theme":null}
DEVICE ok gate-files: none in /tmp
DEVICE ok device-scripts: none left in /tmp
DEVICE ok test-pads-enyo: none
DEVICE ok dialog-pipes-enyo: none
DEVICE ok test-pads-mochi: none
DEVICE ok dialog-pipes-mochi: none
DEVICE ok test-pads-mojo: none
DEVICE ok dialog-pipes-mojo: none
DEVICE FAIL tmp-dumps: /tmp/sb-probe.png /tmp/sb-probe2.png 
DEVICE ok media-scratch: none
DEVICE FAIL overall: 3% of /tmp used
novacom: unexpected EOF from server
```

## Every test's lines

### enyo / addon-site — not run

    needs internet (run with --with internet)

### enyo / addons-lifecycle — pass

    enyo card on about:addons: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 258 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='pad01@jihad.test']
    WAIT ok 13-rect anon:richlistitem[value='pad01@jihad.test']|disable-btn
    WAIT ok 15-rect anon:richlistitem[value='pad01@jihad.test']|enable-btn
    WAIT ok 17-rect anon:richlistitem[value='pad01@jihad.test']|undo-btn
    WAIT ok 19-rect anon:richlistitem[value='pad01@jihad.test']|remove-btn

### enyo / addons-list-pan — pass

    enyo card on about:addons: connected at +2 s, load done at +8 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 258 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:richlistitem[value='pad12@jihad.test']
    WAIT ok 15-still 0,100,756,1000
    WAIT ok 17-rect sel:richlistitem[value='pad12@jihad.test'][selected='true']
    CHECK ok 18-hit: clickAt 843,1439 inside sel:richlistitem[value="pad12@jihad.test"] at 294,1412 656x99

### enyo / addons-page — pass

    enyo card on about:addons: connected at +2 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#category-extension
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 258 landed on document y 224)
    WAIT ok 9-rect sel:#category-extension[selected='true']
    WAIT ok 10-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:#category-plugin[selected='true']
    WAIT ok 14-rect sel:#category-extension[selected='true']
    WAIT ok 16-rect sel:#detail-view:not([hidden])
    CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill]

### enyo / addons-search-hide-key — pass

    enyo card on about:addons: connected at +3 s, load done at +8 s, 3 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#header-search
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 119 landed on document y 47)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-still sel:#header-search
    CHECK ok 14-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 15-still sel:#header-search
    WAIT ok 17-still sel:#header-search
    CHECK ok 18-region-equal: region 470,103-746,134: colours 100 hash 1f1e36f6 rows 31 uniform 0 light 103-133 blue none = the sample's hash
    CHECK ok 20-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / address-select-all — fail

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK FAIL 12-wait-changed: the region 200,88,470,135 did not change within the bound
    WAIT ok 14-still 200,88,470,135
    CHECK ok 16-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / appmenu-find — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +4 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 10-differs 0,30,300,180
    WAIT ok 11-still 0,30,300,180
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 16-log: [jihad-bs] find 'zebrafind' -> 1

### enyo / appmenu-print — fail

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 11-differs 0,30,300,180
    WAIT ok 12-still 0,30,300,180
    CHECK FAIL 14-wait-line: no line luna: findPrinters within the bound
    CHECK FAIL 15-wait-differs: 0,200,756,960 still shows the sample page after the bound
    CHECK FAIL 16-wait-log: no line print: mdns: .* -> [0-9.]*:[0-9]* in the daemon log within the bound
    WAIT ok 17-still 0,200,756,960

### enyo / appmenu-settings-keyboard-down — fail

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    WAIT ok 15-load
    CHECK FAIL 16-url: load done uri=[about:reader?url=http%3A%2F%2F192.168.1.245%3A8077%2Fjihad-long-static.html] lacks about:settings
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK FAIL 18-wait-rect: no element page-title with a size within the bound
    CHECK FAIL 19-content: region 0,100-768,700: colours 433 hash b3587c6b rows 600 uniform 0 light 100-699 blue none; page-title on the screen at 0,82-,, text [(no element)] LACKS [Settings]

### enyo / audio-heard — a person's

    hearing the speaker; the route and the mixer are measured by script (cavekit-addons-extensions R7)

### enyo / autoplay-sound — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +2 s, load done at +8 s, 10 content frames at +8 s; the GPU path, frames off the main thread
    WAIT ok 8-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 10-text status = [play refused NotAllowedError]
    WAIT ok 13-text status = [playing]

### enyo / autoplay-unmute — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&unmute=30000: connected at +2 s, load done at +8 s, 26 content frames at +8 s; the GPU path, frames off the main t
    WAIT ok 8-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 10-text status = [playing]
    WAIT ok 11-text status = [paused t=29.8]

### enyo / busy-script-stop — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 9-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 13-rect sel:h1
    CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36

### enyo / card-dialog — pass

    enyo card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +2 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect c
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 142 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 13-text r = [c: true]
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-2.fifo
    CHECK ok 17-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 18-text r = [c: false]

### enyo / card-select — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 9-text picked = [picked: banana]

### enyo / chrome-buttons — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 9-load
    WAIT ok 11-url: https://lite.duckduckgo.com/lite/
    WAIT ok 13-url: http://192.168.1.245:8077/jihad-long-static.html
    WAIT ok 15-url: https://lite.duckduckgo.com/lite/
    WAIT ok 18-differs 0,100,756,700

### enyo / chrome-history — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### enyo / chrome-padlock — not run

    needs internet (run with --with internet)

### enyo / chrome-share — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### enyo / ddg-results — not run

    needs internet (run with --with internet)

### enyo / dialog-slow-answer — pass

    enyo card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +3 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect c
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 142 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text r = [c: true]

### enyo / download-finishes — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-32

### enyo / download-list — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-33
    CHECK ok 10-file: /media/internal/downloads/jihad-download-33 (17 bytes) holds [Hello from Jihad]
    WAIT ok 12-differs 0,100,756,960
    WAIT ok 14-still 0,100,756,960
    WAIT ok 17-differs 0,100,756,960
    WAIT ok 18-still 0,100,756,960
    WAIT ok 22-differs 450,80,768,400
    WAIT ok 23-still 450,80,768,400
    WAIT ok 24-content 460,90,760,300

### enyo / drag-and-pinch-page — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 9-card-scroll-min: the card's scroll y 639 (at least 400)
    WAIT ok 11-zoom
    CHECK ok 12-zoom-min: zoom 2.6394 (at least 1.5)
    CHECK ok 13-content: region 0,150-756,600: colours 740 hash 33460b6e rows 450 uniform 201 light 150-599 blue none; sel:body on the screen at -630,-2287-1386,51788
    WAIT ok 15-zoom
    WAIT ok 17-zoom
    CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15)

### enyo / etsy-search — not run

    needs internet (run with --with internet)

### enyo / everyday-browsing — not run

    needs internet (run with --with internet)

### enyo / express-burst-wiki — not run

    needs internet (run with --with internet)

### enyo / flash-api — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +2 s, load done at +9 s, 13 content frames at +9 s; the GPU path, 
    WAIT ok 7-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 8-text vars = [vars a=hello]
    WAIT ok 9-text so = [so n=1]
    WAIT ok 10-text echoback = [echoback back hi]
    CHECK ok 11-text: echo = [echo hi]
    WAIT ok 12-text geturl = [geturl ok]
    WAIT ok 13-text fscommand = [fscommand jcmd jarg]
    WAIT ok 14-text data = [data true x=42]
    WAIT ok 15-text kid = [kid w=376]
    WAIT ok 17-text so = [so n=2]
    WAIT ok 18-launch: load done
    WAIT ok 19-rect m
    CHECK ok 20-text: count = [0]
    CHECK ok 22-text: count = [0]

### enyo / flash-fullscreen — fail

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&vars=fs=1: connected at +2 s, load done at +7 s, 12 content frames at +7 s; the GPU path, frames off the main
    WAIT ok 8-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 9-text kid = [kid w=376]
    WAIT ok 11-text fs = [fs fullScreen]
    WAIT ok 14-text fs = [fs normal]
    WAIT ok 17-text fs = [fs fullScreen]
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK FAIL 20-flash-behind: the colour 00ff00 in 73 of 192 cells (wanted yes)

### enyo / flash-input-cover — fail

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +2 s, load done at +10 s, 5 content frames at +10 s; the GPU path, frames off the main thread (the exp
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 229 px up for the keyboard, the page at 229
    WAIT ok 14-settled: the card's scroll 0 0
    CHECK FAIL 16-above-keyboard: m on the screen at 40,602-440,902, under the keyboard
    CHECK FAIL 18-flash-at: the movie reported down 201,80 for stage 200,150 (more than 3 off)

### enyo / flash-input-hold — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +8 s, 9 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-text up = [up 201 150]
    CHECK ok 10-number: heldms = 2308 (wanted 1500..3200)
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-number: n-up = 1 (wanted 1..1)
    WAIT ok 14-text up = [up 361 278]
    CHECK ok 15-flash-el: the movie's move at 355,274 for 360,277 (within 30)
    CHECK ok 16-press-rate: 25.2 moves a second sent on (at least 20): ] a press of 1451 ms: 32 moves from the card in 1272 ms, 32 sent on, 586 ms in sending them

### enyo / flash-input-kernel — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +8 s, 13 content frames at +8 s; the GPU path, frames off the main thread (the express 
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 10-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 13-text keydown = [keydown 38 0]
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 40 0]
    CHECK ok 18-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 23-settled: the card's scroll 0 51
    CHECK ok 24-card-scroll-min: the card's scroll y 51 (at least 20)

### enyo / flash-input-keyhold — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 14-text keyup = [keyup 65 97]
    CHECK ok 15-number: keyheldms = 244 (wanted 60..2000)
    WAIT ok 17-text keyup = [keyup 39 0]
    CHECK ok 18-number: keyheldms = 74 (wanted 60..2000)
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / flash-input-keys — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 49 49]
    WAIT ok 19-text keydown = [keydown 32 32]
    CHECK ok 20-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    WAIT ok 22-text keydown = [keydown 38 0]
    WAIT ok 24-text keydown = [keydown 39 0]
    WAIT ok 26-text keydown = [keydown 40 0]
    WAIT ok 28-text keydown = [keydown 37 0]
    CHECK ok 29-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / flash-input-lock — fail

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express l
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 98
    CHECK ok 10-card-scroll-min: the card's scroll y 98 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-number: n-down = 1 (wanted 1..1)
    CHECK ok 16-number: n-up = 1 (wanted 1..1)
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 20-settled: the card's scroll 0 0
    CHECK ok 21-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK FAIL 22-number: n-move = [2] (wanted 5..400)
    CHECK ok 24-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 27-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 30-settled: the card's scroll 0 98
    CHECK ok 31-card-scroll-min: the card's scroll y 98 (at least 30)

### enyo / flash-input-tap — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-number: n-down = 1 (wanted 1..1)
    CHECK ok 11-number: n-up = 1 (wanted 1..1)
    CHECK ok 14-flash-at: the movie reported down 101,60 for stage 100,60 (within 3)
    CHECK ok 15-number: n-down = 2 (wanted 2..2)
    CHECK ok 16-number: n-up = 2 (wanted 2..2)
    WAIT ok 17-launch: load done
    WAIT ok 18-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 21-number: n-down = 1 (wanted 1..1)
    CHECK ok 22-number: n-up = 1 (wanted 1..1)

### enyo / flash-lane — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off th
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,123-440,421: 3.0 s, flips 41 (13.7/s), changes 39 (13.0/s), reads 42, distinct 40, spacing median 69 p90 121 max 167 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,123-440,421: 3.0 s, flips 43 (14.3/s), changes 0 (0.0/s), reads 44, distinct 1, spacing median 66 p90 91 max 182 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,123-440,421: 3.0 s, flips 44 (14.7/s), changes 43 (14.3/s), reads 45, distinct 44, spacing median 66 p90 89 max 146 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 135
    CHECK ok 16-flash-rate: content 40,0-440,287: 3.0 s, flips 43 (14.3/s), changes 25 (8.3/s), reads 44, distinct 26, spacing median 68 p90 83 max 157 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,287: 3.0 s, flips 43 (14.3/s), changes 0 (0.0/s), reads 44, distinct 1, spacing median 62 p90 85 max 197 ms (wanted 0..4/s)

### enyo / flash-layout — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=percent: connected at +2 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the e
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 4)
    WAIT ok 10-launch: load done
    WAIT ok 11-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 13-flash-at: the movie reported down 101,100 for stage 100,100 (within 3)
    CHECK ok 15-flash-at: the movie reported down 301,200 for stage 300,200 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 19-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 20-launch: load done
    WAIT ok 21-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 23-settled: the card's scroll 0 1400
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-rect show
    WAIT ok 29-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### enyo / flash-sound — a person's

    hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route)

### enyo / flash-tags — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [10.3]
    CHECK ok 11-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [mime yes]
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 19-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 20-launch: load done
    WAIT ok 21-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 23-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 24-launch: load done
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 55 (18.3/s), changes 40 (13.3/s), reads 56, distinct 41, spacing median 52 p90 82 max 126 ms (wanted 5..100/s)

### enyo / flash-wmode — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm1&bg=00ff00&wmode=window: connected at +2 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main threa
    WAIT ok 7-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 8-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 10-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 11-launch: load done
    WAIT ok 12-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 13-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 18-flash-behind: the colour 00ff00 in 188 of 192 cells (wanted yes)
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 21-launch: load done
    WAIT ok 22-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 23-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 28-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### enyo / fonts-pick — pass

    enyo card on about:settings: connected at +3 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    WAIT ok 11-rect fonts-reset
    WAIT ok 13-pref font.name.serif.x-western = [Dotum]
    WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.]

### enyo / forget-identities — pass

    enyo card on about:settings: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.]

### enyo / frame-field — pass

    enyo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +2 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
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

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / keyboard-arbitration — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express l
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    WAIT ok 14-text keydown = [keydown 65 97]
    CHECK ok 15-region-equal: region 150,30-620,78: colours 165 hash e80d768f rows 48 uniform 0 light 55-58 blue none = the sample's hash
    CHECK ok 19-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 22-settled: the card's scroll 0 0
    CHECK ok 23-region-changed: region 150,30-620,78: colours 73 hash 9abd95e1 rows 48 uniform 0 light none blue none against the sample's hash e80d768f
    CHECK ok 25-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / keyboard-fields — pass

    enyo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 301 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 10-visible: area on the screen at 9,160-484,200 (the reach ends at 600, keyboard 1)
    CHECK ok 12-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 13-launch: load done
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 17-visible: field on the screen at 16,284-522,319 (the reach ends at 600, keyboard 1)

### enyo / keyboard-keys — pass

    enyo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 8-rect plain
    calibrated (enyo): the page's top on the screen 83 (the table said 82, the tap at y 143 landed on document y 77)
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 15-text echo = [plain n=2 c=0 last=[ab] value=[ab] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 17-text echo = [plain n=2 c=1 last=[ab] value=[ab] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 20-text echo = [plain n=3 c=1 last=[b] value=[b] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 25-text echo = [plain n=5 c=1 last=[a] value=[a] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 27-text echo = [plain n=5 c=2 last=[a] value=[a] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=0]
    WAIT ok 33-text echo = [plain n=5 c=2 last=[a] value=[a] area n=5 c=0 last=[a b c] value=[a b c] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=2]
    WAIT ok 37-text echo = [plain n=5 c=2 last=[a] value=[a] area n=6 c=0 last=[ab c] value=[ab c] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=2]
    WAIT ok 41-text echo = [plain n=5 c=2 last=[a] value=[a] area n=8 c=0 last=[ab] value=[ab] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=2]

### enyo / long-press-image — not run

    needs internet (run with --with internet)

### enyo / long-press-menu — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +6 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### enyo / long-press-text-field — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 10-differs 0,100,756,600
    WAIT ok 13-still 0,100,756,600
    WAIT ok 15-line: [jihad-bs] hold at 270,219 editable=1 wasFocused=0
    WAIT ok 18-still 0,100,400,240
    WAIT ok 20-load
    CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### enyo / media-controls — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the exp
    WAIT ok 9-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 11-text status = [playing]
    WAIT ok 13-text status = [paused t=11.1]
    WAIT ok 18-rect anon:#v|controlBar
    WAIT ok 19-rect anon:#v|muteButton
    WAIT ok 22-text vol = [volume=1.00 muted=false]
    WAIT ok 24-text vol = [volume=1.00 muted=true]
    WAIT ok 26-text vol = [volume=1.00 muted=false]
    CHECK ok 28-visible: anon:#v|controlBar on the screen at 16,581-586,603 (the reach ends at 1024, keyboard 0)
    CHECK ok 30-visible: anon:#v|controlBar on the screen at 16,581-586,603 (the reach ends at 1024, keyboard 0)

### enyo / media-scrub-drag — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +2 s, load done at +7 s, 10 content frames at +7 s; the GPU path, frames off the main thread (the ex
    WAIT ok 10-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 12-text status = [playing]
    WAIT ok 14-text status = [paused t=11.0]
    WAIT ok 15-rect anon:#v|controlBar
    WAIT ok 16-rect anon:#v|volumeControl
    WAIT ok 20-text vol = [volume=0.16 muted=false]
    WAIT ok 24-settled: the card's scroll 0 0
    WAIT ok 25-text seek = [60]

### enyo / mobile-wiki-pinch — not run

    needs internet (run with --with internet)

### enyo / pan-long-drag — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 8-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 9-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 10-content: region 0,150-768,400: colours 412 hash 072db6f4 rows 250 uniform 0 light 150-396 blue none; sel:#content p on the screen at 16,154-748,210, text holds [quick brown]
    CHECK ok 14-card-scroll: the card's scroll y 628 (want 650 within 100); the engine's 628
    WAIT ok 15-still header
    CHECK ok 16-region-equal: region 0,83-756,137: colours 208 hash 34266017 rows 54 uniform 33 light none blue none = the sample's hash
    CHECK ok 17-region-changed: region 0,300-756,900: colours 680 hash 967072ff rows 600 uniform 219 light 300-899 blue none against the sample's hash 38d1cd3c
    CHECK ok 18-top: the page's top measured 82 (the tap at y 234 landed on document y 780), the model says 82 (header hidden: 1)
    CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### enyo / pan-long-flick — known

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 18-card-scroll-min: the card's scroll y 3310 (at least 300)
    STREAM fs-enyo-pan-long-flick.fbs: 118 samples (14.7/s asked 15, read median 2.3 ms max 82.12 ms, mmap stride 8, re-read 12, torn 1), 150 flips, cpu 7.8% of a core, polls 608/s | shown frames while moving: 147 gaps, median 27 ms, p90 43 ms, max 238 ms | shown scroll: 2494 rows over the stream (0 samples with the shift unknown); stale bands 1, uniform bands 38, still rows moved 0; the columns outside 0-756 changed in 50 samples | tracking (finger down, 21 samples): error median 30 px, max 263 px; lag median 201 ms, max 866 ms |   gesture at 1794535227413: the finger travelled 600 rows, the page showed
    CHECK FAIL 16-stream-stale: stale 1 (want at most 0)
    CHECK FAIL 17-stream-uniform: uniform 38 (want at most 0)

### enyo / pan-speeds — known

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 13-card-scroll-min: the card's scroll y 650 (at least 500)
    CHECK ok 22-card-scroll-min: the card's scroll y 4524 (at least 2500)
    STREAM fs-enyo-pan-speeds.fbs: 277 samples (14.8/s asked 15, read median 2.34 ms max 57.58 ms, mmap stride 8, re-read 22, torn 0), 378 flips, cpu 7.3% of a core, polls 594/s | shown frames while moving: 372 gaps, median 26.0 ms, p90 40 ms, max 284 ms | shown scroll: 3622 rows over the stream (1 samples with the shift unknown); stale bands 3, uniform bands 31, still rows moved 0; the columns outside 0-756 changed in 131 samples | tracking (finger down, 128 samples): error median -11.0 px, max 150 px; lag median 80.5 ms, max 811 ms |   gesture at 1794535284813: the finger travelled 650 rows, the page s
    CHECK FAIL 19-stream-stale: stale 3 (want at most 0)
    CHECK FAIL 20-stream-uniform: uniform 31 (want at most 0)
    CHECK ok 21-stream-error-max: error-max 150 (at most 500)

### enyo / persona-cycle — pass

    enyo card on about:personas: connected at +2 s, load done at +8 s, 9 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

### enyo / persona-wear — not run

    needs internet (run with --with internet)

### enyo / personas-flick — pass

    enyo card on about:personas: connected at +3 s, load done at +9 s, 10 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#mine .card
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 280 landed on document y 198)
    CHECK ok 10-card-scroll-min: the card's scroll y 1644 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### enyo / personas-page-steady — pass

    enyo card on about:personas: connected at +2 s, load done at +8 s, 13 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-rect sel:#mine .card
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 280 landed on document y 198)
    CHECK ok 16-content: region 0,100-768,900: colours 4096 hash 1366b6e2 rows 800 uniform 0 light 100-899 blue 279-753; sel:body on the screen at 0,-414-764,2155
    STREAM fs-enyo-personas-page-steady.fbs: 81 samples (15.0/s asked 15, read median 2.36 ms max 26.29 ms, mmap stride 8, re-read 6, torn 0), 149 flips, cpu 6.3% of a core, polls 585/s | shown frames while moving: 148 gaps, median 24.0 ms, p90 33 ms, max 50 ms | shown scroll: 498 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 0-756 changed in 50 samples | tracking (finger down, 51 samples): error median -10 px, max 26 px; lag median 78 ms, max 163 ms |   gesture at 1794535510474: the finger travelled 500 rows, the page sho
    CHECK ok 15-stream-uniform: uniform 0 (at most 0)

### enyo / proxy-login — pass

    enyo card on about:settings: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    enyo card on about:settings: connected at +2 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    enyo card on about:settings: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 11-rect sel:.row[data-pref='network.proxy.type'] select
    WAIT ok 13-rect sel:.row[data-pref='network.proxy.http'] input
    WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input

### enyo / relay-choice — not run

    needs internet (run with --with internet)

### enyo / rotate — a person's

    the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side

### enyo / screen-lock — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +2 s, load done at +7 s, 14 content frames at +7 s; the GPU path, frames off the main thread (the ex
    WAIT ok 9-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 11-text status = [playing]
    WAIT ok 20-text status = [playing]
    CHECK ok 22-text: status = [playing]
    WAIT ok 34-text status = [paused t=29.2]

### enyo / script-hold-zoom — pass

    enyo card on http://192.168.1.245:8077/jihad-script-hold.html: connected at +2 s, load done at +6 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 110 landed on document y 28)
    WAIT ok 9-zoom
    CHECK ok 10-zoom-min: zoom 1.9326 (at least 1.3)
    CHECK ok 12-card-scroll-min: the card's scroll y 461 (at least 200)
    WAIT ok 14-zoom
    WAIT ok 16-zoom
    CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2)

### enyo / settings-every-control — by a tool

    tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt)

### enyo / settings-field-saves — pass

    enyo card on about:settings: connected at +2 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    enyo card on about:settings: connected at +2 s, load done at +6 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    CHECK ok 10-card-scroll-min: the card's scroll y 720 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### enyo / settings-int-emptied — pass

    enyo card on about:settings: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 16-pref browser.sessionhistory.max_entries = [20]

### enyo / settings-persist — pass

    enyo card on about:settings: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-pref general.smoothScroll = [true]
    WAIT ok 14-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 16-pref general.smoothScroll = [true]

### enyo / settings-tabs — pass

    enyo card on about:settings: connected at +2 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    CHECK ok 9-text: page-title = [Settings]
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content]

### enyo / settings-typing — pass

    enyo card on about:settings: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 15-pref browser.display.background_color = [#FFFFFFx]
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / ssl-confirm — not run

    needs internet (run with --with internet)

### enyo / tabbed-browsing — not run

    not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word)

### enyo / touchpad-go — not run

    owner-accepted without hardware (device-build R6)

### enyo / typing-caret — pass

    enyo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 301 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 11-text echo = [plain n=6 c=0 last=[abcdef] value=[abcdef] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 13-text echo = [plain n=12 c=0 last=[] value=[] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / video-controls — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html: connected at +2 s, load done at +7 s, 21 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect status
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 12-changed sel:video
    WAIT ok 13-changed sel:video
    WAIT ok 15-still sel:video
    WAIT ok 17-still sel:video
    CHECK ok 18-region-equal: region 16,176-517,456: colours 2439 hash e970f523 rows 280 uniform 0 light 444-448 blue none = the sample's hash

### enyo / webgl-cube — pass

    enyo card on http://192.168.1.245:8077/jihad-webgl-cube.html: connected at +1 s, load done at +7 s, 17 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 7-rect c
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 124 landed on document y 57)
    CHECK ok 9-text-has: out = [webgl fps=24.6 shaders=ok texture=ok framebuffer=ok readPixels=ok maxtex=4096 ext=14 missing=ANGLE_instanced_arrays,WEBGL_compressed_texture_s3tc,EXT_blend_minmax,EXT_shader_texture_lod depthrb=ok renderer=Mozilla vendor=Mozilla] holds [webgl]
    CHECK ok 10-content: region 105,175-457,524: colours 923 hash 4998b3d2 rows 349 uniform 130 light none blue 260-445; c on the screen at 105,174-457,525
    WAIT ok 11-changed c
    WAIT ok 13-zoom
    CHECK ok 15-content: region 0,100-768,900: colours 1364 hash 1e826913 rows 800 uniform 0 light none blue none; c on the screen at -51,87-499,637
    WAIT ok 16-changed c

### enyo / webone — not run

    needs webone, keyboard-map (run with --with webone,keyboard-map)

### enyo / wikipedia-keyboard-zoom — not run

    needs internet (run with --with internet)

### enyo / wikipedia-zoom — not run

    needs internet (run with --with internet)

### enyo / xpi-confirm — pass

    enyo card on http://192.168.1.245:8077/jihad-xpi-trigger.html: connected at +2 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 8-rect effect
    calibrated (enyo): the page's top on the screen 83 (the table said 82, the tap at y 129 landed on document y 59)
    CHECK ok 11-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 14-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 15-text r = [r: callback status=0 Jihad Effect Test]
    WAIT ok 16-launch: load done
    WAIT ok 17-rect sel:#category-extension
    WAIT ok 19-rect sel:#category-extension[selected='true']
    WAIT ok 21-rect sel:richlistitem[value='jihad-effect@riverstonerelay.net']

### enyo / youtube-feed-no-preview — not run

    needs internet (run with --with internet)

### enyo / youtube-mobile — not run

    needs internet (run with --with internet)

### enyo / youtube-no-next — not run

    needs internet (run with --with internet)

### enyo / zoom-dtap-column — pass

    enyo card on http://192.168.1.245:8077/jihad-dtap.html: connected at +3 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 7-rect sel:#col
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 114 landed on document y 50)
    WAIT ok 10-zoom
    WAIT ok 11-settled: the card's scroll 590 272
    CHECK ok 12-zoom-min: zoom 1.8286 (at least 1.5)
    CHECK ok 13-element-at: sel:#col on the screen at 18,-193-750,2000 holds 384,420
    CHECK ok 14-content: region 0,100-756,700: colours 438 hash 2976668a rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-193-750,2000

### enyo / zoom-field-kept — pass

    enyo card on http://192.168.1.245:8077/jihad-dtap.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 7-rect sel:#f
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 114 landed on document y 50)
    CHECK ok 9-zoom-max: zoom 0.6400 (at most 0.8)
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-settled: the card's scroll 0 430
    CHECK ok 14-above-keyboard: sel:#f on the screen at 64,639-324,668, above the keyboard
    CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8)

### enyo / zoom-locked-pinch-off — pass

    enyo card on http://192.168.1.245:8077/jihad-locked.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 8-rect sel:h1
    WAIT ok 12-settled: the card's scroll 0 0
    CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05)

### enyo / zoom-locked-pinch — pass

    enyo card on http://192.168.1.245:8077/jihad-locked.html: connected at +2 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 7-rect sel:h1
    WAIT ok 11-zoom
    WAIT ok 12-settled: the card's scroll 188 204
    CHECK ok 13-zoom-min: zoom 1.9539 (at least 1.3)

### enyo / zoom-new-page — not run

    needs internet (run with --with internet)

### mochi / addon-site — not run

    needs internet (run with --with internet)

### mochi / addons-lifecycle — pass

    mochi card on about:addons: connected at +4 s, load done at +13 s, 6 content frames at +15 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 254 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='pad01@jihad.test']
    WAIT ok 13-rect anon:richlistitem[value='pad01@jihad.test']|disable-btn
    WAIT ok 15-rect anon:richlistitem[value='pad01@jihad.test']|enable-btn
    WAIT ok 17-rect anon:richlistitem[value='pad01@jihad.test']|undo-btn
    WAIT ok 19-rect anon:richlistitem[value='pad01@jihad.test']|remove-btn

### mochi / addons-list-pan — pass

    mochi card on about:addons: connected at +1 s, load done at +8 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 254 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:richlistitem[value='pad12@jihad.test']
    WAIT ok 15-still 0,100,756,1000
    WAIT ok 17-rect sel:richlistitem[value='pad12@jihad.test'][selected='true']
    CHECK ok 18-hit: clickAt 843,1439 inside sel:richlistitem[value="pad12@jihad.test"] at 294,1412 656x99

### mochi / addons-page — pass

    mochi card on about:addons: connected at +1 s, load done at +7 s, 9 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#category-extension
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 254 landed on document y 224)
    WAIT ok 9-rect sel:#category-extension[selected='true']
    WAIT ok 10-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:#category-plugin[selected='true']
    WAIT ok 14-rect sel:#category-extension[selected='true']
    WAIT ok 16-rect sel:#detail-view:not([hidden])
    CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill]

### mochi / addons-search-hide-key — pass

    mochi card on about:addons: connected at +1 s, load done at +7 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-changed 200,88,470,135
    WAIT ok 14-still 200,88,470,135
    CHECK ok 16-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / appmenu-find — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 10-differs 0,30,300,180
    WAIT ok 11-still 0,30,300,180
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 16-log: [jihad-bs] find 'zebrafind' -> 1

### mochi / appmenu-print — fail

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 11-differs 0,30,300,180
    WAIT ok 12-still 0,30,300,180
    CHECK FAIL 14-wait-line: no line luna: findPrinters within the bound
    CHECK FAIL 15-wait-differs: 0,200,756,960 still shows the sample page after the bound
    CHECK FAIL 16-wait-log: no line print: mdns: .* -> [0-9.]*:[0-9]* in the daemon log within the bound
    WAIT ok 17-still 0,200,756,960

### mochi / appmenu-settings-keyboard-down — fail

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    WAIT ok 15-load
    CHECK FAIL 16-url: load done uri=[about:reader?url=http%3A%2F%2F192.168.1.245%3A8077%2Fjihad-long-static.html] lacks about:settings
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK FAIL 18-wait-rect: no element page-title with a size within the bound
    CHECK FAIL 19-content: region 0,100-768,700: colours 405 hash 83c241b8 rows 600 uniform 0 light 100-699 blue none; page-title on the screen at 0,78-,, text [(no element)] LACKS [Settings]

### mochi / audio-heard — a person's

    hearing the speaker; the route and the mixer are measured by script (cavekit-addons-extensions R7)

### mochi / busy-script-stop — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 9-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 13-rect sel:h1
    CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36

### mochi / card-dialog — pass

    mochi card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 7-rect c
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 138 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 13-text r = [c: true]
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-2.fifo
    CHECK ok 17-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 18-text r = [c: false]

### mochi / card-select — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +3 s, load done at +7 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 9-text picked = [picked: banana]

### mochi / chrome-buttons — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 9-load
    WAIT ok 11-url: https://lite.duckduckgo.com/lite/
    WAIT ok 13-url: http://192.168.1.245:8077/jihad-long-static.html
    WAIT ok 15-url: https://lite.duckduckgo.com/lite/
    WAIT ok 18-differs 0,100,756,700

### mochi / chrome-history — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### mochi / chrome-padlock — not run

    needs internet (run with --with internet)

### mochi / chrome-share — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### mochi / ddg-results — not run

    needs internet (run with --with internet)

### mochi / dialog-slow-answer — pass

    mochi card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 7-rect c
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 138 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text r = [c: true]

### mochi / download-finishes — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-34

### mochi / download-list — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-35
    CHECK ok 10-file: /media/internal/downloads/jihad-download-35 (17 bytes) holds [Hello from Jihad]
    WAIT ok 12-differs 0,100,756,960
    WAIT ok 14-still 0,100,756,960
    WAIT ok 17-differs 0,100,756,960
    WAIT ok 18-still 0,100,756,960
    WAIT ok 22-differs 450,80,768,400
    WAIT ok 23-still 450,80,768,400
    WAIT ok 25-content 460,90,760,300

### mochi / drag-and-pinch-page — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 9-card-scroll-min: the card's scroll y 639 (at least 400)
    WAIT ok 11-zoom
    CHECK ok 12-zoom-min: zoom 2.6506 (at least 1.5)
    CHECK ok 13-content: region 0,150-756,600: colours 689 hash 8e3adb68 rows 450 uniform 205 light 170-599 blue none; sel:body on the screen at -635,-2309-1389,51995
    WAIT ok 15-zoom
    WAIT ok 17-zoom
    CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15)

### mochi / etsy-search — not run

    needs internet (run with --with internet)

### mochi / everyday-browsing — not run

    needs internet (run with --with internet)

### mochi / express-burst-wiki — not run

    needs internet (run with --with internet)

### mochi / flash-api — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +1 s, load done at +7 s, 3 content frames at +7 s; the GPU path, 
    WAIT ok 7-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 8-text vars = [vars a=hello]
    WAIT ok 9-text so = [so n=1]
    WAIT ok 10-text echoback = [echoback back hi]
    CHECK ok 11-text: echo = [echo hi]
    WAIT ok 12-text geturl = [geturl ok]
    WAIT ok 13-text fscommand = [fscommand jcmd jarg]
    WAIT ok 14-text data = [data true x=42]
    WAIT ok 15-text kid = [kid w=376]
    WAIT ok 17-text so = [so n=2]
    WAIT ok 18-launch: load done
    WAIT ok 19-rect m
    CHECK ok 20-text: count = [0]
    CHECK ok 22-text: count = [0]

### mochi / flash-fullscreen — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&vars=fs=1: connected at +3 s, load done at +8 s, 11 content frames at +8 s; the GPU path, frames off the mai
    WAIT ok 8-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 9-text kid = [kid w=376]
    WAIT ok 11-text fs = [fs fullScreen]
    WAIT ok 14-text fs = [fs normal]
    WAIT ok 17-text fs = [fs fullScreen]
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 20-flash-behind: the colour 00ff00 in 180 of 192 cells (wanted yes)

### mochi / flash-input-cover — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +3 s, load done at +10 s, 12 content frames at +10 s; the GPU path, frames off the main thread (the e
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 225 px up for the keyboard, the page at 225
    WAIT ok 14-settled: the card's scroll 0 225
    CHECK ok 16-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mochi / flash-input-hold — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the express lane) 
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-text up = [up 201 154]
    CHECK ok 10-number: heldms = 2240 (wanted 1500..3200)
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-number: n-up = 1 (wanted 1..1)
    WAIT ok 14-text up = [up 361 282]
    CHECK ok 15-flash-el: the movie's move at 358,280 for 360,277 (within 30)
    CHECK ok 16-press-rate: 24.8 moves a second sent on (at least 20): ] a press of 1439 ms: 33 moves from the card in 1328 ms, 33 sent on, 460 ms in sending them

### mochi / flash-input-kernel — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +3 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express 
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 10-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 13-text keydown = [keydown 38 0]
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 40 0]
    CHECK ok 18-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 23-settled: the card's scroll 0 51
    CHECK ok 24-card-scroll-min: the card's scroll y 51 (at least 20)

### mochi / flash-input-keyhold — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +3 s, load done at +8 s, 9 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 14-text keyup = [keyup 65 97]
    CHECK ok 15-number: keyheldms = 242 (wanted 60..2000)
    WAIT ok 17-text keyup = [keyup 39 0]
    CHECK ok 18-number: keyheldms = 80 (wanted 60..2000)
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / flash-input-keys — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 49 49]
    WAIT ok 19-text keydown = [keydown 32 32]
    CHECK ok 20-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    WAIT ok 22-text keydown = [keydown 38 0]
    WAIT ok 24-text keydown = [keydown 39 0]
    WAIT ok 26-text keydown = [keydown 40 0]
    WAIT ok 28-text keydown = [keydown 37 0]
    CHECK ok 29-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / flash-input-lock — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +1 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express 
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 100
    CHECK ok 10-card-scroll-min: the card's scroll y 100 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-number: n-down = 1 (wanted 1..1)
    CHECK ok 16-number: n-up = 1 (wanted 1..1)
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 20-settled: the card's scroll 0 0
    CHECK ok 21-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 22-number: n-move = 16 (wanted 5..400)
    CHECK ok 24-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 27-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 30-settled: the card's scroll 0 100
    CHECK ok 31-card-scroll-min: the card's scroll y 100 (at least 30)

### mochi / flash-input-tap — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 10 content frames at +7 s; the GPU path, frames off the main thread (the express lane) 
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-number: n-down = 1 (wanted 1..1)
    CHECK ok 11-number: n-up = 1 (wanted 1..1)
    CHECK ok 14-flash-at: the movie reported down 101,60 for stage 100,60 (within 3)
    CHECK ok 15-number: n-down = 2 (wanted 2..2)
    CHECK ok 16-number: n-up = 2 (wanted 2..2)
    WAIT ok 17-launch: load done
    WAIT ok 18-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 21-number: n-down = 1 (wanted 1..1)
    CHECK ok 22-number: n-up = 1 (wanted 1..1)

### mochi / flash-lane — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +1 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off t
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,119-440,417: 3.0 s, flips 44 (14.7/s), changes 39 (13.0/s), reads 45, distinct 40, spacing median 67 p90 105 max 202 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,119-440,417: 3.0 s, flips 41 (13.7/s), changes 0 (0.0/s), reads 42, distinct 1, spacing median 69 p90 144 max 163 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,119-440,417: 3.0 s, flips 45 (15.0/s), changes 44 (14.7/s), reads 46, distinct 45, spacing median 67 p90 76 max 161 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 51
    CHECK ok 16-flash-rate: content 40,69-440,367: 3.0 s, flips 43 (14.3/s), changes 42 (14.0/s), reads 44, distinct 43, spacing median 66 p90 84 max 141 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,69-440,367: 3.0 s, flips 45 (15.0/s), changes 0 (0.0/s), reads 46, distinct 1, spacing median 66 p90 95 max 162 ms (wanted 0..4/s)

### mochi / flash-layout — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=percent: connected at +3 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the 
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 4)
    WAIT ok 10-launch: load done
    WAIT ok 11-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 13-flash-at: the movie reported down 101,100 for stage 100,100 (within 3)
    CHECK ok 15-flash-at: the movie reported down 301,200 for stage 300,200 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 19-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 20-launch: load done
    WAIT ok 21-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 23-settled: the card's scroll 0 1400
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-rect show
    WAIT ok 29-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mochi / flash-sound — a person's

    hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route)

### mochi / flash-tags — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [10.3]
    CHECK ok 11-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [mime yes]
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 19-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 20-launch: load done
    WAIT ok 21-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 23-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 24-launch: load done
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 42 (14.0/s), changes 42 (14.0/s), reads 43, distinct 43, spacing median 70 p90 80 max 96 ms (wanted 5..100/s)

### mochi / flash-wmode — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm1&bg=00ff00&wmode=window: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thre
    WAIT ok 7-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 8-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 10-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 11-launch: load done
    WAIT ok 12-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 13-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 18-flash-behind: the colour 00ff00 in 188 of 192 cells (wanted yes)
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 21-launch: load done
    WAIT ok 22-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 23-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 28-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mochi / fonts-pick — pass

    mochi card on about:settings: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    WAIT ok 11-rect fonts-reset
    WAIT ok 13-pref font.name.serif.x-western = [Dotum]
    WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.]

### mochi / forget-identities — pass

    mochi card on about:settings: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.]

### mochi / frame-field — pass

    mochi card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +3 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
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

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / keyboard-arbitration — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +3 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express 
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    WAIT ok 14-text keydown = [keydown 65 97]
    CHECK ok 15-region-equal: region 150,30-620,78: colours 173 hash 60c76c31 rows 48 uniform 19 light 40-66 blue none = the sample's hash
    CHECK ok 19-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 22-settled: the card's scroll 0 0
    CHECK ok 23-region-changed: region 150,30-620,78: colours 66 hash 2f75eb14 rows 48 uniform 19 light 40-66 blue 39-67 against the sample's hash 60c76c31
    CHECK ok 25-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / keyboard-fields — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 297 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 10-visible: area on the screen at 9,156-484,196 (the reach ends at 600, keyboard 1)
    CHECK ok 12-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 13-launch: load done
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 17-visible: field on the screen at 16,280-522,315 (the reach ends at 600, keyboard 1)

### mochi / keyboard-keys — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 8-rect plain
    calibrated (mochi): the page's top on the screen 79 (the table said 78, the tap at y 139 landed on document y 77)
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 15-text echo = [plain n=2 c=0 last=[ab] value=[ab] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 17-text echo = [plain n=2 c=1 last=[ab] value=[ab] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 20-text echo = [plain n=3 c=1 last=[b] value=[b] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 25-text echo = [plain n=5 c=1 last=[a] value=[a] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 27-text echo = [plain n=5 c=2 last=[a] value=[a] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=0]
    WAIT ok 33-text echo = [plain n=5 c=2 last=[a] value=[a] area n=5 c=0 last=[a b c] value=[a b c] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=2]
    WAIT ok 37-text echo = [plain n=5 c=2 last=[a] value=[a] area n=6 c=0 last=[ab c] value=[ab c] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=2]
    WAIT ok 41-text echo = [plain n=5 c=2 last=[a] value=[a] area n=8 c=0 last=[ab] value=[ab] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=2]

### mochi / long-press-image — not run

    needs internet (run with --with internet)

### mochi / long-press-menu — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +3 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mochi / long-press-text-field — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 10-differs 0,100,756,600
    WAIT ok 13-still 0,100,756,600
    WAIT ok 15-line: [jihad-bs] hold at 270,219 editable=1 wasFocused=0
    WAIT ok 18-still 0,100,400,240
    WAIT ok 20-load
    CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mochi / media-controls — pass

    mochi card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the e
    WAIT ok 9-rect v
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 98 landed on document y 25)
    WAIT ok 11-text status = [playing]
    WAIT ok 13-text status = [paused t=21.7]
    WAIT ok 18-rect anon:#v|controlBar
    WAIT ok 19-rect anon:#v|muteButton
    WAIT ok 22-text vol = [volume=1.00 muted=false]
    WAIT ok 24-text vol = [volume=1.00 muted=true]
    WAIT ok 26-text vol = [volume=1.00 muted=false]
    CHECK ok 28-visible: anon:#v|controlBar on the screen at 16,577-586,599 (the reach ends at 1024, keyboard 0)
    CHECK ok 30-visible: anon:#v|controlBar on the screen at 16,577-586,599 (the reach ends at 1024, keyboard 0)

### mochi / media-scrub-drag — pass

    mochi card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +7 s, 18 content frames at +7 s; the GPU path, frames off the main thread (the e
    WAIT ok 10-rect v
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 98 landed on document y 25)
    WAIT ok 12-text status = [playing]
    WAIT ok 14-text status = [paused t=22.3]
    WAIT ok 15-rect anon:#v|controlBar
    WAIT ok 16-rect anon:#v|volumeControl
    WAIT ok 20-text vol = [volume=0.16 muted=false]
    WAIT ok 24-settled: the card's scroll 0 0
    WAIT ok 25-text seek = [60]

### mochi / mobile-wiki-pinch — not run

    needs internet (run with --with internet)

### mochi / mochi-paste — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +1 s, load done at +6 s, 9 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 297 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-differs 100,100,668,420
    WAIT ok 15-still 100,100,668,420
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / pan-long-drag — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 8-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 9-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 10-content: region 0,150-768,400: colours 354 hash 6220be68 rows 250 uniform 0 light 150-399 blue none; sel:#content p on the screen at 16,150-748,206, text holds [quick brown]
    CHECK ok 14-card-scroll: the card's scroll y 639 (want 650 within 100); the engine's 639
    WAIT ok 15-still header
    CHECK ok 16-region-equal: region 0,79-756,133: colours 204 hash 5486acf3 rows 54 uniform 33 light none blue none = the sample's hash
    CHECK ok 17-region-changed: region 0,300-756,900: colours 840 hash 3f74d198 rows 600 uniform 214 light 307-899 blue none against the sample's hash b34d3334
    CHECK ok 18-top: the page's top measured 78 (the tap at y 219 landed on document y 780), the model says 78 (header hidden: 1)
    CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### mochi / pan-long-flick — known

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 18-card-scroll-min: the card's scroll y 3450 (at least 300)
    STREAM fs-mochi-pan-long-flick.fbs: 166 samples (14.1/s asked 15, read median 2.32 ms max 114.72 ms, mmap stride 8, re-read 11, torn 1), 129 flips, cpu 7.2% of a core, polls 657/s | shown frames while moving: 125 gaps, median 30 ms, p90 45 ms, max 187 ms | shown scroll: 1373 rows over the stream (1 samples with the shift unknown); stale bands 6, uniform bands 17, still rows moved 32; the columns outside 0-756 changed in 43 samples | tracking (finger down, 24 samples): error median 25.0 px, max 150 px; lag median 221.5 ms, max 947 ms |   gesture at 1794540803676: the finger travelled 600 rows, the pag
    CHECK FAIL 16-stream-stale: stale 6 (want at most 0)
    CHECK FAIL 17-stream-uniform: uniform 17 (want at most 0)

### mochi / pan-speeds — known

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 13-card-scroll-min: the card's scroll y 646 (at least 500)
    CHECK ok 22-card-scroll-min: the card's scroll y 4564 (at least 2500)
    STREAM fs-mochi-pan-speeds.fbs: 341 samples (14.6/s asked 15, read median 2.35 ms max 75.45 ms, mmap stride 8, re-read 20, torn 1), 365 flips, cpu 6.8% of a core, polls 647/s | shown frames while moving: 357 gaps, median 25 ms, p90 40 ms, max 219 ms | shown scroll: 4496 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 37, still rows moved 95; the columns outside 0-756 changed in 125 samples | tracking (finger down, 123 samples): error median -12 px, max 188 px; lag median 76 ms, max 845 ms |   gesture at 1794540871732: the finger travelled 650 rows, the page showe
    CHECK ok 19-stream-stale: stale 0 (at most 0)
    CHECK FAIL 20-stream-uniform: uniform 37 (want at most 0)
    CHECK ok 21-stream-error-max: error-max 188 (at most 500)

### mochi / persona-cycle — pass

    mochi card on about:personas: connected at +2 s, load done at +9 s, 11 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

### mochi / persona-wear — not run

    needs internet (run with --with internet)

### mochi / personas-flick — pass

    mochi card on about:personas: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#mine .card
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 276 landed on document y 198)
    CHECK ok 10-card-scroll-min: the card's scroll y 1930 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### mochi / personas-page-steady — pass

    mochi card on about:personas: connected at +1 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-rect sel:#mine .card
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 276 landed on document y 198)
    CHECK ok 16-content: region 0,100-768,900: colours 4096 hash 5c9da272 rows 800 uniform 0 light 102-647 blue 107-655; sel:body on the screen at 0,-418-764,2441
    STREAM fs-mochi-personas-page-steady.fbs: 102 samples (14.8/s asked 15, read median 2.43 ms max 55.56 ms, mmap stride 8, re-read 9, torn 0), 144 flips, cpu 6.9% of a core, polls 580/s | shown frames while moving: 143 gaps, median 26 ms, p90 38 ms, max 137 ms | shown scroll: 498 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 0-756 changed in 51 samples | tracking (finger down, 54 samples): error median -9.0 px, max 26 px; lag median 71.5 ms, max 166 ms |   gesture at 1794541123453: the finger travelled 500 rows, the page
    CHECK ok 15-stream-uniform: uniform 0 (at most 0)

### mochi / proxy-login — pass

    mochi card on about:settings: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    mochi card on about:settings: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    mochi card on about:settings: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 11-rect sel:.row[data-pref='network.proxy.type'] select
    WAIT ok 13-rect sel:.row[data-pref='network.proxy.http'] input
    WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input

### mochi / relay-choice — not run

    needs internet (run with --with internet)

### mochi / rotate — a person's

    the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side

### mochi / screen-lock — fail

    mochi card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +7 s, 17 content frames at +7 s; the GPU path, frames off the main thread (the e
    WAIT ok 9-rect v
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 98 landed on document y 25)
    WAIT ok 11-text status = [playing]
    CHECK FAIL 20-wait-text: status never read playing within the bound
    CHECK FAIL 22-text: status = [paused t=24.9], want [playing]
    CHECK FAIL 34-wait-text: status never read paused within the bound

### mochi / script-hold-zoom — pass

    mochi card on http://192.168.1.245:8077/jihad-script-hold.html: connected at +1 s, load done at +6 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 106 landed on document y 28)
    WAIT ok 9-zoom
    CHECK ok 10-zoom-min: zoom 1.9126 (at least 1.3)
    CHECK ok 12-card-scroll-min: the card's scroll y 463 (at least 200)
    WAIT ok 14-zoom
    WAIT ok 16-zoom
    CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2)

### mochi / settings-every-control — by a tool

    tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt)

### mochi / settings-field-saves — pass

    mochi card on about:settings: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    mochi card on about:settings: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    CHECK ok 10-card-scroll-min: the card's scroll y 707 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 97

### mochi / settings-int-emptied — pass

    mochi card on about:settings: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 16-pref browser.sessionhistory.max_entries = [20]

### mochi / settings-persist — pass

    mochi card on about:settings: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-pref general.smoothScroll = [true]
    WAIT ok 14-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 16-pref general.smoothScroll = [true]

### mochi / settings-tabs — pass

    mochi card on about:settings: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    CHECK ok 9-text: page-title = [Settings]
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content]

### mochi / settings-typing — pass

    mochi card on about:settings: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 15-pref browser.display.background_color = [#FFFFFFx]
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / ssl-confirm — not run

    needs internet (run with --with internet)

### mochi / tabbed-browsing — not run

    not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word)

### mochi / touchpad-go — not run

    owner-accepted without hardware (device-build R6)

### mochi / typing-caret — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +1 s, load done at +6 s, 9 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 297 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 11-text echo = [plain n=6 c=0 last=[abcdef] value=[abcdef] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 13-text echo = [plain n=12 c=0 last=[] value=[] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / video-controls — fail

    mochi card on http://192.168.1.245:8077/jihad-media-test.html: connected at +1 s, load done at +7 s, 25 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect status
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 98 landed on document y 25)
    CHECK FAIL 12-wait-changed: the region sel:video did not change within the bound
    CHECK FAIL 13-wait-changed: the region sel:video did not change within the bound
    WAIT ok 15-still sel:video
    WAIT ok 17-still sel:video
    CHECK ok 18-region-equal: region 16,172-517,452: colours 2439 hash 15d5cf20 rows 280 uniform 0 light 440-444 blue none = the sample's hash

### mochi / webgl-cube — pass

    mochi card on http://192.168.1.245:8077/jihad-webgl-cube.html: connected at +2 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 7-rect c
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 120 landed on document y 57)
    CHECK ok 9-text-has: out = [webgl fps=25.1 shaders=ok texture=ok framebuffer=ok readPixels=ok maxtex=4096 ext=14 missing=ANGLE_instanced_arrays,WEBGL_compressed_texture_s3tc,EXT_blend_minmax,EXT_shader_texture_lod depthrb=ok renderer=Mozilla vendor=Mozilla] holds [webgl]
    CHECK ok 10-content: region 105,171-457,520: colours 920 hash 9b932c16 rows 349 uniform 141 light none blue 241-444; c on the screen at 105,170-457,521
    WAIT ok 11-changed c
    WAIT ok 13-zoom
    CHECK ok 15-content: region 0,100-768,900: colours 1339 hash e231dcc7 rows 800 uniform 266 light none blue 304-367; c on the screen at -51,83-499,633
    WAIT ok 16-changed c

### mochi / webone — not run

    needs webone, keyboard-map (run with --with webone,keyboard-map)

### mochi / wikipedia-keyboard-zoom — not run

    needs internet (run with --with internet)

### mochi / wikipedia-zoom — not run

    needs internet (run with --with internet)

### mochi / xpi-confirm — pass

    mochi card on http://192.168.1.245:8077/jihad-xpi-trigger.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 8-rect effect
    calibrated (mochi): the page's top on the screen 79 (the table said 78, the tap at y 125 landed on document y 59)
    CHECK ok 11-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 14-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 15-text r = [r: callback status=0 Jihad Effect Test]
    WAIT ok 16-launch: load done
    WAIT ok 17-rect sel:#category-extension
    WAIT ok 19-rect sel:#category-extension[selected='true']
    WAIT ok 21-rect sel:richlistitem[value='jihad-effect@riverstonerelay.net']

### mochi / youtube-mobile — not run

    needs internet (run with --with internet)

### mochi / youtube-no-next — not run

    needs internet (run with --with internet)

### mochi / zoom-dtap-column — pass

    mochi card on http://192.168.1.245:8077/jihad-dtap.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    WAIT ok 7-rect sel:#col
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 110 landed on document y 50)
    WAIT ok 10-zoom
    WAIT ok 11-settled: the card's scroll 590 271
    CHECK ok 12-zoom-min: zoom 1.8286 (at least 1.5)
    CHECK ok 13-element-at: sel:#col on the screen at 18,-195-750,1998 holds 384,420
    CHECK ok 14-content: region 0,100-756,700: colours 402 hash 6c6a1b91 rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-195-750,1998

### mochi / zoom-field-kept — pass

    mochi card on http://192.168.1.245:8077/jihad-dtap.html: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    WAIT ok 7-rect sel:#f
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 110 landed on document y 50)
    CHECK ok 9-zoom-max: zoom 0.6400 (at most 0.8)
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-settled: the card's scroll 0 425
    CHECK ok 14-above-keyboard: sel:#f on the screen at 64,638-324,667, above the keyboard
    CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8)

### mochi / zoom-locked-pinch-off — pass

    mochi card on http://192.168.1.245:8077/jihad-locked.html: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 8-rect sel:h1
    WAIT ok 12-settled: the card's scroll 0 0
    CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05)

### mochi / zoom-locked-pinch — pass

    mochi card on http://192.168.1.245:8077/jihad-locked.html: connected at +2 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect sel:h1
    WAIT ok 11-zoom
    WAIT ok 12-settled: the card's scroll 185 203
    CHECK ok 13-zoom-min: zoom 1.9268 (at least 1.3)

### mochi / zoom-new-page — not run

    needs internet (run with --with internet)

### mojo / addon-site — not run

    needs internet (run with --with internet)

### mojo / addons-lifecycle — pass

    mojo card on about:addons: connected at +2 s, load done at +10 s, 7 content frames at +12 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 249 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='pad01@jihad.test']
    WAIT ok 13-rect anon:richlistitem[value='pad01@jihad.test']|disable-btn
    WAIT ok 15-rect anon:richlistitem[value='pad01@jihad.test']|enable-btn
    WAIT ok 17-rect anon:richlistitem[value='pad01@jihad.test']|undo-btn
    WAIT ok 19-rect anon:richlistitem[value='pad01@jihad.test']|remove-btn

### mojo / addons-list-pan — pass

    mojo card on about:addons: connected at +1 s, load done at +8 s, 4 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 249 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:richlistitem[value='pad12@jihad.test']
    WAIT ok 15-still 0,100,756,1000
    WAIT ok 17-rect sel:richlistitem[value='pad12@jihad.test'][selected='true']
    CHECK ok 18-hit: clickAt 843,1439 inside sel:richlistitem[value="pad12@jihad.test"] at 294,1412 656x99

### mojo / addons-page — pass

    mojo card on about:addons: connected at +1 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#category-extension
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 249 landed on document y 224)
    WAIT ok 9-rect sel:#category-extension[selected='true']
    WAIT ok 10-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:#category-plugin[selected='true']
    WAIT ok 14-rect sel:#category-extension[selected='true']
    WAIT ok 16-rect sel:#detail-view:not([hidden])
    CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill]

### mojo / addons-search-hide-key — pass

    mojo card on about:addons: connected at +1 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +5 s, 2 content frames at +35 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 10-differs 0,30,300,180
    WAIT ok 11-still 0,30,300,180
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 16-log: [jihad-bs] find 'Zebrafind' -> 1

### mojo / appmenu-print — fail

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +5 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 11-differs 0,30,300,180
    WAIT ok 12-still 0,30,300,180
    CHECK FAIL 14-wait-line: no line luna: findPrinters within the bound
    CHECK FAIL 15-wait-differs: 0,200,756,960 still shows the sample page after the bound
    CHECK FAIL 16-wait-log: no line print: mdns: .* -> [0-9.]*:[0-9]* in the daemon log within the bound
    WAIT ok 17-still 0,200,756,960

### mojo / appmenu-settings-keyboard-down — fail

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    WAIT ok 15-load
    CHECK FAIL 16-url: load done uri=[about:reader?url=http%3A%2F%2F192.168.1.245%3A8077%2Fjihad-long-static.html] lacks about:settings
    CHECK FAIL 17-keyboard: the keyboard is not down within 8 s (its last reading: focused 0 wanted, 1 read)
    CHECK FAIL 18-wait-rect: no element page-title with a size within the bound
    CHECK FAIL 19-content: region 0,100-768,700: colours 610 hash 01af5e3a rows 600 uniform 0 light 100-691 blue none; page-title on the screen at 0,73-,, text [(no element)] LACKS [Settings]

### mojo / audio-heard — a person's

    hearing the speaker; the route and the mixer are measured by script (cavekit-addons-extensions R7)

### mojo / busy-script-stop — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +5 s, 5 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 9-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 13-rect sel:h1
    CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36

### mojo / card-dialog — pass

    mojo card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +2 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect c
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 133 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 13-text r = [c: true]
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-2.fifo
    CHECK ok 17-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 18-text r = [c: false]

### mojo / card-select — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +6 s, 9 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 9-text picked = [picked: banana]

### mojo / chrome-buttons — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 9-load
    WAIT ok 11-url: https://lite.duckduckgo.com/lite/
    WAIT ok 13-url: http://192.168.1.245:8077/jihad-long-static.html
    WAIT ok 15-url: https://lite.duckduckgo.com/lite/
    WAIT ok 18-differs 0,100,756,700

### mojo / chrome-history — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### mojo / chrome-padlock — not run

    needs internet (run with --with internet)

### mojo / chrome-share — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +5 s, 7 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### mojo / ddg-results — not run

    needs internet (run with --with internet)

### mojo / dialog-slow-answer — pass

    mojo card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +2 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect c
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 133 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text r = [c: true]

### mojo / download-finishes — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-36

### mojo / download-list — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-37
    CHECK ok 10-file: /media/internal/downloads/jihad-download-37 (17 bytes) holds [Hello from Jihad]
    WAIT ok 12-differs 0,100,756,960
    WAIT ok 14-still 0,100,756,960
    WAIT ok 17-differs 0,100,756,960
    WAIT ok 18-still 0,100,756,960
    WAIT ok 22-differs 450,80,768,400
    WAIT ok 23-still 450,80,768,400
    WAIT ok 26-content 10,90,400,300

### mojo / drag-and-pinch-page — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 9-card-scroll-min: the card's scroll y 583 (at least 400)
    WAIT ok 11-zoom
    CHECK ok 12-zoom-min: zoom 2.6497 (at least 1.5)
    CHECK ok 13-content: region 0,150-756,600: colours 337 hash ffd1b832 rows 450 uniform 215 light 176-599 blue none; sel:body on the screen at -635,-2292-1388,51994
    WAIT ok 15-zoom
    WAIT ok 17-zoom
    CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15)

### mojo / etsy-search — not run

    needs internet (run with --with internet)

### mojo / everyday-browsing — not run

    needs internet (run with --with internet)

### mojo / express-burst-wiki — not run

    needs internet (run with --with internet)

### mojo / flash-api — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +1 s, load done at +7 s, 5 content frames at +7 s; the GPU path, f
    WAIT ok 7-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 8-text vars = [vars a=hello]
    WAIT ok 9-text so = [so n=1]
    WAIT ok 10-text echoback = [echoback back hi]
    CHECK ok 11-text: echo = [echo hi]
    WAIT ok 12-text geturl = [geturl ok]
    WAIT ok 13-text fscommand = [fscommand jcmd jarg]
    WAIT ok 14-text data = [data true x=42]
    WAIT ok 15-text kid = [kid w=376]
    WAIT ok 17-text so = [so n=2]
    WAIT ok 18-launch: load done
    WAIT ok 19-rect m
    CHECK ok 20-text: count = [0]
    CHECK ok 22-text: count = [0]

### mojo / flash-fullscreen — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&vars=fs=1: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main 
    WAIT ok 8-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 9-text kid = [kid w=376]
    WAIT ok 11-text fs = [fs fullScreen]
    WAIT ok 14-text fs = [fs normal]
    WAIT ok 17-text fs = [fs fullScreen]
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 20-flash-behind: the colour 00ff00 in 192 of 192 cells (wanted yes)

### mojo / flash-input-cover — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +2 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the expre
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 175 px up for the keyboard, the page at 175
    WAIT ok 14-settled: the card's scroll 0 175
    CHECK ok 16-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mojo / flash-input-hold — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-text up = [up 201 159]
    CHECK ok 10-number: heldms = 2386 (wanted 1500..3200)
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-number: n-up = 1 (wanted 1..1)
    WAIT ok 14-text up = [up 361 287]
    CHECK ok 15-flash-el: the movie's move at 334,269 for 360,277 (within 30)
    CHECK ok 16-press-rate: 22.5 moves a second sent on (at least 20): ] a press of 1491 ms: 34 moves from the card in 1380 ms, 31 sent on, 547 ms in sending them

### mojo / flash-input-kernel — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +8 s, 14 content frames at +8 s; the GPU path, frames off the main thread (the express 
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 10-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 13-text keydown = [keydown 38 0]
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 40 0]
    CHECK ok 18-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 23-settled: the card's scroll 0 51
    CHECK ok 24-card-scroll-min: the card's scroll y 51 (at least 20)

### mojo / flash-input-keyhold — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +8 s, 10 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 14-text keyup = [keyup 65 97]
    CHECK ok 15-number: keyheldms = 128 (wanted 60..2000)
    WAIT ok 17-text keyup = [keyup 39 0]
    CHECK ok 18-number: keyheldms = 146 (wanted 60..2000)
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / flash-input-keys — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 49 49]
    WAIT ok 19-text keydown = [keydown 32 32]
    CHECK ok 20-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    WAIT ok 22-text keydown = [keydown 38 0]
    WAIT ok 24-text keydown = [keydown 39 0]
    WAIT ok 26-text keydown = [keydown 40 0]
    WAIT ok 28-text keydown = [keydown 37 0]
    CHECK ok 29-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / flash-input-lock — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +1 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express l
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 52
    CHECK ok 10-card-scroll-min: the card's scroll y 52 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-number: n-down = 1 (wanted 1..1)
    CHECK ok 16-number: n-up = 1 (wanted 1..1)
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 20-settled: the card's scroll 0 0
    CHECK ok 21-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 22-number: n-move = 16 (wanted 5..400)
    CHECK ok 24-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 27-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 30-settled: the card's scroll 0 100
    CHECK ok 31-card-scroll-min: the card's scroll y 100 (at least 30)

### mojo / flash-input-tap — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 10 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-number: n-down = 1 (wanted 1..1)
    CHECK ok 11-number: n-up = 1 (wanted 1..1)
    CHECK ok 14-flash-at: the movie reported down 101,60 for stage 100,60 (within 3)
    CHECK ok 15-number: n-down = 2 (wanted 2..2)
    CHECK ok 16-number: n-up = 2 (wanted 2..2)
    WAIT ok 17-launch: load done
    WAIT ok 18-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 21-number: n-down = 1 (wanted 1..1)
    CHECK ok 22-number: n-up = 1 (wanted 1..1)

### mojo / flash-lane — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off th
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,114-440,412: 3.0 s, flips 42 (14.0/s), changes 40 (13.3/s), reads 44, distinct 41, spacing median 67 p90 108 max 205 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,114-440,412: 3.0 s, flips 40 (13.3/s), changes 0 (0.0/s), reads 41, distinct 1, spacing median 66 p90 156 max 179 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,114-440,412: 3.0 s, flips 41 (13.7/s), changes 39 (13.0/s), reads 42, distinct 40, spacing median 68 p90 107 max 192 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 93
    CHECK ok 16-flash-rate: content 40,0-440,275: 3.0 s, flips 43 (14.3/s), changes 41 (13.7/s), reads 44, distinct 42, spacing median 67 p90 81 max 145 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,275: 3.0 s, flips 40 (13.3/s), changes 0 (0.0/s), reads 41, distinct 1, spacing median 68 p90 147 max 201 ms (wanted 0..4/s)

### mojo / flash-layout — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=percent: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off the main thread (the e
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 4)
    WAIT ok 10-launch: load done
    WAIT ok 11-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 13-flash-at: the movie reported down 101,100 for stage 100,100 (within 3)
    CHECK ok 15-flash-at: the movie reported down 301,200 for stage 300,200 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 19-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 20-launch: load done
    WAIT ok 21-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 23-settled: the card's scroll 0 1354
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-rect show
    WAIT ok 29-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mojo / flash-sound — a person's

    hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route)

### mojo / flash-tags — fail

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [10.3]
    CHECK ok 11-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [mime yes]
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 19-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 20-launch: load done
    WAIT ok 21-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 23-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 24-launch: load done
    CHECK FAIL 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 8 (2.7/s), changes 4 (1.3/s), reads 20, distinct 5, spacing median 107 p90 319 max 319 ms (wanted 5..100/s)

### mojo / flash-wmode — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm1&bg=00ff00&wmode=window: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main threa
    WAIT ok 7-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 8-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 10-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 11-launch: load done
    WAIT ok 12-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 13-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 18-flash-behind: the colour 00ff00 in 188 of 192 cells (wanted yes)
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 21-launch: load done
    WAIT ok 22-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 23-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 28-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mojo / fonts-pick — pass

    mojo card on about:settings: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    WAIT ok 11-rect fonts-reset
    WAIT ok 13-pref font.name.serif.x-western = [Dotum]
    WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.]

### mojo / forget-identities — pass

    mojo card on about:settings: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.]

### mojo / frame-field — pass

    mojo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
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

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / keyboard-arbitration — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express l
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    WAIT ok 14-text keydown = [keydown 65 97]
    CHECK ok 15-region-equal: region 150,30-620,78: colours 173 hash ff22f881 rows 48 uniform 30 light 35-77 blue none = the sample's hash
    CHECK ok 19-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 22-settled: the card's scroll 0 0
    CHECK ok 23-region-changed: region 150,30-620,78: colours 4 hash f7bf3bef rows 48 uniform 48 light 35-77 blue none against the sample's hash ff22f881
    CHECK ok 25-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / keyboard-fields — pass

    mojo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 292 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 10-visible: area on the screen at 9,151-484,191 (the reach ends at 600, keyboard 1)
    CHECK ok 12-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 13-launch: load done
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 17-visible: field on the screen at 16,275-522,310 (the reach ends at 600, keyboard 1)

### mojo / keyboard-keys — pass

    mojo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 8-rect plain
    calibrated (mojo): the page's top on the screen 74 (the table said 73, the tap at y 134 landed on document y 77)
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 15-text echo = [plain n=2 c=0 last=[ab] value=[ab] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 17-text echo = [plain n=2 c=1 last=[ab] value=[ab] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 20-text echo = [plain n=3 c=1 last=[b] value=[b] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 25-text echo = [plain n=5 c=1 last=[a] value=[a] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 27-text echo = [plain n=5 c=2 last=[a] value=[a] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=0]
    WAIT ok 33-text echo = [plain n=5 c=2 last=[a] value=[a] area n=5 c=0 last=[a b c] value=[a b c] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=2]
    WAIT ok 37-text echo = [plain n=5 c=2 last=[a] value=[a] area n=6 c=0 last=[ab c] value=[ab c] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=2]
    WAIT ok 41-text echo = [plain n=5 c=2 last=[a] value=[a] area n=8 c=0 last=[ab] value=[ab] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=2]

### mojo / long-press-image — not run

    needs internet (run with --with internet)

### mojo / long-press-menu — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mojo / long-press-text-field — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +5 s, 7 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 10-differs 0,100,756,600
    WAIT ok 13-still 0,100,756,600
    WAIT ok 15-line: [jihad-bs] hold at 270,219 editable=1 wasFocused=0
    WAIT ok 18-still 0,100,400,240
    WAIT ok 20-load
    CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mojo / media-controls — pass

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +7 s, 18 content frames at +7 s; the GPU path, frames off the main thread (the ex
    WAIT ok 9-rect v
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 93 landed on document y 25)
    WAIT ok 11-text status = [playing]
    WAIT ok 13-text status = [paused t=16.7]
    WAIT ok 18-rect anon:#v|controlBar
    WAIT ok 19-rect anon:#v|muteButton
    WAIT ok 22-text vol = [volume=1.00 muted=false]
    WAIT ok 24-text vol = [volume=1.00 muted=true]
    WAIT ok 26-text vol = [volume=1.00 muted=false]
    CHECK ok 28-visible: anon:#v|controlBar on the screen at 16,572-586,594 (the reach ends at 1024, keyboard 0)
    CHECK ok 30-visible: anon:#v|controlBar on the screen at 16,572-586,594 (the reach ends at 1024, keyboard 0)

### mojo / media-scrub-drag — known

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +6 s, 11 content frames at +6 s; the GPU path, frames off the main thread (the ex
    WAIT ok 10-rect v
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 93 landed on document y 25)
    WAIT ok 12-text status = [playing]
    WAIT ok 14-text status = [paused t=15.4]
    WAIT ok 15-rect anon:#v|controlBar
    WAIT ok 16-rect anon:#v|volumeControl
    WAIT ok 20-text vol = [volume=0.16 muted=false]
    WAIT ok 24-settled: the card's scroll 0 0
    CHECK FAIL 25-wait-text: seek never read 60 within the bound

### mojo / mobile-wiki-pinch — not run

    needs internet (run with --with internet)

### mojo / mojo-address-typing — not run

    needs internet (run with --with internet)

### mojo / mojo-launch-from-app — a person's

    a launch from another app is not emulated (a launch with a target from the command line showed the page with no keyboard, 2026-10-24)

### mojo / pan-long-drag — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 8-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 9-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 10-content: region 0,150-768,400: colours 374 hash 85b98daf rows 250 uniform 0 light 150-399 blue none; sel:#content p on the screen at 16,145-748,201, text holds [quick brown]
    CHECK ok 14-card-scroll: the card's scroll y 605 (want 650 within 100); the engine's 605
    WAIT ok 15-still header
    CHECK ok 16-region-equal: region 0,29-756,83: colours 185 hash eeaac7d6 rows 54 uniform 33 light none blue none = the sample's hash
    CHECK ok 17-region-changed: region 0,300-756,900: colours 738 hash fc61b043 rows 600 uniform 226 light 300-895 blue none against the sample's hash 39f383a9
    CHECK ok 18-top: the page's top measured 73 (the tap at y 203 landed on document y 780), the model says 73 (header hidden: 1)
    CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### mojo / pan-long-flick — known

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +5 s, 5 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 18-card-scroll-min: the card's scroll y 3030 (at least 300)
    STREAM fs-mojo-pan-long-flick.fbs: 116 samples (14.5/s asked 15, read median 2.42 ms max 102.37 ms, mmap stride 8, re-read 9, torn 0), 116 flips, cpu 7.6% of a core, polls 558/s | shown frames while moving: 111 gaps, median 32 ms, p90 47 ms, max 162 ms | shown scroll: 507 rows over the stream (2 samples with the shift unknown); stale bands 1, uniform bands 16, still rows moved 0; the columns outside 0-756 changed in 42 samples | tracking (finger down, 25 samples): error median 20 px, max 375 px; lag median 311 ms, max 1001 ms |   gesture at 1794546187699: the finger travelled 600 rows, the page showe
    CHECK FAIL 16-stream-stale: stale 1 (want at most 0)
    CHECK FAIL 17-stream-uniform: uniform 16 (want at most 0)

### mojo / pan-speeds — known

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 13-card-scroll-min: the card's scroll y 601 (at least 500)
    CHECK ok 22-card-scroll-min: the card's scroll y 4561 (at least 2500)
    STREAM fs-mojo-pan-speeds.fbs: 242 samples (14.4/s asked 15, read median 2.47 ms max 152.9 ms, mmap stride 8, re-read 14, torn 0), 311 flips, cpu 7.6% of a core, polls 545/s | shown frames while moving: 303 gaps, median 30 ms, p90 42 ms, max 264 ms | shown scroll: 3790 rows over the stream (0 samples with the shift unknown); stale bands 8, uniform bands 25, still rows moved 0; the columns outside 0-756 changed in 118 samples | tracking (finger down, 128 samples): error median -11.5 px, max 262 px; lag median 77.0 ms, max 846 ms |   gesture at 1794546248953: the finger travelled 650 rows, the page sho
    CHECK FAIL 19-stream-stale: stale 8 (want at most 0)
    CHECK FAIL 20-stream-uniform: uniform 25 (want at most 0)
    CHECK ok 21-stream-error-max: error-max 262 (at most 500)

### mojo / persona-cycle — pass

    mojo card on about:personas: connected at +1 s, load done at +12 s, 13 content frames at +12 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

### mojo / persona-wear — not run

    needs internet (run with --with internet)

### mojo / personas-flick — pass

    mojo card on about:personas: connected at +1 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#mine .card
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 271 landed on document y 198)
    CHECK ok 10-card-scroll-min: the card's scroll y 1602 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### mojo / personas-page-steady — pass

    mojo card on about:personas: connected at +1 s, load done at +7 s, 10 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-rect sel:#mine .card
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 271 landed on document y 198)
    CHECK ok 16-content: region 0,100-768,900: colours 4096 hash b09292ba rows 800 uniform 0 light 188-743 blue 162-751; sel:body on the screen at 0,-419-764,2448
    STREAM fs-mojo-personas-page-steady.fbs: 78 samples (15.0/s asked 15, read median 2.21 ms max 19.15 ms, mmap stride 8, re-read 3, torn 0), 107 flips, cpu 6.3% of a core, polls 651/s | shown frames while moving: 106 gaps, median 27.5 ms, p90 40 ms, max 267 ms | shown scroll: 494 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 0-756 changed in 41 samples | tracking (finger down, 49 samples): error median -10 px, max 70 px; lag median 76 ms, max 424 ms |   gesture at 1794546464489: the finger travelled 500 rows, the page sh
    CHECK ok 15-stream-uniform: uniform 0 (at most 0)

### mojo / proxy-login-mojo — pass

    mojo card on about:settings: connected at +1 s, load done at +6 s, 10 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 9-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    CHECK ok 12-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 14-still 0,440,756,720
    CHECK ok 21-dialog: [jihad-bs] dialog auth -> ACCEPT (answer=1)
    WAIT ok 22-line: [jihad-bs] load done uri=http://example.com/jihad-proxy-login

### mojo / proxy-own — pass

    mojo card on about:settings: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    mojo card on about:settings: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 11-rect sel:.row[data-pref='network.proxy.type'] select
    WAIT ok 13-rect sel:.row[data-pref='network.proxy.http'] input
    WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input

### mojo / relay-choice — not run

    needs internet (run with --with internet)

### mojo / rotate — a person's

    the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side

### mojo / screen-lock — pass

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +2 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the ex
    WAIT ok 9-rect v
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 93 landed on document y 25)
    WAIT ok 11-text status = [playing]
    WAIT ok 20-text status = [playing]
    CHECK ok 22-text: status = [playing]
    WAIT ok 34-text status = [paused t=38.2]

### mojo / script-hold-zoom — pass

    mojo card on http://192.168.1.245:8077/jihad-script-hold.html: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 101 landed on document y 28)
    WAIT ok 9-zoom
    CHECK ok 10-zoom-min: zoom 1.8894 (at least 1.3)
    CHECK ok 12-card-scroll-min: the card's scroll y 442 (at least 200)
    WAIT ok 14-zoom
    WAIT ok 16-zoom
    CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2)

### mojo / settings-every-control — by a tool

    tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt)

### mojo / settings-field-saves — pass

    mojo card on about:settings: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    mojo card on about:settings: connected at +1 s, load done at +5 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    CHECK ok 10-card-scroll-min: the card's scroll y 675 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### mojo / settings-int-emptied — pass

    mojo card on about:settings: connected at +1 s, load done at +5 s, 10 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 16-pref browser.sessionhistory.max_entries = [20]

### mojo / settings-persist — pass

    mojo card on about:settings: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-pref general.smoothScroll = [true]
    WAIT ok 14-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 16-pref general.smoothScroll = [true]

### mojo / settings-tabs — pass

    mojo card on about:settings: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    CHECK ok 9-text: page-title = [Settings]
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content]

### mojo / settings-typing — pass

    mojo card on about:settings: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 15-pref browser.display.background_color = [#FFFFFFx]
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / ssl-confirm — not run

    needs internet (run with --with internet)

### mojo / tabbed-browsing — not run

    not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word)

### mojo / touchpad-go — not run

    owner-accepted without hardware (device-build R6)

### mojo / typing-caret — pass

    mojo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 292 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 11-text echo = [plain n=6 c=0 last=[abcdef] value=[abcdef] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 13-text echo = [plain n=12 c=0 last=[] value=[] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / video-controls — pass

    mojo card on http://192.168.1.245:8077/jihad-media-test.html: connected at +1 s, load done at +7 s, 25 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect status
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 93 landed on document y 25)
    WAIT ok 12-changed sel:video
    WAIT ok 13-changed sel:video
    WAIT ok 15-still sel:video
    WAIT ok 17-still sel:video
    CHECK ok 18-region-equal: region 16,167-517,447: colours 2206 hash bfcabf27 rows 280 uniform 0 light 435-439 blue none = the sample's hash

### mojo / webgl-cube — pass

    mojo card on http://192.168.1.245:8077/jihad-webgl-cube.html: connected at +1 s, load done at +6 s, 21 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 7-rect c
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 115 landed on document y 57)
    CHECK ok 9-text-has: out = [webgl fps=21.1 shaders=ok texture=ok framebuffer=ok readPixels=ok maxtex=4096 ext=14 missing=ANGLE_instanced_arrays,WEBGL_compressed_texture_s3tc,EXT_blend_minmax,EXT_shader_texture_lod depthrb=ok renderer=Mozilla vendor=Mozilla] holds [webgl]
    CHECK ok 10-content: region 105,166-457,515: colours 645 hash 6e88cc5b rows 349 uniform 138 light none blue 236-432; c on the screen at 105,165-457,516
    WAIT ok 11-changed c
    WAIT ok 13-zoom
    CHECK ok 15-content: region 0,100-768,900: colours 980 hash b0e7b884 rows 800 uniform 319 light none blue 262-353; c on the screen at -47,35-497,580
    WAIT ok 16-changed c

### mojo / webone — not run

    needs webone, keyboard-map (run with --with webone,keyboard-map)

### mojo / wikipedia-keyboard-zoom — not run

    needs internet (run with --with internet)

### mojo / wikipedia-zoom — not run

    needs internet (run with --with internet)

### mojo / xpi-confirm — pass

    mojo card on http://192.168.1.245:8077/jihad-xpi-trigger.html: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 8-rect effect
    calibrated (mojo): the page's top on the screen 74 (the table said 73, the tap at y 120 landed on document y 59)
    CHECK ok 11-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 14-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 15-text r = [r: callback status=0 Jihad Effect Test]
    WAIT ok 16-launch: load done
    WAIT ok 17-rect sel:#category-extension
    WAIT ok 19-rect sel:#category-extension[selected='true']
    WAIT ok 21-rect sel:richlistitem[value='jihad-effect@riverstonerelay.net']

### mojo / youtube-mobile — not run

    needs internet (run with --with internet)

### mojo / youtube-no-next — not run

    needs internet (run with --with internet)

### mojo / zoom-dtap-column — pass

    mojo card on http://192.168.1.245:8077/jihad-dtap.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 7-rect sel:#col
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 105 landed on document y 50)
    WAIT ok 10-zoom
    WAIT ok 11-settled: the card's scroll 590 277
    CHECK ok 12-zoom-min: zoom 1.8286 (at least 1.5)
    CHECK ok 13-element-at: sel:#col on the screen at 18,-211-750,1982 holds 384,420
    CHECK ok 14-content: region 0,100-756,700: colours 325 hash 735e4dc6 rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-211-750,1982

### mojo / zoom-field-kept — pass

    mojo card on http://192.168.1.245:8077/jihad-dtap.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 7-rect sel:#f
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 105 landed on document y 50)
    CHECK ok 9-zoom-max: zoom 0.6400 (at most 0.8)
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 12-settled: the card's scroll 0 420
    CHECK ok 14-above-keyboard: sel:#f on the screen at 64,591-324,621, above the keyboard
    CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8)

### mojo / zoom-locked-pinch-off — pass

    mojo card on http://192.168.1.245:8077/jihad-locked.html: connected at +1 s, load done at +5 s, 6 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 8-rect sel:h1
    WAIT ok 12-settled: the card's scroll 0 0
    CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05)

### mojo / zoom-locked-pinch — pass

    mojo card on http://192.168.1.245:8077/jihad-locked.html: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 7-rect sel:h1
    WAIT ok 11-zoom
    WAIT ok 12-settled: the card's scroll 184 180
    CHECK ok 13-zoom-min: zoom 1.9123 (at least 1.3)

### mojo / zoom-new-page — not run

    needs internet (run with --with internet)

