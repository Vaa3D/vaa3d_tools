#!/usr/bin/env python
# -*- coding: utf-8 -*-
# File: DQNModel.py
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>
# Modified: Amir Alansary <amiralansary@gmail.com>

import abc
import tensorflow as tf
from tensorpack import ModelDesc, InputDesc
from tensorpack.utils import logger
from tensorpack.tfutils import (
    collection, summary, get_current_tower_context, optimizer, gradproc)
from tensorpack.tfutils.scope_utils import auto_reuse_variable_scope


class Model3D(ModelDesc):
    def __init__(self, image_shape, channel, method, num_actions, gamma):
        """
        :param image_shape: the shape of input 3d image
        :param channel: history length and goes to channel dimension in kernel
        :param method: dqn or double (default is double)
        :param num_actions: number of actions
        :param gamma: discount factor

        See http://tensorpack.readthedocs.io/tutorial/training-interface.html for Mode lDesc documentation.
        """
        self.gamma = gamma
        self.method = method
        self.channel = channel
        self.image_shape = image_shape
        self.num_actions = num_actions

    def _get_inputs(self):
        # Use a combined state for efficiency.
        # The first h channels are the current state, and the last h channels are the next state.
        return [InputDesc(tf.uint8,
                          (None,) + self.image_shape + (self.channel + 1,),
                          'comb_state'),
                InputDesc(tf.int64, (None,), 'action'),
                InputDesc(tf.float32, (None,), 'reward'),
                InputDesc(tf.bool, (None,), 'isOver')]

    @abc.abstractmethod
    def _get_DQN_prediction(self, image):
        """this method is overridden in DQN.py, where it will return a list of predicted Q-values"""
        pass

    # decorate the function
    @auto_reuse_variable_scope
    def get_DQN_prediction(self, image):
        return self._get_DQN_prediction(image)

    def _build_graph(self, inputs):
        # shapes:
        # (?, xdim, ydim, zdim, n_channels) ?, ?, ?
        # specifically, (?, 15, 15, 15, 5)
        comb_state, action, reward, isOver = inputs
        # print("state stuff", comb_state.get_shape().as_list(), action.get_shape().as_list(),
        #       reward.get_shape().as_list(), isOver.get_shape().as_list(), flush=True)

        comb_state = tf.cast(comb_state, tf.float32)
        # TODO: wtf is this?
        # comb_state = tf.Print(comb_state, [comb_state, (comb_state.get_shape().as_list())], message="combstate shp", summarize=50)
        # action = tf.Print(action, [comb_state, (action.get_shape().as_list())], message="action shp", summarize=50)
        # print("combstate shape ", comb_state, flush=True)
        # print("action shape", action.get_shape().as_list(), flush=True)
        # print("reward shape", reward.get_shape().as_list())
        #
        # print("isOver shape", (isOver.get_shape().as_list()))
        # state is now [None, 15, 15, 15, 4] according to state.get_shape().as_list())
        state = tf.slice(comb_state, [0, 0, 0, 0, 0], [-1, -1, -1, -1, self.channel], name='state')

        self.predict_value = self.get_DQN_prediction(state)  # shape: (batch_size, action_space_size)
        tf.summary.histogram("Q_vals", self.predict_value)

        if not get_current_tower_context().is_training:
            return

        # TODO check that this is ok
        # reward = tf.clip_by_value(reward, -1, 1)
        # FIXME I think this is history buffer stuff
        # comb_state = tf.Print(comb_state, [comb_state, tf.shape(comb_state)], message="tf Print out", summarize=50)
        # next_state shp [None, 15, 15, 15, 4]
        next_state = tf.slice(comb_state, [0, 0, 0, 0, 1], [-1, -1, -1, -1, self.channel], name='next_state')
        # print("next_state shape", (next_state.get_shape().as_list()))
        # one_hot params: (indices, depth, on_value, off_value)
        # action_onehot shp: [None, 6]
        action_onehot = tf.one_hot(action, self.num_actions, 1.0, 0.0)
        # print("action_onehot shape", (action_onehot.get_shape().as_list()))
        # this is a fancy way of selecting the predicted value of action taken by agent
        # pred_action_value shape: ?
        pred_action_value = tf.reduce_sum(self.predict_value * action_onehot, axis=1)  # shape: batchsize,
        # print("pred_action_value shape", (pred_action_value.get_shape().as_list()))
        # select maximum q val for each
        # FIXME shape: EMPTY?
        max_pred_reward = pred_action_value
        # max_pred_reward = tf.reduce_mean(tf.reduce_max(
        #     self.predict_value, axis=1), name='predict_reward')
        # max_pred_reward = tf.Print(max_pred_reward, [max_pred_reward.get_shape().as_list()], message="max_pred_reward shp", summarize=50)
        summary.add_moving_summary(max_pred_reward)

        with tf.variable_scope('target'):
            targetQ_predict_value = self.get_DQN_prediction(next_state)  # shape: batch_size x action_space
            # print("targetQ_predict_value shape", (targetQ_predict_value.get_shape().as_list()))

        # TODO disable other models
        if 'Double' not in self.method:
            # DQN or Dueling
            best_v = tf.reduce_max(targetQ_predict_value, axis=1)  # shape: batch_size
        else:
            # Double-DQN or DuelingDouble
            next_predict_value = self.get_DQN_prediction(next_state)  # FIXME isnt this redundant
            self.greedy_choice = tf.argmax(next_predict_value, axis=1)  # N,
            predict_onehot = tf.one_hot(self.greedy_choice, self.num_actions, 1.0, 0.0)
            best_v = tf.reduce_sum(targetQ_predict_value * predict_onehot, axis=1)

        # target shape is ?
        # why do we care about the Q-vals in the next state? arent they irrelevant?
        # why can't we just use the rewards directly?
        target = reward + (1.0 - tf.cast(isOver, tf.float32)) * self.gamma * tf.stop_gradient(best_v)
        print("target shape ", target.get_shape().as_list())
        # loss between the predicted reward for selected action and actual reward recieved
        # TODO is this true?
        # bestV, target comes from discounted*get_DQN_prediction(next_state) + REWARD
        # but pred_val comes from get_DQN_prediction(current_state)
        # FIXME this assumes that the max of both target and prediction will be the same
        self.cost = tf.losses.huber_loss(target, pred_action_value,
                                         reduction=tf.losses.Reduction.MEAN)
        print("cost shape ", self.cost.get_shape().as_list())
        summary.add_param_summary(('conv.*/W', ['histogram', 'rms']),
                                  ('fc.*/W', ['histogram', 'rms']))  # monitor all W
        summary.add_moving_summary(self.cost)

    def _get_optimizer(self):
        lr = tf.get_variable('learning_rate', initializer=1e-3, trainable=False)
        opt = tf.train.AdamOptimizer(lr, epsilon=1e-3)
        return optimizer.apply_grad_processors(
            opt, [gradproc.GlobalNormClip(10), gradproc.SummaryGradient()])

    @staticmethod
    def update_target_param():
        """periodically triggered by trainer"""
        vars = tf.global_variables()
        ops = []
        G = tf.get_default_graph()
        for v in vars:
            target_name = v.op.name
            if target_name.startswith('target'):
                new_name = target_name.replace('target/', '')
                logger.info("{} <- {}".format(target_name, new_name))
                ops.append(v.assign(G.get_tensor_by_name(new_name + ':0')))
        return tf.group(*ops, name='update_target_network')
