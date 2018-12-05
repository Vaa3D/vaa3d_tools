
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/braincenter10/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= detect_type_plugin.h \
    main.h
SOURCES	+= detect_type_plugin.cpp \
    main.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(detect_type)
DESTDIR	= $$VAA3DPATH/bin/plugins/detect_type/
