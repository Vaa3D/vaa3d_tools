import json
from multiprocessing import Pool
from concurrent.futures import ThreadPoolExecutor
import tifffile
import cv2
import numpy as np
import os
import sys
import math

from c_calculate_shift import XY_shift
from c_calculate_shift.XY_shift import get_shift


# p1='Tile06'
# p2='Tile06'
# left_up_path = 'C:/Users/admin/Documents/WXWork/1688850447417369/Cache/File/2022-02/pavelData_Tile06_YZ_MIP.tif'
# right_down_path = 'C:/Users/admin/Documents/WXWork/1688850447417369/Cache/File/2022-02/pavelData_Tile07_YZ_MIP.tif'
# tt_path = 'C:/Users/admin/Documents/WXWork/1688850447417369/Cache/File/2022-02/pavelData_Tile08_YZ_MIP.tif'
# ttt_path = 'C:/Users/admin/Documents/WXWork/1688850447417369/Cache/File/2022-02/pavelData_Tile09_YZ_MIP.tif'
# x_base = 0
# y_base = 0
# x_shift_range = 60
# y_shift_range = 20

FFF = 20 #重要参数


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

json_env = 'F:/ZhaoHuImages/AI_denoise/Zhaohu_StitchCode/Z_shift.json'
with open(json_env, 'r')as fp:
    json_data = json.load(fp)
json_data =json_data["result_z_s"]
max__ = 0
NNN = 201  # 共126层 源数据
for data in json_data:
    if max__<data[2]:
        max__=data[2]
TTT = int((NNN - max__)/2)
PPP = 2  # 重要参数
QQQ = 5  # 重要参数
QNN = 5  # 重要参数
shift_x_P_d = shift_x_P_d - PPP
shift_y_P_d = shift_y_P_d - PPP
shift_x_P_d_ = shift_x_P_d_ - PPP
shift_y_P_d_ = shift_y_P_d_ - PPP
big_x = [x_length - shift_x_P - shift_x_P_d, x_length - shift_x_P + shift_x_P_d]
big_y = [y_length - shift_y_P - shift_y_P_d, y_length - shift_y_P + shift_y_P_d]
small_x = [-shift_x_P_d_, shift_x_P_d_]
small_y = [-shift_y_P_d_, shift_y_P_d_]
def get_shift(p1, p2, path1, path2, x, y, x_s, y_s,xx=None,yy=None):
    if xx is not None:
        x=int(xx)
        x_s=0
    if yy is not None:
        y=int(yy)
        y_s=0

    print(path1)
    print(path2)
    pp1=path1
    pp2=path2

    in1=path1.find('INDEX')
    in2 = path2.find('INDEX')
    path1 = path1[0:in1] + str(int(path1[in1 + 5:in1 + 8]) ).zfill(3) + path1[in1 + 8:len(path1)]
    path2 = path2[0:in2] + str(int(path2[in2 + 5:in2 + 8]) ).zfill(3) + path2[in2 + 8:len(path2)]
    img1 = cv2.imread(path1, -1)
    img2 = cv2.imread(path2, -1)

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

            temp_max = np.sum(np.square(temp_l - temp_r)) / 1.0 / temp_l.shape[0] / temp_l.shape[1]
            # print(temp_max)
            # print((temp_loc_x, temp_loc_y))

            if temp_max < max:
                max = temp_max
                loc = [p1, p2, temp_loc_x, temp_loc_y]
    if len(loc)<4:
        print(p1)
    for i in range(FFF):
        if (big_x[0] < loc[3] < big_x[1] or small_x[0] < loc[3] < small_x[1]) and (
            big_y[0] < loc[2] < big_y[1] or small_y[0] < loc[2] < small_y[1]): return loc
        else:
            path1=pp1[0:in1]+str(int(pp1[in1+5:in1+8])+FFF).zfill(3)+pp1[in1+8:len(pp1)]
            path2 = pp2[0:in2] + str(int(pp2[in2 + 5:in2 + 8]) + FFF).zfill(3) + pp2[in2 + 8:len(pp2)]
            loc = XY_shift.get_shift(p1, p2, path1, path2, x, y, x_s, y_s,xx,yy)
            if (big_x[0] < loc[3] < big_x[1] or small_x[0] < loc[3] < small_x[1]) and (
                    big_y[0] < loc[2] < big_y[1] or small_y[0] < loc[2] < small_y[1]): return loc
            path1 = pp1[0:in1] + str(int(pp1[in1 + 5:in1 + 8]) - FFF).zfill(3) + pp1[in1 + 8:len(pp1)]
            path2 = pp2[0:in2] + str(int(pp2[in2 + 5:in2 + 8]) - FFF).zfill(3) + pp2[in2 + 8:len(pp2)]
            loc = XY_shift.get_shift(p1, p2, path1, path2, x, y, x_s, y_s,xx,yy)
            if (big_x[0] < loc[3] < big_x[1] or small_x[0] < loc[3] < small_x[1]) and (
                    big_y[0] < loc[2] < big_y[1] or small_y[0] < loc[2] < small_y[1]): return loc

    return loc


