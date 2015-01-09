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

/******************
*    CHANGELOG    *
*******************
* 2014-11-17. Alessandro. @FIXED "duplicated annotations" bug
* 2014-11-17. Alessandro. @ADDED 'anoV0', ..., 'anoD1' VOI annotation (global) coordinates as object members in order to fix "duplicated annotations" bug
*/

#include "CViewer.h"
#include "v3dr_mainwindow.h"
#include "CVolume.h"
#include "CAnnotations.h"
#include "CImageUtils.h"
#include "COperation.h"
#include "../presentation/PMain.h"
#include "../presentation/PLog.h"
#include "../presentation/PAnoToolBar.h"
#include "../presentation/PDialogProofreading.h"
#include "renderer.h"
#include "renderer_gl1.h"
#include "v3dr_colormapDialog.h"
#include "V3Dsubclasses.h"
#include "QUndoMarkerCreate.h"
#include "QUndoMarkerDelete.h"
#include "QUndoMarkerDeleteROI.h"

using namespace itm;

CViewer* CViewer::first = 0;
CViewer* CViewer::last = 0;
CViewer* CViewer::current = 0;
int CViewer::nInstances = 0;
int CViewer::nTotalInstances = 0;
int CViewer::newViewerOperationID = 0;

void CViewer::show()
{ 
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    PMain* pMain = PMain::getInstance();

    try
    {
        // open tri-view window (and hiding it asap)
        QElapsedTimer timer;
        timer.start();
        this->window = V3D_env->newImageWindow(QString(title.c_str()));
        this->triViewWidget = (XFormWidget*)window;
        triViewWidget->setWindowState(Qt::WindowMinimized);
        Image4DSimple* image = new Image4DSimple();
        image->setFileName(title.c_str());
        image->setData(imgData, volH1-volH0, volV1-volV0, volD1-volD0, nchannels*(volT1-volT0+1), V3D_UINT8);
        image->setTDim(volT1-volT0+1);
        image->setTimePackType(TIME_PACK_C);
        V3D_env->setImage(window, image);

        // open 3D view window
        V3D_env->open3DWindow(window);
        view3DWidget = (V3dR_GLWidget*)(V3D_env->getView3DControl(window));
        if(!view3DWidget->getiDrawExternalParameter())
            QMessageBox::critical(pMain,QObject::tr("Error"), QObject::tr("Unable to get iDrawExternalParameter from Vaa3D's V3dR_GLWidget"),QObject::tr("Ok"));
        window3D = view3DWidget->getiDrawExternalParameter()->window3D;
        if(prev)
            PLog::getInstance()->appendOperation(new NewViewerOperation(QString("Opened view ").append(title.c_str()).toStdString(), itm::GPU, timer.elapsed()));
        else
            PLog::getInstance()->appendOperation(new ImportOperation( "Opened first viewer", itm::GPU, timer.elapsed()));

        // install the event filter on the 3D renderer and on the 3D window
        view3DWidget->installEventFilter(this);
        window3D->installEventFilter(this);
        window3D->timeSlider->installEventFilter(this);

        // time slider: disconnect Vaa3D event handlers and set the complete (virtual) time range
        disconnect(view3DWidget, SIGNAL(changeVolumeTimePoint(int)), window3D->timeSlider, SLOT(setValue(int)));
        disconnect(window3D->timeSlider, SIGNAL(valueChanged(int)), view3DWidget, SLOT(setVolumeTimePoint(int)));
        window3D->timeSlider->setMinimum(0);
        window3D->timeSlider->setMaximum(CImport::instance()->getTDim()-1);

        // if the previous explorer window exists
        if(prev)
        {           
            // apply the same color map only if it differs from the previous one
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

            // 5D data: select the same time frame (if available)
            if(CImport::instance()->is5D())
            {
                int prev_s = prev->window3D->timeSlider->value();
                if(prev_s < volT0 || prev_s > volT1)
                    window3D->timeSlider->setValue(volT0);
                else
                    window3D->timeSlider->setValue(prev_s);
                view3DWidget->setVolumeTimePoint(window3D->timeSlider->value()-volT0);
            }

            //sync widgets
            syncWindows(prev->window3D, window3D);
            PLog::getInstance()->appendOperation(new NewViewerOperation(QString("Syncronized views \"").append(title.c_str()).append("\" and \"").append(prev->title.c_str()).append("\"").toStdString(), itm::GPU, timer.elapsed()));

            //storing annotations done in the previous view and loading annotations of the current view
            prev->storeAnnotations();
            prev->clearAnnotations();
            this->loadAnnotations();
        }
        //otherwise this is the lowest resolution window
        else
        {
            //registrating the current window as the first window of the multiresolution explorer windows chain
            CViewer::first = this;

            //increasing height if lower than the plugin one's
            if(window3D->height() < pMain->height())
                window3D->setMinimumHeight(pMain->height());

            //centering the current 3D window and the plugin's window
            int screen_height = qApp->desktop()->availableGeometry().height();
            int screen_width = qApp->desktop()->availableGeometry().width();
            int window_x = (screen_width - (window3D->width() + pMain->width()))/2;
            int window_y = (screen_height - window3D->height()) / 2;
            window3D->move(window_x, window_y);
        }

        //registrating the current window as the last and current window of the multiresolution explorer windows chain
        CViewer::last = this;
        CViewer::current = this;

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
        if(!pMain->isPRactive())
        {
            pMain->traslYneg->setEnabled(volV0 > 0);
            pMain->traslYpos->setEnabled(volV1 < CImport::instance()->getVolume(volResIndex)->getDIM_V());
            pMain->traslXneg->setEnabled(volH0 > 0);
            pMain->traslXpos->setEnabled(volH1 < CImport::instance()->getVolume(volResIndex)->getDIM_H());
            pMain->traslZneg->setEnabled(volD0 > 0);
            pMain->traslZpos->setEnabled(volD1 < CImport::instance()->getVolume(volResIndex)->getDIM_D());
            pMain->traslTneg->setEnabled(volT0 > 0);
            pMain->traslTpos->setEnabled(volT1 < CImport::instance()->getVolume(volResIndex)->getDIM_T()-1);
        }

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

        if(pMain->frameCoord->isEnabled())
        {
            pMain->T0_sbox->setText(QString::number(volT0));
            pMain->T1_sbox->setText(QString::number(volT1));
        }

        //signal connections
        connect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
        connect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
        connect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
        connect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
        connect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
        connect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
        connect(window3D->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(Vaa3D_changeTSlider(int)));
        //connect(view3DWidget, SIGNAL(xRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        //connect(view3DWidget, SIGNAL(yRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        //connect(view3DWidget, SIGNAL(zRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        connect(pMain->refSys,  SIGNAL(mouseReleased()),   this, SLOT(PMain_rotationchanged()));
        connect(pMain->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
        connect(pMain->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
        connect(pMain->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
        connect(pMain->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
        connect(pMain->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
        connect(pMain->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));

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

        // updating reference system
        if(!pMain->isPRactive())
            pMain->refSys->setDims(volH1-volH0+1, volV1-volV0+1, volD1-volD0+1);
        this->view3DWidget->updateGL();     // if omitted, Vaa3D_rotationchanged somehow resets rotation to 0,0,0
        Vaa3D_rotationchanged(0);

        // saving subvol spinboxes state ---- Alessandro 2013-04-23: not sure if this is really needed
        saveSubvolSpinboxState();

        // refresh annotation toolbar
        PAnoToolBar::instance()->refreshTools();

        // update curve aspect
        pMain->curveAspectChanged();

        // update marker size
        pMain->markersSizeSpinBoxChanged(pMain->markersSizeSpinBox->value());

        // update visible markers
        PAnoToolBar::instance()->buttonMarkerRoiViewChecked(PAnoToolBar::instance()->buttonMarkerRoiView->isChecked());
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(pMain,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        pMain->closeVolume();
    }
    catch(const char* error)
    {
        QMessageBox::critical(pMain,QObject::tr("Error"), QObject::tr(error),QObject::tr("Ok"));
        pMain->closeVolume();
    }
    catch(...)
    {
        QMessageBox::critical(pMain,QObject::tr("Error"), QObject::tr("Unknown error occurred"),QObject::tr("Ok"));
        pMain->closeVolume();
    }
}

CViewer::CViewer(V3DPluginCallback2 *_V3D_env, int _resIndex, itm::uint8 *_imgData, int _volV0, int _volV1,
                                 int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, CViewer *_prev, int _slidingViewerBlockID /* = -1 */): QWidget()
{
    /**/itm::debug(itm::LEV1, strprintf("_resIndex = %d, _V0 = %d, _V1 = %d, _H0 = %d, _H1 = %d, _D0 = %d, _D1 = %d, _T0 = %d, _T1 = %d, _nchannels = %d",
                                        _resIndex, _volV0, _volV1, _volH0, _volH1, _volD0, _volD1, _volT0, _volT1, _nchannels).c_str(), __itm__current__function__);

    //initializations
    ID = nTotalInstances++;
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
    this->volT0 = _volT0;
    this->volT1 = _volT1;
    this->anoV0 = this->anoV1 = this->anoH0 = this->anoH1 = this->anoD0 = this->anoD1 = -1;
    this->nchannels = _nchannels;
    this->toBeClosed = false;
    this->imgData = _imgData;
    this->isReady = false;
    this->waitingFor5D = false;
    this->has_double_clicked = false;
    this->scribbling = false;
    char ctitle[1024];
    sprintf(ctitle, "ID(%d), Res(%d x %d x %d),Volume X=[%d,%d], Y=[%d,%d], Z=[%d,%d], T=[%d,%d], %d channels", ID, CImport::instance()->getVolume(volResIndex)->getDIM_H(),
            CImport::instance()->getVolume(volResIndex)->getDIM_V(), CImport::instance()->getVolume(volResIndex)->getDIM_D(),
            volH0+1, volH1, volV0+1, volV1, volD0+1, volD1, volT0, volT1, nchannels);
    this->title = ctitle;
    sprintf(ctitle, "ID(%d), Res{%d}, Vol{[%d,%d) [%d,%d) [%d,%d) [%d,%d]}", ID, volResIndex, volH0, volH1, volV0, volV1, volD0, volD1, volT0, volT1);
    this->titleShort = ctitle;
    V0_sbox_min = V0_sbox_val = V1_sbox_max = V1_sbox_val =
    H0_sbox_min = H0_sbox_val = H1_sbox_max = H1_sbox_val =
    D0_sbox_min = D0_sbox_val = D1_sbox_max = D1_sbox_val =
    T0_sbox_min = T0_sbox_val = T1_sbox_max = T1_sbox_val = -1;
    slidingViewerBlockID = _slidingViewerBlockID;
    forceZoomIn = false;

    try
    {
        //making prev (if exist) the last view and checking that it belongs to a lower resolution
        if(prev)
        {
            prev->makeLastView();
            if(prev->volResIndex > volResIndex)
                throw RuntimeException("in CViewer(): attempting to break the ascending order of resolution history. This feature is not supported yet.");
        }

        //check that the number of instantiated objects does not exceed the number of available resolutions
        nInstances++;
        /**/itm::debug(itm::LEV3, strprintf("nInstances++, nInstances = %d", nInstances).c_str(), __itm__current__function__);
        if(nInstances > CImport::instance()->getResolutions() +1)
            throw RuntimeException(QString("in CViewer(): exceeded the maximum number of views opened at the same time.\n\nPlease signal this issue to developers.").toStdString().c_str());


        //deactivating previous window and activating the current one
        if(prev)
        {
            prev->view3DWidget->removeEventFilter(prev);
            prev->window3D->removeEventFilter(prev);
            prev->window3D->timeSlider->removeEventFilter(this);
            prev->resetZoomHistory();
            prev->setActive(false);
        }
        setActive(true);
    }
    catch(RuntimeException &ex)
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

CViewer::~CViewer()
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

    // remove the event filter from the 3D renderer and from the 3D window
    isActive = false;
    view3DWidget->removeEventFilter(this);
    window3D->removeEventFilter(this);
    window3D->timeSlider->removeEventFilter(this);

    // CLOSE 3D window (solution #1)
//    if(!CImport::instance()->is5D())
//        V3D_env->close3DWindow(window); //this causes crash on 5D data when scrolling time slider, but is OK in all the other cases

    // CLOSE 3D window (solution #2)
    // view3DWidget->close();          //this causes crash when makeLastView is called on a very long chain of opened windows
    // window3D->postClose();

    // CLOSE 3D window (solution #3)
    // @fixed  by Alessandro on 2014-04-11: this seems the only way to close the 3D window w/o (randomly) causing TeraFly to crash
    // @update by Alessandro on 2014-07-15: this causes random crash in "Proofreading" mode, but is ok on 5D data (even in "Proofediting mode"!)
    // @update by Alessandro on 2014-07-21: this ALWAYS works on Windows. Still has to be tested on other platforms.
//    else
        POST_EVENT(window3D, QEvent::Close); // this OK

    //close 2D window
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
bool CViewer::eventFilter(QObject *object, QEvent *event)
{
    try
    {
        //ignoring all events when window is not active
        if(!isActive)
        {
            //printf("Ignoring event from CViewer[%s] cause it's not active\n", title.c_str());
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
//                next->restoreViewerFrom(this);
//                event->ignore();
//                return true;
//            }
//            else
            if(prev                                                                    &&  //the previous resolution exists
               !toBeClosed                                                             &&  //the current resolution does not have to be closed
               isZoomDerivativeNeg()                                                   &&  //zoom derivative is negative
               view3DWidget->zoom() < PMain::getInstance()->zoomOutSens->value())          //zoom-out threshold reached
            {
                // if window is not ready for "switch view" events, reset zoom-out and ignore this event
                if(!isReady)
                {
                    resetZoomHistory();
                    return false;
                }
                else
                {
                    setActive(false);
                    resetZoomHistory();
                    prev->restoreViewerFrom(this);
                    return true;
                }
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
        #else
        if ((object == view3DWidget || object == window3D) && event->type() == QEvent::Wheel)
        {
            QWheelEvent* wheelEvt = (QWheelEvent*)event;
            int zr = CLAMP(-ZOOM_RANGE, ZOOM_RANGE, view3DWidget->zoom());
            if(zr > PMain::getInstance()->zoomInSens->value() && !isHighestRes())
            {
                static_cast<Renderer_gl1*>(view3DWidget->getRenderer())->selectMode = Renderer::smMarkerCreate1;
                static_cast<Renderer_gl1*>(view3DWidget->getRenderer())->b_addthismarker = false;
                static_cast<Renderer_gl1*>(view3DWidget->getRenderer())->b_imaging = false;
                static_cast<Renderer_gl1*>(view3DWidget->getRenderer())->b_grabhighrez = true;
                forceZoomIn = true;
                view3DWidget->getRenderer()->hitPoint(wheelEvt->x(), wheelEvt->y());
            }
        }
        #endif

        /****************** INTERCEPTING MOUSE CLICK EVENTS ***********************
        Mouse click events are intercepted for handling annotations tools
        ***************************************************************************/
        if (object == view3DWidget && event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* mouseEvt = (QMouseEvent*)event;
            if(mouseEvt->button() == Qt::RightButton && PAnoToolBar::instance()->buttonMarkerDelete->isChecked())
            {
                deleteMarkerAt(mouseEvt->x(), mouseEvt->y());
                return true;
            }
            else if(mouseEvt->button() == Qt::RightButton && PAnoToolBar::instance()->buttonMarkerCreate->isChecked())
            {
                createMarkerAt(mouseEvt->x(), mouseEvt->y());
                return true;
            }
            else if(mouseEvt->button() == Qt::RightButton && PAnoToolBar::instance()->buttonMarkerRoiDelete->isChecked())
            {
                scribbling = true;
                scribbling_points.clear();
            }

            // ignore Vaa3D-right-click-popup marker create operations
            else if(mouseEvt->button() == Qt::RightButton)
            {
                view3DWidget->getRenderer()->hitPoint(mouseEvt->x(), mouseEvt->y());
                Renderer::SelectMode mode = view3DWidget->getRenderer()->selectMode;
                bool addMarker = static_cast<Renderer_gl1*>(view3DWidget->getRenderer())->b_addthismarker;

                if((mode == Renderer::smMarkerCreate1 && addMarker) ||
                   (mode == Renderer::smMarkerCreate2 && addMarker) ||
                   (mode == Renderer::smMarkerCreate3 && addMarker) ||
                    mode == Renderer::smMarkerCreate1Curve)
                {
                    QMessageBox::information(0, "This feature has been disabled", "This feature has been disabled by the developers of TeraFly.\n\n"
                                             "Please use the TeraFly's annotation toolbar to perform this action.");
                    PAnoToolBar::instance()->buttonMarkerCreateChecked(false);
                }



                event->ignore();
                return true;
            }

            return false;
        }

        /******************* INTERCEPTING MOUSE MOVE EVENTS ***********************
        Mouse move events are intercepted for handling annotation tools
        ***************************************************************************/
        if (scribbling &&                                                               // scribbling is active
            object == view3DWidget &&                                                   // event emitted by 3D renderer
            event->type() == QEvent::MouseMove &&                                       // MouseMove event
            PAnoToolBar::instance()->buttonMarkerRoiDelete->isChecked())                // annotation tool "Marker ROI delete" enabled
        {

            QMouseEvent* mouseEvt = (QMouseEvent*)event;
            scribbling_points.push_back(mouseEvt->pos());

            return false;
        }


        /****************** INTERCEPTING MOUSE RELEASE EVENTS **********************
        Mouse release events are intercepted for...
        ***************************************************************************/
        if (object == view3DWidget && event->type() == QEvent::MouseButtonRelease)
        {
            // ...handling annotation tools...
            if(scribbling &&
               PAnoToolBar::instance()->buttonMarkerRoiDelete->isChecked())
                scribbling = false;
            // ...and emitting a Vaa3D rotation changed event
            else
                this->Vaa3D_rotationchanged(0);
            return false;
        }

        /****************** INTERCEPTING DOUBLE CLICK EVENTS ***********************
        Double click events are intercepted to switch to the higher resolution.
        ***************************************************************************/
        if (object == view3DWidget && event->type() == QEvent::MouseButtonDblClick)
        {
            if(PMain::getInstance()->isPRactive())
            {
                QMessageBox::information(this->window3D, "Warning", "TeraFly is running in \"Proofreading\" mode. All TeraFly's' navigation features are disabled. "
                                         "Please terminate the \"Proofreading\" mode and try again.");
                return true;
            }

            // set double click flag
            has_double_clicked = true;

            QMouseEvent* mouseEvt = (QMouseEvent*)event;

            #ifdef USE_EXPERIMENTAL_FEATURES
            XYZ point = getRenderer3DPoint(mouseEvt->x(), mouseEvt->y());
            newViewer(point.x, point.y, point.z, volResIndex+1, volT0, volT1);
            #else
            static_cast<Renderer_gl1*>(view3DWidget->getRenderer())->selectMode = Renderer::smMarkerCreate1;
            static_cast<Renderer_gl1*>(view3DWidget->getRenderer())->b_addthismarker = false;
            static_cast<Renderer_gl1*>(view3DWidget->getRenderer())->b_imaging = false;
            static_cast<Renderer_gl1*>(view3DWidget->getRenderer())->b_grabhighrez = true;
            forceZoomIn = true;
            view3DWidget->getRenderer()->hitPoint(mouseEvt->x(), mouseEvt->y());
            #endif
            return true;
        }

        /******************** INTERCEPTING KEYPRESS EVENTS ************************
        Double click events are intercepted to switch to the higher resolution.
        ***************************************************************************/
        if (object == window3D && event->type() == 6)
        {
            QKeyEvent* key_evt = (QKeyEvent*)event;
            if(key_evt->key() == Qt::Key_Return)
                PMain::getInstance()->PRblockSpinboxEditingFinished();
        }


        /****************** INTERCEPTING TIME SLIDER EVENTS ************************
        Time slider events are intercepted to navigate through the entire time range
        ***************************************************************************/
        if (object == window3D->timeSlider &&                       // event from Vaa3D time slider
                ( event->type() == QEvent::MouseButtonRelease ||    // mouse release event
                  event->type() == QEvent::Wheel))                  // mouse scroll event
        {
            Vaa3D_changeTSlider(window3D->timeSlider->value(), true);
            return false;
        }

        /***************** INTERCEPTING WINDOW CLOSE EVENTS ***********************
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

        /************ INTERCEPTING WINDOW MOVING/RESIZING EVENTS ******************
        Window moving and resizing events  are intercepted  to let PMain's position
        be syncronized with the explorer.
        ***************************************************************************/
        else if(object == window3D && (event->type() == QEvent::Move || event->type() == QEvent::Resize))
        {
           alignToLeft(PMain::getInstance());
           PAnoToolBar::instance()->alignToLeft(window3D->glWidgetArea);
           return true;
        }

        /************* INTERCEPTING WINDOW STATE CHANGES EVENTS *******************
        Window state changes events are intercepted to let PMain's position be syn-
        cronized with the explorer.
        ***************************************************************************/
        else if(object == window3D && event->type() == QEvent::WindowStateChange)
        {
            alignToLeft(PMain::getInstance());
            PAnoToolBar::instance()->alignToLeft(window3D->glWidgetArea);
            return true;
        }

        return false;
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(PMain::getInstance(),QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        return false;
    }
}

/*********************************************************************************
* Receive data (and metadata) from <CVolume> throughout the loading process
**********************************************************************************/
void CViewer::receiveData(
        itm::uint8* data,                               // data (any dimension)
        integer_array data_s,                           // data start coordinates along X, Y, Z, C, t
        integer_array data_c,                           // data count along X, Y, Z, C, t
        QWidget *dest,                                  // address of the listener
        bool finished,                                  // whether the loading operation is terminated
        itm::RuntimeException* ex /* = 0*/,             // exception (optional)
        qint64 elapsed_time       /* = 0 */,            // elapsed time (optional)
        QString op_dsc            /* = ""*/,            // operation descriptor (optional)
        int step                  /* = 0 */)            // step number (optional)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s, data_s = {%s}, data_c = {%s}, finished = %s",
                                        titleShort.c_str(),
                                        !data_s.empty() ? strprintf("%d,%d,%d,%d,%d", data_s[0], data_s[1], data_s[2], data_s[3], data_s[4]).c_str() : "",
                                        !data_c.empty() ? strprintf("%d,%d,%d,%d,%d", data_c[0], data_c[1], data_c[2], data_c[3], data_c[4]).c_str() : "",
                                        finished ? "true" : "false").c_str(), __itm__current__function__);

    char message[1000];
    CVolume* cVolume = CVolume::instance();

    //if an exception has occurred, showing a message error
    if(ex)
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
    else if(dest == this)
    {
        try
        {
            //first updating IO time
            PLog::getInstance()->appendOperation(new NewViewerOperation(op_dsc.toStdString(), itm::IO, elapsed_time));

            //copying loaded data
            QElapsedTimer timer;
            timer.start();
            uint32 img_dims[5]       = {volH1-volH0,        volV1-volV0,        volD1-volD0,        nchannels,  volT1-volT0+1};
            uint32 img_offset[5]     = {data_s[0]-volH0,    data_s[1]-volV0,    data_s[2]-volD0,    0,          data_s[4]-volT0 };
            uint32 new_img_dims[5]   = {data_c[0],          data_c[1],          data_c[2],          data_c[3],  data_c[4]       };
            uint32 new_img_offset[5] = {0,                  0,                  0,                  0,          0               };
            uint32 new_img_count[5]  = {data_c[0],          data_c[1],          data_c[2],          data_c[3],  data_c[4]       };
            CImageUtils::copyVOI(data, new_img_dims, new_img_offset, new_img_count,
                    view3DWidget->getiDrawExternalParameter()->image4d->getRawData(), img_dims, img_offset);
            qint64 elapsedTime = timer.elapsed();

            // if 5D data, update selected time frame
            if(CImport::instance()->is5D())
                view3DWidget->setVolumeTimePoint(window3D->timeSlider->value()-volT0);

            //updating log
            sprintf(message, "Streaming %d/%d: Copied block X=[%d, %d) Y=[%d, %d) Z=[%d, %d) T=[%d, %d]  to resolution %d",
                              step, cVolume->getStreamingSteps(), cVolume->getVoiH0(), cVolume->getVoiH1(), cVolume->getVoiV0(), cVolume->getVoiV1(),
                              cVolume->getVoiD0(), cVolume->getVoiD1(), cVolume->getVoiT0(), cVolume->getVoiT1(), cVolume->getVoiResIndex());
            PLog::getInstance()->appendOperation(new NewViewerOperation(message, itm::CPU, elapsedTime));

            //releasing memory if streaming is not active
            if(cVolume->getStreamingSteps() == 1)
                delete[] data;

            //updating image data
            /**/itm::debug(itm::LEV1, strprintf("title = %s: update image data", titleShort.c_str()).c_str(), __itm__current__function__);
//            /**/itm::debug(itm::LEV3, "Waiting for updateGraphicsInProgress mutex", __itm__current__function__);
//            /**/ updateGraphicsInProgress.lock();
//            /**/itm::debug(itm::LEV3, "Access granted from updateGraphicsInProgress mutex", __itm__current__function__);
            timer.restart();
            view3DWidget->updateImageData();
            sprintf(message, "Streaming %d/%d: Block X=[%d, %d) Y=[%d, %d) Z=[%d, %d) T=[%d, %d] rendered into view %s",
                    step, cVolume->getStreamingSteps(), cVolume->getVoiH0(), cVolume->getVoiH1(),
                    cVolume->getVoiV0(), cVolume->getVoiV1(),
                    cVolume->getVoiD0(), cVolume->getVoiD1(),
                    cVolume->getVoiT0(), cVolume->getVoiT1(), title.c_str());
            PLog::getInstance()->appendOperation(new NewViewerOperation(message, itm::GPU, timer.elapsed()));
//            /**/itm::debug(itm::LEV3, strprintf("updateGraphicsInProgress.unlock()").c_str(), __itm__current__function__);
//            /**/ updateGraphicsInProgress.unlock();
//            /**/itm::debug(itm::LEV1, strprintf("title = %s: image updated successfully", titleShort.c_str()).c_str(), __itm__current__function__);

            //operations to be performed when all image data have been loaded
            if(finished)
            {
                // disconnect from data producer
                disconnect(CVolume::instance(), SIGNAL(sendData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bool,itm::RuntimeException*,qint64,QString,int)), this, SLOT(receiveData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bool,itm::RuntimeException*,qint64,QString,int)));

                // reset TeraFly's GUI
                PMain::getInstance()->resetGUI();

                // exit from "waiting for 5D data" state, if previously set
                this->setWaitingFor5D(false);

                // reset the cursor
                window3D->setCursor(Qt::ArrowCursor);
                view3DWidget->setCursor(Qt::ArrowCursor);


                //---- Alessandro 2013-09-28 fixed: processing pending events related trasl* buttons (previously deactivated) prevents the user from
                //                                  triggering multiple traslations at the same time.
                //---- Alessandro 2014-01-26 fixed: processEvents() is no longer needed, since the trasl* button slots have been made safer and do not
                //                                  trigger any action when the the current window is not active (or has to be closed)
                //QApplication::processEvents();
                /**/itm::debug(itm::LEV3, strprintf("title = %s: reactivating directional shifts", titleShort.c_str()).c_str(), __itm__current__function__);
                PMain::getInstance()->traslXneg->setActive(true);
                PMain::getInstance()->traslXpos->setActive(true);
                PMain::getInstance()->traslYneg->setActive(true);
                PMain::getInstance()->traslYpos->setActive(true);
                PMain::getInstance()->traslZneg->setActive(true);
                PMain::getInstance()->traslZpos->setActive(true);
                PMain::getInstance()->traslTneg->setActive(true);
                PMain::getInstance()->traslTpos->setActive(true);
                /**/itm::debug(itm::LEV3, strprintf("title = %s: directional shifts successfully reactivated", titleShort.c_str()).c_str(), __itm__current__function__);

                //current window is now ready for user input
                isReady = true;

                //saving elapsed time to log
                if(prev)
                {
                    /**/itm::debug(itm::LEV3, strprintf("title = %s: saving elapsed time to log", titleShort.c_str()).c_str(), __itm__current__function__);
                    sprintf(message, "Successfully generated view %s", title.c_str());
                    PLog::getInstance()->appendOperation(new NewViewerOperation(message, itm::ALL_COMPS, prev->newViewerTimer.elapsed()));
                }

                // refresh annotation toolbar
                PAnoToolBar::instance()->refreshTools();
            }
        }
        catch(RuntimeException &ex)
        {
            QMessageBox::critical(PMain::getInstance(),QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
            PMain::getInstance()->resetGUI();
            isReady = true;
        }
    }
//    QMessageBox::information(0, "Stop", "Wait...");
    /**/itm::debug(itm::LEV3, "method terminated", __itm__current__function__);
}

/**********************************************************************************
* Generates a new view using the given coordinates.
* Called by the current <CViewer> when the user zooms in and the higher res-
* lution has to be loaded.
***********************************************************************************/
void
CViewer::newViewer(int x, int y, int z,                            //can be either the VOI's center (default) or the VOI's ending point (see x0,y0,z0)
    int resolution,                                 //resolution index of the view requested
    int t0, int t1,                                 //time frames selection
    bool fromVaa3Dcoordinates /*= false*/,          //if coordinates were obtained from Vaa3D
    int dx/*=-1*/, int dy/*=-1*/, int dz/*=-1*/,    //VOI [x-dx,x+dx), [y-dy,y+dy), [z-dz,z+dz), [t0, t1]
    int x0/*=-1*/, int y0/*=-1*/, int z0/*=-1*/,    //VOI [x0, x), [y0, y), [z0, z), [t0, t1]
    bool auto_crop /* = true */,                    //whether to crop the VOI to the max dims
    bool scale_coords /* = true */,                 //whether to scale VOI coords to the target res
    int sliding_viewer_block_ID /* = -1 */)         //block ID in "Sliding viewer" mode

{
    /**/itm::debug(itm::LEV1, strprintf("title = %s, x = %d, y = %d, z = %d, res = %d, dx = %d, dy = %d, dz = %d, x0 = %d, y0 = %d, z0 = %d, t0 = %d, t1 = %d, auto_crop = %s, scale_coords = %s, sliding_viewer_block_ID = %d",
                                        titleShort.c_str(),  x, y, z, resolution, dx, dy, dz, x0, y0, z0, t0, t1, auto_crop ? "true" : "false", scale_coords ? "true" : "false", sliding_viewer_block_ID).c_str(), __itm__current__function__);

    // check precondition #1: active window
    if(!isActive || toBeClosed)
    {
        QMessageBox::warning(0, "Unexpected behaviour", "Precondition check \"!isActive || toBeClosed\" failed. Please contact the developers");
        return;
    }

    // check precondition #2: valid resolution
    if(resolution >= CImport::instance()->getResolutions())
        resolution = volResIndex;

    // check precondition #3: window ready for "newView" events
    if( !isReady )
    {
        itm::warning("precondition (!isReady) not met. Aborting newView", __itm__current__function__);
        return;
    }


    // deactivate current window and processing all pending events
    setActive(false);
    QApplication::processEvents();

    // after processEvents(), it might be that this windows is no longer valid, then terminating
    if(toBeClosed)
        return;

    // restart timer (measures the time needed to switch to a new view)
    newViewerTimer.restart();

    // create new macro-group for NewViewerOperation
    itm::NewViewerOperation::newGroup();


    try
    {
        // set GUI to waiting state
        PMain& pMain = *(PMain::getInstance());
        pMain.progressBar->setEnabled(true);
        pMain.progressBar->setMinimum(0);
        pMain.progressBar->setMaximum(0);
        pMain.statusBar->showMessage("Switching view...");
        view3DWidget->setCursor(Qt::BusyCursor);
        window3D->setCursor(Qt::BusyCursor);
        pMain.setCursor(Qt::BusyCursor);

        // scale VOI coordinates to the reference system of the target resolution
        if(scale_coords)
        {
            float ratioX = static_cast<float>(CImport::instance()->getVolume(resolution)->getDIM_H())/CImport::instance()->getVolume(volResIndex)->getDIM_H();
            float ratioY = static_cast<float>(CImport::instance()->getVolume(resolution)->getDIM_V())/CImport::instance()->getVolume(volResIndex)->getDIM_V();
            float ratioZ = static_cast<float>(CImport::instance()->getVolume(resolution)->getDIM_D())/CImport::instance()->getVolume(volResIndex)->getDIM_D();
            x = getGlobalHCoord(x, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
            y = getGlobalVCoord(y, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
            z = getGlobalDCoord(z, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
            if(x0 != -1)
                x0 = getGlobalHCoord(x0, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
            else
                dx = dx == -1 ? int_inf : static_cast<int>(dx*ratioX+0.5f);
            if(y0 != -1)
                y0 = getGlobalVCoord(y0, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
            else
                dy = dy == -1 ? int_inf : static_cast<int>(dy*ratioY+0.5f);
            if(z0 != -1)
                z0 = getGlobalDCoord(z0, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
            else
                dz = dz == -1 ? int_inf : static_cast<int>(dz*ratioZ+0.5f);
        }

        // adjust time size so as to use all the available frames set by the user
        if(CImport::instance()->is5D() && ((t1 - t0 +1) != pMain.Tdim_sbox->value()))
        {
            t1 = t0 + (pMain.Tdim_sbox->value()-1);
            /**/itm::debug(itm::LEV1, strprintf("mismatch between |[t0,t1]| (%d) and max T dims (%d), adjusting it to [%d,%d]", t1-t0+1, pMain.Tdim_sbox->value(), t0, t1).c_str(), __itm__current__function__);
        }


        // crop VOI if its larger than the maximum allowed
        if(auto_crop)
        {
            // modality #1: VOI = [x-dx,x+dx), [y-dy,y+dy), [z-dz,z+dz), [t0, t1]
            if(dx != -1 && dy != -1 && dz != -1)
            {
                /**/itm::debug(itm::LEV3, strprintf("title = %s, cropping bbox dims from (%d,%d,%d) t[%d,%d] to...", titleShort.c_str(),  dx, dy, dz, t0, t1).c_str(), __itm__current__function__);
                dx = std::min(dx, round(pMain.Hdim_sbox->value()/2.0f));
                dy = std::min(dy, round(pMain.Vdim_sbox->value()/2.0f));
                dz = std::min(dz, round(pMain.Ddim_sbox->value()/2.0f));
                t0 = std::max(0, std::min(t0,CImport::instance()->getVolume(volResIndex)->getDIM_T()-1));
                t1 = std::max(0, std::min(t1,CImport::instance()->getVolume(volResIndex)->getDIM_T()-1));
                if(CImport::instance()->is5D() && (t1-t0+1 > pMain.Tdim_sbox->value()))
                    t1 = t0 + pMain.Tdim_sbox->value();
                if(CImport::instance()->is5D() && (t1 >= CImport::instance()->getTDim()-1))
                    t0 = t1 - (pMain.Tdim_sbox->value()-1);
                if(CImport::instance()->is5D() && (t0 == 0))
                    t1 = pMain.Tdim_sbox->value()-1;
                /**/itm::debug(itm::LEV3, strprintf("title = %s, ...to (%d,%d,%d)", titleShort.c_str(),  dx, dy, dz).c_str(), __itm__current__function__);
            }
            // modality #2: VOI = [x0, x), [y0, y), [z0, z), [t0, t1]
            else
            {
                /**/itm::debug(itm::LEV3, strprintf("title = %s, cropping bbox dims from [%d,%d) [%d,%d) [%d,%d) [%d,%d] to...", titleShort.c_str(),  x0, x, y0, y, z0, z, t0, t1).c_str(), __itm__current__function__);
                if(x - x0 > pMain.Hdim_sbox->value())
                {
                    float margin = ( (x - x0) - pMain.Hdim_sbox->value() )/2.0f ;
                    x  = round(x  - margin);
                    x0 = round(x0 + margin);
                }
                if(y - y0 > pMain.Vdim_sbox->value())
                {
                    float margin = ( (y - y0) - pMain.Vdim_sbox->value() )/2.0f ;
                    y  = round(y  - margin);
                    y0 = round(y0 + margin);
                }
                if(z - z0 > pMain.Ddim_sbox->value())
                {
                    float margin = ( (z - z0) - pMain.Ddim_sbox->value() )/2.0f ;
                    z  = round(z  - margin);
                    z0 = round(z0 + margin);
                }
                t0 = std::max(0, std::min(t0,CImport::instance()->getVolume(volResIndex)->getDIM_T()-1));
                t1 = std::max(0, std::min(t1,CImport::instance()->getVolume(volResIndex)->getDIM_T()-1));
                if(CImport::instance()->is5D() && (t1-t0+1 > pMain.Tdim_sbox->value()))
                    t1 = t0 + pMain.Tdim_sbox->value();
                if(CImport::instance()->is5D() && (t1 >= CImport::instance()->getTDim()-1))
                    t0 = t1 - (pMain.Tdim_sbox->value()-1);
                if(CImport::instance()->is5D() && (t0 == 0))
                    t1 = pMain.Tdim_sbox->value()-1;
                /**/itm::debug(itm::LEV3, strprintf("title = %s, ...to [%d,%d) [%d,%d) [%d,%d) [%d,%d]", titleShort.c_str(),  x0, x, y0, y, z0, z, t0, t1).c_str(), __itm__current__function__);
            }
        }



        // ask CVolume to check (and correct) for a valid VOI
        CVolume* cVolume = CVolume::instance();
        try
        {
            if(dx != -1 && dy != -1 && dz != -1)
                cVolume->setVoi(0, resolution, y-dy, y+dy, x-dx, x+dx, z-dz, z+dz, t0, t1);
            else
                cVolume->setVoi(0, resolution, y0, y, x0, x, z0, z, t0, t1);
        }
        catch(RuntimeException &ex)
        {
            /**/itm::warning(strprintf("Exception thrown when setting VOI: \"%s\". Aborting newView", ex.what()).c_str(), __itm__current__function__);

            setActive(true);
            view3DWidget->setCursor(Qt::ArrowCursor);
            window3D->setCursor(Qt::ArrowCursor);
            PMain::getInstance()->resetGUI();
            return;
        }


        // save the state of PMain GUI VOI's widgets
        saveSubvolSpinboxState();


        // disconnect current window from GUI's listeners and event filters
        disconnect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
        disconnect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
        disconnect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
        disconnect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
        disconnect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
        disconnect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
        disconnect(window3D->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(Vaa3D_changeTSlider(int)));
        //disconnect(view3DWidget, SIGNAL(xRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        //disconnect(view3DWidget, SIGNAL(yRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        //disconnect(view3DWidget, SIGNAL(zRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        disconnect(PMain::getInstance()->refSys,  SIGNAL(mouseReleased()),   this, SLOT(PMain_rotationchanged()));
        disconnect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
        disconnect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
        disconnect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
        disconnect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
        disconnect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
        disconnect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));
        view3DWidget->removeEventFilter(this);
        window3D->removeEventFilter(this);
        window3D->timeSlider->removeEventFilter(this);



        //obtaining low res data from current window to be displayed in a new window while the user waits for the new high res data
        QElapsedTimer timer;
        timer.start();
        int voiH0m=0, voiH1m=0, voiV0m=0, voiV1m=0,voiD0m=0, voiD1m=0, voiT0m=0, voiT1m=0;
        int rVoiH0 = CVolume::scaleHCoord(cVolume->getVoiH0(), resolution, volResIndex);
        int rVoiH1 = CVolume::scaleHCoord(cVolume->getVoiH1(), resolution, volResIndex);
        int rVoiV0 = CVolume::scaleVCoord(cVolume->getVoiV0(), resolution, volResIndex);
        int rVoiV1 = CVolume::scaleVCoord(cVolume->getVoiV1(), resolution, volResIndex);
        int rVoiD0 = CVolume::scaleDCoord(cVolume->getVoiD0(), resolution, volResIndex);
        int rVoiD1 = CVolume::scaleDCoord(cVolume->getVoiD1(), resolution, volResIndex);
        uint8* lowresData = getVOI(rVoiH0, rVoiH1, rVoiV0, rVoiV1, rVoiD0, rVoiD1, cVolume->getVoiT0(), cVolume->getVoiT1(),
                                   cVolume->getVoiH1()-cVolume->getVoiH0(),
                                   cVolume->getVoiV1()-cVolume->getVoiV0(),
                                   cVolume->getVoiD1()-cVolume->getVoiD0(),
                                   voiH0m, voiH1m, voiV0m, voiV1m,voiD0m, voiD1m, voiT0m, voiT1m);
        char message[1000];
        sprintf(message, "Block X=[%d, %d) Y=[%d, %d) Z=[%d, %d) T[%d, %d] loaded from view %s, black-filled region is "
                               "X=[%d, %d) Y=[%d, %d) Z=[%d, %d) T[%d, %d]",
                rVoiH0, rVoiH1, rVoiV0, rVoiV1, rVoiD0, rVoiD1, cVolume->getVoiT0(), cVolume->getVoiT1(), title.c_str(),
                voiH0m, voiH1m, voiV0m, voiV1m,voiD0m, voiD1m, voiT0m, voiT1m);
        PLog::getInstance()->appendOperation(new NewViewerOperation(message, itm::CPU, timer.elapsed()));

        //creating a new window
        this->next = new CViewer(V3D_env, resolution, lowresData,
                                         cVolume->getVoiV0(), cVolume->getVoiV1(), cVolume->getVoiH0(), cVolume->getVoiH1(), cVolume->getVoiD0(), cVolume->getVoiD1(),
                                         cVolume->getVoiT0(), cVolume->getVoiT1(), nchannels, this, sliding_viewer_block_ID);

        // connect new window to data producer
        connect(CVolume::instance(), SIGNAL(sendData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bool,itm::RuntimeException*,qint64,QString,int)), next, SLOT(receiveData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bool,itm::RuntimeException*,qint64,QString,int)), Qt::QueuedConnection);

        // update CVolume with the request of the actual missing VOI along t and the current selected frame
        cVolume->setVoiT(voiT0m, voiT1m, window3D->timeSlider->value());
        cVolume->setSource(next);
        cVolume->setStreamingSteps(PMain::getInstance()->debugStreamingStepsSBox->value());


// lock updateGraphicsInProgress mutex on this thread (i.e. the GUI thread or main queue event thread)
/**/itm::debug(itm::LEV3, strprintf("Waiting for updateGraphicsInProgress mutex").c_str(), __itm__current__function__);
/**/ updateGraphicsInProgress.lock();
/**/itm::debug(itm::LEV3, strprintf("Access granted from updateGraphicsInProgress mutex").c_str(), __itm__current__function__);

        // update status bar message
        pMain.statusBar->showMessage("Loading image data...");

        // load new data in a separate thread. When done, the "receiveData" method of the new window will be called
        cVolume->start();

        // meanwhile, show the new window with preview data
        next->show();

        // enter "waiting for 5D data" state, if possible
        next->setWaitingFor5D(true);

        //if the resolution of the loaded voi is the same of the current one, this window will be closed
        if(resolution == volResIndex)
        {
            /**/itm::debug(itm::LEV3, strprintf("object \"%s\" is going to be destroyed", titleShort.c_str()).c_str(), __itm__current__function__);

            if(prev)
            {
                prev->newViewerTimer = newViewerTimer;
                prev->next = next;
                next->prev = prev;
            }
            else
            {
                next->prev = 0;
                CViewer::first = next;
            }

            this->toBeClosed = true;
            delete this;
        }

// unlock updateGraphicsInProgress mutex
/**/itm::debug(itm::LEV3, strprintf("updateGraphicsInProgress.unlock()").c_str(), __itm__current__function__);
/**/ updateGraphicsInProgress.unlock();

    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::getInstance()->resetGUI();
    }
}

/**********************************************************************************
* Resizes  the  given image subvolume in a  newly allocated array using the fastest
* achievable scaling method. The image currently shown is used as data source.
***********************************************************************************/
itm::uint8* CViewer::getVOI(int x0, int x1,              // VOI [x0, x1) in the local reference sys
                               int y0, int y1,              // VOI [y0, y1) in the local reference sys
                               int z0, int z1,              // VOI [z0, z1) in the local reference sys
                               int t0, int t1,              // VOI [t0, t1] in the local reference sys
                               int xDimInterp,              // interpolated VOI dimension along X
                               int yDimInterp,              // interpolated VOI dimension along Y
                               int zDimInterp,              // interpolated VOI dimension along Z
                               int& x0m, int& x1m,          // black-filled VOI [x0m, x1m) in the local rfsys
                               int& y0m, int& y1m,          // black-filled VOI [y0m, y1m) in the local rfsys
                               int& z0m, int& z1m,          // black-filled VOI [z0m, z1m) in the local rfsys
                               int& t0m, int& t1m)          // black-filled VOI [t0m, t1m] in the local rfsys
throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s, x0 = %d, x1 = %d, y0 = %d, y1 = %d, z0 = %d, z1 = %d, t0 = %d, t1 = %d, xDim = %d, yDim = %d, zDim = %d",
                                        titleShort.c_str(), x0, x1, y0, y1, z0, z1, t0, t1, xDimInterp, yDimInterp, zDimInterp).c_str(), __itm__current__function__);

    // allocate image data and initializing to zero (black)
    /**/itm::debug(itm::LEV3, "Allocate image data", __itm__current__function__);
    size_t img_dim = static_cast<size_t>(xDimInterp) * yDimInterp * zDimInterp * nchannels * (t1-t0+1);
    uint8* img = new uint8[img_dim];
    for(uint8* img_p = img; img_p-img < img_dim; img_p++)
        *img_p=0;



    // compute actual VOI that can be copied, i.e. that intersects with the image currently displayed
    /**/itm::debug(itm::LEV3, "Compute intersection VOI", __itm__current__function__);
    QRect XRectDisplayed(QPoint(volH0, 0), QPoint(volH1, 1));
    QRect YRectDisplayed(QPoint(volV0, 0), QPoint(volV1, 1));
    QRect ZRectDisplayed(QPoint(volD0, 0), QPoint(volD1, 1));
    QRect TRectDisplayed(QPoint(volT0, 0), QPoint(volT1, 1));
    QRect XRectVOI(QPoint(x0, 0), QPoint(x1, 1));
    QRect YRectVOI(QPoint(y0, 0), QPoint(y1, 1));
    QRect ZRectVOI(QPoint(z0, 0), QPoint(z1, 1));
    QRect TRectVOI(QPoint(t0, 0), QPoint(t1, 1));
    QRect XRectIntersect = XRectDisplayed.intersected(XRectVOI);
    QRect YRectIntersect = YRectDisplayed.intersected(YRectVOI);
    QRect ZRectIntersect = ZRectDisplayed.intersected(ZRectVOI);
    QRect TRectIntersect = TRectDisplayed.intersected(TRectVOI);
    int x0a = XRectIntersect.left();
    int x1a = XRectIntersect.right();
    int y0a = YRectIntersect.left();
    int y1a = YRectIntersect.right();
    int z0a = ZRectIntersect.left();
    int z1a = ZRectIntersect.right();
    int t0a = TRectIntersect.left();
    int t1a = TRectIntersect.right();
    /**/itm::debug(itm::LEV3, strprintf("title = %s, available voi is [%d, %d) [%d, %d) [%d, %d) [%d, %d]",
                                        titleShort.c_str(), x0a, x1a, y0a, y1a, z0a, z1a, t0a, t1a).c_str(), __itm__current__function__);



    // compute missing VOI. If copyable VOI is empty, returning the black-initialized image
    if(x1a - x0a <= 0 || y1a - y0a <= 0 || z1a - z0a <= 0 || t1a - t0a < 0)
    {
        x0m = x0;
        x1m = x1;
        y0m = y0;
        y1m = y1;
        z0m = z0;
        z1m = z1;
        t0m = t0;
        t1m = t1;
        return img;
    }
    else
    {
        x0m = x0;       // not yet supported (@TODO)
        x1m = x1;       // not yet supported (@TODO)
        y0m = y0;       // not yet supported (@TODO)
        y1m = y1;       // not yet supported (@TODO)
        z0m = z0;       // not yet supported (@TODO)
        z1m = z1;       // not yet supported (@TODO)

        // if all data is available along XYZ, trying to speed up loading along T by requesting only the missing frames
        if( x0 == x0a && x1 == x1a &&
            y0 == y0a && y1 == y1a &&
            z0 == z0a && z1 == z1a)
        {
            // check for intersection contained in the VOI (if it does, optimized VOI loading is disabled)
            if(t0 == t0a &&
               t1 == t1a)        // missing piece is along X,Y,Z
            {
                t0m = t0;
                t1m = t1;
            }
            else if(volT1 == t1a)
            {
                t0m = t1a+1;
                t1m = t1;
                itm::debug(LEV3, strprintf("missing piece along T is [%d,%d]", t0m, t1m).c_str(), __itm__current__function__);
            }
            else if(volT0 == t0a)
            {
                t0m = t0;
                t1m = t0a-1;
                itm::debug(LEV3, strprintf("missing piece along T is [%d,%d]", t0m, t1m).c_str(), __itm__current__function__);
            }
            else
            {
                t0m = t1a;
                t1m = t1;
                itm::warning(strprintf("internal intersection detected, [%d,%d] is within [%d,%d], disabling optimized VOI loading", t0a, t1a, volT0, volT1).c_str(), __itm__current__function__);
            }
        }
        else
        {
            t0m = t0;
            t1m = t1;
        }
    }


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
        {
            uint scaling = std::min(std::min(scalx, scaly), scalz);
            itm::warning(strprintf("Fast nonuniform scaling not supported: requested scaling along X,Y,Z is {%d, %d, %d}, but will perform %d", scalx, scaly, scalz, scaling).c_str());
            scalx = scaly = scalz = scaling;
        }

        uint32 buf_data_dims[5]   = {volH1-volH0, volV1-volV0, volD1-volD0, nchannels, volT1-volT0+1};
        uint32 img_dims[5]        = {xDimInterp,  yDimInterp,  zDimInterp,  nchannels, t1-t0+1};
        uint32 buf_data_offset[5] = {x0a-volH0,   y0a-volV0,   z0a-volD0,   0,         t0a-volT0};
        uint32 img_offset[5]      = {x0a-x0,      y0a-y0,      z0a-z0,      0,         t0a-t0};
        uint32 buf_data_count[5]  = {x1a-x0a,     y1a-y0a,     z1a-z0a,     0,         t1a-t0a+1};

        CImageUtils::copyVOI(view3DWidget->getiDrawExternalParameter()->image4d->getRawData(), buf_data_dims, buf_data_offset, buf_data_count, img, img_dims, img_offset, scalx);
    }

    //interpolation
    else
        QMessageBox::critical(this, "Error", "Interpolation of the pre-buffered image not yet implemented",QObject::tr("Ok"));


    return img;
}

/**********************************************************************************
* Returns  the  maximum intensity projection  of the given VOI in a newly allocated
* array. Data is taken from the currently displayed image.
***********************************************************************************/
itm::uint8*
    CViewer::getMIP(int x0, int x1,                         // VOI [x0, x1) in the local reference sys
                            int y0, int y1,                         // VOI [y0, y1) in the local reference sys
                            int z0, int z1,                         // VOI [z0, z1) in the local reference sys
                            int t0 /* = -1 */, int t1 /* = -1 */,   // VOI [t0, t1] in the local reference sys
                            itm::direction dir /* = z */,
                            bool to_BGRA /* = false */,             //true if mip data must be stored into BGRA format
                            itm::uint8 alpha /* = 255 */)           //alpha transparency used if to_BGRA is true
throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s, x0 = %d, x1 = %d, y0 = %d, y1 = %d, z0 = %d, z1 = %d, t0 = %d, t1 = %d, dir = %d, to_BGRA = %s, alpha = %d",
                                        titleShort.c_str(), x0, x1, y0, y1, z0, z1, t0, t1, dir, to_BGRA ? "true" : "false", alpha).c_str(), __itm__current__function__);

    if(t0 == -1)
        t0 = volT0;
    if(t1 == -1)
        t1 = volT1;

    uint32 img_dims[5]   = {volH1-volH0, volV1-volV0, volD1-volD0, nchannels, volT1-volT0+1};
    uint32 img_offset[5] = {x0   -volH0, y0   -volV0, z0   -volD0, 0,         t0-volT0};
    uint32 img_count[5]  = {x1   -x0,    y1   -y0,    z1   -z0,    0,         t1-t0+1};

    return CImageUtils::mip(view3DWidget->getiDrawExternalParameter()->image4d->getRawData(), img_dims, img_offset, img_count, dir, to_BGRA, alpha);
}



