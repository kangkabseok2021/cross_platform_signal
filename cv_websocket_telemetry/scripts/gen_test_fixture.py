#!/usr/bin/env python3
"""Generate tests/fixtures/test_input.mp4 — 5-sec spinning checkerboard at 30 fps."""
import cv2
import numpy as np
import os

ROOT    = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUTPUT  = os.path.join(ROOT, "tests", "fixtures", "test_input.mp4")
W, H, FPS, SECS = 640, 480, 30, 5

os.makedirs(os.path.dirname(OUTPUT), exist_ok=True)
writer = cv2.VideoWriter(OUTPUT, cv2.VideoWriter_fourcc(*"mp4v"), FPS, (W, H))

checker = np.zeros((H, W), dtype=np.uint8)
block = 40
for y in range(0, H, block):
    for x in range(0, W, block):
        if (x // block + y // block) % 2 == 0:
            checker[y:y + block, x:x + block] = 255

cx, cy = W // 2, H // 2
for i in range(FPS * SECS):
    M       = cv2.getRotationMatrix2D((cx, cy), i * 2.0, 1.0)
    rotated = cv2.warpAffine(checker, M, (W, H))
    frame   = cv2.cvtColor(rotated, cv2.COLOR_GRAY2BGR)
    writer.write(frame)

writer.release()
print(f"Written {FPS * SECS} frames → {OUTPUT}")
