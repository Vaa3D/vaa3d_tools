import numpy as np
import copy
from typing import List
from .utils import divide_nonzero
from .hessian import absolute_3d_hessian_eigenvalues


def filament_3d_wrapper(struct_img: np.ndarray, f3_param: List[List]):
    """wrapper for 3d filament filter

    Parameters:
    ------------
    struct_img: np.ndarray
        the image (should have been smoothed) to be segmented. The image has to be 3D.
    f3_param: List[List]
        [[scale_1, cutoff_1], [scale_2, cutoff_2], ....], e.g., [[1, 0.01]] or
        [[1,0.05], [0.5, 0.1]]. scale_x is set based on the estimated thickness of your
        target filaments. For example, if visually the thickness of the filaments is
        usually 3~4 pixels, then you may want to set scale_x as 1 or something near 1
        (like 1.25). Multiple scales can be used, if you have filaments of very
        different thickness. cutoff_x is a threshold applied on the actual filter
        reponse to get the binary result. Smaller cutoff_x may yielf more filaments,
        especially detecting more dim ones and thicker segmentation, while larger
        cutoff_x could be less permisive and yield less filaments and slimmer
        segmentation.

    Reference:
    ------------
    T. Jerman, et al. Enhancement of vascular structures in 3D and 2D angiographic
    images. IEEE transactions on medical imaging. 2016 Apr 4;35(9):2107-18.
    """
    assert len(struct_img.shape) == 3, "image has to be 3D"
    bw = np.zeros(struct_img.shape, dtype=bool)
    for fid in range(len(f3_param)):
        sigma = f3_param[fid][0]
        eigenvalues = absolute_3d_hessian_eigenvalues(struct_img, sigma=sigma, scale=True, whiteonblack=True)
        responce = compute_vesselness3D(eigenvalues[1], eigenvalues[2], tau=1)
        bw = np.logical_or(bw, responce > f3_param[fid][1])
    return bw


def filament_2d_wrapper(struct_img: np.ndarray, f2_param: List[List]):
    """wrapper for 2d filament filter

    Parameters:
    ------------
    struct_img: np.ndarray
        the image (should have been smoothed) to be segmented. The image is
        either 2D or 3D. If 3D, the filter is applied in a slice by slice
        fashion
    f2_param: List[List]
        [[scale_1, cutoff_1], [scale_2, cutoff_2], ....], e.g., [[1, 0.01]]
        or [[1,0.05], [0.5, 0.1]]. Here, scale_x is set based on the estimated
        thickness of your target filaments. For example, if visually the thickness
        of the filaments is usually 3~4 pixels, then you may want to set scale_x
        as 1 or something near 1 (like 1.25). Multiple scales can be used, if you
        have filaments of very different thickness. cutoff_x is a threshold applied
        on the actual filter reponse to get the binary result. Smaller cutoff_x may
        yielf more filaments, especially detecting more dim ones and thicker
        segmentation, while larger cutoff_x could be less permisive and yield less
        filaments and slimmer segmentation.

    Reference:
    ------------
    T. Jerman, et al. Enhancement of vascular structures in 3D and 2D angiographic
    images. IEEE transactions on medical imaging. 2016 Apr 4;35(9):2107-18.
    """
    bw = np.zeros(struct_img.shape, dtype=bool)

    if len(struct_img.shape) == 2:
        for fid in range(len(f2_param)):
            sigma = f2_param[fid][0]
            eigenvalues = absolute_3d_hessian_eigenvalues(struct_img, sigma=sigma, scale=True, whiteonblack=True)
            responce = compute_vesselness2D(eigenvalues[1], tau=1)
            bw = np.logical_or(bw, responce > f2_param[fid][1])
    elif len(struct_img.shape) == 3:
        mip = np.amax(struct_img, axis=0)
        for fid in range(len(f2_param)):
            sigma = f2_param[fid][0]

            res = np.zeros_like(struct_img)
            for zz in range(struct_img.shape[0]):
                tmp = np.concatenate((struct_img[zz, :, :], mip), axis=1)
                eigenvalues = absolute_3d_hessian_eigenvalues(tmp, sigma=sigma, scale=True, whiteonblack=True)
                responce = compute_vesselness2D(eigenvalues[1], tau=1)
                res[zz, :, : struct_img.shape[2] - 3] = responce[:, : struct_img.shape[2] - 3]
            bw = np.logical_or(bw, res > f2_param[fid][1])
    return bw


