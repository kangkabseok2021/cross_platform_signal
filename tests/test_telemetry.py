import pytest
import requests
import subprocess
import time

@pytest.fixture(scope="module")
def mock_server():
    server_process = subprocess.Popen(["python3", "tools/mock_server.py"])
    
    # Wait for health check
    for _ in range(50):
        try:
            res = requests.get("http://localhost:8080/health")
            if res.status_code == 200:
                break
        except requests.exceptions.ConnectionError:
            pass
        time.sleep(0.1)
    else:
        server_process.kill()
        pytest.fail("Mock server failed to start")
        
    yield
    server_process.kill()

def test_get_health_returns_ok(mock_server):
    res = requests.get("http://localhost:8080/health")
    assert res.status_code == 200
    assert res.json() == {"status": "ok"}

def test_post_discharge_event_returns_201(mock_server):
    payload = {
        "timestamp": "2026-05-28T12:00:00Z",
        "voltage_kv": 3.0,
        "event": "DISCHARGE_THRESHOLD_EXCEEDED"
    }
    res = requests.post("http://localhost:8080/", json=payload)
    assert res.status_code == 201

def test_payload_schema_validated(mock_server):
    payload = {
        "timestamp": "2026-05-28T12:00:00Z",
        "voltage_kv": 3.0,
        "event": "DISCHARGE_THRESHOLD_EXCEEDED"
    }
    res = requests.post("http://localhost:8080/", json=payload)
    assert "id" in res.json()

def test_events_log_grows(mock_server):
    res_before = requests.get("http://localhost:8080/events")
    count_before = len(res_before.json())
    
    for i in range(3):
        payload = {
            "timestamp": f"2026-05-28T12:00:0{i}Z",
            "voltage_kv": 3.0,
            "event": "DISCHARGE_THRESHOLD_EXCEEDED"
        }
        requests.post("http://localhost:8080/", json=payload)
        
    res_after = requests.get("http://localhost:8080/events")
    assert len(res_after.json()) == count_before + 3

def test_missing_fields_handled(mock_server):
    # Missing 'event'
    payload = {
        "timestamp": "2026-05-28T12:00:00Z",
        "voltage_kv": 3.0
    }
    res = requests.post("http://localhost:8080/", json=payload)
    assert res.status_code == 400

def test_large_voltage_marked_critical(mock_server):
    payload = {
        "timestamp": "2026-05-28T12:00:00Z",
        "voltage_kv": -8.0,
        "event": "DISCHARGE_THRESHOLD_EXCEEDED",
        "severity": "CRITICAL"
    }
    requests.post("http://localhost:8080/", json=payload)
    
    events = requests.get("http://localhost:8080/events").json()
    last_event = events[-1]
    assert last_event["severity"] == "CRITICAL"
