User manual
===========

Users need to write two files: one describing the simulated volume and
one containing the growth-rules.

In this section we outline how to write both. Although, it is strongly
recommended not to start either file from scratch but by modifying
one of the examples (:ref:`examples`).

.. _config-file:

Configuration file
------------------

The configuration file describes the brain volume to be simulated. \
Below an example:

.. literalinclude:: ../../examples/demo_attraction/demo_attraction.cfg
    :linenos:
    :language: none

Each configuration file contains at least four main sections:


:code:`[system]`
~~~~~~~~~~~~~~~~

This section contains entries directly related to the framework, rather \
than the volume and the neuronal structures to simulate. The following \
entries are included

- seed=21051982
- proxy_sub_port= 5599
- proxy_pub_port= 5560
- pull_port=55002
- time_out = 10000 *[currently not used]*
- no_cycles=30
- out_db=demo_attraction/demo_attraction.db

The :code:`seed` is used to initialize the system and for the randomized locations of \
the initial parts of the structures to generate. All :code:`*_port` \
entries relate to the configuration of `ZeroMQ <http://zeromq.org/>`_ \
(also denoted as ZMQ). ZMQ is the messaging system used to implement \
the multi-agent system. The configured ports are TCP/IP ports used to \
communicate between the different components.

:code:`no_cycles` defines how many times each front has to be extended. \
:code:`out_db` specifies the location of the raw SQL output database. \

Additional optional parameter: :code:`recursion_limit`. If not set, the 
Python default (that depends on your system) is used. If set, the default
is overwritten by the provided integer number.


:code:`[substrate]`
~~~~~~~~~~~~~~~~~~~

This section describes the boundaries of the brain substrate.

.. note:: Currently, the brain volume is limited to a rectangular volume.

- dim_xyz = [100.0,100.0,100.0]

:code:`dim_xyz` determines the size of the rectangular volume, values \
represent [width,depth,height] (in NeuroMac, these correspond to x,y,z, \
which is different from "standard" physiological coordinates where y \
is used for the height or depth from the top surface, i.e., the pia \
in cortical slices). The left-bottom corner is always :math:`[0,0,0]`.

:code:`[sub_volumes]`
~~~~~~~~~~~~~~~~~~~~~~

Statements in this block define how the total volume should be decomposed \
into smaller blocks, which are subsequently parallelized by NeuroMaC.

.. note:: Currently, the volume decomposition is limited to rectangular \
   slicing of the volume.

- xa=1
- ya=1
- za=2

:code:`xa`, :code:`ya` and :code:`za` set the number of "slices" along \
the appropriate axis. The total number of subvolumes is hence \
:math:`SV = xa \times ya \times za`.


:code:`[cell_type_x]`
~~~~~~~~~~~~~~~~~~~~~~

This block is used to configure the neurites to be grown inside the \
simulated volume.

There are two distinct ways of generating many neurites simultaneously. \
The first way is illustrated in the above example and uses multiple \
:code:`cell_type_x` blocks. :code:`x` represents the name and can be \
any name (that would make a valid Python variable name, hence \
alphanumeric values and underscores). For each cell type the following \
variables need to be set.

- no_seeds=1
- algorithm = TestF_Left
- location = [[20,50,20],[20,50,20]]  
- soma_radius = 10

:code:`no_seeds` sets the number of instances of one cell type need \
to be created (here only 1). The :code:`algorithm` variable defines \
the growth-rules used by the fronts. :code:`location` is used to specify \
the initial position of the structure. The structure of the value is: \
[[x_min,y_min,z_min], [x_max,y_max,z_max]], which create a rectangular \
shape;  NeuroMaC uniformly samples a position from this rectangle. In \
case of only one seed, the location can be defined as any point in space \
(by setting x_min=xmax, y_min=y_max and z_min=z_max).

Optional parameter: :code:`minimum_self_distance` (float) to configure
the minimum distance between two fronts of the same structure. The default,
which is used when not set, is the radius of the "other" front / 2. In 
case you want to have very small increment in the neurite extension, 
you might want to set this value to the minimal increment size (in micron). 

.. warning:: The algorithm name must be in the :code:`$PYTHONPATH` and \
   must have the name :code:`<name>.py`. That is, if in the configuration \
   file the entry :code:`algorithm=my_rules` is listed, a Python file \
   named :code:`my_rules.py` must exist and must be in the path.

