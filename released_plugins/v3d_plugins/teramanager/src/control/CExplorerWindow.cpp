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
#include "../presentation/PLog.h"
#include "renderer.h"
#include "renderer_gl1.h"
#include "v3dr_colormapDialog.h"
#include "V3Dsubclasses.h"

using namespace teramanager;

CExplorerWindow* CExplorerWindow::first = 0;
CExplorerWindow* CExplorerWindow::last = 0;
CExplorerWindow* CExplorerWindow::current = 0;
int CExplorerWindow::nInstances = 0;

void CExplorerWindow::show()
{
    /**/ updateGraphicsInProgress.lock();

    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    PMain* pMain = PMain::getInstance();

    try
    {
        //opening tri-view window (and hiding it asap)
        QElapsedTimer timer;
        timer.start();
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
        if(!view3DWidget->getiDrawExternalParameter())
            QMessageBox::critical(PMain::getInstance(),QObject::tr("Error"), QObject::tr("Unable to get iDrawExternalParameter from Vaa3D's V3dR_GLWidget"),QObject::tr("Ok"));
        window3D = view3DWidget->getiDrawExternalParameter()->window3D;
        PLog::getInstance()->appendGPU(timer.elapsed(), QString("Opened view ").append(title.c_str()).toStdString());


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

            timer.restart();
            if(changed_cmap)
                curr_renderer->applyColormapToImage();

            //positioning the current 3D window exactly at the previous window position
            QPoint location = prev->pos();
            resize(prev->window3D->size());
            move(location);

            //hiding both tri-view and 3D view
            prev->window3D->setVisible(false);
            //prev->triViewWidget->setVisible(false);       //---- Alessandro 2013-09-04 fixed: this causes Vaa3D's setCurHiddenSelectedWindow() to fail
            prev->view3DWidget->setCursor(Qt::ArrowCursor);

            //registrating views: ---- Alessandro 2013-04-18 fixed: determining unique triple of rotation angles and assigning absolute rotation
            float ratio = CImport::instance()->getVolume(volResIndex)->getDIM_D()/CImport::instance()->getVolume(prev->volResIndex)->getDIM_D();
            view3DWidget->setZoom(prev->view3DWidget->zoom()/ratio);
            prev->view3DWidget->absoluteRotPose();
            view3DWidget->doAbsoluteRot(prev->view3DWidget->xRot(), prev->view3DWidget->yRot(), prev->view3DWidget->zRot());

            //sync widgets
            syncWindows(prev->window3D, window3D);
            PLog::getInstance()->appendGPU(timer.elapsed(), QString("Syncronized views \"").append(title.c_str()).append("\" and \"").append(prev->title.c_str()).append("\"").toStdString());

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
        pMain->V0_sbox->setMinimum(getGlobalVCoord(view3DWidget->yCut0(), -1, true, false, __itm__current__function__)+1);
        pMain->V0_sbox->setValue(pMain->V0_sbox->minimum());
        pMain->V1_sbox->setMaximum(getGlobalVCoord(view3DWidget->yCut1(), -1, true, false, __itm__current__function__)+1);
        pMain->V1_sbox->setValue(pMain->V1_sbox->maximum());
        pMain->H0_sbox->setMinimum(getGlobalHCoord(view3DWidget->xCut0(), -1, true, false, __itm__current__function__)+1);
        pMain->H0_sbox->setValue(pMain->H0_sbox->minimum());
        pMain->H1_sbox->setMaximum(getGlobalHCoord(view3DWidget->xCut1(), -1, true, false, __itm__current__function__)+1);
        pMain->H1_sbox->setValue(pMain->H1_sbox->maximum());
        pMain->D0_sbox->setMinimum(getGlobalDCoord(view3DWidget->zCut0(), -1, true, false, __itm__current__function__)+1);
        pMain->D0_sbox->setValue(pMain->D0_sbox->minimum());
        pMain->D1_sbox->setMaximum(getGlobalDCoord(view3DWidget->zCut1(), -1, true, false, __itm__current__function__)+1);
        pMain->D1_sbox->setValue(pMain->D1_sbox->maximum());

        //signal connections
        connect(CVolume::instance(), SIGNAL(sendOperationOutcome(uint8*, MyException*,void*,qint64, QString, int)), this, SLOT(loadingDone(uint8*, MyException*,void*,qint64, QString, int)), Qt::BlockingQueuedConnection);
        connect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
        connect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
        connect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
        connect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
        connect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
        connect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
        connect(view3DWidget, SIGNAL(xRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        connect(view3DWidget, SIGNAL(yRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        connect(view3DWidget, SIGNAL(zRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        connect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
        connect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
        connect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
        connect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
        connect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
        connect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));

        #ifdef USE_EXPERIMENTAL_FEATURES
        disconnect(window3D->zoomSlider, SIGNAL(valueChanged(int)), view3DWidget, SLOT(setZoom(int)));
        connect(window3D->zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setZoom(int)));
        #endif //USE_EXPERIMENTAL_FEATURES

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

    /**/ updateGraphicsInProgress.unlock();
}

CExplorerWindow::CExplorerWindow(V3DPluginCallback2 *_V3D_env, int _resIndex, uint8 *_imgData, int _volV0, int _volV1,
                                 int _volH0, int _volH1, int _volD0, int _volD1, int _nchannels, CExplorerWindow *_prev): QWidget()
{
    /**/itm::debug(itm::LEV1, strprintf("_resIndex = %d, _V0 = %d, _V1 = %d, _H0 = %d, _H1 = %d, _D0 = %d, _D1 = %d, _nchannels = %d",
                                        _resIndex, _volV0, _volV1, _volH0, _volH1, _volD0, _volD1, _nchannels).c_str(), __itm__current__function__);

    //initializations
    resetZoomHistory();
    isActive = isReady = false;
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
    this->imgData = _imgData;
    this->isReady = false;
    char ctitle[1024];
    sprintf(ctitle, "Res(%d x %d x %d),Volume X=[%d,%d], Y=[%d,%d], Z=[%d,%d], %d channels", CImport::instance()->getVolume(volResIndex)->getDIM_H(),
            CImport::instance()->getVolume(volResIndex)->getDIM_V(), CImport::instance()->getVolume(volResIndex)->getDIM_D(),
            volH0+1, volH1, volV0+1, volV1, volD0+1, volD1, nchannels);
    this->title = ctitle;
    sprintf(ctitle, "Res{%d}), Vol{[%d,%d) [%d,%d) [%d,%d)}", volResIndex, volH0, volH1, volV0, volV1, volD0, volD1);
    this->titleShort = ctitle;
    V0_sbox_min = V1_sbox_max = H0_sbox_min = H1_sbox_max = D0_sbox_min = D1_sbox_max = V0_sbox_val = V1_sbox_val = H0_sbox_val = H1_sbox_val = D0_sbox_val = D1_sbox_val = -1;

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
        /**/itm::debug(itm::LEV_MAX, strprintf("nInstances++, nInstances = %d", nInstances).c_str(), __itm__current__function__);
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

    /**/itm::debug(itm::LEV1, "Object successfully constructed", __itm__current__function__);
}

CExplorerWindow::~CExplorerWindow()
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

    //removing the event filter from the 3D renderer and from the 3D window
    isActive = false;
    view3DWidget->removeEventFilter(this);
    window3D->removeEventFilter(this);

    //just closing windows
    V3D_env->close3DWindow(window); //--- Alessandro 24/08/2013: this causes crash at the deepest resolution when the plugin is invoked by Vaa3D
    triViewWidget->close();

    //decreasing the number of instantiated objects
    nInstances--;

    /**/itm::debug(itm::LEV1, strprintf("title = %s, nInstances--, nInstances = %d", titleShort.c_str(), nInstances).c_str(), __itm__current__function__);

    /**/itm::debug(itm::LEV1, strprintf("title = %s, object successfully DESTROYED", titleShort.c_str()).c_str(), __itm__current__function__);
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
                return true;
            }
        }

        #ifdef USE_EXPERIMENTAL_FEATURES
        /******************** REDIRECTING MOUSE-WHEEL EVENTS *************************
        Mouse-wheel events are redirected to the customized wheelEvent handler
        ***************************************************************************/
        if ((object == view3DWidget || object == window3D) && event->type() == QEvent::Wheel)
        {
            QWheelEvent* wheelEvt = (QWheelEvent*)event;
            myV3dR_GLWidget::cast(view3DWidget)->wheelEventO(wheelEvt);
            return true;
        }
        #endif

        /****************** INTERCEPTING DOUBLE CLICK EVENTS ***********************
        Double click events are intercepted to switch to the higher resolution.
        ***************************************************************************/
        if (object == view3DWidget && event->type() == QEvent::MouseButtonDblClick)
        {
            QMouseEvent* mouseEvt = (QMouseEvent*)event;
            XYZ point = getRenderer3DPoint(mouseEvt->x(), mouseEvt->y());
            newView(point.x, point.y, point.z, volResIndex+1);
            return true;
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
           return true;
        }

        /***************** INTERCEPTING STATE CHANGES EVENTS **********************
        Window state changes events are intercepted to let PMain's position be syn-
        cronized with the explorer.
        ***************************************************************************/
        else if(object == window3D && event->type() == QEvent::WindowStateChange)
        {
            alignToLeft(PMain::getInstance());
            return true;
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
void CExplorerWindow::loadingDone(uint8 *data, MyException *ex, void* sourceObject, qint64 elapsed_time, QString op_dsc, int step)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s, ex = ", titleShort.c_str(),  (ex? "error" : "0")).c_str(), __itm__current__function__);

    char message[1000];
    CVolume* cVolume = CVolume::instance();

    //if an exception has occurred, showing a message error
    if(ex)
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
    else if(sourceObject == this)
    {
        try
        {
            //first updating IO time
            PLog::getInstance()->appendIO(elapsed_time, op_dsc.toStdString());

            //copying loaded data
            /**/ CVolume::instance()->bufferMutex.lock();
            QElapsedTimer timer;
            timer.start();
            uint32 img_dims[4]       = {volH1-volH0, volV1-volV0, volD1-volD0, nchannels};
            uint32 img_offset[3]     = {cVolume->getVoiH0()-volH0, cVolume->getVoiV0()-volV0, cVolume->getVoiD0()-volD0};
            uint32 new_img_dims[4]   = {cVolume->getVoiH1()-cVolume->getVoiH0(),  cVolume->getVoiV1()-cVolume->getVoiV0(), cVolume->getVoiD1()-cVolume->getVoiD0(),  nchannels};
            uint32 new_img_offset[3] = {0, 0, 0};
            uint32 new_img_count[3]  = {new_img_dims[0], new_img_dims[1], new_img_dims[2]};
            copyVOI(data, new_img_dims, new_img_offset, new_img_count,
                    view3DWidget->getiDrawExternalParameter()->image4d->getRawData(), img_dims, img_offset);
            qint64 elapsedTime = timer.elapsed();
            /**/ CVolume::instance()->bufferMutex.unlock();

            //updating log
            sprintf(message, "Streaming %d/%d: Copied block X=[%d, %d) Y=[%d, %d) Z=[%d, %d) to resolution %d",
                              step, cVolume->getStreamingSteps(), cVolume->getVoiH0(), cVolume->getVoiH1(), cVolume->getVoiV0(), cVolume->getVoiV1(), cVolume->getVoiD0(), cVolume->getVoiD1(), cVolume->getVoiResIndex());
            PLog::getInstance()->appendCPU(elapsedTime, message);

            //releasing memory if streaming is not active
            if(cVolume->getStreamingSteps() == 1)
                delete[] data;

            //updating image data
            /**/ updateGraphicsInProgress.lock();
            timer.restart();
            view3DWidget->updateImageData();
            sprintf(message, "Streaming %d/%d: Block X=[%d, %d) Y=[%d, %d) Z=[%d, %d) rendered into view %s",
                    step, cVolume->getStreamingSteps(), cVolume->getVoiH0(), cVolume->getVoiH1(),
                    cVolume->getVoiV0(), cVolume->getVoiV1(),
                    cVolume->getVoiD0(), cVolume->getVoiD1(), title.c_str());
            PLog::getInstance()->appendGPU(timer.elapsed(), message);
            /**/ updateGraphicsInProgress.unlock();

            //operations to be performed when all image data have been loaded
            if(cVolume->hasFinished())
            {
                //resetting TeraFly's GUI
                PMain::getInstance()->resetGUI();


                //---- Alessandro 2013-09-28 fixed: processing pending events related trasl* buttons (previously deactivated) prevents the user from
                //                                  triggering multiple traslations at the same time.
                //---- Alessandro 2014-01-26 fixed: processEvents() is no longer needed, since the trasl* button slots have been made safer and do not
                //                                  trigger any action when the the current window is not active (or has to be closed)
                //QApplication::processEvents();
                PMain::getInstance()->traslXneg->setActive(true);
                PMain::getInstance()->traslXpos->setActive(true);
                PMain::getInstance()->traslYneg->setActive(true);
                PMain::getInstance()->traslYpos->setActive(true);
                PMain::getInstance()->traslZneg->setActive(true);
                PMain::getInstance()->traslZpos->setActive(true);

//                PMain::getInstance()->setEnabledDirectionalShifts(true);

                //current window is now ready for user input
                isReady = true;

                //saving elapsed time to log
                sprintf(message, "Successfully generated view %s", title.c_str());
                PLog::getInstance()->appendActual(prev->zoomInTimer.elapsed(), message);
            }
        }
        catch(MyException &ex)
        {
            QMessageBox::critical(PMain::getInstance(),QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
            PMain::getInstance()->resetGUI();
            isReady = true;
        }
    }
}

/**********************************************************************************
* Generates a new view using the given coordinates.
* Called by the current <CExplorerWindow> when the user zooms in and the higher res-
* lution has to be loaded.
***********************************************************************************/
void
CExplorerWindow::newView(
    int x, int y, int z,                            //can be either the VOI's center (default) or the VOI's ending point (see x0,y0,z0)
    int resolution,                                 //resolution index of the view requested
    bool fromVaa3Dcoordinates /*= false*/,          //if coordinates were obtained from Vaa3D
    int dx/*=-1*/, int dy/*=-1*/, int dz/*=-1*/,    //VOI [x-dx,x+dx), [y-dy,y+dy), [z-dz,z+dz)
    int x0/*=-1*/, int y0/*=-1*/, int z0/*=-1*/)    //VOI [x0, x), [y0, y), [z0, z)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s, x = %d, y = %d, z = %d, res = %d, dx = %d, dy = %d, dz = %d, x0 = %d, y0 = %d, z0 = %d",
                                        titleShort.c_str(),  x, y, z, resolution, dx, dy, dz, x0, y0, z0).c_str(), __itm__current__function__);

    //checking preconditions
    if(!isActive || toBeClosed)
    {
        QMessageBox::warning(0, "Unexpected behaviour", "Precondition check \"!isActive || toBeClosed\" failed. Please contact the developers");
        return;
    }

    //deactivating current window and processing all pending events
    setActive(false);
    QApplication::processEvents();

    //after processEvents(), it might be that this windows is no longer valid
    if(toBeClosed)
        return;

    //checking preconditions with automatic correction, where possible
    if(resolution >= CImport::instance()->getResolutions())
        resolution = volResIndex;

    zoomInTimer.restart();

    try
    {
        //putting GUI in waiting state
        view3DWidget->setCursor(Qt::WaitCursor);
        PMain& pMain = *(PMain::getInstance());
        pMain.progressBar->setEnabled(true);
        pMain.progressBar->setMinimum(0);
        pMain.progressBar->setMaximum(0);
        pMain.loadButton->setEnabled(false);
        pMain.statusBar->showMessage("Changing resolution...");

        //scaling VOI to the given resolution
        float ratio = static_cast<float>(CImport::instance()->getVolume(resolution)->getDIM_D())/CImport::instance()->getVolume(volResIndex)->getDIM_D();
        x = getGlobalHCoord(x, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
        y = getGlobalVCoord(y, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
        z = getGlobalDCoord(z, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
        if(x0 != -1)
            x0 = getGlobalHCoord(x0, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
        else
            dx = dx == -1 ? int_inf : static_cast<int>(dx*ratio+0.5f);
        if(y0 != -1)
            y0 = getGlobalVCoord(y0, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
        else
            dy = dy == -1 ? int_inf : static_cast<int>(dy*ratio+0.5f);
        if(z0 != -1)
            z0 = getGlobalDCoord(z0, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
        else
            dz = dz == -1 ? int_inf : static_cast<int>(dz*ratio+0.5f);

        //cropping bounding box if its larger than the maximum allowed
        if(dx != -1 && dy != -1 && dz != -1)
        {
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, cropping bbox dims from (%d,%d,%d) to...", titleShort.c_str(),  dx, dy, dz).c_str(), __itm__current__function__);

            dx = std::min(dx, static_cast<int>(pMain.Hdim_sbox->value()/2.0f+0.5f));
            dy = std::min(dy, static_cast<int>(pMain.Vdim_sbox->value()/2.0f+0.5f));
            dz = std::min(dz, static_cast<int>(pMain.Ddim_sbox->value()/2.0f+0.5f));

            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, ...to (%d,%d,%d)", titleShort.c_str(),  dx, dy, dz).c_str(), __itm__current__function__);
        }
        else
        {
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, cropping bbox dims from [%d,%d) [%d,%d) [%d,%d) to...", titleShort.c_str(),  x0, x, y0, y, z0, z).c_str(), __itm__current__function__);

            if(x - x0 > pMain.Hdim_sbox->value())
            {
                float margin = ( (x - x0) - pMain.Hdim_sbox->value() )/2.0f ;
                x  = static_cast<int>(round(x  - margin));
                x0 = static_cast<int>(round(x0 + margin));
            }
            if(y - y0 > pMain.Vdim_sbox->value())
            {
                float margin = ( (y - y0) - pMain.Vdim_sbox->value() )/2.0f ;
                y  = static_cast<int>(round(y  - margin));
                y0 = static_cast<int>(round(y0 + margin));
            }
            if(z - z0 > pMain.Ddim_sbox->value())
            {
                float margin = ( (z - z0) - pMain.Ddim_sbox->value() )/2.0f ;
                z  = static_cast<int>(round(z  - margin));
                z0 = static_cast<int>(round(z0 + margin));
            }

            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, ...to [%d,%d) [%d,%d) [%d,%d)", titleShort.c_str(),  x0, x, y0, y, z0, z).c_str(), __itm__current__function__);
        }

        //preparing VOI for loading (the actual VOI that can be loaded may differ from the one selected, e.g. along volume's borders)
        CVolume* cVolume = CVolume::instance();
        try
        {
            if(dx != -1 && dy != -1 && dz != -1)
                cVolume->setVoi(0, resolution, y-dy, y+dy, x-dx, x+dx, z-dz, z+dz);
            else
                cVolume->setVoi(0, resolution, y0, y, x0, x, z0, z);
        }
        catch(MyException &ex)
        {
            /**/itm::warning(strprintf("Exception thrown when setting VOI: \"%s\". Aborting newView", ex.what()).c_str(), __itm__current__function__);

            setActive(true);
            view3DWidget->setCursor(Qt::ArrowCursor);
            PMain::getInstance()->resetGUI();
            return;
        }

        //saving min, max and values of PMain GUI VOI's widgets
        saveSubvolSpinboxState();

        //disconnecting current window from GUI's listeners and event filters
        disconnect(CVolume::instance(), SIGNAL(sendOperationOutcome(uint8*, MyException*,void*,qint64, QString, int)), this, SLOT(loadingDone(uint8*, MyException*,void*,qint64, QString, int)));
        disconnect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
        disconnect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
        disconnect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
        disconnect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
        disconnect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
        disconnect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
        disconnect(view3DWidget, SIGNAL(xRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        disconnect(view3DWidget, SIGNAL(yRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        disconnect(view3DWidget, SIGNAL(zRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        disconnect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
        disconnect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
        disconnect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
        disconnect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
        disconnect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
        disconnect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));
        view3DWidget->removeEventFilter(this);
        window3D->removeEventFilter(this);


        //obtaining low res data from current window to be displayed in a new window while the user waits for the new high res data
        QElapsedTimer timer;
        timer.start();
        int rVoiH0 = CVolume::scaleHCoord(cVolume->getVoiH0(), resolution, volResIndex);
        int rVoiH1 = CVolume::scaleHCoord(cVolume->getVoiH1(), resolution, volResIndex);
        int rVoiV0 = CVolume::scaleVCoord(cVolume->getVoiV0(), resolution, volResIndex);
        int rVoiV1 = CVolume::scaleVCoord(cVolume->getVoiV1(), resolution, volResIndex);
        int rVoiD0 = CVolume::scaleDCoord(cVolume->getVoiD0(), resolution, volResIndex);
        int rVoiD1 = CVolume::scaleDCoord(cVolume->getVoiD1(), resolution, volResIndex);
        uint8* lowresData = getVOI(rVoiH0, rVoiH1, rVoiV0, rVoiV1, rVoiD0, rVoiD1,
                                   cVolume->getVoiH1()-cVolume->getVoiH0(),
                                   cVolume->getVoiV1()-cVolume->getVoiV0(),
                                   cVolume->getVoiD1()-cVolume->getVoiD0());
        char message[1000];
        sprintf(message, "Block X=[%d, %d) Y=[%d, %d) Z=[%d, %d) loaded from view %s",
                rVoiH0, rVoiH1, rVoiV0, rVoiV1, rVoiD0, rVoiD1, title.c_str());
        PLog::getInstance()->appendCPU(timer.elapsed(), message);

        //creating a new window
        this->next = new CExplorerWindow(V3D_env, resolution, lowresData,
                                         cVolume->getVoiV0(), cVolume->getVoiV1(), cVolume->getVoiH0(), cVolume->getVoiH1(), cVolume->getVoiD0(), cVolume->getVoiD1(),
                                         nchannels, this);

        //loading new data in a separate thread. When done, the "loadingDone" method of the new window will be called
        pMain.statusBar->showMessage("Loading image data...");
        cVolume->setSource(this->next);
        cVolume->setStreamingSteps(PMain::getInstance()->debugStreamingStepsSBox->value());
        if(PMain::getInstance()->debugStreamingStepsSBox->value() > 1)
            cVolume->initBuffer(lowresData, (cVolume->getVoiH1()-cVolume->getVoiH0())*(cVolume->getVoiV1()-cVolume->getVoiV0())*(cVolume->getVoiD1()-cVolume->getVoiD0())*nchannels);
        cVolume->start();

        //meanwhile, showing the new window
        next->show();

        //if the resolution of the loaded voi is the same of the current one, this window will be closed
        if(resolution == volResIndex)
        {
            /**/itm::debug(itm::LEV_MAX, strprintf("object \"%s\" is going to be destroyed", titleShort.c_str()).c_str(), __itm__current__function__);

            prev->zoomInTimer = zoomInTimer;
            prev->next = next;
            next->prev = prev;
            this->toBeClosed = true;
            delete this;
        }
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::getInstance()->resetGUI();
    }
}

/**********************************************************************************
* Resizes  the  given image subvolume in a  newly allocated array using the fastest
* achievable scaling method. The image currently shown is used as data source.
***********************************************************************************/
uint8* CExplorerWindow::getVOI(int x0, int x1, int y0, int y1, int z0, int z1,
                               int xDimInterp, int yDimInterp, int zDimInterp) throw (MyException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s, x0 = %d, x1 = %d, y0 = %d, y1 = %d, z0 = %d, z1 = %d, xDim = %d, yDim = %d, zDim = %d",
                                        titleShort.c_str(), x0, x1, y0, y1, z0, z1, xDimInterp, yDimInterp, zDimInterp).c_str(), __itm__current__function__);

    //alloating image data and initializing to zero (black)
    size_t img_dim = xDimInterp * yDimInterp * zDimInterp * nchannels;
    uint8* img = new uint8[img_dim];
    for(uint8* img_p = img; img_p-img < img_dim; img_p++)
        *img_p=0;

    //computing actual VOI that can be copied, i.e. that intersects with the image currently displayed
    QRect XRectDisplayed(QPoint(volH0, 0), QPoint(volH1, 1));
    QRect YRectDisplayed(QPoint(volV0, 0), QPoint(volV1, 1));
    QRect ZRectDisplayed(QPoint(volD0, 0), QPoint(volD1, 1));
    QRect XRectVOI(QPoint(x0, 0), QPoint(x1, 1));
    QRect YRectVOI(QPoint(y0, 0), QPoint(y1, 1));
    QRect ZRectVOI(QPoint(z0, 0), QPoint(z1, 1));
    int x0a = XRectDisplayed.intersected(XRectVOI).left();
    int x1a = XRectDisplayed.intersected(XRectVOI).right();
    int y0a = YRectDisplayed.intersected(YRectVOI).left();
    int y1a = YRectDisplayed.intersected(YRectVOI).right();
    int z0a = ZRectDisplayed.intersected(ZRectVOI).left();
    int z1a = ZRectDisplayed.intersected(ZRectVOI).right();

    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, available voi is x0a = %d, x1a = %d, y0a = %d, y1a = %d, z0a = %d, z1a = %d",
                                        titleShort.c_str(), x0a, x1a, y0a, y1a, z0a, z1a).c_str(), __itm__current__function__);

    //if copyable VOI is empty, returning the black-initialized image
    if(x1a - x0a <= 0 || y1a - y0a <= 0 || z1a - z0a <= 0)
        return img;

    //otherwise COPYING + SCALING
    //fast scaling by pixel replication
    // - NOTE: interpolated image is allowed to be slightly larger (or even smaller) than the source image resulting after scaling.
    if( ( (xDimInterp % (x1-x0) <= 1) || (xDimInterp % (x1-x0+1) <= 1) || (xDimInterp % (x1-x0-1) <= 1)) &&
        ( (yDimInterp % (y1-y0) <= 1) || (yDimInterp % (y1-y0+1) <= 1) || (yDimInterp % (y1-y0-1) <= 1)) &&
        ( (zDimInterp % (z1-z0) <= 1) || (zDimInterp % (z1-z0+1) <= 1) || (zDimInterp % (z1-z0-1) <= 1)))
    {
        //checking for uniform scaling along the three axes
        uint scalx = static_cast<uint>(static_cast<float>(xDimInterp) / (x1-x0) +0.5f);
        uint scaly = static_cast<uint>(static_cast<float>(yDimInterp) / (y1-y0) +0.5f);
        uint scalz = static_cast<uint>(static_cast<float>(zDimInterp) / (z1-z0) +0.5f);
        if(scalx != scaly || scaly != scalz || scalx != scalz)
            QMessageBox::critical(this, "Error", QString("Fast nonuniform scaling not supported: requested scaling along X,Y,Z is") + QString::number(scalx) + "," + QString::number(scaly) + "," + QString::number(scalz),QObject::tr("Ok"));

        uint32 buf_data_dims[4]   = {volH1-volH0, volV1-volV0, volD1-volD0, nchannels};
        uint32 img_dims[4]        = {xDimInterp,  yDimInterp,  zDimInterp,  nchannels};
        uint32 buf_data_offset[3] = {x0a-volH0,   y0a-volV0,   z0a-volD0};
        uint32 img_offset[3]      = {x0a-x0,      y0a-y0,      z0a-z0};
        uint32 buf_data_count[3]  = {x1a-x0a,     y1a-y0a,     z1a-z0a};

        copyVOI(view3DWidget->getiDrawExternalParameter()->image4d->getRawData(), buf_data_dims, buf_data_offset, buf_data_count, img, img_dims, img_offset, scalx);
    }

    //interpolation
    else
        QMessageBox::critical(this, "Error", "Interpolation of the pre-buffered image not yet implemented",QObject::tr("Ok"));


    return img;
}


/**********************************************************************************
* Copies the given VOI from "src" to "dst". Offsets and scaling are supported.
***********************************************************************************/
void
    CExplorerWindow::copyVOI(
        uint8 const * src,          //pointer to const data source
        uint src_dims[4],           //dimensions of "src" along X, Y, Z and channels
        uint src_offset[3],         //VOI's offset along X, Y, Z
        uint src_count[3],          //VOI's dimensions along X, Y, Z
        uint8* dst,                 //pointer to data destination
        uint dst_dims[4],           //dimensions of "dst" along X, Y, Z and channels
        uint dst_offset[3],         //offset of "dst" along X, Y, Z
        uint scaling /*= 1 */)      //scaling factor (integer only)
throw (MyException)
{
    /**/itm::debug(itm::LEV1, strprintf("src_dims = (%d x %d x %d x %d), src_offset = (%d, %d, %d), src_count = (%d, %d, %d), dst_dims = (%d x %d x %d x %d), dst_offset = (%d, %d, %d), scaling = %d",
                                        src_dims[0], src_dims[1],src_dims[2],src_dims[3], src_offset[0],src_offset[1],src_offset[2], src_count[0],src_count[1],src_count[2],
                                        dst_dims[0], dst_dims[1],dst_dims[2],dst_dims[3], dst_offset[0],dst_offset[1],dst_offset[2], scaling).c_str(), __itm__current__function__);

    //if source and destination are the same thing, returning without doing anything
    if(src == dst)
        return;

    //cheking preconditions
    if(src_dims[3] != dst_dims[3])
        QMessageBox::critical(0, "Error", "Can't copy VOI to destination image: different number of channels",QObject::tr("Ok"));
    for(int d=0; d<3; d++)
    {
        if(src_offset[d] + src_count[d] > src_dims[d])
            QMessageBox::critical(0, "Error", QString("Can't copy VOI to destination image: VOI exceeded source dimension along axis ").append(QString::number(d)),QObject::tr("Ok"));
        if(dst_offset[d] + src_count[d]*scaling > dst_dims[d])
        {
            //cutting copiable VOI to the largest one that can be stored into the destination image
            int old = src_count[d];
            src_count[d] = (dst_dims[d] - dst_offset[d]) / scaling; //it's ok to approximate this calculation to the floor.

            printf("--------------------- teramanager plugin [thread *] !! WARNING in copyVOI !! VOI exceeded destination dimension along axis %d, then cutting VOI from %d to %d\n",
                   d, old, src_count[d]);

            //QMessageBox::critical(0, "Error", QString("Can't copy VOI to destination image: VOI exceeded destination dimension along axis ").append(QString::number(d)),QObject::tr("Ok"));
        }
    }

    //quick version (with precomputed offsets, strides and counts: "1" for "dst", "2" for "src")
    uint32 stride_c1       =  dst_dims [2] * dst_dims[1]   * dst_dims[0];
    uint32 stride_c2       =  src_dims [2] * src_dims[1]   * src_dims[0];
    const uint32 stride_k1 =                 dst_dims[1]   * dst_dims[0] * scaling;
    const uint32 count_k1  = src_count [2] * dst_dims[1]   * dst_dims[0] * scaling;
    const uint32 offset_k2 = src_offset[2] * src_dims[1]   * src_dims[0];
    const uint32 stride_k2 =                 src_dims[1]   * src_dims[0];
    //const uint32 count_k2= src_count [2] * src_dims[1]   * src_dims[0];            //not used in the OR so as to speed up the inner loops
    const uint32 stride_i1 =                                 dst_dims[0] * scaling;
    const uint32 count_i1  =                 src_count[1]  * dst_dims[0] * scaling;
    const uint32 offset_i2 =                 src_offset[1] * src_dims[0];
    const uint32 stride_i2 =                                 src_dims[0];
    //const uint32 count_i2  =               src_count[1]  * src_dims[0];            //not used in the OR so as to speed up the inner loops
    const uint32 stride_j1 =                                               scaling;
    const uint32 count_j1  =                                 src_count[0]* scaling;
    const uint32 offset_j2 =                                 src_offset[0];
    //const uint32 count_j2  =                               src_count[0];           //not used in the OR so as to speed up the inner loops
    const uint32 dst_dim = dst_dims [3] * dst_dims [2] * dst_dims [1] * dst_dims [0];

    for(int sk = 0; sk < scaling; sk++)
        for(int si = 0; si < scaling; si++)
            for(int sj = 0; sj < scaling; sj++)
            {
                const uint32 offset_k1 = dst_offset[2] * dst_dims[1]    * dst_dims[0]   * scaling + sk * dst_dims[1] * dst_dims[0] ;
                const uint32 offset_i1 =                 dst_offset[1]  * dst_dims[0]   * scaling + si * dst_dims[0];
                const uint32 offset_j1 =                                  dst_offset[0] * scaling + sj;

                for(uint8 *img_c1 = dst, *img_c2 = const_cast<uint8*>(src); img_c1 - dst < dst_dim; img_c1 += stride_c1, img_c2 += stride_c2)
                {
                    uint8* const start_k1 = img_c1 + offset_k1;
                    uint8* const start_k2 = img_c2 + offset_k2;
                    for(uint8 *img_k1 = start_k1, *img_k2 = start_k2; img_k1 - start_k1  < count_k1; img_k1 += stride_k1, img_k2 += stride_k2)
                    {
                        uint8* const start_i1 = img_k1 + offset_i1;
                        uint8* const start_i2 = img_k2 + offset_i2;
                        for(uint8 *img_i1 = start_i1, *img_i2 = start_i2; img_i1 - start_i1  < count_i1; img_i1 += stride_i1, img_i2 += stride_i2)
                        {
                            uint8* const start_j1 = img_i1 + offset_j1;
                            uint8* const start_j2 = img_i2 + offset_j2;
                            for(uint8 *img_j1 = start_j1, *img_j2 = start_j2; img_j1 - start_j1  < count_j1; img_j1 += stride_j1, img_j2++)
                                *img_j1 = *img_j2;
                        }
                    }
                }
             }
}

/**********************************************************************************
* Makes the current view the last one by  deleting (and deallocting) its subsequent
* views.
***********************************************************************************/
void CExplorerWindow::makeLastView() throw (MyException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

    if(CExplorerWindow::current != this)
        throw MyException(QString("in CExplorerWindow::makeLastView(): this view is not the current one, thus can't be made the last view").toStdString().c_str());

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
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

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
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

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
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

    QElapsedTimer timer;
    timer.start();

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
            editedMarkers[i].x = getGlobalHCoord(editedMarkers[i].x, -1, false, false, __itm__current__function__);
            editedMarkers[i].y = getGlobalVCoord(editedMarkers[i].y, -1, false, false, __itm__current__function__);
            editedMarkers[i].z = getGlobalDCoord(editedMarkers[i].z, -1, false, false, __itm__current__function__);
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
            editedCurves.listNeuron[i].x = getGlobalHCoord(editedCurves.listNeuron[i].x, -1, false, false, __itm__current__function__);
            editedCurves.listNeuron[i].y = getGlobalVCoord(editedCurves.listNeuron[i].y, -1, false, false, __itm__current__function__);
            editedCurves.listNeuron[i].z = getGlobalDCoord(editedCurves.listNeuron[i].z, -1, false, false, __itm__current__function__);
        }

        //storing markers
        CAnnotations::getInstance()->addCurves(&editedCurves);
    }

    PLog::getInstance()->appendCPU(timer.elapsed(), QString("Stored 3D annotations from view ").append(title.c_str()).toStdString());
}

void CExplorerWindow::loadAnnotations() throw (MyException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

    QElapsedTimer timer;
    timer.start();

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

    PLog::getInstance()->appendCPU(timer.elapsed(), QString("Loaded 3D annotations into view ").append(title.c_str()).toStdString());

    //assigning annotations
    timer.restart();
    V3D_env->setLandmark(window, vaa3dMarkers);
    V3D_env->setSWC(window, vaa3dCurves);
    V3D_env->pushObjectIn3DWindow(window);
    view3DWidget->enableMarkerLabel(false);
    view3DWidget->getRenderer()->endSelectMode();
    PLog::getInstance()->appendGPU(timer.elapsed(), QString("Loaded 3D annotations into view ").append(title.c_str()).toStdString());
}

/**********************************************************************************
* Restores the current view from the given (neighboring) view.
* Called by the next(prev) <CExplorerWindow>  when the user  zooms out(in) and  the
* lower(higher) resoolution has to be reestabilished.
***********************************************************************************/
void CExplorerWindow::restoreViewFrom(CExplorerWindow* source) throw (MyException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s, source->title = %s", titleShort.c_str(), source->titleShort.c_str()).c_str(), __itm__current__function__);

    if(source)
    {
        //signal disconnections
        source->disconnect(CVolume::instance(), SIGNAL(sendOperationOutcome(uint8*, MyException*,void*,qint64, QString, int)), source, SLOT(loadingDone(uint8*, MyException*,void*,qint64, QString, int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeXCut0(int)), source, SLOT(Vaa3D_changeXCut0(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeXCut1(int)), source, SLOT(Vaa3D_changeXCut1(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeYCut0(int)), source, SLOT(Vaa3D_changeYCut0(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeYCut1(int)), source, SLOT(Vaa3D_changeYCut1(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeZCut0(int)), source, SLOT(Vaa3D_changeZCut0(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeZCut1(int)), source, SLOT(Vaa3D_changeZCut1(int)));
        source->disconnect(source->view3DWidget, SIGNAL(xRotationChanged(int)), source, SLOT(Vaa3D_rotationchanged(int)));
        source->disconnect(source->view3DWidget, SIGNAL(yRotationChanged(int)), source, SLOT(Vaa3D_rotationchanged(int)));
        source->disconnect(source->view3DWidget, SIGNAL(zRotationChanged(int)), source, SLOT(Vaa3D_rotationchanged(int)));
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


        //showing current view (with triViewWidget minimized)
        triViewWidget->setWindowState(Qt::WindowMinimized);

        //positioning the current 3D window exactly at the <source> window position
        QPoint location = source->window3D->pos();
        resize(source->window3D->size());
        move(location);

        //hiding <source>
        source->window3D->setVisible(false);
        //source->triViewWidget->setVisible(false);       //---- Alessandro 2013-09-04 fixed: this causes Vaa3D's setCurHiddenSelectedWindow() to fail
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
        connect(CVolume::instance(), SIGNAL(sendOperationOutcome(uint8*, MyException*,void*,qint64, QString, int)), this, SLOT(loadingDone(uint8*, MyException*,void*,qint64, QString, int)), Qt::BlockingQueuedConnection);
        connect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
        connect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
        connect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
        connect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
        connect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
        connect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
        connect(view3DWidget, SIGNAL(xRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        connect(view3DWidget, SIGNAL(yRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        connect(view3DWidget, SIGNAL(zRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
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

        //current windows not gets ready to user input
        isReady = true;
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
    /**/itm::debug(itm::LEV1, strprintf("title = %s, x = %d, y = %d", titleShort.c_str(), x, y).c_str(), __itm__current__function__);

    #ifdef USE_EXPERIMENTAL_FEATURES
    return myRenderer_gl1::cast(static_cast<Renderer_gl1*>(view3DWidget->getRenderer()))->get3DPoint(x, y);
    #else
    throw MyException("Double click zoom-in feature is disabled in the current version");
    #endif
}

/**********************************************************************************
* method (indirectly) invoked by Vaa3D to propagate VOI's coordinates
***********************************************************************************/
void CExplorerWindow::invokedFromVaa3D(v3d_imaging_paras* params /* = 0 */)
{
    if(!isActive || toBeClosed)
        return;

    if(params)
        /**/itm::debug(itm::LEV1, strprintf("title = %s, params = [%d-%d] x [%d-%d] x [%d-%d]", titleShort.c_str(), params->xs, params->xe, params->ys, params->ye, params->zs, params->ze).c_str(), __itm__current__function__);
    else
        /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

    //--- Alessandro 04/09/2013: added precondition checks to solve a bug which causes invokedFromVaa3D to be called without the customStructPointer available
    //                           in Vaa3D. This happens because Vaa3D someway bypasses TeraFly for handling the zoomin-in with mouse scroll or because TeraFly
    //                           does not correctly disconnects Vaa3D from the setZoom slots as a new view is created.
    v3d_imaging_paras* roi = 0;
    if(params)
        roi = params;
    else
    {
        if(view3DWidget->getiDrawExternalParameter())
        {
            if(view3DWidget->getiDrawExternalParameter()->image4d)
            {
                if(view3DWidget->getiDrawExternalParameter()->image4d->getCustomStructPointer())
                    roi = static_cast<v3d_imaging_paras*>(view3DWidget->getiDrawExternalParameter()->image4d->getCustomStructPointer());
                else
                    ;//QMessageBox::critical(this,QObject::tr("Error"), QObject::tr("Unable to get customStructPointer from Vaa3D V3dR_GLWidget"),QObject::tr("Ok"));
            }
            else
                ;//QMessageBox::critical(this,QObject::tr("Error"), QObject::tr("Unable to get My4DImage from Vaa3D V3dR_GLWidget"),QObject::tr("Ok"));
        }
        else
            ;//QMessageBox::critical(this,QObject::tr("Error"), QObject::tr("Unable to get iDrawExternalParameter from Vaa3D V3dR_GLWidget"),QObject::tr("Ok"));
    }
    if(!roi)
    {
        /**/itm::warning(strprintf("title = %s, Unable to get customStructPointer from Vaa3D V3dR_GLWidget. Aborting invokedFromVaa3D()", titleShort.c_str()).c_str(), __itm__current__function__);
        return;
    }

    if(params == 0)
        /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, Vaa3D ROI is [%d-%d] x [%d-%d] x [%d-%d]", titleShort.c_str(), roi->xs, roi->xe, roi->ys, roi->ye, roi->zs, roi->ze).c_str(), __itm__current__function__);


    //--- Alessandro 24/08/2013: to prevent the plugin to crash at the deepest resolution when the plugin is invoked by Vaa3D
    if(volResIndex == CImport::instance()->getResolutions()-1)
    {
        QMessageBox::warning(this, "Warning", "Vaa3D-invoked actions at the highest resolution have been temporarily removed. "
                             "Please use different operations such as \"Double-click zoom-in\" or \"Traslate\".");
        return;
    }

    //retrieving ROI infos propagated by Vaa3D
    int roiCenterX = roi->xe-(roi->xe-roi->xs)/2;
    int roiCenterY = roi->ye-(roi->ye-roi->ys)/2;
    int roiCenterZ = roi->ze-(roi->ze-roi->zs)/2;

    //zoom-in around marker or ROI triggers a new window
    if(roi->ops_type == 1)
        newView(roi->xe, roi->ye, roi->ze, volResIndex+1, false, -1, -1, -1, roi->xs, roi->ys, roi->zs);

    //zoom-in with mouse scroll up may trigger a new window if caching is not possible
    else if(roi->ops_type == 2)
    {
        if(volResIndex != CImport::instance()->getResolutions()-1 &&   //do nothing if highest resolution has been reached
           isZoomDerivativePos())                                      //accepting zoom-in only when zoom factor derivative is positive
        {
            //trying caching if next view exists
            if(next)
            {
                //converting Vaa3D VOI local coordinates to TeraFly coordinates of the next resolution
                float gXS = getGlobalHCoord(static_cast<float>(roi->xs), next->volResIndex, false, true, __itm__current__function__);
                float gXE = getGlobalHCoord(static_cast<float>(roi->xe), next->volResIndex, false, true, __itm__current__function__);
                float gYS = getGlobalVCoord(static_cast<float>(roi->ys), next->volResIndex, false, true, __itm__current__function__);
                float gYE = getGlobalVCoord(static_cast<float>(roi->ye), next->volResIndex, false, true, __itm__current__function__);
                float gZS = getGlobalDCoord(static_cast<float>(roi->zs), next->volResIndex, false, true, __itm__current__function__);
                float gZE = getGlobalDCoord(static_cast<float>(roi->ze), next->volResIndex, false, true, __itm__current__function__);
                QRectF gXRect(QPointF(gXS, 0), QPointF(gXE, 1));
                QRectF gYRect(QPointF(gYS, 0), QPointF(gYE, 1));
                QRectF gZRect(QPointF(gZS, 0), QPointF(gZE, 1));

                //obtaining the actual requested VOI under the existing constraints (i.e., maximum view dimensions)
                /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, requested voi is [%.0f, %.0f] x [%.0f, %.0f] x [%.0f, %.0f]...BUT",
                                                       titleShort.c_str(), gXS, gXE, gYS, gYE, gZS, gZE).c_str(), __itm__current__function__);

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
                /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, ...BUT actual requested VOI became [%.0f, %.0f] x [%.0f, %.0f] x [%.0f, %.0f]",
                                                       titleShort.c_str(), gXS, gXE, gYS, gYE, gZS, gZE).c_str(), __itm__current__function__);

                //obtaining coordinates of the cached resolution
                float gXScached = static_cast<float>(next->volH0);
                float gXEcached = static_cast<float>(next->volH1);
                float gYScached = static_cast<float>(next->volV0);
                float gYEcached = static_cast<float>(next->volV1);
                float gZScached = static_cast<float>(next->volD0);
                float gZEcached = static_cast<float>(next->volD1);
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

                /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, actual requested voi is [%.0f, %.0f] x [%.0f, %.0f] x [%.0f, %.0f] and cached is [%.0f, %.0f] x [%.0f, %.0f] x [%.0f, %.0f]",
                                                       titleShort.c_str(), gXS, gXE, gYS, gYE, gZS, gZE, gXScached, gXEcached, gYScached, gYEcached, gZScached, gZEcached).c_str(), __itm__current__function__);

                /**/itm::debug(itm::LEV_MAX, strprintf("title = %s,intersection is %.0f x %.0f x %.0f with coverage factor = %.2f ", titleShort.c_str(),
                                                       intersectionX, intersectionY, intersectionZ, coverageFactor).c_str(), __itm__current__function__);

                //if Vaa3D VOI is covered for the selected percentage by the existing cached volume, just restoring its view
                if(coverageFactor >= PMain::getInstance()->cacheSens->value()/100.0f)
                {
                    setActive(false);
                    resetZoomHistory();
                    next->restoreViewFrom(this);
                }

                //otherwise invoking a new view
                else
                    newView(roiCenterX, roiCenterY, roiCenterZ, volResIndex+1, false, static_cast<int>((roi->xe-roi->xs)/2.0f+0.5f), static_cast<int>((roi->ye-roi->ys)/2.0f+0.5f), static_cast<int>((roi->ze-roi->zs)/2.0f+0.5f));
            }
            else
                newView(roiCenterX, roiCenterY, roiCenterZ, volResIndex+1, false, static_cast<int>((roi->xe-roi->xs)/2.0f+0.5f), static_cast<int>((roi->ye-roi->ys)/2.0f+0.5f), static_cast<int>((roi->ze-roi->zs)/2.0f+0.5f));
        }
        else
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, ignoring Vaa3D mouse scroll up zoom-in", titleShort.c_str()).c_str(), __itm__current__function__);
    }
    else
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr("in CExplorerWindow::Vaa3D_selectedROI(): unsupported (or unset) operation type"),QObject::tr("Ok"));
}

/**********************************************************************************
* Returns  the  global coordinate  (which starts from 0) in  the given  resolution
* volume image space given the local coordinate (which starts from 0) in the current
* resolution volume image space. If resIndex is not set, the returned global coord-
* inate will be in the highest resolution image space.
***********************************************************************************/
int CExplorerWindow::getGlobalVCoord(int localVCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = false */, bool cutOutOfRange /* = false */, const char *src /* =0 */)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s, coord = %d, res = %d, fromVaa3D = %s, cutOutOfRange = %s, src = %s",
                                        titleShort.c_str(), localVCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false", cutOutOfRange ? "true" : "false", src ? src : "unknown").c_str(), __itm__current__function__);

    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //cutting out-of-range coordinate, if <cutOutOfRange> is set
    if(cutOutOfRange)
    {
        localVCoord = localVCoord <  0                                                       ? 0                                                       : localVCoord;
        localVCoord = localVCoord >= CImport::instance()->getVolume(volResIndex)->getDIM_V() ? CImport::instance()->getVolume(volResIndex)->getDIM_V() : localVCoord;
    }

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volV1-volV0 > LIMIT_VOLY))
        localVCoord = static_cast<int>(localVCoord* ( static_cast<float>(volV1-volV0-1)/(LIMIT_VOLY-1) ) +0.5f);

    //--- Alessandro 29/09/2013: fixing "division by zero" bug in case of volumes with 1 pixel size along the considered direction
    if(CImport::instance()->getVolume(volResIndex)->getDIM_V() == 1)
        return 0;

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);

    /**/itm::debug(itm::LEV3, strprintf("title = %s, returning %d", titleShort.c_str(), static_cast<int>((volV0+localVCoord)*ratio + 0.5f)).c_str(), __itm__current__function__);

    return (volV0+localVCoord)*ratio + 0.5f;
}
int CExplorerWindow::getGlobalHCoord(int localHCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = false */, bool cutOutOfRange /* = false */, const char *src /* =0 */)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s, coord = %d, res = %d, fromVaa3D = %s, cutOutOfRange = %s, src = %s",
                                        titleShort.c_str(), localHCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false", cutOutOfRange ? "true" : "false", src ? src : "unknown").c_str(), __itm__current__function__);

    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //cutting out-of-range coordinate, if <cutOutOfRange> is set
    if(cutOutOfRange)
    {
        localHCoord = localHCoord <  0                                                       ? 0                                                       : localHCoord;
        localHCoord = localHCoord >= CImport::instance()->getVolume(volResIndex)->getDIM_H() ? CImport::instance()->getVolume(volResIndex)->getDIM_H() : localHCoord;
    }

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volH1-volH0 > LIMIT_VOLX))
        localHCoord = static_cast<int>(localHCoord* ( static_cast<float>(volH1-volH0-1)/(LIMIT_VOLX-1) ) +0.5f);

    //--- Alessandro 29/09/2013: fixing "division by zero" bug in case of volumes with 1 pixel size along the considered direction
    if(CImport::instance()->getVolume(volResIndex)->getDIM_H() == 1)
        return 0;

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);

    /**/itm::debug(itm::LEV3, strprintf("title = %s, returning %d", titleShort.c_str(), static_cast<int>((volH0+localHCoord)*ratio + 0.5f)).c_str(), __itm__current__function__);

    return (volH0+localHCoord)*ratio + 0.5f;
}
int CExplorerWindow::getGlobalDCoord(int localDCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = false */, bool cutOutOfRange /* = false */, const char *src /* =0 */)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s, coord = %d, res = %d, fromVaa3D = %s, cutOutOfRange = %s, src = %s",
                                        titleShort.c_str(), localDCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false", cutOutOfRange ? "true" : "false", src ? src : "unknown").c_str(), __itm__current__function__);

    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //cutting out-of-range coordinate, if <cutOutOfRange> is set
    if(cutOutOfRange)
    {
        localDCoord = localDCoord <  0                                                       ? 0                                                       : localDCoord;
        localDCoord = localDCoord >= CImport::instance()->getVolume(volResIndex)->getDIM_D() ? CImport::instance()->getVolume(volResIndex)->getDIM_D() : localDCoord;
    }

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volD1-volD0 > LIMIT_VOLZ))
        localDCoord = static_cast<int>(localDCoord* ( static_cast<float>(volD1-volD0-1)/(LIMIT_VOLZ-1) ) +0.5f);

    //--- Alessandro 29/09/2013: fixing "division by zero" bug in case of volumes with 1 pixel size along the considered direction
    if(CImport::instance()->getVolume(volResIndex)->getDIM_D() == 1)
        return 0;

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);

    /**/itm::debug(itm::LEV3, strprintf("title = %s, returning %d", titleShort.c_str(), static_cast<int>((volD0+localDCoord)*ratio + 0.5f)).c_str(), __itm__current__function__);

    return (volD0+localDCoord)*ratio + 0.5f;
}
float CExplorerWindow::getGlobalVCoord(float localVCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = false */, bool cutOutOfRange /* = false */, const char *src /* =0 */)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s, coord = %.2f, res = %d, fromVaa3D = %s, cutOutOfRange = %s, src = %s",
                                        titleShort.c_str(), localVCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false", cutOutOfRange ? "true" : "false", src ? src : "unknown").c_str(), __itm__current__function__);

    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //cutting out-of-range coordinate, if <cutOutOfRange> is set
    if(cutOutOfRange)
    {
        localVCoord = localVCoord <  0                                                       ? 0                                                       : localVCoord;
        localVCoord = localVCoord >= CImport::instance()->getVolume(volResIndex)->getDIM_V() ? CImport::instance()->getVolume(volResIndex)->getDIM_V() : localVCoord;
    }

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volV1-volV0 > LIMIT_VOLY))
        localVCoord *= static_cast<float>(volV1-volV0-1)/(LIMIT_VOLY-1);

    //--- Alessandro 29/09/2013: fixing "division by zero" bug in case of volumes with 1 pixel size along the considered direction
    if(CImport::instance()->getVolume(volResIndex)->getDIM_V() == 1)
        return 0.0f;

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);

    /**/itm::debug(itm::LEV3, strprintf("title = %s, returning %.2f", titleShort.c_str(), (volV0+localVCoord)*ratio).c_str(), __itm__current__function__);

    return (volV0+localVCoord)*ratio;
}
float CExplorerWindow::getGlobalHCoord(float localHCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = false */, bool cutOutOfRange /* = false */, const char *src /* =0 */)
{

    /**/itm::debug(itm::LEV3, strprintf("title = %s, coord = %.2f, res = %d, fromVaa3D = %s, cutOutOfRange = %s, src = %s",
                                        titleShort.c_str(), localHCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false", cutOutOfRange ? "true" : "false", src ? src : "unknown").c_str(), __itm__current__function__);

    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //cutting out-of-range coordinate, if <cutOutOfRange> is set
    if(cutOutOfRange)
    {
        localHCoord = localHCoord <  0                                                       ? 0                                                       : localHCoord;
        localHCoord = localHCoord >= CImport::instance()->getVolume(volResIndex)->getDIM_H() ? CImport::instance()->getVolume(volResIndex)->getDIM_H() : localHCoord;
    }

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volH1-volH0 > LIMIT_VOLX))
        localHCoord *= static_cast<float>(volH1-volH0-1)/(LIMIT_VOLX-1);

    //--- Alessandro 29/09/2013: fixing "division by zero" bug in case of volumes with 1 pixel size along the considered direction
    if(CImport::instance()->getVolume(volResIndex)->getDIM_H() == 1)
        return 0.0f;

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);

    /**/itm::debug(itm::LEV3, strprintf("title = %s, returning %.2f", titleShort.c_str(), (volH0+localHCoord)*ratio).c_str(), __itm__current__function__);

    return (volH0+localHCoord)*ratio;
}
float CExplorerWindow::getGlobalDCoord(float localDCoord, int resIndex /* = -1 */, bool fromVaa3Dcoordinates /* = false */, bool cutOutOfRange /* = false */, const char *src /* =0 */)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s, coord = %.2f, res = %d, fromVaa3D = %s, cutOutOfRange = %s, src = %s",
                                        titleShort.c_str(), localDCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false", cutOutOfRange ? "true" : "false", src ? src : "unknown").c_str(), __itm__current__function__);

    //setting resIndex if it has not been set
    if(resIndex == -1)
        resIndex = CImport::instance()->getResolutions()-1;

    //cutting out-of-range coordinate, if <cutOutOfRange> is set
    if(cutOutOfRange)
    {
        localDCoord = localDCoord <  0                                                       ? 0                                                       : localDCoord;
        localDCoord = localDCoord >= CImport::instance()->getVolume(volResIndex)->getDIM_D() ? CImport::instance()->getVolume(volResIndex)->getDIM_D() : localDCoord;
    }

    //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
    if(fromVaa3Dcoordinates && (volD1-volD0 > LIMIT_VOLZ))
        localDCoord *= static_cast<float>(volD1-volD0-1)/(LIMIT_VOLZ-1);

    //--- Alessandro 29/09/2013: fixing "division by zero" bug in case of volumes with 1 pixel size along the considered direction
    if(CImport::instance()->getVolume(volResIndex)->getDIM_D() == 1)
        return 0.0f;

    float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);

    /**/itm::debug(itm::LEV3, strprintf("title = %s, returning %.2f", titleShort.c_str(), (volD0+localDCoord)*ratio).c_str(), __itm__current__function__);

    return (volD0+localDCoord)*ratio;
}

