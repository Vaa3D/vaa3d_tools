
"""
Load and save v3draw stack files that are supported by V3D
by Lei Qu
20200330
"""
import SimpleITK as sitk
import numpy as np
import struct
import os


def load_v3d_raw_img_file1(filename):
    im = {}

    f_obj = open(filename, 'rb')

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
        return im

    im_data = im_data.reshape((size[3], size[2], size[1], size[0]))
    # print(im_data.shape)
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


def save_v3d_raw_img_file1(im, filename):
    if filename[-4:] != '.raw' and filename[-7:] != '.v3draw':
        print("ERROR: Unsupportted file format. return!")


    f_obj = open(filename, 'wb')

    # write image header - formatkey(24bytes)
    formatkey = b'raw_image_stack_by_hpeng'
    formatkey = struct.pack('<24s', formatkey)
    f_obj.write(formatkey)

    # write image header - endianCode(1byte)
    endiancode = b'L'
    endiancode = struct.pack('<s', endiancode)
    f_obj.write(endiancode)

    # write image header - datatype(2bytes)
    datatype = im['datatype']
    datatype = struct.pack('<h', datatype)  # 'h' = short
    f_obj.write(datatype)

    # write image header - size(4*4bytes)
    size = im['size']
    size = struct.pack('<4l', size[1], size[0], size[2], size[3])  # 'l' = long
    f_obj.write(size)

    # write image data
    im_data = im['data']

    im_data = np.moveaxis(im_data, -2, 0)
    im_data = np.moveaxis(im_data, -1, 0)
    # print(im_data.shape)
    im_data = im_data.tobytes()
    f_obj.write(im_data)
    f_obj.close()
    
    
    