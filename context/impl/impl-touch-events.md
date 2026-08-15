---
created: "2026-08-15"
last_edited: "2026-08-15"
status: T-120 code-complete and built; DEFAULT OFF (JIHAD_TOUCH_EVENTS=1). NOT verified on hardware —
        no device was connected this session. cavekit-input-bridging.md R3's pinch/tap criterion
        stays OPEN on the human two-finger check (T-151).
---

# DOM touch forwarding (T-120): header re-stage, fence removal, double-activation suppressor

Everything below was done on the BUILD HOST. **No device was used. Nothing here is a runtime
result.** The only new runtime evidence in this document is the compiler's.

## 2026-08-15 — what changed

Three pieces, in the order they have to be done:

1. **The npapi.h the adapter compiles against was re-staged** with the 28-byte
   `NpPalmTouchEvent`, which is what the device actually pushes.
2. **`BrowserAdapter::doTouchEvent` came out of `#ifdef QT_FIXME`** and now serializes touch
   points using `JihadTouchState`, behind a default-off runtime switch.
3. **A double-activation suppressor landed daemon-side**, because the same finger also produces
   pen events on a separate YAP command and the engine cannot know the two are related.

### 1. Header re-stage

The adapter includes `<webkit/npapi/npapi.h>`. Proven (not assumed) by preprocessing
`render/adapter/BrowserAdapter.cpp` with `-E -dI` and reading the resolved path: it comes from
`-I$DEPS/staging/include`, i.e.

    build/webos-oe/adapter-deps/staging/include/webkit/npapi/npapi.h

which is the same file for BOTH adapter builds (`build-adapter-arm.sh` line 78,
`build-adapter-pdk.sh` line 91 — both list `-I$DEPS/staging/include` first). It is a gitignored
build-input directory, so this is a re-stage, not a source change.

| | sha256 | size |
|---|---|---|
| before | `462bcc8930b65f37c0ada64e49846c52fbb97cc9d9359f768501fcd368a1752b` | 35552 |
| after  | `36b58a83636bdc19de927772ee6a111db0b657b87595ae51d0766e20ff21c199` | 36646 |

`build/webos-oe/adapter-deps.manifest` records the new hash, marks the origin
`npapi-headers+jihad`, and carries the recipe to reproduce it.

**It is a SURGICAL re-stage, not a whole-file swap, and that was a deliberate call.** The 28-byte
declaration lives in the same checkout's debian INSTALL copy
(`adapter-deps/npapi-headers/debian/npapiheaders/usr/include/npapi.h`), but that file differs from
the staged one in more than the touch struct: it also **moves `npPalmApplicationIdentifier` from
`NPNVariable` 2005 to `NPPVariable` 10002** (and `npPalmInputFieldFocused` 2006 → 10006), drops the
`MOZ_X11` guards, and changes `NPP_GetMIMEDescription` to return `char*`.
`BrowserAdapter.cpp` reads the app identifier through
`NPN_GetValue((NPNVariable) npPalmApplicationIdentifier)` on a **device-verified** path (it is how
a variant learns its own identity), so swapping the whole file would have silently changed a
working call's enum value with no device available to catch it. Only the `NpPalmTouchEvent` struct
was taken across; the header carries a comment saying so.

**Verification is the compiler's, in both directions.** `BrowserAdapter.cpp` carries

```c
typedef char jihadTouchAbiGuard[(sizeof(NpPalmTouchEvent) == 28) ? 1 : -1];
```

(moved out of the function body to namespace scope, so it is checked even where nothing calls
`doTouchEvent`).

* POSITIVE: both adapter builds compile clean → the staged header is the 28-byte one.
* NEGATIVE, run deliberately: recompiling `BrowserAdapter.cpp` against an include tree whose
  `npapi.h` is the old 12-byte file fails with
  `BrowserAdapter.cpp:1750: error: size '-1' of array 'jihadTouchAbiGuard' is negative`
  plus four `request for member 'points' in … which is of pointer type` errors. A future re-stage
  that reverts the header breaks the BUILD, not LunaSysMgr.

### 2. Adapter: fence removed, switch added

`render/adapter/BrowserAdapter.cpp`

* `doTouchEvent`'s body is compiled in. `Palm::TouchPointPalm::State` (a header that does not
  exist in this build) is replaced by `JihadTouchState`, which was already declared there and is
  documented as OURS, not the frozen contract's — nothing consumes it
  (`BrowserPageGoanna::touchEvent` parses only `"x"`/`"y"`).
* The gate is `mTouchEventsEnabled && shouldPassTouchEvents()`. `shouldPassTouchEvents()` is
  unchanged (`m_passInputEvents || non-user-scalable viewport`), as the kit describes. Since no
  variant passes `usemouseevents`, that still means **user-scalable=no pages only**.
