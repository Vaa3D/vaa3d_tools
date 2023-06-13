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
from models.myunet import unet3d
#from models.generic_UNetPlusPlus import Generic_UNetPlusPlus
#from test import test

from torch.optim.lr_scheduler import StepLR

def test(model, dataset, save_path):
    test_loader = DataLoader(dataset=dataset, batch_size=1, num_workers=0, shuffle=False)
    #print(len(test_loader))
    model.eval()
    #save_tool = Recompone_tool(save_path,filename,dataset.ori_shape,dataset.new_shape,dataset.cut)
    '''
    target = torch.from_numpy(np.expand_dims(dataset.label_np,axis=0)).long()
    target=target.unsqueeze(1)
    print(target.shape)
    torch.no_grad()
    for data in tqdm(dataloader,total=len(dataloader)):
            #data = data.unsqueeze(1)
            #print(data.shape)
        data = data.float().to(device)
        data=data.unsqueeze(1)
            #print(data.shape)
        output = model(data)
            #save_tool.add_result(output.detach().cpu())

    #pred = save_tool.recompone_overlap()
    #pred = output.unsqueeze(dim=0)
    print(output.shape)
    print(target.shape)
    val_loss = metrics.DiceMeanLoss()(output, target)
    val_dice0 = metrics.dice(output, target, 0)
    #val_dice1 = metrics.dice(pred, target, 1)
    #val_dice2 = metrics.dice(pred, target, 2)
    '''
    val_loss = 0
    val_dice0 = 0
    with torch.no_grad():
        for idx,(data, target) in tqdm(enumerate(test_loader),total=len(test_loader)):

            target = target.unsqueeze(1)
            data, target = data.float(), target.float()
            data, target = data.to(device), target.to(device)
            output = model(data)

            img = output.cpu().detach().numpy()
            img[img <= 0.5] = 0.
            img[img > 0.5] = 1.
            img = img * 255
            a = np.array(img, dtype='uint8')
            a = np.squeeze(a, axis=0)
            a = np.squeeze(a, axis=0)
            img = sitk.GetImageFromArray(a)
            sitk.WriteImage(img, os.path.join(save_path, 'result-' + dataset.filename_list[idx] ))

            loss = metrics.DiceMeanLoss()(output, target)
            #loss = metrics.cross_entropy_3D(output, target.long())
            #loss = metrics.WeightDiceLoss()(output, target)
            #loss=metrics.SoftDiceLoss()(output,target)
            dice0 = metrics.dice(output, target, 0)


            val_loss += float(loss)
            val_dice0 += float(dice0)
    val_loss /= len(test_loader)
    val_dice0 /= len(test_loader)
    '''
    pred_img = torch.argmax(output,dim=1)
    img = sitk.GetImageFromArray(np.squeeze(np.array(pred_img.numpy(),dtype='uint8'),axis=0))
    sitk.WriteImage(img, os.path.join(save_path, filename))
    '''

    # save_tool.save(filename)
    #print('\nAverage loss: {:.4f}\tdice0: {:.4f}\t\n'.format(
        #val_loss, val_dice0))
    #return val_loss, val_dice0
    return OrderedDict({'Test Loss': val_loss, 'Test dice0': val_dice0})
def val(model, val_loader):
    model.eval()
    val_loss = 0
    val_dice0 = 0

    with torch.no_grad():
        for idx,(data, target) in tqdm(enumerate(val_loader),total=len(val_loader)):

            target = target.unsqueeze(1)
            data, target = data.float(), target.float()
            data, target = data.to(device), target.to(device)
            output = model(data)

            loss = metrics.DiceMeanLoss()(output, target)
            #loss = metrics.cross_entropy_3D(output, target.long())
            #loss=metrics.SoftDiceLoss()(output,target)
            #loss = metrics.WeightDiceLoss()(output, target)
            dice0 = metrics.dice(output, target, 0)


            val_loss += float(loss)
            val_dice0 += float(dice0)


    val_loss /= len(val_loader)
    val_dice0 /= len(val_loader)


    return OrderedDict({'Val Loss': val_loss, 'Val dice0': val_dice0})


