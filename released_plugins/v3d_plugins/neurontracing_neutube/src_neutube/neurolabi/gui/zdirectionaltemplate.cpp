#include "zdirectionaltemplate.h"
#include "tz_trace_utils.h"
#include "zpoint.h"

ZDirectionalTemplate::ZDirectionalTemplate()
{

}

ZDirectionalTemplate::ZDirectionalTemplate(const Trace_Record &tr)
{
  Trace_Record_Copy(&m_tr, &tr);
}

ZDirectionalTemplate::ZDirectionalTemplate(const ZDirectionalTemplate &dt) :
    ZInterface(dt.isSelected()), ZStackDrawable(dt)
{
  Trace_Record_Copy(&m_tr, &dt.m_tr);
}
