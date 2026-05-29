import os
import json
import requests
import pydicom
from pathlib import Path

def test_endpoints(anonymizer_daemon):
    port = anonymizer_daemon["port"]
    
    # 1. Test /health
    res = requests.get(f"http://localhost:{port}/health", timeout=1.0)
    assert res.status_code == 200
    assert res.json() == {"status": "ok"}
    
    # 2. Test /ready (should be ready since it successfully processed the startup file)
    res = requests.get(f"http://localhost:{port}/ready", timeout=1.0)
    assert res.status_code == 200
    assert res.json() == {"status": "ready"}
    
    # 3. Test /metrics
    res = requests.get(f"http://localhost:{port}/metrics", timeout=1.0)
    assert res.status_code == 200
    metrics_body = res.text
    
    # Assert prometheus metrics exist
    assert "dicom_files_processed_total" in metrics_body
    assert 'result="success"' in metrics_body
    assert "dicom_processing_seconds_count 1" in metrics_body
    assert "dicom_processing_seconds_sum" in metrics_body
    assert "dicom_processing_seconds_bucket" in metrics_body

def test_anonymized_output_file(anonymizer_daemon):
    output_dir = anonymizer_daemon["temp_dirs"]["output"]
    output_file = output_dir / "sample_64x64.dcm"
    
    # Wait up to 1 second for output file to exist
    import time
    for _ in range(10):
        if output_file.exists():
            break
        time.sleep(0.1)
        
    assert output_file.exists()
    
    # Read the anonymized file with pydicom
    ds = pydicom.dcmread(output_file)
    
    # Verify sensitive tags are anonymized (hashed to 64-char hex strings)
    assert len(ds.PatientName) == 64
    assert len(ds.PatientID) == 64
    assert ds.PatientName != "Test^Patient"
    assert ds.PatientID != "PAT001"
    
    # Verify non-sensitive tags are preserved
    assert ds.Modality == "CT"
    assert ds.Rows == 64
    assert ds.Columns == 64

def test_structured_log_output(anonymizer_daemon):
    log_file = anonymizer_daemon["temp_dirs"]["log_file"]
    
    # Wait for log entries to be flushed
    import time
    for _ in range(10):
        if log_file.exists() and log_file.stat().st_size > 0:
            break
        time.sleep(0.1)
        
    assert log_file.exists()
    
    # Read first line of NDJSON
    with open(log_file, "r") as f:
        line = f.readline()
        
    log_entry = json.loads(line)
    
    # Validate structure
    assert "ts_ns" in log_entry
    assert "file_size_bytes" in log_entry
    assert "duration_us" in log_entry
    assert log_entry["status"] == "success"
    assert log_entry["tags_scrubbed"] > 0

