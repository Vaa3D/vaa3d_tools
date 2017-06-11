/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */
/************
                                            ********* LICENSE NOTICE ************
This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.
You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.
1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.
2. You agree to appropriately cite this work in your related studies and publications.
Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )
Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )
3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.
*************/
/****************************************************************************
***
   by Hanchuan Peng
   Feb 28, 2006
   July 21, 2006
   060923: v3d v0.12
March 13, 2008: add an atlas view button. function to be added later.
March 18, 2008: add an import tif series function
June 8, 2008: add v3d_aboutinfo()
June 13: add menu items and funcitons to image process modules
June 18: correct an error of incorrect menu items
July 20, 2008: change thr oder of main menu
July 23, 2008: add a control to compile and add some processing functions _BLOCK_ADVANCE_PROCESSING_MENU_
July 28, 2008: add mask channel menu
Aug, 27, 2008: add drop item support
Aug 29, 2008: there is a remaining bug in update processing menu for images with different numbers of channels
Sept 25, 2008: add open window fail exception catch
Sept 30, 2008: disable  open in the same window function, also add flip image function
 Oct 2, 2008: add direct open of apo, swc, etc
 Oct 16, 2008: add the ano file support
 Nov 24, 2008: add the atlas file support
 April  2009: add preference menu item
 May 16, 2009: add import to atlas file menu and adjust other menus
 May 17, 2009: add import point cloud atlas menu
 Oct 6, 2010: CMB add web URL open method
**
****************************************************************************/
#include "../3drenderer/v3dr_common.h"
#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
  #include <QtWidgets>
#else
  #include <QtGui>
#endif
#include "mainwindow.h"
#include "v3d_application.h"
#include "v3d_version_info.h"
#include "../basic_c_fun/basic_surf_objs.h"
#include "../plugin_loader/v3d_plugin_loader.h"
#include "v3d_core.h"
#include "../3drenderer/v3dr_mainwindow.h"
#include "import_filelist_dialog.h"
#include "import_images_tool_dialog.h"
#include "DownloadManager.h" // CMB 08-Oct-2010
#include "mapview.h"

#include "../imaging/v3d_imaging.h"

#ifdef __v3d_custom_toolbar__
#include "../custom_toolbar/v3d_custom_toolbar.h" // Hang Aug-08-2011
#endif

#ifdef _ALLOW_TERAFLY_MENU_
#include "../terafly/src/control/CPlugin.h"
#endif

//#include "dialog_pointcloudatlas_linkerloader.h"
//#include "atlas_window.h"
MainWindow::MainWindow()
{
    //initialize every pointer to 0. added on 080612
    workspace = 0;
    windowMapper = 0;
    fileMenu = 0;
    editMenu = 0;
    windowMenu = 0;
#ifdef _ALLOW_WORKMODE_MENU_
    modeMenu = 0;
#endif
    helpMenu = 0;
    proc_export_menu = 0;
    proc_import_menu = 0;
    basicProcMenu = advancedProcMenu = pipelineProcMenu = visualizeProcMenu = pluginProcMenu= 0;
    proc_datatype_menu = proc_geometry_transform_menu = proc_intensity_transform_menu = proc_colorchannel_transform_menu = proc_landmark_control_menu = 0;
    proc_general_menu = 0;
    proc_segmentation_menu = 0;
    proc_tracing_menu = 0;
    proc_registration_menu = 0;
    proc_standarization_menu = 0;
    proc_pointcloud_atlas_menu = 0;
    proc_image_atlas_menu = 0;
    proc_automarker_menu = 0;
    fileToolBar = 0;
    editToolBar = 0;
    newAct = 0;
    openAct = 0;
    openWebUrlAct = 0; // CMB Oct-07-2010
#ifdef __v3d_custom_toolbar__
    customToolbarAct = 0;
#endif
    atlasViewAct = 0;
    saveAct = 0;
    saveAsAct = 0;
    separator_RecentFiles_Act = 0;
    exitAct = 0;
    cutAct = 0;
    copyAct = 0;
    pasteAct = 0;
    closeAct = 0;
    closeAllAct = 0;
    tileAct = 0;
    cascadeAct = 0;
    arrangeAct = 0;
    nextAct = 0;
    previousAct = 0;
    separator_ImgWindows_Act = 0;
    checkForUpdatesAct = 0;
    generateVersionInfoAct = 0;
    aboutAct = 0;
    procLandmarkManager = 0;
    procAtlasViewer = 0;
    proc3DViewer = 0;
	procVRViewer = 0;//wwbchange
    proc3DLocalRoiViewer = 0;
    procSettings = 0;
    proc_plugin_manager = 0;
    procIO_export_to_vano_format = 0;
    procIO_export_to_movie = 0;
    procIO_export_landmark_to_pointcloud=0;
    procIO_export_landmark_to_swc=0;
    procIO_export_tracedneuron_to_swc=0;
    import_GeneralImageFileAct = 0;
    import_LeicaAct = 0;
    procIO_import_atlas_imgfolder = 0;
    procIO_import_atlas_apofolder = 0;
    procPC_Atlas_edit_atlaslinkerfile = 0;
    procPC_Atlas_create_atlaslinkerfile = 0;
    procPC_Atlas_view_atlas = 0;
    procPC_Atlas_view_atlas_computeVanoObjStat = 0;
    procGeneral_rotate_paxis = 0;
    procGeneral_rotate_angle = 0;
    procGeneral_flip = 0;
    procGeneral_clear_all_landmark = 0;
    procGeneral_toggle_landmark_label = 0;
    procGeneral_clear_connectmap = 0;
    procGeneral_rescale_landmarks_only = 0;
    procGeneral_automarker_entireimg = 0;
    procGeneral_automarker_roi = 0;
    procGeneral_split_channels = 0;
    procGeneral_extract_a_channel = 0;
    procGeneral_crop_image_minMaxBox = 0;
    procGeneral_crop_bbox_roi = 0;
    procGeneral_mask_roi = 0;
    procGeneral_mask_nonroi_xy = 0;
    procGeneral_mask_channel = 0;
    procGeneral_clear_roi = 0;
    procGeneral_resample_image = 0;
    procGeneral_projection_max = 0;
    procGeneral_blend_image = 0;
    procGeneral_stitch_image = 0;
    procGeneral_display_histogram = 0;
    procGeneral_linear_adjustment = 0;
    procGeneral_histogram_equalization = 0;
    procGeneral_intensity_rescale = 0;
    procGeneral_intensity_threshold = 0;
    procGeneral_intensity_binarize = 0;
    procGeneral_intensity_updateminmax = 0;
    procGeneral_color_invert = 0;
    procGeneral_16bit_to_8bit = 0;
    procGeneral_32bit_to_8bit = 0;
    procGeneral_scaleandconvert28bit = 0;
    procGeneral_scaleandconvert28bit_1percent = 0;
    procGeneral_indexedimg2rgb = 0;
    //	procGeneral_open_image_in_windows = 0;
    procGeneral_save_image = 0;
    procElongated_randomSeeding = 0;
    procElongated_minSpanTree = 0;
    procElongated_mstDiameter = 0;
    procElongated_genCuttingPlaneLocations = 0;
    procElongated_restackingCuttingPlanes = 0;
    procElongated_bdbminus = 0;
    procReg_gridSeeding = 0;
    procReg_randomSeeding = 0;
    procReg_bigGradient_edge_Seeding = 0;
    procReg_big_curvature_corner_Seeding = 0;
    procReg_fileSeeding = 0;
    procReg_global_align = 0;
    procReg_affine_markers_align = 0;
    procReg_flybrain_lobeseg = 0;
    procReg_detect_matching = 0;
    procReg_detect_matching_1pt = 0;
    procReg_warp_using_landmarks = 0;
    procReg_all_in_one_warp = 0;
    procTracing_topdownSkeleton = 0;
    procTracing_bottomupSearch = 0;
    procTracing_glocal_combine = 0;
    procTracing_manualCorrect = 0;
    procTracing_APP2auto = 0;
    procTracing_one2others = 0;
    procTracing_trace_a_curve = 0;
    procTracing_undo_laststep = 0;
    procTracing_redo_laststep = 0;
    procTracing_clear = 0;
    procTracing_update3Dview = 0;
    procTracing_save = 0;
    procCellSeg_localTemplate = 0;
    procCellSeg_cellcounting = 0;
    procCellSeg_watershed = 0;
    procCellSeg_levelset = 0;
    procCellSeg_Gaussian_fit_1_spot_1_Gauss = 0;
    procCellSeg_Gaussian_fit_1_spot_N_Gauss = 0;
    procCellSeg_Gaussian_partition = 0;
    procCellSeg_manualCorrect = 0;
#ifdef _ALLOW_WORKMODE_MENU_
    // Mode
    procModeDefault = 0;
    procModeNeuronAnnotator = 0;
#endif
    setup_global_imgproc_parameter_default(); //set up the default parameter for some of the global parameters of image processing or viewing
    //set the drop function
    setAcceptDrops(true); //080827
    //

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
    workspace = new QMdiArea;
#else
    workspace = new QWorkspace;
#endif
    setCentralWidget(workspace);
    connect(workspace, SIGNAL(windowActivated(QWidget *)),  this, SLOT(updateMenus()));
    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget *)),  workspace, SLOT(setActiveWindow(QWidget *)));
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    updateMenus();
    readSettings();
    setWindowTitle(tr("Vaa3D"));
#if defined(__V3DWSDEVELOP__)
    v3dws = new V3DWebService(9125); //20110309 YuY
    initWebService(v3dws);
    connect(v3dws, SIGNAL(webserviceRequest()), this, SLOT(webserviceResponse()), Qt::QueuedConnection); // Qt::AutoConnection
#endif
#if COMPILE_TARGET_LEVEL == 0
    v3d_Lite_info();
#endif
    //090811 RZC
    pluginLoader = new V3d_PluginLoader(pluginProcMenu, this);
#ifdef __v3d_custom_toolbar__
    // Aug-08-2011 Hang
    this->addCustomToolbar();
#endif
    // Dec-20-2010 YuY
    //connect(&sub_thread, SIGNAL(transactionStarted()), this, SLOT(transactionStart()), Qt::DirectConnection); //Qt::QueuedConnection
    //connect(&sub_thread, SIGNAL(allTransactionsDone()), this, SLOT(allTransactionsDone()), Qt::DirectConnection);
    connect(this, SIGNAL(triviewUpdateTriggered()), this, SLOT(updateTriview()), Qt::QueuedConnection); // Qt::AutoConnection
    cl_plugin = false; // init
    connect(this, SIGNAL(imageLoaded2Plugin()), this, SLOT(updateRunPlugin())); // command line call plugin 20110426 YuY
}
//void MainWindow::postClose() //090812 RZC
//{
//	qDebug("***v3d: MainWindow::postClose");
//	QCoreApplication::postEvent(this, new QEvent(QEvent::Close)); // this OK
//}


MainWindow::~MainWindow()
{
    qDebug("***vaa3d: ~MainWindow");
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    //if (workspace)  workspace->deleteLater(); //110802 RZC //will call ~XFormView to raise BAD_ACCESS
    disconnect(workspace, SIGNAL(windowActivated(QWidget *)),  this, SLOT(updateMenus())); //instead of above line
    V3dApplication::handleCloseEvent(event);
}
void MainWindow::transactionStart()
{
    v3d_msg("Transaction starts now ...", 0);
}
void MainWindow::allTransactionsDone()
{
    v3d_msg("All transactions are done successfully.", 0);
}
void MainWindow::exit()
{
    emit closeAct->activate(QAction::Trigger);
    emit closeAllAct->activate(QAction::Trigger);
    emit exitAct->activate(QAction::Trigger);
}
void MainWindow::updateTriviewWindow()
{
    emit triviewUpdateTriggered();
}
void MainWindow::updateTriview()
{
    qDebug()<<"triggered in MainWindow ... ...";
    //	TriviewControl *tvControl = (TriviewControl *)(this->curHiddenSelectedWindow());
    //	if(tvControl)
    //	{
    //		// updateMinMax then changeFocus
    //		V3DLONG currslice = tvControl->getValidZslice();
    //		V3DLONG preslice = tvControl->getPreValidZslice();
    //
    //		qDebug()<<"the triview window exist ... ..."<<currslice<<preslice;
    //
    //		if(currslice>preslice)
    //		{
    //			qDebug()<<"update triview window ... ...";
    //
    //			tvControl->updateMinMax(currslice-1);
    //
    //			V3DLONG x, y, z;
    //			tvControl->getFocusLocation( x, y, z);
    //			tvControl->setFocusLocation( x, y, currslice);
    //
    //			tvControl->setPreValidZslice(currslice);
    //		}
    //
    //		QCoreApplication::processEvents();
    //		return;
    //	}
    //	else
    //	{
    //		printf("The pointer to triview window is NULL!\n");
    //		QCoreApplication::processEvents();
    //		return;
    //	}
    sub_thread.setPriority(QThread::HighPriority);
    if(this->curHiddenSelectedWindow())
    {
        sub_thread.addTransaction(new UpdateTVTransaction( (TriviewControl *)(this->curHiddenSelectedWindow()) ) );
    }
    else if(this->currentImageWindow())
    {
        sub_thread.addTransaction(new UpdateTVTransaction( (TriviewControl *)(this->currentImageWindow()) ) );
    }
}
void MainWindow::updateRunPlugin() //20110426 YuY
{
    if (cl_plugin)
    {
        cl_plugin = false; // make sure plugin run only once 20110502 YuY
        // match plugin name
        int numfind = 0; //20110429 YuY
        QString v3dpluginFind;
        QStringList existingPluginsList = pluginLoader->getPluginNameList();
        QString canonicalFilePath = QFileInfo(pluginname).canonicalFilePath();
        if (canonicalFilePath.size()==0)
            canonicalFilePath = pluginname; //this would be the case when the partial file name is given
        else
            canonicalFilePath = QFileInfo(pluginname).fileName();
        v3d_msg(QString("Current canonical path = [")+canonicalFilePath+"]", 0);
        foreach(QString qstr, existingPluginsList)
        {
            if (qstr==canonicalFilePath || QFileInfo(qstr).fileName() == canonicalFilePath) //20110429 YuY
            {
                v3dpluginFind = qstr;
                numfind++;
                //v3d_msg(QString("Now find [")+canonicalFilePath+"]", 0);
            }
        }
        if(numfind<=0) //20110427 YuY
        {
            // try find image name contains the input string from the end
            foreach(QString qstr, existingPluginsList)
            {
                if ( qstr.contains(canonicalFilePath) ||
                    //qstr.endsWith(canonicalFilePath) || //by PHC, 20120210
                    //QFileInfo(qstr).fileName().endsWith(canonicalFilePath)
                    QFileInfo(qstr).fileName().contains(canonicalFilePath)
                    ) //20110429 YuY
                {
                    v3dpluginFind = qstr;
                    numfind++;
                }
            }
        }
        if (numfind<=0)
        {
            qCritical()<<"Error: Vaa3D really cannot find this plugin. Do nothing.";
            return;
        }
        else if(numfind > 1)	//20110429 YuY
        {
            qCritical()<<QString("Error: Too many choices. Please specify your plugin with whole name including absolute path and try again.");
            return;
        }
        else if(numfind == 1)
        {
            QPluginLoader* loader = new QPluginLoader(v3dpluginFind);
            if (!loader)
            {
                v3d_msg(QString("ERROR open the specified Vaa3D plugin [%1]").arg(v3dpluginFind), 1);
                return;
            }

            QElapsedTimer timer_plugin;
            timer_plugin.start();

            // run plugin
            V3d_PluginLoader mypluginloader(this);
            // help info
            QObject *plugin = loader->instance();
            QStringList menulist = v3d_getInterfaceMenuList(plugin);
            QStringList funclist = v3d_getInterfaceFuncList(plugin);
            if(v3dclp.pluginhelp)
            {
                cout<<endl<<"plugin: "<<v3dpluginFind.toStdString().c_str()<<endl<<endl;
                cout<<"usage: "<<endl;
                cout<<"menu -- ";
                for (int i=0; i<menulist.size(); i++) {
                    if(i==0)
                    {
                        cout<<menulist.at(i).toStdString().c_str()<<endl;
                    }
                    else
                    {
                        cout<<"     -- "<<menulist.at(i).toStdString().c_str()<<endl;
                    }
                }
                cout<<"func -- ";
                for (int i=0; i<funclist.size(); i++) {
                    if(i==0)
                    {
                        cout<<funclist.at(i).toStdString().c_str()<<endl;
                    }
                    else
                    {
                        cout<<"     -- "<<funclist.at(i).toStdString().c_str()<<endl;
                    }
                }
                cout<<endl<<endl;
                return;
            }
            // run
            if(pluginmethod)
            {
                mypluginloader.runPlugin(loader, pluginmethod);
            }
            if(pluginfunc)
            {
                PLUGINFH pluginFuncHandler;
                pluginFuncHandler.doPluginFunc(v3dclp, mypluginloader, v3dpluginFind, (void *)this);
            }
            qint64 etime_plugin = timer_plugin.elapsed();
            qDebug() << " **** the plugin preprocessing takes [" << etime_plugin <<" milliseconds]";
            //uncommented version is only used for bench testing by Zhi Z, 20151103
//            if(v3dclp.fileList.size()>0)
//            {
//                QString timer_log = QString(v3dclp.fileList.at(0)) + "_" + QFileInfo(pluginname).baseName() + "_" + pluginfunc +"_time.log";
//                QFile file(timer_log);
//                if (!file.open(QFile::WriteOnly|QFile::Truncate))
//                {
//                    cout <<"Error opening the log file "<<timer_log.toStdString().c_str() << endl;
//                }

//                QTextStream stream (&file);
//                stream << "the plugin preprocessing takes\t"<< etime_plugin <<" milliseconds"<<"\n";
//                file.close();
//            }
        }
        else
        {
            v3d_msg(QString("The plugin [%1] does not exist! Do nothing.").arg(pluginname), 0);
            return;
        }
    }
}
void MainWindow::setBooleanCLplugin(bool cl_plugininput)
{
    cl_plugin = cl_plugininput;
}
void MainWindow::setPluginName(char *pluginnameinput)
{
    pluginname = pluginnameinput;
}
void MainWindow::setPluginMethod(char *pluginmethodinput)
{
    pluginmethod = pluginmethodinput;
}
void MainWindow::setPluginFunc(char *pluginfuncinput)
{
    pluginfunc = pluginfuncinput;
}
char *MainWindow::getPluginName()
{
    return pluginname;
}
char *MainWindow::getPluginMethod()
{
    return pluginmethod;
}
char *MainWindow::getPluginFunc()
{
    return pluginfunc;
}
void MainWindow::triggerRunPlugin()
{
    emit imageLoaded2Plugin();
}
void MainWindow::handleCoordinatedCloseEvent_real() {
    // qDebug("***vaa3d: MainWindow::closeEvent");
    writeSettings(); //added on 090501 to save setting (default preferences)
    foreach (V3dR_MainWindow* p3DView, list_3Dview_win)
    {
        if (p3DView)
        {
            p3DView->postClose(); //151117. PHC
//        v3d_msg("haha");
        }
    }
    //exit(1); //this is one bruteforce way to disable the strange seg fault. 080430. A simple to enhance this is to set a b_changedContent flag indicates if there is any unsaved edit of an image,

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
    workspace->closeAllSubWindows();
#else
    workspace->closeAllWindows();
#endif
}
void MainWindow::handleCoordinatedCloseEvent(QCloseEvent *event)
{
    handleCoordinatedCloseEvent_real();
    if (activeMdiChild())
    {
        event->ignore();
        return; //090812 RZC
    }
    else
    {
        //writeSettings();
        event->accept();
    }
}
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    //setText(tr("<drop content>"));
    setBackgroundRole(QPalette::Highlight);
    event->acceptProposedAction();
    //emit changed(event->mimeData());
}
void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}
void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    //clear();
    event->accept();
}
void MainWindow::dropEvent(QDropEvent *event)
{
    QString fileName;
    qDebug("Vaa3D MainWindow::dropEvent");
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasImage())
    {
        qDebug() <<tr("  drop Image data");
    }
    else if (mimeData->hasHtml())
    {
        qDebug() <<tr("  drop Html data");
    }
    else if (mimeData->hasText())
    {
        qDebug() <<tr("  drop Text data: ")+(mimeData->text());
        fileName = mimeData->text().trimmed();
#ifdef Q_OS_LINUX
        fileName.remove(0,7); // remove the first 'file://' of the name string, 09012581102
#endif
        qDebug("the file to open=[%s]",qPrintable(fileName));
    }
    else if (mimeData->hasUrls())
    {
        QList<QUrl> urlList = mimeData->urls();
        for (int i = 0; i < urlList.size() && (i < 1); ++i)
        {
            QString url = urlList.at(i).path().trimmed();
            qDebug() <<tr("  drop Url data: ")+url;
#ifdef WIN32
            url.remove(0,1); // remove the first '/' of "/C:/...", 081102
#endif

// @FIXED by Alessandro on 2015-05-09. Call method to fix the file-based URL (if any)
#ifdef Q_OS_MAC

#ifdef __TEST_DROP_QT5_MAC_
            if (urlList.at(i).path().startsWith("file:///.file/id=")) {
                    QUrl url(urlList.at(i).path());
                    CFURLRef cfurl = url.toCFURL();
                    CFErrorRef error = 0;
                    CFURLRef absurl = CFURLCreateFilePathURL(kCFAllocatorDefault, cfurl, &error);
                    url = QUrl::fromCFURL(absurl);
                    CFRelease(cfurl);
                    CFRelease(absurl);
                }
#endif

#ifdef _ENABLE_MACX_DRAG_DROP_FIX_
            if (urlList.at(i).path().startsWith("/.file/id="))
                url = getPathFromYosemiteFileReferenceURL(urlList.at(i));
#endif
#endif

            fileName = url;
            qDebug() <<tr("  the file to open: [")+ fileName +("]");
        }
        event->acceptProposedAction();
    }
    else
    {
        qDebug() <<tr("  Unknown drop data");
    }
