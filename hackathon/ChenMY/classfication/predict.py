# import the necessary packages
from keras.preprocessing.image import img_to_array
from keras.models import load_model
import numpy as np
import argparse
import imutils
import cv2
import os
from optparse import OptionParser

norm_size = 64


# def args_parse():
#     # construct the argument parse and parse the arguments
#     ap = argparse.ArgumentParser()
#     ap.add_argument("-m", "--model", required=True,
#                     help="path to trained model model")
#     ap.add_argument("-i", "--image", required=True,
#                     help="path to input image")
#     ap.add_argument("-s", "--show", action="store_true",
#                     help="show predict image", default=False)
#     args = vars(ap.parse_args())
#     return args


def predict(infolder,outfolder0,outfolder1):
    # load the trained convolutional neural network
    print("[INFO] loading network...")
    # model = load_model(args["model"])
    model = load_model("3brian.model")

    for file in os.listdir(infolder):
        if file[-3:]=='png':
            print(file)
            # load the image
            # image = cv2.imread(args["image"])
            image=cv2.imread(infolder+"\\"+file)
            orig = image.copy()

            # pre-process the image for classification
            image = cv2.resize(image, (norm_size, norm_size))
            image = image.astype("float") / 255.0
            image = img_to_array(image)
            image = np.expand_dims(image, axis=0)

            # classify the input image
            result = model.predict(image)[0]
            # print (result.shape)
            proba = np.max(result)
            if proba>result[1] and proba<=0.55:
                number=1
            else :
                number=np.where(result == proba)[0]
            label = str(number)
            label = "{}: {:.2f}%".format(label, proba * 100)
            print(label)

            # if args['show']:
            # draw the label on the image
            output = imutils.resize(orig, width=400)

            cv2.putText(output, label, (10, 25), cv2.FONT_HERSHEY_SIMPLEX,
                        0.7, (0, 255, 0), 2)
            if int(number)==0:
                cv2.imwrite(outfolder0+"\\"+file,output);
            else:
                cv2.imwrite(outfolder1 + "\\" + file, output);
            # # show the output image
            # cv2.imshow("Output", output)
            # cv2.waitKey(0)
def createApo1(list,saveapo,multiple,id,color0,color1,color2):
    outfile = open(saveapo, 'w')
    outfile.write("##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b\n")
    lines=len(list)
    for i in range(lines):
        outfile.write("%d, ,  %d,, %d,%d,%d, 0,0,0,50,0,,,,%d,%d,%d\n" % (
        i + 1+id, i + 1+id, (list[i][2])*multiple, (list[i][0])*multiple, (list[i][1])*multiple,color0,color1,color2))
    print("create apo ok")

def picToapo(picfolder):
    xyz=[]
    filename=os.listdir(picfolder)
    for file in filename:
        if os.path.splitext(file)[1]==".png":
            name0=os.path.splitext(file)[0]
            x=int(name0.split('_')[0])
            y = int(name0.split('_')[1])
            z = int(name0.split('_')[2])
            xyz.append([x,y,z])
    return xyz

def createAno(apoPath,anoPath):
     filename=os.path.basename(apoPath)
     file=open(os.path.dirname(apoPath)+"\\"+ anoPath,"w")
     file.write("APOFILE={}\n".format(filename))
     file.write("SWCFILE=wholebrain_stamp_2019_04_17_15_53.ano.eswc")

# def get_args():
#     parser = OptionParser()
#     parser.add_option('-s', '--savedir', dest='savedir',
#
#                        help='Which folder the pics save.')
#     parser.add_option('-t', '--truedir', dest='truedir',
#
#                       help='Which folder the true pics save.')
#     parser.add_option('-f', '--falsedir', dest='falsedir',
#
#                       help='Which folder the false pics save.')
#     parser.add_option('-a', '--apopath', dest='apopath',
#
#                       help='set the apo path')
#     (options, args) = parser.parse_args()
#
#     return options
#
#
# if __name__ == '__main__':
#     args = get_args()
#     infolder = args.savedir#set the cropped pics folder
#     outfolder0=args.truedir# set the predicted true folder
#     outfolder1=args.falsedir#set  the predicted false floder
#     savePath = args.apopath # set the trueapo path
#     predict(infolder, outfolder0, outfolder1)
#     xyz = picToapo(outfolder0)
#     createApo(xyz, savePath, 1, 40000, 255, 255, 0)




