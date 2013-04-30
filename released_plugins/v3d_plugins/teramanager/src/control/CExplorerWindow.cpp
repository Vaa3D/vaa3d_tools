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
#include "CVolume.h"
#include "CAnnotations.h"
#include "../presentation/PMain.h"
#include "renderer_gl1.h"
#include "v3dr_colormapDialog.h"
#include "V3Dsubclasses.h"

using namespace teramanager;

CExplorerWindow* CExplorerWindow::first = 0;
CExplorerWindow* CExplorerWindow::last = 0;
CExplorerWindow* CExplorerWindow::current = 0;
int CExplorerWindow::nInstances = 0;

CExplorerWindow::CExplorerWindow(V3DPluginCallback2 *_V3D_env, int _resIndex, uint8 *imgData, int _volV0, int _volV1,
                                 int _volH0, int _volH1, int _volD0, int _volD1, int _nchannels, CExplorerWindow *_prev): QWidget()
{
    //initializations
    resetZoomHistory();
    setActive(false);
    this->V3D_env = _V3D_env;
    this->prev = _prev;
    this->next = 0;
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
    sprintf(ctitle, "Res{%d}), Vol{[%d,%d) [%d,%d) [%d,%d)}", volResIndex, volH0, volH1, volV0, volV1, volD0, volD1);
    this->titleShort = ctitle;
    V0_sbox_min = V1_sbox_max = H0_sbox_min = H1_sbox_max = D0_sbox_min = D1_sbox_max = V0_sbox_val = V1_sbox_val = H0_sbox_val = H1_sbox_val = D0_sbox_val = D1_sbox_val = -1;
    PMain* pMain = PMain::getInstance();

    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::CExplorerWindow()\n",  titleShort.c_str());
    #endif

    try
    {
        //making prev (if exist) the last view and checking that it belongs to a lower resolution
        if(prev)
        {
            prev->makeLastView();
            if(prev->volResIndex > volResIndex)
                throw MyException("in CExplorerWindow(): attempting to break the ascending order of resolution history. This feature is not supported yet.");
        }

        //check that the number of instantiated objects does not exceed the number of available resolutions
        nInstances++;
        #ifdef TMP_DEBUG
        printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow::nInstances++, nInstances = %d\n",  nInstances);
        #endif
        if(nInstances > CImport::instance()->getResolutions() +1)
            throw MyException(QString("in CExplorerWindow(): exceeded the maximum number of views opened at the same time.\n\nPlease signal this issue to developers.").toStdString().c_str());


        //deactivating previous window and activating the current one
        if(prev)
        {
            prev->view3DWidget->removeEventFilter(prev);
            prev->window3D->removeEventFilter(prev);
            prev->resetZoomHistory();
            prev->setActive(false);
        }
        setActive(true);

        //opening tri-view window (and hiding it asap)
        this->window = V3D_env->newImageWindow(QString(title.c_str()));
        this->triViewWidget = (XFormWidget*)window;
        triViewWidget->setWindowState(Qt::WindowMinimized);
        Image4DSimple* image = new Image4DSimple();
        image->setFileName(title.c_str());
        image->setData(imgData, volH1-volH0, volV1-volV0, volD1-volD0, nchannels, V3D_UINT8);
        V3D_env->setImage(window, image);

        //opening 3D view window
        V3D_env->open3DWindow(window);
        view3DWidget = (V3dR_GLWidget*)(V3D_env->getView3DControl(window));
        window3D = view3DWidget->getiDrawExternalParameter()->window3D;

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
            window3D->resize(prev->window3D->size());
            window3D->move(location);

            //hiding both tri-view and 3D view
            prev->window3D->setVisible(false);
            prev->triViewWidget->setVisible(false);
            prev->view3DWidget->setCursor(Qt::ArrowCursor);

            //registrating views: ---- Alessandro 2013-04-18 fixed: determining unique triple of rotation angles and assigning absolute rotation
            float ratio = CImport::instance()->getVolume(volResIndex)->getDIM_D()/CImport::instance()->getVolume(prev->volResIndex)->getDIM_D();
            view3DWidget->setZoom(prev->view3DWidget->zoom()/ratio);
            prev->view3DWidget->absoluteRotPose();
            view3DWidget->doAbsoluteRot(prev->view3DWidget->xRot(), prev->view3DWidget->yRot(), prev->view3DWidget->zRot());

            //sync widgets
            syncWindows(prev->window3D, window3D);

            //storing annotations done in the previous view and loading annotations of the current view
            prev->storeAnnotations();
            this->loadAnnotations();
        }
        //otherwise this is the lowest resolution window
        else
        {
            //registrating the current window as the first window of the multiresolution explorer windows chain
            CExplorerWindow::first = this;

            //increasing height if lower than the plugin one's
            if(window3D->height() < PMain::getInstance()->height())
                window3D->setMinimumHeight(PMain::getInstance()->height());

            //centering the current 3D window and the plugin's window
            int screen_height = qApp->desktop()->availableGeometry().height();
            int screen_width = qApp->desktop()->availableGeometry().width();
            int window_x = (screen_width - (window3D->width() + PMain::getInstance()->width()))/2;
//            int window_x = (screen_width - (window3D->width() + PMain::getInstance()->width()))/2 + PMain::instance()->width();
            int window_y = (screen_height - window3D->height()) / 2;
            window3D->move(window_x, window_y);
        }

        //registrating the current window as the last and current window of the multiresolution explorer windows chain
        CExplorerWindow::last = this;
        CExplorerWindow::current = this;

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
        pMain->gradientBar->setStep(volResIndex);
        pMain->gradientBar->update();

        //disabling translate buttons if needed
        pMain->traslYneg->setEnabled(volV0 > 0);
        pMain->traslYpos->setEnabled(volV1 < CImport::instance()->getVolume(volResIndex)->getDIM_V());
        pMain->traslXneg->setEnabled(volH0 > 0);
        pMain->traslXpos->setEnabled(volH1 < CImport::instance()->getVolume(volResIndex)->getDIM_H());
        pMain->traslZneg->setEnabled(volD0 > 0);
        pMain->traslZpos->setEnabled(volD1 < CImport::instance()->getVolume(volResIndex)->getDIM_D());

        //setting min, max and value of PMain GUI VOI's widgets
        pMain->V0_sbox->setMinimum(getGlobalVCoord(view3DWidget->yCut0(), -1, true)+1);
        pMain->V0_sbox->setValue(pMain->V0_sbox->minimum());
        pMain->V1_sbox->setMaximum(getGlobalVCoord(view3DWidget->yCut1(), -1, true)+1);
        pMain->V1_sbox->setValue(pMain->V1_sbox->maximum());
        pMain->H0_sbox->setMinimum(getGlobalHCoord(view3DWidget->xCut0(), -1, true)+1);
        pMain->H0_sbox->setValue(pMain->H0_sbox->minimum());
        pMain->H1_sbox->setMaximum(getGlobalHCoord(view3DWidget->xCut1(), -1, true)+1);
        pMain->H1_sbox->setValue(pMain->H1_sbox->maximum());
        pMain->D0_sbox->setMinimum(getGlobalDCoord(view3DWidget->zCut0(), -1, true)+1);
        pMain->D0_sbox->setValue(pMain->D0_sbox->minimum());
        pMain->D1_sbox->setMaximum(getGlobalDCoord(view3DWidget->zCut1(), -1, true)+1);
        pMain->D1_sbox->setValue(pMain->D1_sbox->maximum());

        //signal connections
        connect(CVolume::instance(), SIGNAL(sendOperationOutcome(MyException*,void*)), this, SLOT(loadingDone(MyException*,void*)), Qt::QueuedConnection);
        connect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
        connect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
        connect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
        connect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
        connect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
        connect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
        connect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
        connect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
        connect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
        connect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
        connect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
        connect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));

