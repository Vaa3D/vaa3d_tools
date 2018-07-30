"""
_jaccard.py : Jaccard metric for comparing set similarity.

"""

import numpy as np


def jaccard(im1, im2):
    """
    Computes the Jaccard metric, a measure of set similarity.

    Parameters
    ----------
    im1 : array-like, bool
        Any array of arbitrary size. If not boolean, will be converted.
    im2 : array-like, bool
        Any other array of identical size. If not boolean, will be converted.

    Returns
    -------
    jaccard : float
        Jaccard metric returned is a float on range [0,1].
        Maximum similarity = 1
        No similarity = 0
    
    Notes
    -----
    The order of inputs for `jaccard` is irrelevant. The result will be
    identical if `im1` and `im2` are switched.

    """
    im1 = im1.flatten().astype(np.bool)
    im2 = im2.flatten().astype(np.bool)


    if im1.shape != im2.shape:
        raise ValueError("Shape mismatch: im1 and im2 must have the same shape.")

    intersection = np.logical_and(im1, im2)

    union = np.logical_or(im1, im2)

    jaccard_coeff = intersection.sum() / float(union.sum())

    assert isinstance(jaccard_coeff, float)

    return jaccard_coeff
