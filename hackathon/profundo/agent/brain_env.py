#!/usr/bin/env python
# -*- coding: utf-8 -*-
# File: brain_env.py
# Author: Amir Alansary <amiralansary@gmail.com>

import csv
import itertools


def warn(*args, **kwargs):
    pass


import warnings

warnings.warn = warn
warnings.simplefilter("ignore", category=PendingDeprecationWarning)

import os
import sys
import six
import random
import threading
import numpy as np
from tensorpack import logger
from collections import (Counter, defaultdict, deque, namedtuple)
import tempfile

import cv2
import math
import time
from PIL import Image
import subprocess
import shutil

import gym
from gym import spaces

from tensorpack.utils.utils import get_rng
from tensorpack.utils.stats import StatCounter

from dataAPI import FilesListCubeNPY
from viewer import SimpleImageViewer as Viewer
from jaccard import jaccard
from scipy.spatial import distance
from data_processing.swc_io import (locations_to_swc,
                                    swc_to_TIFF, TIFF_to_npy,
                                    swc_to_nparray, locations_to_img)

__all__ = ['Brain_Env', 'FrameStack']

_ALE_LOCK = threading.Lock()

ObservationBounds = namedtuple('ObservationBounds', ['xmin', 'xmax', 'ymin', 'ymax', 'zmin', 'zmax'])


# ===================================================================
# =================== 3d medical environment ========================
# ===================================================================

