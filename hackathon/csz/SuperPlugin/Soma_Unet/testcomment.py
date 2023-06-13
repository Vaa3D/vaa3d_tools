from torch.utils.data import DataLoader
import torch
from tqdm import tqdm
import config
from dataset.predict_dataset import predict_dataset
import SimpleITK as sitk
import os
import numpy as np
from models.soma_unet import unet3d
import glob as glob
from dataset.dataset_soma import Soma_DataSet
from comment import *

def sitk_write_img(img,path):
    pred=img.cpu().detach().numpy()
    pred_v=pred[0][1]*255
    pred_v=np.array(pred_v,dtype='uint8')
    sitk.WriteImage(sitk.GetImageFromArray(pred_v),path)

def predict(model, dataset):
    predict_loader = DataLoader(dataset=dataset, batch_size=1, num_workers=0, shuffle=False)
    print(len(predict_loader))
    model.eval()
    # f = open(save_path + '/score.txt', 'w')
    with torch.no_grad():
        for idx,(data,target) in tqdm(enumerate(predict_loader),total=len(predict_loader)):
            data, target = data.float(), target.float()
            data, target = data.to(device), target.to(device)
            output = model(data)
            print(output.shape,target.shape)
            print(f1(output,target,1))



if __name__ == '__main__':
    args = config.args
    device = torch.device('cpu' if args.cpu else 'cuda')

    # model info

    model = unet3d(1, [8, 16, 32, 64, 128], 2).to(device)

    ckpt = torch.load('./output/soma/best_model.pth')
    model.load_state_dict(ckpt['net'])

    # data info
    predict_data_path = r'D:/A_beshe/'
    result_save_path=r'D:/A_beshe/pred'

    datasets=Soma_DataSet(predict_data_path, mode='train')
    predict(model,datasets)
