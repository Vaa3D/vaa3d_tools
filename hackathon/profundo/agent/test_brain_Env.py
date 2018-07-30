import unittest
from unittest import TestCase
from brain_env import Brain_Env
from data_processing.swc_io import get_fnames_and_abspath_from_dir
from sampleTrain import FilesListCubeNPY
import numpy as np
from jaccard import jaccard



class TestBrain_Env(TestCase):

    def setUp(self):
        data_dir = "../data/08_cube_npy"

        fnames, abs_paths = get_fnames_and_abspath_from_dir(data_dir)
        # train_data_fpaths, test_data_fpaths = train_test_split(abs_paths, test_size=0.7, shuffle=True)
        self.env = Brain_Env(directory=data_dir, observation_dims=(15,15,15),
                    viz=False, saveGif=False, saveVideo=False,
                    task='train', files_list=abs_paths, max_num_frames=10)  # TODO add args
        # always load same file
        files = FilesListCubeNPY(directory=data_dir, filepaths_list=abs_paths)
        self.env.file_sampler = files.sample_first()
        self.env._clear_history()
        self.env._restart_episode()

    def test_jaccard(self):
        assert np.isclose(jaccard(self.env.original_state, self.env.original_state), 1)

    def test_new_random_game(self):
        self.fail()

    def test_calc_IOU(self):
        """method tests self._state with self.original_state"""
        self.fail()

if __name__ == '__main__':
    unittest.main()