* **BUG FIXED while enabling.** The TouchEnd/TouchCancelled loop was bounded by
  `changedTouches.length` but indexed `touches.points`. On a release the touches list is the
  SHORTER of the two — the lifted finger is gone from it — so it read past the end of the array.
  That is the real source of the "coordinates in excess of 7,000,000" the surrounding comment
  apologises for; the clamp to `m_touchPtDoc` is kept as a belt, but the index is now
  `changedTouches.points`. Null-`points` guards added to both loops and to `isPointInList`.
* **A missing opt-in was found.** WebKit sends a plugin NO `npPalmTouch*Event` until the plugin
  sets `npPalmEnableTouchEvents` to a non-NULL value (documented above `handleTouchStart` in
  `AdapterBase.h`; nothing in this tree had ever called it). Without that call, un-fencing
  `doTouchEvent` would have been a silent no-op. `AdapterBase::NPN_SetValue` was added
  (`render/adapter/AdapterBase.{h,cpp}`) and the constructor makes the call — **only when the
  switch is on**, so a default build does not even ask the host to generate the events.

**The switch: `JIHAD_TOUCH_EVENTS=1` in LunaSysMgr's environment.** Env rather than a plugin param
or a pref, because the adapter already reads exactly one switch that way (`LOG_BROWSER_ALERTS`) and
because the value has to be decidable in the constructor, before any card JS runs. Only the literal
string `"1"` enables it. Off by default for three independent reasons, any one of which would be
enough: this is NEW capability that stock never shipped (isis and Atlas both fence `doTouchEvent`
the same way), it has never run on hardware, and it is the double-activation class.

When it is on, the adapter logs one line at `LOG_WARNING`:

```
JIHAD_TOUCH_EVENTS=1: DOM touch forwarding ENABLED (npPalmEnableTouchEvents rc=%d) — …
```

### 3. Daemon: the double-activation suppressor

`render/goanna/GoannaRenderPage.{cpp,h}` — `TouchEvent` (both overloads) now **returns** the
`aPreventDefault` out-param of `nsIDOMWindowUtils::sendTouchEventToWindow` instead of discarding
it, and returns `false` if the dispatch itself failed (a failed dispatch is not a consumed
gesture; reporting it as one would suppress the pen events too and the tap would do nothing).

`render/goanna/BrowserPageGoanna.{cpp,h}` — `touchSuppressesMouse()` plus five members, armed
from the pump where the touch events are dispatched.

**Why a suppressor is needed at all:** LunaSysMgr generates pen events for every finger whether or
not the plugin also asked for touch. Gecko's own touch-to-mouse suppression only ever suppresses
mouse events it synthesized itself, so it cannot see the relationship. A page that
`preventDefault`s touchstart would get its gesture AND a full mousedown/mouseup/click.

**The correlation rule, since the two streams share no id, no sequence number and no timestamp.**
What they DO share is the input queue: touch and pen both land in `mPendingMouse`, so their
relative ORDER is total.

1. **Sequence state** (primary). From a touchstart content consumed until the matching touchend,
   every queued `PM_DOWN`/`PM_UP`/`PM_MOVE`/`PM_CLICK`/`PM_CONTEXTMENU` is dropped. No geometry —
   during a live consumed gesture there is nothing else the pen stream can be.
2. **Time + distance window** for the tail. The pen-up and the synthesized click arrive just AFTER
   the touchend, when rule 1 has disarmed. For **400 ms** after the touchend, a mouse event within
   **48 document px** of the last touch point is dropped too.
3. **Runaway cap.** If a touchend never arrives (lost cancel, card switch, navigation mid-gesture —
   a navigation clears the queue but cannot know a finger is still down), rule 1 alone would
   suppress ALL pointer input for the life of the page. Rule-1 suppression therefore also expires
   **5000 ms** after the touchstart, logging one line when it does.

Comparison happens on DOCUMENT coordinates, before the pump's `docToViewport` — the touch anchor is
in document space too, and mapping one and not the other would silently break the distance test.

The suppressor needs **no switch of its own**: it only ever arms when a touch command arrives,
which only happens when the adapter's switch is on.

**Failure modes, stated rather than hidden.** None of this has run on hardware; the ordering and
both constants are reasoned from the code, not measured.

* A page that consumes touch**move** but not touch**start** (a scroll-hijacking list) is not
  suppressed. The rule keys on touchstart, matching the W3C compatibility-event rule. Such a page
  gets the mouse stream exactly as it does today — current behaviour, not a regression.
