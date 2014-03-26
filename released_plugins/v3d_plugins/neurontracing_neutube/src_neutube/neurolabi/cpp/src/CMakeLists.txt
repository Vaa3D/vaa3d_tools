SET(NeurolabiDir ../..)
SET(GuiDir ${NeurolabiDir}/gui)
SET(GenelibDir ${NeurolabiDir}/lib/genelib/src)
SET(NeurolabiLibDir ${NeurolabiDir}/c/lib)
SET(NeurolabiCDir ${NeurolabiDir}/c)
MESSAGE(${GuiDir})

SET (ZSWCTREE_SOURCE ${GuiDir}/zswctree.cpp 
  ${GuiDir}/zswcforest.cpp 
  ${GuiDir}/zswcbranch.cpp 
  ${GuiDir}/swctreenode.cpp 
  ${GuiDir}/zswcpath.cpp ${GuiDir}/zgraph.cpp
  ${GuiDir}/flyem/zsynapselocationmatcher.cpp 
  ${GuiDir}/zweightedpoint.cpp        
  ${GuiDir}/zweightedpointarray.cpp         
  ${GuiDir}/zrandomgenerator.cpp
  ${GuiDir}/zcuboid.cpp
  ${GuiDir}/zpoint.cpp
  )

SET (ZJSON_SOURCE
    ${GuiDir}/zjsonvalue.cpp ${GuiDir}/zjsonobject.cpp
    ${GuiDir}/zjsonarray.cpp
    ${GuiDir}/zjsonparser.cpp
    )

SET (ZSYNAPSEANNOTATION_SOURCE 
  ${GuiDir}/zinttree.cpp
  ${GuiDir}/flyem/zsynapseannotationarray.cpp 
  ${GuiDir}/flyem/zsynapseannotation.cpp
  ${GuiDir}/flyem/zsynapseannotationmetadata.cpp
  ${GuiDir}/flyem/zsynapseannotationanalyzer.cpp
  ${GuiDir}/flyem/zsynapselocationmetric.cpp
  ${GuiDir}/zvaa3dmarker.cpp  
  ${GuiDir}/zvaa3dapo.cpp
  ${GuiDir}/zfiletype.cpp
  ${ZJSON_SOURCE}
  )

SET (ZSTACK_SOURCE 
  ${GuiDir}/zstack.cxx  
  ${GuiDir}/zsinglechannelstack.cpp ${GuiDir}/c_stack.cpp
  ${GuiDir}/zstackfile.cpp
  ${GuiDir}/zfiletype.cpp
  ${GuiDir}/zstring.cpp
  ${GuiDir}/zxmldoc.cpp
  ${GuiDir}/zhdf5reader.cpp
  ${GuiDir}/mylib/array.cpp ${GuiDir}/mylib/utilities.cpp 
  ${GuiDir}/mylib/mylib.c
  ${ZJSON_SOURCE}
  )

#ADD_EXECUTABLE (flyem main.cpp ${GuiDir}/zswctree.cpp
#    ${GuiDir}/zspgrowparser.cpp ${GuiDir}/zsegmentmaparray.cpp
#    ${GuiDir}/zsuperpixelmap.cpp ${GuiDir}/zsuperpixelmaparray.cpp
#    ${GuiDir}/zvoxel.cpp ${GuiDir}/zvoxelarray.cpp ${GuiDir}/zsegmentmap.cpp
#    ${GuiDir}/zswcforest.cpp ${GuiDir}/zpoint.cpp ${GuiDir}/zswcbranch.cpp
#    ${GuiDir}/zfilelist)

