#!/bin/bash

all_trace_program=(imbinarize bwsolid rmsobj extract_line imexpr trace_seed sort_seed drawmask trace_neuron3 reconstruct_neuron edswc)
all_lib=(libjansson.4.dylib libpng15.15.dylib)

cp ../shell/assert_succ.sh ../app/trace_package

for lib in ${all_lib[@]}
do 
  cp /usr/local/lib/$lib ../app/trace_package/lib/
  chmod a+w ../app/trace_package/lib/$lib
  install_name_tool -id @executable_path/../lib/$lib ../app/trace_package/lib/$lib 
done

for trace_program in ${all_trace_program[@]}
do
  make PROG=$trace_program
  cp bin/$trace_program ../app/trace_package/bin/
  for lib in ${all_lib[@]}
  do
    install_name_tool -change /usr/local/lib/$lib @executable_path/../lib/$lib ../app/trace_package/bin/$trace_program
  done
done

cp trace_all.sh ../app/trace_package/
