import copy
import json
from multiprocessing import Pool
from concurrent.futures import ThreadPoolExecutor
import tifffile
import cv2
import numpy as np
import os
import sys
import math
from skimage import exposure


#
# p1='floor1_y12'
# p2='floor1_y13'
# left_up_path = 'F:/ZhaoHuImages/results/MAX_floor1_y12.tif'
# right_down_path = 'F:/ZhaoHuImages/results/MAX_floor1_y13.tif'
# x_base = 0
# y_base = 0
# x_shift_range = 30
# y_shift_range = 30


def get_shift(p1, p2, path1, path2, x, y, x_s, y_s,t_p=None):
    a=p1
    # print(path1)
    # print(path2)
    img1 = cv2.imread(path1, -1)
    img2 = cv2.imread(path2, -1)
    img1__ = img1.copy()
    img2__ = img2.copy()
    if x < y:
        img1 = img1[0:img1.shape[0], y - y_s - 1:img1.shape[1]]
        img2 = img2[0:img2.shape[0], 0:img2.shape[1] - (y - y_s - 1)]
    else:
        img1 = img1[x - x_s - 1:img1.shape[0], 0:img1.shape[1]]
        img2 = img2[0:img2.shape[0] - (x - x_s - 1), 0:img2.shape[1]]
    bj_img1 = np.mean(img1) + 0.5 * np.std(img1)
    bj_img2 = np.mean(img2) + 0.5 * np.std(img2)
    bj_ = 0.5 * (bj_img1 + bj_img2)
    img1_ = img1 > bj_img1
    img2_ = img2 > bj_img2

    img1 = img1 * img1_
    img2 = img2 * img2_
    img1 = exposure.equalize_hist(img1) * 65535
    img2 = exposure.equalize_hist(img2) * 65535
    img1 = img1.astype(np.uint16)
    img2 = img2.astype(np.uint16)

    bj_img1 = np.mean(img1) + 0.5 * np.std(img1)
    bj_img2 = np.mean(img2) + 0.5 * np.std(img2)
    bj_ = 0.5 * (bj_img1 + bj_img2)
    img1_ = img1 > bj_img1
    img2_ = img2 > bj_img2

    img1 = img1 * img1_
    img2 = img2 * img2_

    path1_ = copy.copy(path1).replace('.', '_.')
    path2_ = copy.copy(path2).replace('.', '_.')
    cv2.imwrite(path1_, img1)
    cv2.imwrite(path2_, img2)
    if x < y:
        img1__[0:img1__.shape[0], y - y_s - 1:img1__.shape[1]] = img1
        img2__[0:img2__.shape[0], 0:img2__.shape[1] - (y - y_s - 1)] = img2
        img1 = img1__
        img2 = img2__
    else:
        img1__[x - x_s - 1:img1__.shape[0], 0:img1__.shape[1]] = img1
        img2__[0:img2__.shape[0] - (x - x_s - 1), 0:img2__.shape[1]] = img2
        img1 = img1__
        img2 = img2__

    # print(img1.shape)
    #
    # mean = (np.mean(img1) + np.mean(img2))/2
    # print(mean)
    # mean = int(mean + 0.5*math.sqrt(np.sum(np.square(img1-mean)+np.square(img2-mean))/2/(img1.shape[0]*img1.shape[1])))
    # print(mean)
    #
    # cv2.threshold(img1, mean, 65535, cv2.THRESH_BINARY, img1)
    # cv2.threshold(img2, mean, 65535, cv2.THRESH_BINARY, img2)

    #     img1_t = np.zeros(img1.shape,np.uint8)
    #     img2_t = np.zeros(img2.shape, np.uint8)
    #     cv2.threshold(img1,mean,255,cv2.THRESH_BINARY,img1_t)
    #     cv2.threshold(img2,mean,255,cv2.THRESH_BINARY,img2_t)
    #     cv2.imwrite(tt_path,img1_t)
    #     img1_t_t = cv2.distanceTransform(img1_t,1,3)
    #     img2_t_t = cv2.distanceTransform(img2_t,1,3)
    #     cv2.imwrite(ttt_path, img1_t_t)
    # #    cv2.threshold(img1,mean,1,cv2.THRESH_BINARY,img1_t)
    # #    cv2.threshold(img2,mean,1,cv2.THRESH_BINARY,img2_t)
    #     img1 = img1_t_t
    #     img2 =img2_t_t
    #   #  img1 = img1_t
    #    # img2 = img2_t
    #     #img1 = img1 * img1_t * img1_t_t
    #     #img2 = img2 * img2_t * img2_t_t
    #     #img1 = (img1*65536/(np.max(img1)-np.min(img1))).astype(np.uint16)
    #     #img2 = (img2 * 65536 / (np.max(img2) - np.min(img2))).astype(np.uint16)

    max = sys.maxsize
    loc = (0, 0)
    a = 0

    for i in range(-x_s, x_s+1):
        for j in range(-y_s, y_s+1):
            a += 1
            # print(a)

            temp_loc_x = x + i
            temp_loc_y = y + j
            if temp_loc_x > 0:
                if temp_loc_y > 0:
                    temp_l = img1[temp_loc_x:img1.shape[0], temp_loc_y:img1.shape[1]]
                    temp_r = img2[0:img2.shape[0] - temp_loc_x, 0:img2.shape[1] - temp_loc_y]
                else:
                    temp_l = img1[temp_loc_x:img1.shape[0], 0:img1.shape[1] + temp_loc_y]
                    temp_r = img2[0:img2.shape[0] - temp_loc_x, -temp_loc_y:img2.shape[1]]
            else:
                if temp_loc_y > 0:
                    temp_l = img1[0:img1.shape[0] + temp_loc_x, temp_loc_y:img1.shape[1]]
                    temp_r = img2[-temp_loc_x:img2.shape[0], 0:img2.shape[1] - temp_loc_y]
                else:
                    temp_l = img1[0:img1.shape[0] + temp_loc_x, 0:img1.shape[1] + temp_loc_y]
                    temp_r = img2[-temp_loc_x:img2.shape[0], -temp_loc_y:img2.shape[1]]

            # temp_max = np.sum(np.square(temp_l - temp_r)) / 1.0 / temp_l.shape[0] / temp_l.shape[1]
            # print(temp_max)
            # print((temp_loc_x, temp_loc_y))
            # print(p1 + ' ' + p2)

            ttt = np.sum((temp_l == 0) * (temp_r == 0))

            # print(p1 + ' ' + p2 + 'ttt=' + str(ttt) )


            temp_max = np.sum(np.square(temp_l - temp_r)) / 1.0 / (temp_l.shape[0] * temp_l.shape[1] - ttt)

            if temp_max < max:
                max = temp_max
                loc = (p1, p2, temp_loc_x, temp_loc_y)
    print(loc)
    return loc


