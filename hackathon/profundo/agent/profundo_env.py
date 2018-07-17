import os, signal
import gym
from gym import error, spaces, logger
from gym.utils import seeding
import numpy as np

# TODO: port to brain_env

class ProfundoEnv(gym.Env):
    metadata = {'render.modes': ['human']}  # fps?

    def __init__(self):
        self.agent_position = None
        self.agent_stepsize = 1.0
        self.viewer = None
        self.vaa3d_path = None # TODO
        # TODO update obs space
        self.observation_space = spaces.Box(np.array([-5, -5, -5]),
                                            np.array([5, 5, 5]))
        self.action_space = spaces.Discrete(6)

        self.seed()
        self.origin = np.array([0., 0., 0.])
        self.state = self.origin  # TODO: should state be the multichannel image?

        self.n_steps_taken = 0
        self.max_steps_per_episode = 100

        self.out_of_bounds = False

        self.viewer = None

    def __init_env__(self):
        # TODO:
        pass


    def seed(self, seed=None):
        self.np_random, seed = seeding.np_random(seed)

        return [seed]

    def __del__(self):
        pass



    def step(self, action):
        assert self.action_space.contains(action), "%r (%s) invalid" % (action, type(action))

        step = self._action_to_step(action)
        self.state = self.state + step

        self.n_steps_taken += 1
        done = bool(self.n_steps_taken > self.max_steps_per_episode)

        if not done:
            reward = self._calc_reward()
        elif self.out_of_bounds:
            # Pole just fell!
            # self.steps_beyond_done = 0
            reward = 1.0
        else:
            # if self.steps_beyond_done == 0:
            #     logger.warn(
            #         "You are calling 'step()' even though this environment has already returned done = True. You should always call 'reset()' once you receive 'done = True' -- any further steps are undefined behavior.")
            # self.steps_beyond_done += 1
            reward = 0.0

        return self.state, reward, done, {}


    def _action_to_step(self, action):
        if action == 0:  # up
            step = [0, self.agent_stepsize, 0]
        elif action == 1:  # down
            step = [0, -self.agent_stepsize, 0]
        elif action == 2:  # left
            step = [-self.agent_stepsize, 0, 0]
        elif action == 3:  # right
            step = [self.agent_stepsize, 0, 0]
        elif action == 4:  # in
            step = [0, 0, -self.agent_stepsize]
        elif action == 5:  # out
            step = [0, 0, self.agent_stepsize]
        return np.array(step)


    def _calc_reward(self):
        """ Reward is given for scoring a goal. """
        # TODO: implement
        return 0

    def reset(self):
        # assume we always start at origin
        self.state =
        return self.state

    def reset_agent_position(self):
        # TODO reset to origin
        self.agent_position = None

    def render(self, mode='human', close=False):
        """ Viewer only supports human mode currently.

        I don't want to reimplement animations to update frame by frame,
        so only render if  done"""
        if self.viewer is None:
            from rendering import Viewer
            self.viewer = Viewer()


    def close(self):
        if self.viewer:
            self.view.close()

