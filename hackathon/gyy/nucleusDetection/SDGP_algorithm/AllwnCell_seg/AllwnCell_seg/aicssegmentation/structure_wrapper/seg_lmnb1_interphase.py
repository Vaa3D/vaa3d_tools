import numpy as np
from typing import Union
from pathlib import Path

# function for core algorithm
from aicssegmentation.core.vessel import filament_2d_wrapper
from aicssegmentation.core.pre_processing_utils import (
    intensity_normalization,
    image_smoothing_gaussian_3d,
)
from aicssegmentation.core.utils import (
    get_middle_frame,
    hole_filling,
    get_3dseed_from_mid_frame,
)
from skimage.segmentation import find_boundaries, watershed
from aicssegmentation.core.output_utils import (
    save_segmentation,
    generate_segmentation_contour,
)
from scipy.ndimage import zoom


def Workflow_lmnb1_interphase(
    struct_img: np.ndarray,
    rescale_ratio: float = -1,
    output_type: str = "default",
    output_path: Union[str, Path] = None,
    fn: Union[str, Path] = None,
    output_func=None,
):
    """
    classic segmentation workflow wrapper for structure LMNB1 interphase

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
    #   note that these parameters are supposed to be fixed for the structure
    #   and work well accross different datasets

    intensity_norm_param = [4000]
    gaussian_smoothing_sigma = 1
    gaussian_smoothing_truncate_range = 3.0
    f2_param = [[1, 0.01], [2, 0.01], [3, 0.01]]
    hole_max = 40000
    hole_min = 400
    ##########################################################################

    out_img_list = []
    out_name_list = []

    ###################
    # PRE_PROCESSING
    ###################
    # intenisty normalization (min/max)
    struct_img = intensity_normalization(struct_img, scaling_param=intensity_norm_param)

    out_img_list.append(struct_img.copy())
    out_name_list.append("im_norm")

    # rescale if needed
    if rescale_ratio > 0:
        struct_img = zoom(struct_img, (1, rescale_ratio, rescale_ratio), order=2)

        struct_img = (struct_img - struct_img.min() + 1e-8) / (struct_img.max() - struct_img.min() + 1e-8)

    # smoothing with boundary preserving smoothing
    structure_img_smooth = image_smoothing_gaussian_3d(
        struct_img,
        sigma=gaussian_smoothing_sigma,
        truncate_range=gaussian_smoothing_truncate_range,
    )

    out_img_list.append(structure_img_smooth.copy())
    out_name_list.append("im_smooth")

    ###################
    # core algorithm
    ###################

    # get the mid frame
    mid_z = get_middle_frame(structure_img_smooth, method="intensity")

    # 2d filament filter on the middle frame
    bw_mid_z = filament_2d_wrapper(structure_img_smooth[mid_z, :, :], f2_param)

    # hole filling
    bw_fill_mid_z = hole_filling(bw_mid_z, hole_min, hole_max)
    seed = get_3dseed_from_mid_frame(
        np.logical_xor(bw_fill_mid_z, bw_mid_z),
        struct_img.shape,
        mid_z,
        hole_min,
        bg_seed=True,
    )
    seg_filled = (
        watershed(struct_img, seed.astype(int), watershed_line=True) > 1
    )  # in watershed result, 1 is for background

    # get the actual shell
    seg = find_boundaries(seg_filled, connectivity=1, mode="thick")

    # output
    seg = seg.astype(np.uint8)
    seg[seg > 0] = 255

    out_img_list.append(seg.copy())
    out_name_list.append("bw_final")

    if output_type == "default":
        # the default final output, simply save it to the output path
        save_segmentation(seg, False, Path(output_path), fn)
    elif output_type == "customize":
        # the hook for passing in a customized output function
        # use "out_img_list" and "out_name_list" in your hook to
        # customize your output functions
        output_func(out_img_list, out_name_list, Path(output_path), fn)
    elif output_type == "array":
        return seg
    elif output_type == "array_with_contour":
        return (seg, generate_segmentation_contour(seg))
    else:
        raise NotImplementedError("invalid output type: {output_type}")
