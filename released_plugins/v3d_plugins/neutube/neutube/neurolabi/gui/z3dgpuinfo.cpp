#include "z3dgpuinfo.h"

#include <QStringList>
#include <QProcess>
#include "QsLog.h"

Z3DGpuInfo& Z3DGpuInfo::getInstance()
{
  static Z3DGpuInfo gpuInfo;
  return gpuInfo;
}

Z3DGpuInfo::Z3DGpuInfo()
  : m_isSupported(false)
{
  detectGpuInfo();
}

int Z3DGpuInfo::getGlslMajorVersion() const
{
  if (isSupported()) {
    return m_glslMajorVersion;
  } else {
    LERROR() << "Current GPU card not supported. This function call should not happen.";
    return -1;
  }
}

int Z3DGpuInfo::getGlslMinorVersion() const
{
  if (isSupported()) {
    return m_glslMinorVersion;
  } else {
    LERROR() << "Current GPU card not supported. This function call should not happen.";
    return -1;
  }
}

int Z3DGpuInfo::getGlslReleaseVersion() const
{
  if (isSupported()) {
    return m_glslReleaseVersion;
  } else {
    LERROR() << "Current GPU card not supported. This function call should not happen.";
    return -1;
  }
}

Z3DGpuInfo::GpuVendor Z3DGpuInfo::getGpuVendor() const
{
  return m_gpuVendor;
}

bool Z3DGpuInfo::isExtensionSupported(QString extension) const
{
  return m_glExtensionsString.contains(extension, Qt::CaseInsensitive);
}

QString Z3DGpuInfo::getGlVersionString() const
{
  return m_glVersionString;
}

QString Z3DGpuInfo::getGlVendorString() const
{
  return m_glVendorString;
}

QString Z3DGpuInfo::getGlRendererString() const
{
  return m_glRendererString;
}

QString Z3DGpuInfo::getGlShadingLanguageVersionString() const
{
  return m_glslVersionString;
}

QString Z3DGpuInfo::getGlExtensionsString() const
{
  return m_glExtensionsString;
}

int Z3DGpuInfo::getAvailableTextureMemory() const
{
  int availableTexMem = -1;

  if (m_gpuVendor == GPU_VENDOR_NVIDIA) {
    if(isExtensionSupported("GL_NVX_gpu_memory_info")) {
      int retVal;
      glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &retVal);
      availableTexMem = retVal;
    }
  } else if (m_gpuVendor == GPU_VENDOR_ATI) {
    if(isExtensionSupported("GL_ATI_meminfo")) {
      int retVal[4];
      glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, retVal);
      availableTexMem = retVal[1];
    }
  }

  return availableTexMem;
}

int Z3DGpuInfo::getTotalTextureMemory() const
{
  int totalTexMem = -1;

  if (m_gpuVendor == GPU_VENDOR_NVIDIA) {
    if(isExtensionSupported("GL_NVX_gpu_memory_info")) {
      int retVal;
      glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &retVal);
      totalTexMem = retVal;
    }
  } else if (m_gpuVendor == GPU_VENDOR_ATI) {
    if(isExtensionSupported("GL_ATI_meminfo")) {
      int retVal[4];
      glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, retVal);
      totalTexMem = retVal[0];
    }
  }

  return totalTexMem;
}

bool Z3DGpuInfo::isFrameBufferObjectSupported() const
{
  return GLEW_VERSION_3_0 || isExtensionSupported("GL_EXT_framebuffer_object");
}

bool Z3DGpuInfo::isNonPowerOfTwoTextureSupported() const
{
  return GLEW_VERSION_2_0 || isExtensionSupported("GL_ARB_texture_non_power_of_two");
}

bool Z3DGpuInfo::isGeometryShaderSupported() const
{
  return GLEW_VERSION_3_2 ||
      isExtensionSupported("GL_ARB_geometry_shader4") ||
      isExtensionSupported("GL_EXT_geometry_shader4");
}

