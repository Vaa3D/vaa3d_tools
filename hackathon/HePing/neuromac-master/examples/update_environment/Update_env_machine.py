import numpy as np

from growth_procs import prepare_next_front

L_NORM = 5

def extend_front(front,seed,constellation) :
    new_pos =front.xyz + np.array([-1.0*L_NORM,0,0])
    new_front = prepare_next_front(front,new_pos,set_radius=3.0)

    # secrete some "substance_x" at some point
    if front.path_length > 40 and front.path_length < 46 :
        # special syntax for updating the environent
        return [new_front],{"substance_x":front}
    else :
        return [new_front]
