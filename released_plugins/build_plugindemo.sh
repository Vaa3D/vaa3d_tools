#!/bin/bash
# batch build all released V3D plugin projects
# by Hanchuan Peng
# 2010-04-20, 2009-08-14
# 
# build only 1 sepecifed sub plugin project
# sh build_plugindemo.sh -ddt
# add a new -n for mac 32bit make
#
# add a new build.sh mechanism for possible auto unzip and other needs of the special building processes. 
# by Hanchuan Peng, 2016-05-09


cd v3d_plugins
 
QMAKE_CONFIG=
MAKE_ARGS=
MYDIR=

for arg in $*; do
  #echo $arg		
  if [ $arg == "-m" ]; then
  	QMAKE_CONFIG="CONFIG+=x86_64"
  elif [ $arg == "-n" ]; then
  	QMAKE_CONFIG="CONFIG+=x86"
  elif [ ${arg:0:2} == "-d" ]; then
  	MYDIR="${arg:2}"
  else
  	MAKE_ARGS+=" $arg"
  fi
done

if [ ${#MYDIR} -gt 0 ]; then
  ALLDIRS=$MYDIR
else
  ALLDIRS=$( ls -d */ )	
fi

# CMB 01 Dec, 2010
# Need to define QMAKESPEC on Mac
# because recent Qt installs default to creating xcode project files.
# We want Makefiles for this script.
if [[ `uname` == 'Darwin' ]]; then
   QMAKE_ARGS='-spec macx-g++'
else
   QMAKE_ARGS=''
fi

for mydir in $ALLDIRS; do
  echo 
  echo $mydir
  echo ===============================  
  cd $mydir

  if [ -f build.sh ]; then
    sh build.sh $MAKE_ARGS;
  else
    #if [ -f *.pro ]; then
    for mypro in $( ls *.pro ); do
  	  qmake $QMAKE_ARGS $mypro $QMAKE_CONFIG
    	  make $MAKE_ARGS 
    done;
    #fi
  fi;

  cd ..
done  

cd ..

