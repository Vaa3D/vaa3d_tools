from xml.etree import ElementTree
import time
import numpy as np
import matplotlib.pyplot as plt

time_start = time.time()

xml_path = 'F:/ZhaoHuImages/AI_denoise/MetaData/TileScan 1_Properties.xml'


with open(xml_path) as f:
    tree = ElementTree.parse(f)

abs_posX_set = set()  # Sets the X-axis absolute position parameter set for removing the same
abs_posY_set = set()  # Sets the Y-axis absolute position parameter set for removing the same
tile_abs_info = {}  # The dictionary for tile information
coors = []
for node in tree.iter('Tile'):  # Iterate over nodes labeled 'tiles' in the XML document
    tile_label = int(node.attrib.get('FieldX'))
    abs_posX = node.attrib.get('PosX')
    abs_posY = node.attrib.get('PosY')
    line_coor = [tile_label, abs_posX, abs_posY]
    coors.append(line_coor)
coors=np.array(coors,dtype='double')


plt.scatter(coors[0:2366,1],coors[0:2366,2]*(-1),marker='.')
plt.xlim(0.023, 0.034)
plt.ylim(-0.042, -0.030)
for i in range(2366):
    plt.text(coors[i][1],coors[i][2]*(-1),int(coors[i][0]),fontsize=8)
plt.show()
print(coors)