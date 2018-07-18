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

        self.isopen = False
        self.scale_x = scale_x
        self.scale_y = scale_y
        self.display = display
        self.filepath = filepath
        self.filename = os.path.basename(filepath)

        # initialize window with the input image
        self.x_span, self.y_span, self.z_span = original_state.shape
        self.x, self.y, self.z = np.indices((self.x_span, self.y_span, self.z_span)) - .5
        assert original_state.shape == (
        self.x_span, self.y_span, 3), "You passed in an image with the wrong number shape"
        self.fig, self.ax = self.__init_fig__()

        self.draw_image(original_state)
        self.data_generator = data_generator

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

    def saveGif(self, filename=None, arr=None, duration=0):
        arr[0].save(filename, save_all=True,
                    append_images=arr[1:],
                    duration=500,
                    quality=95)  # duration milliseconds

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
        positions, colors = next(self.data_generator)
        if len(positions) > 0:
            pc = self._plotCubeAt(positions, colors=colors, edgecolor="k")
            self.ax.add_collection3d(pc)
        self.ax.view_init(30, 0.3 * i)
        self.fig.canvas.draw()

