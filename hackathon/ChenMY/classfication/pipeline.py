from crop_image_batch import *
from  predict import *
from optparse import OptionParser
import shutil
def get_args():
    parser = OptionParser()
    parser.add_option('-v', '--v3dExePath', dest='v3dExePath',

                       help='vaa3d_msvc.exe path.')
    parser.add_option('-c', '--cropped3DImageSeriesPath', dest='cropped3DImageSeriesPath',

                      help='cropped3DImageSeries.dll path.')
    parser.add_option('-m', '--mipZSlicesPath', dest='mipZSlicesPath',

                      help='mipZSlices.dll path.')
    parser.add_option('-p', '--pics_folder', dest='pics_folder',

                      help='Which folder the pics save')
    parser.add_option('-s', '--soma_candidates_path', dest='soma_candidates_path',

                      help='soma candidates path')
    parser.add_option('-t', '--terafly_folder', dest='terafly_folder',

                      help='Which folder the terafly save')
    # parser.add_option('-t', '--truedir', dest='truedir',
    #
    #                   help='Which folder the true pics save.')
    # parser.add_option('-f', '--falsedir', dest='falsedir',
    #
    #                   help='Which folder the false pics save.')
    # parser.add_option('-a', '--apopath', dest='apopath',
    #
    #                   help='set the apo path')
    (options, args) = parser.parse_args()

    return options
if __name__ == "__main__":
    args = get_args()
    v3dExePath=args.v3dExePath# set v3dExePath
    cropped3DImageSeriesPath=args.cropped3DImageSeriesPath#set cropped3DImageSeriesPath
    mipZSlicesPath=args.mipZSlicesPath#set mipZSlicesPath
    # step1 generate apo file
    path = args.pics_folder  # set pics saved folder
    if not os.path.exists(path):
        os.mkdir(path)
    wholebrainntxt = args.soma_candidates_path  # set soma candidates ".txt"path
    saveapo = "wholebrain1.apo"
    multiple = 1
    createApo(wholebrainntxt, saveapo, multiple)
    saveapo2 = "wholebrain2.apo"
    multiple2 = 2
    createApo(wholebrainntxt, saveapo2, multiple2)
    # step2  generate two folders
    path128 = "128_128_64"
    v3draw = "v3draw"
    twotif = "2DTif"
    twopng = "png"
    if os.path.isdir(path) and not os.path.exists(path + "\\" + path128):
        os.mkdir(os.path.join(path, path128))
    if os.path.isdir(path) and not os.path.exists(path + "\\" + path128 + "\\" + v3draw):
        os.mkdir(os.path.join(path + "\\" + path128 + "\\", v3draw))
    if os.path.isdir(path) and not os.path.exists(path + "\\" + path128 + "\\" + twotif):
        os.mkdir(os.path.join(path + "\\" + path128 + "\\", twotif))
    if os.path.isdir(path) and not os.path.exists(path + "\\" + path128 + "\\" + twopng):
        os.mkdir(os.path.join(path + "\\" + path128 + "\\", twopng))
    # step3 genenate apo
    somapath=os.path.dirname(wholebrainntxt) + "\\" + saveapo2
    print(somapath)
    # step 4 genenate v3draw
    teraflyFolder = args.terafly_folder # set teraflyFolder path
    v3drawpath=path + "\\" + path128 + "\\"+ v3draw
    crop3Ddraw( v3dExePath,cropped3DImageSeriesPath,teraflyFolder, somapath, v3drawpath, 128, 128, 64)
    # setp 5 generate tif
    tifpath = path + "\\" + path128 + "\\" + twotif
    crop2DTif(v3dExePath,mipZSlicesPath,v3drawpath,tifpath)
    # step 6 generate png
    pngpath= path + "\\" + path128 + "\\" + twopng
    tif_to_png(tifpath, pngpath)
    infolder = pngpath  # set the cropped pics folder
    truefolder="true"
    falsefolder="false"
    if os.path.isdir(path) and not os.path.exists(path + "\\" + path128 + "\\" + twopng+"\\"+truefolder):
        os.mkdir(os.path.join(path + "\\" + path128 + "\\"+twopng+"\\",truefolder))
    if os.path.isdir(path) and not os.path.exists(path + "\\" + path128 + "\\" + twopng+"\\"+falsefolder):
        os.mkdir(os.path.join(path + "\\" + path128 + "\\"+twopng+"\\",falsefolder))
    outfolder0 =path + "\\" + path128 + "\\"+twopng+"\\"+truefolder
    outfolder1 = path + "\\" + path128 + "\\"+twopng+"\\"+falsefolder
    savePath = wholebrainntxt+"\\..\\"+"true_whilebrain.apo"  # set the trueapo path
    predict(infolder, outfolder0, outfolder1)
    xyz = picToapo(outfolder0)
    createApo1(xyz, savePath, 1, 40000, 255, 255, 0)
    if os.path.exists(wholebrainntxt+"\\..\\"+"wholebrain1.apo"):
       os.remove(wholebrainntxt+"\\..\\"+"wholebrain1.apo")
    else:
       print("wholebrain1.apo doesn't exist")
    if os.path.exists(wholebrainntxt+"\\..\\"+"wholebrain2.apo"):
       os.remove(wholebrainntxt+"\\..\\"+"wholebrain2.apo")
    else:
       print("wholebrain2.apo doesn't exist")
    if os.path.exists(path + "\\" + path128):
        shutil.rmtree(path + "\\" + path128)