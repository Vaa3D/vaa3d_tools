QT -= core gui

TARGET = z
TEMPLATE = lib
# Use this for static zlib rather than the default dynamic
# CONFIG += staticlib

include(zlib.pri)
