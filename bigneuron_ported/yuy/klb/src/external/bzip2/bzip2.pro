QT -= core gui

TARGET = bz2lib
TEMPLATE = lib
# Use this for static zlib rather than the default dynamic
# CONFIG += staticlib

include(bzip2.pri)