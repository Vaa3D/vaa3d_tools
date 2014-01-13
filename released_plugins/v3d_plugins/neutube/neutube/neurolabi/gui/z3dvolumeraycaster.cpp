#include "z3dvolumeraycaster.h"

#include "z3dvolumeraycasterrenderer.h"
#include "z3dvolumeslicerenderer.h"
#include "z3dtexturecoordinaterenderer.h"
#include "z3dtexturecopyrenderer.h"
#include "z3dimage2drenderer.h"
#include "z3dlinerenderer.h"
#include "z3dgpuinfo.h"
#include "zeventlistenerparameter.h"
#include "z3dmesh.h"
#include "QsLog.h"
#include "zbenchtimer.h"
#include "z3dutils.h"

const size_t Z3DVolumeRaycaster::m_maxNumOfFullResolutionVolumeSlice = 6;

Z3DVolumeRaycaster::Z3DVolumeRaycaster()
  : Z3DRenderProcessor()
  , m_interactionDownsample("Interaction Downsample", 1, 1, 16)
  , m_volumes("Volumes", true)
  , m_stackInputPort("Stack")
  , m_entryPort("ImageEntryPoints")
  , m_exitPort("ImageExitPoints")
  , m_tmpPort("Tmp")
  , m_outport("Image", true, InvalidMonoViewResult)
  , m_leftEyeOutport("LeftEyeImage", true, InvalidLeftEyeResult)
  , m_rightEyeOutport("RightEyeImage", true, InvalidRightEyeResult)
  , m_volumeRaycasterRenderer(NULL)
  , m_volumeSliceRenderer(NULL)
  , m_textureCoordinateRenderer(NULL)
  , m_textureCopyRenderer(NULL)
  , m_boundBoxRenderer(NULL)
  , m_FRVolumeSlices(m_maxNumOfFullResolutionVolumeSlice)
  , m_FRVolumeSlicesValidState(m_maxNumOfFullResolutionVolumeSlice, false)
  , m_useFRVolumeSlice("Use Full Resolution Volume Slice", true)
  , m_showXSlice("Show X Slice", false)
  , m_xSlicePosition("X Slice Position", 0, 0, 1)
  , m_showYSlice("Show Y Slice", false)
  , m_ySlicePosition("Y Slice Position", 0, 0, 1)
  , m_showZSlice("Show Z Slice", false)
  , m_zSlicePosition("Z Slice Position", 0, 0, 1)
  , m_showXSlice2("Show X Slice 2", false)
  , m_xSlice2Position("X Slice 2 Position", 0, 0, 1)
  , m_showYSlice2("Show Y Slice 2", false)
  , m_ySlice2Position("Y Slice 2 Position", 0, 0, 1)
  , m_showZSlice2("Show Z Slice 2", false)
  , m_zSlice2Position("Z Slice 2 Position", 0, 0, 1)
  , m_showBoundBox("Show Bound Box", true)
  , m_boundBoxLineWidth("Bound Box Line Width", 2, 1, 5)
  , m_boundBoxLineColor("Bound Box Line Color", glm::vec4(0.f, 1.f, 1.f, 1.f))
  , m_camera("Camera", Z3DCamera())
  , m_widgetsGroup(NULL)
  , m_xCut("X Cut", glm::ivec2(0, 0), 0, 0)
  , m_yCut("Y Cut", glm::ivec2(0, 0), 0, 0)
  , m_zCut("Z Cut", glm::ivec2(0, 0), 0, 0)
  , m_2DImageQuad(GL_TRIANGLE_STRIP)
{
  addParameter(m_interactionDownsample);

  // ports
  addPort(m_volumes);
  addPort(m_stackInputPort);
  addPrivateRenderPort(m_entryPort);
  addPrivateRenderPort(m_exitPort);
  addPrivateRenderPort(m_tmpPort);
  addPort(m_outport);
  addPort(m_leftEyeOutport);
  addPort(m_rightEyeOutport);

  addParameter(m_camera);

  addParameter(m_xCut);
  addParameter(m_yCut);
  addParameter(m_zCut);

  addParameter(m_useFRVolumeSlice);
  addParameter(m_showXSlice);
  addParameter(m_xSlicePosition);
  addParameter(m_showYSlice);
  addParameter(m_ySlicePosition);
  addParameter(m_showZSlice);
  addParameter(m_zSlicePosition);
  addParameter(m_showXSlice2);
  addParameter(m_xSlice2Position);
  addParameter(m_showYSlice2);
  addParameter(m_ySlice2Position);
  addParameter(m_showZSlice2);
  addParameter(m_zSlice2Position);
  addParameter(m_showBoundBox);
  m_boundBoxLineColor.setStyle("COLOR");
  addParameter(m_boundBoxLineWidth);
  addParameter(m_boundBoxLineColor);

  connect(&m_boundBoxLineColor, SIGNAL(valueChanged()), this, SLOT(updateBoundBoxLineColors()));
  connect(&m_showXSlice, SIGNAL(valueChanged()), this, SLOT(adjustWidget()));
  connect(&m_showYSlice, SIGNAL(valueChanged()), this, SLOT(adjustWidget()));
  connect(&m_showZSlice, SIGNAL(valueChanged()), this, SLOT(adjustWidget()));
  connect(&m_showXSlice2, SIGNAL(valueChanged()), this, SLOT(adjustWidget()));
  connect(&m_showYSlice2, SIGNAL(valueChanged()), this, SLOT(adjustWidget()));
  connect(&m_showZSlice2, SIGNAL(valueChanged()), this, SLOT(adjustWidget()));
  connect(&m_showBoundBox, SIGNAL(valueChanged()), this, SLOT(adjustWidget()));

  connect(&m_xSlicePosition, SIGNAL(valueChanged()), this, SLOT(invalidateFRVolumeXSlice()));
  connect(&m_ySlicePosition, SIGNAL(valueChanged()), this, SLOT(invalidateFRVolumeYSlice()));
  connect(&m_zSlicePosition, SIGNAL(valueChanged()), this, SLOT(invalidateFRVolumeZSlice()));
  connect(&m_xSlice2Position, SIGNAL(valueChanged()), this, SLOT(invalidateFRVolumeXSlice2()));
  connect(&m_ySlice2Position, SIGNAL(valueChanged()), this, SLOT(invalidateFRVolumeXSlice2()));
  connect(&m_zSlice2Position, SIGNAL(valueChanged()), this, SLOT(invalidateFRVolumeXSlice2()));

  m_leftMouseButtonPressEvent = new ZEventListenerParameter("Left Mouse Button Pressed", true, false, this);
  m_leftMouseButtonPressEvent->listenTo("trace", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonPress);
  m_leftMouseButtonPressEvent->listenTo("trace", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonRelease);
  connect(m_leftMouseButtonPressEvent, SIGNAL(mouseEventTriggered(QMouseEvent*,int,int)),
          this, SLOT(leftMouseButtonPressed(QMouseEvent*,int,int)));
  addEventListener(m_leftMouseButtonPressEvent);
}

