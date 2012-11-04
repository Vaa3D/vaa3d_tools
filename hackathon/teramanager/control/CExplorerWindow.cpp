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
#include "presentation/PMain.h"
#include "renderer_gl1.h"
#include "v3d_imaging_para.h"
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

    //opening tri-view window
    this->window = V3D_env->newImageWindow(QString(title.c_str()));
    Image4DSimple* image = new Image4DSimple();
    image->setFileName(title.c_str());
    image->setData(imgData, volH1-volH0, volV1-volV0, volD1-volD0, nchannels, V3D_UINT8);

    /*unsigned char* pdata = new unsigned char[300*300*100*3];
    for(int c=0; c<3; c++)
        for(int z=0; z<100; z++)
            for(int y=0; y<300; y++)
                for(int x=0; x<300*3; x++)
                {
                    if(y<100)
                    {
                        pdata[c*300*300*100 + z*300*300 +y*300 +x] = (c==0 ? 255 : 0);
                    }
                    else
                    {
                        pdata[c*300*300*100 + z*300*300 +y*300 +x] = 0;
                    }
                }
    image->setData(pdata, 300, 300, 100, 3, V3D_UINT8);*/

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
    }
    //otherwise this is the highest resolution window
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

    //selecting the current resolution in the PMain GUI
    pMain->resolution_cbox->setCurrentIndex(volResIndex);

    //setting min, max and value of PMain GUI VOI's widgets
    pMain->V0_sbox->setMinimum(getHighestResGlobalVCoord(view3DWidget->yCut0())+1);
    pMain->V0_sbox->setValue(pMain->V0_sbox->minimum());
    pMain->V1_sbox->setMaximum(getHighestResGlobalVCoord(view3DWidget->yCut1())+1);
    pMain->V1_sbox->setValue(pMain->V1_sbox->maximum());
    pMain->H0_sbox->setMinimum(getHighestResGlobalHCoord(view3DWidget->xCut0())+1);
    pMain->H0_sbox->setValue(pMain->H0_sbox->minimum());
    pMain->H1_sbox->setMaximum(getHighestResGlobalHCoord(view3DWidget->xCut1())+1);
    pMain->H1_sbox->setValue(pMain->H1_sbox->maximum());
    pMain->D0_sbox->setMinimum(getHighestResGlobalDCoord(view3DWidget->zCut0())+1);
    pMain->D0_sbox->setValue(pMain->D0_sbox->minimum());
    pMain->D1_sbox->setMaximum(getHighestResGlobalDCoord(view3DWidget->zCut1())+1);
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
            LandmarkList markers = V3D_env->getLandmark(triViewWidget);
            if(view3DWidget->zoom() > 30        &&              //zoom-in threshold reached
               markers.size() == 1)                             //only one marker exists
            {
                switchToHigherRes(markers.first().x, markers.first().y, markers.first().z);
                markers.clear();
                V3D_env->setLandmark(triViewWidget, markers);
                view3DWidget->getRenderer()->updateLandmark();
            }
            else if(view3DWidget->zoom() < 0    &&              //zoom-out threshold reached
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
        else if(object == view3DWidget && event->type()==QEvent::Close)
        {
            if(prev                        &&                   //the previous resolution exists
               !toBeClosed)                                     //the current resolution does not have to be closed)
            {
                event->ignore();
                toBeClosed = true;
                prev->restore();
                return true;
            }
            else if(!prev)
                PMain::instance()->close();
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
            QMessageBox::critical(this,QObject::tr("Error"), "Incorrect resolution generation",QObject::tr("Ok"));

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
    pMain.import_form->setEnabled(false);
    if(CImport::instance()->getVolume(volResIndex+1))
        pMain.statusBar->showMessage("Zooming in to the higher resolution...");
    else
        pMain.statusBar->showMessage("Loading selected VOI...");

    //if the higher resolution exists, obtaining VOI at the higher resolution
    if(CImport::instance()->getVolume(volResIndex+1))
    {
        float ratio = static_cast<float>(CImport::instance()->getVolume(volResIndex+1)->getDIM_D())/CImport::instance()->getVolume(volResIndex)->getDIM_D();
        int VoiCenterX = (x + volH0)*ratio+0.5f;
        int VoiCenterY = (y + volV0)*ratio+0.5f;
        int VoiCenterZ = (z + volD0)*ratio+0.5f;
        dx = dx == -1 ? pMain.Hdim_sbox->value()/2 : (dx*ratio+0.5f);
        dy = dy == -1 ? pMain.Vdim_sbox->value()/2 : (dy*ratio+0.5f);
        dz = dz == -1 ? pMain.Ddim_sbox->value()/2 : (dz*ratio+0.5f);
        CVolume::instance()->setVoi(this, volResIndex+1, VoiCenterY-dy, VoiCenterY+dy, VoiCenterX-dx, VoiCenterX+dx, VoiCenterZ-dz, VoiCenterZ+dz);
    }
    //otherwise obtaining VOI at the current resolution
    else
    {
        int VoiCenterX = x + volH0;
        int VoiCenterY = y + volV0;
        int VoiCenterZ = z + volD0;
        dx = dx == -1 ? pMain.Hdim_sbox->value()/2 : dx;
        dy = dy == -1 ? pMain.Vdim_sbox->value()/2 : dy;
        dz = dz == -1 ? pMain.Ddim_sbox->value()/2 : dz;
        CVolume::instance()->setVoi(this, volResIndex, VoiCenterY-dy, VoiCenterY+dy, VoiCenterX-dx, VoiCenterX+dx, VoiCenterZ-dz, VoiCenterZ+dz);
    }

    //launching thread where the VOI has to be loaded
    CVolume::instance()->start();
}

/**********************************************************************************
* Restores the current window and destroys the next <CExplorerWindow>.
* Called by the next <CExplorerWindow> when the user zooms out and  the lower reso-
* lution has to be reestabilished.
***********************************************************************************/
void CExplorerWindow::restore()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] restore() launched\n",
           this->thread()->currentThreadId(), title.c_str() );
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

        //closing next
        delete next;
        next = 0;

        //registrating the current window as the last window of the multiresolution explorer windows chain
        CExplorerWindow::last = this;

        //selecting the current resolution in the PMain GUI
        PMain* pMain = PMain::instance();
        pMain->resolution_cbox->setCurrentIndex(volResIndex);

        //setting min, max and value of PMain GUI VOI's widgets
        pMain->V0_sbox->setMinimum(getHighestResGlobalVCoord(view3DWidget->yCut0())+1);
        pMain->V0_sbox->setValue(pMain->V0_sbox->minimum());
        pMain->V1_sbox->setMaximum(getHighestResGlobalVCoord(view3DWidget->yCut1())+1);
        pMain->V1_sbox->setValue(pMain->V1_sbox->maximum());
        pMain->H0_sbox->setMinimum(getHighestResGlobalHCoord(view3DWidget->xCut0())+1);
        pMain->H0_sbox->setValue(pMain->H0_sbox->minimum());
        pMain->H1_sbox->setMaximum(getHighestResGlobalHCoord(view3DWidget->xCut1())+1);
        pMain->H1_sbox->setValue(pMain->H1_sbox->maximum());
        pMain->D0_sbox->setMinimum(getHighestResGlobalDCoord(view3DWidget->zCut0())+1);
        pMain->D0_sbox->setValue(pMain->D0_sbox->minimum());
        pMain->D1_sbox->setMaximum(getHighestResGlobalDCoord(view3DWidget->zCut1())+1);
        pMain->D1_sbox->setValue(pMain->D1_sbox->maximum());

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
* Returns  the  global coordinate  (which starts from 0) in  the highest resolution
* volume image space given the local coordinate (which starts from 0) in the current
* resolution volume image space.
***********************************************************************************/
int CExplorerWindow::getHighestResGlobalVCoord(int localVCoord)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);
    return (volV0+localVCoord)*ratio + 0.5f;
}
int CExplorerWindow::getHighestResGlobalHCoord(int localHCoord)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);
    return (volH0+localHCoord)*ratio + 0.5f;
}
int CExplorerWindow::getHighestResGlobalDCoord(int localDCoord)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);
    return (volD0+localDCoord)*ratio + 0.5f;
}

