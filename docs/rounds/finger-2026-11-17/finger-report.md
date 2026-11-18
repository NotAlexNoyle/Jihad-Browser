# Finger suite report — 2026-11-18T10:30:41

First run 2026-11-17T11:47:07; runs marked (rerun) were run again by `--merge`, and the device-check below is the last run's.

Shells: enyo, mochi, mojo. Scripts: 1. 32704 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | addon-site | **pass** | 8 ok, 0 failed | WAIT ok 21-pref lightweightThemes.isThemeSelected = [false] | cavekit-addons-site.md R7 |
| enyo | addons-lifecycle | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/profile/extensions | cavekit-addons-extensions.md R2 |
| enyo | addons-list-pan | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/profile/extensions | cavekit-chrome-look.md R2 |
| enyo | addons-page | **pass** | 7 ok, 0 failed | CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill] | cavekit-addons-extensions.md R2 |
| enyo | addons-search-hide-key | **pass** | 8 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| enyo | address-select-all | **pass** | 4 ok, 0 failed | CHECK ok 16-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| enyo | appmenu-find | **pass** | 4 ok, 0 failed | WAIT ok 16-log: [jihad-bs] find 'zebrafind' -> 1 | cavekit-ui-shell.md R7 |
| enyo | appmenu-print (rerun) | **pass** | 6 ok, 0 failed | WAIT ok 17-still 0,200,756,960; frames: kept/enyo-appmenu-print-print-dialog_.jpg | cavekit-ui-shell.md R7 |
| enyo | appmenu-settings-keyboard-down | **pass** | 8 ok, 0 failed | CHECK ok 19-content: region 0,100-768,700: colours 499 hash 333a5bcc rows 600 uniform 0 light none blue none; page-title on the screen at 20,100-744,134, text holds [Settings] | cavekit-chrome-look.md R4 |
| enyo | autoplay-unmute | **pass** | 3 ok, 0 failed | WAIT ok 11-text status = [paused t=29.7]; frames: kept/enyo-autoplay-unmute-paused-on-unmute_.jpg | cavekit-gre-widgets.md R1 |
| enyo | busy-script-stop | **pass** | 4 ok, 0 failed | CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36; frames: kept/enyo-busy-script-stop-busy-dialog_.jpg | cavekit-modern-web.md R2 |
| enyo | card-dialog | **pass** | 7 ok, 0 failed | WAIT ok 18-text r = [c: false] | cavekit-browser-services.md R3 |
| enyo | card-select | **pass** | 1 ok, 0 failed | WAIT ok 9-text picked = [picked: banana] | cavekit-device-build.md R4 |
| enyo | chrome-buttons | **pass** | 5 ok, 0 failed | WAIT ok 18-differs 0,100,756,700; frames: kept/enyo-chrome-buttons-new-card_.jpg | cavekit-device-build.md R4 |
| enyo | chrome-history | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/enyo-chrome-history-history-panel_.jpg | cavekit-device-build.md R4 |
| enyo | chrome-padlock | **pass** | 3 ok, 0 failed | WAIT ok 13-differs 0,100,756,960; frames: kept/enyo-chrome-padlock-padlock_.jpg | cavekit-ui-shell.md R8 |
| enyo | chrome-share | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/enyo-chrome-share-share-menu_.jpg | cavekit-device-build.md R4 |
| enyo | codecs-play | **pass** | 11 ok, 0 failed | WAIT ok 21-text r10 = [aac canPlay=probably result=ended t=10.03/10.03 err=0] | cavekit-lowpower-uxp.md R4 |
| enyo | ddg-results | **pass** | 3 ok, 0 failed | CHECK ok 10-content: region 0,100-756,900: colours 963 hash 5895ba32 rows 800 uniform 195 light 100-899 blue none; sel:body on the screen at 0,82-768,2010 | cavekit-modern-web.md R2 |
| enyo | dialog-slow-answer | **pass** | 4 ok, 0 failed | WAIT ok 14-text r = [c: true] | cavekit-finger-testing.md R4 |
| enyo | download-finishes | **pass** | 1 ok, 0 failed | WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-38 | cavekit-device-build.md R4 |
| enyo | download-list | **pass** | 9 ok, 0 failed | WAIT ok 24-content 460,90,760,300; frames: kept/enyo-download-list-panel_.jpg, kept/enyo-download-list-downloads_.jpg | cavekit-device-build.md R4 |
| enyo | drag-and-pinch-page | **pass** | 7 ok, 0 failed | CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15) | cavekit-input-bridging.md R3 |
| enyo | etsy-search (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 12-content: region 0,100-768,900: colours 4096 hash 105343fb rows 800 uniform 119 light 100-899 blue none; sel:body on the screen at 0,82-768,8849 | cavekit-modern-web.md R2 |
| enyo | everyday-browsing | **pass** | 5 ok, 0 failed | WAIT ok 17-url: https://duckduckgo.com/ | cavekit-engine-update.md R8 |
| enyo | express-burst-wiki | **pass** | 5 ok, 0 failed | CHECK ok 17-content: region 0,100-756,700: colours 1097 hash 7b6c2598 rows 600 uniform 0 light 101-699 blue 100-584; sel:body on the screen at -1237,-2489-1359,708 | cavekit-gpu-rendering.md R3 |
| enyo | flash-api (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| enyo | flash-fullscreen | **pass** | 8 ok, 0 failed | CHECK ok 20-flash-behind: the colour 00ff00 in 180 of 192 cells (wanted yes); frames: kept/enyo-flash-fullscreen-fullscreen_.jpg | cavekit-flash.md R4 |
| enyo | flash-input-cover | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| enyo | flash-input-hold (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 17-press-rate: 26.5 moves a second sent on (at least 20): ] a press of 1441 ms: 32 moves from the card in 1207 ms, 32 sent on, 232 ms in sending them | cavekit-flash.md R5 |
| enyo | flash-input-kbdrag | **pass** | 8 ok, 0 failed | CHECK ok 23-flash-at: the movie reported down 301,100 for stage 300,100 (within 8) | cavekit-flash.md R5 |
| enyo | flash-input-kernel (rerun) | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| enyo | flash-input-keyhold | **pass** | 7 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-flash.md R6 |
| enyo | flash-input-keys | **pass** | 13 ok, 0 failed | CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-flash.md R5 |
| enyo | flash-input-lock | **pass** | 15 ok, 0 failed | CHECK ok 31-card-scroll-min: the card's scroll y 100 (at least 30) | cavekit-flash.md R5 |
| enyo | flash-input-tap (rerun) | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| enyo | flash-layout | **pass** | 17 ok, 0 failed | CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| enyo | flash-tags | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 55 (18.3/s), changes 32 (10.7/s), reads 57, distinct 33, spacing median 44 p90 88 max 246 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| enyo | flash-text (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-flash.md R7 |
| enyo | flash-wmode | **pass** | 19 ok, 0 failed | CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| enyo | fonts-pick | **pass** | 5 ok, 0 failed | WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.] | cavekit-preferences-ui.md R9 |
| enyo | forget-identities | **pass** | 3 ok, 0 failed | WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.] | cavekit-site-identity.md R1 |
| enyo | frame-field (rerun) | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| enyo | keyboard-address | **pass** | 2 ok, 0 failed | CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| enyo | keyboard-arbitration | **pass** | 8 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R7 |
| enyo | keyboard-fields (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 17-visible: field on the screen at 16,284-522,319 (the reach ends at 600, keyboard 1) | cavekit-input-bridging.md R2 |
| enyo | keyboard-keys (rerun) | **pass** | 10 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R2 |
| enyo | long-press-image | **pass** | 2 ok, 0 failed | WAIT ok 12-differs 0,100,756,900; frames: kept/enyo-long-press-image-hold-image_.jpg | cavekit-ui-shell.md R7 |
| enyo | long-press-menu | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| enyo | long-press-text-field (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html; frames: kept/enyo-long-press-text-field-hold-text_.jpg, kept/enyo-long-press-text-field-hold-field_.jpg | cavekit-ui-shell.md R7 |
| enyo | media-controls | **pass** | 10 ok, 0 failed | CHECK ok 30-visible: anon:#v/controlBar on the screen at 16,581-586,603 (the reach ends at 1024, keyboard 0); frames: kept/enyo-media-controls-controls_.jpg, kept/enyo-media-controls-bar_.jpg | cavekit-gre-widgets.md R1 |
| enyo | media-scrub-drag | **pass** | 8 ok, 0 failed | WAIT ok 25-text seek = [60]; frames: kept/enyo-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |
| enyo | mobile-wiki-pinch | **pass** | 8 ok, 0 failed | CHECK ok 18-content: region 0,100-756,700: colours 4096 hash dd9a5fda rows 600 uniform 8 light 100-699 blue none; sel:body on the screen at -634,-441-1401,2065 | cavekit-mojo-ui.md R6 |
| enyo | pan-long-drag | **pass** | 9 ok, 0 failed | CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| enyo | pan-long-flick (rerun) | **known** | 3 ok, 2 failed | the open defect T-567, T-567: CHECK FAIL 16-stream-stale: stale 2 (want at most 0) / CHECK FAIL 17-stream-uniform: uniform 46 (want at most 0) | cavekit-gpu-rendering.md R3 |
| enyo | pan-speeds (rerun) | **known** | 5 ok, 2 failed | the open defect T-567, T-567: CHECK FAIL 19-stream-stale: stale 1 (want at most 0) / CHECK FAIL 20-stream-uniform: uniform 5 (want at most 0) | cavekit-finger-testing.md R4 |
| enyo | persona-cycle | **pass** | 12 ok, 0 failed | persona file: put | cavekit-personas-ui.md R6 |
| enyo | persona-wear | **pass** | 7 ok, 0 failed | WAIT ok 18-pref lightweightThemes.isThemeSelected = [false] | cavekit-personas-ui.md R6 |
| enyo | personas-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| enyo | personas-page-steady (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 18-stream-gap-p90: gap-p90 39 (at most 50) | cavekit-personas-ui.md R9 |
| enyo | proxy-login | **pass** | 11 ok, 0 failed | auth proxy: running; frames: kept/enyo-proxy-login-auth-dialog_.jpg | cavekit-proxies-certs.md R5 |
| enyo | proxy-own | **pass** | 8 ok, 0 failed | auth proxy: running | cavekit-proxies-certs.md R1 |
| enyo | proxy-section-redraw | **pass** | 5 ok, 0 failed | WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input | cavekit-proxies-certs.md R1 |
| enyo | relay-choice | **pass** | 6 ok, 0 failed | CHECK ok 17-text-has: sel:p = [This domain is for use in documentation examples without needing permission. This is not a service; avoid relying on it for testing and monitoring purposes.] holds [documentation] | cavekit-proxies-certs.md R2 |
| enyo | rotate (rerun) | **pass** | 10 ok, 0 failed | CHECK ok 28-card-scroll-min: the card's scroll y 650 (at least 400); frames: kept/enyo-rotate-landscape-left_.jpg, kept/enyo-rotate-landscape-right_.jpg | cavekit-device-build.md R4 |
| enyo | screen-lock | **pass** | 5 ok, 0 failed | WAIT ok 34-text status = [paused t=37.9]; frames: kept/enyo-screen-lock-lock-button_.jpg, kept/enyo-screen-lock-locked_.jpg, kept/enyo-screen-lock-ask_.jpg, kept/enyo-screen-lock-unlocked_.jpg | cavekit-gre-widgets.md R1 |
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
| enyo | video-controls | **pass** | 6 ok, 0 failed | CHECK ok 18-region-equal: region 16,176-517,456: colours 2439 hash e970f523 rows 280 uniform 0 light 444-448 blue none = the sample's hash | cavekit-gre-widgets.md R1 |
| enyo | webgl-cube | **pass** | 7 ok, 0 failed | WAIT ok 16-changed c | cavekit-gpu-rendering.md R8 |
| enyo | webone | **not run** | 0 ok, 0 failed | needs webone, keyboard-map (run with --with webone,keyboard-map) | cavekit-proxies-certs.md R3 |
| enyo | wikipedia-keyboard-zoom | **pass** | 10 ok, 0 failed | CHECK ok 20-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad] | cavekit-mochi-ui.md R6 |
| enyo | wikipedia-zoom (rerun) | **known** | 11 ok, 1 failed | the open defect T-567: CHECK FAIL 27-stream-uniform: uniform 13 (want at most 0) | cavekit-gpu-rendering.md R6 |
| enyo | xpi-confirm | **pass** | 8 ok, 0 failed | xpi: 649 bytes served as jihad-effect.xpi; frames: kept/enyo-xpi-confirm-xpi-prompt_.jpg, kept/enyo-xpi-confirm-extensions-pane_.jpg | cavekit-addons-extensions.md R3 |
| enyo | youtube-feed-no-preview | **pass** | 5 ok, 0 failed | CHECK ok 17-probe-has: the probe's answer holds [playing 0] | cavekit-device-headroom.md R4 |
| enyo | youtube-mobile | **pass** | 3 ok, 0 failed | WAIT ok 11-changed 0,130,756,560; frames: kept/enyo-youtube-mobile-playing_.jpg | cavekit-modern-web.md R2 |
| enyo | youtube-no-next | **pass** | 6 ok, 0 failed | CHECK ok 17-url: load done uri=https://m.youtube.com/watch?v=jNQXAC9IVRw&pp=0gcJCVgCo7VqN5tD | cavekit-device-headroom.md R4 |
| enyo | zoom-dtap-column | **pass** | 6 ok, 0 failed | CHECK ok 14-content: region 0,100-756,700: colours 438 hash 2976668a rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-193-750,2000 | cavekit-input-bridging.md R5 |
| enyo | zoom-field-kept | **pass** | 6 ok, 0 failed | CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8) | cavekit-input-bridging.md R5 |
| enyo | zoom-locked-pinch-off | **pass** | 3 ok, 0 failed | CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05) | cavekit-input-bridging.md R5 |
| enyo | zoom-locked-pinch | **pass** | 4 ok, 0 failed | CHECK ok 13-zoom-min: zoom 1.9484 (at least 1.3) | cavekit-input-bridging.md R5 |
| enyo | zoom-new-page | **pass** | 8 ok, 0 failed | CHECK ok 17-zoom: zoom 0.6882 (want 0.69 within 0.06) | cavekit-input-bridging.md R3 |
| mochi | addon-site | **pass** | 8 ok, 0 failed | WAIT ok 21-pref lightweightThemes.isThemeSelected = [false] | cavekit-addons-site.md R7 |
| mochi | addons-lifecycle | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser-mochi/profile/extensions | cavekit-addons-extensions.md R2 |
| mochi | addons-list-pan | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser-mochi/profile/extensions | cavekit-chrome-look.md R2 |
| mochi | addons-page | **pass** | 7 ok, 0 failed | CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill] | cavekit-addons-extensions.md R2 |
| mochi | addons-search-hide-key | **pass** | 8 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mochi | address-select-all | **pass** | 4 ok, 0 failed | CHECK ok 16-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mochi | appmenu-find | **pass** | 4 ok, 0 failed | WAIT ok 16-log: [jihad-bs] find 'zebrafind' -> 1 | cavekit-ui-shell.md R7 |
| mochi | appmenu-print (rerun) | **pass** | 6 ok, 0 failed | WAIT ok 17-still 0,200,756,960; frames: kept/mochi-appmenu-print-print-dialog_.jpg | cavekit-ui-shell.md R7 |
| mochi | appmenu-settings-keyboard-down | **pass** | 8 ok, 0 failed | CHECK ok 19-content: region 0,100-768,700: colours 787 hash 3b16afe0 rows 600 uniform 0 light none blue none; page-title on the screen at 20,96-744,130, text holds [Settings] | cavekit-chrome-look.md R4 |
| mochi | busy-script-stop | **pass** | 4 ok, 0 failed | CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36; frames: kept/mochi-busy-script-stop-busy-dialog_.jpg | cavekit-modern-web.md R2 |
| mochi | card-dialog | **pass** | 7 ok, 0 failed | WAIT ok 18-text r = [c: false] | cavekit-browser-services.md R3 |
| mochi | card-select | **pass** | 1 ok, 0 failed | WAIT ok 9-text picked = [picked: banana] | cavekit-device-build.md R4 |
| mochi | chrome-buttons | **pass** | 5 ok, 0 failed | WAIT ok 18-differs 0,100,756,700; frames: kept/mochi-chrome-buttons-new-card_.jpg | cavekit-device-build.md R4 |
| mochi | chrome-history | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/mochi-chrome-history-history-panel_.jpg | cavekit-device-build.md R4 |
| mochi | chrome-padlock | **pass** | 3 ok, 0 failed | WAIT ok 13-differs 0,100,756,960; frames: kept/mochi-chrome-padlock-padlock_.jpg | cavekit-ui-shell.md R8 |
| mochi | chrome-share | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/mochi-chrome-share-share-menu_.jpg | cavekit-device-build.md R4 |
| mochi | ddg-results | **pass** | 3 ok, 0 failed | CHECK ok 10-content: region 0,100-756,900: colours 133 hash c5b1089c rows 800 uniform 775 light 100-899 blue none; sel:body on the screen at 0,78-768,1942 | cavekit-modern-web.md R2 |
| mochi | dialog-slow-answer | **pass** | 4 ok, 0 failed | WAIT ok 14-text r = [c: true] | cavekit-finger-testing.md R4 |
| mochi | download-finishes | **pass** | 1 ok, 0 failed | WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-40 | cavekit-device-build.md R4 |
| mochi | download-list | **pass** | 9 ok, 0 failed | WAIT ok 25-content 460,90,760,300; frames: kept/mochi-download-list-panel_.jpg, kept/mochi-download-list-downloads_.jpg | cavekit-device-build.md R4 |
| mochi | drag-and-pinch-page | **pass** | 7 ok, 0 failed | CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15) | cavekit-input-bridging.md R3 |
| mochi | etsy-search (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 12-content: region 0,100-768,900: colours 4096 hash db85be2b rows 800 uniform 162 light 100-899 blue none; sel:body on the screen at 0,78-768,8821 | cavekit-modern-web.md R2 |
| mochi | everyday-browsing | **pass** | 5 ok, 0 failed | WAIT ok 17-url: https://duckduckgo.com/ | cavekit-engine-update.md R8 |
| mochi | express-burst-wiki | **pass** | 5 ok, 0 failed | CHECK ok 17-content: region 0,100-756,700: colours 1007 hash 48dc43f9 rows 600 uniform 0 light 100-699 blue 520-697; sel:body on the screen at -1237,-2535-1359,676 | cavekit-gpu-rendering.md R3 |
| mochi | flash-api (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| mochi | flash-fullscreen | **pass** | 8 ok, 0 failed | CHECK ok 20-flash-behind: the colour 00ff00 in 180 of 192 cells (wanted yes); frames: kept/mochi-flash-fullscreen-fullscreen_.jpg | cavekit-flash.md R4 |
| mochi | flash-input-cover | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| mochi | flash-input-hold (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 17-press-rate: 26.4 moves a second sent on (at least 20): ] a press of 1424 ms: 33 moves from the card in 1248 ms, 33 sent on, 143 ms in sending them | cavekit-flash.md R5 |
| mochi | flash-input-kbdrag | **pass** | 8 ok, 0 failed | CHECK ok 23-flash-at: the movie reported down 301,100 for stage 300,100 (within 8) | cavekit-flash.md R5 |
| mochi | flash-input-kernel (rerun) | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| mochi | flash-input-keyhold | **pass** | 7 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-flash.md R6 |
| mochi | flash-input-keys | **pass** | 13 ok, 0 failed | CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-flash.md R5 |
| mochi | flash-input-lock | **pass** | 15 ok, 0 failed | CHECK ok 31-card-scroll-min: the card's scroll y 98 (at least 30) | cavekit-flash.md R5 |
| mochi | flash-input-tap (rerun) | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| mochi | flash-layout | **pass** | 17 ok, 0 failed | CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mochi | flash-tags | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 48 (16.0/s), changes 43 (14.3/s), reads 49, distinct 44, spacing median 67 p90 92 max 106 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| mochi | flash-text (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-flash.md R7 |
| mochi | flash-wmode | **pass** | 19 ok, 0 failed | CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mochi | fonts-pick | **pass** | 5 ok, 0 failed | WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.] | cavekit-preferences-ui.md R9 |
| mochi | forget-identities | **pass** | 3 ok, 0 failed | WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.] | cavekit-site-identity.md R1 |
| mochi | frame-field (rerun) | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mochi | keyboard-address | **pass** | 2 ok, 0 failed | CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| mochi | keyboard-arbitration | **pass** | 8 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R7 |
| mochi | keyboard-fields (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 17-visible: field on the screen at 16,280-522,315 (the reach ends at 600, keyboard 1) | cavekit-input-bridging.md R2 |
| mochi | keyboard-keys (rerun) | **pass** | 10 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R2 |
| mochi | long-press-image | **pass** | 2 ok, 0 failed | WAIT ok 12-differs 0,100,756,900; frames: kept/mochi-long-press-image-hold-image_.jpg | cavekit-ui-shell.md R7 |
| mochi | long-press-menu | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| mochi | long-press-text-field (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html; frames: kept/mochi-long-press-text-field-hold-text_.jpg, kept/mochi-long-press-text-field-hold-field_.jpg | cavekit-ui-shell.md R7 |
| mochi | media-controls | **pass** | 10 ok, 0 failed | CHECK ok 30-visible: anon:#v/controlBar on the screen at 16,577-586,599 (the reach ends at 1024, keyboard 0); frames: kept/mochi-media-controls-controls_.jpg, kept/mochi-media-controls-bar_.jpg | cavekit-gre-widgets.md R1 |
| mochi | media-scrub-drag | **pass** | 8 ok, 0 failed | WAIT ok 25-text seek = [60]; frames: kept/mochi-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |
| mochi | mobile-wiki-pinch | **pass** | 8 ok, 0 failed | CHECK ok 18-content: region 0,100-756,700: colours 4096 hash a219fe3e rows 600 uniform 3 light 100-699 blue none; sel:body on the screen at -634,-452-1401,2064 | cavekit-mojo-ui.md R6 |
| mochi | mochi-paste | **pass** | 4 ok, 0 failed | CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused); frames: kept/mochi-mochi-paste-mochi-field-sheet_.jpg | cavekit-mochi-ui.md R2 |
| mochi | pan-long-drag | **pass** | 9 ok, 0 failed | CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| mochi | pan-long-flick (rerun) | **known** | 3 ok, 2 failed | the open defect T-567, T-567: CHECK FAIL 16-stream-stale: stale 3 (want at most 0) / CHECK FAIL 17-stream-uniform: uniform 26 (want at most 0) | cavekit-gpu-rendering.md R3 |
| mochi | pan-speeds (rerun) | **known** | 6 ok, 1 failed | the open defect T-567: CHECK FAIL 20-stream-uniform: uniform 29 (want at most 0) | cavekit-finger-testing.md R4 |
| mochi | persona-cycle | **pass** | 12 ok, 0 failed | persona file: put | cavekit-personas-ui.md R6 |
| mochi | persona-wear | **pass** | 7 ok, 0 failed | WAIT ok 18-pref lightweightThemes.isThemeSelected = [false] | cavekit-personas-ui.md R6 |
| mochi | personas-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| mochi | personas-page-steady (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 18-stream-gap-p90: gap-p90 36 (at most 50) | cavekit-personas-ui.md R9 |
| mochi | proxy-login | **pass** | 11 ok, 0 failed | auth proxy: running; frames: kept/mochi-proxy-login-auth-dialog_.jpg | cavekit-proxies-certs.md R5 |
| mochi | proxy-own | **pass** | 8 ok, 0 failed | auth proxy: running | cavekit-proxies-certs.md R1 |
| mochi | proxy-section-redraw | **pass** | 5 ok, 0 failed | WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input | cavekit-proxies-certs.md R1 |
| mochi | relay-choice | **pass** | 6 ok, 0 failed | CHECK ok 17-text-has: sel:p = [This domain is for use in documentation examples without needing permission. This is not a service; avoid relying on it for testing and monitoring purposes.] holds [documentation] | cavekit-proxies-certs.md R2 |
| mochi | rotate (rerun) | **pass** | 10 ok, 0 failed | CHECK ok 28-card-scroll-min: the card's scroll y 650 (at least 400); frames: kept/mochi-rotate-landscape-left_.jpg, kept/mochi-rotate-landscape-right_.jpg | cavekit-device-build.md R4 |
| mochi | screen-lock | **pass** | 5 ok, 0 failed | WAIT ok 34-text status = [paused t=33.7]; frames: kept/mochi-screen-lock-lock-button_.jpg, kept/mochi-screen-lock-locked_.jpg, kept/mochi-screen-lock-ask_.jpg, kept/mochi-screen-lock-unlocked_.jpg | cavekit-gre-widgets.md R1 |
| mochi | script-hold-zoom | **pass** | 6 ok, 0 failed | CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2) | cavekit-gpu-rendering.md R3 |
| mochi | settings-every-control | **by a tool** | 0 ok, 0 failed | tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt) | cavekit-preferences-ui.md R6 |
| mochi | settings-field-saves | **pass** | 8 ok, 0 failed | CHECK ok 20-region-equal: region 416,645-735,705: colours 44 hash 9db921ae rows 60 uniform 0 light 645-704 blue none = the sample's hash | cavekit-chrome-look.md R4 |
| mochi | settings-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| mochi | settings-int-emptied | **pass** | 5 ok, 0 failed | WAIT ok 16-pref browser.sessionhistory.max_entries = [20] | cavekit-preferences-ui.md R6 |
| mochi | settings-persist | **pass** | 5 ok, 0 failed | WAIT ok 16-pref general.smoothScroll = [true] | cavekit-preferences-ui.md R2 |
| mochi | settings-tabs | **pass** | 4 ok, 0 failed | CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content] | cavekit-preferences-ui.md R6 |
| mochi | settings-typing | **pass** | 5 ok, 0 failed | CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-preferences-ui.md R6 |
| mochi | ssl-confirm | **pass** | 2 ok, 0 failed | CHECK ok 13-dialog: [jihad-bs] dialog ssl-confirm -> cancel (answer=0); frames: kept/mochi-ssl-confirm-ssl-dialog_.jpg | cavekit-device-build.md R4 |
| mochi | tabbed-browsing | **not run** | 0 ok, 0 failed | not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word) | cavekit-tabbed-browsing.md R1 |
| mochi | touchpad-go | **not run** | 0 ok, 0 failed | owner-accepted without hardware (device-build R6) | cavekit-device-build.md R6 |
| mochi | typing-caret | **pass** | 4 ok, 0 failed | CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| mochi | video-controls | **pass** | 6 ok, 0 failed | CHECK ok 18-region-equal: region 16,172-517,452: colours 2439 hash 2b6888d4 rows 280 uniform 0 light 440-444 blue none = the sample's hash | cavekit-gre-widgets.md R1 |
| mochi | webgl-cube | **pass** | 7 ok, 0 failed | WAIT ok 16-changed c | cavekit-gpu-rendering.md R8 |
| mochi | webone | **not run** | 0 ok, 0 failed | needs webone, keyboard-map (run with --with webone,keyboard-map) | cavekit-proxies-certs.md R3 |
| mochi | wikipedia-keyboard-zoom | **pass** | 10 ok, 0 failed | CHECK ok 20-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad] | cavekit-mochi-ui.md R6 |
| mochi | wikipedia-zoom (rerun) | **known** | 11 ok, 1 failed | the open defect T-567: CHECK FAIL 27-stream-uniform: uniform 5 (want at most 0) | cavekit-gpu-rendering.md R6 |
| mochi | xpi-confirm | **pass** | 8 ok, 0 failed | xpi: 649 bytes served as jihad-effect.xpi; frames: kept/mochi-xpi-confirm-xpi-prompt_.jpg, kept/mochi-xpi-confirm-extensions-pane_.jpg | cavekit-addons-extensions.md R3 |
| mochi | youtube-mobile | **pass** | 3 ok, 0 failed | WAIT ok 11-changed 0,130,756,560; frames: kept/mochi-youtube-mobile-playing_.jpg | cavekit-modern-web.md R2 |
| mochi | youtube-no-next | **pass** | 6 ok, 0 failed | CHECK ok 17-url: load done uri=https://m.youtube.com/watch?v=jNQXAC9IVRw | cavekit-device-headroom.md R4 |
| mochi | zoom-dtap-column | **pass** | 6 ok, 0 failed | CHECK ok 14-content: region 0,100-756,700: colours 402 hash 6c6a1b91 rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-195-750,1998 | cavekit-input-bridging.md R5 |
| mochi | zoom-field-kept | **pass** | 6 ok, 0 failed | CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8) | cavekit-input-bridging.md R5 |
| mochi | zoom-locked-pinch-off | **pass** | 3 ok, 0 failed | CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05) | cavekit-input-bridging.md R5 |
| mochi | zoom-locked-pinch | **pass** | 4 ok, 0 failed | CHECK ok 13-zoom-min: zoom 1.9361 (at least 1.3) | cavekit-input-bridging.md R5 |
| mochi | zoom-new-page | **pass** | 8 ok, 0 failed | CHECK ok 17-zoom: zoom 0.6882 (want 0.69 within 0.06) | cavekit-input-bridging.md R3 |
| mojo | addon-site | **pass** | 8 ok, 0 failed | WAIT ok 21-pref lightweightThemes.isThemeSelected = [false] | cavekit-addons-site.md R7 |
| mojo | addons-lifecycle | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser-mojo/profile/extensions | cavekit-addons-extensions.md R2 |
| mojo | addons-list-pan | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser-mojo/profile/extensions | cavekit-chrome-look.md R2 |
| mojo | addons-page | **pass** | 7 ok, 0 failed | CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill] | cavekit-addons-extensions.md R2 |
| mojo | addons-search-hide-key | **pass** | 8 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mojo | appmenu-find | **pass** | 4 ok, 0 failed | WAIT ok 16-log: [jihad-bs] find 'Zebrafind' -> 1 | cavekit-ui-shell.md R7 |
| mojo | appmenu-print (rerun) | **pass** | 6 ok, 0 failed | WAIT ok 17-still 0,200,756,960; frames: kept/mojo-appmenu-print-print-dialog_.jpg | cavekit-ui-shell.md R7 |
| mojo | appmenu-settings-keyboard-down | **pass** | 8 ok, 0 failed | CHECK ok 19-content: region 0,100-768,700: colours 448 hash 7a103668 rows 600 uniform 0 light none blue none; page-title on the screen at 20,91-744,125, text holds [Settings] | cavekit-chrome-look.md R4 |
| mojo | busy-script-stop | **pass** | 4 ok, 0 failed | CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36; frames: kept/mojo-busy-script-stop-busy-dialog_.jpg | cavekit-modern-web.md R2 |
| mojo | card-dialog | **pass** | 7 ok, 0 failed | WAIT ok 18-text r = [c: false] | cavekit-browser-services.md R3 |
| mojo | card-select | **pass** | 1 ok, 0 failed | WAIT ok 9-text picked = [picked: banana] | cavekit-device-build.md R4 |
| mojo | chrome-buttons | **pass** | 5 ok, 0 failed | WAIT ok 18-differs 0,100,756,700; frames: kept/mojo-chrome-buttons-new-card_.jpg | cavekit-device-build.md R4 |
| mojo | chrome-history | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/mojo-chrome-history-history-panel_.jpg | cavekit-device-build.md R4 |
| mojo | chrome-padlock | **pass** | 3 ok, 0 failed | WAIT ok 13-differs 0,100,756,960; frames: kept/mojo-chrome-padlock-padlock_.jpg | cavekit-ui-shell.md R8 |
| mojo | chrome-share | **pass** | 3 ok, 0 failed | CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56; frames: kept/mojo-chrome-share-share-menu_.jpg | cavekit-device-build.md R4 |
| mojo | ddg-results | **pass** | 3 ok, 0 failed | CHECK ok 10-content: region 0,100-756,900: colours 102 hash 9584832d rows 800 uniform 780 light 100-899 blue none; sel:body on the screen at 0,73-768,2556 | cavekit-modern-web.md R2 |
| mojo | dialog-slow-answer | **pass** | 4 ok, 0 failed | WAIT ok 14-text r = [c: true] | cavekit-finger-testing.md R4 |
| mojo | download-finishes | **pass** | 1 ok, 0 failed | WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-42 | cavekit-device-build.md R4 |
| mojo | download-list | **pass** | 9 ok, 0 failed | WAIT ok 26-content 10,90,400,300; frames: kept/mojo-download-list-panel_.jpg, kept/mojo-download-list-downloads_.jpg | cavekit-device-build.md R4 |
| mojo | drag-and-pinch-page | **pass** | 7 ok, 0 failed | CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15) | cavekit-input-bridging.md R3 |
| mojo | etsy-search (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 12-content: region 0,100-768,900: colours 4096 hash ff3f3399 rows 800 uniform 139 light 100-899 blue 502-537; sel:body on the screen at 0,73-768,8785 | cavekit-modern-web.md R2 |
| mojo | everyday-browsing | **pass** | 5 ok, 0 failed | WAIT ok 17-url: https://duckduckgo.com/ | cavekit-engine-update.md R8 |
| mojo | express-burst-wiki | **pass** | 5 ok, 0 failed | CHECK ok 17-content: region 0,100-756,700: colours 727 hash 4aaa01d3 rows 600 uniform 0 light 100-699 blue 449-674; sel:body on the screen at -1237,-2606-1359,775 | cavekit-gpu-rendering.md R3 |
| mojo | flash-api (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| mojo | flash-fullscreen | **pass** | 8 ok, 0 failed | CHECK ok 20-flash-behind: the colour 00ff00 in 192 of 192 cells (wanted yes); frames: kept/mojo-flash-fullscreen-fullscreen_.jpg | cavekit-flash.md R4 |
| mojo | flash-input-cover | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| mojo | flash-input-hold (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 17-press-rate: 26.1 moves a second sent on (at least 20): ] a press of 1360 ms: 32 moves from the card in 1228 ms, 32 sent on, 100 ms in sending them | cavekit-flash.md R5 |
| mojo | flash-input-kbdrag | **pass** | 8 ok, 0 failed | CHECK ok 23-flash-at: the movie reported down 301,100 for stage 300,100 (within 8) | cavekit-flash.md R5 |
| mojo | flash-input-kernel (rerun) | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| mojo | flash-input-keyhold | **pass** | 7 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-flash.md R6 |
| mojo | flash-input-keys | **pass** | 13 ok, 0 failed | CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-flash.md R5 |
| mojo | flash-input-lock | **pass** | 15 ok, 0 failed | CHECK ok 31-card-scroll-min: the card's scroll y 100 (at least 30) | cavekit-flash.md R5 |
| mojo | flash-input-tap (rerun) | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| mojo | flash-layout | **pass** | 17 ok, 0 failed | CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mojo | flash-tags | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 43 (14.3/s), changes 36 (12.0/s), reads 44, distinct 37, spacing median 67 p90 82 max 165 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| mojo | flash-text (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-flash.md R7 |
| mojo | flash-wmode | **pass** | 19 ok, 0 failed | CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mojo | fonts-pick | **pass** | 5 ok, 0 failed | WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.] | cavekit-preferences-ui.md R9 |
| mojo | forget-identities | **pass** | 3 ok, 0 failed | WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.] | cavekit-site-identity.md R1 |
| mojo | frame-field (rerun) | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mojo | keyboard-address | **pass** | 2 ok, 0 failed | CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| mojo | keyboard-arbitration | **pass** | 8 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R7 |
| mojo | keyboard-fields (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 17-visible: field on the screen at 16,275-522,310 (the reach ends at 600, keyboard 1) | cavekit-input-bridging.md R2 |
| mojo | keyboard-keys (rerun) | **pass** | 10 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R2 |
| mojo | long-press-image | **pass** | 2 ok, 0 failed | WAIT ok 12-differs 0,100,756,900; frames: kept/mojo-long-press-image-hold-image_.jpg | cavekit-ui-shell.md R7 |
| mojo | long-press-menu | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| mojo | long-press-text-field (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html; frames: kept/mojo-long-press-text-field-hold-text_.jpg, kept/mojo-long-press-text-field-hold-field_.jpg | cavekit-ui-shell.md R7 |
| mojo | media-controls | **pass** | 10 ok, 0 failed | CHECK ok 30-visible: anon:#v/controlBar on the screen at 16,572-586,594 (the reach ends at 1024, keyboard 0); frames: kept/mojo-media-controls-controls_.jpg, kept/mojo-media-controls-bar_.jpg | cavekit-gre-widgets.md R1 |
| mojo | media-scrub-drag | **pass** | 8 ok, 0 failed | WAIT ok 25-text seek = [60]; frames: kept/mojo-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |
| mojo | mobile-wiki-pinch | **pass** | 8 ok, 0 failed | CHECK ok 18-content: region 0,100-756,700: colours 4096 hash 1462974a rows 600 uniform 0 light 100-699 blue none; sel:body on the screen at -646,-427-1410,2250 | cavekit-mojo-ui.md R6 |
| mojo | mojo-address-typing (rerun) | **pass** | 4 ok, 0 failed | CHECK ok 16-url: load done uri=https://lite.duckduckgo.com/lite/?q=webos | cavekit-mojo-ui.md R6 |
| mojo | mojo-launch-from-app (rerun) | **pass** | 4 ok, 0 failed | CHECK ok 13-content: region 0,150-756,600: colours 373 hash f5159163 rows 450 uniform 164 light 150-593 blue none; sel:body on the screen at 0,73-764,20561 | cavekit-chrome-look.md R4 |
| mojo | pan-long-drag | **pass** | 9 ok, 0 failed | CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| mojo | pan-long-flick (rerun) | **known** | 1 ok, 4 failed | the open defect T-567, T-567, T-656, T-656: CHECK FAIL 16-stream-stale: stale 4 (want at most 0) / CHECK FAIL 17-stream-uniform: uniform 14 (want at most 0) / CHECK FAIL 19-stream-gap-median: gap-median 35 (want at most 34) / CHECK FAIL 20-stream-gap-p90: gap-p90 55 (want at most 50) | cavekit-gpu-rendering.md R3 |
| mojo | pan-speeds (rerun) | **known** | 6 ok, 1 failed | the open defect T-567: CHECK FAIL 20-stream-uniform: uniform 13 (want at most 0) | cavekit-finger-testing.md R4 |
| mojo | persona-cycle | **pass** | 12 ok, 0 failed | persona file: put | cavekit-personas-ui.md R6 |
| mojo | persona-wear | **pass** | 7 ok, 0 failed | WAIT ok 18-pref lightweightThemes.isThemeSelected = [false] | cavekit-personas-ui.md R6 |
| mojo | personas-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| mojo | personas-page-steady (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 18-stream-gap-p90: gap-p90 40 (at most 50) | cavekit-personas-ui.md R9 |
| mojo | proxy-login-mojo | **pass** | 6 ok, 0 failed | auth proxy: running; frames: kept/mojo-proxy-login-mojo-auth-sheet_.jpg | cavekit-proxies-certs.md R5 |
| mojo | proxy-own | **pass** | 8 ok, 0 failed | auth proxy: running | cavekit-proxies-certs.md R1 |
| mojo | proxy-section-redraw | **pass** | 5 ok, 0 failed | WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input | cavekit-proxies-certs.md R1 |
| mojo | relay-choice | **pass** | 6 ok, 0 failed | CHECK ok 17-text-has: sel:p = [This domain is for use in documentation examples without needing permission. This is not a service; avoid relying on it for testing and monitoring purposes.] holds [documentation] | cavekit-proxies-certs.md R2 |
| mojo | rotate (rerun) | **pass** | 10 ok, 0 failed | CHECK ok 28-card-scroll-min: the card's scroll y 584 (at least 400); frames: kept/mojo-rotate-landscape-left_.jpg, kept/mojo-rotate-landscape-right_.jpg | cavekit-device-build.md R4 |
| mojo | screen-lock | **pass** | 5 ok, 0 failed | WAIT ok 34-text status = [paused t=35.9]; frames: kept/mojo-screen-lock-lock-button_.jpg, kept/mojo-screen-lock-locked_.jpg, kept/mojo-screen-lock-ask_.jpg, kept/mojo-screen-lock-unlocked_.jpg | cavekit-gre-widgets.md R1 |
| mojo | script-hold-zoom | **pass** | 6 ok, 0 failed | CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2) | cavekit-gpu-rendering.md R3 |
| mojo | settings-every-control | **by a tool** | 0 ok, 0 failed | tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt) | cavekit-preferences-ui.md R6 |
| mojo | settings-field-saves | **pass** | 8 ok, 0 failed | CHECK ok 20-region-equal: region 416,643-735,703: colours 38 hash c1c06359 rows 60 uniform 0 light 643-702 blue none = the sample's hash | cavekit-chrome-look.md R4 |
| mojo | settings-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| mojo | settings-int-emptied | **pass** | 5 ok, 0 failed | WAIT ok 16-pref browser.sessionhistory.max_entries = [20] | cavekit-preferences-ui.md R6 |
| mojo | settings-persist | **pass** | 5 ok, 0 failed | WAIT ok 16-pref general.smoothScroll = [true] | cavekit-preferences-ui.md R2 |
| mojo | settings-tabs | **pass** | 4 ok, 0 failed | CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content] | cavekit-preferences-ui.md R6 |
| mojo | settings-typing | **pass** | 5 ok, 0 failed | CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-preferences-ui.md R6 |
| mojo | ssl-confirm | **pass** | 2 ok, 0 failed | CHECK ok 13-dialog: [jihad-bs] dialog ssl-confirm -> cancel (answer=0); frames: kept/mojo-ssl-confirm-ssl-dialog_.jpg | cavekit-device-build.md R4 |
| mojo | tabbed-browsing | **not run** | 0 ok, 0 failed | not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word) | cavekit-tabbed-browsing.md R1 |
| mojo | touchpad-go | **not run** | 0 ok, 0 failed | owner-accepted without hardware (device-build R6) | cavekit-device-build.md R6 |
| mojo | typing-caret | **pass** | 4 ok, 0 failed | CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-input-bridging.md R2 |
| mojo | video-controls | **pass** | 6 ok, 0 failed | CHECK ok 18-region-equal: region 16,167-517,447: colours 2781 hash 6ad38d99 rows 280 uniform 0 light 435-439 blue none = the sample's hash | cavekit-gre-widgets.md R1 |
| mojo | webgl-cube | **pass** | 7 ok, 0 failed | WAIT ok 16-changed c | cavekit-gpu-rendering.md R8 |
| mojo | webone | **not run** | 0 ok, 0 failed | needs webone, keyboard-map (run with --with webone,keyboard-map) | cavekit-proxies-certs.md R3 |
| mojo | wikipedia-keyboard-zoom | **pass** | 10 ok, 0 failed | CHECK ok 20-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad] | cavekit-mochi-ui.md R6 |
| mojo | wikipedia-zoom (rerun) | **known** | 11 ok, 1 failed | the open defect T-567: CHECK FAIL 27-stream-uniform: uniform 8 (want at most 0) | cavekit-gpu-rendering.md R6 |
| mojo | xpi-confirm | **pass** | 8 ok, 0 failed | xpi: 649 bytes served as jihad-effect.xpi; frames: kept/mojo-xpi-confirm-xpi-prompt_.jpg, kept/mojo-xpi-confirm-extensions-pane_.jpg | cavekit-addons-extensions.md R3 |
| mojo | youtube-mobile | **pass** | 3 ok, 0 failed | WAIT ok 11-changed 0,130,756,560; frames: kept/mojo-youtube-mobile-playing_.jpg | cavekit-modern-web.md R2 |
| mojo | youtube-no-next | **pass** | 6 ok, 0 failed | CHECK ok 17-url: load done uri=https://m.youtube.com/watch?v=jNQXAC9IVRw | cavekit-device-headroom.md R4 |
| mojo | zoom-dtap-column | **pass** | 6 ok, 0 failed | CHECK ok 14-content: region 0,100-756,700: colours 325 hash 735e4dc6 rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-211-750,1982 | cavekit-input-bridging.md R5 |
| mojo | zoom-field-kept | **pass** | 6 ok, 0 failed | CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8) | cavekit-input-bridging.md R5 |
| mojo | zoom-locked-pinch-off | **pass** | 3 ok, 0 failed | CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05) | cavekit-input-bridging.md R5 |
| mojo | zoom-locked-pinch | **pass** | 4 ok, 0 failed | CHECK ok 13-zoom-min: zoom 1.9361 (at least 1.3) | cavekit-input-bridging.md R5 |
| mojo | zoom-new-page | **pass** | 8 ok, 0 failed | CHECK ok 17-zoom: zoom 0.6882 (want 0.69 within 0.06) | cavekit-input-bridging.md R3 |
| enyo | frame-field-hold (rerun) | **pass** | 14 ok, 0 failed | WAIT ok 35-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false; frames: kept/enyo-frame-field-hold-field-menu_.jpg | cavekit-ui-shell.md R7 |
| mochi | frame-field-hold (rerun) | **pass** | 14 ok, 0 failed | WAIT ok 35-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false; frames: kept/mochi-frame-field-hold-field-menu_.jpg | cavekit-ui-shell.md R7 |
| mojo | frame-field-hold (rerun) | **pass** | 14 ok, 0 failed | WAIT ok 35-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false; frames: kept/mojo-frame-field-hold-field-menu_.jpg | cavekit-ui-shell.md R7 |
| enyo | flash-app-calculator (rerun) | **pass** | 9 ok, 0 failed | CHECK ok 33-region-equal: region 165,181-397,237: colours 131 hash 175995c6 rows 56 uniform 0 light none blue 183-236 = the sample's hash; frames: kept/enyo-flash-app-calculator-keyed-15_.jpg, kept/enyo-flash-app-calculator-sum_.jpg | cavekit-flash.md R7 |
| enyo | flash-app-chart (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 17-text last = [click 4 May series 0]; frames: kept/enyo-flash-app-chart-chart_.jpg | cavekit-flash.md R7 |
| enyo | flash-app-clock (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 15-region-changed: region 0,83-300,381: colours 1345 hash 57ebf8d7 rows 298 uniform 41 light 215-216 blue 101-364 against the sample's hash d1d62938 | cavekit-flash.md R7 |
| enyo | flash-app-drums (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 22-flash-drawn: drawn: region 51,338-222,449: colours 73 hash 86a060fd rows 111 uniform 37 light 375-448 blue none; frames: kept/enyo-flash-app-drums-players_.jpg | cavekit-flash.md R7 |
| enyo | flash-app-form (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8097/title/website-viewer; frames: kept/enyo-flash-app-form-asked_.jpg | cavekit-flash.md R7 |
| enyo | flash-app-paint (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 22-region-equal: region 95,287-200,320: colours 5 hash ab012dda rows 33 uniform 21 light 287-319 blue none = the sample's hash; frames: kept/enyo-flash-app-paint-poster_.jpg, kept/enyo-flash-app-paint-painted_.jpg | cavekit-flash.md R7 |
| enyo | flash-app-phonemic (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 26-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 | cavekit-flash.md R7 |
| enyo | flash-app-piano (rerun) | **pass** | 7 ok, 0 failed | CHECK ok 19-sound: sound at the output sink: peak 12626 rms 1747 frames 45056 hz 492 env 0,0,12626,9057,3514,1911, the speaker amp on, the media volume 23; frames: kept/enyo-flash-app-piano-key_.jpg | cavekit-flash.md R7 |
| enyo | flash-app-postcards (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 26-flash-drawn: drawn: region 269,370-499,543: colours 309 hash 0536a9b6 rows 173 uniform 67 light 412-519 blue 413-458; frames: kept/enyo-flash-app-postcards-forward_.jpg | cavekit-flash.md R7 |
| enyo | flash-clipboard-out (rerun) | **pass** | 10 ok, 0 failed | CHECK ok 34-quiet: no line jihad-dl\/dialog  for 4 s; frames: kept/enyo-flash-clipboard-out-menu_.jpg | cavekit-flash.md R7 |
| mochi | flash-app-calculator (rerun) | **pass** | 9 ok, 0 failed | CHECK ok 33-region-equal: region 165,177-397,233: colours 128 hash 6a298bcf rows 56 uniform 0 light none blue 179-232 = the sample's hash; frames: kept/mochi-flash-app-calculator-keyed-15_.jpg, kept/mochi-flash-app-calculator-sum_.jpg | cavekit-flash.md R7 |
| mochi | flash-app-chart (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 17-text last = [click 4 May series 0]; frames: kept/mochi-flash-app-chart-chart_.jpg | cavekit-flash.md R7 |
| mochi | flash-app-clock (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 15-region-changed: region 0,79-300,377: colours 1344 hash 4f3a20c0 rows 298 uniform 25 light 211-212 blue 97-360 against the sample's hash 9b4d8301 | cavekit-flash.md R7 |
| mochi | flash-app-drums (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 22-flash-drawn: drawn: region 51,334-222,445: colours 114 hash 6df0297f rows 111 uniform 37 light 371-444 blue none; frames: kept/mochi-flash-app-drums-players_.jpg | cavekit-flash.md R7 |
| mochi | flash-app-form (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8097/title/website-viewer; frames: kept/mochi-flash-app-form-asked_.jpg | cavekit-flash.md R7 |
| mochi | flash-app-paint (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 22-region-equal: region 95,283-200,316: colours 11 hash c0bb1138 rows 33 uniform 21 light 283-315 blue none = the sample's hash; frames: kept/mochi-flash-app-paint-poster_.jpg, kept/mochi-flash-app-paint-painted_.jpg | cavekit-flash.md R7 |
| mochi | flash-app-phonemic (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 26-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 | cavekit-flash.md R7 |
| mochi | flash-app-piano (rerun) | **pass** | 7 ok, 0 failed | CHECK ok 19-sound: sound at the output sink: peak 12626 rms 1712 frames 45056 hz 514 env 0,0,6,12626,5300,2755, the speaker amp on, the media volume 23; frames: kept/mochi-flash-app-piano-key_.jpg | cavekit-flash.md R7 |
| mochi | flash-app-postcards (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 26-flash-drawn: drawn: region 269,366-499,539: colours 274 hash 02916fe5 rows 173 uniform 65 light 408-515 blue 409-454; frames: kept/mochi-flash-app-postcards-forward_.jpg | cavekit-flash.md R7 |
| mochi | flash-clipboard-out (rerun) | **pass** | 10 ok, 0 failed | CHECK ok 34-quiet: no line jihad-dl\/dialog  for 4 s; frames: kept/mochi-flash-clipboard-out-menu_.jpg | cavekit-flash.md R7 |
| mojo | flash-app-calculator (rerun) | **pass** | 9 ok, 0 failed | CHECK ok 33-region-equal: region 165,172-397,228: colours 115 hash 708ae6c9 rows 56 uniform 0 light none blue 174-227 = the sample's hash; frames: kept/mojo-flash-app-calculator-keyed-15_.jpg, kept/mojo-flash-app-calculator-sum_.jpg | cavekit-flash.md R7 |
| mojo | flash-app-chart (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 17-text last = [click 4 May series 0]; frames: kept/mojo-flash-app-chart-chart_.jpg | cavekit-flash.md R7 |
| mojo | flash-app-clock (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 15-region-changed: region 0,74-300,372: colours 1106 hash 84fe1e9b rows 298 uniform 41 light 206-207 blue 92-355 against the sample's hash 193b490a | cavekit-flash.md R7 |
| mojo | flash-app-drums (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 22-flash-drawn: drawn: region 51,329-222,440: colours 49 hash 65c12f83 rows 111 uniform 37 light 366-439 blue none; frames: kept/mojo-flash-app-drums-players_.jpg | cavekit-flash.md R7 |
| mojo | flash-app-form (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8097/title/website-viewer; frames: kept/mojo-flash-app-form-asked_.jpg | cavekit-flash.md R7 |
| mojo | flash-app-paint (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 22-region-equal: region 95,278-200,311: colours 5 hash ab012dda rows 33 uniform 21 light 278-310 blue none = the sample's hash; frames: kept/mojo-flash-app-paint-poster_.jpg, kept/mojo-flash-app-paint-painted_.jpg | cavekit-flash.md R7 |
| mojo | flash-app-phonemic (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 26-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 | cavekit-flash.md R7 |
| mojo | flash-app-piano (rerun) | **pass** | 7 ok, 0 failed | CHECK ok 19-sound: sound at the output sink: peak 12626 rms 1761 frames 45056 hz 479 env 0,0,12626,6643,3333,1371, the speaker amp on, the media volume 23; frames: kept/mojo-flash-app-piano-key_.jpg | cavekit-flash.md R7 |
| mojo | flash-app-postcards (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 26-flash-drawn: drawn: region 269,361-499,534: colours 256 hash 7d7798ab rows 173 uniform 67 light 403-510 blue none; frames: kept/mojo-flash-app-postcards-forward_.jpg | cavekit-flash.md R7 |
| mojo | flash-clipboard-out (rerun) | **pass** | 10 ok, 0 failed | CHECK ok 34-quiet: no line jihad-dl\/dialog  for 4 s; frames: kept/mojo-flash-clipboard-out-menu_.jpg | cavekit-flash.md R7 |
| enyo | flash-key-modifiers (rerun) | **pass** | 10 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R7 |
| mochi | flash-key-modifiers (rerun) | **pass** | 10 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R7 |
| mojo | flash-key-modifiers (rerun) | **pass** | 10 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R7 |
| enyo | keyboard-modifiers (rerun) | **pass** | 5 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R2 |
| mochi | keyboard-modifiers (rerun) | **pass** | 5 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R2 |
| mojo | keyboard-modifiers (rerun) | **pass** | 5 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R2 |
| enyo | audio-heard | **pass** | 6 ok, 0 failed | CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2951,2948,2956,2935,2950,2913, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-device-build.md R4 |
| enyo | autoplay-sound | **pass** | 3 ok, 0 failed | WAIT ok 13-text status = [playing]; frames: kept/enyo-autoplay-sound-refused_.jpg | cavekit-gre-widgets.md R1 |
| enyo | flash-app-mp3 | **pass** | 5 ok, 0 failed | CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 | cavekit-flash.md R7 |
| enyo | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/enyo-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| enyo | flash-lane (rerun) | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,284: 3.0 s, flips 41 (13.7/s), changes 0 (0.0/s), reads 43, distinct 1, spacing median 67 p90 162 max 222 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| enyo | flash-sound (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 16-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2937,2951,2956,2939,2950,2924, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-engine-update.md R8 |
| enyo | volume-keys (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 33-sound: sound at the output sink: peak 2956 rms 2059 frames 45056 hz 440 env 2938,2946,2956,2944,2950,2933, the speaker amp on, the media volume 22, the pitch wanted 440 Hz | cavekit-finger-testing.md R7 |
| mochi | audio-heard | **pass** | 6 ok, 0 failed | CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2951,2956,2939,2950,2955,2919, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-device-build.md R4 |
| mochi | flash-app-mp3 | **pass** | 5 ok, 0 failed | CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 | cavekit-flash.md R7 |
| mochi | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/mochi-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| mochi | flash-lane (rerun) | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,280: 3.0 s, flips 39 (13.0/s), changes 0 (0.0/s), reads 41, distinct 1, spacing median 68 p90 147 max 210 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| mochi | flash-sound (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 16-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2942,2951,2948,2956,2935,2950, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-engine-update.md R8 |
| mochi | volume-keys (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 33-sound: sound at the output sink: peak 2954 rms 2059 frames 45056 hz 440 env 2943,2950,2954,2936,2949,2949, the speaker amp on, the media volume 22, the pitch wanted 440 Hz | cavekit-finger-testing.md R7 |
| mojo | audio-heard | **pass** | 6 ok, 0 failed | CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2951,2956,2939,2950,2955,2919, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-device-build.md R4 |
| mojo | flash-app-mp3 | **pass** | 5 ok, 0 failed | CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 | cavekit-flash.md R7 |
| mojo | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/mojo-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| mojo | flash-lane (rerun) | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,29-440,327: 3.0 s, flips 40 (13.3/s), changes 0 (0.0/s), reads 41, distinct 1, spacing median 68 p90 150 max 176 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| mojo | flash-sound (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 16-sound: sound at the output sink: peak 2957 rms 2056 frames 45056 hz 440 env 2955,2938,2951,2957,2944,2935, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-engine-update.md R8 |
| mojo | volume-keys (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 33-sound: sound at the output sink: peak 2955 rms 2059 frames 45056 hz 440 env 2950,2948,2946,2955,2949,2935, the speaker amp on, the media volume 22, the pitch wanted 440 Hz | cavekit-finger-testing.md R7 |
| enyo | flash-video-controls (rerun) | **pass** | 18 ok, 0 failed | CHECK ok 35-plugin: plugin-container 7664: alive (state S, cpu ticks 2275 to 2286) | cavekit-flash.md R8 |
| enyo | flash-video-flowplayer (rerun) | **pass** | 9 ok, 0 failed | CHECK ok 18-plugin: plugin-container 12829: alive (state S, cpu ticks 814 to 828) | cavekit-flash.md R8 |
| enyo | flash-video-h264-240 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 16425: alive (state S, cpu ticks 1044 to 1056) | cavekit-flash.md R8 |
| enyo | flash-video-h264-360 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 19939: alive (state S, cpu ticks 953 to 965) | cavekit-flash.md R8 |
| enyo | flash-video-h264-480 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 23340: alive (state S, cpu ticks 1154 to 1166) | cavekit-flash.md R8 |
| enyo | flash-video-h264-mp4 (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 26932: alive (state S, cpu ticks 563 to 571) | cavekit-flash.md R8 |
| enyo | flash-video-mediaelement (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 17-plugin: plugin-container 30265: alive (state S, cpu ticks 617 to 625) | cavekit-flash.md R8 |
| enyo | flash-video-slow (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 27-plugin: plugin-container 1020: alive (state S, cpu ticks 707 to 717) | cavekit-flash.md R8 |
| enyo | flash-video-spark-240 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 5378: alive (state S, cpu ticks 585 to 597) | cavekit-flash.md R8 |
| enyo | flash-video-spark-360 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 8871: alive (state S, cpu ticks 681 to 691) | cavekit-flash.md R8 |
| enyo | flash-video-spark-480 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 12326: alive (state S, cpu ticks 821 to 833) | cavekit-flash.md R8 |
| enyo | flash-video-spark-sheep (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 15681: alive (state S, cpu ticks 325 to 336) | cavekit-flash.md R8 |
| enyo | flash-video-spark-small (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 19026: alive (state S, cpu ticks 442 to 453) | cavekit-flash.md R8 |
| enyo | flash-video-vp6-176 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 25-plugin: plugin-container 22818: alive (state S, cpu ticks 2829 to 2840) | cavekit-flash.md R8 |
| enyo | flash-video-vp6-272 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 25-plugin: plugin-container 27693: alive (state S, cpu ticks 2417 to 2429) | cavekit-flash.md R8 |
| enyo | flash-video-vp6a (rerun) | **pass** | 12 ok, 0 failed | CHECK ok 24-plugin: plugin-container 31973: alive (state S, cpu ticks 575 to 586) | cavekit-flash.md R8 |
| mochi | flash-video-controls (rerun) | **pass** | 18 ok, 0 failed | CHECK ok 35-plugin: plugin-container 14669: alive (state S, cpu ticks 2172 to 2184) | cavekit-flash.md R8 |
| mochi | flash-video-flowplayer (rerun) | **pass** | 9 ok, 0 failed | CHECK ok 18-plugin: plugin-container 19766: alive (state S, cpu ticks 762 to 775) | cavekit-flash.md R8 |
| mochi | flash-video-h264-240 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 23411: alive (state S, cpu ticks 1140 to 1151) | cavekit-flash.md R8 |
| mochi | flash-video-h264-360 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 26890: alive (state S, cpu ticks 1047 to 1059) | cavekit-flash.md R8 |
| mochi | flash-video-h264-480 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 30570: alive (state S, cpu ticks 1152 to 1164) | cavekit-flash.md R8 |
| mochi | flash-video-h264-mp4 (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 1740: alive (state S, cpu ticks 568 to 575) | cavekit-flash.md R8 |
| mochi | flash-video-mediaelement (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 17-plugin: plugin-container 4974: alive (state S, cpu ticks 624 to 632) | cavekit-flash.md R8 |
| mochi | flash-video-slow (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 27-plugin: plugin-container 8265: alive (state S, cpu ticks 712 to 724) | cavekit-flash.md R8 |
| mochi | flash-video-spark-240 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 12789: alive (state S, cpu ticks 591 to 602) | cavekit-flash.md R8 |
| mochi | flash-video-spark-360 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 16246: alive (state S, cpu ticks 695 to 707) | cavekit-flash.md R8 |
| mochi | flash-video-spark-480 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 19736: alive (state S, cpu ticks 818 to 829) | cavekit-flash.md R8 |
| mochi | flash-video-spark-sheep (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 23196: alive (state S, cpu ticks 333 to 344) | cavekit-flash.md R8 |
| mochi | flash-video-spark-small (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 26774: alive (state R, cpu ticks 453 to 463) | cavekit-flash.md R8 |
| mochi | flash-video-vp6-176 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 25-plugin: plugin-container 30795: alive (state S, cpu ticks 2667 to 2678) | cavekit-flash.md R8 |
| mochi | flash-video-vp6-272 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 25-plugin: plugin-container 3833: alive (state R, cpu ticks 2304 to 2315) | cavekit-flash.md R8 |
| mochi | flash-video-vp6a (rerun) | **pass** | 12 ok, 0 failed | CHECK ok 24-plugin: plugin-container 8220: alive (state S, cpu ticks 586 to 597) | cavekit-flash.md R8 |
| mojo | flash-video-controls (rerun) | **pass** | 18 ok, 0 failed | CHECK ok 35-plugin: plugin-container 23063: alive (state S, cpu ticks 2090 to 2106) | cavekit-flash.md R8 |
| mojo | flash-video-flowplayer (rerun) | **pass** | 9 ok, 0 failed | CHECK ok 18-plugin: plugin-container 28336: alive (state S, cpu ticks 804 to 816) | cavekit-flash.md R8 |
| mojo | flash-video-h264-240 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 31978: alive (state S, cpu ticks 936 to 946) | cavekit-flash.md R8 |
| mojo | flash-video-h264-360 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 3179: alive (state S, cpu ticks 815 to 826) | cavekit-flash.md R8 |
| mojo | flash-video-h264-480 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 6683: alive (state S, cpu ticks 1020 to 1031) | cavekit-flash.md R8 |
| mojo | flash-video-h264-mp4 (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 10288: alive (state S, cpu ticks 428 to 436) | cavekit-flash.md R8 |
| mojo | flash-video-mediaelement (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 17-plugin: plugin-container 13521: alive (state S, cpu ticks 451 to 458) | cavekit-flash.md R8 |
| mojo | flash-video-slow (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 27-plugin: plugin-container 16770: alive (state S, cpu ticks 651 to 663) | cavekit-flash.md R8 |
| mojo | flash-video-spark-240 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 20901: alive (state S, cpu ticks 539 to 551) | cavekit-flash.md R8 |
| mojo | flash-video-spark-360 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 24358: alive (state S, cpu ticks 643 to 655) | cavekit-flash.md R8 |
| mojo | flash-video-spark-480 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 27897: alive (state S, cpu ticks 758 to 770) | cavekit-flash.md R8 |
| mojo | flash-video-spark-sheep (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 31332: alive (state R, cpu ticks 324 to 335) | cavekit-flash.md R8 |
| mojo | flash-video-spark-small (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 2536: alive (state S, cpu ticks 427 to 438) | cavekit-flash.md R8 |
| mojo | flash-video-vp6-176 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 25-plugin: plugin-container 6256: alive (state S, cpu ticks 2489 to 2500) | cavekit-flash.md R8 |
| mojo | flash-video-vp6-272 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 25-plugin: plugin-container 11137: alive (state S, cpu ticks 2158 to 2170) | cavekit-flash.md R8 |
| mojo | flash-video-vp6a (rerun) | **pass** | 12 ok, 0 failed | CHECK ok 24-plugin: plugin-container 15403: alive (state S, cpu ticks 570 to 581) | cavekit-flash.md R8 |

## The device at the end

```
(not checked)
```

## Every test's lines

### enyo / addon-site — pass

    enyo card on https://addons.riverstonerelay.org/: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPO
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

    enyo card on about:addons: connected at +2 s, load done at +8 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 258 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='pad01@jihad.test']
    WAIT ok 13-rect anon:richlistitem[value='pad01@jihad.test']|disable-btn
    WAIT ok 15-rect anon:richlistitem[value='pad01@jihad.test']|enable-btn
    WAIT ok 17-rect anon:richlistitem[value='pad01@jihad.test']|undo-btn
    WAIT ok 19-rect anon:richlistitem[value='pad01@jihad.test']|remove-btn

### enyo / addons-list-pan — pass

    enyo card on about:addons: connected at +2 s, load done at +8 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    enyo card on about:addons: connected at +2 s, load done at +8 s, 7 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-changed 200,88,470,135
    WAIT ok 14-still 200,88,470,135
    CHECK ok 16-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / appmenu-find — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 10-differs 0,30,300,180
    WAIT ok 11-still 0,30,300,180
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 16-log: [jihad-bs] find 'zebrafind' -> 1

### enyo / appmenu-print — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 11-differs 0,30,300,180
    WAIT ok 12-still 0,30,300,180
    WAIT ok 14-line: [jihad-bs] luna: findPrinters (started)
    WAIT ok 15-differs 0,200,756,960
    WAIT ok 16-log: [jihad-bs] js-console (no source):0:0 print: mdns: EPSON ET-2720 Series 2 @ macmini._ipp._tcp.local -> 192.168.1.162:631 (answer from 192.168.1.162)
    WAIT ok 17-still 0,200,756,960

### enyo / appmenu-settings-keyboard-down — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    WAIT ok 15-load
    CHECK ok 16-url: load done uri=about:settings#chrome=%7B%22liteMigrated%22%3Atrue%2C%22homeLiteMigrated%22%3Atrue%2C%22searchEngine%22%3A%22google%22%2C%22homeUrl%22%3A%22https%3A%2F%2Flite.duckduckgo.com%2Flite%2F%22%2C%22startLinks%22%3A%5B%7B%22title%22%3A%22DuckDuckGo%22%2C%22url%22%3A%22https%3A%2F%2Fduckduckgo.com%2F%22%7D%
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 18-rect page-title
    CHECK ok 19-content: region 0,100-768,700: colours 499 hash 333a5bcc rows 600 uniform 0 light none blue none; page-title on the screen at 20,100-744,134, text holds [Settings]

### enyo / autoplay-unmute — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&unmute=30000: connected at +2 s, load done at +8 s, 17 content frames at +8 s; the GPU path, frames off the main t
    WAIT ok 8-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 10-text status = [playing]
    WAIT ok 11-text status = [paused t=29.7]

### enyo / busy-script-stop — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 9-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 13-rect sel:h1
    CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36

### enyo / card-dialog — pass

    enyo card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect c
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 142 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 13-text r = [c: true]
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-2.fifo
    CHECK ok 17-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 18-text r = [c: false]

### enyo / card-select — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
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

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +3 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### enyo / chrome-padlock — pass

    enyo card on https://example.com/: connected at +2 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:p
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 385 landed on document y 303)
    WAIT ok 12-line: [jihad-bs] luna: pageCertificate (sent)
    WAIT ok 13-differs 0,100,756,960

### enyo / chrome-share — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +3 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### enyo / codecs-play — pass

    enyo card on http://192.168.1.245:8077/jihad-codecs.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 11-text state = [DONE 11 rows]
    WAIT ok 12-text r0 = [vp8 canPlay=probably result=ended t=10.00/10.00 err=0 frames=149/0 vw=320x240]
    WAIT ok 13-text r0 = [vp8 canPlay=probably result=ended t=10.00/10.00 err=0 frames=149/0 vw=320x240]
    WAIT ok 14-text r1 = [vp9 canPlay=probably result=ended t=10.00/10.00 err=0 frames=149/0 vw=320x240]
    WAIT ok 15-text r1 = [vp9 canPlay=probably result=ended t=10.00/10.00 err=0 frames=149/0 vw=320x240]
    WAIT ok 16-text r8 = [flac canPlay=maybe result=ended t=10.00/10.00 err=0]
    WAIT ok 17-text r8 = [flac canPlay=maybe result=ended t=10.00/10.00 err=0]
    WAIT ok 18-text r9 = [mp3 canPlay=maybe result=ended t=10.06/10.06 err=0]
    WAIT ok 19-text r9 = [mp3 canPlay=maybe result=ended t=10.06/10.06 err=0]
    WAIT ok 20-text r10 = [aac canPlay=probably result=ended t=10.03/10.03 err=0]
    WAIT ok 21-text r10 = [aac canPlay=probably result=ended t=10.03/10.03 err=0]

### enyo / ddg-results — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 8-launch: load done
    WAIT ok 9-line: [jihad-bs] titleAndUrl title=[webOS at DuckDuckGo] uri=https://duckduckgo.com/?q=webOS&ia=web back=1 fwd=0
    CHECK ok 10-content: region 0,100-756,900: colours 963 hash 5895ba32 rows 800 uniform 195 light 100-899 blue none; sel:body on the screen at 0,82-768,2010

### enyo / dialog-slow-answer — pass

    enyo card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect c
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 142 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text r = [c: true]

### enyo / download-finishes — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-38

### enyo / download-list — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-39
    CHECK ok 10-file: /media/internal/downloads/jihad-download-39 (17 bytes) holds [Hello from Jihad]
    WAIT ok 12-differs 0,100,756,960
    WAIT ok 14-still 0,100,756,960
    WAIT ok 17-differs 0,100,756,960
    WAIT ok 18-still 0,100,756,960
    WAIT ok 22-differs 450,80,768,400
    WAIT ok 23-still 450,80,768,400
    WAIT ok 24-content 460,90,760,300

### enyo / drag-and-pinch-page — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 9-card-scroll-min: the card's scroll y 650 (at least 400)
    WAIT ok 11-zoom
    CHECK ok 12-zoom-min: zoom 2.6506 (at least 1.5)
    CHECK ok 13-content: region 0,150-756,600: colours 714 hash 9c7c6a79 rows 450 uniform 201 light 150-599 blue none; sel:body on the screen at 0,-564-2025,53741
    WAIT ok 15-zoom
    WAIT ok 17-zoom
    CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15)

### enyo / etsy-search — pass

    enyo card on https://www.etsy.com/search?q=webos: connected at +2 s, load done at +58 s, 27 content frames at +58 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_CO
    WAIT ok 8-rect sel:body
    WAIT ok 9-url: https://www.etsy.com/search?q=webos
    CHECK ok 10-url: load done uri=https://www.etsy.com/search?q=webos
    WAIT ok 11-content 0,100,768,900
    CHECK ok 12-content: region 0,100-768,900: colours 4096 hash 105343fb rows 800 uniform 119 light 100-899 blue none; sel:body on the screen at 0,82-768,8849

### enyo / everyday-browsing — pass

    enyo card on https://duckduckgo.com/: connected at +2 s, load done at +30 s, 31 content frames at +30 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:textarea[name='q']
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 440 landed on document y 358)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-launch: load done
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 158 landed on document y 110)
    CHECK ok 15-card-scroll-min: the card's scroll y 770 (at least 300)
    WAIT ok 17-url: https://duckduckgo.com/

### enyo / express-burst-wiki — pass

    enyo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +3 s, load done at +16 s, 10 content frames at +16 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 8-rect sel:#firstHeading
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 158 landed on document y 110)
    WAIT ok 14-zoom
    WAIT ok 15-settled: the card's scroll 532 1106
    CHECK ok 16-zoom-min: zoom 2.3273 (at least 1.0)
    CHECK ok 17-content: region 0,100-756,700: colours 1097 hash 7b6c2598 rows 600 uniform 0 light 101-699 blue 100-584; sel:body on the screen at -1237,-2489-1359,708

### enyo / flash-api — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +2 s, load done at +8 s, 6 content frames at +8 s; the GPU path, f
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

### enyo / flash-fullscreen — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&vars=fs=1: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main 
    WAIT ok 8-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 9-text kid = [kid w=376]
    WAIT ok 11-text fs = [fs fullScreen]
    WAIT ok 14-text fs = [fs normal]
    WAIT ok 17-text fs = [fs fullScreen]
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 20-flash-behind: the colour 00ff00 in 180 of 192 cells (wanted yes)

### enyo / flash-input-cover — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +2 s, load done at +8 s, 13 content frames at +8 s; the GPU path, frames off the main thread (the expr
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 229 px up for the keyboard, the page at 229
    WAIT ok 14-settled: the card's scroll 0 229
    CHECK ok 16-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### enyo / flash-input-hold — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +9 s, 8 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-text up = [up 201 150]
    CHECK ok 10-number: heldms = 2374 (wanted 1500..3200)
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-number: n-up = 1 (wanted 1..1)
    WAIT ok 15-text up = [up 361 278]
    CHECK ok 16-flash-el: the movie's move at 355,274 for 360,277 (within 30)
    CHECK ok 17-press-rate: 26.5 moves a second sent on (at least 20): ] a press of 1441 ms: 32 moves from the card in 1207 ms, 32 sent on, 232 ms in sending them

### enyo / flash-input-kbdrag — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 16-settled: the card's scroll 0 0
    CHECK ok 17-number: n-move = 13 (wanted 5..400)
    WAIT ok 20-settled: the card's scroll 0 0
    CHECK ok 21-number: n-move = 21 (wanted 14..800)
    CHECK ok 23-flash-at: the movie reported down 301,100 for stage 300,100 (within 8)

### enyo / flash-input-kernel — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express l
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

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +8 s, 11 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 14-text keyup = [keyup 65 97]
    CHECK ok 15-number: keyheldms = 180 (wanted 60..2000)
    WAIT ok 17-text keyup = [keyup 39 0]
    CHECK ok 18-number: keyheldms = 74 (wanted 60..2000)
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / flash-input-keys — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +8 s, 11 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (
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

### enyo / flash-input-lock — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express l
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
    CHECK ok 22-number: n-move = 8 (wanted 5..400)
    CHECK ok 24-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 27-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 30-settled: the card's scroll 0 100
    CHECK ok 31-card-scroll-min: the card's scroll y 100 (at least 30)

### enyo / flash-input-tap — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
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

### enyo / flash-layout — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=percent: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the e
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

### enyo / flash-tags — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
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
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 55 (18.3/s), changes 32 (10.7/s), reads 57, distinct 33, spacing median 44 p90 88 max 246 ms (wanted 5..100/s)

### enyo / flash-text — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text: connected at +2 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 15-text text = [text Hello]
    WAIT ok 17-text text = [text Hello World 123]
    WAIT ok 19-text text = [text Hello World x9]
    CHECK ok 20-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / flash-wmode — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm1&bg=00ff00&wmode=window: connected at +2 s, load done at +8 s, 12 content frames at +8 s; the GPU path, frames off the main thre
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

    enyo card on about:settings: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    WAIT ok 11-rect fonts-reset
    WAIT ok 13-pref font.name.serif.x-western = [Dotum]
    WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.]

### enyo / forget-identities — pass

    enyo card on about:settings: connected at +2 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.]

### enyo / frame-field — pass

    enyo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
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

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / keyboard-arbitration — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +8 s, 9 content frames at +8 s; the GPU path, frames off the main thread (the express l
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    WAIT ok 14-text keydown = [keydown 65 97]
    CHECK ok 15-region-equal: region 150,30-620,78: colours 165 hash e80d768f rows 48 uniform 0 light 55-58 blue none = the sample's hash
    CHECK ok 19-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 22-settled: the card's scroll 0 0
    CHECK ok 23-region-changed: region 150,30-620,78: colours 73 hash 9abd95e1 rows 48 uniform 0 light none blue none against the sample's hash e80d768f
    CHECK ok 25-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / keyboard-fields — pass

    enyo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
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

### enyo / long-press-image — pass

    enyo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +2 s, load done at +17 s, 9 content frames at +17 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 8-rect sel:.infobox img
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 158 landed on document y 110)
    WAIT ok 12-differs 0,100,756,900

### enyo / long-press-menu — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### enyo / long-press-text-field — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 10-differs 0,100,756,600
    WAIT ok 13-still 0,100,756,600
    WAIT ok 15-line: [jihad-bs] hold at 269,219 editable=1 wasFocused=0
    WAIT ok 18-still 0,100,400,240
    WAIT ok 20-load
    CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### enyo / media-controls — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +2 s, load done at +8 s, 21 content frames at +8 s; the GPU path, frames off the main thread (the ex
    WAIT ok 9-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 11-text status = [playing]
    WAIT ok 13-text status = [paused t=14.0]
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
    WAIT ok 14-text status = [paused t=14.4]
    WAIT ok 15-rect anon:#v|controlBar
    WAIT ok 16-rect anon:#v|volumeControl
    WAIT ok 20-text vol = [volume=0.16 muted=false]
    WAIT ok 24-settled: the card's scroll 0 0
    WAIT ok 25-text seek = [60]

### enyo / mobile-wiki-pinch — pass

    enyo card on https://en.m.wikipedia.org/wiki/HP_TouchPad: connected at +2 s, load done at +22 s, 13 content frames at +22 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 8-rect sel:#firstHeading
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 158 landed on document y 110)
    WAIT ok 11-still 0,100,756,700
    CHECK ok 12-zoom-min: zoom 0.6882 (at least 0.65)
    CHECK ok 13-content: region 0,100-756,700: colours 4096 hash 5f7c410b rows 600 uniform 69 light 100-699 blue 236-240; sel:body on the screen at 0,82-768,1028
    CHECK ok 14-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    WAIT ok 16-zoom
    CHECK ok 17-zoom-min: zoom 1.8241 (at least 1.05)
    CHECK ok 18-content: region 0,100-756,700: colours 4096 hash dd9a5fda rows 600 uniform 8 light 100-699 blue none; sel:body on the screen at -634,-441-1401,2065

### enyo / pan-long-drag — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 8-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 9-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 10-content: region 0,150-768,400: colours 412 hash 072db6f4 rows 250 uniform 0 light 150-396 blue none; sel:#content p on the screen at 16,154-748,210, text holds [quick brown]
    CHECK ok 14-card-scroll: the card's scroll y 650 (want 650 within 100); the engine's 650
    WAIT ok 15-still header
    CHECK ok 16-region-equal: region 0,83-756,137: colours 208 hash 34266017 rows 54 uniform 33 light none blue none = the sample's hash
    CHECK ok 17-region-changed: region 0,300-756,900: colours 680 hash 52c6519f rows 600 uniform 219 light 300-899 blue none against the sample's hash 38d1cd3c
    CHECK ok 18-top: the page's top measured 82 (the tap at y 212 landed on document y 780), the model says 82 (header hidden: 1)
    CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### enyo / pan-long-flick — known

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 21-card-scroll-min: the card's scroll y 3471 (at least 300)
    STREAM fs-enyo-pan-long-flick.fbs: 163 samples (14.7/s asked 15, read median 2.24 ms max 30.36 ms, mmap stride 8, re-read 9, torn 0), 140 flips, cpu 7.7% of a core, polls 672/s | shown frames while moving: 137 gaps, median 27 ms, p90 42 ms, max 276 ms | shown scroll: 1703 rows over the stream (0 samples with the shift unknown); stale bands 2, uniform bands 46, still rows moved 0; the columns outside 0-756 changed in 51 samples | tracking (finger down, 22 samples): error median 29.0 px, max 188 px; lag median 179.5 ms, max 871 ms |   gesture at 1794842076327: the finger travelled 600 rows, the page sh
    CHECK FAIL 16-stream-stale: stale 2 (want at most 0)
    CHECK FAIL 17-stream-uniform: uniform 46 (want at most 0)
    CHECK ok 19-stream-gap-median: gap-median 27 (at most 34)
    CHECK ok 20-stream-gap-p90: gap-p90 42 (at most 50)

### enyo / pan-speeds — known

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 13-card-scroll-min: the card's scroll y 650 (at least 500)
    CHECK ok 25-card-scroll-min: the card's scroll y 4662 (at least 2500)
    STREAM fs-enyo-pan-speeds.fbs: 364 samples (14.7/s asked 15, read median 2.31 ms max 62.11 ms, mmap stride 8, re-read 25, torn 0), 359 flips, cpu 7.0% of a core, polls 674/s | shown frames while moving: 353 gaps, median 25 ms, p90 40 ms, max 294 ms | shown scroll: 3501 rows over the stream (1 samples with the shift unknown); stale bands 1, uniform bands 5, still rows moved 0; the columns outside 0-756 changed in 125 samples | tracking (finger down, 122 samples): error median -13.0 px, max 188 px; lag median 70.0 ms, max 871 ms |   gesture at 1794842137052: the finger travelled 650 rows, the page show
    CHECK FAIL 19-stream-stale: stale 1 (want at most 0)
    CHECK FAIL 20-stream-uniform: uniform 5 (want at most 0)
    CHECK ok 21-stream-error-max: error-max 188 (at most 500)
    CHECK ok 23-stream-gap-median: gap-median 25 (at most 34)
    CHECK ok 24-stream-gap-p90: gap-p90 40 (at most 50)

### enyo / persona-cycle — pass

    enyo card on about:personas: connected at +2 s, load done at +9 s, 8 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    enyo card on about:personas: connected at +2 s, load done at +8 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#online .card button
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 280 landed on document y 198)
    CHECK ok 11-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text status-text = [Wearing Autumn Forest.]
    WAIT ok 15-pref lightweightThemes.isThemeSelected = [true]
    WAIT ok 17-text status-text = [Undone.]
    WAIT ok 18-pref lightweightThemes.isThemeSelected = [false]

### enyo / personas-flick — pass

    enyo card on about:personas: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#mine .card
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 280 landed on document y 198)
    CHECK ok 10-card-scroll-min: the card's scroll y 1644 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### enyo / personas-page-steady — pass

    enyo card on about:personas: connected at +3 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-rect sel:#mine .card
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 280 landed on document y 198)
    CHECK ok 19-content: region 0,100-768,900: colours 4096 hash 234685cc rows 800 uniform 0 light 100-899 blue 276-750; sel:body on the screen at 0,-417-764,2152
    STREAM fs-enyo-personas-page-steady.fbs: 103 samples (15.0/s asked 15, read median 2.4 ms max 24.31 ms, mmap stride 8, re-read 8, torn 0), 147 flips, cpu 6.6% of a core, polls 636/s | shown frames while moving: 146 gaps, median 24.0 ms, p90 39 ms, max 53 ms | shown scroll: 501 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 0-756 changed in 52 samples | tracking (finger down, 52 samples): error median -8.0 px, max 24 px; lag median 67.0 ms, max 165 ms |   gesture at 1794842211038: the finger travelled 500 rows, the page 
    CHECK ok 15-stream-uniform: uniform 0 (at most 0)
    CHECK ok 17-stream-gap-median: gap-median 24 (at most 34)
    CHECK ok 18-stream-gap-p90: gap-p90 39 (at most 50)

### enyo / proxy-login — pass

    enyo card on about:settings: connected at +2 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    enyo card on about:settings: connected at +2 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    enyo card on about:settings: connected at +2 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 11-rect sel:.row[data-pref='network.proxy.type'] select
    WAIT ok 13-rect sel:.row[data-pref='network.proxy.http'] input
    WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input

### enyo / relay-choice — pass

    enyo card on about:settings: connected at +3 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 12-rect sel:.row[data-pref='network.proxy.type'] select
    WAIT ok 14-pref network.proxy.http = [proxy.riverstonerelay.org]
    WAIT ok 15-launch: load done
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 385 landed on document y 303)
    CHECK ok 17-text-has: sel:p = [This domain is for use in documentation examples without needing permission. This is not a service; avoid relying on it for testing and monitoring purposes.] holds [documentation]

### enyo / rotate — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 13-line: [jihad-bs] emitGeometry contentSize=1024x20488 win=1024x686 mZoom=1.3333
    WAIT ok 14-content 0,150,756,900
    WAIT ok 17-line: [jihad-bs] emitGeometry contentSize=768x20488 win=768x686 mZoom=0.7500
    WAIT ok 18-content 0,150,756,900
    WAIT ok 20-line: [jihad-bs] emitGeometry contentSize=1024x20488 win=1024x686 mZoom=1.3333
    WAIT ok 21-content 0,150,756,900
    WAIT ok 24-line: [jihad-bs] emitGeometry contentSize=768x20488 win=768x686 mZoom=0.7500
    WAIT ok 25-settled: the card's scroll 0 0
    CHECK ok 26-content: region 0,150-756,600: colours 421 hash 727e4039 rows 450 uniform 173 light 150-599 blue none; sel:body on the screen at 0,82-764,20570
    CHECK ok 28-card-scroll-min: the card's scroll y 650 (at least 400)

### enyo / screen-lock — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +2 s, load done at +7 s, 12 content frames at +7 s; the GPU path, frames off the main thread (the ex
    WAIT ok 9-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 11-text status = [playing]
    WAIT ok 20-text status = [playing]
    CHECK ok 22-text: status = [playing]
    WAIT ok 34-text status = [paused t=37.9]

### enyo / script-hold-zoom — pass

    enyo card on http://192.168.1.245:8077/jihad-script-hold.html: connected at +2 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 110 landed on document y 28)
    WAIT ok 9-zoom
    CHECK ok 10-zoom-min: zoom 1.9326 (at least 1.3)
    CHECK ok 12-card-scroll-min: the card's scroll y 452 (at least 200)
    WAIT ok 14-zoom
    WAIT ok 16-zoom
    CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2)

### enyo / settings-every-control — by a tool

    tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt)

### enyo / settings-field-saves — pass

    enyo card on about:settings: connected at +2 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    enyo card on about:settings: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    CHECK ok 10-card-scroll-min: the card's scroll y 720 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### enyo / settings-int-emptied — pass

    enyo card on about:settings: connected at +3 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 16-pref browser.sessionhistory.max_entries = [20]

### enyo / settings-persist — pass

    enyo card on about:settings: connected at +3 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-pref general.smoothScroll = [true]
    WAIT ok 14-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 16-pref general.smoothScroll = [true]

### enyo / settings-tabs — pass

    enyo card on about:settings: connected at +2 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    CHECK ok 9-text: page-title = [Settings]
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content]

### enyo / settings-typing — pass

    enyo card on about:settings: connected at +2 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 15-pref browser.display.background_color = [#FFFFFFx]
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / ssl-confirm — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog ssl-confirm -> cancel (answer=0)

### enyo / tabbed-browsing — not run

    not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word)

### enyo / touchpad-go — not run

    owner-accepted without hardware (device-build R6)

### enyo / typing-caret — pass

    enyo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 301 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 11-text echo = [plain n=6 c=0 last=[abcdef] value=[abcdef] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 13-text echo = [plain n=12 c=0 last=[] value=[] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / video-controls — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html: connected at +2 s, load done at +8 s, 19 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect status
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 12-changed sel:video
    WAIT ok 13-changed sel:video
    WAIT ok 15-still sel:video
    WAIT ok 17-still sel:video
    CHECK ok 18-region-equal: region 16,176-517,456: colours 2439 hash e970f523 rows 280 uniform 0 light 444-448 blue none = the sample's hash

### enyo / webgl-cube — pass

    enyo card on http://192.168.1.245:8077/jihad-webgl-cube.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 7-rect c
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 124 landed on document y 57)
    CHECK ok 9-text-has: out = [webgl fps=25.6 shaders=ok texture=ok framebuffer=ok readPixels=ok maxtex=4096 ext=14 missing=ANGLE_instanced_arrays,WEBGL_compressed_texture_s3tc,EXT_blend_minmax,EXT_shader_texture_lod depthrb=ok renderer=Mozilla vendor=Mozilla] holds [webgl]
    CHECK ok 10-content: region 105,175-457,524: colours 851 hash 775abca3 rows 349 uniform 150 light none blue 242-433; c on the screen at 105,174-457,525
    WAIT ok 11-changed c
    WAIT ok 13-zoom
    CHECK ok 15-content: region 0,100-768,900: colours 1350 hash 33ebda51 rows 800 uniform 0 light none blue none; c on the screen at -52,84-499,636
    WAIT ok 16-changed c

### enyo / webone — not run

    needs webone, keyboard-map (run with --with webone,keyboard-map)

### enyo / wikipedia-keyboard-zoom — pass

    enyo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +3 s, load done at +16 s, 7 content frames at +16 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
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

### enyo / wikipedia-zoom — known

    enyo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +2 s, load done at +15 s, 12 content frames at +15 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 11-rect sel:.infobox
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 158 landed on document y 110)
    WAIT ok 15-zoom
    CHECK ok 16-zoom-min: zoom 2.8444 (at least 1.3)
    WAIT ok 18-zoom
    WAIT ok 20-zoom
    WAIT ok 22-still 0,100,756,700
    CHECK ok 24-zoom: zoom 0.6882 (want 0.69 within 0.12)
    CHECK ok 25-content: region 0,100-756,700: colours 4096 hash 33ba05dc rows 600 uniform 62 light 100-699 blue 236-240; sel:body on the screen at 0,82-768,1028
    CHECK ok 26-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    STREAM fs-enyo-wikipedia-zoom.fbs: 235 samples (14.9/s asked 15, read median 1.73 ms max 62.19 ms, mmap stride 8, re-read 7, torn 0), 164 flips, cpu 6.3% of a core, polls 725/s | shown frames while moving: 156 gaps, median 37.0 ms, p90 60 ms, max 240 ms | shown scroll: 80 rows over the stream (44 samples with the shift unknown); stale bands 0, uniform bands 13, still rows moved 0; the columns outside 170-600 changed in 20 samples | tracking (finger down, 38 samples): error median 0.0 px, max 0 px; lag median 364.5 ms, max 817 ms |   gesture at 1794842423113: the finger travelled 0 rows, the page show
    CHECK FAIL 27-stream-uniform: uniform 13 (want at most 0)
    CHECK ok 29-stream-gap-median: gap-median 37 (at most 45)
    CHECK ok 30-stream-gap-p90: gap-p90 60 (at most 80)

### enyo / xpi-confirm — pass

    enyo card on http://192.168.1.245:8077/jihad-xpi-trigger.html: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 8-rect effect
    calibrated (enyo): the page's top on the screen 83 (the table said 82, the tap at y 129 landed on document y 59)
    CHECK ok 11-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 14-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 15-text r = [r: callback status=0 Jihad Effect Test]
    WAIT ok 16-launch: load done
    WAIT ok 17-rect sel:#category-extension
    WAIT ok 19-rect sel:#category-extension[selected='true']
    WAIT ok 21-rect sel:richlistitem[value='jihad-effect@riverstonerelay.net']

### enyo / youtube-feed-no-preview — pass

    enyo card on https://m.youtube.com/: connected at +2 s, load done at +13 s, 3 content frames at +13 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    CHECK ok 10-quiet: no line play() allowed for 30 s
    CHECK ok 12-probe-has: the probe's answer holds [playing 0]
    WAIT ok 14-settled: the card's scroll 0 0
    CHECK ok 15-quiet: no line play() allowed for 10 s
    CHECK ok 17-probe-has: the probe's answer holds [playing 0]

### enyo / youtube-mobile — pass

    enyo card on https://m.youtube.com/watch?v=dQw4w9WgXcQ: connected at +3 s, load done at +18 s, 9 content frames at +18 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 8-rect sel:video
    WAIT ok 10-changed 0,130,756,560
    WAIT ok 11-changed 0,130,756,560

### enyo / youtube-no-next — pass

    enyo card on https://m.youtube.com/watch?v=jNQXAC9IVRw: connected at +1 s, load done at +17 s, 12 content frames at +17 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 9-rect sel:video
    WAIT ok 11-line: [jihad-bs] js-console (no source):0:0 compat: youtube [m.youtube.com] play() allowed: 1130 ms after a click
    CHECK ok 13-probe-has: the probe's answer holds ["paused":false]
    CHECK ok 14-quiet: no line play() allowed for 70 s
    CHECK ok 16-probe-has: the probe's answer holds ["paused":true]
    CHECK ok 17-url: load done uri=https://m.youtube.com/watch?v=jNQXAC9IVRw&pp=0gcJCVgCo7VqN5tD

### enyo / zoom-dtap-column — pass

    enyo card on http://192.168.1.245:8077/jihad-dtap.html: connected at +2 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 7-rect sel:#col
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 114 landed on document y 50)
    WAIT ok 10-zoom
    WAIT ok 11-settled: the card's scroll 590 272
    CHECK ok 12-zoom-min: zoom 1.8286 (at least 1.5)
    CHECK ok 13-element-at: sel:#col on the screen at 18,-193-750,2000 holds 384,420
    CHECK ok 14-content: region 0,100-756,700: colours 438 hash 2976668a rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-193-750,2000

### enyo / zoom-field-kept — pass

    enyo card on http://192.168.1.245:8077/jihad-dtap.html: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 7-rect sel:#f
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 114 landed on document y 50)
    CHECK ok 9-zoom-max: zoom 0.6400 (at most 0.8)
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-settled: the card's scroll 0 430
    CHECK ok 14-above-keyboard: sel:#f on the screen at 64,639-324,668, above the keyboard
    CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8)

### enyo / zoom-locked-pinch-off — pass

    enyo card on http://192.168.1.245:8077/jihad-locked.html: connected at +3 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 8-rect sel:h1
    WAIT ok 12-settled: the card's scroll 0 0
    CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05)

### enyo / zoom-locked-pinch — pass

    enyo card on http://192.168.1.245:8077/jihad-locked.html: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 7-rect sel:h1
    WAIT ok 11-zoom
    WAIT ok 12-settled: the card's scroll 187 203
    CHECK ok 13-zoom-min: zoom 1.9484 (at least 1.3)

### enyo / zoom-new-page — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 10-zoom
    CHECK ok 11-zoom-min: zoom 1.9139 (at least 1.5)
    WAIT ok 12-launch: load done
    WAIT ok 13-still 0,100,756,600
    CHECK ok 14-zoom: zoom 1.0000 (want 1.0 within 0.05)
    WAIT ok 15-launch: load done
    WAIT ok 16-still 0,100,756,600
    CHECK ok 17-zoom: zoom 0.6882 (want 0.69 within 0.06)

### mochi / addon-site — pass

    mochi card on https://addons.riverstonerelay.org/: connected at +3 s, load done at +14 s, 9 content frames at +14 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_CO
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

    mochi card on about:addons: connected at +2 s, load done at +10 s, 3 content frames at +11 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 254 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='pad01@jihad.test']
    WAIT ok 13-rect anon:richlistitem[value='pad01@jihad.test']|disable-btn
    WAIT ok 15-rect anon:richlistitem[value='pad01@jihad.test']|enable-btn
    WAIT ok 17-rect anon:richlistitem[value='pad01@jihad.test']|undo-btn
    WAIT ok 19-rect anon:richlistitem[value='pad01@jihad.test']|remove-btn

### mochi / addons-list-pan — pass

    mochi card on about:addons: connected at +1 s, load done at +9 s, 5 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 254 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:richlistitem[value='pad12@jihad.test']
    WAIT ok 15-still 0,100,756,1000
    WAIT ok 17-rect sel:richlistitem[value='pad12@jihad.test'][selected='true']
    CHECK ok 18-hit: clickAt 843,1439 inside sel:richlistitem[value="pad12@jihad.test"] at 294,1412 656x99

### mochi / addons-page — pass

    mochi card on about:addons: connected at +1 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#category-extension
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 254 landed on document y 224)
    WAIT ok 9-rect sel:#category-extension[selected='true']
    WAIT ok 10-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:#category-plugin[selected='true']
    WAIT ok 14-rect sel:#category-extension[selected='true']
    WAIT ok 16-rect sel:#detail-view:not([hidden])
    CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill]

### mochi / addons-search-hide-key — pass

    mochi card on about:addons: connected at +1 s, load done at +7 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-changed 200,88,470,135
    WAIT ok 14-still 200,88,470,135
    CHECK ok 16-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / appmenu-find — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 10-differs 0,30,300,180
    WAIT ok 11-still 0,30,300,180
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 16-log: [jihad-bs] find 'zebrafind' -> 1

### mochi / appmenu-print — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 11-differs 0,30,300,180
    WAIT ok 12-still 0,30,300,180
    WAIT ok 14-line: [jihad-bs] luna: findPrinters (started)
    WAIT ok 15-differs 0,200,756,960
    WAIT ok 16-log: [jihad-bs] js-console (no source):0:0 print: mdns: EPSON ET-2800 Series._ipps._tcp.local -> 192.168.1.253:631 (answer from 192.168.1.253)
    WAIT ok 17-still 0,200,756,960

### mochi / appmenu-settings-keyboard-down — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    WAIT ok 15-load
    CHECK ok 16-url: load done uri=about:settings#chrome=%7B%22searchEngine%22%3A%22duckduckgo-lite%22%2C%22homeUrl%22%3A%22https%3A%2F%2Flite.duckduckgo.com%2Flite%2F%22%2C%22startLinks%22%3A%5B%7B%22title%22%3A%22DuckDuckGo%22%2C%22url%22%3A%22https%3A%2F%2Fduckduckgo.com%2F%22%7D%2C%7B%22title%22%3A%22Wikipedia%22%2C%22url%22%3A%2
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 18-rect page-title
    CHECK ok 19-content: region 0,100-768,700: colours 787 hash 3b16afe0 rows 600 uniform 0 light none blue none; page-title on the screen at 20,96-744,130, text holds [Settings]

### mochi / busy-script-stop — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 9-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 13-rect sel:h1
    CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36

### mochi / card-dialog — pass

    mochi card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +2 s, load done at +6 s, 2 content frames at +36 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 7-rect c
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 138 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 13-text r = [c: true]
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-2.fifo
    CHECK ok 17-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 18-text r = [c: false]

### mochi / card-select — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 9-text picked = [picked: banana]

### mochi / chrome-buttons — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 9-load
    WAIT ok 11-url: https://lite.duckduckgo.com/lite/
    WAIT ok 13-url: http://192.168.1.245:8077/jihad-long-static.html
    WAIT ok 15-url: https://lite.duckduckgo.com/lite/
    WAIT ok 18-differs 0,100,756,700

### mochi / chrome-history — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### mochi / chrome-padlock — pass

    mochi card on https://example.com/: connected at +1 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:p
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 382 landed on document y 304)
    WAIT ok 12-line: [jihad-bs] luna: pageCertificate (sent)
    WAIT ok 13-differs 0,100,756,960

### mochi / chrome-share — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### mochi / ddg-results — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 8-launch: load done
    WAIT ok 9-line: [jihad-bs] titleAndUrl title=[webOS at DuckDuckGo] uri=https://duckduckgo.com/?q=webOS&ia=web back=1 fwd=0
    CHECK ok 10-content: region 0,100-756,900: colours 133 hash c5b1089c rows 800 uniform 775 light 100-899 blue none; sel:body on the screen at 0,78-768,1942

### mochi / dialog-slow-answer — pass

    mochi card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 7-rect c
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 138 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text r = [c: true]

### mochi / download-finishes — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-40

### mochi / download-list — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-41
    CHECK ok 10-file: /media/internal/downloads/jihad-download-41 (17 bytes) holds [Hello from Jihad]
    WAIT ok 12-differs 0,100,756,960
    WAIT ok 14-still 0,100,756,960
    WAIT ok 17-differs 0,100,756,960
    WAIT ok 18-still 0,100,756,960
    WAIT ok 22-differs 450,80,768,400
    WAIT ok 23-still 450,80,768,400
    WAIT ok 25-content 460,90,760,300

### mochi / drag-and-pinch-page — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 9-card-scroll-min: the card's scroll y 650 (at least 400)
    WAIT ok 11-zoom
    CHECK ok 12-zoom-min: zoom 2.6242 (at least 1.5)
    CHECK ok 13-content: region 0,150-756,600: colours 686 hash 4fd58729 rows 450 uniform 197 light 168-599 blue 172-172; sel:body on the screen at -624,-2309-1380,51455
    WAIT ok 15-zoom
    WAIT ok 17-zoom
    CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15)

### mochi / etsy-search — pass

    mochi card on https://www.etsy.com/search?q=webos: connected at +1 s, load done at +39 s, 23 content frames at +39 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_C
    WAIT ok 8-rect sel:body
    WAIT ok 9-url: https://www.etsy.com/search?q=webos
    CHECK ok 10-url: load done uri=https://www.etsy.com/search?q=webos
    WAIT ok 11-content 0,100,768,900
    CHECK ok 12-content: region 0,100-768,900: colours 4096 hash db85be2b rows 800 uniform 162 light 100-899 blue none; sel:body on the screen at 0,78-768,8821

### mochi / everyday-browsing — pass

    mochi card on https://duckduckgo.com/: connected at +1 s, load done at +32 s, 38 content frames at +32 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:textarea[name='q']
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 437 landed on document y 359)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-launch: load done
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 154 landed on document y 110)
    CHECK ok 15-card-scroll-min: the card's scroll y 857 (at least 300)
    WAIT ok 17-url: https://duckduckgo.com/

### mochi / express-burst-wiki — pass

    mochi card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +18 s, 14 content frames at +18 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 8-rect sel:#firstHeading
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 154 landed on document y 110)
    WAIT ok 14-zoom
    WAIT ok 15-settled: the card's scroll 532 1124
    CHECK ok 16-zoom-min: zoom 2.3273 (at least 1.0)
    CHECK ok 17-content: region 0,100-756,700: colours 1007 hash 48dc43f9 rows 600 uniform 0 light 100-699 blue 520-697; sel:body on the screen at -1237,-2535-1359,676

### mochi / flash-api — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +1 s, load done at +8 s, 9 content frames at +8 s; the GPU path, 
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

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&vars=fs=1: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main
    WAIT ok 8-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 9-text kid = [kid w=376]
    WAIT ok 11-text fs = [fs fullScreen]
    WAIT ok 14-text fs = [fs normal]
    WAIT ok 17-text fs = [fs fullScreen]
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 20-flash-behind: the colour 00ff00 in 180 of 192 cells (wanted yes)

### mochi / flash-input-cover — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the expr
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 225 px up for the keyboard, the page at 225
    WAIT ok 14-settled: the card's scroll 0 225
    CHECK ok 16-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mochi / flash-input-hold — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-text up = [up 201 154]
    CHECK ok 10-number: heldms = 2322 (wanted 1500..3200)
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-number: n-up = 1 (wanted 1..1)
    WAIT ok 15-text up = [up 361 282]
    CHECK ok 16-flash-el: the movie's move at 355,278 for 360,277 (within 30)
    CHECK ok 17-press-rate: 26.4 moves a second sent on (at least 20): ] a press of 1424 ms: 33 moves from the card in 1248 ms, 33 sent on, 143 ms in sending them

### mochi / flash-input-kbdrag — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 12 content frames at +7 s; the GPU path, frames off the main thread (the express lane) 
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 16-settled: the card's scroll 0 0
    CHECK ok 17-number: n-move = 15 (wanted 5..400)
    WAIT ok 20-settled: the card's scroll 0 0
    CHECK ok 21-number: n-move = 23 (wanted 14..800)
    CHECK ok 23-flash-at: the movie reported down 301,100 for stage 300,100 (within 8)

### mochi / flash-input-kernel — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +3 s, load done at +8 s, 10 content frames at +8 s; the GPU path, frames off the main thread (the express
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

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +3 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 14-text keyup = [keyup 65 97]
    CHECK ok 15-number: keyheldms = 126 (wanted 60..2000)
    WAIT ok 17-text keyup = [keyup 39 0]
    CHECK ok 18-number: keyheldms = 60 (wanted 60..2000)
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / flash-input-keys — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (
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

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express 
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 98
    CHECK ok 10-card-scroll-min: the card's scroll y 98 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-number: n-down = 1 (wanted 1..1)
    CHECK ok 16-number: n-up = 1 (wanted 1..1)
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 20-settled: the card's scroll 0 0
    CHECK ok 21-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 22-number: n-move = 12 (wanted 5..400)
    CHECK ok 24-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 27-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 30-settled: the card's scroll 0 98
    CHECK ok 31-card-scroll-min: the card's scroll y 98 (at least 30)

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

### mochi / flash-layout — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=percent: connected at +1 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the
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
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 48 (16.0/s), changes 43 (14.3/s), reads 49, distinct 44, spacing median 67 p90 92 max 106 ms (wanted 5..100/s)

### mochi / flash-text — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text: connected at +3 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 15-text text = [text Hello]
    WAIT ok 17-text text = [text Hello World 123]
    WAIT ok 19-text text = [text Hello World x9]
    CHECK ok 20-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / flash-wmode — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm1&bg=00ff00&wmode=window: connected at +2 s, load done at +8 s, 14 content frames at +8 s; the GPU path, frames off the main thr
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

    mochi card on about:settings: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    WAIT ok 11-rect fonts-reset
    WAIT ok 13-pref font.name.serif.x-western = [Dotum]
    WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.]

### mochi / forget-identities — pass

    mochi card on about:settings: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.]

### mochi / frame-field — pass

    mochi card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
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

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 9 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / keyboard-arbitration — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +8 s, 10 content frames at +8 s; the GPU path, frames off the main thread (the express
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    WAIT ok 14-text keydown = [keydown 65 97]
    CHECK ok 15-region-equal: region 150,30-620,78: colours 173 hash 60c76c31 rows 48 uniform 19 light 40-66 blue none = the sample's hash
    CHECK ok 19-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 22-settled: the card's scroll 0 0
    CHECK ok 23-region-changed: region 150,30-620,78: colours 67 hash df7ac382 rows 48 uniform 19 light 40-66 blue 39-67 against the sample's hash 60c76c31
    CHECK ok 25-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / keyboard-fields — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 297 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 10-visible: area on the screen at 9,156-484,196 (the reach ends at 600, keyboard 1)
    CHECK ok 12-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 13-launch: load done
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 17-visible: field on the screen at 16,280-522,315 (the reach ends at 600, keyboard 1)

### mochi / keyboard-keys — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +3 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
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

### mochi / long-press-image — pass

    mochi card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +3 s, load done at +15 s, 9 content frames at +15 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 8-rect sel:.infobox img
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 154 landed on document y 110)
    WAIT ok 12-differs 0,100,756,900

### mochi / long-press-menu — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mochi / long-press-text-field — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 10-differs 0,100,756,600
    WAIT ok 13-still 0,100,756,600
    WAIT ok 15-line: [jihad-bs] hold at 269,219 editable=1 wasFocused=0
    WAIT ok 18-still 0,100,400,240
    WAIT ok 20-load
    CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mochi / media-controls — pass

    mochi card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +7 s, 14 content frames at +7 s; the GPU path, frames off the main thread (the e
    WAIT ok 9-rect v
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 98 landed on document y 25)
    WAIT ok 11-text status = [playing]
    WAIT ok 13-text status = [paused t=14.4]
    WAIT ok 18-rect anon:#v|controlBar
    WAIT ok 19-rect anon:#v|muteButton
    WAIT ok 22-text vol = [volume=1.00 muted=false]
    WAIT ok 24-text vol = [volume=1.00 muted=true]
    WAIT ok 26-text vol = [volume=1.00 muted=false]
    CHECK ok 28-visible: anon:#v|controlBar on the screen at 16,577-586,599 (the reach ends at 1024, keyboard 0)
    CHECK ok 30-visible: anon:#v|controlBar on the screen at 16,577-586,599 (the reach ends at 1024, keyboard 0)

### mochi / media-scrub-drag — pass

    mochi card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +7 s, 15 content frames at +7 s; the GPU path, frames off the main thread (the e
    WAIT ok 10-rect v
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 98 landed on document y 25)
    WAIT ok 12-text status = [playing]
    WAIT ok 14-text status = [paused t=14.8]
    WAIT ok 15-rect anon:#v|controlBar
    WAIT ok 16-rect anon:#v|volumeControl
    WAIT ok 20-text vol = [volume=0.16 muted=false]
    WAIT ok 24-settled: the card's scroll 0 0
    WAIT ok 25-text seek = [60]

### mochi / mobile-wiki-pinch — pass

    mochi card on https://en.m.wikipedia.org/wiki/HP_TouchPad: connected at +2 s, load done at +22 s, 10 content frames at +22 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 8-rect sel:#firstHeading
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 154 landed on document y 110)
    WAIT ok 11-still 0,100,756,700
    CHECK ok 12-zoom-min: zoom 0.6882 (at least 0.65)
    CHECK ok 13-content: region 0,100-756,700: colours 4096 hash 6a48c95d rows 600 uniform 69 light 100-699 blue 232-236; sel:body on the screen at 0,78-768,1028
    CHECK ok 14-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    WAIT ok 16-zoom
    CHECK ok 17-zoom-min: zoom 1.8241 (at least 1.05)
    CHECK ok 18-content: region 0,100-756,700: colours 4096 hash a219fe3e rows 600 uniform 3 light 100-699 blue none; sel:body on the screen at -634,-452-1401,2064

### mochi / mochi-paste — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 297 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-differs 100,100,668,420
    WAIT ok 15-still 100,100,668,420
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / pan-long-drag — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 8-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 9-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 10-content: region 0,150-768,400: colours 354 hash 6220be68 rows 250 uniform 0 light 150-399 blue none; sel:#content p on the screen at 16,150-748,206, text holds [quick brown]
    CHECK ok 14-card-scroll: the card's scroll y 650 (want 650 within 100); the engine's 650
    WAIT ok 15-still header
    CHECK ok 16-region-equal: region 0,79-756,133: colours 204 hash 5486acf3 rows 54 uniform 33 light none blue none = the sample's hash
    CHECK ok 17-region-changed: region 0,300-756,900: colours 573 hash 597e3ddc rows 600 uniform 219 light 300-899 blue none against the sample's hash b34d3334
    CHECK ok 18-top: the page's top measured 78 (the tap at y 208 landed on document y 780), the model says 78 (header hidden: 1)
    CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### mochi / pan-long-flick — known

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 21-card-scroll-min: the card's scroll y 3301 (at least 300)
    STREAM fs-mochi-pan-long-flick.fbs: 187 samples (14.5/s asked 15, read median 2.17 ms max 50.95 ms, mmap stride 8, re-read 5, torn 0), 134 flips, cpu 6.6% of a core, polls 714/s | shown frames while moving: 130 gaps, median 27.0 ms, p90 41 ms, max 72 ms | shown scroll: 1873 rows over the stream (0 samples with the shift unknown); stale bands 3, uniform bands 26, still rows moved 39; the columns outside 0-756 changed in 46 samples | tracking (finger down, 20 samples): error median 34.5 px, max 188 px; lag median 167.0 ms, max 800 ms |   gesture at 1794842703488: the finger travelled 600 rows, the page
    CHECK FAIL 16-stream-stale: stale 3 (want at most 0)
    CHECK FAIL 17-stream-uniform: uniform 26 (want at most 0)
    CHECK ok 19-stream-gap-median: gap-median 27 (at most 34)
    CHECK ok 20-stream-gap-p90: gap-p90 41 (at most 50)

### mochi / pan-speeds — known

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 13-card-scroll-min: the card's scroll y 650 (at least 500)
    CHECK ok 25-card-scroll-min: the card's scroll y 4586 (at least 2500)
    STREAM fs-mochi-pan-speeds.fbs: 429 samples (15.0/s asked 15, read median 2.19 ms max 27.17 ms, mmap stride 8, re-read 17, torn 0), 358 flips, cpu 6.0% of a core, polls 711/s | shown frames while moving: 351 gaps, median 27 ms, p90 41 ms, max 110 ms | shown scroll: 4382 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 29, still rows moved 98; the columns outside 0-756 changed in 127 samples | tracking (finger down, 121 samples): error median -12 px, max 225 px; lag median 69 ms, max 783 ms |   gesture at 1794842766491: the finger travelled 650 rows, the page showe
    CHECK ok 19-stream-stale: stale 0 (at most 0)
    CHECK FAIL 20-stream-uniform: uniform 29 (want at most 0)
    CHECK ok 21-stream-error-max: error-max 225 (at most 500)
    CHECK ok 23-stream-gap-median: gap-median 27 (at most 34)
    CHECK ok 24-stream-gap-p90: gap-p90 41 (at most 50)

### mochi / persona-cycle — pass

    mochi card on about:personas: connected at +1 s, load done at +8 s, 11 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    mochi card on about:personas: connected at +2 s, load done at +9 s, 12 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#online .card button
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 276 landed on document y 198)
    CHECK ok 11-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text status-text = [Wearing Autumn Forest.]
    WAIT ok 15-pref lightweightThemes.isThemeSelected = [true]
    WAIT ok 17-text status-text = [Undone.]
    WAIT ok 18-pref lightweightThemes.isThemeSelected = [false]

### mochi / personas-flick — pass

    mochi card on about:personas: connected at +3 s, load done at +9 s, 10 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#mine .card
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 276 landed on document y 198)
    CHECK ok 10-card-scroll-min: the card's scroll y 1930 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### mochi / personas-page-steady — pass

    mochi card on about:personas: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-rect sel:#mine .card
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 276 landed on document y 198)
    CHECK ok 19-content: region 0,100-768,900: colours 4096 hash d49b3af3 rows 800 uniform 0 light 101-646 blue 106-654; sel:body on the screen at 0,-419-764,2440
    STREAM fs-mochi-personas-page-steady.fbs: 115 samples (15.0/s asked 15, read median 2.32 ms max 35.3 ms, mmap stride 8, re-read 8, torn 0), 135 flips, cpu 6.3% of a core, polls 665/s | shown frames while moving: 134 gaps, median 25.0 ms, p90 36 ms, max 76 ms | shown scroll: 499 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 0-756 changed in 49 samples | tracking (finger down, 49 samples): error median -10 px, max 22 px; lag median 66 ms, max 155 ms |   gesture at 1794842849448: the finger travelled 500 rows, the page sh
    CHECK ok 15-stream-uniform: uniform 0 (at most 0)
    CHECK ok 17-stream-gap-median: gap-median 25 (at most 34)
    CHECK ok 18-stream-gap-p90: gap-p90 36 (at most 50)

### mochi / proxy-login — pass

    mochi card on about:settings: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

### mochi / relay-choice — pass

    mochi card on about:settings: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 12-rect sel:.row[data-pref='network.proxy.type'] select
    WAIT ok 14-pref network.proxy.http = [proxy.riverstonerelay.org]
    WAIT ok 15-launch: load done
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 382 landed on document y 304)
    CHECK ok 17-text-has: sel:p = [This domain is for use in documentation examples without needing permission. This is not a service; avoid relying on it for testing and monitoring purposes.] holds [documentation]

### mochi / rotate — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 13-line: [jihad-bs] emitGeometry contentSize=1024x20488 win=1024x690 mZoom=1.3333
    WAIT ok 14-content 0,150,756,900
    WAIT ok 17-line: [jihad-bs] emitGeometry contentSize=768x20488 win=768x946 mZoom=0.7500
    WAIT ok 18-content 0,150,756,900
    WAIT ok 20-line: [jihad-bs] emitGeometry contentSize=1024x20488 win=1024x690 mZoom=1.3333
    WAIT ok 21-content 0,150,756,900
    WAIT ok 24-line: [jihad-bs] emitGeometry contentSize=768x20488 win=768x946 mZoom=0.7500
    WAIT ok 25-settled: the card's scroll 0 0
    CHECK ok 26-content: region 0,150-756,600: colours 361 hash 20376874 rows 450 uniform 169 light 150-598 blue none; sel:body on the screen at 0,78-764,20566
    CHECK ok 28-card-scroll-min: the card's scroll y 650 (at least 400)

### mochi / screen-lock — pass

    mochi card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +7 s, 14 content frames at +7 s; the GPU path, frames off the main thread (the e
    WAIT ok 9-rect v
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 98 landed on document y 25)
    WAIT ok 11-text status = [playing]
    WAIT ok 20-text status = [playing]
    CHECK ok 22-text: status = [playing]
    WAIT ok 34-text status = [paused t=33.7]

### mochi / script-hold-zoom — pass

    mochi card on http://192.168.1.245:8077/jihad-script-hold.html: connected at +1 s, load done at +6 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 106 landed on document y 28)
    WAIT ok 9-zoom
    CHECK ok 10-zoom-min: zoom 1.9265 (at least 1.3)
    CHECK ok 12-card-scroll-min: the card's scroll y 458 (at least 200)
    WAIT ok 14-zoom
    WAIT ok 16-zoom
    CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2)

### mochi / settings-every-control — by a tool

    tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt)

### mochi / settings-field-saves — pass

    mochi card on about:settings: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    mochi card on about:settings: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    CHECK ok 10-card-scroll-min: the card's scroll y 707 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### mochi / settings-int-emptied — pass

    mochi card on about:settings: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 16-pref browser.sessionhistory.max_entries = [20]

### mochi / settings-persist — pass

    mochi card on about:settings: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-pref general.smoothScroll = [true]
    WAIT ok 14-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 16-pref general.smoothScroll = [true]

### mochi / settings-tabs — pass

    mochi card on about:settings: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    CHECK ok 9-text: page-title = [Settings]
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content]

### mochi / settings-typing — pass

    mochi card on about:settings: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 15-pref browser.display.background_color = [#FFFFFFx]
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / ssl-confirm — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog ssl-confirm -> cancel (answer=0)

### mochi / tabbed-browsing — not run

    not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word)

### mochi / touchpad-go — not run

    owner-accepted without hardware (device-build R6)

### mochi / typing-caret — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +3 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 297 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 11-text echo = [plain n=6 c=0 last=[abcdef] value=[abcdef] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 13-text echo = [plain n=12 c=0 last=[] value=[] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / video-controls — pass

    mochi card on http://192.168.1.245:8077/jihad-media-test.html: connected at +1 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect status
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 98 landed on document y 25)
    WAIT ok 12-changed sel:video
    WAIT ok 13-changed sel:video
    WAIT ok 15-still sel:video
    WAIT ok 17-still sel:video
    CHECK ok 18-region-equal: region 16,172-517,452: colours 2439 hash 2b6888d4 rows 280 uniform 0 light 440-444 blue none = the sample's hash

### mochi / webgl-cube — pass

    mochi card on http://192.168.1.245:8077/jihad-webgl-cube.html: connected at +1 s, load done at +7 s, 15 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 7-rect c
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 120 landed on document y 57)
    CHECK ok 9-text-has: out = [webgl fps=24.8 shaders=ok texture=ok framebuffer=ok readPixels=ok maxtex=4096 ext=14 missing=ANGLE_instanced_arrays,WEBGL_compressed_texture_s3tc,EXT_blend_minmax,EXT_shader_texture_lod depthrb=ok renderer=Mozilla vendor=Mozilla] holds [webgl]
    CHECK ok 10-content: region 105,171-457,520: colours 807 hash a727fcd0 rows 349 uniform 153 light none blue 270-427; c on the screen at 105,170-457,521
    WAIT ok 11-changed c
    WAIT ok 13-zoom
    CHECK ok 15-content: region 0,100-768,900: colours 1290 hash 56064a7a rows 800 uniform 267 light none blue 238-300; c on the screen at -52,80-499,632
    WAIT ok 16-changed c

### mochi / webone — not run

    needs webone, keyboard-map (run with --with webone,keyboard-map)

### mochi / wikipedia-keyboard-zoom — pass

    mochi card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +18 s, 12 content frames at +18 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
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

### mochi / wikipedia-zoom — known

    mochi card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +14 s, 9 content frames at +14 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 11-rect sel:.infobox
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 154 landed on document y 110)
    WAIT ok 15-zoom
    CHECK ok 16-zoom-min: zoom 2.8444 (at least 1.3)
    WAIT ok 18-zoom
    WAIT ok 20-zoom
    WAIT ok 22-still 0,100,756,700
    CHECK ok 24-zoom: zoom 0.6882 (want 0.69 within 0.12)
    CHECK ok 25-content: region 0,100-756,700: colours 4096 hash 2d40fbff rows 600 uniform 66 light 100-699 blue 232-236; sel:body on the screen at 0,78-768,1028
    CHECK ok 26-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    STREAM fs-mochi-wikipedia-zoom.fbs: 223 samples (14.9/s asked 15, read median 1.81 ms max 99.22 ms, mmap stride 8, re-read 10, torn 1), 151 flips, cpu 6.8% of a core, polls 699/s | shown frames while moving: 143 gaps, median 38 ms, p90 62 ms, max 260 ms | shown scroll: 71 rows over the stream (40 samples with the shift unknown); stale bands 0, uniform bands 5, still rows moved 1; the columns outside 170-600 changed in 19 samples | tracking (finger down, 38 samples): error median 0.0 px, max 1 px; lag median 381.0 ms, max 848 ms |   gesture at 1794843071934: the finger travelled 0 rows, the page showe
    CHECK FAIL 27-stream-uniform: uniform 5 (want at most 0)
    CHECK ok 29-stream-gap-median: gap-median 38 (at most 45)
    CHECK ok 30-stream-gap-p90: gap-p90 62 (at most 80)

### mochi / xpi-confirm — pass

    mochi card on http://192.168.1.245:8077/jihad-xpi-trigger.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 8-rect effect
    calibrated (mochi): the page's top on the screen 79 (the table said 78, the tap at y 125 landed on document y 59)
    CHECK ok 11-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 14-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 15-text r = [r: callback status=0 Jihad Effect Test]
    WAIT ok 16-launch: load done
    WAIT ok 17-rect sel:#category-extension
    WAIT ok 19-rect sel:#category-extension[selected='true']
    WAIT ok 21-rect sel:richlistitem[value='jihad-effect@riverstonerelay.net']

### mochi / youtube-mobile — pass

    mochi card on https://m.youtube.com/watch?v=dQw4w9WgXcQ: connected at +1 s, load done at +18 s, 21 content frames at +18 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 8-rect sel:video
    WAIT ok 10-changed 0,130,756,560
    WAIT ok 11-changed 0,130,756,560

### mochi / youtube-no-next — pass

    mochi card on https://m.youtube.com/watch?v=jNQXAC9IVRw: connected at +2 s, load done at +17 s, 11 content frames at +17 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 9-rect sel:video
    WAIT ok 11-line: [jihad-bs] js-console (no source):0:0 compat: youtube [m.youtube.com] play() allowed: 1080 ms after a click
    CHECK ok 13-probe-has: the probe's answer holds ["paused":false]
    CHECK ok 14-quiet: no line play() allowed for 70 s
    CHECK ok 16-probe-has: the probe's answer holds ["paused":true]
    CHECK ok 17-url: load done uri=https://m.youtube.com/watch?v=jNQXAC9IVRw

### mochi / zoom-dtap-column — pass

    mochi card on http://192.168.1.245:8077/jihad-dtap.html: connected at +1 s, load done at +6 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    WAIT ok 7-rect sel:#col
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 110 landed on document y 50)
    WAIT ok 10-zoom
    WAIT ok 11-settled: the card's scroll 590 271
    CHECK ok 12-zoom-min: zoom 1.8286 (at least 1.5)
    CHECK ok 13-element-at: sel:#col on the screen at 18,-195-750,1998 holds 384,420
    CHECK ok 14-content: region 0,100-756,700: colours 402 hash 6c6a1b91 rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-195-750,1998