#ADD_EXECUTABLE (test test.cpp ${GuiDir}/zswctree.cpp
#   ${GuiDir}/zspgrowparser.cpp ${GuiDir}/zsegmentmaparray.cpp
#   ${GuiDir}/zsuperpixelmap.cpp ${GuiDir}/zsuperpixelmaparray.cpp
#   ${GuiDir}/zvoxel.cpp ${GuiDir}/zvoxelarray.cpp ${GuiDir}/zsegmentmap.cpp
#   ${GuiDir}/zswcforest.cpp ${GuiDir}/zpoint.cpp ${GuiDir}/zswcbranch.cpp
#   ${GuiDir}/zfilelist.cpp ${GuiDir}/zargumentprocessor.cpp 
#   ${GuiDir}/zswctree.cpp
#   ${GuiDir}/zswcforest.cpp ${GuiDir}/zswcbranch.cpp 
#   ${GuiDir}/zswctreematcher.cpp ${GuiDir}/zrandomgenerator.cpp
#   ${GuiDir}/zmatrix.cpp ${GuiDir}/zinttree.cpp)

#ADD_EXECUTABLE (sort_bodymap sort_bodymap.cpp ${GuiDir}/zswctree.cpp
# ${GuiDir}/zspgrowparser.cpp ${GuiDir}/zsegmentmaparray.cpp
# ${GuiDir}/zsuperpixelmap.cpp ${GuiDir}/zsuperpixelmaparray.cpp
# ${GuiDir}/zvoxel.cpp ${GuiDir}/zvoxelarray.cpp ${GuiDir}/zsegmentmap.cpp
# ${GuiDir}/zswcforest.cpp ${GuiDir}/zpoint.cpp ${GuiDir}/zswcbranch.cpp
# ${GuiDir}/zstring.cpp ${GuiDir}/zfilelist.cpp ${GuiDir}/swctreenode.cpp 
# ${GuiDir}/zrandomgenerator.cpp ${GuiDir}/zstack.cxx ${GuiDir}/zswcpath.cpp 
# ${GuiDir}/zstackfile.cpp 
# ${GuiDir}/zsinglechannelstack.cpp ${GuiDir}/c_stack.cpp
# ${GuiDir}/zxmldoc.cpp ${GuiDir}/zfiletype.cpp ${GuiDir}/zjsonobject.cpp
# ${GuiDir}/zjsonvalue.cpp ${GuiDir}/zjsonparser.cpp
#   ${GuiDir}/zweightedpoint.cpp        
#   ${GuiDir}/zweightedpointarray.cpp ${GuiDir}/zgraph.cpp         
#   ${GuiDir}/zcuboid.cpp
# ${GuiDir}/zstring.cpp)

#ADD_EXECUTABLE (map_body map_body.cpp ${GuiDir}/zswctree.cpp
#  ${GuiDir}/zspgrowparser.cpp ${GuiDir}/zsegmentmaparray.cpp
#  ${GuiDir}/zsuperpixelmap.cpp ${GuiDir}/zsuperpixelmaparray.cpp
#  ${GuiDir}/zvoxel.cpp ${GuiDir}/zvoxelarray.cpp ${GuiDir}/zsegmentmap.cpp
#  ${GuiDir}/zswcforest.cpp ${GuiDir}/zpoint.cpp ${GuiDir}/zswcbranch.cpp
#  ${GuiDir}/zstring.cpp ${GuiDir}/zfilelist.cpp ${GuiDir}/swctreenode.cpp 
#  ${GuiDir}/zrandomgenerator.cpp ${GuiDir}/zstack.cxx ${GuiDir}/zswcpath.cpp 
#  ${GuiDir}/zstackfile.cpp 
#  ${GuiDir}/zsinglechannelstack.cpp ${GuiDir}/c_stack.cpp
#  ${GuiDir}/zxmldoc.cpp ${GuiDir}/zfiletype.cpp ${GuiDir}/zjsonobject.cpp
#  ${GuiDir}/zjsonvalue.cpp ${GuiDir}/zjsonparser.cpp
#  ${GuiDir}/zstring.cpp)

