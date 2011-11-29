

TEMPLATE	= lib
CONFIG		+= plugin warn_off
CONFIG		+= x86_64

INCLUDEPATH	  = ../../../v3d_main/basic_c_fun \
                dependent/include \
                "C:\Program Files\Java\jdk1.6.0_21\include" \
                "C:\Program Files\Java\jdk1.6.0_21\include\win32" \
                "C:\work\bioformat\boost_1_43_0"
                
LIBS          = -L./dependent/winlib/x64/ -llibboost_thread-vc90-mt-1_43 \
				-llibboost_date_time-vc90-mt-1_43 \
				C:\work\v3d_2.0\plugin_demo\bioformat_lib\winlib\x64\jace.lib \
				C:\work\v3d_2.0\plugin_demo\bioformat_lib\winlib\x64\bfcpp.lib \
				"C:\Program Files\Java\jdk1.6.0_21\lib\jvm.lib"
				
HEADERS       = bioFormat_ex.h
SOURCES       = bioFormat_ex.cpp

HEADERS		 += ../../../v3d_main/basic_c_fun/mg_utilities.h \
				../../../v3d_main/basic_c_fun/mg_image_lib.h \
				../../../v3d_main/basic_c_fun/stackutil.h
				
SOURCES      += ../../basic_c_fun/mg_utilities.cpp \
				../../basic_c_fun/mg_image_lib.cpp \
				../../basic_c_fun/stackutil.cpp
				
LIBS		 += -L../../../v3d_main/common_lib/winlib64/ -llibtiff

TARGET        = $$qtLibraryTarget(readImage_bioformat)

DESTDIR       = ../../v3d/plugins/ImageIO_bioformat

