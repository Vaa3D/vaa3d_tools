import copy,time
import random
import numpy as np

from growth_procs import unit_sample_on_sphere,\
     direction_to,\
     gradient_to,\
     normalize_length,\
     get_entity,\
     prepare_next_front

L_NORM=4.0

def extend_front(front,seed,constellation) :
    new_pos =front.xyz + np.array([-1*L_NORM,0,0])
    new_front = prepare_next_front(front,new_pos,set_radius=1.0)
    return [new_front]