#ADD_EXECUTABLE (extract_bounded_neuron extract_bounded_neuron.cpp 
#   ${GuiDir}/zswctree.cpp
#   ${GuiDir}/zspgrowparser.cpp ${GuiDir}/zsegmentmaparray.cpp
#   ${GuiDir}/zsuperpixelmap.cpp ${GuiDir}/zsuperpixelmaparray.cpp
#   ${GuiDir}/zvoxel.cpp ${GuiDir}/zvoxelarray.cpp ${GuiDir}/zsegmentmap.cpp
#   ${GuiDir}/zswcforest.cpp ${GuiDir}/zpoint.cpp ${GuiDir}/zswcbranch.cpp
#   ${GuiDir}/zstring.cpp ${GuiDir}/zfilelist.cpp
#   ${GuiDir}/zfilelist.cpp ${GuiDir}/swctreenode.cpp 
#   ${GuiDir}/zrandomgenerator.cpp ${GuiDir}/zstack.cxx ${GuiDir}/zswcpath.cpp 
#   ${GuiDir}/zstackfile.cpp 
#   ${GuiDir}/zsinglechannelstack.cpp ${GuiDir}/c_stack.cpp
#   ${GuiDir}/zxmldoc.cpp ${GuiDir}/zfiletype.cpp ${GuiDir}/zjsonobject.cpp
#   ${GuiDir}/zjsonvalue.cpp ${GuiDir}/zjsonparser.cpp
#   ${GuiDir}/zstring.cpp)

ADD_EXECUTABLE (extract_em_neuron extract_em_neuron.cpp 
    ${GuiDir}/zswctree.cpp
    ${GuiDir}/zspgrowparser.cpp ${GuiDir}/zsegmentmaparray.cpp
    ${GuiDir}/zsuperpixelmap.cpp ${GuiDir}/zsuperpixelmaparray.cpp
    ${GuiDir}/zvoxel.cpp ${GuiDir}/zvoxelarray.cpp ${GuiDir}/zsegmentmap.cpp
    ${GuiDir}/zswcforest.cpp ${GuiDir}/zpoint.cpp ${GuiDir}/zswcbranch.cpp
    ${GuiDir}/zstring.cpp ${GuiDir}/zfilelist.cpp ${GuiDir}/swctreenode.cpp 
    ${GuiDir}/zrandomgenerator.cpp ${GuiDir}/zstack.cxx ${GuiDir}/zswcpath.cpp 
    ${GuiDir}/zstackfile.cpp 
    ${GuiDir}/zsinglechannelstack.cpp ${GuiDir}/c_stack.cpp
    ${GuiDir}/zxmldoc.cpp ${GuiDir}/zfiletype.cpp ${GuiDir}/zjsonobject.cpp
    ${GuiDir}/zjsonvalue.cpp ${GuiDir}/zjsonparser.cpp
    ${GuiDir}/zstring.cpp
    ${GuiDir}/zweightedpoint.cpp        
    ${GuiDir}/zweightedpointarray.cpp ${GuiDir}/zgraph.cpp         
    ${GuiDir}/zcuboid.cpp ${GuiDir}/zhdf5reader.cpp ${GuiDir}/mylib/array.cpp ${GuiDir}/mylib/utilities.cpp ${GuiDir}/mylib/mylib.c
    )


ADD_EXECUTABLE (crop_binary crop_binary.cpp 
  ${ZSTACK_SOURCE}
  ${GuiDir}/zsegmentmaparray.cpp
  ${GuiDir}/zsuperpixelmap.cpp ${GuiDir}/zsuperpixelmaparray.cpp
  ${GuiDir}/zsegmentmap.cpp
  ${GuiDir}/zfilelist.cpp  
  )

