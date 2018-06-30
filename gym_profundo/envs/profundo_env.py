import os, signal
import gym
from gym import error, spaces, logger
from gym.utils import seeding
import numpy as np

class ProfundoEnv(gym.Env):
    metadata = {'render.modes': ['human']}  # fps?

    def __init__(self):
        self.agent_stepsize = 1.0
        self.viewer = None
        self.vaa3d_path = None # TODO
        # TODO update obs space
        self.observation_space = spaces.Box(low=-5, high=5)
        self.action_space = spaces.Discrete(6)

        self.seed()
        self.viewer = None
        self.state = None


    def seed(self, seed=None):
        self.np_random, seed = seeding.np_random(seed)

        return [seed]

    def __del__(self):
        self.env.act(game.QUIT)
        self.env.step()
        os.kill(self.server_process.pid, signal.SIGINT)
        if self.viewer is not None:
            os.kill(self.viewer.pid, signal.SIGKILL)



    def step(self, action):
        self._take_action(action)
        self.status = self.env.step()
        reward = self._get_reward()
        ob = self.env.getState()
        episode_over = self.status != game.IN_GAME
        return ob, reward, episode_over, {}

    def _take_action(self, action):
        """ Converts the action space into an HFO action. """
        action_type = ACTION_LOOKUP[action[0]]
        if action_type == game.DASH:
            self.env.act(action_type, action[1], action[2])
        elif action_type == game.TURN:
            self.env.act(action_type, action[3])
        elif action_type == game.KICK:
            self.env.act(action_type, action[4], action[5])
        else:
            print('Unrecognized action %d' % action_type)
            self.env.act(game.NOOP)

    def _get_reward(self):
        """ Reward is given for scoring a goal. """
        if self.status == game.GOAL:
            return 1
        else:
            return 0

    def reset(self):
        """ Repeats NO-OP action until a new episode begins. """
        while self.status == game.IN_GAME:
            self.env.act(game.NOOP)
            self.status = self.env.step()
        while self.status != game.IN_GAME:
            self.env.act(game.NOOP)
            self.status = self.env.step()
        return self.env.getState()

    def render(self, mode='human', close=False):
        """ Viewer only supports human mode currently. """
        if close:
            if self.viewer is not None:
                os.kill(self.viewer.pid, signal.SIGKILL)
        else:
            if self.viewer is None:
                self._start_viewer()

ACTION_LOOKUP = {
    0 : "up",
    1 : "down",
    2 : "left",
    3 : "right",
    4 : "up",
    5 : "down"
}
