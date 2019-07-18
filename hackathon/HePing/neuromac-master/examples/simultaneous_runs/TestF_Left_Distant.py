import numpy as np

from growth_procs import direction_to,\
     normalize_length,\
     get_entity,\
     prepare_next_front

L_NORM=3.0

def extend_front(front,seed,constellation) :
    # attract by a different neuron, get information
    other_entities = get_entity("cell_type_2",constellation)
    if not len(other_entities) == 0:
        dir_to_entity = direction_to(front,other_entities,what="nearest")
        dir_to_entity = normalize_length(dir_to_entity,1.0)
    else: # during the first extension cycle, no distal info is known locally
        dir_to_entity = np.array([.5,.0,.0])
    new_pos = front.xyz + normalize_length(dir_to_entity,L_NORM)
    new_front = prepare_next_front(front,new_pos,set_radius=1.5)
    return [new_front]
