import os


def aaa(path, path1):
    path2 = path + '_/' + path1
    path = path + '/' + path1
    points = []
    with open(path) as f:
        lines = f.readlines()
        for line in lines:
            if '#' in line:
                continue
            if line == '\n':
                continue
            else:
                ss = line.split(',')
                point_3D = [float(ss[0]), float(ss[1]), float(ss[2])]
                points.append(point_3D)
    f.close()
    with open(path2, mode='w') as f:
        path_ = path1.split('_')
        temp = path_[2].split('.')
        path_[2] = temp[0]
        for point in points:
            point[0] = point[0] + float(path_[1])/10
            point[1] = point[1] + float(path_[0])/10
            point[2] = point[2] + float(path_[2])/10
            f.write(str(point[0]) + ',' + str(point[1]) + ',' + str(point[2]) + ', 0, 0, , , 255,0,0\n')
        f.write('\n')
    f.close()


def makers_to_points(paths):
    points = []
    for ii, jj, kk in os.walk(paths):
        for name_ in kk:
            path = ii + '/' + name_
            with open(path) as f:
                lines = f.readlines()
                for line in lines:
                    if '#' in line:
                        continue
                    if line == '\n':
                        continue
                    else:
                        ss = line.split(',')
                        point_3D = (float(ss[2]), float(ss[1]), float(ss[0]))
                        points.append(point_3D)
            f.close()
    return points


files_path = 'F:/QualityControlProject/Data/results/Xuan'
files_path2 = 'F:/QualityControlProject/Data/results/Xuan_'  # 这个文件夹放结果
all_path = 'F:/QualityControlProject/Data/results/Xuan_/all_tif_marker.swc'  # 合并成1个文件
for i, j, k in os.walk(files_path):
    for name in k:
        aaa(files_path, name)

points__ = makers_to_points(files_path2)
new_list = list(dict.fromkeys(points__))
with open(all_path, mode='w') as f:
    iii = 0
    f.write('#name \n')
    f.write('#comment \n')
    f.write('##n,type,x,y,z,radius,parent\n')

    for point in points__:
        f.write(str(iii) + ' 2 ' + str(point[0]) + ' ' + str(point[1]) + ' ' + str(point[2]) + ' 1.000 -1\n')
    f.write('\n')
f.close()