//        disconnect(window3D->zoomSlider, SIGNAL(valueChanged(int)), view3DWidget, SLOT(setZoom(int)));
//        connect(window3D->zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setZoom(int)));

        //changing window flags (disabling minimize/maximize buttons)
        // ---- Alessandro 2013-04-22 fixed: this causes (somehow) window3D not to respond correctly to the move() method
//        window3D->setWindowFlags(Qt::Tool
//                                 | Qt::WindowTitleHint
//                                 | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
        this->window3D->raise();
        this->window3D->activateWindow();
        this->window3D->show();

        //saving subvol spinboxes state ---- Alessandro 2013-04-23: not sure if this is really needed
        saveSubvolSpinboxState();
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(PMain::getInstance(),QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::getInstance()->closeVolume();
    }
    catch(const char* error)
    {
        QMessageBox::critical(PMain::getInstance(),QObject::tr("Error"), QObject::tr(error),QObject::tr("Ok"));
        PMain::getInstance()->closeVolume();
    }
    catch(...)
    {
        QMessageBox::critical(PMain::getInstance(),QObject::tr("Error"), QObject::tr("Unknown error occurred"),QObject::tr("Ok"));
        PMain::getInstance()->closeVolume();
    }

    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s] created\n",  titleShort.c_str());
    #endif
}

CExplorerWindow::~CExplorerWindow()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::~CExplorerWindow()\n",  titleShort.c_str() );
    #endif

    //removing the event filter from the 3D renderer and from the 3D window
    view3DWidget->removeEventFilter(this);
    window3D->removeEventFilter(this);

    //just closing windows
    V3D_env->close3DWindow(window);
    triViewWidget->close();

    //decreasing the number of instantiated objects
    nInstances--;

    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow::nInstances--, nInstances = %d\n",  nInstances);
    #endif

    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s] destroyed\n",  titleShort.c_str() );
    #endif
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
        //ignoring all events when window is not active
        if(!isActive)
        {
            //printf("Ignoring event from CExplorerWindow[%s] cause it's not active\n", title.c_str());
            event->ignore();
            return true;
        }

        //updating zoom factor history
        zoomHistoryPushBack(view3DWidget->zoom());

        /******************** INTERCEPTING ZOOMING CHANGES *************************
        Zoom-in and zoom-out changes generated by the current 3D renderer are inter-
        cepted to switch to the higher/lower resolution.
        ***************************************************************************/
        //---- Alessandro 2013-04-25: triggering zoom-in is now Vaa3D responsibility
        //if (object == view3DWidget && event->type() == QEvent::Wheel)
        {
//            if(next                                                                         &&  //the next resolution exists
//               isZoomDerivativePos()                                                        &&  //zoom derivative is positive
//               view3DWidget->zoom() > 130 -PMain::instance()->zoomSensitivity->value())         //zoom-in threshold reached
//            {
//                //printf("Switching to the next view zoom={%d, %d, %d, %d}\n", zoomHistory[0], zoomHistory[1], zoomHistory[2], zoomHistory[3]);
//                setActive(false);
//                resetZoomHistory();
//                next->restoreViewFrom(this);
//                event->ignore();
//                return true;
//            }
//            else
            if(prev                                                                         &&  //the previous resolution exists
                    !toBeClosed                                                             &&  //the current resolution does not have to be closed
                    isZoomDerivativeNeg()                                                   &&  //zoom derivative is negative
                    view3DWidget->zoom() < PMain::getInstance()->zoomOutSens->value())          //zoom-out threshold reached
            {
                //printf("Switching to the prev view zoom={%d, %d, %d, %d}\n", zoomHistory[0], zoomHistory[1], zoomHistory[2], zoomHistory[3]);
                setActive(false);
                resetZoomHistory();
                prev->restoreViewFrom(this);
                event->ignore();
                return true;
            }
        }
        /******************** REDIRECTING MOUSE-WHEEL EVENTS *************************
        Mouse-wheel events are redirected to the customized wheelEvent handler
        ***************************************************************************/
