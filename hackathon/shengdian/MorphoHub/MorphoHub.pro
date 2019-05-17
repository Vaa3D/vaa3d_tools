
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/penglab/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= MorphoHub_plugin.h \
    database/morphohub_database.h \
    MainWindow/morphohub_mainwindow.h \
    DBMS/dbms_basic.h \
    DBMS/dbmsdialog.h
SOURCES	+= MorphoHub_plugin.cpp \
    database/morphohub_database.cpp \
    MainWindow/morphohub_mainwindow.cpp \
    DBMS/dbmsdialog.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(MorphoHub)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/MorphoHub/
