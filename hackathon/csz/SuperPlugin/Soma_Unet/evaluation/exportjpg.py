import glob as glob
import numpy as np
import SimpleITK as sitk
import os

def gen_review(raw_path,seg_path,review_path):
    raw = sitk.ReadImage(raw_path, sitk.sitkInt8)
    raw_array = sitk.GetArrayFromImage(raw)

    seg = sitk.ReadImage(seg_path, sitk.sitkInt8)
    seg_array = sitk.GetArrayFromImage(seg)

    z,x,y=raw_array.shape
    print(raw_array.shape)

    raw_review=np.hstack((raw_array[int(z/2),:,:],raw_array[:,int(x/2),:]))
    raw_review=np.hstack((raw_review,raw_array[:,:,int(y/2)]))

    seg_review = np.hstack((seg_array[int(z / 2), :, :], seg_array[:, int(x / 2), :]))
    seg_review = np.hstack((seg_review, seg_array[:, :, int(y / 2)]))

    review=np.vstack((raw_review,seg_review))

    review=np.array(review,dtype='uint8')

    new_review = sitk.GetImageFromArray(review)
    sitk.WriteImage(new_review, review_path)



def main():
    #raw_path="E:/soma_seg/17783/data/17783_1150_9467_5913_crop.tiff"
    #seg_path="E:/soma_seg/17783/label/17783_seg_1150_9467_5913.tiff"
    '''files=glob.glob("D:/A_predictcsz/Crop_image_1st_res_128/*")
    seg_path="D:/A_predictcsz/new/"
    for file in files:
        somafile=file.split("\\")[-1]
        somaid=somafile.split(".tif")[0]
        segfile_path=seg_path+"result-"+str(somafile)+".tiff"
        #print(segfile_path)
        gen_review(file,segfile_path,somaid)'''
    #gen_review(raw_path,seg_path)
    raw_path = "E:/soma_img_crop_uint8/soma_img_crop_uint8/"
    seg_path="E:/soma_img_crop_uint8/seg_postprocess/"
    fixed_path = "E:/review/"
    file_path = glob.glob(raw_path + '*')
    for file in file_path:
        file_name = file.split("\\")[-1]  #brainid file
        review_files = glob.glob(file + '/*') #somaid files
        #print(file_name)
        for review_file in review_files:
            review_file_name=review_file.split('\\')[-1]
            if review_file_name=="predict_name_list.txt":
                continue
            gen_review(raw_path+file_name+'/'+review_file_name,seg_path+file_name+'/seg_'+review_file_name,fixed_path+file_name+'/'+review_file_name+".jpg")
            #print(fixed_path+'/'+file_name+'/'+review_file_name)
        #if not os.path.exists(fixed_path + '/' + file_name): os.mkdir(fixed_path + '/' + file_name)




if __name__ == '__main__':
    main()