/**********************************************************************************
* Returns the local coordinate (which starts from 0) in the current resolution vol-
* ume image space given the global coordinate  (which starts from 0) in the highest
* resolution volume image space.
***********************************************************************************/
int CExplorerWindow::getLocalVCoord(int highestResGlobalVCoord, bool toVaa3Dcoordinates /* = false */)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s, coord = %d, toVaa3Dcoordinates = %s",
                                        titleShort.c_str(), highestResGlobalVCoord, toVaa3Dcoordinates ? "true" : "false").c_str(), __itm__current__function__);

    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);
    int localCoord =  static_cast<int>(highestResGlobalVCoord/ratio - volV0 + 0.5f);

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(toVaa3Dcoordinates && (volV1-volV0 > LIMIT_VOLY))
        localCoord = static_cast<int>(localCoord* ( static_cast<float>(LIMIT_VOLY-1)/(volV1-volV0-1) ) +0.5f);

    /**/itm::debug(itm::LEV3, strprintf("title = %s, returning %d", titleShort.c_str(), localCoord).c_str(), __itm__current__function__);

    return localCoord;
}
int CExplorerWindow::getLocalHCoord(int highestResGlobalHCoord, bool toVaa3Dcoordinates /* = false */)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s, coord = %d, toVaa3Dcoordinates = %s",
                                        titleShort.c_str(), highestResGlobalHCoord, toVaa3Dcoordinates ? "true" : "false").c_str(), __itm__current__function__);

    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);
    int localCoord =  static_cast<int>(highestResGlobalHCoord/ratio - volH0 + 0.5f);

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(toVaa3Dcoordinates && (volH1-volH0 > LIMIT_VOLX))
        localCoord = static_cast<int>(localCoord* ( static_cast<float>(LIMIT_VOLX-1)/(volH1-volH0-1) ) +0.5f);

    /**/itm::debug(itm::LEV3, strprintf("title = %s, returning %d", titleShort.c_str(), localCoord).c_str(), __itm__current__function__);

    return localCoord;
}
int CExplorerWindow::getLocalDCoord(int highestResGlobalDCoord, bool toVaa3Dcoordinates /* = false */)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s, coord = %d, toVaa3Dcoordinates = %s",
                                        titleShort.c_str(), highestResGlobalDCoord, toVaa3Dcoordinates ? "true" : "false").c_str(), __itm__current__function__);

    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);
    int localCoord =  static_cast<int>(highestResGlobalDCoord/ratio - volD0 + 0.5f);

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(toVaa3Dcoordinates && (volD1-volD0 > LIMIT_VOLZ))
        localCoord = static_cast<int>(localCoord* ( static_cast<float>(LIMIT_VOLZ-1)/(volD1-volD0-1) ) +0.5f);

    /**/itm::debug(itm::LEV3, strprintf("title = %s, returning %d", titleShort.c_str(), localCoord).c_str(), __itm__current__function__);

    return localCoord;
}
float CExplorerWindow::getLocalVCoord(float highestResGlobalVCoord, bool toVaa3Dcoordinates /* = false */)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s, coord = %.2f, toVaa3Dcoordinates = %s",
                                        titleShort.c_str(), highestResGlobalVCoord, toVaa3Dcoordinates ? "true" : "false").c_str(), __itm__current__function__);

    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);
    float localCoord = highestResGlobalVCoord/ratio - volV0;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(toVaa3Dcoordinates && (volV1-volV0 > LIMIT_VOLY))
        localCoord = localCoord* ( static_cast<float>(LIMIT_VOLY-1)/(volV1-volV0-1) );

    /**/itm::debug(itm::LEV3, strprintf("title = %s, returning %.2f", titleShort.c_str(), localCoord).c_str(), __itm__current__function__);

    return localCoord;
}
float CExplorerWindow::getLocalHCoord(float highestResGlobalHCoord, bool toVaa3Dcoordinates /* = false */)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s, coord = %.2f, toVaa3Dcoordinates = %s",
                                        titleShort.c_str(), highestResGlobalHCoord, toVaa3Dcoordinates ? "true" : "false").c_str(), __itm__current__function__);

    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);
    float localCoord = highestResGlobalHCoord/ratio - volH0;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(toVaa3Dcoordinates && (volH1-volH0 > LIMIT_VOLX))
        localCoord = localCoord* ( static_cast<float>(LIMIT_VOLX-1)/(volH1-volH0-1) );

    /**/itm::debug(itm::LEV3, strprintf("title = %s, returning %.2f", titleShort.c_str(), localCoord).c_str(), __itm__current__function__);

    return localCoord;
}
float CExplorerWindow::getLocalDCoord(float highestResGlobalDCoord, bool toVaa3Dcoordinates /* = false */)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s, coord = %.2f, toVaa3Dcoordinates = %s",
                                        titleShort.c_str(), highestResGlobalDCoord, toVaa3Dcoordinates ? "true" : "false").c_str(), __itm__current__function__);

    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);
    float localCoord = highestResGlobalDCoord/ratio - volD0;

    //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
    if(toVaa3Dcoordinates && (volD1-volD0 > LIMIT_VOLZ))
        localCoord = localCoord* ( static_cast<float>(LIMIT_VOLZ-1)/(volD1-volD0-1) );

    /**/itm::debug(itm::LEV3, strprintf("title = %s, returning %.2f", titleShort.c_str(), localCoord).c_str(), __itm__current__function__);

    return localCoord;
}

