#!/usr/bin/env python
# -*- coding: utf-8 -*-
# File: viewer.py
# Author: Amir Alansary <amiralansary@gmail.com>

import os
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
from matplotlib import animation, cm
import matplotlib.patches as mpatches

plt.rcParams['animation.ffmpeg_path'] = '/usr/bin/ffmpeg'
plt.style.use('dark_background')

class SimpleImageViewer(object):
    # TODO multiprocess https://stackoverflow.com/a/4662511/4212158
    # TODO https://matplotlib.org/gallery/misc/multiprocess_sgskip.html
    def __init__(self, human_nodes, agent_nodes, reward_history, filepath=None, display=None,
                 state_dimensions=None):
        print("spawning animator...")
        self.counter = 0
        self.isopen = False
        self.display = display
        if filepath:
            self.filepath = filepath
            self.filename = os.path.basename(filepath)


        self.already_plotted = set()
        self.agent_nodes = agent_nodes
        self.reward_history = reward_history
        self.num_frames = len(agent_nodes)

        # initialize window with the input image
        # assert np.allclose(human_nodes.shape, human_nodes.shape[0])
        if state_dimensions is not None:
            self.x_span, self.y_span, self.z_span =state_dimensions

            # TODO: understand why -0.5
            self.x, self.y, self.z = np.indices((self.x_span, self.y_span, self.z_span)) - .5

        self.fig, self.ax = self.__init_fig__()

        transparent_white = [1, 1, 1, 0.5]  # RGBA tuple
        # light_gray = [0, 0, 0, 0.3]  # RGBA tuple
        # FIXME renable
        # self.draw_image(human_nodes, colors=transparent_white)
        # for pos in human_nodes:
            # print("agent pos before transpose", pos)
        pc = self.make_cubes(human_nodes, colors=[1, 1, 1, 0.8])
        self.ax.add_collection3d(pc)
        # FIXME use generator instead of converting to list

        # self.data = [list(a) for a in data_generator]

        # xs = []
        # ys = []
        # zs = []
        # for pos, _ in self.data:
        #     for x, y, z in np.nditer(pos, flags=['external_loop']):
        #         xs.append(x)
        #         ys.append(y)
        #         zs.append(z)
        # print("xs =", xs)
        # print("ys =", ys)
        # print("zs =", zs)
        # self.ax.scatter(xs, ys,zs)
        # self.fig.savefig(str(np.random.randint(100,10000))+".png")
        # self.fig, self.ax = self.__init_fig__()


        self.isopen = True

    def __init_fig__(self):
        fig = plt.figure()
        ax = fig.gca(projection='3d')
        ax.set_aspect('equal')
        ax.axis('off')

        # try:
        ax.set_xlim([0, self.x_span])
        ax.set_ylim([0, self.y_span])
        ax.set_zlim([0, self.z_span])
        # except AttributeError:
        #     pass

        self.scoreboard = ax.text(0.9 * self.x_span, 0, 0.9 * self.z_span,
                             s=" ",
                             bbox={'facecolor': 'w', 'alpha': 0.5, 'pad': 5})

        # plot legends
        white_patch = mpatches.Patch(color='white', label='human annotation')
        blue_patch = mpatches.Patch(color='blue', label='agent trajectory')
        ax.legend(handles=[white_patch, blue_patch])

        return fig, ax

    # def draw_image(self, arr, colors=None):
    #     """given a dense img grid, filter out blank voxels and plot cubes at filled voxels
    #     used for drawing the initial background or "ground truth"
    #
    #     sends (N, 3) array to plotCubes"""
    #     # print("bg arr shape ", arr.shape)
    #     binary_grid = arr.astype(bool)
    #     positions = np.c_[self.x[binary_grid == 1], self.y[binary_grid == 1], self.z[binary_grid == 1]]
    #     # print("bg pos ", positions.shape, positions.dtype)# positions)
    #     # print("plotting background voxels, pos shape ", np.shape(positions))
    #     pc = self._plotCubeAt(positions, colors=colors)  # , edgecolor="k")
    #     self.ax.add_collection3d(pc)

    def save_vid(self, filename):
        print("saving figure...")
        dir_name, filename = os.path.split(filename)
        # if not os.path.exists(dir_name):
        #     os.mkdir(dir_name)
        # print("saving video {}".format(filename), flush=True)
        if not hasattr(self, 'anim'):  # we haven't animated yet
            self.render_animation()
        FFwriter = animation.FFMpegWriter(fps=15,
                       extra_args=['-vcodec', 'libx264'])
        self.anim.save(filename, writer=FFwriter)
        # self.anim.save(filename,
        #           fps=15,
        #           extra_args=['-vcodec', 'libx264'])

    def saveGif(self, filename=None, arr=None, duration=0):
        raise NotImplementedError
        arr[0].save(filename, save_all=True,
                    append_images=arr[1:],
                    duration=500,
                    quality=95)  # duration milliseconds

    def show(self):
        plt.show(block=True)
    #
    # def show_agent(self):
    #     fig1 = plt.figure()
    #     ax = fig1.add_subplot(111, projection='3d')
    #     ax.relim()
    #     ax.autoscale()
    #
    #     # print("dat ", self.data)
    #
    #     xs = []
    #     ys = []
    #     zs = []
    #     for pos, _ in self.data:
    #         for x, y, z in np.nditer(pos, flags=['external_loop']):
    #             xs.append(x)
    #             ys.append(y)
    #             zs.append(z)
    #
    #     print("zs ", zs)
    #     plt.scatter(xs, ys, zs)
    #     plt.show()

    def close(self):
        if self.isopen:
            plt.close()
            self.isopen = False

    def __del__(self):
        self.close()

    def _cuboid_data(self, position, size=(1, 1, 1)):
        """used for turning a coordinate into 6 3D polygons"""
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

    def make_cubes(self, positions, sizes=None, colors=None, **kwargs):
        """positions can either be dense bool array, or """
        # print("positions ", np.shape(positions))
        # print("dtype ", positions.dtype)
        # positions = zip(x,y,z)
        # colors are provided with the original state,
        # so this will only be used with agent timeseries
        # print("colors ", colors)
        if not isinstance(colors, (list, np.ndarray)):
            # print("overriding colors ", type(colors), colors)
            colors = [(0, 0, 1, 1)] * len(positions)
        if not isinstance(sizes, (list, np.ndarray)):
            sizes = [(1, 1, 1)] * len(positions)

        try:
            # colors[:, :, :, -1] = colors[:, :, :, -1] / 3
            colors[:, -1] /= 5
            colors = colors[0]
        except TypeError:  # colors is list of tuples
            colors[-1] /= 5  # reduce alpha
        except IndexError:  # mising alpha vals
            # print("index before ", colors.shape)
            alpha_col = np.repeat(0.3, len(colors))[np.newaxis].T
            colors = np.hstack((colors, alpha_col))

        if len(colors) != len(positions):
            colors = [tuple(colors)] * len(positions)

        # print("colors shape ", type(colors), colors)

        g = []
        for pos, size, colr in zip(positions, sizes, colors):
            # print("POS ", pos)
            g.append(self._cuboid_data(pos, size=size))

        # there are 6 faces to a cube
        # print("facecol", np.shape(np.repeat(colors, 6, axis=0)), np.repeat(colors, 6, axis=0))
        return Poly3DCollection(np.concatenate(g),
                                # facecolors=np.repeat(colors, 6, axis=0), **kwargs)
                                facecolors=np.tile(colors, (6, 1)), **kwargs)

    def _animate(self, i):
        """given a list of new positions at a given timestep, plot those positions"""

            # rewards = self.reward_history[:i]
            # self.ax.collections = []
            # # print("data", data, "len ", len(data))
            # for pos, reward in zip(traj, rewards):
            #     # print("agent nodes pos ", pos)
            #     if reward > 0:
            #         color = [0,1,0,0.5]  # green
            #     else:
            #         color = [1,0,0,0.5]  # red
            #     if len(pos) > 0:
            #         pc = self._plotCubeAt([pos], colors=color, edgecolor="c")
            #         self.ax.add_collection3d(pc)
        new_node = self.agent_nodes[i]
        node_ = tuple(new_node)

        # avoid replotting
        if node_ not in self.already_plotted:
            self.already_plotted.add(node_)
            # print("adding cube at ", new_node)
            # if len(traj) > 0:
            pc = self.make_cubes([new_node], colors=[1, 0, 1, 0.3])
            self.ax.add_collection3d(pc)
        self.scoreboard.set_text("Score: {}".format(sum(self.reward_history[:i])))

        # xs = []
        # ys = []
        # zs = []
        # rewards = []
        # for pos, reward in data:
        #     for x, y, z in np.nditer(pos, flags=['external_loop']):
        #         xs.append(x)
        #         ys.append(y)
        #         zs.append(z)
        #     rewards.append(reward.flatten())
        # # print("agent ious ", ious)
        # colors = cm.RdBu(rewards)  # map val to colormap
        # if len(colors) != 0:
        #     colors = np.reshape(colors, (-1, 4))  # invalid nesting
        #     colors[:, -1] /= 10
        # if len(xs) > 0:
        #     # print("xs ys zs", xs, ys, zs)
        #     positions = np.vstack((xs, ys, zs)).T
        #     # print("agent positions ", np.shape(positions))
        #     # print("animating agent ")
        #     # print("agent colors = ", np.shape(colors), colors)
        #     # print("anim positions ", positions.shape, positions)
        #     pc = self._plotCubeAt(positions, colors=colors, edgecolor="w")
        #     self.ax.add_collection3d(pc)
        #     # self.ax.scatter(xs[-1], ys[-1], zs[-1], color="cyan", marker="s")
        self.ax.view_init(30, 0.8 * self.counter)
        self.fig.canvas.draw()
        self.counter += 1

    def render_animation(self):
        # instantiate the animator.
        self.anim = animation.FuncAnimation(self.fig, self._animate,
                                            frames=self.num_frames,
                                            # frames=self.data_generator,
                                            interval=30,
                                            repeat=False)
        # TODO: figure out how to use blitting to make this faster
        # print("rendering")