class Brain_Env(gym.Env):
    """Class that provides 3D medical image environment.
    This is just an implementation of the classic "agent-environment loop".
    Each time-step, the agent chooses an action, and the environment returns
    an observation and a reward."""

    def __init__(self, directory=None, viz=False, task=False, files_list=None,
                 observation_dims=(27, 27, 27), multiscale=False,  # FIXME automatic dimensions
                 max_num_frames=0, saveGif=False, saveVideo=False):  # FIXME hardcoded max num frames!
        """
        :param train_directory: environment or game name
        :param viz: visualization
            set to 0 to disable
            set to +ve number to be the delay between frames to show
            set to a string to be the directory for storing frames
        :param observation_dims: shape of the frame cropped from the image to feed
            it to dqn (d,w,h) - defaults (27,27,27)
        :param nullop_start: start with random number of null ops
        :param location_history_length: consider lost of lives as end of
            episode (useful for training)
        :max_num_frames: maximum number of frames per episode.
        """
        super(Brain_Env, self).__init__()

        # inits stat counters
        self.reset_stat()

        # counter to limit number of steps per episodes
        self.cnt = 0
        # maximum number of frames (steps) per episodes
        self.max_num_frames = max_num_frames
        # stores information: terminal, score, distError
        self.info = None
        # option to save display as gif
        self.saveGif = saveGif
        self.saveVideo = saveVideo
        # training flag
        self.task = task
        # image dimension (2D/3D)
        self.observation_dims = observation_dims
        self.dims = len(self.observation_dims)
        # multi-scale agent
        self.multiscale = multiscale
        # FIXME force multiscale false for now
        self.multiscale = False

        # init env dimensions
        if self.dims == 2:
            self.width, self.height = observation_dims
        elif self.dims == 3:
            self.width, self.height, self.depth = observation_dims
        else:
            raise ValueError

        with _ALE_LOCK:
            self.rng = get_rng(self)
        self.viz = viz

        print("viz {} gif {} video {}".format(self.viz, self.saveGif, self.saveVideo))

        # stat counter to store current score or accumlated reward
        self.current_episode_score = StatCounter()
        # get action space and minimal action set
        self.action_space = spaces.Discrete(6)  # change number actions here
        self.actions = self.action_space.n
        self.observation_space = spaces.Box(low=-1., high=1.,
                                            shape=self.observation_dims,
                                            dtype=np.uint8)
        # history buffer for storing last locations to check oscilations
        # TODO initialize _observation_bounds limits from input image coordinates
        # -1 to compensate for 0 indexing
        self._observation_bounds = ObservationBounds(0,
                                                     self.observation_dims[0] - 1,
                                                     0,
                                                     self.observation_dims[1] - 1,
                                                     0,
                                                     self.observation_dims[2] - 1)

        self.files = FilesListCubeNPY(directory, files_list)

        # self.files = filesListFetalUSLandmark(directory,files_list)
        # self.files = filesListCardioMRLandmark(directory,files_list)
        # prepare file sampler
        self.filepath = None
        self.file_sampler = self.files.sample_circular()  # returns generator
        # reset buffer, terminal, counters, and init new_random_game
        # we put this here so that init_player in DQN.py doesn't try to update_history
        self._clear_history()  # init arrays
        self._restart_episode()
        assert (np.shape(self._state) == self.observation_dims)
        # test jaccard
        assert np.isclose(jaccard(self.original_state, self.original_state)[0], 1)

    def reset(self):
        # with _ALE_LOCK:
        self._restart_episode()
        return self._observe()

    def _restart_episode(self):
        """
        restart current episode
        """
        self.terminal = False
        self.cnt = 0  # counter to limit number of steps per episodes
        self.num_games.feed(1)
        self.current_episode_score.reset()  # reset the stat counter
        self.new_random_game()

    def new_random_game(self):
        """
        load image,
        set dimensions,
        randomize start point,
        init _screen, qvals,
        calc distance to goal
        """
        self.terminal = False
        self.viewer = None

        # # sample a new image
        self.filepath, self.filename, begin, end = next(self.file_sampler)
        human_locations = swc_to_nparray(self.filepath, include_node_id=False)
        self.human_locations = np.unique(human_locations.astype(int), axis=0)
        # FIXME: input data should all be inbounds already!
        lim = self.observation_dims[0]
        np.clip(self.human_locations, 0, lim - 1, out=self.human_locations)
        # print("human locs ", self.human_locations)
        # print("calling locations_to_img from new_random")
        self.original_state = locations_to_img(self.human_locations, self.observation_dims,
                                               img=None, val=1)

        # check no negatives
        # print(np.where(self.original_state < 0))
        assert not np.any(self.original_state < 0)
        # check not all False
        assert self.original_state.all() == False

        self.original_state = self.original_state[:lim, :lim, :lim]  # FIXME data should be already in this shape
        # self._state = np.load(self.filepath).astype(float)
        # self._state = np.load(self.filepath).astype(float)
        # normalize inputs in-place
        # np.clip(self._state, 0, 1, out=self._state)

        self._state = np.copy(self.original_state)

        # multiscale (e.g. start with 3 -> 2 -> 1)
        # scale can be thought of as sampling stride
        if self.multiscale:
            raise NotImplementedError
            # ## brain
            # self.stepsize = 9
            # self.xscale = 3
            # self.yscale = 3
            # self.zscale = 3
            ## cardiac
            # self.stepsize = 6
            # self.xscale = 2
            # self.yscale = 2
            # self.zscale = 2
        else:
            self.stepsize = 1
            self.xscale = 1
            self.yscale = 1
            self.zscale = 1
        # image volume size
        # self._state_dims = np.shape(self._state)
        #######################################################################
        ## select random starting point
        # add padding to avoid start right on the border of the image
        # if (self.task == 'train'):
        #     skip_thickness = (int(self._state_dims[0] / 5),
        #                       int(self._state_dims[1] / 5),
        #                       int(self._state_dims[2] / 5))

        # binary_grid = self.original_state.astype(bool)
        # x_span, y_span, z_span = self.original_state.shape
        # x, y, z = np.indices((x_span, y_span, z_span))
        # positions = np.c_[x[binary_grid == 1], y[binary_grid == 1], z[binary_grid == 1]]

        # keep starting positions in bounds
        self._start_location = np.clip(np.array(begin, dtype=int), 0, lim - 1)
        self._terminal_node = np.clip(np.array(end, dtype=int), 0, lim - 1)
        # self._start_location = np.clip(np.array(begin, dtype=float),
        #                          self._observation_bounds.xmin+1e-15,
        #                          self._observation_bounds.xmax-1e-15)
        # self._terminal_node = np.clip(np.array(end, dtype=float),
        #                                self._observation_bounds.xmin+1e-15,
        #                                self._observation_bounds.xmax-1e-15)

        self._location = self._start_location

        assert self._is_in_bounds(self._start_location)
        assert self._is_in_bounds(self._terminal_node)

        self._qvalues = np.zeros(self.actions)
        # TODO: when doing multiscale, make difference bw state and observation
        self._state = self._observe()
        # self.curr_IOU = self.calc_IOU()
        # print("first IOU ", self.curr_IOU)
        self.reward = self._calc_reward(False, False, False)
        self._update_history()
        # we've finished iteration 0. now, step begins with cnt = 1
        self.cnt += 1

    def calc_IOU(self):
        """ calculate the Intersection over Union AKA Jaccard Index
        between two images

        https://en.wikipedia.org/wiki/Jaccard_index
        """
        # flatten bc  jaccard_similarity_score expects 1D arrays
        # agent_trajectory = self._state
        # state = self.swc_to_tiff.ravel()
        # agent_trajectory[agent_trajectory != -1] = 0  # mask out non-agent trajectory
        # state = state.astype(bool)  # everything non-zero => True

        # images should not be all True
        # assert agent_trajectory.all() == False
        # print("calling locations_to_img from IOU")
        agent_trajectory = locations_to_img(self._agent_nodes[:self.cnt],
                                            self.observation_dims,
                                            img=None,
                                            val=-1)

        # if self.cnt > 0:
        #     if self.curr_IOU > 0.7:
        #         np.set_printoptions(threshold=np.nan)
        #         print("arr1 \n ", np.array_repr(agent_trajectory))
        #         print("arr2 \n ", np.array_repr(self.original_state))
        #         print(self.original_state[np.where(self.original_state < 0)])
        #         raise Exception
        iou, _, _ = jaccard(agent_trajectory, self.original_state)
        # print("computed iou ", iou)
        # print("sum(agent) ", np.sum(agent_trajectory), "sum(original state)", np.sum(self.original_state), "computed iou ", iou)
        # print("agent shape\n", agent_trajectory.shape)
        # print("og shape\n", self.original_state.shape)
        # np.save("agent", state)
        # np.save("og", original_state)
        # assert isinstance(iou, )
        return iou

    def step(self, act, qvalues):
        """The environment's step function returns exactly what we need.
        Args:
          act:
        Returns:
          observation (object):
            an environment-specific object representing your observation of
            the environment. For example, pixel data from a camera, joint angles
            and joint velocities of a robot, or the board state in a board game.
          reward (float):
            amount of reward achieved by the previous action. The scale varies
            between environments, but the goal is always to increase your total
            reward.
          done (boolean):
            whether it's time to reset the environment again. Most (but not all)
            tasks are divided up into well-defined episodes, and done being True
            indicates the episode has terminated. (For example, perhaps the pole
            tipped too far, or you lost your last life.)
          info (dict):
            diagnostic information useful for debugging. It can sometimes be
            useful for learning (for example, it might contain the raw
            probabilities behind the environment's last state change). However,
            official evaluations of your agent are not allowed to use this for
            learning.
        """
        self._qvalues = qvalues
        self.best_q_vals.feed(np.max(qvalues))
        self._act = act

        current_loc = self._location
        self.terminal = False
        go_out = False
        backtrack = False
        node_found = False

        # print("action ", act)
        # UP Z+ -----------------------------------------------------------
        if (act == 0):
            proposed_location = current_loc + np.array([0, 0, 1]) * self.stepsize
            self.num_act0.feed(1)
        # FORWARD Y+ ---------------------------------------------------------
        elif (act == 1):
            proposed_location = current_loc + np.array([0, 1, 0]) * self.stepsize
            self.num_act1.feed(1)
        # RIGHT X+ -----------------------------------------------------------
        elif (act == 2):
            proposed_location = current_loc + np.array([1, 0, 0]) * self.stepsize
            self.num_act2.feed(1)
        # LEFT X- -----------------------------------------------------------
        elif act == 3:
            proposed_location = current_loc + np.array([-1, 0, 0]) * self.stepsize
            self.num_act3.feed(1)
        # BACKWARD Y- ---------------------------------------------------------
        elif act == 4:
            proposed_location = current_loc + np.array([0, -1, 0]) * self.stepsize
            self.num_act4.feed(1)
        # DOWN Z- -----------------------------------------------------------
        elif act == 5:
            proposed_location = current_loc + np.array([0, 0, -1]) * self.stepsize
            self.num_act5.feed(1)
        else:
            raise ValueError

        # print("action ", act, "diff ", proposed_location-self._location, "q vals ", qvalues)

        if not self._is_in_bounds(proposed_location):  # went out of bounds
            # print("proposed out of bounds ", proposed_location)
            # do not update current_loc
            go_out = True
            self.num_go_out.feed(1)
        else:  # in bounds
            # https://stackoverflow.com/a/25823710/4212158
            # .all(axis=1) makes sure that all of x,y,z isclose
            # np.any() checks is any coord is close
            value_at_proposed = self._state[proposed_location[0], proposed_location[1], proposed_location[2]]
            if value_at_proposed == -1:
                # if np.any(np.isclose(self._agent_nodes, proposed_location.T).all(axis=1)):
                #     print("backtracking detected ", transposed, "hist ", np.unique(self._agent_nodes, axis=0), np.isclose(np.unique(self._agent_nodes, axis=0), transposed).all(axis=1))
                # we backtracked
                backtrack = True
                self.num_backtracked.feed(1)
            elif value_at_proposed == 1:
                node_found = True

            # we are in bounds, accept new location
            self._location = proposed_location
            # only update state, iou if we've changed location
            self._state = self._observe()
            # self.curr_IOU = self.calc_IOU()

        # check that all 3 coords match terminal node location
        # atol gives us some tolerance

        if np.isclose(self._terminal_node, proposed_location.T, atol=1).all():
            terminal_found = True
            # print("finishing episode, terminal found")
            self.terminal = True
            self.num_success.feed(1)
        # print("new location ", self._location, go_out, backtrack)

        # punish -1 reward if the agent tries to go out
        # if (self.task != 'play'):  # TODO: why is this necessary?
        self.reward = self._calc_reward(go_out, backtrack,
                                        node_found)  # TODO I think reward needs to be calculated after increment cnt
        # update screen, reward ,location, terminal
        self._update_history()

        # # terminate if the distance is less than 1 during trainig
        # if (self.task == 'train'):
        #     if self.curr_IOU >= 0.9:
        #         # print("finishing episode, IOU = ", self.curr_IOU)
        #         self.terminal = True
        #         self.num_success.feed(1)
        #         # self.display()

        # terminate if maximum number of steps is reached

        if self.cnt >= self.max_num_frames - 1:  # compensate for 0 indexing
            # print("finishing episode, exceeded max_frames ", self.max_num_frames, " IOU = ", self.curr_IOU)
            self.terminal = True

        # check if agent oscillates
        # if self._oscillate:
        # TODO: rewind history, recalculate IOU
        # self._location = self.get_best_node()  # TODO replace
        # self._observation = self._observe()
        # if (self.task != 'play'):
        # self.curr_IOU = self.calc_IOU()
        # multi-scale steps
        # if self.multiscale:
        #     if self.xscale > 1:
        #         self.xscale -= 1
        #         self.yscale -= 1
        #         self.zscale -= 1
        #         self.stepsize = int(self.stepsize / 3)
        #         self._clear_history()
        #     # terminate if scale is less than 1
        #     else:
        #         self.terminal = True
        #         if self.curr_IOU >= 0.9: self.num_success.feed(1)
        # else:
        # self.terminal = True
        # if self.curr_IOU >= 0.9: self.num_success.feed(1)

        self.current_episode_score.feed(self.reward)
        self.cnt += 1

        info = {'score': self.current_episode_score.sum,
                'gameOver': self.terminal,
                'filename': self.filename,
                "qvals": qvalues,
                "backtracked": backtrack}

        if self.terminal:
            self.episode_duration.feed(self.cnt)
            info['IoU'] = self.calc_IOU()
            self._trim_arrays()
            if (self.saveGif or self.saveVideo or self.viz):
                self.display()

        return self._state, self.reward, self.terminal, info

    # def get_best_node(self):
    #     ''' get best location with best qvalue from last for locations
    #     stored in history
    #
    #     TODO: make sure nodes dont have overlap
    #     '''
    #     raise NotImplemented
    #     last_qvalues_history = self._qvalues_history[-4:]
    #     last_loc_history = self._agent_nodes[-4:]
    #     best_qvalues = np.max(last_qvalues_history, axis=1)
    #     # best_idx = best_qvalues.argmax()
    #     best_idx = best_qvalues.argmin()
    #     best_location = last_loc_history[best_idx]
    #
    #     return best_location

    def _clear_history(self):
        """ clear history buffer with current state
        """
        self._agent_nodes = np.zeros((self.max_num_frames, self.dims),
                                     dtype=int)  # [(0,) * self.dims] * self._history_length
        # self._IOU_history = np.zeros((self._history_length,))
        self._distances = np.zeros((self.max_num_frames,))
        # list of value lists
        self._qvalues_history = np.zeros(
            (self.max_num_frames, self.actions))  # [(0,) * self.actions] * self._history_length
        self.reward_history = np.zeros((self.max_num_frames,))

    def _update_history(self):
        """ update history buffer with current state
        """
        # update location history
        self._agent_nodes[self.cnt] = self._location
        # update jaccard index history
        # self._IOU_history[self.cnt] = self.curr_IOU
        self._distances[self.cnt] = self.curr_distance
        # and the reward
        self.reward_history[self.cnt] = self.reward
        # update q-value history
        self._qvalues_history[self.cnt] = self._qvalues

    def _trim_arrays(self):
        for arr in [self._agent_nodes, self._distances, self.reward_history, self._qvalues_history]:
            arr = arr[:self.cnt]  # self._IOU_history

    def _observe(self):
        # print("agent nodes ", self._agent_nodes[:self.cnt])
        # print("calling locations_to_img from observe")
        observation = locations_to_img(self._agent_nodes[:self.cnt],
                                       self.observation_dims,
                                       img=self.original_state,
                                       val=-1)
        loc = self._location.astype(int)
        observation[loc[0], loc[1], loc[2]] = -3
        return observation

        #
        # # take image, mask it w agent trajectory
        # agent_trajectory = self.connect_nodes_to_img()
        # # extact locations where agent TIFF is active, and add it to state
        # self.agent_nodes.append(self.img_to_locations(agent_trajectory))
        #
        # agent_trajectory *= -1  # agent frames are negative
        # # paste agent trajectory ontop of original state, but only when vals are not 0
        # agent_mask = agent_trajectory.astype(bool)
        # # print("agent traj shape", np.shape(agent_trajectory), np.shape(agent_mask))
        # if agent_mask.any():  # agent trajectory not empty
        #     np.copyto(observation, agent_trajectory, casting='no', where=agent_mask)
        #     _loc = self._location.astype(int)
        #     # set current location value to -10 to indicate where the agent currently is
        #     observation[_loc[0], _loc[1], _loc[2]] = -10
        # return observation

    def img_to_locations(self, img):
        """take dense 4D matrix, return coords where there is data"""
        binary_grid = img.astype(bool)
        x_span, y_span, z_span = img.shape
        x, y, z = np.indices((x_span, y_span, z_span))
        positions = np.c_[x[binary_grid == 1], y[binary_grid == 1], z[binary_grid == 1]]
        return positions

        # """
        # crop image data around current location to update what network sees.
        # update _observation_bounds
        #
        # :return: new state
        # """
        # # initialize screen with zeros - all background
        # observation = np.zeros((self.observation_dims))
        # # print("observation dims", np.shape(observation))
        #
        # # screen uses coordinate system relative to origin (0, 0, 0)
        # screen_xmin, screen_ymin, screen_zmin = 0, 0, 0
        # screen_xmax, screen_ymax, screen_zmax = self.observation_dims
        #
        # # extract boundary locations using coordinate system relative to "global" image
        # # width, height, depth in terms of screen coord system
        # if self.xscale % 2:
        #     xmin = self._location[0] - int(self.width * self.xscale / 2) - 1
        #     xmax = self._location[0] + int(self.width * self.xscale / 2)
        #     ymin = self._location[1] - int(self.height * self.yscale / 2) - 1
        #     ymax = self._location[1] + int(self.height * self.yscale / 2)
        #     zmin = self._location[2] - int(self.depth * self.zscale / 2) - 1
        #     zmax = self._location[2] + int(self.depth * self.zscale / 2)
        # else:
        #     xmin = self._location[0] - round(self.width * self.xscale / 2)
        #     xmax = self._location[0] + round(self.width * self.xscale / 2)
        #     ymin = self._location[1] - round(self.height * self.yscale / 2)
        #     ymax = self._location[1] + round(self.height * self.yscale / 2)
        #     zmin = self._location[2] - round(self.depth * self.zscale / 2)
        #     zmax = self._location[2] + round(self.depth * self.zscale / 2)
        #
        # TODO mv to crop_brain()?
        # # check if they violate image boundary and fix it
        # if xmin < 0:
        #     xmin = 0
        #     screen_xmin = screen_xmax - len(np.arange(xmin, xmax, self.xscale))
        # if ymin < 0:
        #     ymin = 0
        #     screen_ymin = screen_ymax - len(np.arange(ymin, ymax, self.yscale))
        # if zmin < 0:
        #     zmin = 0
        #     screen_zmin = screen_zmax - len(np.arange(zmin, zmax, self.zscale))
        # if xmax > self._state_dims[0]:
        #     xmax = self._state_dims[0]
        #     screen_xmax = screen_xmin + len(np.arange(xmin, xmax, self.xscale))
        # if ymax > self._state_dims[1]:
        #     ymax = self._state_dims[1]
        #     screen_ymax = screen_ymin + len(np.arange(ymin, ymax, self.yscale))
        # if zmax > self._state_dims[2]:
        #     zmax = self._state_dims[2]
        #     screen_zmax = screen_zmin + len(np.arange(zmin, zmax, self.zscale))
        #
        # # take image, mask it w agent trajectory
        # agent_trajectory = self.trajectory_to_branch()
        # agent_trajectory *= -1  # agent frames are negative
        # # paste agent trajectory ontop of original state, but only when vals are not 0
        # agent_mask = agent_trajectory.astype(bool)
        # # print("agent traj shape", np.shape(agent_trajectory), np.shape(agent_mask))
        # if agent_mask.any():  # agent trajectory not empty
        #     np.copyto(self.swc_to_tiff, agent_trajectory, casting='no', where=agent_mask)
        #     assert self.swc_to_tiff is not None
        #
        # # crop image data to update what network sees
        # # image coordinate system becomes screen coordinates
        # # scale can be thought of as a stride
        # # TODO: check if we need to keep "stride" from upstream
        # observation[screen_xmin:screen_xmax, screen_ymin:screen_ymax, screen_zmin:screen_zmax] = self.swc_to_tiff[
        #                                                                                          xmin:xmax,
        #                                                                                          ymin:ymax,
        #                                                                                          zmin:zmax]
        #
        # # update _observation_bounds limits from input image coordinates
        # # this is what the network sees
        # self._observation_bounds = ObservationBounds(xmin, xmax,
        #                                              ymin, ymax,
        #                                              zmin, zmax)
        #
        # return observation

    # def connect_nodes_to_img(self):
    #     """take location history, generate connected branches using Vaa3d plugin
    #     FIXME this function is horribly inefficient
    #     """
    #     locations = self._agent_nodes[:self.cnt]   # grab everything up until the current ts
    #     # print("iter ", self.cnt, "locations: ", locations)
    #     # print("og state shape ", np.shape(self.original_state))
    # # print("self obs dims ", self.observation_dims)
    #     # if the agent hasn't drawn any nodes, then the branch is empty. skip pipeline, return empty arr.
    #     if not locations.any():  # if all zeros, evals to False
    #         output_npy = np.zeros_like(self.original_state)
    #     else:
    #         fname = 'agent_trajectory' + str(np.random.randint(low=0, high=int(99e10)))
    #
    #         # try:
    #         with tempfile.TemporaryDirectory() as tmpdir:
    #
    #             output_swc = locations_to_swc(locations, fname, output_dir=tmpdir, overwrite=False)
    #             # TODO: be explicit about bounds to swc_to_tiff
    #             output_tiff_path = swc_to_TIFF(fname, output_swc, output_dir=tmpdir, overwrite=False)
    #             with _ALE_LOCK:
    #                 output_npy_path = TIFF_to_npy(fname, output_tiff_path, output_dir=tmpdir,
    #                                           overwrite=False)
    #             output_npy = np.load(output_npy_path).astype(float)
    #         # except IOError as e:
    #         #     print('IOError', e)
    #         # finally:
    #         # print("agent trajectory shape ", np.shape(output_npy))
    #         np.clip(output_npy, 0, 1, out=output_npy)
    #
    #     return output_npy

    # def crop_brain(self, xmin, xmax, ymin, ymax, zmin, zmax):
    #     return self.state[xmin:xmax, ymin:ymax, zmin:zmax]

    def distance_to_nearest_unexplored_node(self, location=None):
        # allow to chose reference location
        if location is None:
            location = self._location

        unvisisted_nodes = np.transpose(np.where(self._state > 0))
        if unvisisted_nodes.size == 0:
            raise Exception
        else:
            distance_to_nearest = distance.cdist([location], unvisisted_nodes).min()
        self.curr_distance = distance_to_nearest

        return distance_to_nearest

    def _calc_reward(self, go_out, backtrack, node_found):
        """ Calculate the new reward based on the increase in IoU
        """
        if go_out or backtrack:
            reward = -1
            return reward
        if node_found:
            reward = 2
            return reward

        if self.cnt == 0:
            previous_distance = np.inf
        else:
            previous_distance = self._distances[self.cnt - 1]
        distance_to_nearest = self.distance_to_nearest_unexplored_node()
        # print("distance: ", distance_to_nearest)

        if distance_to_nearest < previous_distance:
            reward = 1
        else:  # going farther away
            reward = -1

        return reward

        # # overrides everything else
        # # if terminal_found:
        # #     reward = 500
        # #     return reward
        # if go_out:
        #     reward = -20
        #     return reward
        # if backtrack:
        #     reward = -5
        #     return reward
        #
        #
        # # TODO, double check if indexes are correct
        # if self.cnt == 0:
        #     previous_IOU = 0.
        # else:
        #     previous_IOU = self._IOU_history[self.cnt - 1]
        # IOU_difference = self.curr_IOU - previous_IOU
        # # print(self.cnt, self._history_length)
        # # print("curr IOU = ", self.curr_IOU, "prev IOU = ", self._IOU_history[self.cnt - 1], "diff = ", IOU_difference,
        # #       "loc ", self._location)
        # assert isinstance(IOU_difference, float)
        #
        # if IOU_difference > 0:
        #     reward = 200
        # else:  # didn't go out, backtrack, or improve score
        #     reward = -1
        #
        # if terminal_found:
        #     reward = 1000
        #
        #
        #
        # return reward

    def _is_in_bounds(self, coords):
        assert len(coords) == 3
        x, y, z = coords
        bounds = self._observation_bounds
        is_in_bounds = ((bounds.xmin <= x <= bounds.xmax and
                         bounds.ymin <= y <= bounds.ymax and
                         bounds.zmin <= z <= bounds.zmax))

        # if not is_in_bounds:
        #     print("out of bounds :", coords)

        return is_in_bounds

    # @property
    # def _oscillate(self):
    #     """ Return True if the agent is stuck and oscillating
    #     """
    #     # TODO reimplement
    #     # TODO: erase last few frames if oscillation is detected
    #     counter = Counter(self._agent_nodes)
    #     freq = counter.most_common()
    #
    #     # TODO: wtF?
    #     if freq[0][0] == (0, 0, 0):
    #         if (freq[1][1] > 3):
    #             return True
    #         else:
    #             return False
    #     elif (freq[0][1] > 3):
    #         return True
    #
    # def get_action_meanings(self):
    #     """ return array of integers for actions"""
    #     ACTION_MEANING = {
    #         1: "UP",  # MOVE Z+
    #         2: "FORWARD",  # MOVE Y+
    #         3: "RIGHT",  # MOVE X+
    #         4: "LEFT",  # MOVE X-
    #         5: "BACKWARD",  # MOVE Y-
    #         6: "DOWN",  # MOVE Z-
    #     }
    #     return [ACTION_MEANING[i] for i in self.actions]

    # @property
    # def getScreenDims(self):
    #     """
    #     return screen dimensions
    #     """
    #     return (self.width, self.height, self.depth)
    #
    # def lives(self):
    #     return None

    def reset_stat(self):
        """ Reset all statistics counter"""
        self.stats = defaultdict(list)
        self.episode_duration = StatCounter()
        self.num_games = StatCounter()
        self.num_success = StatCounter()
        self.num_backtracked = StatCounter()
        self.num_go_out = StatCounter()
        self.best_q_vals = StatCounter()

        self.num_act0 = StatCounter()
        self.num_act1 = StatCounter()
        self.num_act2 = StatCounter()
        self.num_act3 = StatCounter()
        self.num_act4 = StatCounter()
        self.num_act5 = StatCounter()

    def display(self):
        """this is called at every step"""
        # current_point = self._location
        # img = cv2.cvtColor(plane, cv2.COLOR_GRAY2RGB)  # congvert to rgb
        # rescale image
        # INTER_NEAREST, INTER_LINEAR, INTER_AREA, INTER_CUBIC, INTER_LANCZOS4
        # scale_x = 1
        # scale_y = 1

        # print("nodes ", self._agent_nodes)
        # print("ious", self._IOU_history)
        # print("reward history ", np.unique(self.reward_history))
        # print("IOU history ", np.unique(self._IOU_history))
        plotter = Viewer(self.human_locations, self._agent_nodes, self.reward_history,
                         filepath=self.filename, state_dimensions=self.original_state.shape)
        #
        # #
        # # from viewer import SimpleImageViewer
        # # self.viewer = SimpleImageViewer(self.swc_to_tiff,
        # #                                 scale_x=1,
        # #                                 scale_y=1,
        # #                                 filepath=self.filename)
        #     self.gif_buffer = []
        #
        #
        # # render and wait (viz) time between frames
        # self.viewer.render()
        # # time.sleep(self.viz)
        # # save gif
        if self.saveGif:
            raise NotImplementedError
            # image_data = pyglet.image.get_buffer_manager().get_color_buffer().get_image_data()
            # data = image_data.get_data('RGB', image_data.width * 3)
            # arr = np.array(bytearray(data)).astype('uint8')
            # arr = np.flip(np.reshape(arr, (image_data.height, image_data.width, -1)), 0)
            # im = Image.fromarray(arr)
            # self.gif_buffer.append(im)
            #
            # if not self.terminal:
            #     gifname = self.filename.split('.')[0] + '.gif'
            #     self.viewer.saveGif(gifname, arr=self.gif_buffer,
            #                         duration=self.viz)
        if self.saveVideo:
            dirname = 'tmp_video'
            # if self.cnt <= 1:
            #     if os.path.isdir(dirname):
            #         logger.warn("""Log directory {} exists! Use 'd' to delete it. """.format(dirname))
            #         act = input("select action: d (delete) / q (quit): ").lower().strip()
            #         if act == 'd':
            #             shutil.rmtree(dirname, ignore_errors=True)
            #         else:
            #             raise OSError("Directory {} exits!".format(dirname))
            #     os.mkdir(dirname)

            vid_fpath = self.filename + '.mp4'
            # vid_fpath = dirname + '/' + self.filename + '.mp4'
            plotter.save_vid(vid_fpath)
            # plotter.show_agent()

        if self.viz:  # show progress
            # plotter.show()
            # actually, let's just save the files for later
            output_dir = os.path.abspath("saved_trajectories/")
            if not os.path.exists(output_dir):
                os.mkdir(output_dir)

            # outfile_fpath = os.path.join(output_dir, input_fname + ".npy")
            #
            # # don't overwrite
            # if not os.path.isfile(outfile_fpath) or overwrite:
            #     desired_len = 16
            #     img_array = tiff2array.imread(input_fpath)
            #     # make all arrays the same shape
            #     # format: ((top, bottom), (left, right))
            #     shp = img_array.shape
            #     # print(shp, flush=True)
            #     if shp != (desired_len, desired_len, desired_len):
            #         try:
            #             img_array = np.pad(img_array, (
            #             (0, desired_len - shp[0]), (0, desired_len - shp[1]), (0, desired_len - shp[2])),
            #                                'constant')
            #         except ValueError:
            #             raise
            #             # print(shp, flush=True)  # don't wait for all threads to finish before printing
            #
            np.savez(output_dir + self.filename, locations=self._agent_nodes, original_state=self.original_state,
                     reward_history=self.reward_history)
            #     return outfile_fpath


