//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
*
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

#include "CExplorerWindow.h"
#include "v3dr_mainwindow.h"
#include "control/CVolume.h"
#include "control/CAnnotations.h"
#include "presentation/PMain.h"
#include "renderer_gl1.h"
#include "v3d_imaging_para.h"
#include "v3dr_colormapDialog.h"
//#include "newmat.h"

using namespace teramanager;

CExplorerWindow* CExplorerWindow::first = NULL;
CExplorerWindow* CExplorerWindow::last = NULL;

CExplorerWindow::CExplorerWindow(V3DPluginCallback2 *_V3D_env, int _resIndex, uint8 *imgData, int _volV0, int _volV1,
                                 int _volH0, int _volH1, int _volD0, int _volD1, int _nchannels, CExplorerWindow *_prev) : QWidget()
{
    //initializations
    this->V3D_env = _V3D_env;
    this->prev = _prev;
    this->next = NULL;
    this->volResIndex = _resIndex;
    this->volV0 = _volV0;
    this->volV1 = _volV1;
    this->volH0 = _volH0;
    this->volH1 = _volH1;
    this->volD0 = _volD0;
    this->volD1 = _volD1;
    this->nchannels = _nchannels;
    this->toBeClosed = false;
    char ctitle[1024];
    sprintf(ctitle, "Res(%d x %d x %d),Volume X=[%d,%d], Y=[%d,%d], Z=[%d,%d], %d channels", CImport::instance()->getVolume(volResIndex)->getDIM_H(),
            CImport::instance()->getVolume(volResIndex)->getDIM_V(), CImport::instance()->getVolume(volResIndex)->getDIM_D(),
            volH0+1, volH1, volV0+1, volV1, volD0+1, volD1, nchannels);
    this->title = ctitle;
    V0_sbox_min = V1_sbox_max = H0_sbox_min = H1_sbox_max = D0_sbox_min = D1_sbox_max = V0_sbox_val = V1_sbox_val = H0_sbox_val = H1_sbox_val = D0_sbox_val = D1_sbox_val = -1;
    PMain* pMain = PMain::instance();

    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] created\n", this->thread()->currentThreadId(), title.c_str());
    #endif

    try
    {
        //opening tri-view window
        this->window = V3D_env->newImageWindow(QString(title.c_str()));
        Image4DSimple* image = new Image4DSimple();
        image->setFileName(title.c_str());
        image->setData(imgData, volH1-volH0, volV1-volV0, volD1-volD0, nchannels, V3D_UINT8);
        V3D_env->setImage(window, image);
        this->triViewWidget = (XFormWidget*)window;

        //opening 3D view window and hiding the tri-view window
        V3D_env->open3DWindow(window);
        view3DWidget = (V3dR_GLWidget*)(V3D_env->getView3DControl(window));
        view3DWidget->setVolCompress(false);
        window3D = view3DWidget->getiDrawExternalParameter()->window3D;
        triViewWidget->setWindowState(Qt::WindowMinimized);

        //installing the event filter on the 3D renderer and on the 3D window
        view3DWidget->installEventFilter(this);
        window3D->installEventFilter(this);

        //if the previous explorer window exists
        if(prev)
        {
            //applying the same color map only if it differs from the previous one
            Renderer_gl2* prev_renderer = (Renderer_gl2*)(prev->view3DWidget->getRenderer());
            Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
            bool changed_cmap = false;
            for(int k=0; k<3; k++)
            {
                RGBA8* prev_cmap = prev_renderer->colormap[k];
                RGBA8* curr_cmap = curr_renderer->colormap[k];
                for(int i=0; i<256; i++)
                {
                    if(curr_cmap[i].i != prev_cmap[i].i)
                        changed_cmap = true;
                    curr_cmap[i] = prev_cmap[i];
                }
            }
            if(changed_cmap)
                curr_renderer->applyColormapToImage();

            //positioning the current 3D window exactly at the previous window position
            QPoint location = prev->window3D->pos();
            prev->window3D->setVisible(false);
            window3D->resize(prev->window3D->size());
            printf("Calling window3D->move() in the IF of constructor\n");
            window3D->move(location);

            //hiding both tri-view and 3D view
            prev->triViewWidget->setVisible(false);
            prev->window3D->setVisible(false);
            prev->view3DWidget->setCursor(Qt::ArrowCursor);

            //registrating views: ---- Alessandro 2013-04-18 fixed: determining unique triple of rotation angles and assigning absolute rotation
            float ratio = CImport::instance()->getVolume(volResIndex)->getDIM_D()/CImport::instance()->getVolume(prev->volResIndex)->getDIM_D();
            view3DWidget->setZoom(prev->view3DWidget->zoom()/ratio);
            prev->view3DWidget->absoluteRotPose();
            view3DWidget->doAbsoluteRot(prev->view3DWidget->xRot(), prev->view3DWidget->yRot(), prev->view3DWidget->zRot());

            //storing annotations done in the previous view and loading annotations of the current view
            prev->storeAnnotations();
            this->loadAnnotations();
        }
        //otherwise this is the lowest resolution window
        else
        {
            //registrating the current window as the first window of the multiresolution explorer windows chain
            CExplorerWindow::first = this;

            //centering the current 3D window and the plugin's window
            int screen_height = qApp->desktop()->availableGeometry().height();
            int screen_width = qApp->desktop()->availableGeometry().width();
            int window_x = (screen_width - (window3D->width() + PMain::instance()->width()))/2;
//            int window_x = (screen_width - (window3D->width() + PMain::instance()->width()))/2 + PMain::instance()->width();
            int window_y = (screen_height - window3D->height()) / 2;
            window3D->move(window_x, window_y);
        }

        //registrating the current window as the last window of the multiresolution explorer windows chain
        CExplorerWindow::last = this;

        //selecting the current resolution in the PMain GUI and disabling previous resolutions
        pMain->resolution_cbox->setCurrentIndex(volResIndex);
        for(int i=0; i<pMain->resolution_cbox->count(); i++)
        {
            // Get the index of the value to disable
            QModelIndex index = pMain->resolution_cbox->model()->index(i,0);

            // These are the effective 'disable/enable' flags
            QVariant v1(Qt::NoItemFlags);
            QVariant v2(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

            //the magic
            if(i<volResIndex)
                pMain->resolution_cbox->model()->setData( index, v1, Qt::UserRole -1);
            else
                pMain->resolution_cbox->model()->setData( index, v2, Qt::UserRole -1);
        }

        //disabling translate buttons if needed
        pMain->traslYneg->setEnabled(volV0 > 0);
        pMain->traslYpos->setEnabled(volV1 < CImport::instance()->getVolume(volResIndex)->getDIM_V());
        pMain->traslXneg->setEnabled(volH0 > 0);
        pMain->traslXpos->setEnabled(volH1 < CImport::instance()->getVolume(volResIndex)->getDIM_H());
        pMain->traslZneg->setEnabled(volD0 > 0);
        pMain->traslZpos->setEnabled(volD1 < CImport::instance()->getVolume(volResIndex)->getDIM_D());

        //setting min, max and value of PMain GUI VOI's widgets
        pMain->V0_sbox->setMinimum(getGlobalVCoord(view3DWidget->yCut0())+1);
        pMain->V0_sbox->setValue(pMain->V0_sbox->minimum());
        pMain->V1_sbox->setMaximum(getGlobalVCoord(view3DWidget->yCut1())+1);
        pMain->V1_sbox->setValue(pMain->V1_sbox->maximum());
        pMain->H0_sbox->setMinimum(getGlobalHCoord(view3DWidget->xCut0())+1);
        pMain->H0_sbox->setValue(pMain->H0_sbox->minimum());
        pMain->H1_sbox->setMaximum(getGlobalHCoord(view3DWidget->xCut1())+1);
        pMain->H1_sbox->setValue(pMain->H1_sbox->maximum());
        pMain->D0_sbox->setMinimum(getGlobalDCoord(view3DWidget->zCut0())+1);
        pMain->D0_sbox->setValue(pMain->D0_sbox->minimum());
        pMain->D1_sbox->setMaximum(getGlobalDCoord(view3DWidget->zCut1())+1);
        pMain->D1_sbox->setValue(pMain->D1_sbox->maximum());

        //signal connections
        connect(CVolume::instance(), SIGNAL(sendOperationOutcome(MyException*,void*)), this, SLOT(loadingDone(MyException*,void*)), Qt::QueuedConnection);
        connect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
        connect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
        connect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
        connect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
        connect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
        connect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
        connect(PMain::instance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
        connect(PMain::instance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
        connect(PMain::instance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
        connect(PMain::instance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
        connect(PMain::instance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
        connect(PMain::instance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));

        //changing window flags (disabling minimize/maximize buttons)
        window3D->setWindowFlags(Qt::WindowStaysOnTopHint);
        // ---- Alessandro 2013-04-22 fixed: this causes (somehow) window3D not to respond correctly to the move() method
//        window3D->setWindowFlags(Qt::Tool
//                                 | Qt::WindowTitleHint
//                                 | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
        window3D->show();
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
    catch(const char* error)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(error),QObject::tr("Ok"));
    }
    catch(...)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr("Unknown error occurred"),QObject::tr("Ok"));
    }
}

CExplorerWindow::~CExplorerWindow()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] destroyed\n", this->thread()->currentThreadId(), title.c_str() );
    #endif

    //just closing windows
    window3D->close();
    triViewWidget->close();
}


