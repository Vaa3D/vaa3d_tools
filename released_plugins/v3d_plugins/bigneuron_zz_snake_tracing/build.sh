#!/bin/bash
#snake
if [ ! -d ITK_include ]; then
	unzip ITK_include.zip		
fi	
qmake
make $*



