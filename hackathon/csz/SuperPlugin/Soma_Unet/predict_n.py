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

def sitk_write_img(img,path):
    pred=img.cpu().detach().numpy()
    pred_v=pred[0][1]*255
    pred_v=np.array(pred_v,dtype='uint8')
    sitk.WriteImage(sitk.GetImageFromArray(pred_v),path)

def predict(model, dataset, save_path):
    predict_loader = DataLoader(dataset=dataset, batch_size=1, num_workers=0, shuffle=False)
    print(len(predict_loader))
    model.eval()
    # f = open(save_path + '/score.txt', 'w')
    with torch.no_grad():
        for idx,(data) in tqdm(enumerate(predict_loader),total=len(predict_loader)):
            data= data.float()

            data= data.to(device)
            output = model(data)
            sitk_write_img(output,os.path.join(save_path, 'seg_' + dataset.filename_list[idx] ))


if __name__ == '__main__':
    args = config.args
    device = torch.device('cpu' if args.cpu else 'cuda')

    # model info

    model = unet3d(1, [8, 16, 32, 64, 128], 2).to(device)

    ckpt = torch.load('./output/soma_crop64/best_model.pth')
    model.load_state_dict(ckpt['net'])

    # data info
    predict_data_path = r'D:\A_DLcsz\DLtrain\fixed_data\data'
    result_save_path=r'D:\A_DLcsz\DLtrain\fixed_data\uniontrain'

    datasets=predict_dataset(predict_data_path+'/')
    predict(model,datasets,result_save_path)
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


    #for dataset,file_idx in datasets:
    #predict(model, datasets,result_save_path)