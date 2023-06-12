import importlib
import numpy as np
import torch
from scipy.ndimage import rotate, map_coordinates, gaussian_filter, zoom
from scipy.ndimage.filters import convolve
from skimage import measure
from skimage.filters import gaussian
from skimage.segmentation import find_boundaries

GLOBAL_RANDOM_STATE = np.random.RandomState(47)

class RandomFlip:
    """
    Randomly flips the image across the given axes. Image can be either 3D (DxHxW) or 4D (CxDxHxW).

    When creating make sure that the provided RandomStates are consistent between raw and labeled datasets,
    otherwise the models won't converge.
    """

    def __init__(self, p,random_state, axis_prob=0.5, axes=[0,1,2], **kwargs):
        assert random_state is not None, 'RandomState cannot be None'
        self.p=p
        self.random_state = random_state
        self.axes = axes
        self.axis_prob = axis_prob

    def __call__(self, m,n):
        assert m.ndim in [3, 4], 'Supports only 3D (DxHxW) or 4D (CxDxHxW) images'
        if np.random.random()>1-self.p:
            return m,n
        for axis in self.axes:
            if self.random_state.uniform() > self.axis_prob:
                if m.ndim == 3:
                    m = np.flip(m, axis)
                    n = np.flip(n, axis)
                else:
                    channels = [np.flip(m[c], axis) for c in range(m.shape[0])]
                    m = np.stack(channels, axis=0)
        # print("flip",np.max(m),np.min(m))
        # print("flip", np.max(n), np.min(n))
        return m,n


class RandomRotate90:
    """
    Rotate an array by 90 degrees around a randomly chosen plane. Image can be either 3D (DxHxW) or 4D (CxDxHxW).

    When creating make sure that the provided RandomStates are consistent between raw and labeled datasets,
    otherwise the models won't converge.

    IMPORTANT: assumes DHW axis order (that's why rotation is performed across (1,2) axis)
    """

    def __init__(self, p,random_state, **kwargs):
        self.random_state = random_state
        # always rotate around z-axis
        self.axis = (1, 2)
        self.p=p
    def __call__(self, m,n):
        assert m.ndim in [3, 4], 'Supports only 3D (DxHxW) or 4D (CxDxHxW) images'
        if np.random.random()>1-self.p:
            return m,n
        # pick number of rotations at random
        k = self.random_state.randint(0, 4)
        # rotate k times around a given plane
        if m.ndim == 3:
            m = np.rot90(m, k, self.axis)
            n = np.rot90(n, k, self.axis)
            #  y: The first two dimensions are rotated; therefore, the array must be at least 2-D.
        else:
            channels = [np.rot90(m[c], k, self.axis) for c in range(m.shape[0])]
            m = np.stack(channels, axis=0)
        # print("r90",np.max(m),np.min(m))
        # print("r90", np.max(n), np.min(n))
        return m,n


class RandomRotate:
    """
    Rotate an array by a random degrees from taken from (-angle_spectrum, angle_spectrum) interval.
    Rotation axis is picked at random from the list of provided axes.
    """

    def __init__(self, p,random_state, angle_spectrum=60, axes=None, mode='constant', order=0, **kwargs):
        if axes is None:
            axes = [(1, 0), (2, 1), (2, 0)]
        else:
            assert isinstance(axes, list) and len(axes) > 0

        self.random_state = random_state
        self.angle_spectrum = angle_spectrum
        self.axes = axes
        self.mode = mode
        self.order = order
        self.p=p
    def __call__(self, m,n):
        if np.random.random()>1-self.p:
            return m,n
        axis = self.axes[self.random_state.randint(len(self.axes))]
        angle = self.random_state.randint(-self.angle_spectrum, self.angle_spectrum)

        if m.ndim == 3:
            m = rotate(m, angle, axes=axis, reshape=False, order=self.order, mode='nearest', cval=-1)
            n = rotate(n, angle, axes=axis, reshape=False, order=self.order, mode=self.mode, cval=0)
        else:
            channels = [rotate(m[c], angle, axes=axis, reshape=False, order=self.order, mode=self.mode, cval=-1) for c
                        in range(m.shape[0])]
            m = np.stack(channels, axis=0)
        # print("r",np.max(m),np.min(m))
        # print("r", np.max(n), np.min(n))
        return m,n


class RandomContrast:
    """
    Adjust contrast by scaling each voxel to `mean + alpha * (v - mean)`.
    """

    def __init__(self, random_state, alpha=(0.5, 1.5), mean=0.0, execution_probability=1, **kwargs):
        self.random_state = random_state
        assert len(alpha) == 2
        self.alpha = alpha
        self.mean = mean
        self.execution_probability = execution_probability

    def __call__(self, m):
        if self.random_state.uniform() < self.execution_probability:
            alpha = self.random_state.uniform(self.alpha[0], self.alpha[1])
            result1 = self.mean + alpha * (m - self.mean)
            #result2 = self.mean + alpha * (n - self.mean)
            m=np.clip(result1, -1, 1)
            #n=np.clip(result2, -1, 1)
            m=m.astype(np.uint8)
            #n=n.astype(np.uint8)
            #print(m.dtype)
            #print(n.dtype)
            return m

        return m


