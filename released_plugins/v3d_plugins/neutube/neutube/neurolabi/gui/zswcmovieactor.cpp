#include "zswcmovieactor.h"
#include "zswctree.h"
#include "tz_math.h"
#include "zmoviestage.h"

ZSwcMovieActor::ZSwcMovieActor()
{
  m_tree = NULL;
  m_type = SWC;
}

ZSwcMovieActor::~ZSwcMovieActor()
{
  m_tree = NULL;
}

void ZSwcMovieActor::setActor(ZSwcTree *tree)
{
  m_tree = tree;
}

void ZSwcMovieActor::hide()
{
  if (m_tree != NULL) {
    m_tree->setVisible(false);
    getStage()->setSwcChanged(true);
  }
}

void ZSwcMovieActor::show()
{
  if (m_tree != NULL) {
    m_tree->setVisible(true);
    getStage()->setSwcChanged(true);
  }
}

void ZSwcMovieActor::move(double t)
{
  if (m_tree != NULL) {
    m_tree->translate(m_movingOffset * t);
    getStage()->setSwcChanged(true);
  }
}

void ZSwcMovieActor::pushColor()
{
  if (m_tree != NULL) {
    m_tree->setColor(iround(m_red * 255.0), iround(m_green * 255.0),
                     iround(m_blue * 255.0));
    getStage()->setSwcChanged(true);
  }
}

void ZSwcMovieActor::pullColor()
{
  if (m_tree != NULL) {
    m_red = m_tree->getRedF();
    m_green = m_tree->getGreenF();
    m_blue = m_tree->getBlueF();
  }
}

void ZSwcMovieActor::pushAlpha()
{
  if (m_tree != NULL) {
    m_tree->setAlpha(iround(m_alpha * 255.0));
    getStage()->setSwcChanged(true);
  }
}

void ZSwcMovieActor::pullAlpha()
{
  if (m_tree != NULL) {
    m_alpha = m_tree->getAlphaF();
  }
}

void ZSwcMovieActor::reset()
{
  pushColor();
  pushAlpha();
}