Z3DVolumeRaycaster::~Z3DVolumeRaycaster()
{
  delete m_leftMouseButtonPressEvent;
}

void Z3DVolumeRaycaster::enterInteractionMode()
{
  glm::ivec2 expectedSize = m_outport.getExpectedSize();
  if (m_interactionDownsample.get() != 1) {
    const std::vector<Z3DOutputPortBase*> outports = getOutputPorts();
    for(size_t i=0; i<outports.size(); ++i) {
      Z3DRenderOutputPort* rp = dynamic_cast<Z3DRenderOutputPort*>(outports[i]);
      if (rp)
        rp->resize(expectedSize / m_interactionDownsample.get());
    }

    const std::vector<Z3DRenderOutputPort*> pports = getPrivateRenderPorts();
    for (size_t i=0; i<pports.size(); ++i) {
      Z3DRenderOutputPort* rp = pports[i];
      rp->resize(expectedSize / m_interactionDownsample.get());
    }

    const std::vector<Z3DInputPortBase*> inports = getInputPorts();
    for(size_t i=0; i<inports.size(); ++i) {
      Z3DRenderInputPort* rp = dynamic_cast<Z3DRenderInputPort*>(inports[i]);
      if (rp)
        rp->setExpectedSize(expectedSize / m_interactionDownsample.get());
    }
    emit requestUpstreamSizeChange(this);

    // upstream will invalidate the network, but in case there are no upstream
    // do one more invalidation
    invalidateResult();
  }
}

void Z3DVolumeRaycaster::exitInteractionMode()
{
  glm::ivec2 expectedSize = m_outport.getExpectedSize();
  if (m_interactionDownsample.get() != 1) {
    const std::vector<Z3DOutputPortBase*> outports = getOutputPorts();
    for(size_t i=0; i<outports.size(); ++i) {
      Z3DRenderOutputPort* rp = dynamic_cast<Z3DRenderOutputPort*>(outports[i]);
      if (rp)
        rp->resize(expectedSize);
    }

    const std::vector<Z3DRenderOutputPort*> pports = getPrivateRenderPorts();
    for (size_t i=0; i<pports.size(); ++i) {
      Z3DRenderOutputPort* rp = pports[i];
      rp->resize(expectedSize);
    }

    const std::vector<Z3DInputPortBase*> inports = getInputPorts();
    for(size_t i=0; i<inports.size(); ++i) {
      Z3DRenderInputPort* rp = dynamic_cast<Z3DRenderInputPort*>(inports[i]);
      if (rp)
        rp->setExpectedSize(expectedSize);
    }
    emit requestUpstreamSizeChange(this);

    // upstream will invalidate the network, but in case there are no upstream
    // do one more invalidation
    invalidateResult();
  }
}

void Z3DVolumeRaycaster::initialize()
{
  Z3DRenderProcessor::initialize();

  m_volumeRaycasterRenderer = new Z3DVolumeRaycasterRenderer();
  m_rendererBase->addRenderer(m_volumeRaycasterRenderer);
  m_volumeSliceRenderer = new Z3DVolumeSliceRenderer();
  m_rendererBase->addRenderer(m_volumeSliceRenderer);
  m_textureCoordinateRenderer = new Z3DTextureCoordinateRenderer();
  m_rendererBase->addRenderer(m_textureCoordinateRenderer);
  m_textureCopyRenderer = new Z3DTextureCopyRenderer();
  m_rendererBase->addRenderer(m_textureCopyRenderer);
  m_textureCopyRenderer->setDiscardTransparent(true);
  for (size_t i=0; i<m_maxNumOfFullResolutionVolumeSlice; ++i) {
    m_image2DRenderers.push_back(new Z3DImage2DRenderer());
    m_rendererBase->addRenderer(m_image2DRenderers[i]);
  }
  m_boundBoxRenderer = new Z3DLineRenderer();
  m_boundBoxRenderer->setUseDisplayList(false);
  m_boundBoxRenderer->setRespectRendererBaseCoordScales(false);
  m_rendererBase->addRenderer(m_boundBoxRenderer);

  std::vector<ZParameter*> paras = m_rendererBase->getParameters();
  for (size_t i=0; i<paras.size(); i++) {
    //connect(paras[i], SIGNAL(valueChanged()), this, SLOT(invalidateResult()));
    addParameter(paras[i]);
  }

  adjustWidget();
  CHECK_GL_ERROR;
}

void Z3DVolumeRaycaster::deinitialize()
{
  std::vector<ZParameter*> paras = m_rendererBase->getParameters();
  for (size_t i=0; i<paras.size(); i++) {
    //paras[i]->disconnect(this);
    removeParameter(paras[i]);
  }
  clearFRVolumeSlices();
  CHECK_GL_ERROR;
  Z3DRenderProcessor::deinitialize();
}

bool Z3DVolumeRaycaster::isReady(Z3DEye eye) const
{
  if(m_volumes.isReady()) {
    if (eye == CenterEye && m_outport.isReady())
      return true;
    else if (eye == LeftEye && m_leftEyeOutport.isReady())
      return true;
    else if (eye == RightEye && m_rightEyeOutport.isReady())
      return true;
  }

  return false;
}

ZWidgetsGroup *Z3DVolumeRaycaster::getWidgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = new ZWidgetsGroup("Volume", NULL, 1);
    std::vector<ZParameter*> paras = getParameters();
    new ZWidgetsGroup(&m_xCut, m_widgetsGroup, 12);
    new ZWidgetsGroup(&m_yCut, m_widgetsGroup, 12);
    new ZWidgetsGroup(&m_zCut, m_widgetsGroup, 12);
    for (size_t i=0; i<paras.size(); i++) {
      ZParameter *para = paras[i];
      if (para->getName() == "Camera")
        continue;
      if (para->getName().contains("Slice") &&
          !para->getName().contains("2"))
        new ZWidgetsGroup(para, m_widgetsGroup, 11);
      else if (para->getName().contains("Bound Box"))
        new ZWidgetsGroup(para, m_widgetsGroup, 13);
      else
        new ZWidgetsGroup(para, m_widgetsGroup, 19);
    }
    paras = m_rendererBase->getParameters();
    for (size_t i=0; i<paras.size(); i++) {
      ZParameter *para = paras[i];
      if (para->getName().contains("Show Channel"))
        new ZWidgetsGroup(para, m_widgetsGroup, 1);
      else if (para->getName().contains("Transfer Function"))
        new ZWidgetsGroup(para, m_widgetsGroup, 2);
      else if (para->getName() == "Compositing")
        new ZWidgetsGroup(para, m_widgetsGroup, 9);
      else if (para->getName() == "ISO Value")
        new ZWidgetsGroup(para, m_widgetsGroup, 9);
      else if (para->getName() == "Local MIP Threshold")
        new ZWidgetsGroup(para, m_widgetsGroup, 9);
      else if (para->getName() == "Sampling Rate")
        new ZWidgetsGroup(para, m_widgetsGroup, 15);
      else if (para->getName().contains("Texture"))
        new ZWidgetsGroup(para, m_widgetsGroup, 15);
      //else
        //new ZWidgetsGroup(para, m_widgetsGroup, 19);
    }
    m_widgetsGroup->setBasicAdvancedCutoff(14);
  }
  return m_widgetsGroup;
}

