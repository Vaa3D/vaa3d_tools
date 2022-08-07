import numpy as np
import matplotlib.pyplot as plt


def sliceViewer(im: np.ndarray, zz: int):
    """simple wrapper to view one slice of a z-stack


    Parameter
    -----------
    im: np.ndarray
        3D image stack to view
    zz: int
        the slice to return


    Example:
    ---------
    >>> from ipywidgets import interact, fixed
    >>> import ipywidgets as widgets
    >>> interact(
    >>>     sliceViewer,
    >>>     im = fixed(struct_img),
    >>>     zz = widgets.IntSlider(
    >>>         min = 0,
    >>>         max = struct_img.shape[0] - 1,
    >>>         step = 1,
    >>>         value = struct_img.shape[0] // 2,
    >>>         continuous_update = False
    >>>     )
    >>> );
    """
    plt.imshow(im[zz, :, :])
    plt.show()


def random_colormap(nn: int = 10000):
    """generate a random colormap with nn different colors

    Parameter:
    ----------
    nn: int
        the number of random colors needed


    Example:
    ----------
    >>> import matplotlib.pyplot as plt
    >>> # img_label is output of a label function and represent all connected components
    >>> plt.imshow(img_label, cmap=random_colormap())
    """
    from matplotlib import cm

    viridis = cm.get_cmap("viridis", nn)
    for ii in range(nn):
        for jj in range(3):
            viridis.colors[ii][jj] = np.random.rand()

    # always set first color index as black
    viridis.colors[0][0] = 0
    viridis.colors[0][1] = 0
    viridis.colors[0][2] = 0

    return viridis


def blob2dExplorer_single(im, sigma, th):
    """backend for trying 2D spot filter on a single Z slice

    Parameters
    ----------
    im : np.ndarray
        2D image array
    sigma : float
        sigma in the spot filter
    th : float
        threshold to be applied as cutoff on filter output


    Example:
    ----------
    >>> from ipywidgets import interact, fixed
    >>> import ipywidgets as widgets
    >>> # define slide bars for trying different parameters
    >>> interact(
    >>>     blob2dExplorer_single,
    >>>     im = fixed(img),
    >>>     sigma = widgets.FloatRangeSlider(
    >>>         value = (1, 5),
    >>>         min = 1,
    >>>         max = 15,
    >>>         step = 1,
    >>>         continuous_update = False
    >>>     ),
    >>>     th = widgets.FloatSlider(
    >>>         value = 0.02,
    >>>         min = 0.01,
    >>>         max = 0.1,
    >>>         step = 0.01,
    >>>         continuous_update = False
    >>>     )
    >>> );
    """
    from aicssegmentation.core.seg_dot import logSlice

    bw = logSlice(im, (sigma[0], sigma[1], 1), th)
    plt.imshow(im)
    plt.show()
    plt.imshow(bw)
    plt.show()


def fila2dExplorer_single(im, sigma, th):
    """backend for trying 2D filament filter on a single Z slice

    Parameters
    ----------
    im : np.ndarray
        2D image array
    sigma : float
        sigma in the filament filter
    th : float
        threshold to be applied as cutoff on filter output


    Example:
    ----------
    >>> from ipywidgets import interact, fixed
    >>> import ipywidgets as widgets
    >>> # define slide bars for trying different parameters
    >>> interact(
    >>>     fila2dExplorer_single,
    >>>     im = fixed(img),
    >>>     sigma = widgets.FloatRangeSlider(
    >>>         value = 3,
    >>>         min = 1,
    >>>         max = 11,
    >>>         step = 1,
    >>>         continuous_update = False
    >>>     ),
    >>>     th = widgets.FloatSlider(
    >>>         value = 0.05,
    >>>         min = 0.01,
    >>>         max = 0.5,
    >>>         step = 0.01,
    >>>         continuous_update = False
    >>>     )
    >>> );
    """
    from .vessel import vesselness2D

    tmp = vesselness2D(im, [sigma], tau=1, whiteonblack=True)
    plt.imshow(im)
    plt.show()
    plt.imshow(tmp > th)
    plt.show()


def mipView(im):
    """simple wrapper to view maximum intensity projection"""
    mip = np.amax(im, axis=0)
    plt.imshow(mip)
    plt.show()


def img_seg_combine(img, seg, roi=["Full", None]):
    """creating raw and segmentation side-by-side for visualizaiton"""
    # normalize to 0~1
    img = img.astype(np.float32)
    img = (img - img.min()) / (img.max() - img.min())
    seg = seg.astype(np.float32)
    seg[seg > 0] = 1

    if roi[0] == "ROI" or roi[0] == "roi":
        img = img[roi[1]]
        seg = seg[roi[1]]
    elif roi[0] == "manual" or roi[0] == "M":
        img = img[:, roi[1][1] : roi[1][3], roi[1][0] : roi[1][2]]
        seg = seg[:, roi[1][1] : roi[1][3], roi[1][0] : roi[1][2]]

    # combine
    combined = np.concatenate((seg, img), axis=2)

    #  view
    return combined


def seg_fluo_side_by_side(im, seg, roi=["Full", None]):
    """wrapper for displaying raw and segmentation side by side"""
    out = img_seg_combine(im, seg, roi)

    return out


def segmentation_quick_view(seg: np.ndarray):
    """wrapper for visualizing segmentation in ITK viewer

    Parameter:
    -----------
    seg: np.ndarray
        3D stack of segmentation to view

    Example:
    -----------
    >>> from itkwidgets import view
    >>> view(segmentation_quick_view(seg))

    """
    valid_pxl = np.unique(seg[seg > 0])
    if len(valid_pxl) < 1:
        print("segmentation is empty")
        return

    seg = seg > 0
    seg = seg.astype(np.uint8)
    seg[seg > 0] = 255

    return seg


def single_fluorescent_view(im):
    """wrapper for visualizing an image stack in ITK viewer

    Parameter:
    -----------
    im: np.ndarray
        3D image stack to view

    Example:
    -----------
    >>> from itkwidgets import view
    >>> view(single_fluorescent_view(im))

    """

    assert len(im.shape) == 3

    im = im.astype(np.float32)
    im = (im - im.min()) / (im.max() - im.min())

    return im
