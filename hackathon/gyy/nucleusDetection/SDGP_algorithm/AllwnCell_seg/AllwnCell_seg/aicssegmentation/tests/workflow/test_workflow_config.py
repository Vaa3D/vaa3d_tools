import pytest
import json

from pathlib import Path
from aicssegmentation.workflow.workflow_config import WorkflowConfig
from aicssegmentation.util.directories import Directories
from aicssegmentation.workflow.workflow_definition import WorkflowDefinition, PrebuiltWorkflowDefinition
from . import SUPPORTED_STRUCTURE_NAMES


class TestWorkflowConfig:
    def setup_method(self):
        self._workflow_config = WorkflowConfig()

    def test_get_available_workflows(self):
        workflows = self._workflow_config.get_available_workflows()
        assert workflows == SUPPORTED_STRUCTURE_NAMES

    def test_get_all_functions(self):
        functions = self._workflow_config.get_all_functions()

        assert functions is not None
        assert len(functions) > 0

    @pytest.mark.parametrize("name", [None, "", "  "])
    def test_get_workflow_definition_empty_name_fails(self, name):
        with pytest.raises(ValueError):
            workflow_def = self._workflow_config.get_workflow_definition(name)

    def test_get_workflow_definition_unavailable_workflow_fails(self):
        with pytest.raises(ValueError):
            workflow_def = self._workflow_config.get_workflow_definition("unsupported workflow")

    @pytest.mark.parametrize("name", SUPPORTED_STRUCTURE_NAMES)
    def test_get_workflow_definition(self, name):
        workflow_def = self._workflow_config.get_workflow_definition(name)
        assert isinstance(workflow_def, PrebuiltWorkflowDefinition)
        assert workflow_def.name == name

    def test_get_workflow_definition_from_config_file(self):
        path = Directories.get_structure_config_dir() / "conf_actb.json"
        workflow_def = self._workflow_config.get_workflow_definition_from_config_file(path)
        assert isinstance(workflow_def, WorkflowDefinition)
        assert workflow_def.name == "conf_actb.json"

    @pytest.mark.parametrize("name", SUPPORTED_STRUCTURE_NAMES)
    def test_save_workflow_definition_as_json(self, name, tmp_path: Path):
        # Arrange
        workflow_config_path = Directories.get_structure_config_dir() / f"conf_{name}.json"
        expected_json = json.dumps(json.load(open(workflow_config_path, "r")), sort_keys=True)
        actb_workflow_def = self._workflow_config.get_workflow_definition_from_config_file(workflow_config_path)

        # Act
        output_path = tmp_path / "test_output.json"
        self._workflow_config.save_workflow_definition_as_json(actb_workflow_def, output_path)
        result = json.dumps(json.load(open(output_path, "r")), sort_keys=True)

        # Assert
        assert result == expected_json
