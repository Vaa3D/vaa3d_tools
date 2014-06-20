#include "QUndoMarkerDelete.h"
#include "../control/CExplorerWindow.h"

itm::QUndoMarkerDelete::QUndoMarkerDelete(itm::CExplorerWindow* _source) :  QUndoCommand()
{
    source = _source;
}

// undo and redo methods
void itm::QUndoMarkerDelete::undo()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);
}

void itm::QUndoMarkerDelete::redo()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);
}
