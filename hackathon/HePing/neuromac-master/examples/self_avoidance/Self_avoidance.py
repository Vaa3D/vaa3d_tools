import copy,time
import random
import numpy as np

from growth_procs import unit_sample_on_sphere,\
     direction_to,\
     gradient_to,\
     normalize_length,\
     get_entity,\
     get_eigen_entity,\
     prepare_next_front

L_NORM=5.0

def extend_front(front,seed,constellation) :
    """ Grow away from self-contained stretches of neurites.
    """
    eigen_entities = get_eigen_entity(front,constellation)
    if len(eigen_entities) == 0:
        if front.order == 0:
            rnd_dir = unit_sample_on_sphere()
            new_pos = front.xyz + np.array([26,0,0])
            new_front = prepare_next_front(front,new_pos,set_radius=1.0,add_order=True)
            return [new_front]
        else:
            new_pos1 = front.xyz + np.array([5,5,0])
            new_front1 = prepare_next_front(front,new_pos1,set_radius=1.0,add_order=True)
            new_pos2 = front.xyz + np.array([5,-5,0])
            new_front2 = prepare_next_front(front,new_pos2,set_radius=1.0,add_order=True)
            return [new_front1,new_front2]
    else:
        if np.random.random() > 0.2 :
            rnd_dir = unit_sample_on_sphere()
            dir_away = -1.0 * direction_to(front,eigen_entities,what="nearest")
            dir_away = normalize_length(dir_away,4) + rnd_dir 
            new_pos = front.xyz + normalize_length(dir_away,L_NORM)
            new_front = prepare_next_front(front,new_pos,set_radius=1.0)
            return [new_front]
        else:
            daughters = []
            for i in range(2):
                rnd_dir = unit_sample_on_sphere()
                dir_away = -1.0 * direction_to(front,eigen_entities,what="nearest")
                dir_away = normalize_length(dir_away,4.0) + rnd_dir 
                new_pos = front.xyz + normalize_length(dir_away,L_NORM)
                new_front = prepare_next_front(front,new_pos,set_radius=1.0)
                daughters.append(new_front)
            return daughters
                
        
    
