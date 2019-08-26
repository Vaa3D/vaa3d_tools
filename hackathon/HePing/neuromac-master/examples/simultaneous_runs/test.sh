#! /bin/bash

#exec 1> log.txt

PYTHONPATH=.:$PYTHONPATH python ~/work/OIST/git/neuromac/Admin.py 16 demo_distant_attraction.cfg  &
PYTHONPATH=.:$PYTHONPATH python ~/work/OIST/git/neuromac/Admin.py  8 to_pia.cfg  &
