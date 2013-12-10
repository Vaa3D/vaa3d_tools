#ifndef Z3DSHADERPROGRAM_H
#define Z3DSHADERPROGRAM_H

#include "z3dgl.h"
#include <QGLShaderProgram>
#include "zglmutils.h"
#include "z3dtexture.h"

class Z3DVolume;

class Z3DShaderProgram : public QGLShaderProgram
{
public:
  explicit Z3DShaderProgram(QObject * parent = 0);
  explicit Z3DShaderProgram(const QGLContext *context, QObject *parent = 0);
  ~Z3DShaderProgram();

  // overriding bind() to reset texture unit manager
  bool bind();

  using QGLShaderProgram::setUniformValue;
  using QGLShaderProgram::setUniformValueArray;

  void bindFragDataLocation(GLuint colorNumber, const QString &name);

  // bind samplers
  void bindTexture(const QString &name, const Z3DTexture *texture);
  void bindTexture(const QString &name, GLenum target, GLuint textureId);
  void bindVolume(const QString &name, Z3DVolume *volume);
  void bindVolume(const QString &name, Z3DVolume *volume, GLint minFilter, GLint magFilter);

  // load functions will load shaders and link, throw Exception if error
  // input filenames should not contain path, shader paths are managed by Z3DApplication
  void loadFromSourceFile(const QString &vertFilename, const QString &geomFilename,
                          const QString &fragFilename, const QString &header);
  void loadFromSourceFile(const QString &vertFilename, const QString &fragFilename,
                          const QString &header);
  void loadFromSourceFile(const QStringList &shaderFilenames, const QString &header);
  // header will be prepended to all srcs
  void loadFromSourceCode(const QStringList &vertSrcs, const QStringList &geomSrcs,
                          const QStringList &fragSrcs, const QString &header = "");
  void loadFromSourceCode(const QStringList &vertSrcs, const QStringList &fragSrcs,
                          const QString &header = "");

  // set new header for current readed src and rebuild
  void setHeaderAndRebuild(const QString &header);

  // QGLShaderProgram already has a uniformLocation() function, getUniformLocation() will call
  // uniformLocation() and log error if uniform can not be found, many typos can be found this way.
  // Logging can be turn off by setLogUniformLocationError(false)
  GLint getUniformLocation(const QString& name);

  void setLogUniformLocationError(bool logError) {m_logUniformLocationError = logError;}
  bool logUniformLocationError() const {return m_logUniformLocationError;}

  void setUniformValue(GLint loc, GLint v1, GLint v2);
  void setUniformValue(GLint loc, GLint v1, GLint v2, GLint v3);
  void setUniformValue(GLint loc, GLint v1, GLint v2, GLint v3, GLint v4);
  void setUniformValue(GLint loc, GLuint v1, GLuint v2);
  void setUniformValue(GLint loc, GLuint v1, GLuint v2, GLuint v3);
  void setUniformValue(GLint loc, GLuint v1, GLuint v2, GLuint v3, GLuint v4);

