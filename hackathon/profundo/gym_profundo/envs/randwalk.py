import faulthandler
faulthandler.enable()

import numpy as np
import matplotlib
matplotlib.use('TkAgg')
from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.colors import cnames
from matplotlib import animation
from scipy.spatial import cKDTree
from scipy import interpolate
from data_processing.swc_io import get_fnames_and_abspath_from_dir, swc_to_dframe


DATA_DIR = "../../data/06_centered_cubes"
N_trajectories = 10
TRAJECTORY_LEN = 1000
LINE_WIDTH = 1
INTERSECTION_LEEWAY = 0.5
SAVE_VIDEO = False
RAND_WALK_STEPSIZE = INTERSECTION_LEEWAY

X_MAX = 12
X_MIN = -X_MAX
Y_MAX = 12
Y_MIN = -Y_MAX
Z_MAX = 12
Z_MIN = -Z_MAX


def upsample_coords(coord_list):
    ## rand walk coord list is generated transposed
    # make an exception block to transpose if necessary
    try:
        assert (np.shape(coord_list)[0] == 3)
    except AssertionError:
        if np.shape(coord_list)[1] == 3:
            # transpose
            coord_list = np.array(coord_list).T

    # s is smoothness, set to zero
    # k is degree of the spline. setting to 1 for linear spline
    tck, u = interpolate.splprep(coord_list, k=1, s=0.0)
    upsampled_coords = interpolate.splev(np.linspace(0, 1, 100), tck)
    return upsampled_coords


def rand_walk(starting_coords, length, stepsize=RAND_WALK_STEPSIZE):
    """Compute the time-derivative of a Lorentz system."""
    dims = 3
    trajectory = np.empty((length, dims))
    # replace first col with starting coords
    trajectory[0, :] = list(starting_coords)

    # step in fixed directions
    action_space = np.array([[0, 0, 1],
                             [0, 1, 0],
                             [1, 0, 0]], dtype=float)
    action_space = np.concatenate([-1*action_space, action_space])
    action_space = action_space * stepsize  # scale steps

    for index in range(1, length):
        # scaling the random numbers by 0.1 so
        # movement is small compared to position.
        # to allow a line to move backwards.
        #step = ((np.random.uniform(-1, 1, dims)) * stepsize)

        # since action space is discrete, randomly select one a row
        step = action_space[np.random.randint(0, action_space.shape[0])]
        # replace only the  next col in the sequence
        # take last point and add step to it
        trajectory[index, :] = trajectory[index - 1, :] + step

    return trajectory

# Choose random starting points, uniformly distributed from -15 to 15
starting_positions = np.random.random((N_trajectories, 3))


# Solve for the trajectories
# for starting point in x0
# generate timeseries

trajectories = np.asarray([rand_walk(start_pos, TRAJECTORY_LEN)
                           for start_pos in starting_positions])

trajectories_upsampled = [upsample_coords(t) for t in trajectories]
trajectories_coords = np.column_stack(trajectories_upsampled)


# Set up figure & 3D axis for animation
fig = plt.figure()
ax = fig.add_axes([0, 0, 1, 1], projection='3d')
ax.axis('off')

# prepare the axes limits
ax.set_xlim((X_MIN, X_MAX))
ax.set_ylim((Y_MIN, Y_MAX))
ax.set_zlim((Z_MIN, Z_MAX))

# set point-of-view: specified by (altitude degrees, azimuth degrees)
ax.view_init(30, 0)

# add scoreboard
scoreboard = ax.text(0.9*X_MIN, 0, 0.9*Z_MAX, s=" ", bbox={'facecolor':'w', 'alpha':0.5, 'pad': 5},
                )
_, training_fabspaths = get_fnames_and_abspath_from_dir(DATA_DIR)
random_i = np.random.choice(range(len(training_fabspaths)))
groundtruth_fabspath = training_fabspaths[random_i]
groundtruth = swc_to_dframe(groundtruth_fabspath)
# plot ground truth and leave it up

# theta = np.linspace(-4 * np.pi, 4 * np.pi, 100)
# z_targ = np.linspace(-10, 10, 100)
# r = z_targ ** 2 + 1
# x_targ = r * np.sin(theta)
# y_targ = r * np.cos(theta)
x_targ = groundtruth.x
y_targ = groundtruth.y
z_targ = groundtruth.z
#print(groundtruth)
ax.plot(x_targ, y_targ, z_targ, label='ground truth', c="black", linewidth=LINE_WIDTH)

