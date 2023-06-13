#-*- coding: utf-8 -*
import os
from os import path
import hashlib
import requests



path = "C:/Users/penglab/Downloads/Vaa3D_V3.458_Windows_MSVC_64bit/vaa3d_msvc.exe"
input_path = "C:/Users/penglab/Desktop/1/input"
output_path = "C:/Users/penglab/Desktop/1/output"
os.system(path + " /x 5D_Stack_Converter /f 4D_to_5D /i " + input_path + "/test.tif /o " + output_path + "/5dstackconverter /p 1")

os.system(path + " /x threshold /f adath /i " + input_path + "/test.tif /o " + output_path + "/Simple_Adaptive_Thresholding.v3draw /p 1")

os.system(path + " /x affine_transform_swc /f apply_transform /i " + input_path + "/test.swc /o " + output_path + "/transformed.swc /p 1")

os.system(path + " /x eraser /f cae /i " + input_path + "/test.tif /o " + output_path + "/canvas_eraser.v3draw /p 1 1 1")

os.system(path + " /x N3DFix /f N3DFix /i " + input_path + "/test.swc /o " + output_path + "/N3DFix.swc /p 0")

os.system(path + " /x eraser /f cae /i /Users/jazz/Downloads/1/test.tif /o /Users/jazz/Downloads/1/output/canvas_eraser.v3draw /p 1 1 1")

os.system(path + " /x gvf_cellseg  /f  gvf_segmentation /i " + input_path + "/test.tif /o " + output_path + "/cellseg_gvf.v3draw /p 1")

os.system(path + " /x updatepxlval /f change_pixel_value /i " + input_path + "/test.raw /o " + output_path + "/change_pixel_value.v3draw /p 1 2 3 0")

os.system(path + " /x channelsplit /f split_extract_channels /i " + input_path + "/test.tif /o " + output_path + "/channel_split.v3draw /p 0")

os.system(path + " /x neuron_fragment_extractor /f label_components /i " + input_path + "/test.swc /o " + output_path + "/connect_neuron_fragments_extractor.swc")

os.system(path + " /x convert_file_format /f convert_format /i " + input_path + "/test.tif /o " + output_path + "/convert_file_format.v3draw")

os.system(path + " /x Convert_Image_to_AtlasViewMode /f toatlas /i " + input_path + "/test.tif /o " + output_path + "")

os.system(path + " /x datatypeconvert /f dtc /i " + input_path + "/test.v3draw /o " + output_path + "/datatype_cnvrt.v3draw")

os.system(path + " /x Fast_Distance_Transform /f dt /i " + input_path + "/test.tif /o " + output_path + "/dt.v3draw")

os.system(path + " /x Edge_Extraction_from_Mask_Image /f find_edge /i " + input_path + "/test.v3draw /o " + output_path + "/edge_of_masking.v3draw")

os.system(path + " /x eswc_converter /f swc_to_eswc  /i " + input_path + "/test.swc /o " + output_path + "/eswc_converter.eswc")

os.system(path + " /x extract_Z_Slices /f subzslices2stack /i " + input_path + "/test.v3draw /o " + output_path + "/extract_z.v3draw /p 1:3:e")

os.system(path + " /x gaussian /f gf /i " + input_path + "/test.tif /o " + output_path + "/gaussianfilter.v3draw")

os.system(path + " /x gsdt /f gsdt /i " + input_path + "/test.tif /o " + output_path + "/gsdt.v3draw /p 1")

os.system(path + " /x hier_label /f hierachical_labeling /i " + input_path + "/test.swc /o " + output_path + "/hierachical.eswc")

os.system(path + " /x histogram /f histogram /i " + input_path + "/test.tif /o " + output_path + "/histogram.csv")

os.system(path + " /x pruning_swc_simple /f pruning_simple /i " + input_path + "/test.swc /o " + output_path + "/pruning.swc /p 10")

os.system(path + " /x recenterimage /f iRecenter /i " + input_path + "/test.v3draw /o " + output_path + "/recenterimage.v3draw")

os.system(path + " /x resample_swc /f resample_swc /i " + input_path + "/test.swc /o " + output_path + "/resample.swc /p 1")

os.system(path + " /x image_resample /f up_sample /i " + input_path + "/test.tif /o " + output_path + "/up_sample.v3draw")

