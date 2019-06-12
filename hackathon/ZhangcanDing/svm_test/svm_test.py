#!/usr/bin/env python
# coding: utf-8

# In[ ]:





# In[120]:


from sklearn import svm
import SimpleITK as sitk
import numpy as np
import random



# reader= sitk.ImageFileReader()
# reader.SetImageIO("TIFFImageIO")
# reader.SetFileName("/home/penglab/Ding/18455_00127/new_startpoints/011_x_13600.6_y_7614.94_z_1344.marker_tmp_APP2/x_13536_y_7550_z_1328.tif")
# image = reader.Execute();

# writer= sitk.ImageFileWriter()
# writer.SetFileName("/home/penglab/Ding/18455_00127/new_startpoints/011_x_13600.6_y_7614.94_z_1344.marker_tmp_APP2/test.tif")
# writer.Execute(image)

def load_itk(filename):
    itkimage= sitk.ReadImage(filename)
    # Convert the image to a  numpy array first and then shuffle the dimensions to get axis in the order z,y,x
    scan= sitk.GetArrayFromImage(itkimage)
    #origin= np.array(list(reversed(itkimage.GetOrigin())))
    # Read the spacing along each dimension
    #spacing= np.array(list(reversed(itkimage.GetSpacing())))
    return scan 
#, origin, spacing

#def feature_extract(scan)
    

#print (load_itk("/home/penglab/Desktop/weak_signal_retrace_sample/011_x_13600.6_y_7614.94_z_1344.marker_tmp_APP2/x_13536_y_7550_z_1328.tif"))
scan0=load_itk("/home/penglab/Desktop/weak_signal_retrace_sample/011_x_13600.6_y_7614.94_z_1344.marker_tmp_APP2/x_13536_y_7550_z_1328.tif")
#print (scan0.shape)

scan1= load_itk ("/home/penglab/Desktop/weak_signal_retrace_sample/011_x_13600.6_y_7614.94_z_1344.marker_tmp_APP2/output_swc_mask.tif")
#print (scan1.shape[0])
#print (scan1[5,1,1])

X=[]
y=[]
all_feature=[]

for i in range(scan1.shape[0]):
    for j in range(scan1.shape[1]):
        for k in range(scan1.shape[2]):
                    if i==0 or j==0 or k==0 or i==scan1.shape[0]-1 or j==scan1.shape[1]-1 or k==scan1.shape[2]-1:
                        all_feature.append([scan0[i,j,k],scan0[i,j,k]])
                    elif scan1[i,j,k]==255:             
                        ave_neighbor=  int ((scan0[i-1,j,k]+scan0[i+1,j,k]+scan0[i,j-1,k]+scan0[i,j+1,k]+scan0[i,j,k-1]+scan0[i,j,k+1])/6)
                        all_feature.append([scan0[i,j,k],ave_neighbor])
                        X.append([scan0[i,j,k],ave_neighbor])
                        y.append(255)
                    else:
                        ave_neighbor=  int ((scan0[i-1,j,k]+scan0[i+1,j,k]+scan0[i,j-1,k]+scan0[i,j+1,k]+scan0[i,j,k-1]+scan0[i,j,k+1])/6)
                        all_feature.append([scan0[i,j,k],ave_neighbor])
                    
#print (X)
#X= np.array(X,dtype=int)
print (len(X))
length= len(X)

for a in range(length):
    i=random.randint(1,scan1.shape[0]-2)
    j=random.randint(1,scan1.shape[1]-2)
    k=random.randint(1,scan1.shape[2]-2)
    #print(i,j,k)
    #if scan1[i,j,k]==0 and i>0 and j>0 and k>0 and i< scan1.shape[0]-1 and j<scan1.shape[1]-1 and k< scan1.shape[2]-1:
    if scan1[i,j,k]==0:
                    #print("found")
                    ave_neighbor=  int ((scan0[i-1,j,k]+scan0[i+1,j,k]+scan0[i,j-1,k]+scan0[i,j+1,k]+scan0[i,j,k-1]+scan0[i,j,k+1])/6)
                    if(ave_neighbor <20):
                        X.append([scan0[i,j,k],ave_neighbor])
                        y.append(0)
                    
    else:
        continue
        

#print (X)
print (len(X))
#print (y)
print (len(y))


# use svm to predict
# X = [[0, 0], [1, 1]]
# y = [0, 1]
clf = svm.SVC(gamma='scale')
clf.fit(X, y)  
print(len(all_feature))
print(clf.predict(all_feature))

mask_array=np.array(clf.predict(all_feature))
#mask_array.reshape(scan1.shape[0],scan1.shape[1],scan1.shape[2])
#print(mask_array.reshape(scan1.shape[0],scan1.shape[1],scan1.shape[2]).shape)
mask_img=sitk.GetImageFromArray(mask_array.reshape(scan1.shape[0],scan1.shape[1],scan1.shape[2]))
print(mask_img.GetSize())
#print("pixel id: {0} ({2})".format(mask_img.GetPixelID(), mask_img.GetPixelIDTypeAsString()))
mask_img_conv=sitk.Cast(mask_img,sitk.sitkUInt8)
sitk.WriteImage(mask_img_conv,"/home/penglab/Desktop/weak_signal_retrace_sample/011_x_13600.6_y_7614.94_z_1344.marker_tmp_APP2/svm_mask.tif" )






    








# In[ ]:




