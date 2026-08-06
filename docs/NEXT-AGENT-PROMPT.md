# Pickup prompt for the next agent

Paste everything in the fenced block below as the opening message of a fresh session.

```
Continue the Jihad Browser port. You are picking up mid-stream from a previous agent that
ran out of context; the work is committed and the device is live.

READ FIRST, IN THIS ORDER
1. Jihad-Browser/docs/HANDOFF-2026-08-06.md — what the last session changed, what is
   known-broken with the exact next step for each, and a "traps" list of things that cost
   real time and are NOT discoverable from the source. Do not skip the traps.
2. Jihad-Browser/context/kits/cavekit-preferences-ui.md, requirements R4 and R5 — the
   unfinished work is specified there, including two routes and why the first was abandoned.
3. Jihad-Browser/docs/DEVICE-HANDOFF.md — the older device/.ipk track. Still current.

STATE
- Branch jihad/about-preferences-and-settings-merge, two commits on top of main. Clean tree,
  NOT pushed. Do not push without being asked.
- The TouchPad is connected over novacom and is ALREADY RUNNING this branch's card JS and a
  daemon rebuilt from it. It got there via build/webos-oe/push-card-js.sh and direct
  novacom put — NOT via an .ipk. The packaged install path has not been rebuilt or tested
  since these changes.
- Kit status: 307 met / 11 partial / 30 open across 348 criteria. Re-count before quoting
  any figure; the recipe is in the handoff and the overview header has drifted before.

DO THIS FIRST
Settle where the url fragment is lost, because it blocks the settings merge (R5) and it is
one measurement, not a design decision. The settings page publishes an edit by rewriting its
own url fragment and the card never sees it. Add one fprintf of the raw url at the top of
BrowserPageGoanna::openUrl and one of mPage->CurrentUri() after load-stop, rebuild the daemon
(build/webos-oe/build-daemon-arm.sh in the container — the exact podman invocation is in
docs/DEVICE-BUILD.md), push it, and launch with about:preferences#chrome=X. If openUrl
already lacks the fragment the engine is innocent and the fix is card-side.
Note the previous agent's claim that CurrentUri() strips the ref was DISPROVED — GetSpec()
appends it, and this project's own Mojo start page round-trips start.html#links= fine. Do not
re-derive that; find where it actually goes.

THEN, IN PRIORITY ORDER
- Same-document location changes: PageChrome::OnLocationChange in render/goanna/
  GoannaRenderPage.cpp is an empty stub, and PageChrome is registered with NOTIFY_ALL, so the
  notification is already being delivered and discarded. A fragment change produces no
  load-stop, so this stub is why an edit can never reach the card even with the ref present.
- Public-bus Luna: JihadLunaService.cpp calls LSRegister (private handle). packaging/
  gen-variant-scripts.sh ALREADY installs a public role file with inbound:["*"], so this is a
  ~10-line change, not a policy wall. It also answers whether app/source/Browser.js's
  card-side clearCookies has ever worked — its kit marks were verified with a privileged
  luna-send caller, which does not exercise the shipped path.
- The unexplained scroll report ("I cant scroll down in about:preferences"). Two diagnoses
  were wrong. REMOVE the jihad-effect test add-on first — it paints every document magenta
  via a user stylesheet and caused both wrong diagnoses.
- Then work remaining kit criteria by cost. Several cannot be met at all: device-build R6
  needs TouchPad Go hardware the user does not have, some are [human-review], gre-widgets R1
  needs codec measurement, and addons R7 is a windowless-NPAPI port the kit says must be
  PORTED, not enabled. Say so rather than reporting motion toward them.

HOW TO WORK HERE
- Prove things from the other side. A pref read back is not proof it took effect; check the
  profile, the rendered pixels, or the daemon log. The last session found a control that
  wrote its pref correctly and did nothing, because the daemon also gates that value
  per-page on the docShell.
- The device screenshots itself:
  luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/x.png"}'
  Frames are framebuffer-oriented; magick -rotate 90. Most UI bugs here were invisible in
  source and obvious in a screenshot.
- push-card-js.sh is stamp-proven — trust its PASS/FAIL. The stamp lives in Browser.js
  (Enyo) / JihadBrowser.js (Mochi) / main-assistant.js (Mojo); include that file or the
  check is a false negative.
- Assert your edits. Three times last session a pattern-based edit silently did nothing and
  reported success: a str.replace that did not match, a grep -q guard on a name upstream
  already set, and a block delete bounded by endpoints that swallowed five unrelated
  functions and removed working features from the product. Assert the anchor exists; assert
  a delete does not span something it should not.
- Re-count kit criteria programmatically; never hand-assert the totals.
- Ask for an adversarial review of your own conclusions before declaring a domain done. Last
  session's review found a shipped regression, a security hole, and two false claims.

STANDING CONSTRAINTS FROM THE USER — DO NOT VIOLATE
- Never read or scan /media backup mounts.
- All work stays in eclipse-workspace. Never Downloads/, /home/admin, or backups.
- Do not let cards pile up; restart LunaSysMgr if you have to. Install/remove cycles are
  authorised so long as you restart luna.
- NEVER reboot into an untested boot-critical config. Writing LS2 role files and rebooting in
  one step bricked this device once. Stage boot-critical config, test live, know the way back.
- $JIHAD_INJECT stays OFF by default and gated to a root-owned private regular file. Never
  log keystrokes or field values.
- Commit or push only when asked.
```
