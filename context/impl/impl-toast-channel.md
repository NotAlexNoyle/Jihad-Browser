# The non-blocking message channel — a card toast over a Luna subscription

**T-146 (build the channel) + T-148 (move the informational call sites), 2026-08-15.**
Closes the last open criterion of `../kits/cavekit-gre-widgets.md` R5 to `[~]`.
**No device was available. Everything below is either a desktop-harness run or a source read,
and each claim says which.** The call-site classification table is appended to
`impl-gre-widget-inventory.md` rather than duplicated here.

## The problem, stated precisely

Every message the daemon could send the user went through `msgDialog*`, and that path does not
merely draw a modal — it **stops the daemon**. `BrowserPageGoanna::OnDialog` creates a FIFO,
sends the card its path, and then `awaitDialogReply` POLLS that pipe until the card answers or
the deadline expires (`render/goanna/BrowserPageGoanna.cpp:1150-1235`). For a question — "may
this site install an add-on?" — that is the correct price. For "Cookies cleared." it is a
render stall bought for nothing, and for anything the user ignores it is the full deadline.

So the daemon needed a second, one-way channel. It had none.

## The shape that was chosen, and the two that were not

**CHOSEN: a daemon Luna subscription, drawn card-side as a transient toast.**

**REJECTED — a new YAP message.** `../kits/cavekit-ipc-contract.md` R1's second criterion was
amended on 2026-08-10 to permit additions, but only ones that are optional in both directions,
documented, AND carry their own **user authorisation**. There is no authorisation for this one.
The amendment says in terms that it "is NOT a licence to grow the contract". Luna needs no
contract change at all and has direct precedent in this same file: `getChromePrefs` was added
on 2026-08-06 (`../kits/cavekit-preferences-ui.md` R5, "the Luna route").

**REJECTED — a `notificationbox` in a chrome page.** R5's FIRST criterion is about exactly that
widget and it is closed (T-139): `about:preferences` raises real bars and dismisses them. It
cannot carry these messages, because a `notificationbox` only exists inside a chrome document we
author and **none of the three named events happens while that document is on screen**. "Cookies
cleared" is raised by a button in the CARD's own Preferences view; an add-on installs while an
arbitrary content page is displayed. A bar in a page nobody is looking at is not a message.

## The daemon side

Three pieces, in the order a message travels.

**1. `jihad::PostNotification(category, text)` — `render/goanna/DialogService.{h,cpp}`.**
Fire-and-forget: no reply pipe, no deadline, no return value a caller can wait on. It lives
next to `DialogSink` deliberately — one file owns "how the daemon talks to the card", and the
blocking and non-blocking channels should be read together. With no sink installed it writes
ONE line (`[jihad-bs] notify (no channel): <cat>: <text>`) and returns.

*Placement note, because it looks arbitrary and is not.* It went into `DialogService.cpp`
rather than a new file because **every one of the 39 desktop build scripts that links
`GoannaRenderPage.o` also links `DialogService.o`** (checked script by script). A new
translation unit would have had to be added to all of them; this way the count of build-script
edits for the whole task is zero.

**2. The JS door — the `"jihad-notify"` observer, same file.** Bundled components cannot call
C++, and a new XPCOM component to carry two strings would be a new binary interface for no
gain. The subject is a property bag with `category` and `text`, the same idiom as the existing
`"jihad-xpi-confirm"` observer beside it. Unlike that one it **writes nothing back and answers
nothing**, so `notifyObservers` returns as soon as the payload is handed on. Content-controlled
text is sanitised on BOTH sides — control characters to spaces, hard length budget — because an
add-on name comes out of a downloaded `install.rdf` and a stray control character would break
the JSON line it becomes on the wire.

**3. The Luna sink — `render/browserserver/JihadLunaService.cpp`.** A new `notifications`
method on the existing per-variant service, registered on **both** method tables:

