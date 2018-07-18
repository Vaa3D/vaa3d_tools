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

    def __init__(self, original_state, scale_x=1, scale_y=1, filepath=None, display=None):

        self.isopen = False
        self.scale_x = scale_x
        self.scale_y = scale_y
        self.display = display
        self.filepath = filepath
        self.filename = os.path.basename(filepath)

        # initialize window with the input image
        self.x_span, self.y_span, self.z_span = original_state.shape
        self.x, self.y, self.z = np.indices((self.x_span, self.y_span, self.z_span)) - .5
        assert original_state.shape == (self.x_span, self.y_span, 3), "You passed in an image with the wrong number shape"
        self.fig, self.ax = self.__init_fig__()


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




    def draw_image(self, arr):
        binary_grid = arr.astype(bool)
        positions = np.c_[self.x[binary_grid == 1], self.y[binary_grid == 1], self.z[binary_grid == 1]]
        r = lambda: np.random.randint(0, 255)
        random_colors = np.array(['#%02X%02X%02X%02X' % (r(), r(), r(), r()) for _ in enumerate(positions)])

        pc = self.plotCubeAt(positions, colors=random_colors, edgecolor="k")
        self.ax.add_collection3d(pc)



    def draw_point(self,x=0.0,y=0.0,z=0.0):
        x = self.img_height - x
        y = y
        # pyglet.graphics.draw(1, GL_POINTS,
        #     ('v2i', (x_new, y_new)),
        #     ('c3B', (255, 0, 0))
        # )
        glBegin(GL_POINTS) # draw point
        glVertex3f(x, y, z)
        glEnd()


    def draw_circle(self, radius=10, res=30, pos_x=0, pos_y=0,
                    color=(1.0,1.0,1.0,1.0),**attrs):

        points = []
        # window start indexing from bottom left
        x = self.img_height - pos_x
        y = pos_y

        for i in range(res):
            ang = 2*math.pi*i / res
            points.append((math.cos(ang)*radius + y ,
                           math.sin(ang)*radius + x))

        # draw filled polygon
        if   len(points) == 4 : glBegin(GL_QUADS)
        elif len(points)  > 4 : glBegin(GL_POLYGON)
        else: glBegin(GL_TRIANGLES)
        for p in points:
            # choose color
            glColor4f(color[0],color[1],color[2],color[3]);
            glVertex3f(p[0], p[1],0)  # draw each vertex
        glEnd()
        # reset color
        glColor4f(1.0, 1.0, 1.0, 1.0);


    def draw_rect(self, x_min_init, y_min, x_max_init, y_max):
        main_batch = pyglet.graphics.Batch()
        # fix location
        x_max = self.img_height - x_max_init
        x_min = self.img_height - x_min_init
        # draw lines
        glColor4f(0.8, 0.8, 0.0, 1.0)
        main_batch.add(2, gl.GL_LINES, None,
                       ('v2f', (y_min, x_min, y_max, x_min)))
                       # ('c3B', (204, 204, 0, 0, 255, 0)))
        main_batch.add(2, gl.GL_LINES, None,
                       ('v2f', (y_min, x_min, y_min, x_max)))
                       # ('c3B', (204, 204, 0, 0, 255, 0)))
        main_batch.add(2, gl.GL_LINES, None,
                       ('v2f', (y_max, x_max, y_min, x_max)))
                       # ('c3B', (204, 204, 0, 0, 255, 0)))
        main_batch.add(2, gl.GL_LINES, None,
                       ('v2f', (y_max, x_max, y_max, x_min)))
                       # ('c3B', (204, 204, 0, 0, 255, 0)))

        main_batch.draw()
        # reset color
        glColor4f(1.0, 1.0, 1.0, 1.0);



    def display_text(self, text, x, y, color=(0,0,204,255), #RGBA
                     anchor_x='left', anchor_y='top'):
        x = int(self.img_height - x)
        y = int(y)
        label = pyglet.text.Label(text,
                                  font_name='Ariel', color=color,
                                  font_size=8, bold=True,
                                  x=y, y=x,
                                  anchor_x=anchor_x, anchor_y=anchor_y)
        label.draw()


    def render(self):
        self.window.flip()

    def saveGif(self,filename=None,arr=None,duration=0):
        arr[0].save(filename, save_all=True,
                    append_images=arr[1:],
                    duration=500,
                    quality=95) # duration milliseconds

    def close(self):
        if self.isopen:
            self.window.close()
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
