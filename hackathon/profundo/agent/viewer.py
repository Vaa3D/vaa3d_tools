#!/usr/bin/env python
# -*- coding: utf-8 -*-
# File: viewer.py
# Author: Amir Alansary <amiralansary@gmail.com>

import os
import math
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
from matplotlib import animation, cm


class SimpleImageViewer(object):

    def __init__(self, original_state, data_generator, scale_x=1, scale_y=1, filepath=None, display=None):
        self.counter = 0
        self.isopen = False
        self.scale_x = scale_x
        self.scale_y = scale_y
        self.display = display
        if filepath:
            self.filepath = filepath
            self.filename = os.path.basename(filepath)

        # initialize window with the input image
        self.x_span, self.y_span, self.z_span = original_state.shape
        self.x, self.y, self.z = np.indices((self.x_span, self.y_span, self.z_span)) - .5

        self.fig, self.ax = self.__init_fig__()

        self.draw_image(original_state)
        self.data = [list(a) for a in data_generator]

        self.isopen = True

    def __init_fig__(self):
        fig = plt.figure()
        ax = fig.gca(projection='3d')
        ax.set_aspect('equal')
        ax.axis('off')

        ax.set_xlim([0, self.x_span])
        ax.set_ylim([0, self.y_span])
        ax.set_zlim([0, self.z_span])

        return fig, ax

    def draw_image(self, arr, colors=None):
        binary_grid = arr.astype(bool)
        positions = np.c_[self.x[binary_grid == 1], self.y[binary_grid == 1], self.z[binary_grid == 1]]

        pc = self._plotCubeAt(positions, colors=colors, edgecolor="k")
        self.ax.add_collection3d(pc)

    def save_vid(self, filename, num_frames):
        dir_name, filename = os.path.split(filename)
        # if not os.path.exists(dir_name):
        #     os.mkdir(dir_name)
        print("saving video {}".format(filename), flush=True)
        if not hasattr(self, 'anim'):  # we haven't animated yet
            self.render_animation(num_frames)
        self.anim.save(filename,
                  fps=1,  # 15
                  extra_args=['-vcodec', 'libx264'])

    def saveGif(self, filename=None, arr=None, duration=0):
        arr[0].save(filename, save_all=True,
                    append_images=arr[1:],
                    duration=500,
                    quality=95)  # duration milliseconds

    def show(self):
        plt.show()
        # plt.show(block=False)

    def close(self):
        if self.isopen:
            plt.close()
            self.isopen = False

    def __del__(self):
        self.close()

    def _cuboid_data(self, position, size=(1, 1, 1)):
        cube_corners = [[[0, 1, 0], [0, 0, 0], [1, 0, 0], [1, 1, 0]],
                        [[0, 0, 0], [0, 0, 1], [1, 0, 1], [1, 0, 0]],
                        [[1, 0, 1], [1, 0, 0], [1, 1, 0], [1, 1, 1]],
                        [[0, 0, 1], [0, 0, 0], [0, 1, 0], [0, 1, 1]],
                        [[0, 1, 0], [0, 1, 1], [1, 1, 1], [1, 1, 0]],
                        [[0, 1, 1], [0, 0, 1], [1, 0, 1], [1, 1, 1]]]
        cube_corners = np.array(cube_corners).astype(float)
        # scale each dimension of cube_corners by size
        for i in range(3):
            cube_corners[:, :, i] *= size[i]
        # translate cuboid to final position
        cube_corners += np.array(position)
        return cube_corners

    def _plotCubeAt(self, positions, sizes=None, colors=None, **kwargs):
        if not isinstance(colors, (list, np.ndarray)): colors = ["C0"] * len(positions)
        if not isinstance(sizes, (list, np.ndarray)): sizes = [(1, 1, 1)] * len(positions)
        g = []
        for p, s, c in zip(positions, sizes, colors):
            g.append(self._cuboid_data(p, size=s))
        return Poly3DCollection(np.concatenate(g),
                                facecolors=np.repeat(colors, 6, axis=0), **kwargs)

    def _animate(self, i):
        positions, iou = self.data[i]
        colors = cm.RdBu(iou)  # map val to colormap
        if len(positions) > 0:
            pc = self._plotCubeAt(positions, colors=colors, edgecolor="k")
            self.ax.add_collection3d(pc)
        self.ax.view_init(30, 0.3 * self.counter)
        self.fig.canvas.draw()
        self.counter += 1

    def render_animation(self, num_frames):
        # instantiate the animator.
        self.anim = animation.FuncAnimation(self.fig, self._animate,
                                       frames=num_frames,
                                            # frames=self.data_generator,
                                       interval=1,
                                       repeat = False)
        print("rendering")

