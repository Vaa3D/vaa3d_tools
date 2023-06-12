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
#from models.generic_UNetPlusPlus import Generic_UNetPlusPlus
#from test import test

from torch.optim.lr_scheduler import StepLR

def save_debug_file(save_path,img,lab,pred,idx,epoch,mode):
    if os.path.exists(save_path) is False:
        os.mkdir(save_path)
    img_v=img.cpu().detach().numpy()[0][0]*255
    lab_v=lab.cpu().detach().numpy()[0]*255
    pred_v=pred.cpu().detach().numpy()[0][1]*255
    img_v=np.array(img_v,dtype='uint8')
    lab_v = np.array(lab_v, dtype='uint8')
    pred_v = np.array(pred_v, dtype='uint8')
    if mode=='train':
        out_img_file = f'debug_train_epoch_{epoch}_{idx}_img.tiff'
        out_lab_file = f'debug_train_epoch_{epoch}_{idx}_lab.tiff'
        out_pred_file = f'debug_train_epoch_{epoch}_{idx}_pred.tiff'
    elif mode=='val':
        out_img_file = f'debug_val_epoch_{epoch}_{idx}_img.tiff'
        out_lab_file = f'debug_val_epoch_{epoch}_{idx}_lab.tiff'
        out_pred_file = f'debug_val_epoch_{epoch}_{idx}_pred.tiff'
    sitk.WriteImage(sitk.GetImageFromArray(img_v), os.path.join(save_path, out_img_file))
    sitk.WriteImage(sitk.GetImageFromArray(lab_v), os.path.join(save_path, out_lab_file))
    sitk.WriteImage(sitk.GetImageFromArray(pred_v), os.path.join(save_path, out_pred_file))

def test(model, dataset, save_path):
    test_loader = DataLoader(dataset=dataset, batch_size=1, num_workers=0, shuffle=False)
    model.eval()

    val_loss = 0
    val_dice0 = 0
    val_dice1 =0
    with torch.no_grad():
        for idx,(data, target) in tqdm(enumerate(test_loader),total=len(test_loader)):

            data, target = data.float(), target.float()
            data, target = data.to(device), target.to(device)
            output = model(data)

            img = output.cpu().detach().numpy()
            img[img <= 0.5] = 0.
            img[img > 0.5] = 1.
            img = img * 255
            a = np.array(img, dtype='uint8')
            a = np.squeeze(a, axis=0)
            #a = np.squeeze(a, axis=0)
            img = sitk.GetImageFromArray(a[0])
            sitk.WriteImage(img, os.path.join(save_path, 'result-' + dataset.filename_list[idx] + '.tiff'))

            #loss = metrics.DiceMeanLoss()(output, target)
            #loss = metrics.cross_entropy_3D(output, target.long())
            loss = metrics.WeightDiceLoss()(output, target)
            #loss=metrics.SoftDiceLoss()(output,target)
            dice0 = metrics.dice(output, target, 0)
            dice1 = metrics.dice(output, target, 1)


            val_loss += float(loss)
            val_dice0 += float(dice0)
            val_dice1 += float(dice1)
    val_loss /= len(test_loader)
    val_dice0 /= len(test_loader)
    val_dice1 /= len(test_loader)

    return OrderedDict({'Test Loss': val_loss, 'Test dice0': val_dice0,'Test dice1': val_dice1})

def val(model, val_loader,epoch):
    model.eval()
    val_loss = 0
    val_ces = 0
    val_dices = 0

    with torch.no_grad():
        for idx,(data, target) in tqdm(enumerate(val_loader),total=len(val_loader)):

            data, target = data.float(), target.float()
            data, target = data.to(device), target.to(device)
            output = model(data)

            if epoch%10==1 and idx%10==0:
                save_debug_file(args.debug,data,target,output,idx,epoch,mode='val')

            crit_ce = torch.nn.CrossEntropyLoss(reduction='none').to(device)
            crit_dice = metrics.BinaryDiceLoss(smooth=1e-5, input_logits=False).to(device)
            loss_ces, loss_dices, loss, logits = metrics.get_forward_eval(output, target, crit_ce, crit_dice, model)


            val_loss += float(loss.item())
            val_ces += float(loss_ces[0])
            val_dices += float(loss_dices[0])



    val_loss /= len(val_loader)
    val_ces /= len(val_loader)
    val_dices /= len(val_loader)


    return OrderedDict({'Val Loss': val_loss, 'Val ces': val_ces,'Val dices': val_dices})


