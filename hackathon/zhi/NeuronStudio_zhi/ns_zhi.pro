
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/v3d $$VAA3DPATH/basic_c_fun $$VAA3DPATH/common_lib/include $$SRCNEUTUBEPATH/neurolabi/c \

HEADERS	+= ns_zhi_plugin.h
SOURCES	+= ns_zhi_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

SOURCES +=  image/nsimage.c
SOURCES +=  image/nspixels.c
SOURCES +=  image/nspixels-blur.c
SOURCES +=  image/nsio-writejpeg.c
SOURCES +=  image/nspixels-brightnesscontrast.c
SOURCES +=  image/nsio-writeraw.c
SOURCES +=  image/nsio-writetiff.c
SOURCES +=  image/tiff.c
SOURCES +=  image/nspixels-convert.c
SOURCES +=  image/nspixels-convert-imp.c
SOURCES +=  image/nspixels-copy.c
SOURCES +=  image/nspixels-dynamicrange.c
SOURCES +=  image/nspixels-flip.c
SOURCES +=  image/nspixels-gammacorrect.c
SOURCES +=  image/nspixels-getpixel.c
SOURCES +=  image/nspixels-mpfilter.c
SOURCES +=  image/raw.c
SOURCES +=  image/nspixels-subsample.c
SOURCES +=  image/nspixels-setpixel.c
SOURCES +=  image/nspixels-resize.c
SOURCES +=  image/nspixels-orthoproject.c
SOURCES +=  image/nspixels-noise.c

SOURCES +=  std/nsdebug.c
SOURCES +=  std/nsprint.c
SOURCES +=  std/nsutil.c
SOURCES +=  std/nsbytearray.c
SOURCES +=  std/nsmemory.c
SOURCES +=  std/nsvalue.c
SOURCES +=  std/nsascii.c
SOURCES +=  std/nsstring.c
SOURCES +=  std/nscookie.c
SOURCES +=  std/nserror.c
SOURCES +=  std/nsfile.c
SOURCES +=  std/nshashtable.c
SOURCES +=  std/nsprogress.c
SOURCES +=  std/nsprocdb.c
SOURCES +=  std/nsclosure.c
SOURCES +=  std/nsmath.c
SOURCES +=  std/nslist.c
SOURCES +=  std/nslog.c
SOURCES +=  std/nsmutex.c
SOURCES +=  std/nsprimes.c
SOURCES +=  std/nsthread.c

SOURCES += math/nsconv.c
SOURCES += math/nsrandom.c
SOURCES += math/nsvector.c
SOURCES += math/nsline.c

SOURCES +=  ext/subsample.c
SOURCES +=  ext/filtermgr.c


TARGET	= $$qtLibraryTarget(ns_zhi)
DESTDIR	= ../../../../v3d_external/bin/plugins/ns_zhi/