#ifdef Q_OS_LINUX
    fileName.replace("%20"," ");//fixed the space path issue on Linux machine by Zhi Zhou May 14 2015
#endif

    //
    if (!QFile::exists(fileName))
    {
        v3d_msg(QString("The file [%1] specified does not exist").arg(fileName));
        return;
    }
    loadV3DFile(fileName, true, global_setting.b_autoOpenImg3DViewer); // loadV3DFile func changed to 3 args. YuY Nov. 18, 2010
    setBackgroundRole(QPalette::Dark);
    event->acceptProposedAction();
}
void MainWindow::newFile()
{
    XFormWidget *child = createMdiChild();
    //child->newFile();
    //child->setOpenFileName();
    child->show();
}
void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    loadV3DFile(fileName, true, global_setting.b_autoOpenImg3DViewer); // loadV3DFile func changed to 3 args. YuY Nov. 18, 2010. change false to global_setting.b_autoOpenImg3DViewer. 2011-02-09, PHC
}
// By CMB Oct-08-2010
// Ask user for URL to download image stack from.
void MainWindow::openWebUrl()
{
    // testing Oct-18-2010
    DownloadManager *downloadManager = new DownloadManager(this);
    // Pop a dialog to ask the user for URL
    QUrl imageUrl = downloadManager->askUserForUrl();
    bool b_cacheLocalFile = downloadManager->b_cacheFile;
    if (! imageUrl.isValid())
        return; // User entered nothing or error
    loadV3DUrl(imageUrl, b_cacheLocalFile);
}
#ifdef __v3d_custom_toolbar__
// By Hang Aug-06-2011
void MainWindow::addCustomToolbar()
{
    //QMessageBox::information(0,"","Waiting ..");
    static int bar_num = 1;
    static bool isFirstLoading = true;
    QList<CustomToolbarSetting*> & settingList = getToolBarSettingList();
    if(isFirstLoading)
    {
        //setPluginRootPath(QObject::tr("/Users/xiaoh10/Applications/v3d/plugins"));
        setPluginRootPathAutomaticly();
        //setToolbarSettingFilePath(QObject::tr("/Users/xiaoh10/.v3d_toolbox_layout"));
        setToolbarSettingFilePathAutomaticly();
        // loadToolBarSettings will return to  settingList
        //if((!settingList.empty()) && loadToolBarSettings())
        if(loadToolBarSettings() && (!settingList.empty()))
        {
            bar_num = settingList.size() + 1;
            foreach(CustomToolbarSetting* cts, settingList)
            {
                CustomToolbar * ct = new CustomToolbar(cts, this->pluginLoader, this); // 20120705 Hang, set parent = this
                if(!ct->showToMainWindow(this)) ct->show();
            }
        }
    }
    else
    {
        QString barTitle = (bar_num > 1) ? QObject::tr("Custom Toolbar - %1").arg(bar_num) : QObject::tr("Custom Toolbar");
        CustomToolbar * ct = new CustomToolbar(barTitle, this->pluginLoader, 0/*this*/);
        if(!ct->showToMainWindow(this)) ct->show();
        settingList.push_back(ct->cts);
        bar_num++;
    }
    isFirstLoading = false;
}
#endif
void MainWindow::loadV3DUrl(QUrl url, bool b_cacheLocalFile, bool b_forceopen3dviewer)
{
    QString localFileName = QFileInfo(url.path()).fileName();
    QString localFilePath = QDir::tempPath();
    QString fileName = localFilePath + "/" + localFileName;
    DownloadManager *downloadManager = new DownloadManager(this);
    connect(downloadManager, SIGNAL(downloadFinishedSignal(QUrl, QString, bool, bool)),
            this, SLOT(finishedLoadingWebImage(QUrl, QString, bool, bool)));
    downloadManager->b_cacheFile = b_cacheLocalFile;
    downloadManager->b_forceopen3dviewer = b_forceopen3dviewer;
    if (b_cacheLocalFile)
        downloadManager->startDownloadCheckCache(url, fileName);
    else
        downloadManager->startDownload(url, fileName);
}

// This method is called once an asynchronous web download has completed.
// By CMB Oct-08-2010
void MainWindow::finishedLoadingWebImage(QUrl url, QString fileName, bool b_cacheLocalFile, bool b_forceopen3dviewer)
{
    // Empty file name means something went wrong
    if (fileName.size() > 0) {
        // false means Don't add local file name to recent files list
        loadV3DFile(fileName, false, b_forceopen3dviewer); // loadV3DFile func changed to 3 args. YuY Nov. 18, 2010
        if (! b_cacheLocalFile)
            QFile::remove(fileName); // delete downloaded file
        // Set window title to URL
        XFormWidget *image_window = findMdiChild(fileName);
        if (image_window)
            image_window->setWindowTitle(url.toString());
        // Put URL in recent file list
        setCurrentFile(url.toString());
        emit imageLoaded2Plugin(); //20110426 YuY
    }
}

void MainWindow::checkForUpdates(bool b_verbose)
{
    // In interactive mode, bring up a pre-dialog where user can set update frequency
    if (b_verbose) {
        v3d::CheckForUpdatesDialog dialog(this);
        dialog.exec();
    }
    // In automatic mode, just check for updates
    else {
        v3d::V3DVersionChecker *versionChecker = new v3d::V3DVersionChecker(this);
        versionChecker->checkForLatestVersion(b_verbose);
    }
}
void MainWindow::generateVersionInfo()
{
    v3d::V3DVersionChecker *versionChecker = new v3d::V3DVersionChecker(this);
    versionChecker->createVersionXml();
}

