#  local_registration
#  by ZHD
#  20200516

win32{
#Setting the basic running environment of the program，The program is compiled based on C + + 11 compiler, 
#using OpenMP for multithreading. The output executable file name is global_ registration
TARGET = local_registration
TEMPLATE	= app
CONFIG += c++11
CONFIG += console
CONFIG	+= qt 
QMAKE_CXXFLAGS+=/openmp

#Set the basic path of the program，source code placed in the SRC directory ，V3DMAINPATH is the path of v3d_main in vaa3d installed by the user，
#QTPATH is the QT path installed by the user when vaa3d is installed (Note: opencv path is based on the user's installation path, so no special settings are made here）

CODEPATH  =./src
V3DMAINPATH   = D:/Allen/Vaa3d/vaa3d_tools/v3d_main
QTPATH   = D:/Allen/Vaa3d/3rdparty/qt-4.8.6

#Set the additional inclusion directory required for the program to run

INCLUDEPATH  += $$QTPATH/mkspecs/win32-msvc2013
INCLUDEPATH  += D:/opencv3.1/opencv/build/include
INCLUDEPATH  += $$QTPATH/include/QtGui
INCLUDEPATH  += $$QTPATH/include/QtCore
INCLUDEPATH  += $$QTPATH/include
INCLUDEPATH  += $$V3DMAINPATH/common_lib/include
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH  += $$V3DMAINPATH/jba/newmat11
INCLUDEPATH  += $$V3DMAINPATH/jba/c++

#Set the library path required for the program to run, and - L is followed by the library required for the program to run

Release:LIBS  += -L./ -lnewmat11
Release:LIBS  += -LD:/Allen/Vaa3d/3rdparty/qt-4.8.6/lib -lQtCore4 -lQtGui4
Release:LIBS  += -LD:/opencv3.1/opencv/build/x64/vc12/lib -lopencv_world310
Release:LIBS  += -LD:/Allen/Vaa3d/vaa3d_tools/v3d_main/common_lib/winlib64 -llibtiff 

}
unix{

#Setting the basic running environment of the program，The program is compiled based on g++ compiler, 
#using OpenMP for multithreading. The output executable file name is global_ registration

TARGET = local_registration
TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
QMAKE_CXXFLAGS += -fopenmp

#Set the basic path of the program，source code placed in the SRC directory ，V3DMAINPATH is the path of v3d_main in vaa3d installed by the user，
#QTPATH is the QT path installed by the user when vaa3d is installed (Note: opencv path is based on the user's installation path, so no special settings are made here)

CODEPATH  =./src
V3DMAINPATH   = /home/brain_code/Vaa3d/vaa3d_tools/v3d_main
QTPATH   = /usr/local/Qt-4.7.3
Opencv   = /home/opencv_build

#Set the additional inclusion directory required for the program to run

INCLUDEPATH  += $$V3DMAINPATH/common_lib/include
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH  += $$V3DMAINPATH/jba/newmat11
INCLUDEPATH  += $$V3DMAINPATH/jba/c++
INCLUDEPATH  += $$QTPATH/include
INCLUDEPATH  += $$QTPATH/include/QtCore
INCLUDEPATH  += $$QTPATH/include/QtGui
INCLUDEPATH  += /usr/local/include/opencv
INCLUDEPATH  += /usr/local/include

#Set the path of the library required for the program run, - L is followed by the name of the library required for the program run

LIBS += -fopenmp
LIBS  += -L$$V3DMAINPATH/common_lib/tiff4/lib_linux/ -ltiff 
LIBS  += -L./ -lnewmat11
LIBS  += -L$$QTPATH/lib/ -lopencv_imgproc -lopencv_core
LIBS  += -L/usr/local/lib
LIBS  += -L/usr/local/lib/
LIBS  += -lQtCore -lQtGui
}

#Place all the header files needed to run the program here
#now we start to build headers files


HEADERS      += $$V3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS	     += $$CODEPATH/stackutil.h
HEADERS      += $$V3DMAINPATH/basic_c_fun/mg_image_lib.h
HEADERS      += $$V3DMAINPATH/basic_c_fun/mg_utilities.h
HEADERS      += $$CODEPATH/q_interpolate.h
HEADERS      += $$CODEPATH/q_bspline.h
HEADERS      += $$CODEPATH/config.h
HEADERS      += $$CODEPATH/getopt.h
HEADERS      += $$CODEPATH/landmarker_deal.h
HEADERS      += $$CODEPATH/load_data.h
HEADERS      += $$CODEPATH/preprocessing.h
HEADERS      += $$CODEPATH/q_derivatives3D.h
HEADERS      += $$CODEPATH/q_EigenVectors3D.h
HEADERS      += $$CODEPATH/q_imgaussian3D.h
HEADERS      += $$CODEPATH/q_morphology.h
HEADERS      += $$CODEPATH/q_dfblcokinterp.h
HEADERS      += $$CODEPATH/T_bsplines_registration.h
HEADERS      += $$CODEPATH/until.h
HEADERS      += $$CODEPATH/mBrainAligner.h
HEADERS      += $$CODEPATH/calHogFeature.h
HEADERS      += $$V3DMAINPATH/jba/newmat11/newmat.h
HEADERS      += $$V3DMAINPATH/jba/newmat11/newmatio.h

#Place all the source files needed for the program to run here
#now we start to build sources files

SOURCES      += $$CODEPATH/q_interpolate.cpp
SOURCES      += $$CODEPATH/q_bspline.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES      += $$CODEPATH/stackutil.cpp
SOURCES      += $$V3DMAINPATH//basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES      += $$CODEPATH/getopt.c
SOURCES      += $$CODEPATH/jba_mainfunc.cpp
SOURCES      += $$CODEPATH/jba_match_landmarks.cpp
SOURCES      += $$CODEPATH/remove_nonaffine_points.cpp
SOURCES      += $$CODEPATH/q_derivatives3D.cpp
SOURCES      += $$CODEPATH/q_imgaussian3D.cpp
SOURCES      += $$CODEPATH/q_EigenVectors3D.cpp
SOURCES      += $$CODEPATH/mBrainAligner.cpp
SOURCES      += $$CODEPATH/q_imgwarp_tps_quicksmallmemory.cpp
SOURCES      += $$CODEPATH/q_imresize.cpp
SOURCES      += $$CODEPATH/q_morphology.cpp
SOURCES      += $$CODEPATH/calHogFeature.cpp
SOURCES      += $$CODEPATH/q_dfblcokinterp.cpp
SOURCES      += $$CODEPATH/landmarker_deal.cpp
SOURCES      += $$CODEPATH/load_data.cpp
SOURCES      += $$CODEPATH/preprocessing.cpp
SOURCES      += $$CODEPATH/T_bsplines_registration.cpp
SOURCES      += $$CODEPATH/config.cpp
SOURCES      += $$CODEPATH/main_local_registration.cpp