/**********************************************************************************
* Returns the local coordinate (which starts from 0) in the current resolution vol-
* ume image space given the global coordinate  (which starts from 0) in the highest
* resolution volume image space.
***********************************************************************************/
int CExplorerWindow::getLocalVCoord(int highestResGlobalVCoord)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);
    return highestResGlobalVCoord/ratio - volV0 + 0.5f;
}
int CExplorerWindow::getLocalHCoord(int highestResGlobalHCoord)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);
    return highestResGlobalHCoord/ratio - volH0 + 0.5f;
}
int CExplorerWindow::getLocalDCoord(int highestResGlobalDCoord)
{
    float ratio = (CImport::instance()->getHighestResVolume()->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);
    return highestResGlobalDCoord/ratio - volD0 + 0.5f;
}

/**********************************************************************************
* Linked to volume cut scrollbars of Vaa3D widget containing the 3D renderer.
* This implements the syncronization Vaa3D-->TeraManager of subvolume selection.
***********************************************************************************/
void CExplorerWindow::Vaa3D_changeYCut0(int s)
{
    disconnect(PMain::instance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
    PMain::instance()->V0_sbox->setValue(getHighestResGlobalVCoord(s)+1);
    connect(PMain::instance()->V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV0sbox(int)));
}
void CExplorerWindow::Vaa3D_changeYCut1(int s)
{
    disconnect(PMain::instance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
    PMain::instance()->V1_sbox->setValue(getHighestResGlobalVCoord(s)+1);
    connect(PMain::instance()->V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeV1sbox(int)));
}
void CExplorerWindow::Vaa3D_changeXCut0(int s)
{
    disconnect(PMain::instance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
    PMain::instance()->H0_sbox->setValue(getHighestResGlobalHCoord(s)+1);
    connect(PMain::instance()->H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH0sbox(int)));
}
void CExplorerWindow::Vaa3D_changeXCut1(int s)
{
    disconnect(PMain::instance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
    PMain::instance()->H1_sbox->setValue(getHighestResGlobalHCoord(s)+1);
    connect(PMain::instance()->H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeH1sbox(int)));
}
void CExplorerWindow::Vaa3D_changeZCut0(int s)
{
    disconnect(PMain::instance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
    PMain::instance()->D0_sbox->setValue(getHighestResGlobalDCoord(s)+1);
    connect(PMain::instance()->D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD0sbox(int)));
}
void CExplorerWindow::Vaa3D_changeZCut1(int s)
{
    disconnect(PMain::instance()->D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(PMain_changeD1sbox(int)));
    PMain::instance()->D1_sbox->setValue(getHighestResGlobalDCoord(s)+1);
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


