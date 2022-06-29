from xml.etree import ElementTree
import time

time_start = time.time()

# xml_path = input('请输入xml文件路径：')
xml_path = 'F:/ZhaoHuImages/thy1_egfp_m/MetaData/TileScan 1_Properties.xml'
with open(xml_path) as f:
    tree = ElementTree.parse(f)

abs_posX_set = set()  # 设置X轴绝对位置参数集合，用于去重
abs_posY_set = set()  # 设置Y轴绝对位置参数集合，用于去重
tile_abs_info = {}  # 设置有关tile信息的字典
tile_coor_info = {}
for node in tree.iter('Tile'):  # 遍历xml文档中标记为‘Tile’的节点
    tile_label = int(node.attrib.get('FieldX'))
    abs_posX = node.attrib.get('PosX')
    abs_posY = node.attrib.get('PosY')
    abs_posX_set.add(abs_posX)
    abs_posY_set.add(abs_posY)
    tile_abs_info.setdefault((abs_posX, abs_posY), tile_label)  # 向字典中添加tile信息

abs_posX_list = list(abs_posX_set)  # 把posX_set转为列表
abs_posY_list = list(abs_posY_set)  # 把posY_set转为列表
abs_posX_list.sort()  # 对posX_list从小到达排列
abs_posY_list.sort()  # 对posY_list从小到达排列

i = 0
j = 0
for y in abs_posY_list:
    for x in abs_posX_list:
        if (x, y) in tile_abs_info:
            print(f'{tile_abs_info[(x, y)]},', end='')
            tile_coor_info.setdefault((i, j), tile_abs_info[(x, y)])
            i += 1
        else:
            print('0,', end='')
            i += 1
    j += 1
    print()

print(f'{tile_abs_info[(abs_posY_list[0], abs_posX_list[0])]},')
print(f'{tile_coor_info[(25, 25)]}')
time_end = time.time()

print(f'tile矩阵生成完成，共用时{time_end - time_start}秒')