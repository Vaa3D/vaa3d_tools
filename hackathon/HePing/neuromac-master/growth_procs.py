import time,copy
import numpy as np
from front import Front

compute_exp_decay_factor = lambda i,d,x : i*np.exp(-d*x)

def unit_sample_on_sphere() :
    """
    Sample a random point on a sphere with :math:`r=1`
    
    Returns
    -------
    v : np.array
       Unit-length vector originating from [0,0,0]
    """
    xs = np.random.normal(size=3)
    denominator = np.sqrt(np.sum(xs**2))
    origin = np.array([0,0,0])
    to = [x / denominator for x in xs]
    return np.array(to)

def get_entity(entity_name,constellation) :
    """
    Search for all entities with a specific name.

    .. warning:: Does not yield desired results when searching for "own" \
       entities, that is, all other components of the same entity.    

    Parameters
    ----------
    entity_name : string
       Name of the entity. Searching is done by :code:`string.startswith(string)`, \
       which acts as some sort of wild card
    constellation : dict of list of np.array
       A "point-only" constellation. That is, entries in this dict are \
       lists of 3D np.array vectors

    Returns
    -------
    entities : list
       List contains 3D positions
    """
    entities = []
    for key in constellation.keys() :
        if key.startswith(entity_name):
            entities = entities + constellation[key]
    return entities

def get_other_entity(front,constellation) :
    """
    Get a list of all point associated to all other entities except for
    entities of the same structure

    .. warning:: Does not yield desired results when searching for "own" \
       entities, that is, all other components of the same entity.    

    Parameters
    ----------
    front : :class:`front.Front`
       To specify "self" in contrast to "others"
    constellation : dict of list of np.array
       A "point-only" constellation. That is, entries in this dict are \
       lists of 3D np.array vectors

    Returns
    -------
    entities : list
       List contains 3D positions of all structure except for "self".
    """
    entity_name = front.entity_name
    entities = []
    for key in constellation.keys() :
        if not key.startswith(entity_name):
            entities = entities + constellation[key]
    return entities

def get_eigen_entity(front,constellation,ancestry_limit=25,common_ancestry_limit=10):
    """
    Search for all entity components of a structure. For instance, if \
    you want to implement self-repulsion you have to use this method.

    It is implemented in such way that direct siblings and parent \
    structures are ignored. If this would not be the case, sel-repulsion \
    would always direct away from the parent structure and result in a \
    straight line.

    Parameters
    ----------
    front : :py:class:`front.Front`
    constellation : dict of list of np.array
       A "point-only" constellation. That is, entries in this dict are \
       lists of 3D np.array vectors

    Returns
    -------
    entities : list of np.array
       List contains 3D positions
    """
    entity_name = front.entity_name
    print("front.entity_name: ", entity_name)
    entities = []
    for key in constellation.keys() :
        if key.startswith(entity_name):
            entities = entities + constellation[key]
    entities = map(tuple,entities)
    #print "entities: ", entities

    """Now prune the list of entities. Remove:
    1. ancestors less than <ancestry_limit> micron away
    2. fronts that have common ancestors less then <common_ancestry_limit> micron away (this happens at bifurcations)
    
    construct a list of positions, that will be removed from the entities list
    """
    len_before = len(entities)
    # to_be_removed = []
    # to_be_removed.append(front.xyz) # no cue from yourself
    #print "self as tuple:", tuple(front.xyz)

    try:
        entities.remove(tuple(front.xyz))
        path_L = 0
        c_front = front
        while path_L < ancestry_limit:
            parent = c_front.parent
            if parent == None:
                break
            path_L = path_L + np.sqrt(np.sum((c_front.xyz-parent.xyz)**2))
            #to_be_removed.append(parent.xyz)
            c_front= parent
            entities.remove(tuple(parent.xyz))
    except Exception as error:
        print("growth_procs.get_eigen_entity: caught unknown removal: ", str(error))

    len_after = len(entities)
    #print "len(entities), before=%i, after=%i" % (len_before,len_after)
    entities = map(np.array,entities)
    return entities

def prepare_next_front(front,new_pos,radius_factor=None,set_radius=None,add_order=False) :
    """
    Wrapper function to prepare a new front based on a new positions. \
    This function takes care of the internal variables (:code:`path_length`) \
    and :code:`order` of the new front to be created.

    Parameters
    ----------
    front : :py:class:`front.Front`
    new_pos : np.array
       New position in 3D space
    radius_factor : float
       If the radius is not set, a radius_factor can be given. Then, the \
       radius value is simply multiplied by this factor.
    set_radius : float
       New radius
    add_order : boolean
       Set to True of this front is the first of a branch (as well as \
       at the soma, whose order=0 per NeuroMac convention while intial \
       neurites have order=1)

    Returns
    -------
    new_front : :py:class:`front.Front`
       
    """
    #new_front = copy.deepcopy(front)
    new_front = copy.copy(front)
    new_front.parent = front
    new_front.xyz = new_pos
    if not radius_factor == None:
        #print "front, radius_factor, front.radius:",radius_factor," * radius:",front.radius," = ", (front.radius*radius_factor)
        #time.sleep(1)
        new_front.radius = front.radius*radius_factor
    if not set_radius == None:
        new_front.radius= set_radius
    new_front.path_length = new_front.path_length+np.sqrt(np.sum((front.xyz-new_front.xyz)**2))
    if add_order :
        new_front.order = front.order + 1
    return new_front        