/**********************************************************************************
* Filters events generated by the 3D rendering window <view3DWidget>
* We're interested to intercept these events to provide many useful ways to explore
* the 3D volume at different resolutions without changing Vaa3D code.
***********************************************************************************/
bool CExplorerWindow::eventFilter(QObject *object, QEvent *event)
{
    try
    {
        /******************** INTERCEPTING ZOOMING EVENTS *************************
        Zoom-in and zoom-out events generated by the current 3D renderer are inter-
        cepted to switch to the higher/lower resolution.
        ***************************************************************************/
        if (object == view3DWidget && event->type() == QEvent::Wheel)
        {
/*            LandmarkList markers = V3D_env->getLandmark(triViewWidget);
            if(view3DWidget->zoom() > 30        &&              //zoom-in threshold reached
               markers.size() == 1)                             //only one marker exists
            {
                switchToHigherRes(markers.first().x, markers.first().y, markers.first().z);
                markers.clear();
                V3D_env->setLandmark(triViewWidget, markers);
                view3DWidget->getRenderer()->updateLandmark();
            }
            else */ if(view3DWidget->zoom() < -100+PMain::instance()->zoomSensitivity->value() &&   //zoom-out threshold reached
                    prev                        &&                                                  //the previous resolution exists
                    !toBeClosed)                                                                    //the current resolution does not have to be closed
            {
                //printf("\n\nzoom[%d] < 100-sens[%d]\n\n", view3DWidget->zoom(), PMain::instance()->zoomSensitivity->value());
                toBeClosed = true;
                prev->restore();
            }
        }
        /****************** INTERCEPTING DOUBLE CLICK EVENTS ***********************
        Double click events are intercepted to switch to the higher resolution.
        ***************************************************************************/
        else if (object == view3DWidget && event->type() == QEvent::MouseButtonDblClick)
        {
            QMouseEvent* mouseEvt = (QMouseEvent*)event;
            XYZ point = getRenderer3DPoint(mouseEvt->x(), mouseEvt->y());
            newView(point.x, point.y, point.z, volResIndex+1);
        }
        /********************* INTERCEPTING CLOSE EVENTS **************************
        Close events are intercepted to switch to  the lower resolution,  if avail-
        able. Otherwise, the plugin is closed.
        ***************************************************************************/
        else if(object == window3D && event->type()==QEvent::Close)
        {
            if(!toBeClosed)
                PMain::instance()->closeVolume();
        }
        /**************** INTERCEPTING MOVING/RESIZING EVENTS *********************
        Window moving and resizing events  are intercepted  to let PMain's position
        be syncronized with the explorer.
        ***************************************************************************/
        else if(object == window3D && (event->type() == QEvent::Move || event->type() == QEvent::Resize))
        {
           alignToLeft(PMain::instance());
//            alignToRight(PMain::instance());
        }
        /***************** INTERCEPTING STATE CHANGES EVENTS **********************
        Window state changes events are intercepted to let PMain's position be syn-
        cronized with the explorer.
        ***************************************************************************/
        else if(object == window3D && event->type() == QEvent::WindowStateChange)
        {
            //PMain::instance()->setWindowState(window3D->windowState());
//            QWindowStateChangeEvent* stateChangeEvt = (QWindowStateChangeEvent*)event;
//            printf("old state = %d\n", int(stateChangeEvt->oldState()));
//            //event->ignore();
//            window3D->setWindowState(Qt::WindowActive);
            //return true;
        }
        return false;
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        return false;
    }
}

