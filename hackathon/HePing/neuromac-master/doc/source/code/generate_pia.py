from pylab import *
import pickle
import numpy as np
pia =[]
radius = 5
for i in range(10):
    for j in range(10):
        x = i*10
        y = j *10
        z = 0
        tup = (np.array([x,y,z]),radius)
        pia.append(tup)
pickle.dump(pia,open("pia_point.pkl","w"))
