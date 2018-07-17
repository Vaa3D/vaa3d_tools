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
from collections import OrderedDict

DATA_DIR = "../data/08_cube_npy"
N_trajectories = 1
TRAJECTORY_LEN = 1000
LINE_WIDTH = 2

SAVE_VIDEO = True

X_MAX = 5
X_MIN = -X_MAX
Y_MAX = X_MAX
Y_MIN = -Y_MAX
Z_MAX = X_MAX
Z_MIN = -Z_MAX

INTERSECTION_LEEWAY = (X_MAX - X_MIN) / 50
RAND_WALK_STEPSIZE = INTERSECTION_LEEWAY


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
    print(starting_coords.shape, trajectory.shape)
    trajectory[0, :] = starting_coords

    # step in fixed directions
    action_space = np.array([[0, 0, 1],
                             [0, 1, 0],
                             [1, 0, 0]], dtype=float)
    action_space = np.concatenate([-1 * action_space, action_space])
    action_space = action_space * stepsize  # scale steps

    for index in range(1, length):
        in_bounds = False
        while not in_bounds:
            # scaling the random numbers by 0.1 so
            # movement is small compared to position.
            # to allow a line to move backwards.
            # step = ((np.random.uniform(-1, 1, dims)) * stepsize)

            # since action space is discrete, randomly select one a row
            step = action_space[np.random.randint(0, action_space.shape[0])]
            # replace only the  next col in the sequence
            # take last point and add step to it
            proposed_spot = trajectory[index - 1, :] + step

            if is_in_bounds(proposed_spot):
                trajectory[index, :] = proposed_spot
                in_bounds = True

    return trajectory


def is_in_bounds(coords):
    x, y, z = coords
    return ((X_MIN < x < X_MAX and
             Y_MIN < y < Y_MAX and
             Z_MIN < z < Z_MAX))


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
scoreboard = ax.text(0.9 * X_MIN, 0, 0.9 * Z_MAX, s=" ", bbox={'facecolor': 'w', 'alpha': 0.5, 'pad': 5},
                     )
fnames, training_fabspaths = get_fnames_and_abspath_from_dir(DATA_DIR)
random_file = np.random.choice(range(len(training_fabspaths)))
print("using ", fnames[random_file])
groundtruth_fabspath = training_fabspaths[random_file]
groundtruth = swc_to_dframe(groundtruth_fabspath)

# plot ground truth and leave it up
x_targ = groundtruth.x
y_targ = groundtruth.y
z_targ = groundtruth.z

ax.plot(x_targ, y_targ, z_targ, label='human annotation', c="black", linewidth=LINE_WIDTH)

targ_upsampled = upsample_coords([x_targ, y_targ, z_targ])
targ_coords = np.column_stack(targ_upsampled)

# KD-tree for nearest neighbor search
targ_kdtree = cKDTree(targ_coords)

# finding root node
node_ids = set(groundtruth.node_id)
parent_ids = set(groundtruth.parent_node_id)
root_id = parent_ids - node_ids
root_id = np.int32(next(iter(root_id)))  # convert set to int
print("roots ", root_id)

# make all trajectories start at root node
starting_position = groundtruth[groundtruth.parent_node_id == root_id][["x", "y", "z"]]
print("gt ", groundtruth)

# Solve for the trajectories

trajectories = np.asarray([rand_walk(starting_position, TRAJECTORY_LEN)
                           for _ in range(N_trajectories)])

trajectories_upsampled = [upsample_coords(t) for t in trajectories]
trajectories_coords = np.column_stack(trajectories_upsampled)

# choose a different color for each trajectory
colors = plt.cm.hsv(np.linspace(0, 1, N_trajectories))

# set up lines and points
# each item in these lists is a separate list of points, one for each frame in the final animation
# this is redundant, but we'll only animate pre-computed data this way
agent_trajectories = sum([ax.plot([], [], [], '-', c=c, linewidth=LINE_WIDTH, alpha=0.2, label="agents")
                          for c in colors], [])
agent_tip = sum([ax.plot([], [], [], '*', c=c, markersize=10, alpha=0.5)  # leading edge of agents
                 for c in colors], [])
intersections = sum([ax.plot([], [], [], 'X', c="red", markersize=4, fillstyle='none', markeredgewidth=1,
                             label="intersection")
                     for i in range(N_trajectories)], [])

handles, labels = plt.gca().get_legend_handles_labels()
by_label = OrderedDict(zip(labels, handles))
ax.legend(by_label.values(), by_label.keys())


# initialization function: plot the background of each frame
def init():
    for line, pt, inter in zip(agent_trajectories, agent_tip, intersections):
        line.set_data([], [])
        line.set_3d_properties([])

        pt.set_data([], [])
        pt.set_3d_properties([])

        inter.set_data([], [])
        inter.set_3d_properties([])
    return agent_trajectories + agent_tip


# animation function.  This will be called sequentially with the frame number
def animate(i):
    # we'll step two time-steps per frame.  This leads to nice results.
    og_i = i
    i = (2 * i) % trajectories.shape[1]

    scores_per_trajectory = []

    for idx, (line, pt, inter, traj) in enumerate(zip(agent_trajectories, agent_tip, intersections, trajectories)):
        if og_i == 0:
            # print(np.shape(xi.T))
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
        # print(np.shape(np.concatenate(list(zip(xs.ravel(),ys.ravel(),zs.ravel())))
        distances, close_indexes = targ_kdtree.query(list(zip(xs.ravel(), ys.ravel(), zs.ravel())),
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

        scoreboard.set_text("# Intersections: {}".format(sum(scores_per_trajectory)))

    # rotate the scene
    ax.view_init(30, 0.3 * i)
    fig.canvas.draw()
    return agent_trajectories + agent_tip


# instantiate the animator.
anim = animation.FuncAnimation(fig, animate, init_func=init,
                               frames=TRAJECTORY_LEN,
                               interval=30,
                               blit=True)

# Save as mp4. This requires mplayer or ffmpeg to be installed
if SAVE_VIDEO is True:
    import string

    tokens = list(string.ascii_lowercase + string.digits)
    anim.save('trajectory {}.mp4'.format("".join(np.random.choice(tokens, size=6))),
              fps=15,
              extra_args=['-vcodec', 'libx264'])

# plt.show()
