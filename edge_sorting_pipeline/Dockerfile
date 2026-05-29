# Stage 1: Build C++ application
FROM ubuntu:22.04 AS cpp-builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy the entire monorepo
COPY . .

# Build sorting app (TFLITE=OFF by default)
RUN cmake -B build -S . -DBUILD_SORTING=ON -DBUILD_WITH_TFLITE=OFF
RUN cmake --build build --target edge_sorting_app -j$(nproc)

# Stage 2: Build Python environment
FROM python:3.12-slim AS python-builder

COPY --from=ghcr.io/astral-sh/uv:latest /uv /uvx /bin/

WORKDIR /app
COPY pyproject.toml uv.lock ./
RUN uv sync --frozen --no-dev --no-install-project

# Stage 3: Runtime
FROM python:3.12-slim AS runner

WORKDIR /app

# Create non-root user
RUN groupadd -r app && useradd -r -g app app

# Copy virtual env from builder
COPY --from=python-builder /app/.venv /app/.venv

# Copy C++ executable from builder
COPY --from=cpp-builder /app/build/edge_sorting_pipeline/edge_sorting_app /app/edge_sorting_app

# Copy FastAPI backend code and frontend code
COPY edge_sorting_pipeline/dashboard/backend /app/dashboard/backend
COPY edge_sorting_pipeline/dashboard/frontend /app/dashboard/frontend

# Ensure permissions
RUN chown -R app:app /app

ENV PATH="/app/.venv/bin:$PATH"
ENV PYTHONUNBUFFERED=1

USER app

EXPOSE 8000

# Start FastAPI server
CMD ["uvicorn", "dashboard.backend.main:app", "--host", "0.0.0.0", "--port", "8000"]
