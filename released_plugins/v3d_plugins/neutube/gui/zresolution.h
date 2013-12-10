#ifndef ZRESOLUTION_H
#define ZRESOLUTION_H

class ZResolution {
public:
  ZResolution();

  inline const double* voxelSize() const { return m_voxelSize; }
  inline double voxelSizeX() const { return m_voxelSize[0]; }
  inline double voxelSizeY() const { return m_voxelSize[1]; }
  inline double voxelSizeZ() const { return m_voxelSize[2]; }
  inline char unit() const { return m_unit; }

  inline void setVoxelSize(double x, double y, double z) {
    m_voxelSize[0] = x;
    m_voxelSize[1] = y;
    m_voxelSize[2] = z;
  }

  inline void setUnit(char unit) { m_unit = unit; }

private:
  double m_voxelSize[3];
  char m_unit; //'p' for pixel, 'u' for um
};


#endif // ZRESOLUTION_H
