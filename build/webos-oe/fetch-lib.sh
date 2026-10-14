# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# fetch_url, shared by the scripts that download a pinned input (sourced; no side effects).
#
# archive.debian.org sits behind a CDN that has been seen serving a cached 404 for a file the
# archive has (2026-10-14: 32 of the 187 sysroot packages), and a cached 200 with the wrong bytes
# gets the same treatment: up to five more tries with a fresh cache-buster and a short back-off.
# curl's own error stays visible. The sha256, when given, decides what is accepted.
fetch_url() { # <url> <file> [sha256]: succeeds only with the file downloaded and, if given, matching
  local i url got
  for i in 0 1 2 3 4 5; do
    if [ "$i" = 0 ]; then url="$1"; else sleep "$i"; url="$1?nocache=$RANDOM$RANDOM"; fi
    rm -f "$2"
    if curl -fsSL --retry 3 -m 600 -o "$2" "$url"; then
      [ -n "${3:-}" ] || return 0
      got="$(sha256sum "$2" | cut -d' ' -f1)"
      [ "$got" = "$3" ] && return 0
      echo "   sha256 $got is not the pinned $3; trying again past the cache" >&2
    fi
  done
  rm -f "$2"; return 1
}
