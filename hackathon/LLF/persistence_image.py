import sys
import matplotlib.pyplot as plt
from matplotlib.colors import BoundaryNorm
from matplotlib.ticker import MaxNLocator
import numpy as np
import math

file_name = sys.argv[1]

birth_list = []
death_list = []

# read bcd file
# with open('C:/Users/Administrator/Desktop/PD_test/12070404c1.CNG.swc.bcd','r') as f:
with open(file_name, 'r') as f:
    lines = f.readlines()
for line in lines:
    line = line.rstrip('\n')
    if line.startswith('#'):
        continue
    birth, death = line.split()
    birth = int(float(birth))
    birth_list.append(birth)
    death = int(float(death))
    death_list.append(death)

bound = max(max(birth_list),max(death_list))+10
data = [[0.]*bound for i in range(bound)]

PI = 3.14159265358979323846
sigma = 5
s = 2 * sigma*sigma
# (u, v) : (death, birth) gaussian's center point
for u, v in zip(death_list, birth_list):
    for i in range(len(data)):
        for j in range(len(data[0])):
            x = i - u
            y = j - v
            data[j][i] += math.exp(-(x*x + y*y) / s) / (PI*s)
#print(data)

# matpolt begins
l = []
for d in data:
    l.append(max(d))
upper_bound = max(l)
y, x = np.mgrid[0:bound, 0:bound]

levels = MaxNLocator(nbins=bound).tick_values(0, upper_bound)

# pick the desired colormap, sensible levels, and define a normalization
# instance which takes data values and translates those into levels.
cmap = plt.get_cmap('jet')  # jet is GREAT!!!
norm = BoundaryNorm(levels, ncolors=cmap.N, clip=True)

fig, ax0 = plt.subplots(nrows=1)

# resolution
dx, dy = 0.05, 0.05
# contours are *point* based plots, so convert our bound into point
# centers
cf = ax0.contourf(x + dx/2.,
                  y + dy/2., data, levels=levels,
                  cmap=cmap)
fig.colorbar(cf, ax=ax0)
plt.xlabel('birth')
plt.ylabel('death')
ax0.set_title('persistence image')

# adjust spacing between subplots so `ax1` title and `ax0` tick labels
# don't overlap
fig.tight_layout()
plt.show()
