import numpy as np
from typing import Union
from pathlib import Path
from skimage.morphology import remove_small_objects
from skimage.measure import label
from aicssegmentation.core.vessel import vesselnessSliceBySlice
from aicssegmentation.core.seg_dot import dot_slice_by_slice
from aicssegmentation.core.pre_processing_utils import (
    intensity_normalization,
    image_smoothing_gaussian_slice_by_slice,
)
from aicssegmentation.core.output_utils import (
    save_segmentation,
    generate_segmentation_contour,
)
from scipy.ndimage import zoom


def Workflow_lamp1(
    struct_img: np.ndarray,
    rescale_ratio: float = -1,
    output_type: str = "default",
    output_path: Union[str, Path] = None,
    fn: Union[str, Path] = None,
    output_func=None,
):
    """
    classic segmentation workflow wrapper for structure LAMP1

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
    intensity_scaling_param = [3, 19]
    gaussian_smoothing_sigma = 1
    gaussian_smoothing_truncate_range = 3.0

    minArea = 15
    # ves_th_2d = 0.1

    vesselness_sigma = [1]
    vesselness_cutoff = 0.15

    # hole_min = 60
    hole_max = 1600

    log_sigma_1 = 5
    log_cutoff_1 = 0.09
    log_sigma_2 = 2.5
    log_cutoff_2 = 0.07
    log_sigma_3 = 1
    log_cutoff_3 = 0.01
    ##########################################################################

    out_img_list = []
    out_name_list = []

    # intenisty normalization
    struct_img = intensity_normalization(struct_img, scaling_param=intensity_scaling_param)

    out_img_list.append(struct_img.copy())
    out_name_list.append("im_norm")

    if rescale_ratio > 0:
        struct_img = zoom(struct_img, (1, rescale_ratio, rescale_ratio), order=2)

        struct_img = (struct_img - struct_img.min() + 1e-8) / (struct_img.max() - struct_img.min() + 1e-8)
        gaussian_smoothing_truncate_range = gaussian_smoothing_truncate_range * rescale_ratio

    structure_img_smooth = image_smoothing_gaussian_slice_by_slice(
        struct_img,
        sigma=gaussian_smoothing_sigma,
        truncate_range=gaussian_smoothing_truncate_range,
    )

    out_img_list.append(structure_img_smooth.copy())
    out_name_list.append("im_smooth")

    # spot detection
    response1 = dot_slice_by_slice(structure_img_smooth, log_sigma=log_sigma_1)
    bw1 = response1 > log_cutoff_1
    response2 = dot_slice_by_slice(structure_img_smooth, log_sigma=log_sigma_2)
    bw2 = response2 > log_cutoff_2
    bw_spot = np.logical_or(bw1, bw2)
    response3 = dot_slice_by_slice(structure_img_smooth, log_sigma=log_sigma_3)
    bw3 = response3 > log_cutoff_3
    bw_spot = np.logical_or(bw_spot, bw3)

    # ring/filament detection
    ves = vesselnessSliceBySlice(structure_img_smooth, sigmas=vesselness_sigma, tau=1, whiteonblack=True)
    bw_ves = ves > vesselness_cutoff

    # fill holes
    partial_fill = np.logical_or(bw_spot, bw_ves)

    out_img_list.append(partial_fill.copy())
    out_name_list.append("interm_before_hole")

    holes = np.zeros_like(partial_fill)
    for zz in range(partial_fill.shape[0]):
        background_lab = label(~partial_fill[zz, :, :], connectivity=1)

        out = np.copy(background_lab)
        component_sizes = np.bincount(background_lab.ravel())
        too_big = component_sizes > hole_max
        too_big_mask = too_big[background_lab]
        out[too_big_mask] = 0
        # too_small = component_sizes <hole_min
        # too_small_mask = too_small[background_lab]
        # out[too_small_mask] = 0

        holes[zz, :, :] = out

    full_fill = np.logical_or(partial_fill, holes)

    seg = remove_small_objects(full_fill, min_size=minArea, connectivity=1, in_place=False)

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
        output_func(out_img_list, out_name_list, Path(output_path), fn)
    elif output_type == "array":
        return seg
    elif output_type == "array_with_contour":
        return (seg, generate_segmentation_contour(seg))
    else:
        raise NotImplementedError("invalid output type: {output_type}")