V3dR_MainWindow * MainWindow::find3DViewer(QString fileName)
{
    int numfind = 0; //20110427 YuY
    V3dR_MainWindow * v3dRMWFind;
    // support image with relative path
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath(); //20110427 YuY
    if (canonicalFilePath.size()==0) canonicalFilePath = fileName; //20110427 YuY
    for (int i=0; i<list_3Dview_win.size(); i++)
    {
        if (list_3Dview_win.at(i)->getDataTitle()==canonicalFilePath || QFileInfo(list_3Dview_win.at(i)->getDataTitle()).fileName() == canonicalFilePath) //20110427 YuY
        {
            v3dRMWFind = list_3Dview_win.at(i);
            numfind++;
        }
    }
    if(!numfind) //20110427 YuY
    {
        // try find image name contains the input string from the end
        for (int i=0; i<list_3Dview_win.size(); i++)
        {
            if ( list_3Dview_win.at(i)->getDataTitle().endsWith(canonicalFilePath) ||
                QFileInfo(list_3Dview_win.at(i)->getDataTitle()).fileName().endsWith(canonicalFilePath) ) //20110427 YuY
            {
                v3dRMWFind = list_3Dview_win.at(i);
                numfind++;
            }
        }
    }
    if(numfind > 1)	//20110427 YuY
    {
        v3d_msg(QString("Too many windows sharing the same [partial] name. Please specify your image with whole name including absolute path and try again."), 1);
        return 0;
    }
    else if(numfind == 1)
    {
        return v3dRMWFind;
    }
    else
    {
        return 0;
    }
}
void MainWindow::loadV3DFile(QString fileName, bool b_putinrecentfilelist, bool b_forceopen3dviewer)
{
    if (!fileName.isEmpty())
    {
        XFormWidget *existing_imgwin = findMdiChild(fileName);
        if (existing_imgwin)
        {

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
            workspace->setActiveSubWindow(existing_imgwin);
#else
            workspace->setActiveWindow(existing_imgwin);
#endif
            return;
        }
        V3dR_MainWindow *existing_3dviewer = find3DViewer(fileName);
        if (existing_3dviewer)
        {
            existing_3dviewer->activateWindow();
            return;
        }

        //

        QFileInfo curfile_info(fileName);

        QString cur_suffix = curfile_info.suffix().toUpper();

        if (cur_suffix=="ANO")
        {
            P_ObjectFileType cc;
            if (!loadAnoFile(fileName, cc))
            {
                v3d_msg("Fail to load useful info from the specified anofile. Do nothing.\n");
                return;
            }
            XFormWidget *child_rawimg=0, *child_maskimg=0; //090124. in this case, the last opened raw image will be used to associate the surface files
            if (cc.raw_image_file_list.size()>0)
            {
                for (int i=0;i<cc.raw_image_file_list.size();i++)
                {
                    try
                    {
                        child_rawimg = createMdiChild();
                        if (child_rawimg->loadFile(cc.raw_image_file_list.at(i))) {
                            statusBar()->showMessage(tr("File loaded [%1]").arg(cc.raw_image_file_list.at(i)), 2000);
                            if (global_setting.b_yaxis_up)
                            {
                                child_rawimg->getImageData()->flip(axis_y);
                            }
                            if ((b_forceopen3dviewer || (global_setting.b_autoOpenImg3DViewer))&& cc.pointcloud_file_list.size()<=00 && cc.swc_file_list.size()<=0 && cc.surface_file_list.size()<=0)
                            {
                                child_rawimg->doImage3DView();
                            }

                            child_rawimg->show();

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
                            workspace->cascadeSubWindows();
#else
                            workspace->cascade();
#endif
                            //setCurrentFile(fileName);
                        } else {
                            child_rawimg->close();
                        }
                    }
                    catch(...)
                    {
                        v3d_msg("You fail to open a new window for the specified image. The file may have certain problem, or is simply too big but you don't have enough memory.");
                        return;
                    }
                }
            }
            if (cc.labelfield_image_file_list.size()>0)
            {
                for (int i=0;i<cc.labelfield_image_file_list.size();i++)
                {
                    try
                    {
                        child_maskimg = createMdiChild();
                        if (child_maskimg->loadFile(cc.labelfield_image_file_list.at(i))) {
                            statusBar()->showMessage(tr("File loaded [%1]").arg(cc.labelfield_image_file_list.at(i)), 2000);
                            child_maskimg->show();

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
                            workspace->cascadeSubWindows();
#else
                            workspace->cascade();
#endif
                            //setCurrentFile(fileName);
                        } else {
                            child_maskimg->close();
                        }
                    }
                    catch(...)
                    {
                        v3d_msg("You fail to open a new window for the specified image. The file may have certain problem, or is simply too big but you don't have enough memory.");
                        return;
                    }
                }
            }
            if (cc.pointcloud_file_list.size()>0 || cc.swc_file_list.size()>0 || cc.surface_file_list.size()>0)
            {
                //directly open the 3D viewer
                iDrawExternalParameter * mypara_3Dview = 0;
                if (child_rawimg)
                {
                    mypara_3Dview = &(child_rawimg->mypara_3Dview);
                }
                else
                {
                    mypara_3Dview = new iDrawExternalParameter;
                }
                mypara_3Dview->p_list_3Dview_win = &list_3Dview_win; //always keep an record
                if (child_rawimg)
                {
                    mypara_3Dview->image4d = child_rawimg->getImageData();
                    mypara_3Dview->xwidget = child_rawimg;
                }
                else
                {
                    mypara_3Dview->image4d = 0;
                    mypara_3Dview->xwidget = 0;
                }
                mypara_3Dview->b_use_512x512x256 = true;
                mypara_3Dview->V3Dmainwindow = this; //added on 090503
                //set up data.
                int ii;
                for (ii=0;ii<cc.pointcloud_file_list.size();ii++)
                    mypara_3Dview->pointcloud_file_list.append(cc.pointcloud_file_list.at(ii));
                for (ii=0;ii<cc.swc_file_list.size();ii++)
                    mypara_3Dview->swc_file_list.append(cc.swc_file_list.at(ii));
                if (cc.surface_file_list.size()>0) //081016: at this moment I use only the first one
                    mypara_3Dview->surface_file = cc.surface_file_list.at(0);
                //			if (cc.labelfield_image_file_list.size()>0)
                //				mypara_3Dview->swc_file = cc.labelfield_image_file_list.at(0);
                //081016. Note: an interesting side-effect of this ano open function is that the RAW-image window will have "memory" of the associated files.
                //Therefore, when a 3D viewer win is closed, and re-open via clicking the "see in 3D", all the associated file will be opened again!
                V3dR_MainWindow *my3dwin = 0;
                try
                {
                    my3dwin = new V3dR_MainWindow(mypara_3Dview);
                    my3dwin->setParent(0);
                    my3dwin->setDataTitle(fileName);
                    my3dwin->show();
                    mypara_3Dview->window3D = my3dwin;
                    if (child_rawimg)
                    {
                        child_rawimg->mypara_3Dview.window3D = my3dwin;
                        child_rawimg->mypara_3Dview.image4d = mypara_3Dview->image4d;
                        child_rawimg->mypara_3Dview.b_use_512x512x256 = mypara_3Dview->b_use_512x512x256;
                        child_rawimg->mypara_3Dview.xwidget = mypara_3Dview->xwidget;
                        child_rawimg->mypara_3Dview.b_still_open = true;
                    }
                }
                catch (...)
                {
                    v3d_msg("You fail to open a 3D view window. You may have opened too many stacks (if so please close some first) or try to render a too-big 3D view (if so please contact Hanchuan Peng for a 64-bit version of Vaa3D).");
                    return;
                }
            }
        }
        else if (cur_suffix=="MARKER" ||  cur_suffix=="CSV")
                 //added by PHC, 20130420.
        {
            v3d_msg("Directly loading a marker or csv file into Vaa3D's main window is ambiguous. \n"
                    "You can either open it directly in a 3D viewer window of an image, or literally associate \n"
                    "it with an existing image, or literally convert it to a Point Cloud .APO file and then open.");
        }
        else if (cur_suffix=="APO" ||
                 cur_suffix=="SWC" ||
                 cur_suffix=="ESWC" ||
                 cur_suffix=="ASC" ||
                 cur_suffix=="OBJ" ||
                 cur_suffix=="VAA3DS" ||
                 cur_suffix=="V3DS" ||
                 cur_suffix=="NULL3DVIEWER")
        {
            //directly open the 3D viewer
            iDrawExternalParameter * mypara_3Dview = new iDrawExternalParameter;
            mypara_3Dview->p_list_3Dview_win = &list_3Dview_win; //always keep an record
            mypara_3Dview->image4d = 0;
            mypara_3Dview->b_use_512x512x256 = true;
            mypara_3Dview->xwidget = 0;
            mypara_3Dview->V3Dmainwindow = this; //added on 090503

            //set up data
            if (cur_suffix=="APO")
                mypara_3Dview->pointcloud_file_list.append(fileName);
            else if (cur_suffix=="SWC" ||
                     cur_suffix=="ESWC" ||
                     cur_suffix=="ASC" )
                mypara_3Dview->swc_file_list.append(fileName);
            else if (cur_suffix=="OBJ" ||
                     cur_suffix=="V3DS" ||
                     cur_suffix=="VAA3DS")
                mypara_3Dview->surface_file = fileName;
            else if (cur_suffix=="NULL3DVIEWER" || fileName=="NULL3DVIEWER")         //check if we should open a new empty 3D viewer. by PHC 2015-02-10
            {
                //do nothing
            }
            else
            {
                delete mypara_3Dview; mypara_3Dview=0; return;
            }

            //
            V3dR_MainWindow *my3dwin = 0;
            try
            {
                my3dwin = new V3dR_MainWindow(mypara_3Dview);
                my3dwin->setParent(0);
                my3dwin->setDataTitle(fileName);
                my3dwin->show();
                mypara_3Dview->window3D = my3dwin;
            }
            catch (...)
            {
                v3d_msg("You fail to open a 3D view window. You may have opened too many stacks (if so please close some first) or try to render a too-big 3D view (if so please contact Hanchuan Peng for a 64-bit version of Vaa3D).");
                return;
            }
            //list_3Dview_win.append(my3dwin); //081003: no longer need to do this here. I changed the V3dR_MainWindow so that when it create, it will add it into the list; and close the window, then it will delete itself from the list
        }
        else if (cur_suffix=="ATLAS")
        {
            try
            {
                //first read the atlas file list
                QList <InvidualAtlasFileInfo> cur_atlas_list = readAtlasFormatFile(qPrintable(fileName.trimmed()));
                if (cur_atlas_list.count()<=0)
                {
                    v3d_msg("Empty or invalid atlas file");
                    return;
                }
                //load the first image file in the file list, and only display it using RGB weight [255, 255, 255] and "on". all others set to [0,0,0] and "off"
                int kk=0;
                for (kk=0;kk<cur_atlas_list.count();kk++)
                    if (cur_atlas_list[kk].exist)
                        break;
                if (kk>=cur_atlas_list.count())
                {
                    v3d_msg("The atlas file contains list of files of which none can be opened.\n");
                    return;
                }
                XFormWidget *child = createMdiChild();
                if (child->loadFile(cur_atlas_list[kk].imgfile)) {
                    statusBar()->showMessage(tr("File loaded [%1]").arg(cur_atlas_list[kk].imgfile), 2000);
                    child->show();

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
                    workspace->cascadeSubWindows();
#else
                    workspace->cascade();
#endif
                    //update the image data listAtlasFiles member
                    cur_atlas_list[kk].on = true; //since this one has been opened
                    child->getImageData()->listAtlasFiles = cur_atlas_list;
                } else {
                    child->close();
                }
            }
            catch(...)
            {
                v3d_msg("You fail to open a new window for the specified image. The file may have certain problem, or is simply too big but you don't have enough memory.");
                return;
            }
        }
        else if (cur_suffix=="ZIP")
        {
            QString cmd_unzip = QString("unzip -o %1 -j -d %2").arg(fileName, curfile_info.canonicalPath());
            //v3d_msg(cmd_unzip);
            system(qPrintable(cmd_unzip));
            QString tmp_filename = fileName.left(fileName.size()-4);
            //v3d_msg(QString(tmp_filename).prepend("[").append("]"));
            if (QFile::exists(tmp_filename))
            {
                loadV3DFile(tmp_filename, false, global_setting.b_autoOpenImg3DViewer); //the 2nd parameter is false, so that the unzipped file will not be put into "Recent files" change the 3rd para to global_setting.b_autoOpenImg3DViewer, 2011-02-09, PHC
                system(qPrintable(QString("rm -f %1").arg(tmp_filename)));
            }
        }
        else if ( (cur_suffix=="LSM") ||
                  (cur_suffix=="TIF") ||
                  (cur_suffix=="TIFF") ||
                  (cur_suffix=="RAW") ||
                  (cur_suffix=="V3DRAW") ||
                  (cur_suffix=="VAA3DRAW") ||
                  (cur_suffix=="RAW5") ||
                  (cur_suffix=="V3DRAW5") ||
                  (cur_suffix=="VAA3DRAW5") ||
                  (cur_suffix=="MRC") ||
                  (cur_suffix=="V3DPBD") ||
                  (cur_suffix=="NRRD") ||
                  (cur_suffix=="NHDR") ||
                  (cur_suffix=="VAA3DPBD") ||
                  (cur_suffix=="MP4") ||
                  (cur_suffix=="H5J") ||
                  curfile_info.suffix().isEmpty() //then invoke raw reader in this case, 20120410. PHC
                  )
        {
            try
            {
                size_t start_t = clock();
                XFormWidget *child = createMdiChild();
                v3d_msg(QString("Trying to load an image file [%1]").arg(fileName), 0);

                if (child->loadFile(fileName))
                {
//                    if(!child) return;
//                    if(!child->getImageData()) return;

                    //if(child->getValidZslice()<child->getImageData()->getZDim()-1) return; // avoid crash when the child is closed by user, Dec 29, 2010 by YuY
                    //bug!!! by PHC. This is a very bad bug. 2011-02-09. this makes all subsequent operations unable to finish. should be disabled!!.

                    statusBar()->showMessage(QString("File [%1] loaded").arg(fileName), 2000);
                    if (global_setting.b_autoConvert2_8bit)
                    {
                        if (global_setting.default_rightshift_bits<0) //when set as -1 or other <0 values, then invoke the dialog.
                        {
                            if (child->getImageData()->getDatatype()==V3D_UINT16)
                                child->popupImageProcessingDialog(tr(" -- convert 16bit image to 8 bit"));
                            else if (child->getImageData()->getDatatype()==V3D_FLOAT32)
                                child->popupImageProcessingDialog(tr(" -- convert 32bit (single-precision float) image to 8 bit"));
                        }
                        else //otherwise do the conversion directly
                        {
                            if (child->getImageData()->getDatatype()==V3D_UINT16)
                                child->getImageData()->proj_general_convert16bit_to_8bit(global_setting.default_rightshift_bits);
                            else if (child->getImageData()->getDatatype()==V3D_FLOAT32)
                                child->getImageData()->proj_general_convert32bit_to_8bit(global_setting.default_rightshift_bits);
                        }
                    }
                    if (global_setting.b_yaxis_up)
                    {
                        child->getImageData()->flip(axis_y);
                    }
                    child->show();
                    //workspace->cascade(); //080821 //110805, by PHC, since RZC claims the resize MDI works now, so this should not be needed.
                    // create sampled data 512x512x256 and save it for use in 3dviewer
                    // to improve openning speed. ZJL 111019
                    // qDebug("   child->mypara_3Dview = %0p", &(child->mypara_3Dview));
                    // saveDataFor3DViewer( &(child->mypara_3Dview));
                    if (b_forceopen3dviewer || (global_setting.b_autoOpenImg3DViewer))
                    {
                        child->doImage3DView();
                    }
                    size_t end_t = clock();
                    qDebug()<<"time consume ..."<<end_t-start_t;
                }
                else
                {
                    child->close();
                    if (QMessageBox::question(0, "File Open options",
                                              QString("Cannot open the specified image [%1] using Vaa3D's default file opener. "
                                              "Do you want to try other file opener in Vaa3D plugins?").arg(fileName), QMessageBox::Yes, QMessageBox::No)
                            == QMessageBox::Yes)
                    {
                        //call 3rd party file loader //20131125. by PHC
                        //(QString("").arg(fileName));

                        v3d_imaging_paras myimagingp;
                        myimagingp.OPS = "Load file using Vaa3D data IO manager";
                        myimagingp.datafilename = fileName;
                        myimagingp.imgp = 0; //the image data for a plugin to call
                        //do data loading
                        if (!v3d_imaging(this, myimagingp))
                            v3d_msg("Even the data IO manager cannot load this file. You need to use 3rd party converter to convert the file format first.");
                        return;
                    }
                }
            }
            catch (...)
            {
                v3d_msg(QString("You fail to open a new window for the specified image [%1]."
                                "The file may have certain problem - check the file format, or is simply too big but you don't have enough memory.").arg(fileName));
            }
        }
        //the following is to use bioformats to load the file. NOTE THE FOLLOWING LIST HAS SOME REDUNDANCY. But the executation should be fine
        else if ( (cur_suffix=="BMP") ||
                  (cur_suffix=="PNG") ||
                  (cur_suffix=="JPG") ||
                  (cur_suffix=="JPEG") ||
                  (cur_suffix=="NRRD") ||
                  (cur_suffix=="CZI") ||
                  (cur_suffix=="LIF") ||
                  (cur_suffix=="JP2") ||

                  (cur_suffix=="SLD") ||
                  (cur_suffix=="AIM") ||
                  (cur_suffix=="AL3D") ||
                  (cur_suffix=="GEL") ||
                  (cur_suffix=="AM") ||
                  (cur_suffix=="AMIRAMESH") ||
                  (cur_suffix=="GREY") ||
                  (cur_suffix=="HX") ||
                  (cur_suffix=="LABELS") ||
                  (cur_suffix=="IMG") ||
                  (cur_suffix=="HDR") ||
                  (cur_suffix=="PNG") ||
                  (cur_suffix=="SVS") ||
                  (cur_suffix=="HTD") ||
                  (cur_suffix=="PNL") ||
                  (cur_suffix=="AVI") ||
                  (cur_suffix=="ARF") ||
                  (cur_suffix=="EXP") ||
                  (cur_suffix=="SDT") ||
                  (cur_suffix=="1SC") ||
                  (cur_suffix=="PIC") ||
                  (cur_suffix=="XML") ||
                  (cur_suffix=="IMS") ||
                  (cur_suffix=="CR2") ||
                  (cur_suffix=="CRW") ||
                  (cur_suffix=="C01") ||
                  (cur_suffix=="VSI") ||
                  (cur_suffix=="DV") ||
                  (cur_suffix=="R3D") ||
                  (cur_suffix=="DCM") ||
                  (cur_suffix=="DICOM") ||
                  (cur_suffix=="V") ||
                  (cur_suffix=="EPS") ||
                  (cur_suffix=="EPSI") ||
                  (cur_suffix=="PS") ||
                  (cur_suffix=="FLEX") ||
                  (cur_suffix=="MEA") ||
                  (cur_suffix=="RES") ||
                  (cur_suffix=="FITS") ||
                  (cur_suffix=="DM3") ||
                  (cur_suffix=="DM2") ||
                  (cur_suffix=="GIF") ||
                  (cur_suffix=="NAF") ||
                  (cur_suffix=="HIS") ||
                  (cur_suffix=="NDPI") ||
                  (cur_suffix=="VMS") ||
                  (cur_suffix=="TXT") ||
                  (cur_suffix=="BMP") ||
                  (cur_suffix=="JPG") ||
                  (cur_suffix=="JPEG") ||
                  (cur_suffix=="ICS") ||
                  (cur_suffix=="IDS") ||
                  (cur_suffix=="FFF") ||
                  (cur_suffix=="SEQ") ||
                  (cur_suffix=="IPW") ||
                  (cur_suffix=="HED") ||
                  (cur_suffix=="MOD") ||
                  (cur_suffix=="LIFF") ||
                  (cur_suffix=="XDCE") ||
                  (cur_suffix=="FRM") ||
                  (cur_suffix=="INR") ||
                  (cur_suffix=="IPL") ||
                  (cur_suffix=="IPM") ||
                  (cur_suffix=="DAT") ||
                  (cur_suffix=="PAR") ||
                  (cur_suffix=="JP2") ||
                  (cur_suffix=="JPK") ||
                  (cur_suffix=="JPX") ||
                  (cur_suffix=="XV") ||
                  (cur_suffix=="BIP") ||
                  (cur_suffix=="FLI") ||
                  (cur_suffix=="LEI") ||
                  (cur_suffix=="LIF") ||
                  (cur_suffix=="SCN") ||
                  (cur_suffix=="SXM") ||
                  (cur_suffix=="L2D") ||
                  (cur_suffix=="LIM") ||
                  (cur_suffix=="STK") ||
                  (cur_suffix=="ND") ||
                  (cur_suffix=="MNC") ||
                  (cur_suffix=="MRW") ||
                  (cur_suffix=="MNG") ||
                  (cur_suffix=="STP") ||
                  (cur_suffix=="MRC") ||
                  (cur_suffix=="NEF") ||
                  (cur_suffix=="ND2") ||
                  (cur_suffix=="NRRD") ||
                  (cur_suffix=="NHDR") ||
                  (cur_suffix=="RAW") ||
                  (cur_suffix=="APL") ||
                  (cur_suffix=="MTB") ||
                  (cur_suffix=="TNB") ||
                  (cur_suffix=="OBSEP") ||
                  (cur_suffix=="OIB") ||
                  (cur_suffix=="OIF") ||
                  (cur_suffix=="OME") ||
                  (cur_suffix=="TOP") ||
                  (cur_suffix=="PCX") ||
                  (cur_suffix=="PDS") ||
                  (cur_suffix=="2") ||
                  (cur_suffix=="3") ||
                  (cur_suffix=="4") ||
                  (cur_suffix=="PGM") ||
                  (cur_suffix=="PSD") ||
                  (cur_suffix=="PICT") ||
                  (cur_suffix=="PNG") ||
                  (cur_suffix=="CFG") ||
                  (cur_suffix=="AFM") ||
                  (cur_suffix=="MOV") ||
                  (cur_suffix=="SM2") ||
                  (cur_suffix=="SM3") ||
                  (cur_suffix=="XQD") ||
                  (cur_suffix=="XQF") ||
                  (cur_suffix=="CXD") ||
                  (cur_suffix=="SPI") ||
                  (cur_suffix=="STK") ||
                  (cur_suffix=="TGA") ||
                  (cur_suffix=="VWS") ||
                  (cur_suffix=="TFR") ||
                  (cur_suffix=="FFR") ||
                  (cur_suffix=="ZFR") ||
                  (cur_suffix=="ZFP") ||
                  (cur_suffix=="2FL") ||
                  (cur_suffix=="SLD") ||
                  (cur_suffix=="PR3") ||
                  (cur_suffix=="DAT") ||
                  (cur_suffix=="FDF") ||
                  (cur_suffix=="DTI") ||
                  (cur_suffix=="XYS") ||
                  (cur_suffix=="HTML") ||
                  (cur_suffix=="MVD2") ||
                  (cur_suffix=="ACFF") ||
                  (cur_suffix=="WAT") ||
                  (cur_suffix=="ZVI") ||
                  (cur_suffix=="CZI") ||
                  (cur_suffix=="LSM") ||
                  (cur_suffix=="MDB")
                  )
        {
            try
            {
                size_t start_t = clock();
                XFormWidget *child = createMdiChild();
                v3d_msg(QString("Trying to load an image file [%1] using Bioformats IO library").arg(fileName), 0);

                unsigned char * b_data1d=0;
                V3DLONG * b_sz=0;
                ImagePixelType b_dt;

                if (readSingleImageFile((char *)qPrintable(fileName), b_data1d, b_sz, b_dt))
                {
                    child->setImageData(b_data1d, b_sz[0], b_sz[1], b_sz[2], b_sz[3], b_dt);
                    child->setCurrentFileName(fileName);

                    statusBar()->showMessage(QString("File [%1] loaded").arg(fileName), 2000);
                    if (global_setting.b_autoConvert2_8bit)
                    {
                        if (global_setting.default_rightshift_bits<0) //when set as -1 or other <0 values, then invoke the dialog.
                        {
                            if (child->getImageData()->getDatatype()==V3D_UINT16)
                                child->popupImageProcessingDialog(tr(" -- convert 16bit image to 8 bit"));
                            else if (child->getImageData()->getDatatype()==V3D_FLOAT32)
                                child->popupImageProcessingDialog(tr(" -- convert 32bit (single-precision float) image to 8 bit"));
                        }
                        else //otherwise do the conversion directly
                        {
                            if (child->getImageData()->getDatatype()==V3D_UINT16)
                                child->getImageData()->proj_general_convert16bit_to_8bit(global_setting.default_rightshift_bits);
                            else if (child->getImageData()->getDatatype()==V3D_FLOAT32)
                                child->getImageData()->proj_general_convert32bit_to_8bit(global_setting.default_rightshift_bits);
                        }
                    }
                    if (global_setting.b_yaxis_up)
                        //add 1 here so that this condition is always true. This is becasue it seems Bioformats has a different way to orient an image from vaa3d. So always flip. by PHC, 140128
                        //remove 1 now because I use v3draw as tmp file right now.
                    {
                        child->getImageData()->flip(axis_y);
                    }
                    child->show();
                    if (b_forceopen3dviewer || (global_setting.b_autoOpenImg3DViewer))
                    {
                        child->doImage3DView();
                    }
                    size_t end_t = clock();
                    qDebug()<<"time consume ..."<<end_t-start_t;
                }
                else
                {
                    child->close();
                    v3d_msg("fail to load the image using Bioformats. You may want to use a Vaa3D plugin or some other 3rd party programs to load this image, or convert the format and load into Vaa3D.");
                }

                if (b_sz) {delete []b_sz; b_sz=0;}
            }
            catch (...)
            {
                v3d_msg(QString("You fail to open a new window for the specified image [%1]."
                                "The file may have certain problem - check the file format, or is simply too big but you don't have enough memory.").arg(fileName));
            }
        }
//		else if (cur_suffix=="HRAW") // For openning hierarchical data from large data set. ZJL 20120302
//        {
//			QString basename = curfile_info.baseName();
//
//			QString hraw_prefix = "test";//""curfile_info.absolutePath()  + basename.left(basename.indexOf(".")); // before the first "."
//
//			string prefix = hraw_prefix.toStdString();
//			//string prefix ="/Volumes/PengMapView/mapview_testdata/ananya/test";
//
//             try
//             {
//                  size_t start_t = clock();
//
//                  // contents of .hraw file
//                  // L, M, N, l, m, n
//                  // level : level nums
//                  // outsz[3]
//
//                  int L = 14; //log(8)/log(2.0);
//                  int M = 38; //log(8)/log(2.0);
//                  int N = 3;  //log(8)/log(2.0);
//                  int l = 512;//log(256)/log(2.0);
//                  int m = 256;//log(128)/log(2.0);
//                  int n = 64; //log(64)/log(2.0);
//                  int level = 0;
//
//                  ImageMapView mapview;
//                  mapview.setPara(prefix, L, M, N, l, m, n);
//
//                  unsigned char * outimg1d = 0;
//                  V3DLONG origin[3] = {0, 0, 0};
//                  V3DLONG outsz[4] = {512, 256, 64, 1};
//
//                  mapview.getImage(level, outimg1d, origin[0], origin[1], origin[2], outsz[0], outsz[1], outsz[2]);
//
//                  XFormWidget *child = createMdiChild();
//                  child->setImageData(outimg1d, outsz[0], outsz[1], outsz[2], outsz[3], V3D_UINT8);
//                  child->mypara_3Dview.image4d = child->getImageData();
//
//                  // mapview control
//                  Mapview_Paras mv_paras;
//                  mv_paras.L=L; mv_paras.M=M; mv_paras.N=N;
//                  mv_paras.l=l; mv_paras.m=m; mv_paras.n=n;
//                  mv_paras.origin[0] = origin[0]; mv_paras.origin[1] = origin[1]; mv_paras.origin[2] = origin[2];
//                  mv_paras.outsz[0] = outsz[0]; mv_paras.outsz[1] = outsz[1]; mv_paras.outsz[2] = outsz[2]; mv_paras.outsz[3] = outsz[3]; mv_paras.outsz[3] = outsz[3];
//                  mv_paras.hraw_prefix=hraw_prefix;
//                  mv_paras.level = level;
//
//                  child->mapview_paras = mv_paras;
//                  child->mapview = mapview;
//
//                  child->setWindowTitle_Prefix(hraw_prefix.toAscii());
//                  child->setWindowTitle_Suffix("");
//
//                  child->reset();
//
//                  if (global_setting.b_autoConvert2_8bit)
//                  {
//                       if (global_setting.default_rightshift_bits<0) //when set as -1 or other <0 values, then invoke the dialog.
//                       {
//                            if (child->getImageData()->getDatatype()==V3D_UINT16)
//                                 child->popupImageProcessingDialog(tr(" -- convert 16bit image to 8 bit"));
//                            else if (child->getImageData()->getDatatype()==V3D_FLOAT32)
//                                 child->popupImageProcessingDialog(tr(" -- convert 32bit (single-precision float) image to 8 bit"));
//                       }
//                       else //otherwise do the conversion directly
//                       {
//                            if (child->getImageData()->getDatatype()==V3D_UINT16)
//                                 child->getImageData()->proj_general_convert16bit_to_8bit(global_setting.default_rightshift_bits);
//                            else if (child->getImageData()->getDatatype()==V3D_FLOAT32)
//                                 child->getImageData()->proj_general_convert32bit_to_8bit(global_setting.default_rightshift_bits);
//                       }
//                  }
//
//                  if (global_setting.b_yaxis_up)
//                  {
//                       child->getImageData()->flip(axis_y);
//                  }
//
//                  child->show();
//
//                  // create mapview control window
//                  child->createMapviewControlWin();
//
//                  if (b_forceopen3dviewer || (global_setting.b_autoOpenImg3DViewer))
//                  {
//                       child->doImage3DView();
//                  }
//
//                  size_t end_t = clock();
//                  qDebug()<<"time consume ..."<<end_t-start_t;
//
//             }
//             catch(...)
//             {
//                  QMessageBox::warning(0, "warning: fail to create window", "You fail to open a new window for the specified image. The file may have certain problem, or is simply too big but you don't have enough memory.");
//                  v3d_msg(QString("Fail to create window for the file [%1]\n").arg(fileName));
//             }
//        } // end hraw
        else // changed by YuY Nov. 19, 2010. Msg corrected by PHC, 2011-06-04
        {
            v3d_msg(QString("The file [%1] has an unsupported file name extension and cannot be opened properly! "
                            "You should check the data type or file extension and you can convert the file format to something Vaa3D can read (e.g. a standard TIF file); but now Vaa3D is going to use the special Vaa3D file IO plugin (e.g. BioFormat plugin, etc) "
                            "to attempt reading the data ...").arg(fileName), 1);
            return;
        }
        //child->close();delete child; child=0; //this should be correspond to the error place! by phc, 080429
    }
    //if success then out in recent file list
    if (b_putinrecentfilelist)
    {
        setCurrentFile(fileName);
        emit imageLoaded2Plugin(); //20110426 YuY  //why need to send this signal?? PHC 2012-02-10
    }
}
void MainWindow::setup_global_imgproc_parameter_default()
{
    //for fly brain registration
    flybrain_lobeseg_para.f_image = 1;
    flybrain_lobeseg_para.f_smooth = 0.3;
    flybrain_lobeseg_para.f_length = 0.3;
    flybrain_lobeseg_para.radius_x = 20;
    flybrain_lobeseg_para.radius_y = 10;
    flybrain_lobeseg_para.nloops = 500;
    flybrain_lobeseg_para.TH = 0.1;
    //global_setting.GPara_df_compute_method = (int)DF_GEN_TPS_LINEAR_INTERP;
    //global_setting.GPara_landmarkMatchingMethod = (int)MATCH_MI;
    global_setting.b_3dcurve_autodeform=false;
    global_setting.b_3dcurve_autowidth=false;
}
void MainWindow::import_GeneralImageFile()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        XFormWidget *existing = findMdiChild(fileName);
        if (existing) {

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
            workspace->setActiveSubWindow(existing);
#else
            workspace->setActiveWindow(existing);
#endif
            return;
        }
        try
        {
            XFormWidget *child = createMdiChild();
            if (child->importGeneralImageFile(fileName)) {
                statusBar()->showMessage(tr("File imported"), 2000);
                child->show();
                //workspace->cascade(); //080821 //110805. by PHC, should not need cascade anymore
                //setCurrentFile(fileName);//080930 //note that for the imported files, I don't keep the history record
            } else {
                child->close();
            }
        }
        catch (...)
        {
            v3d_msg("You fail to import the specified image(s). The file may have certain problem, or is simply too big but you don't have enough memory.");
        }
    }
}
void MainWindow::import_Leica()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        XFormWidget *existing = findMdiChild(fileName);
        if (existing) {

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
            workspace->setActiveSubWindow(existing);
#else
            workspace->setActiveWindow(existing);
#endif
            return;
        }
        try
        {
            XFormWidget *child = createMdiChild();
            if (child->importLeicaFile(fileName)) {
                statusBar()->showMessage(tr("File imported"), 2000);
                child->show();
                //workspace->cascade(); //080821 //110805. by PHC, should not need cascade anymore
                //setCurrentFile(fileName);//080930 //note that for the imported files, I don't keep the history record
            } else {
                child->close();
            }
        }
        catch (...)
        {
            v3d_msg("You fail to import the specified image(s). The file may have certain problem, or is simply too big but you don't have enough memory.");
        }
    }
}
void MainWindow::func_procIO_import_atlas_imgfolder()
{
    QMessageBox::information(0, "Information about import files to an .atlas linker file",
                             "To import a series of images to an .atlas linker file, these files should be put into the same folder and have the same size (X,Y,Z dimensions and number of color channels). <br><br>"
                             "You will be first asked to specify one of these files, and you can define a string filter of their file names.<br><br>"
                             "Normally these files should be aligned/registered images; but this function can also be used to produce a linker file just for screening a series of image stacks.<br><br>"
                             "Note that these files must be valid .tif or Vaa3D's Raw files (or another image file format supported in Vaa3D).<br><br>"
                             "You will be then asked to specify a file name of the output .atlas linker file.<br><br>"
                             );
    //get the input individual file
    QString fileName = QFileDialog::getOpenFileName(this);
    if (fileName.isEmpty())
    {
        v3d_msg("No input file is selected. Do nothing.");
        return;
    }
    //get the list of files
    QStringList mylist = importSeriesFileList_simplesort(fileName);
    if (mylist.size()<=0)
    {
        v3d_msg("The file list is empty.");
        return;
    }
    //now determine the output .atlas file
    fileName = QFileDialog::getSaveFileName(this, tr("Save Atlas File"),
                                            fileName + "_import.atlas",
                                            tr("Atlas linker file (*.atlas)"));
    if (fileName.isEmpty())
    {
        v3d_msg("No output file is selected. Do nothing.");
        return;
    }
    //now save the file list to the output, using the random color
    FILE *fp = fopen(qPrintable(fileName), "wt");
    if (!fp)
    {
        v3d_msg("Unable to open the specified file to write. Do nothing.");
        return;
    }
    for (int i=0;i<mylist.size();i++)
    {
        QFileInfo fileInfo(mylist.at(i));
        QString bname = fileInfo.baseName();
        int j=bname.length();
        if (j>10) j=10;
        RGBA8 color = random_rgba8(255);
        fprintf(fp, "%d, %s, %d, %d, %d, %s\n",i, qPrintable(bname.remove(j,bname.length())), int(color.r), int(color.g), int(color.b), qPrintable(mylist.at(i))); //use the first 10 letters as the line name
    }
    fclose(fp);
    v3d_msg(qPrintable(fileName.prepend("The atlas linker file [").append("] has been created.")));
}
void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)	{
        // File name might be a URL -- by CMB Oct-14-2010
        QString fileOrUrl(action->data().toString());
        QUrl url(fileOrUrl);
        // Note that file names are interpreted as "valid" URLs
        if (QFile(fileOrUrl).exists()) {
            loadV3DFile(fileOrUrl, true, false); // loadV3DFile func changed to 3 args. YuY Nov. 18, 2010
        }
        else if (url.isValid())
        {
            // Only download ftp, http, https, etc.
            // not "file" nor empty "" URL scheme
            if ( (! url.isRelative())
                 && (url.scheme() != "file") )
            {
                // qDebug("Recent URL chosen");
                loadV3DUrl(url);
                return;
            }
            else
            {
                v3d_msg(QString("The file you try to open [%1] does not seem to be valid. Do nothing.").arg(fileOrUrl));
            }
        }
        else {
            v3d_msg(QString("The file you try to open [%1] does not seem to exist. Do nothing.").arg(fileOrUrl));
            // error in file name
        }
        // qDebug("Recent file chosen");
    }
}
void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    //	if (curFile.isEmpty())
    //		setWindowTitle(tr("Recent Files"));
    //	else
    //		setWindowTitle(tr("%1 - %2").arg(strippedName(curFile)));
    QSettings settings("HHMI", "Vaa3D");
    QStringList files = settings.value("recentFileList").toStringList();
    QString curAddName = QFileInfo(fileName).canonicalFilePath();
    if (curAddName.isEmpty())
        curAddName = fileName;
    files.removeAll(curAddName);
    files.prepend(curAddName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();
    settings.setValue("recentFileList", files);
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}
void MainWindow::updateRecentFileActions()
{
    QSettings settings("HHMI", "Vaa3D");
    QStringList files = settings.value("recentFileList").toStringList();
    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);
    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
        // Entry might be a URL -- by CMB Oct-14-2010
        if (QUrl(files[i]).isValid())
            text = files[i];
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);
    separator_RecentFiles_Act->setVisible(numRecentFiles > 0);
}
QString MainWindow::strippedName(const QString &fullFileName)
{
    //	return QFileInfo(fullFileName).fileName();
    return QFileInfo(fullFileName).absoluteFilePath();
}
void MainWindow::atlasView()
{
    /*
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        XFormWidget *existing = findMdiChild(fileName);
        if (existing) {
            workspace->setActiveWindow(existing);
            return;
        }
        XFormWidget *child = createMdiChild();
        if (child->loadFile(fileName)) {
            statusBar()->showMessage(tr("File loaded"), 2000);
            child->show();
        } else {
            child->close();
        }
    }
 */
}
void MainWindow::save()
{
    if (activeMdiChild())
        if (activeMdiChild()->saveData())
	{
            setCurrentFile(activeMdiChild()->userFriendlyCurrentFile());
            statusBar()->showMessage(tr("File saved [%1]").arg(activeMdiChild()->userFriendlyCurrentFile()), 2000);
	}
}
void MainWindow::saveAs()
{
    if (activeMdiChild())
        if (activeMdiChild()->saveData())
	{
            setCurrentFile(activeMdiChild()->userFriendlyCurrentFile());
            statusBar()->showMessage(tr("File saved"), 2000);
	}
}
void MainWindow::cut()
{
    //    activeMdiChild()->cut();
}
void MainWindow::copy()
{
    //    activeMdiChild()->copy();
}
void MainWindow::paste()
{
    //    activeMdiChild()->paste();
}
//void MainWindow::about()
//{
//	v3d_aboutinfo();
//}
void MainWindow::updateMenus()
{
    if (activeMdiChild()==0 || activeMdiChild()->getImageData()==0) //110804 RZC
        return;
    bool hasMdiChild = (activeMdiChild() != 0);
    saveAct->setEnabled(hasMdiChild);
    procSettings->setEnabled(true);//always true
    {
        QSettings settings("HHMI", "Vaa3D");
        QStringList files = settings.value("recentFileList").toStringList();
        int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);
        separator_RecentFiles_Act->setVisible(numRecentFiles > 0);
    }
    //    saveAsAct->setEnabled(hasMdiChild);
    //    pasteAct->setEnabled(hasMdiChild);
    closeAct->setEnabled(hasMdiChild);
    closeAllAct->setEnabled(hasMdiChild);
    tileAct->setEnabled(hasMdiChild);
    cascadeAct->setEnabled(hasMdiChild);
    arrangeAct->setEnabled(hasMdiChild);
    nextAct->setEnabled(hasMdiChild);
    previousAct->setEnabled(hasMdiChild);
    separator_ImgWindows_Act->setVisible(hasMdiChild);
    //    bool hasSelection = (activeMdiChild() && activeMdiChild()->textCursor().hasSelection());
    //    bool hasSelection = true;
    //    cutAct->setEnabled(hasSelection);
    //    copyAct->setEnabled(hasSelection);
    //update the IO menus
    procIO_import_atlas_imgfolder->setEnabled(true); //always enable import atlas file option
    procIO_export_to_vano_format->setEnabled(hasMdiChild);
    procIO_export_to_movie->setEnabled(hasMdiChild);
    if (hasMdiChild)
    {
        //qDebug("%d, %d\n", activeMdiChild()->getImageData()->listLandmarks.size(), activeMdiChild()->getImageData()->tracedNeuron.row.size());
        procIO_export_landmark_to_pointcloud->setEnabled(activeMdiChild()->getImageData()->listLandmarks.size()>0);
        procIO_export_landmark_to_swc->setEnabled(activeMdiChild()->getImageData()->listLandmarks.size()>0);
        procIO_export_tracedneuron_to_swc->setEnabled(activeMdiChild()->getImageData()->tracedNeuron.nsegs()>0);
    }
    else
    {
        procIO_export_landmark_to_pointcloud->setEnabled(hasMdiChild);
        procIO_export_landmark_to_swc->setEnabled(hasMdiChild);
        procIO_export_tracedneuron_to_swc->setEnabled(hasMdiChild);
    }
    //update the image processing menus
    procGeneral_rotate_paxis->setEnabled(hasMdiChild);
    procGeneral_rotate_angle->setEnabled(hasMdiChild);
    procGeneral_flip->setEnabled(hasMdiChild);
    procLandmarkManager->setEnabled(hasMdiChild);
    procGeneral_clear_all_landmark->setEnabled(hasMdiChild);
    procGeneral_clear_connectmap->setEnabled(hasMdiChild);
    procGeneral_rescale_landmarks_only->setEnabled(hasMdiChild);
    procGeneral_toggle_landmark_label->setEnabled(hasMdiChild);
    procGeneral_automarker_entireimg->setEnabled(hasMdiChild);
    procGeneral_automarker_roi->setEnabled(hasMdiChild);
    if (hasMdiChild)
    {
        procGeneral_toggle_landmark_label->setText( (activeMdiChild()->bDispMarkerLabel) ?
                                                        "Turn OFF landmark label in the current active tri-view" :
                                                        "Turn ON landmark label in the current active tri-view");
    }
    if (hasMdiChild)
    {
        bool b_res = (activeMdiChild()->getImageData()->getCDim()>1);
        procGeneral_split_channels->setEnabled(b_res);
        procGeneral_extract_a_channel->setEnabled(b_res);
    }
    else
    {
        procGeneral_split_channels->setEnabled(hasMdiChild);
        procGeneral_extract_a_channel->setEnabled(hasMdiChild);
    }
    procGeneral_crop_image_minMaxBox->setEnabled(hasMdiChild);
    procGeneral_crop_bbox_roi->setEnabled(hasMdiChild);
    procGeneral_mask_roi->setEnabled(hasMdiChild);
    procGeneral_mask_nonroi_xy->setEnabled(hasMdiChild);
    procGeneral_mask_channel->setEnabled(hasMdiChild);
    procGeneral_clear_roi->setEnabled(hasMdiChild);
    procGeneral_resample_image->setEnabled(hasMdiChild);
    procGeneral_projection_max->setEnabled(hasMdiChild);
    procGeneral_blend_image->setEnabled(hasMdiChild);
    procGeneral_stitch_image->setEnabled(hasMdiChild);
