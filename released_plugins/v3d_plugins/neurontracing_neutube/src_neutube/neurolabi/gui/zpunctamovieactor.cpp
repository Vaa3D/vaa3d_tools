#include "zpunctamovieactor.h"
#include "zpunctum.h"
#include "zmoviestage.h"

ZPunctaMovieActor::ZPunctaMovieActor()
{
  m_type = PUNCTA;
}

ZPunctaMovieActor::~ZPunctaMovieActor()
{
  m_puncta.clear();
}

void ZPunctaMovieActor::setActor(const std::vector<ZPunctum *> puncta)
{
  m_puncta = puncta;
}

void ZPunctaMovieActor::hide()
{
  for (std::vector<ZPunctum *>::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    (*iter)->setVisible(false);
    getStage()->setPunctaChanged(true);
  }
}

void ZPunctaMovieActor::show()
{
  for (std::vector<ZPunctum *>::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    (*iter)->setVisible(true);
    getStage()->setPunctaChanged(true);
  }
}

void ZPunctaMovieActor::move(double t)
{
  for (std::vector<ZPunctum *>::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    (*iter)->translate(m_movingOffset * t);
    getStage()->setPunctaChanged(true);
  }
}
/*
void ZPunctaMovieActor::setColor(int red, int green, int blue)
{
  for (std::vector<ZPunctum *>::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    (*iter)->setColor(red, green, blue);
  }
}

void ZPunctaMovieActor::setAlpha(int alpha)
{
  for (std::vector<ZPunctum *>::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    (*iter)->setAlpha(alpha);
  }
}
*/
void ZPunctaMovieActor::reset()
{
  pushColor();
  pushAlpha();
}

void ZPunctaMovieActor::pushColor()
{
  for (std::vector<ZPunctum *>::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    (*iter)->setColor(iround(m_red * 255.0), iround(m_green * 255.0),
                      iround(m_blue * 255.0));
    getStage()->setPunctaChanged(true);
  }
}

void ZPunctaMovieActor::pushAlpha()
{
  for (std::vector<ZPunctum *>::iterator iter = m_puncta.begin();
       iter != m_puncta.end(); ++iter) {
    (*iter)->setAlpha(iround(m_alpha * 255.0));
    getStage()->setPunctaChanged(true);
  }
}

void ZPunctaMovieActor::pullColor()
{
  if (!m_puncta.empty()) {
    m_red = (*m_puncta.begin())->getRedF();
    m_green = (*m_puncta.begin())->getGreenF();
    m_blue = (*m_puncta.begin())->getBlueF();
  }
}

void ZPunctaMovieActor::pullAlpha()
{
  if (!m_puncta.empty()) {
    m_alpha = (*m_puncta.begin())->getAlphaF();
  }
}
