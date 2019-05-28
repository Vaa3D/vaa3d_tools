#!/bin/bash

# uncomment next line for interactive checking of generated output
PYTHON="ipython2 --pylab -i"
# non-interactive shell. Check results afterwards
PYTHON="python2.7"

# Random walk
# Not truly a rondom walk as the generated structure will terminate \
# when a structural overlap occurs. Detecting and avoiding a \
# structual ooverlap is a main feature of NeuroMaC
time PYTHONPATH=/home/wpkenan/anaconda3/envs/py27/bin/python $PYTHONPATH ../Admin.py 1 random_walk/random_walk.cfg
$PYTHONPATH ../scripts/generate_SWC_data.py random_walk/random_walk.cfg random_walk/random_walk.db
$PYTHONPATH ../scripts/helper_generate_movie.py random_walk/random_walk.cfg random_walk/random_walk.db
$PYTHONPATH ../scripts/generate_wire_plot.py random_walk/random_walk.cfg random_walk/random_walk.db
$PYTHONPATH         ../scripts/generate_radii_plot.py random_walk/random_walk.cfg random_walk/random_walk.db

# Attraction demo: two structures are grown
# With two subvolumes to indicates that environmental cues are not \
# limited to one Subvolume. One branch will grow straight from left to \
# right, one curve starting at the left is attracted by the other \
# curve.
time PYTHONPATH=/home/wpkenan/anaconda3/envs/py27/bin/python:$PYTHONPATH python ../Admin.py 2 demo_attraction/demo_attraction.cfg
python ../scripts/generate_SWC_data.py demo_attraction/demo_attraction.cfg demo_attraction/demo_attraction.db
# python ../scripts/helper_generate_movie.py demo_attraction/demo_attraction.cfg demo_attraction/demo_attraction.db
$PYTHON ../scripts/generate_wire_plot.py demo_attraction/demo_attraction.cfg demo_attraction/demo_attraction.db
$PYTHON ../scripts/generate_radii_plot.py demo_attraction/demo_attraction.cfg demo_attraction/demo_attraction.db

# Straight to pia: one front runs straight to the pia.
# The pia is a point-cloud, see cfg file and online documentation
time PYTHONPATH=/home/wpkenan/anaconda3/envs/py27/bin/python:$PYTHONPATH python ../Admin.py 1 straight_to_pia/straight.cfg
python ../scripts/generate_SWC_data.py straight_to_pia/straight.cfg straight_to_pia/straight.db
# python ../scripts/helper_generate_movie.py straight_to_pia/straight.cfg straight_to_pia/straight.db
$PYTHON ../scripts/generate_wire_plot.py straight_to_pia/straight.cfg straight_to_pia/straight.db
$PYTHON ../scripts/generate_radii_plot.py straight_to_pia/straight.cfg straight_to_pia/straight.db

# Update environment demo
# One process grows straight and "drops a cue" / "secretes a cue" at \
# some point. Another growing process senses this cue when secreted and \
# turns toward this cue
time PYTHONPATH=/home/wpkenan/anaconda3/envs/py27/bin/python:$PYTHONPATH python ../Admin.py 1 update_environment/update_env.cfg
python ../scripts/generate_SWC_data.py update_environment/update_env.cfg update_environment/update.db
# python ../scripts/helper_generate_movie.py update_environment/update_env.cfg update_environment/update.db
$PYTHON ../scripts/generate_wire_plot.py update_environment/update_env.cfg update_environment/update.db
$PYTHON ../scripts/generate_radii_plot.py update_environment/update_env.cfg update_environment/update.db

# Many to pia demo
# Multiple branching structure growing towards the pia
# For demonstration purposes, putative synapse locations are recorded 
# between neurites.
# Movie commented by default as it takes a long time to generate
time PYTHONPATH=to_pia/:$PYTHONPATH python ../Admin.py 8 to_pia/many.cfg
python ../scripts/generate_SWC_data.py update_environment/update_env.cfg update_environment/update.db
# python ../scripts/helper_generate_movie.py to_pia/many.cfg to_pia/many.db
$PYTHON ../scripts/generate_wire_plot.py to_pia/many.cfg to_pia/many.db to_pia/many_syn_locs.db
$PYTHON ../scripts/generate_radii_plot.py to_pia/many.cfg to_pia/many.db

