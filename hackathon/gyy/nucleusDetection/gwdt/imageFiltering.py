import os
import numpy as np
import cv2
import tifffile
import math
import heapq
import copy


# main 函数
def gwdtmain():
    # dir_path = 'F:/QualityControlProject/Data/tif_Ex_488_Em_525_tileX3Y3'
    save_path = 'F:/QualityControlProject/Data/label_image/gwdt_localMax/results47/localMax' # 保存路径
    sigma = 2 # 最大值滤波核大小 2*sigma+1


    # cut_list = cutImage(dir_path, save_path, imgSzX, imgSzY, imgSzZ)
    # print('cutImage is done!')

    img_name = 'F:/QualityControlProject/Data/label_image/gwdt_localMax/ID(47).tif' # 输入图像路径
    cut_list = tifffile.imread(img_name) #读入图像
    print(cut_list.shape)

    bkg_thresh = calculate_bkg_thresh(cut_list) # 计算背景阈值 thres=ave+0.5*std
    print('bkg_thresh is done!')

    phi = fastMarching_dt(cut_list, bkg_thresh, save_path) # gwdt预处理

    print('gwdt is done!')

    # LoG_image = myFunction.LoG_filter(phi, save_path, sigma)
    # print('LoG filtering is done!')
    fImage = searchLocalMax(phi, sigma, save_path, bkg_thresh) # 寻找 原图像或gwdt后的图像的 局部最大值
    print('search local max is done!')
    writeMarker(fImage, save_path) # 局部最大值点记录到一个 .marker 文件中，marker文件是vaa3d的点标记文件
    print('marker writing is done!')


# cutImage 将2D stack文件cut为特定大小的3D图像
def cutImage(dir_path, save_path, imgSzX, imgSzY, imgSzZ):
    save_image = save_path + '/cutImg3D.tif'
    img_list = os.listdir(dir_path)
    cnt_num = 0
    for i in range(imgSzZ):
        img = img_list[i+900]
        #img_path = os.path.join(dir_path, img)
        img_path = dir_path + '/' + img
        gray_img = cv2.imread(img_path, -1)
        new_gray = gray_img[np.newaxis, ::]
        # print('gray_img shape:{}, new_gray shape:{}'.format(gray_img.shape, new_gray.shape))
        # global cnt_num
        if cnt_num == 0:
            tiff_list = new_gray
        else:
            tiff_list = np.append(tiff_list, new_gray, axis=0)
        cnt_num += 1

    cut_list = tiff_list[:imgSzZ, 900:imgSzY+900, 900:imgSzX+900]

    print('cutImg size = ', cut_list.shape)
    tifffile.imsave(save_image, cut_list)
    return cut_list

# 计算背景阈值 thres=ave+0.5*std
def calculate_bkg_thresh(cut_list):
    [imgSzZ, imgSzY, imgSzX] = cut_list.shape
    sum = 0.0
    sum_diff = 0.0
    for i in range(imgSzZ):
        for m in range(imgSzY):
            for n in range(imgSzX):
                sum += cut_list[i][m][n]
    ave = sum / (imgSzZ*imgSzY*imgSzX) # 平均值

    for i in range(imgSzZ):
        for m in range(imgSzY):
            for n in range(imgSzX):
                sum_diff += pow((cut_list[i][m][n] - ave), 2)
    std = math.sqrt(sum_diff/(imgSzZ*imgSzY*imgSzX)) # 方差

    # print('ave = ', ave)
    # print('std = ', std)
    bkg_thresh = ave + 0.5*std
    # print('thresh = ', bkg_thresh)
    return bkg_thresh

# 最大值滤波 begin
def spilt(a): # 最大值滤波中的一个函数
    if a%2 == 0:
        x1 = x2 = a/2
    else:
        x1 = math.floor(a/2)
        x2 = a - x1
    return -int(x1), int(x2)

def ori_function(i, j, k, kernel, img): # 最大值滤波中的一个函数，处理边界
    x1, x2 = spilt(kernel)
    y1, y2 = spilt(kernel)
    z1, z2 = spilt(kernel)
    temp = np.zeros(kernel*kernel*kernel)
    count = 0

    for l in range(x1, x2):
        for m in range(y1, y2):
            for n in range(z1, z2):
                if i+l<0 or i+l>img.shape[0]-1 or j+m<0 or j+m>img.shape[1]-1 or k+n<0 or k+n>img.shape[2]-1:
                    temp[count] = img[i, j, k]
                else:
                    temp[count] = img[i+l, j+m, k+n]
                count += 1
    return temp

