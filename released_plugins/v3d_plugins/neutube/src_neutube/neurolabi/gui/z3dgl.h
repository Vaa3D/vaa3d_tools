#ifndef Z3DGL_H
#define Z3DGL_H

#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif // __APPLE__

#include <string>
#include "zglmutils.h"
#include <QsLog.h>

enum Z3DEye
{
  LeftEye, CenterEye, RightEye
};

enum Z3DScreenShotType
{
  MonoView, HalfSideBySideStereoView, FullSideBySideStereoView
};

GLenum _CheckGLError(const char* file, int line, const char* function);

#define CHECK_GL_ERROR _CheckGLError(__FILE__, __LINE__, __PRETTY_FUNCTION__)

bool checkGLState(GLenum pname, bool value);
bool checkGLState(GLenum pname, GLint value);
bool checkGLState(GLenum pname, GLfloat value);
bool checkGLState(GLenum pname, const glm::vec4 value);

class Exception : public std::exception {
public:
  Exception(const std::string& what = "") : m_what(what) {}
  virtual ~Exception() throw() {}

  virtual const char* what() const throw() {return m_what.c_str();}
protected:
  std::string m_what;
};

#endif  //Z3DGL_H