ADD_EXECUTABLE (extract_neuron extract_neuron.cpp ${GuiDir}/zswctree.cpp
    ${GuiDir}/zspgrowparser.cpp ${GuiDir}/zsegmentmaparray.cpp
    ${GuiDir}/zsuperpixelmap.cpp ${GuiDir}/zsuperpixelmaparray.cpp
    ${GuiDir}/zvoxel.cpp ${GuiDir}/zvoxelarray.cpp ${GuiDir}/zsegmentmap.cpp
    ${GuiDir}/zswcforest.cpp ${GuiDir}/zpoint.cpp ${GuiDir}/zswcbranch.cpp
    ${GuiDir}/zfilelist.cpp ${GuiDir}/swctreenode.cpp 
    ${GuiDir}/zrandomgenerator.cpp ${GuiDir}/zstack.cxx ${GuiDir}/zswcpath.cpp 
    ${GuiDir}/zstackfile.cpp 
    ${GuiDir}/zsinglechannelstack.cpp ${GuiDir}/c_stack.cpp
    ${GuiDir}/zxmldoc.cpp ${GuiDir}/zfiletype.cpp ${GuiDir}/zjsonobject.cpp
    ${GuiDir}/zjsonvalue.cpp ${GuiDir}/zjsonparser.cpp
    ${GuiDir}/zweightedpoint.cpp        
    ${GuiDir}/zweightedpointarray.cpp ${GuiDir}/zgraph.cpp         
    ${GuiDir}/zcuboid.cpp
    ${GuiDir}/zstring.cpp ${GuiDir}/zhdf5reader.cpp ${GuiDir}/mylib/array.cpp ${GuiDir}/mylib/utilities.cpp ${GuiDir}/mylib/mylib.c)

#ADD_EXECUTABLE (skeletonize skeletonize.cpp ${GuiDir}/zswctree.cpp
# ${GuiDir}/zspgrowparser.cpp ${GuiDir}/zsegmentmaparray.cpp
# ${GuiDir}/zsuperpixelmap.cpp ${GuiDir}/zsuperpixelmaparray.cpp
# ${GuiDir}/zvoxel.cpp ${GuiDir}/zvoxelarray.cpp ${GuiDir}/zsegmentmap.cpp
# ${GuiDir}/zswcforest.cpp ${GuiDir}/zpoint.cpp ${GuiDir}/zswcbranch.cpp
# ${GuiDir}/zfilelist.cpp ${GuiDir}/swctreenode.cpp 
# ${GuiDir}/zrandomgenerator.cpp ${GuiDir}/zstack.cxx ${GuiDir}/zswcpath.cpp 
# ${GuiDir}/zstackfile.cpp 
# ${GuiDir}/zsinglechannelstack.cpp ${GuiDir}/c_stack.cpp
# ${GuiDir}/zxmldoc.cpp ${GuiDir}/zfiletype.cpp ${GuiDir}/zjsonobject.cpp
# ${GuiDir}/zjsonvalue.cpp ${GuiDir}/zjsonparser.cpp
# ${GuiDir}/zstring.cpp)

ADD_EXECUTABLE (compare_swc compare_swc.cpp ${ZSWCTREE_SOURCE}
  ${ZSYNAPSEANNOTATION_SOURCE}
  ${GuiDir}/zswctreematcher.cpp
  ${GuiDir}/zmatrix.cpp
  ${GuiDir}/zswctrunkanalyzer.cpp
  ${GuiDir}/zfilelist.cpp
  ${GuiDir}/zstring.cpp
  )

  #ADD_EXECUTABLE (search_swc search_swc.cpp ${GuiDir}/zswctree.cpp
  #  ${GuiDir}/zspgrowparser.cpp ${GuiDir}/zsegmentmaparray.cpp
  #  ${GuiDir}/zsuperpixelmap.cpp ${GuiDir}/zsuperpixelmaparray.cpp
  #  ${GuiDir}/zvoxel.cpp ${GuiDir}/zvoxelarray.cpp ${GuiDir}/zsegmentmap.cpp
  #  ${GuiDir}/zswcforest.cpp ${GuiDir}/zpoint.cpp ${GuiDir}/zswcbranch.cpp
  #  ${GuiDir}/zswctreematcher.cpp ${GuiDir}/zmatrix.cpp ${GuiDir}/zinttree.cpp
  #  ${GuiDir}/zstring.cpp ${GuiDir}/zrandomgenerator.cpp 
  #  ${GuiDir}/zfilelist.cpp)