### mochi / zoom-field-kept — pass

    mochi card on http://192.168.1.245:8077/jihad-dtap.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    WAIT ok 7-rect sel:#f
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 110 landed on document y 50)
    CHECK ok 9-zoom-max: zoom 0.6400 (at most 0.8)
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-settled: the card's scroll 0 425
    CHECK ok 14-above-keyboard: sel:#f on the screen at 64,638-324,667, above the keyboard
    CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8)

### mochi / zoom-locked-pinch-off — pass

    mochi card on http://192.168.1.245:8077/jihad-locked.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 8-rect sel:h1
    WAIT ok 12-settled: the card's scroll 0 0
    CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05)

### mochi / zoom-locked-pinch — pass

    mochi card on http://192.168.1.245:8077/jihad-locked.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect sel:h1
    WAIT ok 11-zoom
    WAIT ok 12-settled: the card's scroll 186 204
    CHECK ok 13-zoom-min: zoom 1.9361 (at least 1.3)

### mochi / zoom-new-page — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 10-zoom
    CHECK ok 11-zoom-min: zoom 1.9123 (at least 1.5)
    WAIT ok 12-launch: load done
    WAIT ok 13-still 0,100,756,600
    CHECK ok 14-zoom: zoom 1.0000 (want 1.0 within 0.05)
    WAIT ok 15-launch: load done
    WAIT ok 16-still 0,100,756,600
    CHECK ok 17-zoom: zoom 0.6882 (want 0.69 within 0.06)