/**********************************************************************************
* Linked to volume cut scrollbars of Vaa3D widget containing the 3D renderer.
* This implements the syncronization Vaa3D-->TeraManager of subvolume selection.
***********************************************************************************/
void CExplorerWindow::Vaa3D_changeYCut0(int s)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);

    disconnect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
    PMain::getInstance()->V0_sbox->setValue(getGlobalVCoord(s, -1, true, false, __itm__current__function__)+1);
    connect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
}
void CExplorerWindow::Vaa3D_changeYCut1(int s)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);

    disconnect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
    PMain::getInstance()->V1_sbox->setValue(getGlobalVCoord(s, -1, true, false, __itm__current__function__)+1);
    connect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
}
void CExplorerWindow::Vaa3D_changeXCut0(int s)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);

    disconnect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
    PMain::getInstance()->H0_sbox->setValue(getGlobalHCoord(s, -1, true, false, __itm__current__function__)+1);
    connect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
}
void CExplorerWindow::Vaa3D_changeXCut1(int s)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);

    disconnect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
    PMain::getInstance()->H1_sbox->setValue(getGlobalHCoord(s, -1, true, false, __itm__current__function__)+1);
    connect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
}
void CExplorerWindow::Vaa3D_changeZCut0(int s)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);

    disconnect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
    PMain::getInstance()->D0_sbox->setValue(getGlobalDCoord(s, -1, true, false, __itm__current__function__)+1);
    connect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
}
void CExplorerWindow::Vaa3D_changeZCut1(int s)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);

    disconnect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));
    PMain::getInstance()->D1_sbox->setValue(getGlobalDCoord(s, -1, true, false, __itm__current__function__)+1);
    connect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));
}

