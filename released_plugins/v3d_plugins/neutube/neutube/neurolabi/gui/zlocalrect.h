/**@file zlocalrect.h
 * @brief Local rectangle receptor
 * @author Ting Zhao
 */
#ifndef ZLOCALRECT_H
#define ZLOCALRECT_H

#include "zdirectionaltemplate.h"
#include "tz_r2_rect.h"

class ZLocalRect : public ZDirectionalTemplate
{
public:
  ZLocalRect();
  ZLocalRect(double x, double y, double z, double theta, double r);
  ZLocalRect(const ZLocalRect &rect);

  virtual const std::string& className() const;

public:
  void toLocalNeuroseg(Local_Neuroseg *locseg) const;
  Local_Neuroseg* toLocalNeuroseg() const;

public:
  virtual void display(QPainter &painter, int z = 0, Display_Style option = NORMAL) const;

public:
    virtual void fitStack(const Stack *stack, Receptor_Fit_Workspace *ws);
    virtual double fittingScore(const Stack *stack,
                        ZDirectionalTemplate::FittingScoreOption option,
                        Stack *mask = NULL);
    virtual ZDirectionalTemplate* extend(Dlist_Direction_e direction = DL_FORWARD,
                                         double step = 0.5);
    virtual void flip();
    virtual BOOL hitTest(double x, double y, double z);
    virtual ZPoint bottom();
    virtual ZPoint center();
    virtual ZPoint top();

private:
    double setFittingScoreOption(Stack_Fit_Score *fs,
                                 ZDirectionalTemplate::FittingScoreOption option);

private:
    Local_R2_Rect m_template;
};

#endif // ZLOCALRECT_H
