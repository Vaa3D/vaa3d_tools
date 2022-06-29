from aicssegmentation.workflow.segmenter_function import FunctionParameter, WidgetType
import pytest
import numpy as np

from aicssegmentation.workflow.workflow_step import (
    WorkflowStep,
    WorkflowStepCategory,
    SegmenterFunction,
)


class TestWorkflowStep:
    def test_get_name(self):
        function_name = "Intensity Normalization"
        function = SegmenterFunction(
            name="intensity_normalization",
            display_name=function_name,
            function="intensity_normalization",
            module="aicssegmentation.core.pre_processing_utils",
        )
        step = WorkflowStep(category=WorkflowStepCategory.PRE_PROCESSING, function=function, step_number=1, parent=[0])

        assert step.name == function_name

    def test_execute_bad_input_fails(self):
        # Arrange
        function = SegmenterFunction(
            name="single_input",
            display_name="Test single input",
            function="function_single_input",
            module="aicssegmentation.tests.workflow.mock_module",
        )
        step = WorkflowStep(category=WorkflowStepCategory.PRE_PROCESSING, function=function, step_number=1, parent=[0])
        img = np.ones((75, 200, 200))

        # Act / Assert
        with pytest.raises(ValueError):
            step.execute(img)

    def test_execute_bad_module_fails(self):
        # Arrange
        function = SegmenterFunction(
            name="single_input",
            display_name="Test single input",
            function="function_single_input",
            module="aicssegmentation.not_a_module",
        )
        step = WorkflowStep(category=WorkflowStepCategory.PRE_PROCESSING, function=function, step_number=1, parent=[0])
        img = np.ones((75, 200, 200))

        # Act / Assert
        with pytest.raises(ModuleNotFoundError):
            step.execute([img])

    def test_execute_bad_function_fails(self):
        # Arrange
        function = SegmenterFunction(
            name="single_input",
            display_name="Test single input",
            function="not_a_function",
            module="aicssegmentation.tests.workflow.mock_module",
        )
        step = WorkflowStep(category=WorkflowStepCategory.PRE_PROCESSING, function=function, step_number=1, parent=[0])

        img = np.ones((75, 200, 200))

        # Act / Assert
        with pytest.raises(AttributeError):
            step.execute([img])

    def test_execute_bad_parameters_fails(self):
        # Arrange
        function = SegmenterFunction(
            name="single_input",
            display_name="Test single input",
            function="not_a_function",
            module="aicssegmentation.tests.workflow.mock_module",
        )
        step = WorkflowStep(category=WorkflowStepCategory.PRE_PROCESSING, function=function, step_number=1, parent=[0])

        img = np.ones((75, 200, 200))

        # Act / Assert
        with pytest.raises(AttributeError):
            step.execute([img])

    def test_execute_function_single_input(self):
        # Arrange
        function = SegmenterFunction(
            name="single_input",
            display_name="Test single input",
            function="function_single_input",
            module="aicssegmentation.tests.workflow.mock_module",
            parameters={
                "x": FunctionParameter("x", WidgetType.SLIDER, "int"),
                "y": FunctionParameter("y", WidgetType.SLIDER, "int"),
            },
        )
        step = WorkflowStep(category=WorkflowStepCategory.PRE_PROCESSING, function=function, step_number=1, parent=[0])
        img = np.ones((75, 200, 200))
        params = {"a": 1, "b": 2}

        # Act
        with pytest.raises(ValueError):
            step.execute([img], params)

    def test_execute_function_single_input_with_parameters(self):
        # Arrange
        function = SegmenterFunction(
            name="single_input_with_parameters",
            display_name="Test single input with parameters",
            function="function_single_input_with_parameters",
            module="aicssegmentation.tests.workflow.mock_module",
            parameters={
                "x": FunctionParameter("x", WidgetType.SLIDER, "int"),
                "y": FunctionParameter("y", WidgetType.SLIDER, "int"),
            },
        )
        step = WorkflowStep(category=WorkflowStepCategory.PRE_PROCESSING, function=function, step_number=1, parent=[0])
        img = np.ones((75, 200, 200))
        params = {"x": 5, "y": 10}

        # Act
        result = step.execute([img], params)

        # Assert
        assert result == 15

    def test_execute_function_list_input(self):
        # Arrange
        function = SegmenterFunction(
            name="list_input",
            display_name="Test list input",
            function="function_list_input",
            module="aicssegmentation.tests.workflow.mock_module",
        )
        step = WorkflowStep(category=WorkflowStepCategory.PRE_PROCESSING, function=function, step_number=1, parent=[0])
        img1 = np.ones((75, 200, 200))
        img2 = np.ones((75, 200, 200))

        # Act
        result = step.execute([img1, img2])

        # Assert
        assert result == 1

    def test_execute_function_list_input_with_parameters(self):
        # Arrange
        function = SegmenterFunction(
            name="list_input_with_parameters",
            display_name="Test list input with parameters",
            function="function_list_input_with_parameters",
            module="aicssegmentation.tests.workflow.mock_module",
            parameters={
                "x": FunctionParameter("x", WidgetType.SLIDER, "int"),
                "y": FunctionParameter("y", WidgetType.SLIDER, "int"),
            },
        )
        step = WorkflowStep(category=WorkflowStepCategory.PRE_PROCESSING, function=function, step_number=1, parent=[0])
        img1 = np.ones((75, 200, 200))
        img2 = np.ones((75, 200, 200))
        params = {"x": 5, "y": 10}

        # Act
        result = step.execute([img1, img2], params)

        # Assert
        assert result == 15
