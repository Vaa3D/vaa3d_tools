from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
from matplotlib import animation, cm

TRAJECTORY_LEN = 100

def cuboid_data(position, size=(1, 1, 1)):
    cube_corners = [[[0, 1, 0], [0, 0, 0], [1, 0, 0], [1, 1, 0]],
         [[0, 0, 0], [0, 0, 1], [1, 0, 1], [1, 0, 0]],
         [[1, 0, 1], [1, 0, 0], [1, 1, 0], [1, 1, 1]],
         [[0, 0, 1], [0, 0, 0], [0, 1, 0], [0, 1, 1]],
         [[0, 1, 0], [0, 1, 1], [1, 1, 1], [1, 1, 0]],
         [[0, 1, 1], [0, 0, 1], [1, 0, 1], [1, 1, 1]]]
    cube_corners = np.array(cube_corners).astype(float)
    # scale each dimension of cube_corners by size
    for i in range(3):
        cube_corners[:, :, i] *= size[i]
    # translate cuboid to final position
    cube_corners += np.array(position)
    return cube_corners


def plotCubeAt(positions, sizes=None, colors=None, **kwargs):
    if not isinstance(colors, (list, np.ndarray)): colors = ["C0"] * len(positions)
    if not isinstance(sizes, (list, np.ndarray)): sizes = [(1, 1, 1)] * len(positions)
    g = []
    for p, s, c in zip(positions, sizes, colors):
        g.append(cuboid_data(p, size=s))
    return Poly3DCollection(np.concatenate(g),
                            facecolors=np.repeat(colors, 6, axis=0), **kwargs)


x_span = 10
y_span = 10
z_span = 10
binary_grid = np.random.choice([0, 1], size=(x_span, y_span, z_span), p=[0.98, 0.02])
x, y, z = np.indices((x_span, y_span, z_span)) - .5
# filter for the ones
positions = np.c_[x[binary_grid == 1], y[binary_grid == 1], z[binary_grid == 1]]
r = lambda: np.random.randint(0,255)
random_colors = np.array(['#%02X%02X%02X%02X' % (r(), r(), r(), r()) for _ in enumerate(positions)])

fig = plt.figure()
ax = fig.gca(projection='3d')
ax.set_aspect('equal')
ax.axis('off')

pc = plotCubeAt(positions, colors=random_colors, edgecolor="k")
ax.add_collection3d(pc)

ax.set_xlim([0, x_span])
ax.set_ylim([0, y_span])
ax.set_zlim([0, z_span])

def generate_positions():
    while True:
        binary_grid = np.random.choice([0, 1], size=(x_span, y_span, z_span), p=[0.995, 0.005])
        positions = np.c_[x[binary_grid == 1], y[binary_grid == 1], z[binary_grid == 1]]
        r = lambda: np.random.randint(0, 255)
        random_colors = np.array(['#%02X%02X%02X%02X' % (r(), r(), r(), r()) for _ in enumerate(positions)])
        yield positions, random_colors


# animation function.  This will be called sequentially with the frame number
def animate(i, data_gen):
    # filter for the ones
    positions, colors = next(data_gen)
    if len(positions) > 0:
        pc = plotCubeAt(positions, colors=colors, edgecolor="k")
        ax.add_collection3d(pc)
    ax.view_init(30, 0.3 * i)
    fig.canvas.draw()


anim = animation.FuncAnimation(fig, animate,
                               fargs=[generate_positions()],
                               frames=TRAJECTORY_LEN,
                               interval=30)

plt.show()