bool Z3DGpuInfo::isTextureFilterAnisotropicSupported() const
{
  return isExtensionSupported("GL_EXT_texture_filter_anisotropic");
}

bool Z3DGpuInfo::isTextureRectangleSupported() const
{
  return GLEW_VERSION_3_1 || isExtensionSupported("GL_ARB_texture_rectangle");
}

bool Z3DGpuInfo::isImagingSupported() const
{
  return isExtensionSupported("GL_ARB_imaging");
}

bool Z3DGpuInfo::isColorBufferFloatSupported() const
{
  return GLEW_VERSION_3_0 || isExtensionSupported("GL_ARB_color_buffer_float");
}

bool Z3DGpuInfo::isDepthBufferFloatSupported() const
{
  return GLEW_VERSION_3_0 || isExtensionSupported("GL_ARB_depth_buffer_float");
}

bool Z3DGpuInfo::isTextureFloatSupported() const
{
  return GLEW_VERSION_3_0 || isExtensionSupported("GL_ARB_texture_float");
}

bool Z3DGpuInfo::isTextureRGSupported() const
{
  return GLEW_VERSION_3_0 || isExtensionSupported("GL_ARB_texture_rg");
}

bool Z3DGpuInfo::isVAOSupported() const
{
  return GLEW_VERSION_3_0 || isExtensionSupported("GL_ARB_vertex_array_object") ||
      isExtensionSupported("GL_APPLE_vertex_array_object");
}

void Z3DGpuInfo::logGpuInfo() const
{
  if (!isSupported()) {
    LINFO() << "Current GPU card is not supported. Reason: " << m_notSupportedReason;
    LWARN() << "3D functions will be disabled.";
    return;
  }

#ifdef __APPLE__
  QProcess dispInfo;
  dispInfo.start("system_profiler", QStringList() << "SPDisplaysDataType");

  if (dispInfo.waitForFinished(-1))
    LINFO() << dispInfo.readAllStandardOutput();
  else
    LINFO() << dispInfo.readAllStandardError();
#endif

  LINFO() << "OpenGL Vendor:" << m_glVendorString;
  LINFO() << "OpenGL Renderer:" << m_glRendererString;
  LINFO() << "OpenGL Version:" << m_glVersionString;
  LINFO() << "OpenGL SL Version:" << m_glslVersionString;
  LINFO() << "OpenGL Extensions:" << m_glExtensionsString;
  LINFO() << "Max Texture Size:              " << m_maxTexureSize;
  LINFO() << "Max 3D Texture Size:           " << m_max3DTextureSize;
  LINFO() << "Max Color Attachments:         " << m_maxColorAttachments;
  LINFO() << "Max Draw Buffer:               " << m_maxDrawBuffer;
  if(isGeometryShaderSupported() && m_maxGeometryOutputVertices > 0) {
    LINFO() << "Max GS Output Vertices:        " << m_maxGeometryOutputVertices;
  }
  LINFO() << "Max VS Texture Image Units:    " << m_maxVertexTextureImageUnits;
  if (isGeometryShaderSupported() && m_maxGeometryTextureImageUnits > 0) {
    LINFO() << "Max GS Texture Image Units:    " << m_maxGeometryTextureImageUnits;
  }
  LINFO() << "Max FS Texture Image Units:    " << m_maxTextureImageUnits;
  LINFO() << "VS+GS+FS Texture Image Units:  " << m_maxCombinedTextureImageUnits;
  LINFO() << "Max Texture Coordinates:       " << m_maxTextureCoords;

  if(getTotalTextureMemory() != -1) {
    LINFO() << "Total Graphics Memory Size:    " << getTotalTextureMemory()/1024 << "MB";
  }

  if(getAvailableTextureMemory() != -1) {
    LINFO() << "Available Graphics Memory Size:" << getAvailableTextureMemory()/1024 << "MB";
  }

  LINFO() << "Smooth Point Size Range:       " << "(" << m_minSmoothPointSize << "," << m_maxSmoothPointSize << ")";
  LINFO() << "Smooth Point Size Granularity: " << m_smoothPointSizeGranularity;
  LINFO() << "Aliased Point Size Range:      " << "(" << m_minAliasedPointSize << "," << m_maxAliasedPointSize << ")";

  LINFO() << "Smooth Line Width Range:       " << "(" << m_minSmoothLineWidth << "," << m_maxSmoothLineWidth << ")";
  LINFO() << "Smooth Line Width Granularity: " << m_smoothLineWidthGranularity;
  LINFO() << "Aliased Line Width Range:      " << "(" << m_minAliasedLineWidth << "," << m_maxAliasedLineWidth << ")";

  LINFO() << "";
}

