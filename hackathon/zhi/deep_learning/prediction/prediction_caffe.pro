
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include



macx{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

    LIBS += -L$$VAA3DPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat

    INCLUDEPATH	+= /Users/zhiz/Documents/work/caffe/include
    INCLUDEPATH	+= /Users/zhiz/Documents/work/caffe/build/include
    INCLUDEPATH	+= /Users/zhiz/Documents/work/caffe/.build_release/src
    INCLUDEPATH	+= /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/Headers/

    LIBS += -L ../caffe/lib_mac -Wl,-all_load -lcaffe
    LIBS += -L/usr/local/lib
    LIBS += -lglog -lgflags -lprotobuf -llmdb  -lcblas -lm -lhdf5 -lsz  -lz -lhdf5_hl
    LIBS += -lsnappy
    LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs
    LIBS += -lboost_system -lboost_filesystem -lboost_thread-mt

    DEFINES += USE_OPENCV CPU_ONLY
}

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat

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


}

HEADERS	+= prediction_caffe_plugin.h
HEADERS	+= classification.h
HEADERS += ../../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h


SOURCES	+= prediction_caffe_plugin.cpp
SOURCES	+= classification.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += ../../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp

SOURCES += $$VAA3DPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/stackutil.cpp
SOURCES	+= $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/readrawfile_func.cpp
SOURCES	+= ../../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp


TARGET	= $$qtLibraryTarget(prediction_caffe)
DESTDIR	= $$VAA3DPATH/../bin/plugins/prediction_caffe/
