#include "zpaintbundle.h"

ZPaintBundle::ZPaintBundle()
{
  m_swcNodes = &m_emptyNodeSet;
  clearAllDrawableLists();
}