bool Z3DGpuInfo::isWeightedAverageSupported() const
{
#ifdef _ENABLE_WAVG_
  return Z3DGpuInfoInstance.isTextureRGSupported() && Z3DGpuInfoInstance.isTextureRectangleSupported() &&
      Z3DGpuInfoInstance.isTextureFloatSupported() && Z3DGpuInfoInstance.isImagingSupported() &&
      Z3DGpuInfoInstance.isColorBufferFloatSupported() &&
      Z3DGpuInfoInstance.getMaxColorAttachments() >= 2;
#else
  return false;
#endif
}

bool Z3DGpuInfo::isDualDepthPeelingSupported() const
{
#ifdef _ENABLE_DDP_
  return Z3DGpuInfoInstance.isTextureRGSupported() && Z3DGpuInfoInstance.isTextureRectangleSupported() &&
      Z3DGpuInfoInstance.isTextureFloatSupported() && Z3DGpuInfoInstance.isImagingSupported() &&
      Z3DGpuInfoInstance.isColorBufferFloatSupported() &&
      Z3DGpuInfoInstance.getMaxColorAttachments() >= 8;
#else
  return false;
#endif
}

bool Z3DGpuInfo::isLinkedListSupported() const
{
  return GLEW_VERSION_4_2;
}

void Z3DGpuInfo::detectGpuInfo()
{
  m_glVersionString = QString(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
  m_glVendorString  = QString(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
  m_glRendererString  = QString(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
  m_glExtensionsString = QString(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));

  if (GLEW_VERSION_2_1) {
    if (!isFrameBufferObjectSupported()) {
      m_isSupported = false;
      m_notSupportedReason = "Frame Buffer Object (FBO) is not supported by current openGL context.";
      return;
    } else if (!isNonPowerOfTwoTextureSupported()) { // not necessary, NPOT texture is supported since opengl 2.0
      m_isSupported = false;
      m_notSupportedReason = "Non power of two texture is not supported by current openGL context.";
      return;
    } else if (getGpuVendor() == GPU_VENDOR_ATI && isNonPowerOfTwoTextureSupported() &&
               (m_glRendererString.contains("RADEON X", Qt::CaseInsensitive)||
                m_glRendererString.contains("RADEON 9", Qt::CaseInsensitive))) { //from http://www.opengl.org/wiki/NPOT_Texture
      m_isSupported = false;
      m_notSupportedReason = "The R300 and R400-based cards (Radeon 9500+ and X500+) are incapable of generic NPOT usage. You can use NPOTs, \
          but only if the texture has no mipmaps.";
      return;
    } else if (getGpuVendor() == GPU_VENDOR_NVIDIA && isNonPowerOfTwoTextureSupported() &&
               m_glRendererString.contains("GeForce FX", Qt::CaseInsensitive)) { //from http://www.opengl.org/wiki/NPOT_Texture
      m_isSupported = false;
      m_notSupportedReason = "NV30-based cards (GeForce FX of any kind) are incapable of NPOTs at all, despite implementing OpenGL 2.0 \
          (which requires NPOT). It will do software rendering if you try to use it. ";
      return;
    } else {
      m_isSupported = true;
    }

    // Prevent segfault
    const char* glslVS = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    if (glslVS)
      m_glslVersionString = QString(glslVS);
    else
      m_glslVersionString = "";

    if (!parseVersionString(m_glVersionString, m_glMajorVersion, m_glMinorVersion, m_glReleaseVersion)) {
      LERROR() << "Malformed OpenGL version string:" << m_glVersionString;
    }

    // GPU Vendor
    if (m_glVendorString.contains("NVIDIA", Qt::CaseInsensitive))
      m_gpuVendor = GPU_VENDOR_NVIDIA;
    else if (m_glVendorString.contains("ATI", Qt::CaseInsensitive))
      m_gpuVendor = GPU_VENDOR_ATI;
    else if (m_glVendorString.contains("INTEL", Qt::CaseInsensitive))
      m_gpuVendor = GPU_VENDOR_INTEL;
    else {
      m_gpuVendor = GPU_VENDOR_UNKNOWN;
    }

    // Shaders
    if (!parseVersionString(m_glslVersionString, m_glslMajorVersion, m_glslMinorVersion,
                            m_glslReleaseVersion)) {
      LERROR() << "Malformed GLSL version string:" << m_glslVersionString;
    }

    m_maxGeometryOutputVertices = -1;
    if(isGeometryShaderSupported())
      glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &m_maxGeometryOutputVertices);

    // Texturing
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_maxTexureSize);
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &m_max3DTextureSize);
    // http://www.opengl.org/wiki/Textures_-_more
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_maxTextureImageUnits);
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &m_maxVertexTextureImageUnits);
    m_maxGeometryTextureImageUnits = -1;
    if (isGeometryShaderSupported())
      glGetIntegerv(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, &m_maxGeometryTextureImageUnits);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_maxCombinedTextureImageUnits);
    glGetIntegerv(GL_MAX_TEXTURE_COORDS, &m_maxTextureCoords);

    if (isTextureFilterAnisotropicSupported())
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_maxTextureAnisotropy);
    else
      m_maxTextureAnisotropy = 1.0;

    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &m_maxColorAttachments);
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &m_maxDrawBuffer);

    // Point
    GLfloat range[2];
    glGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, range);
    glGetFloatv(GL_SMOOTH_POINT_SIZE_GRANULARITY, &m_smoothPointSizeGranularity);
    m_minSmoothPointSize = range[0];
    m_maxSmoothPointSize = range[1];
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, range);
    m_minAliasedPointSize = range[0];
    m_maxAliasedPointSize = range[1];

    // Line
    glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, range);
    glGetFloatv(GL_SMOOTH_LINE_WIDTH_GRANULARITY, &m_smoothLineWidthGranularity);
    m_minSmoothLineWidth = range[0];
    m_maxSmoothLineWidth = range[1];
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, range);
    m_minAliasedLineWidth = range[0];
    m_maxAliasedLineWidth = range[1];

  } else {
    m_isSupported = false;
    m_notSupportedReason = "Minimum OpenGL version required is 2.1, while current openGL version is: \"" + m_glVersionString + "\"";
  }
}

// format "2.1[.1] otherstring"
bool Z3DGpuInfo::parseVersionString(const QString &versionString, int &major, int &minor, int &release)
{
  major = -1;
  minor = -1;
  release = -1;

  if (versionString.isEmpty())
    return false;

  QString str = versionString.mid(0, versionString.indexOf(" "));
  QStringList list = str.split(".");
  if (list.size() < 2 || list.size() > 3)
    return false;

  bool ok;
  major = list[0].toInt(&ok);
  if (!ok) {
    major = -1;
    return false;
  }

  minor = list[1].toInt(&ok);
  if (!ok) {
    major = -1;
    minor = -1;
    return false;
  }

  if (list.size() > 2) {
    release = list[2].toInt(&ok);
    if (!ok) {
      major = -1;
      minor = -1;
      release = -1;
      return false;
    }
  } else
    release = 0;

  return true;
}
