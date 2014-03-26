/**@file zdirectionaltemplate.h
 * @brief Directional template
 * @author Ting Zhao
 */

#ifndef ZDIRECTIONALTEMPLATE_H
#define ZDIRECTIONALTEMPLATE_H

#include "zstackdrawable.h"
#include "tz_workspace.h"
#include "tz_trace_defs.h"
#include "tz_local_neuroseg.h"

class ZPoint;

class ZDirectionalTemplate : public ZStackDrawable
{
public:
    ZDirectionalTemplate();
    ZDirectionalTemplate(const Trace_Record &tr);
    ZDirectionalTemplate(const ZDirectionalTemplate &dt);

    enum FittingScoreOption { STACK_FIT_DOT, STACK_FIT_CORRCOEF,
                              STACK_FIT_EDOT,
                              STACK_FIT_STAT, STACK_FIT_PDOT,
                              STACK_FIT_MEAN_SIGNAL, STACK_FIT_CORRCOEF_SC,
                              STACK_FIT_DOT_CENTER, STACK_FIT_OUTER_SIGNAL,
                              STACK_FIT_VALID_SIGNAL_RATIO,
                              STACK_FIT_LOW_MEAN_SIGNAL, STACK_FIT_DOT_P,
                              STACK_FIT_CORRCOEF_P, STACK_FIT_EDOT_P,
                              STACK_FIT_STAT_P
                            };

public:
    virtual void display(QPainter &painter, int z = 0, Display_Style option = NORMAL) const = 0;
    virtual void fitStack(const Stack *stack, Receptor_Fit_Workspace *ws) = 0;
    virtual double fittingScore(const Stack *stack,
                                ZDirectionalTemplate::FittingScoreOption option,
                                Stack *mask = NULL) = 0;
    virtual ZDirectionalTemplate* extend(Dlist_Direction_e direction = DL_FORWARD,
                                         double step = 0.5) = 0;
    virtual Local_Neuroseg* toLocalNeuroseg() const = 0;
    virtual void flip() = 0;
    virtual BOOL hitTest(double x, double y, double z) = 0;
    virtual ZPoint bottom() = 0;
    virtual ZPoint center() = 0;
    virtual ZPoint top() = 0;

    Trace_Record record() const { return m_tr; }
    Trace_Record* recordRef() { return &m_tr; }

private:
    Trace_Record m_tr;
};

#endif // ZDIRECTIONALTEMPLATE_H
