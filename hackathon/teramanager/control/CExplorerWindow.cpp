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
#include "renderer_gl1.h"
#include "v3dr_mainwindow.h"
#include "control/CVolume.h"
#include "presentation/PMain.h"

using namespace teramanager;

CExplorerWindow* CExplorerWindow::first = NULL;

CExplorerWindow::CExplorerWindow(V3DPluginCallback2 *_V3D_env, int _resIndex, const char *title, uint8 *imgData, int imgDimV, int imgDimH, int imgDimD, CExplorerWindow *_prev) : QWidget()
{
    #ifdef TMP_DEBUG
    printf("teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] created\n", title, this->thread()->currentThreadId());
    #endif

    //initializations
    this->V3D_env = _V3D_env;
    this->prev = _prev;
    this->next = NULL;
    this->resIndex = _resIndex;

    //opening tri-view window
    this->window = V3D_env->newImageWindow(QString(title));
    Image4DSimple* image = new Image4DSimple();
    image->setFileName(title);
    image->setData(imgData, imgDimH, imgDimV, imgDimD, 1, V3D_UINT8);
    V3D_env->setImage(window, image);
    this->triViewWidget = (XFormWidget*)window;

    //opening 3D view window and hiding the tri-view window
    V3D_env->open3DWindow(window);
    this->view3DWidget = (V3dR_GLWidget*)(V3D_env->getView3DControl(window));
    this->window3D = view3DWidget->getiDrawExternalParameter()->window3D;
    triViewWidget->setWindowState(Qt::WindowMinimized);

    //installing the event filter on the 3D renderer
    view3DWidget->installEventFilter(this);

    //if the previous explorer window exists
    if(prev)
    {
        //positioning the current 3D window exactly at the previous window position
        QPoint location = prev->window3D->pos();
        prev->window3D->setVisible(false);
        window3D->move(location);

        //and hiding both tri-view and 3D view
        prev->triViewWidget->setVisible(false);
        prev->window3D->setVisible(false);
    }
    //otherwise centering the current 3D window
    else
        window3D->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,window3D->size(),qApp->desktop()->availableGeometry()));


    connect(CVolume::instance(), SIGNAL(sendOperationOutcome(MyException*,void*)), this, SLOT(loadingDone(MyException*)), Qt::QueuedConnection);
}

CExplorerWindow::~CExplorerWindow()
{
    #ifdef TMP_DEBUG
    printf("teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] destroyed\n",
           triViewWidget->windowTitle().toStdString().c_str(), this->thread()->currentThreadId());
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
        if (object == view3DWidget && event->type() == QEvent::Wheel)
        {
            LandmarkList markers =  V3D_env->getLandmark(triViewWidget);

            if(view3DWidget->zoom() > 30 && markers.size() == 1)
            {
                //printf("Zooming-in at %.0f %.0f %.0f\n", markers.first().y, markers.first().x, markers.first().z);
                //view3DWidget->setCursor(Qt::WaitCursor);
                PMain& pMain = *(PMain::instance());
                pMain.progressBar->setEnabled(true);
                pMain.progressBar->setMinimum(0);
                pMain.progressBar->setMaximum(0);
                pMain.loadButton->setEnabled(false);
                pMain.import_form->setEnabled(false);
                pMain.statusBar->showMessage("Zooming in to the highest resolution...");

                /*float zratio = CImport::instance()->getMapZoominRatio();
                CVolume::instance()->setVOI(markers.first().y*zratio-Vdim_sbox->value()/2, markers.first().y*zratio+Vdim_sbox->value()/2,
                                                   markers.first().x*zratio-Hdim_sbox->value()/2, markers.first().x*zratio+Hdim_sbox->value()/2,
                                                   markers.first().z*zratio-Ddim_sbox->value()/2, markers.first().z*zratio+Ddim_sbox->value()/2);*/
                markers.clear();
                V3D_env->setLandmark(triViewWidget, markers);
                view3DWidget->getRenderer()->updateLandmark();
                CVolume::instance()->start();
            }
            else if(view3DWidget->zoom() < 0)
            {
                /*view3DWidget->setCursor(Qt::WaitCursor);
                view3DWidget->getiDrawExternalParameter()->image4d->setRawDataPointerToNull();
                view3DWidget->getiDrawExternalParameter()->image4d->setData(CImport::instance()->getVMap(),
                                                                            CImport::instance()->getVMapWidth(),
                                                                            CImport::instance()->getVMapHeight(),
                                                                            CImport::instance()->getVMapDepth(), 1, V3D_UINT8);
                view3DWidget->updateImageData();
                CVolume::instance()->reset();
                view3DWidget->setCursor(Qt::ArrowCursor);
                view3DWidget->getView3DControl()->setZoom(CImport::instance()->getMapZoominRatio()*10);*/
            }
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
    printf("teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] loadingDone(%s) launched\n",
           triViewWidget->windowTitle().toStdString().c_str(), this->thread()->currentThreadId(), (ex? "ex" : "NULL"));
    #endif

    CVolume* cVolume = CVolume::instance();

    //if an exception has occurred, showing a message error
    if(ex)
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
    else if(sourceObject == this)
    {

        this->next = new CExplorerWindow(this->V3D_env, resIndex+1, QString("multires_").append(QString::number(resIndex+1)).toStdString().c_str(),
                                         cVolume->getVoiData(), cVolume->getVoiV1()-cVolume->getVoiV0(),cVolume->getVoiH1()-cVolume->getVoiH0(),
                                         cVolume->getVoiD1()-cVolume->getVoiD0(), this);


        //----
        //view3DWidget->setCursor(Qt::ArrowCursor);
        //view3DWidget->setZoom(view3DWidget->zoom()/CImport::instance()->getMapZoominRatio());
    }

    //resetting some widgets
    PMain::instance()->resetGUI();
    PMain::instance()->subvol_panel->setEnabled(true);
    PMain::instance()->loadButton->setEnabled(true);
}
