from pathlib import Path
import numpy as np
from skimage.morphology import erosion, ball
from aicsimageio.writers import OmeTiffWriter


def save_segmentation(
    bw: np.ndarray,
    contour_flag: bool,
    output_path: Path,
    fn: str,
    suffix: str = "_struct_segmentation",
):
    """save the segmentation into a tiff file

    Parameters:
    ------------
    bw: np.ndarray
        the segmentation to save
    contour_flag: book
        whether to also save segmentation contour
    output_path: Path
        the path to save
    fn: str
        the core file name to use, for example, "img_102", then
        after a suffix (say "_seg") is added, the file name of the output
        is "img_101_seg.tiff"
    suffix: str
        the suffix to add to the output filename
    """
    OmeTiffWriter.save(data=bw, uri=str(output_path / (fn + suffix + ".tiff")), dim_order="ZYX")

    if contour_flag:
        bd = generate_segmentation_contour(bw)

        out_fn = str(output_path / (fn + suffix + "_contour.tiff"))
        OmeTiffWriter.save(data=bd, uri=out_fn, dim_order="ZYX")


def generate_segmentation_contour(im):
    """generate the contour of the segmentation"""

    bd = np.logical_xor(erosion(im > 0, selem=ball(1)), im > 0)

    bd = bd.astype(np.uint8)
    bd[bd > 0] = 255

    return bd


def output_hook(im, names, out_flag, output_path, fn):
    """general hook for cutomized output"""
    assert len(im) == len(names) and len(names) == len(out_flag)

    for i in range(len(out_flag)):
        if out_flag[i]:
            if names[i].startswith("bw_"):
                segmentation_type = names[i]
                bw = im[i].astype(np.uint8)
                bw[bw > 0] = 255
                OmeTiffWriter.save(data=bw, uri=str(output_path / (fn + "_bw_" + segmentation_type[3:] + ".tiff")))
            else:
                OmeTiffWriter.save(data=im[i], uri=str(output_path / (fn + "_" + names[i] + ".tiff")))
