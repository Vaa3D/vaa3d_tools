# flake8: noqa
# ##### import functions ####

# ### do not remove ####
from typing import Union
from pathlib import Path
import numpy as np
from aicssegmentation.core.output_utils import (
    save_segmentation,
    generate_segmentation_contour,
)


def Workflow_template(
    struct_img: np.ndarray,
    rescale_ratio: float = -1,
    output_type: str = "default",
    output_path: Union[str, Path] = None,
    fn: Union[str, Path] = None,
    output_func=None,
):
    """
    classic segmentation workflow wrapper tempalte

    Parameter:
    -----------
    struct_img: np.ndarray
        the 3D image to be segmented
    rescale_ratio: float
        an optional parameter to allow rescale the image before running the
        segmentation functions, default is no rescaling
    output_type: str
        select how to handle output. Currently, four types are supported:
        1. default: the result will be saved at output_path whose filename is
            original name without extention + "_struct_segmentaiton.tiff"
        2. array: the segmentation result will be simply returned as a numpy array
        3. array_with_contour: segmentation result will be returned together with
            the contour of the segmentation
        4. customize: pass in an extra output_func to do a special save. All the
            intermediate results, names of these results, the output_path, and the
            original filename (without extension) will be passed in to output_func.
    """

    ##########################################################################
    # PARAMETERS:

    ##########################################################################

    ###################
    # PRE_PROCESSING
    # make sure the variable name of original image is 'struct_img'
    ###################
    # intenisty normalization

    # smoothing

    ###################
    # core algorithm
    ###################

    ###################
    # POST-PROCESSING
    # make sure the variable name of final segmentation is 'seg'
    ###################

    ##########################################################################
    # ## no need to change below
    ##########################################################################
    # output
    seg = struct_img
    seg = seg > 0
    seg = seg.astype(np.uint8)
    seg[seg > 0] = 255

    if output_type == "default":
        # the default final output
        save_segmentation(seg, False, output_path, fn)
    elif output_type == "array":
        return seg
    elif output_type == "array_with_contour":
        return (seg, generate_segmentation_contour(seg))
    else:
        print("your can implement your output hook here, but not yet")
        quit()
