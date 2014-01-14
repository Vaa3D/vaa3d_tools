#include "z3dscene.h"

#include "z3dnetworkevaluator.h"

Z3DScene::Z3DScene(int width, int height, bool stereo, QObject *parent)
  : QGraphicsScene(0, 0, width, height, parent)
  , m_networkEvaluator(NULL)
  , m_isStereoScene(stereo)
  , m_fakeStereoOnce(false)
{
}

Z3DScene::~Z3DScene()
{
}

void Z3DScene::drawBackground(QPainter */*painter*/, const QRectF &)
{
  if (!m_networkEvaluator) {
    return;
  }

  // QPainter set glclearcolor to white, we set it back
  glClearColor(0.f, 0.f, 0.f, 0.f);

  m_networkEvaluator->process(m_isStereoScene || m_fakeStereoOnce);
  m_fakeStereoOnce = false;
}