os.system(path + " /x intensity_rescale /f rescale /i " + input_path + "/test.tif /o " + output_path + "/rescale.v3draw /p 0")

os.system(path + " /x rotate /f left90 /i " + input_path + "/test.v3draw /o " + output_path + "/rotateimg.v3draw")

os.system(path + " /x sort_neuron_swc /f sort_swc /i " + input_path + "/test.swc /o " + output_path + "/sort_neuron_swc.swc /p 1")

os.system(path + " /x standardize /f standardize /i " + input_path + "/test.swc " + input_path + "/sort_neuron_swc.swc  /o " + output_path + "/standardize.swc /p 10 1")

os.system(path + " /x swc_to_maskimage_sphere_unit /f swc_to_maskimage /i " + input_path + "/test.swc /o " + output_path + "/swc2maskimage.raw")

os.system(path + " /x swc_to_maskimage_cylinder_unit /f swc2mask /i " + input_path + "/test.swc /o " + output_path + "/swc2cylinder.swc")

os.system(path + " /x TipDetection /f tracing_func /i " + input_path + "/test.tif /p 1")

os.system(path + " /x laplacian /f laplacian /i " + input_path + "/test.tif /o " + output_path + "/laplacian.v3draw")

os.system(path + " /x medianfilter /f fixed_window /i " + input_path + "/test.tif /o " + output_path + "/median.v3draw")

os.system(path + " /x minMaxfilter /f mmf /i " + input_path + "/test.tif /o " + output_path + "/minmax.v3draw")

os.system(path + " /x mipZSlices /f mip_zslices /i " + input_path + "/test.tif  /o " + output_path + "/mip_zslices.v3draw /p 1:1:e")

os.system(path + " /x montage /f rmtg /i " + input_path + "/test_zslice.v3draw /o " + output_path + "/montage.v3draw /p 1")

os.system(path + " /x multi_channel_swc /f render_swc_by_eswc /i " + input_path + "/test.swc /o " + output_path + "/multi_channel.eswc")

os.system(path + " /x muitiscaleEnhancement /f adaptive_auto /i " + input_path + "/test.tif /o " + output_path + "/multiscale.v3draw")

os.system(path + " /x neuron_connector /f connect_neuron_SWC /i " + input_path + "/test.swc /o " + output_path + "/neuron_connector.swc")

os.system(path + " /x neuron_image_profiling /f profile_swc /i " + input_path + "/test.v3draw " + input_path + "/test.swc /o " + output_path + "/neuron_image_profiling.csv")

os.system(path + " /x neuron_populator /f populate /i " + input_path + "/test.swc")

os.system(path + " /x calculate_reliability_score /f calculate_score /i " + input_path + "/test.tif " + input_path + "/test.swc /o " + output_path + "/neuron_reliability.txt")

os.system(path + " /x SimpleTracing /f tracing /i " + input_path + "/test.tif  /o " + output_path + "/simpletracing.swc")

os.system(path + " /x smartTrace /f smartTrace /i " + input_path + "/test.tif")

os.system(path + " /x Vaa3D_Neuron2 /f app2 /i " + input_path + "/test.tif /o " + output_path + "/neurontracevn2.swc")

#比较部分代码
# 定义一个函数
def scaner_file(url):
    # 遍历当前路径下所有文件
    list = []
    file = os.listdir(url)
    for f in file:
        # 字符串拼接
        real_url = path.join(url, f)
        # 打印出来
        list.append(real_url)
    return list



def readfile(url):
    with open(url,'rb') as f:
        content = f.read(-1)
        return hashlib.md5(content).hexdigest()


qt4 = scaner_file("/Users/jazz/Downloads/1 2/qt4 test/output")
sorted(qt4)

qt6 = scaner_file("/Users/jazz/Downloads/1 2/qt6 test/output")
sorted(qt6)

print(len(qt4))
print(len(qt6))
for i in range(len(qt4)):
    a = readfile(qt4[i])
    b = readfile(qt6[i])
    if a == b:
        print('文件内容一致')
    else:
        print("---------------------------")
        print(qt4[i])
        print(qt6[i])

# fromlines = readfile('/Users/jazz/Desktop/testVaa3D/testData/output/multiscale.v3draw')
# tolines = readfile('/Users/jazz/Desktop/testVaa3D/testData/output/multi_channel.eswc')
# if tolines == fromlines:
#     print('文件内容一致')
# else:
#     print('文件内容不一致')