```
->  {"subscribe":true}
<-  {"returnValue":true,"subscribed":true}              (once, immediately)
<-  {"category":"privacy","text":"Cookies cleared."}    (zero or more, later)
```

The pushes carry no `returnValue` — they are not answers to anything — so a card tells them
from the handshake by the presence of `text`.

- **PUBLIC bus.** An app card is a public-bus caller; that was measured on device 2026-08-06,
  and it is why `clearCookies` from the card had never once worked before the palm-service
  registration landed. A private-only method would simply never be reached.
- **What that exposes, weighed rather than assumed.** Any app on the device may subscribe and
  will then see the same one-line statements already on the user's screen. Nothing here carries
  a URL, a page title, cookie contents or the home page — which is precisely why
  `getChromePrefs` stays PRIVATE-only in the table two lines below it. This method must never
  become a general log tap.
- **Which handle to reply on.** `LSSubscriptionReply` takes an `LSHandle*` and a palm-service
  registration serves two buses. Rather than resolve two more unconfirmed symbols
  (`LSPalmServiceGet{Public,Private}Connection`), the subscribe callback's own `sh` argument is
  recorded — a handle only lands there because a real subscriber called through it. Two slots,
  because there are exactly two buses.
- **Degradation.** `LSSubscriptionAdd`/`LSSubscriptionReply` are `dlsym`'d and NULL-checked
  like every other entry point in that file. If either is missing, `notifications` answers
  `{"returnValue":false,"subscribed":false,…}`, the sink is never installed, and
  `PostNotification` falls back to its log line. `clearCache`/`clearCookies`/`getChromePrefs`
  are unaffected — the channel is an added capability, never a startup requirement.

### The symbol risk, named rather than buried

The signatures came from the webOS 3 sources in this workspace —
`luna-sysmgr/Src/base/DisplayManager.cpp:1836` and `HapticsController.cpp:161` for
`LSSubscriptionAdd`, `AmbientLightSensor.cpp:403` and `application/ApplicationInstaller.cpp:3657`
for `LSSubscriptionReply`. **Those are LS1-era callers and this device runs luna-service2**,
which is the exact class of source that produced the `LSMethod`-has-three-fields bug (a
registration that succeeded while registering nothing). What makes this case different is that
neither function takes a struct BY VALUE and no layout is involved: both are pointer-and-string
calls whose shape is identical across the two generations. **Presence on this device is still
an assumption** — there is no `liblunaservice.so` anywhere on this host to check against, and
novacom was down.

## The card side (all three shells, none of it run)

One toast implementation per shell, deliberately near-identical, plus one subscription.

| Shell | Toast | Subscription | Service |
|---|---|---|---|
| Enyo 1.0 | `app/source/JihadToast.js` (+ `depends.js`) | `notificationService` (`enyo.PalmService`, `subscribe`+`resubscribe`) in `app/source/BrowserApp.js`, opened in `create()` | `palm://net.riverstonerelay.jihadBrowser/` |
| Mochi (Enyo 2) | `app-mochi/source/JihadToast.js` (+ `package.js`) | `enyo.jihad.lunaSubscribe` (new, in `JihadServices.js`) called from `JihadBrowser.js` `create()` | `palm://net.riverstonerelay.jihadBrowserMochi/` |
| Mojo | `app-mojo/app/models/jihad-toast.js` (+ `sources.json`) | `controller.serviceRequest` with `parameters:{subscribe:true}` in `main-assistant.js` `setup()` | `palm://net.riverstonerelay.jihadBrowserMojo/` |

Design points that are not obvious from the code:

- **Plain DOM, not a framework control, in all three.** The toast is transient, owns no state,
  belongs to no view, and must float above the WebView wherever the card happens to be. A kind
  or widget would buy nothing and would have to be added per pane. The precedent that card-side
  HTML composites over the NPAPI plugin surface at all is the `<select>` popup, which is
  device-proven (`impl-select-popup-2026-08-03.md`).
