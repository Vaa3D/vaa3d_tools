import numpy as np
import os
import SimpleITK as sitk
import random
from scipy import ndimage
from process.transform3d import *#RandomFlip,RandomRotate90,RandomRotate,RandomContrast,ElasticDeformation,AdditiveGaussianNoise
from utilsa.common import *


class augmentation:
    def __init__(self, raw_dataset_path, fixed_dataset_path):
        self.raw_root_path = raw_dataset_path
        self.fixed_path = fixed_dataset_path

        if not os.path.exists(self.fixed_path):  # 创建保存目录
            os.makedirs(self.fixed_path + 'data/')
            os.makedirs(self.fixed_path + 'label/')

        #self.fix_data()
        # self.rdflip()
        # self.rdrotate90()
        # self.rdrotate()
        ##self.rdcontrast()
        # self.ElDe()
        # self.addgauss()
        # self.addpoisson()
        #self.crop()
        #self.cropto8()
        #self.write_train_val_test_name_list()  # 创建索引txt文件

    def fix_data(self):
        print('the raw dataset total numbers of samples is :', len(os.listdir(self.raw_root_path+ 'data')))
        for data_file in os.listdir(self.raw_root_path+ 'data/'):
            #print(data_file)
            data = sitk.ReadImage(os.path.join(self.raw_root_path + 'data/', data_file), sitk.sitkInt8)
            data_array = sitk.GetArrayFromImage(data)
            #data_array=norm_img(data_array)
            #data_array*=255
            seg = sitk.ReadImage(os.path.join(self.raw_root_path + 'label/', 'seg_'+data_file),
                                 sitk.sitkInt8)
            seg_array = sitk.GetArrayFromImage(seg)

            #seg_array=np.array(seg_array,dtype='uint8')

            if (data_array.shape != (128, 128, 128)):
                print(data_file)
            if (seg_array.shape != (128, 128, 128)):
                print(data_file + '_seg')

            #seg_array=norm_img(seg_array)
            #seg_array=-seg_array*255

            #print(data_array.shape, seg_array.shape)

            seg_array = np.flip(seg_array, axis=1)
            '''a = np.max(data_array)
            b = np.min(data_array)
            if (a>255 or b<0):
                print(a, " ", b,' ',data_file)'''
            #a=np.max(seg_array)
            #b=np.min(seg_array)
            #if (a > 255 or b < 0):
            #print(a," ",b)

            new_data = sitk.GetImageFromArray(data_array)
            new_seg = sitk.GetImageFromArray(seg_array)

            sitk.WriteImage(new_data, os.path.join(self.fixed_path + 'data/', data_file))
            sitk.WriteImage(new_seg,
                            os.path.join(self.fixed_path + 'label/', 'seg_'+data_file))

    def rdflip(self):
        print('the raw dataset total numbers of samples is :', len(os.listdir(self.raw_root_path+ 'data')))
        for data_file in os.listdir(self.raw_root_path+ 'data/'):
            print(data_file)
            data = sitk.ReadImage(os.path.join(self.raw_root_path + 'data/', data_file), sitk.sitkInt8)
            data_array = sitk.GetArrayFromImage(data)

            seg = sitk.ReadImage(os.path.join(self.raw_root_path + 'label/', 'seg_'+data_file),
                                 sitk.sitkInt8)
            seg_array = sitk.GetArrayFromImage(seg)
            #seg_array = -seg_array * 255

            print(data_array.shape, seg_array.shape)
            random_state=np.random.RandomState()
            data_array,seg_array=RandomFlip(random_state)(data_array,seg_array)
            #print(data_array)

            new_data = sitk.GetImageFromArray(data_array)
            new_seg = sitk.GetImageFromArray(seg_array)

            sitk.WriteImage(new_data, os.path.join(self.fixed_path + 'data/', 'rdflip-'+data_file))
            sitk.WriteImage(new_seg,
                            os.path.join(self.fixed_path + 'label/', 'seg_rdflip-'+data_file))

    def rdrotate90(self):
        print('the raw dataset total numbers of samples is :', len(os.listdir(self.raw_root_path + 'data')))
        for data_file in os.listdir(self.raw_root_path + 'data/'):
            print(data_file)
            data = sitk.ReadImage(os.path.join(self.raw_root_path + 'data/', data_file), sitk.sitkInt8)
            data_array = sitk.GetArrayFromImage(data)

            seg = sitk.ReadImage(os.path.join(self.raw_root_path + 'label/', 'seg_' + data_file),
                                     sitk.sitkInt8)
            seg_array = sitk.GetArrayFromImage(seg)
            #seg_array = -seg_array * 255


            print(data_array.shape, seg_array.shape)
            random_state=np.random.RandomState()
            data_array,seg_array = RandomRotate90(random_state)(data_array, seg_array)

            new_data = sitk.GetImageFromArray(data_array)

            new_seg = sitk.GetImageFromArray(seg_array)

            sitk.WriteImage(new_data, os.path.join(self.fixed_path + 'data/', 'rdrotate90-' + data_file))
            sitk.WriteImage(new_seg,
                            os.path.join(self.fixed_path + 'label/', 'seg_rdrotate90-' + data_file))

    def rdrotate(self):
        print('the raw dataset total numbers of samples is :', len(os.listdir(self.raw_root_path+ 'data')))
        for data_file in os.listdir(self.raw_root_path+ 'data/'):
            print(data_file)
            data = sitk.ReadImage(os.path.join(self.raw_root_path + 'data/', data_file), sitk.sitkInt8)
            data_array = sitk.GetArrayFromImage(data)

            seg = sitk.ReadImage(os.path.join(self.raw_root_path + 'label/', 'seg_'+data_file),
                                 sitk.sitkInt8)
            seg_array = sitk.GetArrayFromImage(seg)
            #seg_array = -seg_array * 255

            print(data_array.shape, seg_array.shape)
            random_state=np.random.RandomState()
            data_array,seg_array=RandomRotate(random_state)(data_array,seg_array)
            #seg_array=RandomRotate(random_state)(seg_array)
            #print(data_array)

            new_data = sitk.GetImageFromArray(data_array)
            new_seg = sitk.GetImageFromArray(seg_array)

            sitk.WriteImage(new_data, os.path.join(self.fixed_path + 'data/', 'rdrotate-'+data_file))
            sitk.WriteImage(new_seg,
                            os.path.join(self.fixed_path + 'label/', 'seg_rdrotate-'+data_file))

    def ElDe(self):#弹性形变
        print('the raw dataset total numbers of samples is :', len(os.listdir(self.raw_root_path+ 'data')))
        for data_file in os.listdir(self.raw_root_path+ 'data/'):
            print(data_file)
            data = sitk.ReadImage(os.path.join(self.raw_root_path + 'data/', data_file), sitk.sitkInt8)
            data_array = sitk.GetArrayFromImage(data)

            seg = sitk.ReadImage(os.path.join(self.raw_root_path + 'label/', 'seg_'+data_file),
                                 sitk.sitkInt8)
            seg_array = sitk.GetArrayFromImage(seg)
            seg_array = -seg_array * 255

            print(data_array.shape, seg_array.shape)
            data_array=norm_img(data_array)
            seg_array=norm_img(seg_array)
            random_state=np.random.RandomState()
            data_array,seg_array=ElasticDeformation(random_state,spline_order=3)(data_array,seg_array)

            data_array = data_array * 255
            seg_array=seg_array*255
            #seg_array=np.flip(seg_array,axis=1)
            data_array = np.array(data_array, dtype='uint8')
            seg_array=np.array(seg_array, dtype='uint8')

            new_data = sitk.GetImageFromArray(data_array)
            new_seg = sitk.GetImageFromArray(seg_array)

            sitk.WriteImage(new_data, os.path.join(self.fixed_path + 'data/', 'elde-'+data_file))
            sitk.WriteImage(new_seg,
                            os.path.join(self.fixed_path + 'label/', 'seg_elde-'+data_file))

    def addgauss(self):
        print('the raw dataset total numbers of samples is :', len(os.listdir(self.raw_root_path+ 'data')))
        for data_file in os.listdir(self.raw_root_path+ 'data/'):
            #print(data_file)
            data = sitk.ReadImage(os.path.join(self.raw_root_path + 'data/', data_file), sitk.sitkInt8)
            data_array = sitk.GetArrayFromImage(data)

            seg = sitk.ReadImage(os.path.join(self.raw_root_path + 'label/', 'seg_'+data_file),
                                 sitk.sitkInt8)
            seg_array = sitk.GetArrayFromImage(seg)
            #seg_array = np.flip(seg_array, axis=1)
            #seg_array = -seg_array * 255
            #seg_array = np.array(seg_array, dtype='uint8')
            #seg_array = np.flip(seg_array, axis=1)
            data_array=norm_img(data_array)

            #print(data_array.shape, seg_array.shape)
            '''if(data_array.shape!=(128,128,128)):
                print(data_file)
            if (seg_array.shape != (128, 128, 128)):
                print(data_file+'_seg')'''
            random_state=np.random.RandomState()
            data_array=AdditiveGaussianNoise(random_state)(data_array)
            #print(data_array)

            data_array=data_array*255
            data_array=np.array(data_array, dtype='uint8')
            new_data = sitk.GetImageFromArray(data_array)
            new_seg = sitk.GetImageFromArray(seg_array)

            sitk.WriteImage(new_data, os.path.join(self.fixed_path + 'data/', 'adgauss-'+data_file))
            sitk.WriteImage(new_seg,
                            os.path.join(self.fixed_path + 'label/', 'seg_adgauss-'+data_file))

    def addpoisson(self):
        print('the raw dataset total numbers of samples is :', len(os.listdir(self.raw_root_path+ 'data')))
        for data_file in os.listdir(self.raw_root_path+ 'data/'):
            print(data_file)
            data = sitk.ReadImage(os.path.join(self.raw_root_path + 'data/', data_file), sitk.sitkInt8)
            data_array = sitk.GetArrayFromImage(data)

            seg = sitk.ReadImage(os.path.join(self.raw_root_path + 'label/', 'seg_'+data_file),
                                 sitk.sitkInt8)
            seg_array = sitk.GetArrayFromImage(seg)
            #seg_array = np.flip(seg_array, axis=1)
            #seg_array=-seg_array*255
            #seg_array = np.array(seg_array, dtype='uint8')
            #seg_array = np.flip(seg_array, axis=1)
            data_array=norm_img(data_array)

            print(data_array.shape, seg_array.shape)
            '''if (data_array.shape != (128, 128, 128)):
                print(data_file)
            if (seg_array.shape != (128, 128, 128)):
                print(data_file+'_seg')'''
            random_state=np.random.RandomState()
            data_array=AdditivePoissonNoise(random_state)(data_array)
            #print(data_array)

            data_array=data_array*255
            data_array=np.array(data_array, dtype='uint8')
            new_data = sitk.GetImageFromArray(data_array)
            new_seg = sitk.GetImageFromArray(seg_array)

            sitk.WriteImage(new_data, os.path.join(self.fixed_path + 'data/', 'adpoisson-'+data_file))
            sitk.WriteImage(new_seg,
                            os.path.join(self.fixed_path + 'label/', 'seg_adpoisson-'+data_file))

    def rdcontrast(self):
        print('the raw dataset total numbers of samples is :', len(os.listdir(self.raw_root_path+ 'data')))
        for data_file in os.listdir(self.raw_root_path+ 'data/'):
            print(data_file)
            data = sitk.ReadImage(os.path.join(self.raw_root_path + 'data/', data_file), sitk.sitkInt8)
            data_array = sitk.GetArrayFromImage(data)

            seg = sitk.ReadImage(os.path.join(self.raw_root_path + 'label/', 'seg_'+data_file),
                                 sitk.sitkInt8)
            seg_array = sitk.GetArrayFromImage(seg)
            seg_array = -seg_array * 255

            print(data_array.shape, seg_array.shape)
            random_state=np.random.RandomState()
            data_array=RandomContrast(random_state)(data_array)
            #seg_array=RandomContrast(random_state)(seg_array)
            #print(data_array)

            new_data = sitk.GetImageFromArray(data_array)
            new_seg = sitk.GetImageFromArray(seg_array)

            sitk.WriteImage(new_data, os.path.join(self.fixed_path + 'data/', 'rdcontrast-'+data_file))
            sitk.WriteImage(new_seg,
                            os.path.join(self.fixed_path + 'label/', 'seg_rdcontrast-'+data_file))

    def crop(self):
        print('the raw dataset total numbers of samples is :', len(os.listdir(self.raw_root_path+ 'data')))
        for data_file in os.listdir(self.raw_root_path+ 'data/'):
            print(data_file)
            data = sitk.ReadImage(os.path.join(self.raw_root_path + 'data/', data_file), sitk.sitkInt8)
            data_array = sitk.GetArrayFromImage(data)

            seg = sitk.ReadImage(os.path.join(self.raw_root_path + 'label/', 'seg_'+data_file),
                                 sitk.sitkInt8)
            seg_array = sitk.GetArrayFromImage(seg)
            seg_array = -seg_array * 255
            #seg_array=np.flip(seg_array,axis=1)

            print(data_array.shape, seg_array.shape)
            random_state=np.random.RandomState()
            data_array,seg_array=CropToFixed(random_state)(data_array,seg_array)
            #print(data_array)
            #a = np.max(data_array)
            #b = np.min(data_array)
            #print(a, " ", b)
            #a = np.max(seg_array)
            #b = np.min(seg_array)
            #print(a, " ", b)

            new_data = sitk.GetImageFromArray(data_array)
            new_seg = sitk.GetImageFromArray(seg_array)

            sitk.WriteImage(new_data, os.path.join(self.fixed_path + 'data/', 'crop-'+data_file))
            sitk.WriteImage(new_seg,
                            os.path.join(self.fixed_path + 'label/', 'seg_crop-'+data_file))

    def cropto8(self):
        print('the raw dataset total numbers of samples is :', len(os.listdir(self.raw_root_path+ 'data')))
        for data_file in os.listdir(self.raw_root_path+ 'data/'):
            print(data_file)
            data = sitk.ReadImage(os.path.join(self.raw_root_path + 'data/', data_file), sitk.sitkInt8)
            data_array = sitk.GetArrayFromImage(data)
            data_array=np.flip(data_array,axis=1)
            seg = sitk.ReadImage(os.path.join(self.raw_root_path + 'label/', 'seg_'+data_file),
                                 sitk.sitkInt8)
            seg_array = sitk.GetArrayFromImage(seg)
            seg_array = -seg_array * 255
            seg_array=np.flip(seg_array,axis=1)

            a = np.max(data_array)
            b = np.min(data_array)
            print(a, " ", b)
            a = np.max(seg_array)
            b = np.min(seg_array)
            print(a, " ", b)

            print(data_array.shape, seg_array.shape)
            a=data_array
            b=seg_array
            a=a.reshape(2,2,64,32,32)
            b=b.reshape(2,2,64,32,32)
            for i in range(2):
                i0=i*32
                for j in range(2):
                    j0=j*32
                    a[i][j] = data_array[:, i0:i0 + 32, j0:j0 + 32]
                    b[i][j] = seg_array[:, i0:i0 + 32, j0:j0 + 32]
            for i in range(2):
                #i = i * 64
                for j in range(2):
                    #j = j * 64
                    new_data = sitk.GetImageFromArray(a[i][j])
                    new_seg = sitk.GetImageFromArray(b[i][j])
                    #new_data=new_data[:,:,:]
                    if i==0:
                        sitk.WriteImage(new_data, os.path.join(self.fixed_path + 'data/', 'crop-' + str(2)+ '-'+str(j + 1)+ '-'+ data_file ))
                    else:
                        sitk.WriteImage(new_data, os.path.join(self.fixed_path + 'data/',
                                                               'crop-' + str(1) + '-' + str(j + 1) + '-' + data_file))
                    sitk.WriteImage(new_seg, os.path.join(self.fixed_path + 'label/', 'seg_crop-' + str(i + 1) + '-' + str(j + 1) + '-' + data_file))
            '''
            for i in range(2):
                i0 = i * 40
                for j in range(2):
                    j0 = j * 40
                    b[i][j] = seg_array[:,i0:i0 + 40, j0:j0 + 40]
            for i in range(2):
                    # i = i * 64
                for j in range(2):
                        # j = j * 64
                    new_seg = sitk.GetImageFromArray(b[i][j])
                    sitk.WriteImage(new_seg,
                                    os.path.join(self.fixed_path + 'label/', 'seg_crop-' + str(i+1)+ '-'+str(j+1)+'-'+data_file))
                #new_data = sitk.GetImageFromArray(a[i])
            #new_seg = sitk.GetImageFromArray(seg_array)
            #for i in range(8):
                #sitk.WriteImage(new_data, os.path.join(self.fixed_path + 'data/', 'crop-'+str(i+1)+'-'+data_file))
            #sitk.WriteImage(new_seg,
                            #os.path.join(self.fixed_path + 'label/', 'seg_crop-'+data_file))
            '''

    def write_train_val_test_name_list(self):
        data_name_list = os.listdir(self.fixed_path + "data")
        data_num = len(data_name_list)
        print('the fixed dataset total numbers of samples is :', data_num)
        random.shuffle(data_name_list)

        train_rate = 1.
        val_rate = 0

        assert val_rate + train_rate == 1.0
        train_name_list = data_name_list[0:int(data_num * train_rate)]
        #val_name_list = data_name_list[int(data_num * train_rate):int(data_num * (train_rate + val_rate))]

        self.write_name_list(train_name_list, "train_name_list.txt")
        #self.write_name_list(val_name_list, "val_name_list.txt")

    def write_name_list(self, name_list, file_name):
        f = open(self.fixed_path + file_name, 'w')
        for i in range(len(name_list)):
            f.write(str(name_list[i]) + "\n")
        f.close()

def main():
    raw_dataset_path = 'D:/A_DLcsz/DLtrain/fixed_data/'
    fixed_dataset_path = 'D:/A_DLcsz/test/'

    #tiff_fix(raw_dataset_path, fixed_dataset_path)
    augmentation(raw_dataset_path, fixed_dataset_path)

if __name__ == '__main__':
    main()
