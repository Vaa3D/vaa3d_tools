#include "pVWrap.h"

/* this will be a general wrapper for remote control of a microscope using high-level commands
the object will have an attribute that states which microscope device we're controlling
and that attribute will determine what code is ran for each high-level command

this is slightly limiting, because the high-level commands will require the same inputs/outputs regardless of
device state

how do I deal with callbacks and returned information from the server?  I think I want calling the method to 
complete the whole process- returning information to the high-level caller. 
that means:
1. this class will have to be initialized with a functioning TCP/IP connection
2. this class will have a method that listens to the port and parses the response
3. this class could also include a buffer for image data, providing a pointer to it for the high-level caller.
this could work via filesystem also, and the high-level caller would never know the difference.