def max_filter(oriImage, K_size):
    # maxFilPath = save_path + '/max_filter.tif'

    [imgSzZ, imgSzY, imgSzX] = oriImage.shape
    max_image = np.zeros((imgSzZ, imgSzY, imgSzX), dtype=np.float)
    # img_copy = copy.copy(oriImage)

    for i in range(0, imgSzZ):
        for j in range(0, imgSzY):
            for k in range(0, imgSzX):
                temp = ori_function(i, j, k, K_size, oriImage)
                max_image[i, j, k] = np.max(temp)
                # max2 = np.sort(temp)[-2]
                # if max_image[i, j, k] == max2:
                #     max_image[i, j, k] = 0
    max_image = max_image.astype(np.uint16)
    # tifffile.imsave(maxFilPath, max_image)
    return max_image
# 最大值滤波 end

# 寻找局部最大值
def searchLocalMax(ori_image, sigma, bkg_thresh):
    # maxPath = save_path + '/maxImage.tif'
    [imgSzZ, imgSzY, imgSzX] = ori_image.shape
    K_size = 2 * math.ceil(sigma) + 1
    fImage = max_filter(ori_image, K_size)

    for i in range(0, imgSzZ):
        for j in range(0, imgSzY):
            for k in range(0, imgSzX):
                if fImage[i, j, k] != ori_image[i, j, k] or fImage[i, j, k] < bkg_thresh:
                    fImage[i, j, k] = 0
    # set image border to zero
    b = (K_size-1)//2
    fImage[:, :, 0: b-1] = 0
    fImage[:, :, imgSzX-b+1: imgSzX] = 0
    fImage[:, 0: b-1, :] = 0
    fImage[:, imgSzY - b+1: imgSzY, :] = 0
    fImage[0: b-1, :, :] = 0
    fImage[imgSzZ - b+1: imgSzZ, :, :] = 0

    fImage = fImage.astype(np.uint16)
    # tifffile.imsave(maxPath, fImage)
    return fImage

# 将局部最大值点记录为marker文件
def writeMarker(fImage, save_path):
    markerPath = save_path + '/localMax_decon.marker'
    markerTitle = '##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n'
    [imgSzZ, imgSzY, imgSzX] = fImage.shape
    file = open(markerPath, 'w')
    file.write(markerTitle)
    for i in range(0, imgSzZ):
        for j in range(0, imgSzY):
            for k in range(0, imgSzX):
                if fImage[i, j, k] != 0:
                    file.write(str(k+1+0.01)+','+str(j+1+0.01)+','+str(i+1+0.01)+', 0, 0, , , 0,255,0\n') # 记录点坐标+颜色，对应上面的markerTitle
    file.close()


# gwdt begin
ALIVE = -1
TRIAL = 0
FAR = 1

class HeapElem: # gwdt 中的一个对象，堆元素
    def __init__(self, _ind, _value):
        self.heap_id = -1
        self.img_ind = _ind
        self.value = _value
    def __cmp__(self, other):
        if self.value < other.value:
            return -1
        elif self.value == other.value:
            return 0
        else:
            return 1

    def __lt__(self, other):
        return self.value < other.value


def adjust_heap_down(heap, elem, value0):
    length = len(heap)
    list = heapq.nsmallest(length, heap)
    elemNew = HeapElem(elem.img_ind, value0)
    list.remove(elem)
    heapq.heappush(list, elemNew)
    return heap