ADD_EXECUTABLE (generate_qsub_script generate_qsub_script.cpp 
  ${GuiDir}/zsuperpixelmap.cpp ${GuiDir}/zsuperpixelmaparray.cpp
  ${GuiDir}/zvoxel.cpp ${GuiDir}/zvoxelarray.cpp ${GuiDir}/zsegmentmap.cpp
  ${GuiDir}/zpoint.cpp
  ${GuiDir}/zmatrix.cpp ${GuiDir}/zinttree.cpp
  ${GuiDir}/zstring.cpp ${GuiDir}/zrandomgenerator.cpp
  ${GuiDir}/zstack.cxx ${GuiDir}/zswcpath.cpp 
  ${GuiDir}/zstackfile.cpp ${GuiDir}/swctreenode.cpp 
  ${GuiDir}/zswctree.cpp ${GuiDir}/zswcbranch.cpp
  ${GuiDir}/zswcforest.cpp ${GuiDir}/zfilelist.cpp
  ${GuiDir}/zsinglechannelstack.cpp ${GuiDir}/c_stack.cpp
  ${GuiDir}/zxmldoc.cpp ${GuiDir}/zfiletype.cpp ${GuiDir}/zjsonobject.cpp
  ${GuiDir}/zjsonvalue.cpp ${GuiDir}/zjsonparser.cpp
  ${GuiDir}/zweightedpoint.cpp        
  ${GuiDir}/zweightedpointarray.cpp ${GuiDir}/zgraph.cpp         
  ${GuiDir}/zcuboid.cpp
  ${GuiDir}/zstring.cpp 
  ${GuiDir}/zhdf5reader.cpp ${GuiDir}/mylib/array.cpp ${GuiDir}/mylib/utilities.cpp ${GuiDir}/mylib/mylib.c
  )

#ADD_EXECUTABLE (adjust_swc adjust_swc.cpp ${GuiDir}/zswctree.cpp
#   ${GuiDir}/zspgrowparser.cpp ${GuiDir}/zsegmentmaparray.cpp
#   ${GuiDir}/zsuperpixelmap.cpp ${GuiDir}/zsuperpixelmaparray.cpp
#   ${GuiDir}/zvoxel.cpp ${GuiDir}/zvoxelarray.cpp ${GuiDir}/zsegmentmap.cpp
#   ${GuiDir}/zswcforest.cpp ${GuiDir}/zpoint.cpp ${GuiDir}/zswcbranch.cpp
#   ${GuiDir}/zswctreematcher.cpp ${GuiDir}/zmatrix.cpp ${GuiDir}/zinttree.cpp
#   ${GuiDir}/zstring.cpp ${GuiDir}/zrandomgenerator.cpp 
#   ${GuiDir}/zfilelist.cpp ${GuiDir}/flyem/zsynapseannotationanalyzer.cpp
#   ${GuiDir}/flyem/zsynapseannotation.cpp 
#   ${GuiDir}/flyem/zsynapseannotationarray.cpp ${GuiDir}/zjsonparser.cpp
#   ${GuiDir}/zvaa3dmarker.cpp ${GuiDir}/zjsonarray.cpp 
#   ${GuiDir}/flyem/zsynapseannotationmetadata.cpp
#   ${GuiDir}/zvaa3dapo.cpp
#   ${GuiDir}/zjsonvalue.cpp ${GuiDir}/zjsonobject.cpp
#   ${GuiDir}/swctreenode.cpp ${GuiDir}/flyem/zsynapselocationmetric.cpp
#   ${GuiDir}/flyem/zsynapselocationmatcher.cpp ${GuiDir}/zstack.cxx
#   ${GuiDir}/zswcpath.cpp ${GuiDir}/zstackfile.cpp 
#   ${GuiDir}/zsinglechannelstack.cpp ${GuiDir}/c_stack.cpp
#   ${GuiDir}/zxmldoc.cpp ${GuiDir}/zfiletype.cpp)


