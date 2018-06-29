"""
2D rendering framework
"""
import os
import six
import sys

from gym.utils import reraise
from gym import error
import matplotlib.pyplot as plt
import mpl_toolkits.mplot3d.axes3d as p3
import matplotlib.animation as animation
import math
import numpy as np

RAD2DEG = 57.29577951308232

class Viewer(object):
    def __init__(self, width, height, display=None):

        self.width = width
        self.height = height

        # Attaching 3D axis to the figure
        self.fig = plt.figure()
        self.ax = p3.Axes3D(self.fig)

        # init with shape (3,0)
        # each list is for (x,y,z)
        self.agent_trajectory = [[], [], []]
        # internally, we call this "Gold standard data" because we don't actually
        # have ground truth, but this an easier
        self.ground_truth = self._gen_rand_trajectory(100)

        #self.window = pyglet.window.Window(width=width, height=height, display=display)
        #self.window.on_close = self.window_closed_by_user
        self.isopen = True
        self.trajectories = []
        self.onetime_geoms = []


    def close(self):
        plt.close(self.fig)

    def window_closed_by_user(self):
        self.isopen = False

    def set_bounds(self, left, right, bottom, top):
        assert right > left and top > bottom
        scalex = self.width/(right-left)
        scaley = self.height/(top-bottom)

    def add_trajectory(self, trajectory):
        # make sure trajectory is 3D
        assert(np.shape(trajectory)[0] == 3)
        self.trajectories.append(trajectory)

    def add_onetime(self, geom):
        self.onetime_geoms.append(geom)

    def render(self, return_rgb_array=False):
        self.window.clear()
        self.window.switch_to()
        self.window.dispatch_events()
        self.transform.enable()
        for geom in self.geoms:
            geom.render()
        for geom in self.onetime_geoms:
            geom.render()
        self.transform.disable()
        arr = None
        return arr if return_rgb_array else self.isopen

    def _gen_rand_trajectory(self, n_nodes, stepsize=0.1):
        """
        Create a line using a random walk algorithm

        length is the number of points for the line.
        dims is the number of dimensions the line has.
        """
        dims = 3
        trajectory = np.empty((dims, n_nodes))
        # replace first col with random numbers
        trajectory[:, 0] = np.random.rand(dims)
        for index in range(1, n_nodes):
            # scaling the random numbers by 0.1 so
            # movement is small compared to position.
            # subtraction by 0.5 is to change the range to [-0.5, 0.5]
            # to allow a line to move backwards.
            step = ((np.random.uniform(-1, 1, dims)) * stepsize)
            # replace only the  next col in the sequence
            # take last point and add step to it
            trajectory[:, index] = trajectory[:, index - 1] + step

        return trajectory

    def __del__(self):
        self.close()

class SimpleImageViewer(object):
    def __init__(self, display=None):
        self.window = None
        self.isopen = False
        self.display = display
    def imshow(self, arr):
        if self.window is None:
            height, width, _channels = arr.shape
            self.window = pyglet.window.Window(width=4*width, height=4*height, display=self.display, vsync=False, resizable=True)
            self.width = width
            self.height = height
            self.isopen = True

            @self.window.event
            def on_resize(width, height):
                self.width = width
                self.height = height

            @self.window.event
            def on_close():
                self.isopen = False

        assert len(arr.shape) == 3, "You passed in an image with the wrong number shape"
        self.window.clear()
        self.window.switch_to()
        self.window.dispatch_events()
        image.blit(0, 0, width=self.window.width, height=self.window.height)
        self.window.flip()
    def close(self):
        if self.isopen:
            self.window.close()
            self.isopen = False

    def __del__(self):
        self.close()