class ElasticDeformation:
    """
    Apply elasitc deformations of 3D patches on a per-voxel mesh. Assumes ZYX axis order (or CZYX if the data is 4D).
    Based on: https://github.com/fcalvet/image_tools/blob/master/image_augmentation.py#L62
    """

    def __init__(self, p,random_state, spline_order, alpha=150, sigma=10, execution_probability=1, apply_3d=True,
                 **kwargs):
        """
        :param spline_order: the order of spline interpolation (use 0 for labeled images)
        :param alpha: scaling factor for deformations
        :param sigma: smoothing factor for Gaussian filter
        :param execution_probability: probability of executing this transform
        :param apply_3d: if True apply deformations in each axis
        """
        self.random_state = random_state
        self.spline_order = spline_order
        self.alpha = alpha
        self.sigma = sigma
        self.execution_probability = execution_probability
        self.apply_3d = apply_3d
        self.p=p
    def __call__(self,m,n):
        if np.random.random()>1-self.p:
            return m,n
        if self.random_state.uniform() < self.execution_probability:
            assert m.ndim in [3, 4]

            if m.ndim == 3:
                volume_shape = m.shape
            else:
                volume_shape = m[0].shape

            if self.apply_3d:
                dz = gaussian_filter(self.random_state.randn(*volume_shape), self.sigma, mode="reflect") * self.alpha
            else:
                dz = np.zeros_like(m)

            dy, dx = [
                gaussian_filter(
                    self.random_state.randn(*volume_shape),
                    self.sigma, mode="reflect"
                ) * self.alpha for _ in range(2)
            ]

            z_dim, y_dim, x_dim = volume_shape
            z, y, x = np.meshgrid(np.arange(z_dim), np.arange(y_dim), np.arange(x_dim), indexing='ij')
            # 网格坐标值 Cartesian (‘xy’, default) or matrix (‘ij’) indexing of output
            indices = z + dz, y + dy, x + dx

            if m.ndim == 3:
                return map_coordinates(m, indices, order=self.spline_order, mode='reflect'),map_coordinates(n, indices, order=0, mode='constant')
            #              coordinates : [[x1, x2], [y1, y2]]
            else:
                channels = [map_coordinates(m[c], indices, order=self.spline_order, mode='reflect')
                            for c in range(m.shape[0])]
                return np.stack(channels, axis=0)
        # print("el",np.max(m),np.min(m))
        # print("el", np.max(n), np.min(n))
        return m,n


class CropToFixed:
    def __init__(self, random_state, size=(64,64), centered=True, **kwargs):
        self.random_state = random_state
        self.crop_y, self.crop_x = size
        self.centered = centered

    def __call__(self, m,n):
        def _padding(pad_total):
            half_total = pad_total // 2
            return (half_total, pad_total - half_total)

        def _rand_range_and_pad(crop_size, max_size):
            """
            Returns a tuple:
                max_value (int) for the corner dimension. The corner dimension is chosen as `self.random_state(max_value)`
                pad (int): padding in both directions; if crop_size is lt max_size the pad is 0
            """
            if crop_size < max_size:
                return max_size - crop_size, (0, 0)
            else:
                return 1, _padding(crop_size - max_size)

        def _start_and_pad(crop_size, max_size):   # 中心裁剪
            if crop_size < max_size:
                return (max_size - crop_size) // 2, (0, 0)
            else:
                return 0, _padding(crop_size - max_size)

        assert m.ndim in (3, 4)
        if m.ndim == 3:
            _, y, x = m.shape
        else:
            _, _, y, x = m.shape

        if not self.centered:  # 随机裁剪
            y_range, y_pad = _rand_range_and_pad(self.crop_y, y)
            x_range, x_pad = _rand_range_and_pad(self.crop_x, x)

            y_start = self.random_state.randint(y_range)
            # If high is None (the default), then results are from [0, low).
            x_start = self.random_state.randint(x_range)

        else:   # 中心裁剪
            y_start, y_pad = _start_and_pad(self.crop_y, y)
            x_start, x_pad = _start_and_pad(self.crop_x, x)

        crop_yf = min(self.crop_y, y)
        crop_xf = min(self.crop_x, x)

        if m.ndim == 3:
            result1 = m[32:96, y_start:y_start + crop_yf, x_start:x_start + crop_xf]
            result2 = n[32:96, y_start:y_start + crop_yf, x_start:x_start + crop_xf]
            return np.pad(result1, pad_width=((0, 0), y_pad, x_pad), mode='reflect'),np.pad(result2, pad_width=((0, 0), y_pad, x_pad), mode='reflect')
        else:
            channels = []
            for c in range(m.shape[0]):
                result = m[c][:, y_start:y_start + crop_yf, x_start:x_start + crop_xf]
                channels.append(np.pad(result, pad_width=((0, 0), y_pad, x_pad), mode='reflect'))
            return np.stack(channels, axis=0)


