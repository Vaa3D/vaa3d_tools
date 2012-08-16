
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3D_DIR = /home/brad/Desktop/code/vaa3d/v3d_external
INCLUDEPATH	+= $$VAA3D_DIR/v3d_main/basic_c_fun

# R-specific includes
INCLUDEPATH += /usr/share/R/include
INCLUDEPATH += /usr/lib/R/site-library/Rcpp/include
INCLUDEPATH += /usr/local/lib/R/site-library/RInside/include

# R-specific libs
LIBS += -L/usr/lib/R/lib -lR  -lblas -llapack
LIBS += -L/usr/lib/R/site-library/Rcpp/lib -lRcpp -Wl,-rpath,/usr/lib/R/site-library/Rcpp/lib
LIBS += -L/usr/local/lib/R/site-library/RInside/lib -lRInside -Wl,-rpath,/usr/local/lib/R/site-library/RInside/lib

HEADERS	= r_plugin.h
HEADERS += r_miscfunc.h
SOURCES	+= $$VAA3D_DIR/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += r_plugin.cpp
SOURCES += r_miscfunc.cpp

TARGET	= $$qtLibraryTarget(r_plugin)
DESTDIR	= $$VAA3D_DIR/bin/plugins/r_plugin/
