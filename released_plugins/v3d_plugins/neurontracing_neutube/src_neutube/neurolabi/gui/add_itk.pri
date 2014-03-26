exists(../lib/ITK) {
    system(echo ITK)
    ITK_INCLUDE_PATH = ../lib/ITK/include
    ITK_LIB_PATH = ../lib/ITK/lib
    INCLUDEPATH += $$ITK_INCLUDE_PATH
    LIBS += -L$$ITK_LIB_PATH
    DEFINES += _USE_ITK_
    HEADERS += itkimagedefs.h
}

exists(../lib/ITK/lib/libITKCommon-4.2.a) {
    system(echo ITK4.2)
    LIBS += -lITKCommon-4.2\
      -litkv3p_lsqr-4.2 \
      -lITKVNLInstantiation-4.2 \
      -litkvnl_algo-4.2 \
      -litkvnl-4.2 \
      -litkvcl-4.2 \
      -litksys-4.2 \
      -litkv3p_netlib-4.2 \
      -lITKIOImageBase-4.2 -lITKIOTIFF-4.2 \
      -lITKLabelMap-4.2 -litktiff-4.2 -litkzlib-4.2 \
      -litkjpeg-4.2 -lITKStatistics-4.2 -lITKNetlibSlatec-4.2 \
      -litkOptimizers-4.2
}
else:exists(../lib/ITK/lib/libITKCommon-4.3.a) {
    LIBS += -lITKCommon-4.3 \
      -litkv3p_lsqr-4.3 \
      -lITKVNLInstantiation-4.3 \
      -litkvnl_algo-4.3 \
      -litkvnl-4.3 \
      -litkvcl-4.3 \
      -litksys-4.3 \
      -litkv3p_netlib-4.3 \
      -lITKIOImageBase-4.3 -lITKIOTIFF-4.3 \
      -lITKLabelMap-4.3 -litktiff-4.3 -litkzlib-4.3 \
      -litkjpeg-4.3 -lITKStatistics-4.3 -lITKNetlibSlatec-4.3 \
      -litkOptimizers-4.3
}
else:exists(../lib/ITK/lib/libITKCommon-4.4.a) {
    LIBS += -lITKCommon-4.4 \
      -litkv3p_lsqr-4.4 \
      -lITKVNLInstantiation-4.4 \
      -litkvnl_algo-4.4 \
      -litkvnl-4.4 \
      -litkvcl-4.4 \
      -litkdouble-conversion-4.4 -litksys-4.4 \
      -litkv3p_netlib-4.4 \
      -lITKIOImageBase-4.4 -lITKIOTIFF-4.4 \
      -lITKLabelMap-4.4 -litktiff-4.4 -litkzlib-4.4 \
      -litkjpeg-4.4 -lITKStatistics-4.4 -lITKNetlibSlatec-4.4 \
      -litkOptimizers-4.4
}
else:exists(../lib/ITK/lib/libITKCommon.a) {
    INCLUDEPATH += \
      $$ITK_INCLUDE_PATH/BasicFilters \
      $$ITK_INCLUDE_PATH/Algorithms \
      $$ITK_INCLUDE_PATH/Common \
      $$ITK_INCLUDE_PATH/Numerics \
      $$ITK_INCLUDE_PATH/Utilities \
      $$ITK_INCLUDE_PATH/Utilities/vxl/core \
      $$ITK_INCLUDE_PATH/Utilities/vxl/vcl
    LIBS += -lITKCommon \
      -litkvnl_algo \
      -litkvnl \
      -litkvcl \
      -litksys \
      -litkv3p_netlib
}

CONFIG(debug, debug|release) {
    exists(/usr/local/include/opencv/cv.h) {
        DEFINES += _USE_OPENCV_
        INCLUDEPATH += /usr/local/include/opencv
        LIBS += -lopencv_core -lopencv_ml
    }
}

