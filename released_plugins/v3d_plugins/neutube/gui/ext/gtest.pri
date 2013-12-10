INCLUDEPATH += $$PWD/gtest/include $$PWD/gtest

DEFINES += _USE_GTEST_

SOURCES += $$PWD/gtest/src/gtest-all.cc

unix {
LIBS += -lpthread
}