### mojo / addon-site — pass

    mojo card on https://addons.riverstonerelay.org/: connected at +2 s, load done at +12 s, 4 content frames at +12 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COM
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

    mojo card on about:addons: connected at +2 s, load done at +10 s, 5 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 249 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='pad01@jihad.test']
    WAIT ok 13-rect anon:richlistitem[value='pad01@jihad.test']|disable-btn
    WAIT ok 15-rect anon:richlistitem[value='pad01@jihad.test']|enable-btn
    WAIT ok 17-rect anon:richlistitem[value='pad01@jihad.test']|undo-btn
    WAIT ok 19-rect anon:richlistitem[value='pad01@jihad.test']|remove-btn

### mojo / addons-list-pan — pass

    mojo card on about:addons: connected at +1 s, load done at +9 s, 3 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 249 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:richlistitem[value='pad12@jihad.test']
    WAIT ok 15-still 0,100,756,1000
    WAIT ok 17-rect sel:richlistitem[value='pad12@jihad.test'][selected='true']
    CHECK ok 18-hit: clickAt 843,1439 inside sel:richlistitem[value="pad12@jihad.test"] at 294,1412 656x99

### mojo / addons-page — pass

    mojo card on about:addons: connected at +1 s, load done at +6 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#category-extension
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 249 landed on document y 224)
    WAIT ok 9-rect sel:#category-extension[selected='true']
    WAIT ok 10-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:#category-plugin[selected='true']
    WAIT ok 14-rect sel:#category-extension[selected='true']
    WAIT ok 16-rect sel:#detail-view:not([hidden])
    CHECK ok 17-text-has: sel:#detail-name = [Palefill Web Technologies Polyfill] holds [Palefill]

