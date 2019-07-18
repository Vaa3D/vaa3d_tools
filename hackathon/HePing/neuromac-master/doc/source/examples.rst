.. _examples:

NeuroMaC examples
=================

This section provides examples highlighting the  key features of NeuroMac 
and illustrate how to use the software suite.

All examples are found in the :code:`examples` directory. Bash shell 
scripts are available to run the models and convert the raw output into 
useful output (such as a SWC-file or figure). Check these shell scripts 
for individual commands.

.. note:: For beginners it is strongly advised not to start a NeuroMac \
   model from scratch but to start with an example and adopt it to \
   meet your needs.

.. _example-random-walk:

Random walk
-----------

Not truly a random walk. Rather, several branches grow from the soma, 
start of straight before orientating themselves randomly. 
Consequently, the branches do not grow for long as they will inevitably 
bump into one another. The resultant structural overlaps are detected 
by NeuroMac and cause the branches to terminate.

Run code
~~~~~~~~

Run the example by executing the prepared bash script
  
.. code-block:: bash

   $ cd examples # Or from wherever you came. From now I assume you're in the examples directory
   $ ./run_random_walk.sh

This script (as well as all further example scripts) will do several things: 

1. Run the model. Running the model generates a SQL database 
   containing all generated structures.
2. Generate one SWC-file of the produced structure
3. Generate a movie of the *development* of the structure.
4. Generate two plots of the produced structure with and without 
   radii, respectively.

The following files will be produced: ::

   random_walk.db # raw SQL database
   cell_type_1__0.swc  # SWC file
   random_walk.mp4 # movie
   random_walk_radii.pdf # plot with radii
   random_walk_wire.pdf  # wire plot


Implementation details
~~~~~~~~~~~~~~~~~~~~~~

The user has to write two files: a configuration file and a file 
containing the growth cones. The configuration file 
(:code:`random_walk/random_walk.cfg`) defines properties of the brain 
volume to simulate as well as which structures to grow in accordance 
to a specified set of rules. Also, the configuration file contains 
some parameters for NeuroMaC itself (see :ref:`config-file`).

Of most interest is the file containing the growth-rules: 

:code:`random_walk/Random_machine.py`

.. literalinclude:: ../../examples/random_walk/Random_machine.py
    :linenos:
    :language: python

In the first three lines several helper functions are imported. These 
come with NeuroMac (:ref:`helper-functions`). 
In short, the :code:`unit_sample_on_sphere` is used to sample a 3D 
vector used as the component for random growth. :code:`normalize_length` 
is used to scale a vector to the desired length while 
:code:`prepare_next_front` is a secure wrapper to extend a front (If 
the work "front" does not ring a bell, please consult the :ref:`neuromac-rationale`)

The actual growth-rule is described in :code:`extend_front(front,seed,constellation)`. 
**Always include this exact method definition** or NeuroMaC will not work!

In this case two separate rules are included. Two is the unofficial 
default: one rule describes the behaviour at the soma (or any other 
initial point with :code:`front.order==0`) while the other rules 
covers the rest.  Here five initial branches are started. Afterwards 
a front elongates with purely random angles.

- :code:`rnd_dir = unit_sample_on_sphere()`: Get a unit length random
  3D vector.
- :code:`new_pos = normalize_length(rnd_dir,L_NORM*5)`: Scale a vector 
  to a length of :code:`L_NORM*5`.
- :code:`new_front = prepare_next_front(front,new_pos,set_radius=1.0,add_order=True)`: 
  Create the next front.

.. warning:: :code:`extend_front()` always has to return a list of fronts. 
   If only a single front is returned (as happens for elongations), return a 
   list: :code:`return [new_front]`. See :ref:`neuromac-rules`

.. _example-attraction:

Attraction between fronts
-------------------------

Basic example illustrating phenomenological attraction between two 
developing neurites. One neurite structure simply grows straight 
while the second neurite is attracted by the first one.

In addition, this example illustrates how to parallelize the execution
by decomposing the volume.

Code can be run by executing the following wrapper-command:

.. code-block:: bash

   $ ./run_attraction_demo.sh


Configuration file
~~~~~~~~~~~~~~~~~~

Below the configuration file. Note the section :code:`sub_volumes`. 
The volume can be decomposed in a number of rectangles along the 
x, y and z-directions as specified by :code:`xa`, :code:`ya` and 
:code:`za`. Think of this decomposition as making the specified number 
of slices along an axis. The total number of volumes is 
:math:`SV = xa \times ya \times za`. The number :math:`SV` has to be 
given as argument to the Admin.py script as it specifies the number of 
Subvolume agents.

.. literalinclude:: ../../examples/demo_attraction/demo_attraction.cfg
    :linenos:
    :language: none               
    :emphasize-lines: 14-17

