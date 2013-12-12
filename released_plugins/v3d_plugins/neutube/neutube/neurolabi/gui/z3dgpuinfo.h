#ifndef Z3DGPUINFO_H
#define Z3DGPUINFO_H

#include "z3dgl.h"
#include <QString>

// This class provides information about the GPU
// If the openGL version is too low or certain critical extensions are not supported,
// isSupported() will return false, and getNotSupportedReason() will return the reason.
// If isSupported() return false, other functions (except getGl*Version functions
// which will still be correct.) will return uninitialized values.

#define Z3DGpuInfoInstance Z3DGpuInfo::getInstance()

class Z3DGpuInfo
{
public:
  static Z3DGpuInfo& getInstance();

  enum GpuVendor {
    GPU_VENDOR_NVIDIA,
    GPU_VENDOR_ATI,
    GPU_VENDOR_INTEL,
    GPU_VENDOR_UNKNOWN
  };

  Z3DGpuInfo();
  ~Z3DGpuInfo() {}

  bool isSupported() const { return m_isSupported; }
  QString getNotSupportedReason() const { return m_notSupportedReason; }

  int getGlMajorVersion() const { return m_glMajorVersion; }
  int getGlMinorVersion() const { return m_glMinorVersion; }
  int getGlReleaseVersion() const { return m_glReleaseVersion; }
  int getGlslMajorVersion() const;
  int getGlslMinorVersion() const;
  int getGlslReleaseVersion() const;

  GpuVendor getGpuVendor() const;

  bool isExtensionSupported(QString extension) const;

  QString getGlVersionString() const;
  QString getGlVendorString() const;
  QString getGlRendererString() const;
  QString getGlExtensionsString() const;
  QString getGlShadingLanguageVersionString() const;

  // Returns the maximal side length of 1D and 2D textures.
  int getMaxTextureSize() const { return m_maxTexureSize; }
  // Returns the maximal side length of 3D textures.
#ifdef __APPLE__
  // mac pro bug.
  int getMax3DTextureSize() const { return std::min(2048, m_max3DTextureSize); }
#else
  int getMax3DTextureSize() const { return m_max3DTextureSize; }
#endif

  // Return a value such as 16 or 32. That is the number of image samplers that your GPU supports in the fragment shader.
  int getMaxTextureImageUnits() const { return m_maxTextureImageUnits; }
  // The following is for the vertex shader (available since GL 2.0). This might return 0 for certain GPUs.
  int getMaxVertexTextureImageUnits() const { return m_maxVertexTextureImageUnits; }
  // The following is for the geometry shader (available since GL 3.2)
  int getMaxGeometryTextureImageUnits() const { return m_maxGeometryTextureImageUnits; }
  // The following is VS + GS + FS (available since GL 2.0)
  int getMaxCombinedTextureImageUnits() const { return m_maxCombinedTextureImageUnits; }
  // and the following is the number of texture coordinates available which usually is 8
  int getMaxTextureCoordinates() const { return m_maxTextureCoords; }

  int getAvailableTextureMemory() const;
  int getTotalTextureMemory() const;

  bool isFrameBufferObjectSupported() const;
  bool isNonPowerOfTwoTextureSupported() const;
  bool isGeometryShaderSupported() const;
  bool isTextureFilterAnisotropicSupported() const;
  bool isTextureRectangleSupported() const;
  // for glBlendEquation
  bool isImagingSupported() const;
  bool isColorBufferFloatSupported() const;
  bool isDepthBufferFloatSupported() const;
  bool isTextureFloatSupported() const;
  bool isTextureRGSupported() const;

  bool isVAOSupported() const;

  /**
     * Returns the maximum anisotropy. If
     * anisotropic texture filtering is not
     * supported, 1.0 is returned.
     */
  float getMaxTextureAnisotropy() const { return m_maxTextureAnisotropy; }

  // Returns the maximal number of color attachments for a FBO
  int getMaxColorAttachments() const { return m_maxColorAttachments; }
  int getMaxDrawBuffer() const { return m_maxDrawBuffer; }

  float getMinSmoothPointSize() const { return m_minSmoothPointSize; }
  float getMaxSmoothPointSize() const { return m_maxSmoothPointSize; }
  float getSmoothPointSizeGranularity() const { return m_smoothPointSizeGranularity; }
  float getMinAliasedPointSize() const { return m_minAliasedPointSize; }
  float getMaxAliasedPointSize() const { return m_maxAliasedPointSize; }

  float getMinSmoothLineWidth() const { return m_minSmoothLineWidth; }
  float getMaxSmoothLineWidth() const { return m_maxSmoothLineWidth; }
  float getSmoothLineWidthGranularity() const { return m_smoothLineWidthGranularity; }
  float getMinAliasedLineWidth() const { return m_minAliasedLineWidth; }
  float getMaxAliasedLineWidth() const { return m_maxAliasedLineWidth; }

  // log useful gpu info
  void logGpuInfo() const;

  // check avalibility of some special effect
  bool isWeightedAverageSupported() const;
  bool isDualDepthPeelingSupported() const;
  bool isLinkedListSupported() const;

protected:
  void detectGpuInfo();
  bool parseVersionString(const QString &versionString, int &major, int &minor, int &release);

private:
  bool m_isSupported;   //whether current graphic card is supported
  QString m_notSupportedReason;  // Reason why current gpu card are not supported

  int m_glMajorVersion;
  int m_glMinorVersion;
  int m_glReleaseVersion;
  int m_glslMajorVersion;
  int m_glslMinorVersion;
  int m_glslReleaseVersion;

  QString m_glVersionString;
  QString m_glExtensionsString;
  QString m_glVendorString;
  QString m_glRendererString;
  QString m_glslVersionString;
  GpuVendor m_gpuVendor;

  int m_maxTexureSize;
  int m_max3DTextureSize;
  float m_maxTextureAnisotropy;
  int m_maxColorAttachments;
  int m_maxDrawBuffer;
  int m_maxGeometryOutputVertices;

  // Return a value such as 16 or 32. That is the number of image samplers that your GPU supports in the fragment shader.
  int m_maxTextureImageUnits;
  // The following is for the vertex shader (available since GL 2.0). This might return 0 for certain GPUs.
  int m_maxVertexTextureImageUnits;
  // The following is for the geometry shader (available since GL 3.2)
  int m_maxGeometryTextureImageUnits;
  // The following is VS + GS + FS (available since GL 2.0)
  int m_maxCombinedTextureImageUnits;
  // and the following is the number of texture coordinates available which usually is 8
  int m_maxTextureCoords;

  float m_minSmoothPointSize;
  float m_maxSmoothPointSize;
  float m_smoothPointSizeGranularity;
  float m_minAliasedPointSize;
  float m_maxAliasedPointSize;

  float m_minSmoothLineWidth;
  float m_maxSmoothLineWidth;
  float m_smoothLineWidthGranularity;
  float m_minAliasedLineWidth;
  float m_maxAliasedLineWidth;
};

#endif // Z3DGPUINFO_H
