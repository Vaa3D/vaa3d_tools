import numpy as np
from typing import Union
from pathlib import Path
from aicssegmentation.core.vessel import vesselness3D
from aicssegmentation.core.pre_processing_utils import (
    intensity_normalization,
    edge_preserving_smoothing_3d,
)
from skimage.morphology import remove_small_objects
from aicssegmentation.core.output_utils import (
    save_segmentation,
    generate_segmentation_contour,
)
from scipy.ndimage import zoom


def Workflow_pxn(
    struct_img: np.ndarray,
    rescale_ratio: float = -1,
    output_type: str = "default",
    output_path: Union[str, Path] = None,
    fn: Union[str, Path] = None,
    output_func=None,
):
    """
    classic segmentation workflow wrapper for structure PXN

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
    ##########################################################################
    # PARAMETERS:
    #   note that these parameters are supposed to be fixed for the structure
    #   and work well accross different datasets

    intensity_norm_param = [11, 8]
    vesselness_sigma = [1]
    vesselness_cutoff = 0.35
    minArea3D = 15
    minArea2D = 4
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
    structure_img_smooth = edge_preserving_smoothing_3d(struct_img)

    out_img_list.append(structure_img_smooth.copy())
    out_name_list.append("im_smooth")

    ###################
    # core algorithm
    ###################

    # vesselness 3d
    response = vesselness3D(structure_img_smooth, sigmas=vesselness_sigma, tau=1, whiteonblack=True)
    bw = response > vesselness_cutoff

    ###################
    # POST-PROCESSING
    ###################

    seg = np.zeros_like(bw)
    for zz in range(bw.shape[0]):
        seg[zz, :, :] = remove_small_objects(bw[zz, :, :] > 0, min_size=minArea2D, connectivity=1, in_place=False)

    seg = remove_small_objects(seg > 0, min_size=minArea3D, connectivity=1, in_place=False)

    # determine z-range
    bw_z = np.zeros(bw.shape[0], dtype=np.uint16)
    for zz in range(bw.shape[0]):
        bw_z[zz] = np.count_nonzero(seg[zz, :, :] > 0)

    mid_z = np.argmax(bw_z)
    low_z = 0
    high_z = seg.shape[0] - 2
    for ii in np.arange(mid_z - 1, 0, -1):
        if bw_z[ii] < 100:
            low_z = ii
            break
    for ii in range(mid_z + 1, bw.shape[0] - 1, 1):
        if bw_z[ii] < 100:
            high_z = ii
            break

    seg[:low_z, :, :] = 0
    seg[high_z + 1 :, :, :] = 0

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
