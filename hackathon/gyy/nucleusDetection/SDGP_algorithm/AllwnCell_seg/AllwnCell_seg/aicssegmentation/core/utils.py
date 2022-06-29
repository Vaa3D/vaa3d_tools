from typing import List

import numpy as np
from scipy.ndimage import distance_transform_edt
from skimage.measure import label, regionprops
from skimage.morphology import ball, disk, dilation, erosion, medial_axis, remove_small_objects


def hole_filling(bw: np.ndarray, hole_min: int, hole_max: int, fill_2d: bool = True) -> np.ndarray:
    """Fill holes in 2D/3D segmentation

    Parameters:
    -------------
    bw: np.ndarray
        a binary 2D/3D image.
    hole_min: int
        the minimum size of the holes to be filled
    hole_max: int
        the maximum size of the holes to be filled
    fill_2d: bool
        if fill_2d=True, a 3D image will be filled slice by slice.
        If you think of a hollow tube alone z direction, the inside
        is not a hole under 3D topology, but the inside on each slice
        is indeed a hole under 2D topology.

    Return:
        a binary image after hole filling
    """
    bw = bw > 0
    if len(bw.shape) == 2:
        background_lab = label(~bw, connectivity=1)
        fill_out = np.copy(background_lab)
        component_sizes = np.bincount(background_lab.ravel())
        too_big = component_sizes > hole_max
        too_big_mask = too_big[background_lab]
        fill_out[too_big_mask] = 0
        too_small = component_sizes < hole_min
        too_small_mask = too_small[background_lab]
        fill_out[too_small_mask] = 0
    elif len(bw.shape) == 3:
        if fill_2d:
            fill_out = np.zeros_like(bw)
            for zz in range(bw.shape[0]):
                background_lab = label(~bw[zz, :, :], connectivity=1)
                out = np.copy(background_lab)
                component_sizes = np.bincount(background_lab.ravel())
                too_big = component_sizes > hole_max
                too_big_mask = too_big[background_lab]
                out[too_big_mask] = 0
                too_small = component_sizes < hole_min
                too_small_mask = too_small[background_lab]
                out[too_small_mask] = 0
                fill_out[zz, :, :] = out
        else:
            background_lab = label(~bw, connectivity=1)
            fill_out = np.copy(background_lab)
            component_sizes = np.bincount(background_lab.ravel())
            too_big = component_sizes > hole_max
            too_big_mask = too_big[background_lab]
            fill_out[too_big_mask] = 0
            too_small = component_sizes < hole_min
            too_small_mask = too_small[background_lab]
            fill_out[too_small_mask] = 0
    else:
        print("error in image shape")
        return

    return np.logical_or(bw, fill_out)


def size_filter(img: np.ndarray, min_size: int, method: str = "3D", connectivity: int = 1):
    """size filter

    Parameters:
    ------------
    img: np.ndarray
        the image to filter on
    min_size: int
        the minimum size to keep
    method: str
        either "3D" or "slice_by_slice", default is "3D"
    connnectivity: int
        the connectivity to use when computing object size
    """
    assert len(img.shape) == 3, "image has to be 3D"
    if method == "3D":
        return remove_small_objects(img > 0, min_size=min_size, connectivity=connectivity, in_place=False)
    elif method == "slice_by_slice":
        seg = np.zeros(img.shape, dtype=bool)
        for zz in range(img.shape[0]):
            seg[zz, :, :] = remove_small_objects(
                img[zz, :, :] > 0,
                min_size=min_size,
                connectivity=connectivity,
                in_place=False,
            )
        return seg
    else:
        raise NotImplementedError(f"unsupported method {method}")


