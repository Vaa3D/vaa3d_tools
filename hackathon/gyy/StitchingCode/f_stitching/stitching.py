import json
from concurrent.futures import ThreadPoolExecutor
import tifffile
import cv2
import numpy as np
json_env = 'F:/ZhaoHuImages/AI_denoise/Zhaohu_StitchCode/z_y_x_p.json'

with open(json_env, 'r')as fp:
    json_data = json.load(fp)
z_y_x_p = json_data['z_y_x_p']


json_env = "F:/ZhaoHuImages/AI_denoise/Zhaohu_StitchCode/f_stitching/stitching_p.json"  # 参数文件
with open(json_env, 'r')as fp:
    json_data = json.load(fp)
# print(json_data)
input_folder = json_data['input_folder']
result_folder = json_data['result_folder']
planes_num = int(json_data['planes_num'])
thread_num = int(json_data['thread_num'])
planes_num_0 =planes_num

tiles_num = len(z_y_x_p)
pre = 'TileScan 1_'
mid = '_z'
end = '_RAW_ch00_dst.tif'

result_x = 0
result_y = 0
result_z = 0
tiles = set()
for tile in z_y_x_p:
    if result_x < tile['x'] + tile['x_length'] - 1:
        result_x = tile['x'] + tile['x_length'] - 1
    if result_y < tile['y'] + tile['y_length'] - 1:
        result_y = tile['y'] + tile['y_length'] - 1
    if result_z<tile['z']:
        result_z = tile['z']
    tiles.add(tile['Tile'])

result_x = result_x + 1
result_y = result_y + 1
planes_num = planes_num + result_z
aaa=1

def stitch_one_plane(index,planes_num,result_x,result_y,result_folder,tiles_num,input_folder,pre,mid,end,tiles=None):
    # z_ = len(str(planes_num))
    # dz_ = z_ - len(str(index))
    # z_str = str(index)
    # for i in range(dz_):
    #     z_str = '0' + z_str
    z_str = str(index).zfill(3)
    result_plane = np.zeros((result_y, result_x), dtype=np.int)
    result_path = result_folder + '/z' + mid + z_str + end
    print('result_path1:', result_path)

    #print(tiles_num)
    for i in range(tiles_num):
        # print('tile_i: ',i)
        # print("index:"+str(i))
        temp_t = z_y_x_p[i]
        l_str = temp_t['Tile']
        if index< temp_t['z'] or index>temp_t['z'] + planes_num_0-1:
            continue


        #
        # dl_ = l_ - len(str(i))
        # l_str = str(i)
        # for j in range(dl_):
        #     l_str = '0' + l_str
        z_str_i = str(index-temp_t['z']).zfill(3)  # Z number
        img_path = input_folder + '/' + pre + l_str + mid + z_str_i + end
        # print(img_path)
        img = cv2.imread(img_path, -1)

        posX = temp_t['x']
        posY = temp_t['y']
        x_length = temp_t['x_length']
        y_length = temp_t['y_length']
        for j in range(x_length):
            for k in range(y_length):

                if result_plane[posY + k][posX + j] == 0:
                    result_plane[posY + k][posX + j] = img[k][j]  # 先y后x
                else:
                    jj = 0
                    kk = 0
                    if j < x_length / 2:
                        jj = j
                    else:
                        jj = x_length - j
                    if k < y_length / 2:
                        kk = k
                    else:
                        kk = y_length - k
                    radio = jj / x_length * (1/0.1)
                    if radio > kk / y_length * (1/0.1):
                        radio = kk / y_length * (1/0.1)
                    #print("radio:" + str(radio))
                    # print('www',result_plane[posY + k][posX + j])
                    result_plane[posY + k][posX + j] = result_plane[posY + k][posX + j] * (1 - radio) + img[k][j] * radio

    cv2.imwrite(result_path, result_plane)

    return 0


pool = ThreadPoolExecutor(thread_num)
print(planes_num)
for i in range(105,120):
    pool.submit(stitch_one_plane, i,planes_num,result_x,result_y,result_folder,tiles_num,input_folder,pre,mid,end)
    pass
pool.shutdown()


# stitch_one_plane(0,planes_num,result_x,result_y,result_folder,tiles_num,input_folder,pre,mid,end)