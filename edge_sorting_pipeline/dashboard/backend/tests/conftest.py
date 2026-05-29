import pytest
from fastapi.testclient import TestClient
from dashboard.backend.main import app, state

@pytest.fixture(autouse=True)
def reset_state():
    state.reset()
    yield

@pytest.fixture
def client():
    return TestClient(app)
