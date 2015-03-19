
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/common_lib/include
INCLUDEPATH += C:/Users/lymhust/Desktop/OpenCVQT/include/opencv

HEADERS += FaceDetection_plugin.h

SOURCES += FaceDetection_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

LIBS += C:/Users/lymhust/Desktop/OpenCVQT/lib/libcv200.dll.a
LIBS += C:/Users/lymhust/Desktop/OpenCVQT/lib/libcxcore200.dll.a
LIBS += C:/Users/lymhust/Desktop/OpenCVQT/lib/libml200.dll.a
LIBS += C:/Users/lymhust/Desktop/OpenCVQT/lib/libcvaux200.dll.a
LIBS += C:/Users/lymhust/Desktop/OpenCVQT/lib/libhighgui200.dll.a


TARGET	= $$qtLibraryTarget(FaceDetection)
DESTDIR	= $$VAA3DPATH/bin/plugins/FaceDetection/