/**********************************************************************************
* Makes the current view the last one by  deleting (and deallocting) its subsequent
* views.
***********************************************************************************/
void CViewer::makeLastView() throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

    if(CViewer::current != this)
        throw RuntimeException(QString("in CViewer::makeLastView(): this view is not the current one, thus can't be made the last view").toStdString().c_str());

    while(CViewer::last != this)
    {
        CViewer::last = CViewer::last->prev;
        CViewer::last->next->toBeClosed = true;
        delete CViewer::last->next;
        CViewer::last->next = 0;
    }
}

/**********************************************************************************
* Saves/restore the state of PMain spinboxes for subvolume selection
***********************************************************************************/
void CViewer::saveSubvolSpinboxState()
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

    PMain& pMain = *(PMain::getInstance());
    V0_sbox_min = pMain.V0_sbox->minimum();
    V1_sbox_max = pMain.V1_sbox->maximum();
    H0_sbox_min = pMain.H0_sbox->minimum();
    H1_sbox_max = pMain.H1_sbox->maximum();
    D0_sbox_min = pMain.D0_sbox->minimum();
    D1_sbox_max = pMain.D1_sbox->maximum();
//    T0_sbox_min = pMain.T0_sbox->minimum();
//    T1_sbox_max = pMain.T1_sbox->maximum();
    V0_sbox_val = pMain.V0_sbox->value();
    V1_sbox_val = pMain.V1_sbox->value();
    H0_sbox_val = pMain.H0_sbox->value();
    H1_sbox_val = pMain.H1_sbox->value();
    D0_sbox_val = pMain.D0_sbox->value();
    D1_sbox_val = pMain.D1_sbox->value();
    if(pMain.frameCoord->isEnabled())
    {
        T0_sbox_val = pMain.T0_sbox->text().toInt();
        T1_sbox_val = pMain.T1_sbox->text().toInt();
    }
}

