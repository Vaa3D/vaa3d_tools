import os
import tifffile
import matplotlib.pyplot as plt
from u3d.unet3d.model.unet import *

model_path = r'F:\NuMorph\3DNucleiTracingData\cut_Training\relu_center_200_model_35_semi_0001_.h5'
path = r'F:\NuMorph\3DNucleiTracingData\cut_Training'
path_ms = 'selected_image'
path_is = 'ideal_ims_sphere'

model = unet_model_3d()
raw_ims = []

ideal_ims = []
i = 0
#os.environ["CUDA_VISIBLE_DEVICES"] = "0"
for root, dirs, files in os.walk(os.path.join(path, path_ms)):
    for f in files:
        raw_im_path = root + '/' + f
        raw_im = tifffile.imread(raw_im_path) / 65535
        a = raw_im.reshape(1, 64, 64, 64)
        raw_ims.append(a)


        # raw_ims[i][0].append(raw_im)
        # i=i+1
        # raw_ims.append([[]])
raw_ims = np.asarray(raw_ims)
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

history = model.fit(raw_ims, ideal_ims, batch_size=1, epochs=200)
#model.fit([[[a]]], [[[a]]], epochs=100)
model.save(model_path)

# acc = history.history['acc']
loss = history.history['loss']
epochs = range(1, len(loss)+1)

plt.title('loss')
# plt.plot(epochs, acc, 'red', label='acc')
plt.plot(epochs, loss, 'blue', label='loss')
plt.legend()
plt.show()
