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
    sprintf(ctitle, "Res(%d x %d x %d),Volume(%d-%d,%d-%d,%d-%d), %d channels", CImport::instance()->getVolume(volResIndex)->getDIM_H(),
            CImport::instance()->getVolume(volResIndex)->getDIM_V(), CImport::instance()->getVolume(volResIndex)->getDIM_D(),
            volH0+1, volH1, volV0+1, volV1, volD0+1, volD1, nchannels);
    this->title = ctitle;
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
            window3D->move(location);

            //hiding both tri-view and 3D view
            prev->triViewWidget->setVisible(false);
            prev->window3D->setVisible(false);
            prev->view3DWidget->setCursor(Qt::ArrowCursor);

            //registrating views
            float ratio = CImport::instance()->getVolume(volResIndex)->getDIM_D()/CImport::instance()->getVolume(prev->volResIndex)->getDIM_D();
            view3DWidget->setZoom(prev->view3DWidget->zoom()/ratio);
            view3DWidget->setXRotation(prev->view3DWidget->xRot());
            view3DWidget->setYRotation(prev->view3DWidget->yRot());
            view3DWidget->setZRotation(prev->view3DWidget->zRot());

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
            int window_y = (screen_height - window3D->height()) / 2;
            window3D->move(window_x, window_y);
            pMain->move(window_x + window3D->width() + 3, window_y);
            pMain->setMaximumHeight(std::max(window3D->height(),pMain->height()));
            pMain->resize(pMain->width(), window3D->height());
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
            else */ if(view3DWidget->zoom() < 0    &&              //zoom-out threshold reached
                    prev                        &&              //the previous resolution exists
                    !toBeClosed)                                //the current resolution does not have to be closed
            {
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
            switchToHigherRes(point.x, point.y, point.z);
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
            PMain* pMain = PMain::instance();
            pMain->move(window3D->x() + window3D->width() + 3, window3D->y());
            pMain->setMaximumHeight(std::max(window3D->height(),pMain->height()));
            pMain->resize(pMain->width(), window3D->height());
        }
        /***************** INTERCEPTING STATE CHANGES EVENTS **********************
        Window state changes events are intercepted to let PMain's position be syn-
        cronized with the explorer.
        ***************************************************************************/
        else if(object == window3D && event->type() == QEvent::WindowStateChange)
        {
            PMain::instance()->setWindowState(window3D->windowState());
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
            QMessageBox::critical(this,QObject::tr("Error"), "",QObject::tr("Ok"));

        view3DWidget->setCursor(Qt::ArrowCursor);
    }

    //resetting some widgets
    PMain::instance()->resetGUI();
    PMain::instance()->subvol_panel->setEnabled(true);
    PMain::instance()->loadButton->setEnabled(true);
}