### mojo / addons-search-hide-key — pass

    mojo card on about:addons: connected at +1 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 10-differs 0,30,300,180
    WAIT ok 11-still 0,30,300,180
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 16-log: [jihad-bs] find 'Zebrafind' -> 1

### mojo / appmenu-print — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 11-differs 0,30,300,180
    WAIT ok 12-still 0,30,300,180
    WAIT ok 14-line: [jihad-bs] luna: findPrinters (started)
    WAIT ok 15-differs 0,200,756,960
    WAIT ok 16-log: [jihad-bs] js-console (no source):0:0 print: mdns: EPSON ET-2800 Series @ macmini._ipp._tcp.local -> 192.168.1.162:631 (answer from 192.168.1.162)
    WAIT ok 17-still 0,200,756,960

### mojo / appmenu-settings-keyboard-down — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    WAIT ok 15-load
    CHECK ok 16-url: load done uri=about:settings#chrome=%7B%22searchEngine%22%3A%22duckduckgo-lite%22%2C%22homeUrl%22%3A%22https%3A%2F%2Flite.duckduckgo.com%2Flite%2F%22%2C%22startLinks%22%3A%5B%7B%22title%22%3A%22DuckDuckGo%22%2C%22url%22%3A%22https%3A%2F%2Fduckduckgo.com%2F%22%7D%2C%7B%22title%22%3A%22Wikipedia%22%2C%22url%22%3A%2
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 18-rect page-title
    CHECK ok 19-content: region 0,100-768,700: colours 448 hash 7a103668 rows 600 uniform 0 light none blue none; page-title on the screen at 20,91-744,125, text holds [Settings]