def topology_preserving_thinning(bw: np.ndarray, min_thickness: int = 1, thin: int = 1) -> np.ndarray:
    """perform thinning on segmentation without breaking topology

    Parameters:
    --------------
    bw: np.ndarray
        the 3D binary image to be thinned
    min_thinkness: int
        Half of the minimum width you want to keep from being thinned.
        For example, when the object width is smaller than 4, you don't
        want to make this part even thinner (may break the thin object
        and alter the topology), you can set this value as 2.
    thin: int
        the amount to thin (has to be an positive integer). The number of
         pixels to be removed from outter boundary towards center.

    Return:
    -------------
        A binary image after thinning
    """
    bw = bw > 0
    safe_zone = np.zeros_like(bw)
    for zz in range(bw.shape[0]):
        if np.any(bw[zz, :, :]):
            ctl = medial_axis(bw[zz, :, :] > 0)
            dist = distance_transform_edt(ctl == 0)
            safe_zone[zz, :, :] = dist > min_thickness + 1e-5

    rm_candidate = np.logical_xor(bw > 0, erosion(bw > 0, ball(thin)))

    bw[np.logical_and(safe_zone, rm_candidate)] = 0

    return bw


def divide_nonzero(array1, array2):
    """
    Divides two arrays. Returns zero when dividing by zero.
    """
    denominator = np.copy(array2)
    denominator[denominator == 0] = 1e-10
    return np.divide(array1, denominator)


def histogram_otsu(hist):
    """Apply Otsu thresholding method on 1D histogram"""

    # modify the elements in hist to avoid completely zero value in cumsum
    hist = hist + 1e-5

    bin_size = 1 / (len(hist) - 1)
    bin_centers = np.arange(0, 1 + 0.5 * bin_size, bin_size)
    hist = hist.astype(float)

    # class probabilities for all possible thresholds
    weight1 = np.cumsum(hist)
    weight2 = np.cumsum(hist[::-1])[::-1]
    # class means for all possible thresholds

    mean1 = np.cumsum(hist * bin_centers) / weight1
    mean2 = (np.cumsum((hist * bin_centers)[::-1]) / weight2[::-1])[::-1]

    # Clip ends to align class 1 and class 2 variables:
    # The last value of `weight1`/`mean1` should pair with zero values in
    # `weight2`/`mean2`, which do not exist.
    variance12 = weight1[:-1] * weight2[1:] * (mean1[:-1] - mean2[1:]) ** 2

    idx = np.argmax(variance12)
    threshold = bin_centers[:-1][idx]
    return threshold


def absolute_eigenvaluesh(nd_array):
    """Computes the eigenvalues sorted by absolute value from the symmetrical matrix.

    Parameters:
    -------------
    nd_array: nd.ndarray
        array from which the eigenvalues will be calculated.

    Return:
    -------------
        A list with the eigenvalues sorted in absolute ascending order (e.g.
        [eigenvalue1, eigenvalue2, ...])
    """
    eigenvalues = np.linalg.eigvalsh(nd_array)
    sorted_eigenvalues = sortbyabs(eigenvalues, axis=-1)
    return [
        np.squeeze(eigenvalue, axis=-1)
        for eigenvalue in np.split(sorted_eigenvalues, sorted_eigenvalues.shape[-1], axis=-1)
    ]


def sortbyabs(a, axis=0):
    """Sort array along a given axis by the absolute value
    modified from: http://stackoverflow.com/a/11253931/4067734
    """
    index = list(np.ix_(*[np.arange(i) for i in a.shape]))
    index[axis] = np.abs(a).argsort(axis)
    return a[index]


def get_middle_frame(struct_img: np.ndarray, method: str = "z") -> int:
    """find the middle z frame of an image stack

    Parameters:
    ------------
    struct_img: np.ndarray
        the 3D image to process
    method: str
        which method to use to determine the middle frame. Options
        are "z" or "intensity". "z" is solely based on the number of z
        frames. "intensity" method uses Otsu threshod to estimate the
        volume of foreground signals in the stack, then estimated volume
        of each z frame forms a z-profile, and finally another Otsu
        method is apply on the z profile to find the best z frame (with
        an assumption of two peaks along z profile, one near the bottom
        of the cells and one near the bottom of the cells, so the optimal
        separation is the middle of the stack).

    Return:
    -----------
    mid_frame: int
        the z index of the middle z frame
    """

    from skimage.filters import threshold_otsu

    if method == "intensity":
        bw = struct_img > threshold_otsu(struct_img)
        z_profile = np.zeros((bw.shape[0],), dtype=int)
        for zz in range(bw.shape[0]):
            z_profile[zz] = np.count_nonzero(bw[zz, :, :])
        mid_frame = None
        if isinstance(round(histogram_otsu(z_profile) * bw.shape[0]), int):
            mid_frame = round(histogram_otsu(z_profile) * bw.shape[0])
        else:
            mid_frame = round(histogram_otsu(z_profile) * bw.shape[0]).astype(int)

    elif method == "z":
        mid_frame = struct_img.shape[0] // 2

    else:
        print("unsupported method")
        quit()

    return mid_frame