# a = get_shift(p1,p2,left_up_path, right_down_path, y_base, x_base, y_shift_range, x_shift_range)
# print(a)


if __name__ == '__main__':
    file_name_list = []
    tiles_p = {}
    file_path = 'C:/Users/admin/Desktop/Update/Zhaohu/brain1HR_cor.txt'
    with open(file_path, 'r') as file_to_read:
        i = 0
        while i < 87:
            lines = file_to_read.readline().strip()  # 整行读取数据
            if not lines:
                break
                pass
            file_name_list.append(lines)
            pass
            i += 1

    for i in range(len(file_name_list)):
        line = file_name_list[i].split(',')
        line_c = [int(round(float(line[2].split(')')[0]), 0)), int(round(float(line[1]), 0)),int(round(float(line[0].split('(')[1]), 0))
                  ]
        # line_coorfile_name_list[i].split(',')=np.array(line_coor,dtype='')
        tiles_p['floor'+file_name_list[i].split('.')[0].split('r')[1]] = line_c

    json_env = "F:/ZhaoHuImages/AI_denoise/Zhaohu_StitchCode/tiles_position.json"  # 参数文件
    with open(json_env, 'r')as fp:
        json_data = json.load(fp)
    # print(json_data)
    input_folder = json_data['input_folder']
    xy_result_file = json_data['xy_result_file']
    locations = json_data['locations']
    # tiles_names = json_data['tiles_names']
    x_length = int(json_data['x_length'])
    y_length = int(json_data['y_length'])
    shift_x_P = int(json_data['shift_x_P'])
    shift_y_P = int(json_data['shift_y_P'])
    shift_x_P_d = int(json_data['shift_x_P_d'])
    shift_y_P_d = int(json_data['shift_y_P_d'])
    shift_x_P_d_ = int(json_data['shift_x_P_d_'])
    shift_y_P_d_ = int(json_data['shift_y_P_d_'])
    shift_z_P_d_ = int(json_data['shift_z_P_d_'])
    thread_num = int(json_data['thread_num'])
    locations = np.asarray(locations)



    # p = Pool(thread_num)
    # res_l = []
    # for i in range(locations.shape[0]):
    #     for j in range(locations.shape[1]):
    #         if locations[i][j] != 'None':
    #             if i < locations.shape[0] - 1 and locations[i + 1][j] != 'None':
    #                 res = p.apply_async(get_shift, args=(locations[i][j],
    #                                                      locations[i + 1][j],
    #                                                      input_folder + '/' + locations[i][
    #                                                          j] + '_' + 'XZ' + '_MIP_' + 'D.tif',
    #                                                      input_folder + '/' + locations[i + 1][
    #                                                          j] + '_' + 'XZ' + '_MIP_' + 'U.tif',
    #                                                      0,
    #                                                      0,
    #                                                      shift_z_P_d_,
    #                                                      shift_x_P_d_,))
    #                 res_l.append(res)
    #             if j < locations.shape[1] - 1 and locations[i][j + 1] != 'None':
    #                 res = p.apply_async(get_shift, args=(locations[i][j],
    #                                                      locations[i][j + 1],
    #                                                      input_folder + '/' + locations[i][
    #                                                          j] + '_' + 'YZ' + '_MIP_' + 'R.tif',
    #                                                      input_folder + '/' + locations[i][
    #                                                          j + 1] + '_' + 'YZ' + '_MIP_' + 'L.tif',
    #                                                      0,
    #                                                      0,
    #                                                      shift_z_P_d_,
    #                                                      shift_y_P_d_,))
    #                 res_l.append(res)
    # result_z_s = []
    # for res in res_l:
    #     result_z_s.append(res.get())

    p = Pool(thread_num)
    res_l = []



    for i in range(locations.shape[0]):
        for j in range(locations.shape[1]):
            if locations[i][j] != 'None':
                # print(input_folder+'/MAX_'+locations[i][j]+'.tif')
                if i<locations.shape[0]-1 and locations[i+1][j]!= 'None':

                    res = p.apply_async(get_shift, args=(locations[i][j],
                                                         locations[i+1][j],
                                                         input_folder+'/MAX_'+locations[i][j]+'.tif',
                                                         input_folder+'/MAX_'+locations[i+1][j]+'.tif',
                                                         tiles_p[locations[i+1][j]][1] - tiles_p[locations[i][j]][1],
                                                         0,
                                                         shift_y_P_d_ if shift_y_P_d_< int((1024-tiles_p[locations[i+1][j]][1] + tiles_p[locations[i][j]][1])/2) else int((1024-tiles_p[locations[i+1][j]][1] + tiles_p[locations[i][j]][1])/2) ,
                                                         shift_x_P_d,))
                    res_l.append(res)
                if j < locations.shape[1] - 1 and locations[i][j+1] != 'None':

                    res = p.apply_async(get_shift, args=(locations[i][j],
                                                         locations[i][j+1],
                                                         input_folder + '/MAX_' + locations[i][j] +'.tif',
                                                         input_folder + '/MAX_' + locations[i][j+ 1] +'.tif',
                                                         0,
                                                         tiles_p[locations[i][j+1]][2] - tiles_p[locations[i][j]][2],
                                                         shift_y_P_d,
                                                         shift_x_P_d_ if shift_x_P_d_< int((2048-tiles_p[locations[i][j+1]][2] + tiles_p[locations[i][j]][2])/2) else int((2048-tiles_p[locations[i][j+1]][2] + tiles_p[locations[i][j]][2])/2),))
                    res_l.append(res)

    result_y_x_s = []
    for res in res_l:
        result_y_x_s.append(res.get())

    result = {#'result_z_s': result_z_s,
                       'result_y_x_s': result_y_x_s
              }
    open(xy_result_file, 'w')
    with open(xy_result_file, 'w') as f:
        json.dump(result, f, indent=4, ensure_ascii=False)
