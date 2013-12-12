#ifndef Z3DVOLUME_H
#define Z3DVOLUME_H

#include "z3dgl.h"
#include <QObject>
#include "zglmutils.h"
#include "zstack.hxx"
#include <QThread>

class Z3DVolume;
class Z3DShaderProgram;
class Z3DTexture;

class Z3DVolumeHistogramThread : public QThread
{
public:
  Z3DVolumeHistogramThread(Z3DVolume *volume, QObject* parent = 0);
  void run();
  std::vector<size_t>& getHistogram() { return m_histogram; }

private:
  Z3DVolume* m_volume;
  std::vector<size_t> m_histogram;
};

// Z3DVolume coordinates:
// 1. Voxel Coordinate:    [0, dim.x-1] x [0, dim.y-1] x [0, dim.z-1]
//                     in which (0,0,0) is LeftUpFront Corner (LUF)
//                         and dim-1 is RightDownBack Corner (RDB)
// 2. Texture Coordinate:  [0.0, 1.0] x [0.0, 1.0] x [0.0, 1.0]
// 2. Physical Coordinate: VoxelCoord * Spacing + Offset
//                 Note: Offset is physical space offset (after scaling)
// 3. World Coordinate: TransformationMatrix * PhysicalCoord

// Z3DVolume data source info:
// ParentVolume ------crop----->  DetailVolume -------downsample------>  current Z3DVolume

// scale spacing is spacing caused by different image resolution in each direction
// downsample spacing is spacing caused by downsampling a volume to fit it into graphical memory
// spacing = scaleSpacing * downsampleSpacing

// only one channel GREY8, GREY16 or FLOAT32 volume is supported, so convert first
class Z3DVolume : public QObject
{
  Q_OBJECT
public:
  // Z3DVolume will take ownership of the stack
  Z3DVolume(Stack* stack,
            const glm::vec3& downsampleSpacing = glm::vec3(1.f),
            const glm::vec3& scaleSpacing = glm::vec3(1.f),
            const glm::vec3& offset = glm::vec3(0.f),
            const glm::mat4& transformation = glm::mat4(),
            QObject *parent = 0);
  virtual ~Z3DVolume();

  // by default same as dimension.
  void setParentVolumeDimensions(const glm::svec3 &dim) { m_parentVolumeDimensions = dim; }
  // by default same as current offset
  void setParentVolumeOffset(const glm::vec3 &of) { m_parentVolumeOffset  = of; }

  bool isSubVolume() const { return m_parentVolumeDimensions != m_detailVolumeDimensions; }
  bool isDownsampledVolume() const { return m_detailVolumeDimensions != m_dimensions; }

  // actual data dimension of current stack
  glm::svec3 getDimensions() const { return m_dimensions; }
  // detail stack dimension, current stack might be downsampled from detail stack
  glm::svec3 getOriginalDimensions() const { return m_detailVolumeDimensions; }
  // parent stack dimension, detailed stack might be cropped form parent stack
  glm::svec3 getParentVolumeDimensions() const { return m_parentVolumeDimensions; }

  int getBitsStored() const;
  size_t getNumVoxels() const;
  bool is1DData() const { return m_dimensions.z == 1 && (m_dimensions.x == 1 || m_dimensions.y == 1); }
  bool is2DData() const { return m_dimensions.z == 1 && m_dimensions.x > 1 && m_dimensions.y > 1; }
  bool is3DData() const { return m_dimensions.z > 1 && m_dimensions.x > 1 && m_dimensions.y > 1; }

  // Returns a string representation of the sampler type: "sampler2D" for 2D image, "sampler3D" for 3D volume
  QString getSamplerType() const;

  glm::vec3 getSpacing() const { return m_scaleSpacing * m_downsampleSpacing; }
  glm::vec3 getScaleSpacing() const { return m_scaleSpacing; }
  glm::vec3 getOffset() const { return m_offset; }
  double getMinValue() const { return m_minValue; }
  double getMaxValue() const { return m_maxValue; }
  // float version return pixel value in range [0.0 1.0]
  double getFloatMinValue() const;
  double getFloatMaxValue() const;
  double getValue(int x, int y, int z) const;
  double getValue(size_t index) const;

  // to use histogram, first check hasHistogram(), if not, then call asyncGenerateHistogram() and wait for signal histogramFinished().
  bool hasHistogram() const { return !m_histogram.empty(); }
  void asyncGenerateHistogram();
  size_t getHistogramBinCount() const;
  size_t getHistogramValue(size_t index) const;
  size_t getHistogramValue(double fraction) const;    // input value in range [0.0, 1.0]
  double getNormalizedHistogramValue(size_t index) const;
  double getNormalizedHistogramValue(double fraction) const;
  double getLogNormalizedHistogramValue(size_t index) const;
  double getLogNormalizedHistogramValue(double fraction) const;

