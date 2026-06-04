#!/usr/bin/env python3
"""FPS benchmark harness for endoscopic_pipeline.

Usage:
    python3 benchmark/benchmark.py [--binary PATH] [--width W] [--height H]
                                   [--threshold FPS] [--frames N]
"""
from __future__ import annotations

import argparse
import json
import statistics
import subprocess
import sys
from pathlib import Path


def run_benchmark(
    binary: str,
    width: int,
    height: int,
    frames: int,
    threshold: float,
) -> dict:
    cmd = [
        binary,
        "--synthetic",
        "--width", str(width),
        "--height", str(height),
        "--frames", str(frames),
        "--benchmark",
        "--json-status",
    ]

    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    frame_ms: list[float] = []
    total_frames = 0
    mean_fps = 0.0
    dropped = 0

    for line in proc.stdout:  # type: ignore[union-attr]
        line = line.strip()
        if not line:
            continue
        try:
            ev = json.loads(line)
        except json.JSONDecodeError:
            continue

        if ev.get("status") == "frame":
            frame_ms.append(ev["ms"])
        elif ev.get("status") == "done":
            total_frames = ev.get("total_frames", 0)
            mean_fps     = ev.get("mean_fps", 0.0)
            dropped      = ev.get("dropped", 0)

    proc.wait()

    p95_ms = statistics.quantiles(frame_ms, n=100)[94] if frame_ms else 0.0
    drop_ratio = dropped / max(total_frames, 1)

    result = {
        "width": width,
        "height": height,
        "total_frames": total_frames,
        "mean_fps": round(mean_fps, 1),
        "p95_ms": round(p95_ms, 2),
        "dropped": dropped,
        "drop_ratio": round(drop_ratio, 4),
        "pass": mean_fps >= threshold,
        "threshold_fps": threshold,
    }
    return result


def main() -> int:
    ap = argparse.ArgumentParser(description="endoscopic_pipeline FPS benchmark")
    ap.add_argument("--binary",    default="./endoscopic_pipeline")
    ap.add_argument("--width",     type=int,   default=1280)
    ap.add_argument("--height",    type=int,   default=720)
    ap.add_argument("--threshold", type=float, default=25.0)
    ap.add_argument("--frames",    type=int,   default=60)
    args = ap.parse_args()

    result = run_benchmark(
        args.binary, args.width, args.height, args.frames, args.threshold
    )
    print(json.dumps(result, indent=2))

    if result["pass"]:
        print(f"\nPASS  {result['mean_fps']} FPS ≥ {args.threshold} FPS", flush=True)
        return 0
    else:
        print(f"\nFAIL  {result['mean_fps']} FPS < {args.threshold} FPS", flush=True)
        return 1


if __name__ == "__main__":
    sys.exit(main())
