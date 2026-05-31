#!/usr/bin/env python3
"""NumPy oracle: verify the C99 acoustic.c output matches the ISO 9613-1 formula.

Usage:
    ./demo_safe > demo_safe_output.txt
    python3 scripts/validate_c.py demo_safe_output.txt

Exits 0 if max absolute error < 0.001 dB, else 1.
"""
import re
import sys
import numpy as np


def parse_demo_output(path: str) -> tuple[np.ndarray, float, float, float]:
    with open(path) as f:
        lines = f.read().strip().splitlines()
    header = lines[0]
    m = re.search(r"Lw=([\d.]+).*g_air_absorption=([\d.]+)", header)
    Lw, alpha = float(m.group(1)), float(m.group(2))
    rows = [[float(x) for x in l.split()] for l in lines[1:] if l.strip()]
    return np.array(rows, dtype=np.float32), Lw, alpha


def expected_spl(Lw: float, coords: np.ndarray, alpha: float) -> np.ndarray:
    xs, ys = np.meshgrid(coords, coords)
    r = np.sqrt(xs**2 + ys**2)
    return Lw - 20.0 * np.log10(r) - 11.0 - alpha * r


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print("Usage: validate_c.py <demo_safe_output.txt>")
        return 1

    c_grid, Lw, alpha = parse_demo_output(argv[1])
    coords = np.array([1.0, 5.0, 10.0, 20.0, 50.0], dtype=np.float32)
    ref    = expected_spl(Lw, coords, alpha).astype(np.float32)
    err    = np.max(np.abs(c_grid - ref))

    print(f"Max absolute error vs NumPy oracle: {err:.6f} dB")
    if err < 0.001:
        print("PASS — C legacy output matches ISO 9613-1 formula")
        return 0
    print(f"FAIL — error {err:.6f} dB exceeds 0.001 dB threshold")
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
