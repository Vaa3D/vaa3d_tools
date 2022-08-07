import numpy as np
from typing import Union
from pathlib import Path

from aicssegmentation.core.vessel import vesselness3D
from aicssegmentation.core.seg_dot import dot_3d
from aicssegmentation.core.pre_processing_utils import (
    intensity_normalization,
    edge_preserving_smoothing_3d,
)
from skimage.morphology import remove_small_objects
from aicssegmentation.core.output_utils import (
    save_segmentation,
    generate_segmentation_contour,
)


def Workflow_son(
    struct_img: np.ndarray,
    rescale_ratio: float = -1,
    output_type: str = "default",
    output_path: Union[str, Path] = None,
    fn: Union[str, Path] = None,
    output_func=None,
):
    """
    classic segmentation workflow wrapper for structure SON

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
    ##########################################################################

    intensity_norm_param = [2, 30]
    vesselness_sigma = [1.2]
    vesselness_cutoff = 0.15
    minArea = 15

    # dot_2d_sigma = 1
    dot_3d_sigma = 1.15
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

    # smoothing with boundary preserving smoothing
    structure_img_smooth = edge_preserving_smoothing_3d(struct_img)

    out_img_list.append(structure_img_smooth.copy())
    out_name_list.append("im_smooth")

    ###################
    # core algorithm
    ###################
    response_f3 = vesselness3D(structure_img_smooth, sigmas=vesselness_sigma, tau=1, whiteonblack=True)
    response_f3 = response_f3 > vesselness_cutoff

    response_s3_1 = dot_3d(structure_img_smooth, log_sigma=dot_3d_sigma)
    response_s3_3 = dot_3d(structure_img_smooth, log_sigma=3)

    bw_small_inverse = remove_small_objects(response_s3_1 > 0.03, min_size=150)
    bw_small = np.logical_xor(bw_small_inverse, response_s3_1 > 0.02)

    bw_medium = np.logical_or(bw_small, response_s3_1 > 0.07)
    bw_large = np.logical_or(response_s3_3 > 0.2, response_f3 > 0.25)
    bw = np.logical_or(np.logical_or(bw_small, bw_medium), bw_large)

    ###################
    # POST-PROCESSING
    ###################
    bw = remove_small_objects(bw > 0, min_size=minArea, connectivity=1, in_place=False)
    for zz in range(bw.shape[0]):
        bw[zz, :, :] = remove_small_objects(bw[zz, :, :], min_size=3, connectivity=1, in_place=False)

    seg = remove_small_objects(bw > 0, min_size=minArea, connectivity=1, in_place=False)

    seg = seg > 0
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
