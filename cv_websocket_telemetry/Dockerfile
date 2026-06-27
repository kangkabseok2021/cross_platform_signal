# ── Stage 1: Build ─────────────────────────────────────────────────────────
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake ninja-build g++ \
        python3-pip pkg-config git \
        libgtk2.0-dev \
    && rm -rf /var/lib/apt/lists/*

RUN pip install conan --break-system-packages && conan profile detect

WORKDIR /src

# Cache Conan packages separately from source copy
COPY conanfile.py .
RUN conan install . --output-folder=build --build=missing -s build_type=Release

COPY . .
RUN cmake --preset docker && cmake --build --preset docker --parallel

# ── Stage 2: Runtime ───────────────────────────────────────────────────────
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        libgomp1 libgtk2.0-0 libglib2.0-0 libgl1 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/build/docker/cv_ws_telemetry /app/cv_ws_telemetry
COPY --from=builder /src/tests/fixtures/test_input.mp4 /app/test_input.mp4

WORKDIR /app
EXPOSE 9001
ENTRYPOINT ["/app/cv_ws_telemetry"]
CMD ["--input", "/app/test_input.mp4", "--port", "9001"]
