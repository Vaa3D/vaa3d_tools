import json
from concurrent.futures import ThreadPoolExecutor
import tifffile
import cv2
import numpy as np

json_env = "C:/Users/admin/Desktop/program_Stitch/jsonFiles/stitching_pavel_64_22.json"  # 参数文件
with open(json_env, 'r')as fp:
    json_data = json.load(fp)
# print(json_data)
input_folder = json_data['nput_folder']
result_folder = json_data['result_folder']
planes_num = int(json_data['planes_num'])
thread_num = int(json_data['thread_num'])
tiles = json_data['tiles']

tiles_num = len(tiles)
pre = 'Tile'
mid = '_'
end = '.tif'

result_x = 0
result_y = 0
for tile in tiles:
    if result_x < tile['posX'] + tile['x_length'] - 1:
        result_x = tile['posX'] + tile['x_length'] - 1
    if result_y < tile['posY'] + tile['y_length'] - 1:
        result_y = tile['posY'] + tile['y_length'] - 1
result_x = result_x + 1
result_y = result_y + 1


def stitch_one_plane(index,planes_num,result_x,result_y,result_folder,tiles_num,input_folder,pre,mid,end,tiles):
    z_ = len(str(planes_num))
    dz_ = z_ - len(str(index))
    z_str = str(index)
    for i in range(dz_):
        z_str = '0' + z_str
    result_plane = np.zeros((result_y, result_x), dtype=np.int)
    result_path = result_folder + '/z' + mid + z_str + end
    print('result_path1:', result_path)
    l_ = len(str(tiles_num))
    #print(tiles_num)
    for i in range(tiles_num):
        # print('tile_i: ',i)
        # print("index:"+str(i))
        dl_ = l_ - len(str(i))
        l_str = str(i)
        for j in range(dl_):
            l_str = '0' + l_str
        img_path = input_folder + '/' + pre + l_str + mid + z_str + end
        # print(img_path)
        img = cv2.imread(img_path, -1)

        posX = int(tiles[i]['posX'])
        posY = int(tiles[i]['posY'])
        x_length = int(tiles[i]['x_length'])
        y_length = int(tiles[i]['y_length'])
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
                    radio = jj / x_length * (1/0.064)
                    if radio > kk / y_length * (1/0.22):
                        radio = kk / y_length * (1/0.22)
                    #print("radio:" + str(radio))
                    # print('www',result_plane[posY + k][posX + j])
                    result_plane[posY + k][posX + j] = result_plane[posY + k][posX + j] * (1 - radio) + img[k][j] * radio

    result_plane = result_plane.astype(np.uint16)
    tifffile.imsave(result_path,result_plane)

    return 0


#thread_result = []
#for i in range(planes_num):
#    thread_result.append(-1)
pool = ThreadPoolExecutor(thread_num)
for i in range(planes_num):
    pool.submit(stitch_one_plane, i,planes_num,result_x,result_y,result_folder,tiles_num,input_folder,pre,mid,end,tiles)
    pass

# stitch_one_plane(1,planes_num,result_x,result_y,result_folder,tiles_num,input_folder,pre,mid,end,tiles)


#pool.shutdown()

