# High-Speed Edge Inference Pipeline for Optical Sorting

A real-time, low-latency C++20 edge processing service simulating an optical sorting machine. The pipeline processes simulated high-speed camera frames, performs object detection, runs Non-Maximum Suppression (NMS) to eliminate duplicate candidate bounding boxes, queues valid target coordinates via a lock-free Single Producer Single Consumer (SPSC) queue to a pneumatic actuator loop, and streams real-time telemetry over raw TCP sockets using manual HTTP/1.1 POST payloads to a Python FastAPI and TypeScript dashboard.

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│  Camera Simulator                                               │
│  Simulates high-speed frame capture at 30 fps (33.3ms interval) │
└───────────────────────┬─────────────────────────────────────────┘
                        │ raw frame data
┌───────────────────────▼─────────────────────────────────────────┐
│  InferenceEngine (Mock / TensorFlow Lite)                       │
│  Runs object detection to predict bounding boxes + class IDs   │
└───────────────────────┬─────────────────────────────────────────┘
                        │ candidate bounding boxes
┌───────────────────────▼─────────────────────────────────────────┐
│  Non-Maximum Suppression (NMS)                                  │
│  Filters overlapping predictions (IoU > 0.5 threshold)          │
└───────────────────────┬─────────────────────────────────────────┘
                        │ filtered target event
┌───────────────────────▼─────────────────────────────────────────┐
│  SortingQueue (Lock-Free SPSC Ring Buffer)                       │
│  Thread-safe transfer using std::atomic memory ordering         │
└───────────────────────┬─────────────────────────────────────────┘
                        │
      ┌─────────────────┴─────────────────┐
      │                                   │
┌─────▼─────────────────────────┐   ┌─────▼─────────────────────────┐
│  MachineController            │   │  TelemetrySender              │
│  std::jthread ejector loop    │   │  Raw POSIX TCP Socket POST    │
│  Cooperative stop token       │   │  HTTP/1.1 formatted payloads  │
└───────────────────────────────┘   └─────────────┬─────────────────┘
                                                  │
                                                  │ Telemetry JSON
                                    ┌─────────────▼─────────────────┐
                                    │  FastAPI Telemetry Backend    │
                                    │  Sliding queue, aggregates   │
                                    │  real-time sorting stats      │
                                    └─────────────┬─────────────────┘
                                                  │
                                                  │ GET /metrics
                                    ┌─────────────▼─────────────────┐
                                    │  TypeScript Web Dashboard     │
                                    │  Visualizes metrics, sorting  │
                                    │  throughput, and efficiency   │
                                    └───────────────────────────────┘
```

---

## Design Patterns & Safety

**Lock-Free SPSC Ring Buffer** — Thread boundary handoffs between the inference pipeline thread and the machine controller actuator thread utilize a strict Single Producer Single Consumer template queue. Head and tail indices are declared as `std::atomic<size_t>` with explicit `std::memory_order_acquire` and `std::memory_order_release` ordering constraints to ensure cache coherency without lock contention on x86/ARM architectures.

**RAII & Modern Threads** — The actuator and inference loops run inside standard `std::jthread` executors. Cooperative cancellation is managed via `std::stop_token` to guarantee clean shutdown sequences. The telemetry socket connection implements an RAII pattern, closing the socket descriptor automatically upon destruction.

**Zero-Dependency Socket Telemetry** — To minimize dependencies and avoid heavy HTTP client library weights on edge devices, telemetry data is formatted into compliant raw HTTP/1.1 POST chunks and written directly to a raw POSIX TCP socket.

---

## Observability API

The dashboard backend is implemented using FastAPI and exposes the following endpoints:

| Endpoint | Method | Status | Description |
|---|---|---|---|
| `/health` | `GET` | 200 | Health status check, returns `{"status":"healthy"}`. |
| `/ready` | `GET` | 200 | Readiness check, returns `{"status":"ready"}`. |
| `/telemetry` | `POST` | 200 | Receives JSON telemetry payloads from the C++ service. |
| `/metrics` | `GET` | 200 | Aggregates and returns sliding-window sorting metrics (throughput, counts, efficiency). |

---

## Testing

### C++ — GoogleTest (27 tests)

The test suite is compiled and run locally using:
```bash
cmake -B build -S . -DBUILD_SORTING=ON -DBUILD_WITH_TFLITE=OFF
cmake --build build -j4
ctest --test-dir build -R "Sorting|NMS|Queue|Controller|Telemetry"
```

| Suite | Tests |
|---|---|
| `BoundingBoxAndNMSTest` | IoUOverlapCorrectness, IoUNoOverlap, IoUIdentical, NMS_SuppressesOverlappingBoxes, NMS_PreservesDifferentClasses, NMS_ConfidenceOrdering |
| `SortingQueueTest` | SPSC_FifoOrder, SPSC_FullAndEmptyStates, SPSC_WrapAroundCorrectness, SPSC_MultiThreadedStressTest |
| `MachineControllerTest` | Controller_ProcessesQueueElements, Controller_ThreadStopsOnRequest, Controller_HandlesEmptyQueue |
| `TelemetrySenderTest` | StreamFormattingCorrectness, HandlesSocketDisconnects Gracefully, ConnectRetryTimeout |
| `SortingPipelineEndToEndTest` | SimulatorFiresInference, FullPipelineProcessesMockFramesToEjection |

### Python — Integration tests (5 tests)

The Python test suite verifies FastAPI ingestion, metrics processing, and error conditions. Run using:
```bash
PYTHONPATH=edge_sorting_pipeline uv run pytest edge_sorting_pipeline/dashboard/backend/tests -v
```

| Test File | Verification |
|---|---|
| `test_backend` | Validates `/health` endpoint response payload structure and status. |
| `test_backend` | Validates `/ready` endpoint response payload. |
| `test_backend` | Simulates C++ telemetry ingestion and asserts that statistics are aggregated correctly. |
| `test_backend` | Asserts `/metrics` responds with expected metrics keys (throughput, efficiency). |
| `test_backend` | Verifies edge case limits (e.g. empty metrics windows, invalid JSON payload handling). |

---

## Containerization & Orchestration

The service is fully containerized for local development and edge simulation testing.

### Docker Compose Build & Up

```bash
cd edge_sorting_pipeline
docker compose build
docker compose up -d
```

The stack configures:
- **api**: FastAPI application and static server.
- **edge-core**: The C++ edge simulator executable compiled and run inside a minimal runtime environment.