  Z3DTexture* getTexture();

  void setUniform(Z3DShaderProgram &shader, const QString &uniform, const GLint texUnit) const;

  int getType() const { return m_stack->kind; }

  // Useful coordinate L->Left U->Up F->Front R->Right D->Down B->Back

  glm::vec3 getCubeSize() const { return glm::vec3(getDimensions()) * getSpacing(); }
  glm::vec3 getPhysicalLUF() const { return getOffset(); }
  glm::vec3 getPhysicalRDB() const { return getOffset() + getCubeSize() - getSpacing(); }
  glm::vec3 getPhysicalLDF() const { return glm::vec3(getPhysicalLUF().x, getPhysicalRDB().y, getPhysicalLUF().z); }
  glm::vec3 getPhysicalRDF() const { return glm::vec3(getPhysicalRDB().x, getPhysicalRDB().y, getPhysicalLUF().z); }
  glm::vec3 getPhysicalRUF() const { return glm::vec3(getPhysicalRDB().x, getPhysicalLUF().y, getPhysicalLUF().z); }
  glm::vec3 getPhysicalLUB() const { return glm::vec3(getPhysicalLUF().x, getPhysicalLUF().y, getPhysicalRDB().z); }
  glm::vec3 getPhysicalLDB() const { return glm::vec3(getPhysicalLUF().x, getPhysicalRDB().y, getPhysicalRDB().z); }
  glm::vec3 getPhysicalRUB() const { return glm::vec3(getPhysicalRDB().x, getPhysicalLUF().y, getPhysicalRDB().z); }

  // xmin, xmax, ymin, ymax, zmin, zmax
  std::vector<double> getPhysicalBoundBox() const;

