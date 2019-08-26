import copy,time
import random
import numpy as np

from growth_procs import unit_sample_on_sphere,\
     direction_to,\
     gradient_to,\
     normalize_length,\
     get_entity,\
     prepare_next_front

L_NORM=3.0

def extend_front(front,seed,constellation) :
    """ follow gradient to a_point Attractor.
    The stronger the attraction becomes, the straighter the path
    to the attractor
    """
    other_entities = get_entity("a_point",constellation)
    rnd_dir = unit_sample_on_sphere()
    dir_to_entity = gradient_to(front,other_entities,1.7,0.01,what="nearest")
    dir_to_entity = dir_to_entity + 0.5*rnd_dir 
    new_pos = front.xyz + normalize_length(dir_to_entity,L_NORM)
    
    new_front = prepare_next_front(front,new_pos,set_radius=1.0)

    return [new_front]