#ifdef _ALLOW_ALPHA_TEST_MENUS_
    procGeneral_display_histogram->setEnabled(hasMdiChild);
    procGeneral_linear_adjustment->setEnabled(hasMdiChild);
#endif
    procGeneral_histogram_equalization->setEnabled(hasMdiChild);
    procGeneral_intensity_rescale->setEnabled(hasMdiChild);
    procGeneral_intensity_threshold->setEnabled(hasMdiChild);
    procGeneral_intensity_binarize->setEnabled(hasMdiChild);
    procGeneral_intensity_updateminmax->setEnabled(hasMdiChild);
    if (hasMdiChild)
    {
        procGeneral_indexedimg2rgb->setEnabled(activeMdiChild()->getImageData()->getCDim()==1);
        procGeneral_16bit_to_8bit->setEnabled(activeMdiChild()->getImageData()->getDatatype()==V3D_UINT16);
        procGeneral_32bit_to_8bit->setEnabled(activeMdiChild()->getImageData()->getDatatype()==V3D_FLOAT32);
        procGeneral_color_invert->setEnabled(activeMdiChild()->getImageData()->getDatatype()==V3D_UINT8);
    }
    else
    {
        procGeneral_indexedimg2rgb->setEnabled(hasMdiChild);
        procGeneral_16bit_to_8bit->setEnabled(hasMdiChild);
        procGeneral_32bit_to_8bit->setEnabled(hasMdiChild);
        procGeneral_color_invert->setEnabled(hasMdiChild);
    }
    procGeneral_scaleandconvert28bit->setEnabled(hasMdiChild);
    procGeneral_scaleandconvert28bit_1percent->setEnabled(hasMdiChild);
    //procGeneral_open_image_in_windows->setEnabled(hasMdiChild);  //080930 disabled
    procGeneral_save_image->setEnabled(hasMdiChild);
#ifdef _ALLOW_IMGSTD_MENU_
    procElongated_randomSeeding->setEnabled(hasMdiChild);
    procElongated_minSpanTree->setEnabled(hasMdiChild);
    procElongated_mstDiameter->setEnabled(hasMdiChild);
    procElongated_genCuttingPlaneLocations->setEnabled(hasMdiChild);
    procElongated_restackingCuttingPlanes->setEnabled(hasMdiChild);
    procElongated_bdbminus->setEnabled(hasMdiChild);
#endif
#ifdef _ALLOW_IMGREG_MENU_
    procReg_gridSeeding->setEnabled(hasMdiChild);
    procReg_randomSeeding->setEnabled(hasMdiChild);
    procReg_bigGradient_edge_Seeding->setEnabled(hasMdiChild);
    procReg_big_curvature_corner_Seeding->setEnabled(hasMdiChild);
    procReg_fileSeeding->setEnabled(hasMdiChild);
    procReg_global_align->setEnabled(hasMdiChild);
    procReg_affine_markers_align->setEnabled(hasMdiChild);
    procReg_flybrain_lobeseg->setEnabled(hasMdiChild);
    procReg_detect_matching->setEnabled(hasMdiChild);
    procReg_detect_matching_1pt->setEnabled(hasMdiChild);
    procReg_warp_using_landmarks->setEnabled(hasMdiChild);
    procReg_all_in_one_warp->setEnabled(hasMdiChild);
#endif
#ifdef _ALLOW_NEURONSEG_MENU_
    procTracing_topdownSkeleton->setEnabled(hasMdiChild);
    procTracing_bottomupSearch->setEnabled(hasMdiChild);
    procTracing_glocal_combine->setEnabled(hasMdiChild);
    procTracing_manualCorrect->setEnabled(hasMdiChild);
    if (hasMdiChild)
    {
		QDir pluginsDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
		if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
			pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
		if (pluginsDir.dirName() == "MacOS") {
			pluginsDir.cdUp();
			pluginsDir.cdUp();
			pluginsDir.cdUp();
		}
#endif
        procTracing_APP2auto->setEnabled(pluginsDir.cd("plugins/neuron_tracing/Vaa3D_Neuron2"));

        procTracing_one2others->setEnabled(activeMdiChild()->getImageData()->listLandmarks.size()>1);
        procTracing_trace_a_curve->setEnabled(activeMdiChild()->getImageData()->listLandmarks.size()>1);
        procTracing_undo_laststep->setEnabled(activeMdiChild()->getImageData()->tracedNeuron_historylist.size()>0);
        procTracing_redo_laststep->setEnabled(activeMdiChild()->getImageData()->tracedNeuron_historylist.size()>0);
    }
    else
    {
        procTracing_APP2auto->setEnabled(hasMdiChild);

        procTracing_one2others->setEnabled(hasMdiChild);
        procTracing_trace_a_curve->setEnabled(hasMdiChild);
        procTracing_undo_laststep->setEnabled(hasMdiChild);
        procTracing_redo_laststep->setEnabled(hasMdiChild);
    }
    procTracing_clear->setEnabled(hasMdiChild);
    procTracing_update3Dview->setEnabled(hasMdiChild);
    procTracing_save->setEnabled(hasMdiChild);
