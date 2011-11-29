
TEMPLATE      = app
CONFIG       += qt
macx: CONFIG	     -= app_bundle
#CONFIG       += x86_64  #this cannot be added as there will be 32bit compatibility issue. Should add on command line. by PHC, 101223
#QT          -= gui # Only the core module is used

V3DMAINDIR = ../../v3d_main
INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun

HEADERS      += $$V3DMAINDIR/basic_c_fun/v3d_message.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.h

SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.cpp
SOURCES      += main_annoresultanalysis.cpp


