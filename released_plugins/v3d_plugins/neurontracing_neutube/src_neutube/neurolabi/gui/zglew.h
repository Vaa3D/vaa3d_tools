#ifndef ZGLEW_H
#define ZGLEW_H

#ifdef __APPLE__
    #include <GL/glew.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#undef glGenVertexArrays
#undef glBindVertexArray
#undef glDeleteVertexArrays
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glBindVertexArray glBindVertexArrayAPPLE
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
#else
    #include <GL/glew.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif // __APPLE__

#endif // ZGLEW_H
