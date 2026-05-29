# High-Speed Edge Inference Pipeline for Optical Sorting

A containerized C++20 edge processing service simulating a high-speed optical sorting machine. The system captures camera frame telemetry, runs real-time object detection inference, performs Non-Maximum Suppression (NMS) to filter redundant bounding boxes, queues sorting events through a lock-free Single Producer Single Consumer (SPSC) queue, and controls a pneumatic sorting ejector. Telemetry is streamed over raw POSIX TCP sockets to a Python FastAPI backend served on a strict vanilla TypeScript metrics dashboard.

## Tech Stack

| Layer | Technology |
|---|---|
| Edge Application | C++20, standard threads (`std::jthread` / `std::stop_token`) |
| Lock-Free Queue | SPSC Ring Buffer with `std::atomic` acquire/release indices |
| Inference Engine | TensorFlow Lite (EfficientDet-Lite0 quantized) or Mock/Simulation |
| Telemetry | Raw HTTP/1.1 over POSIX TCP Sockets |
| API Backend | Python FastAPI + Pydantic v2 |
| Dashboard UI | Vanilla HTML5 / CSS3 / strict TypeScript 5 (no framework) |
| Containerization | Multi-stage Dockerfile + Docker Compose |
| Test Suites | GoogleTest (27 C++ tests) + pytest (5 Python tests) + strict typechecking |

## Quickstart

### Run with Docker Compose

To spin up the telemetry server and C++ simulator:

```bash
cd edge_sorting_pipeline
docker compose build
docker compose up -d
```

- Telemetry dashboard: `http://localhost:8002` (FastAPI backend port `8000` mapped to `8002` on host)
- Metrics JSON: `http://localhost:8002/metrics`
- Health check: `http://localhost:8002/health`

### Run Local Development & Tests

#### C++ GoogleTest Suite
We compile without TFLite by default for rapid local verification.

```bash
# Configure and build
cmake -B build -S . -DBUILD_SORTING=ON -DBUILD_WITH_TFLITE=OFF
cmake --build build -j4

# Run GTest suite (27 tests)
ctest --test-dir build --output-on-failure
```

#### Python Pytest Backend Suite
```bash
# Sync monorepo python dependencies
uv sync

# Run backend integration tests (5 tests)
PYTHONPATH=edge_sorting_pipeline uv run pytest edge_sorting_pipeline/dashboard/backend/tests -v
```

#### TypeScript Frontend Typechecking
```bash
npx -y -p typescript tsc --project edge_sorting_pipeline/dashboard/frontend/tsconfig.json --noEmit
```

## Statistical NMS & SPSC Queue

- **NMS:** Sorts predicted bounding boxes by confidence descending, computing the Intersection over Union (IoU) between boxes:
  $$\text{IoU} = \frac{|A \cap B|}{|A \cup B|}$$
  Detections of the same class ID are suppressed if the overlap IoU exceeds the configurable threshold `0.5`.
- **Sorting Queue:** Ring buffer of size `64` using `std::atomic` memory acquire/release barriers. Prevents threads from blocking during sorting operations.