//        if ((object == view3DWidget || object == window3D) && event->type() == QEvent::Wheel)
//        {
////            QWheelEvent* wheelEvt = (QWheelEvent*)event;
////            myV3dR_GLWidget::wheelEventO(view3DWidget, wheelEvt);
////            return true;
//        }
        /****************** INTERCEPTING DOUBLE CLICK EVENTS ***********************
        Double click events are intercepted to switch to the higher resolution.
        ***************************************************************************/
        if (object == view3DWidget && event->type() == QEvent::MouseButtonDblClick)
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
            {
                PMain::getInstance()->closeVolume();
                event->ignore();
                return true;
            }
        }
        /**************** INTERCEPTING MOVING/RESIZING EVENTS *********************
        Window moving and resizing events  are intercepted  to let PMain's position
        be syncronized with the explorer.
        ***************************************************************************/
        else if(object == window3D && (event->type() == QEvent::Move || event->type() == QEvent::Resize))
        {
           alignToLeft(PMain::getInstance());
//            alignToRight(PMain::instance());
        }
        /***************** INTERCEPTING STATE CHANGES EVENTS **********************
        Window state changes events are intercepted to let PMain's position be syn-
        cronized with the explorer.
        ***************************************************************************/
        else if(object == window3D && event->type() == QEvent::WindowStateChange)
        {
            alignToLeft(PMain::getInstance());
        }
        return false;
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(PMain::getInstance(),QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
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
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::loadingDone(%s)\n",
            titleShort.c_str(),  (ex? "error" : ""));
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
        disconnect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
        disconnect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
        disconnect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
        disconnect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
        disconnect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
        disconnect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));

        //if the resolution of the loaded voi is higher than the current one, opening the "next" explorer
        if(cVolume->getVoiResIndex() > volResIndex)
            this->next = new CExplorerWindow(V3D_env, cVolume->getVoiResIndex(), cVolume->getVoiData(), cVolume->getVoiV0(), cVolume->getVoiV1(),
                                             cVolume->getVoiH0(),cVolume->getVoiH1(), cVolume->getVoiD0(),cVolume->getVoiD1(), cVolume->getNChannels(), this);
        //if the resolution of the loaded voi is the same of the current one
        else if(cVolume->getVoiResIndex() == volResIndex)
        {
            //uninstalling event filter
            view3DWidget->removeEventFilter(this);
            window3D->removeEventFilter(this);

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
    PMain::getInstance()->resetGUI();
    PMain::getInstance()->subvol_panel->setEnabled(true);
    PMain::getInstance()->loadButton->setEnabled(true);
}

/**********************************************************************************
* Generates new view centered at the given 3D point on the given resolution and ha-
* ving the given dimensions (optional).  VOI's dimensions from the GUI will be used
* if dx,dy,dz are not provided.
* Called by the current <CExplorerWindow> when the user zooms in and the higher res-
* lution has to be loaded.
***********************************************************************************/
void CExplorerWindow::newView(int x, int y, int z, int resolution, bool fromVaa3Dcoordinates /* = false */,
                              int dx /* = -1 */, int dy /* = -1 */, int dz /* = -1 */)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::newView(x = %d, y = %d, z = %d, res = %d, dx = %d, dy = %d, dz = %d)\n",
           title.c_str(), x, y, z, resolution, dx, dy, dz );
    #endif

    //checks
    if(resolution >= CImport::instance()->getResolutions())
        resolution = volResIndex;

    //deactivating current window
    setActive(false);

    //preparing GUI
    view3DWidget->setCursor(Qt::WaitCursor);
    PMain& pMain = *(PMain::getInstance());
    pMain.progressBar->setEnabled(true);
    pMain.progressBar->setMinimum(0);
    pMain.progressBar->setMaximum(0);
    pMain.loadButton->setEnabled(false);
    pMain.statusBar->showMessage("Loading...");

    //computing VOI in the coordinates of the given resolution
    float ratio = static_cast<float>(CImport::instance()->getVolume(resolution)->getDIM_D())/CImport::instance()->getVolume(volResIndex)->getDIM_D();
    int VoiCenterX = getGlobalHCoord(x, resolution, fromVaa3Dcoordinates);
    int VoiCenterY = getGlobalVCoord(y, resolution, fromVaa3Dcoordinates);
    int VoiCenterZ = getGlobalDCoord(z, resolution, fromVaa3Dcoordinates);
    dx = dx == -1 ? int_inf : static_cast<int>(dx*ratio+0.5f);
    dy = dy == -1 ? int_inf : static_cast<int>(dy*ratio+0.5f);
    dz = dz == -1 ? int_inf : static_cast<int>(dz*ratio+0.5f);

    //---- Alessandro 2013-04-25: cropping bounding box if its larger than the maximum allowed
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]: cropping bbox dims from (%d,%d,%d) to ",
           title.c_str(), dx, dy, dz );
    dx = std::min(dx, static_cast<int>(pMain.Hdim_sbox->value()/2.0f+0.5f));
    dy = std::min(dy, static_cast<int>(pMain.Vdim_sbox->value()/2.0f+0.5f));
    dz = std::min(dz, static_cast<int>(pMain.Ddim_sbox->value()/2.0f+0.5f));
    printf("(%d,%d,%d)\n", dx, dy, dz );

    CVolume::instance()->setVoi(this, resolution, VoiCenterY-dy, VoiCenterY+dy, VoiCenterX-dx, VoiCenterX+dx, VoiCenterZ-dz, VoiCenterZ+dz);

    //saving min, max and values of PMain GUI VOI's widgets
    saveSubvolSpinboxState();

    //launching thread where the VOI has to be loaded
    CVolume::instance()->start();
}

/**********************************************************************************
* Makes the current view the last one by  deleting (and deallocting) its subsequent
* views.
***********************************************************************************/
void CExplorerWindow::makeLastView() throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::makeLastView()\n",
            titleShort.c_str());
    #endif

    if(CExplorerWindow::current != this)
        throw MyException(QString("in CExplorerWindow::makeLastView(): this view is not the current one, thus can't be make the last view").toStdString().c_str());

    while(CExplorerWindow::last != this)
    {
        CExplorerWindow::last = CExplorerWindow::last->prev;
        CExplorerWindow::last->next->toBeClosed = true;
        delete CExplorerWindow::last->next;
        CExplorerWindow::last->next = 0;
    }
}