def fastMarching_dt(cut_image, bkg_thresh, save_path):
    gwdt_path = save_path + '/gwdt_Img3D.tif'
    cnn_type = 3
    [imgSzZ, imgSzY, imgSzX] = cut_image.shape
    phi = np.zeros((imgSzZ, imgSzY, imgSzX), dtype=np.float)
    state = np.zeros((imgSzZ, imgSzY, imgSzX))
    bkg_count = 0
    bdr_count = 0
    print('bkg_thresh = ', bkg_thresh)

    for l in range(imgSzZ):
        for m in range(imgSzY):
            for n in range(imgSzX):
                if cut_image[l][m][n] <= bkg_thresh:
                    phi[l][m][n] = cut_image[l][m][n]
                    state[l][m][n] = ALIVE
                    bkg_count += 1
                else:
                    phi[l][m][n] = float("inf")
                    state[l][m][n] = FAR

    heap = []
    dicElem = {}

    for k in range(0, imgSzZ):
        for j in range(0, imgSzY):
            for i in range(0, imgSzX):
                if state[k][j][i] == ALIVE:
                    for kk in range(-1, 2):
                        k2 = k + kk
                        if k2 < 0 or k2 >= imgSzZ:
                            continue
                        for jj in range(-1, 2):
                            j2 = j + jj
                            if j2 < 0 or j2 >= imgSzY:
                                continue
                            for ii in range(-1, 2):
                                i2 = i + ii
                                if i2 < 0 or i2 >= imgSzX:
                                    continue
                                offset = abs(ii) + abs(jj) + abs(kk)
                                if offset == 0 or offset > cnn_type:
                                    continue

                                if state[k2][j2][i2] == FAR:
                                    min_ind = [k, j, i]

                                    # get minimum Alive point around ind2
                                    if phi[min_ind[0]][min_ind[1]][min_ind[2]] > 0.0:
                                        for kkk in range(-1, 2):
                                            k3 = k2 + kkk
                                            if k3 < 0 or k3 >= imgSzZ:
                                                continue
                                            for jjj in range(-1, 2):
                                                j3 = j2 + jjj
                                                if j3 < 0 or j3 >= imgSzY:
                                                    continue
                                                for iii in range(-1, 2):
                                                    i3 = i2 + iii
                                                    if i3 < 0 or i3 >= imgSzX:
                                                        continue
                                                    offset2 = abs(iii) + abs(jjj) + abs(kkk)
                                                    if offset2 == 0 or offset2 > cnn_type:
                                                        continue
                                                    if state[k3][j3][i3] == ALIVE and phi[k3][j3][i3] < phi[min_ind[0]][min_ind[1]][min_ind[2]]:
                                                        min_ind = [k3, j3, i3]

                                    # over
                                    phi[k2][j2][i2] = phi[min_ind[0]][min_ind[1]][min_ind[2]] + cut_image[k2][j2][i2]
                                    state[k2][j2][i2] = TRIAL
                                    heap1 = HeapElem([k2, j2, i2], phi[k2][j2][i2])
                                    heapq.heappush(heap, heap1)
                                    dicElem[k2 * imgSzX * imgSzY + j2 * imgSzX + i2] = heap1
                                    bdr_count += 1


    print('bkg_count = ', bkg_count)
    print('bdr_count = ', bdr_count)
    print('dicElem.size = ', len(dicElem))

    #loop
    print('heap size = ', len(heap))
    while len(heap) != 0:
        min_elem = heapq.heappop(heap)
        dicElem.pop(min_elem.img_ind[0]* imgSzX *imgSzY+min_elem.img_ind[1]* imgSzX+min_elem.img_ind[2])
        min_ind1 = min_elem.img_ind

        state[min_ind1[0]][min_ind1[1]][min_ind1[2]] = ALIVE
        k = min_ind1[0]
        j = min_ind1[1]
        i = min_ind1[2]
        for kk in range(-1, 2):
            d = k+kk
            if d<0 or d >= imgSzZ:
                continue
            for jj in range(-1, 2):
                h = j+jj
                if h<0 or h >=imgSzY:
                    continue
                for ii in range(-1, 2):
                    w = i + ii
                    if w<0 or w >=imgSzX:
                        continue
                    offset3 = abs(ii) + abs(jj) + abs(kk)
                    if offset3 == 0 or offset3 > cnn_type:
                        continue

                    if state[d][h][w] != ALIVE:
                        new_dist = phi[min_ind1[0]][min_ind1[1]][min_ind1[2]] + cut_image[d][h][w] * math.sqrt(offset3)

                        if state[d][h][w] == FAR:
                            phi[d][h][w] = new_dist
                            elem = HeapElem([d, h, w], phi[d][h][w])
                            heapq.heappush(heap, elem)
                            dicElem[d*imgSzX*imgSzY+h*imgSzX+w] = elem
                            state[d][h][w] = TRIAL
                        elif state[d][h][w] == TRIAL:
                            if phi[d][h][w] > new_dist:
                                phi[d][h][w] = new_dist
                                elem = dicElem[d*imgSzX*imgSzY+h*imgSzX+w]
                                adjust_heap_down(heap, elem, phi[d][h][w])

    phi = phi.astype(np.uint16)
    tifffile.imsave(gwdt_path, phi)
    return phi
# gwdt end

