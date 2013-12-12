#include "z3dvolumesource.h"

#include "zstack.hxx"
#include "zstackdoc.h"
#include "tz_stack_attribute.h"
#include "z3dgpuinfo.h"

const size_t Z3DVolumeSource::m_nChannelSupport = 5;

Z3DVolumeSource::Z3DVolumeSource(ZStackDoc *doc)
  : Z3DProcessor()
  , m_stackOutputPort("Stack")
  , m_xScale("X Scale", 1.0f, 0.1f, 50.f)
  , m_yScale("Y Scale", 1.0f, 0.1f, 50.f)
  , m_zScale("Z Scale", 1.0f, 0.1f, 500.f)
  , m_isVolumeDownsampled("Volume Is Downsampled", false)
  , m_isSubVolume("Is Subvolume", false)
  , m_zoomInViewSize("Zoom In View Size", 256, 128, 512)
  , m_doc(doc)
  , m_widgetsGroup(NULL)
{
  int currentAvailableTexMem = Z3DGpuInfoInstance.getAvailableTextureMemory();
  if (currentAvailableTexMem != -1 && currentAvailableTexMem <= 256000)
    m_maxVoxelNumber = 256 * 256 * 256 * 2;
  else
    m_maxVoxelNumber = 512 * 512 * 512 * 1;

  for (size_t i=0; i<m_nChannelSupport; i++) {
    QString name = QString("Volume%1").arg(i+1);
    m_outputPorts.push_back(new Z3DOutputPort<Z3DVolume>(name));
    addPort(m_outputPorts[i]);
  }
  addPort(m_stackOutputPort);

  if (m_doc != NULL) {
    if (m_doc->hasStackData() && m_doc->stack()->channelNumber() > 0) {
      readVolumes();
    }
  }

  addParameter(m_xScale);
  addParameter(m_yScale);
  addParameter(m_zScale);
  m_isVolumeDownsampled.setEnabled(false);
  addParameter(m_isVolumeDownsampled);
  m_isSubVolume.setEnabled(false);
  addParameter(m_isSubVolume);
  m_zoomInViewSize.setTracking(false);
  m_zoomInViewSize.setSingleStep(32);
  addParameter(m_zoomInViewSize);
  connect(&m_xScale, SIGNAL(valueChanged()), this, SLOT(changeXScale()));
  connect(&m_yScale, SIGNAL(valueChanged()), this, SLOT(changeYScale()));
  connect(&m_zScale, SIGNAL(valueChanged()), this, SLOT(changeZScale()));
  connect(&m_zoomInViewSize, SIGNAL(valueChanged()), this, SLOT(changeZoomInViewSize()));
}

Z3DVolumeSource::~Z3DVolumeSource()
{
  for (size_t i=0; i<m_outputPorts.size(); i++)
    delete m_outputPorts[i];
}

void Z3DVolumeSource::process(Z3DEye) {}

void Z3DVolumeSource::initialize()
{
  Z3DProcessor::initialize();
  sendData();
  CHECK_GL_ERROR;
}

void Z3DVolumeSource::deinitialize()
{
  clearVolume();
  clearZoomInVolume();
  CHECK_GL_ERROR;
  Z3DProcessor::deinitialize();
}

