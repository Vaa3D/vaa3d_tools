#!/bin/bash

if [ ! -n `which uname` ]
then
  echo "uname is missing. Abort."
  exit 1
fi

cd $neutube_dir

os=`uname`
if [ $os = "Darwin" ]
then
  if [ $# -eq 0 ]
  then
    neutube_dir=/Users/zhaot/Work/neutube/neurolabi/neuTube/neuTube.app/Contents
  else
    neutube_dir=$1/neuTube.app/Contents
  fi

  cd $neutube_dir
  #qt_framework_dir=/Users/zhaot/QtSDK/Desktop/Qt/4.8.1/gcc/lib
  qt_framework_dir=/Library/FrameWorks
  mkdir Frameworks
  mkdir lib
  all_frameworks=(QtXml QtCore QtGui QtOpenGL)
  for framework in ${all_frameworks[@]}
  do
    cp -r $qt_framework_dir/${framework}.framework FrameWorks/ 
    rm -rf FrameWorks/${framework}.framework/Versions/Current
    rm -rf FrameWorks/${framework}.framework/Headers
    rm -rf FrameWorks/${framework}.framework/${framework}_debug
    rm -rf FrameWorks/${framework}.framework/${framework}
    rm -rf FrameWorks/${framework}.framework/Versions/4/${framework}_debug
    install_name_tool -change ${qt_framework_dir}/${framework}.framework/Versions/4/${framework} @executable_path/../Frameworks/${framework}.framework/Versions/4/${framework} MacOS/neuTube
    install_name_tool -id @executable_path/../Frameworks/${framework}.framework/Versions/4/${framework} Frameworks/${framework}.framework/Versions/4/${framework}
    for fw2 in ${all_frameworks[@]}
    do
      install_name_tool -change ${qt_framework_dir}/${fw2}.framework/Versions/4/${fw2} @executable_path/../Frameworks/${fw2}.framework/Versions/4/${fw2} Frameworks/${framework}.framework/Versions/4/$framework
    done
  done

  all_lib=(libopencv_core.2.4.dylib libopencv_ml.2.4.dylib)
  #all_lib=(libjansson.4.dylib libpng15.15.dylib libhdf5.7.3.0.dylib libopencv_core.2.4.dylib libopencv_ml.2.4.dylib)

  #cp ../shell/assert_succ.sh ../app/trace_package

  #cp ../../../c/lib/libneurolabi.dylib lib
  #install_name_tool -id @executable_path/../lib/libneurolabi.dylib lib/libneurolabi.dylib

  for lib in ${all_lib[@]}
  do
    cp /usr/local/lib/$lib lib/
    chmod a+w lib/$lib
    install_name_tool -id @executable_path/../lib/$lib lib/$lib
    install_name_tool -change /usr/local/lib/$lib @executable_path/../lib/$lib MacOS/neuTube
    #install_name_tool -change /usr/local/lib/$lib @executable_path/../lib/$lib lib/libneurolabi.dylib
  done

  #special handling
  #install_name_tool -change libhdf5.7.3.0.dylib @executable_path/../lib/libhdf5.7.3.0.dylib MacOS/neuTube

  all_lib2=(libopencv_core.2.4.dylib libopencv_ml.2.4.dylib)
  for lib in ${all_lib2[@]}
  do
    install_name_tool -change lib/$lib @executable_path/../lib/$lib MacOS/neuTube
  done
  install_name_tool -change lib/libopencv_core.2.4.dylib @executable_path/../lib/libopencv_core.2.4.dylib lib/libopencv_ml.2.4.dylib 

  #cp /usr/lib/libGLEW.1.9.0.dylib lib/libGLEW.1.9.0.dylib
  #chmod a+w lib/libGLEW.1.9.0.dylib
  #install_name_tool -id @executable_path/../lib/libGLEW.1.9.0.dylib lib/libGLEW.1.9.0.dylib
  #install_name_tool -change /usr/lib/libGLEW.1.9.0.dylib @executable_path/../lib/libGLEW.1.9.0.dylib MacOS/neuTube

  #To avoid 'Class Q* is implemented in both ...' error
  cp ../../../gui/qt.conf Resources
elif [ $os = "Linux" ]
then
  if [ $# -lt 2 ]
  then
    echo "deploy_neutube.sh <input_dir> <output_dir>"
    exit 1
  fi

  extlib=(libfftw3 libfftw3f libxml2 libpng15 libneurolabi libjansson libc libGLEW libQtNetwork libQtSvg libQtXml libQtOpenGL libQtGui libQtCore libQtDBus)
  input_dir=$1
  output_dir=$2
  if [ ! -d $output_dir ]
  then
    echo "Creating $output_dir"
    mkdir $output_dir
  fi
  cp $input_dir/neuTube $output_dir/neuTube_bin
  touch $output_dir/run
  chmod a+x  $output_dir/run
  printf '#!/bin/bash\nexport LD_LIBRARY_PATH=.\n./neuTube_bin' > $output_dir/run
  mv $output_dir/run $output_dir/neuTube
  cp -r $input_dir/fonts $output_dir
  cp -r $input_dir/shader $output_dir
  cp $input_dir/config.xml $output_dir
  for lib in ${extlib[@]}
  do
    libpath=`ldd $input_dir/neuTube | grep $lib.so | sed 's/.*=>\s\(.*\)\s(.*/\1/'`
    if [ -n "${libpath##+([[:space:]])}" ]
    then
      echo $libpath
      cp $libpath $output_dir
      if [ $lib = libfftw3 ]
      then
        fftwlibpath=$libpath
      fi
    fi
  done

  #Addtional lib for better portability
  if [ -n $fftwlibpath ]
  then
    fftwdeplib=(libm)
    for lib in ${fftwdeplib[@]}
    do
      libpath=`ldd $fftwlibpath | grep $lib.so | sed 's/.*=>\s\(.*\)\s(.*/\1/'`
      if [ -n "${libpath##+([[:space:]])}" ]
      then
        echo $libpath
        cp $libpath $output_dir
      fi
    done
  fi

  if [ -f $output_dir/neuTube_log.txt ]
  then
    rm -f $output_dir/neuTube_log.txt
  fi

  echo "neuTube deployed in $output_dir"
fi