void CViewer::restoreSubvolSpinboxState()
{  
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

    PMain& pMain = *(PMain::getInstance());
    pMain.V0_sbox->setMinimum(V0_sbox_min);
    pMain.V1_sbox->setMaximum(V1_sbox_max);
    pMain.H0_sbox->setMinimum(H0_sbox_min);
    pMain.H1_sbox->setMaximum(H1_sbox_max);
    pMain.D0_sbox->setMinimum(D0_sbox_min);
    pMain.D1_sbox->setMaximum(D1_sbox_max);
//    pMain.T0_sbox->setMinimum(T0_sbox_min);
//    pMain.T1_sbox->setMaximum(T1_sbox_max);
    pMain.V0_sbox->setValue(V0_sbox_val);
    pMain.V1_sbox->setValue(V1_sbox_val);
    pMain.H0_sbox->setValue(H0_sbox_val);
    pMain.H1_sbox->setValue(H1_sbox_val);
    pMain.D0_sbox->setValue(D0_sbox_val);
    pMain.D1_sbox->setValue(D1_sbox_val);
    if(pMain.frameCoord->isEnabled())
    {
        pMain.T0_sbox->setText(QString::number(T0_sbox_val));
        pMain.T1_sbox->setText(QString::number(T1_sbox_val));
    }
}

