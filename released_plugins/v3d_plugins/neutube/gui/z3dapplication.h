#ifndef Z3DAPPLICATION_H
#define Z3DAPPLICATION_H

#include <QString>

class Z3DApplication
{
public:
  Z3DApplication(const QString& appDir);

  virtual ~Z3DApplication();

  static Z3DApplication* app() { return m_app; }

  bool is3DSupported() const { return m_glInitialized; }

  bool isStereoViewSupported() const { return m_stereoViewSupported; }
  void setStereoSupported(bool v) { m_stereoViewSupported = v; }

  virtual void initialize();
  virtual void deinitialize();
  // return false if failed
  virtual bool initializeGL();
  virtual bool deinitializeGL();

  QString getErrorMessage() const { return m_errorMsg; }

  QString getShaderPath(const QString& filename = "") const;
  QString getFontPath(const QString& filename = "") const;

protected:
  QString m_osString;

  static Z3DApplication* m_app;

  QString m_errorMsg;

  QString m_applicationDirPath;

  QString m_fontPath;
  QString m_shaderPath;

  bool m_initialized;
  bool m_glInitialized;

  bool m_stereoViewSupported;

private:
  void detectOS();
};

#endif // Z3DAPPLICATION_H
