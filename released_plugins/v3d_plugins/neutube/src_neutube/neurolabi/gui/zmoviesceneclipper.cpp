#include "zmoviesceneclipper.h"

#include "tz_math.h"
#include "z3dwindow.h"
#include "z3dvolumeraycaster.h"
#include "z3dswcfilter.h"

using namespace std;

ZMovieSceneClipper::ZMovieSceneClipper() : m_target(UNKNOWN_TARGET),
  m_lowerClipSpeed(0.0), m_upperClipSpeed(0.0)
{
  m_hasReset[0] = false;
  m_hasReset[1] = false;
  m_clipReset[0] = 0.0;
  m_clipReset[1] = 0.0;
}

void ZMovieSceneClipper::print() const
{
  cout << "Target: " << m_target << endl;
  cout << "Axis: " << m_axis << endl;
  cout << "Speed: " << m_lowerClipSpeed << ", " << m_upperClipSpeed << endl;
}

void ZMovieSceneClipper::reset(Z3DWindow *stage,
                               ZMovieSceneClipperState *state) const
{
  if (m_hasReset[0]) {
    state->setLowerClip(m_target, m_axis, m_clipReset[0]);
  }

  if (m_hasReset[1]) {
    state->setUpperClip(m_target, m_axis, m_clipReset[1]);
  }

  clip(stage, state);
}

void ZMovieSceneClipper::clip(
    Z3DWindow *stage, ZMovieSceneClipperState *state) const
{
  if (m_target == UNKNOWN_TARGET) {
    return;
  }

  switch (m_target) {
  case VOLUME:
  {
    Z3DVolumeRaycaster *volume = stage->getVolumeRaycaster();
    int lowerCut = state->getLowerClip(m_target, m_axis);
    int upperCut = state->getUpperClip(m_target, m_axis);
    switch (m_axis) {
    case X:
      volume->setXCutLower(lowerCut);
      volume->setXCutUpper(upperCut);
      break;
    case Y:
      volume->setYCutLower(lowerCut);
      volume->setYCutUpper(upperCut);
      break;
    case Z:
      volume->setZCutLower(lowerCut);
      volume->setZCutUpper(upperCut);
      break;
    }
  }
    break;
  case SWC:
  {
    Z3DSwcFilter *renderer = stage->getSwcFilter();
    int lowerCut = state->getLowerClip(m_target, m_axis);
    int upperCut = state->getUpperClip(m_target, m_axis);
    switch (m_axis) {
    case X:
      renderer->setXCutLower(lowerCut);
      renderer->setXCutUpper(upperCut);
      break;
    case Y:
      renderer->setYCutLower(lowerCut);
      renderer->setYCutUpper(upperCut);
      break;
    case Z:
      renderer->setZCutLower(lowerCut);
      renderer->setZCutUpper(upperCut);
      break;
    }
  }
    break;
  case PUNCTA:
    break;
  default:
    break;
  }
}

void ZMovieSceneClipper::clip(
    Z3DWindow *stage, ZMovieSceneClipperState *state, double t) const
{
  state->updateLower(m_target, m_axis, m_lowerClipSpeed * t);
  state->updateUpper(m_target, m_axis, m_upperClipSpeed * t);

  clip(stage, state);
}

void ZMovieSceneClipper::loadJsonObject(const ZJsonObject &obj)
{
  map<string, json_t*> entryMap = obj.toEntryMap(false);
  if (entryMap.count("target") > 0 && entryMap.count("axis") > 0) {
    if (eqstr(ZJsonParser::stringValue(entryMap["target"]), "volume")) {
      m_target = VOLUME;
    }

    if (eqstr(ZJsonParser::stringValue(entryMap["target"]), "swc")) {
      m_target = SWC;
    }

    if (entryMap.count("reset") > 0) {
      ZJsonObject resetObj = ZJsonObject(entryMap["reset"], false);
      json_t *value = resetObj["lower"];
      if (value != NULL) {
        m_hasReset[0] = true;
        m_clipReset[0] = ZJsonParser::numberValue(value);
      }

      value = resetObj["upper"];
      if (value != NULL) {
        m_hasReset[1] = true;
        m_clipReset[1] = ZJsonParser::numberValue(value);
      }
    }

    if (eqstr(ZJsonParser::stringValue(entryMap["axis"]), "x")) {
      m_axis = X;
    } else if (eqstr(ZJsonParser::stringValue(entryMap["axis"]), "y")) {
      m_axis = Y;
    } else if (eqstr(ZJsonParser::stringValue(entryMap["axis"]), "z")) {
      m_axis = Z;
    }

    if (entryMap.count("lower") > 0) {
      m_lowerClipSpeed = ZJsonParser::numberValue(entryMap["lower"]);
    }

    if (entryMap.count("upper") > 0) {
      m_upperClipSpeed = ZJsonParser::numberValue(entryMap["upper"]);
    }
  }
}

