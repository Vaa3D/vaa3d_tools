#ifndef QUNDOMARKERDELETE_H
#define QUNDOMARKERDELETE_H

#include <QUndoCommand>
#include "../control/CPlugin.h"

class teramanager::QUndoMarkerDelete : public QUndoCommand
{
    private:

        itm::CExplorerWindow* source;       //where the command has been applied

    public:

        QUndoMarkerDelete(itm::CExplorerWindow* _source);

        // undo and redo methods
        virtual void undo();
        virtual void redo();
};

#endif // QUNDOMARKERDELETE_H
