"""
=====================================
Rotating 3D voxel animation of PYTHON
=====================================

Demonstrates using ``ax.voxels`` with uneven coordinates
"""
import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.animation as manimation
from math import copysign

def explode(data):
    size = np.array(data.shape)*2
    data_e = np.zeros(size - 1, dtype=data.dtype)
    data_e[::2, ::2, ::2] = data
    return data_e

def voxel_face(corns, dm, nf):
    '''
    Grab the corner coordinates of one voxel face

    Parameters
    ----------
    corns : np.indices array of corners for one voxel
    dm : (dimension), values can be  0(x), 1(y), 2(z)
    nf : (near/far face), values can be 0(near), 1(far)
    '''

    lc = corns.copy() #local copy so we don't swap original
    if dm == 1 : #swap y into x and correct ordering
        lc[0], lc[1] = corns[1].transpose(1,0,2), corns[0].transpose(1,0,2)
    if dm == 2 : #swap z into x and correct ordering
        lc[0], lc[2] = corns[2].transpose(2,1,0), corns[0].transpose(2,1,0)

    ret = np.zeros((3,2,2))
    xc1 = lc[0,nf,0,0] #hold x dim constant
    ret[0,:] = np.array([[xc1, xc1], [xc1, xc1]])
    yc1, yc2 = lc[1,0,0:2,0]
    ret[1,:] = np.array([[yc1, yc2], [yc1, yc2]])
    zc1, zc2 = lc[2,0,0,0:2]
    ret[2,:] = np.array([[zc1, zc1], [zc2, zc2]])

    if dm != 0 : #swap x back into desired dimension
        ret[0], ret[dm] = ret[dm].copy(), ret[0].copy()
    return ret


# build PYTHON letters
n_voxels = np.zeros((4, 4, 5), dtype=bool)
letters = [None]*6
letter_faces = np.zeros((6,2),dtype=int)

#P
n_voxels[0, 0, :] = True
n_voxels[:, 0, -3] = True
n_voxels[:, 0, -1] = True
n_voxels[-1, 0, -2] = True
letters[0] = np.array(np.where(n_voxels)).T
letter_faces[0] = [1, 0] #close y face
n_voxels[...] = False
#Y
n_voxels[-1, 0, -3:] = True
n_voxels[-1, -1, :] = True
n_voxels[-1, :, -3] = True
n_voxels[-1, :, 0] = True
letters[1] = np.array(np.where(n_voxels)).T
letter_faces[1] = [0, 1] #far x face
n_voxels[...] = False
#T
n_voxels[:, 0, -1] = True
n_voxels[1:3, :, -1] = True
letters[2] = np.array(np.where(n_voxels)).T
letter_faces[2] = [2, 1] #far z face
n_voxels[...] = False
#H
n_voxels[0, 0, :] = True
n_voxels[0, -1, :] = True
n_voxels[0, :, 2] = True
letters[3] = np.array(np.where(n_voxels)).T
letter_faces[3] = [0, 0] #close x face
n_voxels[...] = False
#O
n_voxels[0, 1:3, 0] = True
n_voxels[-1, 1:3, 0] = True
n_voxels[1:3, 0, 0] = True
n_voxels[1:3, -1, 0] = True
letters[4] = np.array(np.where(n_voxels)).T
letter_faces[4] = [2, 0] #close z face
n_voxels[...] = False
#N
n_voxels[0, -1, :] = True
n_voxels[-1, -1, :] = True
n_voxels[1, -1, 1:3] = True
n_voxels[2, -1, 2:4] = True
letters[5] = np.array(np.where(n_voxels)).T
letter_faces[5] = [1, 1] #far y face
n_voxels[...] = False

fcol = np.full(n_voxels.shape, '#7A88CC60')
ecol = np.full(n_voxels.shape,  '#7D84A6')
filled = np.ones(n_voxels.shape)

# upscale the above voxel image, leaving gaps
filled_2 = explode(filled)
fcolors_2 = explode(fcol)
ecolors_2 = explode(ecol)

# Shrink the gaps
corn = np.indices(np.array(filled_2.shape) + 1).astype(float) // 2
ccorn = 0.05 #close corner
fcorn = 1.0 - ccorn
corn[0,0::2, :, :] += ccorn
corn[1,:, 0::2, :] += ccorn
corn[2,:, :, 0::2] += ccorn
corn[0,1::2, :, :] += fcorn
corn[1,:, 1::2, :] += fcorn
corn[2,:, :, 1::2] += fcorn


fig = plt.figure()
ax = fig.gca(projection='3d')
ax.axis("off")

#Plot the voxels
x, y, z = corn
x, y, z = np.indices((8, 8, 8))

# draw cuboids in the top left and bottom right corners, and a link between them
cube1 = (x < 3) & (y < 3) & (z < 3)
cube2 = (x >= 5) & (y >= 5) & (z >= 5)
link = abs(x - y) + abs(y - z) + abs(z - x) <= 2

# combine the objects into a single boolean array
voxels = cube1 | cube2 | link
# ax.voxels(x, y, z, filled_2, facecolors=fcolors_2, edgecolors=ecolors_2)

#Plot the letter square faces
jj=0
for j in [x for x in letters if x is not None]:

    locf = np.empty((j.shape[0],3,2,2)) #local face

    ji = 0
    for i in j:
        i = i * 2 #skip empty voxels
        loc = corn[:,i[0]:i[0]+2,i[1]:i[1]+2,i[2]:i[2]+2] #local corners
        locf[ji] = voxel_face(loc, letter_faces[jj,0], letter_faces[jj,1])
        ax.plot_surface(locf[ji,0],locf[ji,1],locf[ji,2],color='#ffe500a0',
                        shade=False)
        ji += 1

    jj += 1


#Views:        PY,  P, Y,  T,   H,   O,  N,  PY
view_elev = [  5,   0, 0, 90,   0, -90,  0,   5]
view_azim = [-60, -90, 0, 90, 180, 180, 90, -60]
#'''
FFMpegWriter = manimation.writers['ffmpeg']
metadata = dict(title='Movie Test', artist='Matplotlib',
                comment='Movie support!')
writer = FFMpegWriter(fps=25, metadata=metadata)

with writer.saving(fig, "pythonRot2.mp4", 100):

    for j in range(20):
        ax.view_init(view_elev[0], view_azim[0])
        plt.draw()
        writer.grab_frame()

    for i in range(1,len(view_elev)):

        de = (view_elev[i] - view_elev[i-1])
        da = (view_azim[i] - view_azim[i-1])

        if abs(da) >= 180 : #unecessary in this config
            da -= copysign(360, da)
        if abs(de) >= 180 :
            de -= copysign(360, de)

        if i != 1 :
            steps = 60
        else :
            steps = 10
        da = da / steps
        de = de / steps

        for j in range(10): #Pause on direct view of a letter
            ax.view_init(view_elev[i-1], view_azim[i-1])
            plt.draw()
            writer.grab_frame()
        for j in range(steps): #Rotate to next letter
            ax.view_init(view_elev[i-1] + j*de,
                         view_azim[i-1] + j*da)
            plt.draw()
            writer.grab_frame()