- **No box-sizing and no flexbox.** This card's WebKit needs `-webkit-` prefixes for both and
  silently drops the unprefixed forms. Width comes from `left` + `right` on a positioned block,
  which needs neither.
- **A timer, not a `transitionend` listener, ends the fade.** If transitions do not fire in this
  card a listener would never run and the node would sit at `opacity:0` forever, invisible and
  still occupying the bottom of the screen. This is the same trap T-139 hit with
  `notification.xml`'s close button, in a different guise.
- **The handshake is filtered out.** `{"returnValue":true,"subscribed":true}` has no `text`;
  showing it would put an empty toast on screen at every card launch.
- **Newest wins.** A second message replaces the first and restarts the clock rather than
  queueing. These are status lines.
- **Mojo specifics.** `Mojo.Log.error`, not `.warn` (warn does not reach the device log in this
  framework build). `controller.serviceRequest`, not a bare `Mojo.Service.Request`, so the
  subscription dies with the scene. `JihadToast.detach()` in `cleanup()`, because the toast
  holds a node in the scene's document and a pending timer in its window.
- **This is the first `palm://` URI the Mojo variant uses.** It is our own per-variant service,
  never `com.palm.browserServer` (which would clear the wrong browser). Its frozen-set comment
  was updated to say so rather than left to read as still-true-by-omission.

## What is verified, and how

**Desktop harness, in the pinned container. No device.**

| Claim | Evidence |
|---|---|
| The moved call site no longer blocks | `xpi_mismatch_test`: **8 checks, 0 fails, exit 0**. `dialogs=0 notifications=1`; text is the incompatibility message, category `addon`. Both halves asserted — a regression that raised BOTH a toast and a dialog would fail. |
| "Add-on installed" reaches the channel | New `JIHAD_XPI_ACCEPT=1` mode: **6 checks, 0 fails**, `page status=0`, one notification `Jihad T103 Good installed.`, 0 alerts. |
| "Cookies cleared" / "Cache cleared" reach it, with no dialog, and the flow completes | `services_test`: 2 notifications (`Cache cleared.`, `Cookies cleared.`, both `privacy`), **0 dialogs**, both calls returned. |
| The no-Luna degrade path is a clean no-op | Same run, sink detached: exactly one `[jihad-bs] notify (no channel): privacy: Cookies cleared.`, no crash, no dialog, nothing delivered to the detached sink. |
| The daemon still starts with no Luna at all | `build-daemon.sh`: `DAEMON_UP`, and `luna: liblunaservice.so unavailable ((null)) — no Luna service`. |
| Nothing in the frozen wire moved | `build-daemon-arm.sh` cross-builds clean and its guard prints `YAP contract OK: 73 commands (incl. the 0x1600 Jihad addition), 58 messages, both sides agree.` |
| Every touched card file parses | `node --check` on all 10 JS files + `json.load` on `app-mojo/sources.json`. |

**The instruments were proved able to fail.** `JIHAD_SVC_NEG=148` expects notification texts no
build emits: 1 failure becomes 3. `JIHAD_XPI_GOOD=1` (matching add-on, declined at the confirm)
gives 1 confirm, 0 alerts, **0 notifications** — so the observer's `appDisabled` filter is doing
real work and the positive result is not "it fires on everything".

**Two pre-existing failures, measured rather than assumed, because both would otherwise be
blamed on this work.**

- ***`services_test` still exits 4.*** *Its PIXEL half (`green=0 blue=0`, the JS-disabled click
  check) was **already failing at HEAD** — verified by restoring the unmodified file from git,
  rebuilding and running it, which produced the identical `green=0 blue=0` and exit 4.*