/**********************************************************************************
* Linked to PMain GUI VOI's widgets.
* This implements the syncronization TeraManager-->Vaa3D of subvolume selection.
***********************************************************************************/
void CExplorerWindow::PMain_changeV0sbox(int s)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);

    disconnect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
    view3DWidget->setYCut0(getLocalVCoord(s-1, true)+1);
    connect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
}
void CExplorerWindow::PMain_changeV1sbox(int s)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);

    disconnect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
    view3DWidget->setYCut1(getLocalVCoord(s-1, true)+1);
    connect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
}
void CExplorerWindow::PMain_changeH0sbox(int s)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);

    disconnect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
    view3DWidget->setXCut0(getLocalHCoord(s-1, true)+1);
    connect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
}
void CExplorerWindow::PMain_changeH1sbox(int s)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);

    disconnect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
    view3DWidget->setXCut1(getLocalHCoord(s-1, true)+1);
    connect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
}
void CExplorerWindow::PMain_changeD0sbox(int s)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);

    disconnect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
    view3DWidget->setZCut0(getLocalDCoord(s-1, true)+1);
    connect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
}
void CExplorerWindow::PMain_changeD1sbox(int s)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);

    disconnect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
    view3DWidget->setZCut1(getLocalDCoord(s-1, true)+1);
    connect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
}

