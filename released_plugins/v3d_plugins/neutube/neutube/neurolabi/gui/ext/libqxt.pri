INCLUDEPATH += $$PWD/libqxt/core $$PWD/libqxt/widgets

DEFINES += BUILD_QXT

HEADERS += \
    $$PWD/libqxt/core/qxtglobal.h \
    $$PWD/libqxt/core/qxtcsvmodel.h \
    $$PWD/libqxt/widgets/qxtspanslider.h \
    $$PWD/libqxt/widgets/qxtspanslider_p.h \
    $$PWD/libqxt/core/qxtnamespace.h

SOURCES += \
    $$PWD/libqxt/core/qxtglobal.cpp \
    $$PWD/libqxt/core/qxtcsvmodel.cpp \
    $$PWD/libqxt/widgets/qxtspanslider.cpp


