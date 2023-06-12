#from dataset.dataset_soma import Soma_DataSet
from torch.utils.data import DataLoader
import torch
from tqdm import tqdm
import config
#from utilsa import logger, init_util, metrics,common
from dataset.predict_dataset import predict_dataset
import SimpleITK as sitk
import os
import numpy as np
from models.soma_unet import unet3d
import glob as glob
#from utilsa.common import load_file_name_list
#from collections import OrderedDict
#import matplotlib.pyplot as plt
from scipy.ndimage import zoom

def write_name_list(self, name_list, file_name,score):
    f = open(self.fixed_path + file_name, 'w')
    for i in range(len(name_list)):
        f.write(str(name_list[i]) +'    '+score +"\n")
    f.close()

def predict(model, dataset, save_path):
    predict_loader = DataLoader(dataset=dataset, batch_size=1, num_workers=0, shuffle=False)
    print(len(predict_loader))
    model.eval()
    # f = open(save_path + '/score.txt', 'w')
    with torch.no_grad():
        for idx,(data) in tqdm(enumerate(predict_loader),total=len(predict_loader)):
            init=data.float()
            data= data.float()

            data= data.to(device)
            output = model(data)
            img = output.cpu().detach().numpy()


            img[img <= 0.5] = 0.
            img[img > 0.5] = 1.
            img = img * 255
            #print(img.shape)
            a = np.array(img, dtype='uint8')
            a = np.squeeze(a, axis=0)

            pred=a[0]

            pred=np.array(pred,dtype='uint8')

            img = sitk.GetImageFromArray(pred)

            sitk.WriteImage(img, os.path.join(save_path, 'seg_' + dataset.filename_list[idx] ))

def getstart(data):

    outshape=data.shape
    # if data.shape[0]%32!=0:
    #     x=data.shape[0]+(32-data.shape[0]%32)
    # else:
    #     x=data.shape[0]
    # if data.shape[1]%32!=0:
    #     y=data.shape[1]+(32-data.shape[1]%32)
    # else:
    #     y=data.shape[1]
    # if data.shape[2]%32!=0:
    #     z=data.shape[2]+(32-data.shape[2]%32)
    # else:
    #     z=data.shape[2]
    # newdata=np.zeros((x,y,z))
    # newdata[:outshape[0],:outshape[1],:outshape[2]]=data
    newdata=zoom(data,(128/data.shape[0],256/data.shape[1],256/data.shape[2]))
    args = config.args
    device = torch.device('cpu' if args.cpu else 'cuda')
    model = unet3d(1, [8, 16, 32, 64, 128], 1).to(device)  # 2021.10.13
    ckpt = torch.load('./output/best_model_0825.pth')
    model.load_state_dict(ckpt['net'])
    model.eval()
    with torch.no_grad():
        print(newdata.shape)
        newdata = np.expand_dims(newdata, axis=0)
        newdata = np.expand_dims(newdata, axis=0)
        newdata=torch.from_numpy(newdata.copy())
        newdata=newdata.float()
        newdata = newdata.to(device)

        output = model(newdata)
        img = output.cpu().detach().numpy()
        img[img <= 0.5] = 0.
        img[img > 0.5] = 1.
        img = img * 255
        img = np.array(img, dtype='uint8')
        img = np.squeeze(img, axis=0)
        img = np.squeeze(img, axis=0)
        print(img.shape)
        pred = np.array(img, dtype='uint8')
        pred=zoom(pred,(outshape[0]/pred.shape[0],outshape[1]/pred.shape[1],outshape[2]/pred.shape[2]))
        image = sitk.GetImageFromArray(pred)
        image=sitk.BinaryFillhole(image)
        # 查找所有连通域并分配标签
        label_image = sitk.ConnectedComponent(image)
        label_image = sitk.RelabelComponent(label_image)

        # 保存每个连通域
        statistics_filter = sitk.LabelShapeStatisticsImageFilter()
        statistics_filter.Execute(label_image)

        number_of_labels = statistics_filter.GetNumberOfLabels() + 1
        # if number_of_labels>5:
        #     return
        print(number_of_labels)
        maxlabel=1
        maxsize=0
        for label in range(1, number_of_labels):
            size=statistics_filter.GetNumberOfPixels(label)
            if size>maxsize:
                maxlabel=label
                maxsize=size
        maxcomponent=sitk.Equal(label_image,maxlabel)
        cent=statistics_filter.GetCentroid(maxlabel)
        result=[]
        result.append(cent[0])
        result.append(outshape[1]-cent[1])
        result.append(cent[2])
        return result

if __name__ == '__main__':
    args = config.args
    device = torch.device('cpu' if args.cpu else 'cuda')

        # model info
    #model = UNet(1, [32, 48, 64, 96, 128], 2, net_mode='3d',conv_block=RecombinationBlock).to(device)
    # model = unet3d(1,[4,8,16,32,64],1).to(device)
    model = unet3d(1, [8, 16, 32, 64, 128], 1).to(device) #2021.10.13
    #ckpt = torch.load('./output/{}/best_model.pth'.format(args.predict)) #2021.10.13 model:resizeadd result_path:'D:/A_predictcsz/bad_predict'
    ckpt = torch.load('./output/best_model_0825.pth')
    model.load_state_dict(ckpt['net'])

    # data info
    predict_data_path = r'D:\A_DLcsz\DLtrain\fixed_data\data'
    #result_save_path = r'./output/{}/result'.format(args.save)
    #result_save_path=r'D:/A_predictcsz/augmor_predict'
    result_save_path=r'D:\A_DLcsz\DLtrain\fixed_data\finetune'

    '''if not os.path.exists(result_save_path): os.mkdir(result_save_path)
    datasets = predict_dataset(predict_data_path)'''
    # predict_root_files=glob.glob(predict_data_path)
    # for predict_root_file in predict_root_files:
    #     print(predict_root_file)
    #     seg_root_filename=predict_root_file.split('\\')[-1]
    #     result_final_save_path=result_save_path+'/'+seg_root_filename
    #     if not os.path.exists(result_final_save_path):
    #         os.mkdir(result_final_save_path)
    #     print(result_final_save_path)
    #     datasets=predict_dataset(predict_root_file+'/')
    #     #print(result_final_save_path)
    #     predict(model,datasets,result_final_save_path)

    datasets=predict_dataset(predict_data_path+'/')

    predict(model,datasets,result_save_path)
    #for dataset,file_idx in datasets:
    #predict(model, datasets,result_save_path)