#!/bin/bash
#snake
if [ ! -d ../bigneuron_zz_snake_tracing/ITK_include ]; then
	unzip ITK_include.zip		
fi	
qmake
make



