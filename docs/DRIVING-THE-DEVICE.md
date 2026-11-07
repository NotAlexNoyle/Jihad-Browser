# Driving the browser on the device

Moved here from `context/kits/cavekit-overview.md` on 2026-11-07 (a kit holds requirements; these are how-tos).
The finger suite (`tools/device/finger/`, cavekit-finger-testing.md) is the current way to drive the card; the
inject channel below is what it stands on.

### Working on the card UIs
`build/webos-oe/push-card-js.sh <enyo|mochi|mojo> <files…>` is the card-JS loop: it stamps the
push, md5-verifies it on both sides, closes the card by its real `processid`, restarts LunaSysMgr
(the WebAppMgr JS cache is real), relaunches, and **fails unless the new stamp appears in the
device log**. A screenshot is not proof a card is alive (fb1 holds the last painted frame) and an
on-disk md5 is not proof a card reloaded — the stamp is. Hold stdin open on every `novacom run`
whose output matters (`sleep 4 | novacom run …`).

### Driving the browser without a human
Most acceptance criteria here are reachable without anyone touching the screen, but only if you
know the four traps below. All of this is gated on `$JIHAD_INJECT`, which stays OFF unless set.

**`jsurl` does NOT execute in a chrome document.** It loads a `javascript:` URL with the system
principal, which works in content — and in `about:addons` it does nothing at all:
`LoadURIWithOptions` returns NS_OK, so the inject line prints **`ok=1`**, and no code runs, no
error is raised, nothing reaches the console. Measured 2026-08-04 three ways (setting
`document.title`, `Components.utils.reportError`, a `gViewController` call). Treat `ok=1` from
`jsurl` as "the load was issued", never as "the script ran".

**So click the real control.** `rect`/`clickid` resolve an element and click its own centre in
viewport space, which is also zoom- and scroll-independent. Three forms:
`<id>` (getElementById) · `sel:<css>` (querySelector — an about:addons row is
`richlistitem[value="<addon id>"]`) · `anon:<css>|<anonid>` (XBL **anonymous** content, which is
where a row's `enable-btn`/`disable-btn`/`remove-btn` live — no id reaches them). Clicking the
real button is also the more honest test: it exercises the same command path a finger does.

**`clickid` is not a tap.** `clickid`/`dblclickid`/`clickoff` resolve an element and call the
engine's `ClickAt` DIRECTLY, skipping the input QUEUE — so anything `pump()` does after a click
does not happen: no link-clicked message, no click-nav re-drive, no `<select>` popup emission.
That is fine for reaching a chrome control, and wrong for anything about content navigation
semantics, where `click <x> <y> <n>` (card coordinates, the path a finger takes) is the one to
use. Measured 2026-08-04: the same anchor tap emitted nothing and ended `NS_BINDING_ABORTED`
through `clickid`, and emitted `linkClicked` and navigated through `click`.

**A dialog must be answered the way a PERSON would.** The daemon blocks on a reply FIFO with a
60 s deadline; a harness that answers in 300 ms proves almost nothing about the human path, and
one that never answers takes the default. `JIHAD_DIALOG_MS` shortens the deadline for tests that
WANT the default fast. When a test's whole point is the human path, **answer late on purpose** —
that is what caught the 5 s deadline that had been silently denying every real dialog
(cavekit-browser-services.md R3).

**On device, the card creates no engine page until it navigates.** The supervised upstart job
deliberately does not set `JIHAD_INJECT`, so driving the device means stopping the job and
running an ad-hoc daemon with the same environment plus `JIHAD_INJECT=1` — then launching the
card **at a URL**: `palm-launch -p '{"target":"…"}' <appid>`. Launch it bare and every inject
command answers `inject: no page`, because the start page is card-side HTML and no
`BrowserPageGoanna` exists yet. Kill the ad-hoc daemon with **SIGTERM** (not `-9`) when you are
done, or the add-on database and prefs never flush, and restart the job. Its process name is
`ld-2.23.so`, so `killall jihad-browserserver` matches nothing.