void Z3DVolumeSource::readVolumes()
{
  if (m_doc == NULL) {
    return;
  }

  clearVolume();
  int nchannel = m_doc->hasStackData() ? m_doc->stack()->channelNumber() : 0;
  if (nchannel > 0) {
    for (int i=0; i<nchannel; i++) {
      Stack *stack = m_doc->stack()->c_stack(i);
      if (m_doc->stack()->getVoxelNumber() * nchannel > m_maxVoxelNumber) { //Downsample big stack
        m_isVolumeDownsampled.set(true);
        double scale = std::sqrt((m_maxVoxelNumber*1.0) /
                                 (m_doc->stack()->getVoxelNumber() * nchannel));
        int height = (int)(stack->height * scale);
        int width = (int)(stack->width * scale);
        int depth = stack->depth;
        double widthScale = 1.0;
        double heightScale = 1.0;
        double depthScale = 1.0;
        int maxTextureSize = 100;
        if (stack->depth > 1)
          maxTextureSize = Z3DGpuInfoInstance.getMax3DTextureSize();
        else
          maxTextureSize = Z3DGpuInfoInstance.getMaxTextureSize();

        if (height > maxTextureSize) {
          heightScale = (double)maxTextureSize / height;
          height = std::floor(height * heightScale);
        }
        if (width > maxTextureSize) {
          widthScale = (double)maxTextureSize / width;
          width = std::floor(width * widthScale);
        }
        if (depth > maxTextureSize) {
          depthScale = (double)maxTextureSize / depth;
          depth = std::floor(depth * depthScale);
        }

        widthScale *= scale;
        heightScale *= scale;

        Stack *stack2 = Resize_Stack(stack, width, height, depth);
        Translate_Stack(stack2, GREY, 1);

        if (m_doc->stack()->isBinary()) {
          size_t volume = Stack_Voxel_Number(stack2);
          for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
            if (stack2->array[voxelIndex] == 1) {
              stack2->array[voxelIndex] = 255;
            }
          }
        }

        Z3DVolume *vh = new Z3DVolume(stack2,
                                      glm::vec3(1.f/widthScale, 1.f/heightScale, 1.f/depthScale),
                                      glm::vec3(m_xScale.get(),
                                                m_yScale.get(),
                                                m_zScale.get()),
                                      glm::vec3(.0));

        m_volumes.push_back(vh);
      } else { //small stack
        double widthScale = 1.0;
        double heightScale = 1.0;
        double depthScale = 1.0;
        int height = C_Stack::height(stack);
        int width = C_Stack::width(stack);
        int depth = C_Stack::depth(stack);
        int maxTextureSize = 100;
        if (stack->depth > 1)
          maxTextureSize = Z3DGpuInfoInstance.getMax3DTextureSize();
        else
          maxTextureSize = Z3DGpuInfoInstance.getMaxTextureSize();

        if (height > maxTextureSize) {
          heightScale = (double)maxTextureSize / height;
          height = std::floor(height * heightScale);
        }
        if (width > maxTextureSize) {
          widthScale = (double)maxTextureSize / width;
          width = std::floor(width * widthScale);
        }
        if (depth > maxTextureSize) {
          depthScale = (double)maxTextureSize / depth;
          depth = std::floor(depth * depthScale);
        }
        Stack *stack2;
        if (widthScale != 1.0 || heightScale != 1.0)
          stack2 = C_Stack::resize(stack, width, height, depth);
        else
          stack2 = Copy_Stack(stack);

        if (stack->kind == GREY && m_doc->stack()->isBinary()) {
          size_t volume = m_doc->stack()->getVoxelNumber();
          for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
            if (stack2->array[voxelIndex] == 1) {
              stack2->array[voxelIndex] = 255;
            }
          }
        }

        Translate_Stack(stack2, GREY, 1);

        Z3DVolume *vh = new Z3DVolume(stack2,
                                      glm::vec3(1.f/widthScale, 1.f/heightScale, 1.f/depthScale),
                                      glm::vec3(m_xScale.get(),
                                                m_yScale.get(),
                                                m_zScale.get()),
                                      glm::vec3(.0));

        m_volumes.push_back(vh);

      }
    } //for each cannel

    std::vector<ZVec3Parameter*>& chCols = m_doc->stack()->channelColors();
    for (int i=0; i<nchannel; i++) {
      m_volumes[i]->setVolColor(chCols[i]->get());
    }
  }
}

