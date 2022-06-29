import os

import tifffile

from u3d.unet3d.model.unet import *

model_path = 'C:/Users/admin/Desktop/data/relu_center_10_150_model_4.h5'
path = 'C:/Users/admin/Desktop/data'
path_ms = 'selected_image'
path_is = 'ideal_ims'

model = unet_model_3d()
raw_ims = []
ideal_ims = []
i = 0
for root, dirs, files in os.walk(os.path.join(path, path_ms)):
    for f in files:
        raw_im_path = root + '/' + f
        raw_im = tifffile.imread(raw_im_path) / 65535
        a = raw_im.reshape(1, 64, 64, 64)
        raw_ims.append(a)


        # raw_ims[i][0].append(raw_im)
        # i=i+1
        # raw_ims.append([[]])
raw_ims =np.asarray(raw_ims)
i = 0
for root, dirs, files in os.walk(os.path.join(path, path_is)):
    for f in files:
        ideal_im_path = root + '/' + f
        ideal_im = tifffile.imread(ideal_im_path) / 65535
        a = ideal_im.reshape(1, 64, 64, 64)
        ideal_ims.append(a)

        # ideal_ims[i][0].append(ideal_im)
        # ideal_ims.append([[]])
        # i = i + 1
ideal_ims = np.asarray(ideal_ims)
# for r, d, f in os.walk(path):
#     for name in d:
#         for root, dirs, files in os.walk(os.path.join(r, name)):
#             maker_path = root + '/' + 'allMarker.marker'
#             ideal_im_path = root + '/' + 'decon_ideal.tif'
#             raw_im_path = root + '/' + 'decon.tif'
#             raw_ims[0][0].append(tifffile.imread(raw_im_path))
#             ideal_ims[0][0].append(tifffile.imread(ideal_im_path))

model.fit(raw_ims, ideal_ims, epochs=150)
#model.fit([[[a]]], [[[a]]], epochs=100)
model.save(model_path)
