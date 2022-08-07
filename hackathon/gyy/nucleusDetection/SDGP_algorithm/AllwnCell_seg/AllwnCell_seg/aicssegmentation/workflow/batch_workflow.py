import numpy as np

from datetime import datetime
from typing import List, Union
from aicsimageio import AICSImage
from aicsimageio.writers import OmeTiffWriter
from pathlib import Path
from aicssegmentation.util.filesystem import FileSystemUtilities
from aicssegmentation.exceptions import ArgumentNullError
from .workflow import Workflow
from .workflow_definition import WorkflowDefinition

SUPPORTED_FILE_EXTENSIONS = ["tiff", "tif", "czi"]


class BatchWorkflow:
    """
    Represents a batch of workflows to process.
    This class provides the functionality to run batches of workflows using multiple image inputs from a input directory
    according to the steps defined in its WorkflowDefinition.
    """

    def __init__(
        self,
        workflow_definition: WorkflowDefinition,
        input_dir: Union[str, Path],
        output_dir: Union[str, Path],
        channel_index: int = 0,
    ):
        if workflow_definition is None:
            raise ArgumentNullError("workflow_definition")
        if input_dir is None:
            raise ArgumentNullError("input_dir")
        if output_dir is None:
            raise ArgumentNullError("output_dir")

        self._workflow_definition = workflow_definition
        self._input_dir = Path(input_dir)

        if not self._input_dir.exists():
            raise ValueError("The input directory does not exist")

        self._output_dir = Path(output_dir)
        self._channel_index = channel_index
        self._processed_files: int = 0
        self._failed_files: int = 0
        self._log_path: Path = self._output_dir / f"log_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"

        # Create the output directory at output_dir if it does not exist already
        if not self._output_dir.exists():
            FileSystemUtilities.create_directory(self._output_dir)

        self._input_files = self._get_input_files(self._input_dir, SUPPORTED_FILE_EXTENSIONS)
        self._execute_generator = self._execute_generator_func()

    @property
    def total_files(self) -> int:
        return len(self._input_files)

    @property
    def processed_files(self) -> int:
        return self._processed_files

    @property
    def failed_files(self) -> int:
        return self._failed_files

    @property
    def input_dir(self) -> Path:
        return self._input_dir

    @property
    def output_dir(self) -> Path:
        return self._output_dir

    def is_done(self) -> bool:
        """
        Indicates whether all files / steps have been executed

        Use this to know when the batch workflow is complete if manually executing the workflow
        with execute_next()

        Returns:
            (bool): True if all files/steps have been executed, False if not
        """
        return self._processed_files == self.total_files

    def execute_all(self):
        if self.is_done():
            print("No files left to process")
            return

        print("Starting batch workflow...")
        print(f"Found {self.total_files} files to process.")

        while not self.is_done():
            self.execute_next()

        self.write_log_file_summary()

        print(f"Batch workflow complete. Check {self._log_path} for output log and summary.")

    def execute_next(self):
        if self.is_done():
            print("No files left to process")
            return

        next(self._execute_generator)

    def _execute_generator_func(self):
        for f in self._input_files:
            try:
                print(f"Start file {f.name}")

                # read and format image in the way we expect
                read_image = AICSImage(f)
                image_from_path = self._format_image_to_3d(read_image)

                # Run workflow on image
                workflow = Workflow(self._workflow_definition, image_from_path)
                while not workflow.is_done():
                    workflow.execute_next()
                    yield

                # Save output
                output_path = self._output_dir / f"{f.stem}.segmentation.tiff"
                result = workflow.get_most_recent_result()
                OmeTiffWriter.save(data=self._format_output(result), uri=output_path, dim_order="ZYX")

                msg = f"SUCCESS: {f}. Output saved at {output_path}"
                print(msg)
                self._write_to_log_file(msg)

            except Exception as ex:
                self._failed_files += 1
                msg = f"FAILED: {f}, ERROR: {ex}"
                print(msg)
                self._write_to_log_file(msg)
            finally:
                self._processed_files += 1

            yield

    def write_log_file_summary(self):
        """
        Write a log file to the output folder.
        """
        if self._processed_files == 0:
            report = (
                f"There were no files to process in the input directory to process \n "
                f"Using the Workflow: {self._workflow_definition.name}"
            )
        else:

            files_processed = self._processed_files - self._failed_files
            report = (
                f"{files_processed}/{self._processed_files} files were successfully processed \n "
                f"Using the Workflow: {self._workflow_definition.name}"
            )
        self._write_to_log_file(report)

    def _format_image_to_3d(self, image: AICSImage) -> np.ndarray:
        """
        Format images in the way that aics-segmention expects for most workflows (3d, zyx)

        Params:
            image_path (AICSImage): image to format

        Returns:
            np.ndarray: segment-able image for aics-segmentation
        """
        if len(image.scenes) > 1:
            raise ValueError("Multi-Scene images are unsupported")

        if image.dims.T > 1:
            raise ValueError("Timelapse images are unsupported.")

        if image.dims.C > 1:
            return image.get_image_data("ZYX", C=self._channel_index)

        return image.get_image_data("ZYX")

    def _format_output(self, image: np.ndarray):
        """
        Format segmented images to uint8 to save via AICSImage

        Params:
            image (np.ndarray): segmented image

        Returns:
            np.ndarray: image converted to uint8 for saving
        """
        image = image.astype(np.uint8)
        image[image > 0] = 255
        return image

    def _write_to_log_file(self, text: str):
        with open(self._log_path, "a") as writer:
            writer.write(f"{text}\n")

    def _get_input_files(self, input_dir: Path, extensions: List[str]) -> List[Path]:
        input_files = list()
        for ext in extensions:
            input_files.extend(input_dir.glob(f"*.{ext}"))
        return input_files