- ***`dialog_test` fails too — `alerts=0 text='' confirms=0 green=0`, exit 4 — and it is NOT this
  change.*** *That one had to be proved the hard way: a clean `git show HEAD:` revert of the
  three daemon files does not even COMPILE, because this tree's uncommitted wave-1/2 work has
  already moved `GoannaRenderPage.h` ahead of its `.cpp` at HEAD (`no declaration matches
  AcceptCurrentCert / TouchEvent`). So the baseline was taken by stripping ONLY this task's own
  hunks — the `DialogService.h` include and the two `PostNotification` calls in
  `GoannaRenderPage.cpp`, and the two notify `AddObserver`/`RemoveObserver` lines in
  `DialogService.cpp` — and rebuilding. **Byte-identical failure.** The signature also explains
  itself: `green=0` means the harness page never rendered, and a page that never ran cannot call
  `alert()`. Same class as the `services_test` pixel half.*

Both are the recorded desktop-harness readback dead end, not regressions from this work.

## What is device-gated

Nobody has seen a toast. Three independent unknowns:

1. **Do `LSSubscriptionAdd`/`LSSubscriptionReply` exist in this device's `liblunaservice.so`?**
   Handled by NULL-check + degrade; unconfirmed. The daemon log settles it in one line at
   startup (`toast channel ON`/`OFF`).
2. **Does a card's subscribe actually arrive?** It is on the public bus for the measured reason
   and the role files already ship, so it should behave exactly like `clearCookies` — but
   private-only looked fine too until somebody measured it.
3. **Does the toast composite over the WebView?** Good precedent (`<select>` popup), not a
   measurement of this element.

**One run closes all three:** launch each variant, Preferences -> Clear cookies, look at the
screen, and read the daemon log. `notify: privacy: Cookies cleared. (pushed)` means the daemon
did its part and the card did not; `(no subscribers)` means the subscription never arrived; the
absence of the line entirely means the post never happened.

**Deploy note.** The moved JS lives in `render/goanna/components/jihadInstallPrompt.js`, which
`push-engine-update.sh` does NOT ship (it pushes only `libxul.so`, the daemon and `goanna.js`).
Testing the add-on message after a dev-loop push would measure the OLD component and read as
"the toast does not work". It needs a bundle or `.ipk` push. The card files need
`push-card-js.sh` with the new filenames, or an `.ipk`.

## Files touched

**Daemon**
- `render/goanna/DialogService.h` — `NotificationSink`, `SetNotificationSink`, `PostNotification`.
- `render/goanna/DialogService.cpp` — the sink, the post, the `"jihad-notify"` observer + its
  registration/removal alongside the existing XPI observer.
- `render/goanna/GoannaRenderPage.cpp` — `ClearCache`/`ClearCookies` acknowledge themselves.
- `render/goanna/components/jihadInstallPrompt.js` — `_alert` -> `_notify`; new
  `addon-install-complete` observer.
- `render/browserserver/JihadLunaService.{h,cpp}` — subscription entry points, the
  `notifications` method, the `LunaNotifier` sink, sink lifecycle, shared JSON escaper.

**Tests**
- `render/goanna/test/services_test.cpp` — 7 new checks + `JIHAD_SVC_NEG`.
- `render/goanna/test/xpi_mismatch_test.cpp` — asserts the notification instead of the alert,
  asserts 0 alerts, adds `JIHAD_XPI_ACCEPT=1`.

**Cards**
- `app/source/JihadToast.js` (new), `app/depends.js`, `app/source/BrowserApp.js`.
- `app-mochi/source/JihadToast.js` (new), `app-mochi/source/package.js`,
  `app-mochi/source/JihadServices.js`, `app-mochi/source/JihadBrowser.js`.
- `app-mojo/app/models/jihad-toast.js` (new), `app-mojo/sources.json`,
  `app-mojo/app/assistants/main-assistant.js`.

**Docs**
- `../kits/cavekit-gre-widgets.md` R5 last criterion + changelog.
- `impl-gre-widget-inventory.md` — the call-site classification table.
