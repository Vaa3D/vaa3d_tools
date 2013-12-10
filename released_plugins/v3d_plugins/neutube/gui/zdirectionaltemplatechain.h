/**@file zdirectionaltemplatechain.h
 * @brief Chain of directional templates
 * @author Ting Zhao
 */

#ifndef ZDIRECTIONALTEMPLATECHAIN_H
#define ZDIRECTIONALTEMPLATECHAIN_H

#include <QList>
#include "zstackdrawable.h"
#include "tz_trace_defs.h"

class ZDirectionalTemplate;
class ZStack;
class ZLocsegChain;

class ZDirectionalTemplateChain : public ZStackDrawable
{
public:
    ZDirectionalTemplateChain();
    ~ZDirectionalTemplateChain();

    virtual const std::string& className() const;

public:
    virtual void display(QPainter &painter, int z = 0, Display_Style option = NORMAL) const;
    void trace(const ZStack *stack, Trace_Workspace *tws);
    void append(ZDirectionalTemplate* dt);
    void prepend(ZDirectionalTemplate* dt);
    int hitTest(double x, double y, double z);
    ZLocsegChain* toLocsegChain();
private:
    QList<ZDirectionalTemplate*> m_chain;
};

#endif // ZDIRECTIONALTEMPLATECHAIN_H
