"""Smoke tests for the endoscopic video pipeline (CPU / --stub-gpu path)."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

import pytest


# ── helper ────────────────────────────────────────────────────────────────────

def run(binary: str, *extra_args: str, timeout: int = 60) -> subprocess.CompletedProcess:
    return subprocess.run(
        [binary, "--synthetic", "--frames", "10",
         "--width", "320", "--height", "240", "--json-status",
         *extra_args],
        capture_output=True, text=True, timeout=timeout,
    )


# ── tests ─────────────────────────────────────────────────────────────────────

def test_output_file_created(pipeline_binary, tmp_path):
    """Pipeline creates a non-empty output MP4 from synthetic input."""
    out = tmp_path / "test_out.mp4"
    result = run(pipeline_binary, "--output", str(out))
    assert result.returncode == 0, result.stderr
    assert out.exists(), "output.mp4 was not created"
    assert out.stat().st_size > 0, "output.mp4 is empty"


def test_fps_above_threshold(pipeline_binary):
    """CPU Sobel on 320×240 synthetic frames achieves > 10 FPS."""
    result = run(pipeline_binary, "--benchmark")
    assert result.returncode == 0, result.stderr

    lines = [l for l in result.stdout.splitlines() if l.strip()]
    done_events = [
        json.loads(l) for l in lines
        if l.startswith("{") and json.loads(l).get("status") == "done"
    ]
    assert done_events, "No 'done' JSON event emitted"
    fps = done_events[-1]["mean_fps"]
    assert fps > 10.0, f"FPS {fps:.1f} below 10 FPS threshold"


def test_sobel_response_nonzero(pipeline_binary, tmp_path):
    """Sobel output has non-zero pixel values (edges detected)."""
    try:
        import cv2  # type: ignore[import-untyped]
        import numpy as np
    except ImportError:
        pytest.skip("opencv-python not installed")

    out = tmp_path / "sobel_out.mp4"
    result = run(pipeline_binary, "--output", str(out), "--frames", "5")
    assert result.returncode == 0, result.stderr

    cap = cv2.VideoCapture(str(out))
    ret, frame = cap.read()
    cap.release()
    assert ret, "Could not read first frame from output video"
    assert frame.mean() > 0, "All pixels are zero — Sobel produced no response"


def test_invalid_input_exits_nonzero(pipeline_binary):
    """Pipeline returns non-zero exit code for a non-existent input file."""
    result = subprocess.run(
        [pipeline_binary, "--input", "/nonexistent/path/video.mp4",
         "--output", "/dev/null", "--frames", "1"],
        capture_output=True, text=True, timeout=15,
    )
    assert result.returncode != 0, "Expected non-zero exit for missing input"


def test_version_flag(pipeline_binary):
    """--version prints a version string and exits 0."""
    result = subprocess.run(
        [pipeline_binary, "--version"],
        capture_output=True, text=True, timeout=5,
    )
    assert result.returncode == 0
    assert "1.0.0" in result.stdout