/**********************************************************************************
* Called by the signal emitted by <CVolume> when the associated  operation has been
* performed. If an exception has occurred in the <CVolume> thread, it is propagated
* and managed in the current thread (ex != 0).
***********************************************************************************/
void CExplorerWindow::loadingDone(MyException *ex, void* sourceObject)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] loadingDone(%s) launched\n",
           this->thread()->currentThreadId(), title.c_str(),  (ex? "ex" : "NULL"));
    #endif

    CVolume* cVolume = CVolume::instance();

    //if an exception has occurred, showing a message error
    if(ex)
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
    else if(sourceObject == this)
    {
        //signal disconnections
        disconnect(CVolume::instance(), SIGNAL(sendOperationOutcome(MyException*,void*)), this, SLOT(loadingDone(MyException*,void*)));
        disconnect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
        disconnect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
        disconnect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
        disconnect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
        disconnect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
        disconnect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
        disconnect(PMain::instance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
        disconnect(PMain::instance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
        disconnect(PMain::instance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
        disconnect(PMain::instance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
        disconnect(PMain::instance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
        disconnect(PMain::instance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));

        //if the resolution of the loaded voi is higher than the current one, opening the "next" explorer
        if(cVolume->getVoiResIndex() > volResIndex)
            this->next = new CExplorerWindow(V3D_env, cVolume->getVoiResIndex(), cVolume->getVoiData(), cVolume->getVoiV0(), cVolume->getVoiV1(),
                                             cVolume->getVoiH0(),cVolume->getVoiH1(), cVolume->getVoiD0(),cVolume->getVoiD1(), cVolume->getNChannels(), this);
        //if the resolution of the loaded voi is the same of the current one
        else if(cVolume->getVoiResIndex() == volResIndex)
        {
            //opening a new explorer in the current resolution and closing the current one
            this->next = new CExplorerWindow(this->V3D_env, volResIndex, cVolume->getVoiData(), cVolume->getVoiV0(), cVolume->getVoiV1(),
                                             cVolume->getVoiH0(),cVolume->getVoiH1(), cVolume->getVoiD0(),cVolume->getVoiD1(), cVolume->getNChannels(), this);
            prev->next = next;
            next->prev = prev;
            this->toBeClosed = true;
            delete this;
        }
        else
            QMessageBox::critical(this,QObject::tr("Error"), "The new view refers to a lower resolution. This feature is not supported yet.",QObject::tr("Ok"));
    }

    //resetting some widgets
    printf("----- Resetting some widgets....\n");
    PMain::instance()->resetGUI();
    PMain::instance()->subvol_panel->setEnabled(true);
    PMain::instance()->loadButton->setEnabled(true);
}

/**********************************************************************************
* Generates new view centered at the given 3D point on the given resolution and ha-
* ving the given dimensions (optional).  VOI's dimensions from the GUI will be used
* if dx,dy,dz are not provided.
* Called by the current <CExplorerWindow> when the user zooms in and the higher res-
* lution has to be loaded.
***********************************************************************************/
void CExplorerWindow::newView(int x, int y, int z, int resolution, bool fromVaa3Dcoordinates /* = true */,
                              int dx /* = -1 */, int dy /* = -1 */, int dz /* = -1 */)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] newView(x = %d, y = %d, z = %d, res = %d, dx=%d, dy=%d, dz=%d) launched\n",
           this->thread()->currentThreadId(),title.c_str(), x, y, z, resolution, dx, dy, dz );
    #endif

    //checks
    if(resolution >= CImport::instance()->getResolutions())
        resolution = volResIndex;

    //preparing GUI
    view3DWidget->setCursor(Qt::WaitCursor);
    PMain& pMain = *(PMain::instance());
    pMain.progressBar->setEnabled(true);
    pMain.progressBar->setMinimum(0);
    pMain.progressBar->setMaximum(0);
    pMain.loadButton->setEnabled(false);
    pMain.statusBar->showMessage("Loading...");

    //computing VOI
    float ratio = static_cast<float>(CImport::instance()->getVolume(resolution)->getDIM_D())/CImport::instance()->getVolume(volResIndex)->getDIM_D();
    int VoiCenterX = getGlobalHCoord(x, resolution, fromVaa3Dcoordinates);
    int VoiCenterY = getGlobalVCoord(y, resolution, fromVaa3Dcoordinates);
    int VoiCenterZ = getGlobalDCoord(z, resolution, fromVaa3Dcoordinates);
    dx = dx == -1 ? pMain.Hdim_sbox->value()/2 : (dx*ratio+0.5f);
    dy = dy == -1 ? pMain.Vdim_sbox->value()/2 : (dy*ratio+0.5f);
    dz = dz == -1 ? pMain.Ddim_sbox->value()/2 : (dz*ratio+0.5f);
    CVolume::instance()->setVoi(this, resolution, VoiCenterY-dy, VoiCenterY+dy, VoiCenterX-dx, VoiCenterX+dx, VoiCenterZ-dz, VoiCenterZ+dz);

    //saving min, max and values of PMain GUI VOI's widgets
    saveSubvolSpinboxState();

    //launching thread where the VOI has to be loaded
    CVolume::instance()->start();
}

