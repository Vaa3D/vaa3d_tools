#ifndef Z3DVOLUMERAYCASTER_H
#define Z3DVOLUMERAYCASTER_H

#include <QObject>
#include <QPoint>
#include "z3drenderprocessor.h"

#include "z3dcameraparameter.h"
#include "znumericparameter.h"
#include "z3dvolume.h"
#include "z3dmesh.h"

#include "zwidgetsgroup.h"

class Z3DVolumeRaycasterRenderer;
class Z3DTextureCoordinateRenderer;
class Z3DTextureCopyRenderer;
class Z3DVolumeSliceRenderer;
class Z3DImage2DRenderer;
class Z3DTriangleList;
class Z3DLineRenderer;

class Z3DVolumeRaycaster : public Z3DRenderProcessor
{
  Q_OBJECT
public:
  explicit Z3DVolumeRaycaster();
  virtual ~Z3DVolumeRaycaster();

  virtual void enterInteractionMode();
  virtual void exitInteractionMode();

  bool isReady(Z3DEye eye) const;

  // caller should clean up this (by delete parent of this zwidgetgroup)
  ZWidgetsGroup *getWidgetsGroup();

  inline Z3DCameraParameter* getCamera() { return &m_camera; }
  inline Z3DVolumeRaycasterRenderer* getRenderer() {
    return m_volumeRaycasterRenderer; }

  void hideBoundBox();

  // check success before using the returned value
  // if first hit 3d position is in volume, success will be true,
  // otherwise don't use the returned value
  glm::vec3 getFirstHit3DPosition(int x, int y, int width, int height,
                                  bool &success);

  inline int xCutLowerValue() { return m_xCut.lowerValue(); }
  inline int xCutUpperValue() { return m_xCut.upperValue(); }
  inline int yCutLowerValue() { return m_yCut.lowerValue(); }
  inline int yCutUpperValue() { return m_yCut.upperValue(); }
  inline int zCutLowerValue() { return m_zCut.lowerValue(); }
  inline int zCutUpperValue() { return m_zCut.upperValue(); }

  inline int xCutMin() { return m_xCut.minimum(); }
  inline int xCutMax() { return m_xCut.maximum(); }
  inline int yCutMin() { return m_yCut.minimum(); }
  inline int yCutMax() { return m_yCut.maximum(); }
  inline int zCutMin() { return m_zCut.minimum(); }
  inline int zCutMax() { return m_zCut.maximum(); }


  inline void setXCutLower(int v) { m_xCut.setLowerValue(v); }
  inline void setXCutUpper(int v) { m_xCut.setUpperValue(v); }
  inline void setYCutLower(int v) { m_yCut.setLowerValue(v); }
  inline void setYCutUpper(int v) { m_yCut.setUpperValue(v); }
  inline void setZCutLower(int v) { m_zCut.setLowerValue(v); }
  inline void setZCutUpper(int v) { m_zCut.setUpperValue(v); }

signals:
  void pointInVolumeLeftClicked(QPoint pt, glm::ivec3 pos3D);
  void pointInVolumeRightClicked(QPoint pt, glm::ivec3 pos3D);

protected slots:
  void adjustWidget();
  void leftMouseButtonPressed(QMouseEvent *e, int w, int h);
  void updateBoundBoxLineColors();

  void invalidateFRVolumeZSlice();
  void invalidateFRVolumeYSlice();
  void invalidateFRVolumeXSlice();
  void invalidateFRVolumeZSlice2();
  void invalidateFRVolumeYSlice2();
  void invalidateFRVolumeXSlice2();

  void updateCubeSerieSlices();

private:
  //get 3D position from 2D screen position
  glm::vec3 get3DPosition(glm::ivec2 pos2D, int width, int height, Z3DRenderOutputPort &port);
  bool posIsInVolume(glm::vec3 pos3D) const;
  void clearFRVolumeSlices();

  // based on context, prepare minimum necessary data and send to raycasterrenderer
  void prepareDataForRaycaster(Z3DVolume *volume, Z3DEye eye);

protected:
  void process(Z3DEye eye);

  void initialize();

  void deinitialize();

  void invalidateAllFRVolumeSlices();

  ZIntParameter m_interactionDownsample;      // screen space downsample during interaction

  Z3DInputPort<Z3DVolume> m_volumes;
  Z3DInputPort<ZStack> m_stackInputPort;
  Z3DRenderOutputPort m_entryPort;
  Z3DRenderOutputPort m_exitPort;
  Z3DRenderOutputPort m_tmpPort;

  Z3DRenderOutputPort m_outport;
  Z3DRenderOutputPort m_leftEyeOutport;
  Z3DRenderOutputPort m_rightEyeOutport;

  Z3DVolumeRaycasterRenderer *m_volumeRaycasterRenderer;
  Z3DVolumeSliceRenderer *m_volumeSliceRenderer;
  Z3DTextureCoordinateRenderer *m_textureCoordinateRenderer;
  Z3DTextureCopyRenderer *m_textureCopyRenderer;
  std::vector<Z3DImage2DRenderer*> m_image2DRenderers;
  Z3DLineRenderer *m_boundBoxRenderer;

  static const size_t m_maxNumOfFullResolutionVolumeSlice;
  // each channel is represented by a Z3DVolume
  std::vector<std::vector<Z3DVolume*> > m_FRVolumeSlices;
  std::vector<bool> m_FRVolumeSlicesValidState;
  ZBoolParameter m_useFRVolumeSlice;
  ZBoolParameter m_showXSlice;
  ZIntParameter m_xSlicePosition;
  ZBoolParameter m_showYSlice;
  ZIntParameter m_ySlicePosition;
  ZBoolParameter m_showZSlice;
  ZIntParameter m_zSlicePosition;
  ZBoolParameter m_showXSlice2;
  ZIntParameter m_xSlice2Position;
  ZBoolParameter m_showYSlice2;
  ZIntParameter m_ySlice2Position;
  ZBoolParameter m_showZSlice2;
  ZIntParameter m_zSlice2Position;

  ZBoolParameter m_showBoundBox;
  ZIntParameter m_boundBoxLineWidth;
  ZVec4Parameter m_boundBoxLineColor;
  std::vector<glm::vec3> m_boundBoxLines;
  std::vector<glm::vec4> m_boundBoxLineColors;

  Z3DCameraParameter m_camera;

  ZEventListenerParameter* m_leftMouseButtonPressEvent;
  glm::ivec2 m_startCoord;

  ZWidgetsGroup *m_widgetsGroup;

  ZIntSpanParameter m_xCut;
  ZIntSpanParameter m_yCut;
  ZIntSpanParameter m_zCut;

  Z3DTriangleList m_2DImageQuad;

  std::map<std::string, Z3DTriangleList> m_cubeSerieSlices;
};

#endif // Z3DVOLUMERAYCASTER_H