class AdditiveGaussianNoise:
    def __init__(self, p,random_state, scale=(0.0, 0.01), execution_probability=1, **kwargs):
        self.execution_probability = execution_probability
        self.random_state = random_state
        self.scale = scale
        self.p=p
    def __call__(self, m,n):
        if np.random.random()>1-self.p:
            return m,n
        if self.random_state.uniform() < self.execution_probability:
            std = self.random_state.uniform(self.scale[0], self.scale[1])
            gaussian_noise = self.random_state.normal(0, std, size=m.shape)
            m=m + gaussian_noise
            m[m>1]=1

        # print("ad",np.max(m),np.min(m))
        # print("ad", np.max(n), np.min(n))
        return m,n


class AdditivePoissonNoise:
    def __init__(self, p,random_state, lam=(0.0, 0.01), execution_probability=1, **kwargs):
        self.execution_probability = execution_probability
        self.random_state = random_state
        self.lam = lam
        self.p=p
    def __call__(self, m,n):
        if np.random.random()>1-self.p:
            return m,n
        if self.random_state.uniform() < self.execution_probability:
            lam = self.random_state.uniform(self.lam[0], self.lam[1])
            poisson_noise = self.random_state.poisson(lam, size=m.shape)
            m=m+poisson_noise
            m[m>1]=1

        # print("ap",np.max(m),np.min(m))
        # print("ap", np.max(n), np.min(n))
        return m,n


class Standardize:
    """
    Apply Z-score normalization to a given input tensor, i.e. re-scaling the values to be 0-mean and 1-std.
    """

    def __init__(self, eps=1e-10, mean=None, std=None, channelwise=False, **kwargs):
        if mean is not None or std is not None:
            assert mean is not None and std is not None
        self.mean = mean
        self.std = std
        self.eps = eps
        self.channelwise = channelwise

    def __call__(self, m):
        if self.mean is not None:
            mean, std = self.mean, self.std
        else:
            if self.channelwise:
                # normalize per-channel
                axes = list(range(m.ndim))
                # average across channels
                axes = tuple(axes[1:])
                mean = np.mean(m, axis=axes, keepdims=True)
                std = np.std(m, axis=axes, keepdims=True)
            else:
                mean = np.mean(m)
                std = np.std(m)

        return (m - mean) / np.clip(std, a_min=self.eps, a_max=None)


class PercentileNormalizer:
    def __init__(self, pmin, pmax, channelwise=False, eps=1e-10, **kwargs):
        self.eps = eps
        self.pmin = pmin
        self.pmax = pmax
        self.channelwise = channelwise

    def __call__(self, m):
        if self.channelwise:
            axes = list(range(m.ndim))
            # average across channels
            axes = tuple(axes[1:])
            pmin = np.percentile(m, self.pmin, axis=axes, keepdims=True)
            pmax = np.percentile(m, self.pmax, axis=axes, keepdims=True)
        else:
            pmin = np.percentile(m, self.pmin)
            pmax = np.percentile(m, self.pmax)

        return (m - pmin) / (pmax - pmin + self.eps)


class Normalize:
    """
    Apply simple min-max scaling to a given input tensor, i.e. shrinks the range of the data in a fixed range of [0, 1].
    """

    def __init__(self, min_value, max_value, **kwargs):
        assert max_value > min_value
        self.min_value = min_value
        self.value_range = max_value - min_value

    def __call__(self, m):
        norm_0_1 = (m - self.min_value) / self.value_range
        return np.clip(norm_0_1, 0, 1)


class ToTensor:
    """
    Converts a given input numpy.ndarray into torch.Tensor. Adds additional 'channel' axis when the input is 3D
    and expand_dims=True (use for raw data of the shape (D, H, W)).
    """

    def __init__(self, expand_dims, dtype=np.float32, **kwargs):
        self.expand_dims = expand_dims
        self.dtype = dtype

    def __call__(self, m):
        assert m.ndim in [3, 4], 'Supports only 3D (DxHxW) or 4D (CxDxHxW) images'
        # add channel dimension
        if self.expand_dims and m.ndim == 3:
            m = np.expand_dims(m, axis=0)

        return torch.from_numpy(m.astype(dtype=self.dtype))


class Resize:
    """
    Resize a given input numpy.ndarray into the shape (D, H, W) zoom_size
    """
    def __init__(self, zoom_size, order=0, mode='reflect'):
        self.zoom_size = zoom_size
        self.mode = mode
        self.order = order

    def __call__(self, m):
        assert  m.ndim in [3, 4], 'Supports only 3D (DxHxW) or 4D (CxDxHxW) images'

        if m.ndim == 3:
            return zoom(m, self.zoom_size, mode=self.mode, order=self.order)

        else:
            channels = []
            channels.append(zoom(m[c], self.zoom_size, mode=self.mode, order=self.order)
                            for c in range(m.shape[0]))
            return np.stack(channels, axis=0)