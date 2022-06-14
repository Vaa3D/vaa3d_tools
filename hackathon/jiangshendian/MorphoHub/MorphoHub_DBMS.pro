
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
QT += widgets
HEADERS	+= MorphoHub_DBMS_plugin.h \
    MainWindow/morphohub_mainwindow.h \
    MainDialog/maindialog.h \
    BasicInfo.h \
    MainDialog/sourcedatamanagement.h  \
    MainDialog/annotatormanagement.h \
    MainDialog/screenwallDialog.h \
    Service/BigScreenDispaly_func.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
SOURCES	+= MorphoHub_DBMS_plugin.cpp \
    MainWindow/morphohub_mainwindow.cpp \
    MainDialog/maindialog.cpp \
    MainDialog/sourcedatamanagement.cpp \
    BasicInfo.cpp \
    MainDialog/annotatormanagement.cpp \
    Service/BigScreenDisplay_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(MorphoHub_DBMS)
DESTDIR	= $$VAA3DPATH/bin/plugins/MorphoHub_DBMS/


