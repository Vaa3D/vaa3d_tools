import struct
def load_v3d_raw_img_file(filename):
    im = {}
    try:
        f_obj = open(filename, 'rb')
    except FileNotFoundError:
        print("ERROR: Failed in reading [" + filename + "], Exit.")
        f_obj.close()
        return im
    else:
        # read image header - formatkey(24bytes)
        len_formatkey = len('raw_image_stack_by_hpeng')
        formatkey = f_obj.read(len_formatkey)
        formatkey = struct.unpack(str(len_formatkey) + 's', formatkey)
        if formatkey[0] != b'raw_image_stack_by_hpeng':
            print("ERROR: File unrecognized (not raw, v3draw) or corrupted.")
            f_obj.close()
            return im

        # read image header - endianCode(1byte)
        endiancode = f_obj.read(1)
        endiancode = struct.unpack('c', endiancode)  # 'c' = char
        endiancode = endiancode[0]
        if endiancode != b'B' and endiancode != b'L':
            print("ERROR: Only supports big- or little- endian,"
                  " but not other format. Check your data endian.")
            f_obj.close()
            return im

        # read image header - datatype(2bytes)
        datatype = f_obj.read(2)
        if endiancode == b'L':
            datatype = struct.unpack('<h', datatype)  # 'h' = short
        else:
            datatype = struct.unpack('>h', datatype)  # 'h' = short
        datatype = datatype[0]
        if datatype < 1 or datatype > 4:
            print("ERROR: Unrecognized data type code [%d]. "
                  "The file type is incorrect or this code is not supported." % (datatype))
            f_obj.close()
            return im

        # read image header - size(4*4bytes)
        size = f_obj.read(4 * 4)
        if endiancode == b'L':
            size = struct.unpack('<4l', size)  # 'l' = long
        else:
            size = struct.unpack('>4l', size)  # 'l' = long
        # print(size)

        # read image data
        npixels = size[0] * size[1] * size[2] * size[3]
        im_data = f_obj.read()
        if datatype == 1:
            im_data = np.frombuffer(im_data, np.uint8)
        elif datatype == 2:
            im_data = np.frombuffer(im_data, np.uint16)
        else:
            im_data = np.frombuffer(im_data, np.float32)
        if len(im_data) != npixels:
            print("ERROR: Read image data size != image size. Check your data.")
            f_obj.close()
            return im

        im_data = im_data.reshape((size[3], size[2], size[1], size[0]))
        # print(im_data.shape)
        im_data = np.moveaxis(im_data, 0, -1)
        # print(im_data.shape)
        im_data = np.moveaxis(im_data, 0, -2)
        # print(im_data.shape)
    f_obj.close()

    im['endian'] = endiancode
    im['datatype'] = datatype
    im['size'] = im_data.shape
    im['data'] = im_data

    return im


import numpy as np
import json


yyannov3d = load_v3d_raw_img_file(r"F:\QualityControlProject\registration\annotation\Ex_488_Em_525_8bit_10_9_annotation25_16bit.v3draw")
yyanno = yyannov3d['data'].copy()
with open(r"F:\QualityControlProject\registration\annotation\annotation25_16bit_id_mapping.json") as f:
    anno_id_map = json.load(f)
with open(r"F:\QualityControlProject\registration\annotation\tree.json") as f:
    tree = json.load(f)
for key in anno_id_map.keys():
    yyanno[yyanno==int(key)] = int(anno_id_map.get(key))
brain_region_list = ["FRP","SS","AUD","VIS","ACA","PL","ILA","ORB","AI","RSP","PTLp","TEa","ECT"] + ["IB"] + ["MB"]

lutnametoid = dict(zip([t["acronym"] for t in tree], [t["id"] for t in tree]))
lutidtoname = dict(zip([t["id"] for t in tree], [t["acronym"] for t in tree]))
def find_children(id_):
    idlist = []
    for t in tree:
        if id_ in t["structure_id_path"]:
            idlist.append(t['id'])
    if not idlist:
        idlist = [id_]
    return idlist
brain_region_idmapping = {}
for br in brain_region_list:
    brain_region_idmapping[br] = np.array(find_children(lutnametoid.get(br)))