glm::vec3 Z3DVolumeRaycaster::getFirstHit3DPosition(int x, int y, int width, int height,
                                                    bool &success)
{
  glm::vec3 res(-1);
  success = false;
  if (m_volumeRaycasterRenderer->hasVisibleRendering() &&
      (m_outport.hasValidData() || m_rightEyeOutport.hasValidData())) {
    glm::ivec2 pos2D = glm::ivec2(x, height - y);
    Z3DRenderOutputPort &port = m_outport.hasValidData() ? m_outport : m_rightEyeOutport;
    if (port.getSize() == port.getExpectedSize() / m_interactionDownsample.get()) {
      pos2D /= m_interactionDownsample.get();
      width /= m_interactionDownsample.get();
      height /= m_interactionDownsample.get();
    }
    glm::vec3 fpos3D = get3DPosition(pos2D, width, height, port);
    glm::vec3 pos3D = glm::round(glm::applyMatrix(m_volumes.getFirstValidData()->getWorldToPhysicalMatrix(), fpos3D));
    if (posIsInVolume(pos3D)) {
      res = pos3D;
      success = true;
    }
  }
  return res;
}

void Z3DVolumeRaycaster::process(Z3DEye eye)
{
  bool hasVol = m_volumes.isReady();
  if (!hasVol)
    return;

  glEnable(GL_DEPTH_TEST);

  Z3DVolume *volume = m_volumes.getFirstValidData();
  if (volume->is1DData())
    return;
  bool is2DImage = (volume->is2DData());

  if (m_volumes.hasChanged()) {
    glm::svec3 volDim = volume->getOriginalDimensions();
    m_xCut.setRange(0, volDim.x - 1);
    m_xCut.setLowerValue(0);
    m_xCut.setUpperValue(volDim.x - 1);
    m_yCut.setRange(0, volDim.y - 1);
    m_yCut.setLowerValue(0);
    m_yCut.setUpperValue(volDim.y - 1);
    m_zCut.setRange(0, volDim.z - 1);
    m_zCut.setLowerValue(0);
    m_zCut.setUpperValue(volDim.z - 1);
    m_zSlicePosition.setRange(0, volDim.z-1);
    m_ySlicePosition.setRange(0, volDim.y-1);
    m_xSlicePosition.setRange(0, volDim.x-1);
    m_zSlice2Position.setRange(0, volDim.z-1);
    m_ySlice2Position.setRange(0, volDim.y-1);
    m_xSlice2Position.setRange(0, volDim.x-1);
    invalidateAllFRVolumeSlices();
    if (is2DImage) {
      m_useFRVolumeSlice.set(false);
      m_useFRVolumeSlice.setVisible(false);
      m_showXSlice.set(false);
      m_showYSlice.set(false);
      m_showZSlice.set(false);
      m_showXSlice2.set(false);
      m_showYSlice2.set(false);
      m_showZSlice2.set(false);
      m_showXSlice.setVisible(false);
      m_showYSlice.setVisible(false);
      m_showZSlice.setVisible(false);
      m_showXSlice2.setVisible(false);
      m_showYSlice2.setVisible(false);
      m_showZSlice2.setVisible(false);
    }

    m_volumeRaycasterRenderer->setChannels(m_volumes.getAllValidData());
    if (!is2DImage)
      m_volumeSliceRenderer->setChannels(m_volumes.getAllValidData());

    // bound box
    m_boundBoxLines.clear();

    glm::vec3 p1 = volume->getParentVolWorldLUF();
    glm::vec3 p2 = volume->getParentVolWorldLDF();
    glm::vec3 p3 = volume->getParentVolWorldRDF();
    glm::vec3 p4 = volume->getParentVolWorldRUF();
    glm::vec3 p5 = volume->getParentVolWorldLUB();
    glm::vec3 p6 = volume->getParentVolWorldLDB();
    glm::vec3 p7 = volume->getParentVolWorldRDB();
    glm::vec3 p8 = volume->getParentVolWorldRUB();

    m_boundBoxLines.push_back(p1); m_boundBoxLines.push_back(p2);
    m_boundBoxLines.push_back(p2); m_boundBoxLines.push_back(p3);
    m_boundBoxLines.push_back(p3); m_boundBoxLines.push_back(p4);
    m_boundBoxLines.push_back(p4); m_boundBoxLines.push_back(p1);

    m_boundBoxLines.push_back(p5); m_boundBoxLines.push_back(p6);
    m_boundBoxLines.push_back(p6); m_boundBoxLines.push_back(p7);
    m_boundBoxLines.push_back(p7); m_boundBoxLines.push_back(p8);
    m_boundBoxLines.push_back(p8); m_boundBoxLines.push_back(p5);

    m_boundBoxLines.push_back(p1); m_boundBoxLines.push_back(p5);
    m_boundBoxLines.push_back(p2); m_boundBoxLines.push_back(p6);
    m_boundBoxLines.push_back(p3); m_boundBoxLines.push_back(p7);
    m_boundBoxLines.push_back(p4); m_boundBoxLines.push_back(p8);

    m_boundBoxRenderer->setData(&m_boundBoxLines);
    updateBoundBoxLineColors();
  }

  prepareDataForRaycaster(volume, eye);

  m_rendererBase->setCamera(m_camera.get());

  Z3DRenderOutputPort &currentOutport = (eye == CenterEye) ?
        m_outport : (eye == LeftEye) ? m_leftEyeOutport : m_rightEyeOutport;

  if (m_showBoundBox.get()) {
    m_tmpPort.resize(currentOutport.getSize() * 2);
    m_tmpPort.bindTarget();
    m_tmpPort.clearTarget();
    m_rendererBase->setViewport(m_tmpPort.getSize());
    m_boundBoxRenderer->setLineWidth(m_boundBoxLineWidth.get() * 2);
    m_rendererBase->activateRenderer(m_boundBoxRenderer);
    m_rendererBase->render(eye);
    m_tmpPort.releaseTarget();
  }

  currentOutport.bindTarget();
  currentOutport.clearTarget();
  m_rendererBase->setViewport(currentOutport.getSize());
  if (m_showBoundBox.get()) {
    m_textureCopyRenderer->setColorTexture(m_tmpPort.getColorTexture());
    m_textureCopyRenderer->setDepthTexture(m_tmpPort.getDepthTexture());
    m_rendererBase->activateRenderer(m_textureCopyRenderer);
    m_rendererBase->render(eye);
  }
  CHECK_GL_ERROR;

  if (m_volumeRaycasterRenderer->hasVisibleRendering()) {
    m_rendererBase->activateRenderer(m_volumeRaycasterRenderer);
    m_rendererBase->render(eye);
  }

  // renderVolumeSlice
  bool renderSlice = m_showZSlice.get() || m_showXSlice.get() || m_showYSlice.get()
      || m_showXSlice2.get() || m_showYSlice2.get() || m_showZSlice2.get();
  if (renderSlice) {
    glm::svec3 volDim = volume->getOriginalDimensions();
    glm::vec3 coordLuf = volume->getPhysicalLUF();
    glm::vec3 coordRdb = volume->getPhysicalRDB();

    if (m_useFRVolumeSlice.get() && volume->isDownsampledVolume()) {
      m_rendererBase->deactivateAllRenderers();

      ZStack *zstack = m_stackInputPort.getFirstValidData();
#ifdef _NEUTUBE_
      const std::vector<ZVec3Parameter*>& chCols = zstack->channelColors();
#endif
      int maxTextureSize = Z3DGpuInfoInstance.getMaxTextureSize();

      size_t sliceRendererIdx = 0;
      if (m_showZSlice.get()) {
        if (!m_FRVolumeSlicesValidState[sliceRendererIdx]) {
          m_image2DRenderers[sliceRendererIdx]->clearQuads();
          for (size_t c=0; c<m_FRVolumeSlices[sliceRendererIdx].size(); ++c)
            delete m_FRVolumeSlices[sliceRendererIdx][c];
          m_FRVolumeSlices[sliceRendererIdx].clear();

          float zTexCoord = m_zSlicePosition.get() / static_cast<float>(volDim.z-1);
          float zCoord = glm::mix(coordLuf.z, coordRdb.z, zTexCoord);

          for (int c=0; c<zstack->channelNumber(); ++c) {
            Stack* croped = C_Stack::crop(zstack->c_stack(c), 0, 0, m_zSlicePosition.get(),
                                          zstack->width(), zstack->height(), 1, NULL);
            if (croped->width > maxTextureSize || croped->height > maxTextureSize) {
              Stack* croped_1 = C_Stack::resize(croped, std::min(maxTextureSize, croped->width),
                                                std::min(maxTextureSize, croped->height), 1);
              C_Stack::kill(croped);
              croped = croped_1;
            }
            Z3DVolume *vh = new Z3DVolume(croped);
#ifdef _NEUTUBE_
            vh->setVolColor(chCols[c]->get());
#endif
            m_FRVolumeSlices[sliceRendererIdx].push_back(vh);
          }
          m_image2DRenderers[sliceRendererIdx]->setChannels(m_FRVolumeSlices[sliceRendererIdx]);

          Z3DTriangleList slice = Z3DTriangleList::createCubeSliceWith2DTexture(zCoord, 2, coordLuf.xy(), coordRdb.xy());
          slice.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
          m_image2DRenderers[sliceRendererIdx]->addQuad(slice);
          m_FRVolumeSlicesValidState[sliceRendererIdx] = true;
        }
        m_rendererBase->activateRenderer(m_image2DRenderers[sliceRendererIdx], Z3DRendererBase::None);
      }
      sliceRendererIdx = 1;
      if (m_showYSlice.get()) {
        if (!m_FRVolumeSlicesValidState[sliceRendererIdx]) {
          m_image2DRenderers[sliceRendererIdx]->clearQuads();
          for (size_t c=0; c<m_FRVolumeSlices[sliceRendererIdx].size(); ++c)
            delete m_FRVolumeSlices[sliceRendererIdx][c];
          m_FRVolumeSlices[sliceRendererIdx].clear();

          float yTexCoord = m_ySlicePosition.get() / static_cast<float>(volDim.y-1);
          float yCoord = glm::mix(coordLuf.y, coordRdb.y, yTexCoord);

          for (int c=0; c<zstack->channelNumber(); ++c) {
            Stack* croped = C_Stack::crop(zstack->c_stack(c), 0, m_ySlicePosition.get(), 0,
                                          zstack->width(), 1, zstack->depth(), NULL);
            croped->height = zstack->depth();
            croped->depth = 1;
            if (croped->width > maxTextureSize || croped->height > maxTextureSize) {
              Stack* croped_1 = C_Stack::resize(croped, std::min(maxTextureSize, croped->width),
                                                std::min(maxTextureSize, croped->height), 1);
              C_Stack::kill(croped);
              croped = croped_1;
            }
            Z3DVolume *vh = new Z3DVolume(croped);
#ifdef _NEUTUBE_
            vh->setVolColor(chCols[c]->get());
#endif
            m_FRVolumeSlices[sliceRendererIdx].push_back(vh);
          }
          m_image2DRenderers[sliceRendererIdx]->setChannels(m_FRVolumeSlices[sliceRendererIdx]);

          Z3DTriangleList slice = Z3DTriangleList::createCubeSliceWith2DTexture(yCoord, 1, coordLuf.xz(), coordRdb.xz());
          slice.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
          m_image2DRenderers[sliceRendererIdx]->addQuad(slice);
          m_FRVolumeSlicesValidState[sliceRendererIdx] = true;
        }
        m_rendererBase->activateRenderer(m_image2DRenderers[sliceRendererIdx], Z3DRendererBase::None);
      }
      sliceRendererIdx = 2;
      if (m_showXSlice.get()) {
        if (!m_FRVolumeSlicesValidState[sliceRendererIdx]) {
          m_image2DRenderers[sliceRendererIdx]->clearQuads();
          for (size_t c=0; c<m_FRVolumeSlices[sliceRendererIdx].size(); ++c)
            delete m_FRVolumeSlices[sliceRendererIdx][c];
          m_FRVolumeSlices[sliceRendererIdx].clear();

          float xTexCoord = m_xSlicePosition.get() / static_cast<float>(volDim.x-1);
          float xCoord = glm::mix(coordLuf.x, coordRdb.x, xTexCoord);

          for (int c=0; c<zstack->channelNumber(); ++c) {
            Stack* croped = C_Stack::crop(zstack->c_stack(c), m_xSlicePosition.get(), 0, 0,
                                          1, zstack->height(), zstack->depth(), NULL);
            croped->width = zstack->height();
            croped->height = zstack->depth();
            croped->depth = 1;
            if (croped->width > maxTextureSize || croped->height > maxTextureSize) {
              Stack* croped_1 = C_Stack::resize(croped, std::min(maxTextureSize, croped->width),
                                                std::min(maxTextureSize, croped->height), 1);
              C_Stack::kill(croped);
              croped = croped_1;
            }
            Z3DVolume *vh = new Z3DVolume(croped);
#ifdef _NEUTUBE_
            vh->setVolColor(chCols[c]->get());
#endif
            m_FRVolumeSlices[sliceRendererIdx].push_back(vh);
          }
          m_image2DRenderers[sliceRendererIdx]->setChannels(m_FRVolumeSlices[sliceRendererIdx]);

          Z3DTriangleList slice = Z3DTriangleList::createCubeSliceWith2DTexture(xCoord, 0, coordLuf.yz(), coordRdb.yz());
          slice.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
          m_image2DRenderers[sliceRendererIdx]->addQuad(slice);
          m_FRVolumeSlicesValidState[sliceRendererIdx] = true;
        }
        m_rendererBase->activateRenderer(m_image2DRenderers[sliceRendererIdx], Z3DRendererBase::None);
      }
      sliceRendererIdx = 3;
      if (m_showZSlice2.get()) {
        if (!m_FRVolumeSlicesValidState[sliceRendererIdx]) {
          m_image2DRenderers[sliceRendererIdx]->clearQuads();
          for (size_t c=0; c<m_FRVolumeSlices[sliceRendererIdx].size(); ++c)
            delete m_FRVolumeSlices[sliceRendererIdx][c];
          m_FRVolumeSlices[sliceRendererIdx].clear();

          float zTexCoord = m_zSlice2Position.get() / static_cast<float>(volDim.z-1);
          float zCoord = glm::mix(coordLuf.z, coordRdb.z, zTexCoord);

          for (int c=0; c<zstack->channelNumber(); ++c) {
            Stack* croped = C_Stack::crop(zstack->c_stack(c), 0, 0, m_zSlice2Position.get(),
                                          zstack->width(), zstack->height(), 1, NULL);
            if (croped->width > maxTextureSize || croped->height > maxTextureSize) {
              Stack* croped_1 = C_Stack::resize(croped, std::min(maxTextureSize, croped->width),
                                                std::min(maxTextureSize, croped->height), 1);
              C_Stack::kill(croped);
              croped = croped_1;
            }
            Z3DVolume *vh = new Z3DVolume(croped);
#ifdef _NEUTUBE_
            vh->setVolColor(chCols[c]->get());
#endif
            m_FRVolumeSlices[sliceRendererIdx].push_back(vh);
          }
          m_image2DRenderers[sliceRendererIdx]->setChannels(m_FRVolumeSlices[sliceRendererIdx]);

          Z3DTriangleList slice = Z3DTriangleList::createCubeSliceWith2DTexture(zCoord, 2, coordLuf.xy(), coordRdb.xy());
          slice.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
          m_image2DRenderers[sliceRendererIdx]->addQuad(slice);
          m_FRVolumeSlicesValidState[sliceRendererIdx] = true;
        }
        m_rendererBase->activateRenderer(m_image2DRenderers[sliceRendererIdx], Z3DRendererBase::None);
      }
      sliceRendererIdx = 4;
      if (m_showYSlice2.get()) {
        if (!m_FRVolumeSlicesValidState[sliceRendererIdx]) {
          m_image2DRenderers[sliceRendererIdx]->clearQuads();
          for (size_t c=0; c<m_FRVolumeSlices[sliceRendererIdx].size(); ++c)
            delete m_FRVolumeSlices[sliceRendererIdx][c];
          m_FRVolumeSlices[sliceRendererIdx].clear();

          float yTexCoord = m_ySlice2Position.get() / static_cast<float>(volDim.y-1);
          float yCoord = glm::mix(coordLuf.y, coordRdb.y, yTexCoord);

          for (int c=0; c<zstack->channelNumber(); ++c) {
            Stack* croped = C_Stack::crop(zstack->c_stack(c), 0, m_ySlice2Position.get(), 0,
                                          zstack->width(), 1, zstack->depth(), NULL);
            croped->height = zstack->depth();
            croped->depth = 1;
            if (croped->width > maxTextureSize || croped->height > maxTextureSize) {
              Stack* croped_1 = C_Stack::resize(croped, std::min(maxTextureSize, croped->width),
                                                std::min(maxTextureSize, croped->height), 1);
              C_Stack::kill(croped);
              croped = croped_1;
            }
            Z3DVolume *vh = new Z3DVolume(croped);
#ifdef _NEUTUBE_
            vh->setVolColor(chCols[c]->get());
#endif
            m_FRVolumeSlices[sliceRendererIdx].push_back(vh);
          }
          m_image2DRenderers[sliceRendererIdx]->setChannels(m_FRVolumeSlices[sliceRendererIdx]);

          Z3DTriangleList slice = Z3DTriangleList::createCubeSliceWith2DTexture(yCoord, 1, coordLuf.xz(), coordRdb.xz());
          slice.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
          m_image2DRenderers[sliceRendererIdx]->addQuad(slice);
          m_FRVolumeSlicesValidState[sliceRendererIdx] = true;
        }
        m_rendererBase->activateRenderer(m_image2DRenderers[sliceRendererIdx], Z3DRendererBase::None);
      }
      sliceRendererIdx = 5;
      if (m_showXSlice2.get()) {
        if (!m_FRVolumeSlicesValidState[sliceRendererIdx]) {
          m_image2DRenderers[sliceRendererIdx]->clearQuads();
          for (size_t c=0; c<m_FRVolumeSlices[sliceRendererIdx].size(); ++c)
            delete m_FRVolumeSlices[sliceRendererIdx][c];
          m_FRVolumeSlices[sliceRendererIdx].clear();

          float xTexCoord = m_xSlice2Position.get() / static_cast<float>(volDim.x-1);
          float xCoord = glm::mix(coordLuf.x, coordRdb.x, xTexCoord);

          for (int c=0; c<zstack->channelNumber(); ++c) {
            Stack* croped = C_Stack::crop(zstack->c_stack(c), m_xSlice2Position.get(), 0, 0,
                                          1, zstack->height(), zstack->depth(), NULL);
            croped->width = zstack->height();
            croped->height = zstack->depth();
            croped->depth = 1;
            if (croped->width > maxTextureSize || croped->height > maxTextureSize) {
              Stack* croped_1 = C_Stack::resize(croped, std::min(maxTextureSize, croped->width),
                                                std::min(maxTextureSize, croped->height), 1);
              C_Stack::kill(croped);
              croped = croped_1;
            }
            Z3DVolume *vh = new Z3DVolume(croped);
#ifdef _NEUTUBE_
            vh->setVolColor(chCols[c]->get());
#endif
            m_FRVolumeSlices[sliceRendererIdx].push_back(vh);
          }
          m_image2DRenderers[sliceRendererIdx]->setChannels(m_FRVolumeSlices[sliceRendererIdx]);

          Z3DTriangleList slice = Z3DTriangleList::createCubeSliceWith2DTexture(xCoord, 0, coordLuf.yz(), coordRdb.yz());
          slice.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
          m_image2DRenderers[sliceRendererIdx]->addQuad(slice);
          m_FRVolumeSlicesValidState[sliceRendererIdx] = true;
        }
        m_rendererBase->activateRenderer(m_image2DRenderers[sliceRendererIdx], Z3DRendererBase::None);
      }
      m_rendererBase->render(eye);

    } else {

      m_volumeSliceRenderer->clearQuads();

      if (m_showZSlice.get()) {

#ifdef _FLYEM_2
        for (int z = 0; z < 3; ++z) {
          float zTexCoord = z / static_cast<float>(volDim.z - 1);
          float zCoord =  glm::mix(coordLuf.z, coordRdb.z, zTexCoord);

          Z3DTriangleList slice = Z3DTriangleList::createCubeSlice(
                zCoord, zTexCoord, 2, coordLuf.xy(), coordRdb.xy());
          slice.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
          m_volumeSliceRenderer->addQuad(slice);
        }
#else
        float zTexCoord = m_zSlicePosition.get() / static_cast<float>(volDim.z-1);
        float zCoord = glm::mix(coordLuf.z, coordRdb.z, zTexCoord);

        Z3DTriangleList slice = Z3DTriangleList::createCubeSlice(zCoord, zTexCoord, 2, coordLuf.xy(), coordRdb.xy());
        slice.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
        m_volumeSliceRenderer->addQuad(slice);
#endif
      }
      if (m_showYSlice.get()) {
        float yTexCoord = m_ySlicePosition.get() / static_cast<float>(volDim.y-1);
        float yCoord = glm::mix(coordLuf.y, coordRdb.y, yTexCoord);

        Z3DTriangleList slice = Z3DTriangleList::createCubeSlice(yCoord, yTexCoord, 1, coordLuf.xz(), coordRdb.xz());
        slice.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
        m_volumeSliceRenderer->addQuad(slice);
      }
      if (m_showXSlice.get()) {
        float xTexCoord = m_xSlicePosition.get() / static_cast<float>(volDim.x-1);
        float xCoord = glm::mix(coordLuf.x, coordRdb.x, xTexCoord);

        Z3DTriangleList slice = Z3DTriangleList::createCubeSlice(xCoord, xTexCoord, 0, coordLuf.yz(), coordRdb.yz());
        slice.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
        m_volumeSliceRenderer->addQuad(slice);
      }

      if (m_showZSlice2.get()) {
        float zTexCoord = m_zSlice2Position.get() / static_cast<float>(volDim.z-1);
        float zCoord = glm::mix(coordLuf.z, coordRdb.z, zTexCoord);

        Z3DTriangleList slice = Z3DTriangleList::createCubeSlice(zCoord, zTexCoord, 2, coordLuf.xy(), coordRdb.xy());
        slice.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
        m_volumeSliceRenderer->addQuad(slice);
      }
      if (m_showYSlice2.get()) {
        float yTexCoord = m_ySlice2Position.get() / static_cast<float>(volDim.y-1);
        float yCoord = glm::mix(coordLuf.y, coordRdb.y, yTexCoord);

        Z3DTriangleList slice = Z3DTriangleList::createCubeSlice(yCoord, yTexCoord, 1, coordLuf.xz(), coordRdb.xz());
        slice.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
        m_volumeSliceRenderer->addQuad(slice);
      }
      if (m_showXSlice2.get()) {
        float xTexCoord = m_xSlice2Position.get() / static_cast<float>(volDim.x-1);
        float xCoord = glm::mix(coordLuf.x, coordRdb.x, xTexCoord);

        Z3DTriangleList slice = Z3DTriangleList::createCubeSlice(xCoord, xTexCoord, 0, coordLuf.yz(), coordRdb.yz());
        slice.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
        m_volumeSliceRenderer->addQuad(slice);
      }
      m_rendererBase->activateRenderer(m_volumeSliceRenderer);
      m_rendererBase->render(eye);
    }
  }

  currentOutport.releaseTarget();

  glDisable(GL_DEPTH_TEST);

  CHECK_GL_ERROR;
}

