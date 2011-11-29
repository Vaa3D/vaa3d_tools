
TEMPLATE      = lib
CONFIG       += plugin warning_off 
INCLUDEPATH  += ../../basic_c_fun
HEADERS     = crashTest3.h			 
				
SOURCES     = crashTest3.cpp
			
TARGET        = $$qtLibraryTarget(crashTest3)
DESTDIR       = ../../v3d/plugins/Wavelets #win32 qmake couldn't handle space in path

