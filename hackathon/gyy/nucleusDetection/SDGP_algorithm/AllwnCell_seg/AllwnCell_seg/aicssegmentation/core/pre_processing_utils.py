import numpy as np
from typing import List
from scipy.stats import norm
from scipy.ndimage import gaussian_filter


def intensity_normalization(struct_img: np.ndarray, scaling_param: List):
    """Normalize the intensity of input image so that the value range is from 0 to 1.

    Parameters:
    ------------
    img: np.ndarray
        a 3d image
    scaling_param: List
        a list with only one value 0, i.e. [0]: Min-Max normlaizaiton,
            the max intensity of img will be mapped to 1 and min will
            be mapped to 0
        a list with a single positive integer v, e.g. [5000]: Min-Max normalization,
            but first any original intensity value > v will be considered as outlier
            and reset of min intensity of img. After the max will be mapped to 1
            and min will be mapped to 0
        a list with two float values [a, b], e.g. [1.5, 10.5]: Auto-contrast
            normalizaiton. First, mean and standard deviaion (std) of the original
            intensity in img are calculated. Next, the intensity is truncated into
            range [mean - a * std, mean + b * std], and then recaled to [0, 1]
        a list with four float values [a, b, c, d], e.g. [0.5, 15.5, 200, 4000]:
            Auto-contrast normalization. Similat to above case, but only intensity value
            between c and d will be used to calculated mean and std.
    """
    assert len(scaling_param) > 0

    if len(scaling_param) == 1:
        if scaling_param[0] < 1:
            print("intensity normalization: min-max normalization with NO absolute" + "intensity upper bound")
        else:
            print(f"intensity norm: min-max norm with upper bound {scaling_param[0]}")
            struct_img[struct_img > scaling_param[0]] = struct_img.min()
        strech_min = struct_img.min()
        strech_max = struct_img.max()
    elif len(scaling_param) == 2:
        m, s = norm.fit(struct_img.flat)
        strech_min = max(m - scaling_param[0] * s, struct_img.min())
        strech_max = min(m + scaling_param[1] * s, struct_img.max())
        struct_img[struct_img > strech_max] = strech_max
        struct_img[struct_img < strech_min] = strech_min
    elif len(scaling_param) == 4:
        img_valid = struct_img[np.logical_and(struct_img > scaling_param[2], struct_img < scaling_param[3])]
        assert (
            img_valid.size > 0
        ), f"Adjust intensity normalization parameters {scaling_param[2]} and {scaling_param[3]} to include the image with range {struct_img.min()}:{struct_img.max()}"  # noqa: E501
        m, s = norm.fit(img_valid.flat)
        strech_min = max(scaling_param[2] - scaling_param[0] * s, struct_img.min())
        strech_max = min(scaling_param[3] + scaling_param[1] * s, struct_img.max())
        struct_img[struct_img > strech_max] = strech_max
        struct_img[struct_img < strech_min] = strech_min
    assert (
        strech_min <= strech_max
    ), f"Please adjust intensity normalization parameters so that {strech_min}<={strech_max}"
    struct_img = (struct_img - strech_min + 1e-8) / (strech_max - strech_min + 1e-8)

    # print('intensity normalization completes')
    return struct_img


def image_smoothing_gaussian_3d(struct_img, sigma, truncate_range=3.0):
    """
    wrapper for 3D Guassian smoothing
    """

    structure_img_smooth = gaussian_filter(struct_img, sigma=sigma, mode="nearest", truncate=truncate_range)

    return structure_img_smooth


def image_smoothing_gaussian_slice_by_slice(struct_img, sigma, truncate_range=3.0):
    """
    wrapper for applying 2D Guassian smoothing slice by slice on a 3D image
    """
    structure_img_smooth = np.zeros_like(struct_img)
    for zz in range(struct_img.shape[0]):
        structure_img_smooth[zz, :, :] = gaussian_filter(
            struct_img[zz, :, :], sigma=sigma, mode="nearest", truncate=truncate_range
        )

    return structure_img_smooth


def edge_preserving_smoothing_3d(
    struct_img: np.ndarray,
    numberOfIterations: int = 10,
    conductance: float = 1.2,
    timeStep: float = 0.0625,
    spacing: List = [1, 1, 1],
):
    """perform edge preserving smoothing on a 3D image

    Parameters:
    -------------
    struct_img: np.ndarray
        the image to be smoothed
    numberOfInterations: int
        how many smoothing iterations to perform. More iterations give more
        smoothing effect. Default is 10.
    timeStep: float
         the time step to be used for each iteration, important for numberical
         stability. Default is 0.0625 for 3D images. Do not suggest to change.
    spacing: List
        the spacing of voxels in three dimensions. Default is [1, 1, 1]

    Reference:
    -------------
    https://itk.org/Doxygen/html/classitk_1_1GradientAnisotropicDiffusionImageFilter.html
    """
    import itk

    itk_img = itk.GetImageFromArray(struct_img.astype(np.float32))

    # set spacing
    itk_img.SetSpacing(spacing)

    gradientAnisotropicDiffusionFilter = itk.GradientAnisotropicDiffusionImageFilter.New(itk_img)
    gradientAnisotropicDiffusionFilter.SetNumberOfIterations(numberOfIterations)
    gradientAnisotropicDiffusionFilter.SetTimeStep(timeStep)
    gradientAnisotropicDiffusionFilter.SetConductanceParameter(conductance)
    gradientAnisotropicDiffusionFilter.Update()

    itk_img_smooth = gradientAnisotropicDiffusionFilter.GetOutput()

    img_smooth_ag = itk.GetArrayFromImage(itk_img_smooth)

    return img_smooth_ag


def suggest_normalization_param(structure_img0):
    """
    suggest scaling parameter assuming the image is a representative example
    of this cell structure
    """
    m, s = norm.fit(structure_img0.flat)
    print(f"mean intensity of the stack: {m}")
    print(f"the standard deviation of intensity of the stack: {s}")

    p99 = np.percentile(structure_img0, 99.99)
    print(f"0.9999 percentile of the stack intensity is: {p99}")

    pmin = structure_img0.min()
    print(f"minimum intensity of the stack: {pmin}")

    pmax = structure_img0.max()
    print(f"maximum intensity of the stack: {pmax}")

    up_ratio = 0
    for up_i in np.arange(0.5, 1000, 0.5):
        if m + s * up_i > p99:
            if m + s * up_i > pmax:
                print(f"suggested upper range is {up_i-0.5}, which is {m+s*(up_i-0.5)}")
                up_ratio = up_i - 0.5
            else:
                print(f"suggested upper range is {up_i}, which is {m+s*up_i}")
                up_ratio = up_i
            break

    low_ratio = 0
    for low_i in np.arange(0.5, 1000, 0.5):
        if m - s * low_i < pmin:
            print(f"suggested lower range is {low_i-0.5}, which is {m-s*(low_i-0.5)}")
            low_ratio = low_i - 0.5
            break

    print(f"So, suggested parameter for normalization is [{low_ratio}, {up_ratio}]")
    print(
        "To further enhance the contrast: You may increase the first value "
        + "(may loss some dim parts), or decrease the second value"
        + "(may loss some texture in super bright regions)"
    )
    print("To slightly reduce the contrast: You may decrease the first value, or " + "increase the second value")
