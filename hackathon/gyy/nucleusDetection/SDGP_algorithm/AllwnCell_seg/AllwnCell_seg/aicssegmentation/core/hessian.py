from itertools import combinations_with_replacement

import numpy as np
from scipy import ndimage as ndi

from .utils import absolute_eigenvaluesh


def compute_3d_hessian_matrix(
    nd_array: np.ndarray,
    sigma: float = 1,
    scale: bool = True,
    whiteonblack: bool = True,
) -> np.ndarray:
    """
    Computes the hessian matrix for an nd_array. The implementation was adapted from:
    https://github.com/ellisdg/frangi3d/blob/master/frangi/hessian.py

    Parameters:
    ----------
    nd_array: np.ndarray
        nd array from which to compute the hessian matrix.
    sigma: float
        Standard deviation used for the Gaussian kernel to smooth the array. Defaul is 1
    scale: bool
        whether the hessian elements will be scaled by sigma squared. Default is True
    whiteonblack: boolean
        image is white objects on black blackground or not. Default is True


    Return:
    ----------
    hessian array of shape (..., ndim, ndim)
    """
    ndim = nd_array.ndim

    # smooth the nd_array
    smoothed = ndi.gaussian_filter(nd_array, sigma=sigma, mode="nearest", truncate=3.0)

    # compute the first order gradients
    gradient_list = np.gradient(smoothed)

    # compute the hessian elements
    hessian_elements = [
        np.gradient(gradient_list[ax0], axis=ax1) for ax0, ax1 in combinations_with_replacement(range(ndim), 2)
    ]

    if sigma > 0 and scale:
        # scale the elements of the hessian matrix
        if whiteonblack:
            hessian_elements = [(sigma**2) * element for element in hessian_elements]
        else:
            hessian_elements = [-1 * (sigma**2) * element for element in hessian_elements]

    # create hessian matrix from hessian elements
    hessian_full = [[()] * ndim for x in range(ndim)]
    # hessian_full = [[None] * ndim] * ndim

    for index, (ax0, ax1) in enumerate(combinations_with_replacement(range(ndim), 2)):
        element = hessian_elements[index]
        hessian_full[ax0][ax1] = element
        if ax0 != ax1:
            hessian_full[ax1][ax0] = element

    hessian_rows = list()
    for row in hessian_full:
        # print(row.shape)
        hessian_rows.append(np.stack(row, axis=-1))

    hessian = np.stack(hessian_rows, axis=-2)
    return hessian


def absolute_3d_hessian_eigenvalues(
    nd_array: np.ndarray,
    sigma: float = 1,
    scale: bool = True,
    whiteonblack: bool = True,
):
    """
    Eigenvalues of the hessian matrix calculated from the input array sorted by
    absolute value.

    Parameters:
    ------------
    nd_array: np.ndarray
        nd array from which to compute the hessian matrix.
    sigma: float
        Standard deviation used for the Gaussian kernel to smooth the array. Defaul is 1
    scale: bool
        whether the hessian elements will be scaled by sigma squared. Default is True
    whiteonblack: boolean
        image is white objects on black blackground or not. Default is True

    Return:
    ------------
    list of eigenvalues [eigenvalue1, eigenvalue2, ...]
    """
    return absolute_eigenvaluesh(
        compute_3d_hessian_matrix(nd_array, sigma=sigma, scale=scale, whiteonblack=whiteonblack)
    )
