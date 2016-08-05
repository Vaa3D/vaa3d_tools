#!/bin/bash
if [ ! -d eigen3.2.2 ]; then
	unzip eigen3.2.2	
fi	
qmake
make

