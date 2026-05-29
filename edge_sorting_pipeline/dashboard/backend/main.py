import os
from collections import deque
from fastapi import FastAPI, HTTPException
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel

app = FastAPI(title="Optical Sorting Telemetry Backend")

class TelemetryPayload(BaseModel):
    material: str
    confidence: float
    sorted_count: int

class AppState:
    def __init__(self):
        self.readings = deque(maxlen=100)
        self.sorted_count = 0
        self.last_material = "None"

    def reset(self):
        self.readings.clear()
        self.sorted_count = 0
        self.last_material = "None"

state = AppState()

@app.get("/health")
def health_check():
    return {"status": "ok"}

@app.post("/telemetry")
def post_telemetry(payload: TelemetryPayload):
    state.sorted_count = payload.sorted_count
    state.last_material = payload.material
    state.readings.append(payload.confidence)
    return {"ok": True}

@app.get("/metrics")
def get_metrics():
    avg_conf = 0.0
    if state.readings:
        avg_conf = sum(state.readings) / len(state.readings)
    return {
        "sorted_count": state.sorted_count,
        "avg_confidence": avg_conf,
        "last_material": state.last_material
    }

# Mount static frontend files
current_dir = os.path.dirname(os.path.abspath(__file__))
frontend_dir = os.path.join(current_dir, "..", "frontend")
if os.path.exists(frontend_dir):
    app.mount("/", StaticFiles(directory=frontend_dir, html=True), name="static")
