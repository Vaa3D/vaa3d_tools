#include "QUndoMarkerDeleteROI.h"
#include "../control/CExplorerWindow.h"
#include "v3dr_glwidget.h"

itm::QUndoMarkerDeleteROI::QUndoMarkerDeleteROI(itm::CExplorerWindow* _source, QList<LocationSimple> _markers) :  QUndoCommand()
{
    source = _source;
    markers = _markers;
    redoFirstTime = true;
}

// undo and redo methods
void itm::QUndoMarkerDeleteROI::undo()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    // get markers from Vaa3D
    QList<LocationSimple> vaa3dMarkers = source->V3D_env->getLandmark(source->window);

    // add previously deleted markers
    for(int i=0; i<markers.size(); i++)
        vaa3dMarkers.push_back(markers[i]);

    // set new markers
    source->V3D_env->setLandmark(source->window, vaa3dMarkers);
    source->V3D_env->pushObjectIn3DWindow(source->window);


    // end select mode
    //source->view3DWidget->getRenderer()->endSelectMode();
}

void itm::QUndoMarkerDeleteROI::redo()
{
    /**/itm::debug(itm::LEV1, itm::strprintf("redoFirstTime = %s", redoFirstTime ? "true" : "false").c_str(), __itm__current__function__);

    // first time redo's call is aborted: we don't want it to be called once the command is pushed into the QUndoStack
    if(!redoFirstTime)
    {
        // get markers from Vaa3D
        QList<LocationSimple> vaa3dMarkers = source->V3D_env->getLandmark(source->window);

        // remove again the markers previosly deleted
        for(int i=0; i<vaa3dMarkers.size(); i++)
            for(int j=0; j<markers.size(); j++)
            {
                if(vaa3dMarkers[i].x == markers[j].x && vaa3dMarkers[i].y == markers[j].y && vaa3dMarkers[i].z == markers[j].z)
                    vaa3dMarkers.removeAt(i);
                else if(fabs(vaa3dMarkers[i].x-markers[j].x)<4 && fabs(vaa3dMarkers[i].y-markers[j].y)<4 && fabs(vaa3dMarkers[i].z-markers[j].z)<4)
                    QMessageBox::information(0, "Warning", itm::strprintf("I am not removing vaa3d marker (%.1f, %.1f, %.1f) because is not equal to the searched marker (%.1f, %.1f, %.1f)",
                           vaa3dMarkers[i].x, vaa3dMarkers[i].y, vaa3dMarkers[i].z, markers[j].x, markers[j].y, markers[j].z).c_str());
            }

        // set new markers
        source->V3D_env->setLandmark(source->window, vaa3dMarkers);
        source->V3D_env->pushObjectIn3DWindow(source->window);


        // end select mode
        //source->view3DWidget->getRenderer()->endSelectMode();
    }
    else
        redoFirstTime = false;
}

