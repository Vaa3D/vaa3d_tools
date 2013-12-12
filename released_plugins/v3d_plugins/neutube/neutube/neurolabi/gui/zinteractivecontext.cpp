#include "zinteractivecontext.h"

ZInteractiveContext::ZInteractiveContext()
{
  m_traceMode = TRACE_OFF;
  m_tubeEditMode = TUBE_EDIT_OFF;
  m_viewMode = VIEW_NORMAL;
  m_exploreMode = EXPLORE_OFF;
  m_oldExploreMode = EXPLORE_OFF;
  m_markPunctaMode = MARK_PUNCTA_OFF;
  m_swcEditMode = SWC_EDIT_SELECT;
  m_strokeEditMode = STROKE_EDIT_OFF;
  m_exitingEdit = false;
}


bool ZInteractiveContext::isTraceModeOff() const
{
  if (m_swcEditMode != SWC_EDIT_SELECT ||
      m_swcEditMode != SWC_EDIT_OFF) {
    return false;
  }

  return (m_traceMode == TRACE_OFF);
}

bool ZInteractiveContext::isContextMenuActivated() const
{
  return ((m_swcEditMode == SWC_EDIT_OFF || m_swcEditMode == SWC_EDIT_SELECT) &&
          m_tubeEditMode == TUBE_EDIT_OFF &&
          m_strokeEditMode == STROKE_EDIT_OFF && !m_exitingEdit);
}