### mojo / busy-script-stop — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 9-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 13-rect sel:h1
    CHECK ok 15-hit: clickAt 491,39 inside sel:h1 at 8,21 964x36

### mojo / card-dialog — pass

    mojo card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +1 s, load done at +5 s, 6 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect c
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 133 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 12-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 13-text r = [c: true]
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-2.fifo
    CHECK ok 17-dialog: [jihad-bs] dialog confirm -> cancel (answer=0)
    WAIT ok 18-text r = [c: false]

### mojo / card-select — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +5 s, 2 content frames at +35 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 9-text picked = [picked: banana]

### mojo / chrome-buttons — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 9-load
    WAIT ok 11-url: https://lite.duckduckgo.com/lite/
    WAIT ok 13-url: http://192.168.1.245:8077/jihad-long-static.html
    WAIT ok 15-url: https://lite.duckduckgo.com/lite/
    WAIT ok 18-differs 0,100,756,700

### mojo / chrome-history — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### mojo / chrome-padlock — pass

    mojo card on https://example.com/: connected at +1 s, load done at +5 s, 5 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:p
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 389 landed on document y 316)
    WAIT ok 12-line: [jihad-bs] luna: pageCertificate (sent)
    WAIT ok 13-differs 0,100,756,960

### mojo / chrome-share — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 10-differs 0,100,756,960
    WAIT ok 13-still 0,100,756,960
    CHECK ok 15-hit: clickAt 383,372 inside sel:#content p:nth-child(5) at 16,344 732x56

### mojo / ddg-results — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +5 s, 5 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 8-launch: load done
    WAIT ok 9-line: [jihad-bs] titleAndUrl title=[webOS at DuckDuckGo] uri=https://duckduckgo.com/?q=webOS&ia=web back=1 fwd=0
    CHECK ok 10-content: region 0,100-756,900: colours 102 hash 9584832d rows 800 uniform 780 light 100-899 blue none; sel:body on the screen at 0,73-768,2556

### mojo / dialog-slow-answer — pass

    mojo card on http://192.168.1.245:8077/jihad-dialogs.html: connected at +1 s, load done at +5 s, 5 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect c
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 133 landed on document y 60)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text r = [c: true]

### mojo / download-finishes — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-42

### mojo / download-list — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-43
    CHECK ok 10-file: /media/internal/downloads/jihad-download-43 (17 bytes) holds [Hello from Jihad]
    WAIT ok 12-differs 0,100,756,960
    WAIT ok 14-still 0,100,756,960
    WAIT ok 17-differs 0,100,756,960
    WAIT ok 18-still 0,100,756,960
    WAIT ok 22-differs 450,80,768,400
    WAIT ok 23-still 450,80,768,400
    WAIT ok 26-content 10,90,400,300

### mojo / drag-and-pinch-page — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 9-card-scroll-min: the card's scroll y 594 (at least 400)
    WAIT ok 11-zoom
    CHECK ok 12-zoom-min: zoom 2.6684 (at least 1.5)
    CHECK ok 13-content: region 0,150-756,600: colours 314 hash cddf36c0 rows 450 uniform 226 light 177-599 blue none; sel:body on the screen at -642,-2341-1396,52329
    WAIT ok 15-zoom
    WAIT ok 17-zoom
    CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15)

### mojo / etsy-search — pass

    mojo card on https://www.etsy.com/search?q=webos: connected at +1 s, load done at +38 s, 20 content frames at +38 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_CO
    WAIT ok 8-rect sel:body
    WAIT ok 9-url: https://www.etsy.com/search?q=webos
    CHECK ok 10-url: load done uri=https://www.etsy.com/search?q=webos
    WAIT ok 11-content 0,100,768,900
    CHECK ok 12-content: region 0,100-768,900: colours 4096 hash ff3f3399 rows 800 uniform 139 light 100-899 blue 502-537; sel:body on the screen at 0,73-768,8785

### mojo / everyday-browsing — pass

    mojo card on https://duckduckgo.com/: connected at +1 s, load done at +29 s, 30 content frames at +29 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:textarea[name='q']
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 436 landed on document y 363)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 12-launch: load done
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 149 landed on document y 110)
    CHECK ok 15-card-scroll-min: the card's scroll y 777 (at least 300)
    WAIT ok 17-url: https://duckduckgo.com/

### mojo / express-burst-wiki — pass

    mojo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +16 s, 7 content frames at +16 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 8-rect sel:#firstHeading
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 149 landed on document y 110)
    WAIT ok 14-zoom
    WAIT ok 15-settled: the card's scroll 532 1133
    CHECK ok 16-zoom-min: zoom 2.3273 (at least 1.0)
    CHECK ok 17-content: region 0,100-756,700: colours 727 hash 4aaa01d3 rows 600 uniform 0 light 100-699 blue 449-674; sel:body on the screen at -1237,-2606-1359,775

### mojo / flash-api — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +1 s, load done at +7 s, 11 content frames at +7 s; the GPU path, 
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

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&vars=fs=1: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off the main 
    WAIT ok 8-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 9-text kid = [kid w=376]
    WAIT ok 11-text fs = [fs fullScreen]
    WAIT ok 14-text fs = [fs normal]
    WAIT ok 17-text fs = [fs fullScreen]
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 20-flash-behind: the colour 00ff00 in 192 of 192 cells (wanted yes)

### mojo / flash-input-cover — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the expre
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 175 px up for the keyboard, the page at 175
    WAIT ok 14-settled: the card's scroll 0 175
    CHECK ok 16-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mojo / flash-input-hold — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-text up = [up 201 159]
    CHECK ok 10-number: heldms = 2330 (wanted 1500..3200)
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-number: n-up = 1 (wanted 1..1)
    WAIT ok 15-text up = [up 361 287]
    CHECK ok 16-flash-el: the movie's move at 361,287 for 360,277 (within 30)
    CHECK ok 17-press-rate: 26.1 moves a second sent on (at least 20): ] a press of 1360 ms: 32 moves from the card in 1228 ms, 32 sent on, 100 ms in sending them

### mojo / flash-input-kbdrag — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 16-settled: the card's scroll 0 0
    CHECK ok 17-number: n-move = 14 (wanted 5..400)
    WAIT ok 20-settled: the card's scroll 0 0
    CHECK ok 21-number: n-move = 20 (wanted 14..800)
    CHECK ok 23-flash-at: the movie reported down 301,100 for stage 300,100 (within 8)

### mojo / flash-input-kernel — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +8 s, 9 content frames at +8 s; the GPU path, frames off the main thread (the express l
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

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +8 s, 9 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 14-text keyup = [keyup 65 97]
    CHECK ok 15-number: keyheldms = 250 (wanted 60..2000)
    WAIT ok 17-text keyup = [keyup 39 0]
    CHECK ok 18-number: keyheldms = 78 (wanted 60..2000)
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / flash-input-keys — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
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

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +1 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express l
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 53
    CHECK ok 10-card-scroll-min: the card's scroll y 53 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-number: n-down = 1 (wanted 1..1)
    CHECK ok 16-number: n-up = 1 (wanted 1..1)
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 20-settled: the card's scroll 0 0
    CHECK ok 21-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 22-number: n-move = 8 (wanted 5..400)
    CHECK ok 24-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 27-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 30-settled: the card's scroll 0 100
    CHECK ok 31-card-scroll-min: the card's scroll y 100 (at least 30)

### mojo / flash-input-tap — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 10 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (
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
    WAIT ok 23-settled: the card's scroll 0 1400
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-rect show
    WAIT ok 29-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mojo / flash-tags — pass

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
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 43 (14.3/s), changes 36 (12.0/s), reads 44, distinct 37, spacing median 67 p90 82 max 165 ms (wanted 5..100/s)

### mojo / flash-text — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 15-text text = [text Hello]
    WAIT ok 17-text text = [text Hello World 123]
    WAIT ok 19-text text = [text Hello World x9]
    CHECK ok 20-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / flash-wmode — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm1&bg=00ff00&wmode=window: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off the main threa
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

    mojo card on about:settings: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    WAIT ok 11-rect fonts-reset
    WAIT ok 13-pref font.name.serif.x-western = [Dotum]
    WAIT ok 15-text sel:#status = [The Latin (Western) fonts are back to their defaults.]

### mojo / forget-identities — pass

    mojo card on about:settings: connected at +1 s, load done at +5 s, 5 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-text sel:.row[data-label='Sites remembered'] = [Sites rememberedNone yet. The browser says it is Firefox 52.9; a site whose bot check turns the page away is given a current Firefox identity and remembered here.]

### mojo / frame-field — pass

    mojo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +1 s, load done at +5 s, 8 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
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

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / keyboard-arbitration — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express l
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    WAIT ok 14-text keydown = [keydown 65 97]
    CHECK ok 15-region-equal: region 150,30-620,78: colours 173 hash ff22f881 rows 48 uniform 30 light 35-77 blue none = the sample's hash
    CHECK ok 19-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 22-settled: the card's scroll 0 0
    CHECK ok 23-region-changed: region 150,30-620,78: colours 4 hash f7bf3bef rows 48 uniform 48 light 35-77 blue none against the sample's hash ff22f881
    CHECK ok 25-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / keyboard-fields — pass

    mojo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 292 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 10-visible: area on the screen at 9,151-484,191 (the reach ends at 600, keyboard 1)
    CHECK ok 12-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 13-launch: load done
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 17-visible: field on the screen at 16,275-522,310 (the reach ends at 600, keyboard 1)

### mojo / keyboard-keys — pass

    mojo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
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

### mojo / long-press-image — pass

    mojo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +2 s, load done at +15 s, 11 content frames at +15 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 8-rect sel:.infobox img
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 149 landed on document y 110)
    WAIT ok 12-differs 0,100,756,900

### mojo / long-press-menu — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mojo / long-press-text-field — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 10-differs 0,100,756,600
    WAIT ok 13-still 0,100,756,600
    WAIT ok 15-line: [jihad-bs] hold at 269,219 editable=1 wasFocused=0
    WAIT ok 18-still 0,100,400,240
    WAIT ok 20-load
    CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mojo / media-controls — pass

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +7 s, 26 content frames at +7 s; the GPU path, frames off the main thread (the ex
    WAIT ok 9-rect v
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 93 landed on document y 25)
    WAIT ok 11-text status = [playing]
    WAIT ok 13-text status = [paused t=47.6]
    WAIT ok 18-rect anon:#v|controlBar
    WAIT ok 19-rect anon:#v|muteButton
    WAIT ok 22-text vol = [volume=1.00 muted=false]
    WAIT ok 24-text vol = [volume=1.00 muted=true]
    WAIT ok 26-text vol = [volume=1.00 muted=false]
    CHECK ok 28-visible: anon:#v|controlBar on the screen at 16,572-586,594 (the reach ends at 1024, keyboard 0)
    CHECK ok 30-visible: anon:#v|controlBar on the screen at 16,572-586,594 (the reach ends at 1024, keyboard 0)

### mojo / media-scrub-drag — pass

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +7 s, 24 content frames at +7 s; the GPU path, frames off the main thread (the ex
    WAIT ok 10-rect v
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 93 landed on document y 25)
    WAIT ok 12-text status = [playing]
    WAIT ok 14-text status = [paused t=12.9]
    WAIT ok 15-rect anon:#v|controlBar
    WAIT ok 16-rect anon:#v|volumeControl
    WAIT ok 20-text vol = [volume=0.16 muted=false]
    WAIT ok 24-settled: the card's scroll 0 0
    WAIT ok 25-text seek = [60]

### mojo / mobile-wiki-pinch — pass

    mojo card on https://en.m.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +21 s, 14 content frames at +21 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 8-rect sel:#firstHeading
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 149 landed on document y 110)
    WAIT ok 11-still 0,100,756,700
    CHECK ok 12-zoom-min: zoom 0.6882 (at least 0.65)
    CHECK ok 13-content: region 0,100-756,700: colours 4096 hash 5667be4f rows 600 uniform 67 light 100-699 blue 227-231; sel:body on the screen at 0,73-768,1073
    CHECK ok 14-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    WAIT ok 16-zoom
    CHECK ok 17-zoom-min: zoom 1.8430 (at least 1.05)
    CHECK ok 18-content: region 0,100-756,700: colours 4096 hash 1462974a rows 600 uniform 0 light 100-699 blue none; sel:body on the screen at -646,-427-1410,2250

### mojo / mojo-address-typing — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 14-nav-paint: the new page's first paint 2792 ms after the load started (at most 6000)
    WAIT ok 15-load
    CHECK ok 16-url: load done uri=https://lite.duckduckgo.com/lite/?q=webos

### mojo / mojo-launch-from-app — pass

    mojo card on about:blank: connected at +1 s, load done at +5 s, 0 content frames at +35 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-url: http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 12-content 0,150,756,600
    CHECK ok 13-content: region 0,150-756,600: colours 373 hash f5159163 rows 450 uniform 164 light 150-593 blue none; sel:body on the screen at 0,73-764,20561

### mojo / pan-long-drag — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 8-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 9-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 10-content: region 0,150-768,400: colours 374 hash 85b98daf rows 250 uniform 0 light 150-399 blue none; sel:#content p on the screen at 16,145-748,201, text holds [quick brown]
    CHECK ok 14-card-scroll: the card's scroll y 605 (want 650 within 100); the engine's 605
    WAIT ok 15-still header
    CHECK ok 16-region-equal: region 0,29-756,83: colours 185 hash eeaac7d6 rows 54 uniform 33 light none blue none = the sample's hash
    CHECK ok 17-region-changed: region 0,300-756,900: colours 658 hash 6a684a79 rows 600 uniform 217 light 300-895 blue none against the sample's hash 39f383a9
    CHECK ok 18-top: the page's top measured 73 (the tap at y 203 landed on document y 780), the model says 73 (header hidden: 1)
    CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### mojo / pan-long-flick — known

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 21-card-scroll-min: the card's scroll y 2917 (at least 300)
    STREAM fs-mojo-pan-long-flick.fbs: 141 samples (14.2/s asked 15, read median 2.22 ms max 97.56 ms, mmap stride 8, re-read 8, torn 0), 97 flips, cpu 7.1% of a core, polls 610/s | shown frames while moving: 93 gaps, median 35 ms, p90 55 ms, max 258 ms | shown scroll: 162 rows over the stream (2 samples with the shift unknown); stale bands 4, uniform bands 14, still rows moved 0; the columns outside 0-756 changed in 34 samples | tracking (finger down, 23 samples): error median 39 px, max 450 px; lag median 202 ms, max 909 ms |   gesture at 1794845696861: the finger travelled 600 rows, the page showed 0 
    CHECK FAIL 16-stream-stale: stale 4 (want at most 0)
    CHECK FAIL 17-stream-uniform: uniform 14 (want at most 0)
    CHECK FAIL 19-stream-gap-median: gap-median 35 (want at most 34)
    CHECK FAIL 20-stream-gap-p90: gap-p90 55 (want at most 50)

### mojo / pan-speeds — known

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +6 s, 2 content frames at +36 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 13-card-scroll-min: the card's scroll y 605 (at least 500)
    CHECK ok 25-card-scroll-min: the card's scroll y 4568 (at least 2500)
    STREAM fs-mojo-pan-speeds.fbs: 476 samples (14.6/s asked 15, read median 2.23 ms max 90.05 ms, mmap stride 8, re-read 19, torn 0), 283 flips, cpu 6.5% of a core, polls 722/s | shown frames while moving: 275 gaps, median 32 ms, p90 48 ms, max 281 ms | shown scroll: 2740 rows over the stream (1 samples with the shift unknown); stale bands 0, uniform bands 13, still rows moved 0; the columns outside 0-756 changed in 112 samples | tracking (finger down, 125 samples): error median -10 px, max 263 px; lag median 72 ms, max 787 ms |   gesture at 1794843637469: the finger travelled 650 rows, the page showed 
    CHECK ok 19-stream-stale: stale 0 (at most 0)
    CHECK FAIL 20-stream-uniform: uniform 13 (want at most 0)
    CHECK ok 21-stream-error-max: error-max 263 (at most 500)
    CHECK ok 23-stream-gap-median: gap-median 32 (at most 34)
    CHECK ok 24-stream-gap-p90: gap-p90 48 (at most 50)

### mojo / persona-cycle — pass

    mojo card on about:personas: connected at +1 s, load done at +8 s, 12 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    mojo card on about:personas: connected at +1 s, load done at +7 s, 12 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#online .card button
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 271 landed on document y 198)
    CHECK ok 11-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 14-text status-text = [Wearing Autumn Forest.]
    WAIT ok 15-pref lightweightThemes.isThemeSelected = [true]
    WAIT ok 17-text status-text = [Undone.]
    WAIT ok 18-pref lightweightThemes.isThemeSelected = [false]

### mojo / personas-flick — pass

    mojo card on about:personas: connected at +2 s, load done at +9 s, 10 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#mine .card
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 271 landed on document y 198)
    CHECK ok 10-card-scroll-min: the card's scroll y 1602 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### mojo / personas-page-steady — pass

    mojo card on about:personas: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-rect sel:#mine .card
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 271 landed on document y 198)
    CHECK ok 19-content: region 0,100-768,900: colours 4096 hash 4f6212d6 rows 800 uniform 0 light 181-736 blue 155-744; sel:body on the screen at 0,-426-764,2441
    STREAM fs-mojo-personas-page-steady.fbs: 122 samples (15.0/s asked 15, read median 2.21 ms max 37.92 ms, mmap stride 8, re-read 3, torn 0), 107 flips, cpu 6.2% of a core, polls 724/s | shown frames while moving: 106 gaps, median 28.0 ms, p90 40 ms, max 271 ms | shown scroll: 501 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 0-756 changed in 42 samples | tracking (finger down, 50 samples): error median -10.0 px, max 60 px; lag median 70.0 ms, max 439 ms |   gesture at 1794843731322: the finger travelled 500 rows, the pa
    CHECK ok 15-stream-uniform: uniform 0 (at most 0)
    CHECK ok 17-stream-gap-median: gap-median 28 (at most 34)
    CHECK ok 18-stream-gap-p90: gap-p90 40 (at most 50)

### mojo / proxy-login-mojo — pass

    mojo card on about:settings: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 9-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    CHECK ok 12-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 13-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 14-still 0,440,756,720
    CHECK ok 21-dialog: [jihad-bs] dialog auth -> ACCEPT (answer=1)
    WAIT ok 22-line: [jihad-bs] load done uri=http://example.com/jihad-proxy-login

### mojo / proxy-own — pass

    mojo card on about:settings: connected at +1 s, load done at +5 s, 5 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    mojo card on about:settings: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 11-rect sel:.row[data-pref='network.proxy.type'] select
    WAIT ok 13-rect sel:.row[data-pref='network.proxy.http'] input
    WAIT ok 15-gone sel:.row[data-pref='network.proxy.http'] input

### mojo / relay-choice — pass

    mojo card on about:settings: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='advanced']
    WAIT ok 12-rect sel:.row[data-pref='network.proxy.type'] select
    WAIT ok 14-pref network.proxy.http = [proxy.riverstonerelay.org]
    WAIT ok 15-launch: load done
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 389 landed on document y 316)
    CHECK ok 17-text-has: sel:p = [This domain is for use in documentation examples without needing permission. This is not a service; avoid relying on it for testing and monitoring purposes.] holds [documentation]

### mojo / rotate — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 13-line: [jihad-bs] emitGeometry contentSize=1024x20488 win=1024x740 mZoom=1.3333
    WAIT ok 14-content 0,150,756,900
    WAIT ok 17-line: [jihad-bs] emitGeometry contentSize=768x20488 win=768x996 mZoom=0.7500
    WAIT ok 18-content 0,150,756,900
    WAIT ok 20-line: [jihad-bs] emitGeometry contentSize=1024x20488 win=1024x740 mZoom=1.0000
    WAIT ok 21-content 0,150,756,900
    WAIT ok 24-line: [jihad-bs] emitGeometry contentSize=768x20488 win=768x996 mZoom=0.7500
    WAIT ok 25-settled: the card's scroll 0 0
    CHECK ok 26-content: region 0,150-756,600: colours 373 hash f5159163 rows 450 uniform 164 light 150-593 blue none; sel:body on the screen at 0,73-764,20561
    CHECK ok 28-card-scroll-min: the card's scroll y 584 (at least 400)

### mojo / screen-lock — pass

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +2 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the ex
    WAIT ok 9-rect v
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 93 landed on document y 25)
    WAIT ok 11-text status = [playing]
    WAIT ok 20-text status = [playing]
    CHECK ok 22-text: status = [playing]
    WAIT ok 34-text status = [paused t=35.9]

### mojo / script-hold-zoom — pass

    mojo card on http://192.168.1.245:8077/jihad-script-hold.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 101 landed on document y 28)
    WAIT ok 9-zoom
    CHECK ok 10-zoom-min: zoom 1.9265 (at least 1.3)
    CHECK ok 12-card-scroll-min: the card's scroll y 442 (at least 200)
    WAIT ok 14-zoom
    WAIT ok 16-zoom
    CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2)

### mojo / settings-every-control — by a tool

    tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt)

### mojo / settings-field-saves — pass

    mojo card on about:settings: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
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

    mojo card on about:settings: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    CHECK ok 10-card-scroll-min: the card's scroll y 675 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### mojo / settings-int-emptied — pass

    mojo card on about:settings: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 16-pref browser.sessionhistory.max_entries = [20]

### mojo / settings-persist — pass

    mojo card on about:settings: connected at +1 s, load done at +5 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='general']
    WAIT ok 12-pref general.smoothScroll = [true]
    WAIT ok 14-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 16-pref general.smoothScroll = [true]

### mojo / settings-tabs — pass

    mojo card on about:settings: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    CHECK ok 9-text: page-title = [Settings]
    WAIT ok 11-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-text-has: sel:h2.group-title = [Content] holds [Content]

### mojo / settings-typing — pass

    mojo card on about:settings: connected at +1 s, load done at +5 s, 7 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane='content']
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 15-pref browser.display.background_color = [#FFFFFFx]
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / ssl-confirm — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    CHECK ok 10-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 13-dialog: [jihad-bs] dialog ssl-confirm -> cancel (answer=0)

### mojo / tabbed-browsing — not run

    not built (tabbed-browsing R1/R6 — groundwork only, by the owner's word)

### mojo / touchpad-go — not run

    owner-accepted without hardware (device-build R6)

### mojo / typing-caret — pass

    mojo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +1 s, load done at +5 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 292 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 11-text echo = [plain n=6 c=0 last=[abcdef] value=[abcdef] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 13-text echo = [plain n=12 c=0 last=[] value=[] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    CHECK ok 15-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / video-controls — pass

    mojo card on http://192.168.1.245:8077/jihad-media-test.html: connected at +1 s, load done at +6 s, 20 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect status
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 93 landed on document y 25)
    WAIT ok 12-changed sel:video
    WAIT ok 13-changed sel:video
    WAIT ok 15-still sel:video
    WAIT ok 17-still sel:video
    CHECK ok 18-region-equal: region 16,167-517,447: colours 2781 hash 6ad38d99 rows 280 uniform 0 light 435-439 blue none = the sample's hash

### mojo / webgl-cube — pass

    mojo card on http://192.168.1.245:8077/jihad-webgl-cube.html: connected at +2 s, load done at +6 s, 31 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 7-rect c
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 115 landed on document y 57)
    CHECK ok 9-text-has: out = [webgl fps=21.1 shaders=ok texture=ok framebuffer=ok readPixels=ok maxtex=4096 ext=14 missing=ANGLE_instanced_arrays,WEBGL_compressed_texture_s3tc,EXT_blend_minmax,EXT_shader_texture_lod depthrb=ok renderer=Mozilla vendor=Mozilla] holds [webgl]
    CHECK ok 10-content: region 105,166-457,515: colours 720 hash f77d14a6 rows 349 uniform 104 light none blue 233-469; c on the screen at 105,165-457,516
    WAIT ok 11-changed c
    WAIT ok 13-zoom
    CHECK ok 15-content: region 0,100-768,900: colours 951 hash 00b0dd1f rows 800 uniform 325 light none blue 175-184; c on the screen at -52,22-499,574
    WAIT ok 16-changed c

### mojo / webone — not run

    needs webone, keyboard-map (run with --with webone,keyboard-map)

### mojo / wikipedia-keyboard-zoom — pass

    mojo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +15 s, 9 content frames at +15 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
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

    mojo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +13 s, 7 content frames at +13 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 11-rect sel:.infobox
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 149 landed on document y 110)
    WAIT ok 15-zoom
    CHECK ok 16-zoom-min: zoom 2.7529 (at least 1.3)
    WAIT ok 18-zoom
    WAIT ok 20-zoom
    WAIT ok 22-still 0,100,756,700
    CHECK ok 24-zoom: zoom 0.6882 (want 0.69 within 0.12)
    CHECK ok 25-content: region 0,100-756,700: colours 4096 hash 979450f1 rows 600 uniform 62 light 100-699 blue 227-231; sel:body on the screen at 0,73-768,1073
    CHECK ok 26-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    STREAM fs-mojo-wikipedia-zoom.fbs: 244 samples (14.9/s asked 15, read median 1.72 ms max 20.16 ms, mmap stride 8, re-read 4, torn 0), 141 flips, cpu 6.0% of a core, polls 736/s | shown frames while moving: 135 gaps, median 39 ms, p90 77 ms, max 285 ms | shown scroll: 110 rows over the stream (40 samples with the shift unknown); stale bands 0, uniform bands 8, still rows moved 0; the columns outside 170-600 changed in 20 samples | tracking (finger down, 37 samples): error median 0 px, max 14 px; lag median 345 ms, max 791 ms |   gesture at 1794843966894: the finger travelled 0 rows, the page showed 0 
    CHECK FAIL 27-stream-uniform: uniform 8 (want at most 0)
    CHECK ok 29-stream-gap-median: gap-median 39 (at most 45)
    CHECK ok 30-stream-gap-p90: gap-p90 77 (at most 80)

### mojo / xpi-confirm — pass

    mojo card on http://192.168.1.245:8077/jihad-xpi-trigger.html: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 8-rect effect
    calibrated (mojo): the page's top on the screen 74 (the table said 73, the tap at y 120 landed on document y 59)
    CHECK ok 11-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 14-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 15-text r = [r: callback status=0 Jihad Effect Test]
    WAIT ok 16-launch: load done
    WAIT ok 17-rect sel:#category-extension
    WAIT ok 19-rect sel:#category-extension[selected='true']
    WAIT ok 21-rect sel:richlistitem[value='jihad-effect@riverstonerelay.net']

### mojo / youtube-mobile — pass

    mojo card on https://m.youtube.com/watch?v=dQw4w9WgXcQ: connected at +1 s, load done at +17 s, 14 content frames at +17 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 8-rect sel:video
    WAIT ok 10-changed 0,130,756,560
    WAIT ok 11-changed 0,130,756,560

### mojo / youtube-no-next — pass

    mojo card on https://m.youtube.com/watch?v=jNQXAC9IVRw: connected at +1 s, load done at +16 s, 18 content frames at +16 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 9-rect sel:video
    WAIT ok 11-line: [jihad-bs] js-console (no source):0:0 compat: youtube [m.youtube.com] play() allowed: 1108 ms after a click
    CHECK ok 13-probe-has: the probe's answer holds ["paused":false]
    CHECK ok 14-quiet: no line play() allowed for 70 s
    CHECK ok 16-probe-has: the probe's answer holds ["paused":true]
    CHECK ok 17-url: load done uri=https://m.youtube.com/watch?v=jNQXAC9IVRw

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

    mojo card on http://192.168.1.245:8077/jihad-locked.html: connected at +1 s, load done at +5 s, 2 content frames at +35 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect sel:h1
    WAIT ok 11-zoom
    WAIT ok 12-settled: the card's scroll 186 183
    CHECK ok 13-zoom-min: zoom 1.9361 (at least 1.3)

### mojo / zoom-new-page — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 10-zoom
    CHECK ok 11-zoom-min: zoom 1.9361 (at least 1.5)
    WAIT ok 12-launch: load done
    WAIT ok 13-still 0,100,756,600
    CHECK ok 14-zoom: zoom 1.0000 (want 1.0 within 0.05)
    WAIT ok 15-launch: load done
    WAIT ok 16-still 0,100,756,600
    CHECK ok 17-zoom: zoom 0.6882 (want 0.69 within 0.06)

### enyo / frame-field-hold — pass

    enyo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +3 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect f
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 151 landed on document y 88)
    WAIT ok 12-line: [jihad-bs] hold at 77,336 editable=0 wasFocused=0
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=false readOnly=false selected=true
    WAIT ok 15-still f
    WAIT ok 18-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=0
    WAIT ok 19-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false
    WAIT ok 22-line: [jihad-bs] notify: page-hold: {"editable":true,"readOnly":false,"selected":false} (pushed)
    WAIT ok 23-differs f
    CHECK ok 24-region-changed: region 13,323-756,528: colours 299 hash 49e48b9b rows 205 uniform 0 light 324-526 blue none against the sample's hash 261e5084
    CHECK ok 25-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 30-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 32-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 34-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=1
    WAIT ok 35-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false

### mochi / frame-field-hold — pass

    mochi card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +2 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect f
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 147 landed on document y 88)
    WAIT ok 12-line: [jihad-bs] hold at 77,336 editable=0 wasFocused=0
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=false readOnly=false selected=true
    WAIT ok 15-still f
    WAIT ok 18-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=0
    WAIT ok 19-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false
    WAIT ok 22-line: [jihad-bs] notify: page-hold: {"editable":true,"readOnly":false,"selected":false} (pushed)
    WAIT ok 23-differs f
    CHECK ok 24-region-changed: region 13,319-756,524: colours 218 hash c90db968 rows 205 uniform 0 light none blue none against the sample's hash 22936c02
    CHECK ok 25-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 30-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 32-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 34-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=1
    WAIT ok 35-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false

