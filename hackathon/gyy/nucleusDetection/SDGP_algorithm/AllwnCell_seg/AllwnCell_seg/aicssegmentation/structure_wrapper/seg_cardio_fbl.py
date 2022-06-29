import numpy as np
from typing import Union
from pathlib import Path
from skimage.morphology import remove_small_objects
from aicssegmentation.core.pre_processing_utils import (
    intensity_normalization,
    image_smoothing_gaussian_3d,
)
from aicssegmentation.core.seg_dot import dot_slice_by_slice
from skimage.filters import threshold_triangle, threshold_otsu
from skimage.measure import label
from aicssegmentation.core.output_utils import (
    save_segmentation,
    generate_segmentation_contour,
)
from scipy.ndimage import zoom


def Workflow_cardio_fbl(
    struct_img: np.ndarray,
    rescale_ratio: float = -1,
    output_type: str = "default",
    output_path: Union[str, Path] = None,
    fn: Union[str, Path] = None,
    output_func=None,
):
    """
    classic segmentation workflow wrapper for structure Cardio FBL

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

    intensity_norm_param = [0.5, 140]
    gaussian_smoothing_sigma = 1
    gaussian_smoothing_truncate_range = 3.0
    dot_2d_sigma = 1
    dot_2d_cutoff = 0.015
    minArea = 1
    low_level_min_size = 1000
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
        gaussian_smoothing_truncate_range = gaussian_smoothing_truncate_range * rescale_ratio

    # smoothing with gaussian filter
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

    # step 1: low level thresholding
    # global_otsu = threshold_otsu(structure_img_smooth)
    global_tri = threshold_triangle(structure_img_smooth)
    global_median = np.percentile(structure_img_smooth, 50)

    th_low_level = (global_tri + global_median) / 2
    bw_low_level = structure_img_smooth > th_low_level
    bw_low_level = remove_small_objects(bw_low_level, min_size=low_level_min_size, connectivity=1, in_place=True)

    # step 2: high level thresholding
    local_cutoff = 0.333 * threshold_otsu(structure_img_smooth)
    bw_high_level = np.zeros_like(bw_low_level)
    lab_low, num_obj = label(bw_low_level, return_num=True, connectivity=1)
    for idx in range(num_obj):
        single_obj = lab_low == (idx + 1)
        local_otsu = threshold_otsu(structure_img_smooth[single_obj])
        if local_otsu > local_cutoff:
            bw_high_level[np.logical_and(structure_img_smooth > local_otsu, single_obj)] = 1

    out_img_list.append(bw_high_level.copy())
    out_name_list.append("interm_high")

    # step 3: finer segmentation
    response2d = dot_slice_by_slice(structure_img_smooth, log_sigma=dot_2d_sigma)

    bw_finer = remove_small_objects(response2d > dot_2d_cutoff, min_size=minArea, connectivity=1, in_place=True)

    out_img_list.append(bw_finer.copy())
    out_name_list.append("bw_fine")

    # merge finer level detection into high level coarse segmentation to include
    # outside dim parts
    bw_high_level[bw_finer > 0] = 1

    ###################
    # POST-PROCESSING
    ###################
    seg = remove_small_objects(bw_high_level > 0, min_size=minArea, connectivity=1, in_place=False)

    # output
    seg = seg > 0
    seg = seg.astype(np.uint8)
    seg[seg > 0] = 255

    out_img_list.append(seg.copy())
    out_name_list.append("bw_coarse")

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