#endif
#ifdef _ALLOW_CELLSEG_MENU_
    procCellSeg_localTemplate->setEnabled(hasMdiChild);
    procCellSeg_cellcounting->setEnabled(hasMdiChild);
    procCellSeg_watershed->setEnabled(hasMdiChild);
    procCellSeg_levelset->setEnabled(hasMdiChild);
    procCellSeg_Gaussian_fit_1_spot_1_Gauss->setEnabled(hasMdiChild);
    procCellSeg_Gaussian_fit_1_spot_N_Gauss->setEnabled(hasMdiChild);
    procCellSeg_Gaussian_partition->setEnabled(false);
    procCellSeg_manualCorrect->setEnabled(false);
#endif
#ifdef _ALLOW_ATLAS_POINTCLOUD_MENU_
#endif
#ifdef _ALLOW_ATLAS_IMAGE_MENU_
    if (hasMdiChild)
        procAtlasViewer->setEnabled(activeMdiChild()->getImageData()->listAtlasFiles.size()>0);
    else
        procAtlasViewer->setEnabled(hasMdiChild);
#endif
    proc3DViewer->setText("3D viewer for the entire image");
	procVRViewer->setText("VR viewer for the entire image");//wwbchange
    proc3DLocalRoiViewer->setText("3D viewer for Region of Interest (ROI)");
    proc3DViewer->setEnabled(hasMdiChild);
	procVRViewer->setEnabled(hasMdiChild);//wwbchange
    proc3DLocalRoiViewer->setEnabled(hasMdiChild); //need to ensure the availability of roi later

}

void MainWindow::updatePluginMenu()
{
    v3d_msg("hello updatePluginMenu enter");
    if (pluginLoader)  // rescanPlugins() on 20130826 to ensure every time there is a refresh plugin list.
                   //This may be a memory leak issue as the few menus might need to be created every time. by PHC
    {
        v3d_msg("hello updatePluginMenu");
        pluginLoader->rescanPlugins(); //do nothing for now, as it seems rescanning every time is slowing down other menus and also is related to TeraFly zoom-out warning. by PHC 20130830
    }
}


#ifdef _ALLOW_WORKMODE_MENU_
void MainWindow::updateModeMenu()
{
    if (modeMenu)
    {
        modeMenu->clear();
        modeMenu->addAction(procModeDefault);
        modeMenu->addAction(procModeNeuronAnnotator);
    }
}
#endif

void MainWindow::updateWindowMenu()
{
    if (!windowMenu)
        return;
    windowMenu->clear();
    windowMenu->addAction(closeAct);
    windowMenu->addAction(closeAllAct);
    windowMenu->addSeparator();
    windowMenu->addAction(tileAct);
    windowMenu->addAction(cascadeAct);
    windowMenu->addAction(arrangeAct);
    windowMenu->addSeparator();
    windowMenu->addAction(nextAct);
    windowMenu->addAction(previousAct);
    windowMenu->addAction(separator_ImgWindows_Act);

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
    QList<QMdiSubWindow *> windows = workspace->subWindowList();
#else
    QList<QWidget *> windows = workspace->windowList();
#endif
    separator_ImgWindows_Act->setVisible(!windows.isEmpty());
    int i;
    for (i = 0; i < windows.size(); ++i) {
        XFormWidget *child = qobject_cast<XFormWidget *>(windows.at(i));
        QString text;
        if (i < 9) {
            text = tr("tri-view: &%1 %2").arg(i + 1)
                    .arg(child->userFriendlyCurrentFile());
        } else {
            text = tr("tri-view: %1 %2").arg(i + 1)
                    .arg(child->userFriendlyCurrentFile());
        }
        QAction *action  = windowMenu->addAction(text);
        action->setCheckable(true);
        action ->setChecked(child == activeMdiChild());
        connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
        windowMapper->setMapping(action, child);
    }
    //now add the 3D viewer list
    if (list_3Dview_win.size()>0)
    {
        windowMenu->addSeparator();
        for (i = 0; i < list_3Dview_win.size(); ++i)
        {
            V3dR_MainWindow *vchild = qobject_cast<V3dR_MainWindow *>(list_3Dview_win.at(i));
            QString text;
            if (i < 9) {
                text = tr("%1: &%2 %3").arg(vchild->getTitlePrefix()).arg(i + 1).arg(vchild->getDataTitle());
            } else {
                text = tr("%1: %2 %3").arg(vchild->getTitlePrefix()).arg(i + 1).arg(vchild->getDataTitle());
            }
            QAction *action  = windowMenu->addAction(text);
            action->setCheckable(true);
            //raise() is the right function to bring it to front (not activateWindow() which is not a slot, or show()). by PHC. 090626
            //however because raise does not mean the window is activated, thus I write a wrapper slot called raise_and_activate to call both raise() followed by activatedWindow()
            connect(action, SIGNAL(triggered()), vchild, SLOT(raise_and_activate()));
        }
    }
}
void MainWindow::updateProcessingMenu()
{
    if (!basicProcMenu || !advancedProcMenu || !visualizeProcMenu || !pluginProcMenu)
        return;
    //for image / data basic operations
    basicProcMenu->clear();
    advancedProcMenu->clear();
    visualizeProcMenu->clear();
    pluginProcMenu->clear();

#if COMPILE_TARGET_LEVEL != 0  //for V3D Pro or Advantage compiling
    proc_datatype_menu = basicProcMenu->addMenu(tr("image type"));
    proc_geometry_transform_menu = basicProcMenu->addMenu(tr("geometry"));
    proc_intensity_transform_menu = basicProcMenu->addMenu(tr("intensity"));
    proc_colorchannel_transform_menu = basicProcMenu->addMenu(tr("color channel"));
    proc_landmark_control_menu = basicProcMenu->addMenu(tr("landmark"));
    proc_datatype_menu->addAction(procGeneral_indexedimg2rgb);
    proc_datatype_menu->addAction(procGeneral_scaleandconvert28bit);
    proc_datatype_menu->addAction(procGeneral_scaleandconvert28bit_1percent);
    proc_datatype_menu->addAction(procGeneral_16bit_to_8bit);
    proc_datatype_menu->addAction(procGeneral_32bit_to_8bit);
    proc_geometry_transform_menu->addAction(procGeneral_rotate_paxis);
    proc_geometry_transform_menu->addAction(procGeneral_rotate_angle);
    proc_geometry_transform_menu->addAction(procGeneral_flip);
    proc_geometry_transform_menu->addSeparator();
    proc_geometry_transform_menu->addAction(procGeneral_crop_image_minMaxBox);
    proc_geometry_transform_menu->addAction(procGeneral_crop_bbox_roi);
    proc_geometry_transform_menu->addSeparator();
    proc_geometry_transform_menu->addAction(procGeneral_resample_image);
    //proc_geometry_transform_menu->addAction(procGeneral_stitch_image);
    proc_intensity_transform_menu->addAction(procGeneral_mask_roi);
    proc_intensity_transform_menu->addAction(procGeneral_mask_nonroi_xy);
    proc_intensity_transform_menu->addAction(procGeneral_mask_channel);
    proc_intensity_transform_menu->addAction(procGeneral_clear_roi);
    proc_intensity_transform_menu->addSeparator();
    proc_intensity_transform_menu->addAction(procGeneral_projection_max);
    proc_intensity_transform_menu->addSeparator();
#ifdef _ALLOW_ALPHA_TEST_MENUS_
    proc_intensity_transform_menu->addAction(procGeneral_display_histogram);
    proc_intensity_transform_menu->addAction(procGeneral_linear_adjustment);
#endif
    proc_intensity_transform_menu->addAction(procGeneral_histogram_equalization);
    proc_intensity_transform_menu->addAction(procGeneral_intensity_rescale);
    proc_intensity_transform_menu->addAction(procGeneral_intensity_threshold);
    proc_intensity_transform_menu->addAction(procGeneral_intensity_binarize);
    proc_intensity_transform_menu->addAction(procGeneral_color_invert);
    proc_intensity_transform_menu->addSeparator();
    proc_intensity_transform_menu->addAction(procGeneral_intensity_updateminmax);
    proc_colorchannel_transform_menu->addAction(procGeneral_split_channels);
    proc_colorchannel_transform_menu->addAction(procGeneral_extract_a_channel);
    proc_colorchannel_transform_menu->addAction(procGeneral_blend_image);
    proc_colorchannel_transform_menu->addSeparator();
    proc_colorchannel_transform_menu->addAction(procGeneral_color_invert);
    proc_landmark_control_menu->addAction(procLandmarkManager);
    proc_landmark_control_menu->addSeparator();
    proc_landmark_control_menu->addAction(procGeneral_clear_all_landmark);
    proc_landmark_control_menu->addAction(procGeneral_clear_connectmap);
    proc_landmark_control_menu->addAction(procGeneral_rescale_landmarks_only);
    proc_landmark_control_menu->addAction(procGeneral_toggle_landmark_label);
    //proc_landmark_control_menu->addSeparator();
    //proc_landmark_control_menu->addAction(procGeneral_automarker_entireimg);
    //proc_landmark_control_menu->addAction(procGeneral_automarker_roi);
    //for image processing menu
#ifdef _ALLOW_CELLSEG_MENU_
    proc_segmentation_menu = advancedProcMenu->addMenu(tr("3D segmentation"));
#endif
#ifdef _ALLOW_NEURONSEG_MENU_
    proc_tracing_menu = advancedProcMenu->addMenu(tr("3D tracing (Vaa3D-Neuron tracing v2.0)"));
#endif
#ifdef _ALLOW_IMGREG_MENU_
    proc_registration_menu = advancedProcMenu->addMenu(tr("3D registration"));
#endif
#ifdef _ALLOW_IMGSTD_MENU_
    proc_standarization_menu = advancedProcMenu->addMenu(tr("3D standardization"));
#endif
#ifdef _ALLOW_ATLAS_POINTCLOUD_MENU_
    proc_pointcloud_atlas_menu = advancedProcMenu->addMenu(tr("3D point cloud atlas"));
#endif
#ifdef _ALLOW_ATLAS_IMAGE_MENU_
    proc_image_atlas_menu = advancedProcMenu->addMenu(tr("3D image atlas"));
#endif
#ifdef _ALLOW_AUTOMARKER_MENU_
    proc_automarker_menu = advancedProcMenu->addMenu(tr("AutoMarker"));
#endif
#ifdef _ALLOW_TERAFLY_MENU_
    QMenu *proc_terafly_menu = advancedProcMenu->addMenu(tr("Big-Image-Data"));
    QAction* open_terafly_action = new QAction(tr("TeraFly"), this);
    proc_terafly_menu->addAction(open_terafly_action);
    connect(open_terafly_action, SIGNAL(triggered()), this, SLOT(func_open_terafly()));
    QAction* open_teraconverter_action = new QAction(tr("TeraConverter"), this);
    proc_terafly_menu->addAction(open_teraconverter_action);
    connect(open_teraconverter_action, SIGNAL(triggered()), this, SLOT(func_open_teraconverter()));
#endif
    //
#ifdef _ALLOW_IMGSTD_MENU_
    proc_standarization_menu->addAction(procElongated_randomSeeding);
    proc_standarization_menu->addSeparator();
    proc_standarization_menu->addAction(procElongated_minSpanTree);
    proc_standarization_menu->addAction(procElongated_mstDiameter);
    proc_standarization_menu->addAction(procElongated_genCuttingPlaneLocations);
    proc_standarization_menu->addAction(procElongated_restackingCuttingPlanes);
    proc_standarization_menu->addAction(procElongated_bdbminus);
#endif
#ifdef _ALLOW_NEURONSEG_MENU_
    //proc_tracing_menu->addAction(procTracing_topdownSkeleton);
    //proc_tracing_menu->addAction(procTracing_bottomupSearch);
    //proc_tracing_menu->addAction(procTracing_glocal_combine);
    //proc_tracing_menu->addSeparator();
    proc_tracing_menu->addAction(procTracing_APP2auto);
    proc_tracing_menu->addSeparator();
    proc_tracing_menu->addAction(procTracing_one2others);
    proc_tracing_menu->addAction(procTracing_trace_a_curve);
    proc_tracing_menu->addAction(procTracing_undo_laststep);
    proc_tracing_menu->addAction(procTracing_redo_laststep);
    proc_tracing_menu->addSeparator();
    //proc_tracing_menu->addAction(procTracing_manualCorrect);
    proc_tracing_menu->addAction(procTracing_clear);
    proc_tracing_menu->addAction(procTracing_update3Dview);
    proc_tracing_menu->addAction(procTracing_save);
#endif
#ifdef _ALLOW_IMGREG_MENU_
    proc_registration_menu->addAction(procReg_gridSeeding);
    proc_registration_menu->addAction(procReg_randomSeeding);
    //proc_registration_menu->addAction(procReg_bigGradient_edge_Seeding);
    //proc_registration_menu->addAction(procReg_big_curvature_corner_Seeding);
    proc_registration_menu->addAction(procReg_fileSeeding);
    proc_registration_menu->addSeparator();
    proc_registration_menu->addAction(procReg_global_align);
    proc_registration_menu->addAction(procReg_affine_markers_align);
    proc_registration_menu->addSeparator();
    proc_registration_menu->addAction(procReg_flybrain_lobeseg);
    proc_registration_menu->addSeparator();
    proc_registration_menu->addAction(procReg_detect_matching);
    proc_registration_menu->addAction(procReg_detect_matching_1pt);
    proc_registration_menu->addSeparator();
    proc_registration_menu->addAction(procReg_warp_using_landmarks);
    proc_registration_menu->addSeparator();
    proc_registration_menu->addAction(procReg_all_in_one_warp);
#endif
#ifdef _ALLOW_CELLSEG_MENU_
    proc_segmentation_menu->addAction(procCellSeg_localTemplate);
    proc_segmentation_menu->addAction(procCellSeg_cellcounting);
    proc_segmentation_menu->addAction(procCellSeg_watershed);
    //proc_segmentation_menu->addAction(procCellSeg_levelset);
    //proc_segmentation_menu->addAction(procCellSeg_Gaussian_partition);
    proc_segmentation_menu->addSeparator();
    proc_segmentation_menu->addAction(procCellSeg_Gaussian_fit_1_spot_1_Gauss);
    proc_segmentation_menu->addAction(procCellSeg_Gaussian_fit_1_spot_N_Gauss);
    //proc_segmentation_menu->addAction(procCellSeg_manualCorrect);
#endif
#ifdef _ALLOW_ATLAS_POINTCLOUD_MENU_
    proc_pointcloud_atlas_menu->addAction(procPC_Atlas_view_atlas);
    proc_pointcloud_atlas_menu->addSeparator();
    proc_pointcloud_atlas_menu->addAction(procPC_Atlas_create_atlaslinkerfile);
    proc_pointcloud_atlas_menu->addAction(procPC_Atlas_edit_atlaslinkerfile);
    proc_pointcloud_atlas_menu->addAction(procPC_Atlas_view_atlas_computeVanoObjStat);
#endif
#ifdef _ALLOW_ATLAS_IMAGE_MENU_
    proc_image_atlas_menu->addAction(procAtlasViewer);
    proc_image_atlas_menu->addSeparator();
    proc_image_atlas_menu->addAction(procIO_import_atlas_imgfolder);
#endif
#ifdef _ALLOW_AUTOMARKER_MENU_
    proc_automarker_menu->addAction(procGeneral_automarker_entireimg);
    proc_automarker_menu->addAction(procGeneral_automarker_roi);

#endif
    //Visualization menu
    visualizeProcMenu->addAction(proc3DViewer);
	visualizeProcMenu->addAction(procVRViewer);//wwbchange
    visualizeProcMenu->addAction(proc3DLocalRoiViewer);
    //Plug-in menu
    if (pluginLoader)
    {
        //		if (proc_plugin_manager) pluginProcMenu->addAction(proc_plugin_manager);
        //		pluginProcMenu->addSeparator();
        pluginLoader->populateMenus();
    }
#endif //end V3D Pro compiling
}
void MainWindow::createActions()
{
    newAct = new QAction(QIcon(":/pic/new.png"), tr("&New"), this);
    newAct->setShortcut(tr("Ctrl+N"));
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));
    openAct = new QAction(QIcon(":/pic/open.png"), tr("&Open image/stack/surface_file in a new window ..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing image"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
    // New Open Web URL action, based on Open Action (openAct) example
    // By CMB 06-Oct-2010
    openWebUrlAct = new QAction(QIcon(":/pic/web.png"), tr("&Open web image/stack/surface_file ..."), this);
    openWebUrlAct->setShortcut(tr("Ctrl+W"));
    openWebUrlAct->setStatusTip(tr("Open a web (URL) image"));
    connect(openWebUrlAct, SIGNAL(triggered()), this, SLOT(openWebUrl()));
#ifdef __v3d_custom_toolbar__
    // Custom toolbar, By Hang 06-Aug-2011
    customToolbarAct = new QAction(QIcon(":pic/customize.png"), tr("&Customize a toolbar"), this);
    //customToolbarAct->setShortcut(tr("Ctrl+C"));
    customToolbarAct->setStatusTip(tr("Customize a toolbar"));
    connect(customToolbarAct, SIGNAL(triggered()), this, SLOT(addCustomToolbar()));
#endif
    saveAct = new QAction(QIcon(":/pic/save.png"), tr("&Save or Save as"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the image to disk"));
    //connect(saveAct, SIGNAL(triggered()), this, SLOT(func_procGeneral_save_image()));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
    /*
  saveAsAct = new QAction(tr("Save &As..."), this);
  saveAsAct->setStatusTip(tr("Save the document under a new name"));
  connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));
  */
    procSettings = new QAction(tr("Adjust preferences"), this); //note that there is a strange problem that when I use "Preference or setting" as the begining of the this menu item, it just dees NOT display in the file menu. Is this a QT reserved word?
    procSettings->setStatusTip(tr("Adjust the user preferences"));
    connect(procSettings, SIGNAL(triggered()), this, SLOT(func_procSettings()));
    import_GeneralImageFileAct = new QAction(QIcon(":/pic/import.png"), tr("&Import general image series to an image stack..."), this);
    import_GeneralImageFileAct->setShortcut(tr("Ctrl+I"));
    import_GeneralImageFileAct->setStatusTip(tr("Import general image series"));
    connect(import_GeneralImageFileAct, SIGNAL(triggered()), this, SLOT(import_GeneralImageFile()));
    import_LeicaAct = new QAction(QIcon(":/pic/import.png"), tr("&Import Leica 2D tiff series to an image stack..."), this);
    import_LeicaAct->setStatusTip(tr("Import Leica tiff file series"));
    connect(import_LeicaAct, SIGNAL(triggered()), this, SLOT(import_Leica()));
    procIO_import_atlas_imgfolder = new QAction(tr("Build an atlas linker file for [registered] images under a folder"), this);
    procIO_import_atlas_imgfolder->setStatusTip(tr("Build an atlas linker file for [registered] images under a folder..."));
    connect(procIO_import_atlas_imgfolder, SIGNAL(triggered()), this, SLOT(func_procIO_import_atlas_imgfolder()));
    //	procIO_import_atlas_apofolder = new QAction(tr("Build an atlas linker file for pointcloud (APO) files under a folder"), this);
    //    procIO_import_atlas_apofolder->setStatusTip(tr("Build an atlas linker file for pointcloud (APO) files under a folder..."));
    //    connect(procIO_import_atlas_apofolder, SIGNAL(triggered()), this, SLOT(func_procIO_import_atlas_apofolder()));
    procIO_export_to_vano_format = new QAction(tr("Export 3D [cell] segmentation results to VANO annotation files"), this);
    procIO_export_to_vano_format->setStatusTip(tr("Export 3D [cell] segmentation results to VANO annotation files..."));
    connect(procIO_export_to_vano_format, SIGNAL(triggered()), this, SLOT(func_procIO_export_to_vano_format()));
    procIO_export_to_movie = new QAction(tr("Export 3D image stack to movie file"), this);
    procIO_export_to_movie->setStatusTip(tr("Export 3D image stack to movie file"));
    connect(procIO_export_to_movie, SIGNAL(triggered()), this, SLOT(func_procIO_export_to_movie()));
    procIO_export_landmark_to_pointcloud = new QAction(tr("Export landmarks to pointcloud (.apo) file"), this);
    procIO_export_landmark_to_pointcloud->setStatusTip(tr("Export landmarks to pointcloud (.apo) file"));
    connect(procIO_export_landmark_to_pointcloud, SIGNAL(triggered()), this, SLOT(func_procIO_export_landmark_to_pointcloud()));
    procIO_export_landmark_to_swc = new QAction(tr("Export landmarks and their relations to graph (.swc) file"), this);
    procIO_export_landmark_to_swc->setStatusTip(tr("Export landmarks and their relations to graph (.swc) file"));
    connect(procIO_export_landmark_to_swc, SIGNAL(triggered()), this, SLOT(func_procIO_export_landmark_to_swc()));
    procIO_export_tracedneuron_to_swc = new QAction(tr("Export traced neuron/fibrous-structure path-info to graph (.swc) file"), this);
    procIO_export_tracedneuron_to_swc->setStatusTip(tr("Export traced neuron/fibrous-structure to graph (.swc) file"));
    connect(procIO_export_tracedneuron_to_swc, SIGNAL(triggered()), this, SLOT(func_procIO_export_tracedneuron_to_swc()));
    //atlasViewAct = new QAction(QIcon(":/pic/new.png"), tr("A&tlasView..."), this);
    atlasViewAct = new QAction(QIcon(":/pic/atlasView.png"), tr("A&tlasView..."), this);
    atlasViewAct->setShortcut(tr("Ctrl+t"));
    atlasViewAct->setStatusTip(tr("Atlas view of registered images"));
    connect(atlasViewAct, SIGNAL(triggered()), this, SLOT(atlasView()));
    procAtlasViewer = new QAction(tr("3D image atlas viewer"), this);
    connect(procAtlasViewer, SIGNAL(triggered()), this, SLOT(func_procAtlasViewer()));
    proc3DViewer = new QAction(tr("3D viewer for entire image"), this);
    proc3DViewer->setShortcut(tr("Ctrl+V"));
    connect(proc3DViewer, SIGNAL(triggered()), this, SLOT(func_proc3DViewer()));
	//wwbchange
	procVRViewer = new QAction(tr("VR viewer for entire image"), this);
    connect(procVRViewer, SIGNAL(triggered()), this, SLOT(func_procVRViewer()));
	//wwbchange
    proc3DLocalRoiViewer = new QAction(tr("3D viewer for Region of Interest (ROI)"), this);
    proc3DLocalRoiViewer->setShortcut(tr("Shift+V"));
    connect(proc3DLocalRoiViewer, SIGNAL(triggered()), this, SLOT(func_proc3DLocalRoiViewer()));
    separator_RecentFiles_Act = new QAction(this);
    separator_RecentFiles_Act->setSeparator(true);
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }
    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close())); //090812 RZC
    //qApp, SLOT(closeAllWindows()));
    closeAct = new QAction(tr("Cl&ose"), this);
    closeAct->setShortcut(tr("Ctrl+F4"));
    closeAct->setStatusTip(tr("Close the active window"));
    connect(closeAct, SIGNAL(triggered()),
            workspace, SLOT(closeActiveWindow()));
    closeAllAct = new QAction(tr("Close &All"), this);
    closeAllAct->setStatusTip(tr("Close all the windows"));


//    connect(closeAllAct, SIGNAL(triggered()), workspace, SLOT(closeAllWindows()));
    connect(closeAllAct, SIGNAL(triggered()), this, SLOT(handleCoordinatedCloseEvent_real()));


    tileAct = new QAction(tr("&Tile"), this);
    tileAct->setStatusTip(tr("Tile the windows"));
    connect(tileAct, SIGNAL(triggered()), workspace, SLOT(tile()));
    cascadeAct = new QAction(tr("&Cascade"), this);
    cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAct, SIGNAL(triggered()), workspace, SLOT(cascade()));
    arrangeAct = new QAction(tr("Arrange &icons"), this);
    arrangeAct->setStatusTip(tr("Arrange the icons"));
    connect(arrangeAct, SIGNAL(triggered()), workspace, SLOT(arrangeIcons()));
    nextAct = new QAction(tr("Ne&xt"), this);
    nextAct->setShortcut(tr("Ctrl+F6"));
    nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAct, SIGNAL(triggered()),
            workspace, SLOT(activateNextWindow()));
    previousAct = new QAction(tr("Pre&vious"), this);
    previousAct->setShortcut(tr("Ctrl+Shift+F6"));
    previousAct->setStatusTip(tr("Move the focus to the previous "
                                 "window"));
    connect(previousAct, SIGNAL(triggered()),
            workspace, SLOT(activatePreviousWindow()));
    separator_ImgWindows_Act = new QAction(this);
    separator_ImgWindows_Act->setSeparator(true);
    checkForUpdatesAct = new QAction(tr("Check for Updates..."), this);
    checkForUpdatesAct->setStatusTip(tr("Check whether a more recent version "
                                        "of Vaa3D is available."));
    connect(checkForUpdatesAct, SIGNAL(triggered()), this, SLOT(checkForUpdates()));

    generateVersionInfoAct = new QAction(tr("Generate/check current version info on the local machine"), this);
    connect(generateVersionInfoAct, SIGNAL(triggered()), this, SLOT(generateVersionInfo()));
    aboutAct = new v3d::ShowV3dAboutDialogAction(this);
    //    aboutQtAct = new QAction(tr("About &Qt"), this);
    //    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    //    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    //for the plugin menu
    proc_plugin_manager = new QAction(tr("Plug-in manager"), this);
    connect(proc_plugin_manager, SIGNAL(triggered()), this, SLOT(func_proc_plugin_manager()));
    //the processing menu. 080613
    procGeneral_rotate_paxis = new QAction(tr("rotate principal axis"), this);
    connect(procGeneral_rotate_paxis, SIGNAL(triggered()), this, SLOT(func_procGeneral_rotate_paxis()));
    procGeneral_rotate_angle = new QAction(tr("rotate arbitrary angle"), this);
    connect(procGeneral_rotate_angle, SIGNAL(triggered()), this, SLOT(func_procGeneral_rotate_angle()));
    procGeneral_flip = new QAction(tr("flip image"), this);
    connect(procGeneral_flip, SIGNAL(triggered()), this, SLOT(func_procGeneral_flip()));
    procLandmarkManager = new QAction(tr("landmark manager"), this);
    connect(procLandmarkManager, SIGNAL(triggered()), this, SLOT(func_procLandmarkManager()));
    procGeneral_clear_all_landmark = new QAction(tr("clear all landmarks"), this);
    connect(procGeneral_clear_all_landmark, SIGNAL(triggered()), this, SLOT(func_procGeneral_clear_all_landmark()));
    procGeneral_clear_connectmap = new QAction(tr("clear graph edges/connection map"), this);
    connect(procGeneral_clear_connectmap, SIGNAL(triggered()), this, SLOT(func_procGeneral_clear_connectmap()));
    procGeneral_rescale_landmarks_only = new QAction(tr("rescale landmark only"), this);
    connect(procGeneral_rescale_landmarks_only, SIGNAL(triggered()), this, SLOT(func_procGeneral_rescale_landmarks_only()));
    procGeneral_toggle_landmark_label = new QAction(tr("turn on/off displaying landmark labels"), this);
    connect(procGeneral_toggle_landmark_label, SIGNAL(triggered()), this, SLOT(func_procGeneral_toggle_landmark_label()));
    procGeneral_automarker_entireimg = new QAction(tr("AutoMarker (for entire image)"), this);
    connect(procGeneral_automarker_entireimg, SIGNAL(triggered()), this, SLOT(func_procGeneral_automarker_entireimg()));
    procGeneral_automarker_roi = new QAction(tr("AuroMarker (for bounding box of defined region of interest)"), this);
    connect(procGeneral_automarker_roi, SIGNAL(triggered()), this, SLOT(func_procGeneral_automarker_roi()));
    procGeneral_split_channels = new QAction(tr("split color channels (also see ColorChannel plugin for a better version)"), this);
    connect(procGeneral_split_channels, SIGNAL(triggered()), this, SLOT(func_procGeneral_split_channels()));
    procGeneral_extract_a_channel = new QAction(tr("extract a color channel (also see ColorChannel plugin for a better version)"), this);
    connect(procGeneral_extract_a_channel, SIGNAL(triggered()), this, SLOT(func_procGeneral_extract_a_channel()));
    procGeneral_crop_image_minMaxBox = new QAction(tr("crop image (minMax Bounding Box)"), this);
    connect(procGeneral_crop_image_minMaxBox, SIGNAL(triggered()), this, SLOT(func_procGeneral_crop_image_minMaxBox()));
    procGeneral_crop_bbox_roi = new QAction(tr("crop image (ROI-based)"), this);
    connect(procGeneral_crop_bbox_roi, SIGNAL(triggered()), this, SLOT(func_procGeneral_crop_bbox_roi()));
    procGeneral_mask_roi = new QAction(tr("mask ROI or non-ROI"), this);
    connect(procGeneral_mask_roi, SIGNAL(triggered()), this, SLOT(func_procGeneral_mask_roi()));
    procGeneral_mask_nonroi_xy = new QAction(tr("fill value to non-ROI region for all XY planes"), this);
    connect(procGeneral_mask_nonroi_xy, SIGNAL(triggered()), this, SLOT(func_procGeneral_mask_nonroi_xy()));
    procGeneral_mask_channel = new QAction(tr("mask channel"), this);
    connect(procGeneral_mask_channel, SIGNAL(triggered()), this, SLOT(func_procGeneral_mask_channel()));
    procGeneral_clear_roi = new QAction(tr("clear ROI"), this);
    connect(procGeneral_clear_roi, SIGNAL(triggered()), this, SLOT(func_procGeneral_clear_roi()));
    procGeneral_resample_image = new QAction(tr("image resampling"), this);
    connect(procGeneral_resample_image, SIGNAL(triggered()), this, SLOT(func_procGeneral_resample_image()));
    procGeneral_projection_max = new QAction(tr("max projection"), this);
    connect(procGeneral_projection_max, SIGNAL(triggered()), this, SLOT(func_procGeneral_projection_max()));
    procGeneral_blend_image = new QAction(tr("image blending"), this);
    connect(procGeneral_blend_image, SIGNAL(triggered()), this, SLOT(func_procGeneral_blend_image()));
    procGeneral_stitch_image = new QAction(tr("3D/2D image stitching"), this);
    connect(procGeneral_stitch_image, SIGNAL(triggered()), this, SLOT(func_procGeneral_stitch_image()));