/**********************************************************************************
* Annotations are stored/loaded to/from the <CAnnotations> object
***********************************************************************************/
void CViewer::storeAnnotations() throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

    QElapsedTimer timer;

    // 2014-11-17. Alessandro. @FIXED "duplicated annotations" bug
    // use the same annotation VOI that was set for ::loadAnnotations at current explorer creation
    /**/itm::debug(itm::LEV3, strprintf("use annotation VOI X[%d,%d), Y[%d,%d), Z[%d,%d)", anoH0, anoH1, anoV0, anoV1, anoD0, anoD1).c_str(), __itm__current__function__);
    interval_t x_range(anoH0, anoH1);
    interval_t y_range(anoV0, anoV1);
    interval_t z_range(anoD0, anoD1);


    // begin new macro group of AnnotationOperation
    //if(triViewWidget->getImageData()->listLandmarks.empty() == false || V3D_env->getSWC(this->window).listNeuron.empty() == false)
        itm::AnnotationOperation::newGroup();

    /**********************************************************************************
    * MARKERS
    ***********************************************************************************/
    //storing edited markers
    QList<LocationSimple> markers = triViewWidget->getImageData()->listLandmarks;
    if(!markers.empty())
    {       
        // @fixed by Alessandro on 2014-07-21: excluding hidden markers from store operation
        timer.start();
        QList<LocationSimple>::iterator it = markers.begin();
        while (it != markers.end())
        {
            if (is_outside((*it).x, (*it).y, (*it).z))
                it = markers.erase(it);
            else
                ++it;
        }
        PLog::getInstance()->appendOperation(new AnnotationOperation(QString("store annotations: exclude hidden landmarks, view ").append(title.c_str()).toStdString(), itm::CPU, timer.elapsed()));

        //converting local coordinates into global coordinates
        timer.restart();
        for(int i=0; i<markers.size(); i++)
        {
            markers[i].x = getGlobalHCoord(markers[i].x, -1, false, false, __itm__current__function__);
            markers[i].y = getGlobalVCoord(markers[i].y, -1, false, false, __itm__current__function__);
            markers[i].z = getGlobalDCoord(markers[i].z, -1, false, false, __itm__current__function__);
        }
        PLog::getInstance()->appendOperation(new AnnotationOperation(QString("store annotations: convert landmark coordinates, view ").append(title.c_str()).toStdString(), itm::CPU, timer.elapsed()));

        //storing markers
        timer.restart();
        CAnnotations::getInstance()->addLandmarks(x_range, y_range, z_range, markers);
        PLog::getInstance()->appendOperation(new AnnotationOperation(QString("store annotations: store landmarks in the octree, view ").append(title.c_str()).toStdString(), itm::CPU, timer.elapsed()));
    }

    /**********************************************************************************
    * CURVES
    ***********************************************************************************/
    //storing edited curves
    NeuronTree nt = this->V3D_env->getSWC(this->window);
    if(!nt.listNeuron.empty())
    {
        /* @debug */ //printf("\ngoing to store in TeraFly the curve points ");

        //converting local coordinates into global coordinates
        timer.restart();
        for(int i=0; i<nt.listNeuron.size(); i++)
        {
            /* @debug */ //printf("%d(%d) [(%.0f,%.0f,%.0f) ->", nt.listNeuron[i].n, nt.listNeuron[i].pn, nt.listNeuron[i].x, nt.listNeuron[i].y, nt.listNeuron[i].z);
            nt.listNeuron[i].x = getGlobalHCoord(nt.listNeuron[i].x, -1, false, false, __itm__current__function__);
            nt.listNeuron[i].y = getGlobalVCoord(nt.listNeuron[i].y, -1, false, false, __itm__current__function__);
            nt.listNeuron[i].z = getGlobalDCoord(nt.listNeuron[i].z, -1, false, false, __itm__current__function__);
            /* @debug */ //printf("(%.0f,%.0f,%.0f)]  ", nt.listNeuron[i].x, nt.listNeuron[i].y, nt.listNeuron[i].z);
        }
        PLog::getInstance()->appendOperation(new AnnotationOperation(QString("store annotations: convert curve nodes coordinates, view ").append(title.c_str()).toStdString(), itm::CPU, timer.elapsed()));
        /* @debug */ //printf("\n");

        //storing curves
        timer.restart();
        CAnnotations::getInstance()->addCurves(x_range, y_range, z_range, nt);
        PLog::getInstance()->appendOperation(new AnnotationOperation(QString("store annotations: store curves in the octree, view ").append(title.c_str()).toStdString(), itm::CPU, timer.elapsed()));
    }

}