void Z3DVolumeSource::readSubVolumes(int left, int top, int front, int width,
                                     int height, int depth)
{
  if (m_doc == NULL) {
    return;
  }

  clearZoomInVolume();
  int nchannel = m_doc->hasStackData() ? m_doc->stack()->channelNumber() : 0;
  if (nchannel > 0) {
    glm::vec3 scaleSpacing = glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get());
    glm::vec3 downsampleSpacing = glm::vec3(1.f, 1.f, 1.f);
    glm::vec3 offset = glm::vec3(left, top, front) * scaleSpacing + getVolume(0)->getOffset();
    for (int i=0; i<nchannel; i++) {
      Stack *stack = m_doc->stack()->c_stack(i);
      Stack *subStack = Crop_Stack(stack, left, top, front, width, height, depth, NULL);
      if (subStack->kind == GREY) {
        Z3DVolume *vh = new Z3DVolume(subStack, downsampleSpacing, scaleSpacing, offset,
                                      getVolume(0)->getPhysicalToWorldMatrix());
        vh->setParentVolumeDimensions(glm::svec3(stack->width, stack->height, stack->depth));
        vh->setParentVolumeOffset(getVolume(0)->getOffset());
        m_zoomInVolumes.push_back(vh);
      } else {
        Translate_Stack(subStack, GREY, 1);
        Z3DVolume *vh = new Z3DVolume(subStack, downsampleSpacing, scaleSpacing, offset,
                                      getVolume(0)->getPhysicalToWorldMatrix());
        vh->setParentVolumeDimensions(glm::svec3(stack->width, stack->height, stack->depth));
        vh->setParentVolumeOffset(getVolume(0)->getOffset());
        m_zoomInVolumes.push_back(vh);
      }
    }

    std::vector<ZVec3Parameter*>& chCols = m_doc->stack()->channelColors();
    for (int i=0; i<nchannel; i++) {
      m_zoomInVolumes[i]->setVolColor(chCols[i]->get());
    }
  }
}

void Z3DVolumeSource::sendData()
{
  for (size_t i=0; i<m_volumes.size(); i++) {
    if (i < m_nChannelSupport) {
      m_outputPorts[i]->setData(m_volumes[i], false);
    }
  }
  for (size_t i=m_volumes.size(); i<m_outputPorts.size(); i++) {
    m_outputPorts[i]->setData(NULL);
  }
  if (m_volumes.size() > 0) {
    m_stackOutputPort.setData(m_doc->stack());
  }
}

void Z3DVolumeSource::sendZoomInVolumeData()
{
  if (m_doc == NULL) {
    return;
  }

  for (size_t i=0; i<m_zoomInVolumes.size(); i++) {
    if (i < m_nChannelSupport) {
      m_outputPorts[i]->setData(m_zoomInVolumes[i], false);
    }
  }
  for (size_t i=m_zoomInVolumes.size(); i<m_outputPorts.size(); i++) {
    m_outputPorts[i]->setData(NULL);
  }
  if (m_volumes.size() > 0) {
    m_stackOutputPort.setData(m_doc->stack());
  }
}

void Z3DVolumeSource::changeXScale()
{
  if (m_volumes.empty())
    return;
  for (size_t i=0; i<m_volumes.size(); i++) {
    m_volumes[i]->setScaleSpacing(glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()));
  }
  for (size_t i=0; i<m_zoomInVolumes.size(); i++) {
    m_zoomInVolumes[i]->setScaleSpacing(glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()));
  }
  emit xScaleChanged();
}

void Z3DVolumeSource::changeYScale()
{
  if (m_volumes.empty())
    return;
  for (size_t i=0; i<m_volumes.size(); i++) {
    m_volumes[i]->setScaleSpacing(glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()));
  }
  for (size_t i=0; i<m_zoomInVolumes.size(); i++) {
    m_zoomInVolumes[i]->setScaleSpacing(glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()));
  }
  emit yScaleChanged();
}

void Z3DVolumeSource::changeZScale()
{
  if (m_volumes.empty())
    return;
  for (size_t i=0; i<m_volumes.size(); i++) {
    m_volumes[i]->setScaleSpacing(glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()));
  }
  for (size_t i=0; i<m_zoomInVolumes.size(); i++) {
    m_zoomInVolumes[i]->setScaleSpacing(glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()));
  }
  emit zScaleChanged();
}

void Z3DVolumeSource::changeZoomInViewSize()
{
  if (m_zoomInVolumes.empty())
    return;
  exitZoomInView();
  openZoomInView(m_zoomInPos);
}

void Z3DVolumeSource::exitZoomInView()
{
  if (m_zoomInVolumes.empty())
    return;

  // copy transform matrix from sub volume, in case it is changed
  for (size_t i=0; i<m_volumes.size(); i++) {
    m_volumes[i]->setPhysicalToWorldMatrix(m_zoomInVolumes[i]->getPhysicalToWorldMatrix());
  }
  clearZoomInVolume();
  sendData();
  m_isSubVolume.set(false);
  m_isVolumeDownsampled.set(true);
}

