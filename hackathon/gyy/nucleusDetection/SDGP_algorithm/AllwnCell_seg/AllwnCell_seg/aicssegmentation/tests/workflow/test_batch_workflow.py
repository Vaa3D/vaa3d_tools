import numpy as np
import pytest

from unittest import mock
from aicsimageio.writers.ome_tiff_writer import OmeTiffWriter
from pathlib import Path
from aicsimageio import AICSImage
from numpy import random
from aicssegmentation.workflow.batch_workflow import BatchWorkflow
from aicssegmentation.workflow.workflow_config import WorkflowConfig


@pytest.fixture
def batch_workflow(tmp_path: Path):
    input_dir = tmp_path / "input"
    output_dir = tmp_path / "output"
    input_dir.mkdir(parents=True, exist_ok=True)
    output_dir.mkdir(parents=True, exist_ok=True)

    for i in range(0, 10):
        three_d_image = np.zeros((10, 100, 100))
        OmeTiffWriter.save(data=three_d_image, uri=input_dir / f"test{i}.tiff", dim_order="ZYX")

    definition = WorkflowConfig().get_workflow_definition("sec61b")
    return BatchWorkflow(definition, input_dir, output_dir, channel_index=0)


class TestBatchWorkflow:
    def test_format_image_to_3d(self, batch_workflow: BatchWorkflow):
        three_d_image = AICSImage(random.random((2, 3, 4)), dim_order="ZYX")

        assert len(batch_workflow._format_image_to_3d(three_d_image).shape) == 3

    def test_format_image_to_3d_timeseries(self, batch_workflow: BatchWorkflow):
        image = AICSImage(np.ones((5, 1, 10, 100, 100)), dim_order="TCZYX")
        with pytest.raises(ValueError):
            batch_workflow._format_image_to_3d(image)

    def test_format_image_to_3d_multiscene(self, batch_workflow: BatchWorkflow):
        # Two scenes
        image = AICSImage([np.ones((5, 1, 10, 100, 100)), np.ones((5, 1, 10, 100, 100))], known_dims="TCZYX")
        with pytest.raises(ValueError):
            batch_workflow._format_image_to_3d(image)

    @mock.patch("aicssegmentation.workflow.batch_workflow.Workflow.execute_all")
    def test_process_all(self, mock_workflow_execute_all, batch_workflow: BatchWorkflow):
        # Arrange
        mock_workflow_execute_all.return_value = np.zeros((10, 100, 100))

        # Act
        batch_workflow.execute_all()

        # Assert
        assert batch_workflow.output_dir.exists()
        batch_workflow.output_dir.joinpath("log.txt").exists()
        assert len(list(batch_workflow.output_dir.glob("*.tiff"))) == 10
