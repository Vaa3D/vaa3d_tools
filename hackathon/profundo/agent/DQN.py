#!/usr/bin/env python
# -*- coding: utf-8 -*-
# File: DQN.py
# Author: Amir Alansary <amiralansary@gmail.com>

def warn(*args, **kwargs):
    pass


import warnings

warnings.warn = warn
warnings.simplefilter("ignore", category=PendingDeprecationWarning)
import faulthandler

faulthandler.enable()
import numpy as np

import os
import sys

# get around module not found errors
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
import time
import argparse
from collections import deque

import tensorflow as tf
from brain_env import Brain_Env, FrameStack
from tensorpack.input_source import QueueInput
from tensorpack_medical.models.conv3d import Conv3D
from tensorpack_medical.models.pool3d import MaxPooling3D
from common import Evaluator, eval_model_multithread, play_n_episodes
from DQNModel import Model3D as DQNModel
from expreplay import ExpReplay

from tensorpack import (PredictConfig, OfflinePredictor, get_model_loader,
                        logger, TrainConfig, ModelSaver, PeriodicTrigger,
                        ScheduledHyperParamSetter, ObjAttrParam,
                        HumanHyperParamSetter, argscope, RunOp, LinearWrap,
                        FullyConnected, PReLU, SimpleTrainer,
                        launch_train_with_config)

from data_processing.swc_io import get_fnames_and_abspath_from_dir
from sklearn.model_selection import train_test_split
LeakyRelu = tf.nn.leaky_relu
###############################################################################
# import your data here
data_dir = "../data/08_cube_npy"
fnames, abs_paths = get_fnames_and_abspath_from_dir(data_dir)
train_data_fpaths, test_data_fpaths = train_test_split(abs_paths, test_size=0.7, shuffle=True)

logger_dir = os.path.join('train_log', 'expriment_1')

###############################################################################
# BATCH SIZE USED IN NATURE PAPER IS 32 - MEDICAL IS 256
BATCH_SIZE = 48
# BREAKOUT (84,84) - MEDICAL 2D (60,60) - MEDICAL 3D (26,26,26)
OBSERVATION_DIMS = (11, 11, 11)
# how many frames to keep
# in other words, how many past observations the network can see
FRAME_HISTORY = 4
# the frequency of updating the target network
UPDATE_FREQ = 4
# DISCOUNT FACTOR - NATURE (0.99) - MEDICAL (0.9)
GAMMA = 0.9  # 0.99
# REPLAY MEMORY SIZE - NATURE (1e6) - MEDICAL (1e5 view-patches)
MEMORY_SIZE = 1e5  # 6
# consume at least 1e6 * 27 * 27 * 27 bytes
INIT_MEMORY_SIZE = MEMORY_SIZE // 20  # 5e4
# each epoch is 100k played frames
STEPS_PER_EPOCH = 10000 // UPDATE_FREQ * 10
# num training epochs in between model evaluations
EPOCHS_PER_EVAL = 2
# the number of episodes to run during evaluation
EVAL_EPISODE = 50
MAX_EPISODE_LENGTH = 5


###############################################################################

# FIXME hard coded save video True
def get_player(directory=None, files_list= None, viz=False,
               task=False, saveGif=False, saveVideo=True):
    # in atari paper, max_num_frames = 30000
    env = Brain_Env(directory=directory, observation_dims=OBSERVATION_DIMS,
                    viz=viz, saveGif=saveGif, saveVideo=saveVideo,
                    task=task, files_list=files_list, max_num_frames=MAX_EPISODE_LENGTH)
    if (task != 'train'):
        # in training, env will be decorated by ExpReplay, and history
        # is taken care of in expreplay buffer
        # otherwise, FrameStack modifies self.step to save observations into a queue
        env = FrameStack(env, FRAME_HISTORY)
    return env


###############################################################################

class Model(DQNModel):
    def __init__(self):
        super(Model, self).__init__(OBSERVATION_DIMS, FRAME_HISTORY, METHOD, NUM_ACTIONS, GAMMA)

    def _get_DQN_prediction(self, image):
        """ image: [0,255]

        :returns predicted Q values"""
        # FIXME norm not needed
        # normalize image values to [0, 1]
        image = image / 255.0

        with argscope(Conv3D, nl=PReLU.symbolic_function, use_bias=True):
            # core layers of the network
            conv = (LinearWrap(image)
                    .Conv3D('conv0', out_channel=32,
                            kernel_shape=[5, 5, 5], stride=[1, 1, 1])
                    .MaxPooling3D('pool0', 2)
                    .Conv3D('conv1', out_channel=32,
                            kernel_shape=[5, 5, 5], stride=[1, 1, 1])
                    .MaxPooling3D('pool1', 2)
                    .Conv3D('conv2', out_channel=64,
                            kernel_shape=[4, 4, 4], stride=[1, 1, 1])
                    .MaxPooling3D('pool2', 2)
                    .Conv3D('conv3', out_channel=64,
                            kernel_shape=[3, 3, 3], stride=[1, 1, 1])
                    # .MaxPooling3D('pool3',2)
                    )

        if 'Dueling' not in self.method:
            lq = (conv
                  .FullyConnected('fc0', 512).tf.nn.leaky_relu(alpha=0.01)
                  .FullyConnected('fc1', 256).tf.nn.leaky_relu(alpha=0.01)
                  .FullyConnected('fc2', 128).tf.nn.leaky_relu(alpha=0.01)())
            Q = FullyConnected('fct', lq, self.num_actions, nl=tf.identity)
        else:
            # Dueling DQN or Double Dueling
            # state value function
            lv = (conv
                  .FullyConnected('fc0V', 512).tf.nn.leaky_relu(alpha=0.01)
                  .FullyConnected('fc1V', 256).tf.nn.leaky_relu(alpha=0.01)
                  .FullyConnected('fc2V', 128).tf.nn.leaky_relu(alpha=0.01)())
            V = FullyConnected('fctV', lv, 1, nl=tf.identity)
            # advantage value function
            la = (conv
                  .FullyConnected('fc0A', 512).tf.nn.leaky_relu(alpha=0.01)
                  .FullyConnected('fc1A', 256).tf.nn.leaky_relu(alpha=0.01)
                  .FullyConnected('fc2A', 128).tf.nn.leaky_relu(alpha=0.01)())
            As = FullyConnected('fctA', la, self.num_actions, nl=tf.identity)

            Q = tf.add(As, V - tf.reduce_mean(As, 1, keepdims=True))

        return tf.identity(Q, name='Qvalue')


