import numpy as np
from typing import List
from scipy.ndimage import gaussian_laplace


def dot_3d(struct_img: np.ndarray, log_sigma: float, cutoff=-1):
    """apply 3D spot filter on a 3D image

    Parameters:
    ------------
    struct_img: np.ndarray
        the 3D image to segment
    log_sigma: float
        the size of the filter, which can be set based on the estimated radius
        of your target dots. For example, if visually the diameter of the
        dots is usually 3~4 pixels, then you may want to set this as 1
        or something near 1 (like 1.25).
    cutoff: float
        the cutoff value to apply on the filter result. If the cutoff is
        negative, no cutoff will be applied. Default is -1
    """
    assert len(struct_img.shape) == 3
    responce = -1 * (log_sigma**2) * gaussian_laplace(struct_img, log_sigma)
    if cutoff < 0:
        return responce
    else:
        return responce > cutoff


def dot_2d(struct_img, log_sigma, cutoff=-1):
    """apply 2D spot filter on a 2D image

    Parameters:
    ------------
    struct_img: np.ndarray
        the 2D image to segment
    log_sigma: float
        the size of the filter, which can be set based on the estimated radius
        of your target dots. For example, if visually the diameter of the
        dots is usually 3~4 pixels, then you may want to set this as 1
        or something near 1 (like 1.25).
    cutoff: float
        the cutoff value to apply on the filter result. If the cutoff is
        negative, no cutoff will be applied. Default is -1
    """
    assert len(struct_img.shape) == 2
    responce = -1 * (log_sigma**2) * gaussian_laplace(struct_img, log_sigma)
    if cutoff < 0:
        return responce
    else:
        return responce > cutoff


def dot_3d_wrapper(struct_img: np.ndarray, s3_param: List):
    """wrapper for 3D spot filter

    Parameters:
    ------------
    struct_img: np.ndarray
        a 3d numpy array, usually the image after smoothing
    s3_param: List
        [[scale_1, cutoff_1], [scale_2, cutoff_2], ....], e.g. [[1, 0.1]]
        or [[1,0.12], [3,0.1]]. scale_x is set based on the estimated radius of
        your target dots. For example, if visually the diameter of the dots is
        about 3~4 pixels, then you may want to set scale_x as 1 or something
        near 1 (like 1.25). Multiple scales can be used, if you have dots of
        very different sizes. cutoff_x is a threshold applied on the actual filter
        reponse to get the binary result. Smaller cutoff_x may yielf more dots and
        "fatter" segmentation, while larger cutoff_x could be less permisive and
        yield less dots and slimmer segmentation.
    """

    bw = np.zeros(struct_img.shape, dtype=bool)
    for fid in range(len(s3_param)):
        log_sigma = s3_param[fid][0]
        responce = -1 * (log_sigma**2) * gaussian_laplace(struct_img, log_sigma)
        bw = np.logical_or(bw, responce > s3_param[fid][1])
    return bw


def logSlice(image: np.ndarray, sigma_list: List, threshold: float):
    """apply multi-scale 2D spot filter on a 2D image and binarize with threshold

    Parameters:
    -------------
    image: np.ndarray
        the 2D image to segment
    sigma_list: List
        The list of sigma representing filters in multiple scales
    threshold: float
        the cutoff to apply to get the binary output
    """

    gl_images = [-gaussian_laplace(image, s) * (s**2) for s in sigma_list]

    # get the mask
    seg = np.zeros_like(image)
    for zi in range(len(sigma_list)):
        seg = np.logical_or(seg, gl_images[zi] > threshold)

    return seg


def dot_slice_by_slice(struct_img: np.ndarray, log_sigma: float, cutoff=-1):
    """apply 2D spot filter on 3D image slice by slice

    Parameters:
    ------------
    struct_img: np.ndarray
        a 3d numpy array, usually the image after smoothing
    log_sigma: float
        the size of the filter, which can be set based on the estimated radius
        of your target dots. For example, if visually the diameter of the
        dots is usually 3~4 pixels, then you may want to set this as 1
        or something near 1 (like 1.25).
    cutoff: float
        the cutoff value to apply on the filter result. If the cutoff is
        negative, no cutoff will be applied. Default is -1
    """
    res = np.zeros_like(struct_img)
    for zz in range(struct_img.shape[0]):
        res[zz, :, :] = -1 * (log_sigma**2) * gaussian_laplace(struct_img[zz, :, :], log_sigma)

    if cutoff < 0:
        return res
    else:
        return res > cutoff


def dot_2d_slice_by_slice_wrapper(struct_img: np.ndarray, s2_param: List):
    """wrapper for 2D spot filter on 3D image slice by slice

    Parameters:
    ------------
    struct_img: np.ndarray
        a 3d numpy array, usually the image after smoothing
    s2_param: List
        [[scale_1, cutoff_1], [scale_2, cutoff_2], ....], e.g. [[1, 0.1]]
        or [[1, 0.12], [3,0.1]]: scale_x is set based on the estimated radius
        of your target dots. For example, if visually the diameter of the
        dots is usually 3~4 pixels, then you may want to set scale_x as 1
        or something near 1 (like 1.25). Multiple scales can be used, if
        you have dots of very different sizes. cutoff_x is a threshold
        applied on the actual filter reponse to get the binary result.
        Smaller cutoff_x may yielf more dots and fatter segmentation,
        while larger cutoff_x could be less permisive and yield less
        dots and slimmer segmentation.
    """
    bw = np.zeros(struct_img.shape, dtype=bool)
    for fid in range(len(s2_param)):
        log_sigma = s2_param[fid][0]
        responce = np.zeros_like(struct_img)
        for zz in range(struct_img.shape[0]):
            responce[zz, :, :] = -1 * (log_sigma**2) * gaussian_laplace(struct_img[zz, :, :], log_sigma)
        bw = np.logical_or(bw, responce > s2_param[fid][1])
    return bw