/**********************************************************************************
* Saves/restore the state of PMain spinboxes for subvolume selection
***********************************************************************************/
void CExplorerWindow::saveSubvolSpinboxState()
{
    PMain& pMain = *(PMain::instance());
    V0_sbox_min = pMain.V0_sbox->minimum();
    V1_sbox_max = pMain.V1_sbox->maximum();
    H0_sbox_min = pMain.H0_sbox->minimum();
    H1_sbox_max = pMain.H1_sbox->maximum();
    D0_sbox_min = pMain.D0_sbox->minimum();
    D1_sbox_max = pMain.D1_sbox->maximum();
    V0_sbox_val = pMain.V0_sbox->value();
    V1_sbox_val = pMain.V1_sbox->value();
    H0_sbox_val = pMain.H0_sbox->value();
    H1_sbox_val = pMain.H1_sbox->value();
    D0_sbox_val = pMain.D0_sbox->value();
    D1_sbox_val = pMain.D1_sbox->value();
}
void CExplorerWindow::restoreSubvolSpinboxState()
{
    PMain& pMain = *(PMain::instance());
    pMain.V0_sbox->setMinimum(V0_sbox_min);
    pMain.V1_sbox->setMaximum(V1_sbox_max);
    pMain.H0_sbox->setMinimum(H0_sbox_min);
    pMain.H1_sbox->setMaximum(H1_sbox_max);
    pMain.D0_sbox->setMinimum(D0_sbox_min);
    pMain.D1_sbox->setMaximum(D1_sbox_max);
    pMain.V0_sbox->setValue(V0_sbox_val);
    pMain.V1_sbox->setValue(V1_sbox_val);
    pMain.H0_sbox->setValue(H0_sbox_val);
    pMain.H1_sbox->setValue(H1_sbox_val);
    pMain.D0_sbox->setValue(D0_sbox_val);
    pMain.D1_sbox->setValue(D1_sbox_val);
}