/**********************************************************************************
* Saves/restore the state of PMain spinboxes for subvolume selection
***********************************************************************************/
void CExplorerWindow::saveSubvolSpinboxState()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::saveSubvolSpinboxState()\n",
            titleShort.c_str());
    #endif

    PMain& pMain = *(PMain::getInstance());
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
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::restoreSubvolSpinboxState()\n",
            titleShort.c_str());
    #endif

    PMain& pMain = *(PMain::getInstance());
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
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::storeAnnotations()\n",  titleShort.c_str() );
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
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::loadAnnotations()\n",  titleShort.c_str() );
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
* Restores the current view from the given (neighboring) view.
* Called by the next(prev) <CExplorerWindow>  when the user  zooms out(in) and  the
* lower(higher) resoolution has to be reestabilished.
***********************************************************************************/
void CExplorerWindow::restoreViewFrom(CExplorerWindow* source) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::restoreViewFrom([%s])\n",
            titleShort.c_str(), source->titleShort.c_str() );
    #endif

    if(source)
    {
        //signal disconnections
        source->disconnect(CVolume::instance(), SIGNAL(sendOperationOutcome(MyException*,void*)), source, SLOT(loadingDone(MyException*,void*)));
        source->disconnect(source->view3DWidget, SIGNAL(changeXCut0(int)), source, SLOT(Vaa3D_changeXCut0(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeXCut1(int)), source, SLOT(Vaa3D_changeXCut1(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeYCut0(int)), source, SLOT(Vaa3D_changeYCut0(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeYCut1(int)), source, SLOT(Vaa3D_changeYCut1(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeZCut0(int)), source, SLOT(Vaa3D_changeZCut0(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeZCut1(int)), source, SLOT(Vaa3D_changeZCut1(int)));
        source->disconnect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), source, SLOT(PMain_changeV0sbox(int)));
        source->disconnect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), source, SLOT(PMain_changeV1sbox(int)));
        source->disconnect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), source, SLOT(PMain_changeH0sbox(int)));
        source->disconnect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), source, SLOT(PMain_changeH1sbox(int)));
        source->disconnect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), source, SLOT(PMain_changeD0sbox(int)));
        source->disconnect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), source, SLOT(PMain_changeD1sbox(int)));

        source->view3DWidget->removeEventFilter(source);
        source->window3D->removeEventFilter(source);

        //saving source spinbox state
        source->saveSubvolSpinboxState();

        //activating current view
        setActive(true);

        //registrating views: ---- Alessandro 2013-04-18 fixed: determining unique triple of rotation angles and assigning absolute rotation
        source->view3DWidget->absoluteRotPose();
        view3DWidget->doAbsoluteRot(source->view3DWidget->xRot(), source->view3DWidget->yRot(), source->view3DWidget->zRot());

        //setting zoom only if user has zoomed in
        if(source->volResIndex < volResIndex)
        {
            float ratio = CImport::instance()->getVolume(volResIndex)->getDIM_D()/CImport::instance()->getVolume(source->volResIndex)->getDIM_D();
            view3DWidget->setZoom(source->view3DWidget->zoom()/ratio);
        }
        if(source->volResIndex > volResIndex)
        {
//            float ratio = CImport::instance()->getVolume(volResIndex)->getDIM_D()/CImport::instance()->getVolume(source->volResIndex)->getDIM_D();
            if(this != first)
                view3DWidget->setZoom(16);
            else
                view3DWidget->setZoom(30);
        }


        //positioning the current 3D window exactly at the <source> window position
        QPoint location = source->window3D->pos();
        triViewWidget->setVisible(true);
        triViewWidget->setWindowState(Qt::WindowMinimized);
        window3D->setVisible(true);
        window3D->resize(source->window3D->size());
        window3D->move(location);

        source->window3D->setVisible(false);
        source->triViewWidget->setVisible(false);
        source->view3DWidget->setCursor(Qt::ArrowCursor);

        //applying the same color map only if it differs from the source one
        Renderer_gl2* source_renderer = (Renderer_gl2*)(source->view3DWidget->getRenderer());
        Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
        bool changed_cmap = false;
        for(int k=0; k<3; k++)
        {
            RGBA8* source_cmap = source_renderer->colormap[k];
            RGBA8* curr_cmap = curr_renderer->colormap[k];
            for(int i=0; i<256; i++)
            {
                if(curr_cmap[i].i != source_cmap[i].i)
                    changed_cmap = true;
                curr_cmap[i] = source_cmap[i];
            }
        }
        if(changed_cmap)
            curr_renderer->applyColormapToImage();

        //storing annotations done in the source view
        source->storeAnnotations();

        //registrating the current window as the current window of the multiresolution explorer windows chain
        CExplorerWindow::current = this;

        //selecting the current resolution in the PMain GUI and disabling previous resolutions
        PMain* pMain = PMain::getInstance();
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
        pMain->gradientBar->setStep(volResIndex);
        pMain->gradientBar->update();

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
        connect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
        connect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
        connect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
        connect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
        connect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
        connect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));

        view3DWidget->installEventFilter(this);
        window3D->installEventFilter(this);

        //loading annotations of the current view
        this->loadAnnotations();

        //sync widgets
        syncWindows(source->window3D, window3D);

        //showing window
        this->window3D->raise();
        this->window3D->activateWindow();
        this->window3D->show();
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
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::getRenderer3DPoint(x = %d, y = %d)\n",
            titleShort.c_str(), x, y );
    #endif

//    if(QMessageBox::Yes == QMessageBox::question(this, "Confirm", QString("Double-click zoom-in feature may cause Vaa3D to crash. Do you confirm?"), QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes))
//    {
//        Renderer_gl1::MarkerPos pos;
//        pos.x = x;
//        pos.y = y;
//        for (int i=0; i<4; i++)
//                pos.view[i] = ((Renderer_gl1*)view3DWidget->getRenderer())->viewport[i];
//        for (int i=0; i<16; i++)
//        {
//            pos.P[i]  = ((Renderer_gl1*)view3DWidget->getRenderer())->projectionMatrix[i];
//            pos.MV[i] = ((Renderer_gl1*)view3DWidget->getRenderer())->markerViewMatrix[i];
//        }
//        return ((Renderer_gl1*)view3DWidget->getRenderer())->getCenterOfMarkerPos(pos);
//    }
//    else
        throw MyException("Action cancelled by the user");

