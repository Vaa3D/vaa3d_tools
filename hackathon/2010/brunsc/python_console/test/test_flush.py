#!/usr/bin/python

# Test flushing of stdout before delay.
# For manual interactive testing of whether python console behaves like 
# python in a terminal.

import time

print "There should be a pause between this statement..."
time.sleep(2)
print "...and this one"