def get_3dseed_from_mid_frame(
    bw: np.ndarray,
    stack_shape: List = None,
    mid_frame: int = -1,
    hole_min: int = 1,
    bg_seed: bool = True,
) -> np.ndarray:
    """build a 3D seed image from the binary segmentation of a single slice

    Parameters:
    ------------
    bw: np.ndarray
        the 2d segmentation of a single frame, or a 3D array with only one slice
        containing segmentation
    stack_shape: List
        (only used when bw is 2d) the shape of original 3d image, e.g.
        shape_3d = img.shape
    frame_index: int
        (only used when bw is 2d) the index of where bw is from the whole z-stack
    hole_min: int
        any connected component in bw2d with size smaller than area_min
        will be excluded from seed image generation
    bg_seed: bool
        bg_seed=True will add a background seed at the first frame (z=0).

    """
    from skimage.morphology import remove_small_objects

    out = remove_small_objects(bw > 0, hole_min)

    out1 = label(out)
    stat = regionprops(out1)

    # build the seed
    seed = np.zeros(stack_shape)
    seed_count = 0
    if bg_seed:
        seed[0, :, :] = 1
        seed_count += 1

    for idx in range(len(stat)):
        py, px = np.round(stat[idx].centroid)
        seed_count += 1
        seed[mid_frame, int(py), int(px)] = seed_count

    return seed


def remove_hot_pixel(seg: np.ndarray) -> np.ndarray:
    """
    remove hot pixel from segmentation
    """

    assert len(seg.shape) == 3, "input segmentation must be 3D"

    # make sure the segmentation is 0/1
    seg = seg.astype(np.uint8)
    seg[seg > 0] = 1

    # get sum projection along z
    seg_proj = np.sum(seg, axis=0)

    # find hot pixels
    hot_pixel = seg_proj >= seg.shape[0] - 2

    # dilate the area to cover the surrounding pixels
    hot_pixel = dilation(hot_pixel, disk(2))

    # clean up every z
    for z in range(seg.shape[0]):
        seg_z = seg[z, :, :]
        seg_z[hot_pixel] = 0
        seg[z, :, :] = seg_z

    return seg


def get_seed_for_objects(
    raw: np.ndarray,
    bw: np.ndarray,
    area_min: int = 1,
    area_max: int = 10000,
    bg_seed: bool = True,
) -> np.ndarray:
    """
    build a seed image for an image of 3D objects (assuming roughly convex shape
    in 3D) using the information in the middle slice

    Parameters:
    ------------
    raw: np.ndarray
        orignal image used to determine middle slice
    bw: np.ndarray
        a round 3D segmentation, expecting the segmentation in the middle slice
        having relatively good quality
    area_min: int
        estimated minimal size on one single slice (major body chunk, e.g. the
        center XY plane of a 3D ball) of an object
    area_max: int
        estimated maximal size on one single slice (major body chunk, e.g. the
        center XY plane of a 3D ball) of an object. It is recommended to be
        conservertive (setting this value a little larger)
    bg_seed: bool
        bg_seed=True will add a background seed at the first frame (z=0).

    """
    from skimage.morphology import remove_small_objects

    # determine middle slice
    mid_z = get_middle_frame(raw, method="intensity")

    # take seg of middle slice
    bw2d = bw[mid_z, :, :]

    # fillin holes to form solid objects
    bw2d_fill = hole_filling(bw2d, area_min, area_max)

    # prune the objects in middle slice
    out = remove_small_objects(bw2d_fill > 0, area_min)

    # extract object and calculate centroid
    out1 = label(out)
    stat = regionprops(out1)

    # use each centroid as one seed
    seed = np.zeros(raw.shape)
    seed_count = 0
    if bg_seed:
        seed[0, :, :] = 1
        seed_count += 1

    for idx in range(len(stat)):
        py, px = np.round(stat[idx].centroid)
        seed_count += 1
        seed[mid_z, int(py), int(px)] = seed_count

    return seed.astype(int)