# a = get_shift(p1,p2,left_up_path, right_down_path, y_base, x_base, y_shift_range, x_shift_range)
# print(a)


if __name__ == '__main__':
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

    json_env = 'F:/ZhaoHuImages/AI_denoise/Zhaohu_StitchCode/Z_shift.json'
    with open(json_env, 'r')as fp:
        json_data = json.load(fp)
    json_data = json_data["result_z_s"]
    max__ = 0
    NNN = 201  # 共126层 源数据
    for data in json_data:
        if max__ < data[2]:
            max__ = data[2]
    TTT = int((NNN + max__)/2)



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
                # print(input_folder+'/'+'Region 1_'+locations[i][j]+'_z100_RAW_ch00.tif')
                if i < locations.shape[0] - 1 and locations[i + 1][j] != 'None':
                    TTT = int((NNN + abs(
                        json_data['Tile' + locations[i][j][1:5]] - json_data['Tile' + locations[i + 1][j][1:5]])) / 2)
                    res = p.apply_async(get_shift, args=(locations[i][j],
                                                         locations[i + 1][j],
                                                         input_folder + '/' + 'Region 1_' + locations[i][
                                                             j] + '_z' + 'INDEX'+str(
                                                             -json_data['Tile' + locations[i][j][1:5]] + TTT).zfill(
                                                             3) + '_RAW_ch00.tif',
                                                         input_folder + '/' + 'Region 1_' + locations[i + 1][
                                                             j] + '_z' + 'INDEX'+str(
                                                             -json_data['Tile' + locations[i + 1][j][1:5]] + TTT).zfill(
                                                             3) + '_RAW_ch00.tif',
                                                         y_length - shift_y_P,
                                                         0,
                                                         shift_y_P_d_,
                                                         shift_x_P_d,))
                    res_l.append(res)
                if j < locations.shape[1] - 1 and locations[i][j + 1] != 'None':
                    TTT = int((NNN + abs(
                        json_data['Tile' + locations[i][j][1:5]] - json_data['Tile' + locations[i][j + 1][1:5]])) / 2)
                    res = p.apply_async(get_shift, args=(locations[i][j],
                                                         locations[i][j + 1],
                                                         input_folder + '/' + 'Region 1_' + locations[i][
                                                             j] + '_z' + 'INDEX'+str(
                                                             -json_data['Tile' + locations[i][j][1:5]] + TTT).zfill(
                                                             3) + '_RAW_ch00.tif',
                                                         input_folder + '/' + 'Region 1_' + locations[i][
                                                             j + 1] + '_z' + 'INDEX'+str(
                                                             -json_data['Tile' + locations[i][j + 1][1:5]] + TTT).zfill(
                                                             3) + '_RAW_ch00.tif',

                                                         0,
                                                         x_length - shift_x_P,
                                                         shift_y_P_d,
                                                         shift_x_P_d_,))
                    res_l.append(res)

    result_y_x_s = []
    for res in res_l:
        result_y_x_s.append(res.get())

    result = {  # 'result_z_s': result_z_s,
        'result_y_x_s': result_y_x_s
    }
    open(xy_result_file, 'w')
    with open(xy_result_file, 'w') as f:
        json.dump(result, f, indent=4, ensure_ascii=False)