void ZMovieSceneClipperState::init(Z3DWindow *stage)
{
  m_volumeClip[0].first = stage->getVolumeRaycaster()->xCutLowerValue();
  m_volumeClip[0].second = stage->getVolumeRaycaster()->xCutUpperValue();
  m_volumeClip[1].first = stage->getVolumeRaycaster()->yCutLowerValue();
  m_volumeClip[1].second = stage->getVolumeRaycaster()->yCutUpperValue();
  m_volumeClip[2].first = stage->getVolumeRaycaster()->zCutLowerValue();
  m_volumeClip[2].second = stage->getVolumeRaycaster()->zCutUpperValue();

  m_volumeClipRange[0].first = stage->getVolumeRaycaster()->xCutMin();
  m_volumeClipRange[0].second = stage->getVolumeRaycaster()->xCutMax();
  m_volumeClipRange[1].first = stage->getVolumeRaycaster()->yCutMin();
  m_volumeClipRange[1].second = stage->getVolumeRaycaster()->yCutMax();
  m_volumeClipRange[2].first = stage->getVolumeRaycaster()->zCutMin();
  m_volumeClipRange[2].second = stage->getVolumeRaycaster()->zCutMax();

  m_swcClip[0].first = stage->getSwcFilter()->xCutLowerValue();
  m_swcClip[0].second = stage->getSwcFilter()->xCutUpperValue();
  m_swcClip[1].first = stage->getSwcFilter()->yCutLowerValue();
  m_swcClip[1].second = stage->getSwcFilter()->yCutUpperValue();
  m_swcClip[2].first = stage->getSwcFilter()->zCutLowerValue();
  m_swcClip[2].second = stage->getSwcFilter()->zCutUpperValue();

  m_swcClipRange[0].first = stage->getSwcFilter()->xCutMin();
  m_swcClipRange[0].second = stage->getSwcFilter()->xCutMax();
  m_swcClipRange[1].first = stage->getSwcFilter()->yCutMin();
  m_swcClipRange[1].second = stage->getSwcFilter()->yCutMax();
  m_swcClipRange[2].first = stage->getSwcFilter()->zCutMin();
  m_swcClipRange[2].second = stage->getSwcFilter()->zCutMax();
}

std::pair<double, double>* ZMovieSceneClipperState::getClipHandle(
    ZMovieSceneClipper::ETarget target)
{
  pair<double, double> *clipHandle = NULL;

  switch (target) {
  case ZMovieSceneClipper::VOLUME:
    clipHandle = m_volumeClip;
    break;
  case ZMovieSceneClipper::SWC:
    clipHandle = m_swcClip;
    break;
  case ZMovieSceneClipper:: PUNCTA:
    clipHandle = m_punctaClip;
    break;
  default:
    break;
  }

  return clipHandle;
}

std::pair<int, int>* ZMovieSceneClipperState::getClipRangeHandle(
    ZMovieSceneClipper::ETarget target)
{
  std::pair<int, int> *clipRangeHandle = NULL;

  switch (target) {
  case ZMovieSceneClipper::VOLUME:
    clipRangeHandle = m_volumeClipRange;
    break;
  case ZMovieSceneClipper::SWC:
    clipRangeHandle = m_swcClipRange;
    break;
  case ZMovieSceneClipper:: PUNCTA:
    clipRangeHandle = m_punctaClipRange;
    break;
  default:
    break;
  }

  return clipRangeHandle;
}

int ZMovieSceneClipperState::getClipAxisIndex(ZMovieSceneClipper::EClipAxis axis)
{
  int index = -1;
  switch (axis) {
  case ZMovieSceneClipper::X:
    index = 0;
    break;
  case ZMovieSceneClipper::Y:
    index = 1;
    break;
  case ZMovieSceneClipper::Z:
    index = 2;
    break;
  }

  return index;
}

void ZMovieSceneClipperState::updateLower(
    ZMovieSceneClipper::ETarget target, ZMovieSceneClipper::EClipAxis axis,
    double dv)
{
  pair<double, double>* clipHandle = getClipHandle(target);
  pair<int, int>* clipRangeHandle = getClipRangeHandle(target);
  int index = getClipAxisIndex(axis);

  clipHandle[index].first += dv;
  if (clipHandle[index].first < (double) clipRangeHandle[index].first) {
    clipHandle[index].first = clipRangeHandle[index].first;
  }
}

void ZMovieSceneClipperState::updateUpper(
    ZMovieSceneClipper::ETarget target, ZMovieSceneClipper::EClipAxis axis,
    double dv)
{
  pair<double, double>* clipHandle = getClipHandle(target);
  pair<int, int>* clipRangeHandle = getClipRangeHandle(target);
  int index = getClipAxisIndex(axis);

  clipHandle[index].second += dv;
  if (clipHandle[index].second > (double) clipRangeHandle[index].second) {
    clipHandle[index].second = clipRangeHandle[index].second;
  }
}

int ZMovieSceneClipperState::getLowerClip(
    ZMovieSceneClipper::ETarget target, ZMovieSceneClipper::EClipAxis axis)
{
  pair<double, double> *clipHandle = getClipHandle(target);
  int index = getClipAxisIndex(axis);

  int clip = iround(clipHandle[index].first);

  return clip;
}

int ZMovieSceneClipperState::getUpperClip(
    ZMovieSceneClipper::ETarget target, ZMovieSceneClipper::EClipAxis axis)
{
  pair<double, double> *clipHandle = getClipHandle(target);
  int index = getClipAxisIndex(axis);

  int clip = iround(clipHandle[index].second);

  return clip;
}

void ZMovieSceneClipperState::setLowerClip(
    ZMovieSceneClipper::ETarget target, ZMovieSceneClipper::EClipAxis axis,
    double v)
{
  pair<double, double> *clipHandle = getClipHandle(target);
  int index = getClipAxisIndex(axis);

  clipHandle[index].first = v;
  if (clipHandle[index].first <
      (double) getClipRangeHandle(target)[index].first) {
    clipHandle[index].first = getClipRangeHandle(target)[index].first;
  }
}

void ZMovieSceneClipperState::setUpperClip(
    ZMovieSceneClipper::ETarget target, ZMovieSceneClipper::EClipAxis axis,
    double v)
{
  pair<double, double> *clipHandle = getClipHandle(target);
  int index = getClipAxisIndex(axis);

  clipHandle[index].second = v;
  if (clipHandle[index].second >
      (double) getClipRangeHandle(target)[index].second) {
    clipHandle[index].second = getClipRangeHandle(target)[index].second;
  }
}