void CViewer::clearAnnotations() throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

    //clearing previous annotations (useful when this view has been already visited)
    V3D_env->getHandleNeuronTrees_Any3DViewer(window3D)->clear();

    QList<LocationSimple> vaa3dMarkers;
    V3D_env->setLandmark(window, vaa3dMarkers);
//    V3D_env->setSWC(window, vaa3dCurves);
    V3D_env->pushObjectIn3DWindow(window);
    view3DWidget->enableMarkerLabel(false);
    view3DWidget->getRenderer()->endSelectMode();
}

void CViewer::deleteMarkerROI(QVector<QPoint> ROI_contour) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s, ROI.size() = %d", titleShort.c_str(), ROI_contour.size()).c_str(), __itm__current__function__);

    // compute polygon from the given contour
    QPolygon ROI_poly(ROI_contour);

    // save current cursor and set wait cursor
    QCursor cursor = view3DWidget->cursor();
    CViewer::setCursor(Qt::WaitCursor);

    // pixel sampling step
    int sampling = CSettings::instance()->getAnnotationMarkersDeleteROISampling();

    // delete marker (if any) at each location inside the polygon
    QList<LocationSimple> deletedMarkers;
    QRect bbox = ROI_poly.boundingRect();
    for(int i=bbox.top(); i<bbox.bottom(); i=i+sampling)
        for(int j=bbox.left(); j<bbox.right(); j=j+sampling)
            if(ROI_poly.containsPoint(QPoint(j,i), Qt::OddEvenFill))    // Ray-crossing algorithm
                deleteMarkerAt(j,i, &deletedMarkers);

    undoStack.beginMacro("delete markers");
    undoStack.push(new QUndoMarkerDeleteROI(this, deletedMarkers));
    undoStack.endMacro();
    PAnoToolBar::instance()->buttonUndo->setEnabled(true);

    // reset saved cursor
    CViewer::setCursor(cursor);

    //update visible markers
    PAnoToolBar::instance()->buttonMarkerRoiViewChecked(PAnoToolBar::instance()->buttonMarkerRoiView->isChecked());
}


