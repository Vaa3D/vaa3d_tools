import numpy as np
from typing import Union
from pathlib import Path
from skimage.morphology import remove_small_objects
from aicssegmentation.core.pre_processing_utils import (
    intensity_normalization,
    image_smoothing_gaussian_3d,
)
from aicssegmentation.core.seg_dot import dot_2d_slice_by_slice_wrapper
from skimage.filters import threshold_triangle, threshold_otsu
from skimage.measure import label

# do not remove ####
from aicssegmentation.core.output_utils import (
    save_segmentation,
    generate_segmentation_contour,
)


def Workflow_fbl_labelfree_4dn(
    struct_img: np.ndarray,
    rescale_ratio: float = -1,
    output_type: str = "default",
    output_path: Union[str, Path] = None,
    fn: Union[str, Path] = None,
    output_func=None,
):
    """
    classic segmentation workflow wrapper for structure FBL Labelfree 4dn

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
    minArea = 5
    low_level_min_size = 7000
    s2_param = [[0.5, 0.1]]
    intensity_scaling_param = [0.5, 19.5]
    gaussian_smoothing_sigma = 1

    ##########################################################################
    out_img_list = []
    out_name_list = []

    ###################
    # PRE_PROCESSING
    ###################
    # intenisty normalization
    struct_norm = intensity_normalization(struct_img, scaling_param=intensity_scaling_param)
    out_img_list.append(struct_img.copy())
    out_name_list.append("im_norm")

    # smoothing
    struct_smooth = image_smoothing_gaussian_3d(struct_norm, sigma=gaussian_smoothing_sigma)

    out_img_list.append(struct_smooth.copy())
    out_name_list.append("im_smooth")

    ###################
    # core algorithm
    ###################
    # step 1: low level thresholding
    # global_otsu = threshold_otsu(structure_img_smooth)
    global_tri = threshold_triangle(struct_smooth)
    global_median = np.percentile(struct_smooth, 50)

    th_low_level = (global_tri + global_median) / 2
    bw_low_level = struct_smooth > th_low_level
    bw_low_level = remove_small_objects(bw_low_level, min_size=low_level_min_size, connectivity=1, in_place=True)

    # step 2: high level thresholding
    bw_high_level = np.zeros_like(bw_low_level)
    lab_low, num_obj = label(bw_low_level, return_num=True, connectivity=1)
    for idx in range(num_obj):
        single_obj = lab_low == (idx + 1)
        local_otsu = threshold_otsu(struct_smooth[single_obj])
        bw_high_level[np.logical_and(struct_smooth > local_otsu * 1.2, single_obj)] = 1

    # step 3: finer segmentation
    response2d = dot_2d_slice_by_slice_wrapper(struct_smooth, s2_param)
    bw_finer = remove_small_objects(response2d, min_size=minArea, connectivity=1, in_place=True)

    # merge finer level detection into high level coarse segmentation
    # to include outside dim parts
    bw_high_level[bw_finer > 0] = 1

    ###################
    # POST-PROCESSING
    # make sure the variable name of final segmentation is 'seg'
    ###################
    seg = remove_small_objects(bw_high_level, min_size=minArea, connectivity=1, in_place=True)

    # output
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
        # TODO CREATE OUT_IMG_LIST/OUT_NAMELIST?
        output_func(out_img_list, out_name_list, Path(output_path), fn)
    elif output_type == "array":
        return seg
    elif output_type == "array_with_contour":
        return (seg, generate_segmentation_contour(seg))
    else:
        raise NotImplementedError("invalid output type: {output_type}")