### mojo / frame-field-hold — pass

    mojo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect f
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 142 landed on document y 88)
    WAIT ok 12-line: [jihad-bs] hold at 77,336 editable=0 wasFocused=0
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=false readOnly=false selected=true
    WAIT ok 15-still f
    WAIT ok 18-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=0
    WAIT ok 19-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false
    WAIT ok 22-line: [jihad-bs] notify: page-hold: {"editable":true,"readOnly":false,"selected":false} (pushed)
    WAIT ok 23-differs f
    CHECK ok 24-region-changed: region 13,314-756,519: colours 609 hash 4f7ab6f1 rows 205 uniform 0 light 326-444 blue none against the sample's hash ce1bdacc
    CHECK ok 25-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 30-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 32-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 34-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=1
    WAIT ok 35-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false

### enyo / flash-app-calculator — pass

    enyo card on http://192.168.1.245:8097/title/cyberchase-calculator: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job
    WAIT ok 10-rect m
    WAIT ok 11-still m@550,550:165,99,397,155
    WAIT ok 13-still m@550,550:165,99,397,155
    WAIT ok 18-still m@550,550:165,99,397,155
    CHECK ok 20-region-changed: region 165,181-397,237: colours 131 hash 175995c6 rows 56 uniform 0 light none blue 183-236 against the sample's hash 44702382
    WAIT ok 23-still m@550,550:165,99,397,155
    CHECK ok 24-region-equal: region 165,181-397,237: colours 131 hash 44702382 rows 56 uniform 0 light none blue 183-236 = the sample's hash
    WAIT ok 32-still m@550,550:165,99,397,155
    CHECK ok 33-region-equal: region 165,181-397,237: colours 131 hash 175995c6 rows 56 uniform 0 light none blue 183-236 = the sample's hash

### enyo / flash-app-chart — pass

    enyo card on http://192.168.1.245:8097/title/yui-charts: connected at +2 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    WAIT ok 11-rect m
    WAIT ok 12-text last = [ready]
    CHECK ok 13-flash-drawn: drawn: region 0,83-500,456: colours 107 hash 88958c2d rows 373 uniform 27 light 83-455 blue 140-427
    WAIT ok 15-text last = [click 2 Mar series 0]
    WAIT ok 17-text last = [click 4 May series 0]

### enyo / flash-app-clock — pass

    enyo card on http://192.168.1.245:8097/title/flash-clock: connected at +2 s, load done at +8 s, 11 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 9-rect m
    CHECK ok 10-flash-drawn: drawn: region 0,83-300,381: colours 1355 hash 32ddf3c0 rows 298 uniform 41 light 215-216 blue 101-364
    WAIT ok 12-differs m
    WAIT ok 14-differs m
    CHECK ok 15-region-changed: region 0,83-300,381: colours 1345 hash 57ebf8d7 rows 298 uniform 41 light 215-216 blue 101-364 against the sample's hash d1d62938

### enyo / flash-app-drums — pass

    enyo card on http://192.168.1.245:8097/title/drum-machine: connected at +3 s, load done at +8 s, 28 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 10-rect m
    WAIT ok 11-still m@900,450:60,300,260,430
    CHECK ok 12-flash-drawn: reported not drawn, as it should be: region 51,338-222,449: colours 1 hash a2e425f4 rows 111 uniform 111 light 338-448 blue none
    WAIT ok 20-differs m@900,450:60,300,260,430
    CHECK ok 21-sound-watched: sound at the output sink over the gesture: peak 32685 rms 3370 frames 397312 hz 521 env 508,367,32685,32013,17808,12645,5695,2402,1094,671,366,3994,5492,7350,7700,9238,9529,8169,7567,7881,6243,7646,5866,7352,4667,4268,4639,7869,6157,5979,7307,5629,6182,5963,7296,4921,3870,3856,2951,3312,3436,3117,2732,2164,2874,2363, the speaker amp on, the media volume 23
    CHECK ok 22-flash-drawn: drawn: region 51,338-222,449: colours 73 hash 86a060fd rows 111 uniform 37 light 375-448 blue none

### enyo / flash-app-form — pass

    enyo card on http://192.168.1.245:8097/title/website-viewer: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 0,83-640,561: colours 4096 hash e07ca3aa rows 478 uniform 0 light none blue none
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 18-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 19-still m
    CHECK ok 20-flash-drawn: drawn: region 0,83-640,561: colours 4096 hash edc73612 rows 478 uniform 0 light none blue none
    CHECK ok 21-url: load done uri=http://192.168.1.245:8097/title/website-viewer

### enyo / flash-app-paint — pass

    enyo card on http://192.168.1.245:8097/title/poster-paint-party: connected at +2 s, load done at +7 s, 10 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's
    WAIT ok 10-rect m
    WAIT ok 11-still m
    WAIT ok 13-still m
    WAIT ok 20-differs m@480,320:150,40,270,90
    CHECK ok 21-region-changed: region 150,122-270,172: colours 82 hash 7871ec3e rows 50 uniform 8 light 122-171 blue 127-158 against the sample's hash 42ffb962
    CHECK ok 22-region-equal: region 95,287-200,320: colours 5 hash ab012dda rows 33 uniform 21 light 287-319 blue none = the sample's hash

### enyo / flash-app-phonemic — pass

    enyo card on http://192.168.1.245:8097/title/phonemic-chart: connected at +2 s, load done at +8 s, 10 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 0,83-654,652: colours 3877 hash 6f903e29 rows 569 uniform 0 light 116-634 blue none
    CHECK ok 15-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    CHECK ok 18-sound-watched: sound at the output sink over the gesture: peak 7524 rms 213 frames 309248 hz 860 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7524,3677,824,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, the speaker amp on, the media volume 23
    CHECK ok 21-sound-watched: sound at the output sink over the gesture: peak 6688 rms 119 frames 309248 hz 2064 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,44,6688,2470,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, the speaker amp on, the media volume 23
    CHECK ok 24-sound-watched: sound at the output sink over the gesture: peak 1666 rms 132 frames 309248 hz 384 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1666,1638,508,9,0,0,0,0,0,0,0,0,0,0,0,0,0, the speaker amp on, the media volume 23
    CHECK ok 26-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

### enyo / flash-app-piano — pass

    enyo card on http://192.168.1.245:8097/title/virtual-piano: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 9-rect m
    WAIT ok 10-still m
    WAIT ok 12-still m
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    CHECK ok 15-sound: sound at the output sink: peak 19426 rms 1933 frames 45056 hz 609 env 0,0,1,19426,7372,5467, the speaker amp on, the media volume 23
    CHECK ok 17-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    CHECK ok 19-sound: sound at the output sink: peak 12626 rms 1747 frames 45056 hz 492 env 0,0,12626,9057,3514,1911, the speaker amp on, the media volume 23

### enyo / flash-app-postcards — pass

    enyo card on http://192.168.1.245:8097/title/postcard-viewer: connected at +2 s, load done at +7 s, 13 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 269,370-499,543: colours 258 hash ef79a669 rows 173 uniform 67 light 412-519 blue 413-458
    CHECK ok 14-flash-drawn: reported not drawn, as it should be: region 96,140-672,293: colours 1 hash 65667185 rows 153 uniform 153 light none blue none
    WAIT ok 17-differs m
    WAIT ok 18-still m
    CHECK ok 20-flash-drawn: drawn: region 96,140-672,293: colours 33 hash 9d62fcb9 rows 153 uniform 65 light 217-225 blue 140-216
    WAIT ok 23-differs m
    WAIT ok 24-still m
    CHECK ok 25-flash-drawn: reported not drawn, as it should be: region 96,140-672,293: colours 1 hash 65667185 rows 153 uniform 153 light none blue none
    CHECK ok 26-flash-drawn: drawn: region 269,370-499,543: colours 309 hash 0536a9b6 rows 173 uniform 67 light 412-519 blue 413-458

### enyo / flash-clipboard-out — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text&clip=1: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express l
    WAIT ok 16-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 19-text clip = [clip set]
    CHECK ok 20-clipboard: the system clipboard holds [jihad-prime-41]
    CHECK ok 22-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 24-line: [jihad-bs] hold at 224,385 editable=1 wasFocused=1
    WAIT ok 27-text pfv = [pf jihad-prime-41]
    CHECK ok 29-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 31-text file = [file browse false]
    WAIT ok 33-text file = [file download false]
    CHECK ok 34-quiet: no line jihad-dl\|dialog  for 4 s

### mochi / flash-app-calculator — pass

    mochi card on http://192.168.1.245:8097/title/cyberchase-calculator: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the jo
    WAIT ok 10-rect m
    WAIT ok 11-still m@550,550:165,99,397,155
    WAIT ok 13-still m@550,550:165,99,397,155
    WAIT ok 18-still m@550,550:165,99,397,155
    CHECK ok 20-region-changed: region 165,177-397,233: colours 128 hash 6a298bcf rows 56 uniform 0 light none blue 179-232 against the sample's hash fb4ff083
    WAIT ok 23-still m@550,550:165,99,397,155
    CHECK ok 24-region-equal: region 165,177-397,233: colours 128 hash fb4ff083 rows 56 uniform 0 light none blue 179-232 = the sample's hash
    WAIT ok 32-still m@550,550:165,99,397,155
    CHECK ok 33-region-equal: region 165,177-397,233: colours 128 hash 6a298bcf rows 56 uniform 0 light none blue 179-232 = the sample's hash

### mochi / flash-app-chart — pass

    mochi card on http://192.168.1.245:8097/title/yui-charts: connected at +1 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 11-rect m
    WAIT ok 12-text last = [ready]
    CHECK ok 13-flash-drawn: drawn: region 0,79-500,452: colours 107 hash 88958c2d rows 373 uniform 27 light 79-451 blue 136-423
    WAIT ok 15-text last = [click 2 Mar series 0]
    WAIT ok 17-text last = [click 4 May series 0]

### mochi / flash-app-clock — pass

    mochi card on http://192.168.1.245:8097/title/flash-clock: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 9-rect m
    CHECK ok 10-flash-drawn: drawn: region 0,79-300,377: colours 1343 hash a1be4eae rows 298 uniform 25 light 211-212 blue 97-360
    WAIT ok 12-differs m
    WAIT ok 14-differs m
    CHECK ok 15-region-changed: region 0,79-300,377: colours 1344 hash 4f3a20c0 rows 298 uniform 25 light 211-212 blue 97-360 against the sample's hash 9b4d8301

### mochi / flash-app-drums — pass

    mochi card on http://192.168.1.245:8097/title/drum-machine: connected at +1 s, load done at +7 s, 12 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 10-rect m
    WAIT ok 11-still m@900,450:60,300,260,430
    CHECK ok 12-flash-drawn: reported not drawn, as it should be: region 51,334-222,445: colours 1 hash a2e425f4 rows 111 uniform 111 light 334-444 blue none
    WAIT ok 20-differs m@900,450:60,300,260,430
    CHECK ok 21-sound-watched: sound at the output sink over the gesture: peak 9529 rms 1843 frames 397312 hz 818 env 3128,1520,962,495,3792,5465,7380,7169,9238,9529,8169,7567,7881,6243,7646,5866,7352,4667,3872,4639,7869,7155,5979,7307,5629,6182,5963,7296,4921,4377,3856,2951,3190,3436,3164,2841,2164,2874,4129,4945,4779,3745,2990,3627,2523,1799, the speaker amp on, the media volume 23
    CHECK ok 22-flash-drawn: drawn: region 51,334-222,445: colours 114 hash 6df0297f rows 111 uniform 37 light 371-444 blue none

### mochi / flash-app-form — pass

    mochi card on http://192.168.1.245:8097/title/website-viewer: connected at +3 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 0,79-640,557: colours 4096 hash e07ca3aa rows 478 uniform 0 light none blue none
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 18-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 19-still m
    CHECK ok 20-flash-drawn: drawn: region 0,79-640,557: colours 4096 hash c4460238 rows 478 uniform 0 light none blue none
    CHECK ok 21-url: load done uri=http://192.168.1.245:8097/title/website-viewer

### mochi / flash-app-paint — pass

    mochi card on http://192.168.1.245:8097/title/poster-paint-party: connected at +1 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's
    WAIT ok 10-rect m
    WAIT ok 11-still m
    WAIT ok 13-still m
    WAIT ok 20-differs m@480,320:150,40,270,90
    CHECK ok 21-region-changed: region 150,118-270,168: colours 86 hash 16c5703f rows 50 uniform 6 light 118-167 blue 123-154 against the sample's hash d0397bfa
    CHECK ok 22-region-equal: region 95,283-200,316: colours 11 hash c0bb1138 rows 33 uniform 21 light 283-315 blue none = the sample's hash

### mochi / flash-app-phonemic — pass

    mochi card on http://192.168.1.245:8097/title/phonemic-chart: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 0,79-654,648: colours 3877 hash 6f903e29 rows 569 uniform 0 light 112-630 blue none
    CHECK ok 15-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    CHECK ok 18-sound-watched: sound at the output sink over the gesture: peak 7524 rms 213 frames 309248 hz 860 env 0,0,0,0,0,0,0,0,0,0,0,0,0,7524,5745,1010,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, the speaker amp on, the media volume 23
    CHECK ok 21-sound-watched: sound at the output sink over the gesture: peak 6688 rms 119 frames 309248 hz 2064 env 0,0,0,0,0,0,0,0,0,0,0,0,0,301,6688,174,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, the speaker amp on, the media volume 23
    CHECK ok 24-sound-watched: sound at the output sink over the gesture: peak 1666 rms 132 frames 309248 hz 267 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1666,1649,508,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, the speaker amp on, the media volume 23
    CHECK ok 26-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

### mochi / flash-app-piano — pass

    mochi card on http://192.168.1.245:8097/title/virtual-piano: connected at +1 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 9-rect m
    WAIT ok 10-still m
    WAIT ok 12-still m
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    CHECK ok 15-sound: sound at the output sink: peak 19426 rms 1934 frames 45056 hz 607 env 0,0,2,19426,7372,5467, the speaker amp on, the media volume 23
    CHECK ok 17-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    CHECK ok 19-sound: sound at the output sink: peak 12626 rms 1712 frames 45056 hz 514 env 0,0,6,12626,5300,2755, the speaker amp on, the media volume 23

### mochi / flash-app-postcards — pass

    mochi card on http://192.168.1.245:8097/title/postcard-viewer: connected at +1 s, load done at +7 s, 14 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 269,366-499,539: colours 251 hash 098f815d rows 173 uniform 65 light 408-515 blue 409-454
    CHECK ok 14-flash-drawn: reported not drawn, as it should be: region 96,136-672,289: colours 1 hash 65667185 rows 153 uniform 153 light none blue none
    WAIT ok 17-differs m
    WAIT ok 18-still m
    CHECK ok 20-flash-drawn: drawn: region 96,136-672,289: colours 35 hash 670b63ec rows 153 uniform 44 light 213-221 blue none
    WAIT ok 23-differs m
    WAIT ok 24-still m
    CHECK ok 25-flash-drawn: reported not drawn, as it should be: region 96,136-672,289: colours 1 hash 65667185 rows 153 uniform 153 light none blue none
    CHECK ok 26-flash-drawn: drawn: region 269,366-499,539: colours 274 hash 02916fe5 rows 173 uniform 65 light 408-515 blue 409-454

### mochi / flash-clipboard-out — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text&clip=1: connected at +1 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the express
    WAIT ok 16-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 19-text clip = [clip set]
    CHECK ok 20-clipboard: the system clipboard holds [jihad-prime-41]
    CHECK ok 22-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 24-line: [jihad-bs] hold at 224,385 editable=1 wasFocused=1
    WAIT ok 27-text pfv = [pf jihad-prime-41]
    CHECK ok 29-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 31-text file = [file browse false]
    WAIT ok 33-text file = [file download false]
    CHECK ok 34-quiet: no line jihad-dl\|dialog  for 4 s

### mojo / flash-app-calculator — pass

    mojo card on http://192.168.1.245:8097/title/cyberchase-calculator: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job
    WAIT ok 10-rect m
    WAIT ok 11-still m@550,550:165,99,397,155
    WAIT ok 13-still m@550,550:165,99,397,155
    WAIT ok 18-still m@550,550:165,99,397,155
    CHECK ok 20-region-changed: region 165,172-397,228: colours 115 hash 708ae6c9 rows 56 uniform 0 light none blue 174-227 against the sample's hash 17d55502
    WAIT ok 23-still m@550,550:165,99,397,155
    CHECK ok 24-region-equal: region 165,172-397,228: colours 115 hash 17d55502 rows 56 uniform 0 light none blue 174-227 = the sample's hash
    WAIT ok 32-still m@550,550:165,99,397,155
    CHECK ok 33-region-equal: region 165,172-397,228: colours 115 hash 708ae6c9 rows 56 uniform 0 light none blue 174-227 = the sample's hash

### mojo / flash-app-chart — pass

    mojo card on http://192.168.1.245:8097/title/yui-charts: connected at +1 s, load done at +7 s, 15 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 11-rect m
    WAIT ok 12-text last = [ready]
    CHECK ok 13-flash-drawn: drawn: region 0,74-500,447: colours 96 hash 29da18c5 rows 373 uniform 27 light 74-446 blue 131-418
    WAIT ok 15-text last = [click 2 Mar series 0]
    WAIT ok 17-text last = [click 4 May series 0]

### mojo / flash-app-clock — pass

    mojo card on http://192.168.1.245:8097/title/flash-clock: connected at +1 s, load done at +6 s, 9 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 9-rect m
    CHECK ok 10-flash-drawn: drawn: region 0,74-300,372: colours 1105 hash a12afe2d rows 298 uniform 41 light 206-207 blue 92-355
    WAIT ok 12-differs m
    WAIT ok 14-differs m
    CHECK ok 15-region-changed: region 0,74-300,372: colours 1106 hash 84fe1e9b rows 298 uniform 41 light 206-207 blue 92-355 against the sample's hash 193b490a

### mojo / flash-app-drums — pass

    mojo card on http://192.168.1.245:8097/title/drum-machine: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 10-rect m
    WAIT ok 11-still m@900,450:60,300,260,430
    CHECK ok 12-flash-drawn: reported not drawn, as it should be: region 51,329-222,440: colours 1 hash a2e425f4 rows 111 uniform 111 light 329-439 blue none
    WAIT ok 20-differs m@900,450:60,300,260,430
    CHECK ok 21-sound-watched: sound at the output sink over the gesture: peak 7869 rms 1181 frames 397312 hz 950 env 3324,4639,4700,7869,5940,7307,6666,6182,5269,7296,4085,4921,3870,2951,2026,3312,3436,2841,2307,2432,3539,4129,4945,3864,3622,2990,3627,2523,1815,1694,1505,1171,801,570,372,185,154,160,92,69,50,28,19,16,12,7, the speaker amp on, the media volume 23
    CHECK ok 22-flash-drawn: drawn: region 51,329-222,440: colours 49 hash 65c12f83 rows 111 uniform 37 light 366-439 blue none

### mojo / flash-app-form — pass

    mojo card on http://192.168.1.245:8097/title/website-viewer: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 0,74-640,552: colours 4096 hash 82825cf1 rows 478 uniform 0 light none blue none
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 18-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 19-still m
    CHECK ok 20-flash-drawn: drawn: region 0,74-640,552: colours 4096 hash 80ac146e rows 478 uniform 0 light none blue none
    CHECK ok 21-url: load done uri=http://192.168.1.245:8097/title/website-viewer

### mojo / flash-app-paint — pass

    mojo card on http://192.168.1.245:8097/title/poster-paint-party: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's 
    WAIT ok 10-rect m
    WAIT ok 11-still m
    WAIT ok 13-still m
    WAIT ok 20-differs m@480,320:150,40,270,90
    CHECK ok 21-region-changed: region 150,113-270,163: colours 56 hash 272829a5 rows 50 uniform 8 light 113-162 blue 118-149 against the sample's hash 1b151848
    CHECK ok 22-region-equal: region 95,278-200,311: colours 5 hash ab012dda rows 33 uniform 21 light 278-310 blue none = the sample's hash

### mojo / flash-app-phonemic — pass

    mojo card on http://192.168.1.245:8097/title/phonemic-chart: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 0,74-654,643: colours 3117 hash ed71f967 rows 569 uniform 0 light 107-625 blue none
    CHECK ok 15-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    CHECK ok 18-sound-watched: sound at the output sink over the gesture: peak 7524 rms 213 frames 309248 hz 1459 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7524,3677,824,12,0,0,0,0,0,0,0,0,0,0,0,0,0, the speaker amp on, the media volume 23
    CHECK ok 21-sound-watched: sound at the output sink over the gesture: peak 6688 rms 119 frames 309248 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,151,6688,199,0,0,0,0,0,0,0,0,0,0,0,0,0, the speaker amp on, the media volume 23
    CHECK ok 24-sound-watched: sound at the output sink over the gesture: peak 1666 rms 132 frames 309248 hz 529 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1666,1651,1112,134,0,0,0,0,0,0,0,0,0,0,0,0,0, the speaker amp on, the media volume 23
    CHECK ok 26-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

### mojo / flash-app-piano — pass

    mojo card on http://192.168.1.245:8097/title/virtual-piano: connected at +1 s, load done at +7 s, 10 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 9-rect m
    WAIT ok 10-still m
    WAIT ok 12-still m
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    CHECK ok 15-sound: sound at the output sink: peak 19426 rms 1974 frames 45056 hz 562 env 0,0,19426,14533,7372,4144, the speaker amp on, the media volume 23
    CHECK ok 17-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    CHECK ok 19-sound: sound at the output sink: peak 12626 rms 1761 frames 45056 hz 479 env 0,0,12626,6643,3333,1371, the speaker amp on, the media volume 23

### mojo / flash-app-postcards — pass

    mojo card on http://192.168.1.245:8097/title/postcard-viewer: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 269,361-499,534: colours 147 hash b1fa561e rows 173 uniform 67 light 403-510 blue none
    CHECK ok 14-flash-drawn: reported not drawn, as it should be: region 96,131-672,284: colours 1 hash 65667185 rows 153 uniform 153 light none blue none
    WAIT ok 17-differs m
    WAIT ok 18-still m
    CHECK ok 20-flash-drawn: drawn: region 96,131-672,284: colours 11 hash 6acb8ae8 rows 153 uniform 58 light 208-216 blue 131-207
    WAIT ok 23-differs m
    WAIT ok 24-still m
    CHECK ok 25-flash-drawn: reported not drawn, as it should be: region 96,131-672,284: colours 1 hash 65667185 rows 153 uniform 153 light none blue none
    CHECK ok 26-flash-drawn: drawn: region 269,361-499,534: colours 256 hash 7d7798ab rows 173 uniform 67 light 403-510 blue none

### mojo / flash-clipboard-out — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text&clip=1: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express l
    WAIT ok 16-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 19-text clip = [clip set]
    CHECK ok 20-clipboard: the system clipboard holds [jihad-prime-41]
    CHECK ok 22-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 24-line: [jihad-bs] hold at 224,385 editable=1 wasFocused=1
    WAIT ok 27-text pfv = [pf jihad-prime-41]
    CHECK ok 29-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 31-text file = [file browse false]
    WAIT ok 33-text file = [file download false]
    CHECK ok 34-quiet: no line jihad-dl\|dialog  for 4 s

### enyo / flash-key-modifiers — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text: connected at +2 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 15-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 20-text text = [text a]
    WAIT ok 23-text text = [text aB]
    WAIT ok 24-line: [jihad-bs] key: the modifier mask 0x80 read as 0x4
    WAIT ok 26-text keydown = [keydown 17 17]
    WAIT ok 28-text keydown = [keydown 86 118]
    WAIT ok 29-line: [jihad-bs] key: the modifier mask 0x20 read as 0x1
    CHECK ok 30-text-has: text = [text aBv] holds [v]
    CHECK ok 31-text-lacks: text = [text aBv] lacks [prime]
    CHECK ok 32-clipboard: the system clipboard holds [jihad-prime-42]

### mochi / flash-key-modifiers — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text: connected at +3 s, load done at +8 s, 11 content frames at +8 s; the GPU path, frames off the main thread (the express lane) 
    WAIT ok 15-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 20-text text = [text a]
    WAIT ok 23-text text = [text aB]
    WAIT ok 24-line: [jihad-bs] key: the modifier mask 0x80 read as 0x4
    WAIT ok 26-text keydown = [keydown 17 17]
    WAIT ok 28-text keydown = [keydown 86 118]
    WAIT ok 29-line: [jihad-bs] key: the modifier mask 0x20 read as 0x1
    CHECK ok 30-text-has: text = [text aBv] holds [v]
    CHECK ok 31-text-lacks: text = [text aBv] lacks [prime]
    CHECK ok 32-clipboard: the system clipboard holds [jihad-prime-42]

### mojo / flash-key-modifiers — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 15-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 20-text text = [text a]
    WAIT ok 23-text text = [text aB]
    WAIT ok 24-line: [jihad-bs] key: the modifier mask 0x80 read as 0x4
    WAIT ok 26-text keydown = [keydown 17 17]
    WAIT ok 28-text keydown = [keydown 86 118]
    WAIT ok 29-line: [jihad-bs] key: the modifier mask 0x20 read as 0x1
    CHECK ok 30-text-has: text = [text aBv] holds [v]
    CHECK ok 31-text-lacks: text = [text aBv] lacks [prime]
    CHECK ok 32-clipboard: the system clipboard holds [jihad-prime-42]

### enyo / keyboard-modifiers — pass

    enyo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 11-rect plain
    calibrated (enyo): the page's top on the screen 83 (the table said 82, the tap at y 143 landed on document y 77)
    CHECK ok 14-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 20-text echo = [plain n=1 c=0 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 22-text echo = [plain n=1 c=1 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=0]
    WAIT ok 24-text echo = [plain n=1 c=1 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]

### mochi / keyboard-modifiers — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 11-rect plain
    calibrated (mochi): the page's top on the screen 79 (the table said 78, the tap at y 139 landed on document y 77)
    CHECK ok 14-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 20-text echo = [plain n=1 c=0 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 22-text echo = [plain n=1 c=1 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=0]
    WAIT ok 24-text echo = [plain n=1 c=1 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]

### mojo / keyboard-modifiers — pass

    mojo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 11-rect plain
    calibrated (mojo): the page's top on the screen 74 (the table said 73, the tap at y 134 landed on document y 77)
    CHECK ok 14-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 20-text echo = [plain n=1 c=0 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 22-text echo = [plain n=1 c=1 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=0]
    WAIT ok 24-text echo = [plain n=1 c=1 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]

### enyo / audio-heard — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +2 s, load done at +8 s, 18 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31144 rms 21986 frames 45056 hz 440 env 31130,31134,31137,31144,31137,31131, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    WAIT ok 13-text status = [playing]
    CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2951,2948,2956,2935,2950,2913, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### enyo / autoplay-sound — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +2 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread 
    WAIT ok 8-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 10-text status = [play refused NotAllowedError]
    WAIT ok 13-text status = [playing]

### enyo / flash-app-mp3 — pass

    enyo card on http://192.168.1.245:8097/title/mp3-player-multi: connected at +2 s, load done at +7 s, 14 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect m
    WAIT ok 10-still m
    CHECK ok 12-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    CHECK ok 15-sound-watched: sound at the output sink over the gesture: peak 1820 rms 191 frames 353280 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,289,502,509,265,1131,1757,1746,1761,1203,1257,919,1023,1537,1011,1155,750,820,959,1820,1196,1694,224, the speaker amp on, the media volume 23
    CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

### enyo / flash-game-sound — pass

    enyo card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +2 s, load done at +10 s, 13 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's 
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

### enyo / flash-lane — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +3 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off th
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,123-440,421: 3.0 s, flips 41 (13.7/s), changes 39 (13.0/s), reads 43, distinct 40, spacing median 62 p90 142 max 194 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,123-440,421: 3.0 s, flips 42 (14.0/s), changes 0 (0.0/s), reads 43, distinct 1, spacing median 67 p90 129 max 173 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,123-440,421: 3.0 s, flips 41 (13.7/s), changes 39 (13.0/s), reads 43, distinct 40, spacing median 69 p90 103 max 207 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 138
    CHECK ok 16-flash-rate: content 40,0-440,284: 3.0 s, flips 45 (15.0/s), changes 40 (13.3/s), reads 46, distinct 41, spacing median 67 p90 85 max 150 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,284: 3.0 s, flips 41 (13.7/s), changes 0 (0.0/s), reads 43, distinct 1, spacing median 67 p90 162 max 222 ms (wanted 0..4/s)

### enyo / flash-sound — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +2 s, load done at +9 s, 13 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31141 rms 21984 frames 45056 hz 440 env 31138,31133,31133,31137,31132,31141, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    WAIT ok 14-launch: load done
    WAIT ok 15-text status = [playing]
    CHECK ok 16-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2937,2951,2956,2939,2950,2924, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### enyo / volume-keys — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +2 s, load done at +8 s, 18 content frames at +8 s; the GPU path, frames off the main thread
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2952 rms 2058 frames 45056 hz 440 env 2952,2943,2950,2938,2948,2946, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume-hud: the window server raised its alert for the key (syslog: void LockWindow::slotTransientAlertActivated(): has displayable content); it draws no HUD in the frame buffer on this build
    CHECK ok 17-volume: the media volume 66, higher (marked 44)
    CHECK ok 18-volume: the codec's output level 41 of 63 for the media volume 66
    CHECK ok 19-sound: sound at the output sink: peak 2956 rms 2057 frames 45056 hz 440 env 2941,2939,2956,2938,2942,2937, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 22-volume: the media volume 44, lower (marked 66)
    CHECK ok 23-volume: the codec's output level 27 of 63 for the media volume 44
    CHECK ok 24-sound: sound at the output sink: peak 2963 rms 2059 frames 45056 hz 440 env 2943,2945,2963,2963,2941,2920, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 28-volume: the media volume 0, zero (marked 66)
    CHECK ok 29-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2955 rms 2059 frames 45056 hz 440 env 2948,2953,2955,2949,2952,2941): no sound out
    CHECK ok 32-volume: the media volume 22, higher (marked 0)
    CHECK ok 33-sound: sound at the output sink: peak 2956 rms 2059 frames 45056 hz 440 env 2938,2946,2956,2944,2950,2933, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

### mochi / audio-heard — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +4 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31139 rms 21985 frames 45056 hz 440 env 31138,31133,31134,31139,31130,31127, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    WAIT ok 13-text status = [playing]
    CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2951,2956,2939,2950,2955,2919, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mochi / flash-app-mp3 — pass

    mochi card on http://192.168.1.245:8097/title/mp3-player-multi: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect m
    WAIT ok 10-still m
    CHECK ok 12-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    CHECK ok 15-sound-watched: sound at the output sink over the gesture: peak 1761 rms 178 frames 353280 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,502,509,265,1131,1757,1746,1761,1531,1033,1257,930,1537,1106,1155,1052,820,959,694,46, the speaker amp on, the media volume 23
    CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

### mochi / flash-game-sound — pass

    mochi card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +1 s, load done at +8 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

### mochi / flash-lane — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +1 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off t
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,119-440,417: 3.0 s, flips 44 (14.7/s), changes 40 (13.3/s), reads 45, distinct 41, spacing median 67 p90 94 max 182 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,119-440,417: 3.0 s, flips 43 (14.3/s), changes 0 (0.0/s), reads 44, distinct 1, spacing median 67 p90 85 max 147 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,119-440,417: 3.0 s, flips 43 (14.3/s), changes 42 (14.0/s), reads 45, distinct 43, spacing median 66 p90 91 max 213 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 138
    CHECK ok 16-flash-rate: content 40,0-440,280: 3.0 s, flips 43 (14.3/s), changes 28 (9.3/s), reads 44, distinct 29, spacing median 68 p90 82 max 159 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,280: 3.0 s, flips 39 (13.0/s), changes 0 (0.0/s), reads 41, distinct 1, spacing median 68 p90 147 max 210 ms (wanted 0..4/s)

### mochi / flash-sound — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +2 s, load done at +9 s, 17 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31139 rms 21985 frames 45056 hz 440 env 31134,31135,31139,31130,31134,31126, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    WAIT ok 14-launch: load done
    WAIT ok 15-text status = [playing]
    CHECK ok 16-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2942,2951,2948,2956,2935,2950, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mochi / volume-keys — pass

    mochi card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +1 s, load done at +7 s, 14 content frames at +7 s; the GPU path, frames off the main threa
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2955 rms 2058 frames 45056 hz 440 env 2950,2938,2948,2955,2949,2939, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume-hud: the window server raised its alert for the key (syslog: void LockWindow::slotTransientAlertActivated(): has displayable content); it draws no HUD in the frame buffer on this build
    CHECK ok 17-volume: the media volume 66, higher (marked 44)
    CHECK ok 18-volume: the codec's output level 41 of 63 for the media volume 66
    CHECK ok 19-sound: sound at the output sink: peak 2958 rms 2057 frames 45056 hz 440 env 2938,2942,2958,2950,2956,2923, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 22-volume: the media volume 44, lower (marked 66)
    CHECK ok 23-volume: the codec's output level 27 of 63 for the media volume 44
    CHECK ok 24-sound: sound at the output sink: peak 2963 rms 2058 frames 45056 hz 440 env 2963,2941,2953,2940,2938,2943, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 28-volume: the media volume 0, zero (marked 66)
    CHECK ok 29-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2958 rms 2059 frames 45056 hz 440 env 2949,2952,2946,2958,2944,2941): no sound out
    CHECK ok 32-volume: the media volume 22, higher (marked 0)
    CHECK ok 33-sound: sound at the output sink: peak 2954 rms 2059 frames 45056 hz 440 env 2943,2950,2954,2936,2949,2949, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

### mojo / audio-heard — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +1 s, load done at +6 s, 14 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31141 rms 21985 frames 45056 hz 440 env 31133,31133,31137,31141,31139,31127, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    WAIT ok 13-text status = [playing]
    CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2951,2956,2939,2950,2955,2919, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mojo / flash-app-mp3 — pass

    mojo card on http://192.168.1.245:8097/title/mp3-player-multi: connected at +1 s, load done at +6 s, 17 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect m
    WAIT ok 10-still m
    CHECK ok 12-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    CHECK ok 15-sound-watched: sound at the output sink over the gesture: peak 1820 rms 208 frames 353280 hz 379 env 0,0,0,0,0,0,0,0,0,0,0,0,0,289,502,509,265,1131,1757,1746,1761,1203,1257,919,1023,1537,1106,1155,1052,820,959,1820,1196,1694,416,151,1127,1126,1068,688,121, the speaker amp on, the media volume 23
    CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

### mojo / flash-game-sound — pass

    mojo card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +2 s, load done at +8 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

### mojo / flash-lane — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off th
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,114-440,412: 3.0 s, flips 39 (13.0/s), changes 38 (12.7/s), reads 40, distinct 39, spacing median 68 p90 144 max 200 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,114-440,412: 3.0 s, flips 41 (13.7/s), changes 0 (0.0/s), reads 42, distinct 1, spacing median 66 p90 155 max 202 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,114-440,412: 3.0 s, flips 44 (14.7/s), changes 42 (14.0/s), reads 45, distinct 43, spacing median 66 p90 77 max 154 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 41
    CHECK ok 16-flash-rate: content 40,29-440,327: 3.0 s, flips 44 (14.7/s), changes 42 (14.0/s), reads 45, distinct 43, spacing median 67 p90 81 max 146 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,29-440,327: 3.0 s, flips 40 (13.3/s), changes 0 (0.0/s), reads 41, distinct 1, spacing median 68 p90 150 max 176 ms (wanted 0..4/s)

### mojo / flash-sound — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +3 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31144 rms 21987 frames 45056 hz 440 env 31144,31139,31133,31135,31134,31124, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    WAIT ok 14-launch: load done
    WAIT ok 15-text status = [playing]
    CHECK ok 16-sound: sound at the output sink: peak 2957 rms 2056 frames 45056 hz 440 env 2955,2938,2951,2957,2944,2935, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mojo / volume-keys — pass

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +1 s, load done at +7 s, 26 content frames at +7 s; the GPU path, frames off the main thread
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2958 rms 2058 frames 45056 hz 440 env 2947,2949,2939,2946,2958,2932, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume-hud: the window server raised its alert for the key (syslog: void LockWindow::slotTransientAlertActivated(): has displayable content); it draws no HUD in the frame buffer on this build
    CHECK ok 17-volume: the media volume 66, higher (marked 44)
    CHECK ok 18-volume: the codec's output level 41 of 63 for the media volume 66
    CHECK ok 19-sound: sound at the output sink: peak 2964 rms 2060 frames 45056 hz 440 env 2955,2964,2937,2960,2943,2918, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 22-volume: the media volume 44, lower (marked 66)
    CHECK ok 23-volume: the codec's output level 27 of 63 for the media volume 44
    CHECK ok 24-sound: sound at the output sink: peak 2953 rms 2059 frames 45056 hz 440 env 2946,2953,2951,2950,2947,2931, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 28-volume: the media volume 0, zero (marked 66)
    CHECK ok 29-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2958 rms 2058 frames 45056 hz 440 env 2938,2949,2954,2958,2938,2942): no sound out
    CHECK ok 32-volume: the media volume 22, higher (marked 0)
    CHECK ok 33-sound: sound at the output sink: peak 2955 rms 2059 frames 45056 hz 440 env 2950,2948,2946,2955,2949,2935, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

