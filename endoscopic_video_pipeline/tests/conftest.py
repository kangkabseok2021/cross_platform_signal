"""Pytest fixtures for endoscopic pipeline tests."""
from __future__ import annotations

import os
import subprocess
from pathlib import Path

import pytest


def _find_binary(name: str = "endoscopic_pipeline") -> str:
    """Locate the compiled binary relative to the project tree."""
    candidates = [
        Path(__file__).parents[2] / "build-endo" / name,
        Path(__file__).parents[1] / "build" / name,
        Path(name),
    ]
    env_bin = os.environ.get("ENDOSCOPIC_BIN")
    if env_bin:
        candidates.insert(0, Path(env_bin))
    for p in candidates:
        if p.exists():
            return str(p)
    pytest.skip(f"Binary '{name}' not found; set ENDOSCOPIC_BIN env var")


@pytest.fixture(scope="session")
def pipeline_binary() -> str:
    return _find_binary()


@pytest.fixture(scope="session")
def benchmark_script() -> Path:
    return Path(__file__).parents[1] / "benchmark" / "benchmark.py"