//    throw MyException("Double click zoom-in has been temporarily disabled.");
//    return myRenderer_gl1::get3DPoint(static_cast<Renderer_gl1*>(view3DWidget->getRenderer()), x, y);
}

/**********************************************************************************
* method (indirectly) invoked by Vaa3D to propagate VOI's coordinates
***********************************************************************************/
void CExplorerWindow::invokedFromVaa3D(v3d_imaging_paras* params /* = 0 */)
{
    v3d_imaging_paras* roi = params? params : (v3d_imaging_paras*) current->view3DWidget->getiDrawExternalParameter()->image4d->getCustomStructPointer();
    #ifdef TMP_DEBUG
    if(!current)
        printf("--------------------- teramanager plugin [thread ?] >> CExplorerWindow::Vaa3D_selectedROI([%d-%d], [%d-%d], [%d-%d])\n",
               roi->xs, roi->xe, roi->ys, roi->ye, roi->zs, roi->ze);
    else
        printf("--------------------- teramanager plugin [thread ?] >> CExplorerWindow[%s]::Vaa3D_selectedROI([%d-%d], [%d-%d], [%d-%d])\n",
               current->titleShort.c_str(), roi->xs, roi->xe, roi->ys, roi->ye, roi->zs, roi->ze);
    #endif

    //the ROI selection is catched only if a <CExplorerWindow> is opened
    if(current)
    {
        //retrieving ROI infos propagated by Vaa3D
        int roiCenterX = roi->xe-(roi->xe-roi->xs)/2;
        int roiCenterY = roi->ye-(roi->ye-roi->ys)/2;
        int roiCenterZ = roi->ze-(roi->ze-roi->zs)/2;

        //zoom-in around marker or ROI triggers a new window
        if(roi->ops_type == 1)
            current->newView(roiCenterX, roiCenterY, roiCenterZ, volResIndex+1, false, static_cast<int>((roi->xe-roi->xs)/2.0f+0.5f), static_cast<int>((roi->ye-roi->ys)/2.0f+0.5f), static_cast<int>((roi->ze-roi->zs)/2.0f+0.5f));

        //zoom-in with mouse scroll up may trigger a new window if caching is not possible
        else if(roi->ops_type == 2)
        {
            if(current->volResIndex != CImport::instance()->getResolutions()-1 &&   //do nothing if highest resolution has been reached
               isZoomDerivativePos())                                               //accepting zoom-in only when zoom factor derivative is positive
            {
                //trying caching if next view exists
                if(current->next)
                {
                    //converting Vaa3D VOI local coordinates to TeraFly coordinates of the next resolution
                    float gXS = current->getGlobalHCoord(static_cast<float>(roi->xs), current->next->volResIndex);
                    float gXE = current->getGlobalHCoord(static_cast<float>(roi->xe), current->next->volResIndex);
                    float gYS = current->getGlobalVCoord(static_cast<float>(roi->ys), current->next->volResIndex);
                    float gYE = current->getGlobalVCoord(static_cast<float>(roi->ye), current->next->volResIndex);
                    float gZS = current->getGlobalDCoord(static_cast<float>(roi->zs), current->next->volResIndex);
                    float gZE = current->getGlobalDCoord(static_cast<float>(roi->ze), current->next->volResIndex);
                    QRectF gXRect(QPointF(gXS, 0), QPointF(gXE, 1));
                    QRectF gYRect(QPointF(gYS, 0), QPointF(gYE, 1));
                    QRectF gZRect(QPointF(gZS, 0), QPointF(gZE, 1));

                    //obtaining the actual requested VOI under the existing constraints (i.e., maximum view dimensions)
                    printf("--------------------- teramanager plugin [thread ?] >> CExplorerWindow::Vaa3D_selectedROI(): requested voi [%.0f-%.0f][%.0f-%.0f][%.0f-%.0f]...",
                           gXS, gXE, gYS, gYE, gZS, gZE);
                    if(gXE-gXS > PMain::getInstance()->Hdim_sbox->value())
                    {
                        float center = gXS+(gXE-gXS)/2;
                        gXS = center - PMain::getInstance()->Hdim_sbox->value()/2;
                        gXE = center + PMain::getInstance()->Hdim_sbox->value()/2;
                    }
                    if(gYE-gYS > PMain::getInstance()->Vdim_sbox->value())
                    {
                        float center = gYS+(gYE-gYS)/2;
                        gYS = center - PMain::getInstance()->Vdim_sbox->value()/2;
                        gYE = center + PMain::getInstance()->Vdim_sbox->value()/2;
                    }
                    if(gZE-gZS > PMain::getInstance()->Ddim_sbox->value())
                    {
                        float center = gZS+(gZE-gZS)/2;
                        gZS = center - PMain::getInstance()->Ddim_sbox->value()/2;
                        gZE = center + PMain::getInstance()->Ddim_sbox->value()/2;
                    }
                    printf("but actual requested VOI is [%.0f-%.0f][%.0f-%.0f][%.0f-%.0f]\n", gXS, gXE, gYS, gYE, gZS, gZE);

                    //obtaining coordinates of the cached resolution
                    float gXScached = static_cast<float>(current->next->volH0);
                    float gXEcached = static_cast<float>(current->next->volH1);
                    float gYScached = static_cast<float>(current->next->volV0);
                    float gYEcached = static_cast<float>(current->next->volV1);
                    float gZScached = static_cast<float>(current->next->volD0);
                    float gZEcached = static_cast<float>(current->next->volD1);
                    QRectF gXRectCached(QPoint(gXScached, 0), QPoint(gXEcached, 1));
                    QRectF gYRectCached(QPoint(gYScached, 0), QPoint(gYEcached, 1));
                    QRectF gZRectCached(QPoint(gZScached, 0), QPoint(gZEcached, 1));

                    //computing intersection volume and Vaa3D VOI coverage factor
                    float intersectionX = gXRect.intersected(gXRectCached).width();
                    float intersectionY = gYRect.intersected(gYRectCached).width();
                    float intersectionZ = gZRect.intersected(gZRectCached).width();
                    float intersectionVol =  intersectionX*intersectionY*intersectionZ;
                    float voiVol = (gXE-gXS)*(gYE-gYS)*(gZE-gZS);
                    float cachedVol = (gXEcached-gXScached)*(gYEcached-gYScached)*(gZEcached-gZScached);
                    float coverageFactor = voiVol != 0 ? intersectionVol/voiVol : 0;
                    printf("--------------------- teramanager plugin [thread ?] >> CExplorerWindow::Vaa3D_selectedROI(): actual requested voi[%.0f-%.0f][%.0f-%.0f][%.0f-%.0f] and cached[%.0f-%.0f][%.0f-%.0f][%.0f-%.0f]...\n",
                           gXS, gXE, gYS, gYE, gZS, gZE, gXScached, gXEcached, gYScached, gYEcached, gZScached, gZEcached);
                    printf("--------------------- teramanager plugin [thread ?] >> CExplorerWindow::Vaa3D_selectedROI(): intersection is %.0f x %.0f x %.0f with coverage factor = %.2f\n",
                           intersectionX, intersectionY, intersectionZ, coverageFactor);

                    //if Vaa3D VOI is covered for the selected percentage by the existing cached volume, just restoring its view
                    if(coverageFactor >= PMain::getInstance()->cacheSens->value()/100.0f)
                    {
                        current->setActive(false);
                        current->resetZoomHistory();
                        current->next->restoreViewFrom(current);
                    }

                    //otherwise invoking a new view
                    else
                        current->newView(roiCenterX, roiCenterY, roiCenterZ, volResIndex+1, false, static_cast<int>((roi->xe-roi->xs)/2.0f+0.5f), static_cast<int>((roi->ye-roi->ys)/2.0f+0.5f), static_cast<int>((roi->ze-roi->zs)/2.0f+0.5f));
                }
                else
                    current->newView(roiCenterX, roiCenterY, roiCenterZ, volResIndex+1, false, static_cast<int>((roi->xe-roi->xs)/2.0f+0.5f), static_cast<int>((roi->ye-roi->ys)/2.0f+0.5f), static_cast<int>((roi->ze-roi->zs)/2.0f+0.5f));
            }
            else
                printf("--------------------- teramanager plugin [thread ?] >> CExplorerWindow::Vaa3D_selectedROI(): ignoring Vaa3D mouse scroll up zoom-in\n");

        }
        else
            QMessageBox::critical(PMain::getInstance(),QObject::tr("Error"), QObject::tr("in CExplorerWindow::Vaa3D_selectedROI(): unsupported (or unset) operation type"),QObject::tr("Ok"));
    }
}

