#include "zdirectionaltemplatechain.h"
#include "zdirectionaltemplate.h"
#include "tz_locseg_chain.h"
#include "tz_trace_utils.h"
#include "zlocsegchain.h"
#include "zstack.hxx"
#include "zpoint.h"

ZDirectionalTemplateChain::ZDirectionalTemplateChain()
{
}

ZDirectionalTemplateChain::~ZDirectionalTemplateChain()
{
  qDeleteAll(m_chain.begin(), m_chain.end());
  m_chain.clear();
}

void ZDirectionalTemplateChain::display(QPainter &painter, int z, Display_Style option) const
{
  for (int i = 0; i < m_chain.size(); i++) {
    m_chain.at(i)->display(painter, z, option);
  }
}

void ZDirectionalTemplateChain::append(ZDirectionalTemplate *dt)
{
  m_chain.append(dt);
}

void ZDirectionalTemplateChain::prepend(ZDirectionalTemplate *dt)
{
  m_chain.prepend(dt);
}

void ZDirectionalTemplateChain::trace(const ZStack *stack,
                                    Trace_Workspace *tws)
{
  Receptor_Fit_Workspace *rfw = (Receptor_Fit_Workspace*) tws->fit_workspace;
  if (stack != NULL) {
    for (int i = 0; i < tws->length; i++) {
      ZDirectionalTemplate *dt = NULL;

      if (tws->trace_status[1] == TRACE_NORMAL) {
        dt = m_chain.last()->extend();
        dt->fitStack(stack->c_stack(), rfw);
        dt->recordRef()->fix_point = 0.0;
        if (rfw->sws->fs.scores[1] >= tws->min_score) {
          ZPoint pt = dt->top();
          if (hitTest(pt.x(), pt.y(), pt.z()) > 0) {
            tws->trace_status[1] = TRACE_LOOP_FORMED;
            //delete dt;
            dt = NULL;
            qDebug() << "Forward loop formed";
          } else {
            m_chain.append(dt);
          }
        } else {
          qDebug() << "Forward low score: " << rfw->sws->fs.scores[1];
          delete dt;
          dt = NULL;
          tws->trace_status[1] = TRACE_LOW_SCORE;
        }
      }

      if (tws->trace_status[0] == TRACE_NORMAL) {
        dt = m_chain.first()->extend(DL_BACKWARD);
        dt->flip();
        dt->fitStack(stack->c_stack(), rfw);
        dt->flip();
        dt->recordRef()->fix_point = 1.0;
        if (rfw->sws->fs.scores[1] >= tws->min_score) {
          ZPoint pt = dt->bottom();
          if (hitTest(pt.x(), pt.y(), pt.z()) > 0) {
            tws->trace_status[0] = TRACE_LOOP_FORMED;
            //delete dt;
            dt = NULL;
            qDebug() << "Backward loop formed";
          } else {
            m_chain.prepend(dt);
          }
        } else {
          qDebug() << "Low score: " << rfw->sws->fs.scores[1];
          delete dt;
          dt = NULL;
          tws->trace_status[0] = TRACE_LOW_SCORE;
        }
      }

      if (tws->trace_status[0] != TRACE_NORMAL &&
          tws->trace_status[1] != TRACE_NORMAL) {
        qDebug() << "Stop.";
        break;
      }
    }
  }
}

ZLocsegChain* ZDirectionalTemplateChain::toLocsegChain()
{
  if (m_chain.size() > 0) {
    Locseg_Chain *chain = New_Locseg_Chain();
    for (int i = 0; i < m_chain.size(); i++) {
      Trace_Record *tr = Copy_Trace_Record(m_chain.at(i)->recordRef());
      Locseg_Chain_Add(chain, m_chain.at(i)->toLocalNeuroseg(),
                       tr, DL_TAIL);
    }
    return new ZLocsegChain(chain);
  }

  return NULL;
}

int ZDirectionalTemplateChain::hitTest(double x, double y, double z)
{
  for (int i = 0; i < m_chain.size(); i++) {
    if (m_chain.at(i)->hitTest(x, y, z) == TRUE) {
      return i + 1;
    }
  }

  return 0;
}

ZINTERFACE_DEFINE_CLASS_NAME(ZDirectionalTemplateChain)