def train(model, train_loader, optimizer, epoch):
    print("=======Epoch:{}=======".format(epoch))
    model.train()
    train_loss = 0
    train_ces = 0
    train_dices = 0

    for idx, (data, target) in tqdm(enumerate(train_loader),total=len(train_loader)):


        data, target = data.float(), target.float()
        data, target = data.to(device), target.to(device)
        output = model(data)

        if epoch % 10 == 1 and idx % 10 == 0:
            save_debug_file(args.debug, data, target, output, idx, epoch,mode='train')

        optimizer.zero_grad()

        crit_ce = torch.nn.CrossEntropyLoss(reduction='none').to(device)
        crit_dice = metrics.BinaryDiceLoss(smooth=1e-5, input_logits=False).to(device)
        loss_ces, loss_dices, loss, logits = metrics.get_forward(output, target, crit_ce, crit_dice, model)
        loss.backward()
        torch.nn.utils.clip_grad_norm_(model.parameters(), 12)
        optimizer.step()

        train_loss+= float(loss.item())
        train_ces += float(loss_ces[0])
        train_dices += float(loss_dices[0])

    train_loss /= len(train_loader)
    train_ces /= len(train_loader)
    train_dices /= len(train_loader)


    return OrderedDict({'Train Loss': train_loss, 'Train ces': train_ces,'Train dices': train_dices})

if __name__ == '__main__':
    args = config.args
    save_path = os.path.join('./output', args.save)
    if not os.path.exists(save_path): os.mkdir(save_path)
    device = torch.device('cpu' if args.cpu else 'cuda')
    # data info
    train_set = Soma_DataSet( args.dataset_path, mode='train')
    val_set = Soma_DataSet( args.dataset_path, mode='val')
    train_loader = DataLoader(dataset=train_set,batch_size=args.batch_size,num_workers=1, shuffle=True)
    val_loader = DataLoader(dataset=val_set,batch_size=args.batch_size,num_workers=1, shuffle=True)
    # model info
    #model = UNet(1, [16, 32, 48, 64, 96], 1, net_mode='3d',conv_block=RecombinationBlock).to(device)
    model = unet3d(1, [8,16,32,64,128],2,debug=False).to(device) #2021.10.8

    # modelpath='./output/soma_noflip/best_model.pth'
    # if os.path.exists(modelpath):
    #     ckpt = torch.load(modelpath)
    #     model.load_state_dict(ckpt['net'])
    #     print("load the pth!")
    optimizer = optim.Adam(model.parameters(), lr=args.lr)

    pytorch_total_params = sum(p.numel() for p in model.parameters())
    pytorch_total_required_params = sum(p.numel() for p in model.parameters() if p.requires_grad)
    print(pytorch_total_params,pytorch_total_required_params)
    # optimizer=optim.SGD(model.parameters(), lr=args.lr,weight_decay=args.weight_decay, momentum=args.momentum,nesterov=args.nesterov)

    #init_util.print_network(model)

    result_save_path = r'D:/A_DLcsz/morpredict'

    trainsavepath=r'D:/A_result/ldresult'
    trainnum=len(train_set)
    log = logger.Logger(save_path)

    best = [0,np.inf] # 初始化最优模型的epoch和performance
    trigger = 0  # early stop 计数器
    for epoch in range(1, args.epochs + 1):
        common.adjust_learning_rate(optimizer, epoch, args)
        for param_group in optimizer.param_groups:
            print(param_group['lr'])
        train_log = train(model, train_loader, optimizer, epoch)

        val_log = val(model, val_loader,epoch)

        log.update(epoch,train_log,val_log)

        # Save checkpoint.
        state = {'net': model.state_dict(),'optimizer':optimizer.state_dict(),'epoch': epoch}
        torch.save(state, os.path.join(save_path, 'latest_model.pth'))
        trigger += 1
        if val_log['Val Loss'] < best[1]:
            print('Saving best model')
            torch.save(state, os.path.join(save_path, 'best_model.pth'))
            best[0] = epoch
            best[1] = val_log['Val Loss']
            trigger = 0
        print('Best performance at Epoch: {} | {}'.format(best[0],best[1]))
        # early stopping
        if args.early_stop is not None:
            if trigger >= args.early_stop:
                print("=> early stopping")
                break
        torch.cuda.empty_cache()