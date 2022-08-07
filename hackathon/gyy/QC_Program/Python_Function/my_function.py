import os
import numpy as np
import cv2


def stitch_2img_down_up(imagePath1, imagePath2, overlap, shift):
    image1 = cv2.imread(imagePath1, -1)
    image2 = cv2.imread(imagePath2, -1)

    [imgSzY, imgSzX] = image1.shape
    # print(image1.shape)
    resultImg = np.zeros((imgSzY*2-overlap, imgSzX+shift), dtype=np.float)

    for i in range(imgSzY-overlap):
        for j in range(imgSzX):
            resultImg[i][j] = image1[i][j]

    for i in range(imgSzY-overlap, imgSzY):
        for j in range(shift):
            resultImg[i][j] = image1[i][j]
        for j in range(shift, shift*2):
            d1 = i-(imgSzY-overlap)
            d2 = j-shift
            w2_up = d1/overlap
            w1_up = 1-w2_up
            intensity_up = image1[i][j]*w1_up+image2[d1][d2]*w2_up

            w2_left = d2/shift
            w1_left = 1-w2_left
            intensity_left = image1[i][j]*w1_left+image2[d1][d2]*w2_left

            resultImg[i][j] = (intensity_up + intensity_left)/2
        for j in range(shift*2, imgSzX-shift):
            d1 = i - (imgSzY - overlap)
            d2 = j - shift
            w2_up = d1 / overlap
            w1_up = 1 - w2_up
            resultImg[i][j] = image1[i][j] * w1_up + image2[d1][d2] * w2_up
        for j in range(imgSzX-shift, imgSzX):
            d1 = i-(imgSzY-overlap)
            d2 = j-shift
            w2_up = d1/overlap
            w1_up = 1-w2_up
            intensity_up = image1[i][j]*w1_up+image2[d1][d2]*w2_up

            w1_left = (imgSzX-j)/shift
            w2_left = 1 - w1_left
            intensity_left = image1[i][j]*w1_left+image2[d1][d2]*w2_left

            resultImg[i][j] = (intensity_up + intensity_left)/2
        for j in range(imgSzX, imgSzX+shift):
            d1 = i - (imgSzY - overlap)
            d2 = j - shift
            resultImg[i][j] = image2[d1][d2]

    for i in range(imgSzY, imgSzY*2-overlap):
        for j in range(shift, imgSzX+shift):
            d1 = i - (imgSzY - overlap)
            d2 = j - shift
            resultImg[i][j] = image2[d1][d2]

    resultImg = resultImg.astype(np.uint16)

    return resultImg

def stitch_2img_all_plane_down_up(imagesPath1, imagesPath2, overlap, shift, savePath):
    img_list1 = os.listdir(imagesPath1)
    img_list2 = os.listdir(imagesPath2)
    cnt_num = 0
    for image1 in img_list1:
        image2 = img_list2[cnt_num]
        saveName = savePath + '/' + image1

        imgPath1 = imagesPath1 + '/' + image1
        imgPath2 = imagesPath2 + '/' + image2
        result = stitch_2img_down_up(imgPath1, imgPath2, overlap, shift)
        cv2.imwrite(saveName, result)

        cnt_num += 1

    return 0

def stitch_2img_left_right_after_up_down(imagePath1, imagePath2, overlapLeft, shiftLeft, overlapUp, shiftUp):
    image1 = cv2.imread(imagePath1, -1)
    image2 = cv2.imread(imagePath2, -1)

    [imgSzY, imgSzX] = image1.shape
    # print(image1.shape)
    resultImg = np.zeros((imgSzY + shiftLeft, imgSzX + 2000 - overlapLeft), dtype=np.float)

    for i in range(2000 - overlapLeft):
        for j in range(shiftLeft, imgSzY):
            resultImg[j][i] = image1[j-shiftLeft][i]

    for i in range(2000 - overlapLeft, 2000):
        for j in range(shiftLeft):
            resultImg[j][i] = image2[j][i+2000-overlapLeft]
        for j in range(shiftLeft, shiftLeft + 15):
            d1 = j - shiftLeft
            w1_up = d1/15
            w2_up = 1-w1_up
            intensity1 = image1[j-shiftLeft][i]*w1_up+image2[j][i+2000-overlapLeft]*w2_up
            d2 = i+2000-overlapLeft
            w2_left = d2/overlapLeft
            w1_left = 1- w2_left
            intensity2 = image1[j - shiftLeft][i] * w1_left + image2[j][i + 2000 - overlapLeft] * w2_left
            resultImg[j][i] = (intensity1 + intensity2)/2
        for j in range(shiftLeft + 15, imgSzY - 15):
            d2 = i + 2000 - overlapLeft
            w2_left = d2 / overlapLeft
            w1_left = 1 - w2_left
            resultImg[j][i] = image1[j - shiftLeft][i] * w1_left + image2[j][i + 2000 - overlapLeft] * w2_left
        for j in range(imgSzY - 15, imgSzY):
            d1 = imgSzY - j
            w2_up = d1 / 15
            w1_up = 1 - w1_up
            intensity1 = image1[j - shiftLeft][i] * w1_up + image2[j][i + 2000 - overlapLeft] * w2_up
            d2 = i + 2000 - overlapLeft
            w2_left = d2 / overlapLeft
            w1_left = 1 - w2_left
            intensity2 = image1[j - shiftLeft][i] * w1_left + image2[j][i + 2000 - overlapLeft] * w2_left
            resultImg[j][i] = (intensity1 + intensity2) / 2
        for j in range(imgSzY, imgSzY + shiftLeft):
            resultImg[j][i] = image1[j - shiftLeft][i]

    for i in range(2000, 2015):
        for j in range(1600-overlapUp+shiftLeft):
            resultImg[j][i] = image2[j][i + 2000 - overlapLeft]
        for j in range(1600-overlapUp+shiftLeft, 1600-overlapUp+shiftLeft+15):
            d1 = j-(1600-overlapUp+shiftLeft)
            w2_up = d1 / 15
            w1_up = 1 - w2_up
            intensity1 = image1[j - shiftLeft][i] * w1_up + image2[j][i + 2000 - overlapLeft] * w2_up
            d2 = i + 2000 - overlapLeft
            w2_left = d2 / overlapLeft
            w1_left = 1 - w2_left
            intensity2 = image1[j - shiftLeft][i] * w1_left + image2[j][i + 2000 - overlapLeft] * w2_left
            resultImg[j][i] = (intensity1 + intensity2) / 2




    resultImg = resultImg.astype(np.uint16)

    return resultImg

