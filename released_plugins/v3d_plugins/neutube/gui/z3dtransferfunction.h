#ifndef Z3DTRANSFERFUNCTION_H
#define Z3DTRANSFERFUNCTION_H

#include "z3dgl.h"
#include <QObject>
#include <vector>
#include "zcolormap.h"
#include "zparameter.h"

class Z3DVolume;
class Z3DTexture;

// only support 1d transfer function now
class Z3DTransferFunction : public ZColorMap
{
  Q_OBJECT
public:

  Z3DTransferFunction(double min = 0.0, double max = 1.0, const glm::col4 &minColor = glm::col4(0,0,0,0),
                      const glm::col4 &maxColor = glm::col4(255,255,255,255),
                      int width = 256,
                      QObject *parent = 0);


  Z3DTransferFunction(const Z3DTransferFunction &tf);
  virtual ~Z3DTransferFunction();

  void swap(Z3DTransferFunction& other);
  Z3DTransferFunction& operator =(Z3DTransferFunction other);


  bool operator==(const Z3DTransferFunction& tf) const;
  bool operator!=(const Z3DTransferFunction& tf) const;

  void resetToDefault();

  QString getSamplerType() const;

  inline glm::ivec3 getTextureDimensions() const { return m_dimensions; }

  // Returns the texture of the transfer function.
  Z3DTexture* getTexture();

  virtual void resize(int width);

  inline bool isTextureInvalid() const { return m_textureIsInvalid; }

  virtual void updateTexture();

  // domain should be in [0.0, 1.0] range
  virtual bool isValidDomainMin(double min) const;
  virtual bool isValidDomainMax(double max) const;
  
signals:
  
public slots:
  void invalidateTexture() { m_textureIsInvalid = true; }

protected:
  virtual void createTexture();

  Z3DTexture* m_texture;
  glm::ivec3 m_dimensions;
  GLint m_textureFormat;
  GLenum m_textureDataType;
  bool m_textureIsInvalid;

private:
  // Adapts the given width and height of transfer function to graphics board capabilities.
  void fitDimensions(int& width, int& height, int& depth) const;
};

class Z3DTransferFunctionParameter : public ZSingleValueParameter<Z3DTransferFunction>
{
public:
  Z3DTransferFunctionParameter(const QString& name, double min = 0.0, double max = 1.0, const glm::col4 &minColor = glm::col4(0,0,0,0),
                               const glm::col4 &maxColor = glm::col4(255,255,255,255), int width = 256,
                               QObject *parent = 0);

  void setVolume(Z3DVolume* volume);

  inline Z3DVolume* getVolume() const { return m_volume; }

protected:
  virtual QWidget* actualCreateWidget(QWidget *parent);

  Z3DVolume* m_volume;
};

#endif // Z3DTRANSFERFUNCTION_H
