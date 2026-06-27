# Build Instructions

## Prerequisites

### Linux (GCC 13+)
```bash
sudo apt-get install cmake ninja-build g++ libgtk2.0-dev pkg-config
pip install conan
```

### macOS (Apple Clang)
```bash
brew install cmake ninja
pip install conan
```

## Build (Linux / macOS)

```bash
cd cv_websocket_telemetry
conan profile detect                              # first time only
conan install . --output-folder=build --build=missing -s build_type=Release
cmake --preset linux
cmake --build --preset linux --parallel
```

## Run Tests

```bash
ctest --test-dir build/linux --output-on-failure -V
```

Expected: 13/13 tests pass.

## Run the Application

Generate test fixture first (requires `pip install opencv-python`):
```bash
python3 scripts/gen_test_fixture.py
```

Start the server:
```bash
./build/linux/cv_ws_telemetry --input tests/fixtures/test_input.mp4 --port 9001
```

In another terminal, run the Python client:
```bash
pip install websockets
python3 scripts/ws_client.py
```

## Docker

```bash
docker build -t cv_ws_telemetry .
docker run --rm -p 9001:9001 cv_ws_telemetry
```

Or with docker-compose (server + auto-test client):
```bash
docker compose up --abort-on-container-exit
```
