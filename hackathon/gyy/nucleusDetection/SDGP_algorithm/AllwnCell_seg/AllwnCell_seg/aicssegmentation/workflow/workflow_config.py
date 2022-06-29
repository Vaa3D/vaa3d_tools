import json

from pathlib import Path
from typing import Dict, List
from aicssegmentation.util.directories import Directories
from .segmenter_function import SegmenterFunction, FunctionParameter, WidgetType
from .workflow_definition import WorkflowDefinition, PrebuiltWorkflowDefinition
from .workflow_step import WorkflowStep, WorkflowStepCategory


class ConfigurationException(Exception):
    """
    Raised when errors are encountered reading from Configuration files
    """

    pass


class WorkflowConfig:
    """
    Provides access to structure workflow configuration
    """

    def __init__(self):
        self._all_functions = None
        self._available_workflow_names = None

    def get_available_workflows(self) -> List[str]:
        """
        Get the list of all workflows available through configuration
        """
        if self._available_workflow_names is None:
            json_list = sorted(Directories.get_structure_config_dir().glob("conf_*.json"))
            self._available_workflow_names = [p.stem[5:] for p in json_list]

        return self._available_workflow_names

    def get_all_functions(self) -> List[SegmenterFunction]:
        """
        Get the list of all available Functions from configuration
        """
        if self._all_functions is None:
            path = Directories.get_structure_config_dir() / "all_functions.json"

            try:
                with open(path) as file:
                    obj = json.load(file)
                    self._all_functions = self._all_functions_decoder(obj)
            except Exception as ex:
                raise ConfigurationException(f"Error reading json configuration from {path}") from ex

        return self._all_functions

    def get_workflow_definition(self, workflow_name: str) -> PrebuiltWorkflowDefinition:
        """
        Get a WorkflowDefinition for the given workflow from the corresponding
        prebuilt json structure config
        """
        if workflow_name is None or len(workflow_name.strip()) == 0:
            raise ValueError("workflow_name cannot be empty")

        if workflow_name not in self.get_available_workflows():
            raise ValueError(f"No workflow configuration available for {workflow_name}")

        path = Directories.get_structure_config_dir() / f"conf_{workflow_name}.json"

        return self.get_workflow_definition_from_config_file(path, workflow_name, prebuilt=True)

    def get_workflow_definition_from_config_file(
        self, file_path: Path, workflow_name: str = None, prebuilt: bool = False
    ) -> WorkflowDefinition:
        """
        Get a WorkflowDefinition based off the given json configuration file
        """
        if file_path.suffix.lower() != ".json":
            raise ValueError("Workflow configuration file must be a json file with .json file extension.")

        with open(file_path) as file:
            try:
                obj = json.load(file)
                return self._workflow_decoder(obj, workflow_name or file_path.name, prebuilt)
            except Exception as ex:
                raise ConfigurationException(f"Error reading json configuration from {file_path}") from ex

    def save_workflow_definition_as_json(self, workflow_definition: WorkflowDefinition, output_file_path: Path):
        """
        Save a WorkflowDefinition as a json config file
        """
        if output_file_path.suffix.lower() != ".json":
            raise ValueError("Workflow configuration file save path must have a .json extension.")

        with open(output_file_path, "w") as file:
            json.dump(self._workflow_encoder(workflow_definition), file, indent=4, sort_keys=True)

    def _all_functions_decoder(self, obj: Dict) -> List[SegmenterFunction]:
        """
        Decode Functions config (all_functions.json)
        """

        def build_function_parameter(name: str, data: Dict):
            return FunctionParameter(
                name=name,
                widget_type=WidgetType.from_str(data["widget_type"]),
                data_type=data["data_type"],
                min_value=data.get("min", None),
                max_value=data.get("max", None),
                increment=data.get("increment", None),
                options=data.get("options", None),
            )

        functions = list()
        for function_k, function_v in obj.items():
            function = SegmenterFunction(
                name=function_k,
                display_name=function_v["name"],
                function=function_v["python::function"],
                module=function_v["python::module"],
            )

            if function_v.get("parameters") is not None and len(function_v["parameters"]) > 0:
                params = dict()

                for param_k, param_v in function_v["parameters"].items():
                    param_name = param_k
                    params[param_name] = list()

                    if isinstance(param_v, dict):
                        params[param_name].append(build_function_parameter(param_name, param_v))
                    elif isinstance(param_v, list):
                        for item in param_v:
                            params[param_name].append(build_function_parameter(param_name, item))

                function.parameters = params

            functions.append(function)

        return functions

    def _workflow_decoder(self, obj: Dict, workflow_name: str, prebuilt: bool = False) -> WorkflowDefinition:
        """
        Decode Workflow config (conf_{workflow_name}.json)
        """
        functions = self.get_all_functions()
        steps: List[WorkflowStep] = list()

        for step_k, step_v in obj.items():
            step_number = int(step_k)
            function_id = step_v["function"]
            function = next(filter(lambda f: f.name == function_id, functions), None)

            if function is None:
                raise ConfigurationException(
                    f"Could not find a Segmenter function matching the function identifier <{function_id}>."
                )

            if isinstance(step_v["parent"], list):
                parent = step_v["parent"]
            else:
                parent = [step_v["parent"]]

            step = WorkflowStep(
                category=WorkflowStepCategory.from_str(step_v["category"]),
                function=function,
                step_number=step_number,
                parent=parent,
            )

            if step_v.get("parameter_values") is not None and len(step_v["parameter_values"]) > 0:
                param_defaults = dict()

                for param_k, param_v in step_v["parameter_values"].items():
                    param_name = param_k
                    param_defaults[param_name] = param_v

                step.parameter_values = param_defaults

            steps.append(step)

        steps.sort(key=lambda s: s.step_number)

        if prebuilt:
            return PrebuiltWorkflowDefinition(workflow_name, steps)
        else:
            return WorkflowDefinition(workflow_name, steps)

    def _workflow_encoder(self, workflow_definition: WorkflowDefinition) -> Dict:
        """
        Encode a WorkflowDefinition to a json dictionary
        """

        # TODO add header / version ?
        result = dict()
        for step in workflow_definition.steps:
            step_number = str(step.step_number)
            parent = step.parent[0] if len(step.parent) == 1 else step.parent

            step_dict = {
                step_number: {"function": step.function.name, "category": step.category.value, "parent": parent}
            }
            if step.parameter_values is not None:
                step_dict[step_number].update({"parameter_values": step.parameter_values})

            result.update(step_dict)

        return result
