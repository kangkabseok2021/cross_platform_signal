"""Pytest tests for the Python signal generator."""
import json
import subprocess
import sys
from pathlib import Path
import numpy as np
import pytest


def run_generator(tmp_path, **kwargs):
    out_csv  = tmp_path / "signal.csv"
    out_json = tmp_path / "ref.json"
    args = [sys.executable, "tools/generate_signal.py",
            "--out", str(out_csv), "--ref", str(out_json)]
    for k, v in kwargs.items():
        args += [f"--{k}", str(v)]
    subprocess.run(args, check=True, cwd=Path(__file__).parent.parent)
    return out_csv, out_json


def test_csv_has_correct_line_count(tmp_path):
    csv, _ = run_generator(tmp_path, n=256)
    lines = [l for l in csv.read_text().splitlines() if l.strip()]
    assert len(lines) == 256


def test_reference_json_has_required_keys(tmp_path):
    _, ref = run_generator(tmp_path, n=512, freq=100)
    data = json.loads(ref.read_text())
    assert {"fundamental_hz", "fundamental_magnitude",
            "bin_resolution_hz", "n_samples"} <= data.keys()


def test_fundamental_frequency_close_to_input(tmp_path):
    freq = 75.0
    _, ref = run_generator(tmp_path, n=1024, freq=freq, snr=40)
    data   = json.loads(ref.read_text())
    # Allow ±2 bins tolerance
    tol = data["bin_resolution_hz"] * 2
    assert abs(data["fundamental_hz"] - freq) <= tol


def test_csv_values_are_floats(tmp_path):
    csv, _ = run_generator(tmp_path, n=64)
    for line in csv.read_text().splitlines():
        float(line)   # raises if not parseable


def test_different_seeds_produce_different_signals(tmp_path):
    csv1, _ = run_generator(tmp_path / "a", n=64, seed=0)
    csv2, _ = run_generator(tmp_path / "b", n=64, seed=1)
    v1 = [float(l) for l in csv1.read_text().splitlines()]
    v2 = [float(l) for l in csv2.read_text().splitlines()]
    assert v1 != v2
