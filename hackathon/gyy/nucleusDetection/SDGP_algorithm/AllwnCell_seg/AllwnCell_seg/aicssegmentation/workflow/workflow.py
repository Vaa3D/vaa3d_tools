import numpy as np
import logging

from typing import Any, Dict, List
from aicssegmentation.exceptions import ArgumentNullError
from .workflow_step import WorkflowStep
from .workflow_definition import WorkflowDefinition

log = logging.getLogger(__name__)


class Workflow:
    """
    Represents an executable aics-segmentation workflow
    This class provides the functionality to run a workflow using an image input
    according to the steps defined in its WorkflowDefinition.
    """

    def __init__(self, workflow_definition: WorkflowDefinition, input_image: np.ndarray):
        if workflow_definition is None:
            raise ArgumentNullError("workflow_definition")
        if input_image is None:
            raise ArgumentNullError("input_image")
        self._definition: WorkflowDefinition = workflow_definition
        self._starting_image: np.ndarray = input_image
        self._next_step: int = 0  # Next step to execute
        self._results: List = list()  # Store step results

    @property
    def workflow_definition(self) -> WorkflowDefinition:
        return self._definition

    def reset(self):
        """
        Reset the workflow so it can be run again
        """
        self._next_step = 0
        self._results = list()

    def get_next_step(self) -> WorkflowStep:
        """
        Get the next step to be performed

        Params:
            none

        Returns:
            (WorkflowStep): next WorkflowStep object to perform on image
            None if all steps have already been executed
        """
        if self._next_step >= len(self._definition.steps):
            return None
        return self._definition.steps[self._next_step]

    def execute_next(self, parameters: Dict[str, Any] = None) -> np.ndarray:
        """
        Execute the next workflow step.

        Params:
            parameters: Optional dictionary of parameter inputs to use when executing the step
                        If parameters are not provided, the step's default parameters will be used

        Returns:
            result (np.ndarray): resultant image from running the
            next workflow step
        """

        step = self.get_next_step()

        log.info(f"Executing step #{step.step_number}")

        # Pick which image to perform the workflow step on
        image: np.ndarray = None

        if self._next_step == 0:
            # First image, so use the starting image for the next workflow step
            image = [self._starting_image]
        elif self.is_done():
            # No more workflow steps to perform
            # TODO: what to do if done with workflow
            #  but execute_next is prompted?
            # printing message for now
            log.info("No steps left to run")
        else:
            image = list()
            for i in step.parent:
                res = self.get_result(i - 1)  # parents are 1 indexed
                image.append(res)

        result: np.ndarray = self.get_next_step().execute(image, parameters or step.parameter_values)
        self._results.append(result)

        # Only increment after running step
        self._next_step += 1
        return result

    # TODO maybe change this to match the step number instead?
    #      Review when we implement rerunning single workflow steps
    def get_result(self, step_index: int) -> np.ndarray:
        """
        Get the result image for a workflow step.

        You must call execute() on the workflow step in order to
        produce a result first before calling this function.

        Params:
            step_index (int): index of the WorkflowStep in the
            workflowengine to get the result image of.

        Returns:
            self.image (np.ndarray): Result of performing workflow step
                                     on the given image
                                     None if step has not been executed yet.
        """
        if step_index < 0:
            return self._starting_image
        if step_index >= len(self._results):
            return None  # returns None if the WorkflowStep has not been executed.

        return self._results[step_index]

    def get_most_recent_result(self) -> np.ndarray:
        """
        Get the result from the last executed WorkflowStep.

        Params:
           none

        Returns:
            (np.ndarray): Result of the last executed WorkflowStep,
                            returns the starting image if no Workflowsteps have
                            been run.
        """
        if self._next_step == 0:
            return self._starting_image  # TODO does this behavior make sense? Return None instead?
        else:
            return self.get_result(self._next_step - 1)

    def execute_all(self) -> np.ndarray:
        """
        Execute all steps in the Workflow
        Note: default parameters will be used to execute the steps. To execute a step
              with user-provided parameters, use execute_next()

        Params:
            none

        Returns:
            (np.ndarray): Result of the final WorkflowStep.
        """
        self.reset()
        while not self.is_done():
            self.execute_next()
        return self.get_most_recent_result()

    def is_done(self) -> bool:
        """
        Check if all WorkflowSteps have been executed.

        Params:
            none

        Returns:
            (bool): True if all WorkflowSteps have been executed, False if not
        """
        return self._next_step >= len(self._definition.steps)
