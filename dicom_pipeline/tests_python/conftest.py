import os
import socket
import subprocess
import time
import shutil
import pytest
from pathlib import Path

def find_free_port():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("", 0))
        return s.getsockname()[1]

@pytest.fixture(scope="module")
def project_root():
    return Path(__file__).resolve().parents[2]

@pytest.fixture(scope="module")
def dicom_fixture(project_root):
    fixture_path = project_root / "dicom_pipeline" / "tests" / "fixtures" / "sample_64x64.dcm"
    if not fixture_path.exists():
        pytest.fail(f"Fixture sample_64x64.dcm not found at {fixture_path}")
    return fixture_path

@pytest.fixture(scope="module")
def temp_dirs(tmp_path_factory):
    tmp_dir = tmp_path_factory.mktemp("dicom_pipeline_integration")
    input_dir = tmp_dir / "input"
    output_dir = tmp_dir / "output"
    log_dir = tmp_dir / "logs"
    
    input_dir.mkdir()
    output_dir.mkdir()
    log_dir.mkdir()
    
    return {
        "input": input_dir,
        "output": output_dir,
        "log_file": log_dir / "pipeline.ndjson"
    }

@pytest.fixture(scope="module")
def anonymizer_daemon(project_root, temp_dirs, dicom_fixture):
    # Copy fixture to input directory so pipeline processes it on start
    shutil.copy(dicom_fixture, temp_dirs["input"] / "sample_64x64.dcm")
    
    # Locate binary
    binary_path = project_root / "build" / "dicom_pipeline" / "dicom_anonymizer"
    if not binary_path.exists():
        pytest.fail(f"dicom_anonymizer executable not found at {binary_path}. Did you build the project?")
        
    port = find_free_port()
    
    cmd = [
        str(binary_path),
        "--input-dir", str(temp_dirs["input"]),
        "--output-dir", str(temp_dirs["output"]),
        "--metrics-port", str(port),
        "--log-file", str(temp_dirs["log_file"])
    ]
    
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    # Wait for process to spin up and bind to port by querying /health
    import requests
    health_url = f"http://localhost:{port}/health"
    ready = False
    for _ in range(50):
        if process.poll() is not None:
            # Daemon crashed early
            stdout, stderr = process.communicate()
            pytest.fail(f"Daemon failed to start.\nStdout:\n{stdout}\nStderr:\n{stderr}")
        try:
            res = requests.get(health_url, timeout=0.2)
            if res.status_code == 200:
                ready = True
                break
        except requests.exceptions.ConnectionError:
            pass
        time.sleep(0.1)
        
    if not ready:
        process.terminate()
        stdout, stderr = process.communicate()
        pytest.fail(f"Daemon failed to bind to port {port} in time.\nStdout:\n{stdout}\nStderr:\n{stderr}")
        
    yield {
        "process": process,
        "port": port,
        "temp_dirs": temp_dirs
    }
    
    # Cleanup: SIGTERM
    process.terminate()
    try:
        process.wait(timeout=2.0)
    except subprocess.TimeoutExpired:
        process.kill()