/**********************************************************************************
* Switches to the higher resolution centered at the given 3D point and with the gi-
* ven dimensions (optional). VOI's dimensions from the GUI will be used if dx,dy,dz
* are not provided.
* Called by the current <CExplorerWindow> when the user zooms in and the higher res-
* lution has to be loaded.
***********************************************************************************/
void CExplorerWindow::switchToHigherRes(int x, int y, int z, int dx, int dy, int dz)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] switchToHigherRes(x = %d, y = %d, z = %d, dx=%d, dy=%d, dz=%d) launched\n",
           this->thread()->currentThreadId(),title.c_str(), x, y, z, dx, dy, dz );
    #endif

    //preparing GUI
    view3DWidget->setCursor(Qt::WaitCursor);
    PMain& pMain = *(PMain::instance());
    pMain.progressBar->setEnabled(true);
    pMain.progressBar->setMinimum(0);
    pMain.progressBar->setMaximum(0);
    pMain.loadButton->setEnabled(false);
    //pMain.import_form->setEnabled(false);
    if(CImport::instance()->getVolume(volResIndex+1))
        pMain.statusBar->showMessage("Zooming in to the higher resolution...");
    else
        pMain.statusBar->showMessage("Loading selected VOI...");

    //if the higher resolution exists, obtaining VOI at the higher resolution
    if(CImport::instance()->getVolume(volResIndex+1))
    {
        float ratio = static_cast<float>(CImport::instance()->getVolume(volResIndex+1)->getDIM_D())/CImport::instance()->getVolume(volResIndex)->getDIM_D();
        /*int VoiCenterX = (x + volH0)*ratio+0.5f;
        int VoiCenterY = (y + volV0)*ratio+0.5f;
        int VoiCenterZ = (z + volD0)*ratio+0.5f;*/
        int VoiCenterX = getGlobalHCoord(x, volResIndex+1);
        int VoiCenterY = getGlobalVCoord(y, volResIndex+1);
        int VoiCenterZ = getGlobalDCoord(z, volResIndex+1);
        dx = dx == -1 ? pMain.Hdim_sbox->value()/2 : (dx*ratio+0.5f);
        dy = dy == -1 ? pMain.Vdim_sbox->value()/2 : (dy*ratio+0.5f);
        dz = dz == -1 ? pMain.Ddim_sbox->value()/2 : (dz*ratio+0.5f);
        CVolume::instance()->setVoi(this, volResIndex+1, VoiCenterY-dy, VoiCenterY+dy, VoiCenterX-dx, VoiCenterX+dx, VoiCenterZ-dz, VoiCenterZ+dz);
    }
    //otherwise obtaining VOI at the current resolution
    else
    {
        int VoiCenterX = getGlobalHCoord(x, volResIndex);
        int VoiCenterY = getGlobalVCoord(y, volResIndex);
        int VoiCenterZ = getGlobalDCoord(z, volResIndex);
        dx = dx == -1 ? pMain.Hdim_sbox->value()/2 : dx;
        dy = dy == -1 ? pMain.Vdim_sbox->value()/2 : dy;
        dz = dz == -1 ? pMain.Ddim_sbox->value()/2 : dz;
        CVolume::instance()->setVoi(this, volResIndex, VoiCenterY-dy, VoiCenterY+dy, VoiCenterX-dx, VoiCenterX+dx, VoiCenterZ-dz, VoiCenterZ+dz);
    }

    //launching thread where the VOI has to be loaded
    CVolume::instance()->start();
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
    //retrieving new and deleted markers
    LandmarkList new_markers, deleted_markers;
    for(int i=0; i<loaded_markers.size(); i++)
        if(triViewWidget->getImageData()->listLandmarks.contains(loaded_markers[i]) == false)
            deleted_markers.push_back(loaded_markers[i]);
    for(int i=0; i<triViewWidget->getImageData()->listLandmarks.size(); i++)
        if(loaded_markers.contains(triViewWidget->getImageData()->listLandmarks[i]) == false)
            new_markers.push_back(triViewWidget->getImageData()->listLandmarks[i]);

    //storing new markers
    if(!new_markers.empty())
    {
        //converting local coordinates into global coordinates
        for(int i=0; i<new_markers.size(); i++)
        {
            new_markers[i].x = getGlobalHCoord(new_markers[i].x);
            new_markers[i].y = getGlobalVCoord(new_markers[i].y);
            new_markers[i].z = getGlobalDCoord(new_markers[i].z);
        }

        //storing markers
        CAnnotations::getInstance()->addLandmarks(&new_markers);
    }

    //removing deleted markers
    if(!deleted_markers.empty())
    {
        //converting local coordinates into global coordinates
        for(int i=0; i<deleted_markers.size(); i++)
        {
            deleted_markers[i].x = getGlobalHCoord(deleted_markers[i].x);
            deleted_markers[i].y = getGlobalVCoord(deleted_markers[i].y);
            deleted_markers[i].z = getGlobalDCoord(deleted_markers[i].z);
        }

        //removing markers
        CAnnotations::getInstance()->removeLandmarks(&deleted_markers);
    }

    /**********************************************************************************
    * CURVES
    ***********************************************************************************/
    NeuronTree editedCurves = this->V3D_env->getSWC(this->window);

    //storing new curves
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
    loaded_curves.listNeuron.clear();
    loaded_curves.hashNeuron.clear();

    //computing the current volume range in the highest resolution image space
    interval_t x_range(getGlobalHCoord(0), getGlobalHCoord(static_cast<int>(triViewWidget->getImageData()->getXDim())));
    interval_t y_range(getGlobalVCoord(0), getGlobalVCoord(static_cast<int>(triViewWidget->getImageData()->getYDim())));
    interval_t z_range(getGlobalDCoord(0), getGlobalDCoord(static_cast<int>(triViewWidget->getImageData()->getZDim())));

    //obtaining the annotations within the current window
    CAnnotations::getInstance()->findLandmarks(x_range, y_range, z_range, loaded_markers);
    CAnnotations::getInstance()->findCurves(x_range, y_range, z_range, loaded_curves);

    //converting global coordinates into local coordinates
    for(int i=0; i<loaded_markers.size(); i++)
    {
        loaded_markers[i].x = getLocalHCoord(loaded_markers[i].x);
        loaded_markers[i].y = getLocalVCoord(loaded_markers[i].y);
        loaded_markers[i].z = getLocalDCoord(loaded_markers[i].z);
        //printf("Marker[%d] = {%.0f %.0f %.0f}\n", i, loaded_markers[i].x, loaded_markers[i].y, loaded_markers[i].z);
    }
    for(int i=0; i<loaded_curves.listNeuron.size(); i++)
    {
        loaded_curves.listNeuron[i].x = getLocalHCoord(loaded_curves.listNeuron[i].x);
        loaded_curves.listNeuron[i].y = getLocalVCoord(loaded_curves.listNeuron[i].y);
        loaded_curves.listNeuron[i].z = getLocalDCoord(loaded_curves.listNeuron[i].z);
        //printf("CurveNode[%d] = {%.0f %.0f %.0f}\n", i, loaded_curves.listNeuron[i].x, loaded_curves.listNeuron[i].y, loaded_curves.listNeuron[i].z);
    }

    //assigning annotations
    V3D_env->setLandmark(window, loaded_markers);
    V3D_env->setSWC(window, loaded_curves);
    V3D_env->pushObjectIn3DWindow(window);
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
        //positioning the current 3D window exactly at the next window position
        QPoint location = next->window3D->pos();
        next->window3D->setVisible(false);
        triViewWidget->setVisible(true);
        window3D->setVisible(true);
        triViewWidget->setWindowState(Qt::WindowMinimized);        
        window3D->resize(next->window3D->size());
        window3D->move(location);

        //registrating views
        view3DWidget->setXRotation(next->view3DWidget->xRot());
        view3DWidget->setYRotation(next->view3DWidget->yRot());
        view3DWidget->setZRotation(next->view3DWidget->zRot());

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

        //setting min, max and value of PMain GUI VOI's widgets
        pMain->V0_sbox->setMinimum(getGlobalVCoord(volV0)+1);
        pMain->V1_sbox->setMaximum(getGlobalVCoord(volV1)+1);
        pMain->H0_sbox->setMinimum(getGlobalHCoord(volH0)+1);
        pMain->H1_sbox->setMaximum(getGlobalHCoord(volH1)+1);
        pMain->D0_sbox->setMinimum(getGlobalDCoord(volD0)+1);
        pMain->D1_sbox->setMaximum(getGlobalDCoord(volD1)+1);

        pMain->V0_sbox->setValue(getGlobalDCoord(view3DWidget->yCut0())+1);
        pMain->V1_sbox->setValue(getGlobalDCoord(view3DWidget->yCut1())+1);
        pMain->H0_sbox->setValue(getGlobalDCoord(view3DWidget->xCut0())+1);
        pMain->H1_sbox->setValue(getGlobalDCoord(view3DWidget->xCut1())+1);
        pMain->D0_sbox->setValue(getGlobalDCoord(view3DWidget->zCut0())+1);
        pMain->D1_sbox->setValue(getGlobalDCoord(view3DWidget->zCut1())+1);

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
    /*myRenderer* myRend = (myRenderer*)(view3DWidget->getRenderer());
    return myRend->get3DPoint(x,y);*/
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
        last->switchToHigherRes(roiCenterX, roiCenterY, roiCenterZ);
    }
}