A second way to generate multiple structures is by setting the seed \
to any value > 1. In this case, the same growth-rules apply for all \
structures.

.. note:: Instantiating multiple structures with the same algorithm \
   does not mean they will be identical as the growth-rules usually \
   also include a random component.

.. _implement-rules:

Recording synapse locations
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The location of putative synapses between *any neurites* can be recorded
by specifying a :code:`synapse_distance` and :code:`syn_db` option in the
:code:`[system]` section of the configuration file.

- synapse_distance = 5
- syn_db = demo_attraction/syn.db 


:code:`synapse_distance` is optional and sets the maximum distance 
between line segments  (e.g., the frustum or cylinder between two 
successive fronts) in micron at which a synapse could form. If set, these 
putative synapse locations will be recorded in a separate SQL 
database named with the value from :code:`syn_db`.


.. note:: Recorded synapse location are indiscriminate to the neurite type:
          locations are recorded as long as the minimal distance between two
          frusta belonging to some neurite is smaller than the specific
          distance. As such, a post-processing step is required to filter
          the synapse locations of interest.

.. _neuromac-rules:

Front growth-rule specification
--------------------------------

The second part required for running a simulation using NeuroMaC is 
the specification of the growth-rules. Growth-rules are expressed 
in plain Python code.

The fixed part of the growth-rule is the definition so that NeuroMaC 
knows which function to execute. As such Python growth-rule *must* 
contain the following: ::

   def extend_front(front,seed,constellation) :
       # Do things, this front either branches, terminates or 
       # elongates. In these cases either two, none or one new front 
       # has to be returned

       # In case of elongation
       return [new_front]

A front is a :py:class:`front.Front` data structure. In NeuroMaC its 
role is double: a front is both a physical entity in space with a 
location and radius but as well a phenomenological implementation of 
a growth-cone that contains the growth-rules.

Because Python is a full programming language there are no real 
limitations on how to implement the growth-rules. A few  :ref:`examples` 
are provided. 

Helper functions
~~~~~~~~~~~~~~~~

A few helper functions are packed with NeuroMaC to ease the implementation 
of growth-rules :ref:`helper-functions`.

Terminating a front
~~~~~~~~~~~~~~~~~~~

The most trivial case is the termination of a front. In this case, the 
:code:`extend_front()` can be empty (bad practice) or contain 
:code:`return []` (good practice).

Elongating a front
~~~~~~~~~~~~~~~~~~

The simplest functional case is the extension of a front. In that case 
the user specifies the next position of the front. 

.. code-block:: python
   :emphasize-lines: 8,9,11

   L_EXTEND=5

   def extend_front(front,seed,constellation) :
       # Elongate: decide the next location
       current_position = front.xyz

       # extend in random direction
       rnd_dir = unit_sample_on_sphere()
       new_pos = normalize_length(rnd_dir,L_EXTEND)
       new_pos = front.xyz + new_pos
       new_front = prepare_next_front(front,new_pos,set_radius=1.0)
       return [new_front]

This snippet highlights the use of helper functions 
:py:func:`growth_procs.unit_sample_on_sphere`, :py:func:`growth_procs.normalize_length` 
and :py:func:`growth_procs.prepare_next_front`.

Branching a front
~~~~~~~~~~~~~~~~~

Branching a front is similar to elongating a front. The difference lies 
in the creation of two new fronts rather than one. 

.. code-block:: python
   :emphasize-lines: 7,8,14,15

   L_EXTEND=5

   def extend_front(front,seed,constellation) :
       # Elongate: decide the next location
       current_position = front.xyz

       new_fronts = []
       for i in range(2):
           # extend in random direction
           rnd_dir = unit_sample_on_sphere()
           new_pos = normalize_length(rnd_dir,L_EXTEND)
           new_pos = front.xyz + new_pos
           new_front = prepare_next_front(front,new_pos,set_radius=1.0)
           new_fronts.append(new_front)
       return new_fronts

.. note:: Make sure the newly created child fronts do not overlap because
   NeuroMaC will terminate one of them because of illegal structural overlap.


Interactions between structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Suppose the following declaration of front in the configuration file 
(taken from the :ref:`example-attraction`):

