import json
from multiprocessing import Pool
from concurrent.futures import ThreadPoolExecutor
import tifffile
import cv2
import numpy as np
import os
import sys
import math

json_env = "F:/ZhaoHuImages/AI_denoise/Zhaohu_StitchCode/tiles_position.json"  # 参数文件

def get_shift(p1, p2, path1, path2, x_base, y_base, x_shift_range, y_shift_range):
    img1 = cv2.imread(path1, -1)
    img2 = cv2.imread(path2, -1)

    # print(img1.shape)3_Z-shift_.py
    #
    # mean = (np.mean(img1) + np.mean(img2))/2
    # print(mean)
    # mean = int(mean + 0.5*math.sqrt(np.sum(np.square(img1-mean)+np.square(img2-mean))/2/(img1.shape[0]*img1.shape[1])))
    # print(mean)
    #
    # cv2.threshold(img1, mean, 65535, cv2.THRESH_BINARY, img1)
    # cv2.threshold(img2, mean, 65535, cv2.THRESH_BINARY, img2)


    max = sys.maxsize
    loc = (0, 0)

    for i in range(-x_shift_range, x_shift_range):
        for j in range(-y_shift_range, y_shift_range):

            temp_loc_x = x_base + i
            temp_loc_y = y_base + j
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

            temp_max = np.sum(np.square(temp_l - temp_r)) / 1.0 / temp_l.shape[0] / temp_l.shape[1]
            # print(temp_max)
            # print((temp_loc_x, temp_loc_y))

            if temp_max < max:
                max = temp_max
                loc = (p1, p2, temp_loc_x, temp_loc_y)
    return loc


# a = get_shift(p1,p2,left_up_path, right_down_path, y_base, x_base, y_shift_range, x_shift_range)
# print(a)


if __name__ == '__main__':

    with open(json_env, 'r')as fp:
        json_data = json.load(fp)
    # print(json_data)
    input_folder = json_data['input_folder']
    z_result_file = json_data['z_result_file']
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


    p = Pool(thread_num)
    res_l = []
    for i in range(locations.shape[0]):
        for j in range(locations.shape[1]):
            if locations[i][j] != 'None':
                if i < locations.shape[0] - 1 and locations[i + 1][j] != 'None':
                    res = p.apply_async(get_shift, args=(locations[i][j],
                                                         locations[i + 1][j],
                                                         input_folder + '/' + locations[i][
                                                             j] + '_' + 'XZ' + '_MIP_' + 'D.tif',
                                                         input_folder + '/' + locations[i + 1][
                                                             j] + '_' + 'XZ' + '_MIP_' + 'U.tif',
                                                         0,
                                                         0,
                                                         shift_z_P_d_,
                                                         shift_x_P_d_,))
                    res_l.append(res)
                if j < locations.shape[1] - 1 and locations[i][j + 1] != 'None':
                    res = p.apply_async(get_shift, args=(locations[i][j],
                                                         locations[i][j + 1],
                                                         input_folder + '/' + locations[i][
                                                             j] + '_' + 'YZ' + '_MIP_' + 'R.tif',
                                                         input_folder + '/' + locations[i][
                                                             j + 1] + '_' + 'YZ' + '_MIP_' + 'L.tif',
                                                         0,
                                                         0,
                                                         shift_z_P_d_,
                                                         shift_y_P_d_,))
                    res_l.append(res)
    result_z_s = []
    for res in res_l:
        result_z_s.append(res.get())

    result = {
        'result_z_s': result_z_s
              }
    open(z_result_file, 'w')
    with open(z_result_file, 'w') as f:
        json.dump(result, f, indent=4, ensure_ascii=False)
