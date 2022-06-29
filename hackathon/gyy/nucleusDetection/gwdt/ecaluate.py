import os
import numpy as np
import cv2
import tifffile
import math
import heapq
import copy
def distance_3D_point(pointA, pointB):
    A = pointA
    B = pointB
    A_xyz_list = []
    B_xyz_list = []
    for i in range(3):
        A_xyz_list.append(A[i])
        B_xyz_list.append(B[i])
    distance = 0
    for i in range(3):
        distance = distance + (float(A[i]) - float(B[i])) * (float(A[i]) - float(B[i]))
    distance = math.sqrt(distance)
    return distance


def is_3D_pointA_in_pointB_neighbourhood(pointA, pointB, neighbourhood):
    distance = distance_3D_point(pointA, pointB)
    if distance < neighbourhood:
        return True
    else:
        return False


def marker_to_points(path):
    points = []
    with open(path) as f:
        lines = f.readlines()
        for line in lines:
            if '#' in line:
                continue
            else:
                ss = line.split(',')
                point_3D = [ss[0], ss[1], ss[2]]
                points.append(point_3D)
    f.close()
    return points


def accuracy_ratio_by_path(points_detection_path, points_label_path, neighbourhood):
    def nearest_point(center, points):
        nearest = points[0]
        print(center)
        print(points_label[center])
        print(points_detection[points[0]])
        distance = distance_3D_point(points_label[center], points_detection[points[0]])
        for ii in points:
            t = distance_3D_point(points_label[center], points_detection[ii])
            if t < distance:
                nearest = ii
                distance = t
        return nearest

    def nearest_point_re(center, points):
        nearest = points[0]

        distance = distance_3D_point(points_detection[center], points_label[points[0]])
        for ii in points:
            t = distance_3D_point(points_detection[center], points_label[ii])
            if t < distance:
                nearest = ii
                distance = t
        return nearest

    def con():
        for ii in temp_temp_points_index_label:
            if len(ii) > 0:
                return True
        return False

    detection_ratio = 0
    wrong_ratio = 0
    repet_ratio = 0
    points_detection = marker_to_points(points_detection_path)
    points_label = marker_to_points(points_label_path)
    print(len(points_label))
    nb = neighbourhood
    points_index_label = []
    points_index_detection = []
    for i in range(len(points_label)):
        points_index_label.append([])
    for i in range(len(points_detection)):
        points_index_detection.append([])
    for i in range(len(points_index_label)):
        for j in range(len(points_detection)):
            if is_3D_pointA_in_pointB_neighbourhood(points_detection[j], points_label[i], nb):
                points_index_label[i].append(j)
                points_index_detection[j].append(i)
    temp_points_index_label = []
    for i in range(len(points_index_label)):
        temp_points_index_label.append(-1)
    temp_points_index_detection = []
    for i in range(len(points_index_detection)):
        temp_points_index_detection.append(-1)

    temp_temp_points_index_label = copy.deepcopy(points_index_label)
    temp_temp_points_index_detection = copy.deepcopy(points_index_detection)
    print(temp_temp_points_index_label)
    print(temp_temp_points_index_detection)

    while con():
        temp3_points_index_label = copy.deepcopy(temp_temp_points_index_label)
        temp3_points_index_detection = copy.deepcopy(temp_temp_points_index_detection)

        for i in range(len(temp3_points_index_label)):
            tt = temp3_points_index_label[i]
            n = None
            if len(tt) > 1:
                n = nearest_point(i, tt)
                tt.remove(n)
                for j in tt:
                    if len(temp3_points_index_detection[j])>0:
                        temp3_points_index_detection[j].remove(i)
                temp3_points_index_label[i] = [n]
        for i in range(len(temp3_points_index_detection)):
            tt = temp3_points_index_detection[i]
            n = None
            if len(tt) > 1:
                n = nearest_point_re(i, tt)
                tt.remove(n)
                for j in tt:
#                    print(i)
#                    print(temp3_points_index_label)
                    if len(temp3_points_index_label[j])>0:
                        temp3_points_index_label[j].remove(i)
                temp3_points_index_detection[i] = [n]
        for i in range(len(temp_points_index_label)):
            if len(temp3_points_index_label[i]) == 1:
                temp_points_index_label[i] = temp3_points_index_label[i][0]
                temp_points_index_detection[temp3_points_index_label[i][0]] = i
                temp_temp_points_index_label[i] = []
                temp_temp_points_index_detection[temp3_points_index_label[i][0]] = []
        c=1
    sum_wrong_detc = 0
    sum_wright_detc = 0
    detc = len(points_index_detection)
    for i in points_index_detection:
        if len(i) == 0:
            sum_wrong_detc = sum_wrong_detc + 1
    for i in temp_points_index_detection:
        if i != -1:
            sum_wright_detc = sum_wright_detc + 1
    repet_detc = detc - sum_wrong_detc - sum_wright_detc
    detection_ratio = float(sum_wright_detc) / len(points_index_label)
    wrong_ratio = float(sum_wrong_detc) / len(points_index_detection)
    repet_ratio = float(repet_detc) / len(points_index_detection)
    return detection_ratio, wrong_ratio, repet_ratio

a = accuracy_ratio_by_path('C:/Users/seusmy/Desktop/新建文件夹 (12)/image/image/ID(47)_gwdt_max.marker','C:/Users/seusmy/Desktop/新建文件夹 (12)/image/image/ID(47)_annotation.marker',7)
b = accuracy_ratio_by_path('C:/Users/seusmy/Desktop/新建文件夹 (12)/image/image/ID(47)_maxMarker.marker','C:/Users/seusmy/Desktop/新建文件夹 (12)/image/image/ID(47)_annotation.marker',7)
c = accuracy_ratio_by_path('C:/Users/seusmy/Desktop/新建文件夹 (12)/image/image/ID(68)_gwdt_max.marker','C:/Users/seusmy/Desktop/新建文件夹 (12)/image/image/ID(68)_annotation.marker',7)
d = accuracy_ratio_by_path('C:/Users/seusmy/Desktop/新建文件夹 (12)/image/image/ID(68)_maxMarker.marker','C:/Users/seusmy/Desktop/新建文件夹 (12)/image/image/ID(68)_annotation.marker',7)
print(a)
print(b)
print(c)
print(d)