###############################################################################

def get_config():
    """This is only used during training."""
    expreplay = ExpReplay(
        predictor_io_names=(['state'], ['Qvalue']),
        player=get_player(directory=data_dir, task='train',
                          files_list=train_data_fpaths),
        state_shape=OBSERVATION_DIMS,
        batch_size=BATCH_SIZE,
        memory_size=MEMORY_SIZE,
        init_memory_size=INIT_MEMORY_SIZE,
        init_exploration=1.0,
        update_frequency=UPDATE_FREQ,
        history_len=FRAME_HISTORY
    )

    return TrainConfig(
        # dataflow=expreplay,
        data=QueueInput(expreplay),
        model=Model(),
        callbacks=[  # TODO: periodically save videos
            ModelSaver(checkpoint_dir="model_checkpoints",
                       max_to_keep=1000),
            PeriodicTrigger(
                RunOp(DQNModel.update_target_param, verbose=True),
                # update target network every 10k steps
                every_k_steps=10000 // UPDATE_FREQ),
            expreplay,
            ScheduledHyperParamSetter('learning_rate',
                                      [(60, 4e-4), (100, 2e-4)]),
            ScheduledHyperParamSetter(
                ObjAttrParam(expreplay, 'exploration'),
                # 1->0.1 in the first million steps
                [(0, 1), (10, 0.1), (320, 0.01)],
                interp='linear'),
            PeriodicTrigger(
                Evaluator(nr_eval=EVAL_EPISODE, input_names=['state'],
                          output_names=['Qvalue'], directory=data_dir,
                          files_list=test_data_fpaths, get_player_fn=get_player),
                every_k_epochs=EPOCHS_PER_EVAL),
            HumanHyperParamSetter('learning_rate'),
        ],
        steps_per_epoch=STEPS_PER_EPOCH,
        max_epoch=1000,
    )


###############################################################################
###############################################################################


if __name__ == '__main__':
    # e.g. python DQN.py --algo DQN --gpu 0
    parser = argparse.ArgumentParser()

    parser.add_argument('--gpu', help='comma separated list of GPU(s) to use.')
    parser.add_argument('--load', help='load model')
    parser.add_argument('--task', help='task to perform. Must load a pretrained model if task is "play" or "eval"',
                        choices=['play', 'eval', 'train'], default='train')
    parser.add_argument('--algo', help='algorithm',
                        choices=['DQN', 'Double', 'Dueling', 'DuelingDouble'],
                        default='DQN')
    parser.add_argument('--saveGif', help='save gif image of the game',
                        action='store_true', default=False)
    parser.add_argument('--saveVideo', help='save video of the game',
                        action='store_true', default=False)
    args = parser.parse_args()

    if args.gpu:
        os.environ['CUDA_VISIBLE_DEVICES'] = args.gpu

    METHOD = args.algo
    # load files into env to set num_actions, num_validation_files
    init_player = Brain_Env(directory=data_dir,
                            files_list=test_data_fpaths,
                            observation_dims=OBSERVATION_DIMS)
    NUM_ACTIONS = init_player.action_space.n
    num_validation_files = init_player.files.num_files

    if args.task != 'train':
        assert args.load is not None
        pred = OfflinePredictor(PredictConfig(
            model=Model(),
            session_init=get_model_loader(args.load),
            input_names=['state'],
            output_names=['Qvalue']))
        # demo pretrained model one episode at a time
        if args.task == 'play':
            play_n_episodes(get_player(directory=data_dir,
                                       files_list=test_data_fpaths, viz=0.01,
                                       saveGif=args.saveGif,
                                       saveVideo=args.saveVideo,
				       task='play'),
                            pred, num_validation_files)
        # run episodes in parallel and evaluate pretrained model
        elif args.task == 'eval':
            play_n_episodes(get_player(directory=data_dir,
                                       files_list=eval_list, viz=0.01,
                                       saveGif=args.saveGif,
                                       saveVideo=args.saveVideo,
                                       task='eval'),
                            pred, num_files)
    else:  # train model
        logger.set_logger_dir(logger_dir)
        config = get_config()
        if args.load:  # resume training from a saved checkpoint
            config.session_init = get_model_loader(args.load)
        launch_train_with_config(config, SimpleTrainer())