### enyo / flash-video-controls — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/harrypotter-vp6.flv%26seek=40: connected at +4 s, load done at +9 s, 6 content frame
    WAIT ok 13-text vmeta = [vmeta codec=4 w=480 h=272 d=86.833]
    WAIT ok 14-text video = [video t=3.134 dropped=19 fps=26.31578947368421]
    WAIT ok 15-content m
    CHECK ok 16-sound: sound at the output sink: peak 2065 rms 369 frames 45056 hz 159 env 870,735,1292,1132,2065,1248, the speaker amp on, the media volume 23
    WAIT ok 18-text vctl = [vctl pause t=9.822]
    WAIT ok 19-still m
    CHECK ok 21-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    CHECK ok 22-region-near: the picture against the sample paused: difference 0.1 (2 or less)
    WAIT ok 24-text vctl = [vctl resume t=9.822]
    WAIT ok 25-differs m
    CHECK ok 26-sound: sound at the output sink: peak 4231 rms 1005 frames 45056 hz 218 env 2234,3263,3734,4231,2948,2747, the speaker amp on, the media volume 23
    WAIT ok 29-text vctl = [vctl seek 40 t=21.708]
    WAIT ok 30-differs m
    WAIT ok 31-text video = [video t=43.023 dropped=92 fps=19.9203187250996]
    CHECK ok 32-number: vtime = 45.714 (wanted 38..58)
    WAIT ok 33-text vend = [vend NetStream.Play.Stop t=86.833 after video t=86.778 dropped=328 fps=23.357664233576642]
    CHECK ok 34-number: vendtime = 86.778 (wanted 84.3..87.3)
    CHECK ok 35-plugin: plugin-container 7664: alive (state S, cpu ticks 2275 to 2286)

### enyo / flash-video-flowplayer — pass

    enyo card on http://192.168.1.245:8097/title/i004-vp6a: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 10-rect m
    WAIT ok 11-content m
    CHECK ok 12-flash-rate: content 0,83-478,511: 5.0 s, flips 66 (13.2/s), changes 66 (13.2/s), reads 68, distinct 67, spacing median 67 p90 92 max 361 ms (wanted 5..40/s)
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-drawn: drawn: region 0,83-478,511: colours 3069 hash 45f157a0 rows 428 uniform 95 light 83-510 blue none
    CHECK ok 15-flash-rate: content 0,83-478,511: 3.0 s, flips 46 (15.3/s), changes 46 (15.3/s), reads 47, distinct 47, spacing median 67 p90 74 max 81 ms (wanted 5..40/s)
    WAIT ok 16-still m
    CHECK ok 17-flash-drawn: drawn: region 0,83-478,511: colours 4096 hash 6b8e994c rows 428 uniform 103 light 83-483 blue none
    CHECK ok 18-plugin: plugin-container 12829: alive (state S, cpu ticks 814 to 828)

### enyo / flash-video-h264-240 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-240.flv: connected at +2 s, load done at +7 s, 5 content frames at +7 s; th
    WAIT ok 13-text vmeta = [vmeta codec=7 w=426 h=240 d=20.025]
    WAIT ok 14-text video = [video t=1.067 dropped=0 fps=20.775623268698062]
    CHECK ok 15-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 16-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 81a8ea81 rows 298 uniform 0 light none blue 187-206
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3898,3897,3896,3897,3897,3893, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,123-440,421: 10.0 s, flips 226 (22.6/s), changes 187 (18.7/s), reads 227, distinct 188, spacing median 38 p90 77 max 191 ms (wanted 11.2..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash e43e58d7 rows 298 uniform 0 light none blue none
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.122 dropped=37 fps=22.624434389140273]
    CHECK ok 23-number: vendtime = 19.122 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 37 (wanted 0..288)
    CHECK ok 25-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 63582f90 rows 298 uniform 0 light none blue none
    CHECK ok 26-plugin: plugin-container 16425: alive (state S, cpu ticks 1044 to 1056)

### enyo / flash-video-h264-360 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-360.flv: connected at +2 s, load done at +8 s, 8 content frames at +8 s; th
    WAIT ok 13-text vmeta = [vmeta codec=7 w=640 h=360 d=20.025]
    WAIT ok 14-text video = [video t=2.116 dropped=11 fps=10.504201680672269]
    CHECK ok 15-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 16-flash-drawn: drawn: region 40,123-440,421: colours 3782 hash eee45459 rows 298 uniform 0 light none blue 123-420 (TORN)
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3896 rms 2750 frames 45056 hz 440 env 3896,3895,3895,3896,3896,3894, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,123-440,421: 10.0 s, flips 265 (26.5/s), changes 192 (19.2/s), reads 266, distinct 193, spacing median 29 p90 66 max 197 ms (wanted 13.4..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,123-440,421: colours 3809 hash 7d43f379 rows 298 uniform 0 light none blue 123-420
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.275 dropped=65 fps=26.08695652173913]
    CHECK ok 23-number: vendtime = 19.275 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 65 (wanted 0..288)
    CHECK ok 25-flash-drawn: drawn: region 40,123-440,421: colours 3867 hash a9c9b9af rows 298 uniform 0 light none blue 123-420
    CHECK ok 26-plugin: plugin-container 19939: alive (state S, cpu ticks 953 to 965)

### enyo / flash-video-h264-480 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-480.flv: connected at +2 s, load done at +8 s, 8 content frames at +8 s; th
    WAIT ok 13-text vmeta = [vmeta codec=7 w=854 h=480 d=20.025]
    WAIT ok 14-text video = [video t=2.037 dropped=8 fps=14.6484375]
    CHECK ok 15-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 16-flash-drawn: drawn: region 40,123-440,421: colours 3193 hash 4a96b328 rows 298 uniform 0 light none blue 123-420
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3898,3897,3896,3896,3895,3895, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,123-440,421: 10.0 s, flips 204 (20.4/s), changes 175 (17.5/s), reads 208, distinct 176, spacing median 38 p90 80 max 254 ms (wanted 9.3..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,123-440,421: colours 3391 hash 0f6afb65 rows 298 uniform 0 light none blue 123-420
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=20.01 after video t=20.01 dropped=83 fps=24.077046548956663]
    CHECK ok 23-number: vendtime = 20.01 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 83 (wanted 0..288)
    CHECK ok 25-flash-drawn: drawn: region 40,123-440,421: colours 3495 hash e242d890 rows 298 uniform 0 light none blue 123-420
    CHECK ok 26-plugin: plugin-container 23340: alive (state S, cpu ticks 1154 to 1166)

### enyo / flash-video-h264-mp4 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/bbb-h264-360.mp4: connected at +2 s, load done at +7 s, 3 content frames at +7 s; th
    WAIT ok 13-text vmeta = [vmeta codec=avc1 w=640 h=360 d=10]
    WAIT ok 14-text video = [video t=0.967 dropped=8 fps=0]
    CHECK ok 15-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 16-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash ebfbee0b rows 298 uniform 0 light none blue none
    CHECK ok 17-flash-rate: content 40,123-440,421: 5.0 s, flips 141 (28.2/s), changes 100 (20.0/s), reads 143, distinct 101, spacing median 26 p90 54 max 217 ms (wanted 7.5..40/s)
    CHECK ok 18-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 48d56f1b rows 298 uniform 0 light none blue none
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=10 after video t=9.8 dropped=25 fps=48.07692307692308]
    CHECK ok 20-number: vendtime = 9.8 (wanted 7.5..10.5)
    CHECK ok 21-number: venddropped = 25 (wanted 0..180)
    CHECK ok 22-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 48d56f1b rows 298 uniform 0 light none blue none
    CHECK ok 23-plugin: plugin-container 26932: alive (state S, cpu ticks 563 to 571)

### enyo / flash-video-mediaelement — pass

    enyo card on http://192.168.1.245:8097/title/bbb-h264-360: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 10-rect m
    WAIT ok 11-content m
    CHECK ok 12-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 13-flash-rate: content 0,83-640,441: 5.0 s, flips 170 (34.0/s), changes 113 (22.6/s), reads 171, distinct 114, spacing median 24 p90 34 max 192 ms (wanted 5..40/s)
    CHECK ok 14-flash-drawn: drawn: region 0,83-640,441: colours 4096 hash 561b5711 rows 358 uniform 0 light none blue none
    WAIT ok 15-still m
    CHECK ok 16-flash-drawn: drawn: region 0,83-640,441: colours 4096 hash 561b5711 rows 358 uniform 0 light none blue none
    CHECK ok 17-plugin: plugin-container 30265: alive (state S, cpu ticks 617 to 625)

### enyo / flash-video-slow — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/slow/40000/500000/10/video/gen-spark-240.flv: connected at +2 s, load done at +7 s, 6 cont
    WAIT ok 14-text vmeta = [vmeta codec=2 w=426 h=240 d=20.025]
    WAIT ok 15-text video = [video t=0.025 dropped=0 fps=0]
    CHECK ok 16-flash-drawn: drawn: region 40,123-440,421: colours 3906 hash 28d6e5f7 rows 298 uniform 0 light none blue 198-217
    WAIT ok 17-text vstatus = [vstatus NetStream.Buffer.Empty t=4.023]
    CHECK ok 18-plugin: plugin-container 1020: alive (state S, cpu ticks 213 to 229)
    WAIT ok 19-text vstatus = [vstatus NetStream.Buffer.Full t=4.023]
    WAIT ok 21-differs m
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.722 dropped=15 fps=23.25581395348837]
    CHECK ok 23-number: vmaxgap = 16760 (wanted 8000..30000)
    CHECK ok 24-number: vgapat = 4.023 (wanted 4..16)
    CHECK ok 25-number: vendtime = 19.722 (wanted 17.5..20.5)
    CHECK ok 26-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash c2e9e7b1 rows 298 uniform 0 light none blue none
    CHECK ok 27-plugin: plugin-container 1020: alive (state S, cpu ticks 707 to 717)

### enyo / flash-video-spark-240 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-240.flv: connected at +2 s, load done at +7 s, 6 content frames at +7 s; t
    WAIT ok 13-text vmeta = [vmeta codec=2 w=426 h=240 d=20.025]
    WAIT ok 14-text video = [video t=3.344 dropped=14 fps=22.058823529411764]
    CHECK ok 15-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 4bd60fbc rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3896 rms 2750 frames 45056 hz 440 env 3896,3895,3895,3896,3896,3893, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,123-440,421: 10.0 s, flips 200 (20.0/s), changes 195 (19.5/s), reads 202, distinct 196, spacing median 41 p90 83 max 223 ms (wanted 10.3..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 64dfa9ac rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=20.01 after video t=19.304 dropped=52 fps=24.232633279483036]
    CHECK ok 22-number: vendtime = 19.304 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 52 (wanted 0..288)
    CHECK ok 24-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 53017963 rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 5378: alive (state S, cpu ticks 585 to 597)

### enyo / flash-video-spark-360 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-360.flv: connected at +2 s, load done at +7 s, 8 content frames at +7 s; t
    WAIT ok 13-text vmeta = [vmeta codec=2 w=640 h=360 d=20.025]
    WAIT ok 14-text video = [video t=3.526 dropped=27 fps=22.008253094910593]
    CHECK ok 15-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 7aa9f516 rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3897 rms 2750 frames 45056 hz 440 env 3896,3896,3897,3895,3896,3893, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,123-440,421: 10.0 s, flips 192 (19.2/s), changes 189 (18.9/s), reads 194, distinct 190, spacing median 43 p90 91 max 227 ms (wanted 12.8..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 70587089 rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.957 dropped=86 fps=13.250883392226148]
    CHECK ok 22-number: vendtime = 19.957 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 86 (wanted 0..288)
    CHECK ok 24-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 59fc9a0b rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 8871: alive (state S, cpu ticks 681 to 691)

### enyo / flash-video-spark-480 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-480.flv: connected at +2 s, load done at +8 s, 11 content frames at +8 s; 
    WAIT ok 13-text vmeta = [vmeta codec=2 w=854 h=480 d=20.025]
    WAIT ok 14-text video = [video t=3.971 dropped=38 fps=17.921146953405017]
    CHECK ok 15-flash-drawn: drawn: region 40,123-440,421: colours 4020 hash 44fccd81 rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3897 rms 2750 frames 45056 hz 440 env 3896,3897,3895,3896,3895,3893, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,123-440,421: 10.0 s, flips 192 (19.2/s), changes 187 (18.7/s), reads 194, distinct 188, spacing median 42 p90 90 max 290 ms (wanted 11.5..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash b3689f88 rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.67 dropped=99 fps=24.72952086553323]
    CHECK ok 22-number: vendtime = 19.67 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 99 (wanted 0..288)
    CHECK ok 24-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 3d63abdf rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 12326: alive (state S, cpu ticks 821 to 833)

### enyo / flash-video-spark-sheep — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/sheep-spark.flv: connected at +2 s, load done at +8 s, 9 content frames at +8 s; the
    WAIT ok 13-text video = [video t=1.6 dropped=22 fps=7.865757734661773]
    CHECK ok 14-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 44b2a20e rows 298 uniform 0 light 123-234 blue none
    CHECK ok 15-audio: the movie's audio: open running audible
    CHECK ok 16-flash-rate: content 40,123-440,421: 5.0 s, flips 50 (10.0/s), changes 37 (7.4/s), reads 54, distinct 35, spacing median 79 p90 188 max 259 ms (wanted 6.9..40/s)
    CHECK ok 17-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 70252f20 rows 298 uniform 0 light 123-148 blue none
    CHECK ok 18-audio: the movie's audio: open running audible
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=8.725 after video t=8.62 dropped=102 fps=16.556291390728475]
    CHECK ok 20-number: vendtime = 8.62 (wanted 6.3..9.3)
    CHECK ok 21-number: venddropped = 102 (wanted 0..154)
    CHECK ok 22-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 7f2a41df rows 298 uniform 0 light 123-148 blue none
    CHECK ok 23-plugin: plugin-container 15681: alive (state S, cpu ticks 325 to 336)

### enyo / flash-video-spark-small — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/zelda-spark.flv: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the
    WAIT ok 13-text video = [video t=2.972 dropped=7 fps=9.931719428926133]
    CHECK ok 14-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash c71f8a78 rows 298 uniform 0 light 123-139 blue none
    CHECK ok 15-audio: the movie's audio: open running audible
    CHECK ok 16-flash-rate: content 40,123-440,421: 10.0 s, flips 125 (12.5/s), changes 125 (12.5/s), reads 128, distinct 126, spacing median 77 p90 100 max 232 ms (wanted 7.7..40/s)
    CHECK ok 17-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 5eeb39d2 rows 298 uniform 0 light none blue none
    CHECK ok 18-audio: the movie's audio: open running audible
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=29.75 after video t=29.535 dropped=20 fps=12.057877813504824]
    CHECK ok 20-number: vendtime = 29.535 (wanted 27.2..30.2)
    CHECK ok 21-number: venddropped = 20 (wanted 0..212)
    CHECK ok 22-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash b83d2678 rows 298 uniform 0 light 245-420 blue none
    CHECK ok 23-plugin: plugin-container 19026: alive (state S, cpu ticks 442 to 453)

### enyo / flash-video-vp6-176 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/mrandmrs-vp6.flv%26seek=60: connected at +3 s, load done at +8 s, 6 content frames a
    WAIT ok 10-text vmeta = [vmeta codec=4 w=320 h=176 d=151.125]
    WAIT ok 11-text video = [video t=3.709 dropped=10 fps=22.970903522205205]
    WAIT ok 12-content m
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-rate: content 40,123-440,421: 10.0 s, flips 213 (21.3/s), changes 212 (21.2/s), reads 215, distinct 213, spacing median 42 p90 75 max 212 ms (wanted 11.0..40/s)
    CHECK ok 15-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 0feb6558 rows 298 uniform 0 light none blue none
    WAIT ok 18-text vctl = [vctl seek 60 t=27.951]
    WAIT ok 19-differs m
    WAIT ok 20-text video = [video t=67.343 dropped=38 fps=23.65930599369085]
    CHECK ok 21-number: vtime = 68.362 (wanted 58..78)
    CHECK ok 22-audio: the movie's audio: open running audible
    WAIT ok 23-text vend = [vend NetStream.Play.Stop t=151.125 after video t=150.831 dropped=123 fps=25.078369905956112]
    CHECK ok 24-number: vendtime = 150.831 (wanted 148.6..151.6)
    CHECK ok 25-plugin: plugin-container 22818: alive (state S, cpu ticks 2829 to 2840)

### enyo / flash-video-vp6-272 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/harrypotter-vp6.flv%26seek=40: connected at +2 s, load done at +7 s, 6 content frame
    WAIT ok 10-text vmeta = [vmeta codec=4 w=480 h=272 d=86.833]
    WAIT ok 11-text video = [video t=3.526 dropped=19 fps=23.84737678855326]
    WAIT ok 12-content m
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-rate: content 40,123-440,421: 10.0 s, flips 207 (20.7/s), changes 206 (20.6/s), reads 211, distinct 207, spacing median 41 p90 70 max 292 ms (wanted 7.1..40/s)
    CHECK ok 15-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash fa88ead8 rows 298 uniform 0 light none blue 209-269
    WAIT ok 18-text vctl = [vctl seek 40 t=26.644]
    WAIT ok 19-differs m
    WAIT ok 20-text video = [video t=46.184 dropped=110 fps=23.112480739599384]
    CHECK ok 21-number: vtime = 46.184 (wanted 38..58)
    CHECK ok 22-audio: the movie's audio: open running audible
    WAIT ok 23-text vend = [vend NetStream.Play.Stop t=86.833 after video t=86.413 dropped=306 fps=25.31645569620253]
    CHECK ok 24-number: vendtime = 86.413 (wanted 84.3..87.3)
    CHECK ok 25-plugin: plugin-container 27693: alive (state S, cpu ticks 2417 to 2429)

### enyo / flash-video-vp6a — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/i004-vp6a.flv: connected at +1 s, load done at +7 s, 5 content frames at +7 s; the G
    WAIT ok 13-text vmeta = [vmeta codec=5 w=478 h=430 d=14]
    WAIT ok 14-text video = [video t=1.533 dropped=9 fps=0]
    CHECK ok 15-flash-drawn: drawn: region 40,123-440,421: colours 2588 hash f5ca77cb rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-flash-rate: content 40,123-440,421: 10.0 s, flips 139 (13.9/s), changes 133 (13.3/s), reads 143, distinct 134, spacing median 58 p90 136 max 251 ms (wanted 9.8..40/s)
    CHECK ok 18-flash-drawn: drawn: region 40,123-440,421: colours 1605 hash df749eb5 rows 298 uniform 0 light none blue none
    CHECK ok 19-audio: the movie's audio: open running audible
    WAIT ok 20-text vend = [vend NetStream.Play.Stop t=13.975 after video t=13.975 dropped=30 fps=11.569052783803325]
    CHECK ok 21-number: vendtime = 13.975 (wanted 11.5..14.5)
    CHECK ok 22-number: venddropped = 30 (wanted 0..125)
    CHECK ok 23-flash-drawn: drawn: region 40,123-440,421: colours 1605 hash df749eb5 rows 298 uniform 0 light none blue none
    CHECK ok 24-plugin: plugin-container 31973: alive (state S, cpu ticks 575 to 586)

### mochi / flash-video-controls — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/harrypotter-vp6.flv%26seek=40: connected at +1 s, load done at +7 s, 8 content fram
    WAIT ok 13-text vmeta = [vmeta codec=4 w=480 h=272 d=86.833]
    WAIT ok 14-text video = [video t=1.279 dropped=3 fps=0]
    WAIT ok 15-content m
    CHECK ok 16-sound: sound at the output sink: peak 870 rms 126 frames 45056 hz 124 env 23,55,165,591,870,594, the speaker amp on, the media volume 23
    WAIT ok 18-text vctl = [vctl pause t=7.706]
    WAIT ok 19-still m
    CHECK ok 21-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    CHECK ok 22-region-near: the picture against the sample paused: difference 0.0 (2 or less)
    WAIT ok 24-text vctl = [vctl resume t=7.706]
    WAIT ok 25-differs m
    CHECK ok 26-sound: sound at the output sink: peak 8401 rms 1529 frames 45056 hz 434 env 8401,5474,4721,4647,4617,3114, the speaker amp on, the media volume 23
    WAIT ok 29-text vctl = [vctl seek 40 t=15.281]
    WAIT ok 30-differs m
    WAIT ok 31-text video = [video t=41.456 dropped=51 fps=26.016260162601625]
    CHECK ok 32-number: vtime = 41.456 (wanted 38..58)
    WAIT ok 33-text vend = [vend NetStream.Play.Stop t=86.833 after video t=86.595 dropped=219 fps=25.848142164781905]
    CHECK ok 34-number: vendtime = 86.595 (wanted 84.3..87.3)
    CHECK ok 35-plugin: plugin-container 14669: alive (state S, cpu ticks 2172 to 2184)

### mochi / flash-video-flowplayer — pass

    mochi card on http://192.168.1.245:8097/title/i004-vp6a: connected at +2 s, load done at +8 s, 9 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    WAIT ok 10-rect m
    WAIT ok 11-content m
    CHECK ok 12-flash-rate: content 0,79-478,507: 5.0 s, flips 58 (11.6/s), changes 56 (11.2/s), reads 65, distinct 57, spacing median 60 p90 82 max 1373 ms (wanted 5..40/s)
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-drawn: drawn: region 0,79-478,507: colours 4096 hash e8290108 rows 428 uniform 97 light 79-479 blue none
    CHECK ok 15-flash-rate: content 0,79-478,507: 3.0 s, flips 41 (13.7/s), changes 41 (13.7/s), reads 43, distinct 42, spacing median 69 p90 103 max 268 ms (wanted 5..40/s)
    WAIT ok 16-still m
    CHECK ok 17-flash-drawn: drawn: region 0,79-478,507: colours 4096 hash 6b8e994c rows 428 uniform 103 light 79-479 blue none
    CHECK ok 18-plugin: plugin-container 19766: alive (state S, cpu ticks 762 to 775)

### mochi / flash-video-h264-240 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-240.flv: connected at +2 s, load done at +8 s, 11 content frames at +8 s; 
    WAIT ok 13-text vmeta = [vmeta codec=7 w=426 h=240 d=20.025]
    WAIT ok 14-text video = [video t=1.202 dropped=0 fps=0]
    CHECK ok 15-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 16-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 7b847995 rows 298 uniform 0 light none blue 218-237
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3895,3898,3896,3897,3896,3897, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,119-440,417: 10.0 s, flips 207 (20.7/s), changes 175 (17.5/s), reads 209, distinct 176, spacing median 41 p90 88 max 215 ms (wanted 11.2..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash f178f5f6 rows 298 uniform 0 light none blue none
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.317 dropped=72 fps=24.154589371980677]
    CHECK ok 23-number: vendtime = 19.317 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 72 (wanted 0..288)
    CHECK ok 25-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 481a5d0c rows 298 uniform 0 light none blue none
    CHECK ok 26-plugin: plugin-container 23411: alive (state S, cpu ticks 1140 to 1151)

### mochi / flash-video-h264-360 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-360.flv: connected at +1 s, load done at +7 s, 10 content frames at +7 s; 
    WAIT ok 13-text vmeta = [vmeta codec=7 w=640 h=360 d=20.025]
    WAIT ok 14-text video = [video t=1.28 dropped=0 fps=0]
    CHECK ok 15-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 16-flash-drawn: drawn: region 40,119-440,417: colours 3766 hash 95739b3f rows 298 uniform 0 light none blue 119-416
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3895,3898,3897,3896,3897,3892, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,119-440,417: 10.0 s, flips 244 (24.4/s), changes 182 (18.2/s), reads 247, distinct 183, spacing median 28 p90 79 max 221 ms (wanted 13.4..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,119-440,417: colours 3717 hash 556e94fc rows 298 uniform 0 light none blue 119-416
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=20.01 after video t=20.01 dropped=40 fps=25.59726962457338]
    CHECK ok 23-number: vendtime = 20.01 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 40 (wanted 0..288)
    CHECK ok 25-flash-drawn: drawn: region 40,119-440,417: colours 3799 hash fd39121a rows 298 uniform 0 light none blue 119-416
    CHECK ok 26-plugin: plugin-container 26890: alive (state S, cpu ticks 1047 to 1059)

### mochi / flash-video-h264-480 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-480.flv: connected at +1 s, load done at +7 s, 9 content frames at +7 s; t
    WAIT ok 13-text vmeta = [vmeta codec=7 w=854 h=480 d=20.025]
    WAIT ok 14-text video = [video t=1.097 dropped=0 fps=0]
    CHECK ok 15-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 16-flash-drawn: drawn: region 40,119-440,417: colours 3400 hash 7d38aef2 rows 298 uniform 0 light none blue 119-416
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3896,3895,3898,3895,3897,3894, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,119-440,417: 10.0 s, flips 200 (20.0/s), changes 167 (16.7/s), reads 202, distinct 168, spacing median 32 p90 106 max 222 ms (wanted 9.3..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,119-440,417: colours 3429 hash 2833c789 rows 298 uniform 0 light none blue 119-416
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=20.01 after video t=19.67 dropped=84 fps=25.848142164781905]
    CHECK ok 23-number: vendtime = 19.67 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 84 (wanted 0..288)
    CHECK ok 25-flash-drawn: drawn: region 40,119-440,417: colours 3547 hash 5e60f899 rows 298 uniform 0 light none blue 119-416
    CHECK ok 26-plugin: plugin-container 30570: alive (state S, cpu ticks 1152 to 1164)