# class DiscreteActionSpace(object):
#
#     def __init__(self, num):
#         super(DiscreteActionSpace, self).__init__()
#         self.num = num
#         self.rng = get_rng(self)
#
#     def sample(self):
#         return self.rng.randint(self.num)
#
#     def num_actions(self):
#         return self.num
#
#     def __repr__(self):
#         return "DiscreteActionSpace({})".format(self.num)
#
#     def __str__(self):
#         return "DiscreteActionSpace({})".format(self.num)


# =============================================================================
# ================================ FrameStack =================================
# =============================================================================
class FrameStack(gym.Wrapper):
    """used when not training. wrapper for Medical Env"""

    def __init__(self, env, k):
        """Buffer observations and stack across channels (last axis)."""
        print("CALLING FRAMESTACK!!!!")
        gym.Wrapper.__init__(self, env)  # TODO shouldn't this be a super() method?
        self.k = k  # history length
        self.frames = deque([], maxlen=k)
        shp = env.observation_space.shape
        self._base_dim = len(shp)
        new_shape = shp + (k,)
        # fixme observation bounds
        self.observation_space = spaces.Box(low=-1., high=1., shape=new_shape,
                                            dtype=np.uint8)

    def reset(self):
        """flush buffer with empty frames, then add the first observation."""
        ob = self.env.reset()
        for _ in range(self.k - 1):
            self.frames.append(np.zeros_like(ob))
        self.frames.append(ob)
        return self._observation()  # fixme should this be self._state

    def step(self, action, q_values):
        ob, reward, done, info = self.env.step(action, q_values)
        self.frames.append(ob)
        return self._observation(), reward, done, info  # fixme should this be self._state

    # TODO: check if we can change to get_last_observation
    def _observation(self):
        assert len(self.frames) == self.k
        return np.stack(self.frames, axis=-1)
        # if self._base_dim == 2:
        #     return np.stack(self.frames, axis=-1)
        # else:
        #     return np.concatenate(self.frames, axis=2)