targ_upsampled = upsample_coords([x_targ, y_targ, z_targ])
targ_coords = np.column_stack(targ_upsampled)

# KD-tree for nearest neighbor search
targ_kdtree = cKDTree(targ_coords)




# choose a different color for each trajectory
colors = plt.cm.hsv(np.linspace(0, 1, N_trajectories))

# set up lines and points
# each item in these lists is a separate list of points, one for each frame in the final animation
# this is redundant, but we'll only animate pre-computed data this way
lines = sum([ax.plot([], [], [], '-', c=c, linewidth=LINE_WIDTH, alpha = 0.2)
             for c in colors], [])
pts = sum([ax.plot([], [], [], '*', c=c, markersize=15, alpha=0.2)
           for c in colors], [])
intersections = sum([ax.plot([], [], [], 'X', c="red", markersize=10, fillstyle='none', markeredgewidth=3)
           for i in range(N_trajectories)], [])




# initialization function: plot the background of each frame
def init():
    for line, pt, inter in zip(lines, pts, intersections):
        line.set_data([], [])
        line.set_3d_properties([])

        pt.set_data([], [])
        pt.set_3d_properties([])

        inter.set_data([], [])
        inter.set_3d_properties([])
    return lines + pts

# animation function.  This will be called sequentially with the frame number
def animate(i):
    # we'll step two time-steps per frame.  This leads to nice results.
    og_i = i
    #i = (2 * i) % x_t.shape[1]

    scores_per_trajectory = []

    for idx, (line, pt, inter, traj) in enumerate(zip(lines, pts, intersections, trajectories)):
        if og_i == 0:
            #print(np.shape(xi.T))
            pass
        xs, ys, zs = traj[:i].T
        line.set_data(xs, ys)
        line.set_3d_properties(zs)

        # [-1:] gets last row
        pt.set_data(xs[-1:], ys[-1:])
        pt.set_3d_properties(zs[-1:])

        # calculate if intersected
        if i == 0:  # skip first, there is no previous point
            continue

        # FIXME this is very redundant and expensive, don't recompute whole
        # trajectory in query every time
        #print(np.shape(np.concatenate(list(zip(xs.ravel(),ys.ravel(),zs.ravel())))
        distances, close_indexes = targ_kdtree.query(list(zip(xs.ravel(),ys.ravel(),zs.ravel())),
                                                  distance_upper_bound=INTERSECTION_LEEWAY)

        # strangely, points infinitely far away are somehow within the upper bound
        inf_mask = np.ma.masked_invalid(distances)
        masked_indexes = np.ma.masked_where(np.ma.getmask(inf_mask), close_indexes)
        masked_indexes = np.ma.compressed(np.unique(masked_indexes))  # deduplicate

        # plot ground truth that was activated
        intersection_coords = targ_kdtree.data[masked_indexes]
        _x, _y, _z = intersection_coords.T
        inter.set_data(_x, _y)
        inter.set_3d_properties(_z)

        # calculating rewards
        # if very last point in trajectory crosses the target trajectory,
        # give reward; else give penalty
        distances, close_indexes = targ_kdtree.query(traj[i].T, distance_upper_bound=INTERSECTION_LEEWAY)
        inf_mask = np.ma.masked_invalid(distances)
        masked_indexes = np.ma.masked_where(np.ma.getmask(inf_mask),
                                            close_indexes)
        masked_indexes = np.ma.compressed(
            np.unique(masked_indexes))  # deduplicate
        intersection_coords = targ_kdtree.data[masked_indexes]

        try:
            scores_per_trajectory[idx] = len(_z)
        except IndexError:  # first pass through data
            scores_per_trajectory.append(len(_z))

        scoreboard.set_text("Current Score: {}".format(sum(scores_per_trajectory)))

    # rotate the scene
    ax.view_init(30, 0.3 * i)
    fig.canvas.draw()
    return lines + pts

# instantiate the animator.
anim = animation.FuncAnimation(fig, animate, init_func=init,
                               frames=TRAJECTORY_LEN, interval=30, blit=True)

# Save as mp4. This requires mplayer or ffmpeg to be installed
if SAVE_VIDEO is True:
    import string
    tokens = list(string.ascii_lowercase + string.digits)
    anim.save('trajectory {}.mp4'.format("".join(np.random.choice(tokens, size=6))), fps=15, extra_args=['-vcodec', 'libx264'])

plt.show()