ADD_EXECUTABLE (project_synapse project_synapse.cpp 
  ${ZSWCTREE_SOURCE} ${ZSYNAPSEANNOTATION_SOURCE}
    ${GuiDir}/zfilelist.cpp 
    ${GuiDir}/flyem/zfileparser.cpp
    ${GuiDir}/zfiletype.cpp 
    ${GuiDir}/zstring.cpp 
    )

  ADD_EXECUTABLE (sort_connection sort_connection.cpp ${GuiDir}/zswctree.cpp
     ${GuiDir}/zspgrowparser.cpp ${GuiDir}/zsegmentmaparray.cpp
     ${GuiDir}/zsuperpixelmap.cpp ${GuiDir}/zsuperpixelmaparray.cpp
     ${GuiDir}/zvoxel.cpp ${GuiDir}/zvoxelarray.cpp ${GuiDir}/zsegmentmap.cpp
     ${GuiDir}/zswcforest.cpp ${GuiDir}/zpoint.cpp ${GuiDir}/zswcbranch.cpp
     ${GuiDir}/zswctreematcher.cpp ${GuiDir}/zmatrix.cpp ${GuiDir}/zinttree.cpp
     ${GuiDir}/zstring.cpp ${GuiDir}/zrandomgenerator.cpp 
     ${GuiDir}/zfilelist.cpp ${GuiDir}/flyem/zfileparser.cpp
     ${GuiDir}/flyem/zsynapseannotationarray.cpp 
     ${GuiDir}/flyem/zsynapseannotation.cpp
     ${GuiDir}/flyem/zsynapseannotationmetadata.cpp
     ${GuiDir}/flyem/zsynapseannotationanalyzer.cpp
     ${GuiDir}/zvaa3dmarker.cpp ${GuiDir}/zjsonarray.cpp 
     ${GuiDir}/zvaa3dapo.cpp ${GuiDir}/swctreenode.cpp
     ${GuiDir}/zjsonvalue.cpp ${GuiDir}/zjsonobject.cpp 
     ${GuiDir}/zjsonparser.cpp ${GuiDir}/flyem/zsynapselocationmatcher.cpp
     ${GuiDir}/flyem/zsynapselocationmetric.cpp ${GuiDir}/zstack.cxx
     ${GuiDir}/zstackfile.cpp ${GuiDir}/zfiletype.cpp
     ${GuiDir}/zsinglechannelstack.cpp ${GuiDir}/c_stack.cpp
     ${GuiDir}/zswctrunkanalyzer.cpp
    ${GuiDir}/zweightedpoint.cpp        
    ${GuiDir}/zweightedpointarray.cpp ${GuiDir}/zgraph.cpp         
    ${GuiDir}/zcuboid.cpp
    ${GuiDir}/zxmldoc.cpp ${GuiDir}/zswcpath.cpp 
    ${GuiDir}/zhdf5reader.cpp ${GuiDir}/mylib/array.cpp ${GuiDir}/mylib/utilities.cpp ${GuiDir}/mylib/mylib.c
  )
                                                   
ADD_EXECUTABLE (interpolate interpolate.cpp ${GuiDir}/zargumentprocessor.cpp)
                                                   
