import copy,time
import random
import numpy as np

from growth_procs import unit_sample_on_sphere,\
     direction_to,\
     gradient_to,\
     normalize_length,\
     get_entity,\
     prepare_next_front

def extend_front(front,seed,constellation) :
    if front.order == 0 : # soma
        new_fronts = []
        for i in range(3):
            rnd_dir = unit_sample_on_sphere()
            rnd_dir[2] = np.abs(rnd_dir[2])
            new_pos = normalize_length(rnd_dir,15)
            new_pos = front.xyz + new_pos
            new_front = prepare_next_front(front,new_pos,set_radius=3.0,add_order=1)
            new_fronts.append(new_front)
        return new_fronts
    else :
        if np.random.random() < 0.1 :
            new_fronts = []
            other_entities = get_entity("pia",constellation)
            for i in range(2):
                rnd_dir = unit_sample_on_sphere()

                if len(other_entities) == 0:
                    rnd_dir = unit_sample_on_sphere()
                    rnd_dir[2] = np.abs(rnd_dir[2])
                    new_pos = normalize_length(rnd_dir,15)
                    new_pos = front.xyz + new_pos
                    new_front = prepare_next_front(front,new_pos,set_radius=3.0,add_order=1)
                    new_fronts.append(new_front)
                else:
                    dir_to_pia = direction_to(front,other_entities,what="nearest")
                    dir_to_pia = normalize_length(dir_to_pia,2.0) + rnd_dir 

                    new_pos = front.xyz + normalize_length(dir_to_pia,10)

                    new_front = prepare_next_front(front,new_pos,radius_factor=0.5,add_order=True)
                    new_fronts.append(new_front)
            return new_fronts
        else :
            rnd_dir = unit_sample_on_sphere()

            other_entities = get_entity("pia",constellation)

            gradient_to_pia = gradient_to(front,other_entities,1.0,0.0001,what='nearest')
            # or go directly to the pia
            # gradient_to_pia = direction_to(front,other_entities,what='nearest')
            
            pia_interaction = gradient_to_pia + rnd_dir

            new_dir = normalize_length(pia_interaction,1.5) #+ rnd_dir 
            new_pos = front.xyz + normalize_length(new_dir,10)
            new_front = prepare_next_front(front,new_pos,radius_factor=0.95)
            if new_pos[2] > 190 :
                return None
            else:
                return [new_front]


