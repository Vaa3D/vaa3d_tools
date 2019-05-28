#!/bin/bash

# uncomment next line for interactive checking of generated output
PYTHON="ipython2 --pylab -i"
# non-interactive shell. Check results afterwards
PYTHON="python2.7"

# Update environment demo
# One process grows straight and "drops a cue" / "secretes a cue" at \
# some point. Another growing process senses this cue when secreted and \
# turns toward this cue
time PYTHONPATH=update_environment/:$PYTHONPATH python ../Admin.py 1 update_environment/update_env.cfg 
python ../scripts/generate_SWC_data.py update_environment/update_env.cfg update_environment/update.db
# python ../scripts/helper_generate_movie.py update_environment/update_env.cfg update_environment/update.db
$PYTHON ../scripts/generate_wire_plot.py update_environment/update_env.cfg update_environment/update.db
$PYTHON ../scripts/generate_radii_plot.py update_environment/update_env.cfg update_environment/update.db