ADD_DEFINITIONS (-DHAVE_CONFIG_H)                  
INCLUDE (CheckIncludeFiles)                        
CHECK_INCLUDE_FILES(pair.h HAVE_PAIR_H)            
CONFIGURE_FILE(config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
                                                   
FIND_LIBRARY(myersLib NAMES myers PATHS ${GenelibDir})
FIND_LIBRARY(neuLib NAMES neurolabi_debug PATHS ${NeurolabiCDir}/lib)
FIND_LIBRARY(xmlLib NAMES xml2 PATHS ~/local/lib /usr/local/lib /usr/lib)
FIND_LIBRARY(pngLib NAMES png PATHS ~/local/lib /usr/local/lib /usr/lib)
FIND_LIBRARY(fftwLib NAMES fftw3 PATHS ~/local/lib /usr/local/lib /usr/lib)
FIND_LIBRARY(fftwfLib NAMES fftw3f PATHS ~/local/lib /usr/local/lib /usr/lib)
FIND_LIBRARY(jsonLib NAMES jansson PATHS ~/local/lib /usr/local/lib /usr/lib)
FIND_LIBRARY(hdf5Lib NAMES hdf5 PATHS ~/local/lib /usr/local/lib /usr/lib)
INCLUDE_DIRECTORIES(~/local/include ${NeurolabiCDir}/include ${NeurolabiCDir} 
    ${GenelibDir} ${GuiDir} /usr/include/libxml2)
  #TARGET_LINK_LIBRARIES(flyem ${neuLib} ${myersLib} ${xmlLib} ${pngLib} )
  #TARGET_LINK_LIBRARIES(test ${neuLib} ${myersLib} ${xmlLib} ${pngLib} )
  #TARGET_LINK_LIBRARIES(sort_bodymap ${neuLib} ${myersLib} ${xmlLib} ${pngLib} 
  #${jsonLib} ${fftwLib} ${fftwfLib})
#TARGET_LINK_LIBRARIES(map_body ${neuLib} ${myersLib} ${xmlLib} ${pngLib} ${jsonLib} ${fftwLib} ${fftwfLib})
#TARGET_LINK_LIBRARIES(extract_bounded_neuron ${neuLib} ${myersLib} ${xmlLib} 
#  ${pngLib} ${fftwLib} ${fftwfLib} ${jsonLib})
TARGET_LINK_LIBRARIES(extract_em_neuron ${neuLib} ${myersLib} ${xmlLib} 
  ${pngLib} ${jsonLib} ${fftwLib} ${fftwfLib})
TARGET_LINK_LIBRARIES(crop_binary ${neuLib} ${myersLib} ${xmlLib} ${pngLib} 
    ${jsonLib} ${fftwLib} ${fftwfLib})
TARGET_LINK_LIBRARIES(extract_neuron ${neuLib} ${myersLib} ${xmlLib} ${pngLib} 
  ${jsonLib} ${fftwLib} ${fftwfLib})
#TARGET_LINK_LIBRARIES(skeletonize ${neuLib} ${myersLib} ${xmlLib} ${pngLib} 
#${jsonLib} ${fftwLib} ${fftwfLib})
TARGET_LINK_LIBRARIES(compare_swc ${neuLib} ${myersLib} ${xmlLib} ${pngLib} ${fftwLib} ${fftwfLib} ${jsonLib} ${hdf5Lib})
#TARGET_LINK_LIBRARIES(search_swc ${neuLib} ${myersLib} ${xmlLib} ${pngLib} )
TARGET_LINK_LIBRARIES(generate_qsub_script ${neuLib} ${myersLib} ${xmlLib} 
  ${pngLib} ${fftwLib} ${fftwfLib} ${jsonLib})
#TARGET_LINK_LIBRARIES(adjust_swc ${neuLib} ${myersLib} ${xmlLib} ${pngLib} ${jsonLib} ${fftwLib} ${fftwfLib})
TARGET_LINK_LIBRARIES(interpolate ${neuLib} ${myersLib} ${xmlLib} ${pngLib} ${jsonLib} ${fftwLib} ${fftwfLib})
TARGET_LINK_LIBRARIES(project_synapse ${neuLib} ${myersLib} ${xmlLib} ${pngLib} ${jsonLib}  ${fftwLib} ${fftwfLib})
TARGET_LINK_LIBRARIES(sort_connection ${neuLib} ${myersLib} ${xmlLib} ${pngLib} ${jsonLib} ${fftwLib} ${fftwfLib})
MESSAGE(STATUS "Library path " ${CMAKE_LIBRARY_PATH} ${CMAKE_INCLUDE_PATH})
MESSAGE(STATUS "myers lib " ${myersLib})
MESSAGE(STATUS "neurolabi " ${neuLib})
MESSAGE(STATUS "xmlLib " ${xmlLib})
MESSAGE(STATUS "hdf5Lib " ${hdf5Lib})



