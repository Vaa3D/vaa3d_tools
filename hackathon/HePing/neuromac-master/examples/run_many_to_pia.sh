#!/bin/bash

# uncomment next line for interactive checking of generated output
PYTHON="ipython2 --pylab -i"
# non-interactive shell. Check results afterwards
PYTHON="python2.7"

# Many to pia demo
# Multiple branching structure growing towards the pia
# For demonstration purposes, putative synapse locations are recorded 
# between neurites.
# Movie commented by default as it takes a long time to generate
PYTHONPATH=/home/wpkenan/anaconda3/envs/py27/bin/python
time  $PYTHONPATH ../Admin.py 8 to_pia/many.cfg
$PYTHONPATH ../scripts/generate_SWC_data.py update_environment/update_env.cfg update_environment/update.db
# python ../scripts/helper_generate_movie.py to_pia/many.cfg to_pia/many.db
$PYTHONPATH ../scripts/generate_wire_plot.py to_pia/many.cfg to_pia/many.db to_pia/many_syn_locs.db
$PYTHONPATH ../scripts/generate_radii_plot.py to_pia/many.cfg to_pia/many.db
