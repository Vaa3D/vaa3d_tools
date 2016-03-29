# Vaa3D-SIGEN

## Requirements for Vaa3D Plugin

* [CMake](https://cmake.org/) >= 2.8
* [Boost](http://www.boost.org/)
* [Vaa3D/v3d_external](https://github.com/Vaa3D/v3d_external)

## Requirements for Runner

* [CMake](https://cmake.org/) >= 2.8
* [google-glog](https://github.com/google/glog)
* [OpenCV](http://opencv.org/)
* [Boost](http://www.boost.org/)

## How to Build Plugin

```
$ cd vaa3d
$ nano SIGEN.pro # edit `VAA3DPATH` variable
$ qmake
$ make
```

`libSIGEN.*` will be located in `SIGEN_ROOT/vaa3d/build`. Please put `libSIGEN.*` in `vaa3d/plugins/neuron_tracing/SIGEN` directory.

## How to Build Standalone Runner

```
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_MAIN=ON ..
$ make
```

If you run `cmake .` in project top directory, `CMakeFiles`, `CMakeCache.txt`, `cmake_install.cmake`, `Makefile` will be located in undesirable directories.

## How to Test

```
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_MAIN=ON ..
$ make
$ make test
```

If you run `cmake .` in project top directory, `CMakeFiles`, `CMakeCache.txt`, `cmake_install.cmake`, `Makefile` will be located in undesirable directories.

## Setting

* Interpolation (VT, DT)
* Clipping (level)
* Smoothing (level)

## License

This software is released under the MIT License.

Copyright (c) 2016 Sho Iizuka

## References

* Yamasaki, T., Isokawa, T., Matsui, N., Ikeno, H. and Kanzaki, R.: Reconstruction and simulation for three-dimensional morphological structure of insect neurons, Neurocomputing, Vol.69, pp.1043-1047 (2006). \[[link](http://dx.doi.org/10.1016/j.neucom.2005.12.042)\]