# =============================================================================
# ================================== notes ====================================
# =============================================================================
"""

## Notes from landmark detection Siemens paper
# states  -> ROI - center current pos - size (2D 60x60) (3D 26x26x26)
# actions -> move (up, down, left, right)
# rewards -> delta(d) relative distance change after executing a move (action)

# re-sample -> isotropic (2D 2mm) (3D 1mm)

# gamma = 0.9 , replay memory size P = 100000 , learning rate = 0.00025
# net : 3 conv+pool - 3 FC+dropout (3D kernels for 3d data)

# navigate till oscillation happen (terminate when infinite loop)

# location is a high-confidence landmark -> if the expected reward from this location is max(q*(s_target,a))<1 the agent is closer than one pixel

# object is not in the image: oscillation occurs at points where max(q)>4


## Other Notes:

    DeepMind's original DQN paper
        used frame skipping (for fast playing/learning) and
        applied pixel-wise max to consecutive frames (to handle flickering).

    so an input to the neural network is consisted of four frame;
        [max(T-1, T), max(T+3, T+4), max(T+7, T+8), max(T+11, T+12)]

    ALE provides mechanism for frame skipping (combined with adjustable random repeat) and color averaging over skipped frames. This is also used in simple_dqn's ALEEnvironment

    Gym's Atari Environment has built-in stochastic frame skipping common to all games. So the frames returned from environment are not consecutive.

    The reason behind Gym's stochastic frame skipping is, as mentioned above, to make environment stochastic. (I guess without this, the game will be completely deterministic?)
    cf. in original DQN and simple_dqn same randomness is achieved by having agent performs random number of dummy actions at the beginning of each episode.

    I think if you want to reproduce the behavior of the original DQN paper, the easiest will be disabling frame skip and color averaging in ALEEnvironment then construct the mechanism on agent side.


"""