def mid_filter(oriImage, save_path, K_size):
    midFilPath = save_path + '/mid_filter.tif'

    [imgSzZ, imgSzY, imgSzX] = oriImage.shape
    mid_image = np.zeros((imgSzZ, imgSzY, imgSzX), dtype=np.float)
    # img_copy = copy.copy(oriImage)
    for i in range(0, imgSzZ):
        for j in range(0, imgSzY):
            for k in range(0, imgSzX):
                temp = ori_function(i, j, k, K_size, oriImage)
                tempSort = np.sort(temp)
                half = len(tempSort) // 2
                mid_image[i, j, k] = (tempSort[half] + tempSort[~half]) / 2

    mid_image = mid_image.astype(np.uint16)
    tifffile.imsave(midFilPath, mid_image)
    return mid_image

def mean_filter(oriImage, save_path, K_size):
    midFilPath = save_path + '/mean_filter.tif'

    [imgSzZ, imgSzY, imgSzX] = oriImage.shape
    mean_image = np.zeros((imgSzZ, imgSzY, imgSzX), dtype=np.float)
    # img_copy = copy.copy(oriImage)
    for i in range(0, imgSzZ):
        for j in range(0, imgSzY):
            for k in range(0, imgSzX):
                temp = ori_function(i, j, k, K_size, oriImage)
                tempMean= np.mean(temp)
                mean_image[i, j, k] = tempMean

    mean_image = mean_image.astype(np.uint16)
    tifffile.imsave(midFilPath, mean_image)
    return mean_image

def convert_img_to_16bit(oriImage, save_path):
    save_16bit = save_path + '/gwdt_img_16bit.tif'
    [imgSzZ, imgSzY, imgSzX] = oriImage.shape
    outImg = np.zeros((imgSzZ, imgSzY, imgSzX), dtype=np.float)
    maxInten = oriImage[0][0][0]
    minInten = oriImage[0][0][0]
    for l in range(imgSzZ):
        for m in range(imgSzY):
            for n in range(imgSzX):
                if maxInten<oriImage[l][m][n]:
                    maxInten = oriImage[l][m][n]
                elif minInten>oriImage[l][m][n]:
                    maxInten = oriImage[l][m][n]

    if maxInten != minInten:
        w = 65535/(maxInten-minInten)
        for l in range(imgSzZ):
            for m in range(imgSzY):
                for n in range(imgSzX):
                    outImg[l][m][n] = (oriImage[l][m][n]-minInten)*w

    else:
        for l in range(imgSzZ):
            for m in range(imgSzY):
                for n in range(imgSzX):
                    outImg[l][m][n] = oriImage[l][m][n]

    tifffile.imsave(save_16bit, outImg)
    return outImg


def convolutions3(oriImage, kernel):
    [imgSzZ, imgSzY, imgSzX] = oriImage.shape
    kSizeZ = kernel.shape[0]

    pad = kSizeZ // 2
    conv_image = np.zeros((imgSzZ + pad * 2, imgSzY + pad * 2, imgSzX + pad * 2), dtype=np.float)
    conv_image[pad: pad + imgSzZ, pad: pad + imgSzY, pad: pad + imgSzX] = oriImage.copy().astype(np.float)
    tmpImg = conv_image.copy()

    for z in range(imgSzZ):
        for y in range(imgSzY):
            for x in range(imgSzX):
                for i in range(0, kSizeZ):
                    for j in range(0, kSizeZ):
                        for k in range(0, kSizeZ):
                            conv_image[pad+z, pad+y, pad+x] += tmpImg[z+i, y+j, x+k]*kernel[i][j][k]

    conv_image = np.clip(conv_image, 0, 35535)
    conv_image = conv_image[pad: pad + imgSzZ, pad: pad + imgSzY, pad: pad + imgSzX].astype(np.uint16)
    return conv_image