/**********************************************************************************
* Returns  the  global coordinate  (which starts from 0) in  the given  resolution
* volume image space given the local coordinate (which starts from 0) in the current
* resolution volume image space. If resIndex is not set, the returned global coord-
* inate will be in the highest resolution image space.
***********************************************************************************/
int CExplorerWindow::getGlobalVCoord(int localVCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = false */)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::getGlobalVCoord(coord = %d, res = %d, fromVaa3D = %s)...",
           titleShort.c_str(), localVCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false");
    #endif

    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volV1-volV0 > LIMIT_VOLY))
        localVCoord = static_cast<int>(localVCoord* ( static_cast<float>(volV1-volV0-1)/(LIMIT_VOLY-1) ) +0.5f);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);

    #ifdef TMP_DEBUG
    printf("%d\n", static_cast<int>((volV0+localVCoord)*ratio + 0.5f));
    #endif

    return (volV0+localVCoord)*ratio + 0.5f;
}
int CExplorerWindow::getGlobalHCoord(int localHCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = false */)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::getGlobalHCoord(coord = %d, res = %d, fromVaa3D = %s)...",
           titleShort.c_str(), localHCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false");
    #endif

    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volH1-volH0 > LIMIT_VOLX))
        localHCoord = static_cast<int>(localHCoord* ( static_cast<float>(volH1-volH0-1)/(LIMIT_VOLX-1) ) +0.5f);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);

    #ifdef TMP_DEBUG
    printf("%d\n", static_cast<int>((volH0+localHCoord)*ratio + 0.5f));
    #endif

    return (volH0+localHCoord)*ratio + 0.5f;
}
int CExplorerWindow::getGlobalDCoord(int localDCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = false */)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::getGlobalDCoord(coord = %d, res = %d, fromVaa3D = %s)...",
           titleShort.c_str(), localDCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false");
    #endif

    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volD1-volD0 > LIMIT_VOLZ))
        localDCoord = static_cast<int>(localDCoord* ( static_cast<float>(volD1-volD0-1)/(LIMIT_VOLZ-1) ) +0.5f);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);

    #ifdef TMP_DEBUG
    printf("%d\n", static_cast<int>((volD0+localDCoord)*ratio + 0.5f));
    #endif

    return (volD0+localDCoord)*ratio + 0.5f;
}
float CExplorerWindow::getGlobalVCoord(float localVCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = false */)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::getGlobalVCoord(coord = %.1f, res = %d, fromVaa3D = %s)...",
           titleShort.c_str(), localVCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false");
    #endif

    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volV1-volV0 > LIMIT_VOLY))
        localVCoord *= static_cast<float>(volV1-volV0-1)/(LIMIT_VOLY-1);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);

