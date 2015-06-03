
TEMPLATE = subdirs
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS   += -std=c++11
SUBDIRS = \
    src \
    zlib \
    bzip2

src.file = src/klb.pro

zlib.file = src/external/zlib/zlib.pro

bzip2.file = src/external/bzip2/bzip2.pro

src.depends = zlib bzip2