#ifdef _ALLOW_ALPHA_TEST_MENUS_
    procGeneral_display_histogram = new QAction(tr("display histogram"), this);
    connect(procGeneral_display_histogram, SIGNAL(triggered()), this, SLOT(func_procGeneral_display_histogram()));
    procGeneral_linear_adjustment = new QAction(tr("linear intensity adjustment (2% to be saturated)"), this);
    connect(procGeneral_linear_adjustment, SIGNAL(triggered()), this, SLOT(func_procGeneral_linear_adjustment()));
#endif
    procGeneral_histogram_equalization = new QAction(tr("histogram equalization"), this);
    connect(procGeneral_histogram_equalization, SIGNAL(triggered()), this, SLOT(func_procGeneral_histogram_equalization()));
    procGeneral_intensity_rescale = new QAction(tr("rescaling"), this);
    connect(procGeneral_intensity_rescale, SIGNAL(triggered()), this, SLOT(func_procGeneral_intensity_rescale()));
    procGeneral_intensity_threshold = new QAction(tr("thresholding"), this);
    connect(procGeneral_intensity_threshold, SIGNAL(triggered()), this, SLOT(func_procGeneral_intensity_threshold()));
    procGeneral_intensity_binarize = new QAction(tr("binarization"), this);
    connect(procGeneral_intensity_binarize, SIGNAL(triggered()), this, SLOT(func_procGeneral_intensity_binarize()));
    procGeneral_color_invert = new QAction(tr("invert color"), this);
    connect(procGeneral_color_invert, SIGNAL(triggered()), this, SLOT(func_procGeneral_color_invert()));
    procGeneral_intensity_updateminmax = new QAction(tr("update the displayed min/max value(s)"), this);
    connect(procGeneral_intensity_updateminmax, SIGNAL(triggered()), this, SLOT(func_procGeneral_intensity_updateminmax()));
    procGeneral_scaleandconvert28bit = new QAction(tr("linear rescale to [0~255] and convert to 8bit image"), this);
    connect(procGeneral_scaleandconvert28bit, SIGNAL(triggered()), this, SLOT(func_procGeneral_scaleandconvert28bit()));
    procGeneral_scaleandconvert28bit_1percent = new QAction(tr("saturate top/bottom 1% voxels and linear-rescale to [0~255]/8bit"), this);
    connect(procGeneral_scaleandconvert28bit_1percent, SIGNAL(triggered()), this, SLOT(func_procGeneral_scaleandconvert28bit_1percent()));
    procGeneral_16bit_to_8bit = new QAction(tr("convert 16bit image to 8bit via bit-shift"), this);
    connect(procGeneral_16bit_to_8bit, SIGNAL(triggered()), this, SLOT(func_procGeneral_16bit_to_8bit()));
    procGeneral_32bit_to_8bit = new QAction(tr("convert 32bit image to 8bit via bit-shift"), this);
    connect(procGeneral_32bit_to_8bit, SIGNAL(triggered()), this, SLOT(func_procGeneral_32bit_to_8bit()));
    procGeneral_indexedimg2rgb = new QAction(tr("convert indexed/mask image to RGB image"), this);
    connect(procGeneral_indexedimg2rgb, SIGNAL(triggered()), this, SLOT(func_procGeneral_indexedimg2rgb()));
    //procGeneral_open_image_in_windows = new QAction(tr("open image in this window"), this);
    //connect(procGeneral_open_image_in_windows, SIGNAL(triggered()), this, SLOT(func_procGeneral_open_image_in_windows()));
    procGeneral_save_image = new QAction(tr("save image"), this);
    connect(procGeneral_save_image, SIGNAL(triggered()), this, SLOT(func_procGeneral_save_image()));
    procElongated_randomSeeding = new QAction(tr("random initialization"), this);
    connect(procElongated_randomSeeding, SIGNAL(triggered()), this, SLOT(func_procElongated_randomSeeding()));
    procElongated_minSpanTree = new QAction(tr("detect min spanning tree (MST)"), this);
    connect(procElongated_minSpanTree, SIGNAL(triggered()), this, SLOT(func_procElongated_minSpanTree()));
    procElongated_mstDiameter = new QAction(tr("detect MST diameter"), this);
    connect(procElongated_mstDiameter, SIGNAL(triggered()), this, SLOT(func_procElongated_mstDiameter()));
    procElongated_genCuttingPlaneLocations = new QAction(tr("gen cutting-plane locations"), this);
    connect(procElongated_genCuttingPlaneLocations, SIGNAL(triggered()), this, SLOT(func_procElongated_genCuttingPlaneLocations()));
    procElongated_restackingCuttingPlanes = new QAction(tr("restacking"), this);
    connect(procElongated_restackingCuttingPlanes, SIGNAL(triggered()), this, SLOT(func_procElongated_restackingCuttingPlanes()));
    procElongated_bdbminus = new QAction(tr("BDB-"), this);
    connect(procElongated_bdbminus, SIGNAL(triggered()), this, SLOT(func_procElongated_bdbminus()));
    procReg_gridSeeding = new QAction(tr("initialize using grid"), this);
    connect(procReg_gridSeeding, SIGNAL(triggered()), this, SLOT(func_procReg_gridSeeding()));
    procReg_randomSeeding = new QAction(tr("initialize randomly"), this);
    connect(procReg_randomSeeding, SIGNAL(triggered()), this, SLOT(func_procReg_randomSeeding()));
    procReg_bigGradient_edge_Seeding = new QAction(tr("initialize using edges"), this);
    connect(procReg_bigGradient_edge_Seeding, SIGNAL(triggered()), this, SLOT(func_procReg_bigGradient_edge_Seeding()));
    procReg_big_curvature_corner_Seeding = new QAction(tr("initialize using corners"), this);
    connect(procReg_big_curvature_corner_Seeding, SIGNAL(triggered()), this, SLOT(func_procReg_big_curvature_corner_Seeding()));
    procReg_fileSeeding = new QAction(tr("initialize from file"), this);
    connect(procReg_fileSeeding, SIGNAL(triggered()), this, SLOT(func_procReg_fileSeeding()));
    procReg_global_align = new QAction(tr("global affine alignment by matching image content"), this);
    connect(procReg_global_align, SIGNAL(triggered()), this, SLOT(func_procReg_global_align()));
    procReg_affine_markers_align = new QAction(tr("global affine alignment using transform of matching landmarks"), this);
    connect(procReg_affine_markers_align, SIGNAL(triggered()), this, SLOT(func_procReg_affine_markers_align()));
    procReg_flybrain_lobeseg = new QAction(tr("segmenting optical lobes of a global aligned adult-fly brain"), this);
    connect(procReg_flybrain_lobeseg, SIGNAL(triggered()), this, SLOT(func_procReg_flybrain_lobeseg()));
    procReg_detect_matching = new QAction(tr("detect matching landmarks"), this);
    connect(procReg_detect_matching, SIGNAL(triggered()), this, SLOT(func_procReg_detect_matching()));
    procReg_detect_matching_1pt = new QAction(tr("detect best matching for 1 landmark"), this);
    connect(procReg_detect_matching_1pt, SIGNAL(triggered()), this, SLOT(func_procReg_detect_matching_1pt()));
    procReg_warp_using_landmarks = new QAction(tr("warp image"), this);
    connect(procReg_warp_using_landmarks, SIGNAL(triggered()), this, SLOT(func_procReg_warp_using_landmarks()));
    procReg_all_in_one_warp = new QAction(tr("All-in-one matching and warping"), this);
    connect(procReg_all_in_one_warp, SIGNAL(triggered()), this, SLOT(func_procReg_all_in_one_warp()));
    procTracing_topdownSkeleton = new QAction(tr("top-down global optimization"), this);
    connect(procTracing_topdownSkeleton, SIGNAL(triggered()), this, SLOT(func_procTracing_topdownSkeleton()));
    procTracing_bottomupSearch = new QAction(tr("bottom-up directional kernel search"), this);
    connect(procTracing_bottomupSearch, SIGNAL(triggered()), this, SLOT(func_procTracing_bottomupSearch()));
    procTracing_glocal_combine = new QAction(tr("combined (both top-down and bottom-up)"), this);
    connect(procTracing_glocal_combine, SIGNAL(triggered()), this, SLOT(func_procTracing_glocal_combine()));
    procTracing_manualCorrect = new QAction(tr("manual correction"), this);
    connect(procTracing_manualCorrect, SIGNAL(triggered()), this, SLOT(func_procTracing_manualCorrect()));
    procTracing_APP2auto = new QAction(tr("Vaa3D-Neuron2 auto-tracing"), this);
    connect(procTracing_APP2auto, SIGNAL(triggered()), this, SLOT(func_procTracing_APP2auto()));
    procTracing_one2others = new QAction(tr("trace from one landmark to all others"), this);
    connect(procTracing_one2others, SIGNAL(triggered()), this, SLOT(func_procTracing_one2others()));
    procTracing_trace_a_curve = new QAction(tr("trace a path between two landmarks"), this);
    connect(procTracing_trace_a_curve, SIGNAL(triggered()), this, SLOT(func_procTracing_trace_a_curve()));
    procTracing_undo_laststep = new QAction(tr("undo the last tracing step"), this);
    connect(procTracing_undo_laststep, SIGNAL(triggered()), this, SLOT(func_procTracing_undo_laststep()));
    procTracing_redo_laststep = new QAction(tr("redo the last tracing step"), this);
    connect(procTracing_redo_laststep, SIGNAL(triggered()), this, SLOT(func_procTracing_redo_laststep()));
    procTracing_clear = new QAction(tr("clear the traced neuron"), this);
    connect(procTracing_clear, SIGNAL(triggered()), this, SLOT(func_procTracing_clear()));
    procTracing_update3Dview = new QAction(tr("update 3D view(s) of traced neuron"), this);
    connect(procTracing_update3Dview, SIGNAL(triggered()), this, SLOT(func_procTracing_update3Dview()));
    procTracing_save = new QAction(tr("save the traced neuron to a file"), this);
    connect(procTracing_save, SIGNAL(triggered()), this, SLOT(func_procTracing_save()));
    procCellSeg_localTemplate = new QAction(tr("template matching"), this);
    connect(procCellSeg_localTemplate, SIGNAL(triggered()), this, SLOT(func_procCellSeg_localTemplate()));
    procCellSeg_cellcounting = new QAction(tr("cell counting (Yang Yu method)"), this);
    connect(procCellSeg_cellcounting, SIGNAL(triggered()), this, SLOT(func_procCellSeg_cellcounting()));
    procCellSeg_watershed = new QAction(tr("watershed"), this);
    connect(procCellSeg_watershed, SIGNAL(triggered()), this, SLOT(func_procCellSeg_watershed()));
    procCellSeg_levelset = new QAction(tr("levelset"), this);
    connect(procCellSeg_levelset, SIGNAL(triggered()), this, SLOT(func_procCellSeg_levelset()));
    procCellSeg_Gaussian_fit_1_spot_1_Gauss = new QAction(tr("Gaussian fit (1 Gaussian only, isotropic)"), this);
    connect(procCellSeg_Gaussian_fit_1_spot_1_Gauss, SIGNAL(triggered()), this, SLOT(func_procCellSeg_Gaussian_fit_1_spot_1_Gauss()));
    procCellSeg_Gaussian_fit_1_spot_N_Gauss = new QAction(tr("Gaussian fit (N Gaussians, isotropic)"), this);
    connect(procCellSeg_Gaussian_fit_1_spot_N_Gauss, SIGNAL(triggered()), this, SLOT(func_procCellSeg_Gaussian_fit_1_spot_N_Gauss()));
    procCellSeg_Gaussian_partition = new QAction(tr("Gaussian partition"), this);
    connect(procCellSeg_Gaussian_partition, SIGNAL(triggered()), this, SLOT(func_procCellSeg_Gaussian_partition()));
    procCellSeg_manualCorrect = new QAction(tr("manual correction"), this);
    connect(procCellSeg_manualCorrect, SIGNAL(triggered()), this, SLOT(func_procCellSeg_manualCorrect()));
    //for the atlas menus
    //	procPC_Atlas_edit_atlaslinkerfile = new QAction(tr("Edit an existing point cloud atlas linker file"), this);
    //    procPC_Atlas_edit_atlaslinkerfile->setStatusTip(tr("Edit an existing point cloud atlas linker file..."));
    //    connect(procPC_Atlas_edit_atlaslinkerfile, SIGNAL(triggered()), this, SLOT(func_procPC_Atlas_edit_atlaslinkerfile()));
    //
    //	procPC_Atlas_create_atlaslinkerfile = new QAction(tr("Create a new point cloud atlas linker file"), this);
    //    procPC_Atlas_create_atlaslinkerfile->setStatusTip(tr("Create a new point cloud atlas linker file..."));
    //    connect(procPC_Atlas_create_atlaslinkerfile, SIGNAL(triggered()), this, SLOT(func_procPC_Atlas_create_atlaslinkerfile()));
    //
    //	procPC_Atlas_view_atlas = new QAction(tr("View a 3D point cloud atlas"), this);
    //    procPC_Atlas_view_atlas->setStatusTip(tr("View a 3D point cloud atlas..."));
    //    connect(procPC_Atlas_view_atlas, SIGNAL(triggered()), this, SLOT(func_procPC_Atlas_view_atlas()));
    procPC_Atlas_view_atlas_computeVanoObjStat = new QAction(tr("re-compute image objects statistics for .ano files under a directory"), this);
    procPC_Atlas_view_atlas_computeVanoObjStat->setStatusTip(tr("re-compute image objects statistics for .ano files under a directory"));
    connect(procPC_Atlas_view_atlas_computeVanoObjStat, SIGNAL(triggered()), this, SLOT(func_procPC_Atlas_view_atlas_computeVanoObjStat()));
    // Mode