* A real second finger landing on a different element within 400 ms of a consumed gesture ending
  AND within 48 px of where the first ended loses its click. The slop is deliberately small (a
  fingertip's own wander) to keep that window narrow.
* **If the device delivers the pen-down BEFORE the touchstart, that one leading `mousedown`
  escapes.** Its mouseup/click still do not, so no click is fabricated — but a page watching
  mousedown alone would see one. Establishing the real order needs a device; this is the single
  most likely thing to be wrong.
* The window is wall-clock (`gettimeofday`, as everything else in this file is). A clock step
  during a gesture mis-sizes one window; it cannot latch, because rule 1 uses time only as a cap.

## Builds (all run this session, from the repo root)

| build | result |
|---|---|
| `build-daemon-arm.sh` (podman) | exit 0. `check-yap-contract.sh` PASS — "73 commands (incl. the 0x1600 Jihad addition), 58 messages, both sides agree". Links wave-1's `JihadCertStore.cpp`. Artifact `out-arm/jihad-browserserver-arm`, 310312 B, ARM EABI5, stripped. |
| `build-adapter-arm.sh` (crosstool) | exit 0, all three variants, shim + impl, identity checks pass, GLIBC floor 2.4. |
| `build-adapter-pdk.sh` (gcc 4.3.3 — **the build the .ipk ships**) | exit 0, all three variants, GLIBCXX_3.4 / GLIBC_2.4. |
| `build-variant-ipk.sh` | exit 0. enyo **1.0.4** (45392 KiB), mochi 1.0.0 (45905 KiB), mojo 1.0.0 (44638 KiB). |

`out-ipk/` still holds the stale enyo `1.0.0`–`1.0.3` packages from earlier sessions and **those do
not contain `libasound.so.2`** — a daemon installed from one of them will not start. Install
`net.riverstonerelay.jihad-browser_1.0.4_all.ipk`, not "the enyo .ipk".

Warnings are the pre-existing `std::auto_ptr` deprecations only. No wave-1 (cert store / ALSA)
build breakage occurred, so nothing had to be fixed forward.

### libasound bundling — VERIFIED PRESENT, no fix needed

Wave-1's audio backend made `libasound.so.2` a hard `DT_NEEDED` of the ARM `libxul.so` (confirmed
by `readelf -d`), and the daemon will not start without it. The pre-existing `device-bundle/` did
NOT contain it, so this was a real risk. `build-variant-ipk.sh` re-runs `make-device-bundle.sh`,
whose `readelf -d` closure walk picked it up from the sysroot
(`arm-sysroot/root/usr/lib/arm-linux-gnueabi/libasound.so.2`). Verified by listing the packaged
payload, not by trusting the walk:

```
ar p …jihad-browser_1.0.4_all.ipk data.tar.gz | tar -tzf - | grep asound
  ./usr/palm/applications/net.riverstonerelay.jihad-browser/deviceroot/hl/libasound.so.2
```

Present in all three .ipks. `libasound.so.2`'s own NEEDED set (libm/libdl/libpthread/librt/libc/
ld-linux) is already in the bundle. **`make-device-bundle.sh` needed no change.**

Artifact freshness was checked rather than assumed: the daemon inside the bundle md5-matches
`out-arm/jihad-browserserver-arm`, the `BrowserAdapterImpl.so` inside the enyo .ipk md5-matches
`adapter-deps/build-pdk/enyo/BrowserAdapterImpl.so`, and both the daemon and both adapter builds
contain the new strings (`touch suppressor`, `JIHAD_TOUCH_EVENTS`).

## What is still open

* **Device verification of the whole path.** Nothing here has run on hardware. The order of the
  pen and touch streams, the two suppressor constants, and whether the device's WebKit honours
  `npPalmEnableTouchEvents` from this adapter are all unmeasured.
* **The R3 pinch/tap criterion stays OPEN**, and not because of this work: as the 2026-08-10
  analysis established, neither pinch nor tap rides `doTouchEvent`. What holds that box open is
  the human two-finger gesture check, **T-151 (BLOCKED-EXTERNAL)**.
* A true `touchcancel` still needs a new `PM_` value; type 3 currently ends the sequence as a
  `touchend`. The engine already accepts the string.
* `shouldPassTouchEvents()` still resolves to user-scalable=no pages only, because no variant
  passes `usemouseevents`. Anyone testing this on a device must pick such a page or the switch
  will look broken.

## Device test procedure, for whoever has hardware next

1. Install a .ipk from `build/webos-oe/out-ipk/` (Preware or WebOS Quick Install — `palm-install`
   does not run the control scripts).
2. Put `JIHAD_TOUCH_EVENTS=1` in LunaSysMgr's environment and restart it. The adapter runs INSIDE
   LunaSysMgr, so a daemon restart is not enough.
3. Confirm the one-line enable log, and check the `rc=` in it: a non-zero rc means the host
   refused the opt-in and no touch event will ever arrive.
4. Load a page with `user-scalable=no` (otherwise `shouldPassTouchEvents()` is false and nothing
   forwards) that logs touch and mouse events, and tap it.
5. The thing being tested is what does NOT happen: with a `preventDefault`ing touchstart handler,
   no `mousedown`/`mouseup`/`click` should follow. `[jihad-bs] touchstart consumed by page` and
   `[jihad-bs] touch consumed gesture — dropping mouse` in the daemon log are the observables.
6. Then remove the `preventDefault` and confirm the mouse events come back — a suppressor that
   never disarms is the worse failure and is invisible from step 5 alone.