void Z3DVolumeRaycaster::adjustWidget()
{
  m_zSlicePosition.setVisible(m_showZSlice.get());
  m_ySlicePosition.setVisible(m_showYSlice.get());
  m_xSlicePosition.setVisible(m_showXSlice.get());
  m_zSlice2Position.setVisible(m_showZSlice2.get());
  m_ySlice2Position.setVisible(m_showYSlice2.get());
  m_xSlice2Position.setVisible(m_showXSlice2.get());
  m_boundBoxLineWidth.setVisible(m_showBoundBox.get());
  m_boundBoxLineColor.setVisible(m_showBoundBox.get());
}

glm::vec3 Z3DVolumeRaycaster::get3DPosition(glm::ivec2 pos2D, int width, int height, Z3DRenderOutputPort &port)
{
  glm::mat4 projection = m_camera.get().getProjectionMatrix(CenterEye);
  glm::mat4 modelview = m_camera.get().getViewMatrix(CenterEye);

  glm::ivec4 viewport;
  viewport[0] = 0;
  viewport[1] = 0;
  viewport[2] = width;
  viewport[3] = height;

  GLfloat WindowPosZ;
  port.bindTarget();
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(pos2D.x, pos2D.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &WindowPosZ);
  port.releaseTarget();

  CHECK_GL_ERROR;
  glm::vec3 pos = glm::unProject(glm::vec3(pos2D.x, pos2D.y, WindowPosZ), modelview,
                                 projection, viewport);

  return pos;
}

