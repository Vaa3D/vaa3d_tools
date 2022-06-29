import pytest

from pathlib import Path


@pytest.fixture
def resources_dir() -> Path:
    return Path(__file__).parent / "resources"