#ifdef _ALLOW_WORKMODE_MENU_
    procModeDefault = new QAction(tr("Vaa3D Default"), this);
    procModeDefault->setCheckable(true);
    procModeDefault->setChecked(true);
    connect(procModeDefault, SIGNAL(triggered()), this, SLOT(func_procModeDefault()));
    procModeNeuronAnnotator = new QAction(tr("Neuron Annotator"), this);
    procModeNeuronAnnotator->setCheckable(true);
    procModeNeuronAnnotator->setChecked(false);
    connect(procModeNeuronAnnotator, SIGNAL(triggered()), this, SLOT(func_procModeNeuronAnnotator()));
#endif
}
void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(openWebUrlAct);
    fileMenu->addAction(saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(procSettings);
    fileMenu->addSeparator();
    proc_import_menu = fileMenu->addMenu("Import");
    proc_import_menu->addAction(import_GeneralImageFileAct);
    proc_import_menu->addAction(import_LeicaAct);
    proc_import_menu->addAction(procIO_import_atlas_imgfolder);
    //	proc_import_menu->addAction(procIO_import_atlas_apofolder); //disabled on 100316
    proc_export_menu = fileMenu->addMenu("Export");
    proc_export_menu->addAction(procIO_export_to_vano_format);
    //proc_export_menu->addAction(procIO_export_to_movie);
    proc_export_menu->addAction(procIO_export_landmark_to_pointcloud);
    //proc_export_menu->addAction(procIO_export_landmark_to_swc);
    proc_export_menu->addAction(procIO_export_tracedneuron_to_swc);
    fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        fileMenu->addAction(recentFileActs[i]);
    fileMenu->addAction(exitAct);
    updateRecentFileActions();
    connect(fileMenu, SIGNAL(aboutToShow()), this, SLOT(updateMenus()));

    //basic processing
    basicProcMenu = menuBar()->addMenu(tr("Image/Data"));
    connect(basicProcMenu, SIGNAL(aboutToShow()), this, SLOT(updateProcessingMenu()));
    connect(basicProcMenu, SIGNAL(aboutToShow()), this, SLOT(updateMenus()));
    //Visualize menu
    visualizeProcMenu = menuBar()->addMenu(tr("Visualize"));
    connect(visualizeProcMenu, SIGNAL(aboutToShow()), this, SLOT(updateProcessingMenu()));
    connect(visualizeProcMenu, SIGNAL(aboutToShow()), this, SLOT(updateMenus()));
    //image processing
    advancedProcMenu = menuBar()->addMenu(tr("Advanced"));
    connect(advancedProcMenu, SIGNAL(aboutToShow()), this, SLOT(updateProcessingMenu()));
    connect(advancedProcMenu, SIGNAL(aboutToShow()), this, SLOT(updateMenus()));
    //pipeline menu
    //pipelineProcMenu = menuBar()->addMenu(tr("Pipeline"));
    //connect(pipelineProcMenu, SIGNAL(aboutToShow()), this, SLOT(updateProcessingMenu()));
    //connect(pipelineProcMenu, SIGNAL(aboutToShow()), this, SLOT(updateMenus()));
    //plugin menu

    pluginProcMenu = menuBar()->addMenu(tr("Plug-In"));
//    //20130904, PHC
//    pluginProcMenu = new Vaa3DPluginMenu(tr("Plug-In"));
//    pluginProcMenu->setPluginLoader(pluginLoader);
//    menuBar()->addMenu(pluginProcMenu);
//    //menuBar()->addMenu((QMenu *)pluginProcMenu);

    connect(pluginProcMenu, SIGNAL(aboutToShow()), this, SLOT(updateProcessingMenu()));
//    connect(pluginProcMenu, SIGNAL(aboutToShow()), this, SLOT(updateMenus()));
    connect(pluginProcMenu, SIGNAL(aboutToShow()), this, SLOT(updatePluginMenu()));
//    connect(pluginProcMenu, SIGNAL(QAction::triggered()), this, SLOT(updatePluginMenu()));

    //others
    windowMenu = menuBar()->addMenu(tr("&Window"));
    connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));
    menuBar()->addSeparator();
#ifdef _ALLOW_WORKMODE_MENU_
    // Work-Mode
    modeMenu = menuBar()->addMenu(tr("Work-Mode"));
    connect(modeMenu, SIGNAL(aboutToShow()), this, SLOT(updateModeMenu()));
