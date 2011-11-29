TEMPLATE      = lib
CONFIG       += qt plugin warn_off

INCLUDEPATH  += ../../../v3d_main/basic_c_fun

HEADERS      +=../../../v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS      += neuron_feature.h

HEADERS      += 3D.h
HEADERS      += Feature.h
HEADERS      += pca.h
HEADERS      += Segment.h
HEADERS      += Func.h
HEADERS      += Vector.h
HEADERS      += Limit.h
HEADERS      += Neuron.h
HEADERS      += Nfmain.h
HEADERS      += Random.h

SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
SOURCES      += neuron_feature.cpp
SOURCES      += Feature.cpp
SOURCES      += Neuron.cpp
SOURCES      += pca.c
SOURCES      += Segment.cpp
SOURCES      += Func.cpp

SOURCES      += Vector.cpp

SOURCES      += Nfmain.cpp
SOURCES      += Random.cpp
SOURCES      += 3D.cpp

TARGET        = $$qtLibraryTarget(neuron_feature)
DESTDIR       = ../../../v3d/plugins/neuron_utilities/neuron_feature