void CViewer::createMarkerAt(int x, int y) throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s, point = (%x, %y)", titleShort.c_str(), x, y).c_str(), __itm__current__function__);
    view3DWidget->getRenderer()->hitPen(x, y);
    QList<LocationSimple> vaa3dMarkers = V3D_env->getLandmark(window);
    undoStack.beginMacro("create marker");
    undoStack.push(new QUndoMarkerCreate(this, vaa3dMarkers.back()));
    undoStack.endMacro();
    PAnoToolBar::instance()->buttonUndo->setEnabled(true);

    // all markers have the same color when they are created
    vaa3dMarkers.back().color = CImageUtils::vaa3D_color(0,0,255);
    V3D_env->setLandmark(window, vaa3dMarkers);
    V3D_env->pushObjectIn3DWindow(window);

    //update visible markers
    PAnoToolBar::instance()->buttonMarkerRoiViewChecked(PAnoToolBar::instance()->buttonMarkerRoiView->isChecked());
}

void CViewer::deleteMarkerAt(int x, int y, QList<LocationSimple>* deletedMarkers /* = 0 */) throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s, point = (%x, %y)", titleShort.c_str(), x, y).c_str(), __itm__current__function__);

    // select marker (if any) at the clicked location
    view3DWidget->getRenderer()->selectObj(x,y, false);

    // search for the selected markers
    vector<int> vaa3dMarkers_tbd;
    QList<LocationSimple> vaa3dMarkers = V3D_env->getLandmark(window);
    QList <ImageMarker> imageMarkers = static_cast<Renderer_gl1*>(view3DWidget->getRenderer())->listMarker;
    for(int i=0; i<imageMarkers.size(); i++)
    {
        if(imageMarkers[i].selected)
        {
            for(int j=0; j<vaa3dMarkers.size(); j++)
                if(vaa3dMarkers[j].x == imageMarkers[i].x &&
                   vaa3dMarkers[j].y == imageMarkers[i].y &&
                   vaa3dMarkers[j].z == imageMarkers[i].z &&
                   !CAnnotations::isMarkerOutOfRendererBounds(vaa3dMarkers[j]))
                    vaa3dMarkers_tbd.push_back(j);
        }
    }

    // remove selected markers
    for(int i=0; i<vaa3dMarkers_tbd.size(); i++)
    {
        if(deletedMarkers)
            deletedMarkers->push_back(vaa3dMarkers[vaa3dMarkers_tbd[i]]);
        else
        {
            undoStack.beginMacro("delete marker");
            undoStack.push(new QUndoMarkerDelete(this, vaa3dMarkers[vaa3dMarkers_tbd[i]]));
            undoStack.endMacro();
            PAnoToolBar::instance()->buttonUndo->setEnabled(true);
        }

        vaa3dMarkers.removeAt(vaa3dMarkers_tbd[i]);
    }

    // set new markers
    V3D_env->setLandmark(window, vaa3dMarkers);
    V3D_env->pushObjectIn3DWindow(window);

    // end select mode
    view3DWidget->getRenderer()->endSelectMode();

    //update visible markers
    PAnoToolBar::instance()->buttonMarkerRoiViewChecked(PAnoToolBar::instance()->buttonMarkerRoiView->isChecked());
}

void CViewer::loadAnnotations() throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

    // begin new macro group of AnnotationOperation
    //itm::AnnotationOperation::newGroup();

    // where to put vaa3d annotations
    QList<LocationSimple> vaa3dMarkers;
    NeuronTree vaa3dCurves;

    // to measure elapsed time
    QElapsedTimer timer;

    //clearing previous annotations (useful when this view has been already visited)
    /**/itm::debug(itm::LEV3, strprintf("clearing previous annotations").c_str(), __itm__current__function__);
    V3D_env->getHandleNeuronTrees_Any3DViewer(window3D)->clear();

    //computing the current volume range in the highest resolution image space
    /**/itm::debug(itm::LEV3, strprintf("computing the current volume range in the highest resolution image space").c_str(), __itm__current__function__);
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

    // set volume range to infinite if unlimited space annotation option is active
    if(PMain::getInstance()->spaceSizeUnlimited->isChecked() && this == CViewer::first)
    {
        // unlimited annotation VOI is used only in the first view (whole image) so as to include out-of-bounds annotation objects
        x_range.start = y_range.start = z_range.start = 0;
        x_range.end = y_range.end = z_range.end = std::numeric_limits<itm::uint32>::max();
    }
    else if(this != CViewer::first)
    {
        // for subsequent views (i.e., higher resolutions at certain VOIs), the actual annotation VOI is enlarged by 100%
        // to enable the "Show/hide markers around the displayed ROI" function in the annotation toolbar
        int vmPerc = 100;
        int vmX = (x_range.end - x_range.start)*(vmPerc/100.0f)/2;
        int vmY = (y_range.end - y_range.start)*(vmPerc/100.0f)/2;
        int vmZ = (z_range.end - z_range.start)*(vmPerc/100.0f)/2;
        x_range.start  = std::max(0, x_range.start - vmX);
        x_range.end   += vmX;
        y_range.start  = std::max(0, y_range.start - vmY);
        y_range.end   += vmY;
        z_range.start  = std::max(0, z_range.start - vmZ);
        z_range.end   += vmZ;
    }

    // @FIXED "duplicated annotations" bug by Alessandro on 2014-17-11. Annotation VOI +100% enlargment is the source of "duplicated annotations" bug because
    // this creates an asymmetry between loading annotations in the displayed VOI (which is done with +100% enlargment) and saving annotations
    // from the displayed VOI (which is done w/o +100% enlargment !!!)
    // Then, we save the actual annotation VOI in object members and use those at saving time.
    anoV0 = y_range.start;
    anoV1 = y_range.end;
    anoH0 = x_range.start;
    anoH1 = x_range.end;
    anoD0 = z_range.start;
    anoD1 = z_range.end;
    /**/itm::debug(itm::LEV3, strprintf("store annotation VOI X[%d,%d), Y[%d,%d), Z[%d,%d)", anoH0, anoH1, anoV0, anoV1, anoD0, anoD1).c_str(), __itm__current__function__);

    //obtaining the annotations within the current window
    /**/itm::debug(itm::LEV3, strprintf("obtaining the annotations within the current window").c_str(), __itm__current__function__);
    CAnnotations::getInstance()->findLandmarks(x_range, y_range, z_range, vaa3dMarkers);
    CAnnotations::getInstance()->findCurves(x_range, y_range, z_range, vaa3dCurves.listNeuron);

    //converting global coordinates to local coordinates
    timer.restart();
    /**/itm::debug(itm::LEV3, strprintf("converting global coordinates to local coordinates").c_str(), __itm__current__function__);
    for(int i=0; i<vaa3dMarkers.size(); i++)
    {
        vaa3dMarkers[i].x = getLocalHCoord(vaa3dMarkers[i].x);
        vaa3dMarkers[i].y = getLocalVCoord(vaa3dMarkers[i].y);
        vaa3dMarkers[i].z = getLocalDCoord(vaa3dMarkers[i].z);
    }
    /* @debug */ //printf("\n\ngoing to insert in Vaa3D the curve points ");
    for(int i=0; i<vaa3dCurves.listNeuron.size(); i++)
    {
        /* @debug */ //printf("%d(%d) [(%.0f,%.0f,%.0f) ->", vaa3dCurves.listNeuron[i].n, vaa3dCurves.listNeuron[i].pn, vaa3dCurves.listNeuron[i].x, vaa3dCurves.listNeuron[i].y, vaa3dCurves.listNeuron[i].z);
        vaa3dCurves.listNeuron[i].x = getLocalHCoord(vaa3dCurves.listNeuron[i].x);
        vaa3dCurves.listNeuron[i].y = getLocalVCoord(vaa3dCurves.listNeuron[i].y);
        vaa3dCurves.listNeuron[i].z = getLocalDCoord(vaa3dCurves.listNeuron[i].z);
        /* @debug */ //printf("(%.0f,%.0f,%.0f)]  ", vaa3dCurves.listNeuron[i].x, vaa3dCurves.listNeuron[i].y, vaa3dCurves.listNeuron[i].z);
    }
    PLog::getInstance()->appendOperation(new AnnotationOperation(QString("load annotations: convert coordinates, view ").append(title.c_str()).toStdString(), itm::CPU, timer.elapsed()));
    /* @debug */ //printf("\n\n");


    //assigning annotations
    /**/itm::debug(itm::LEV3, strprintf("assigning annotations").c_str(), __itm__current__function__);
    timer.restart();
    V3D_env->setLandmark(window, vaa3dMarkers);
    V3D_env->setSWC(window, vaa3dCurves);
    V3D_env->pushObjectIn3DWindow(window);
    view3DWidget->enableMarkerLabel(false);
    view3DWidget->getRenderer()->endSelectMode();


    //end curve editing mode
    QList<NeuronTree>* listNeuronTree = static_cast<Renderer_gl1*>(view3DWidget->getRenderer())->getHandleNeuronTrees();
    for (int i=0; i<listNeuronTree->size(); i++)
        (*listNeuronTree)[i].editable = false; //090928
    view3DWidget->updateTool();
    view3DWidget->update();


    //update visible markers
    PAnoToolBar::instance()->buttonMarkerRoiViewChecked(PAnoToolBar::instance()->buttonMarkerRoiView->isChecked());

    PLog::getInstance()->appendOperation(new AnnotationOperation(QString("load annotations: push objects into view ").append(title.c_str()).toStdString(), itm::GPU, timer.elapsed()));
}