#endif
    //
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction( aboutAct );
    helpMenu->addAction(checkForUpdatesAct);
    helpMenu->addAction(generateVersionInfoAct);
    helpMenu->addAction( new v3d::OpenV3dWebPageAction(this) );
    //    helpMenu->addAction(aboutQtAct);
}
void MainWindow::createToolBars()
{
    fileToolBar = new QToolBar(tr("File"));
    //	fileToolBar->setIconSize(QSize(64,64));
    addToolBar(Qt::LeftToolBarArea, fileToolBar);
    //    fileToolBar = addToolBar(tr("File"));
    //fileToolBar->addAction(newAct); //commented on 080313
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(openWebUrlAct);
#ifdef __v3d_custom_toolbar__
    fileToolBar->addAction(customToolbarAct);
#endif
    //    fileToolBar->addAction(import_GeneralImageFileAct);
    //    fileToolBar->addAction(atlasViewAct);
    fileToolBar->addSeparator();
    fileToolBar->addAction(aboutAct);
    //    fileToolBar->addAction(saveAct);
    //    editToolBar = addToolBar(tr("Edit"));
    //    editToolBar->addAction(cutAct);
    //    editToolBar->addAction(copyAct);
    //    editToolBar->addAction(pasteAct);
    fileToolBar->setMovable(false);
}
void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}
void MainWindow::readSettings()
{
    QSettings settings("HHMI", "Vaa3D");
    QPoint pos = settings.value("pos", QPoint(10, 10)).toPoint();
    QSize size = settings.value("size", QSize(1000, 700)).toSize();
    move(pos);
    resize(size);
    V3DGlobalPreferenceDialog::readSettings(global_setting, settings);
}
void MainWindow::writeSettings()
{
    QSettings settings("HHMI", "Vaa3D");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    V3DGlobalPreferenceDialog::writeSettings(global_setting, settings);
}
//110801 RZC
//Notice that *** geometry changes to an MDI child widget must be applied to its parentWidget(), not to the widget itself.
//Similarly, if you want to find out the geometry of an MDI child widget you must use its parentWidget().
//This also applies to intercepting events for MDI child widgets: you must install your event filter on the parentWidget().
XFormWidget *MainWindow::createMdiChild()
{
    //    XFormWidget *child = new XFormWidget((QWidget *)0, Qt::WA_DeleteOnClose); //change to "this" does not change anything of the exit seg fault, 080429
    //																	//080814: important fix to assure the destructor function will be called.
    XFormWidget *child = new XFormWidget((QWidget *)0);

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
    workspace->addSubWindow(child);  //child is wrapped in his parentWidget()
#else
    workspace->addWindow(child);  //child is wrapped in his parentWidget()
#endif
    //for (int j=1; j<1000; j++) QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents); //100811 RZC: no help to update the workspace->windowList()

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
    qDebug()<<"MainWindow::createMdiChild *** workspace->windowList:" << workspace->subWindowList() <<"+="<< child; //STRANGE: child isn't in windowList here ???
#else
    qDebug()<<"MainWindow::createMdiChild *** workspace->windowList:" << workspace->windowList() <<"+="<< child; //STRANGE: child isn't in windowList here ???
#endif
    connect(workspace, SIGNAL(windowActivated(QWidget *)),  child, SLOT(onActivated(QWidget *))); //110802 RZC
    //workspace->setActiveWindow(child);
    //to enable coomunication of child windows
    child->setMainControlWindow(this);
    //child->adjustSize();
    QSize tmpsz = child->size();
    QSize oldszhint = child->sizeHint();
    printf("size hint=%d %d min size hint=%d %d\n", oldszhint.width(), oldszhint.height(), child->minimumSizeHint().width(), child->minimumSizeHint().height());
    //	child->resize(tmpsz.width()+1, tmpsz.height()+1);
    //	child->updateGeometry();
    //    child->showMaximized();
    //	child->showNormal();
    //    connect(child, SIGNAL(copyAvailable(bool)), cutAct, SLOT(setEnabled(bool)));
    //    connect(child, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)));
    return child;
}
XFormWidget *MainWindow::activeMdiChild()
{

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
    return qobject_cast<XFormWidget *>(workspace->activeSubWindow());
#else
    return qobject_cast<XFormWidget *>(workspace->activeWindow());
#endif
}
XFormWidget *MainWindow::findMdiChild(const QString &fileName)
{
    int numfind = 0; //20110427 YuY
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    if (canonicalFilePath.size()==0) canonicalFilePath = fileName; //090818 RZC 20110427 YuY
    XFormWidget *mdiChildFind;

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
    foreach (QMdiSubWindow *window, workspace->subWindowList()) {
#else
    foreach (QWidget *window, workspace->windowList()) {
#endif
        XFormWidget *mdiChild = qobject_cast<XFormWidget *>(window);
        QString mdiChildPath = // CMB Oct-14-2010
                QFileInfo(mdiChild->userFriendlyCurrentFile()).canonicalFilePath();
        if (mdiChildPath.isEmpty()) //in this case, try to handle a plugin-opened/returned image window with a title but without a real meaningful path yet. by PHC, 20120720
        {
            mdiChildPath = mdiChild->getOpenFileNameLabel();
        }
        if (mdiChildPath == canonicalFilePath || QFileInfo(mdiChildPath).fileName() == canonicalFilePath) //20110427 YuY
        {
            mdiChildFind = mdiChild;
            numfind++;
        }
    }
    if(!numfind) //20110427 YuY
    {
        // try find image name contains the input string from the end

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
        foreach (QMdiSubWindow *window, workspace->subWindowList()) {
#else
        foreach (QWidget *window, workspace->windowList()) {
#endif
            XFormWidget *mdiChild = qobject_cast<XFormWidget *>(window);
            QString mdiChildPath = // CMB Oct-14-2010
                    QFileInfo(mdiChild->userFriendlyCurrentFile()).canonicalFilePath();
            if ( mdiChildPath.endsWith(canonicalFilePath) || QFileInfo(mdiChildPath).fileName().endsWith(canonicalFilePath) ) //20110427 YuY
            {
                mdiChildFind = mdiChild;
                numfind++;
            }
        }
    }
    if(numfind > 1)	//20110427 YuY
    {
        v3d_msg(QString("Too many choices. Please specify your image with whole name including absolute path and try again."), 1);
        return 0;
    }
    else if(numfind == 1)
    {
        return mdiChildFind;
    }
    else
    {
        return 0;
    }
}
XFormWidget ** MainWindow::retrieveAllMdiChild(int & nchild)
{
    nchild=0;

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
    foreach (QMdiSubWindow *window, workspace->subWindowList()) {
#else
    foreach (QWidget *window, workspace->windowList()) {
#endif
        nchild++;
    }
    if (nchild<=0)
        return NULL;
    XFormWidget ** plist = new XFormWidget * [nchild];
    int i=0;

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
    foreach (QMdiSubWindow *window, workspace->subWindowList()) {
#else
    foreach (QWidget *window, workspace->windowList()) {
#endif
        plist[i++] = qobject_cast<XFormWidget *>(window);
    }
    return plist;
}
bool MainWindow::setCurHiddenSelectedWindow( XFormWidget* a) //by PHC, 101009
{
    bool b_found=false;

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
    foreach (QMdiSubWindow *window, workspace->subWindowList()) //ensure the value is valid (especially the window has not been closed)
#else
    foreach (QWidget *window, workspace->windowList()) //ensure the value is valid (especially the window has not been closed)
#endif
    {
        if (a == qobject_cast<XFormWidget *>(window))
        {b_found=true; break;}
    }
    if (b_found)
    {
        curHiddenSelectedXWidget = a;
        return true;
    }
    else
    {
        curHiddenSelectedXWidget = 0;
        return false;
    }
}
bool MainWindow::setCurHiddenSelectedWindow_withoutcheckwinlist( XFormWidget* a) // added by YuY, Dec 16, 2010.
{
    //
    if(a)
    {
        curHiddenSelectedXWidget = a;
        return true;
    }
    else
    {
        curHiddenSelectedXWidget = 0;
        return false;
    }
}
//the following are public slot processing functions
//		      << tr(" -- masking image using bounding boxes in 3D (derived from ROIs)") //missing? 080613
void MainWindow::func_procLandmarkManager() {if (activeMdiChild()) activeMdiChild()->launchAtlasViewer(1);}
void MainWindow::func_procAtlasViewer() {if (activeMdiChild()) activeMdiChild()->launchAtlasViewer(0);}
void MainWindow::func_proc3DViewer() {if (activeMdiChild()) activeMdiChild()->doImage3DView();}
void MainWindow::func_procVRViewer() {if (activeMdiChild()) activeMdiChild()->doImageVRView();}//wwbchange
void MainWindow::func_proc3DLocalRoiViewer() {if (activeMdiChild()) activeMdiChild()->doImage3DLocalRoiView();}
void MainWindow::func_procSettings()
{
    V3DGlobalPreferenceDialog d(&global_setting);
    //d.V3D_MainWindows->removeTab(2); //hide page 2 (0-based)
    if (d.exec()==QDialog::Accepted) d.fetchData(&global_setting);
}
void MainWindow::func_proc_plugin_manager() {if (pluginLoader) pluginLoader->aboutPlugins();}
void MainWindow::func_procIO_export_to_vano_format() {if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- save to VANO annotation files"));}
void MainWindow::func_procIO_export_to_movie() {if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- save to movie"));}
void MainWindow::func_procIO_export_landmark_to_pointcloud() {if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Export landmarks to point cloud (APO) file"));}
void MainWindow::func_procIO_export_landmark_to_swc() {if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Export landmarks and their relationship to graph (SWC) file"));}
void MainWindow::func_procIO_export_tracedneuron_to_swc() {if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Export traced neuron or fibrous structures to graph (SWC) file"));}
void MainWindow::func_procGeneral_rotate_paxis(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Rotate image so that principal axis is horizontal"));}
void MainWindow::func_procGeneral_rotate_angle(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Rotate image an arbitrary degree"));}
void MainWindow::func_procGeneral_flip(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Flip image"));}
void MainWindow::func_procGeneral_clear_all_landmark(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- clear all landmarks (and of course the connectivity map of landmarks)"));}
void MainWindow::func_procGeneral_toggle_landmark_label(){if (activeMdiChild()) activeMdiChild()->toggleLandmarkLabelDisp();}
void MainWindow::func_procGeneral_clear_connectmap(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- clear the enitre connectivity map of landmarks"));}
void MainWindow::func_procGeneral_rescale_landmarks_only(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- rescale landmarks only (without resampling image)"));}
void MainWindow::func_procGeneral_automarker_entireimg(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- automarker for entire image"));}
void MainWindow::func_procGeneral_automarker_roi(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- automarker for roi"));}
void MainWindow::func_procGeneral_split_channels(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- split channels"));}
void MainWindow::func_procGeneral_extract_a_channel(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- extract one channel"));}
void MainWindow::func_procGeneral_crop_image_minMaxBox(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- crop image via input min-max bounds"));}
void MainWindow::func_procGeneral_crop_bbox_roi(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- crop image using minMax bounding box in 3D (derived from ROIs)"));}
void MainWindow::func_procGeneral_mask_roi(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- masking image using ROIs in 3D"));}
void MainWindow::func_procGeneral_mask_nonroi_xy(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- masking image using non-ROIs for all XY planes"));}
void MainWindow::func_procGeneral_mask_channel(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- masking image using channels"));}
void MainWindow::func_procGeneral_clear_roi(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- clear the ROI"));}
void MainWindow::func_procGeneral_resample_image(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- resample image (and also associated landmarks)"));}
void MainWindow::func_procGeneral_projection_max(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- projection (max)"));}
void MainWindow::func_procGeneral_blend_image(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- merge channels of multple images"));}
void MainWindow::func_procGeneral_stitch_image(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- stitch two images"));}
void MainWindow::func_procGeneral_display_histogram(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- display histogram"));}
void MainWindow::func_procGeneral_linear_adjustment(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- linear adjustment"));}
void MainWindow::func_procGeneral_histogram_equalization(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- histogram equalization"));}
void MainWindow::func_procGeneral_intensity_rescale(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- intensity scaling"));}
void MainWindow::func_procGeneral_intensity_threshold(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- intensity thresholding"));}
void MainWindow::func_procGeneral_intensity_binarize(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- intensity binarization"));}
void MainWindow::func_procGeneral_intensity_updateminmax(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- intensity minmaxvalue recomputing"));}
void MainWindow::func_procGeneral_color_invert(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- invert image color"));}
void MainWindow::func_procGeneral_scaleandconvert28bit(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- linear scaling to [0,255] and convert to 8 bit"));}
void MainWindow::func_procGeneral_scaleandconvert28bit_1percent(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- linear scaling to [0,255], convert to 8 bit using 1-percent saturation"));}
void MainWindow::func_procGeneral_16bit_to_8bit(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- convert 16bit image to 8 bit"));}
void MainWindow::func_procGeneral_32bit_to_8bit(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- convert 32bit (single-precision float) image to 8 bit"));}
void MainWindow::func_procGeneral_indexedimg2rgb(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- convert indexed image to RGB"));}
//void MainWindow::func_procGeneral_open_image_in_windows(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- open another image/stack in *THIS* window"));}
void MainWindow::func_procGeneral_save_image(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- save to file"));}
void MainWindow::func_procElongated_randomSeeding(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Randomly seed landmarks/control-points"));}
void MainWindow::func_procElongated_minSpanTree(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Find MST (Minimum spanning tree) of landmarks"));}
void MainWindow::func_procElongated_mstDiameter(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Detect diameter graph of MST"));}
void MainWindow::func_procElongated_genCuttingPlaneLocations(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Find cutting plane locations"));}
void MainWindow::func_procElongated_restackingCuttingPlanes(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Straighten using slice-restacking"));}
void MainWindow::func_procElongated_bdbminus(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Adjust backbone graph using BDB_minus algorithm"));}
void MainWindow::func_procReg_gridSeeding(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Seed landmarks/control-points on regular grid"));}
void MainWindow::func_procReg_randomSeeding(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Seed landmarks/control-points randomly"));}
void MainWindow::func_procReg_bigGradient_edge_Seeding(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- (available soon) Seed landmarks/control-points using big gradient (edge) points"));}
void MainWindow::func_procReg_big_curvature_corner_Seeding(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- (available soon) Seed landmarks/control-points using big curvature (corner) points"));}
void MainWindow::func_procReg_fileSeeding(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Seed landmarks/control-points using a file"));}
void MainWindow::func_procReg_global_align(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Global affine alignment (using image content)"));}
void MainWindow::func_procReg_affine_markers_align(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Global affine alignment (using xform derived from matching landmarks)"));}
void MainWindow::func_procReg_flybrain_lobeseg(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Segmenting optical lobes of a fly brain (for a globally aligned fly brain)"));}
void MainWindow::func_procReg_detect_matching(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Match landmarks defined for another image (i.e. registration target)"));}
void MainWindow::func_procReg_detect_matching_1pt(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Match one single landmark in another image"));}
void MainWindow::func_procReg_warp_using_landmarks(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Warp image using corresponding landmarks"));}
void MainWindow::func_procReg_all_in_one_warp(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- ** All-in-one: detect corresponding landmarks and then warp"));}
void MainWindow::func_procTracing_topdownSkeleton(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- top-down skeletonization"));}
void MainWindow::func_procTracing_bottomupSearch(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- (available soon) bottom-up marching"));}
void MainWindow::func_procTracing_glocal_combine(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- (available soon) global-local (Global) integration of top-down/bottom-up results"));}
void MainWindow::func_procTracing_manualCorrect(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- (available soon) manual correction of fibrous segmentation"));}
void MainWindow::func_procTracing_APP2auto(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- Vaa3D-Neuron2 auto-tracing"));}
void MainWindow::func_procTracing_one2others(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- trace one marker to all others"));}
void MainWindow::func_procTracing_trace_a_curve(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- trace between two locations"));}
void MainWindow::func_procTracing_undo_laststep(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- undo last tracing step"));}
void MainWindow::func_procTracing_redo_laststep(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- redo last tracing step"));}
void MainWindow::func_procTracing_clear(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- clear traced neuron"));}
void MainWindow::func_procTracing_update3Dview(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- update 3D view of traced neuron"));}
void MainWindow::func_procTracing_save(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- save traced neuron"));}
void MainWindow::func_procCellSeg_localTemplate(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- local template matching"));}
void MainWindow::func_procCellSeg_cellcounting(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- cell counting (Yang Yu)"));}
void MainWindow::func_procCellSeg_watershed(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- watershed segmentation"));}
void MainWindow::func_procCellSeg_levelset(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- levelset segmentation"));}
void MainWindow::func_procCellSeg_Gaussian_fit_1_spot_1_Gauss(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- 1-Gaussian fit of current focus pos"));}
void MainWindow::func_procCellSeg_Gaussian_fit_1_spot_N_Gauss(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- N-Gaussian fit of current focus pos"));}
void MainWindow::func_procCellSeg_Gaussian_partition(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- (available soon) Gaussian partition"));}
void MainWindow::func_procCellSeg_manualCorrect(){if (activeMdiChild()) activeMdiChild()->popupImageProcessingDialog(tr(" -- (available soon) manual correction/identification of spherical structures"));}
#ifdef _ALLOW_WORKMODE_MENU_
// Mode
void MainWindow::func_procModeDefault()
{
    V3dApplication::deactivateNaMainWindow();
    V3dApplication::activateMainWindow();
}
void MainWindow::func_procModeNeuronAnnotator()
{
    V3dApplication::deactivateMainWindow();
    V3dApplication::activateNaMainWindow();
}
void MainWindow::setV3DDefaultModeCheck(bool checkState) {
    procModeDefault->setChecked(checkState);
}
void MainWindow::setNeuronAnnotatorModeCheck(bool checkState) {
    procModeNeuronAnnotator->setChecked(checkState);
}
#endif

#ifdef _ALLOW_TERAFLY_MENU_
void MainWindow::func_open_terafly()
{
    V3d_PluginLoader *pl = new V3d_PluginLoader(this);
    terafly::TeraFly::domenu("TeraFly", *pl, this);
}
void MainWindow::func_open_teraconverter()
{
    V3d_PluginLoader *pl = new V3d_PluginLoader(this);
    terafly::TeraFly::domenu("TeraConverter", *pl, this);
}
#endif

//class V3D_PlugIn_Interface
//{
//public:
//	void doit() {v3d_msg("do it");}
//};
//void MainWindow::func_procPC_Atlas_edit_atlaslinkerfile()
//{
//	apoAtlasLinkerInfoAll apo_atlas_info;
//	QString fileName = QFileDialog::getOpenFileName(this, tr("Open an Point Cloud Atlas File"),
//													"",
//													tr("point cloud atlas linker file (*.pc_atlas)"));
//    if (fileName.isEmpty())
//	{
//		v3d_msg("No file is selected. Do nothing.");
//		return;
//	}
//
//	if (!loadPointCloudAtlasInfoListFromFile(qPrintable(fileName.trimmed()), apo_atlas_info))
//	{
//		v3d_msg("Fail to load the specified point cloud atlas file.");
//		return;
//	}
//
//	func_procIO_import_atlas_apofolder(apo_atlas_info);//continue to edit
//}
//
//void MainWindow::func_procPC_Atlas_create_atlaslinkerfile()
//{
//	apoAtlasLinkerInfoAll apo_atlas_info;
//	func_procIO_import_atlas_apofolder(apo_atlas_info);
//}
//
//void MainWindow::func_procPC_Atlas_view_atlas(){}
void MainWindow::func_procPC_Atlas_view_atlas_computeVanoObjStat()
{
    //	QString inName = QFileDialog::getOpenFileName(this, tr("Open a Point Cloud Atlas File"),
    //													"",
    //													tr("point cloud annotation linker file (*.ano *)"));
    QString inName = QFileDialog::getExistingDirectory(this, tr("Select a directory contain .ano files"),"");
    QStringList listRecompute;
    QFileInfo fi_inName(inName);
    if (fi_inName.isDir())
    {
        //then search all .ano file under the current directory
        QDir d(inName);
        QStringList suffixList; suffixList << "*.ano" << "*.ANO";
        listRecompute = d.entryList(suffixList, QDir::Files, QDir::Name);
        for (int j=0;j<listRecompute.size();j++) //add the directory info in front of each entry
        {
            listRecompute.replace(j, QString(listRecompute.at(j)).prepend(inName+"/"));
            v3d_msg(tr("file %1 [%2]").arg(j+1).arg(listRecompute[j]), false);
        }
    }
    else if (fi_inName.isFile())
        listRecompute << inName; //then just insert the current file into the list
    else
    {
        v3d_msg(tr("The selected object [%1] is not a valid file or path. Do nothing").arg(inName));
        return;
    }
    //ask which channel to compute info
    bool ok1;

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
    int ch_ind = QInputDialog::getInt(this, tr("channel"),
                                          tr("The selected directory contains %1 .ano files. <br><br> which image channel to compute the image objects statistics?").arg(listRecompute.size()),
                                          1, 1, 3, 1, &ok1) - 1;
    //now do for every file
#else
    int ch_ind = QInputDialog::getInteger(this, tr("channel"),
                                          tr("The selected directory contains %1 .ano files. <br><br> which image channel to compute the image objects statistics?").arg(listRecompute.size()),
                                          1, 1, 3, 1, &ok1) - 1;
    //now do for every file
#endif
    My4DImage *grayimg=0, *maskimg=0;
    grayimg = new My4DImage;
    maskimg = new My4DImage;
    QProgressDialog progress;
#define PROGRESS_TEXT(text)   { QApplication::setActiveWindow(&progress);  progress.setLabelText( QString(text) );  progress.repaint();}
#define PROGRESS_PERCENT(i)	  { QApplication::setActiveWindow(&progress);  progress.setValue(i);  progress.repaint(); QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);}
    for (int k=0;k<listRecompute.size();k++)
    {
        PROGRESS_TEXT( QObject::tr("Processing %1 .ano file of %2: [%3]").arg(k+1).arg(listRecompute.size()).arg(listRecompute.at(k)) );
        PROGRESS_PERCENT( 90*(k+1)/listRecompute.size() );
        P_ObjectFileType cc;
        inName = listRecompute.at(k); //re-use the inName variable for convenience
        if (!loadAnoFile(inName, cc))
        {
            v3d_msg("Fail to load useful info from the specified anofile. Do nothing.\n");
            return;
        }
        if (cc.raw_image_file_list.size()<=0 || cc.labelfield_image_file_list.size()<=0 || cc.annotation_file_list.size()<=0)
        {
            v3d_msg("The information of at least one of the GRAYIMG, MASKIMG, or POINTCLOUD ANNOTATION files is invalid. Do nothing.\n");
            return;
        }
        else
        {
            printf("\n");
            printf("1st grayimg=[%s]\n", qPrintable(cc.raw_image_file_list.at(0)));
            printf("1st maskimg=[%s]\n", qPrintable(cc.labelfield_image_file_list.at(0)));
            printf("1st apofile=[%s]\n", qPrintable(cc.annotation_file_list.at(0)));
            printf("\n");
        }
        //load images and point cloud file
        QString tmpstr;
        QList <CellAPO> aporecord;
        grayimg->loadImage((char *)(qPrintable(cc.raw_image_file_list.at(0))));
        if (!grayimg->valid())
        {
            tmpstr = cc.raw_image_file_list.at(0); tmpstr.prepend("The specified GRAYIMG [").append("] cannot be successfully loaded. Skip.");
            v3d_msg(tmpstr);
            goto Label_exit;
        }
        if (ch_ind<0 || ch_ind>=grayimg->getCDim())
        {
            tmpstr = tr("The image has %1 channel but you ask to collect the %2 channel info which out of range. Do nothing.").arg(grayimg->getCDim()).arg(ch_ind+1);
            v3d_msg(tmpstr);
        }
        maskimg->loadImage((char *)(qPrintable(cc.labelfield_image_file_list.at(0))));
        if (!maskimg->valid())
        {
            tmpstr = cc.labelfield_image_file_list.at(0); tmpstr.prepend("The specified MASKIMG [").append("] cannot be successfully loaded. Skip.");
            v3d_msg(tmpstr);
            v3d_msg("The specified MASKIMG [%s] cannot be successfully loaded. Skip.\n");
            goto Label_exit;
        }
        aporecord = readAPO_file(cc.annotation_file_list.at(0));
        if (aporecord.size()<0)
        {
            tmpstr = cc.annotation_file_list.at(0); tmpstr.prepend("The specified ANOFILE [").append("] has no entry. Skip.");
            v3d_msg(tmpstr);
            goto Label_exit;
        }
        //do computation
        {
            LocationSimple * p_ano = 0;
            V3DLONG n_objects = 0;
            if (!compute_statistics_objects(grayimg, ch_ind, maskimg, p_ano, n_objects))
            {
                v3d_msg("Some errors happen during the computation of image objects' statistics. The annotation is not generated.");
                return;
            }
            if (n_objects != aporecord.size()+1) //this indicates there are an incorrect number of image objects in the MASKIMG that are not recorded in the respective annotation spreadsheet. Thus the file may corrupt.
                //note that +1 is because when I search how many objects, I always include 1 more for indexing convenience
            {
                v3d_msg(tr("The number of image objects = %1, diff from (1 + what recorded in the annotation file %2). Check your data!").arg(n_objects).arg(aporecord.size()));
                return;
            }
            //update the records in the apo-cell data structure
            CellAPO ca;
            for (V3DLONG i=0;i<aporecord.size();i++) //do not process 0 values, as it is background. Thus starts from 1
            {
                ca = aporecord.at(i);
                V3DLONG cur_ind = ca.n; //assuming the indexes match
                if (cur_ind<1 || cur_ind>n_objects) //then an error
                {
                    v3d_msg(tr("The index of the %1 cells/image-objecs seems being messed up. Skip one cell. Check your data.").arg(i+1));
                    continue;
                }
                ca.x = p_ano[cur_ind].x;
                ca.y = p_ano[cur_ind].y;
                ca.z = p_ano[cur_ind].z;		// point coordinates
                ca.pixmax = p_ano[cur_ind].pixmax;
                ca.intensity = p_ano[cur_ind].ave;
                ca.sdev = p_ano[cur_ind].sdev;
                ca.volsize = p_ano[cur_ind].size;
                ca.mass = p_ano[cur_ind].mass;
                //the folowing three remain unchanged
                //ca.orderinfo;
                //ca.name;
                //ca.QString comment;
                //put the record back to the array
                aporecord.replace(i, ca);
            }
            //delete the allocated memory
            if (p_ano) {delete []p_ano; p_ano=0;}
        }
        //save to file
        {
            //first save the new apo record
            QString newapofile = cc.annotation_file_list.at(0);
            QFileInfo fi(newapofile);
            newapofile = fi.absolutePath() + "/" + fi.completeBaseName().append("_new.apo");
            qDebug() << "*" << newapofile<<"*";
            if (!writeAPO_file(newapofile, aporecord))
            {
                tmpstr = newapofile; tmpstr.prepend("Fail to save the recomputed statistics of all images objects to the file [").append("]. Skip.");
                v3d_msg(tmpstr);
            }
            //then save the new annotation linker file
            QStringList commentStrList;
            tmpstr = cc.annotation_file_list.at(0);
            commentStrList.append(tmpstr.prepend("### ANOFILE="));
            cc.annotation_file_list.replace(0, newapofile);
            saveAnoFile(inName, cc, commentStrList);
        }
    }
    //finally free space
Label_exit:
    if (grayimg) {delete grayimg; grayimg=0;}
    if (maskimg) {delete maskimg; maskimg=0;}
    return;
}
// CMB 12-Nov-2010
// Intercept QFileOpenEvent on Mac - drag-onto-app
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FileOpen)
    {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent*>(event);
        QString fileName = openEvent->file();
        // v3d_msg("file open event: " + fileName);
        loadV3DFile(fileName, true, false); // loadV3DFile func changed to 3 args. YuY Nov. 18, 2010
        return true; // consume event
    }
    // Delegate to parent if we don't want to consume the event
    return QMainWindow::eventFilter(obj, event);
}
