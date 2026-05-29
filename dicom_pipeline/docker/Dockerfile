# ── Build Stage ──────────────────────────────────────────────────────────────
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install compilation toolchain and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libdcmtk-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy the entire workspace into builder
COPY . .

# Configure and build the dicom_anonymizer target
# We use static linking for dicom_core, nlohmann_json, and cpp-httplib
RUN cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_DICOM=ON \
    -DCMAKE_POLICY_DEFAULT_CMP0135=NEW \
    && cmake --build build --target dicom_anonymizer -j $(nproc)

# ── Runtime Stage ────────────────────────────────────────────────────────────
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install runtime shared libraries for DCMTK and OpenSSL
RUN apt-get update && apt-get install -y \
    libssl3 \
    libdcmtk16 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user and directories
RUN groupadd -r pipeline && useradd -r -g pipeline -u 10001 pipeline

WORKDIR /app

# Create directories for data processing and observability logs
RUN mkdir -p /app/input /app/output /app/logs && \
    chown -R pipeline:pipeline /app

# Copy built daemon from builder
COPY --from=builder --chown=pipeline:pipeline /build/build/dicom_pipeline/dicom_anonymizer /app/dicom_anonymizer

# Run as non-root user
USER pipeline:pipeline

# Expose metrics port
EXPOSE 8080

# Run daemon by default
ENTRYPOINT ["/app/dicom_anonymizer"]
CMD ["--input-dir", "/app/input", "--output-dir", "/app/output", "--metrics-port", "8080", "--log-file", "/app/logs/processing.ndjson"]
