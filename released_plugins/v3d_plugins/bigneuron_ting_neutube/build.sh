#!/bin/bash
#snake
if [ ! -d NeuTu ]; then
	wget https://www.dropbox.com/s/g2alfleyp6d9dfc/NeuTu.zip?dl=0
	unzip NeuTu.zip		
	cd NeuTu/neurolabi
	sh buildlib.sh
	cd ..
	cd ..
fi
qmake
make $*



