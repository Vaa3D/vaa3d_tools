"""
在测试集目录中进行测试，给出性能评价指标和可视化结果
"""

from dataset.dataset_soma import Soma_DataSet
from torch.utils.data import DataLoader
import torch
import torch.optim as optim
from tqdm import tqdm
import config
from models.Unet import UNet, RecombinationBlock
from utilsa import logger, init_util, metrics,common
import os
import numpy as np
from collections import OrderedDict
import SimpleITK as sitk
from models.soma_unet import unet3d
#from test import test
from dataset.predict_dataset import predict_dataset
from torch.optim.lr_scheduler import StepLR

def predict(model, dataset, save_path):
    predict_loader = DataLoader(dataset=dataset, batch_size=1, num_workers=0, shuffle=False)

    model.eval()

    with torch.no_grad():
        for idx,(data) in tqdm(enumerate(predict_loader),total=len(predict_loader)):

            data = data.float()
            data = data.to(device)
            output = model(data)

            img = output.cpu().detach().numpy()
            img = img * 65535
            a = np.array(img, dtype='uint16')
            a = np.squeeze(a, axis=0)
            a = np.squeeze(a, axis=0)
            img = sitk.GetImageFromArray(a)
            sitk.WriteImage(img, os.path.join(save_path, 'result-' + str(idx) + '.nii.gz'))
    return 0

if __name__ == '__main__':
    args = config.args
    device = torch.device('cpu' if args.cpu else 'cuda')

        # model info
    #model = UNet(1, [32, 48, 64, 96, 128], 2, net_mode='3d',conv_block=RecombinationBlock).to(device)
    model = unet3d(1, [8,16,32,64,128]).to(device)
    ckpt = torch.load('./output/{}/best_model.pth'.format(args.save))
    model.load_state_dict(ckpt['net'])

    # data info
    predict_data_path = r'D:/DLtest/soma_test/'
    #result_save_path = r'./output/{}/result'.format(args.save)
    result_save_path=r'D:\predictcsz'
    if not os.path.exists(result_save_path): os.mkdir(result_save_path)
    datasets = predict_dataset(predict_data_path)

    #for dataset,file_idx in datasets:
    predict(model, datasets,result_save_path)