/**********************************************************************************
* Alignes the given widget to the left (or to the right) of the current window
***********************************************************************************/
void CExplorerWindow::alignToLeft(QWidget* widget)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

    int widget_new_x = window3D->x() + window3D->width() + 3;
    int widget_new_y = window3D->y();
    int widget_new_height = window3D->height();

    if(widget->x() != widget_new_x || widget->y() != widget_new_y)
        widget->move(widget_new_x, widget_new_y);
    if(widget->height() != widget_new_height)
    {
        widget->setMaximumHeight(std::max(widget_new_height,widget->height()));
        widget->resize(widget->width(), widget_new_height);
    }
}
void CExplorerWindow::alignToRight(QWidget* widget)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

    int widget_new_x = window3D->x() - widget->width() - 3;
    int widget_new_y = window3D->y();
    int widget_new_height = window3D->height();

    if(widget->x() != widget_new_x || widget->y() != widget_new_y)
        widget->move(widget_new_x, widget_new_y);
    if(widget->height() != widget_new_height)
    {
        widget->setMaximumHeight(std::max(widget_new_height,widget->height()));
        widget->resize(widget->width(), widget_new_height);
    }
}

/**********************************************************************************
* Linked to PMain GUI QGLRefSys widget.
* This implements the syncronization Vaa3D-->TeraFly of rotations.
***********************************************************************************/
void CExplorerWindow::Vaa3D_rotationchanged(int s)
{
    if(isActive && !toBeClosed)
    {
//        printf("disconnect\n");
//        disconnect(view3DWidget, SIGNAL(xRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
//        disconnect(view3DWidget, SIGNAL(yRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
//        disconnect(view3DWidget, SIGNAL(zRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));

        //printf("view3DWidget->absoluteRotPose()\n");
        //view3DWidget->absoluteRotPose();
        //printf("PMain::getInstance()->refSys\n");
        QGLRefSys* refsys = PMain::getInstance()->refSys;
        //printf(" refsys->setXRotation(view3DWidget->xRot())\n");
        refsys->setXRotation(view3DWidget->xRot());
        //printf(" refsys->setXRotation(view3DWidget->yRot())\n");
        refsys->setYRotation(view3DWidget->yRot());
        //printf(" refsys->setXRotation(view3DWidget->zRot())\n");
        refsys->setZRotation(view3DWidget->zRot());

        //view3DWidget->doAbsoluteRot(view3DWidget->xRot(), view3DWidget->yRot(), view3DWidget->zRot());

//        printf("connect\n");
//        connect(view3DWidget, SIGNAL(xRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
//        connect(view3DWidget, SIGNAL(yRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
//        connect(view3DWidget, SIGNAL(zRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
    }
}

#ifdef USE_EXPERIMENTAL_FEATURES
/**********************************************************************************
* Linked to Vaa3D renderer slider
***********************************************************************************/
void CExplorerWindow::setZoom(int z)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, zoom = %d", titleShort.c_str(), z).c_str(), __itm__current__function__);

    myV3dR_GLWidget::cast(view3DWidget)->setZoomO(z);
}
#endif

/**********************************************************************************
* Syncronizes widgets from <src> to <dst>
***********************************************************************************/
void CExplorerWindow::syncWindows(V3dR_MainWindow* src, V3dR_MainWindow* dst)
{
    /**/itm::debug(itm::LEV1, strprintf("src->title = %s, dst->title = %s",
                                        src->getDataTitle().toStdString().c_str(), dst->getDataTitle().toStdString().c_str()).c_str(), __itm__current__function__);

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

    //propagating skeleton mode and line width
    dst->getGLWidget()->getRenderer()->lineType = src->getGLWidget()->getRenderer()->lineType;
    dst->getGLWidget()->getRenderer()->lineWidth = src->getGLWidget()->getRenderer()->lineWidth;
}

