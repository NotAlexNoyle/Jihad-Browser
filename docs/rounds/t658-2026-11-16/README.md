# T-658: the clipboard between a movie and the page (2026-11-16)

The question was whether a movie's `System.setClipboard` not reaching the card's Paste is this browser's defect or
the player's limit, and what happens the other way (a page's copied text into a movie's field).

## The system clipboard is a file

`/var/tmp/webkit-clipboard`: the card WebKit's shared file (libWebKitLuna's `SharedClipboardFile`), plain text,
written at each copy and read at each paste (read on the device: it held the address the suite's
`address-select-all` copies). A test can therefore prime it and read it back: `clipboard-set TEXT`,
`expect clipboard TEXT`. Both scripts below prime a text of their own, so neither passes on a leftover.

## Out of a movie

The instrumented movie (`jihad-flash-instr.html?movie=text&clip=1`) calls `System.setClipboard("jihad-clip-7391")`
on a press of its left band. The page's own field is then held and Paste tapped.

| browser | the movie's press | the system clipboard after | Paste in the page's field |
|---|---|---|---|
| the stock browser (`stock-bench.sh up`, the system's BrowserServer and the same player) | taken (`down 61 260`; the page's `clip` row was under the keyboard in the frame, not read) | not read | the clipboard's older text (`stock-after-paste.jpg`) |
| Enyo, Mochi, Mojo | taken, `clip set` | the primed text, unchanged | the primed text |

The stock reading is of the symptom (Paste gives the older text after a press that the movie took); that
`System.setClipboard` itself ran there was not read. The verdict rests on the player: `libflashplayer.so`
(10.3.185.65) has 423 undefined symbols and none is a clipboard's, and no string of it names the clipboard's file
or a service for one (`player-clipboard-imports.txt`; the stock host's `libWebKitLuna.so` has `palm_clipboard_*`
and the file, and gives the player neither): its Clipboard is its own. A limit traced to the player, by the kit's
verdict rule. `flash-clipboard-out` checks that the system clipboard still holds the primed text after the
movie's call and that Paste puts exactly that in the field; the day the movie's text arrives, it fails.

## Into a movie

A movie has no editing menu on this player; the way in would be Ctrl+V from a hardware keyboard (`kernel-chord 29
47` on the emulated kernel keyboard), with a text primed on the system clipboard. What that found:

1. A DEFECT, fixed. A key pressed with a modifier held arrives with a mask that is not npPalm's: 0x80 with Shift,
   0x20 with Ctrl (Alt's 0x40 by the same layout, not measured). The daemon forwarded it as it came: the player
   got Ctrl+V with mods 0, and the engine read 0x20 and 0x80 as CapsLock and FnLock. `BrowserPageGoanna::
   palmModifiers` reads the mask into npPalm's bits at the top of keyDown and keyUp, with no state kept (a first
   version kept the held modifier keys instead; the review of this round found its expiry and its held-back
   releases wrong, and the mask makes it unnecessary). The player's V now arrives with mods 0x1.
2. A DEFECT, fixed. Shift went to the player as the DOM's 16, which its key handler takes for a character: a
   movie's field showed a control glyph at each press of Shift. It now goes as webOS's raw 0x80, which the handler
   knows (`player-key-handler.txt`): ActionScript reads keyCode 16 and the field takes nothing; Shift+b types B.
3. Ctrl stays the DOM's 17, unlike the stock host, which hands the player webOS's raw 0x82. That was tried here:
   ActionScript read keyCode -1 and the focused field was emptied (the player's own reading of that key, not
   looked into further). A game's Ctrl (17, a common fire key) is worth more.
4. THE LIMIT. With Ctrl reported, V still types "v" and none of the primed text: the player's key handler has no
   branch that turns a key into a paste (`player-key-handler.txt`), and no clipboard import to paste from. Traced
   to the player; not run on the stock browser.
5. DEFECTS in a page's field, found by the review and fixed: Shift, Ctrl and Alt alone went into a focused field
   as U+0080-U+0083 (they are swallowed now), and Shift+Tab never went backward (the mask's Shift was 0x80; and
   from a textarea Tab only ever inserted a tab: Shift+Tab now leaves it for the field before).

`flash-key-modifiers` holds 1, 2 and 4, `keyboard-modifiers` 5, in the three shells.

Frames: `stock-hold-paste-menu.jpg`, `stock-after-paste.jpg` (the stock browser, by the framebuffer).