bool Z3DVolumeRaycaster::posIsInVolume(glm::vec3 pos3D) const
{
  glm::vec3 luf = m_volumes.getFirstValidData()->getPhysicalLUF();
  glm::vec3 rdb = m_volumes.getFirstValidData()->getPhysicalRDB();
  if (pos3D.x >= luf.x && pos3D.x <= rdb.x && pos3D.y >= luf.y && pos3D.y <= rdb.y && pos3D.z >= luf.z && pos3D.z <= rdb.z) {
    return true;
  } else {
    return false;
  }
}

void Z3DVolumeRaycaster::clearFRVolumeSlices()
{
  for (size_t i=0; i<m_FRVolumeSlices.size(); ++i) {
    for (size_t j=0; j<m_FRVolumeSlices[i].size(); ++j) {
      delete m_FRVolumeSlices[i][j];
    }
    m_FRVolumeSlices[i].clear();
  }
}

void Z3DVolumeRaycaster::prepareDataForRaycaster(Z3DVolume *volume, Z3DEye eye)
{
  if (!m_volumeRaycasterRenderer->hasVisibleRendering())
    return;

  glm::vec3 coordLuf = volume->getPhysicalLUF();
  glm::vec3 coordRdb = volume->getPhysicalRDB();
  glm::svec3 volDim = volume->getOriginalDimensions();

  float xTexCoordStart = m_xCut.lowerValue() / static_cast<float>(volDim.x-1);
  float xTexCoordEnd = m_xCut.upperValue() / static_cast<float>(volDim.x-1);
  float xCoordStart = glm::mix(coordLuf.x, coordRdb.x, xTexCoordStart);
  float xCoordEnd = glm::mix(coordLuf.x, coordRdb.x, xTexCoordEnd);
  float yTexCoordStart = m_yCut.lowerValue() / static_cast<float>(volDim.y-1);
  float yTexCoordEnd = m_yCut.upperValue() / static_cast<float>(volDim.y-1);
  float yCoordStart = glm::mix(coordLuf.y, coordRdb.y, yTexCoordStart);
  float yCoordEnd = glm::mix(coordLuf.y, coordRdb.y, yTexCoordEnd);
  float zTexCoordStart = m_zCut.lowerValue() / static_cast<float>(volDim.z-1);
  float zTexCoordEnd = m_zCut.upperValue() / static_cast<float>(volDim.z-1);
  float zCoordStart = glm::mix(coordLuf.z, coordRdb.z, zTexCoordStart);
  float zCoordEnd = glm::mix(coordLuf.z, coordRdb.z, zTexCoordEnd);

  m_2DImageQuad.clear();

  if (volume->is2DData()) { // for 2d image
    m_2DImageQuad = Z3DTriangleList::createImageSlice(volume->getOffset().z, glm::vec2(xCoordStart, yCoordStart),
                                                      glm::vec2(xCoordEnd, yCoordEnd), glm::vec2(xTexCoordStart, yTexCoordStart),
                                                      glm::vec2(xTexCoordEnd, yTexCoordEnd));
    m_2DImageQuad.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
  } else { // 3d volume but 2d slice
    if (m_zCut.lowerValue() == m_zCut.upperValue()) {
      m_2DImageQuad = Z3DTriangleList::createCubeSlice(zCoordStart, zTexCoordStart, 2, glm::vec2(xCoordStart, yCoordStart),
                                                       glm::vec2(xCoordEnd, yCoordEnd), glm::vec2(xTexCoordStart, yTexCoordStart),
                                                       glm::vec2(xTexCoordEnd, yTexCoordEnd));
      m_2DImageQuad.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
    } else if (m_yCut.lowerValue() == m_yCut.upperValue()) {
      m_2DImageQuad = Z3DTriangleList::createCubeSlice(yCoordStart, yTexCoordStart, 1, glm::vec2(xCoordStart, zCoordStart),
                                                       glm::vec2(xCoordEnd, zCoordEnd), glm::vec2(xTexCoordStart, zTexCoordStart),
                                                       glm::vec2(xTexCoordEnd, zTexCoordEnd));
      m_2DImageQuad.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
    } else if (m_xCut.lowerValue() == m_xCut.upperValue()) {
      m_2DImageQuad = Z3DTriangleList::createCubeSlice(xCoordStart, xTexCoordStart, 0, glm::vec2(yCoordStart, zCoordStart),
                                                       glm::vec2(yCoordEnd, zCoordEnd), glm::vec2(yTexCoordStart, zTexCoordStart),
                                                       glm::vec2(yTexCoordEnd, zTexCoordEnd));
      m_2DImageQuad.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());
    }
  }

  if (!m_2DImageQuad.empty()) {
    m_volumeRaycasterRenderer->clearQuads();
    m_volumeRaycasterRenderer->addQuad(m_2DImageQuad);
    return;
  }

  //  // 3d volume MIP
  //  if (m_volumeRaycasterRenderer->isMIPRendering()) {
  //    m_volumeRaycasterRenderer->clearQuads();
  //    float thre = 0.5;
  //    if (glm::dot(m_camera.getViewVector(), glm::vec3(0,0,1)) > thre)
  //      m_volumeRaycasterRenderer->addQuad(m_cubeSerieSlices["ZB2F"]);
  //    else if (glm::dot(m_camera.getViewVector(), glm::vec3(0,0,-1)) > thre)
  //      m_volumeRaycasterRenderer->addQuad(m_cubeSerieSlices["ZF2B"]);
  //    else if (glm::dot(m_camera.getViewVector(), glm::vec3(0,1,0)) > thre)
  //      m_volumeRaycasterRenderer->addQuad(m_cubeSerieSlices["YB2F"]);
  //    else if (glm::dot(m_camera.getViewVector(), glm::vec3(0,-1,0)) > thre)
  //      m_volumeRaycasterRenderer->addQuad(m_cubeSerieSlices["YF2B"]);
  //    else if (glm::dot(m_camera.getViewVector(), glm::vec3(1,0,0)) > thre)
  //      m_volumeRaycasterRenderer->addQuad(m_cubeSerieSlices["XB2F"]);
  //    else
  //      m_volumeRaycasterRenderer->addQuad(m_cubeSerieSlices["XF2B"]);
  //    return;
  //  }

  // 3d volume Raycasting
  Z3DTriangleList cube = Z3DTriangleList::createCube(glm::vec3(xCoordStart, yCoordStart, zCoordStart),
                                                     glm::vec3(xCoordEnd, yCoordEnd, zCoordEnd),
                                                     glm::vec3(xTexCoordStart, yTexCoordStart, zTexCoordStart),
                                                     glm::vec3(xTexCoordEnd, yTexCoordEnd, zTexCoordEnd));
  cube.transformVerticesByMatrix(volume->getPhysicalToWorldMatrix());

  // enable culling
  glEnable(GL_CULL_FACE);

  m_rendererBase->setCamera(m_camera.get());
  m_rendererBase->setViewport(m_exitPort.getSize());
  m_rendererBase->activateRenderer(m_textureCoordinateRenderer);
  CHECK_GL_ERROR;

  // render back texture
  m_exitPort.bindTarget();
  glClearDepth(0.0f);
  glDepthFunc(GL_GREATER);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glCullFace(GL_FRONT);

  m_textureCoordinateRenderer->setTriangleList(&cube);
  m_rendererBase->render(eye);
  CHECK_GL_ERROR;
  m_exitPort.releaseTarget();
  glDepthFunc(GL_LESS);
  glClearDepth(1.0f);
  CHECK_GL_ERROR;

  // render front texture
  m_entryPort.bindTarget();
  CHECK_GL_ERROR;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glCullFace(GL_BACK);

  float nearPlaneDistToOrigin = glm::dot(m_camera.getEye(), -m_camera.getViewVector()) - m_camera.getNearDist() - 0.01f;
  std::vector<glm::vec4> planes;
  planes.push_back(glm::vec4(-m_camera.getViewVector(), nearPlaneDistToOrigin));
  Z3DTriangleList clipped = Z3DUtils::clipClosedSurface(cube, planes);
  m_textureCoordinateRenderer->setTriangleList(&clipped);
  m_rendererBase->render(eye);

  m_entryPort.releaseTarget();

  // restore OpenGL state
  glCullFace(GL_BACK);
  glDisable(GL_CULL_FACE);

  m_volumeRaycasterRenderer->setEntryExitCoordTextures(m_entryPort.getColorTexture(),
                                                       m_entryPort.getDepthTexture(),
                                                       m_exitPort.getColorTexture(),
                                                       m_exitPort.getDepthTexture());
}

