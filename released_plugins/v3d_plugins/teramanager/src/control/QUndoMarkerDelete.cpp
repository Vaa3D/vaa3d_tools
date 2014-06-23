#include "QUndoMarkerDelete.h"
#include "../control/CExplorerWindow.h"
#include "v3dr_glwidget.h"

itm::QUndoMarkerDelete::QUndoMarkerDelete(itm::CExplorerWindow* _source, LocationSimple _marker) :  QUndoCommand()
{
    source = _source;
    marker = _marker;
    redoFirstTime = true;
}

// undo and redo methods
void itm::QUndoMarkerDelete::undo()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    // get markers from Vaa3D
    QList<LocationSimple> vaa3dMarkers = source->V3D_env->getLandmark(source->window);

    // add previously deleted marker
    vaa3dMarkers.push_back(marker);

    // set new markers
    source->V3D_env->setLandmark(source->window, vaa3dMarkers);
    source->V3D_env->pushObjectIn3DWindow(source->window);

    // end select mode
    //source->view3DWidget->getRenderer()->endSelectMode();
}

void itm::QUndoMarkerDelete::redo()
{
    /**/itm::debug(itm::LEV1, itm::strprintf("redoFirstTime = %s", redoFirstTime ? "true" : "false").c_str(), __itm__current__function__);

    // first time redo's call is aborted: we don't want it to be called once the command is pushed into the QUndoStack
    if(!redoFirstTime)
    {
        // get markers from Vaa3D
        QList<LocationSimple> vaa3dMarkers = source->V3D_env->getLandmark(source->window);

        // remove again the marker previosly deleted
        for(int i=0; i<vaa3dMarkers.size(); i++)
            if(vaa3dMarkers[i].x == marker.x && vaa3dMarkers[i].y == marker.y && vaa3dMarkers[i].z == marker.z)
                vaa3dMarkers.removeAt(i);

        // set new markers
        source->V3D_env->setLandmark(source->window, vaa3dMarkers);
        source->V3D_env->pushObjectIn3DWindow(source->window);

        // end select mode
        //source->view3DWidget->getRenderer()->endSelectMode();
    }
    else
        redoFirstTime = false;
}
