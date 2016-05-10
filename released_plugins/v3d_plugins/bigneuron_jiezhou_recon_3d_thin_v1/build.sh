#!/bin/bash
#snake
if [ ! -d ../bigneuron_zz_snake_tracing/ITK_include ]; then
	unzip ../bigneuron_zz_snake_tracing/ITK_include.zip		
	mv ITK_include ../bigneuron_zz_snake_tracing/
fi	
qmake
make $*



