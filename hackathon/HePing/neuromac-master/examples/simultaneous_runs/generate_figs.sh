#! /bin/bash

python ../../scripts/generate_wire_plot.py demo_distant_attraction.cfg simul_distant.db &
python ../../scripts/generate_wire_plot.py to_pia.cfg simul_pia.db 