/**********************************************************************************
* Annotations are stored/loaded) to/from the <CAnnotations> object
***********************************************************************************/
void CExplorerWindow::storeAnnotations() throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] storeAnnotations() launched\n", this->thread()->currentThreadId(), title.c_str() );
    #endif

    /**********************************************************************************
    * MARKERS
    ***********************************************************************************/
    //removing original markers
    if(!loaded_markers.empty())
        CAnnotations::getInstance()->removeLandmarks(loaded_markers);

    //storing edited markers
    QList<LocationSimple> editedMarkers = triViewWidget->getImageData()->listLandmarks;
    if(!editedMarkers.empty())
    {
        //converting local coordinates into global coordinates
        for(int i=0; i<editedMarkers.size(); i++)
        {
            editedMarkers[i].x = getGlobalHCoord(editedMarkers[i].x);
            editedMarkers[i].y = getGlobalVCoord(editedMarkers[i].y);
            editedMarkers[i].z = getGlobalDCoord(editedMarkers[i].z);
        }

        //storing markers
        CAnnotations::getInstance()->addLandmarks(&editedMarkers);
    }

    /**********************************************************************************
    * CURVES
    ***********************************************************************************/
    //removing original curves
    if(!loaded_curves.empty())
        CAnnotations::getInstance()->removeCurves(loaded_curves);

    //storing edited curves
    NeuronTree editedCurves = this->V3D_env->getSWC(this->window);
    if(!editedCurves.listNeuron.empty())
    {
        //converting local coordinates into global coordinates
        for(int i=0; i<editedCurves.listNeuron.size(); i++)
        {
            editedCurves.listNeuron[i].x = getGlobalHCoord(editedCurves.listNeuron[i].x);
            editedCurves.listNeuron[i].y = getGlobalVCoord(editedCurves.listNeuron[i].y);
            editedCurves.listNeuron[i].z = getGlobalDCoord(editedCurves.listNeuron[i].z);
        }

        //storing markers
        CAnnotations::getInstance()->addCurves(&editedCurves);
    }
}

void CExplorerWindow::loadAnnotations() throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] loadAnnotations() launched\n", this->thread()->currentThreadId(), title.c_str() );
    #endif

    //clearing previous annotations (useful when this view has been already visited)
    loaded_markers.clear();
    loaded_curves.clear();
    V3D_env->getHandleNeuronTrees_Any3DViewer(window3D)->clear();

    //computing the current volume range in the highest resolution image space
    int highestResIndex = CImport::instance()->getResolutions()-1;
    int voiV0 = CVolume::scaleVCoord(volV0, volResIndex, highestResIndex);
    int voiV1 = CVolume::scaleVCoord(volV1, volResIndex, highestResIndex);
    int voiH0 = CVolume::scaleHCoord(volH0, volResIndex, highestResIndex);
    int voiH1 = CVolume::scaleHCoord(volH1, volResIndex, highestResIndex);
    int voiD0 = CVolume::scaleDCoord(volD0, volResIndex, highestResIndex);
    int voiD1 = CVolume::scaleDCoord(volD1, volResIndex, highestResIndex);
    interval_t x_range(voiH0, voiH1);
    interval_t y_range(voiV0, voiV1);
    interval_t z_range(voiD0, voiD1);

    //obtaining the annotations within the current window
    CAnnotations::getInstance()->findLandmarks(x_range, y_range, z_range, loaded_markers);
    CAnnotations::getInstance()->findCurves(x_range, y_range, z_range, loaded_curves);

    //converting global coordinates to local coordinates
    QList<LocationSimple> vaa3dMarkers;
    for(std::list<LocationSimple>::iterator i = loaded_markers.begin(); i != loaded_markers.end(); i++)
    {
        vaa3dMarkers.push_back(*i);
        vaa3dMarkers.back().x = getLocalHCoord(vaa3dMarkers.back().x);
        vaa3dMarkers.back().y = getLocalVCoord(vaa3dMarkers.back().y);
        vaa3dMarkers.back().z = getLocalDCoord(vaa3dMarkers.back().z);
    }
    NeuronTree vaa3dCurves;
    for(std::list<NeuronSWC>::iterator i = loaded_curves.begin(); i != loaded_curves.end(); i++)
    {
        vaa3dCurves.listNeuron.push_back(*i);
        vaa3dCurves.listNeuron.back().x = getLocalHCoord(vaa3dCurves.listNeuron.back().x);
        vaa3dCurves.listNeuron.back().y = getLocalVCoord(vaa3dCurves.listNeuron.back().y);
        vaa3dCurves.listNeuron.back().z = getLocalDCoord(vaa3dCurves.listNeuron.back().z);
    }
    vaa3dCurves.editable=false;

    //assigning annotations
    V3D_env->setLandmark(window, vaa3dMarkers);
    V3D_env->setSWC(window, vaa3dCurves);
    V3D_env->pushObjectIn3DWindow(window);
    view3DWidget->enableMarkerLabel(false);
}

