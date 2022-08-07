import importlib
import logging
import numpy as np

# Defaults cloned from existing ../bin/batch_processing script
DEFAULT_MODULE_PATH = "aicssegmentation.structure_wrapper.seg_"
DEFAULT_RESCALE_RATIO = -1


class StructureSegmenter:
    def __init__(self):
        self.log = logging.getLogger(__name__)

    def process_img(
        self,
        gene: str,
        image: np.array,
        rescale_ratio=DEFAULT_RESCALE_RATIO,
        module_path=DEFAULT_MODULE_PATH,
    ) -> (np.array, np.array):
        lower_gene = gene.lower()
        module_name = module_path + lower_gene
        try:
            logging.info(f"loading module {module_name}")
            seg_module = importlib.import_module(module_name)
            function_name = "Workflow_" + lower_gene
            logging.info("getting function " + function_name)
            SegModuleFunction = getattr(seg_module, function_name)
        except Exception as e:
            logging.error(f"raising failure while trying to get module/function for {module_name}")
            raise e
        try:
            logging.info("executing")
            (array_val, countour_val) = SegModuleFunction(
                struct_img=image,
                rescale_ratio=rescale_ratio,
                output_type="array_with_contour",
                output_path=None,  # these args are used when the output
                fn=None,
            )  # is a file
            return (array_val, countour_val)
        except Exception as e:
            logging.error("raising failure in dispatch for process_img")
            raise e