/**********************************************************************************
* Restores the current view from the given (neighboring) view.
* Called by the next(prev) <CViewer>  when the user  zooms out(in) and  the
* lower(higher) resoolution has to be reestabilished.
***********************************************************************************/
void CViewer::restoreViewerFrom(CViewer* source) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s, source->title = %s", titleShort.c_str(), source->titleShort.c_str()).c_str(), __itm__current__function__);

    // begin new group for RestoreViewerOperation
    itm::RestoreViewerOperation::newGroup();
    QElapsedTimer timer;
    timer.start();

    if(source)
    {
        //signal disconnections
        source->disconnect(source->view3DWidget, SIGNAL(changeXCut0(int)), source, SLOT(Vaa3D_changeXCut0(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeXCut1(int)), source, SLOT(Vaa3D_changeXCut1(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeYCut0(int)), source, SLOT(Vaa3D_changeYCut0(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeYCut1(int)), source, SLOT(Vaa3D_changeYCut1(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeZCut0(int)), source, SLOT(Vaa3D_changeZCut0(int)));
        source->disconnect(source->view3DWidget, SIGNAL(changeZCut1(int)), source, SLOT(Vaa3D_changeZCut1(int)));
        source->connect(source->window3D->timeSlider, SIGNAL(valueChanged(int)), source, SLOT(Vaa3D_changeTSlider(int)));
        //source->disconnect(source->view3DWidget, SIGNAL(xRotationChanged(int)), source, SLOT(Vaa3D_rotationchanged(int)));
        //source->disconnect(source->view3DWidget, SIGNAL(yRotationChanged(int)), source, SLOT(Vaa3D_rotationchanged(int)));
        //source->disconnect(source->view3DWidget, SIGNAL(zRotationChanged(int)), source, SLOT(Vaa3D_rotationchanged(int)));
        source->disconnect(PMain::getInstance()->refSys, SIGNAL(mouseReleased()), source, SLOT(PMain_rotationchanged()));
        source->disconnect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), source, SLOT(PMain_changeV0sbox(int)));
        source->disconnect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), source, SLOT(PMain_changeV1sbox(int)));
        source->disconnect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), source, SLOT(PMain_changeH0sbox(int)));
        source->disconnect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), source, SLOT(PMain_changeH1sbox(int)));
        source->disconnect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), source, SLOT(PMain_changeD0sbox(int)));
        source->disconnect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), source, SLOT(PMain_changeD1sbox(int)));

        source->view3DWidget->removeEventFilter(source);
        source->window3D->removeEventFilter(source);
        window3D->timeSlider->removeEventFilter(source);

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
        source->clearAnnotations();

        //registrating the current window as the current window of the multiresolution explorer windows chain
        CViewer::current = this;

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
        pMain->traslTneg->setEnabled(volT0 > 0);
        pMain->traslTpos->setEnabled(volT1 < CImport::instance()->getVolume(volResIndex)->getDIM_T()-1);

        //signal connections
        connect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
        connect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
        connect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
        connect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
        connect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
        connect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
        connect(window3D->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(Vaa3D_changeTSlider(int)));
        //connect(view3DWidget, SIGNAL(xRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        //connect(view3DWidget, SIGNAL(yRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        //connect(view3DWidget, SIGNAL(zRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
        connect(PMain::getInstance()->refSys,  SIGNAL(mouseReleased()),   this, SLOT(PMain_rotationchanged()));
        connect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
        connect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
        connect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
        connect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
        connect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
        connect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));

        view3DWidget->installEventFilter(this);
        window3D->installEventFilter(this);
        window3D->timeSlider->installEventFilter(this);

        //loading annotations of the current view
        this->loadAnnotations();

        // 5D data: select the same time frame (if available)
        if(CImport::instance()->is5D())
        {
            int source_s = source->window3D->timeSlider->value();
            if(source_s < volT0 || source_s > volT1)
                window3D->timeSlider->setValue(volT0);
            else
                window3D->timeSlider->setValue(source_s);
            view3DWidget->setVolumeTimePoint(window3D->timeSlider->value()-volT0);
        }

        //sync widgets
        syncWindows(source->window3D, window3D);

        //showing window
        this->window3D->raise();
        this->window3D->activateWindow();
        this->window3D->show();

        // update reference system dimension
        if(!PMain::getInstance()->isPRactive())
            PMain::getInstance()->refSys->setDims(volH1-volH0+1, volV1-volV0+1, volD1-volD0+1);

        // refresh annotation toolbar
        PAnoToolBar::instance()->refreshTools();

        //current windows now gets ready to user input
        isReady = true;
    }
    PLog::getInstance()->appendOperation(new RestoreViewerOperation(strprintf("Restored viewer %d from viewer %d", ID, source->ID), itm::ALL_COMPS, timer.elapsed()));

}

/**********************************************************************************
* Returns the most  likely 3D  point in the  image that the user is pointing on the
* renderer at the given location.
* This is based on the Vaa3D 3D point selection with one mouse click.
***********************************************************************************/
XYZ CViewer::getRenderer3DPoint(int x, int y)  throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("title = %s, x = %d, y = %d", titleShort.c_str(), x, y).c_str(), __itm__current__function__);


    //view3DWidget->getRenderer()->selectObj(x,y, false);

    #ifdef USE_EXPERIMENTAL_FEATURES
    return myRenderer_gl1::cast(static_cast<Renderer_gl1*>(view3DWidget->getRenderer()))->get3DPoint(x, y);
    #else
    throw RuntimeException("Double click zoom-in feature is disabled in the current version");
    #endif


//    Renderer_gl1* rend = static_cast<Renderer_gl1*>(view3DWidget->getRenderer());

//    XYZ p =  rend->selectPosition(x, y);
//    printf("\n\npoint = (%.0f, %.0f, %.0f)\n\n", p.x, p.y, p.z);
//    return p;
}

/**********************************************************************************
* method (indirectly) invoked by Vaa3D to propagate VOI's coordinates
***********************************************************************************/
void CViewer::invokedFromVaa3D(v3d_imaging_paras* params /* = 0 */)
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
    else if(view3DWidget->getiDrawExternalParameter()           &&
            view3DWidget->getiDrawExternalParameter()->image4d  &&
            view3DWidget->getiDrawExternalParameter()->image4d->getCustomStructPointer() )
        roi = static_cast<v3d_imaging_paras*>(view3DWidget->getiDrawExternalParameter()->image4d->getCustomStructPointer());

    if(!roi)
    {
        /**/itm::warning(strprintf("title = %s, Unable to get customStructPointer from Vaa3D V3dR_GLWidget. Aborting invokedFromVaa3D()", titleShort.c_str()).c_str(), __itm__current__function__);
        return;
    }

    if(params == 0)
        /**/itm::debug(itm::LEV3, strprintf("title = %s, Vaa3D ROI is [%d-%d] x [%d-%d] x [%d-%d]", titleShort.c_str(), roi->xs, roi->xe, roi->ys, roi->ye, roi->zs, roi->ze).c_str(), __itm__current__function__);


    //--- Alessandro 23/04/2014: after several bug fixes, it seems this bug does not occur anymore
    //--- Alessandro 24/08/2013: to prevent the plugin to crash at the deepest resolution when the plugin is invoked by Vaa3D
    //    if(volResIndex == CImport::instance()->getResolutions()-1)
