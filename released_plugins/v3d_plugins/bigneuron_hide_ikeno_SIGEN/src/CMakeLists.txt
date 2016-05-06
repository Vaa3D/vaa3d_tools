include_directories(. ../third_party)

# http://stackoverflow.com/questions/1167154/listing-header-files-in-visual-studio-c-project-generated-by-cmake

# Vaa3D plugin
add_library(sigen STATIC
  sigen/builder/builder.cpp
  sigen/builder/builder.h
  sigen/common/binary_cube.h
  sigen/common/cluster.h
  sigen/common/disjoint_set.cpp
  sigen/common/disjoint_set.h
  sigen/common/math.h
  sigen/common/neuron.cpp
  sigen/common/neuron.h
  sigen/common/noncopyable.h
  sigen/common/point.h
  sigen/common/variant.h
  sigen/common/voxel.h
  sigen/extractor/extractor.cpp
  sigen/extractor/extractor.h
  sigen/interface.cpp
  sigen/interface.h
  sigen/toolbox/toolbox.cpp
  sigen/toolbox/toolbox.h
  ../third_party/kdtree/kdtree.c
  ../third_party/kdtree/kdtree.h
)

if(BUILD_MAIN)
  include_directories(${OPENCV_INCLUDE_DIRS})

  add_library(sigen_io STATIC
    sigen/binarizer/binarizer.cpp
    sigen/loader/file_loader.cpp
    sigen/writer/swc_writer.cpp
    sigen/writer/fileutils.cpp
  )

  add_executable(main
    sigen/main.cpp
  )

  target_link_libraries(main sigen sigen_io boost_system boost_filesystem glog ${OPENCV_LIBRARIES})
endif(BUILD_MAIN)
