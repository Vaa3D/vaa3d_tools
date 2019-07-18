#!/bin/bash

# uncomment next line for interactive checking of generated output
PYTHON="ipython2 --pylab -i"
# non-interactive shell. Check results afterwards
PYTHON="python2.7"

# One neurite with two branches. Both branches grow straight to the pia.
time PYTHONPATH=to_pia/:$PYTHONPATH python ../Admin.py 8 to_pia/to_pia.cfg
python ../scripts/generate_SWC_data.py to_pia/to_pia.cfg to_pia/to_pia.db
# python ../scripts/helper_generate_movie.py to_pia/to_pia.cfg to_pia/to_pia.db
$PYTHON ../scripts/generate_wire_plot.py to_pia/to_pia.cfg to_pia/to_pia.db 
$PYTHON ../scripts/generate_radii_plot.py to_pia/to_pia.cfg to_pia/to_pia.db
