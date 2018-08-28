import unittest
from unittest import TestCase
from brain_env import Brain_Env
from data_processing.swc_io import get_fnames_and_abspath_from_dir
from sampleTrain import FilesListCubeNPY
import numpy as np
from jaccard import jaccard
from data_processing.swc_io import linked_list_2_swc, swc_to_TIFF, TIFF_to_npy




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

    def test_jaccard_same(self):
        assert np.isclose(jaccard(self.env.original_state, self.env.original_state), 1)


    @unittest.skip("not implemented")
    def test_new_random_game(self):
        self.fail()

    def test_calc_IOU(self):
        """method tests self._state with self.original_state. _state gets masked out everywhere
        that isn't -1"""
        agent_state = self.env.original_state * -1
        self.env._state = agent_state
        assert np.isclose(self.env.calc_IOU(), 1)

    def test_is_in_bounds(self):
        self.fail()



class TestJaccard(TestCase):
    def identical_ones(self):
        a = np.ones((10, 10))
        a_copy = np.array(a, copy=True)
        assert np.isclose(jaccard(a, a_copy), 1)

    def all_zeros(self):
        a = np.zeros((10, 10))
        a_copy = np.array(a, copy=True)
        assert np.isnan(jaccard(a, a_copy))

    def one_different(self):
        a = np.zeros((10, 10))
        b = np.array(a, copy=True)
        a[0,0] = 1
        assert np.isclose(jaccard(a, b), 0)

    def one_same_one_different(self):
        a = np.zeros((10, 10))
        b = np.array(a, copy=True)
        a[0, 0] = 1
        b[0:2] = 1
        assert np.isclose(jaccard(a, b), 0.5)

        b_downwards = np.zeros_like(a)
        b_downwards[:2, 0] = 1
        assert np.isclose(jaccard(a, b), 0.5)

if __name__ == '__main__':
    unittest.main()