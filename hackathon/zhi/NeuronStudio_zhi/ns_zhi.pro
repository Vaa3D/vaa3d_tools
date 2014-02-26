
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/v3d $$VAA3DPATH/basic_c_fun $$VAA3DPATH/common_lib/include $$SRCNEUTUBEPATH/neurolabi/c \

HEADERS	+= ns_zhi_plugin.h
SOURCES	+= ns_zhi_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

SOURCES +=  image/nsimage.c
SOURCES += ./std/nsdebug.c
SOURCES += ./std/nsprint.c
SOURCES +=  std/nsutil.c
SOURCES +=  std/nsbytearray.c
SOURCES +=  std/nsmemory.c
SOURCES +=  std/nsvalue.c
SOURCES +=  std/nsascii.c
SOURCES +=  std/nsstring.c
SOURCES +=  std/nscookie.c
SOURCES +=  std/nserror.c

SOURCES +=  image/nspixels.c


TARGET	= $$qtLibraryTarget(ns_zhi)
DESTDIR	= ../../../../v3d_external/bin/plugins/ns_zhi/
