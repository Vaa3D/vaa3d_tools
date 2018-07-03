import numpy as np
from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.colors import cnames
from matplotlib import animation
from scipy.spatial import cKDTree
from scipy import interpolate

N_trajectories = 5
TRAJECTORY_LEN = 1000


def upsample_coords(coord_list):
    # s is smoothness, set to zero
    # k is degree of the spline. setting to 1 for linear spline
    tck, u = interpolate.splprep(coord_list, k=1, s=0.0)
    upsampled_coords = interpolate.splev(np.linspace(0, 1, 100), tck)
    return upsampled_coords


def rand_walk(starting_coords, length, stepsize=5):
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
    action_space = action_space * stepsize  ## scale steps

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
np.random.seed(1)
x0 = np.random.random((N_trajectories, 3))


# Solve for the trajectories
# for starting point in x0
# generate timeseries

x_t = np.asarray([rand_walk(x0i, TRAJECTORY_LEN)
                  for x0i in x0])
print(np.shape(x_t))

# Set up figure & 3D axis for animation
fig = plt.figure()
ax = fig.add_axes([0, 0, 1, 1], projection='3d')
ax.axis('off')

theta = np.linspace(-4 * np.pi, 4 * np.pi, 100)
z_targ = np.linspace(-10, 10, 100)
r = z_targ ** 2 + 1
x_targ = r * np.sin(theta)
y_targ = r * np.cos(theta)
ax.plot(x_targ, y_targ, z_targ, label='ground truth', c="black", linewidth=7.0)

targ_upsampled = upsample_coords([x_targ, y_targ, z_targ])
targ_coords = np.column_stack(targ_upsampled)



# choose a different color for each trajectory
colors = plt.cm.hsv(np.linspace(0, 1, N_trajectories))

# set up lines and points
# each item in these lists is a separate list of points, one for each frame in the final animation
# this is redundant, but we'll only animate pre-computed data this way
lines = sum([ax.plot([], [], [], '-', c=c, linewidth=7.0, alpha = 0.3)
             for c in colors], [])
pts = sum([ax.plot([], [], [], 'X', c=c, markersize=20)
           for c in colors], [])
intersections = sum([ax.plot([], [], [], 'X', c="red", markersize=20)
           for i in range(N_trajectories)], [])

# prepare the axes limits
ax.set_xlim((-25, 25))
ax.set_ylim((-35, 35))
ax.set_zlim((5, 55))

# set point-of-view: specified by (altitude degrees, azimuth degrees)
ax.view_init(30, 0)

# initialization function: plot the background of each frame
def init():
    for line, pt in zip(lines, pts):
        line.set_data([], [])
        line.set_3d_properties([])

        pt.set_data([], [])
        pt.set_3d_properties([])
    return lines + pts

# animation function.  This will be called sequentially with the frame number
def animate(i):
    # we'll step two time-steps per frame.  This leads to nice results.
    og_i = i
    #i = (2 * i) % x_t.shape[1]

    for line, pt, xi in zip(lines, pts, x_t):
        if og_i == 0:
            #print(np.shape(xi.T))
            pass
        x, y, z = xi[:i].T
        line.set_data(x, y)
        line.set_3d_properties(z)

        # [-1:] gets last row
        pt.set_data(x[-1:], y[-1:])
        pt.set_3d_properties(z[-1:])

    ax.view_init(30, 0.3 * i)
    fig.canvas.draw()
    return lines + pts

# instantiate the animator.
anim = animation.FuncAnimation(fig, animate, init_func=init,
                               frames=TRAJECTORY_LEN, interval=30, blit=True)

# Save as mp4. This requires mplayer or ffmpeg to be installed
#anim.save('lorentz_attractor.mp4', fps=15, extra_args=['-vcodec', 'libx264'])

plt.show()