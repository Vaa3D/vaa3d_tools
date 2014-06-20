#ifndef QUNDOMARKERCREATE_H
#define QUNDOMARKERCREATE_H

#include <QUndoCommand>
#include "../control/CPlugin.h"

class teramanager::QUndoMarkerCreate : public QUndoCommand
{
    private:

        itm::CExplorerWindow* source;       //where the command has been applied
        LocationSimple marker;              //the marker being created
        bool redoFirstTime;                 //

    public:

        QUndoMarkerCreate(itm::CExplorerWindow* _source, LocationSimple _marker);

        // undo and redo methods
        virtual void undo();
        virtual void redo();
    
};

#endif // QUNDOMARKERCREATE_H