def segmentation_union(seg: List) -> np.ndarray:
    """merge multiple segmentations into a single result

    Parameters
    ------------
    seg: List
        a list of segmentations, should all have the same shape
    """

    return np.logical_or.reduce(seg)


def segmentation_intersection(seg: List) -> np.ndarray:
    """get the intersection of multiple segmentations into a single result

    Parameters
    ------------
    seg: List
        a list of segmentations, should all have the same shape
    """

    return np.logical_and.reduce(seg)


def segmentation_xor(seg: List) -> np.ndarray:
    """get the XOR of multiple segmentations into a single result

    Parameters
    ------------
    seg: List
        a list of segmentations, should all have the same shape
    """

    return np.logical_xor.reduce(seg)


def remove_index_object(label: np.ndarray, id_to_remove: List[int] = [1], in_place=False):

    if in_place:
        img = label
    else:
        img = label.copy()

    for id in id_to_remove:
        img[img == id] = 0

    return img


def peak_local_max_wrapper(struct_img_for_peak: np.ndarray, bw: np.ndarray) -> np.ndarray:
    from skimage.feature import peak_local_max

    local_maxi = peak_local_max(struct_img_for_peak, labels=label(bw), min_distance=2, indices=False)
    return local_maxi


def watershed_wrapper(bw: np.ndarray, local_maxi: np.ndarray) -> np.ndarray:
    from scipy.ndimage import distance_transform_edt
    from skimage.measure import label
    from skimage.morphology import dilation, ball
    from skimage.segmentation import watershed

    distance = distance_transform_edt(bw)
    im_watershed = watershed(
        -distance,
        label(dilation(local_maxi, selem=ball(1))),
        mask=bw,
        watershed_line=True,
    )
    return im_watershed


def prune_z_slices(bw: np.ndarray):
    """
    prune the segmentation by only keep a certain range of z-slices
    with the assumption of all signals living only in a few consecutive
    z-slices. This function will first determine the key z-slice where most
    of the signals living on and then include a few slices up/down along z
    to make the segmentation completed. This is useful when you have prior
    knowledge about your segmentation target and can effectively exclude
    small segmented objects due to noise/artifacts in those z-slices we are
    sure the signal should not live on.

    Parameters:
    -----------
    bw: np.ndarray
        the segmentation before pruning
    """
    bw_z = np.zeros(bw.shape[0], dtype=np.uint16)
    for zz in range(bw.shape[0]):
        bw_z[zz] = np.count_nonzero(bw[zz, :, :] > 0)

    mid_z = np.argmax(bw_z)
    low_z = 0
    high_z = bw.shape[0] - 2
    for ii in np.arange(mid_z - 1, 0, -1):
        if bw_z[ii] < 100:
            low_z = ii
            break
    for ii in range(mid_z + 1, bw.shape[0] - 1, 1):
        if bw_z[ii] < 100:
            high_z = ii
            break

    seg = bw.copy()
    seg[:low_z, :, :] = 0
    seg[high_z + 1 :, :, :] = 0

    return seg


def cell_local_adaptive_threshold(structure_img_smooth: np.ndarray, cell_wise_min_area: int):
    from skimage.filters import threshold_triangle, threshold_otsu
    from skimage.morphology import dilation

    # cell-wise local adaptive thresholding
    th_low_level = threshold_triangle(structure_img_smooth)

    bw_low_level = structure_img_smooth > th_low_level
    bw_low_level = remove_small_objects(bw_low_level, min_size=cell_wise_min_area, connectivity=1, in_place=True)
    bw_low_level = dilation(bw_low_level, selem=ball(2))

    bw_high_level = np.zeros_like(bw_low_level)
    lab_low, num_obj = label(bw_low_level, return_num=True, connectivity=1)

    for idx in range(num_obj):
        single_obj = lab_low == (idx + 1)
        local_otsu = threshold_otsu(structure_img_smooth[single_obj > 0])
        bw_high_level[np.logical_and(structure_img_smooth > local_otsu * 0.98, single_obj)] = 1
    return bw_high_level


def invert_mask(img):
    return 1 - img


def mask_image(image, mask, value: int = 0):
    image[mask] = value
    return image