void Z3DVolumeRaycaster::leftMouseButtonPressed(QMouseEvent *e, int w, int h)
{
  e->ignore();
  if (!m_volumeRaycasterRenderer->hasVisibleRendering())
    return;
  // Mouse button pressed
  if (e->type() == QEvent::MouseButtonPress) {
    m_startCoord.x = e->x();
    m_startCoord.y = e->y();
    toggleInteractionMode(true, this);
    return;
  }

  if (e->type() == QEvent::MouseButtonRelease) {
    if (std::abs(e->x() - m_startCoord.x) < 2 && std::abs(m_startCoord.y - e->y()) < 2) {
      bool success;
      glm::vec3 pos3D = getFirstHit3DPosition(e->x(), e->y(), w, h, success);
      if (success) {
        emit pointInVolumeLeftClicked(e->pos(), glm::ivec3(pos3D));
        e->accept();
      }
    }
    toggleInteractionMode(false, this);
  }
}

void Z3DVolumeRaycaster::updateBoundBoxLineColors()
{
  m_boundBoxLineColors.clear();
  m_boundBoxLineColors.resize(24, m_boundBoxLineColor.get());
  m_boundBoxRenderer->setDataColors(&m_boundBoxLineColors);
}

void Z3DVolumeRaycaster::invalidateFRVolumeZSlice()
{
  m_FRVolumeSlicesValidState[0] = false;
}

