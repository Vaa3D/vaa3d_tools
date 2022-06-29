from typing import Union
from pathlib import Path
import numpy as np
from skimage.morphology import remove_small_objects, watershed, dilation, ball
from skimage.feature import peak_local_max
from skimage.measure import label
from aicssegmentation.core.pre_processing_utils import (
    intensity_normalization,
    image_smoothing_gaussian_slice_by_slice,
)
from aicssegmentation.core.seg_dot import dot_3d
from aicssegmentation.core.output_utils import (
    save_segmentation,
    generate_segmentation_contour,
)
from aicssegmentation.core.utils import remove_hot_pixel
from scipy.ndimage import distance_transform_edt
from scipy.ndimage import zoom


def Workflow_polr2a_blob(
    struct_img: np.ndarray,
    rescale_ratio: float = -1,
    output_type: str = "default",
    output_path: Union[str, Path] = None,
    fn: Union[str, Path] = None,
    output_func=None,
):
    """
    classic segmentation workflow wrapper for structure polr2a large condensates

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
            original name without extension + "_struct_segmentation.tiff"
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

    intensity_norm_param = [1000]
    gaussian_smoothing_sigma = 1
    gaussian_smoothing_truncate_range = 3.0
    dot_3d_sigma = 1
    dot_3d_cutoff = 0.030
    min_area = 4
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
    structure_img_smooth = image_smoothing_gaussian_slice_by_slice(
        struct_img,
        sigma=gaussian_smoothing_sigma,
        truncate_range=gaussian_smoothing_truncate_range,
    )

    out_img_list.append(structure_img_smooth.copy())
    out_name_list.append("im_smooth")

    ###################
    # core algorithm
    ###################

    # step 1: LOG 3d
    response = dot_3d(structure_img_smooth, log_sigma=dot_3d_sigma)
    bw = response > dot_3d_cutoff
    bw = remove_small_objects(bw > 0, min_size=min_area, connectivity=1, in_place=False)

    out_img_list.append(bw.copy())
    out_name_list.append("interm_mask")

    # step 2: 'local_maxi + watershed' for splitting objects
    local_maxi = peak_local_max(struct_img, labels=label(bw), min_distance=2, indices=False)

    out_img_list.append(local_maxi.copy())
    out_name_list.append("interm_local_max")

    distance = distance_transform_edt(bw)
    im_watershed = watershed(
        -distance,
        label(dilation(local_maxi, selem=ball(1))),
        mask=bw,
        watershed_line=True,
    )

    ###################
    # POST-PROCESSING
    ###################
    seg = remove_small_objects(im_watershed, min_size=min_area, connectivity=1, in_place=False)

    # remove hot pixels from segmentation output
    seg = seg > 0
    seg_clean = remove_hot_pixel(seg)
    seg = seg_clean.astype(np.uint8)
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
