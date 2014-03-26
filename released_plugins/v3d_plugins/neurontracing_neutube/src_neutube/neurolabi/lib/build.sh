#!/bin/bash

libdir=`pwd`

if [ ! -f fftw3/lib/libfftw3.a ]
then
  if [ ! -d fftw3 ]
  then
    mkdir fftw3
  fi

  echo 'Building libfftw3 ...'
  tar -xvf fftw-3.3.2.tar.gz
  cd fftw-3.3.2
  ./configure --enable-shared=no --prefix=${libdir}/fftw3 
  make
  make install

  ./configure --enable-shared=no --enable-float --prefix=${libdir}/fftw3 
  make
  make install
  cd ..
fi

if [ ! -f jansson/lib/libjansson.a ]
then
  echo 'Building libjansson ...' 
  if [ ! -d jansson ]
  then
    mkdir jansson
  fi
  tar -xvf jansson-2.5.tar.gz
  cd jansson-2.5
  ./configure --enable-shared=no --prefix=${libdir}/jansson
  make
  make install
  cd ..
fi


if [ ! -f xml/lib/libxml2.a ]
then
  echo 'Building libxml ...'
  if [ ! -d xml ]
  then
    mkdir xml
  fi
  tar -xvf libxml2-2.9.1.tar.gz
  cd libxml2-2.9.1
  ./configure --without-iconv --without-zlib --enable-shared=no --prefix=${libdir}/xml
  make
  make install
  cd ..
fi

if [ ! -f png/lib/libpng.a ]
then
  echo 'Building libpng ...'
  if [ ! -d png ]
  then
    mkdir png
  fi
  tar -xvf libpng-1.6.7.tar.gz
  cd libpng-1.6.7
  ./configure --enable-shared=no --prefix=${libdir}/png
  make
  make install
  cd ..
fi

if [ ! -f hdf5/lib/libhdf5.a ]
then
  echo 'Building libpng ...'
  if [ ! -d hdf5 ]
  then
    mkdir hdf5
  fi
  tar -xvf hdf5-1.8.12.tar.gz
  cd hdf5-1.8.12
  ./configure --enable-shared=no --prefix=${libdir}/hdf5
  make
  make install
  cd ..
fi

