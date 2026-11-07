# cavekit-chrome-look.md history, moved out of the kit by the 2026-11-07 sweep; verbatim.

## R2, criterion 5 (long lists scroll) (removed 2026-11-07)

*(Device: the daemon reports the taller page; the pan

  needs a finger and a long list — the owner's test.)* *(2026-10-25, T-527, Enyo by finger with twelve
  test extensions sideloaded: before, the list did NOT pan — the root's min-height never reached the
  reported content size (a XUL root is laid out at the window's size; the card was told 980x1202, the
  window), and on the GPU path the card's scroll could not move a page with no root scroll frame anyway
  (every frame showed its top). Now jihadPersonas.js puts the list's whole length on the root
  (`jihad-content-height`, measured from its items, not the window), the daemon makes the window that tall
  (980x1540 for thirteen rows) and moves a XUL page by the compositor's world offset in both lanes; a page
  that gets shorter (an add-on's details) has the card's scroll clamped. Two drags reach Pad 12 at the
  card's bottom, a flick returns to the top, a tap on Pad 12's More opens its details from their top.
  Mochi and Mojo with the next packages.)* *(2026-10-26, the packages installed in all three: in Mochi and
  Mojo the same twelve pads, one finger drag from Palefill and Pad 01 to Pad 12 at the card's bottom. Still
  `[~]`: the search results list not tried, and an add-on's details longer than the card do not pan (their
  length is not reported; T-527's review #13).)* *(2026-10-26, later: an add-on's details now report their length
  (what overflows them at the card's height, kept while the same view is shown): Palefill's preferences, 1233 rows
  at the 980 layout width, pan to their Disable / Remove row by finger in Enyo. Left: the search results list.)*
  *(2026-10-26, last: the search results too — in Enyo with the twelve pads, "Jihad List Pad" searched (typed into the
  manager's search box, its magnifier tapped), My Add-ons: twelve results, two drags reach Pad 12. Their rows read
  "Remove" where the owner asked for "Uninstall" on every add-on (2026-10-08): fixed (the search list relabelled
  as its results arrive; `search-list` added to JihadEngine.jsm's surface table). Ticked.)* *(suite: `addons-list-pan`; `docs/FINGER-TESTS.md`)*