//    {
//        QMessageBox::warning(this, "Warning", "Vaa3D-invoked actions at the highest resolution have been temporarily removed. "
//                             "Please use different operations such as \"Double-click zoom-in\" or \"Traslate\".");
//        return;
//    }

    //retrieving ROI infos propagated by Vaa3D
    int roiCenterX = roi->xe-(roi->xe-roi->xs)/2;
    int roiCenterY = roi->ye-(roi->ye-roi->ys)/2;
    int roiCenterZ = roi->ze-(roi->ze-roi->zs)/2;

    // Vaa3D's ROI mode triggers deleteAnnotationsROI when TeraFly's "buttonMarkerRoiDelete" mode is active
    if(roi->ops_type == 1 && PAnoToolBar::instance()->buttonMarkerRoiDelete->isChecked())
    {
        deleteMarkerROI(scribbling_points);

        // need to refresh annotation tools as this Vaa3D's action resets the Vaa3D annotation mode
        PAnoToolBar::instance()->refreshTools();
    }

    // otherwise before continue, check "Proofreading" mode is not active
    else if(PMain::getInstance()->isPRactive())
    {
        QMessageBox::information(this->window3D, "Warning", "TeraFly is running in \"Proofreading\" mode. All TeraFly's' navigation features are disabled. "
                                 "Please terminate the \"Proofreading\" mode and try again.");
        return;
    }

    // zoom-in around marker or ROI triggers a new window
    else if(roi->ops_type == 1 && !forceZoomIn)
        newViewer(roi->xe, roi->ye, roi->ze, volResIndex+1, volT0, volT1, false, -1, -1, -1, roi->xs, roi->ys, roi->zs);

    // zoom-in with mouse scroll up may trigger a new window if caching is not possible
    else if(roi->ops_type == 2 || forceZoomIn)
    {
        // reset forceZoomIn
        if(forceZoomIn)
            forceZoomIn = false;

        if(volResIndex != CImport::instance()->getResolutions()-1 &&   // do nothing if highest resolution has been reached
           ( isZoomDerivativePos() || has_double_clicked))             // accept zoom-in only when zoom factor derivative is positive or user has double clicked
        {
            // reset double click status
            has_double_clicked = false;

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
                /**/itm::debug(itm::LEV3, strprintf("title = %s, requested voi is [%.0f, %.0f] x [%.0f, %.0f] x [%.0f, %.0f]...BUT",
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
                /**/itm::debug(itm::LEV3, strprintf("title = %s, ...BUT actual requested VOI became [%.0f, %.0f] x [%.0f, %.0f] x [%.0f, %.0f]",
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

                /**/itm::debug(itm::LEV3, strprintf("title = %s, actual requested voi is [%.0f, %.0f] x [%.0f, %.0f] x [%.0f, %.0f] and cached is [%.0f, %.0f] x [%.0f, %.0f] x [%.0f, %.0f]",
                                                       titleShort.c_str(), gXS, gXE, gYS, gYE, gZS, gZE, gXScached, gXEcached, gYScached, gYEcached, gZScached, gZEcached).c_str(), __itm__current__function__);

                /**/itm::debug(itm::LEV3, strprintf("title = %s,intersection is %.0f x %.0f x %.0f with coverage factor = %.2f ", titleShort.c_str(),
                                                       intersectionX, intersectionY, intersectionZ, coverageFactor).c_str(), __itm__current__function__);

                //if Vaa3D VOI is covered for the selected percentage by the existing cached volume, just restoring its view
                if((coverageFactor >= PMain::getInstance()->cacheSens->value()/100.0f) && isReady)
                {
                    setActive(false);
                    resetZoomHistory();
                    next->restoreViewerFrom(this);
                }

                //otherwise invoking a new view
                else
                    newViewer(roiCenterX, roiCenterY, roiCenterZ, volResIndex+1, volT0, volT1, false, static_cast<int>((roi->xe-roi->xs)/2.0f+0.5f), static_cast<int>((roi->ye-roi->ys)/2.0f+0.5f), static_cast<int>((roi->ze-roi->zs)/2.0f+0.5f));
            }
            else
                newViewer(roiCenterX, roiCenterY, roiCenterZ, volResIndex+1, volT0, volT1, false, static_cast<int>((roi->xe-roi->xs)/2.0f+0.5f), static_cast<int>((roi->ye-roi->ys)/2.0f+0.5f), static_cast<int>((roi->ze-roi->zs)/2.0f+0.5f));
        }
        else
            /**/itm::debug(itm::LEV3, strprintf("title = %s, ignoring Vaa3D mouse scroll up zoom-in", titleShort.c_str()).c_str(), __itm__current__function__);
    }
    else
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr("in CViewer::Vaa3D_selectedROI(): unsupported (or unset) operation type"),QObject::tr("Ok"));
}

/**********************************************************************************
* Linked to volume cut scrollbars of Vaa3D widget containing the 3D renderer.
* This implements the syncronization Vaa3D-->TeraManager of subvolume selection.
***********************************************************************************/
void CViewer::Vaa3D_changeYCut0(int s)
{
    #ifdef terafly_enable_debug_max_level
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);
    #endif

    disconnect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
    PMain::getInstance()->V0_sbox->setValue(getGlobalVCoord(s, -1, true, false, __itm__current__function__)+1);
    PDialogProofreading::instance()->updateBlocks(0);
    connect(PMain::getInstance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
}
void CViewer::Vaa3D_changeYCut1(int s)
{
    #ifdef terafly_enable_debug_max_level
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);
    #endif

    disconnect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
    PMain::getInstance()->V1_sbox->setValue(getGlobalVCoord(s, -1, true, false, __itm__current__function__)+1);
    PDialogProofreading::instance()->updateBlocks(0);
    connect(PMain::getInstance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
}
void CViewer::Vaa3D_changeXCut0(int s)
{
    #ifdef terafly_enable_debug_max_level
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);
    #endif

    disconnect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
    PMain::getInstance()->H0_sbox->setValue(getGlobalHCoord(s, -1, true, false, __itm__current__function__)+1);
    PDialogProofreading::instance()->updateBlocks(0);
    connect(PMain::getInstance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
}
void CViewer::Vaa3D_changeXCut1(int s)
{
    #ifdef terafly_enable_debug_max_level
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);
    #endif

    disconnect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
    PMain::getInstance()->H1_sbox->setValue(getGlobalHCoord(s, -1, true, false, __itm__current__function__)+1);
    PDialogProofreading::instance()->updateBlocks(0);
    connect(PMain::getInstance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
}
void CViewer::Vaa3D_changeZCut0(int s)
{
    #ifdef terafly_enable_debug_max_level
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);
    #endif

    disconnect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
    PMain::getInstance()->D0_sbox->setValue(getGlobalDCoord(s, -1, true, false, __itm__current__function__)+1);
    PDialogProofreading::instance()->updateBlocks(0);
    connect(PMain::getInstance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
}
void CViewer::Vaa3D_changeZCut1(int s)
{
    #ifdef terafly_enable_debug_max_level
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);
    #endif

    disconnect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));
    PMain::getInstance()->D1_sbox->setValue(getGlobalDCoord(s, -1, true, false, __itm__current__function__)+1);
    PDialogProofreading::instance()->updateBlocks(0);
    connect(PMain::getInstance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));
}

void CViewer::Vaa3D_changeTSlider(int s, bool editingFinished /* = false */)
{
    #ifdef terafly_enable_debug_max_level
    /**/itm::debug(itm::LEV3, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);
    #endif

    if(isActive     &&              // window is visible
       isReady      &&              // window is ready for user input
       !toBeClosed  &&              // window is not going to be destroyed
       view3DWidget &&              // Vaa3D renderer has been instantiated
       CImport::instance()->is5D()) // data is 5D type
    {
        // change current frame coordinate
        PMain::getInstance()->frameCoord->setText(strprintf("t = %d/%d", s, CImport::instance()->getTDim()-1).c_str());

        // if frame is out of the displayed range
        if(s < volT0 || s > volT1)
        {
            // enter "waiting for 5D data" state
            setWaitingFor5D(true, true);

            // display message informing the user that something will happen if he/she confirms the operation
            PMain::getInstance()->statusBar->showMessage(strprintf("Ready to jump at time frame %d/%d", s, CImport::instance()->getTDim()-1).c_str());

            // if user operation is confirmed, then switching view
            if(editingFinished)
                newViewer( (volH1-volH0)/2, (volV1-volV0)/2, (volD1-volD0)/2, volResIndex, s-PMain::getInstance()->Tdim_sbox->value()/2, s+PMain::getInstance()->Tdim_sbox->value()/2, false);
        }
        // if frame is within the displayed range
        else
        {
            // exit from "waiting for 5D data" state (if previously set)
            setWaitingFor5D(false, true);

            // display default message
            PMain::getInstance()->statusBar->showMessage("Ready.");

            // display selected frame
            view3DWidget->setVolumeTimePoint(s-volT0);
        }
    }
}

/**********************************************************************************
* Linked to PMain GUI VOI's widgets.
* This implements the syncronization TeraManager-->Vaa3D of subvolume selection.
***********************************************************************************/
void CViewer::PMain_changeV0sbox(int s)
{
    #ifdef terafly_enable_debug_max_level
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);
    #endif

    disconnect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
    view3DWidget->setYCut0(getLocalVCoord(s-1, true)+1);
    PDialogProofreading::instance()->updateBlocks(0);
    connect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
}
void CViewer::PMain_changeV1sbox(int s)
{
    #ifdef terafly_enable_debug_max_level
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);
    #endif

    disconnect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
    view3DWidget->setYCut1(getLocalVCoord(s-1, true)+1);
    PDialogProofreading::instance()->updateBlocks(0);
    connect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
}
void CViewer::PMain_changeH0sbox(int s)
{
    #ifdef terafly_enable_debug_max_level
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);
    #endif

    disconnect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
    view3DWidget->setXCut0(getLocalHCoord(s-1, true)+1);
    PDialogProofreading::instance()->updateBlocks(0);
    connect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
}
void CViewer::PMain_changeH1sbox(int s)
{
    #ifdef terafly_enable_debug_max_level
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);
    #endif

    disconnect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
    view3DWidget->setXCut1(getLocalHCoord(s-1, true)+1);
    PDialogProofreading::instance()->updateBlocks(0);
    connect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
}
void CViewer::PMain_changeD0sbox(int s)
{
    #ifdef terafly_enable_debug_max_level
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);
    #endif

    disconnect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
    view3DWidget->setZCut0(getLocalDCoord(s-1, true)+1);
    PDialogProofreading::instance()->updateBlocks(0);
    connect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
}
void CViewer::PMain_changeD1sbox(int s)
{
    #ifdef terafly_enable_debug_max_level
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, s = %d", title.c_str(), s).c_str(), __itm__current__function__);
    #endif

    disconnect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
    view3DWidget->setZCut1(getLocalDCoord(s-1, true)+1);
    PDialogProofreading::instance()->updateBlocks(0);
    connect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
}

/**********************************************************************************
* Alignes the given widget to the left (or to the right) of the current window
***********************************************************************************/
void CViewer::alignToLeft(QWidget* widget)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

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
void CViewer::alignToRight(QWidget* widget)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s", titleShort.c_str()).c_str(), __itm__current__function__);

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
void CViewer::Vaa3D_rotationchanged(int s)
{
    if(isActive && !toBeClosed)
    {
//        printf("disconnect\n");
//        disconnect(view3DWidget, SIGNAL(xRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
//        disconnect(view3DWidget, SIGNAL(yRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));
//        disconnect(view3DWidget, SIGNAL(zRotationChanged(int)), this, SLOT(Vaa3D_rotationchanged(int)));

        //printf("view3DWidget->absoluteRotPose()\n");
        //QMessageBox::information(0, "Test", "view3DWidget->absoluteRotPose()");
        //view3DWidget->updateGL();
        view3DWidget->absoluteRotPose();
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
void CViewer::PMain_rotationchanged()
{
    if(isActive && !toBeClosed)
    {
        QGLRefSys* refsys = PMain::getInstance()->refSys;
        view3DWidget->doAbsoluteRot(refsys->getXRot(), refsys->getYRot(), refsys->getZRot());
    }
}

#ifdef USE_EXPERIMENTAL_FEATURES
/**********************************************************************************
* Linked to Vaa3D renderer slider
***********************************************************************************/
void CViewer::setZoom(int z)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s, zoom = %d", titleShort.c_str(), z).c_str(), __itm__current__function__);

    myV3dR_GLWidget::cast(view3DWidget)->setZoomO(z);
}
#endif

/**********************************************************************************
* Syncronizes widgets from <src> to <dst>
***********************************************************************************/
void CViewer::syncWindows(V3dR_MainWindow* src, V3dR_MainWindow* dst)
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
    dst->fcutSlider->setValue(src->fcutSlider->value());

    //syncronizing other controls
    dst->checkBox_displayAxes->setChecked(src->checkBox_displayAxes->isChecked());
    dst->checkBox_displayBoundingBox->setChecked(src->checkBox_displayBoundingBox->isChecked());
    dst->checkBox_OrthoView->setChecked(src->checkBox_OrthoView->isChecked());

    //propagating skeleton mode and line width
    dst->getGLWidget()->getRenderer()->lineType = src->getGLWidget()->getRenderer()->lineType;
    dst->getGLWidget()->getRenderer()->lineWidth = src->getGLWidget()->getRenderer()->lineWidth;
}

/**********************************************************************************
* Change to "waiting for 5D" state (i.e., when 5D data are to be loaded or are loading)
***********************************************************************************/
void CViewer::setWaitingFor5D(bool wait, bool pre_wait /* = false */)
{
    /**/itm::debug(itm::LEV3, strprintf("title = %s, wait = %s, pre_wait = %s, this->waitingFor5D = %s",
                                           title.c_str(), wait ? "true" : "false", pre_wait ? "true" : "false", waitingFor5D ? "true" : "false").c_str(), __itm__current__function__);

    if(CImport::instance()->getTDim() > 1 && wait != this->waitingFor5D)
    {
        PMain& pMain = *(PMain::getInstance());
        this->waitingFor5D = wait;
        if(waitingFor5D)
        {
            // change GUI appearance
            QPalette palette = pMain.frameCoord->palette();
            palette.setColor(QPalette::Base, QColor(255, 255, 181));
            palette.setColor(QPalette::Background, QColor(255, 255, 181));
            pMain.frameCoord->setPalette(palette);
            pMain.setPalette(palette);
            window3D->setPalette(palette);

            // disable time bar
            if(!pre_wait)
                window3D->timeSlider->setEnabled(false);
        }
        else
        {
            // change GUI appearance
            pMain.frameCoord->setPalette(QApplication::palette( PMain::getInstance()->frameCoord ));
            pMain.setPalette(QApplication::palette( PMain::getInstance() ));
            window3D->setPalette(QApplication::palette( window3D ));

            // re-enable the time bar
            if(!pre_wait)
                window3D->timeSlider->setEnabled(true);
        }
    }
}

/**********************************************************************************
* Change current Vaa3D's rendered cursor
***********************************************************************************/
void CViewer::setCursor(const QCursor& cur, bool renderer_only /* = false */)
{
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

    if(CViewer::current)
    {
        if(!renderer_only)
            CViewer::current->window3D->setCursor(cur);
        CViewer::current->view3DWidget->setCursor(cur);
    }
}

