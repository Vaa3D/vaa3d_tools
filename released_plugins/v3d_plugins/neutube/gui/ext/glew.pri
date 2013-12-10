INCLUDEPATH += $$PWD/glew/include

DEFINES += GLEW_STATIC

SOURCES += $$PWD/glew/src/glew.c

HEADERS += $$PWD/glew/include/GL/glew.h \
    $$PWD/glew/include/GL/wglew.h \
    $$PWD/glew/include/GL/glxew.h

win32 {
  LIBS += -lopengl32 -lglu32
}

macx {
  LIBS += -framework AGL -framework OpenGL
}

unix:!macx {
  LIBS += -lGL -lGLU
}