### mochi / flash-video-h264-mp4 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/bbb-h264-360.mp4: connected at +2 s, load done at +8 s, 8 content frames at +8 s; t
    WAIT ok 13-text vmeta = [vmeta codec=avc1 w=640 h=360 d=10]
    WAIT ok 14-text video = [video t=0.933 dropped=4 fps=0]
    CHECK ok 15-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 16-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 32cd1f64 rows 298 uniform 0 light none blue none
    CHECK ok 17-flash-rate: content 40,119-440,417: 5.0 s, flips 147 (29.4/s), changes 114 (22.8/s), reads 148, distinct 115, spacing median 25 p90 57 max 189 ms (wanted 7.5..40/s)
    CHECK ok 18-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 17e6c932 rows 298 uniform 0 light none blue none
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=10 after video t=9.8 dropped=17 fps=47.77070063694268]
    CHECK ok 20-number: vendtime = 9.8 (wanted 7.5..10.5)
    CHECK ok 21-number: venddropped = 17 (wanted 0..180)
    CHECK ok 22-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 7332d9d2 rows 298 uniform 0 light none blue none
    CHECK ok 23-plugin: plugin-container 1740: alive (state S, cpu ticks 568 to 575)

### mochi / flash-video-mediaelement — pass

    mochi card on http://192.168.1.245:8097/title/bbb-h264-360: connected at +1 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 10-rect m
    WAIT ok 11-content m
    CHECK ok 12-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 13-flash-rate: content 0,79-640,437: 5.0 s, flips 193 (38.6/s), changes 161 (32.2/s), reads 194, distinct 162, spacing median 25 p90 34 max 98 ms (wanted 5..40/s)
    CHECK ok 14-flash-drawn: drawn: region 0,79-640,437: colours 4096 hash 3100ae6a rows 358 uniform 0 light none blue none
    WAIT ok 15-still m
    CHECK ok 16-flash-drawn: drawn: region 0,79-640,437: colours 4096 hash a30a8c59 rows 358 uniform 0 light none blue none
    CHECK ok 17-plugin: plugin-container 4974: alive (state S, cpu ticks 624 to 632)

### mochi / flash-video-slow — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/slow/40000/500000/10/video/gen-spark-240.flv: connected at +1 s, load done at +7 s, 11 co
    WAIT ok 14-text vmeta = [vmeta codec=2 w=426 h=240 d=20.025]
    WAIT ok 15-text video = [video t=0.025 dropped=0 fps=0]
    CHECK ok 16-flash-drawn: drawn: region 40,119-440,417: colours 3906 hash 28d6e5f7 rows 298 uniform 0 light none blue 194-213
    WAIT ok 17-text vstatus = [vstatus NetStream.Buffer.Empty t=4.023]
    CHECK ok 18-plugin: plugin-container 8265: alive (state S, cpu ticks 214 to 230)
    WAIT ok 19-text vstatus = [vstatus NetStream.Buffer.Full t=4.023]
    WAIT ok 21-differs m
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=20.01 after video t=20.01 dropped=25 fps=20.380434782608695]
    CHECK ok 23-number: vmaxgap = 16600 (wanted 8000..30000)
    CHECK ok 24-number: vgapat = 4.023 (wanted 4..16)
    CHECK ok 25-number: vendtime = 20.01 (wanted 17.5..20.5)
    CHECK ok 26-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 53017963 rows 298 uniform 0 light none blue none
    CHECK ok 27-plugin: plugin-container 8265: alive (state S, cpu ticks 712 to 724)

### mochi / flash-video-spark-240 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-240.flv: connected at +1 s, load done at +7 s, 11 content frames at +7 s;
    WAIT ok 13-text vmeta = [vmeta codec=2 w=426 h=240 d=20.025]
    WAIT ok 14-text video = [video t=1.692 dropped=4 fps=26.229508196721312]
    CHECK ok 15-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash b8240be1 rows 298 uniform 0 light none blue 219-238
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3894,3895,3898,3896,3897,3894, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,119-440,417: 10.0 s, flips 199 (19.9/s), changes 195 (19.5/s), reads 203, distinct 196, spacing median 42 p90 81 max 294 ms (wanted 10.3..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash fac34401 rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=20.01 after video t=19.853 dropped=47 fps=23.916292974588938]
    CHECK ok 22-number: vendtime = 19.853 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 47 (wanted 0..288)
    CHECK ok 24-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash c2e9e7b1 rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 12789: alive (state S, cpu ticks 591 to 602)

### mochi / flash-video-spark-360 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-360.flv: connected at +1 s, load done at +7 s, 9 content frames at +7 s; 
    WAIT ok 13-text vmeta = [vmeta codec=2 w=640 h=360 d=20.025]
    WAIT ok 14-text video = [video t=1.202 dropped=2 fps=0]
    CHECK ok 15-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 2a567d50 rows 298 uniform 0 light none blue 249-261
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3894,3895,3898,3896,3897,3893, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,119-440,417: 10.0 s, flips 201 (20.1/s), changes 201 (20.1/s), reads 205, distinct 202, spacing median 41 p90 82 max 487 ms (wanted 12.8..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 255defb6 rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=20.01 after video t=19.67 dropped=72 fps=23.32814930015552]
    CHECK ok 22-number: vendtime = 19.67 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 72 (wanted 0..288)
    CHECK ok 24-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 59fc9a0b rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 16246: alive (state S, cpu ticks 695 to 707)

### mochi / flash-video-spark-480 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-480.flv: connected at +1 s, load done at +7 s, 12 content frames at +7 s;
    WAIT ok 13-text vmeta = [vmeta codec=2 w=854 h=480 d=20.025]
    WAIT ok 14-text video = [video t=1.019 dropped=7 fps=10.438413361169102]
    CHECK ok 15-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 39e2e961 rows 298 uniform 0 light none blue 231-240
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3895,3898,3895,3897,3896,3894, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,119-440,417: 10.0 s, flips 197 (19.7/s), changes 194 (19.4/s), reads 200, distinct 195, spacing median 43 p90 84 max 272 ms (wanted 11.5..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,119-440,417: colours 3587 hash f64ae01d rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.67 dropped=100 fps=24.50980392156863]
    CHECK ok 22-number: vendtime = 19.67 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 100 (wanted 0..288)
    CHECK ok 24-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 3d63abdf rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 19736: alive (state S, cpu ticks 818 to 829)

### mochi / flash-video-spark-sheep — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/sheep-spark.flv: connected at +1 s, load done at +7 s, 5 content frames at +7 s; th
    WAIT ok 13-text video = [video t=1.202 dropped=2 fps=0]
    CHECK ok 14-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash e5af2f52 rows 298 uniform 0 light 119-162 blue none
    CHECK ok 15-audio: the movie's audio: open running audible
    CHECK ok 16-flash-rate: content 40,119-440,417: 5.0 s, flips 67 (13.4/s), changes 62 (12.4/s), reads 70, distinct 63, spacing median 61 p90 141 max 412 ms (wanted 6.9..40/s)
    CHECK ok 17-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 7f2a41df rows 298 uniform 0 light 119-144 blue none
    CHECK ok 18-audio: the movie's audio: open running audible
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=8.803 after video t=8.803 dropped=96 fps=15.429122468659594]
    CHECK ok 20-number: vendtime = 8.803 (wanted 6.3..9.3)
    CHECK ok 21-number: venddropped = 96 (wanted 0..154)
    CHECK ok 22-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 70252f20 rows 298 uniform 0 light 119-144 blue none
    CHECK ok 23-plugin: plugin-container 23196: alive (state S, cpu ticks 333 to 344)

### mochi / flash-video-spark-small — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/zelda-spark.flv: connected at +2 s, load done at +8 s, 12 content frames at +8 s; t
    WAIT ok 13-text video = [video t=1.393 dropped=1 fps=0]
    CHECK ok 14-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash f5141148 rows 298 uniform 0 light none blue none
    CHECK ok 15-audio: the movie's audio: open running audible
    CHECK ok 16-flash-rate: content 40,119-440,417: 10.0 s, flips 108 (10.8/s), changes 106 (10.6/s), reads 119, distinct 107, spacing median 77 p90 108 max 1406 ms (wanted 7.7..40/s)
    CHECK ok 17-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash ab97a2e1 rows 298 uniform 0 light 119-193 blue none
    CHECK ok 18-audio: the movie's audio: open running audible
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=29.75 after video t=29.442 dropped=7 fps=12.028869286287089]
    CHECK ok 20-number: vendtime = 29.442 (wanted 27.2..30.2)
    CHECK ok 21-number: venddropped = 7 (wanted 0..212)
    CHECK ok 22-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash b83d2678 rows 298 uniform 0 light 241-416 blue none
    CHECK ok 23-plugin: plugin-container 26774: alive (state R, cpu ticks 453 to 463)

### mochi / flash-video-vp6-176 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/mrandmrs-vp6.flv%26seek=60: connected at +2 s, load done at +7 s, 8 content frames 
    WAIT ok 10-text vmeta = [vmeta codec=4 w=320 h=176 d=151.125]
    WAIT ok 11-text video = [video t=1.791 dropped=3 fps=28.089887640449437]
    WAIT ok 12-content m
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-rate: content 40,119-440,417: 10.0 s, flips 227 (22.7/s), changes 218 (21.8/s), reads 228, distinct 219, spacing median 41 p90 70 max 103 ms (wanted 11.0..40/s)
    CHECK ok 15-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash d11c701d rows 298 uniform 0 light none blue none
    WAIT ok 18-text vctl = [vctl seek 60 t=21.446]
    WAIT ok 19-differs m
    WAIT ok 20-text video = [video t=65.018 dropped=13 fps=21.306818181818183]
    CHECK ok 21-number: vtime = 65.018 (wanted 58..78)
    CHECK ok 22-audio: the movie's audio: open running audible
    WAIT ok 23-text vend = [vend NetStream.Play.Stop t=151.125 after video t=151.013 dropped=143 fps=24.72952086553323]
    CHECK ok 24-number: vendtime = 151.013 (wanted 148.6..151.6)
    CHECK ok 25-plugin: plugin-container 30795: alive (state S, cpu ticks 2667 to 2678)

### mochi / flash-video-vp6-272 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/harrypotter-vp6.flv%26seek=40: connected at +1 s, load done at +7 s, 11 content fra
    WAIT ok 10-text vmeta = [vmeta codec=4 w=480 h=272 d=86.833]
    WAIT ok 11-text video = [video t=1.279 dropped=0 fps=0]
    WAIT ok 12-content m
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-rate: content 40,119-440,417: 10.0 s, flips 213 (21.3/s), changes 210 (21.0/s), reads 214, distinct 211, spacing median 42 p90 66 max 198 ms (wanted 7.1..40/s)
    CHECK ok 15-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 5bfef869 rows 298 uniform 0 light none blue none
    WAIT ok 18-text vctl = [vctl seek 40 t=20.793]
    WAIT ok 19-differs m
    WAIT ok 20-text video = [video t=42.84 dropped=50 fps=20.915032679738562]
    CHECK ok 21-number: vtime = 43.964 (wanted 38..58)
    CHECK ok 22-audio: the movie's audio: open running audible
    WAIT ok 23-text vend = [vend NetStream.Play.Stop t=86.833 after video t=86.413 dropped=224 fps=25.93192868719611]
    CHECK ok 24-number: vendtime = 86.413 (wanted 84.3..87.3)
    CHECK ok 25-plugin: plugin-container 3833: alive (state R, cpu ticks 2304 to 2315)

### mochi / flash-video-vp6a — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/i004-vp6a.flv: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the 
    WAIT ok 13-text vmeta = [vmeta codec=5 w=478 h=430 d=14]
    WAIT ok 14-text video = [video t=1.097 dropped=1 fps=0]
    CHECK ok 15-flash-drawn: drawn: region 40,119-440,417: colours 2431 hash 1ac840d2 rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-flash-rate: content 40,119-440,417: 10.0 s, flips 144 (14.4/s), changes 140 (14.0/s), reads 146, distinct 141, spacing median 60 p90 118 max 216 ms (wanted 9.8..40/s)
    CHECK ok 18-flash-drawn: drawn: region 40,119-440,417: colours 1644 hash dfd24d6c rows 298 uniform 0 light none blue none
    CHECK ok 19-audio: the movie's audio: open running audible
    WAIT ok 20-text vend = [vend NetStream.Play.Stop t=13.975 after video t=13.818 dropped=20 fps=15.873015873015873]
    CHECK ok 21-number: vendtime = 13.818 (wanted 11.5..14.5)
    CHECK ok 22-number: venddropped = 20 (wanted 0..125)
    CHECK ok 23-flash-drawn: drawn: region 40,119-440,417: colours 1605 hash df749eb5 rows 298 uniform 0 light none blue none
    CHECK ok 24-plugin: plugin-container 8220: alive (state S, cpu ticks 586 to 597)

### mojo / flash-video-controls — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/harrypotter-vp6.flv%26seek=40: connected at +0 s, load done at +6 s, 5 content frame
    WAIT ok 13-text vmeta = [vmeta codec=4 w=480 h=272 d=86.833]
    WAIT ok 14-text video = [video t=1.279 dropped=3 fps=0]
    WAIT ok 15-content m
    CHECK ok 16-sound: sound at the output sink: peak 870 rms 156 frames 45056 hz 108 env 41,163,395,870,858,667, the speaker amp on, the media volume 23
    WAIT ok 18-text vctl = [vctl pause t=7.958]
    WAIT ok 19-still m
    CHECK ok 21-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    CHECK ok 22-region-near: the picture against the sample paused: difference 0.0 (2 or less)
    WAIT ok 24-text vctl = [vctl resume t=7.958]
    WAIT ok 25-differs m
    CHECK ok 26-sound: sound at the output sink: peak 3928 rms 850 frames 45056 hz 474 env 3928,3626,2754,1911,1815,1432, the speaker amp on, the media volume 23
    WAIT ok 29-text vctl = [vctl seek 40 t=19.333]
    WAIT ok 30-differs m
    WAIT ok 31-text video = [video t=43.75 dropped=153 fps=20.053475935828878]
    CHECK ok 32-number: vtime = 43.75 (wanted 38..58)
    WAIT ok 33-text vend = [vend NetStream.Play.Stop t=86.833 after video t=86.308 dropped=428 fps=23.02158273381295]
    CHECK ok 34-number: vendtime = 86.308 (wanted 84.3..87.3)
    CHECK ok 35-plugin: plugin-container 23063: alive (state S, cpu ticks 2090 to 2106)

### mojo / flash-video-flowplayer — pass

    mojo card on http://192.168.1.245:8097/title/i004-vp6a: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 10-rect m
    WAIT ok 11-content m
    CHECK ok 12-flash-rate: content 0,74-478,502: 5.0 s, flips 76 (15.2/s), changes 76 (15.2/s), reads 77, distinct 77, spacing median 69 p90 82 max 121 ms (wanted 5..40/s)
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-drawn: drawn: region 0,74-478,502: colours 2856 hash d1122055 rows 428 uniform 57 light 74-501 blue none
    CHECK ok 15-flash-rate: content 0,74-478,502: 3.0 s, flips 45 (15.0/s), changes 45 (15.0/s), reads 46, distinct 46, spacing median 66 p90 80 max 89 ms (wanted 5..40/s)
    WAIT ok 16-still m
    CHECK ok 17-flash-drawn: drawn: region 0,74-478,502: colours 4039 hash a7504443 rows 428 uniform 103 light 74-474 blue none
    CHECK ok 18-plugin: plugin-container 28336: alive (state S, cpu ticks 804 to 816)

### mojo / flash-video-h264-240 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-240.flv: connected at +1 s, load done at +7 s, 11 content frames at +7 s; t
    WAIT ok 13-text vmeta = [vmeta codec=7 w=426 h=240 d=20.025]
    WAIT ok 14-text video = [video t=1.985 dropped=6 fps=11.867088607594937]
    CHECK ok 15-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 16-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash b5a579d2 rows 298 uniform 0 light none blue 126-145
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3897,3898,3896,3897,3896,3892, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,114-440,412: 10.0 s, flips 191 (19.1/s), changes 166 (16.6/s), reads 193, distinct 167, spacing median 39 p90 94 max 259 ms (wanted 11.2..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash dcfb29a5 rows 298 uniform 0 light none blue none
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=20.01 after video t=19.122 dropped=108 fps=16.025641025641026]
    CHECK ok 23-number: vendtime = 19.122 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 108 (wanted 0..288)
    CHECK ok 25-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 179c1c49 rows 298 uniform 0 light none blue none
    CHECK ok 26-plugin: plugin-container 31978: alive (state S, cpu ticks 936 to 946)

### mojo / flash-video-h264-360 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-360.flv: connected at +1 s, load done at +6 s, 6 content frames at +6 s; th
    WAIT ok 13-text vmeta = [vmeta codec=7 w=640 h=360 d=20.025]
    WAIT ok 14-text video = [video t=1.28 dropped=0 fps=23.771790808240887]
    CHECK ok 15-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 16-flash-drawn: drawn: region 40,114-440,412: colours 3477 hash b2ccf5e4 rows 298 uniform 0 light none blue 114-411
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3895,3898,3895,3897,3896,3894, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,114-440,412: 10.0 s, flips 180 (18.0/s), changes 154 (15.4/s), reads 182, distinct 155, spacing median 42 p90 95 max 227 ms (wanted 13.4..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,114-440,412: colours 3299 hash 5be8c381 rows 298 uniform 0 light none blue 114-411
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=20.01 after video t=19.775 dropped=66 fps=22.76176024279211]
    CHECK ok 23-number: vendtime = 19.775 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 66 (wanted 0..288)
    CHECK ok 25-flash-drawn: drawn: region 40,114-440,412: colours 3658 hash 25efb4ec rows 298 uniform 0 light none blue 114-411
    CHECK ok 26-plugin: plugin-container 3179: alive (state S, cpu ticks 815 to 826)

### mojo / flash-video-h264-480 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-480.flv: connected at +1 s, load done at +7 s, 9 content frames at +7 s; th
    WAIT ok 13-text vmeta = [vmeta codec=7 w=854 h=480 d=20.025]
    WAIT ok 14-text video = [video t=1.75 dropped=4 fps=0]
    CHECK ok 15-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 16-flash-drawn: drawn: region 40,114-440,412: colours 3044 hash 3715cbf5 rows 298 uniform 0 light none blue 114-411
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3896,3897,3896,3897,3897,3898, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,114-440,412: 10.0 s, flips 172 (17.2/s), changes 155 (15.5/s), reads 174, distinct 156, spacing median 46 p90 106 max 213 ms (wanted 9.3..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,114-440,412: colours 2898 hash a1e4763f rows 298 uniform 0 light none blue 114-411
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.957 dropped=150 fps=13.525698827772768]
    CHECK ok 23-number: vendtime = 19.957 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 150 (wanted 0..288)
    CHECK ok 25-flash-drawn: drawn: region 40,114-440,412: colours 3094 hash 1d326baa rows 298 uniform 0 light none blue 114-411
    CHECK ok 26-plugin: plugin-container 6683: alive (state S, cpu ticks 1020 to 1031)

### mojo / flash-video-h264-mp4 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/bbb-h264-360.mp4: connected at +1 s, load done at +6 s, 4 content frames at +6 s; th
    WAIT ok 13-text vmeta = [vmeta codec=avc1 w=640 h=360 d=10]
    WAIT ok 14-text video = [video t=1.033 dropped=3 fps=0]
    CHECK ok 15-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 16-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 1b6c8165 rows 298 uniform 0 light none blue none
    CHECK ok 17-flash-rate: content 40,114-440,412: 5.0 s, flips 118 (23.6/s), changes 94 (18.8/s), reads 119, distinct 95, spacing median 32 p90 64 max 154 ms (wanted 7.5..40/s)
    CHECK ok 18-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash af718ab1 rows 298 uniform 0 light none blue none
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=10 after video t=8.733 dropped=35 fps=26.501766784452297]
    CHECK ok 20-number: vendtime = 8.733 (wanted 7.5..10.5)
    CHECK ok 21-number: venddropped = 35 (wanted 0..180)
    CHECK ok 22-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash af718ab1 rows 298 uniform 0 light none blue none
    CHECK ok 23-plugin: plugin-container 10288: alive (state S, cpu ticks 428 to 436)

### mojo / flash-video-mediaelement — pass

    mojo card on http://192.168.1.245:8097/title/bbb-h264-360: connected at +1 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 10-rect m
    WAIT ok 11-content m
    CHECK ok 12-decoder: the kernel's video decoder: no arena fault in the last two minutes
    CHECK ok 13-flash-rate: content 0,74-640,432: 5.0 s, flips 132 (26.4/s), changes 106 (21.2/s), reads 133, distinct 107, spacing median 31 p90 48 max 185 ms (wanted 5..40/s)
    CHECK ok 14-flash-drawn: drawn: region 0,74-640,432: colours 4096 hash 18dfd5c4 rows 358 uniform 0 light none blue none
    WAIT ok 15-still m
    CHECK ok 16-flash-drawn: drawn: region 0,74-640,432: colours 4096 hash 18dfd5c4 rows 358 uniform 0 light none blue none
    CHECK ok 17-plugin: plugin-container 13521: alive (state S, cpu ticks 451 to 458)

### mojo / flash-video-slow — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/slow/40000/500000/10/video/gen-spark-240.flv: connected at +1 s, load done at +6 s, 3 cont
    WAIT ok 14-text vmeta = [vmeta codec=2 w=426 h=240 d=20.025]
    WAIT ok 15-text video = [video t=0.025 dropped=0 fps=0]
    CHECK ok 16-flash-drawn: drawn: region 40,114-440,412: colours 3601 hash 1ff8a839 rows 298 uniform 0 light none blue 189-208
    WAIT ok 17-text vstatus = [vstatus NetStream.Buffer.Empty t=4.023]
    CHECK ok 18-plugin: plugin-container 16770: alive (state S, cpu ticks 200 to 216)
    WAIT ok 19-text vstatus = [vstatus NetStream.Buffer.Full t=4.023]
    WAIT ok 21-differs m
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.942 dropped=47 fps=21.83406113537118]
    CHECK ok 23-number: vmaxgap = 16726 (wanted 8000..30000)
    CHECK ok 24-number: vgapat = 4.023 (wanted 4..16)
    CHECK ok 25-number: vendtime = 19.942 (wanted 17.5..20.5)
    CHECK ok 26-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash ea789dc8 rows 298 uniform 0 light none blue none
    CHECK ok 27-plugin: plugin-container 16770: alive (state S, cpu ticks 651 to 663)

### mojo / flash-video-spark-240 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-240.flv: connected at +1 s, load done at +7 s, 3 content frames at +7 s; t
    WAIT ok 13-text vmeta = [vmeta codec=2 w=426 h=240 d=20.025]
    WAIT ok 14-text video = [video t=1.28 dropped=1 fps=0]
    CHECK ok 15-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash cebc915c rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3898,3897,3896,3896,3895,3892, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,114-440,412: 10.0 s, flips 176 (17.6/s), changes 173 (17.3/s), reads 179, distinct 174, spacing median 48 p90 94 max 279 ms (wanted 10.3..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash a8384ce8 rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.67 dropped=66 fps=24.50980392156863]
    CHECK ok 22-number: vendtime = 19.67 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 66 (wanted 0..288)
    CHECK ok 24-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash ea789dc8 rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 20901: alive (state S, cpu ticks 539 to 551)

### mojo / flash-video-spark-360 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-360.flv: connected at +1 s, load done at +6 s, 7 content frames at +6 s; t
    WAIT ok 13-text vmeta = [vmeta codec=2 w=640 h=360 d=20.025]
    WAIT ok 14-text video = [video t=2.358 dropped=16 fps=21.09704641350211]
    CHECK ok 15-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash f47e760b rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3897 rms 2750 frames 45056 hz 440 env 3897,3896,3895,3895,3896,3891, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,114-440,412: 10.0 s, flips 183 (18.3/s), changes 177 (17.7/s), reads 185, distinct 178, spacing median 44 p90 90 max 219 ms (wanted 12.8..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 1577af17 rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.525 dropped=91 fps=20.804438280166437]
    CHECK ok 22-number: vendtime = 19.525 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 91 (wanted 0..288)
    CHECK ok 24-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash eb1248d8 rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 24358: alive (state S, cpu ticks 643 to 655)

### mojo / flash-video-spark-480 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-480.flv: connected at +1 s, load done at +6 s, 5 content frames at +6 s; t
    WAIT ok 13-text vmeta = [vmeta codec=2 w=854 h=480 d=20.025]
    WAIT ok 14-text video = [video t=3.971 dropped=32 fps=16.7973124300112]
    CHECK ok 15-flash-drawn: drawn: region 40,114-440,412: colours 3676 hash 93f0d4d1 rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3897 rms 2750 frames 45056 hz 440 env 3895,3895,3896,3896,3897,3894, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,114-440,412: 10.0 s, flips 174 (17.4/s), changes 172 (17.2/s), reads 175, distinct 173, spacing median 45 p90 103 max 174 ms (wanted 11.5..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,114-440,412: colours 4077 hash b377425f rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=19.983 after video t=18.939 dropped=120 fps=23.357664233576642]
    CHECK ok 22-number: vendtime = 18.939 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 120 (wanted 0..288)
    CHECK ok 24-flash-drawn: drawn: region 40,114-440,412: colours 4077 hash b377425f rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 27897: alive (state S, cpu ticks 758 to 770)

### mojo / flash-video-spark-sheep — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/sheep-spark.flv: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the
    WAIT ok 13-text video = [video t=2.873 dropped=27 fps=18.12004530011325]
    CHECK ok 14-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 8500bfac rows 298 uniform 35 light 114-246 blue none
    CHECK ok 15-audio: the movie's audio: open running audible
    CHECK ok 16-flash-rate: content 40,114-440,412: 5.0 s, flips 57 (11.4/s), changes 45 (9.0/s), reads 61, distinct 44, spacing median 72 p90 189 max 278 ms (wanted 6.9..40/s)
    CHECK ok 17-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash da0e1ad8 rows 298 uniform 5 light 114-139 blue none
    CHECK ok 18-audio: the movie's audio: open running audible
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=8.725 after video t=8.725 dropped=110 fps=16.276703967446593]
    CHECK ok 20-number: vendtime = 8.725 (wanted 6.3..9.3)
    CHECK ok 21-number: venddropped = 110 (wanted 0..154)
    CHECK ok 22-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash da0e1ad8 rows 298 uniform 5 light 114-139 blue none
    CHECK ok 23-plugin: plugin-container 31332: alive (state R, cpu ticks 324 to 335)

### mojo / flash-video-spark-small — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/zelda-spark.flv: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the
    WAIT ok 13-text video = [video t=1.207 dropped=0 fps=0]
    CHECK ok 14-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 35eb43f7 rows 298 uniform 0 light 114-130 blue none
    CHECK ok 15-audio: the movie's audio: open running audible
    CHECK ok 16-flash-rate: content 40,114-440,412: 10.0 s, flips 124 (12.4/s), changes 124 (12.4/s), reads 125, distinct 125, spacing median 77 p90 106 max 180 ms (wanted 7.7..40/s)
    CHECK ok 17-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 9ca254b1 rows 298 uniform 0 light 194-273 blue none
    CHECK ok 18-audio: the movie's audio: open running audible
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=29.75 after video t=29.628 dropped=17 fps=12.279355333844974]
    CHECK ok 20-number: vendtime = 29.628 (wanted 27.2..30.2)
    CHECK ok 21-number: venddropped = 17 (wanted 0..212)
    CHECK ok 22-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 465020e8 rows 298 uniform 0 light 236-411 blue none
    CHECK ok 23-plugin: plugin-container 2536: alive (state S, cpu ticks 427 to 438)

### mojo / flash-video-vp6-176 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/mrandmrs-vp6.flv%26seek=60: connected at +1 s, load done at +6 s, 5 content frames a
    WAIT ok 10-text vmeta = [vmeta codec=4 w=320 h=176 d=151.125]
    WAIT ok 11-text video = [video t=2.69 dropped=10 fps=25.641025641025642]
    WAIT ok 12-content m
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-rate: content 40,114-440,412: 10.0 s, flips 199 (19.9/s), changes 198 (19.8/s), reads 200, distinct 199, spacing median 45 p90 84 max 186 ms (wanted 11.0..40/s)
    CHECK ok 15-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 8fddcc3c rows 298 uniform 0 light none blue none
    WAIT ok 18-text vctl = [vctl seek 60 t=28.029]
    WAIT ok 19-differs m
    WAIT ok 20-text video = [video t=65.708 dropped=66 fps=21.459227467811157]
    CHECK ok 21-number: vtime = 67.422 (wanted 58..78)
    CHECK ok 22-audio: the movie's audio: open running audible
    WAIT ok 23-text vend = [vend NetStream.Play.Stop t=151.125 after video t=150.831 dropped=299 fps=17.0261066969353]
    CHECK ok 24-number: vendtime = 150.831 (wanted 148.6..151.6)
    CHECK ok 25-plugin: plugin-container 6256: alive (state S, cpu ticks 2489 to 2500)

### mojo / flash-video-vp6-272 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/harrypotter-vp6.flv%26seek=40: connected at +0 s, load done at +6 s, 8 content frame
    WAIT ok 10-text vmeta = [vmeta codec=4 w=480 h=272 d=86.833]
    WAIT ok 11-text video = [video t=2.625 dropped=17 fps=19.71090670170828]
    WAIT ok 12-content m
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-rate: content 40,114-440,412: 10.0 s, flips 190 (19.0/s), changes 190 (19.0/s), reads 193, distinct 191, spacing median 43 p90 79 max 310 ms (wanted 7.1..40/s)
    CHECK ok 15-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash c22d94ac rows 298 uniform 0 light none blue 202-288
    WAIT ok 18-text vctl = [vctl seek 40 t=24.708]
    WAIT ok 19-differs m
    WAIT ok 20-text video = [video t=44.146 dropped=154 fps=15.706806282722512]
    CHECK ok 21-number: vtime = 44.146 (wanted 38..58)
    CHECK ok 22-audio: the movie's audio: open running audible
    WAIT ok 23-text vend = [vend NetStream.Play.Stop t=86.833 after video t=86.041 dropped=494 fps=17.8359096313912]
    CHECK ok 24-number: vendtime = 86.041 (wanted 84.3..87.3)
    CHECK ok 25-plugin: plugin-container 11137: alive (state S, cpu ticks 2158 to 2170)

### mojo / flash-video-vp6a — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/i004-vp6a.flv: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the G
    WAIT ok 13-text vmeta = [vmeta codec=5 w=478 h=430 d=14]
    WAIT ok 14-text video = [video t=2.298 dropped=9 fps=7.012622720897616]
    CHECK ok 15-flash-drawn: drawn: region 40,114-440,412: colours 2553 hash d6b0f728 rows 298 uniform 67 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-flash-rate: content 40,114-440,412: 10.0 s, flips 119 (11.9/s), changes 110 (11.0/s), reads 125, distinct 110, spacing median 63 p90 164 max 304 ms (wanted 9.8..40/s)
    CHECK ok 18-flash-drawn: drawn: region 40,114-440,412: colours 1495 hash 9ca6657e rows 298 uniform 67 light none blue none
    CHECK ok 19-audio: the movie's audio: open running audible
    WAIT ok 20-text vend = [vend NetStream.Play.Stop t=13.975 after video t=13.635 dropped=32 fps=16.580310880829014]
    CHECK ok 21-number: vendtime = 13.635 (wanted 11.5..14.5)
    CHECK ok 22-number: venddropped = 32 (wanted 0..125)
    CHECK ok 23-flash-drawn: drawn: region 40,114-440,412: colours 1495 hash 9ca6657e rows 298 uniform 67 light none blue none
    CHECK ok 24-plugin: plugin-container 15403: alive (state S, cpu ticks 570 to 581)

