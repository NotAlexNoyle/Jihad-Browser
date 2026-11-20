# Find on Page, in each interface (2026-11-19)

cavekit-find-on-page.md (T-674..T-678), the owner's "new kit: per-variant find-on-page". Before it, in all three
interfaces: Previous searched forward or was missing (Mojo), nothing said how many matches or that there were none, and
Enyo's bar searched only on Enter.

## What changed

- **Adapter:** `findInPage(text, backwards)`. The IPC's `FindString` always carried the direction; the adapter sent
  forward only. The YAP contract is unchanged.
- **Daemon:** `findString` asks the page's component to count (`findCount`), and Done clears (`findClear`). The answer
  (`find-result`: index, total, more, went round) goes out on the notification channel **without the text**. Neither the
  channel (a public bus) nor the daemon's log carries what was typed: the log gives its length only. Ctrl+F from a page
  sends `find-open` unless the page's keydown takes the key. A step goes on from the frame of the last match; after a
  match, no other frame keeps a selection. Case and whole-word are reset on every search (a page's `window.find()`
  sets them on the same finder).
- **The component's count** (`jihadPageCommands.js`): every match in the page and its frames, with the engine's range
  finder, capped at 1000 matches and 300 ms. The current match is found by its selection, or by its place in a text
  field's value.
- **Each interface's bar:**
  - found as you type, Enter is Next, Prev goes back;
  - "n of m", "No matches" (the field marked), and "from the top" / "from the bottom" when it goes round;
  - Done clears and puts the keyboard away;
  - a navigation closes the bar;
  - opened again, the last word is offered selected;
  - opened from the app menu, the long-press menu and Ctrl+F.
- **Ctrl on this device is the mask 0x40** (measured with Ctrl+F, G and H, both Ctrl keys). The daemon had read it as
  Alt since T-658, so every Ctrl+letter reached pages as Alt+letter. 0x20 is the window server's own Ctrl+V; Alt sets
  no bit. `cavekit-input-bridging.md` R2 was corrected, and `keyboard-modifiers` now reads Ctrl's mask.

## Found on the way (each a frame or a log line of this round's runs)

- **Enyo:**
  - its Input has no `changeOnKeypress`, so the bar searched on Enter only (now `changeOnInput`);
  - the bar stood under the system keyboard (the window now resizes while the bar shows);
  - with a hardware keyboard nothing resizes, so the page view kept its first measured height and the page was drawn
    over the bar (the view is now re-fitted when the bar shows or hides).
- **Mochi:**
  - the count's text grew in a FittableColumns that does not reflow, so the buttons slid right; a tap's blur then
    reflowed them under the finger and the tap was lost (the count now has a fixed width);
  - Done threw on `resized()`, which is `resize()` in this Enyo, so the match was never cleared;
  - one result notification was lost once (Prev/Next now work from the first letter whatever arrives).
- **Mojo:**
  - Enter never reached the field: Mojo stops it and turns it into a Tab unless the field consumes Enter (now
    `requiresEnterKey`, the address field's setting);
  - the address and find strip slid away with the page (it now stays while the bar shows);
  - Mojo reads key 17 (Ctrl) as the phones' Sym key and opened its accent picker on the find field after Ctrl+F (that
    release is now swallowed);
  - its count grew ("6 of 6 · from the bottom") in an inline row and pushed the buttons right, so the next tap on Next
    fell on Prev (a fixed width now, going round on a second line).
- **The daemon:** a resize while the find's scroll was in flight put the page back where the card last had it. Prev
  and Next keep the field's focus, so the keyboard stays up.
- **The suite:** a kernel keyboard left by a failed teardown hid the system keyboard for the scripts after it (the
  fixture now ends any it finds); `fl_vkb` (the system keyboard on screen) is distinct from `fl_kb` (a field focused).

## Codex review (`codex-review.md`, 2026-11-19), and what was done

| # | finding | done |
|---|---|---|
| 1 | the query on a public bus, and in the log | no text in the answer or the log |
| 2 | one card takes another's answer | taken only by the card in front with a search out; the daemon answers in order |
| 3 | 300 ms read only after a match | read after every search and frame; one search over a long text with no match is still one search |
| 4 | a clear counted as a search out | clears are not counted; the count of searches out resets on open and Done |
| 5 | a text field's match: no index, kept after Done | placed by the field's position; Done collapses the field's selection |
| 6 | a page's `window.find()` options kept | reset on every search |
| 7 | reopened: Next stranded, Enter searched twice | a word not yet searched is searched once by Next, Prev or Enter |
| 8 | escaping past the notification's limit | no text in the answer |
| 9 | Ctrl held across cards | `ctrlKey` decides |
| 10 | one match never said "went round" | said |

## The scripts

| script | what it checks (cavekit-find-on-page.md) | Enyo | Mochi | Mojo |
|---|---|---|---|---|
| `find-steps` | R1-R5: the app menu, "zebra" typed 1 of 6, Enter, Next, Prev, round both ways, above the keyboard, Done | pass | pass | pass |
| `find-none` | R4: "qqxz", No matches at once, the field marked, Prev and Next off | pass | pass | pass |
| `find-text` | R2, R4: any case, Straße, café, привет, καλημέρα, a frame, a text field; a zebra the page adds counted at the next search (7) | pass | pass | pass |
| `find-reopen` | R2, R5: the dark band's match seen; reopened, the last word offered and replaced; Next on an offered word searches it once | pass | pass | pass |
| `find-navigate` | R5: a load closes the bar and clears the match | pass | pass | pass |
| `find-ctrl-f` | R1: Ctrl+F on a hardware keyboard, on a card just opened and with a page's field focused | pass | pass | pass |
| `find-own-pages` | R1: about:settings ("personas") and about:data ("site data") | pass | pass | pass |
| `find-hold-menu` | R1: the long-press menu's Find on Page | pass | pass | pass |
| `keyboard-modifiers` | cavekit-input-bridging.md R2, Ctrl's 0x40 read as Ctrl | pass | pass | pass |
| `flash-key-modifiers` | cavekit-flash.md R7, Ctrl+V in a movie's field | pass | pass | pass |

30 runs, all passing (`finger-report.md`; Mojo's eight were rerun with `--merge` after its count's width was fixed: a
count grown by "from the bottom" pushed its buttons right, and the next tap on Next fell on Prev). The kept frames were
read by eye: the match is blue under white on the black band and on white in the three shells; "No matches" is red with
Prev and Next off; going round says "from the bottom"; Ctrl+F's bar stands where the interface puts it, with no accent
picker in Mojo.

Seen, not a defect of find: Mochi's frame shot at once after the jump to the page's foot (6 of 6, "from the bottom") is
still blank; the paint follows (the round before showed the same shot painted, and the next shot is).

Open: every Enyo launch logs "Uncaught SyntaxError: Unexpected end of input, index.html:0" beside an XHR NETWORK_ERR in
`BrowserApp.create` (in every rotated syslog of the day). The card works; unread.
