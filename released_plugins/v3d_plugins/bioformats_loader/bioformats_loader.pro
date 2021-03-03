
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
unix {V3DMAINPATH = ../../../v3d_main
}
win32{V3DMAINPATH = ..\\..\\..\\v3d_main
}
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/v3d

HEADERS	+= bioformats_loader_plugin.h
SOURCES	+= bioformats_loader_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(bioformats_loader)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/data_IO/convert_and_load_with_Bioformats

win32 {
         QMAKE_POST_LINK += mkdir  $$V3DMAINPATH\\..\\bin\\bioformats_tools &
         QMAKE_POST_LINK += xcopy  /f bioformats_tools  $$V3DMAINPATH\\..\\bin\\bioformats_tools\\
      #  QMAKE_CLEAN += -r $$V3DMAINDIR\\..\\bin\\v3d_external\\bin\\bioformats_tools
}
else {    #  copy my version of the Bioformats Tools package into the bin directory and then delete the .svn stuff
        QMAKE_POST_LINK = cp -r bioformats_tools $$V3DMAINPATH/../../v3d_external/bin/
        QMAKE_CLEAN += -r $$V3DMAINPATH/../../v3d_external/bin/bioformats_tools
}

