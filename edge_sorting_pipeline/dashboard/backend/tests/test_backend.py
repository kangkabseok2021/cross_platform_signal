import pytest
from fastapi.testclient import TestClient

def test_health(client: TestClient):
    response = client.get("/health")
    assert response.status_code == 200
    assert response.json() == {"status": "ok"}

def test_post_telemetry(client: TestClient):
    payload = {"material": "PET_Plastic", "confidence": 0.85, "sorted_count": 10}
    response = client.post("/telemetry", json=payload)
    assert response.status_code == 200
    assert response.json() == {"ok": True}

def test_get_metrics_averages(client: TestClient):
    # Post first telemetry
    client.post("/telemetry", json={"material": "PET_Plastic", "confidence": 0.80, "sorted_count": 5})
    # Post second telemetry
    client.post("/telemetry", json={"material": "Glass_Bottle", "confidence": 0.90, "sorted_count": 6})

    response = client.get("/metrics")
    assert response.status_code == 200
    data = response.json()
    assert data["sorted_count"] == 6
    assert data["last_material"] == "Glass_Bottle"
    assert abs(data["avg_confidence"] - 0.85) < 1e-5

def test_empty_deque_avg_is_zero(client: TestClient):
    response = client.get("/metrics")
    assert response.status_code == 200
    data = response.json()
    assert data["sorted_count"] == 0
    assert data["avg_confidence"] == 0.0
    assert data["last_material"] == "None"

def test_validation_error(client: TestClient):
    payload = {"material": "PET_Plastic", "confidence": 0.85}
    response = client.post("/telemetry", json=payload)
    assert response.status_code == 422
