#include "zmoviestage.h"
#include "z3dwindow.h"
#include "z3dvolumeraycaster.h"
#include "z3dvolumeraycasterrenderer.h"
#include "zstackdoc.h"

ZMovieStage::ZMovieStage(Z3DWindow *window) : m_window(window),
  m_isSwcChanged(false), m_isVolumeChanged(false), m_isPunctaChanged(false)
{
}

bool ZMovieStage::hasAnyChange()
{
  return isSwcChanged() || isVolumeChanged() || isPunctaChanged();
}

void ZMovieStage::updateWindow()
{
  bool changed = false;

  if (isSwcChanged()) {
    getWindow()->getDocument()->notifySwcModified();
    setSwcChanged(false);
    changed = true;
  }

  if (isVolumeChanged()) {
    getWindow()->getDocument()->notifyStackModified();
    setVolumeChanged(false);
    changed = true;
  }

  if (isPunctaChanged()) {
    getWindow()->getDocument()->notifyPunctumModified();
    setPunctaChanged(false);
    changed = true;
  }

  if (isVolumeChanged()) {
    getWindow()->getDocument()->notifyStackModified();
    setVolumeChanged(false);
    changed = true;
  }

  getWindow()->resetCameraClippingRange();
}

void ZMovieStage::hideVolume()
{
  getWindow()->getVolumeRaycaster()->getRenderer()->setChannel1Visible(false);
  getWindow()->getVolumeRaycaster()->getRenderer()->setChannel2Visible(false);
  getWindow()->getVolumeRaycaster()->getRenderer()->setChannel3Visible(false);
  getWindow()->getVolumeRaycaster()->getRenderer()->setChannel4Visible(false);
}

void ZMovieStage::showVolume()
{
  getWindow()->getVolumeRaycaster()->getRenderer()->setChannel1Visible(true);
  getWindow()->getVolumeRaycaster()->getRenderer()->setChannel2Visible(true);
  getWindow()->getVolumeRaycaster()->getRenderer()->setChannel3Visible(true);
  getWindow()->getVolumeRaycaster()->getRenderer()->setChannel4Visible(true);
}

void ZMovieStage::saveScreenShot(const std::string &filePath,
                                 int width, int height)
{
  getWindow()->takeScreenShot(filePath.c_str(), width, height, MonoView);
}
