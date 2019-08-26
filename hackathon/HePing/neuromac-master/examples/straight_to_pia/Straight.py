import numpy as np

from growth_procs import direction_to,\
     normalize_length,\
     get_entity,\
     prepare_next_front

L_NORM=3

def extend_front(front,seed,constellation) :
    """
    Growth is directed by an attraction to the "pia"; a point-cloud \
    located at the top of the volume. (See straight.cfg)
    """
    other_entities = get_entity("pia",constellation)
    dir_to_entity = direction_to(front,other_entities,what="nearest")
    dir_to_entity = dir_to_entity
    new_pos = front.xyz + normalize_length(dir_to_entity,L_NORM)
    new_front = prepare_next_front(front,new_pos,set_radius=1.0)

    return [new_front]
