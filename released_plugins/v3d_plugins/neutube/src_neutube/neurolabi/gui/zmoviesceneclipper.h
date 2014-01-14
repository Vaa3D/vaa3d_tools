#ifndef ZMOVIESCENECLIPPER_H
#define ZMOVIESCENECLIPPER_H

#include <utility>

#include "zjsonobject.h"

class Z3DWindow;
class ZMovieSceneClipperState;

class ZMovieSceneClipper
{
public:
  ZMovieSceneClipper();

  enum ETarget {
    VOLUME, SWC, PUNCTA, UNKNOWN_TARGET
  };
  enum EClipAxis {
    X, Y, Z
  };

  void loadJsonObject(const ZJsonObject &obj);
  void clip(Z3DWindow *stage, ZMovieSceneClipperState *state) const;
  void clip(Z3DWindow *stage, ZMovieSceneClipperState *state, double t) const;

  void print() const;

  void reset(Z3DWindow *stage, ZMovieSceneClipperState *state) const;

private:
  ETarget m_target;
  EClipAxis m_axis;
  double m_lowerClipSpeed;
  double m_upperClipSpeed;

  bool m_hasReset[2];
  double m_clipReset[2];
};

class ZMovieSceneClipperState
{
public:
  void init(Z3DWindow *stage);
  void updateLower(ZMovieSceneClipper::ETarget target,
                   ZMovieSceneClipper::EClipAxis axis, double dv);
  void updateUpper(ZMovieSceneClipper::ETarget target,
                   ZMovieSceneClipper::EClipAxis axis, double dv);
  int getLowerClip(ZMovieSceneClipper::ETarget target,
                   ZMovieSceneClipper::EClipAxis axis);
  int getUpperClip(ZMovieSceneClipper::ETarget target,
                   ZMovieSceneClipper::EClipAxis axis);

  void setLowerClip(ZMovieSceneClipper::ETarget target,
                   ZMovieSceneClipper::EClipAxis axis, double v);
  void setUpperClip(ZMovieSceneClipper::ETarget target,
                   ZMovieSceneClipper::EClipAxis axis, double v);

private:
  std::pair<double, double>* getClipHandle(ZMovieSceneClipper::ETarget target);
  std::pair<int, int>* getClipRangeHandle(ZMovieSceneClipper::ETarget target);
  static int getClipAxisIndex(ZMovieSceneClipper::EClipAxis axis);

private:
  std::pair<double, double> m_volumeClip[3];
  std::pair<double, double> m_swcClip[3];
  std::pair<double, double> m_punctaClip[3];
  std::pair<int, int> m_volumeClipRange[3];
  std::pair<int, int> m_swcClipRange[3];
  std::pair<int, int> m_punctaClipRange[3];
};

#endif // ZMOVIESCENECLIPPER_H
