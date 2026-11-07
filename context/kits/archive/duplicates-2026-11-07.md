# Criteria that overlap, found by the 2026-11-07 sweep (left in place: merging criteria changes what is counted, the owner's call)

- Flash: cavekit-addons-extensions.md R7 (frame rate, keyboard, crash bound, the on/off switch, sound, input, windowless
  composite) against cavekit-flash.md R3, R5, R6, R10, R14, R2, R4, R11. addons R7's keyboard box is `[~]` where flash
  R5's is met (suite/flash-input-kernel.test).
- Personas: personas-ui R4 #2, #4 and R5 #3 against addons-extensions R10 #1, #3, #5; persona-formats R4 #5 against
  addons-site R6 #2; addons-site R12 #1-3 against personas-ui R8 #5 (R12 #1 says it was superseded).
- Certificates: certificate-trust R3 #1, #2 and R4 #3 against proxies-certs R4 #1 and R2 #4 (re-verifications).
- content-blocking R4 #2 restates R1 #5 and R3 #3.
- The paint protocol: ipc-contract R2 (one paint-ready notification, the buffer not reused early, pixel format) against
  offscreen-rendering; device-build (portrait and landscape composite) against offscreen-rendering R5.
- engine-embedding (idle CPU) against device-build (an idle daemon is idle); desktop-build (LunaService compiled out)
  against ipc-contract R4; device-build's coexistence against ipc-contract's two-line rebrand.
- gpu-rendering R11 (the patch queue) against engine-update R1-R3 and lowpower-uxp; lowpower-uxp R3 (three daemons in the
  budget) against gpu-rendering (three daemons on the GPU path).
- The shells: long-press with a real finger in ui-shell R7, mochi-ui R2, mojo-ui R6; the card-side `<select>` popup in
  ui-shell R5, mochi-ui R6 and preferences-ui R6; pinch in input-bridging R3 and mojo-ui R6; the keyboard on chrome
  pages in preferences-ui R6 and input-bridging R6; the frozen settings commands in preferences-ui R5 and
  browser-services R1; coexistence in mochi-ui R1 and mojo-ui R1.
- input-bridging R1 and R6 (XUL taps do not crash; XUL buttons activate) within one kit.
- tabbed-browsing: "no tab cap" in R1, R3 and R5; and window.open as a new tab (tabbed-browsing R1) against window.open
  as a new card (browser-services R1).

Stale criterion TEXT the sweep could not touch (its rule: criteria unchanged): ui-shell R6 (settings in the card's own
panel, gone since 2026-10-05); gre-widgets R3 ("they are not today"); engine-update R1 ("39 patches", now 72);
device-build R3's clean-clone box and its own sweep box, both `[~]` though what they waited on was done 2026-10-05/14.