/**********************************************************************************
* Restores the current window and destroys the next <CExplorerWindow>.
* Called by the next <CExplorerWindow> when the user zooms out and  the lower reso-
* lution has to be reestabilished.
***********************************************************************************/
void CExplorerWindow::restore() throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] restore() launched\n", this->thread()->currentThreadId(), title.c_str() );
    #endif

    if(next)
    {
        //registrating views: ---- Alessandro 2013-04-18 fixed: determining unique triple of rotation angles and assigning absolute rotation
        next->view3DWidget->absoluteRotPose();
        view3DWidget->doAbsoluteRot(next->view3DWidget->xRot(), next->view3DWidget->yRot(), next->view3DWidget->zRot());


        //positioning the current 3D window exactly at the next window position
        QPoint location = next->window3D->pos();
        next->window3D->setVisible(false);
        triViewWidget->setVisible(true);
        window3D->setVisible(true);
        triViewWidget->setWindowState(Qt::WindowMinimized);        
        window3D->resize(next->window3D->size());
        window3D->move(location);

        //applying the same color map only if it differs from the next one
        Renderer_gl2* next_renderer = (Renderer_gl2*)(next->view3DWidget->getRenderer());
        Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
        bool changed_cmap = false;
        for(int k=0; k<3; k++)
        {
            RGBA8* next_cmap = next_renderer->colormap[k];
            RGBA8* curr_cmap = curr_renderer->colormap[k];
            for(int i=0; i<256; i++)
            {
                if(curr_cmap[i].i != next_cmap[i].i)
                    changed_cmap = true;
                curr_cmap[i] = next_cmap[i];
            }
        }
        if(changed_cmap)
            curr_renderer->applyColormapToImage();

        //storing annotations done in the next view
        next->storeAnnotations();

        //closing next
        delete next;
        next = 0;

        //registrating the current window as the last window of the multiresolution explorer windows chain
        CExplorerWindow::last = this;

        //selecting the current resolution in the PMain GUI and disabling previous resolutions
        PMain* pMain = PMain::instance();
        pMain->resolution_cbox->setCurrentIndex(volResIndex);
        for(int i=0; i<pMain->resolution_cbox->count(); i++)
        {
            // Get the index of the value to disable
            QModelIndex index = pMain->resolution_cbox->model()->index(i,0);

            // These are the effective 'disable/enable' flags
            QVariant v1(Qt::NoItemFlags);
            QVariant v2(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

            //the magic
            if(i<volResIndex)
                pMain->resolution_cbox->model()->setData( index, v1, Qt::UserRole -1);
            else
                pMain->resolution_cbox->model()->setData( index, v2, Qt::UserRole -1);
        }

        //restoring min, max and value of PMain GUI VOI's widgets
        restoreSubvolSpinboxState();

        //disabling translate buttons if needed
        pMain->traslYneg->setEnabled(volV0 > 0);
        pMain->traslYpos->setEnabled(volV1 < CImport::instance()->getVolume(volResIndex)->getDIM_V());
        pMain->traslXneg->setEnabled(volH0 > 0);
        pMain->traslXpos->setEnabled(volH1 < CImport::instance()->getVolume(volResIndex)->getDIM_H());
        pMain->traslZneg->setEnabled(volD0 > 0);
        pMain->traslZpos->setEnabled(volD1 < CImport::instance()->getVolume(volResIndex)->getDIM_D());

        //signal connections
        connect(CVolume::instance(), SIGNAL(sendOperationOutcome(MyException*,void*)), this, SLOT(loadingDone(MyException*,void*)));
        connect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
        connect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
        connect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
        connect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
        connect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
        connect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
        connect(PMain::instance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
        connect(PMain::instance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
        connect(PMain::instance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
        connect(PMain::instance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
        connect(PMain::instance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
        connect(PMain::instance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));

        //loading annotations of the current view
        this->loadAnnotations();
    }
}

/**********************************************************************************
* Estimates actual image voxel size (in pixels) of the 3D renderer.
* This depends on the zoom and on the dimensions of the 3D  renderer. This calcula-
* tion is based on MATLAB curve  fitting results on  a set of measures manually ex-
* tracted on the renderer.
***********************************************************************************/
float CExplorerWindow::estimateRendererVoxelSize()
{
    float x = this->view3DWidget->height();
    float y = this->view3DWidget->zoom();

    double p00 = -36.22;
    double p10 = 0.119;
    double p01 = 0.1276;
    double p20 = -0.0001223;
    double p11 = -0.0002108;
    double p02 = -0.0007288;
    double p30 = 4.344e-008;
    double p21 = 9.092e-008;
    double p12 = 9.931e-006;
    double p03 = 3.047e-006;

    double res = p00 + p10*x + p01*y +p20*x*x +p11*x*y + p02*y*y + p30*x*x*x + p21*x*x*y +
                p12*x*y*y + p03*y*y*y;
    return (float)res;
}

/**********************************************************************************
* Returns the most  likely 3D  point in the  image that the user is pointing on the
* renderer at the given location.
* This is based on the Vaa3D 3D point selection with one mouse click.
***********************************************************************************/
XYZ CExplorerWindow::getRenderer3DPoint(int x, int y)  throw (MyException)
{
//    myRenderer* myRend = (myRenderer*)(view3DWidget->getRenderer());
//    return myRend->get3DPoint(x,y);
    throw MyException("Double-click zoom-in feature has been disabled because the necessary Vaa3D sources can't be included correctly.");
}

/**********************************************************************************
* Linked to rightStrokeROI and rightClickROI right-menu entries of the 3D renderer.
* This implements the selection of a ROI in the 3D renderer.
***********************************************************************************/
void CExplorerWindow::Vaa3D_selectedROI()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread unknown] >> CExplorerWindow::Vaa3D_selectedROI() launched\n");
    #endif

    //the ROI selection is catched only if a <CExplorerWindow> is opened
    if(last)
    {
        v3d_imaging_paras* roi = (v3d_imaging_paras*) last->view3DWidget->getiDrawExternalParameter()->image4d->getCustomStructPointer();
        int roiCenterX = roi->xe-(roi->xe-roi->xs)/2;
        int roiCenterY = roi->ye-(roi->ye-roi->ys)/2;
        int roiCenterZ = roi->ze-(roi->ze-roi->zs)/2;
        //last->switchToHigherRes(roiCenterX, roiCenterY, roiCenterZ, (roi->xe-roi->xs)/2, (roi->ye-roi->ys)/2, (roi->ze-roi->zs)/2);
        last->newView(roiCenterX, roiCenterY, roiCenterZ, volResIndex+1);
    }
}

/**********************************************************************************
* Returns  the  global coordinate  (which starts from 0) in  the given  resolution
* volume image space given the local coordinate (which starts from 0) in the current
* resolution volume image space. If resIndex is not set, the returned global coord-
* inate will be in the highest resolution image space.
***********************************************************************************/
int CExplorerWindow::getGlobalVCoord(int localVCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = true */)
{
    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volV1-volV0 > LIMIT_VOLY))
        localVCoord = static_cast<int>(localVCoord* ( static_cast<float>(volV1-volV0-1)/(LIMIT_VOLY-1) ) +0.5f);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);
    //printf("\n\n------- getGlobalVCoord(%d) = (volV0[%d]+localVCoord)*ratio[=%.4f] = %d \n\n", localVCoord, volV0, ratio, static_cast<int>((volV0+localVCoord)*ratio + 0.5f));
    return (volV0+localVCoord)*ratio + 0.5f;
}
int CExplorerWindow::getGlobalHCoord(int localHCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = true */)
{
    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volH1-volH0 > LIMIT_VOLX))
        localHCoord = static_cast<int>(localHCoord* ( static_cast<float>(volH1-volH0-1)/(LIMIT_VOLX-1) ) +0.5f);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);
    return (volH0+localHCoord)*ratio + 0.5f;
}
int CExplorerWindow::getGlobalDCoord(int localDCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = true */)
{
    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volD1-volD0 > LIMIT_VOLZ))
        localDCoord = static_cast<int>(localDCoord* ( static_cast<float>(volD1-volD0-1)/(LIMIT_VOLZ-1) ) +0.5f);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);
    return (volD0+localDCoord)*ratio + 0.5f;
}
float CExplorerWindow::getGlobalVCoord(float localVCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = true */)
{
    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volV1-volV0 > LIMIT_VOLY))
        localVCoord *= static_cast<float>(volV1-volV0-1)/(LIMIT_VOLY-1);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);
    return (volV0+localVCoord)*ratio;
}
float CExplorerWindow::getGlobalHCoord(float localHCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = true */)
{
    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volH1-volH0 > LIMIT_VOLX))
        localHCoord *= static_cast<float>(volH1-volH0-1)/(LIMIT_VOLX-1);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);
    return (volH0+localHCoord)*ratio;
}
float CExplorerWindow::getGlobalDCoord(float localDCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = true */)
{
    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volD1-volD0 > LIMIT_VOLZ))
        localDCoord *= static_cast<float>(volD1-volD0-1)/(LIMIT_VOLZ-1);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);
    return (volD0+localDCoord)*ratio;
}

