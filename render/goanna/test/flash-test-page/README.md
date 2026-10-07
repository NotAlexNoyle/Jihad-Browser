# Flash test page

The page the owner bookmarked on the test device (2026-10-07): `index.html` plus three test movies and five
real games on archive.org. On the device it lives at `/media/internal/jihad-flash-test/` (survives reboots and
reinstalls). The SWFs are generated, not stored:

```sh
python3 render/goanna/test/make-anim-swf.py -o anim.swf
python3 render/goanna/test/make-anim-swf.py -o grid.swf --frames 768 --fps 30 --square 10 --grid 32
python3 render/goanna/test/make-key-swf.py -o key.swf
```

`grid.swf` measures the CONTENT frame rate: the square steps one cell per frame on a 32x24 grid, so the cell in
two timed screenshots gives frames advanced per second.
