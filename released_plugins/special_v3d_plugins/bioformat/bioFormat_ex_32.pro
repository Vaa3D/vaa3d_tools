
TEMPLATE      = lib
CONFIG       += plugin warn_off
INCLUDEPATH  += /System/Library/Frameworks/JavaVM.framework/Headers \
                ../../../v3d_main/basic_c_fun \
                dependent/include
LIBS          = -framework JavaVM \
		-L/usr/local/lib/ -lboost_thread \
		dependent/maclib/i386/libbfcpp.dylib \
		dependent/maclib/i386/libjace.dylib

HEADERS       = bioFormat_ex.h
SOURCES       = bioFormat_ex.cpp

TARGET        = $$qtLibraryTarget(readImage_bioformat)
QMAKE_POST_LINK = cp dependent/maclib/i386/* ../../v3d;

DESTDIR       = ../../v3d/plugins/ReadImage_bioformat