/**********************************************************************************
* Returns the local coordinate (which starts from 0) in the current resolution vol-
* ume image space given the global coordinate  (which starts from 0) in the highest
* resolution volume image space.
***********************************************************************************/
int CExplorerWindow::getLocalVCoord(int highestResGlobalVCoord)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);
    int localCoord =  static_cast<int>(highestResGlobalVCoord/ratio - volV0 + 0.5f);

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(volV1-volV0 > LIMIT_VOLY)
        localCoord = static_cast<int>(localCoord* ( static_cast<float>(LIMIT_VOLY-1)/(volV1-volV0-1) ) +0.5f);
    return localCoord;
}
int CExplorerWindow::getLocalHCoord(int highestResGlobalHCoord)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);
    int localCoord =  static_cast<int>(highestResGlobalHCoord/ratio - volH0 + 0.5f);

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(volH1-volH0 > LIMIT_VOLX)
        localCoord = static_cast<int>(localCoord* ( static_cast<float>(LIMIT_VOLX-1)/(volH1-volH0-1) ) +0.5f);
    return localCoord;
}
int CExplorerWindow::getLocalDCoord(int highestResGlobalDCoord)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);
    int localCoord =  static_cast<int>(highestResGlobalDCoord/ratio - volD0 + 0.5f);

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(volD1-volD0 > LIMIT_VOLZ)
        localCoord = static_cast<int>(localCoord* ( static_cast<float>(LIMIT_VOLZ-1)/(volD1-volD0-1) ) +0.5f);
    return localCoord;
}
float CExplorerWindow::getLocalVCoord(float highestResGlobalVCoord)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);
    float localCoord = highestResGlobalVCoord/ratio - volV0;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(volV1-volV0 > LIMIT_VOLY)
        localCoord = localCoord* ( static_cast<float>(LIMIT_VOLY-1)/(volV1-volV0-1) );
    return localCoord;
}
float CExplorerWindow::getLocalHCoord(float highestResGlobalHCoord)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);
    float localCoord = highestResGlobalHCoord/ratio - volH0;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(volH1-volH0 > LIMIT_VOLX)
        localCoord = localCoord* ( static_cast<float>(LIMIT_VOLX-1)/(volH1-volH0-1) );
    return localCoord;
}
float CExplorerWindow::getLocalDCoord(float highestResGlobalDCoord)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);
    float localCoord = highestResGlobalDCoord/ratio - volD0;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(volD1-volD0 > LIMIT_VOLZ)
        localCoord = localCoord* ( static_cast<float>(LIMIT_VOLZ-1)/(volD1-volD0-1) );
    return localCoord;
}