#ifdef TMP_DEBUG
printf("%.1f\n", (volV0+localVCoord)*ratio);
#endif

    return (volV0+localVCoord)*ratio;
}
float CExplorerWindow::getGlobalHCoord(float localHCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = false */)
{

    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::getGlobalHCoord(coord = %.1f, res = %d, fromVaa3D = %s)...",
           titleShort.c_str(), localHCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false");
    #endif

    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volH1-volH0 > LIMIT_VOLX))
        localHCoord *= static_cast<float>(volH1-volH0-1)/(LIMIT_VOLX-1);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);

    #ifdef TMP_DEBUG
    printf("%.1f\n", (volH0+localHCoord)*ratio);
    #endif

    return (volH0+localHCoord)*ratio;
}
float CExplorerWindow::getGlobalDCoord(float localDCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = false */)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::getGlobalDCoord(coord = %.1f, res = %d, fromVaa3D = %s)...",
           titleShort.c_str(), localDCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false");
    #endif

    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volD1-volD0 > LIMIT_VOLZ))
        localDCoord *= static_cast<float>(volD1-volD0-1)/(LIMIT_VOLZ-1);

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);

    #ifdef TMP_DEBUG
    printf("%.1f\n", (volD0+localDCoord)*ratio);
    #endif

    return (volD0+localDCoord)*ratio;
}

/**********************************************************************************
* Returns the local coordinate (which starts from 0) in the current resolution vol-
* ume image space given the global coordinate  (which starts from 0) in the highest
* resolution volume image space.
***********************************************************************************/
int CExplorerWindow::getLocalVCoord(int highestResGlobalVCoord, bool toVaa3Dcoordinates /* = false */)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);
    int localCoord =  static_cast<int>(highestResGlobalVCoord/ratio - volV0 + 0.5f);

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(toVaa3Dcoordinates && (volV1-volV0 > LIMIT_VOLY))
        localCoord = static_cast<int>(localCoord* ( static_cast<float>(LIMIT_VOLY-1)/(volV1-volV0-1) ) +0.5f);
    return localCoord;
}
int CExplorerWindow::getLocalHCoord(int highestResGlobalHCoord, bool toVaa3Dcoordinates /* = false */)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);
    int localCoord =  static_cast<int>(highestResGlobalHCoord/ratio - volH0 + 0.5f);

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(toVaa3Dcoordinates && (volH1-volH0 > LIMIT_VOLX))
        localCoord = static_cast<int>(localCoord* ( static_cast<float>(LIMIT_VOLX-1)/(volH1-volH0-1) ) +0.5f);
    return localCoord;
}
int CExplorerWindow::getLocalDCoord(int highestResGlobalDCoord, bool toVaa3Dcoordinates /* = false */)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);
    int localCoord =  static_cast<int>(highestResGlobalDCoord/ratio - volD0 + 0.5f);

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(toVaa3Dcoordinates && (volD1-volD0 > LIMIT_VOLZ))
        localCoord = static_cast<int>(localCoord* ( static_cast<float>(LIMIT_VOLZ-1)/(volD1-volD0-1) ) +0.5f);
    return localCoord;
}
float CExplorerWindow::getLocalVCoord(float highestResGlobalVCoord, bool toVaa3Dcoordinates /* = false */)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);
    float localCoord = highestResGlobalVCoord/ratio - volV0;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(toVaa3Dcoordinates && (volV1-volV0 > LIMIT_VOLY))
        localCoord = localCoord* ( static_cast<float>(LIMIT_VOLY-1)/(volV1-volV0-1) );
    return localCoord;
}
float CExplorerWindow::getLocalHCoord(float highestResGlobalHCoord, bool toVaa3Dcoordinates /* = false */)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);
    float localCoord = highestResGlobalHCoord/ratio - volH0;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(toVaa3Dcoordinates && (volH1-volH0 > LIMIT_VOLX))
        localCoord = localCoord* ( static_cast<float>(LIMIT_VOLX-1)/(volH1-volH0-1) );
    return localCoord;
}
float CExplorerWindow::getLocalDCoord(float highestResGlobalDCoord, bool toVaa3Dcoordinates /* = false */)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);
    float localCoord = highestResGlobalDCoord/ratio - volD0;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(toVaa3Dcoordinates && (volD1-volD0 > LIMIT_VOLZ))
        localCoord = localCoord* ( static_cast<float>(LIMIT_VOLZ-1)/(volD1-volD0-1) );
    return localCoord;
}

/**********************************************************************************
* Linked to volume cut scrollbars of Vaa3D widget containing the 3D renderer.
* This implements the syncronization Vaa3D-->TeraManager of subvolume selection.
***********************************************************************************/
void CExplorerWindow::Vaa3D_changeYCut0(int s)
{
    disconnect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
    PMain::getInstance()->V0_sbox->setValue(getGlobalVCoord(s, -1, true)+1);
    connect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
}
void CExplorerWindow::Vaa3D_changeYCut1(int s)
{
    disconnect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
    PMain::getInstance()->V1_sbox->setValue(getGlobalVCoord(s, -1, true)+1);
    connect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
}
void CExplorerWindow::Vaa3D_changeXCut0(int s)
{
    disconnect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
    PMain::getInstance()->H0_sbox->setValue(getGlobalHCoord(s, -1, true)+1);
    connect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
}
void CExplorerWindow::Vaa3D_changeXCut1(int s)
{
    disconnect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
    PMain::getInstance()->H1_sbox->setValue(getGlobalHCoord(s, -1, true)+1);
    connect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
}
void CExplorerWindow::Vaa3D_changeZCut0(int s)
{
    disconnect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
    PMain::getInstance()->D0_sbox->setValue(getGlobalDCoord(s, -1, true)+1);
    connect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
}
void CExplorerWindow::Vaa3D_changeZCut1(int s)
{
    disconnect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));
    PMain::getInstance()->D1_sbox->setValue(getGlobalDCoord(s, -1, true)+1);
    connect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));
}

