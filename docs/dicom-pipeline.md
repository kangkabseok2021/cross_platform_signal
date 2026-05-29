# AI-Generated DICOM Anonymization & Observability Pipeline

A C++20 real-time service that ingests medical image directories, performs deterministic SHA-256 anonymization on sensitive Patient Identity Info (PII) tags using OpenSSL, and exposes Prometheus observability telemetry metrics via an asynchronous HTTP server on port 8080. A Python integration test suite uses `pytest` and `pydicom` to run end-to-end endpoint and verification tests.

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│  Raw DICOM Files Directory (.dcm / .DCM)                        │
└───────────────────────┬─────────────────────────────────────────┘
                        │ Ingestion
┌───────────────────────▼─────────────────────────────────────────┐
│  DicomReader                                                    │
│  Validates header, parses pixel sizing, extracts PII tags       │
└───────────────────────┬─────────────────────────────────────────┘
                        │ DicomFile struct
┌───────────────────────▼─────────────────────────────────────────┐
│  DicomAnonymizer (OpenSSL SHA-256)                             │
│  Deterministic hashing of PatientName & PatientID tags          │
└───────────────────────┬───────────────────────┬─────────────────┘
                        │                       │
                        │ Anonymized Files      │ ProcessingStats
┌───────────────────────▼─────────────┐ ┌───────▼─────────────────┐
│  Anonymized DICOM Files Directory   │ │  ProcessingLogger        │
└─────────────────────────────────────┘ │  NDJSON structured logs │
                                        └───────┬─────────────────┘
                                                │ ProcessingStats
                                        ┌───────▼─────────────────┐
                                        │  MetricsServer          │
                                        │  Asynchronous HTTP      │
                                        │  /health, /ready        │
                                        │  /metrics (Prometheus)  │
                                        └─────────────────────────┘
```

---

## Design Patterns & Safety

**Resource Acquisition Is Initialization (RAII)** — Custom RAII wrappers encapsulate C-style OpenSSL EVP message digest contexts (`EVP_MD_CTX`) and DCMTK references to prevent memory leaks and raw pointer errors.

**Structured Asynchronous Telemetry** — The `ProcessingLogger` writes formatted NDJSON rows to log streams, flushing immediately to ensure real-time visibility. In parallel, `MetricsServer` runs on a background `std::thread` via `cpp-httplib`, avoiding processing blocking.

---

## Observability API

The daemon runs an HTTP server exposing:

| Endpoint | Method | Status | Description |
|---|---|---|---|
| `/health` | `GET` | 200 | Always returns `{"status":"ok"}`. |
| `/ready` | `GET` | 200 / 503 | Returns `200` after the first successful file is processed, otherwise `503`. |
| `/metrics` | `GET` | 200 | Exposes Prometheus exposition format counters and processing latency histograms. |

---

## Testing

### C++ — GoogleTest (13 tests)

```bash
cmake -B build -DBUILD_DICOM=ON
cmake --build build --target test_dicom
ctest --test-dir build -R "Dicom|Metrics|Logger"
```

| Suite | Tests |
|---|---|
| `DicomReaderTest` | ReadValidDicom_PixelDataSize, ReadValidDicom_PatientNamePresent, ReadMissingFile_Throws, ReadTruncatedFile_Throws |
| `DicomAnonymizerTest` | Sha256Hex_CorrectLengthAndDeterminism, Anonymize_PatientNameIsHashed, Anonymize_PixelDataUnchanged, Anonymize_NonSensitiveTagPreserved, Anonymize_ScrubbedTagCount_Correct |
| `ProcessingLoggerTest` | LogToStream_ValidNdjson, LogMultipleRecords_FlushesOnFlush |
| `MetricsServerTest` | HealthAndReadyEndpoints, RecordAndScrapeMetrics |

### Python — Integration tests (3 tests)

```bash
uv run pytest dicom_pipeline/tests_python -v
```

| Test File | Verification |
|---|---|
| `test_endpoints` | Queries `/health`, `/ready`, and prometheus `/metrics` for exact counters and buckets. |
| `test_anonymized_output_file` | Uses `pydicom` to read the output file, asserting PII tags are anonymized and image properties are preserved. |
| `test_structured_log_output` | Parses the generated NDJSON telemetry log to verify schema structure and success status. |

---

## Containerization & Kubernetes

The daemon can be built using the multi-stage Dockerfile and deployed on a Kubernetes cluster.

### Docker Build

```bash
docker build -t dicom-anonymizer:latest -f dicom_pipeline/docker/Dockerfile .
```

### Kubernetes Manifests

The Kubernetes configuration is defined in the `k8s/` folder:

- [configmap.yaml](file:///Users/kab/Projects/Portfolio/cross_platform_signal/k8s/configmap.yaml): Sets standard paths and ports.
- [deployment.yaml](file:///Users/kab/Projects/Portfolio/cross_platform_signal/k8s/deployment.yaml): Configures resource boundaries, non-root user execution context, health/readiness probes, and mounts local `emptyDir` scratch directories.
- [service.yaml](file:///Users/kab/Projects/Portfolio/cross_platform_signal/k8s/service.yaml): Exposes metrics port 8080 inside the cluster.
