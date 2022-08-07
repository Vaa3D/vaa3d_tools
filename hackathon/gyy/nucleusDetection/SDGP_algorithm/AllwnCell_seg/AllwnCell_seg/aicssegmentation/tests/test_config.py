import json
from aicssegmentation.util.directories import Directories


class TestConfig:
    def test_category_definition(self):
        json_list = sorted(Directories.get_structure_config_dir().glob("conf_*.json"))
        for workflow_config in json_list:
            with open(workflow_config, "r") as read_file:
                cfg = json.load(read_file)
            for step in cfg:
                assert "category" in cfg[step], f"Step {step} in {workflow_config} needs a category."

    def test_configs_match_all_functions(self):
        with open(Directories.get_structure_config_dir() / "all_functions.json") as all_fctns_file:
            all_functions = json.load(all_fctns_file)

        json_list = sorted(Directories.get_structure_config_dir().glob("conf_*.json"))

        for workflow_config in json_list:
            with open(workflow_config, "r") as read_file:
                cfg = json.load(read_file)
            for step in cfg:
                function_key = cfg[step]["function"]

                # outside packages are not included in all_functions.json
                # if "aicssegmentation" not in cfg[step]["module"]:
                #     continue

                # all functions used in configs must be defined in all_functions.json
                assert (
                    function_key in all_functions.keys()
                ), f'Func "{function_key}" in {workflow_config} is not in all_functions'

                # check that the parameters in the config file match the parameters
                # required in all_functions.json
                reference_parameters = all_functions[function_key]["parameters"]
                if "parameter_values" in cfg[step]:
                    for param in cfg[step]["parameter_values"]:
                        assert param in reference_parameters.keys(), (
                            f'Parameter "{param}" in {workflow_config} is' f"not defined for function {function_key}"
                        )
