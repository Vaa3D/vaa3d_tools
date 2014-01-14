#include "zmoviephotographer.h"
#include "zstackdoc.h"
#include "zmoviestage.h"

using namespace std;

ZMoviePhotographer::ZMoviePhotographer(ZMovieStage *stage)
{
  m_stage = stage;
}

void ZMoviePhotographer::render()
{
  m_stage->updateWindow();
  /*
  m_stage->getDocument()->requestRedrawChain();
  m_stage->getDocument()->requestRedrawObj3d();
  m_stage->getDocument()->requestRedrawPuncta();
  m_stage->getDocument()->requestRedrawStack();
  m_stage->getDocument()->requestRedrawSwc();
  */
}

void ZMoviePhotographer::takePicture(const string &filePath, int width, int height)
{
  m_stage->saveScreenShot(filePath.c_str(), width, height);
  //m_stage->takeScreenShot(filePath.c_str(), width, height, MonoView);
}