void Z3DVolumeRaycaster::invalidateFRVolumeYSlice()
{
  m_FRVolumeSlicesValidState[1] = false;
}

void Z3DVolumeRaycaster::invalidateFRVolumeXSlice()
{
  m_FRVolumeSlicesValidState[2] = false;
}

void Z3DVolumeRaycaster::invalidateFRVolumeZSlice2()
{
  m_FRVolumeSlicesValidState[3] = false;
}

void Z3DVolumeRaycaster::invalidateFRVolumeYSlice2()
{
  m_FRVolumeSlicesValidState[4] = false;
}

void Z3DVolumeRaycaster::invalidateFRVolumeXSlice2()
{
  m_FRVolumeSlicesValidState[5] = false;
}

void Z3DVolumeRaycaster::updateCubeSerieSlices()
{
  m_cubeSerieSlices.clear();
  Z3DVolume *volume = m_volumes.getFirstValidData();
  if (!volume)
    return;

  glm::vec3 coordLuf = volume->getPhysicalLUF();
  glm::vec3 coordRdb = volume->getPhysicalRDB();
  glm::svec3 volDim = volume->getOriginalDimensions();
  glm::svec3 dim = volume->getDimensions();

  float xTexCoordStart = m_xCut.lowerValue() / static_cast<float>(volDim.x-1);
  float xTexCoordEnd = m_xCut.upperValue() / static_cast<float>(volDim.x-1);
  float xCoordStart = glm::mix(coordLuf.x, coordRdb.x, xTexCoordStart);
  float xCoordEnd = glm::mix(coordLuf.x, coordRdb.x, xTexCoordEnd);
  float yTexCoordStart = m_yCut.lowerValue() / static_cast<float>(volDim.y-1);
  float yTexCoordEnd = m_yCut.upperValue() / static_cast<float>(volDim.y-1);
  float yCoordStart = glm::mix(coordLuf.y, coordRdb.y, yTexCoordStart);
  float yCoordEnd = glm::mix(coordLuf.y, coordRdb.y, yTexCoordEnd);
  float zTexCoordStart = m_zCut.lowerValue() / static_cast<float>(volDim.z-1);
  float zTexCoordEnd = m_zCut.upperValue() / static_cast<float>(volDim.z-1);
  float zCoordStart = glm::mix(coordLuf.z, coordRdb.z, zTexCoordStart);
  float zCoordEnd = glm::mix(coordLuf.z, coordRdb.z, zTexCoordEnd);

  // it is no point to make more slices than actual texture dimension
  int numZSlice = std::ceil((m_zCut.upperValue() - m_zCut.lowerValue() + 1.0) / dim.z * volDim.z) * 2;
  int numYSlice = std::ceil((m_yCut.upperValue() - m_yCut.lowerValue() + 1.0) / dim.y * volDim.y) * 2;
  int numXSlice = std::ceil((m_xCut.upperValue() - m_xCut.lowerValue() + 1.0) / dim.x * volDim.x) * 2;
  // Z front to back
  m_cubeSerieSlices["ZF2B"] = Z3DTriangleList::createCubeSerieSlices(numZSlice, 2,
                                                                     glm::vec3(xCoordStart, yCoordStart, zCoordStart),
                                                                     glm::vec3(xCoordEnd, yCoordEnd, zCoordEnd),
                                                                     glm::vec3(xTexCoordStart, yTexCoordStart, zTexCoordStart),
                                                                     glm::vec3(xTexCoordEnd, yTexCoordEnd, zTexCoordEnd));
  // Z back to front
  m_cubeSerieSlices["ZB2F"] = Z3DTriangleList::createCubeSerieSlices(numZSlice, 2,
                                                                     glm::vec3(xCoordStart, yCoordStart, zCoordEnd),
                                                                     glm::vec3(xCoordEnd, yCoordEnd, zCoordStart),
                                                                     glm::vec3(xTexCoordStart, yTexCoordStart, zTexCoordEnd),
                                                                     glm::vec3(xTexCoordEnd, yTexCoordEnd, zTexCoordStart));
  // Y front to back
  m_cubeSerieSlices["YF2B"] = Z3DTriangleList::createCubeSerieSlices(numYSlice, 1,
                                                                     glm::vec3(xCoordStart, yCoordStart, zCoordStart),
                                                                     glm::vec3(xCoordEnd, yCoordEnd, zCoordEnd),
                                                                     glm::vec3(xTexCoordStart, yTexCoordStart, zTexCoordStart),
                                                                     glm::vec3(xTexCoordEnd, yTexCoordEnd, zTexCoordEnd));
  // Y back to front
  m_cubeSerieSlices["YB2F"] = Z3DTriangleList::createCubeSerieSlices(numYSlice, 1,
                                                                     glm::vec3(xCoordStart, yCoordEnd, zCoordStart),
                                                                     glm::vec3(xCoordEnd, yCoordStart, zCoordEnd),
                                                                     glm::vec3(xTexCoordStart, yTexCoordEnd, zTexCoordStart),
                                                                     glm::vec3(xTexCoordEnd, yTexCoordStart, zTexCoordEnd));
  // X front to back
  m_cubeSerieSlices["XF2B"] = Z3DTriangleList::createCubeSerieSlices(numXSlice, 0,
                                                                     glm::vec3(xCoordStart, yCoordStart, zCoordStart),
                                                                     glm::vec3(xCoordEnd, yCoordEnd, zCoordEnd),
                                                                     glm::vec3(xTexCoordStart, yTexCoordStart, zTexCoordStart),
                                                                     glm::vec3(xTexCoordEnd, yTexCoordEnd, zTexCoordEnd));
  // X back to front
  m_cubeSerieSlices["XB2F"] = Z3DTriangleList::createCubeSerieSlices(numXSlice, 0,
                                                                     glm::vec3(xCoordEnd, yCoordStart, zCoordStart),
                                                                     glm::vec3(xCoordStart, yCoordEnd, zCoordEnd),
                                                                     glm::vec3(xTexCoordEnd, yTexCoordStart, zTexCoordStart),
                                                                     glm::vec3(xTexCoordStart, yTexCoordEnd, zTexCoordEnd));
}

void Z3DVolumeRaycaster::invalidateAllFRVolumeSlices()
{
  m_FRVolumeSlicesValidState.clear();
  m_FRVolumeSlicesValidState.resize(m_maxNumOfFullResolutionVolumeSlice, false);
}

void Z3DVolumeRaycaster::hideBoundBox()
{
  m_showBoundBox.setValue(false);
}
