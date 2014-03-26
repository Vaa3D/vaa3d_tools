#CMake file for tbar src

message ("exe: " ${BUILDEM_READY})

if (BUILDEM_READY)
  message ("Building executable: skeletonize")
  SET(GuiDir ${BUILDEM_DIR}/include/neurolabi/neulib_cpp)
  SET(NeulibDir ../../..)
  SET(NeurolabiLibDir ${BUILDEM_DIR}/lib)
else (BUILDEM_READY)
  if (NOT BUILDEM_USED)
    message ("Building executable without buildem: skeletonize")

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG_")

    SET(NeurolabiDir ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
    SET(GuiDir ${NeurolabiDir}/gui)
    SET(NeurolabiLibDir ${NeurolabiDir}/c/lib)
  endif (NOT BUILDEM_USED)
endif(BUILDEM_READY)

SET(ExternalSource 
  ../../../gui/zjsonparser.cpp
  ../../../gui/zjsonobject.cpp
  ../../../gui/zjsonvalue.cpp
  ../../../gui/c_json.cpp
  ../../../gui/zjsonarray.cpp
  ../../../gui/mylib/array.cpp
  ../../../gui/mylib/mylib.c
  ../../../gui/mylib/utilities.cpp
  ../../../gui/zstackprocessor.cpp
  ../../../gui/zstack.cxx
  ../../../gui/zlocalneuroseg.cpp
  ../../../gui/zellipse.cpp
  ../../../gui/zswctree.cpp
  ../../../gui/zobject3d.cpp
  ../../../gui/zcircle.cpp
  ../../../gui/zpoint.cpp
  ../../../gui/zdirectionaltemplate.cpp
  ../../../gui/zlocalrect.cpp
  ../../../gui/zsinglechannelstack.cpp
  ../../../gui/zspgrowparser.cpp
  ../../../gui/zvoxel.cpp
  ../../../gui/zvoxelarray.cpp
  ../../../gui/zsuperpixelmap.cpp
  ../../../gui/zsegmentmap.cpp
  ../../../gui/zsuperpixelmaparray.cpp
  ../../../gui/zsegmentmaparray.cpp
  ../../../gui/zswcforest.cpp
  ../../../gui/zstackaccessor.cpp
  ../../../gui/zswctreematcher.cpp
  ../../../gui/zswcbranch.cpp
  ../../../gui/zmatrix.cpp
  ../../../gui/zinttree.cpp
  ../../../gui/zstring.cpp
  ../../../gui/zfilelist.cpp
  ../../../gui/zrandomgenerator.cpp
  ../../../gui/zfilelock.cpp
  ../../../gui/zargumentprocessor.cpp
  ../../../gui/flyem/zsynapseannotation.cpp
  ../../../gui/flyem/zsynapseannotationarray.cpp
  ../../../gui/flyem/zsynapseannotationmetadata.cpp
  ../../../gui/flyem/zsynapseannotationanalyzer.cpp
  ../../../gui/flyem/zfileparser.cpp
  ../../../gui/flyem/zneuronnetwork.cpp
  ../../../gui/zvaa3dmarker.cpp
  ../../../gui/zvaa3dapo.cpp
  ../../../gui/zswcnode.cpp
  ../../../gui/zlogmessagereporter.cpp
  ../../../gui/zball.cpp
  ../../../gui/zswctreenode.cpp
  ../../../gui/zsquarematrix.cpp
  ../../../gui/zsymmetricmatrix.cpp
  ../../../gui/zsvggenerator.cpp
  ../../../gui/zdendrogram.cpp
  ../../../gui/zcuboid.cpp
  ../../../gui/zswcfeatureanalyzer.cpp
  ../../../gui/zswcsizefeatureanalyzer.cpp
  ../../../gui/zobject3darray.cpp
  ../../../gui/zdoublevector.cpp
  ../../../gui/zkmeansclustering.cpp
  ../../../gui/zswcshollfeatureanalyzer.cpp
  ../../../gui/zswcspatialfeatureanalyzer.cpp
  ../../../gui/swctreenode.cpp
  ../../../gui/zswcnetwork.cpp
  ../../../gui/zcloudnetwork.cpp
  ../../../gui/zweightedpoint.cpp
  ../../../gui/zweightedpointarray.cpp
  ../../../gui/zpointnetwork.cpp
  ../../../gui/zswcpath.cpp
  ../../../gui/zswctrunkanalyzer.cpp
  ../../../gui/zswcdisttrunkanalyzer.cpp
  ../../../gui/zswcbranchingtrunkanalyzer.cpp
  ../../../gui/zswctrunksizefeatureanalyzer.cpp
  ../../../gui/zpointarray.cpp
  ../../../gui/flyem/zsynapselocationmatcher.cpp
  ../../../gui/c_stack.cpp
  ../../../gui/zfiletype.cpp
  ../../../gui/flyem/zsynapselocationmetric.cpp
  ../../../gui/zstackfile.cpp
  ../../../gui/zxmldoc.cpp
  ../../../gui/zintmap.cpp
  ../../../gui/flyem/zsegmentationanalyzer.cpp
  ../../../gui/flyem/zsegmentationbundle.cpp
  ../../../gui/zstackblender.cpp
  ../../../gui/zgraph.cpp
  ../../../gui/zarray.cpp
  ../../../gui/zintpairmap.cpp
  ../../../gui/zswcsizetrunkanalyzer.cpp
  ../../../gui/zswcweighttrunkanalyzer.cpp
  ../../../gui/zdebug.cpp
  ../../../gui/tubemodel.cpp
  ../../../gui/zhdf5reader.cpp
  ../../../gui/neutubeconfig.cpp
  ../../../gui/zhdf5writer.cpp
  ../../../gui/flyem/zbcfset.cpp
  ../../../gui/zstackskeletonizer.cpp
  ../../../gui/zswclayerfeatureanalyzer.cpp
  ../../../gui/flyem/zflyemneuron.cpp
  ../../../gui/zswctypetrunkanalyzer.cpp
  ../../../gui/zobject3dscan.cpp
  ../../../gui/zswclayershollfeatureanalyzer.cpp
  ../../../gui/zswclayertrunkanalyzer.cpp
  ../../../gui/zstackgraph.cpp
  ../../../gui/zgraphcompressor.cpp
  ../../../gui/zprogressreporter.cpp
  ../../../gui/zmessagereporter.cpp
  ../../../gui/zreportable.cpp
  ../../../gui/zstackstatistics.cpp
  ../../../gui/zswcconnector.cpp
  ../../../gui/zprogressable.cpp
  ../../../gui/biocytin/zstackprojector.cpp
  ../../../gui/biocytin/swcprocessor.cpp
  ../../../gui/zresolution.cpp
  ../../../gui/zswcpositionadjuster.cpp
  ../../../gui/zswcrangeanalyzer.cpp
  ../../../gui/zellipsoid.cpp
  ../../../gui/zswcnodeselector.cpp
  ../../../gui/zswcnodezrangeselector.cpp
  ../../../gui/zswcnodecompositeselector.cpp
  ../../../gui/zswcnodeellipsoidrangeselector.cpp
  ../../../gui/flyem/zstitchgrid.cpp
  ../../../gui/flyem/zintcuboidarray.cpp
  ../../../gui/flyem/zflyemqualityanalyzer.cpp
  ../../../gui/flyem/zintcuboidcomposition.cpp
  ../../../gui/zswcglobalfeatureanalyzer.cpp
  ../../../gui/zswclocationanalyzer.cpp
  ../../../gui/biocytin/zbiocytinfilenameparser.cpp
  ../../../gui/zhistogram.cpp
  ../../../gui/misc/miscutility.cpp
  ../../../gui/flyem/zflyemneuronrange.cpp
  ../../../gui/flyem/zflyemneuronaxis.cpp
  ../../../gui/zswcgenerator.cpp
  ../../../gui/zstackdrawable.cpp
  ../../../gui/zswcnodebufferfeatureanalyzer.cpp
  ../../../gui/flyem/zflyemneuronrangecompare.cpp
  ../../../gui/swc/zswcresampler.cpp
  ../../../gui/swc/zswcmetric.cpp
  ../../../gui/swc/zswcterminalsurfacemetric.cpp
  ../../../gui/zapclustering.cpp
  ../../../gui/swc/zswcterminalanglemetric.cpp
  ../../../gui/flyem/zflyemneuronfilter.cpp
  ../../../gui/flyem/zflyemneuronfilterfactory.cpp
  ../../../gui/swc/zswcdeepanglemetric.cpp
  ../../../gui/zswcsubtreefeatureanalyzer.cpp
  ../../../gui/swc/zswcsubtreeanalyzer.cpp
  ../../../gui/zswctreenodearray.cpp
)
message(${GuiDir})

if (BUILDEM_READY)
  EXECUTE_PROCESS(COMMAND mkdir -p ${GuiDir}/flyem)
  EXECUTE_PROCESS(COMMAND mkdir -p ${GuiDir}/mylib)

  foreach (file ${ExternalSource})
    ADD_CUSTOM_COMMAND(OUTPUT ${file} COMMAND touch ${file})
  endforeach(file)

  LINK_DIRECTORIES (${BUILDEM_DIR}/lib)
  INCLUDE_DIRECTORIES(${NeulibDir}/c ${NeulibDir}/c/include 
    ${NeulibDir}/lib/genelib/src ${NeulibDir}/gui /usr/include/libxml2)

  ADD_EXECUTABLE (skeletonize skeletonize.cpp ${ExternalSource})

  SET(jsonLib jansson)
  SET(neuLib neurolabi_jansson)

  TARGET_LINK_LIBRARIES(skeletonize ${neuLib} ${jsonLib})

  MESSAGE(STATUS "Library path " ${CMAKE_LIBRARY_PATH} ${CMAKE_INCLUDE_PATH})
  MESSAGE(STATUS "jsonLib " ${jsonLib})
  MESSAGE(STATUS "neuLib " ${neuLib})
else (BUILDEM_READY)
  if (NOT BUILDEM_USED)
    LINK_DIRECTORIES (${NeurolabiLibDir})
    INCLUDE_DIRECTORIES(${NeurolabiDir}/c ${NeurolabiDir}/c/include
      ${NeurolabiDir}/lib/genelib/src ${GuiDir})

    ADD_EXECUTABLE (skeletonize skeletonize.cpp ${ExternalSource})

    FIND_LIBRARY(jsonLib jansson)
    SET(neuLib neurolabi_debug)

    TARGET_LINK_LIBRARIES(skeletonize ${neuLib} ${jsonLib})

    MESSAGE(STATUS "Library path " ${CMAKE_LIBRARY_PATH} ${CMAKE_INCLUDE_PATH})
    MESSAGE(STATUS "jsonLib " ${jsonLib})
    MESSAGE(STATUS "neuLib " ${neuLib})
  endif(NOT BUILDEM_USED)
endif(BUILDEM_READY)



