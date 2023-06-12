import os
import json

import torch
from PIL import Image
from torchvision import transforms
import matplotlib.pyplot as plt

from model import AlexNet
import SimpleITK as sitk
from dataset import norm_img

import numpy as np
from joblib import load
import pickle

def main():
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

    data_transform = transforms.Compose(
        [transforms.Resize((224, 224)),
         transforms.ToTensor(),
         transforms.Normalize((0.5, 0.5, 0.5), (0.5, 0.5, 0.5))])

    # load image
    img_path = "../tulip.jpg"
    assert os.path.exists(img_path), "file: '{}' dose not exist.".format(img_path)
    img = Image.open(img_path)

    plt.imshow(img)
    # [N, C, H, W]
    img = data_transform(img)
    # expand batch dimension
    img = torch.unsqueeze(img, dim=0)

    # read class_indict
    json_path = './class_indices.json'
    assert os.path.exists(json_path), "file: '{}' dose not exist.".format(json_path)

    with open(json_path, "r") as f:
        class_indict = json.load(f)

    # create model
    model = AlexNet(num_classes=7).to(device)

    # load model weights
    weights_path = "./AlexNet_e_50_tvt_721_leakyrelu.pth"
    assert os.path.exists(weights_path), "file: '{}' dose not exist.".format(weights_path)
    model.load_state_dict(torch.load(weights_path))

    model.eval()
    with torch.no_grad():
        # predict class
        output = torch.squeeze(model(img.to(device))).cpu()
        predict = torch.softmax(output, dim=0)
        predict_cla = torch.argmax(predict).numpy()

    print_res = "class: {}   prob: {:.3}".format(class_indict[str(predict_cla)],
                                                 predict[predict_cla].numpy())
    plt.title(print_res)
    for i in range(len(predict)):
        print("class: {:10}   prob: {:.3}".format(class_indict[str(i)],
                                                  predict[i].numpy()))
    plt.show()

def alexnetpredict(imgpath):
    img=sitk.ReadImage(imgpath,sitk.sitkUInt8)
    imgdata=sitk.GetArrayFromImage(img)

    if imgdata.shape!=(128,128,128):
        # imgdata=np.resize(imgdata,(128,128,128))
        temp = np.zeros((128, 128, 128))
        temp[0:imgdata.shape[0], 0:imgdata.shape[1], 0:imgdata.shape[2]] = imgdata
        imgdata = temp

    imgdata=norm_img(imgdata)
    imgdata = torch.from_numpy(imgdata)

    imgdata = torch.unsqueeze(imgdata, dim=0)
    imgdata = torch.unsqueeze(imgdata, dim=0)
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    # create model
    model = AlexNet(num_classes=5).to(device)

    # load model weights
    weights_path = "./AlexNet.pth"
    assert os.path.exists(weights_path), "file: '{}' dose not exist.".format(weights_path)
    model.load_state_dict(torch.load(weights_path))

    model.eval()
    with torch.no_grad():
        # predict class
        output = torch.squeeze(model(imgdata.float().to(device))).cpu()
        predict = torch.softmax(output, dim=0)
        predict_cla = torch.argmax(predict)
    return predict_cla

def svmpredict(quality):
    qlist=quality.split(',')[:-1]
    for i in range(len(qlist)):
        if qlist[i]=='inf':
            qlist[i]=1e12
        if qlist[i]=='nan':
            qlist[i]=1e12
    X=np.array([qlist],dtype='float')
    X=X[:,1:]
    with open('pca_model.pkl', 'rb') as f:
        pca = pickle.load(f)
        X = pca.transform(X)


    clf=load("svmmodel_raw_ovo_poly.joblib")
    pred=clf.predict(X)
    return pred

if __name__ == '__main__':
    # main()
    alexnetpredict(r"D:\A_DLcsz\DLrawdata\18455\label\seg_ImgSoma_18455_00018-x_13075.1_y_9223.3_z_5938.7.tiff")
