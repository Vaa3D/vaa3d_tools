#include "zmovieactor.h"

#include <iostream>

using namespace std;

ZMovieActor::ZMovieActor()
{
  m_isVisible = false;
  m_isActive = false;
  m_fadingFactor = 0.0;
  m_transitFactor[0] = 0.0;
  m_transitFactor[1] = 0.0;
  m_transitFactor[2] = 0.0;
  m_red = 0.0;
  m_green = 0.0;
  m_blue = 0.0;
  m_alpha = 1.0;
}

ZMovieActor::~ZMovieActor()
{
}

void ZMovieActor::perform(double t)
{
  if (isActive()) {
#ifdef _DEBUG_2
    if (getId() == "tbar_box") {
      cout << "debug here" << endl;
    }
#endif
    if (m_isVisible) {
      show();
    } else {
      hide();
    }

    if (isMoving()) {
      move(t);
    }

    addAlpha(m_fadingFactor * t);
    addColor(m_transitFactor[0] * t, m_transitFactor[1] * t, m_transitFactor[2] * t);

#ifdef _DEBUG_2
    cout << "alpha: " << m_alpha << endl;
#endif
  }
}

void ZMovieActor::setMovingOffset(double x, double y, double z)
{
  m_movingOffset.set(x, y, z);
}

void ZMovieActor::setMovingOffset(const ZPoint &offset)
{
  m_movingOffset.set(offset);
}

bool ZMovieActor::isMoving()
{
  return (m_movingOffset.x() != 0 || m_movingOffset.y() != 0 ||
      m_movingOffset.z() != 0);
}

void ZMovieActor::print()
{
  cout << "Id: " << m_id << endl;
  cout << "Visible: " << m_isVisible << endl;
}

#define ASSIGN_RANGED_VALUE(x, v) \
  x = ((v) > 1.0) ? 1.0 : (((v) < 0.0) ? 0.0 : v)

void ZMovieActor::takeColor(double red, double green, double blue)
{
  ASSIGN_RANGED_VALUE(m_red, red);
  ASSIGN_RANGED_VALUE(m_green, green);
  ASSIGN_RANGED_VALUE(m_blue, blue);
}

void ZMovieActor::setColor(double red, double green, double blue)
{
  takeColor(red, green, blue);
  pushColor();
}

void ZMovieActor::takeAlpha(double alpha)
{
  ASSIGN_RANGED_VALUE(m_alpha, alpha);
}

void ZMovieActor::setAlpha(double alpha)
{
  takeAlpha(alpha);
  pushAlpha();
}

void ZMovieActor::addColor(double dr, double dg, double db)
{
  setColor(m_red + dr, m_green + dg, m_blue + db);
#ifdef _DEBUG_2
  cout << "color added for " << m_id << " " << dr << " " << dg << " " << db
       << " : " << m_red << " " << m_green << " " << m_blue << endl;
#endif
}

void ZMovieActor::addAlpha(double da)
{
  setAlpha(m_alpha + da);
}
