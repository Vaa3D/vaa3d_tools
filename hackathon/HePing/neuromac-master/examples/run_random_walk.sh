#!/bin/bash

# uncomment next line for interactive checking of generated output
PYTHON="ipython2 --pylab -i"
# non-interactive shell. Check results afterwards
PYTHON="python2.7"

# Random walk
# Not truly a rondom walk as the generated structure will terminate \
# when a structural overlap occurs. Detecting and avoiding a \
# structual ooverlap is a main feature of NeuroMaC
#PYTHONPATH=./random_walk/:$PYTHONPATH python
mypython=C:/Python27/python2
time $mypython ../Admin.py 1 random_walk/random_walk.cfg
$mypython ../scripts/generate_SWC_data.py random_walk/random_walk.cfg random_walk/random_walk.db
$mypython ../scripts/helper_generate_movie.py random_walk/random_walk.cfg random_walk/random_walk.db
$mypython ../scripts/generate_wire_plot.py random_walk/random_walk.cfg random_walk/random_walk.db
$mypython ../scripts/generate_radii_plot.py random_walk/random_walk.cfg random_walk/random_walk.db