  void setUniformValue(const QString& name, GLfloat value);
  void setUniformValue(const QString& name, GLint value);
  void setUniformValue(const QString& name, GLuint value);
  void setUniformValue(const QString& name, GLfloat x, GLfloat y);
  void setUniformValue(const QString& name, GLfloat x, GLfloat y, GLfloat z);
  void setUniformValue(const QString& name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
  void setUniformValue(const QString& name, GLint v1, GLint v2);
  void setUniformValue(const QString& name, GLint v1, GLint v2, GLint v3);
  void setUniformValue(const QString& name, GLint v1, GLint v2, GLint v3, GLint v4);
  void setUniformValue(const QString& name, GLuint v1, GLuint v2);
  void setUniformValue(const QString& name, GLuint v1, GLuint v2, GLuint v3);
  void setUniformValue(const QString& name, GLuint v1, GLuint v2, GLuint v3, GLuint v4);

  // Booleans
  //  void setUniformValue(GLint loc, bool value);
  //  void setUniformValue(GLint loc, bool v1, bool v2);
  //  void setUniformValue(GLint loc, bool v1, bool v2, bool v3);
  //  void setUniformValue(GLint loc, bool v1, bool v2, bool v3, bool v4);
  //  void setUniformValue(const QString& name, bool value);
  //  void setUniformValue(const QString& name, bool v1, bool v2);
  //  void setUniformValue(const QString& name, bool v1, bool v2, bool v3);
  //  void setUniformValue(const QString& name, bool v1, bool v2, bool v3, bool v4);

  // Vectors
  void setUniformValue(GLint loc, const glm::vec2& value);
  void setUniformValue(GLint loc, const glm::vec3& value);
  void setUniformValue(GLint loc, const glm::vec4& value);
  void setUniformValue(GLint loc, const glm::ivec2& value);
  void setUniformValue(GLint loc, const glm::ivec3& value);
  void setUniformValue(GLint loc, const glm::ivec4& value);
  void setUniformValue(GLint loc, const glm::uvec2& value);
  void setUniformValue(GLint loc, const glm::uvec3& value);
  void setUniformValue(GLint loc, const glm::uvec4& value);
  void setUniformValue(const QString& name, const glm::vec2& value);
  void setUniformValue(const QString& name, const glm::vec3& value);
  void setUniformValue(const QString& name, const glm::vec4& value);
  void setUniformValue(const QString& name, const glm::ivec2& value);
  void setUniformValue(const QString& name, const glm::ivec3& value);
  void setUniformValue(const QString& name, const glm::ivec4& value);
  void setUniformValue(const QString& name, const glm::uvec2& value);
  void setUniformValue(const QString& name, const glm::uvec3& value);
  void setUniformValue(const QString& name, const glm::uvec4& value);

  void setUniformValueArray(GLint loc, const glm::vec2* values, int count);
  void setUniformValueArray(GLint loc, const glm::vec3* values, int count);
  void setUniformValueArray(GLint loc, const glm::vec4* values, int count);
  void setUniformValueArray(GLint loc, const glm::ivec2* values, int count);
  void setUniformValueArray(GLint loc, const glm::ivec3* values, int count);
  void setUniformValueArray(GLint loc, const glm::ivec4* values, int count);
  void setUniformValueArray(GLint loc, const glm::uvec2* values, int count);
  void setUniformValueArray(GLint loc, const glm::uvec3* values, int count);
  void setUniformValueArray(GLint loc, const glm::uvec4* values, int count);
  void setUniformValueArray(const QString& name, const glm::vec2* values, int count);
  void setUniformValueArray(const QString& name, const glm::vec3* values, int count);
  void setUniformValueArray(const QString& name, const glm::vec4* values, int count);
  void setUniformValueArray(const QString& name, const glm::ivec2* values, int count);
  void setUniformValueArray(const QString& name, const glm::ivec3* values, int count);
  void setUniformValueArray(const QString& name, const glm::ivec4* values, int count);
  void setUniformValueArray(const QString& name, const glm::uvec2* values, int count);
  void setUniformValueArray(const QString& name, const glm::uvec3* values, int count);
  void setUniformValueArray(const QString& name, const glm::uvec4* values, int count);

  void setUniformValue(GLint loc, const glm::mat2& value, bool transpose = false);
  void setUniformValue(GLint loc, const glm::mat3& value, bool transpose = false);
  void setUniformValue(GLint loc, const glm::mat4& value, bool transpose = false);
  void setUniformValue(const QString& name, const glm::mat2& value, bool transpose = false);
  void setUniformValue(const QString& name, const glm::mat3& value, bool transpose = false);
  void setUniformValue(const QString& name, const glm::mat4& value, bool transpose = false);

  void setUniformValueArray(GLint loc, const glm::mat2* values, int count, bool transpose = false);
  void setUniformValueArray(GLint loc, const glm::mat3* values, int count, bool transpose = false);
  void setUniformValueArray(GLint loc, const glm::mat4* values, int count, bool transpose = false);
  void setUniformValueArray(const QString& name, const glm::mat2* values, int count, bool transpose = false);
  void setUniformValueArray(const QString& name, const glm::mat3* values, int count, bool transpose = false);
  void setUniformValueArray(const QString& name, const glm::mat4* values, int count, bool transpose = false);

protected:
  bool m_logUniformLocationError;
  // srcs read from file, withour header
  QStringList m_vertSrcs;
  QStringList m_geomSrcs;
  QStringList m_fragSrcs;

  Z3DTextureUnitManager m_textureUnitManager;
};

#endif // Z3DSHADERPROGRAM_H
