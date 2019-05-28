import copy,time
import random
import numpy as np

from growth_procs import unit_sample_on_sphere,\
     direction_to,\
     gradient_to,\
     normalize_length,\
     get_entity,\
     prepare_next_front

L_NORM=0.25

def extend_front(front,seed,constellation) :
    # new_pos =front.xyz + np.array([L_NORM,0,2.0])
    # new_front = prepare_next_front(front,new_pos,set_radius=1.0)
    # return [new_front]

    # repelled by a different neuron, get information
    other_entities = get_entity("cell_type_1",constellation)
    target = np.array([200,30,200])
    
    if not len(other_entities) == 0:
        dir_to_entity = gradient_to(front,other_entities,strength=10.0,decay_factor=0.15,cutoff=2.5,what="nearest")
        if np.allclose(dir_to_entity,np.array([0.0,0.0,0.0])):
            new_pos = front.xyz + \
              normalize_length(target-front.xyz,L_NORM ) + \
              normalize_length(unit_sample_on_sphere(),L_NORM/1.0 ) # was 10
            new_front = prepare_next_front(front,new_pos,set_radius=1.0)
            print "[debug] To_left.py:: still far"
        else:
            new_pos = front.xyz + normalize_length(-1*dir_to_entity + unit_sample_on_sphere(),L_NORM)
            new_front = prepare_next_front(front,new_pos,set_radius=1.0)
            print "[debug] To_left.py:: potential intercept avoided"
    else: # during the first extension cycle, no distal info is known locally
        new_pos =front.xyz + normalize_length(target-front.xyz,L_NORM )
        new_front = prepare_next_front(front,new_pos,set_radius=1.0)        
    return [new_front]

