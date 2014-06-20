#include "QUndoMarkerCreate.h"
#include "../control/CExplorerWindow.h"

itm::QUndoMarkerCreate::QUndoMarkerCreate(itm::CExplorerWindow* _source, LocationSimple _marker) : QUndoCommand()
{
    source = _source;
    marker = _marker;
    redoFirstTime = true;
}

// undo and redo methods
void itm::QUndoMarkerCreate::undo()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    // get markers from Vaa3D
    QList<LocationSimple> vaa3dMarkers = source->V3D_env->getLandmark(source->window);

    // remove the marker just created
    for(int i=0; i<vaa3dMarkers.size(); i++)
        if(vaa3dMarkers[i].x == marker.x && vaa3dMarkers[i].y == marker.y && vaa3dMarkers[i].z == marker.z)
            vaa3dMarkers.removeAt(i);

    // set new markers
    source->V3D_env->setLandmark(source->window, vaa3dMarkers);
    source->V3D_env->pushObjectIn3DWindow(source->window);
}

void itm::QUndoMarkerCreate::redo()
{
    // first time redo's call is aborted: we don't want it to be called once the command is pushed into the QUndoStack
    if(!redoFirstTime)
    {
        /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

        redoFirstTime = false;

        // get markers from Vaa3D
        QList<LocationSimple> vaa3dMarkers = source->V3D_env->getLandmark(source->window);

        // add previously deleted marker
        vaa3dMarkers.push_back(marker);

        // set new markers
        source->V3D_env->setLandmark(source->window, vaa3dMarkers);
        source->V3D_env->pushObjectIn3DWindow(source->window);
    }
}
