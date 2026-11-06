#!/usr/bin/env python3
"""Render a stage screen from the CONVERTED DATA, to check a captured frame
offline (no emulator).

For a camera position it composes what the game is supposed to show:
  * the parallax from res/stage<N>_bg<M>.png (512 px wide, scrolled by
    -cam*speed/10, rows past the art left as the backdrop colour), and
  * the map window from res/stage<N>_map<M>.png at (cam, 0).
It then finds the camera that best matches a captured frame and reports where
the frame differs from the data - i.e. where the engine drew something the data
does not contain (sprites/HUD are legitimately absent from the data render).

Usage:
  tools/preview_screen.py --stage 0 --map 1 --cam 2880 --out /tmp/opencode/expected.png
  tools/preview_screen.py --stage 0 --map 1 --match capture.png
"""
import argparse
import os
import sys

from PIL import Image
import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RES = os.path.join(ROOT, "res")


def load_art(stage, map_n):
    mp = Image.open(os.path.join(RES, f"stage{stage}_map{map_n}.png")).convert("RGB")
    bg = Image.open(os.path.join(RES, f"stage{stage}_bg{map_n}.png")).convert("RGB")
    return np.asarray(mp, dtype=np.uint8), np.asarray(bg, dtype=np.uint8)


def render(map_img, bg_img, cam, bg_speed, adjust_y, backdrop=(0, 0, 0)):
    h, w = 224, 320
    out = np.zeros((h, w, 3), dtype=np.uint8)
    out[:, :] = backdrop
    if bg_speed:
        off = int(-(cam * bg_speed) / 10) % 512
        bh, bw = bg_img.shape[:2]
        y0 = adjust_y
        for x in range(w):
            sx = (off + x) % 512
            if sx >= bw:
                continue
            ys = max(0, y0)
            ye = min(h, y0 + bh)
            if ye > ys:
                out[ys:ye, x] = bg_img[max(0, -y0):bh, sx]
    cam = max(0, min(cam, map_img.shape[1] - w))
    out[:, :] = np.where(out.any(axis=2, keepdims=True),
                         out, map_img[0:h, cam:cam + w])
    # map art wins where it has content; the data PNG already has the backdrop
    # composited, so just paste it
    out = map_img[0:h, cam:cam + w].copy()
    return out, cam


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--stage", type=int, default=0)
    ap.add_argument("--map", type=int, default=1)
    ap.add_argument("--cam", type=int)
    ap.add_argument("--out")
    ap.add_argument("--match", help="captured frame to align against")
    args = ap.parse_args()

    map_img, bg_img = load_art(args.stage, args.map)
    if args.match:
        shot = np.asarray(Image.open(args.match).convert("RGB"), dtype=np.uint8)
        if shot.shape[1] != 320:                      # window capture (2x)
            shot = shot[::shot.shape[0] // 224, ::shot.shape[1] // 320]
        shot = shot[:224, :320]
        best, best_cam = None, 0
        for cam in range(0, map_img.shape[1] - 320 + 1, 4):
            exp = map_img[:224, cam:cam + 320]
            # ignore the HUD corner and compare the rest
            d = np.abs(exp[60:, 40:].astype(int) - shot[60:, 40:].astype(int)).mean()
            if best is None or d < best:
                best, best_cam = d, cam
        print(f"best camera {best_cam} (mean abs diff {best:.1f})")
        exp = map_img[:224, best_cam:best_cam + 320]
        diff = np.abs(exp.astype(int) - shot.astype(int)).sum(axis=2)
        mask = diff > 90                              # clearly different pixel
        rows = mask.sum(axis=1)
        cols = mask.sum(axis=0)
        print(f"differing pixels: {int(mask.sum())} of {224*320}")
        worst_rows = sorted(range(224), key=lambda y: -rows[y])[:5]
        worst_cols = sorted(range(320), key=lambda x: -cols[x])[:5]
        print("worst rows (y: pixels):", [(y, int(rows[y])) for y in worst_rows])
        print("worst cols (x: pixels):", [(x, int(cols[x])) for x in worst_cols])
        if args.out:
            Image.fromarray(shot).save(args.out.replace(".png", "_shot.png"))
            Image.fromarray(exp).save(args.out.replace(".png", "_data.png"))
            heat = np.zeros_like(shot)
            heat[mask] = (255, 0, 0)
            Image.fromarray(heat).save(args.out.replace(".png", "_diff.png"))
            print("wrote *_shot/_data/_diff.png")
        return 0
    if args.cam is None or not args.out:
        sys.exit("need --cam and --out (or --match)")
    exp, cam = render(map_img, bg_img, args.cam, 0, 0)
    Image.fromarray(exp).save(args.out)
    print(f"wrote {args.out} (camera {cam})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
