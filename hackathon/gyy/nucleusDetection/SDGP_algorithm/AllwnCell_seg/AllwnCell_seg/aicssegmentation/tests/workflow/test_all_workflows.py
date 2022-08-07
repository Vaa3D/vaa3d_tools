import pytest

from aicsimageio import imread
from aicssegmentation.workflow import WorkflowEngine
from . import SUPPORTED_STRUCTURE_NAMES


IMG_SCALING = {"cetn2": 5100}


class TestAllWorkflows:
    def setup_method(self):
        self._workflow_engine = WorkflowEngine()

    @pytest.mark.parametrize("workflow_name", SUPPORTED_STRUCTURE_NAMES)
    def test_execute_all_workflows(self, workflow_name, resources_dir):

        # Arrange
        img_path = resources_dir / "images" / "random_input.tiff"
        random_array = imread(img_path).reshape(*(128, 128, 128))
        random_array *= IMG_SCALING.get(workflow_name, 1)

        workflow = self._workflow_engine.get_executable_workflow(workflow_name, random_array)

        # Act
        workflow.execute_all()

        # Assert
        assert workflow.get_next_step() is None
        assert workflow.is_done()