def vesselness3D(nd_array: np.ndarray, sigmas: List, tau=1, whiteonblack=True, cutoff: float = -1):
    """Multi-scale 3D filament filter

    Parameters:
    ------------
    nd_array: np.ndarray
        the 3D image to be filterd on
    sigmas: List
        a list of scales to use
    tau: float
        parameter that controls response uniformity. The value has to be
        between 0.5 and 1. Lower tau means more intense output response.
        Default is 1
    whiteonblack: bool
        whether the filamentous structures are bright on dark background
        or dark on bright. Default is True.
    cutoff: float
        the cutoff value to apply on the filter result. If the cutoff is
        negative, no cutoff will be applied. Default is -1

    Reference:
    ------------
    T. Jerman, et al. Enhancement of vascular structures in 3D and 2D angiographic
    images. IEEE transactions on medical imaging. 2016 Apr 4;35(9):2107-18.
    """

    if not nd_array.ndim == 3:
        raise (ValueError("Only 3 dimensions is currently supported"))

    # adapted from https://github.com/scikit-image/scikit-image/blob/master/skimage/filters/_frangi.py#L74  # noqa E501
    if np.any(np.asarray(sigmas) < 0.0):
        raise ValueError("Sigma values less than zero are not valid")

    filtered_array = np.zeros(
        tuple(
            [
                len(sigmas),
            ]
        )
        + nd_array.shape
    )

    for i, sigma in enumerate(sigmas):
        eigenvalues = absolute_3d_hessian_eigenvalues(nd_array, sigma=sigma, scale=True, whiteonblack=True)
        filtered_array[i] = compute_vesselness3D(eigenvalues[1], eigenvalues[2], tau=tau)
    response = np.max(filtered_array, axis=0)

    if cutoff < 0:
        return response
    else:
        return response > cutoff


def vesselness2D(
    nd_array: np.ndarray,
    sigmas: List,
    tau: float = 1,
    whiteonblack: bool = True,
    cutoff: float = -1,
):
    """Multi-scale 2D filament filter

    Parameters:
    ------------
    nd_array: np.ndarray
        the 2D image to be filterd on
    sigmas: List
        a list of scales to use
    tau: float
        parameter that controls response uniformity. The value has to be
        between 0.5 and 1. Lower tau means more intense output response.
        Default is 0.5
    whiteonblack: bool
        whether the filamentous structures are bright on dark background
        or dark on bright. Default is True.
    cutoff: float
        the cutoff value to apply on the filter result. If the cutoff is
        negative, no cutoff will be applied. Default is -1

    Reference:
    ------------
    T. Jerman, et al. Enhancement of vascular structures in 3D and 2D angiographic
    images. IEEE transactions on medical imaging. 2016 Apr 4;35(9):2107-18.
    """

    if not nd_array.ndim == 2:
        raise (ValueError("Only 2 dimensions is currently supported"))

    # adapted from https://github.com/scikit-image/scikit-image/blob/master/skimage/filters/_frangi.py#L74  # noqa E501
    if np.any(np.asarray(sigmas) < 0.0):
        raise ValueError("Sigma values less than zero are not valid")

    filtered_array = np.zeros(
        tuple(
            [
                len(sigmas),
            ]
        )
        + nd_array.shape
    )

    for i, sigma in enumerate(sigmas):
        eigenvalues = absolute_3d_hessian_eigenvalues(nd_array, sigma=sigma, scale=True, whiteonblack=True)
        filtered_array[i] = compute_vesselness2D(eigenvalues[1], tau=tau)
    response = np.max(filtered_array, axis=0)

    if cutoff < 0:
        return response
    else:
        return response > cutoff