void Z3DVolumeSource::clearVolume()
{
  for (size_t i=0; i<m_volumes.size(); i++) {
    delete m_volumes[i];
  }
  m_volumes.clear();
}

void Z3DVolumeSource::clearZoomInVolume()
{
  for (size_t i=0; i<m_zoomInVolumes.size(); i++) {
    delete m_zoomInVolumes[i];
  }
  m_zoomInVolumes.clear();
}

void Z3DVolumeSource::reloadVolume()
{
  if (!isInitialized())
    return;

  clearVolume();
  clearZoomInVolume();
  readVolumes();
  sendData();
}

bool Z3DVolumeSource::openZoomInView(const glm::ivec3& volPos)
{
  if (m_doc == NULL) {
    return false;
  }

  if (!m_isVolumeDownsampled.get())
    return false;
  if (!m_doc->hasStackData())
    return false;
  if (!volumeNeedDownsample())
    return false;
  if (getVolume(0) == NULL)
    return false;
  glm::ivec3 voldim = glm::ivec3(getVolume(0)->getCubeSize());
  if (!(volPos[0] >= 0 && volPos[0] < voldim.x  && volPos[1] >= 0 && volPos[1] < voldim.y && volPos[2] >= 0 && volPos[2] < voldim.z))
    return false;

  glm::vec3 offset = getVolume(0)->getOffset();
  m_zoomInPos = volPos;
  if (m_zoomInViewSize.get() % 2 != 0)
    m_zoomInViewSize.set(m_zoomInViewSize.get()+1);
  int halfsize = m_zoomInViewSize.get() / 2;
  int left = std::max(volPos[0]-halfsize+1, 0);
  int right = std::min(volPos[0]+halfsize, m_doc->stack()->width()-1);
  int width = right - left + 1;
  int up = std::max(volPos[1]-halfsize+1, 0);
  int down = std::min(volPos[1]+halfsize, m_doc->stack()->height()-1);
  int height = down - up + 1;
  int front = 0;
  int depth = m_doc->stack()->depth();
  m_zoomInBound.clear();
  m_zoomInBound.push_back(left*m_xScale.get() + offset.x);
  m_zoomInBound.push_back(right*m_xScale.get() + offset.x);
  m_zoomInBound.push_back(up*m_yScale.get() + offset.y);
  m_zoomInBound.push_back(down*m_yScale.get() + offset.y);
  m_zoomInBound.push_back(front*m_zScale.get() + offset.z);
  m_zoomInBound.push_back(depth*m_zScale.get() + offset.z);
  readSubVolumes(left, up, front, width, height, depth);
  sendZoomInVolumeData();

  m_isSubVolume.set(true);
  m_isVolumeDownsampled.set(false);
  return true;
}

Z3DVolume *Z3DVolumeSource::getVolume(size_t index)
{
  if (index < m_volumes.size())
    return m_volumes[index];
  else
    return NULL;
}

bool Z3DVolumeSource::isEmpty()
{
  return m_volumes.empty();
}

bool Z3DVolumeSource::volumeNeedDownsample() const
{
  if (m_doc == NULL) {
    return false;
  }

  if (m_doc->stack()->getVoxelNumber() * m_doc->stack()->channelNumber() <= m_maxVoxelNumber)
    return false;
  else
    return true;
}

bool Z3DVolumeSource::isVolumeDownsampled() const
{
  return m_isVolumeDownsampled.get();
}

ZWidgetsGroup *Z3DVolumeSource::getWidgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = new ZWidgetsGroup("Volume Source", NULL, 1);
    new ZWidgetsGroup(&m_zScale, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_xScale, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_yScale, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_isVolumeDownsampled, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_isSubVolume, m_widgetsGroup, 1);
    new ZWidgetsGroup(&m_zoomInViewSize, m_widgetsGroup, 1);
    m_widgetsGroup->setBasicAdvancedCutoff(4);
  }
  return m_widgetsGroup;
}