.. code-block:: python

    [cell_type_1]
    no_seeds=1
    algorithm = TestF_Left
    location = [[20,50,20],[20,50,20]]  
    soma_radius = 10

    [cell_type_2]
    no_seeds= 1
    algorithm = TestF_Right
    location = [[80,80,80],[80,80,80]] 
    soma_radius = 10

Now the distinct the cell types can be used as a environmental cues. 
A list of all entities of a certain type can be requested through the 
helper :py:func:`growth_procs.get_entity`. Subsequently, in case of
attraction, the vector direction towards the other entity can be computed
using :py:func:`growth_procs.direction_to` (or
entity :py:func:`growth_procs.gradient_to`). The vector direction
can be easily inverted by :math:`-1 \times vec_dir` to simulate
repulsion.

.. code-block:: python
   :emphasize-lines: 5,7

   L_EXTEND=5

   def extend_front(front,seed,constellation) :
       # query structures from another cell
       other_entities = get_entity("cell_type_2",constellation)
       # now, e.g., compute the direction to these structures
       dir_to_entity = direction_to(front,other_entities,what="nearest")
       # scale to a given length
       new_pos = front.xyz + normalize_length(dir_to_entity,L_EXTEND)
       new_front = prepare_next_front(front,new_pos,set_radius=1.5)
       return [new_front]

.. _neuromac-interactions-front-substrate:

Interactions between fronts and substrate
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To allow explicit interaction between a front and the surrounding 
substrate, environmental cues have to be inserted into the simulated 
volume. This can be done in the configuration file. ::

   [substrate]
   dim_xyz = [200.0,200.0,205.0]
   pia = pia_point.pkl

In this case a "pia" is declared in the volume. The value is a pickle 
file containing a set of points. Below an example of how to generate 
such a file. 

.. literalinclude:: code/generate_pia.py
    :language: python

Once the volume is configured to contain this "pia" it can be referred 
to as any other entity in the growth rules.

.. code-block:: python
   :emphasize-lines: 5,7

   L_EXTEND=5

   def extend_front(front,seed,constellation) :
       # query structures from another cell
       other_entities = get_entity("pia",constellation)
       # now, e.g., compute the direction to these structures
       dir_to_entity = direction_to(front,other_entities,what="nearest")
       # scale to a given length
       new_pos = front.xyz + normalize_length(dir_to_entity,L_EXTEND)
       new_front = prepare_next_front(front,new_pos,set_radius=1.5)
       return [new_front]

Updating the substrate
~~~~~~~~~~~~~~~~~~~~~~~

NeuroMaC also features bi-directional interaction with the environment. 
That is, fronts can leave cues in the substrate while growing. These 
cues:

- Do not have a physical extend (no overlap detection)
- Are permanent (do not fade over time)
- Can be accessed as any other contextual cue by other fronts

The bi-directional interaction is implemented in a front's growth-rules:

.. code-block:: python
   :emphasize-lines: 8,9

   L_EXTEND = 5

   def extend_front(front,seed,constellation) :
       new_pos =front.xyz + np.array([-1.0*L_EXTEND,0,0])
       new_front = prepare_next_front(front,new_pos,set_radius=3.0)

       # secrete some "entity_x"
       if front.path_length > 40 and front.path_length < 46 :
           return [new_front],{"substance_x":front} 
       else :
           return [new_front]

Augmenting :py:class:`front.Front`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By using plain Python code to implement growth-rules, we can also exploit 
Python specific features. For instance, a front is a regular Python 
:code:`object`. Hence, we can dynamically add attributes (variables) to 
a front. Two such attributes are automatically added and updated by 
NeuroMaC, namely :code:`path_length` and :code:`order`. Other attributes 
can be used at discretion of the end-user, for instance, to label 
special branches that require a specific set of growth-rules as illustrated 
in the following example.

.. code-block:: python
   :emphasize-lines: 5,7,9

   L_EXTEND = 5

   def extend_front(front,seed,constellation) :
       if np.random.random() <0.2 :
           front.my_label=True
       else :
           front.my_label=False

       if front.my_label:
           new_pos =front.xyz + np.array([-1.0*L_EXTEND,0,0])
       else:
           other_entities = get_entity("cell_type_2",constellation)
           dir_to_entity = direction_to(front,other_entities,what="nearest")
           new_pos = front.xyz + normalize_length(dir_to_entity,L_EXTEND)
           new_front = prepare_next_front(front,new_pos,set_radius=1.5)
       return [new_front]
