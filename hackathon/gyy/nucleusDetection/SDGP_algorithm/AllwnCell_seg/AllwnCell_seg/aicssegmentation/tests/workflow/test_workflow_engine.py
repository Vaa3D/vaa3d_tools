from aicssegmentation.exceptions import ArgumentNullError
from aicssegmentation.workflow.workflow import Workflow
import pytest
import numpy as np

from unittest.mock import MagicMock, create_autospec
from aicssegmentation.workflow.workflow_config import WorkflowConfig
from aicssegmentation.workflow.workflow_engine import WorkflowEngine
from aicssegmentation.workflow.workflow_definition import PrebuiltWorkflowDefinition, WorkflowDefinition
from aicssegmentation.util.directories import Directories


class TestWorkflowEngine:
    expected_workflow_names = ["sec61b", "actn1", "test123"]
    expected_workflow_definitions = [
        PrebuiltWorkflowDefinition(name="sec61b", steps=list()),
        PrebuiltWorkflowDefinition(name="actn1", steps=list()),
        PrebuiltWorkflowDefinition(name="test123", steps=list()),
    ]

    def setup_method(self):
        self._mock_workflow_config: MagicMock = create_autospec(WorkflowConfig)
        self._mock_workflow_config.get_available_workflows.return_value = self.expected_workflow_names
        self._mock_workflow_config.get_workflow_definition.side_effect = self.expected_workflow_definitions
        self._workflow_engine = WorkflowEngine(self._mock_workflow_config)

    def test_workflow_definitions(self):
        assert self._workflow_engine.workflow_definitions == self.expected_workflow_definitions

    def test_get_executable_workflow_null_image_fails(self):
        with pytest.raises(ArgumentNullError):
            self._workflow_engine.get_executable_workflow("sec61b", None)

    def test_get_executable_workflow_unsupported_workflow_fails(self):
        with pytest.raises(ValueError):
            self._workflow_engine.get_executable_workflow("unsupported", np.ones((1, 1, 1)))

    @pytest.mark.parametrize("workflow_name", ["sec61b", "actn1", "test123"])
    def test_get_executable_workflow(self, workflow_name):
        workflow = self._workflow_engine.get_executable_workflow(workflow_name, np.ones((1, 1, 1)))
        assert isinstance(workflow, Workflow)
        assert workflow.workflow_definition.name == workflow_name

    def test_get_executable_workflow_from_config_file_null_path_fails(self):
        with pytest.raises(ArgumentNullError):
            self._workflow_engine.get_executable_workflow_from_config_file(None, np.ones((1, 1, 1)))

    def test_get_executable_workflow_from_config_file_null_image_fails(self):
        with pytest.raises(ArgumentNullError):
            self._workflow_engine.get_executable_workflow(
                Directories.get_structure_config_dir() / "conf_actb.json", None
            )

    @pytest.mark.parametrize(
        "path",
        [
            Directories.get_structure_config_dir() / "conf_actb.json",
            f"{Directories.get_structure_config_dir()}/conf_actb.json",
        ],
    )
    def test_get_executable_workflow_from_config_file(self, path):
        workflow = self._workflow_engine.get_executable_workflow_from_config_file(path, np.ones((1, 1, 1)))
        assert isinstance(workflow, Workflow)

    def test_save_workflow_definition(self):
        # Arrange
        path = "/path/to/workflow.json"
        workflow_def = create_autospec(WorkflowDefinition)

        # Act
        self._workflow_engine.save_workflow_definition(workflow_def, path)
        # Assert
        self._mock_workflow_config.save_workflow_definition_as_json.assert_called_once_with(workflow_def, path)
