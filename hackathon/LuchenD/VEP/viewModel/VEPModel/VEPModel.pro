QT -= gui

TEMPLATE = lib
DEFINES += VEPMODEL_LIBRARY

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    PluginRequestCallback.cpp \
    PyFunctionCallback.cpp \
    lrucache.cpp \
    vepmodel.cpp \
    veppluginmanager.cpp

HEADERS += \
    PluginRequestCallback.h \
    PyFunctionCallback.h \
    VEPModel_global.h \
    extern/pybind11/attr.h \
    extern/pybind11/buffer_info.h \
    extern/pybind11/cast.h \
    extern/pybind11/chrono.h \
    extern/pybind11/common.h \
    extern/pybind11/complex.h \
    extern/pybind11/detail/class.h \
    extern/pybind11/detail/common.h \
    extern/pybind11/detail/descr.h \
    extern/pybind11/detail/init.h \
    extern/pybind11/detail/internals.h \
    extern/pybind11/detail/type_caster_base.h \
    extern/pybind11/detail/typeid.h \
    extern/pybind11/eigen.h \
    extern/pybind11/eigen/matrix.h \
    extern/pybind11/eigen/tensor.h \
    extern/pybind11/embed.h \
    extern/pybind11/eval.h \
    extern/pybind11/functional.h \
    extern/pybind11/gil.h \
    extern/pybind11/iostream.h \
    extern/pybind11/numpy.h \
    extern/pybind11/operators.h \
    extern/pybind11/options.h \
    extern/pybind11/pybind11.h \
    extern/pybind11/pytypes.h \
    extern/pybind11/stl.h \
    extern/pybind11/stl/filesystem.h \
    extern/pybind11/stl_bind.h \
    json-cpp/json.h \
    json-cpp/json_decode.h \
    json-cpp/json_detail.h \
    json-cpp/json_encode.h \
    json-cpp/json_error.h \
    json-cpp/json_ptr.h \
    json-cpp/json_reader.h \
    json-cpp/json_value.h \
    lrucache.h \
    vepmodel.h \
    veppluginmanager.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