/**********************************************************************************
* Linked to PMain GUI VOI's widgets.
* This implements the syncronization TeraManager-->Vaa3D of subvolume selection.
***********************************************************************************/
void CExplorerWindow::PMain_changeV0sbox(int s)
{
    disconnect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
    view3DWidget->setYCut0(getLocalVCoord(s-1, true)+1);
    connect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
}
void CExplorerWindow::PMain_changeV1sbox(int s)
{
    disconnect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
    view3DWidget->setYCut1(getLocalVCoord(s-1, true)+1);
    connect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
}
void CExplorerWindow::PMain_changeH0sbox(int s)
{
    disconnect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
    view3DWidget->setXCut0(getLocalHCoord(s-1, true)+1);
    connect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
}
void CExplorerWindow::PMain_changeH1sbox(int s)
{
    disconnect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
    view3DWidget->setXCut1(getLocalHCoord(s-1, true)+1);
    connect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
}
void CExplorerWindow::PMain_changeD0sbox(int s)
{
    disconnect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
    view3DWidget->setZCut0(getLocalDCoord(s-1, true)+1);
    connect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
}
void CExplorerWindow::PMain_changeD1sbox(int s)
{
    disconnect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
    view3DWidget->setZCut1(getLocalDCoord(s-1, true)+1);
    connect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
}

/**********************************************************************************
* Alignes the given widget to the left(right) of the current window
***********************************************************************************/
void CExplorerWindow::alignToLeft(QWidget* widget)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::alignToLeft()\n",  titleShort.c_str() );
    #endif

    widget->move(window3D->x() + window3D->width() + 3, window3D->y());
    widget->setMaximumHeight(std::max(window3D->height(),widget->height()));
    widget->resize(widget->width(), window3D->height());
}
void CExplorerWindow::alignToRight(QWidget* widget)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CExplorerWindow[%s]::alignToRight()\n",  titleShort.c_str() );
    #endif

    widget->move(window3D->x() - widget->width() - 3, window3D->y());
    widget->setMaximumHeight(std::max(window3D->height(),widget->height()));
    widget->resize(widget->width(), window3D->height());
}

/**********************************************************************************
* Linked to Vaa3D renderer slider
***********************************************************************************/
//void CExplorerWindow::setZoom(int z)
//{
//    myV3dR_GLWidget::setZoomO(view3DWidget, z);
//}

/**********************************************************************************
* Syncronizes widgets from <src> to <dst>
***********************************************************************************/
void CExplorerWindow::syncWindows(V3dR_MainWindow* src, V3dR_MainWindow* dst)
{
    //syncronizing volume display controls
    dst->checkBox_channelR->setChecked(src->checkBox_channelR->isChecked());
    dst->checkBox_channelG->setChecked(src->checkBox_channelG->isChecked());
    dst->checkBox_channelB->setChecked(src->checkBox_channelB->isChecked());
    dst->checkBox_volCompress->setChecked(src->checkBox_volCompress->isChecked());
    dst->dispType_maxip->setChecked(src->dispType_maxip->isChecked());
    dst->dispType_minip->setChecked(src->dispType_minip->isChecked());
    dst->dispType_alpha->setChecked(src->dispType_alpha->isChecked());
    dst->dispType_cs3d->setChecked(src->dispType_cs3d->isChecked());
    dst->zthicknessBox->setValue(src->zthicknessBox->value());
    dst->comboBox_channel->setCurrentIndex(src->comboBox_channel->currentIndex());
    dst->transparentSlider->setValue(src->transparentSlider->value());

    //syncronizing other controls
    dst->checkBox_displayAxes->setChecked(src->checkBox_displayAxes->isChecked());
    dst->checkBox_displayBoundingBox->setChecked(src->checkBox_displayBoundingBox->isChecked());
    dst->checkBox_OrthoView->setChecked(src->checkBox_OrthoView->isChecked());


//    // volume display control
//    QRadioButton *dispType_maxip, *dispType_minip, *dispType_alpha, *dispType_cs3d;
//    QLabel *thicknessSlider_Label, *transparentSlider_Label; //for disable, by RZC 080822
//    QDoubleSpinBox *zthicknessBox; //by PHC, 090215
//    QComboBox *comboBox_channel;
//    QAbstractSlider *thicknessSlider, *transparentSlider;
//    QCheckBox *checkBox_channelR, *checkBox_channelG, *checkBox_channelB, *checkBox_channelA, *checkBox_volCompress;
//    QPushButton *volumeColormapButton;

//    // surface display control
//    QCheckBox *checkBox_displayMarkers, *checkBox_displaySurf, *checkBox_markerLabel, *checkBox_surfStretch;
//    QSpinBox * spinBox_markerSize; // 090422 RZC
//    QPushButton *updateLandmarkButton, *loadSaveObjectsButton;
//    QPushButton *surfobjManagerButton; //or Object Manager button

//    // other control
//    QCheckBox *checkBox_displayAxes, *checkBox_displayBoundingBox, *checkBox_OrthoView;
//    QPushButton *colorButton, *brightButton, *reloadDataButton;
//    QPushButton *movieSaveButton, *animateButton;
//    QComboBox * comboRotType;

//    QTabWidget *tabOptions;

//    // rotation, zoom, shift control
//    QAbstractSlider *xRotSlider, *yRotSlider, *zRotSlider;
//    QAbstractSlider *zoomSlider, *xShiftSlider, *yShiftSlider;
//    QSpinBox *xRotBox, *yRotBox, *zRotBox, *zoomBox, *xShiftBox, *yShiftBox;
//    QCheckBox *checkBox_absoluteRot;
//    QPushButton *zoomReset, *rotReset, *rotAbsolute, *rotAbsolutePose;

//    QTabWidget *tabRotZoom;

//    // cut plane control
//    QAbstractSlider *xcminSlider, *xcmaxSlider, *ycminSlider, *ycmaxSlider, *zcminSlider, *zcmaxSlider, *fcutSlider;
//    QAbstractButton *xcLock, *ycLock, *zcLock;
//    QCheckBox *checkBox_xCS, *checkBox_yCS, *checkBox_zCS, *checkBox_fCS;
//    QAbstractSlider *xCSSlider, *yCSSlider, *zCSSlider, *fCSSlider;
//    QAbstractSlider *xSminSlider, *xSmaxSlider, *ySminSlider, *ySmaxSlider, *zSminSlider, *zSmaxSlider;
//    QStackedLayout *stackedCutPlane;
}