def Gaussian_filter(oriImage, save_path, sigma):
    Gaussian_path = save_path + '/Gaussian_Image.tif'
    [imgSzZ, imgSzY, imgSzX] = oriImage.shape
    K_size = 2 * math.ceil(sigma) + 1

    #zero padding
    pad = K_size // 2
    Gaussian_image = np.zeros((imgSzZ + pad * 2, imgSzY + pad * 2, imgSzX + pad * 2), dtype=np.float)
    Gaussian_image[pad: pad+imgSzZ, pad: pad+imgSzY, pad: pad+imgSzX] = oriImage.copy().astype(np.float)
    tmpImg = Gaussian_image.copy()

    #Gaussian Kernel
    kernel = np.zeros((K_size, K_size, K_size), dtype=np.float)
    for x in range(-pad, -pad+K_size):
        for y in range(-pad, -pad+K_size):
            for z in range(-pad, -pad+K_size):
                kernel[z+pad, y+pad, x+pad] = np.exp(-(x**2 + y**2 + z**2)/(2*(sigma**2)))
    kernel /= (2*np.pi)*(sigma**3)*math.sqrt(2*np.pi)
    kernel /= kernel.sum()

    # filtering
    for z in range(imgSzZ):
        for y in range(imgSzY):
            for x in range(imgSzX):
                Gaussian_image[pad+z, pad+y, pad+x] = np.sum(kernel*tmpImg[z:z+K_size, y:y+K_size, x:x+K_size])

    Gaussian_image = np.clip(Gaussian_image, 0, 35535)
    Gaussian_image = Gaussian_image[pad: pad+imgSzZ, pad: pad+imgSzY, pad: pad+imgSzX].astype(np.uint16)
    tifffile.imsave(Gaussian_path, Gaussian_image)
    return Gaussian_image

def annotation_to_localMax(ori_image, markerFile, save_path, sigma):
    [imgSzZ, imgSzY, imgSzX] = ori_image.shape
    K_size = 2 * math.ceil(sigma) + 1

    annoFile_modified = save_path + '.marker'
    markerTitle = '##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n'
    file = open(annoFile_modified, 'w')
    file.write(markerTitle)

    with open(markerFile, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if '#' in line:
                continue
            ss = line.split(',')

            xyz = []
            xyz.append(round(float(ss[0])-1))
            xyz.append(round(float(ss[1])-1))
            xyz.append(round(float(ss[2])-1))

            x1, x2 = spilt(K_size)
            y1, y2 = spilt(K_size)
            z1, z2 = spilt(K_size)

            maxValue = ori_image[xyz[2], xyz[1], xyz[0]]
            maxX = xyz[0]
            maxY = xyz[1]
            maxZ = xyz[2]
            for l in range(x1, x2):
                for m in range(y1, y2):
                    for n in range(z1, z2):
                        if xyz[0]+l>0 and xyz[0]+l<ori_image.shape[0]-1 and xyz[1]+m>0 and xyz[1]+m<ori_image.shape[1]-1 and xyz[2]+n>0 and xyz[2]+n<ori_image.shape[2]-1:
                            if maxValue < ori_image[xyz[2]+n, xyz[1]+m, xyz[0]+l]:
                                maxValue = ori_image[xyz[2]+n, xyz[1]+m, xyz[0]+l]
                                maxX = xyz[0] + l
                                maxY = xyz[1] + m
                                maxZ = xyz[2] + n

            xyzn = []
            xyzn.append(maxX+1)
            xyzn.append(maxY+1)
            xyzn.append(maxZ+1)

            file.write(str(xyzn[0]) + ',' + str(xyzn[1]) + ',' + str(xyzn[2]) + ', 0, 0, , , 255,0,0\n')

        file.close()


save_path = 'F:/QualityControlProject/filter/image2' # 保存路径
sigma = 2 # 最大值滤波核大小 2*sigma+1

img_name = 'F:/QualityControlProject/filter/image2/047870_059970_012650.tif_cut_decon1.tif' # 输入图像路径
cut_list = tifffile.imread(img_name) #读入图像
print(cut_list.shape)

bkg_thresh = calculate_bkg_thresh(cut_list) # 计算背景阈值 thres=ave+0.5*std
print('bkg_thresh is done!')
#
# phi = fastMarching_dt(cut_list, bkg_thresh, save_path) # gwdt预处理
# print('gwdt is done!')
#
# fImage = searchLocalMax(phi, sigma, bkg_thresh) # 寻找 原图像或gwdt后的图像的 局部最大值
# print('search local max is done!')
#
# writeMarker(fImage, save_path) # 局部最大值点记录到一个 .marker 文件中，marker文件是vaa3d的点标记文件
# print('marker writing is done!')
#
# Gaussian_filter(cut_list, save_path, sigma)
# mid_filter(cut_list, save_path, 2*sigma+1)
# mean_filter(cut_list, save_path, 2*sigma+1)

fImage = searchLocalMax(cut_list, sigma, bkg_thresh) # 寻找 原图像或gwdt后的图像的 局部最大值
print('search local max is done!')

writeMarker(fImage, save_path) # 局部最大值点记录到一个 .marker 文件中，marker文件是vaa3d的点标记文件
print('marker writing is done!')