def normalize_length(vec,norm_L) :
    """
    Scale a direction vector to a given length

    Parameters
    ----------
    vec : np.array
       Direction vector to scale
    norm_L : float
       New length of the vector

    Returns:
    new_vec : np.array
    """
    if vec.all() == None:
        return [0,0,0]
    return vec / np.sqrt(np.sum((vec)**2)) * norm_L

def direction_to(front,list_of_others,what="average") :
    """
    Compute the direction vector towards *some other entities*.

    Parameters
    ----------
    front : :py:class:`front.Front`
       Front to be used as the origin (starting point) of the direction \
       vector
    list_of_others : list
       List created by ***get_enity*** LINK
    what : string
       String options:
       
       - 'nearest': compute the direction vector to the nearest \
          point in the list_of_others.
       - 'average': default. TODO. Currently returns a list of direction \
          vectors
       -  'all':  Return a list of all direction vectors
    """
    if len(list_of_others) == 0 :
        # shouldn't this be array([0,0,0])??? No items, null vector
        #return np.array([0,0,0])
        return None # handle the error somewhere else
    pos = front.xyz
    vecs = []
    smallest_vec = np.array([100000000,100000000,100000000])
    for loc in list_of_others :
        vec = loc-pos#pos-loc
        vecs.append(vec)
        # THIS SHOULD BE MEMORIZED / TABULATED ++++++++++++++++++++ <-----
        if np.sqrt(np.sum((loc-pos)**2)) < np.sqrt(np.sum((smallest_vec)**2)) :
            smallest_vec = vec
    if what == "nearest" :
        return smallest_vec
    else :
        return vecs

def gradient_to(front,list_of_others,strength,decay_factor,what="average",cutoff=0.01) :
    """
    Determines the weighted vector towards *some other entities*. \
    The length of the direction vectors is scaled according to the \
    distance towards the "other entity", a constant weight and its \
    distance-dependent decay constant. Decay is exponential, hence \
    large decay factors (close to 1) behave as constant/linear decay \
    while smaller ones (<< 1) have a strong decay.

    Parameters
    -----------
    front : :py:class:`front.Front`
       Front to be used as the origin (starting point) of the direction \
       vector
    list_of_others : list
       List created by ***get_enity*** LINK
    strength :  float
       initial weight
    decay_factor : float
       Exponent with which the strength decays (or increases if > 1)
    what : string
       String options:
       
       - 'nearest': compute the direction vector to the nearest \
          point in the list_of_others.
       - 'average': default. TODO. Currently returns a list of direction \
          vectors
       -  'all':  Return a list of all direction vectors
    cutoff : float
       Minimal direction vector length. Vectors that are smaller after \
       are discarded.
    """
    #all_vecs = direction_to(front,list_of_others,what=what)
    nearest_vec = np.array([100000000,100000000,100000000])
    if what == "nearest" :
        nearest_vec = direction_to(front,list_of_others,what=what)
        L = np.sqrt(sum((nearest_vec)**2))
        decay = compute_exp_decay_factor(strength,\
                                         decay_factor,\
                                         L)
        # print "self_rep, L=%.2f, decay=%.2f " % (L,decay)
        nearest_vec = (nearest_vec/ L * decay)
        L = np.sqrt(sum((nearest_vec)**2))
        # print "self_rep, L=%.2f, decay=%.2f, cutoff=%.2f" % (L,decay,cutoff)
        if L < cutoff:
            #print "return [0,0,0]"
            return np.array([0,0,0])
        else:
            return nearest_vec
    elif what=="all":
        output=[]
        nearest_vecs = direction_to(front,list_of_others,what=what)
        for nearest_vec in nearest_vecs:
            L = np.sqrt(sum((nearest_vec)**2))
            decay = compute_exp_decay_factor(strength,\
                                             decay_factor,\
                                             L)
            # print "self_rep, L=%.2f, decay=%.2f " % (L,decay)
            nearest_vec = (nearest_vec/ L * decay)
            L = np.sqrt(sum((nearest_vec)**2))
            # print "self_rep, L=%.2f, decay=%.2f, cutoff=%.2f" % (L,decay,cutoff)
            if L < cutoff:
                pass # only gradients exceeding thr cutoff will be returned
            else:
                output.append(nearest_vec)
        return output
        
    
# to be replaced
def compute_soma_rep_vec(front,strength=1.0,decay=1.0) :
    """ Compute the respulsive vector directed away from the soma\
    given a provided reference position. The resultant vector is
    AB, with A=[0,0,0] and B the returned array

    Parameters
    ----------

    Returns
    -------
    """
    if np.all(front.soma_pos == front.xyz) :
        vec_from_soma = np.array([0.000001,0.0,0.0])
        print("################################### soma == front")
        import time
        time.sleep(2)
    else :
        vec_from_soma = front.xyz - front.soma_pos # move to origin (of coordinate system)
    L = np.sqrt(sum(vec_from_soma**2))
    decay = compute_exp_decay_factor(strength,decay,L)
    print("soma_pos=%s,f.xyz: %s,L=%.2f,soma decay=%.2f " % (str(front.soma_pos), str(front.xyz), L, decay))
    return (vec_from_soma)/ L * decay
