#!/usr/bin/env bash
python2 Admin.py 4 generate
python2 ./scripts/generate_SWC_data.py generate generate.db
python2 ./scripts/generate_radii_plot.py generate generate.db