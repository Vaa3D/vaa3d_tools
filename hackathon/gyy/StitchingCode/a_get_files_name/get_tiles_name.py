from xml.etree import ElementTree
import time
import json

"""
Output a json file.

input_folder: Path of the folder where the MIP image is located required for subsequent calculation of offsets.
z_result_file: The path to save the JSON file with the Z shift is calculated.
xy_result_file: The path to save the JSON file with the XY shift is calculated.
locations: The relative position of all tiles.
x_length: Pixel length in X direction per slice.
y_length: Pixel length in Y direction per slice.
shift_x_P: The initial pixel size of the overlapping area of adjacent tiles set in the left and right relationship.
shift_y_P: The initial pixel size of the overlapping area of adjacent tiles set in the up and down relationship.
shift_x_P_d: Variation range of overlap in adjacent tiles settings of the left and right relationship.
shift_y_P_d: Variation range of shift in adjacent tiles settings of the left and right relationship.
shift_x_P_d_: Variation range of overlap in adjacent tiles settings of the up and down relationship.
shift_y_P_d_: Variation range of shift in adjacent tiles settings of the up and down relationship.
shift_z_P_d_: The Z shift range of the adjacent tiles.
thread_num: The number of threads set to run the program.

"""


time_start = time.time()

xml_path = 'E:/Zhaohu0418/MetaData/TileScan 1_Properties.xml'
# <Tile FieldX="1" FieldY="0" PosX="0.0271577952" PosY="0.0306397943" />
# xml_path = input('input xml path：')
result_folder = 'F:/ZhaoHuImages/AI_denoise/Zhaohu_StitchCode/tiles_position.json'

next_input = 'F:/ZhaoHuImages/MIP_results'
z_next_result = 'F:/Zhaohu_StitchCode/Z_shift.json'
xy_next_result = 'F:/Zhaohu_StitchCode/XY_shift.json'

with open(xml_path) as f:
    tree = ElementTree.parse(f)

abs_posX_set = set()  # Sets the X-axis absolute position parameter set for removing the same
abs_posY_set = set()  # Sets the Y-axis absolute position parameter set for removing the same
tile_abs_info = {}  # The dictionary for tile information
for node in tree.iter('Tile'):  # Iterate over nodes labeled 'tiles' in the XML document
    tile_label = int(node.attrib.get('FieldX'))
    abs_posX = node.attrib.get('PosX')
    abs_posY = node.attrib.get('PosY')
    abs_posX_set.add(abs_posX)
    abs_posY_set.add(abs_posY)
    tile_abs_info.setdefault((abs_posX, abs_posY), tile_label)  # 向字典中添加tile信息

abs_posX_list = list(abs_posX_set)  # convert posX_set to a list
abs_posY_list = list(abs_posY_set)  # convert posY_set to a list
abs_posX_list.sort()  # arrange posX_list from smallest to largest
abs_posY_list.sort()  # arrange posY_list from smallest to largest

locations=[]
for y in abs_posY_list:
    location = []
    for x in abs_posX_list:
        if (x, y) in tile_abs_info:
            # print(f'{tile_abs_info[(x, y)]},', end='')
            location.append(f's{str(tile_abs_info[(x, y)]).zfill(4)}')
        else:
            # print('0,', end='')
            location.append('None')
    locations.append(location)
    # print()

result = {
    'input_folder': next_input,
    'z_result_file': z_next_result,
    'xy_result_file': xy_next_result,
    'locations': locations,
    'x_length': 512,
    'y_length': 512,
    'shift_x_P': 51,
    'shift_y_P': 51,
    'shift_x_P_d': 20,
    'shift_y_P_d': 20,
    'shift_x_P_d_': 20,
    'shift_y_P_d_': 20,
    'shift_z_P_d_': 30,
    'thread_num': 10
}
open(result_folder, 'w')
with open(result_folder, 'w') as f:
    json.dump(result, f, indent=4, ensure_ascii=False)

# time_end = time.time()
# print('spend time: ', time_end - time_start)
