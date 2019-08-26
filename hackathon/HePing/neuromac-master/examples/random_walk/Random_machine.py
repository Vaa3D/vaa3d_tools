from growth_procs import unit_sample_on_sphere,\
     normalize_length,\
     prepare_next_front

L_NORM = 3.0

def extend_front(front,seed,constellation) :
    if front.order == 0:
        new_fronts = []
        for i in range(5):
            rnd_dir = unit_sample_on_sphere()
            new_pos = normalize_length(rnd_dir,L_NORM*5)
            new_pos = front.xyz + new_pos
            new_front = prepare_next_front(front,new_pos,set_radius=1.0,add_order=True)
            new_fronts.append(new_front)
        return new_fronts
            
    else:
        rnd_dir = unit_sample_on_sphere()
        new_pos = normalize_length(rnd_dir,L_NORM)
        new_pos = front.xyz + new_pos
        new_front = prepare_next_front(front,new_pos,set_radius=1.0)
        return [new_front]
