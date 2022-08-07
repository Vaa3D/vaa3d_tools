import my_function


if __name__ == '__main__':
    overlapLeft = 254
    shiftLeft = 1
    overlapUp = 202
    shiftUp = 15
    imagesPath1 = 'F:\QualityControlProject\Data\\tif_Ex_647_Em_680_tileX2Y3'
    imagesPath2 = 'F:\QualityControlProject\Data\\tif_Ex_647_Em_680_tileX2Y4'
    savePath = 'F:\QualityControlProject\Data\stitched\Stitched_X2Y3_X2Y4'

    my_function.stitch_2img_left_right_after_up_down(imagesPath1, imagesPath2, overlapLeft, shiftLeft, overlapUp, shiftUp)