/**********************************************************************************
* Linked to volume cut scrollbars of Vaa3D widget containing the 3D renderer.
* This implements the syncronization Vaa3D-->TeraManager of subvolume selection.
***********************************************************************************/
void CExplorerWindow::Vaa3D_changeYCut0(int s)
{
    disconnect(PMain::instance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
    PMain::instance()->V0_sbox->setValue(getGlobalVCoord(s)+1);
    connect(PMain::instance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
}
void CExplorerWindow::Vaa3D_changeYCut1(int s)
{
    disconnect(PMain::instance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
    PMain::instance()->V1_sbox->setValue(getGlobalVCoord(s)+1);
    connect(PMain::instance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
}
void CExplorerWindow::Vaa3D_changeXCut0(int s)
{
    disconnect(PMain::instance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
    PMain::instance()->H0_sbox->setValue(getGlobalHCoord(s)+1);
    connect(PMain::instance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
}
void CExplorerWindow::Vaa3D_changeXCut1(int s)
{
    disconnect(PMain::instance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
    PMain::instance()->H1_sbox->setValue(getGlobalHCoord(s)+1);
    connect(PMain::instance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
}
void CExplorerWindow::Vaa3D_changeZCut0(int s)
{
    disconnect(PMain::instance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
    PMain::instance()->D0_sbox->setValue(getGlobalDCoord(s)+1);
    connect(PMain::instance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
}
void CExplorerWindow::Vaa3D_changeZCut1(int s)
{
    disconnect(PMain::instance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));
    PMain::instance()->D1_sbox->setValue(getGlobalDCoord(s)+1);
    connect(PMain::instance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));
}

/**********************************************************************************
* Linked to PMain GUI VOI's widgets.
* This implements the syncronization TeraManager-->Vaa3D of subvolume selection.
***********************************************************************************/
void CExplorerWindow::PMain_changeV0sbox(int s)
{
    disconnect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
    view3DWidget->setYCut0(getLocalVCoord(s-1)+1);
    connect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
}
void CExplorerWindow::PMain_changeV1sbox(int s)
{
    disconnect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
    view3DWidget->setYCut1(getLocalVCoord(s-1)+1);
    connect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
}
void CExplorerWindow::PMain_changeH0sbox(int s)
{
    disconnect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
    view3DWidget->setXCut0(getLocalHCoord(s-1)+1);
    connect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
}
void CExplorerWindow::PMain_changeH1sbox(int s)
{
    disconnect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
    view3DWidget->setXCut1(getLocalHCoord(s-1)+1);
    connect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
}
void CExplorerWindow::PMain_changeD0sbox(int s)
{
    disconnect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
    view3DWidget->setZCut0(getLocalDCoord(s-1)+1);
    connect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
}
void CExplorerWindow::PMain_changeD1sbox(int s)
{
    disconnect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
    view3DWidget->setZCut1(getLocalDCoord(s-1)+1);
    connect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
}

/**********************************************************************************
* Alignes the given widget to the left(right) of the current window
***********************************************************************************/
void CExplorerWindow::alignToLeft(QWidget* widget)
{
    widget->move(window3D->x() + window3D->width() + 3, window3D->y());
    widget->setMaximumHeight(std::max(window3D->height(),widget->height()));
    widget->resize(widget->width(), window3D->height());
}
void CExplorerWindow::alignToRight(QWidget* widget)
{
    widget->move(window3D->x() - widget->width() - 3, window3D->y());
    widget->setMaximumHeight(std::max(window3D->height(),widget->height()));
    widget->resize(widget->width(), window3D->height());
}