def vesselness2D_single_slice(
    nd_array: np.ndarray,
    single_z: int,
    sigmas: List,
    tau: float = 1,
    whiteonblack: bool = True,
    cutoff: float = -1,
):
    """Multi-scale 2D filament filter

    Parameters:
    ------------
    nd_array: np.ndarray
        the 3D image to be filterd on
    single_z: int
        the index of the slice to apply the filter
    sigmas: List
        a list of scales to use
    tau: float
        parameter that controls response uniformity. The value has to be
        between 0.5 and 1. Lower tau means more intense output response.
        Default is 0.5
    whiteonblack: bool
        whether the filamentous structures are bright on dark background
        or dark on bright. Default is True.
    cutoff: float
        the cutoff value to apply on the filter result. If the cutoff is
        negative, no cutoff will be applied. Default is -1

    Reference:
    ------------
    T. Jerman, et al. Enhancement of vascular structures in 3D and 2D angiographic
    images. IEEE transactions on medical imaging. 2016 Apr 4;35(9):2107-18.
    """

    if not nd_array.ndim == 3:
        raise (ValueError("Only 3 dimensions is currently supported"))

    # adapted from https://github.com/scikit-image/scikit-image/blob/master/skimage/filters/_frangi.py#L74  # noqa E501
    if np.any(np.asarray(sigmas) < 0.0):
        raise ValueError("Sigma values less than zero are not valid")

    response = np.zeros(nd_array.shape)
    response[single_z, :, :] = vesselness2D(nd_array[single_z, :, :], sigmas=sigmas, tau=1, whiteonblack=True)

    if cutoff < 0:
        return response
    else:
        return response > cutoff


def vesselnessSliceBySlice(
    nd_array: np.ndarray,
    sigmas: List,
    tau: float = 1,
    whiteonblack: bool = True,
    cutoff: float = -1,
):
    """
    wrapper for applying multi-scale 2D filament filter on 3D images in a
    slice by slice fashion

    Parameters:
    -----------
    nd_array: np.ndarray
        the 3D image to be filterd on
    sigmas: List
        a list of scales to use
    tau: float
        parameter that controls response uniformity. The value has to be
        between 0.5 and 1. Lower tau means more intense output response.
        Default is 0.5
    whiteonblack: bool
        whether the filamentous structures are bright on dark background
        or dark on bright. Default is True.
    cutoff: float
        the cutoff value to apply on the filter result. If the cutoff is
        negative, no cutoff will be applied. Default is -1
    """

    mip = np.amax(nd_array, axis=0)
    response = np.zeros(nd_array.shape)
    for zz in range(nd_array.shape[0]):
        tmp = np.concatenate((nd_array[zz, :, :], mip), axis=1)
        tmp = vesselness2D(tmp, sigmas=sigmas, tau=1, whiteonblack=True)
        response[zz, :, : nd_array.shape[2] - 3] = tmp[:, : nd_array.shape[2] - 3]

    if cutoff < 0:
        return response
    else:
        return response > cutoff


def compute_vesselness3D(eigen2, eigen3, tau):
    """backend for computing 3D filament filter"""

    lambda3m = copy.copy(eigen3)
    lambda3m[np.logical_and(eigen3 < 0, eigen3 > (tau * eigen3.min()))] = tau * eigen3.min()
    response = np.multiply(np.square(eigen2), np.abs(lambda3m - eigen2))
    response = divide_nonzero(27 * response, np.power(2 * np.abs(eigen2) + np.abs(lambda3m - eigen2), 3))

    response[np.less(eigen2, 0.5 * lambda3m)] = 1
    response[eigen2 >= 0] = 0
    response[eigen3 >= 0] = 0
    response[np.isinf(response)] = 0

    return response


def compute_vesselness2D(eigen2, tau):
    """backend for computing 2D filament filter"""

    Lambda3 = copy.copy(eigen2)
    Lambda3[np.logical_and(Lambda3 < 0, Lambda3 >= (tau * Lambda3.min()))] = tau * Lambda3.min()

    response = np.multiply(np.square(eigen2), np.abs(Lambda3 - eigen2))
    response = divide_nonzero(27 * response, np.power(2 * np.abs(eigen2) + np.abs(Lambda3 - eigen2), 3))

    response[np.less(eigen2, 0.5 * Lambda3)] = 1
    response[eigen2 >= 0] = 0
    response[np.isinf(response)] = 0

    return response
