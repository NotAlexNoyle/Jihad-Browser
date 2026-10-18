
import sys
from PIL import Image
p, w, h, out = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4]
sw, sh = (w + 3) // 4, (h + 3) // 4
d = open(p, "rb").read(); n = len(d) // (sw * sh * 4)
print("seq: %d frames of %dx%d" % (n, sw, sh))
if n == 0: sys.exit(0)
cols = min(n, 6); rows = (n + cols - 1) // cols
sheet = Image.new("RGB", (cols * (sw + 4), rows * (sh + 4)), (128, 128, 128))
for i in range(n):
    im = Image.frombytes("RGBA", (sw, sh), d[i * sw * sh * 4:(i + 1) * sw * sh * 4], "raw", "BGRA").convert("RGB")
    px = im.load()
    rr = [y for y in range(sh) if any(px[x, y] != (255, 255, 255) and max(px[x, y]) < 241 or min(px[x, y]) < 241 for x in range(0, sw, 2))]
    print("  frame %2d: content rows %s..%s (x4)  px(100,100)=%s px(400,500)=%s px(700,900)=%s" % (
        i + 1, rr[0] if rr else "-", rr[-1] if rr else "-", px[25, 25], px[100, 125], px[175, 225]))
    sheet.paste(im, ((i % cols) * (sw + 4) + 2, (i // cols) * (sh + 4) + 2))
sheet.save(out, quality=80)