/**********************************************************************************
* Returns  the  global coordinate  (which starts from 0) in  the given  resolution
* volume image space given the local coordinate (which starts from 0) in the current
* resolution volume image space. If resIndex is not set, the returned global coord-
* inate will be in the highest resolution image space.
***********************************************************************************/
int CExplorerWindow::getGlobalVCoord(int localVCoord, int resIndex /* = -1 */)
{
    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the non-downsampled image space coordinate system
    if(volV1-volV0 > LIMIT_VOLY)
        localVCoord = static_cast<int>(localVCoord* ( static_cast<float>(volV1-volV0-1)/(LIMIT_VOLY-1) ) +0.5f);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);
    return (volV0+localVCoord)*ratio + 0.5f;
}
int CExplorerWindow::getGlobalHCoord(int localHCoord, int resIndex /* = -1 */)
{
    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the non-downsampled image space coordinate system
    if(volH1-volH0 > LIMIT_VOLX)
        localHCoord = static_cast<int>(localHCoord* ( static_cast<float>(volH1-volH0-1)/(LIMIT_VOLX-1) ) +0.5f);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);
    return (volH0+localHCoord)*ratio + 0.5f;
}
int CExplorerWindow::getGlobalDCoord(int localDCoord, int resIndex /* = -1 */)
{
    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the non-downsampled image space coordinate system
    if(volD1-volD0 > LIMIT_VOLZ)
        localDCoord = static_cast<int>(localDCoord* ( static_cast<float>(volD1-volD0-1)/(LIMIT_VOLZ-1) ) +0.5f);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);
    return (volD0+localDCoord)*ratio + 0.5f;
}
float CExplorerWindow::getGlobalVCoord(float localVCoord, int resIndex /* = -1 */)
{
    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the non-downsampled image space coordinate system
    if(volV1-volV0 > LIMIT_VOLY)
        localVCoord *= static_cast<float>(volV1-volV0-1)/(LIMIT_VOLY-1);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);
    return (volV0+localVCoord)*ratio;
}
float CExplorerWindow::getGlobalHCoord(float localHCoord, int resIndex /* = -1 */)
{
    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the non-downsampled image space coordinate system
    if(volH1-volH0 > LIMIT_VOLX)
        localHCoord *= static_cast<float>(volH1-volH0-1)/(LIMIT_VOLX-1);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);
    return (volH0+localHCoord)*ratio;
}
float CExplorerWindow::getGlobalDCoord(float localDCoord, int resIndex /* = -1 */)
{
    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the non-downsampled image space coordinate system
    if(volD1-volD0 > LIMIT_VOLZ)
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


