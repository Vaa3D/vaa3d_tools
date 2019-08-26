import numpy as np

from growth_procs import prepare_next_front

L_NORM=4.0

def extend_front(front,seed,constellation) :
    # this structure grows straight without ccontextual interaction
    new_pos =front.xyz + np.array([-1.0*L_NORM,0,0])
    new_front = prepare_next_front(front,new_pos,set_radius=1.5)
    return [new_front]
