# adapted from https://terbium.io/2017/12/matplotlib-3d/
# import requests
# images = requests.get('http://www.fil.ion.ucl.ac.uk/spm/download/data/attention/attention.zip')
import os
#import zipfile
#archive = zipfile.ZipFile('images.zip', 'r')
#images = zipfile.ZipFile("/home/ubuntu/tmp/attention.zip", "r")

from io import BytesIO

#zipstream = BytesIO(images)
#zf = zipfile.ZipFile(images)

from nibabel import FileHolder
from nibabel.analyze import AnalyzeImage

from skimage.transform import resize
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt

from mpl_toolkits.mplot3d import Axes3D
import numpy as np
from matplotlib import cm


with open('/home/ubuntu/tmp/attention/structural/nsM00587_0002.hdr', 'rb') as header:
    with open('/home/ubuntu/tmp/attention/structural/nsM00587_0002.img', 'rb') as image:
        img = AnalyzeImage.from_file_map({'header': FileHolder(fileobj=header),
                                  'image': FileHolder(fileobj=image)})
        arr = img.get_data()

def explode(data):
    shape_arr = np.array(data.shape)
    size = shape_arr[:3] * 2 - 1
    exploded = np.zeros(np.concatenate([size, shape_arr[3:]]), dtype=data.dtype)
    exploded[::2, ::2, ::2] = data
    return exploded


def normalize(arr):
    arr_min = np.min(arr)
    return (arr - arr_min) / (np.max(arr) - arr_min)

def expand_coordinates(indices):
    x, y, z = indices
    x[1::2, :, :] += 1
    y[:, 1::2, :] += 1
    z[:, :, 1::2] += 1
    return x, y, z


def plot_cube(cube, angle=320):
    cube = normalize(cube)

    facecolors = cm.viridis(cube)
    facecolors[:, :, :, -1] = cube
    facecolors = explode(facecolors)

    filled = facecolors[:, :, :, -1] != 0
    x, y, z = expand_coordinates(np.indices(np.array(filled.shape) + 1))

    fig = plt.figure(figsize=(30 / 2.54, 30 / 2.54))
    ax = fig.gca(projection='3d')
    ax.view_init(30, angle)
    ax.set_xlim(right=IMG_DIM * 2)
    ax.set_ylim(top=IMG_DIM * 2)
    ax.set_zlim(top=IMG_DIM * 2)

    ax.voxels(x, y, z, filled, facecolors=facecolors)
    plt.show()



IMG_DIM = 50

def scale_by(arr, fac):
    mean = np.mean(arr)
    return (arr-mean)*fac + mean

transformed = np.clip(scale_by(np.clip(normalize(arr)-0.1, 0, 1)**0.4, 2)-0.1, 0, 1)


resized = resize(transformed, (IMG_DIM, IMG_DIM, IMG_DIM), mode='constant')


fig = plt.figure()
ax = fig.gca(projection='3d')
ax.axis('off')

plot_cube(resized[:35,::-1,:25])
