# Adversarial review, 2026-11-19 (Codex)

The owner, 2026-11-19: "you can use codex for review instead of fable when its available" (Fable's limit was reached).
Codex CLI 0.169.0, read-only sandbox, on the site-data/logins work, the system clipboard, the background-card freeze, the
memory guard's thread with engine patch 0124, and engine patches 0120-0123: `PROMPT.md` (the brief: the code and the
contracts, not my conclusions), `codex-review.md` (its report, 13 findings). Each finding was checked against the code
before anything changed.

| # | finding | checked | done |
|---|---|---|---|
| 1 | clipboard temp file: a predictable name in sticky /var/tmp, fopen "w" follows a link | holds (root daemon) | `mkstemp` + `fchmod` + rename (EngineHost.cpp) |
| 2 | ANGLE: nesting bounded, expansion size not: `#define D(x) x x` nested 26 deep = 2^26 tokens | holds | a 100 000-token budget per shader shared by nested expanders, counted before vectors grow (engine 0125); `webgl-macro` row m |
| 3 | a password field the page turned to text goes into form history | holds | fields once of type password, credential autocomplete tokens and password-like names are kept out (jihadLogins.js); `site-data-shown-password` submits the form with the field turned to text (no login question; the form history keeps the note and the username, not the password) |
| 4 | one origin's local storage Clear purged the whole host (ports, schemes, subdomains, service workers) | holds | that principal's storage cleared (`getLocalStorageForPrincipal(...).clear()`); the domain purge stays for Forget |
| 5 | the guard's detached thread may call into a destroyed JSContext at shutdown | holds (narrow) | the thread is joined in Shutdown before the engine goes; ~XPCJSContext clears the pointer (0125) |
| 6 | the stop and the shed not tied to one document; an idle stop window could stop an innocent script | holds | only a script running 2 s or more in its task is stopped, its outer window told to the daemon ("jihad-memory-script-stopped"), that page shed first (0125, BrowserPageGoanna.cpp) |
| 7 | trace mode's stack dump at named-button prompts includes the login question (arguments, locals); usernames logged | holds | the dump only for the slow-script dialog ("Stop script"); no usernames in the log |
| 8 | the login question (and every dialog) goes to the last card made | holds: one prompter, the last page its sink | GetPrompt routes by the window's root docshell to its page; the login question asks with the submitting window |
| 9 | Forget keeps a locked store's logins and says "forgotten" | holds | it says the logins were kept (master password not entered) |
| 10 | Forget leaves the form history | by design | form history has no site in the engine (Firefox's Forget leaves it too); the kit's evidence says so |
| 11 | an update can overwrite another action origin's login of the same username | holds | an update takes the login of this form's action or one with none |
| 12 | IPv6 origins parsed wrongly in about:data | holds | hosts through the URI parser |
| 13 | the test master password left in prefs.js and logged when malformed | holds | read once, cleared, never logged |

What Codex found sound: the about: pages are not reachable from content, the chrome package is not content-accessible,
values are shown as text nodes; filling uses the frame's own origin and both switches; the clipboard's write/close checks;
0120's alias; 0121's hook with its null context; 0123's fallback; the freeze counters and the disconnected adapter path;
chrome never stopped by the guard; an expired stop window never fires later.

## Verified on the device (2026-11-19, engine 0125, `verify/finger-report.md`)

27 runs in the three shells: `memory-guard-task`, `site-logins-save`, `-refuse-update`, `-frame` (the login question in the
submitting card), `-persist`, `-master` pass in all three; `site-logins-page` in Mochi and Mojo (Enyo lost one tap, as
before: rerun below); `webgl-macro` in all three, its row m (`#define D(x) x x` 26 deep) refused by the budget as "out
of memory" in 82 ms, rows a-j and l as before (`verify/kept/*-webgl-macro-rows_.jpg`). `site-data-page` failed in the
three on this round's own test change (the password turned to text before the submit leaves no password field, so no
login question and no saved login, and the script's later steps need that login): the change was reverted and the
shown password got its own script, `site-data-shown-password`.

Rerun (`--merge`, marked "(rerun)" in the report): `site-data-page` and `site-data-shown-password` pass in the three
shells (the shown password: no dialog, and the form history keeps none of it), and Enyo's `site-logins-page`. The round
is **30 runs, all passing**.
