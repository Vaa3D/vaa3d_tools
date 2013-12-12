#include "zlocalrect.h"
#include "zlocalneuroseg.h"
#include "tz_r2_rect.h"
#include "zpoint.h"

#include "qdebug.h"

ZLocalRect::ZLocalRect()
{
  Default_Local_R2_Rect(&(m_template));
}

ZLocalRect::ZLocalRect(double x, double y, double z, double theta, double r)
{
  Default_Local_R2_Rect(&(m_template));
  m_template.transform.r = r;
  m_template.transform.theta = theta;
  m_template.transform.x = x;
  m_template.transform.y = y;
  m_template.base.z = z;
}

ZLocalRect::ZLocalRect(const ZLocalRect &rect) :
    ZInterface(rect.isSelected()), ZDirectionalTemplate(rect)
{
  m_template = rect.m_template;
}

void ZLocalRect::toLocalNeuroseg(Local_Neuroseg *locseg) const
{
  Local_R2_Rect_To_Local_Neuroseg(&m_template, locseg);
}

Local_Neuroseg* ZLocalRect::toLocalNeuroseg() const
{
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  toLocalNeuroseg(locseg);

  return locseg;
}

void ZLocalRect::display(QPainter &painter, int z, Display_Style style) const
{
  Local_Neuroseg locseg;
  Local_R2_Rect_To_Local_Neuroseg(&(this->m_template), &locseg);

  ZLocalNeuroseg tmpseg(&locseg);
  tmpseg.display(painter, z, style);
}

void ZLocalRect::fitStack(const Stack *stack, Receptor_Fit_Workspace *ws)
{
  Print_Local_R2_Rect(&m_template);
  //qDebug() << fittingScore(stack, ZDirectionalTemplate::STACK_FIT_CORRCOEF);
  Fit_Local_R2_Rect_W(&m_template, stack, 1.0, ws);
  Print_Local_R2_Rect(&m_template);
  //qDebug() << fittingScore(stack, ZDirectionalTemplate::STACK_FIT_CORRCOEF);
  //Local_R2_Rect_Optimize_W(&m_template, stack, 1.0, 1, ws);
}

double ZLocalRect::setFittingScoreOption(Stack_Fit_Score *fs,
                                         ZDirectionalTemplate::FittingScoreOption option)
{
  fs->n = 1;
  switch (option) {
  case ZDirectionalTemplate::STACK_FIT_DOT:
    fs->options[0] = STACK_FIT_DOT;
    break;
  case ZDirectionalTemplate::STACK_FIT_CORRCOEF:
    fs->options[0] = STACK_FIT_CORRCOEF;
    break;
  case ZDirectionalTemplate::STACK_FIT_EDOT:
    fs->options[0] = STACK_FIT_EDOT;
    break;
  case ZDirectionalTemplate::STACK_FIT_STAT:
    fs->options[0] = STACK_FIT_STAT;
    break;
  case ZDirectionalTemplate::STACK_FIT_PDOT:
    fs->options[0] = STACK_FIT_PDOT;
    break;
  case ZDirectionalTemplate::STACK_FIT_CORRCOEF_SC:
    fs->options[0] = STACK_FIT_CORRCOEF_SC;
    break;
  case ZDirectionalTemplate::STACK_FIT_DOT_CENTER:
    fs->options[0] = STACK_FIT_DOT_CENTER;
    break;
  case ZDirectionalTemplate::STACK_FIT_OUTER_SIGNAL:
    fs->options[0] = STACK_FIT_OUTER_SIGNAL;
    break;
  case ZDirectionalTemplate::STACK_FIT_VALID_SIGNAL_RATIO:
    fs->options[0] = STACK_FIT_VALID_SIGNAL_RATIO;
    break;
  case ZDirectionalTemplate::STACK_FIT_LOW_MEAN_SIGNAL:
    fs->options[0] = STACK_FIT_LOW_MEAN_SIGNAL;
    break;
  case ZDirectionalTemplate::STACK_FIT_DOT_P:
    fs->options[0] = STACK_FIT_DOT;
    break;
  case ZDirectionalTemplate::STACK_FIT_CORRCOEF_P:
    fs->options[0] = STACK_FIT_CORRCOEF;
    break;
  case ZDirectionalTemplate::STACK_FIT_EDOT_P:
    fs->options[0] = STACK_FIT_EDOT;
    break;
  case ZDirectionalTemplate::STACK_FIT_STAT_P:
    fs->options[0] = STACK_FIT_STAT;
    break;
  default:
    fs->options[0] = 0;
    break;
  }

  return fs->scores[0];
}

double ZLocalRect::fittingScore(const Stack *stack,
                                ZDirectionalTemplate::FittingScoreOption option,
                                Stack *mask)
{
  Receptor_Score_Workspace ws;
  Default_Receptor_Score_Workspace(&ws);
  ws.mask = mask;
  setFittingScoreOption(&ws.fs, option);
  Local_R2_Rect_Score_W(&(this->m_template), stack, 1.0, &ws);

  return ws.fs.scores[0];
}

ZDirectionalTemplate* ZLocalRect::extend(Dlist_Direction_e direction,
                                         double step)
{
  ZLocalRect *extension = NULL;
  double vec[2];
  vec[0] = cos(m_template.transform.theta + TZ_PI_2);
  vec[1] = sin(m_template.transform.theta + TZ_PI_2);

  switch (direction) {
  case DL_FORWARD:
    extension = new ZLocalRect(*this);
    extension->m_template.transform.x += vec[0] * step *
                                         m_template.transform.h;
    extension->m_template.transform.y += vec[1] * step *
                                         m_template.transform.h;
    break;
  case DL_BACKWARD:
    extension = new ZLocalRect(*this);
    extension->m_template.transform.x -= vec[0] * step *
                                         m_template.transform.h;
    extension->m_template.transform.y -= vec[1] * step *
                                         m_template.transform.h;

    break;
  default:
    break;
  }

  return extension;
}

void ZLocalRect::flip()
{
  double vec[2];
  vec[0] = cos(m_template.transform.theta + TZ_PI_2);
  vec[1] = sin(m_template.transform.theta + TZ_PI_2);
  m_template.transform.x += vec[0] * m_template.transform.h;
  m_template.transform.y += vec[1] * m_template.transform.h;
  m_template.transform.theta += TZ_PI;
}

BOOL ZLocalRect::hitTest(double x, double y, double z)
{
  return Local_R2_Rect_Hit_Test(&m_template, x, y, z);
}

ZPoint ZLocalRect::bottom()
{
  ZPoint pt;
  double coord[3];
  Local_R2_Rect_Bottom(&m_template, coord);
  pt.set(coord[0], coord[1], coord[2]);

  return pt;
}

ZPoint ZLocalRect::center()
{
  ZPoint pt;
  double coord[3];
  Local_R2_Rect_Bottom(&m_template, coord);
  pt.set(coord[0], coord[1], coord[2]);

  return pt;
}

ZPoint ZLocalRect::top()
{
  ZPoint pt;
  double coord[3];
  Local_R2_Rect_Top(&m_template, coord);
  pt.set(coord[0], coord[1], coord[2]);

  return pt;
}

ZINTERFACE_DEFINE_CLASS_NAME(ZLocalRect)
