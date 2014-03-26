
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#QT += opengl xml
CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
SRCNEUTUBEPATH = ../../../released_plugins/v3d_plugins/neurontracing_neutube/src_neutube

INCLUDEPATH += $$SRCNEUTUBEPATH/neurolabi/lib/libpng-1.6.7 $$SRCNEUTUBEPATH/neurolabi/lib/jansson/include
INCLUDEPATH += $$SRCNEUTUBEPATH/neurolabi/lib/fftw3/include

INCLUDEPATH += $$SRCNEUTUBEPATH/neurolabi/c/include $$SRCNEUTUBEPATH/neurolabi/gui \
    $$SRCNEUTUBEPATH/neurolabi/lib/genelib/src $$SRCNEUTUBEPATH/neurolabi/build \
    $$SRCNEUTUBEPATH/neurolabi/gui/ext $$SRCNEUTUBEPATH/neurolabi/gui/ext/QsLog\
    $$SRCNEUTUBEPATH/neurolabi/lib/xml/include/libxml2\
    $$VAA3DPATH/v3d $$VAA3DPATH/basic_c_fun $$VAA3DPATH/common_lib/include $$SRCNEUTUBEPATH/neurolabi/c

#LIBS += -L$${PWD}/$$SRCNEUTUBEPATH/neurolabi/build -lneutube -L$${PWD}/$$SRCNEUTUBEPATH/neurolabi/c/lib -lneurolabi \
#    -lxml2 -ljansson -lGLEW -framework AGL -framework OpenGL -lfftw3 -lfftw3f -ldl \
#    -framework AppKit -framework IOKit     -framework OpenGL \
#    -framework ApplicationServices -ldl -framework AppKit -framework IOKit \
#    -framework OpenGL -framework ApplicationServices -framework CoreFoundation


LIBS += -L$$SRCNEUTUBEPATH/neurolabi/lib/fftw3/lib -lfftw3f
LIBS += -L$$SRCNEUTUBEPATH/neurolabi/lib/fftw3/lib -lfftw3
LIBS += -L$$SRCNEUTUBEPATH/neurolabi/c/lib -lneurolabi
LIBS += -L$$SRCNEUTUBEPATH/neurolabi/lib/xml/lib -lxml2

DEFINES += HAVE_LIBFFTW3

HEADERS	+= neuTube_zhi_plugin.h

SOURCES	+= neuTube_zhi_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

#SOURCES +=  phc_image_lib.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/lib/genelib/src/image_lib.c

SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_lib.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_attribute.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_neighborhood.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_arrayqueue.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_objlabel.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_imatrix.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_mxutils.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_image_array.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_utilities.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_int_histogram.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_iarray.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_string.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_threshold.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_bwmorph.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_objdetect.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_voxel_graphics.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_sampling.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_u16array.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_u8array.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_farray.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_darray.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_object_3d.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_object_3d_linked_list.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_geo3d_utils.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_voxel.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_voxel_linked_list.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_pixel_array.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_utils.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_geo3d_scalar_field.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_workspace.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_local_neuroseg.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_math.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_locseg_chain.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_trace_utils.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_locseg_chain_com.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_graph.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_neuroseg.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_bitmask.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_neuropos.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_3dgeom.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_coordinate_3d.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_geo3d_point_array.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_geometry.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_cont_fun.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_perceptor.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_optimize_utils.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_geoangle_utils.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_neurofield.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_locseg_node.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_locseg_node_doubly_linked_list.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_geo3d_vector.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_locseg_chain_knot.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_unipointer_arraylist.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_unipointer_linked_list.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_math.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_relation.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_geo3d_circle.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_xz_orientation.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_swc_cell.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_error.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_fimage_lib.c


TARGET	= $$qtLibraryTarget(neuTube_zhi)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_tracing/neuTube_zhi/
