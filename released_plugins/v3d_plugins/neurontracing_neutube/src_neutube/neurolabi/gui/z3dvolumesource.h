#ifndef Z3DVOLUMESOURCE_H
#define Z3DVOLUMESOURCE_H

#include <QObject>

#include "z3dprocessor.h"
#include "z3dvolume.h"
#include "z3dport.h"
#include "znumericparameter.h"
#include "zwidgetsgroup.h"
#include <vector>

class ZStackDoc;

class Z3DVolumeSource : public Z3DProcessor
{
  Q_OBJECT
public:
  Z3DVolumeSource(ZStackDoc *doc);
  virtual ~Z3DVolumeSource();

  void reloadVolume();

  // input volPos should be in volume coordinate
  // means it is in range [0 width-1 0 height-1 0 depth-1]
  bool openZoomInView(const glm::ivec3 &volPos);
  void exitZoomInView();

  Z3DVolume* getVolume(size_t index);
  std::vector<double> getZoomInBound() const { return m_zoomInBound; }

  inline float getXScale() {return m_xScale.get();}
  inline float getYScale() {return m_yScale.get();}
  inline float getZScale() {return m_zScale.get();}
  inline void setZScale(float s) { m_zScale.set(s); }

  bool volumeNeedDownsample() const;
  bool isVolumeDownsampled() const;
  bool isSubvolume() const { return m_isSubVolume.get(); }

  // caller should clean up this (by delete parent of this zwidgetgroup)
  ZWidgetsGroup *getWidgetsGroup();

  bool isEmpty();

  inline void setMaxVoxelNumber(size_t n) { m_maxVoxelNumber = n; }

signals:
  void xScaleChanged();
  void yScaleChanged();
  void zScaleChanged();

protected slots:
  void changeXScale();
  void changeYScale();
  void changeZScale();
  void changeZoomInViewSize();

protected:
  virtual void process(Z3DEye);
  virtual void initialize();
  virtual void deinitialize();

  // once processed, should be valid for both stereo view and mono view
  virtual void setValid(Z3DEye eye) { Z3DProcessor::setValid(eye); m_invalidationState = Valid; }

private:
  void clearVolume();
  void clearZoomInVolume();
  void readVolumes();
  void readSubVolumes(int left, int top, int front, int width, int height, int depth);
  void sendData();
  void sendZoomInVolumeData();

  std::vector<Z3DVolume*> m_volumes;
  std::vector<Z3DVolume*> m_zoomInVolumes;
  std::vector<Z3DOutputPort<Z3DVolume>*> m_outputPorts;
  Z3DOutputPort<ZStack> m_stackOutputPort;

  ZFloatParameter m_xScale;
  ZFloatParameter m_yScale;
  ZFloatParameter m_zScale;
  ZBoolParameter m_isVolumeDownsampled;
  ZBoolParameter m_isSubVolume;
  ZIntParameter m_zoomInViewSize;
  glm::ivec3 m_zoomInPos;
  std::vector<double> m_zoomInBound;

  ZStackDoc *m_doc;
  static const size_t m_nChannelSupport;
  size_t m_maxVoxelNumber;

  ZWidgetsGroup *m_widgetsGroup;
};

#endif // Z3DVOLUMESOURCE_H
