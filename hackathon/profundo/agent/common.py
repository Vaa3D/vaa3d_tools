#!/usr/bin/env python
# -*- coding: utf-8 -*-
# File: common.py
# Author: Yuxin Wu <ppwwyyxxc@gmail.com>
# Modified: Amir Alansary <amiralansary@gmail.com>

import random
import time
import threading
import numpy as np
from tqdm import tqdm
import multiprocessing
from six.moves import queue

# from tensorpack import *
# from tensorpack.utils.stats import *
from tensorpack.utils import logger
# from tensorpack.callbacks import Triggerable
from tensorpack.callbacks.base import Callback
from tensorpack.utils.stats import StatCounter
from tensorpack.utils.utils import get_tqdm_kwargs
from tensorpack.utils.concurrency import (StoppableThread, ShareSessionThread)

import traceback


###############################################################################

def play_one_episode(env, func, render=False):
    def predict(s):
        """
        see also expreplay _populate_exp()

        Run a full episode, mapping observation to action, WITHOUT 0.001 greedy.
    :returns sum of rewards
        """
        # pick action with best predicted Q-value
        q_values = func(s[None, :, :, :, :])[0][0]
        # if there is a tie for max, randomly choose between them
        act = np.random.choice(np.flatnonzero(np.isclose(q_values, q_values.max(), atol=0.5)))


        # eps greedy disabled
        # if random.random() < 0.001:
        #     spc = env.action_space
        #     act = spc.sample()
        return act, q_values

    ob = env.reset()
    sum_r = 0
    while True:
        act, q_values = predict(ob)
        # print("play_one act {} qvals {}".format(act, q_values))
        ob, r, isOver, info = env.step(act, q_values)
        # print("play q vals", q_values, "act ", act, "env_act ", env.env._act, "loc ", env.env._location)
        if render:
            env.render()
        sum_r += r
        if isOver:
            return sum_r, info['filename'], info['IoU'], q_values


###############################################################################

def play_n_episodes(player, predfunc, nr, render=False):
    """wraps play_one_episode, playing a single episode at a time and logs results
    used when playing demos."""
    logger.info("Start Playing ... ")
    for k in range(nr):
        # if k != 0:
        #     player.restart_episode()
        score, filename, IoU, q_values = play_one_episode(player,
                                                                    predfunc,
                                                                    render=render)
        logger.info(
            "{}/{} {} \m score:{}  IoU: {}".format(k + 1, nr, filename, score, IoU))


###############################################################################

def eval_with_funcs(predictors, nr_eval, get_player_fn,
                    directory=None, files_list=None):
    """
    Args:
        predictors ([PredictorBase])

    Runs episodes in parallel, returning statistics about the model performance.
    """

    class Worker(StoppableThread, ShareSessionThread):
        def __init__(self, func, queue, IoUErrorQueue):
            super(Worker, self).__init__()
            self._func = func
            self.q = queue
            self.q_IoU = IoUErrorQueue

        def func(self, *args, **kwargs):
            if self.stopped():
                raise RuntimeError("stopped!")
            return self._func(*args, **kwargs)

        def run(self):
            with self.default_sess():
                player = get_player_fn(directory=directory,
                                       task=False,
                                       files_list=files_list)
                while not self.stopped():
                    try:
                        score, filename, ditance_error, q_values = play_one_episode(player, self.func)
                        # print("Score, ", score)
                    except RuntimeError:
                        return
                    self.queue_put_stoppable(self.q, score)
                    self.queue_put_stoppable(self.q_IoU, ditance_error)

    q = queue.Queue()
    q_IoU = queue.Queue()

    threads = [Worker(f, q, q_IoU) for f in predictors]

    # start all workers
    for k in threads:
        k.start()
        time.sleep(0.1)  # avoid simulator bugs
    stat = StatCounter()
    IoU_stat = StatCounter()

    # show progress bar w/ tqdm
    for _ in tqdm(range(nr_eval), **get_tqdm_kwargs()):
        r = q.get()
        stat.feed(r)
        IoU = q_IoU.get()
        IoU_stat.feed(IoU)

    logger.info("Waiting for all the workers to finish the last run...")
    for k in threads:
        k.stop()
    for k in threads:
        k.join()
    while q.qsize():
        r = q.get()
        stat.feed(r)

    while q_IoU.qsize():
        IoU = q_IoU.get()
        IoU_stat.feed(IoU)

    if stat.count > 0:
        return (stat.average, stat.max, IoU_stat.average, IoU_stat.max)
    return (0, 0, 0, 0)


###############################################################################

def eval_model_multithread(pred, nr_eval, get_player_fn, directory, files_list):
    """
    Args:
        pred (OfflinePredictor): state -> Qvalue

    Evaluate pretrained models, or checkpoints of models during training
    """
    NR_PROC = min(multiprocessing.cpu_count() // 2, 8)
    with pred.sess.as_default():
        mean_score, max_score, mean_IoU, max_IoU = eval_with_funcs(
            [pred] * NR_PROC, nr_eval, get_player_fn,
            directory, files_list)
    logger.info("Average Score: {}; Max Score: {}; Average IoU: {}; Max IoU: {}".format(mean_score, max_score, mean_IoU, max_IoU))

###############################################################################

class Evaluator(Callback):

    def __init__(self, nr_eval, input_names, output_names,
                 directory, get_player_fn, files_list=None):
        self.directory = directory
        self.files_list = files_list
        self.eval_episode = nr_eval
        self.input_names = input_names
        self.output_names = output_names
        self.get_player_fn = get_player_fn

    def _setup_graph(self):
        NR_PROC = min(multiprocessing.cpu_count() // 2, 20)
        self.pred_funcs = [self.trainer.get_predictor(
            self.input_names, self.output_names)] * NR_PROC

    def _trigger(self):
        """triggered by Trainer"""
        t = time.time()
        mean_score, max_score, mean_IoU, max_IoU = eval_with_funcs(
            self.pred_funcs, self.eval_episode, self.get_player_fn,
            self.directory, self.files_list)
        t = time.time() - t
        if t > 10 * 60:  # eval takes too long
            self.eval_episode = int(self.eval_episode * 0.94)

        # log scores
        self.trainer.monitors.put_scalar('evaluator/mean_score', mean_score)
        self.trainer.monitors.put_scalar('evaluator/max_score', max_score)
        self.trainer.monitors.put_scalar('evaluator/mean_IoU', mean_IoU)
        self.trainer.monitors.put_scalar('evaluator/max_IoU', max_IoU)

###############################################################################