# One neurite with two branches. Both branches grow straight to the pia.
time PYTHONPATH=to_pia/:$PYTHONPATH python ../Admin.py 8 to_pia/to_pia.cfg
python ../scripts/generate_SWC_data.py to_pia/to_pia.cfg to_pia/to_pia.db
# python ../scripts/helper_generate_movie.py to_pia/to_pia.cfg to_pia/to_pia.db
$PYTHON ../scripts/generate_wire_plot.py to_pia/to_pia.cfg to_pia/to_pia.db 
$PYTHON ../scripts/generate_radii_plot.py to_pia/to_pia.cfg to_pia/to_pia.db

# Attraction accross many subvolumes.
# Illustration of how the current volume decomposition and handling
# of cues "in the eye of the beholder" influence the results.
# Similar setup as the normmal attraction demo, except that there are
# many more sub volumes and the two fronts are initialized in
# non-neighboring sub volumes.
time PYTHONPATH=demo_attraction/:$PYTHONPATH python ../Admin.py 16 demo_attraction/demo_distant_attraction.cfg 
python ../scripts/generate_SWC_data.py demo_attraction/demo_distant_attraction.cfg demo_attraction/demo_distant_attraction.db
# python ../scripts/helper_generate_movie.py demo_attraction/demo_distant_attraction.cfg demo_attraction/demo_distant_attraction.db
$PYTHON ../scripts/generate_wire_plot.py demo_attraction/demo_distant_attraction.cfg demo_attraction/demo_distant_attraction.db
$PYTHON ../scripts/generate_radii_plot.py demo_attraction/demo_distant_attraction.cfg demo_attraction/demo_distant_attraction.db

# Example of built-in structural-conflict resolution. 
# Four neurites grow straight apired into two planes. NeuroMaC
# does not allow intersections and will try to resolve the intersection.
# If no resolution is found within the specified number of `avoidance_attempts`,
# the neurite is terminated
time PYTHONPATH=intersection/:$PYTHONPATH python ../Admin.py 1 intersection/intersection.cfg 
python ../scripts/generate_SWC_data.py intersection/intersection.cfg intersection/intersection.db
python ../scripts/helper_generate_movie.py intersection/intersection.cfg intersection/intersection.db
$PYTHON ../scripts/generate_wire_plot.py intersection/intersection.cfg intersection/intersection.db
$PYTHON ../scripts/generate_radii_plot.py intersection/intersection.cfg intersection/intersection.db

# Self-avoidance example
time PYTHONPATH=self_avoidance/:$PYTHONPATH python ../Admin.py 1 self_avoidance/selfavoidance.cfg 
python ../scripts/generate_SWC_data.py self_avoidance/selfavoidance.cfg self_avoidance/selfavoidance.db
python ../scripts/helper_generate_movie.py self_avoidance/selfavoidance.cfg self_avoidance/selfavoidance.db
$PYTHON ../scripts/generate_wire_plot.py self_avoidance/selfavoidance.cfg self_avoidance/selfavoidance.db
$PYTHON ../scripts/generate_radii_plot.py self_avoidance/selfavoidance.cfg self_avoidance/selfavoidance.db

# Follow a gradient. Attraction depends on the distance from the attractor.
# The randome xcursions of the neurite become smaller as the neurite approaches
# the attractor
time PYTHONPATH=gradient/:$PYTHONPATH python ../Admin.py 1 gradient/to_gradient.cfg 
python ../scripts/generate_SWC_data.py gradient/to_gradient.cfg gradient/gradient.db
python ../scripts/helper_generate_movie.py gradient/to_gradient.cfg gradient/gradient.db
$PYTHON ../scripts/generate_wire_plot.py gradient/to_gradient.cfg gradient/gradient.db
$PYTHON ../scripts/generate_radii_plot.py gradient/to_gradient.cfg gradient/gradient.db