def train(modelold,modelnew, train_loader, optimizer, trainsavepath,train_num):
    print("=======Epoch:{}=======".format(epoch))
    modelold.eval()
    modelnew.train()
    train_loss = 0
    train_dice0 = 0
    for idx, (data, target) in tqdm(enumerate(train_loader),total=len(train_loader)):
        #print(target.shape)
        target=target.unsqueeze(1)

        data, target = data.float(), target.float()
        data, target = data.to(device), target.to(device)
        oldoutput = modelold(data)
        newoutput = modelnew(data)


        '''img = newoutput.cpu().detach().numpy()
        img = img*255
        a=np.array(img, dtype='uint8')
        a=np.squeeze(a,axis=1)
        #a = np.squeeze(a, axis=0)
        img2 = target.cpu().numpy()
        img2 = img2 * 255
        b = np.array(img2, dtype='uint8')
        b = np.squeeze(b, axis=1)
        img3 = data.cpu().numpy()
        img3 = img3 * 255
        c = np.array(img3, dtype='uint8')
        c = np.squeeze(c, axis=1)
        if idx<len(train_loader)-1:
            for i in range(args.batch_size):
                img = sitk.GetImageFromArray(a[i])
                sitk.WriteImage(img, os.path.join(trainsavepath, str(idx)+'-'+str(i)+'-result.tiff'))
                img2 = sitk.GetImageFromArray(b[i])
                sitk.WriteImage(img2, os.path.join(trainsavepath, str(idx)+'-'+str(i)+'-label.tiff'))
                img3 = sitk.GetImageFromArray(c[i])
                sitk.WriteImage(img3, os.path.join(trainsavepath, str(idx)+'-'+str(i)+'-rawdata.tiff'))
        else:
            for i in range(train_num % args.batch_size):
                img = sitk.GetImageFromArray(a[i])
                sitk.WriteImage(img, os.path.join(trainsavepath, str(idx) + '-' + str(i) + '-result.tiff'))
                img2 = sitk.GetImageFromArray(b[i])
                sitk.WriteImage(img2, os.path.join(trainsavepath, str(idx) + '-' + str(i) + '-label.tiff'))
                img3 = sitk.GetImageFromArray(c[i])
                sitk.WriteImage(img3, os.path.join(trainsavepath, str(idx) + '-' + str(i) + '-rawdata.tiff'))'''


        optimizer.zero_grad()
        #print('grad=%.2f, w=%.2f' % (w.grad, w.data))
        #loss = metrics.cross_entropy_3D(output,target.long())
        #loss=metrics.SoftDiceLoss()(output,target)
        #loss=torch.nn.BCELoss()(output,target)
        #loss = metrics.DiceMeanLoss()(output,target)
        loss=metrics.LwFloss()(oldoutput,newoutput,target)
        #loss=metrics.WeightDiceLoss()(output,target)
        #loss=metrics.cross_entropy_3D(output,target)

        loss.backward()
        optimizer.step()
        #scheduler.step()
        #scheduler=torch.optim.lr_scheduler.StepLR(optimizer, 1, gamma=0.1, last_epoch=-1)
        #print(optimizer.state_dict()['param_groups'][0]['lr'])
        #print("lr:",scheduler.get_lr())

        train_loss += float(loss)
        train_dice0 += float(metrics.dice(newoutput, target, 0))
    train_loss /= len(train_loader)
    train_dice0 /= len(train_loader)
    return OrderedDict({'Train Loss': train_loss, 'Train dice0': train_dice0})

if __name__ == '__main__':
    args = config.args
    save_path = os.path.join('./output', args.save)
    if not os.path.exists(save_path): os.mkdir(save_path)
    device = torch.device('cpu' if args.cpu else 'cuda')
    # data info
    train_set = Soma_DataSet( args.dataset_path, mode='train')
    train_num=len(train_set.filename_list)
    val_set = Soma_DataSet( args.dataset_path, mode='val')
    train_loader = DataLoader(dataset=train_set,batch_size=args.batch_size,num_workers=1, shuffle=True)
    val_loader = DataLoader(dataset=val_set,batch_size=args.batch_size,num_workers=1, shuffle=True)
    # model info
    #model = UNet(1, [16, 32, 48, 64, 96], 1, net_mode='3d',conv_block=RecombinationBlock).to(device)
    modelold = unet3d(1, [8,16,32,64,128],1).to(device)
    modelnew = unet3d(1, [8,16,32,64,128],1).to(device)
    ckpt = torch.load('./output/{}/best_model_s.pth'.format(args.save))
    modelold.load_state_dict(ckpt['net'])
    modelnew.load_state_dict(ckpt['net'])
    #model=Generic_UNetPlusPlus()
    optimizer = optim.Adam(modelnew.parameters(), lr=args.lr)
    #w = torch.tensor(data=[100], dtype=torch.float32, requires_grad=True)
    #optimizer=optim.SGD(model.parameters(), lr=args.lr, momentum=args.momentum)
    #scheduler = torch.optim.lr_scheduler.StepLR(optimizer, step_size=5, gamma=0.1)
    #print(optimizer.state_dict()['param_groups'][0]['lr'])
    #init_util.print_network(model)
    #model = nn.DataParallel(model, device_ids=[0,1])  # multi-GPU
    #test_data_path = r'D:/A_DLcsz/DLtest/soma_test/'
    result_save_path = r'D:/A_DLcsz/score'
    #datasets = test_soma_dataset(test_data_path)
    trainsavepath=r'D:/A_DLcsz/newresult'

    log = logger.Logger(save_path)

    best = [0,np.inf] # 初始化最优模型的epoch和performance
    trigger = 0  # early stop 计数器
    for epoch in range(1, args.epochs + 1):
        common.adjust_learning_rate(optimizer, epoch, args)
        train_log = train(modelold ,modelnew, train_loader, optimizer, trainsavepath,train_num)
        #print("lr:", scheduler.get_lr())
        val_log = val(modelnew, val_loader)
        #test_log=test(modelnew, datasets,result_save_path)
        log.update(epoch,train_log,val_log)

        # Save checkpoint.
        state = {'net': modelnew.state_dict(),'optimizer':optimizer.state_dict(),'epoch': epoch}
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