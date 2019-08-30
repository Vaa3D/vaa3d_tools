import numpy as np

import time # only for testing
class Front(object) :
    """
    Key component of NeuroMac. A Front is a phenomenological \
    growth cone with a dual character:

    - A Front is a point in space with a radius. When connected \
       to another Front, a frustrum (or cylinder) is created.
    - A Front is a phenomenological growth cone that contains a \
       set of growth rules.

    Each Front its own set of rules expressed in a Python file \
    that listed in the algo_name variable. Initially, a \
    neuronal structure consists of one Front but later, when a structure \
    develops by simultaneous expanding of multiple front, each front \
    can contain different growth-rules.

    .. warning:: This code is not to be changed and is directly called \
       by NeuroMaC
    """
    def __init__(self,entity_name,algo_name,xyz,radius,\
                 path_length,order) :
        """
        Constructor
                 
        Parameters
        ----------
        entity_name : string
        algo_name : string
           File name containing the growth rules must be named :code:`<algo_name>.py`. \
           It's this name that is also specified in the configuration file.
        xyz : np.array
           3D vector
        radius : float
        """
        #: The name given to and entity
        #: This name is to be used to query the substrate
        self.entity_name = entity_name
        #: Growth-rules to use
        self.algo_name = algo_name
        #: 3D location
        self.xyz = xyz
        #: Radius of the front
        #: Consecutive fronts form a frustrum or cylinder
        self.radius = radius
        #: *Not directly set*. Attrribute set by :py:func:`growth_procs.prepare_next_front`
        self.path_length= path_length
        #: *Not directly set*. Attrribute set by :py:func:`growth_procs.prepare_next_front`
        self.order = order
        #:  SWC-type as define in Cannon et al 1998
        self.swc_type = 7 # SWC-type field
        self.soma_pos =None
        self.parent = None

    def __str__(self):
        return "Front " + self.entity_name +": " + str(self.xyz)

    def __eq__(self,other):
        if other == None: return False
            
        if not self.entity_name == other.entity_name: return False
        if not self.algo_name == other.algo_name: return False
        if not np.allclose(self.xyz,other.xyz): return False
        if not self.radius == other.radius: return False
        if not self.path_length == other.path_length: return False
        if not self.order == other.order: return False
        if not self.swc_type == other.swc_type: return False
        if other.soma_pos == None:
            if not self.soma_pos == None:
                return False
        else:
            #print "self.soma_pos=", self.soma_pos, ", other.soma_pos=",other.soma_pos
            if not np.allclose(self.soma_pos,other.soma_pos): return False
        if not self.parent == other.parent: return False

        # otherwise, these objects are the same!
        return True

    def __ne__(self,other):
        return not self == other

    def __cmp__(self,other):
        if self.__eq__(other): return 0
        else: return -1

    # quick and dirty way to create a unique hash, needed to work with sets
    def __key(self):
        # WARNING!!! tuple(self.xyz) potentially not correct with rounding errors...
        return (self.entity_name, self.algo_name, tuple(self.xyz),\
                self.radius,self.path_length,self.order,\
                self.swc_type)

    def __hash__(self):
        return hash(self.__key())
    
    def extend_front(self,seed,constellation,virtual_substrate={}) :
        """
        Main function. Outsources the execution to the actual growth-rules \
        specified in the configuration file (:code:`cell_type_x` > \
        :code:`algorithm`).

        See :ref:`implement-rules`

        .. note:: This function is not to be modified and is only used \
        by framework code (Subvolume.py)

        Parameters
        ----------
        seed : int
        constellation : dict of list of np.array
        virtual_substrate : dict of list

        Returns
        -------
        front : :class:`Front`
           List of fronts that are the "extension" of the current front. \
           By convention, an empty list of None represents a terminating \
           front. A list with one or two entries represents an elongating =\
           or a branching front, respectively. One exception can be the \
           soma from which multiple new fronts can sprout.
        """

        # process the virtual substrate information and set variable in "self"
        for key in virtual_substrate.keys():
            # setattr(x, attr, 'magic')
            for x in virtual_substrate[key].keys():
                rectangle = virtual_substrate[key][x]
                b0 = np.array(rectangle[0])
                b1 = np.array(rectangle[1])
                if np.all(self.xyz>b0) and np.all(self.xyz<b1): # works for rectangle only...
                    setattr(self,key,x)
                    print(">>>> FOUND: ", key, " = ", x)

        # outsource the real call to extend a front
        import  sys
        sys.path.insert(0,'D:/gen_vir_experiment_code/neuromac-master/')
        globals()[self.algo_name] = __import__(self.algo_name)
        ret = globals()[self.algo_name].extend_front(self,seed,constellation)

        return ret

