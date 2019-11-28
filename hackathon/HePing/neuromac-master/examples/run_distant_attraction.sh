#!/bin/bash

# uncomment next line for interactive checking of generated output
PYTHON="ipython2 --pylab -i"
# non-interactive shell. Check results afterwards
PYTHON="python2.7"

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
