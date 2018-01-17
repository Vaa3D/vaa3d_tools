TEMPLATE    = lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/common_lib/include
INCLUDEPATH += ../../zhi/deep_learning/prediction
INCLUDEPATH += ../../../released_plugins/v3d_plugins/mean_shift_center

win32 {
    DEFINES += "CMAKE_WINDOWS_BUILD"
    CAFFEPATH = $$(CAFFE_PATH)
    CUDAPATH  = "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v8.0"
    USERPATH = $$(USERPROFILE)
    OTHERINCLUDEPATH = $$USERPATH\\.caffe\\dependencies\\libraries_v120_x64_py27_1.1.0\\libraries\\include
    OTHERLIBPATH = $$USERPATH\\.caffe\\dependencies\\libraries_v120_x64_py27_1.1.0\\libraries

    INCLUDEPATH += $$OTHERINCLUDEPATH
    INCLUDEPATH	+= $$CAFFEPATH\\include
    INCLUDEPATH += $$CAFFEPATH\\include\\caffe\\util
    INCLUDEPATH	+= $$CAFFEPATH\\src\\caffe
    INCLUDEPATH += $$CAFFEPATH\\build\\include
    INCLUDEPATH += $$CAFFEPATH\\scripts\\build
    INCLUDEPATH += $$CAFFEPATH\\scripts\\build\\include
    LIBS += -L$$CAFFEPATH\\build\\lib\\Release
    LIBS += -L$$CAFFEPATH\\scripts\\build\\lib\\Release
    LIBS += -lcaffe -lcaffeproto

    # cuda
    INCLUDEPATH += $$CUDAPATH\\include
    LIBS += -L$$CUDAPATH\\lib\\x64
    LIBS += -lcudart -lcublas -lcurand

    # opencv
    INCLUDEPATH += $$OTHERINCLUDEPATH\\opencv
    INCLUDEPATH += $$OTHERINCLUDEPATH\\opencv2
    INCLUDEPATH += $$OTHERINCLUDEPATH\\opencv2\\core
    LIBS += -L$$OTHERLIBPATH\\x64\\vc12\\lib
    LIBS += -lopencv_core310 -lopencv_imgproc310 -lopencv_highgui310 -lopencv_imgcodecs310 

    # other dependencies
    INCLUDEPATH += $$OTHERINCLUDEPATH\\boost-1_61  
    LIBS += -L3rdPartyLibs_win
    LIBS += -L$$OTHERLIBPATH\\lib
    LIBS += -llibprotobuf -lcaffehdf5 -lcaffehdf5_hl -llibopenblas -lglog -lgflags -llmdb -lleveldb -lntdll 
    LIBS += -lboost_system-vc120-mt-1_61 -lboost_thread-vc120-mt-1_61 -lboost_python-vc120-mt-1_61 -lboost_filesystem-vc120-mt-1_61
}

unix {
    LIBS += -L/local2/zhi/caffe/build/lib
    LIBS += -lcaffe

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
}


macx{
    LIBS += -L$$VAA3DPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat
}

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat

}


HEADERS	+= Deep_Neuron_plugin.h 
HEADERS += ../../zhi/deep_learning/prediction/classification.h
HEADERS += ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h
HEADERS += $$VAA3DPATH/basic_c_fun/basic_surf_objs.h
HEADERS += tester.h
HEADERS += DeepNeuronUI.h
HEADERS += imgOperator.h
HEADERS += DLOperator.h

SOURCES	+= Deep_Neuron_plugin.cpp 
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += ../../zhi/deep_learning/prediction/classification.cpp
SOURCES += ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += tester.cpp
SOURCES += DeepNeuronUI.cpp
SOURCES += imgOperator.cpp
SOURCES += DLOperator.cpp

FORMS += DeepNeuronForm.ui

TARGET	= $$qtLibraryTarget(Deep_Neuron)
DESTDIR	= ../../../../v3d_external/bin/plugins/Deep_Neuron/
