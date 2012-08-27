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
//#include "newmat.h"

using namespace teramanager;

CExplorerWindow* CExplorerWindow::first = NULL;

CExplorerWindow::CExplorerWindow(V3DPluginCallback2 *_V3D_env, int _resIndex, uint8 *imgData, int _volV0, int _volV1,
                                 int _volH0, int _volH1, int _volD0, int _volD1, CExplorerWindow *_prev) : QWidget()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> CExplorerWindow[\"multires_%d\"] created\n", this->thread()->currentThreadId(), _resIndex);
    #endif

    //initializations
    this->V3D_env = _V3D_env;
    this->prev = _prev;
    this->next = NULL;
    this->volResIndex = _resIndex;
    this->title = QString("multires_").append(QString::number(volResIndex)).toStdString();
    this->volV0 = _volV0;
    this->volV1 = _volV1;
    this->volH0 = _volH0;
    this->volH1 = _volH1;
    this->volD0 = _volD0;
    this->volD1 = _volD1;
    this->toBeClosed = false;

    //opening tri-view window
    this->window = V3D_env->newImageWindow(QString(title.c_str()));
    Image4DSimple* image = new Image4DSimple();
    image->setFileName(title.c_str());
    image->setData(imgData, volH1-volH0, volV1-volV0, volD1-volD0, 1, V3D_UINT8);
    V3D_env->setImage(window, image);
    this->triViewWidget = (XFormWidget*)window;

    //opening 3D view window and hiding the tri-view window
    V3D_env->open3DWindow(window);
    view3DWidget = (V3dR_GLWidget*)(V3D_env->getView3DControl(window));
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
        window3D->move(location);

        //hiding both tri-view and 3D view
        prev->triViewWidget->setVisible(false);
        prev->window3D->setVisible(false);
        prev->view3DWidget->setCursor(Qt::ArrowCursor);

        //registrating views
        float ratio = CImport::instance()->getVolume(volResIndex)->getDIM_D()/CImport::instance()->getVolume(volResIndex-1)->getDIM_D();
        view3DWidget->setZoom(prev->view3DWidget->zoom()/ratio);
        view3DWidget->setXRotation(prev->view3DWidget->xRot());
        view3DWidget->setYRotation(prev->view3DWidget->yRot());
        view3DWidget->setZRotation(prev->view3DWidget->zRot());
    }
    //otherwise centering the current 3D window and the plugin's window
    else
    {
        PMain* pMain = PMain::instance();
        int screen_height = qApp->desktop()->availableGeometry().height();
        int screen_width = qApp->desktop()->availableGeometry().width();
        int window_x = (screen_width - (window3D->width() + PMain::instance()->width()))/2;
        int window_y = (screen_height - window3D->height()) / 2;
        window3D->move(window_x, window_y);
        pMain->move(window_x + window3D->width() + 3, window_y);
        pMain->setMaximumHeight(std::max(window3D->height(),pMain->height()));
        pMain->resize(pMain->width(), window3D->height());
    }


    connect(CVolume::instance(), SIGNAL(sendOperationOutcome(MyException*,void*)), this, SLOT(loadingDone(MyException*,void*)), Qt::QueuedConnection);
}

CExplorerWindow::~CExplorerWindow()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] destroyed\n", this->thread()->currentThreadId(), title.c_str() );
    #endif

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
               markers.size() == 1              &&              //only one marker exists
               CImport::instance()->getVolume(volResIndex+1))   //the next resolution exists
            {
                switchToHigherRes(markers.first().x, markers.first().y, markers.first().z);
                markers.clear();
                V3D_env->setLandmark(triViewWidget, markers);
                view3DWidget->getRenderer()->updateLandmark();
            }
            else if(view3DWidget->zoom() < 0    &&              //zoom-out threshold reached
                    prev                        &&              //the previous resolution exists
                    !prev->toBeClosed)                          //the previous resolution does not have to be closed
            {
                prev->toBeClosed = true;
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
        else if(event->type()==QEvent::Close)
        {
            if(prev                        &&                   //the previous resolution exists
               !prev->toBeClosed)                               //the previous resolution does not have to be closed)
            {
                event->ignore();
                prev->toBeClosed = true;
                prev->restore();
                return true;
            }
            else if(!prev)
                PMain::instance()->close();
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

        this->next = new CExplorerWindow(this->V3D_env, volResIndex+1,
                                         cVolume->getVoiData(), cVolume->getVoiV0(), cVolume->getVoiV1(),cVolume->getVoiH0(),cVolume->getVoiH1(),
                                         cVolume->getVoiD0(),cVolume->getVoiD1(), this);

        view3DWidget->setCursor(Qt::ArrowCursor);
    }

    //resetting some widgets
    PMain::instance()->resetGUI();
    PMain::instance()->subvol_panel->setEnabled(true);
    PMain::instance()->loadButton->setEnabled(true);
}

/**********************************************************************************
* Switches to the higher resolution at the given 3D point.
* Called by the current <CExplorerWindow> when the user zooms in and the higher res-
* lution has to be loaded.
***********************************************************************************/
void CExplorerWindow::switchToHigherRes(int x, int y, int z)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] switchToHigherRes(x = %d, y = %d, z = %d) launched\n",
           this->thread()->currentThreadId(),title.c_str(), x, y, z );
    #endif

    //preparing GUI
    view3DWidget->setCursor(Qt::WaitCursor);
    PMain& pMain = *(PMain::instance());
    pMain.progressBar->setEnabled(true);
    pMain.progressBar->setMinimum(0);
    pMain.progressBar->setMaximum(0);
    pMain.loadButton->setEnabled(false);
    pMain.import_form->setEnabled(false);
    pMain.statusBar->showMessage("Zooming in to the higher resolution...");

    //obtaining VOI at the current resolution
    float ratio = CImport::instance()->getVolume(volResIndex+1)->getDIM_D()/CImport::instance()->getVolume(volResIndex)->getDIM_D();
    int VoiCenterX = (x + volH0)*ratio;
    int VoiCenterY = (y + volV0)*ratio;
    int VoiCenterZ = (z + volD0)*ratio;
    CVolume::instance()->setVoi(this, volResIndex+1, VoiCenterY-pMain.Vdim_sbox->value()/2, VoiCenterY+pMain.Vdim_sbox->value()/2,
                                                     VoiCenterX-pMain.Hdim_sbox->value()/2, VoiCenterX+pMain.Hdim_sbox->value()/2,
                                                     VoiCenterZ-pMain.Ddim_sbox->value()/2, VoiCenterZ+pMain.Ddim_sbox->value()/2);

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
        window3D->move(location);

        //registrating views
        //float ratio = CImport::instance()->getVolume(volResIndex)->getDIM_D()/CImport::instance()->getVolume(volResIndex+1)->getDIM_D();
        //view3DWidget->setZoom(next->view3DWidget->zoom()*ratio);
        view3DWidget->setXRotation(next->view3DWidget->xRot());
        view3DWidget->setYRotation(next->view3DWidget->yRot());
        view3DWidget->setZRotation(next->view3DWidget->zRot());

        //closing next
        delete next;
        next = 0;
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


