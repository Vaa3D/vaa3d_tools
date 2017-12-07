
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include
INCLUDEPATH	+= ../../../../../../zhi/caffe/include
INCLUDEPATH	+= ../../../../../../cuda/cuda-8.0/include/
INCLUDEPATH	+= ../../../../../../zhi/caffe/.build_release/src
INCLUDEPATH     += ../../zhi/deep_learning/prediction
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/terastitcher/include
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/mean_shift_center


LIBS += -L/local2/zhi/caffe/build/lib
LIBS += -lcaffe

macx{
    LIBS += -L$$VAA3DPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat
}

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat

}

# cuda
INCLUDEPATH += /usr/local/cuda/include
LIBS += -L/usr/local/cuda/lib64
LIBS += -lcudart -lcublas -lcurand

# opencv
INCLUDEPATH += /local2/zhi/opencv-2.4.12/include/opencv
LIBS += -L/local2/zhi/opencv-2.4.12/release/lib
LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui

# other dependencies
INCLUDEPATH += /local2/zhi/boost_1_63_0
LIBS += -L/local2/zhi/boost_1_63_0/stage/lib
LIBS += -L/usr/lib64/atlas
LIBS += -L/local2/cuda/cuda-8.0/lib64
LIBS += -L/local2/zhi/caffe/.build_release/lib
LIBS += -L/usr/local/lib
LIBS += -lglog -lgflags -lprotobuf -lboost_system -lboost_thread -llmdb -lleveldb -lstdc++ -lcblas -latlas

HEADERS	+= Neurite_Instructor_plugin.h \
    neuriteinstructorui.h \
    funcs.h
HEADERS += ../../zhi/deep_learning/prediction/classification.h
HEADERS += ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h
HEADERS += $$VAA3DPATH/basic_c_fun/basic_surf_objs.h

SOURCES	+= Neurite_Instructor_plugin.cpp \
    neuriteinstructorui.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += ../../zhi/deep_learning/prediction/classification.cpp
SOURCES += ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += funcs.cpp

TARGET	= $$qtLibraryTarget(Neurite_Instructor)
DESTDIR	= ../../../../v3d_external/bin/plugins/Neurite_Instructor/

FORMS += \
    Neurite_Instructor.ui
