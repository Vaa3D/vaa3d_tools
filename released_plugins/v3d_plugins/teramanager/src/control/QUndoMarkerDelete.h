#ifndef QUNDOMARKERDELETE_H
#define QUNDOMARKERDELETE_H

#include <QUndoCommand>
#include "../control/CPlugin.h"

class teramanager::QUndoMarkerDelete : public QUndoCommand
{
    private:

        itm::CExplorerWindow* source;       //where the command has been applied
        LocationSimple marker;              //the marker being deleted
        bool redoFirstTime;                 //to disable redo's first call

    public:

        QUndoMarkerDelete(itm::CExplorerWindow* _source, LocationSimple _marker);

        // undo and redo methods
        virtual void undo();
        virtual void redo();
};

#endif // QUNDOMARKERDELETE_H
