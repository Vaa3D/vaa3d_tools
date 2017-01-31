
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include
INCLUDEPATH	+= /local1/work/caffe/include
INCLUDEPATH	+= /local1/cuda-8.0/include/
INCLUDEPATH	+= /local1/work/caffe/.build_release/src

LIBS += -L/local1/work/caffe/build/lib
LIBS += -lcaffe

# cuda
INCLUDEPATH += /usr/local/cuda/include
LIBS += -L/usr/local/cuda/lib64
LIBS += -lcudart -lcublas -lcurand

# opencv
INCLUDEPATH += /local1/Downloads/opencv-2.4.12/include/opencv
LIBS += -L/local1/Downloads/opencv-2.4.12/release/lib
LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui

# other dependencies
INCLUDEPATH += /local1/Downloads/boost_1_63_0
LIBS += -L/local1/Downloads/boost_1_63_0/stage/lib
LIBS += -L/usr/lib64/atlas
LIBS += -L/local1/Downloads/cuda/lib64
LIBS += -L/local1/work/caffe/.build_release/lib
LIBS += -L/usr/local/lib
LIBS += -lglog -lgflags -lprotobuf -lboost_system -lboost_thread -llmdb -lleveldb -lstdc++ -lcudnn -lcblas -latlas

HEADERS	+= prediction_caffe_plugin.h
HEADERS	+= classification.h

SOURCES	+= prediction_caffe_plugin.cpp
SOURCES	+= classification.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(prediction_caffe)
DESTDIR	= $$VAA3DPATH/../bin/plugins/prediction_caffe/
