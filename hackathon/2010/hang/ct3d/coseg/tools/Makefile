all:
	cd create_view && make
	cd coseg && make
	cd compare && make
	cd ../AT3D/ && qmake && make
install:
	cd create_view && make install
	cd coseg && make install
	cd compare && make install
	cp ../AT3D/AT3D bin/CAT3D
clean:
	cd create_view && make clean
	cd coseg && make clean
	cd compare && make clean
	cd ../AT3D/ && qmake && make clean