  // bound voxels in world coordinate
  glm::vec3 getWorldLUF() const { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getPhysicalLUF()) : getPhysicalLUF(); }
  glm::vec3 getWorldRDB() const { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getPhysicalRDB()) : getPhysicalRDB(); }
  glm::vec3 getWorldLDF() const { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getPhysicalLDF()) : getPhysicalLDF(); }
  glm::vec3 getWorldRDF() const { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getPhysicalRDF()) : getPhysicalRDF(); }
  glm::vec3 getWorldRUF() const { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getPhysicalRUF()) : getPhysicalRUF(); }
  glm::vec3 getWorldLUB() const { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getPhysicalLUB()) : getPhysicalLUB(); }
  glm::vec3 getWorldLDB() const { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getPhysicalLDB()) : getPhysicalLDB(); }
  glm::vec3 getWorldRUB() const { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getPhysicalRUB()) : getPhysicalRUB(); }

  // xmin, xmax, ymin, ymax, zmin, zmax
  std::vector<double> getWorldBoundBox() const;

  // corners of parent volume
  glm::vec3 getParentVolPhysicalLUF() const { return m_parentVolumeOffset; }
  glm::vec3 getParentVolPhysicalRDB() const { return m_parentVolumeOffset + (glm::vec3(m_parentVolumeDimensions)-1.f) * m_scaleSpacing; }
  glm::vec3 getParentVolPhysicalLDF() const { return glm::vec3(getParentVolPhysicalLUF().x, getParentVolPhysicalRDB().y, getParentVolPhysicalLUF().z); }
  glm::vec3 getParentVolPhysicalRDF() const { return glm::vec3(getParentVolPhysicalRDB().x, getParentVolPhysicalRDB().y, getParentVolPhysicalLUF().z); }
  glm::vec3 getParentVolPhysicalRUF() const { return glm::vec3(getParentVolPhysicalRDB().x, getParentVolPhysicalLUF().y, getParentVolPhysicalLUF().z); }
  glm::vec3 getParentVolPhysicalLUB() const { return glm::vec3(getParentVolPhysicalLUF().x, getParentVolPhysicalLUF().y, getParentVolPhysicalRDB().z); }
  glm::vec3 getParentVolPhysicalLDB() const { return glm::vec3(getParentVolPhysicalLUF().x, getParentVolPhysicalRDB().y, getParentVolPhysicalRDB().z); }
  glm::vec3 getParentVolPhysicalRUB() const { return glm::vec3(getParentVolPhysicalRDB().x, getParentVolPhysicalLUF().y, getParentVolPhysicalRDB().z); }

  glm::vec3 getParentVolWorldLUF() const
  {
    return !isSubVolume() ? getWorldLUF() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getParentVolPhysicalLUF()) : getParentVolPhysicalLUF();
  }
  glm::vec3 getParentVolWorldRDB() const
  {
    return !isSubVolume() ? getWorldRDB() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getParentVolPhysicalRDB()) : getParentVolPhysicalRDB();
  }
  glm::vec3 getParentVolWorldLDF() const
  {
    return !isSubVolume() ? getWorldLDF() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getParentVolPhysicalLDF()) : getParentVolPhysicalLDF();
  }
  glm::vec3 getParentVolWorldRDF() const
  {
    return !isSubVolume() ? getWorldRDF() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getParentVolPhysicalRDF()) : getParentVolPhysicalRDF();
  }
  glm::vec3 getParentVolWorldRUF() const
  {
    return !isSubVolume() ? getWorldRUF() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getParentVolPhysicalRUF()) : getParentVolPhysicalRUF();
  }
  glm::vec3 getParentVolWorldLUB() const
  {
    return !isSubVolume() ? getWorldLUB() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getParentVolPhysicalLUB()) : getParentVolPhysicalLUB();
  }
  glm::vec3 getParentVolWorldLDB() const
  {
    return !isSubVolume() ? getWorldLDB() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getParentVolPhysicalLDB()) : getParentVolPhysicalLDB();
  }
  glm::vec3 getParentVolWorldRUB() const
  {
    return !isSubVolume() ? getWorldRUB() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, getParentVolPhysicalRUB()) : getParentVolPhysicalRUB();
  }

  void setScaleSpacing(const glm::vec3 &spacing) { m_scaleSpacing = spacing; }
  void setDownsampleSpacing(const glm::vec3 &spacing) { m_downsampleSpacing = spacing; }
  void setOffset(const glm::vec3 &offset) { m_offset = offset; }
  void setPhysicalToWorldMatrix(const glm::mat4& transformationMatrix);
  void translate(double dx, double dy, double dz);

  // color of this channel, max intensity will map to this color
  void setVolColor(const glm::vec3 &col) { m_volColor = glm::clamp(col, glm::vec3(0.f), glm::vec3(1.f)); }
  glm::vec3 getVolColor() const { return m_volColor; }

  // Returns the matrix mapping from voxel coordinates (i.e. [0; dim-1]) to world coordinates.
  glm::mat4 getVoxelToWorldMatrix() const;

  // Returns the matrix mapping from world coordinates to voxel coordinates (i.e. [0; dim-1]).
  glm::mat4 getWorldToVoxelMatrix() const;

  // Returns the matrix mapping from world coordinates to texture coordinates (i.e. [0.0; 1.0]).
  glm::mat4 getWorldToTextureMatrix() const;

  // Returns the matrix mapping from texture coordinates (i.e. [0.0; 1.0]) to world coordinates.
  glm::mat4 getTextureToWorldMatrix() const;

  glm::mat4 getVoxelToPhysicalMatrix() const;
  glm::mat4 getPhysicalToVoxelMatrix() const;

  glm::mat4 getPhysicalToWorldMatrix() const { return m_transformationMatrix; }
  glm::mat4 getWorldToPhysicalMatrix() const;

  glm::mat4 getTextureToPhysicalMatrix() const;
  glm::mat4 getPhysicalToTextureMatrix() const;

  glm::mat4 getTextureToVoxelMatrix() const;
  glm::mat4 getVoxelToTextureMatrix() const;

signals:
  void histogramFinished();

protected slots:
  void setHistogram();

protected:
  void generateTexture();

private:
  void computeMinValue();
  void computeMaxValue();
  void computeHistogramMaxValue();

protected:
  Stack *m_stack;
  Image_Array m_data;
  glm::svec3 m_dimensions;
  glm::svec3 m_parentVolumeDimensions;
  glm::vec3 m_parentVolumeOffset;
  glm::svec3 m_detailVolumeDimensions;
  double m_minValue;
  double m_maxValue;
  glm::vec3 m_scaleSpacing;
  glm::vec3 m_downsampleSpacing;
  glm::vec3 m_offset;
  glm::mat4 m_transformationMatrix;
  std::vector<size_t> m_histogram;
  double m_histogramMaxValue;
  Z3DTexture *m_texture;

  glm::vec3 m_volColor;

private:
  Z3DVolumeHistogramThread *m_histogramThread;
  bool m_hasTransformMatrix;
};

#endif // Z3DVOLUME_H
