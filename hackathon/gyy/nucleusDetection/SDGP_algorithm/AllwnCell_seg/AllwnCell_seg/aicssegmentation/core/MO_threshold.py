import numpy as np
from skimage.morphology import remove_small_objects, ball, dilation
from skimage.filters import threshold_triangle, threshold_otsu
from skimage.measure import label


def MO_low_level(
    structure_img_smooth: np.ndarray,
    global_thresh_method: str,
    object_minArea: int,
    dilate: bool = False,
) -> np.ndarray:
    """
    Implementation of "Masked Object Thresholding" algorithm. Specifically, the
    algorithm is a hybrid thresholding method combining two levels of thresholds.
    The steps are [1] a global threshold is calculated, [2] extract each individual
    connected componet after applying the global threshold, [3] remove small objects,
    [4] within each remaining object, a local Otsu threshold is calculated and applied
    with an optional local threshold adjustment ratio (to make the segmentation more
    and less conservative). An extra check can be used in step [4], which requires the
    local Otsu threshold larger than 1/3 of global Otsu threhsold and otherwise this
    connected component is discarded. This function implements the low level part.

    Parameters:
    --------------
    structure_img_smooth: np.ndarray
        the image (should have already been smoothed) to apply the method on
    global_thresh_method: str
        which method to use for calculating global threshold. Options include:
        "triangle" (or "tri"), "median" (or "med"), and "ave_tri_med" (or "ave").
        "ave" refers the average of "triangle" threshold and "mean" threshold.
    object_minArea: int
        the size filter for excluding small object before applying local threshold
    dilate: bool
        whether to perform dilation on bw_low_level prior to the high level threshold

    Return:
    --------------
    a binary nd array of the segmentation result
    """

    if global_thresh_method == "tri" or global_thresh_method == "triangle":
        th_low_level = threshold_triangle(structure_img_smooth)
    elif global_thresh_method == "med" or global_thresh_method == "median":
        th_low_level = np.percentile(structure_img_smooth, 50)
    elif global_thresh_method == "ave" or global_thresh_method == "ave_tri_med":
        global_tri = threshold_triangle(structure_img_smooth)
        global_median = np.percentile(structure_img_smooth, 50)
        th_low_level = (global_tri + global_median) / 2

    bw_low_level = structure_img_smooth > th_low_level
    bw_low_level = remove_small_objects(bw_low_level, min_size=object_minArea, connectivity=1, in_place=True)
    if dilate:
        bw_low_level = dilation(bw_low_level, selem=ball(2))

    return bw_low_level


def MO_high_level(
    structure_img_smooth: np.ndarray,
    bw_low_level: np.ndarray,
    extra_criteria: bool = False,
    local_adjust: float = 0.98,
) -> np.ndarray:
    """
    Implementation of "Masked Object Thresholding" algorithm. Specifically, the
    algorithm is a hybrid thresholding method combining two levels of thresholds.
    The steps are [1] a global threshold is calculated, [2] extract each individual
    connected componet after applying the global threshold, [3] remove small objects,
    [4] within each remaining object, a local Otsu threshold is calculated and applied
    with an optional local threshold adjustment ratio (to make the segmentation more
    and less conservative). An extra check can be used in step [4], which requires the
    local Otsu threshold larger than 1/3 of global Otsu threhsold and otherwise this
    connected component is discarded. This function implements the high level part.

    Parameters:
    --------------
    structure_img_smooth: np.ndarray
        the image (should have already been smoothed) to apply the method on
    bw_low_level: np.ndarray
        low level segmentation
    extra_criteria: bool
        whether to use the extra check when doing local thresholding, default is False
    local_adjust: float
        a ratio to apply on local threshold, default is 0.98

    Return:
    --------------
    a binary nd array of the segmentation result
    """

    bw_high_level = np.zeros_like(bw_low_level)
    lab_low, num_obj = label(bw_low_level, return_num=True, connectivity=1)
    if extra_criteria:
        local_cutoff = 0.333 * threshold_otsu(structure_img_smooth)
        for idx in range(num_obj):
            single_obj = lab_low == (idx + 1)
            local_otsu = threshold_otsu(structure_img_smooth[single_obj > 0])
            if local_otsu > local_cutoff:
                bw_high_level[np.logical_and(structure_img_smooth > local_otsu * local_adjust, single_obj)] = 1
    else:
        for idx in range(num_obj):
            single_obj = lab_low == (idx + 1)
            local_otsu = threshold_otsu(structure_img_smooth[single_obj > 0])
            bw_high_level[np.logical_and(structure_img_smooth > local_otsu * local_adjust, single_obj)] = 1

    return bw_high_level > 0


def MO(
    structure_img_smooth: np.ndarray,
    global_thresh_method: str,
    object_minArea: int,
    extra_criteria: bool = False,
    local_adjust: float = 0.98,
    return_object: bool = False,
    dilate: bool = False,
):
    """
    Implementation of "Masked Object Thresholding" algorithm. Specifically, the
    algorithm is a hybrid thresholding method combining two levels of thresholds.
    The steps are [1] a global threshold is calculated, [2] extract each individual
    connected componet after applying the global threshold, [3] remove small objects,
    [4] within each remaining object, a local Otsu threshold is calculated and applied
    with an optional local threshold adjustment ratio (to make the segmentation more
    and less conservative). An extra check can be used in step [4], which requires the
    local Otsu threshold larger than 1/3 of global Otsu threhsold and otherwise this
    connected component is discarded.

    Parameters:
    --------------
    structure_img_smooth: np.ndarray
        the image (should have already been smoothed) to apply the method on
    global_thresh_method: str
        which method to use for calculating global threshold. Options include:
        "triangle" (or "tri"), "median" (or "med"), and "ave_tri_med" (or "ave").
        "ave" refers the average of "triangle" threshold and "mean" threshold.
    object_minArea: int
        the size filter for excluding small object before applying local threshold
    extra_criteria: bool
        whether to use the extra check when doing local thresholding, default is False
    local_adjust: float
        a ratio to apply on local threshold, default is 0.98
    return_object: bool
        whether to return the global thresholding results in order to obtain the
        individual objects the local thresholding is made on
    dilate: bool
        whether to perform dilation on bw_low_level prior to the high level threshold

    Return:
    --------------
    a binary nd array of the segmentation result
    """

    bw_low_level = MO_low_level(structure_img_smooth, global_thresh_method, object_minArea, dilate)

    bw_high_level = MO_high_level(structure_img_smooth, bw_low_level, extra_criteria, local_adjust)

    if return_object:
        return bw_high_level, bw_low_level
    else:
        return bw_high_level
