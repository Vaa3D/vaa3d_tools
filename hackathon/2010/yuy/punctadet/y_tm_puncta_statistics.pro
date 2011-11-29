#GFP Puncta Statistics
#by Yang Yu
#20090726
#updated 20100715

TEMPLATE = app
CONFIG += qt warn_off x86_64

HEADERS += ../../../v3d_main/basic_c_fun/v3d_message.h \
../../../v3d_main/basic_c_fun/basic_surf_objs.h \
../../../v3d_main/basic_c_fun/stackutil.h \
../../../v3d_main/basic_c_fun/mg_image_lib.h \
../../../v3d_main/basic_c_fun/mg_utilities.h \
../../../v3d_main/basic_c_fun/volimg_proc.h \
../../../v3d_main/basic_c_fun/img_definition.h \
../../../v3d_main/basic_c_fun/basic_landmark.h \
y_ytree.h

LIBS += -lm
LIBS += -L../../../v3d_main/common_lib/lib_mac64 -ltiff64

SOURCES += ../../../v3d_main/basic_c_fun/v3d_message.cpp \
../../../v3d_main/basic_c_fun/basic_surf_objs.cpp \
../../../v3d_main/basic_c_fun/stackutil.cpp \
../../../v3d_main/basic_c_fun/mg_utilities.cpp \
../../../v3d_main/basic_c_fun/mg_image_lib.cpp \
y_tm_puncta_statistics.cpp



