"""
REST integration tests for vehicle_telemetry_headless.

Spawns the headless binary on port 18080, waits for the health endpoint,
then exercises the JSON API. Set TELEMETRY_BINARY to override binary path.
"""
import os
import pathlib
import subprocess
import time

import pytest
import requests

PORT = 18080
BASE = f"http://localhost:{PORT}"

_candidates = [
    pathlib.Path(__file__).parents[1] / "build" / "vehicle_telemetry_headless",
    pathlib.Path(__file__).parents[2] / "build" / "vehicle_telemetry_qt" / "vehicle_telemetry_headless",
    pathlib.Path(__file__).parents[1] / "build" / "vehicle_telemetry_qt" / "vehicle_telemetry_headless",
]
BINARY = os.environ.get("TELEMETRY_BINARY") or str(
    next((p for p in _candidates if p.exists()), _candidates[0])
)


@pytest.fixture(scope="session")
def server_process():
    proc = subprocess.Popen(
        [BINARY, "--port", str(PORT)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    deadline = time.time() + 5.0
    started = False
    while time.time() < deadline:
        try:
            r = requests.get(f"{BASE}/api/v1/health", timeout=0.3)
            if r.status_code == 200:
                started = True
                break
        except requests.exceptions.ConnectionError:
            pass
        time.sleep(0.1)

    if not started:
        proc.kill()
        pytest.fail(f"Server did not start within 5 s — binary: {BINARY}")

    yield proc
    proc.terminate()
    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()


def test_health_ok(server_process):
    r = requests.get(f"{BASE}/api/v1/health")
    assert r.status_code == 200
    data = r.json()
    assert data["status"] == "ok"
    assert "uptime_s" in data
    assert isinstance(data["uptime_s"], int)


def test_telemetry_has_four_channels(server_process):
    r = requests.get(f"{BASE}/api/v1/telemetry")
    assert r.status_code == 200
    data = r.json()
    assert "channels" in data
    assert len(data["channels"]) == 4
    for ch in data["channels"]:
        assert "name" in ch
        assert "raw" in ch
        assert "filtered" in ch
        assert "alpha" in ch
        assert "fault" in ch


def test_filtered_differs_from_raw(server_process):
    time.sleep(1.2)  # let EMA warm up for ~12 ticks
    r = requests.get(f"{BASE}/api/v1/telemetry")
    data = r.json()
    diffs = [abs(ch["raw"] - ch["filtered"]) for ch in data["channels"]]
    assert max(diffs) > 0.001, "EMA filter never ran — raw and filtered are identical"


def test_set_alpha(server_process):
    r = requests.post(f"{BASE}/api/v1/channels/0/alpha", json={"alpha": 0.9})
    assert r.status_code == 200
    r2 = requests.get(f"{BASE}/api/v1/channels/0")
    assert r2.status_code == 200
    assert abs(r2.json()["alpha"] - 0.9) < 0.001


def test_cors_header(server_process):
    r = requests.get(f"{BASE}/api/v1/health")
    assert "Access-Control-Allow-Origin" in r.headers
    assert r.headers["Access-Control-Allow-Origin"] == "*"