Implementation details
~~~~~~~~~~~~~~~~~~~~~~

The first neurite has a trivial growth-rule so that it grows straight from
one side to the other of the volume.

.. literalinclude:: ../../examples/demo_attraction/TestF_Right.py
    :linenos:
    :language: python

The second neurite is more interesting in that its direction of growth
depends on the first neurite. More precisely, the second neurite is
attracted to the nearest front of the first neurite.

.. literalinclude:: ../../examples/demo_attraction/TestF_Left.py
    :linenos:
    :emphasize-lines: 12-14


.. warning:: In case the volume is decomposed into many sub volumes, \
  and attraction between fronts of non-neighboring sub volumes occurs \
  , special piece of code needs to be added to the growth rule. In the \
  aforementioned case, during the first update cycle an updating front \
  cannot know anything about fronts that are in sub volumes beyond the \
  direct neighbors. Only in the second cycle, they will receive a "summary" \
  of all distant sub volumes. As such, during the first cycle, one has to \
  catch the case in which no attracting/repulsive fronts are found. 

  .. literalinclude:: ../../examples/demo_attraction/TestF_Left_Distant.py
      :linenos:
      :emphasize-lines: 13-17

  In this example, we quickly fixed the problem by extending the front \
  a little bit during the first cycle when no attraction from a distant \
  front is felt. From the second cycle on, the front behaviour is as \
  expected.

  This example is included in the :code:`examples` directory and can be \
  run by executing :code:`./run_distant_attraction.sh`.


.. _example-pia-attraction:

Interaction between fronts and the substrate
---------------------------------------------

One way of interactions between a front and the substrate is through
entities that are physically located in the substrate. For instance,
borders can be added by means of a point-cloud. In case of the cortex, 
a gradient leading towards the pia exists; most likely through reelin
excreted from Cahal-Retzius cells *REF* or radial glia *REF*.

In this example we set up such a point-cloud representing the pia and to
which a neurite will be attracted. (Check :ref:`neuromac-interactions-front-substrate` 
for info about generating the point cloud).

Code can be run by executing the following wrapper-command:

.. code-block:: bash

   $ ./run_env_attraction1.sh


Substrate configuration
~~~~~~~~~~~~~~~~~~~~~~~

To register the point cloud in the simulated volume, add it in the
:code:`substrate` section of the config file as illustrated below.

.. literalinclude:: ../../examples/straight_to_pia/straight.cfg
    :linenos:
    :emphasize-lines: 13

Implementation details
~~~~~~~~~~~~~~~~~~~~~~~

The growth-rules are very similar to the rules required to implement
attraction between neurites. The sole difference is that a list of
"pia" entities is queried from the substrate.

.. literalinclude:: ../../examples/straight_to_pia/Straight.py
    :linenos:
    :emphasize-lines: 15

.. _example-bi-directional:

Fronts updating the substrate
-----------------------------

This example illustrates how a front can update the substrate by
leaving an cue. This cue can be picked up by another front (or by the 
front itself; but that would be pointless) in the usual way.

Code can be run by executing the following wrapper-command:

.. code-block:: bash

   $ ./run_update_env.sh


Configuration file
~~~~~~~~~~~~~~~~~~~

The configuration file contains no special instructions.

.. literalinclude:: ../../examples/update_environment/update_env.cfg
    :linenos:

Front implementation
~~~~~~~~~~~~~~~~~~~~~

One front drops a series of cues at a specific moment. For demonstration
purposes this moment is expressed in terms of update steps (a attribute
of :py:class:`front.Front`).

.. literalinclude:: ../../examples/update_environment/Update_env_machine.py
    :linenos:
    :emphasize-lines: 14
    :language: python

The other neurite can try to sense this substance. However, as long as
the cue is not available, the neurite should catch the case that 
:py:func:`growth_procs.get_entity` returns an empty list, which cannot
be passed to the :py:func:`growth_procs.direction_to` function.

.. literalinclude:: ../../examples/update_environment/Attracted_by.py
    :linenos:
    :language: python

Miscellaneous examples
-----------------------

Forests of neurons
~~~~~~~~~~~~~~~~~~

NeuroMac offers the opportunity to generate large numbers of neurites 
that may or may not have the same growth-rules. In this example, we
grow a forest of neurons according to the same growth rules.

This can be specified in the configuration file [here :code:`to_pia/many.cfg`]:

.. literalinclude:: ../../examples/to_pia/many.cfg
    :lines: 21-25
    :language: none

Built-in structural conflict detection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:code:`./run_intersection.sh`


Self-avoidance
~~~~~~~~~~~~~~

:code:`./run_selfavoidance.sh`


Gradual attraction
~~~~~~~~~~~~~~~~~~

Distance-dependent attraction

:code:`./run_gradient.sh`
