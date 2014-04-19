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

#include "PMain.h"
#include "PDialogImport.h"
#include "PAbout.h"
#include "PLog.h"
#include "PAnoToolBar.h"
#include "../control/CImport.h"
#include "../control/CVolume.h"
#include "../control/CSettings.h"
#include "../control/CExplorerWindow.h"
#include "../control/CAnnotations.h"
#include "../control/V3Dsubclasses.h"
#include "renderer_gl1.h"
#include "v3dr_mainwindow.h"
#include <typeinfo>
#include "../core/ImageManager/TimeSeries.h"

using namespace teramanager;

string PMain::HTwelcome = "Go to <i>File->Open volume</i> and select the directory of any resolution. To change volume import options, go to <i>Options->Import</i>.";
//string PMain::HTbase =    "<u>Navigate through different resolutions by</u>:<br><br>"
//                          "<b>zoom-in</b>: right-click-><i>Zoom-in HighRezImage</i> on image/marker;<br>"
//                          "<b>zoom-out</b>: mouse scroll down;<br>"
//                          "<b>jump to res</b>: select VOI with volume cut scrollbars/spinboxes and choose resolution from pull-down menu.<br><br>";
string PMain::HTbase =    "<b>What's this?</b><br><i>Move the mouse over an object and its description will be displayed here.</i>";
string PMain::HTvoiDim =  "Set the <b>dimensions</b> (in voxels) of the volume of interest (<b>VOI</b>) to be loaded when zoomin-in. "
                          "Please be careful not to set a too big region or you will soon use up your <b>graphic card's memory</b>. ";
string PMain::HTjumpToRes = "Choose from pull-down menu the <b>resolution</b> you want to jump to and the displayed image will be loaded at the resolution selected. "
                            "To load only a volume of interest (<b>VOI</b>) at the selected resolution, you may use the Vaa3D <i>Volume Cut</i> scrollbars "
                            "or the <i>VOI's coordinates</i> spinboxes embedded in this plugin.";
string PMain::HTzoomOutThres = "Select the <b>zoom</b> factor threshold to restore the lower resolution when zooming-out with <i>mouse scroll down</i>. The default is set to 0. "
                           "Set it to -100 to disable this feature.";
string PMain::HTzoomInThres = "Select the <b>zoom</b> factor threshold to trigger the higher resolution when zooming-in with <i>mouse scroll up</i>. The default is set to 50. "
                             "Set it to 100 to disable this feature.";
string PMain::HTzoomInMethod = "Choose from pull-down menu the method to be used for the computation of the zoom-in volume of interest (<b>VOI</b>)";
string PMain::HTcacheSens = "Adjust data caching sensitivity when zooming-in with <i>mouse scroll up</i>. This controls the minimum amount of overlap between the requested VOI "
                            " and the <b>cached VOI</b> that is required to restore the cached VOI instead of loading a new VOI. If you always want to zoom-in to the cached VOI, please set this to 0\%.";
string PMain::HTtraslatePos = "Translate the view along this axis in its <i>natural</i> direction.";
string PMain::HTtraslateNeg = "Translate the view along this axis in its <i>opposite</i> direction.";
string PMain::HTvolcuts = "Define a volume of interest (<b>VOI</b>) using <b>absolute spatial coordinates</b> (i.e. referred to the highest resolution). "
                          "You may then choose the resolution you want to display it from the <i>Jump to res</i> pull-down menu.";
string PMain::HTrefsys = "Rotate the reference system";
string PMain::HTresolution = "A heat map like bar that indicates the currently displayed resolution (the \"hotter\", the higher)";

PMain* PMain::uniqueInstance = 0;
PMain* PMain::instance(V3DPluginCallback2 *callback, QWidget *parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    if (uniqueInstance == 0)
        uniqueInstance = new PMain(callback, parent);
    else
    {
        uniqueInstance->setWindowState(Qt::WindowNoState);
        uniqueInstance->raise();
        uniqueInstance->activateWindow();
        uniqueInstance->show();
        if(CExplorerWindow::getCurrent())
        {
            CExplorerWindow::getCurrent()->window3D->setWindowState(Qt::WindowNoState);
            CExplorerWindow::getCurrent()->window3D->raise();
            CExplorerWindow::getCurrent()->window3D->activateWindow();
            CExplorerWindow::getCurrent()->window3D->show();
            CExplorerWindow::getCurrent()->alignToLeft(uniqueInstance);
        }
        return uniqueInstance;
    }
}
PMain* PMain::getInstance()
{
    if(uniqueInstance)
        return uniqueInstance;
    else
    {
        itm::warning("TeraFly not yet instantiated", __itm__current__function__);
        QMessageBox::critical(0,QObject::tr("Error"), QObject::tr("TeraFly not yet instantiated"),QObject::tr("Ok"));
    }
}

void PMain::uninstance()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    CImport::uninstance();
    PDialogImport::uninstance();
    PAbout::uninstance();
    CVolume::uninstance();
    CExplorerWindow::uninstance();
    CSettings::uninstance();
    CAnnotations::uninstance();
    PLog::uninstance();
    PAnoToolBar::uninstance();
    if(uniqueInstance)
        delete uniqueInstance;
    uniqueInstance = 0;
}

PMain::~PMain()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);
}

PMain::PMain(V3DPluginCallback2 *callback, QWidget *parent) : QWidget(parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    //initializing members
    V3D_env = callback;
    parentWidget = parent;
    annotationsPathLRU = "";
    marginLeft = 85;

    //creating fonts
    QFont tinyFont = QApplication::font();
    #ifndef _USE_NATIVE_FONTS
    tinyFont.setPointSize(10);
    #endif

    //initializing menu
    /**/itm::debug(itm::LEV3, "initializing menu", __itm__current__function__);
    menuBar = new QMenuBar(0);
    /* --------------------------- "File" menu --------------------------- */
    fileMenu = menuBar->addMenu("File");
    openVolumeAction = new QAction("Open volume", this);
    openVolumeAction->setIcon(QIcon(":/icons/open_volume.png"));
    closeVolumeAction = new QAction("Close volume", this);
    closeVolumeAction->setIcon(QIcon(":/icons/close.png"));
    loadAnnotationsAction = new QAction("Load annotations", this);
    loadAnnotationsAction->setIcon(QIcon(":/icons/open_ano.png"));
    saveAnnotationsAction = new QAction("Save annotations", this);
    saveAnnotationsAction->setIcon(QIcon(":/icons/save.png"));
    saveAnnotationsAsAction = new QAction("Save annotations as", this);
    saveAnnotationsAsAction->setIcon(QIcon(":/icons/saveas.png"));
    clearAnnotationsAction = new QAction("Clear annotations", this);
    clearAnnotationsAction->setIcon(QIcon(":/icons/clear.png"));
    exitAction = new QAction("Quit", this);
    openVolumeAction->setShortcut(QKeySequence("Ctrl+O"));
    closeVolumeAction->setShortcut(QKeySequence("Ctrl+C"));
    loadAnnotationsAction->setShortcut(QKeySequence("Ctrl+L"));
    saveAnnotationsAction->setShortcut(QKeySequence("Ctrl+S"));
    saveAnnotationsAsAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    clearAnnotationsAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(openVolumeAction, SIGNAL(triggered()), this, SLOT(openVolume()));
    connect(closeVolumeAction, SIGNAL(triggered()), this, SLOT(closeVolume()));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(exit()));
    connect(loadAnnotationsAction, SIGNAL(triggered()), this, SLOT(loadAnnotations()));
    connect(saveAnnotationsAction, SIGNAL(triggered()), this, SLOT(saveAnnotations()));
    connect(saveAnnotationsAsAction, SIGNAL(triggered()), this, SLOT(saveAnnotationsAs()));
    connect(clearAnnotationsAction, SIGNAL(triggered()), this, SLOT(clearAnnotations()));
    fileMenu->addAction(openVolumeAction);
    recentVolumesMenu = new QMenu("Recent volumes");
    recentVolumesMenu->setIcon(QIcon(":/icons/open_volume_recent.png"));
    fileMenu->addMenu(recentVolumesMenu);
    fileMenu->addAction(closeVolumeAction);
    fileMenu->addSeparator();
    fileMenu->addAction(loadAnnotationsAction);
    fileMenu->addAction(saveAnnotationsAction);
    fileMenu->addAction(saveAnnotationsAsAction);
    fileMenu->addAction(clearAnnotationsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    /* ------------------------- "Options" menu -------------------------- */
    optionsMenu = menuBar->addMenu("Options");
    /* ------------------------- "Options" menu: Import ------------------ */
    importOptionsMenu = optionsMenu->addMenu("Import");
    regenMData_cAction = new QAction("Regenerate metadata", this);
    regenMData_cAction->setCheckable(true);
    importOptionsMenu-> addAction(regenMData_cAction);
    regenVMap_cAction = new QAction("Regenerate volume map", this);
    regenVMap_cAction->setCheckable(true);
    importOptionsMenu-> addAction(regenVMap_cAction);
    volMapSizeMenu = new QMenu("Maximum volume map size");
    volMapSizeWidget = new QWidgetAction(this);
    volMapSizeSBox = new QSpinBox();
    volMapSizeSBox->setMinimum(1);
    volMapSizeSBox->setValue(CSettings::instance()->getVolMapSizeLimit());
    volMapSizeSBox->setMaximum(500);
    volMapSizeSBox->setSuffix(" MVoxels");
    volMapSizeWidget->setDefaultWidget(volMapSizeSBox);
    volMapSizeMenu->addAction(volMapSizeWidget);
    importOptionsMenu->addMenu(volMapSizeMenu);
    /* ------------------------- "Options" menu: 3D ---------------------- */
    threeDMenu = optionsMenu->addMenu("3D");
    curvesMenu = threeDMenu->addMenu("Curves");
    curveAspectMenu = curvesMenu->addMenu("Aspect");
    curveDimsMenu = curvesMenu->addMenu("Skeleton width");
    curveAspectTube = new QAction("Tube", this);
    curveAspectSkeleton = new QAction("Skeleton", this);
    curveAspectTube->setCheckable(true);
    curveAspectSkeleton->setCheckable(true);
    QActionGroup* curveAspectMutex = new QActionGroup(this);
    curveAspectMutex->addAction(curveAspectTube);
    curveAspectMutex->addAction(curveAspectSkeleton);
    curveAspectMutex->setExclusive(true);
    curveAspectMenu->addAction(curveAspectTube);
    curveAspectMenu->addAction(curveAspectSkeleton);
    curveDimsWidget = new QWidgetAction(this);
    QSpinBox* curveDimsSpinBox = new QSpinBox();
    curveDimsSpinBox->setMinimum(1);
    curveDimsSpinBox->setMaximum(10);
    curveDimsSpinBox->setSuffix(" (pixels)");
    curveDimsWidget->setDefaultWidget(curveDimsSpinBox);
    curveDimsMenu->addAction(curveDimsWidget);
    curveAspectTube->setChecked(true);
    curveDimsSpinBox->setValue(1);
    connect(curveAspectTube, SIGNAL(changed()), this, SLOT(curveAspectChanged()));
    connect(curveAspectSkeleton, SIGNAL(changed()), this, SLOT(curveAspectChanged()));
    connect(curveDimsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(curveDimsChanged(int)));
    /* ------------------------- "Options" menu: directional shift ---------------- */
    DirectionalShiftsMenu = optionsMenu->addMenu("Directional shift");
    /* ------------------------------ x-shift ------------------------------------- */
    xShiftMenu = new QMenu("X-shift overlap");
    xShiftWidget = new QWidgetAction(this);
    xShiftSBox = new QSpinBox();
    xShiftSBox->setSuffix("\%");
    xShiftSBox->setMinimum(1);
    xShiftSBox->setMaximum(99);
    xShiftSBox->setValue(CSettings::instance()->getTraslX());
    xShiftWidget->setDefaultWidget(xShiftSBox);
    xShiftMenu->addAction(xShiftWidget);
    DirectionalShiftsMenu->addMenu(xShiftMenu);
    /* ------------------------------ y-shift ------------------------------------- */
    yShiftMenu = new QMenu("Y-shift overlap");
    yShiftWidget = new QWidgetAction(this);
    yShiftSBox = new QSpinBox();
    yShiftSBox->setSuffix("\%");
    yShiftSBox->setMinimum(1);
    yShiftSBox->setMaximum(99);
    yShiftSBox->setValue(CSettings::instance()->getTraslY());
    yShiftWidget->setDefaultWidget(yShiftSBox);
    yShiftMenu->addAction(yShiftWidget);
    DirectionalShiftsMenu->addMenu(yShiftMenu);
    /* ------------------------------ z-shift ------------------------------------- */
    zShiftMenu = new QMenu("Z-shift overlap");
    zShiftWidget = new QWidgetAction(this);
    zShiftSBox = new QSpinBox();
    zShiftSBox->setSuffix("\%");
    zShiftSBox->setMinimum(1);
    zShiftSBox->setMaximum(99);
    zShiftSBox->setValue(CSettings::instance()->getTraslZ());
    zShiftWidget->setDefaultWidget(zShiftSBox);
    zShiftMenu->addAction(zShiftWidget);
    DirectionalShiftsMenu->addMenu(zShiftMenu);
    /* ------------------------------ t-shift ------------------------------------- */
    tShiftMenu = new QMenu("T-shift overlap");
    tShiftWidget = new QWidgetAction(this);
    tShiftSBox = new QSpinBox();
    tShiftSBox->setSuffix("\%");
    tShiftSBox->setMinimum(0);
    tShiftSBox->setMaximum(99);
    tShiftSBox->setValue(CSettings::instance()->getTraslT());
    tShiftWidget->setDefaultWidget(tShiftSBox);
    tShiftMenu->addAction(tShiftWidget);
    DirectionalShiftsMenu->addMenu(tShiftMenu);


    // "Debug" menu
    debugMenu = menuBar->addMenu("Debug");
    /* --------------------------------- show log --------------------------------- */
    debugShowLogAction = new QAction("Show log", debugMenu);
    connect(debugShowLogAction, SIGNAL(triggered()), this, SLOT(showLogTriggered()));
    debugMenu->addAction(debugShowLogAction);    
    /* ------------------------------ streaming steps ----------------------------- */
    debugStreamingStepsMenu = new QMenu("Streaming steps");
    debugStreamingStepsActionWidget = new QWidgetAction(this);
    debugStreamingStepsSBox = new QSpinBox();
    debugStreamingStepsSBox->setMinimum(1);
    debugStreamingStepsSBox->setMaximum(10);
    debugStreamingStepsActionWidget->setDefaultWidget(debugStreamingStepsSBox);
    debugStreamingStepsMenu->addAction(debugStreamingStepsActionWidget);
    debugMenu->addMenu(debugStreamingStepsMenu);    
    /* --------------------------------- verbosity -------------------------------- */
    debugVerbosityMenu = new QMenu("Verbosity");
    debugVerbosityActionWidget = new QWidgetAction(this);
    debugVerbosityCBox = new QComboBox();
    debugVerbosityCBox->addItem("Silent mode");
    debugVerbosityCBox->addItem("Level 1");
    debugVerbosityCBox->addItem("Level 2");
    debugVerbosityCBox->addItem("Level 3");
    debugVerbosityCBox->addItem("Verbose");
    CSettings::instance()->readSettings();
    debugVerbosityActionWidget->setDefaultWidget(debugVerbosityCBox);
    debugVerbosityMenu->addAction(debugVerbosityActionWidget);
    connect(debugVerbosityCBox, SIGNAL(currentIndexChanged(int)), this, SLOT(verbosityChanged(int)));
    debugMenu->addMenu(debugVerbosityMenu);    
    /* ---------------------------- redirect to stdout ---------------------------- */
    debugRedirectSTDoutMenu = new QMenu("Redirect stdout to file at");
    debugRedirectSTDoutActionWidget = new QWidgetAction(this);
    debugRedirectSTDoutPath = new QLineEdit();
    debugRedirectSTDoutActionWidget->setDefaultWidget(debugRedirectSTDoutPath);
    debugRedirectSTDoutMenu->addAction(debugRedirectSTDoutActionWidget);
    connect(debugRedirectSTDoutPath, SIGNAL(textEdited(QString)), this, SLOT(debugRedirectSTDoutPathEdited(QString)));
    debugMenu->addMenu(debugRedirectSTDoutMenu);
    /* ------------------------------ debug action 1 ------------------------------ */
    debugAction1 = new QAction("Debug action", debugMenu);
    connect(debugAction1, SIGNAL(triggered()), this, SLOT(debugAction1Triggered()));
    debugMenu->addAction(debugAction1);
    /* --------------------- add gaussian noise to time series -------------------- */
    addGaussianNoiseToTimeSeries = new QAction("Add gaussian noise to time series", debugMenu);
    addGaussianNoiseToTimeSeries->setCheckable(true);
    connect(addGaussianNoiseToTimeSeries, SIGNAL(triggered()), this, SLOT(addGaussianNoiseTriggered()));
    debugMenu->addAction(addGaussianNoiseToTimeSeries);
    /* -------------------------------- time series ------------------------------- */
//    debugTimeSeriesMenu = new QMenu("Time series size");
//    debugTimeSeriesWidget = new QWidgetAction(this);
//    debugTimeSeriesSBox = new QSpinBox();
//    debugTimeSeriesSBox->setMinimum(1);
//    debugTimeSeriesSBox->setMaximum(500);
//    debugTimeSeriesSBox->setValue(100);
//    debugTimeSeriesWidget->setDefaultWidget(debugTimeSeriesSBox);
//    debugTimeSeriesMenu->addAction(debugTimeSeriesWidget);
//    debugMenu->addMenu(debugTimeSeriesMenu);

    helpMenu = menuBar->addMenu("Help");
    aboutAction = new QAction("About", this);
    aboutAction->setIcon(QIcon(":/icons/about.png"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    helpMenu->addAction(aboutAction);

    //toolbar
    toolBar = new QToolBar("ToolBar", this);
    toolBar->setOrientation(Qt::Vertical);

    QMenu *openMenu = new QMenu();
    std::list<string> recentVolumes = CSettings::instance()->getVolumePathHistory();
    for(std::list<string>::reverse_iterator it = recentVolumes.rbegin(); it != recentVolumes.rend(); it++)
    {
        QAction *action = new QAction(it->c_str(), this);
        connect(action, SIGNAL(triggered()), this, SLOT(openVolumeActionTriggered()));
        recentVolumesMenu->addAction(action);
    }
    clearRecentVolumesAction = new QAction("Clear menu",recentVolumesMenu);
    connect(clearRecentVolumesAction, SIGNAL(triggered()), this, SLOT(clearRecentVolumesTriggered()));
    recentVolumesMenu->addSeparator();
    recentVolumesMenu->addAction(clearRecentVolumesAction);

    openMenu->addAction(openVolumeAction);
    openMenu->addMenu(recentVolumesMenu);
    openVolumeToolButton = new QToolButton();
    openVolumeToolButton->setMenu(openMenu);
    openVolumeToolButton->setPopupMode(QToolButton::InstantPopup);
    openVolumeToolButton->setIcon(QIcon(":/icons/open_volume.png"));
    toolBar->insertWidget(0, openVolumeToolButton);

    toolBar->insertAction(0, closeVolumeAction);
    toolBar->addAction(loadAnnotationsAction);
    toolBar->addAction(saveAnnotationsAction);
    toolBar->addAction(saveAnnotationsAsAction);
    toolBar->addAction(clearAnnotationsAction);
    toolBar->addAction(aboutAction);
    toolBar->setIconSize(QSize(30,30));
    toolBar->setStyleSheet("QToolBar{background:qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                           "stop: 0 rgb(150,150,150), stop: 1 rgb(190,190,190)); border: none}");

    // TAB widget: where to store pages
    tabs = new QTabWidget(this);

    //Page "Volume's info": contains informations of the loaded volume    
    /**/itm::debug(itm::LEV3, "Page \"Volume's info\"", __itm__current__function__);
    info_page = new QWidget();
    vol_size_voxel_label = new QLabel();
    vol_size_voxel_field = new QLabel();
    vol_size_voxel_field->setAlignment(Qt::AlignCenter);
    vol_size_voxel_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    vol_size_voxel_field->setFont(tinyFont);
    vol_size_files_label = new QLabel();
    vol_size_files_field = new QLabel();
    vol_size_files_field->setAlignment(Qt::AlignCenter);
    vol_size_files_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    vol_size_files_field->setFont(tinyFont);
    vol_size_bytes_label = new QLabel();
    vol_size_bytes_field = new QLabel();
    vol_size_bytes_field->setAlignment(Qt::AlignCenter);
    vol_size_bytes_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    vol_size_bytes_field->setFont(tinyFont);
    volume_dims_label = new QLabel("Dims (vxl):");
    direction_V_label_0 = new QLabel("(Y)");
    direction_H_label_0 = new QLabel("(X)");
    direction_D_label_0 = new QLabel("(Z)");
    by_label_01 = new QLabel(QChar(0x00D7));
    by_label_02 = new QLabel(QChar(0x00D7));
    vol_height_field = new QLabel();
    vol_height_field->setAlignment(Qt::AlignCenter);
    vol_height_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    vol_height_field->setFont(tinyFont);
    vol_width_field = new QLabel();
    vol_width_field->setAlignment(Qt::AlignCenter);
    vol_width_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    vol_width_field->setFont(tinyFont);
    vol_depth_field = new QLabel();
    vol_depth_field->setAlignment(Qt::AlignCenter);
    vol_depth_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    vol_depth_field->setFont(tinyFont);
    vol_height_mm_field = new QLabel();
    vol_height_mm_field->setAlignment(Qt::AlignCenter);
    vol_height_mm_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    vol_height_mm_field->setFont(tinyFont);
    vol_width_mm_field = new QLabel();
    vol_width_mm_field->setAlignment(Qt::AlignCenter);
    vol_width_mm_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    vol_width_mm_field->setFont(tinyFont);
    vol_depth_mm_field = new QLabel();
    vol_depth_mm_field->setAlignment(Qt::AlignCenter);
    vol_depth_mm_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    vol_depth_mm_field->setFont(tinyFont);
    volume_stacks_label = new QLabel("Number of stacks:");
    direction_V_label_1 = new QLabel("(Y)");
    direction_H_label_1 = new QLabel("(X)");
    by_label_1 = new QLabel(QChar(0x00D7));
    nrows_field = new QLabel();
    nrows_field->setAlignment(Qt::AlignCenter);
    nrows_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    nrows_field->setFont(tinyFont);
    ncols_field = new QLabel();
    ncols_field->setAlignment(Qt::AlignCenter);
    ncols_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    ncols_field->setFont(tinyFont);
    stacks_dims_label = new QLabel("Stacks' dims (vxl):");
    direction_V_label_2 = new QLabel("(Y)");
    direction_H_label_2 = new QLabel("(X)");
    direction_D_label_2 = new QLabel("(Z)");
    by_label_2 = new QLabel(QChar(0x00D7));
    by_label_3 = new QLabel(QChar(0x00D7));
    stack_height_field = new QLabel();
    stack_height_field->setAlignment(Qt::AlignCenter);
    stack_height_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    stack_height_field->setFont(tinyFont);
    stack_width_field = new QLabel();
    stack_width_field->setAlignment(Qt::AlignCenter);
    stack_width_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    stack_width_field->setFont(tinyFont);
    stack_depth_field = new QLabel();
    stack_depth_field->setAlignment(Qt::AlignCenter);
    stack_depth_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    stack_depth_field->setFont(tinyFont);
    voxel_dims_label = new QLabel(QString("Voxel's dims (").append(QChar(0x03BC)).append("m):"));
    direction_V_label_3 = new QLabel("(Y)");
    direction_H_label_3 = new QLabel("(X)");
    direction_D_label_3 = new QLabel("(Z)");
    by_label_4 = new QLabel(QChar(0x00D7));
    by_label_5 = new QLabel(QChar(0x00D7));
    vxl_V_field = new QLabel();
    vxl_V_field->setAlignment(Qt::AlignCenter);
    vxl_V_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    vxl_V_field->setFont(tinyFont);
    vxl_H_field = new QLabel();
    vxl_H_field->setAlignment(Qt::AlignCenter);
    vxl_H_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    vxl_H_field->setFont(tinyFont);
    vxl_D_field = new QLabel();
    vxl_D_field->setAlignment(Qt::AlignCenter);
    vxl_D_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    vxl_D_field->setFont(tinyFont);
    origin_label = new QLabel("Origin (mm):");
    direction_V_label_4 = new QLabel("(Y)");
    direction_H_label_4 = new QLabel("(X)");
    direction_D_label_4 = new QLabel("(Z)");
    org_V_field = new QLabel();
    org_V_field->setAlignment(Qt::AlignCenter);
    org_V_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    org_V_field->setFont(tinyFont);
    org_H_field = new QLabel();
    org_H_field->setAlignment(Qt::AlignCenter);
    org_H_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    org_H_field->setFont(tinyFont);
    org_D_field = new QLabel();
    org_D_field->setAlignment(Qt::AlignCenter);
    org_D_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    org_D_field->setFont(tinyFont);

    //Page "Controls": contains navigation controls
    /**/itm::debug(itm::LEV3, "Page \"Controls\"", __itm__current__function__);
    /* ------- local viewer panel widgets ------- */
    controls_page = new QWidget();
    localViewer_panel = new QGroupBox("Viewer");
    gradientBar = new QGradientBar(this);
    gradientBar->installEventFilter(this);
    Vdim_sbox = new QSpinBox();
    Vdim_sbox->setAlignment(Qt::AlignCenter);
    Vdim_sbox->setMaximum(1000);
    Vdim_sbox->setValue(CSettings::instance()->getVOIdimV());
    Vdim_sbox->setSuffix(" (Y)");
    Vdim_sbox->installEventFilter(this);
    Hdim_sbox = new QSpinBox();
    Hdim_sbox->setAlignment(Qt::AlignCenter);
    Hdim_sbox->setMaximum(1000);
    Hdim_sbox->setValue(CSettings::instance()->getVOIdimH());
    Hdim_sbox->setSuffix(" (X)");
    Hdim_sbox->installEventFilter(this);
    Ddim_sbox = new QSpinBox();
    Ddim_sbox->setAlignment(Qt::AlignCenter);
    Ddim_sbox->setMaximum(1000);
    Ddim_sbox->setValue(CSettings::instance()->getVOIdimD());
    Ddim_sbox->setSuffix(" (Z)");
    Ddim_sbox->installEventFilter(this);
    Tdim_sbox = new QSpinBox();
    Tdim_sbox->setAlignment(Qt::AlignCenter);
    Tdim_sbox->setMaximum(1000);
    Tdim_sbox->setMinimum(1);
    Tdim_sbox->setValue(CSettings::instance()->getVOIdimT());
    Tdim_sbox->setSuffix(" (t)");
    Tdim_sbox->installEventFilter(this);
    resolution_cbox = new QComboBox();
    resolution_cbox->installEventFilter(this);

    /* ------- zoom options panel widgets ------- */
    /**/itm::debug(itm::LEV3, "zoom options panel", __itm__current__function__);
    zoom_panel = new QGroupBox("Zoom-in/out");
    zoomOutSens = new QSlider(Qt::Horizontal, this);
    zoomOutSens->setTickPosition(QSlider::TicksBelow);
    zoomOutSens->setMinimum(-100);
    zoomOutSens->setMaximum(0);
    zoomOutSens->setSingleStep(10);
    zoomOutSens->setPageStep(20);
    zoomOutSens->installEventFilter(this);
    zoomInSens = new QSlider(Qt::Horizontal);
    zoomInSens->setTickPosition(QSlider::TicksBelow);
    zoomInSens->setMinimum(0);
    zoomInSens->setMaximum(100);
    zoomInSens->setSingleStep(10);
    zoomInSens->setPageStep(20);
    zoomInSens->installEventFilter(this);
    cacheSens = new QSlider(Qt::Horizontal, this);
    cacheSens->setTickPosition(QSlider::TicksBelow);
    cacheSens->setMinimum(0);
    cacheSens->setMaximum(100);
    cacheSens->setSingleStep(5);
    cacheSens->setPageStep(5);
    cacheSens->installEventFilter(this);
    controlsResetButton = new QPushButton(this);
    controlsResetButton->setIcon(QIcon(":/icons/reset.png"));
    zoomInMethod = new QComboBox();
    zoomInMethod->addItem("WYSIWYG (5 markers)");
    zoomInMethod->addItem("Foreground (20 markers + mean-shift)");
    zoomInMethod->addItem("Foreground (1 marker)");
    zoomInMethod->installEventFilter(this);
    #ifndef USE_EXPERIMENTAL_FEATURES
    zoomInMethod->setCurrentIndex(0);
    zoomInMethod->setEnabled(false);
    zoomInSens->setEnabled(false);
    #else
    zoomInMethod->setCurrentIndex(1);
    #endif

    //"Global coordinates" widgets
    /**/itm::debug(itm::LEV3, "\"VOI's coordinates\" panel", __itm__current__function__);
    globalCoord_panel = new QGroupBox("VOI's coordinates");
    traslXpos = new QArrowButton(this, QColor(255,0,0), 15, 6, 0, Qt::LeftToRight, true);
    traslXneg = new QArrowButton(this, QColor(255,0,0), 15, 6, 0, Qt::RightToLeft, true);
    traslXlabel = new QLabel("");
    traslYpos = new QArrowButton(this, QColor(0,200,0), 15, 6, 0, Qt::LeftToRight, true);
    traslYneg = new QArrowButton(this, QColor(0,200,0), 15, 6, 0, Qt::RightToLeft, true);
    traslYlabel = new QLabel("");
    traslZpos = new QArrowButton(this, QColor(0,0,255), 15, 6, 0, Qt::LeftToRight, true);
    traslZneg = new QArrowButton(this, QColor(0,0,255), 15, 6, 0, Qt::RightToLeft, true);
    traslZlabel = new QLabel("");
    traslTpos = new QArrowButton(this, QColor(150,150,150), 15, 6, 0, Qt::LeftToRight, true);
    traslTneg = new QArrowButton(this, QColor(150,150,150), 15, 6, 0, Qt::RightToLeft, true);
    traslTlabel = new QLabel("");
    traslXpos->installEventFilter(this);
    traslXneg->installEventFilter(this);
    traslYpos->installEventFilter(this);
    traslYneg->installEventFilter(this);
    traslZpos->installEventFilter(this);
    traslZneg->installEventFilter(this);
    traslTpos->installEventFilter(this);
    traslTneg->installEventFilter(this);
    V0_sbox = new QSpinBox();
    V0_sbox->setAlignment(Qt::AlignCenter);
    V1_sbox = new QSpinBox();
    V1_sbox->setAlignment(Qt::AlignCenter);
    H0_sbox = new QSpinBox();
    H0_sbox->setAlignment(Qt::AlignCenter);
    H1_sbox = new QSpinBox();
    H1_sbox->setAlignment(Qt::AlignCenter);
    D0_sbox = new QSpinBox();
    D0_sbox->setAlignment(Qt::AlignCenter);
    D1_sbox = new QSpinBox();
    D1_sbox->setAlignment(Qt::AlignCenter);
    T0_sbox = new QLineEdit();
    T0_sbox->setAlignment(Qt::AlignCenter);
    T0_sbox->setReadOnly(true);
    T1_sbox = new QLineEdit();
    T1_sbox->setAlignment(Qt::AlignCenter);
    T1_sbox->setReadOnly(true);
    V0_sbox->installEventFilter(this);
    V1_sbox->installEventFilter(this);
    H0_sbox->installEventFilter(this);
    H1_sbox->installEventFilter(this);
    D0_sbox->installEventFilter(this);
    D1_sbox->installEventFilter(this);
    T0_sbox->installEventFilter(this);
    T1_sbox->installEventFilter(this);
    to_label_1 = new QLabel("to");
    to_label_1->setAlignment(Qt::AlignCenter);
    to_label_2 = new QLabel("to");
    to_label_2->setAlignment(Qt::AlignCenter);
    to_label_3 = new QLabel("to");
    to_label_3->setAlignment(Qt::AlignCenter);
    to_label_4 = new QLabel("to");
    to_label_4->setAlignment(Qt::AlignCenter);
    refSys = new QGLRefSys(tabs);
    refSys->installEventFilter(this);
    frameCoord = new QLineEdit();
    frameCoord->setReadOnly(true);
    frameCoord->setAlignment(Qt::AlignCenter);

    /* ------- global coord panel widgets ------- */
    ESPanel = new QGroupBox("Sliding viewer");
    ESbutton = new QPushButton("click me");
    ESblockSpbox = new QSpinBox();
    ESblockSpbox->setAlignment(Qt::AlignCenter);
    ESoverlapSpbox = new QSpinBox();
    ESoverlapSpbox->setAlignment(Qt::AlignCenter);
    ESoverlapSpbox->setSuffix("\%");
    ESoverlapSpbox->setPrefix("overlap ");
    ESoverlapSpbox->setValue(20);
    ESoverlapSpbox->setMinimum(0);
    ESoverlapSpbox->setMaximum(50);
    ESmethodCbox = new QComboBox();
    ESmethodCbox->addItem("XYZ");
    ESmethodCbox->setEditable(true);
    ESmethodCbox->lineEdit()->setAlignment(Qt::AlignHCenter);
    for (int i = 0; i < ESmethodCbox->count(); ++i)
        ESmethodCbox->setItemData(i, Qt::AlignHCenter, Qt::TextAlignmentRole);

    //other widgets
    helpBox = new QHelpBox(this);
    progressBar = new QProgressBar(this);
    statusBar = new QStatusBar();

    //****LAYOUT SECTIONS****
    /**/itm::debug(itm::LEV3, "Layouting", __itm__current__function__);

    //Page "Volume's info": contains informations of the loaded volume
    QGridLayout* info_panel_layout = new QGridLayout();

    info_panel_layout->addWidget(new QLabel("Size (in files):"), 0,0,1,1);
    info_panel_layout->addWidget(vol_size_files_field,           0,2,1,2);
    info_panel_layout->addWidget(vol_size_files_label,           0,4,1,2);
    info_panel_layout->addWidget(vol_size_voxel_field,           0,6,1,2);
    info_panel_layout->addWidget(vol_size_voxel_label,           0,8,1,2);

//    info_panel_layout->addWidget(new QLabel("Size (in memory):"),1,0,1,1);
//    info_panel_layout->addWidget(vol_size_bytes_field,           1,2,1,2);
//    info_panel_layout->addWidget(vol_size_bytes_label,           1,4,1,2);

    info_panel_layout->addWidget(new QLabel("Dims (mm):"),      1,0,1,1);
    info_panel_layout->addWidget(vol_width_mm_field,            1,2,1,2);
    info_panel_layout->addWidget(new QLabel("(X)"),             1,4,1,1);
    info_panel_layout->addWidget(new QLabel(QChar(0x00D7)),     1,5,1,1);
    info_panel_layout->addWidget(vol_height_mm_field,           1,6,1,2);
    info_panel_layout->addWidget(new QLabel("(Y)"),             1,8,1,1);
    info_panel_layout->addWidget(new QLabel(QChar(0x00D7)),     1,9,1,1);
    info_panel_layout->addWidget(vol_depth_mm_field,            1,10,1,2);
    info_panel_layout->addWidget(new QLabel("(Z)"),             1,12,1,1);

    volume_dims_label->setFixedWidth(marginLeft);
    info_panel_layout->addWidget(volume_dims_label,     2,0,1,1);
    info_panel_layout->addWidget(vol_width_field,       2,2,1,2);
    info_panel_layout->addWidget(direction_H_label_0,   2,4,1,1);
    info_panel_layout->addWidget(by_label_01,           2,5,1,1);
    info_panel_layout->addWidget(vol_height_field,      2,6,1,2);
    info_panel_layout->addWidget(direction_V_label_0,   2,8,1,1);
    info_panel_layout->addWidget(by_label_02,           2,9,1,1);
    info_panel_layout->addWidget(vol_depth_field,       2,10,1,2);
    info_panel_layout->addWidget(direction_D_label_0,   2,12,1,1);

    info_panel_layout->addWidget(volume_stacks_label,   3,0,1,1);
    info_panel_layout->addWidget(ncols_field,           3,2,1,2);
    info_panel_layout->addWidget(direction_H_label_1,   3,4,1,1);
    info_panel_layout->addWidget(by_label_1,            3,5,1,1);
    info_panel_layout->addWidget(nrows_field,           3,6,1,2);
    info_panel_layout->addWidget(direction_V_label_1,   3,8,1,1);

    info_panel_layout->addWidget(stacks_dims_label,     4,0,1,1);
    info_panel_layout->addWidget(stack_width_field,     4,2,1,2);
    info_panel_layout->addWidget(direction_H_label_2,   4,4,1,1);
    info_panel_layout->addWidget(by_label_2,            4,5,1,1);
    info_panel_layout->addWidget(stack_height_field,    4,6,1,2);
    info_panel_layout->addWidget(direction_V_label_2,   4,8,1,1);
    info_panel_layout->addWidget(by_label_3,            4,9,1,1);
    info_panel_layout->addWidget(stack_depth_field,     4,10,1,2);
    info_panel_layout->addWidget(direction_D_label_2,   4,12,1,1);

    info_panel_layout->addWidget(voxel_dims_label,      5,0,1,1);
    info_panel_layout->addWidget(vxl_H_field,           5,2,1,2);
    info_panel_layout->addWidget(direction_H_label_3,   5,4,1,1);
    info_panel_layout->addWidget(by_label_4,            5,5,1,1);
    info_panel_layout->addWidget(vxl_V_field,           5,6,1,2);
    info_panel_layout->addWidget(direction_V_label_3,   5,8,1,1);
    info_panel_layout->addWidget(by_label_5,            5,9,1,1);
    info_panel_layout->addWidget(vxl_D_field,           5,10,1,2);
    info_panel_layout->addWidget(direction_D_label_3,   5,12,1,1);

    info_panel_layout->addWidget(origin_label,          6,0,1,1);
    info_panel_layout->addWidget(org_H_field,           6,2,1,2);
    info_panel_layout->addWidget(direction_H_label_4,   6,4,1,1);
    info_panel_layout->addWidget(org_V_field,           6,6,1,2);
    info_panel_layout->addWidget(direction_V_label_4,   6,8,1,1);
    info_panel_layout->addWidget(org_D_field,           6,10,1,2);
    info_panel_layout->addWidget(direction_D_label_4,   6,12,1,1);

    QVBoxLayout* info_page_layout = new QVBoxLayout(info_page);
    info_page_layout->addLayout(info_panel_layout, 0);
    info_page_layout->addStretch(1);
    info_page->setLayout(info_page_layout);
    #ifndef _USE_NATIVE_FONTS
    info_page->setStyle(new QWindowsStyle());
    #endif

    // "Global coordinates" panel layout
    QGridLayout* global_coordinates_layout = new QGridLayout();
    global_coordinates_layout->setVerticalSpacing(2);
    /* ------------- fix left block elements size ---------------- */
    QWidget* refSysContainer = new QWidget();
    refSysContainer->setFixedWidth(marginLeft);
    refSysContainer->setStyleSheet(" border-style: solid; border-width: 1px; border-color: rgb(150,150,150);");
    QHBoxLayout* refSysContainerLayout = new QHBoxLayout();
    refSysContainerLayout->setContentsMargins(1,1,1,1);
    refSysContainerLayout->addWidget(refSys, 1);
    refSysContainer->setLayout(refSysContainerLayout);
    frameCoord->setFixedWidth(marginLeft);
    /* ------------ fix central block elements size -------------- */
    QHBoxLayout *xShiftLayout = new QHBoxLayout();
    xShiftLayout->setContentsMargins(0,0,0,0);
    int fixedArrowWidth = 25;
    traslXneg->setFixedWidth(fixedArrowWidth);
    traslXpos->setFixedWidth(fixedArrowWidth);
    xShiftLayout->addStretch();
    xShiftLayout->addWidget(traslXneg, 0);
    xShiftLayout->addWidget(traslXlabel, 0);
    xShiftLayout->addWidget(traslXpos, 0);
    xShiftLayout->addStretch();
    xShiftLayout->setSpacing(5);
    QWidget* xShiftWidget = new QWidget();
    xShiftWidget->setFixedWidth(marginLeft);
    xShiftWidget->setLayout(xShiftLayout);
    QHBoxLayout *yShiftLayout = new QHBoxLayout();
    yShiftLayout->setContentsMargins(0,0,0,0);
    traslYneg->setFixedWidth(fixedArrowWidth);
    traslYpos->setFixedWidth(fixedArrowWidth);
    yShiftLayout->addStretch();
    yShiftLayout->addWidget(traslYneg, 0);
    yShiftLayout->addWidget(traslYlabel, 0);
    yShiftLayout->addWidget(traslYpos, 0);
    yShiftLayout->addStretch();
    yShiftLayout->setSpacing(5);
    QWidget* yShiftWidget = new QWidget();
    yShiftWidget->setFixedWidth(marginLeft);
    yShiftWidget->setLayout(yShiftLayout);
    QHBoxLayout *zShiftLayout = new QHBoxLayout();
    zShiftLayout->setContentsMargins(0,0,0,0);
    traslZneg->setFixedWidth(fixedArrowWidth);
    traslZpos->setFixedWidth(fixedArrowWidth);
    zShiftLayout->addStretch();
    zShiftLayout->addWidget(traslZneg, 0);
    zShiftLayout->addWidget(traslZlabel, 0);
    zShiftLayout->addWidget(traslZpos, 0);
    zShiftLayout->addStretch();
    zShiftLayout->setSpacing(5);
    QWidget* zShiftWidget = new QWidget();
    zShiftWidget->setFixedWidth(marginLeft);
    zShiftWidget->setLayout(zShiftLayout);
    QHBoxLayout *tShiftLayout = new QHBoxLayout();
    tShiftLayout->setContentsMargins(0,0,0,0);
    traslTneg->setFixedWidth(fixedArrowWidth);
    traslTpos->setFixedWidth(fixedArrowWidth);
    tShiftLayout->addStretch();
    tShiftLayout->addWidget(traslTneg, 0);
    tShiftLayout->addWidget(traslTlabel, 0);
    tShiftLayout->addWidget(traslTpos, 0);
    tShiftLayout->addStretch();
    tShiftLayout->setSpacing(5);
    QWidget* tShiftWidget = new QWidget();
    tShiftWidget->setFixedWidth(marginLeft);
    tShiftWidget->setLayout(tShiftLayout);
    /* ------------- fix right block elements size --------------- */
    QHBoxLayout *xGlobalCoordLayout = new QHBoxLayout();
    xGlobalCoordLayout->setSpacing(5);
    xGlobalCoordLayout->setContentsMargins(0,0,0,0);
    xGlobalCoordLayout->addWidget(H0_sbox, 1);
    xGlobalCoordLayout->addWidget(to_label_1, 0);
    xGlobalCoordLayout->addWidget(H1_sbox, 1);
    QHBoxLayout *yGlobalCoordLayout = new QHBoxLayout();
    yGlobalCoordLayout->setSpacing(5);
    yGlobalCoordLayout->setContentsMargins(0,0,0,0);
    yGlobalCoordLayout->addWidget(V0_sbox, 1);
    yGlobalCoordLayout->addWidget(to_label_2, 0);
    yGlobalCoordLayout->addWidget(V1_sbox, 1);
    QHBoxLayout *zGlobalCoordLayout = new QHBoxLayout();
    zGlobalCoordLayout->setSpacing(5);
    zGlobalCoordLayout->setContentsMargins(0,0,0,0);
    zGlobalCoordLayout->addWidget(D0_sbox, 1);
    zGlobalCoordLayout->addWidget(to_label_3, 0);
    zGlobalCoordLayout->addWidget(D1_sbox, 1);
    QHBoxLayout *tGlobalCoordLayout = new QHBoxLayout();
    tGlobalCoordLayout->setSpacing(5);
    tGlobalCoordLayout->setContentsMargins(0,0,0,0);
    tGlobalCoordLayout->addWidget(T0_sbox, 1);
    tGlobalCoordLayout->addWidget(to_label_4, 0);
    tGlobalCoordLayout->addWidget(T1_sbox, 1);
    QVBoxLayout *rightBlockLayout = new QVBoxLayout();
    rightBlockLayout->setContentsMargins(0,0,0,0);
    rightBlockLayout->addLayout(xGlobalCoordLayout, 0);
    rightBlockLayout->addLayout(yGlobalCoordLayout, 0);
    rightBlockLayout->addLayout(zGlobalCoordLayout, 0);
    rightBlockLayout->addLayout(tGlobalCoordLayout, 0);
    /* -------------- put elements into 4x4 grid ----------------- */
    global_coordinates_layout->addWidget(refSysContainer,   0, 0, 3, 1);
    global_coordinates_layout->addWidget(frameCoord,        3, 0, 1, 1);
    global_coordinates_layout->addWidget(xShiftWidget,      0, 1, 1, 1);
    global_coordinates_layout->addWidget(yShiftWidget,      1, 1, 1, 1);
    global_coordinates_layout->addWidget(zShiftWidget,      2, 1, 1, 1);
    global_coordinates_layout->addWidget(tShiftWidget,      3, 1, 1, 1);
    global_coordinates_layout->addLayout(xGlobalCoordLayout,0, 2, 1, 2);
    global_coordinates_layout->addLayout(yGlobalCoordLayout,1, 2, 1, 2);
    global_coordinates_layout->addLayout(zGlobalCoordLayout,2, 2, 1, 2);
    global_coordinates_layout->addLayout(tGlobalCoordLayout,3, 2, 1, 2);
    /* ------------- FINALIZATION -------------- */
    globalCoord_panel->setLayout(global_coordinates_layout);
    #ifndef _USE_NATIVE_FONTS
    globalCoord_panel->setStyle(new QWindowsStyle());
    #endif

    // "Sliding viewer" panel layout
    QHBoxLayout* esPanelLayout = new QHBoxLayout();
    ESbutton->setFixedWidth(marginLeft);
    esPanelLayout->addWidget(ESbutton, 0);
    esPanelLayout->addWidget(ESblockSpbox, 1);
    esPanelLayout->addWidget(ESmethodCbox, 1);
    esPanelLayout->addWidget(ESoverlapSpbox, 1);
    ESPanel->setLayout(esPanelLayout);
    #ifndef _USE_NATIVE_FONTS
    ESPanel->setStyle(new QWindowsStyle());
    #endif

    //local viewer panel
    QVBoxLayout* localviewer_panel_layout= new QVBoxLayout();
    /* --------------------- first row ---------------------- */
    QHBoxLayout *resolutionSelection_layout = new QHBoxLayout();
    resolutionSelection_layout->setContentsMargins(0,0,0,0);
    QLabel *resolutionLabel = new QLabel("Resolution:");
    //resolutionLabel->setStyleSheet("background-color:blue");
    resolutionLabel->setFixedWidth(marginLeft);
    resolutionSelection_layout->addWidget(resolutionLabel, 0);
    resolutionSelection_layout->addWidget(resolution_cbox, 1);
    /* -------------------- second row ---------------------- */
    QHBoxLayout *resolutionBar_layout = new QHBoxLayout();
    resolutionBar_layout->setContentsMargins(0,0,0,0);
    QLabel *emptyLabel = new QLabel("");
    emptyLabel->setFixedWidth(marginLeft);
    resolutionBar_layout->addWidget(emptyLabel, 0);
    resolutionBar_layout->addWidget(gradientBar, 1);
    /* --------------------- third row ---------------------- */
    QHBoxLayout *VOImaxsize_layout = new QHBoxLayout();
    VOImaxsize_layout->setContentsMargins(0,0,0,0);
    QLabel *viewMaxDimsLabel = new QLabel("Max dims:");
    viewMaxDimsLabel->setFixedWidth(marginLeft);
    VOImaxsize_layout->addWidget(viewMaxDimsLabel, 0, Qt::AlignLeft);
    QWidget* fourSpinboxes = new QWidget();
    QHBoxLayout *fourSpinboxes_layout = new QHBoxLayout();
    fourSpinboxes_layout->setContentsMargins(0,0,0,0);
    fourSpinboxes_layout->addWidget(Hdim_sbox, 1);
    fourSpinboxes_layout->addWidget(Vdim_sbox, 1);
    fourSpinboxes_layout->addWidget(Ddim_sbox, 1);
    fourSpinboxes_layout->addWidget(Tdim_sbox, 1);
    fourSpinboxes->setLayout(fourSpinboxes_layout);
    VOImaxsize_layout->addWidget(fourSpinboxes, 1, Qt::AlignLeft);
    /* ------------- FINALIZATION -------------- */
    localviewer_panel_layout->addLayout(resolutionSelection_layout, 0);
    localviewer_panel_layout->addLayout(resolutionBar_layout, 0);
    localviewer_panel_layout->addLayout(VOImaxsize_layout, 0);
    localViewer_panel->setLayout(localviewer_panel_layout);
    #ifndef _USE_NATIVE_FONTS
    localViewer_panel->setStyle(new QWindowsStyle());
    #endif

    //zoom options panel
    QVBoxLayout* zoomOptions_panel_layout= new QVBoxLayout();
    /* --------------------- first row ---------------------- */
    QHBoxLayout *zoomInMethod_layout = new QHBoxLayout();
    zoomInMethod_layout->setContentsMargins(0,0,0,0);
    QLabel *zoomInMethodLabel = new QLabel("Z/i method:");
    //zoomInMethodLabel->setStyleSheet("background-color:blue");
    zoomInMethodLabel->setFixedWidth(marginLeft);
    zoomInMethod_layout->addWidget(zoomInMethodLabel, 0);
    zoomInMethod_layout->addWidget(zoomInMethod, 1);
    /* -------------------- second row ---------------------- */
    QHBoxLayout *zoomControls_layout = new QHBoxLayout();
    zoomControls_layout->setContentsMargins(0,0,0,0);
    /* -------------------- second row: first column -------- */
    QVBoxLayout *zoomControls_col1_layout = new QVBoxLayout();
    zoomControls_col1_layout->setContentsMargins(0,0,0,0);
    QLabel *zoomInThresholdLabel = new QLabel("Z/i thres:");
    zoomInThresholdLabel->setFixedWidth(marginLeft);
    QLabel *cachingSensLabel = new QLabel("Z/i cache:");
    cachingSensLabel->setFixedWidth(marginLeft);
    QLabel* zoomOutThres = new QLabel("Z/o thres:");
    zoomOutThres->setFixedWidth(marginLeft);
    zoomControls_col1_layout->addWidget(zoomInThresholdLabel, 0);
    zoomControls_col1_layout->addWidget(cachingSensLabel, 0);
    zoomControls_col1_layout->addWidget(zoomOutThres, 0);
    /* -------------------- second row: second column ------- */
    QVBoxLayout *zoomControls_col2_layout = new QVBoxLayout();
    zoomControls_col2_layout->setContentsMargins(0,0,0,0);
    zoomControls_col2_layout->addWidget(zoomInSens, 0);
    zoomControls_col2_layout->addWidget(cacheSens, 0);
    zoomControls_col2_layout->addWidget(zoomOutSens, 0);
    /* -------------------- second row: third column -------- */
    /* -------------------- second row: fourth column ------- */
    /* -------------------- second row: FINALIZATION -------- */
    zoomControls_layout->addLayout(zoomControls_col1_layout, 0);
    zoomControls_layout->addLayout(zoomControls_col2_layout, 1);
    zoomControls_layout->addWidget(controlsResetButton, 0);
    /* -------------------- FINALIZATION -------------------- */
    zoomOptions_panel_layout->addLayout(zoomInMethod_layout, 0);
    zoomOptions_panel_layout->addLayout(zoomControls_layout, 0);
    zoom_panel->setLayout(zoomOptions_panel_layout);
    #ifndef _USE_NATIVE_FONTS
    zoom_panel->setStyle(new QWindowsStyle());
    #endif

    // Page "Controls" layout
    QVBoxLayout* controlsLayout = new QVBoxLayout(controls_page);
    controlsLayout->addWidget(localViewer_panel, 0);
    controlsLayout->addWidget(zoom_panel, 0);
    controlsLayout->addWidget(globalCoord_panel, 0);
    controlsLayout->addWidget(ESPanel, 0);
    controlsLayout->addStretch(1);
    controls_page->setLayout(controlsLayout);

    //pages
    tabs->addTab(controls_page, "Controls");
    tabs->addTab(info_page, "Volume's info");

    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    QHBoxLayout* centralLayout = new QHBoxLayout();
    QVBoxLayout* innerLayout = new QVBoxLayout();
    QVBoxLayout* bottomLayout = new QVBoxLayout();
    innerLayout->addWidget(tabs, 0);
    innerLayout->addStretch(1);
    innerLayout->addWidget(helpBox, 0, Qt::AlignBottom);
    innerLayout->setContentsMargins(10, 5, 10, 10);
    innerLayout->setSpacing(5);
    centralLayout->addWidget(toolBar, 0);
    centralLayout->addLayout(innerLayout, 1);
    bottomLayout->addWidget(statusBar);
    bottomLayout->addWidget(progressBar);
    bottomLayout->setContentsMargins(10,0,10,10);
    layout->addWidget(menuBar, 0);
    layout->addLayout(centralLayout, 1);
    layout->addLayout(bottomLayout, 0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    #ifdef USE_EXPERIMENTAL_FEATURES
    setWindowTitle(QString("TeraFly v").append(teramanager::version.c_str()).append("e"));
    #else
    setWindowTitle(QString("TeraFly v").append(teramanager::version.c_str()));
    #endif
    this->setFont(tinyFont);

    // signals and slots    
    /**/itm::debug(itm::LEV3, "Signals and slots", __itm__current__function__);
    connect(CImport::instance(), SIGNAL(sendOperationOutcome(itm::RuntimeException*, qint64)), this, SLOT(importDone(itm::RuntimeException*, qint64)), Qt::QueuedConnection);
    connect(volMapSizeSBox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(xShiftSBox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(yShiftSBox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(zShiftSBox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(tShiftSBox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(Vdim_sbox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(Hdim_sbox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(Ddim_sbox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(Tdim_sbox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(resolution_cbox, SIGNAL(currentIndexChanged(int)), this, SLOT(resolutionIndexChanged(int)));
    connect(traslXpos, SIGNAL(clicked()), this, SLOT(traslXposClicked()));
    connect(traslXneg, SIGNAL(clicked()), this, SLOT(traslXnegClicked()));
    connect(traslYpos, SIGNAL(clicked()), this, SLOT(traslYposClicked()));
    connect(traslYneg, SIGNAL(clicked()), this, SLOT(traslYnegClicked()));
    connect(traslZpos, SIGNAL(clicked()), this, SLOT(traslZposClicked()));
    connect(traslZneg, SIGNAL(clicked()), this, SLOT(traslZnegClicked()));
    connect(traslTpos, SIGNAL(clicked()), this, SLOT(traslTposClicked()));
    connect(traslTneg, SIGNAL(clicked()), this, SLOT(traslTnegClicked()));
    connect(controlsResetButton, SIGNAL(clicked()), this, SLOT(resetMultiresControls()));
    connect(ESbutton, SIGNAL(clicked()), this, SLOT(ESbuttonClicked()));
    connect(ESblockSpbox, SIGNAL(valueChanged(int)), this, SLOT(ESblockSpboxChanged(int)));
    connect(this, SIGNAL(sendProgressBarChanged(int, int, int, const char*)), this, SLOT(progressBarChanged(int, int, int, const char*)), Qt::QueuedConnection);

    //reset widgets
    reset();
    resetMultiresControls();

    //set always on top
    setWindowFlags(Qt::WindowStaysOnTopHint);
    setMaximumSize(this->minimumWidth(), this->minimumHeight());
    show();
    move(QApplication::desktop()->screen()->rect().center() - rect().center());

    setFixedWidth(500);

    /**/itm::debug(itm::LEV1, "object successfully constructed", __itm__current__function__);
}

//reset everything
void PMain::reset()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    //resetting menu options and widgets
    openVolumeAction->setEnabled(true);
    openVolumeToolButton->setEnabled(true);
    recentVolumesMenu->setEnabled(true);
    closeVolumeAction->setEnabled(false);
    importOptionsMenu->setEnabled(true);
    aboutAction->setEnabled(true);
    loadAnnotationsAction->setEnabled(false);
    saveAnnotationsAction->setEnabled(false);
    saveAnnotationsAsAction->setEnabled(false);
    clearAnnotationsAction->setEnabled(false);

    //reseting info panel widgets
    info_page->setEnabled(false);
    vol_size_files_label->setText("");
    vol_size_files_field->setText("n.a.");
    vol_size_voxel_label->setText("");
    vol_size_voxel_field->setText("n.a.");
    vol_size_bytes_label->setText("");
    vol_size_bytes_field->setText("n.a.");
    vol_height_mm_field->setText("n.a.");
    vol_width_mm_field->setText("n.a.");
    vol_depth_mm_field->setText("n.a.");
    vol_height_field->setText("n.a.");
    vol_width_field->setText("n.a.");
    vol_depth_field->setText("n.a.");
    nrows_field->setText("n.a.");
    ncols_field->setText("n.a.");
    stack_height_field->setText("n.a.");
    stack_width_field->setText("n.a.");
    stack_depth_field->setText("n.a.");
    vxl_V_field->setText("n.a.");
    vxl_H_field->setText("n.a.");
    vxl_D_field->setText("n.a.");
    org_V_field->setText("n.a.");
    org_H_field->setText("n.a.");
    org_D_field->setText("n.a.");

    //resetting multiresolution mode widgets
    gradientBar->setEnabled(false);
    resolution_cbox->setEnabled(false);
    zoom_panel->setEnabled(false);
    Vdim_sbox->setValue(CSettings::instance()->getVOIdimV());
    Hdim_sbox->setValue(CSettings::instance()->getVOIdimH());
    Ddim_sbox->setValue(CSettings::instance()->getVOIdimD());
    Tdim_sbox->setValue(CSettings::instance()->getVOIdimT());
    resolution_cbox->setEnabled(false);
    while(resolution_cbox->count())
        resolution_cbox->removeItem(0);
    traslXlabel->setAlignment(Qt::AlignCenter);
//    traslXlabel->setTextFormat(Qt::RichText);
    traslXlabel->setText("<font size=\"4\">X</font>");
    traslYlabel->setAlignment(Qt::AlignCenter);
//    traslYlabel->setTextFormat(Qt::RichText);
    traslYlabel->setText("<font size=\"4\">Y</font>");
    traslZlabel->setAlignment(Qt::AlignCenter);
//    traslZlabel->setTextFormat(Qt::RichText);
    traslZlabel->setText("<font size=\"4\">Z</font>");
    traslTlabel->setAlignment(Qt::AlignCenter);
//    traslTlabel->setTextFormat(Qt::RichText);
    traslTlabel->setText("<font size=\"4\">t</font>");
    traslXpos->setEnabled(false);
    traslXneg->setEnabled(false);
    traslYpos->setEnabled(false);
    traslYneg->setEnabled(false);
    traslZpos->setEnabled(false);
    traslZneg->setEnabled(false);
    traslTpos->setEnabled(false);
    traslTneg->setEnabled(false);
    gradientBar->setEnabled(false);
    gradientBar->setNSteps(-1);
    gradientBar->setStep(0);
    resetMultiresControls();

    //resetting subvol panel widgets
    globalCoord_panel->setEnabled(false);
    ESPanel->setEnabled(false);
    V0_sbox->setValue(0);
    V1_sbox->setValue(0);
    H0_sbox->setValue(0);
    H1_sbox->setValue(0);
    D0_sbox->setValue(0);
    D1_sbox->setValue(0);
    T0_sbox->setText("");
    T1_sbox->setText("");
//    T0_sbox->setValue(0);
//    T1_sbox->setValue(0);
    frameCoord->setText("");
    refSys->setXRotation(200);
    refSys->setYRotation(50);
    refSys->setZRotation(0);
    refSys->setDims(1,1,1);
    refSys->setFilled(true);
    refSys->resetZoom();
    frameCoord->setPalette(globalCoord_panel->palette());

    //reseting ES panel widgets
    //ESPanel->setEnabled(false);
    ESbutton->setIcon(QIcon(":/icons/start.png"));
    ESbutton->setText("Start");
    ESblockSpbox->setPrefix("Block ");
    ESblockSpbox->setSuffix("/0");
    ESblockSpbox->setMaximum(0);
    ESblockSpbox->setMinimum(0);
    ESblockSpbox->setValue(0);
    ESblockSpbox->setEnabled(false);

    //resetting progress bar and text
    progressBar->setEnabled(false);
    progressBar->setMaximum(1);         //needed to stop animation on some operating systems
    statusBar->clearMessage();
    statusBar->showMessage("Ready.");    
    helpBox->setText(HTwelcome);
}


//reset GUI method
void PMain::resetGUI()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    progressBar->setEnabled(false);
    progressBar->setMaximum(1);         //needed to stop animation on some operating systems
    statusBar->clearMessage();
    statusBar->showMessage("Ready.");
    this->setCursor(QCursor(Qt::ArrowCursor));
}


/**********************************************************************************
* Called when a path in the "Recent volumes" menu is selected.
***********************************************************************************/
void PMain::openVolumeActionTriggered()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    this->openVolume(qobject_cast<QAction*>(sender())->text().toStdString());
}

/**********************************************************************************
* Called when "Clear menu" action in "Recent volumes" menu is triggered.
***********************************************************************************/
void PMain::clearRecentVolumesTriggered()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    CSettings::instance()->clearVolumePathHistory();
    QList<QAction*> actions = recentVolumesMenu->actions();
    qDeleteAll(actions.begin(), actions.end());
    clearRecentVolumesAction = new QAction("Clear menu",recentVolumesMenu);
    connect(clearRecentVolumesAction, SIGNAL(triggered()), this, SLOT(clearRecentVolumesTriggered()));
    recentVolumesMenu->addSeparator();
    recentVolumesMenu->addAction(clearRecentVolumesAction);
}

/**********************************************************************************
* Called when "Open volume" menu action is triggered.
* If path is not provided, opens a QFileDialog to select volume's path.
***********************************************************************************/
void PMain::openVolume(string path /* = "" */)
{
    /**/itm::debug(itm::LEV1, strprintf("path = \"%s\"", path.c_str()).c_str(), __itm__current__function__);

    try
    {
        QString import_path = path.c_str();

        if(import_path.isEmpty())
        {
            /**/itm::debug(itm::LEV2, "import_path is empty, launching file dialog", __itm__current__function__);

            #ifdef _USE_QT_DIALOGS
            QFileDialog dialog(0);
            dialog.setFileMode(QFileDialog::Directory);
            dialog.setViewMode(QFileDialog::Detail);
            dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
            dialog.setWindowTitle("Select volume's directory");
            dialog.setDirectory(CSettings::instance()->getVolumePathLRU().c_str());
            if(dialog.exec())
                import_path = dialog.directory().absolutePath().toStdString().c_str();

            #else
            //added by PHC 20130823
            import_path = QFileDialog::getExistingDirectory(this, tr("Select a folder for a resolution of the volume image you want to visualize"),
                                                            CSettings::instance()->getVolumePathLRU().c_str(),
                                                             QFileDialog::ShowDirsOnly
                                                        //     | QFileDialog::DontResolveSymlinks   //maybe I should allow symbolic links as well, by PHC, 20130823
                                                                    );
            #endif

            /**/itm::debug(itm::LEV3, strprintf("import_path = %s", qPrintable(import_path)).c_str(), __itm__current__function__);

            if (import_path.isEmpty())
                return;
        }
        else
        {
            /**/itm::debug(itm::LEV2, strprintf("import_path is not empty (= \"%s\")", import_path.toStdString().c_str()).c_str(), __itm__current__function__);

            if(!QFile::exists(import_path))
                throw RuntimeException(strprintf("Path \"%s\" does not exist", import_path.toStdString().c_str()).c_str());
        }

        /* ---- temporary code ---- */
//        TimeSeries* ts = new TimeSeries(import_path.toStdString().c_str(), iim::RAW_FORMAT);
//        printf("FOUND %d x %d x %d x %d x %d\n", ts->getDIM_H(), ts->getDIM_V(), ts->getDIM_D(), ts->getDIM_C(), ts->getDIM_T());
//        return;
//        VirtualVolume* vol = VirtualVolume::instance(import_path.toStdString().c_str());
//        if(vol)
//            printf("VOLUME FOUND! Type = \"%s\"\n", typeid(*vol).name());
//        else
//            printf("VOLUME not found :-(\n");
//        return;
        /* ------------------------ */

        //then checking that no volume has imported yet
        if(!CImport::instance()->isEmpty())
            throw RuntimeException("A volume has been already imported! Please close the current volume first.");


        // check that folder name matches with the used convention
        QDir dir(import_path);
        if( dir.dirName().toStdString().substr(0,3).compare(itm::RESOLUTION_PREFIX) != 0)
            throw RuntimeException(strprintf("\"%s\" is not a valid resolution: the name of the folder does not start with \"%s\"",
                                             qPrintable(import_path), itm::RESOLUTION_PREFIX.c_str() ).c_str());


        //storing the path into CSettings
        CSettings::instance()->setVolumePathLRU(qPrintable(import_path));
        CSettings::instance()->addVolumePathToHistory(qPrintable(import_path));
        CSettings::instance()->writeSettings();

        //updating recent volumes menu
        QList<QAction*> actions = recentVolumesMenu->actions();
        qDeleteAll(actions.begin(), actions.end());
        std::list<string> recentVolumes = CSettings::instance()->getVolumePathHistory();
        for(std::list<string>::reverse_iterator it = recentVolumes.rbegin(); it != recentVolumes.rend(); it++)
        {
            QAction *action = new QAction(it->c_str(), this);
            connect(action, SIGNAL(triggered()), this, SLOT(openVolumeActionTriggered()));
            recentVolumesMenu->addAction(action);
        }
        clearRecentVolumesAction = new QAction("Clear menu",recentVolumesMenu);
        connect(clearRecentVolumesAction, SIGNAL(triggered()), this, SLOT(clearRecentVolumesTriggered()));
        recentVolumesMenu->addSeparator();
        recentVolumesMenu->addAction(clearRecentVolumesAction);

        //check if additional informations are required
        if(!VirtualVolume::isDirectlyImportable(qPrintable(import_path))  || regenMData_cAction->isChecked())
        {
           if(PDialogImport::instance(this)->exec() == QDialog::Rejected)
                return;
           CImport::instance()->setReimport(true);
           CImport::instance()->setRegenerateVolumeMap(true);
        }
        else
            CImport::instance()->setRegenerateVolumeMap(regenVMap_cAction->isChecked());
        CImport::instance()->setPath(qPrintable(import_path));

        //disabling import form and enabling progress bar animation
        progressBar->setEnabled(true);
        progressBar->setMinimum(0);
        progressBar->setMaximum(0);
        statusBar->showMessage("Importing volume...");

        //starting import
        CImport::instance()->updateMaxDims();
        CImport::instance()->start();
    }
    catch(iim::IOException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::getInstance()->resetGUI();
        CImport::instance()->reset();
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::getInstance()->resetGUI();
        CImport::instance()->reset();
    }
}

/**********************************************************************************
* Called when "Close volume" menu action is triggered.
* All the memory allocated is released and GUI is reset".
***********************************************************************************/
void PMain::closeVolume()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    CImport::instance()->reset();
    CVolume::instance()->reset();

    PDialogImport::uninstance();
    CExplorerWindow::uninstance();
    CAnnotations::uninstance();
    reset();
}

/**********************************************************************************
* Called when "Open annotations" menu action is triggered.
* Opens QFileDialog to select annotation file path.
***********************************************************************************/
void PMain::loadAnnotations()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    try
    {
        if(CExplorerWindow::getCurrent())
        {
            //obtaining current volume's parent folder path
            QDir dir(CImport::instance()->getPath().c_str());
            dir.cdUp();

            #ifdef _USE_QT_DIALOGS
            QString path = "";
            QFileDialog dialog(0);
            dialog.setFileMode(QFileDialog::ExistingFile);
            dialog.setViewMode(QFileDialog::Detail);
            dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
            dialog.setWindowTitle("Open annotation file");
            dialog.setNameFilter(tr("annotation files (*.ano)"));
            dialog.setDirectory(dir.absolutePath().toStdString().c_str());
            if(dialog.exec())
               if(!dialog.selectedFiles().empty())
                   path = dialog.selectedFiles().front();

            #else
            QString path = QFileDialog::getOpenFileName(this, "Open annotation file", dir.absolutePath(), tr("annotation files (*.ano)"));
            #endif

            if(!path.isEmpty())
            {
                annotationsPathLRU = path.toStdString();
                CAnnotations::getInstance()->load(annotationsPathLRU.c_str());
                CExplorerWindow::getCurrent()->loadAnnotations();
                saveAnnotationsAction->setEnabled(true);
            }
            else
                return;
        }
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

/**********************************************************************************
* Called when "Save annotations" or "Save annotations as" menu actions are triggered.
* If required, opens QFileDialog to select annotation file path.
***********************************************************************************/
void PMain::saveAnnotations()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    try
    {
        if(CExplorerWindow::getCurrent())
        {
            if(annotationsPathLRU.compare("")==0)
            {
                saveAnnotationsAs();
                return;
            }
            CExplorerWindow::getCurrent()->storeAnnotations();
            CAnnotations::getInstance()->save(annotationsPathLRU.c_str());
        }
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}
void PMain::saveAnnotationsAs()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    try
    {
        if(CExplorerWindow::getCurrent())
        {
            //obtaining current volume's parent folder path
            QDir dir(CImport::instance()->getPath().c_str());
            dir.cdUp();

            #ifdef _USE_QT_DIALOGS
            QString path = "";
            QFileDialog dialog(0);
            dialog.setFileMode(QFileDialog::AnyFile);
            dialog.setAcceptMode(QFileDialog::AcceptSave);
            dialog.setViewMode(QFileDialog::Detail);
            dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
            dialog.setWindowTitle("Save annotation file as");
            dialog.setNameFilter(tr("annotation files (*.ano)"));
            dialog.setDirectory(dir.absolutePath().toStdString().c_str());
            if(dialog.exec())
               if(!dialog.selectedFiles().empty())
                   path = dialog.selectedFiles().front();

            #else
            QString path = QFileDialog::getSaveFileName(this, "Save annotation file as", dir.absolutePath(), tr("annotation files (*.ano)"));
            #endif

            if(!path.isEmpty())
            {
                annotationsPathLRU = path.toStdString();
                if(annotationsPathLRU.find(".ano") == string::npos)
                    annotationsPathLRU.append(".ano");
                CExplorerWindow::getCurrent()->storeAnnotations();
                CAnnotations::getInstance()->save(annotationsPathLRU.c_str());
                saveAnnotationsAction->setEnabled(true);
            }
            else
                return;
        }
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

/**********************************************************************************
* Called when "Clear annotations" menu action is triggered.
***********************************************************************************/
void PMain::clearAnnotations()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    try
    {
        if(CExplorerWindow::getCurrent())
        {
            CAnnotations::getInstance()->clear();
            CExplorerWindow::getCurrent()->loadAnnotations();
        }
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

/**********************************************************************************
* Called when "Exit" menu action is triggered or TeraFly window is closed.
***********************************************************************************/
void PMain::exit()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    this->close();
}

/**********************************************************************************
* Called when "Help->About" menu action is triggered
***********************************************************************************/
void PMain::about()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    PAbout::instance(this)->exec();
}


/*********************************************************************************
* Called by <CImport> when the associated operation has been performed.
* If an exception has occurred in the <CImport> thread,  it is propagated and man-
* aged in the current thread (ex != 0). Otherwise, volume information are imported
* in the GUI by the <StackedVolume> handle of <CImport>.
**********************************************************************************/
void PMain::importDone(RuntimeException *ex, qint64 elapsed_time)
{
    /**/itm::debug(itm::LEV1, strprintf("ex = %s", (ex? "error" : "0")).c_str(), __itm__current__function__);

    //if an exception has occurred, showing a message error and re-enabling import form
    if(ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
    }
    else
    {
        //first updating IO time
        /**/itm::debug(itm::LEV_MAX, "updating IO time", __itm__current__function__);
        PLog::getInstance()->appendIO(elapsed_time, "Volume imported and map loaded");

        //otherwise inserting volume's informations
        /**/itm::debug(itm::LEV_MAX, "inserting volume's informations", __itm__current__function__);
        QElapsedTimer timerGUI;
        timerGUI.start();
        VirtualVolume* volume = CImport::instance()->getHighestResVolume();
        info_page->setEnabled(true);

        double GVoxels = (volume->getDIM_V()/1000.0f)*(volume->getDIM_H()/1000.0f)*(volume->getDIM_D()/1000.0f);
        double TVoxels = GVoxels/1000.0;
        if(TVoxels < 0.1)
        {
            double GBytes = GVoxels*CImport::instance()->getVMapCDim();
            vol_size_files_field->setText(QString("<b>").append(QString::number(GBytes, 'f', 1).append("</b>")));
            vol_size_bytes_field->setText(QString::number(GBytes, 'f', 1));
            vol_size_voxel_field->setText(QString::number(GVoxels, 'f', 1));
            vol_size_files_label->setText("GBytes");
            vol_size_bytes_label->setText("GBytes");
            vol_size_voxel_label->setText("GVoxels");
        }
        else
        {
            double TBytes = TVoxels*CImport::instance()->getVMapCDim();
            vol_size_files_field->setText(QString("<b>").append(QString::number(TBytes, 'f', 1).append("</b>")));
            vol_size_bytes_field->setText(QString::number(TBytes, 'f', 1));
            vol_size_voxel_field->setText(QString::number(TVoxels, 'f', 1));
            vol_size_files_label->setText("TBytes");
            vol_size_bytes_label->setText("TBytes");
            vol_size_voxel_label->setText("TVoxels");
        }

        vol_height_mm_field->setText(QString::number(fabs(volume->getDIM_V()*volume->getVXL_V()/1000.0f), 'f', 2));
        vol_width_mm_field->setText(QString::number(fabs(volume->getDIM_H()*volume->getVXL_H()/1000.0f), 'f', 2));
        vol_depth_mm_field->setText(QString::number(fabs(volume->getDIM_D()*volume->getVXL_D()/1000.0f), 'f', 2));
        vol_height_field->setText(QString::number(volume->getDIM_V()));
        vol_width_field->setText(QString::number(volume->getDIM_H()));
        vol_depth_field->setText(QString::number(volume->getDIM_D()));
        if(dynamic_cast<StackedVolume*>(volume))
        {
            nrows_field->setText(QString::number(dynamic_cast<StackedVolume*>(volume)->getN_ROWS()));
            ncols_field->setText(QString::number(dynamic_cast<StackedVolume*>(volume)->getN_COLS()));
            stack_height_field->setText(QString::number(dynamic_cast<StackedVolume*>(volume)->getStacksHeight()));
            stack_width_field->setText(QString::number(dynamic_cast<StackedVolume*>(volume)->getStacksWidth()));
        }
        else if(dynamic_cast<TiledVolume*>(volume))
        {
            nrows_field->setText(QString::number(dynamic_cast<TiledVolume*>(volume)->getN_ROWS()));
            ncols_field->setText(QString::number(dynamic_cast<TiledVolume*>(volume)->getN_COLS()));
            stack_height_field->setText(QString::number(dynamic_cast<TiledVolume*>(volume)->getStacksHeight()));
            stack_width_field->setText(QString::number(dynamic_cast<TiledVolume*>(volume)->getStacksWidth()));
        }
        stack_depth_field->setText(QString::number(volume->getDIM_D()));
        vxl_V_field->setText(QString::number(volume->getVXL_V(), 'f', 2));
        vxl_H_field->setText(QString::number(volume->getVXL_H(), 'f', 2));
        vxl_D_field->setText(QString::number(volume->getVXL_D(), 'f', 2));
        org_V_field->setText(QString::number(volume->getORG_V(), 'f', 2));
        org_H_field->setText(QString::number(volume->getORG_H(), 'f', 2));
        org_D_field->setText(QString::number(volume->getORG_D(), 'f', 2));

        //and setting subvol widgets limits
        /**/itm::debug(itm::LEV_MAX, "setting subvol widgets limits", __itm__current__function__);
        V0_sbox->setMinimum(1);
        V0_sbox->setMaximum(volume->getDIM_V());
        V0_sbox->setValue(1);
        V1_sbox->setMinimum(1);
        V1_sbox->setMaximum(volume->getDIM_V());
        V1_sbox->setValue(volume->getDIM_V());
        H0_sbox->setMinimum(1);
        H0_sbox->setMaximum(volume->getDIM_H());
        H0_sbox->setValue(1);
        H1_sbox->setMinimum(1);
        H1_sbox->setMaximum(volume->getDIM_H());
        H1_sbox->setValue(volume->getDIM_H());
        D0_sbox->setMinimum(1);
        D0_sbox->setMaximum(volume->getDIM_D());
        D0_sbox->setValue(1);
        D1_sbox->setMinimum(1);
        D1_sbox->setMaximum(volume->getDIM_D());
        D1_sbox->setValue(volume->getDIM_D());        
//        T0_sbox->setMinimum(0);
//        T0_sbox->setMaximum(CImport::instance()->getVMapTDim()-1);
        if(volume->getDIM_T() > 1)
        {
            T0_sbox->setText("0");
            T1_sbox->setText(QString::number(CImport::instance()->getVMapTDim()-1));
            frameCoord->setText(strprintf("t = %d/%d", 0, volume->getDIM_T()-1).c_str());
        }
//        T1_sbox->setMinimum(0);
//        T1_sbox->setMaximum(CImport::instance()->getVMapTDim()-1);
        globalCoord_panel->setEnabled(true);
        ESPanel->setEnabled(true);
        ESmethodCbox->setEnabled(true);
        ESoverlapSpbox->setEnabled(true);

        //updating menu items
        /**/itm::debug(itm::LEV_MAX, "updating menu items", __itm__current__function__);
        openVolumeAction->setEnabled(false);
        recentVolumesMenu->setEnabled(false);
        openVolumeToolButton->setEnabled(false);
        importOptionsMenu->setEnabled(false);
        closeVolumeAction->setEnabled(true);
        clearAnnotationsAction->setEnabled(true);

        //enabling multiresolution panel and hiding volume map options        
        gradientBar->setEnabled(true);
        resolution_cbox->setEnabled(true);
        this->zoom_panel->setEnabled(true);

        //enabling menu actions
        loadAnnotationsAction->setEnabled(true);
        saveAnnotationsAsAction->setEnabled(true);

        //updating gradient bar widget
        gradientBar->setEnabled(true);
        gradientBar->setNSteps(CImport::instance()->getResolutions());

        //inserting available resolutions
        /**/itm::debug(itm::LEV_MAX, "inserting available resolutions", __itm__current__function__);
        resolution_cbox->setEnabled(false);
        for(int i=0; i<CImport::instance()->getResolutions(); i++)
        {
            QString option = "";
            VirtualVolume* vol = CImport::instance()->getVolume(i);
            float vxl_v = vol->getVXL_V() < 0 ? vol->getVXL_V()*-1 : vol->getVXL_V();
            float vxl_h = vol->getVXL_H() < 0 ? vol->getVXL_H()*-1 : vol->getVXL_H();
            float vxl_d = vol->getVXL_D() < 0 ? vol->getVXL_D()*-1 : vol->getVXL_D();
            option = option + QString::number(vol->getDIM_H()) + QChar(0x00D7) + QString::number(vol->getDIM_V()) + QChar(0x00D7) + QString::number(vol->getDIM_D()) +
                    " (voxel: " + QString::number(vxl_h, 'f', 1) + QChar(0x00D7) + QString::number(vxl_v, 'f', 1) + QChar(0x00D7) +
                    QString::number(vxl_d, 'f', 1) + " " + QChar(0x03BC)+"m)";
            resolution_cbox->insertItem(i, option);
        }
        resolution_cbox->setEnabled(true);

        //updating traslation widgets
        traslXlabel->setText("<font size=\"4\" color=\"red\"><b>X</b></font>");
        traslYlabel->setText("<font size=\"4\" color=\"green\"><b>Y</b></font>");
        traslZlabel->setText("<font size=\"4\" color=\"blue\"><b>Z</b></font>");
        traslTlabel->setText("<font size=\"4\" color=\"gray\"><b>t</b></font>");

        //disabling useless time-related widgets if data is < 5D
        to_label_4->setEnabled(volume->getDIM_T() > 1);
        frameCoord->setEnabled(volume->getDIM_T() > 1);
        traslTpos->setEnabled(volume->getDIM_T() > 1);
        traslTlabel->setEnabled(volume->getDIM_T() > 1);
        traslTneg->setEnabled(volume->getDIM_T() > 1);
        T0_sbox->setEnabled(volume->getDIM_T() > 1);
        T1_sbox->setEnabled(volume->getDIM_T() > 1);


        //instantiating CAnnotations
        /**/itm::debug(itm::LEV_MAX, "instantiating CAnnotations", __itm__current__function__);
        CAnnotations::instance(volume->getDIM_V(), volume->getDIM_H(), volume->getDIM_D());

        //updating GUI time
        /**/itm::debug(itm::LEV_MAX, "updating GUI time", __itm__current__function__);
        PLog::getInstance()->appendGPU(timerGUI.elapsed(), "TeraFly's GUI initialized");

        //starting 3D exploration
        /**/itm::debug(itm::LEV_MAX, "instantiating CExplorerWindow", __itm__current__function__);
        CExplorerWindow *new_win = new CExplorerWindow(V3D_env, CImport::instance()->getVMapResIndex(), CImport::instance()->getVMapRawData(),
                            0, CImport::instance()->getVMapYDim(), 0, CImport::instance()->getVMapXDim(),
                            0, CImport::instance()->getVMapZDim(), 0, CImport::instance()->getVMapTDim()-1, CImport::instance()->getVMapCDim(), 0);
        /**/itm::debug(itm::LEV_MAX, "showing CExplorerWindow", __itm__current__function__);
        new_win->show();
        new_win->isReady = true;

        helpBox->setText(HTbase);

        //finally storing in application settings the path of the opened volume
        CSettings::instance()->setVolumePathLRU(CImport::instance()->getPath());

        //updating actual time
        PLog::getInstance()->appendActual(CImport::instance()->timerIO.elapsed(), "TeraFly 3D exploration started");
    }

    //resetting some widgets
    resetGUI();

}

//overrides closeEvent method of QWidget
void PMain::closeEvent(QCloseEvent *evt)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    if(evt)
    {
        if(progressBar->isEnabled() && QMessageBox::information(this, "Warning", "An operation is still in progress. Terminating it can be unsafe and cause Vaa3D to crash. \n"
                                                                        "\nPlease save your data first.", "Close TeraFly plugin", "Cancel"))
        {
            evt->ignore();
        }
        else
        {
            evt->accept();
            PMain::uninstance();
        }
    }
}


/**********************************************************************************
* Called when the GUI widgets that control application settings change.
* This is used to manage persistent platform-independent application settings.
***********************************************************************************/
void PMain::settingsChanged(int)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    CSettings::instance()->setVolMapSizeLimit(volMapSizeSBox->value());
    CSettings::instance()->setVOIdimV(Vdim_sbox->value());
    CSettings::instance()->setVOIdimH(Hdim_sbox->value());
    CSettings::instance()->setVOIdimD(Ddim_sbox->value());
    CSettings::instance()->setVOIdimT(Tdim_sbox->value());
    CSettings::instance()->setTraslX(xShiftSBox->value());
    CSettings::instance()->setTraslY(yShiftSBox->value());
    CSettings::instance()->setTraslZ(zShiftSBox->value());
    CSettings::instance()->setTraslT(tShiftSBox->value());
    CSettings::instance()->writeSettings();
}

/**********************************************************************************
* Linked to resolution combobox
* This switches to the given resolution index.
***********************************************************************************/
void PMain::resolutionIndexChanged(int i)
{
    /**/itm::debug(itm::LEV1, strprintf("resolution = %d", i).c_str(), __itm__current__function__);

    try
    {
        if(resolution_cbox->isEnabled() && CExplorerWindow::getCurrent() && i > CExplorerWindow::getCurrent()->getResIndex())
        {
            int voiV0 = CVolume::scaleVCoord(V0_sbox->value()-1, CImport::instance()->getResolutions()-1, i);
            int voiV1 = CVolume::scaleVCoord(V1_sbox->value()-1, CImport::instance()->getResolutions()-1, i);
            int voiH0 = CVolume::scaleHCoord(H0_sbox->value()-1, CImport::instance()->getResolutions()-1, i);
            int voiH1 = CVolume::scaleHCoord(H1_sbox->value()-1, CImport::instance()->getResolutions()-1, i);
            int voiD0 = CVolume::scaleDCoord(D0_sbox->value()-1, CImport::instance()->getResolutions()-1, i);
            int voiD1 = CVolume::scaleDCoord(D1_sbox->value()-1, CImport::instance()->getResolutions()-1, i);

            int voiTDim = std::min(static_cast<int>(CImport::instance()->getVMapTDim()), Tdim_sbox->value());
            float MVoxels = ((voiV1-voiV0+1)/1024.0f)*((voiH1-voiH0+1)/1024.0f)*(voiD1-voiD0+1)*voiTDim;
            if(QMessageBox::Yes == QMessageBox::question(this, "Confirm", QString("The volume to be loaded is ").append(QString::number(MVoxels, 'f', 1)).append(" MVoxels big.\n\nDo you confirm?"), QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes))
            {
                int currentRes = CExplorerWindow::getCurrent()->getResIndex();
                int x0 = CVolume::scaleHCoord(H0_sbox->value()-1, CImport::instance()->getResolutions()-1, currentRes);
                int x1 = CVolume::scaleHCoord(H1_sbox->value()-1, CImport::instance()->getResolutions()-1, currentRes);
                int y0 = CVolume::scaleVCoord(V0_sbox->value()-1, CImport::instance()->getResolutions()-1, currentRes);
                int y1 = CVolume::scaleVCoord(V1_sbox->value()-1, CImport::instance()->getResolutions()-1, currentRes);
                int z0 = CVolume::scaleDCoord(D0_sbox->value()-1, CImport::instance()->getResolutions()-1, currentRes);
                int z1 = CVolume::scaleDCoord(D1_sbox->value()-1, CImport::instance()->getResolutions()-1, currentRes);
                int t0 = CExplorerWindow::getCurrent()->volT0;
                int t1 = CExplorerWindow::getCurrent()->volT1;
                /**/itm::debug(itm::LEV_MAX, strprintf("global VOI [%d,%d) [%d,%d) [%d,%d) rescaled to [%d,%d) [%d,%d) [%d,%d) at currentRes = %d",
                                                       H0_sbox->value()-1, H1_sbox->value()-1,
                                                       V0_sbox->value()-1, V1_sbox->value()-1,
                                                       D0_sbox->value()-1, D1_sbox->value()-1,
                                                       x0, x1, y0, y1, z0, z1, currentRes).c_str(), __itm__current__function__);
                CExplorerWindow::getCurrent()->newView(x1, y1, z1, i, t0, t1, false, -1, -1, -1, x0, y0, z0, false);
            }
            else
                resolution_cbox->setCurrentIndex(CExplorerWindow::getCurrent()->getResIndex());
        }
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        resetGUI();
        globalCoord_panel->setEnabled(true);
        resolution_cbox->setCurrentIndex(CExplorerWindow::getCurrent()->getResIndex());
    }
}

/**********************************************************************************
* Enables / Disables directional shift controls
***********************************************************************************/
void PMain::setEnabledDirectionalShifts(bool enabled)
{
    traslXneg->setEnabled(enabled);
    traslXpos->setEnabled(enabled);
    traslYneg->setEnabled(enabled);
    traslYpos->setEnabled(enabled);
    traslZneg->setEnabled(enabled);
    traslZpos->setEnabled(enabled);
    traslTneg->setEnabled(enabled);
    traslTpos->setEnabled(enabled);
}

/**********************************************************************************
* Called when the correspondent buttons are clicked
***********************************************************************************/
void PMain::traslXposClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        expl->newView((expl->volH1-expl->volH0)/2 + (expl->volH1-expl->volH0)*(100-CSettings::instance()->getTraslX())/100.0f,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2, expl->volResIndex, expl->volT0, expl->volT1, false);
    }
}
void PMain::traslXnegClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        expl->newView((expl->volH1-expl->volH0)/2 - (expl->volH1-expl->volH0)*(100-CSettings::instance()->getTraslX())/100.0f,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2, expl->volResIndex, expl->volT0, expl->volT1, false);
    }
}
void PMain::traslYposClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        expl->newView((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2 + (expl->volV1-expl->volV0)*(100-CSettings::instance()->getTraslY())/100.0f,
                      (expl->volD1-expl->volD0)/2, expl->volResIndex, expl->volT0, expl->volT1, false);
    }
}
void PMain::traslYnegClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        expl->newView((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2 - (expl->volV1-expl->volV0)*(100-CSettings::instance()->getTraslY())/100.0f,
                      (expl->volD1-expl->volD0)/2, expl->volResIndex, expl->volT0, expl->volT1, false);
    }
}
void PMain::traslZposClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        expl->newView((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2 + (expl->volD1-expl->volD0)*(100-CSettings::instance()->getTraslZ())/100.0f, expl->volResIndex, expl->volT0, expl->volT1, false);
    }
}
void PMain::traslZnegClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        expl->newView((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2 - (expl->volD1-expl->volD0)*(100-CSettings::instance()->getTraslZ())/100.0f, expl->volResIndex, expl->volT0, expl->volT1, false);
    }
}
void PMain::traslTposClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        int newT0 = expl->volT0 + (expl->volT1-expl->volT0)*(100-CSettings::instance()->getTraslT())/100.0f;
        int newT1 = newT0 + (Tdim_sbox->value()-1);
        if(newT1 >= CImport::instance()->getTDim())
        {
            newT1 = CImport::instance()->getTDim() - 1;
            newT0 = newT1 - (Tdim_sbox->value()-1);
        }
        expl->newView((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2,
                      expl->volResIndex,
                      newT0,
                      newT1, false);
//                      expl->volT0 + (expl->volT1-expl->volT0)*(100-CSettings::instance()->getTraslT())/100.0f,
//                      expl->volT1 + (expl->volT1-expl->volT0)*(100-CSettings::instance()->getTraslT())/100.0f, false);
    }
}
void PMain::traslTnegClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        int newT1 = expl->volT1 - (expl->volT1-expl->volT0)*(100-CSettings::instance()->getTraslT())/100.0f;
        int newT0 = newT1 - (Tdim_sbox->value()-1);
        if(newT0 < 0)
        {
            newT0 = 0;
            newT1 = newT0 + (Tdim_sbox->value()-1);
        }
        expl->newView((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2,
                      expl->volResIndex,
                      newT0,
                      newT1, false);
//                      expl->volT0 - (expl->volT1-expl->volT0)*(100-CSettings::instance()->getTraslT())/100.0f,
//                      expl->volT1 - (expl->volT1-expl->volT0)*(100-CSettings::instance()->getTraslT())/100.0f, false);
    }
}

/**********************************************************************************
* Filters events generated by the widgets to which a help message must be associated
***********************************************************************************/
bool PMain::eventFilter(QObject *object, QEvent *event)
{
    if ((object == Vdim_sbox || object == Hdim_sbox || object == Ddim_sbox || object == Tdim_sbox) && localViewer_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTvoiDim);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if((object == resolution_cbox) && localViewer_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTjumpToRes);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if((object == gradientBar) && localViewer_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTresolution);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if((object == zoomOutSens) && zoom_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTzoomOutThres);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);

        displayToolTip(zoomOutSens, event, QString::number(zoomOutSens->value()).toStdString());
    }
    else if((object == zoomInSens) && zoom_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTzoomInThres);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);

        displayToolTip(zoomInSens, event, QString::number(zoomInSens->value()).toStdString());
    }
    else if((object == zoomInMethod) && zoom_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTzoomInMethod);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if((object == cacheSens) && zoom_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTcacheSens);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);

        displayToolTip(cacheSens, event, QString::number(cacheSens->value()).append("%").toStdString());

    }
    else if ((object == traslXpos || object == traslYpos || object == traslZpos || object == traslTpos) && globalCoord_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTtraslatePos);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if ((object == traslXneg || object == traslYneg || object == traslZneg || object == traslTneg) && globalCoord_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTtraslateNeg);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if ((object == V0_sbox || object == V1_sbox ||
              object == H0_sbox || object == H1_sbox ||
              object == D0_sbox || object == D1_sbox ||
              object == T0_sbox || object == T1_sbox ) && globalCoord_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTvolcuts);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if((object == refSys) && globalCoord_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTrefsys);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    return false;
}

/**********************************************************************************
* Displays tooltip when ToolTip, MouseMove or KeyPress events occur on the widget.
***********************************************************************************/
void PMain::displayToolTip(QWidget* widget, QEvent* event, string msg)
{
    widget->setAttribute(Qt::WA_Hover); // this control the QEvent::ToolTip and QEvent::HoverMove
    widget->setFocusPolicy(Qt::WheelFocus); // accept KeyPressEvent when mouse wheel move

    bool event_tip = false;
    QPoint pos(0,0);
    switch (event->type())
    {
        case QEvent::ToolTip: // must turned on by setAttribute(Qt::WA_Hover) under Mac 64bit
                pos = ((QHelpEvent*)event)->pos();
                event_tip = true;
                break;
        case QEvent::MouseMove: // for mouse dragging
                pos = ((QMouseEvent*)event)->pos();
                event_tip = true;
                break;
        case 6: //QEvent::KeyPress: // for arrow key dragging
                pos = widget->mapFromGlobal(widget->cursor().pos());
                event_tip = true;
                break;
    }
    if (event_tip)
    {
        QPoint gpos = widget->mapToGlobal(pos);
        QToolTip::showText(gpos, msg.c_str(), widget);
    }
}

/**********************************************************************************
* Called when controlsResetButton is clicked
***********************************************************************************/
void PMain::resetMultiresControls()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    cacheSens->setValue(70);
    zoomInSens->setValue(40);
    zoomOutSens->setValue(0);
}

//very useful (not included in Qt): disables the given item of the given combobox
void PMain::setEnabledComboBoxItem(QComboBox* cbox, int _index, bool enabled)
{
    // Get the index of the value to disable
    QModelIndex index = cbox->model()->index(_index,0);

    // These are the effective 'disable/enable' flags
    QVariant v1(Qt::NoItemFlags);
    QVariant v2(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    //the magic
    cbox->model()->setData( index, enabled ? v2 : v1, Qt::UserRole -1);
}


/**********************************************************************************
* Called when the correspondent debug actions are triggered
***********************************************************************************/
void PMain::addGaussianNoiseTriggered()
{
    iim::ADD_NOISE_TO_TIME_SERIES = addGaussianNoiseToTimeSeries->isChecked();
}

void PMain::debugAction1Triggered()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    PAnoToolBar::instance(this)->show();

    CExplorerWindow* cur_win = CExplorerWindow::getCurrent();

    QPixmap cur_img(":/icons/cursor_marker_add.png");
    cur_img = cur_img.scaled(32,32,Qt::KeepAspectRatio, Qt::SmoothTransformation);
    setCursor(QCursor(cur_img));

    // invoke HighRez ROI zoom-in
    /*cur_win->view3DWidget->getRenderer()->selectMode = Renderer::smCurveCreate1;
    static_cast<Renderer_gl1*>(cur_win->view3DWidget->getRenderer())->b_addthiscurve = false;
    static_cast<Renderer_gl1*>(cur_win->view3DWidget->getRenderer())->b_imaging = false;
    static_cast<Renderer_gl1*>(cur_win->view3DWidget->getRenderer())->b_grabhighrez = true;
    cur_win->window3D->setCursor(QCursor(Qt::PointingHandCursor));*/

    // marker create
//    cur_win->view3DWidget->getRenderer()->selectMode = Renderer::smMarkerCreate1;
//    static_cast<Renderer_gl1*>(cur_win->view3DWidget->getRenderer())->b_addthismarker = true;
//    cur_win->window3D->setCursor(QCursor(Qt::PointingHandCursor));

    //cur_win->view3DWidget->getRenderer()->endSelectMode();


//    #ifdef USE_EXPERIMENTAL_FEATURES
//    CExplorerWindow* cur_win = CExplorerWindow::getCurrent();
//    if(cur_win)
//    {
//        int dimx = cur_win->volH1 - cur_win->volH0;
//        int dimy = cur_win->volV1 - cur_win->volV0;
//        int dimz = cur_win->volD1 - cur_win->volD0;
//        int cx = (dimx)/2;
//        int cy = (dimy)/2;
//        int cz = (dimz)/2;
//        int r = 50;
//        int side = 2*r;
//        float vol = pow(2.0f*r, 3);
//        int count = 0;

//        static bool first_time = true;
//        static bool restore_data = false;
//        static uint8* data_saved = new uint8[static_cast<int>(vol)];
//        uint8* data = cur_win->view3DWidget->getiDrawExternalParameter()->image4d->getRawData();
//        if(first_time)
//        {
//            first_time = false;
//            for(int k = cz - r; k < cz + r; k++)
//                for(int i = cy - r; i < cy + r; i++)
//                    for(int j = cx - r; j < cx + r; j++, count++)
//                        data_saved[(k-cz+r)*side*side +(i-cy+r)*side + (j-cx+r)] = data[k*dimy*dimx + i*dimx + j];
//        }

//        if(restore_data)
//        {
//            for(int k = cz - r; k < cz + r; k++)
//                for(int i = cy - r; i < cy + r; i++)
//                    for(int j = cx - r; j < cx + r; j++, count++)
//                        data[k*dimy*dimx + i*dimx + j] = data_saved[(k-cz+r)*side*side +(i-cy+r)*side + (j-cx+r)];
//        }
//        else
//        {
//            for(int k = cz - r; k < cz + r; k++)
//                for(int i = cy - r; i < cy + r; i++)
//                    for(int j = cx - r; j < cx + r; j++, count++)
//                        data[k*dimy*dimx + i*dimx + j] = (count / vol) *255;
//        }
//        restore_data = !restore_data;

//        myV3dR_GLWidget::cast(cur_win->view3DWidget)->updateImageDataFast();
//    }
//    #endif

//    CExplorerWindow* cur_win = CExplorerWindow::getCurrent();
//    if(cur_win)
//    {
//        printf("Retrieve Image4DSimple\n");
//        Image4DSimple *image4D = V3D_env->getImage(cur_win->window);
//        if(!image4D)
//            printf("ERROR!\n");

//        // generate time series from the currently displayed image
//        V3DLONG xDim = cur_win->volH1 - cur_win->volH0;
//        V3DLONG yDim = cur_win->volV1 - cur_win->volV0;
//        V3DLONG zDim = cur_win->volD1 - cur_win->volD0;
//        V3DLONG cDim = cur_win->nchannels;
//        V3DLONG tDim = QInputDialog::getInt(this, "Generation of 5D data", "Number of time frames", 10, 2, 1000);
//        V3DLONG size = xDim*yDim*zDim*cDim*tDim;

//        printf("Allocate memory \n");
//        uint8 *data5D = new uint8[size];
//        uint8 *data4D = image4D->getRawData();
//        printf("Generate 5D series\n");
//        for(int t=0; t<tDim; t++)
//        {
//            float w = static_cast<float>(t)/(tDim-1);
//            V3DLONG st = t*xDim*yDim*zDim*cDim;
//            for(int c=0; c<cDim; c++)
//            {
//                V3DLONG sc = c*xDim*yDim*zDim;
//                for(int z=0; z<zDim; z++)
//                {
//                    V3DLONG sz = z*xDim*yDim;
//                    for(int y=0; y<yDim; y++)
//                    {
//                        V3DLONG sy = y*xDim;
//                        for(int x=0; x<xDim; x++)
//                        {
//                            /*if(t != 0 &&
//                               (z % (tDim-t) == 0 ||
//                               y % (tDim-t) == 0 ||
//                               x % (tDim-t) == 0))
//                                data5D[st+sc+sz+sy+x] = 0;
//                            else
//                                data5D[st+sc+sz+sy+x] = data4D[sc+sz+sy+x];*/
//                            //data5D[st+sc+sz+sy+x] = data4D[sc+sz+sy+x] + rand()%50;
//                            data5D[st+sc+sz+sy+x] = static_cast<uint8>((1-w)*data4D[sc+sz+sy+x] + w*(rand()%256) +0.5f);
//                        }
//                    }
//                }
//            }
//        }

//        printf("Create new Image4DSimple, dimensions are x(%d), y(%d), z(%d), c(%d), t(%d)\n", xDim, yDim, zDim, cDim, tDim);
//        Image4DSimple* image5D = new Image4DSimple();
//        image5D->setFileName("5D Image");
//        image5D->setData(data5D, xDim, yDim, zDim, cDim*tDim, V3D_UINT8);
//        image5D->setTDim(tDim);
//        image5D->setTimePackType(TIME_PACK_C);

//        printf("Display 5D Image in a new window\n");
//        v3dhandle window5D= V3D_env->newImageWindow("5D Image");
//        XFormWidget* treeview = (XFormWidget*)window5D;
//        treeview->setWindowState(Qt::WindowMinimized);
//        V3D_env->setImage(window5D, image5D);
//        V3D_env->open3DWindow(window5D);

//        image4D->setData(data5D, xDim, yDim, zDim, cDim*tDim, V3D_UINT8);
//        image4D->setTDim(tDim);
//        image4D->setTimePackType(TIME_PACK_C);
//        V3D_env->pushImageIn3DWindow(cur_win->window);
//        //V3D_env->pushTimepointIn3DWindow(cur_win->window, 9);
//        cur_win->view3DWidget->updateGL();
//        //cur_win->view3DWidget->updateImageData();
//    }

}

void PMain::showLogTriggered()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    PLog::instance(this)->show();
}

/**********************************************************************************
* Called when the correspondent Options->3D->Curve actions are triggered
***********************************************************************************/
void PMain::curveDimsChanged(int dim)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    if(CExplorerWindow::getCurrent())
    {
        CExplorerWindow::getCurrent()->view3DWidget->getRenderer()->lineWidth = dim;
        CExplorerWindow::getCurrent()->view3DWidget->updateTool();
        CExplorerWindow::getCurrent()->view3DWidget->update();
    }
}

void PMain::curveAspectChanged()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    if(CExplorerWindow::getCurrent())
    {
        if(curveAspectTube->isChecked())
        {
            CExplorerWindow::getCurrent()->view3DWidget->getRenderer()->lineType = 0;
            CExplorerWindow::getCurrent()->view3DWidget->updateTool();
            CExplorerWindow::getCurrent()->view3DWidget->update();
        }
        else if(curveAspectSkeleton->isChecked())
        {
            CExplorerWindow::getCurrent()->view3DWidget->getRenderer()->lineType = 1;
            CExplorerWindow::getCurrent()->view3DWidget->updateTool();
            CExplorerWindow::getCurrent()->view3DWidget->update();
        }
    }
}

//generate blocks for sliding viewer
void PMain::generateESblocks() throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

    CExplorerWindow* curWin = CExplorerWindow::getCurrent();
    if(curWin)
    {
        int dimX   = CImport::instance()->getVolume(curWin->volResIndex)->getDIM_H();
        int dimY   = CImport::instance()->getVolume(curWin->volResIndex)->getDIM_V();
        int dimZ   = CImport::instance()->getVolume(curWin->volResIndex)->getDIM_D();
        int blockX = Hdim_sbox->value();
        int blockY = Vdim_sbox->value();
        int blockZ = Ddim_sbox->value();
        int ovlX  = ( ESoverlapSpbox->value()/100.0f )*Hdim_sbox->value();
        int ovlY  = ( ESoverlapSpbox->value()/100.0f )*Vdim_sbox->value();
        int ovlZ  = ( ESoverlapSpbox->value()/100.0f )*Ddim_sbox->value();
        int tolerance = 30;

        // generate X intervals
        vector<itm::interval_t> xInts;
        int count = 0;
        while(xInts.empty() || xInts.back().end < dimX)
        {
            xInts.push_back(interval_t(count, count+blockX));
            count += blockX-ovlX;
        }
        xInts.back().end = dimX;
        xInts.back().start = std::max(0,dimX-blockX);
        if(xInts.size() > 1 &&                                                  // more than one segment
           xInts[xInts.size()-1].end - xInts[xInts.size()-2].end < tolerance)   // last two segments difference is below tolerance
        {
            xInts.pop_back();           // remove last segment
            xInts.back().end = dimX;    // extend last segment up to the end
        }

        // generate Y intervals
        vector<itm::interval_t> yInts;
        count = 0;
        while(yInts.empty() || yInts.back().end < dimY)
        {
            yInts.push_back(interval_t(count, count+blockY));
            count += blockY-ovlY;
        }
        yInts.back().end = dimY;
        yInts.back().start = std::max(0,dimY-blockY);
        if(yInts.size() > 1 &&                                                  // more than one segment
           yInts[yInts.size()-1].end - yInts[yInts.size()-2].end < tolerance)   // last two segments difference is below tolerance
        {
            yInts.pop_back();           // remove last segment
            yInts.back().end = dimY;    // extend last segment up to the end
        }

        // generate Z intervals
        vector<itm::interval_t> zInts;
        count = 0;
        while(zInts.empty() || zInts.back().end < dimZ)
        {
            zInts.push_back(interval_t(count, count+blockZ));
            count += blockZ-ovlZ;
        }
        zInts.back().end = dimZ;
        zInts.back().start = std::max(0,dimZ-blockZ);
        if(zInts.size() > 1 &&                                                  // more than one segment
           zInts[zInts.size()-1].end - zInts[zInts.size()-2].end < tolerance)   // last two segments difference is below tolerance
        {
            zInts.pop_back();           // remove last segment
            zInts.back().end = dimZ;    // extend last segment up to the end
        }

        // generate 3D blocks
        for(int z=0; z<zInts.size(); z++)
            for(int y=0; y<yInts.size(); y++)
                for(int x=0; x<xInts.size(); x++)
                    ESblocks.push_back(block_t(xInts[x], yInts[y], zInts[z]));

//        printf("xInts:\n");
//        for(int i=0; i<xInts.size(); i++)
//            printf("[%d,%d)  ", xInts[i].start, xInts[i].end);
//        printf("\n");
//        printf("yInts:\n");
//        for(int i=0; i<yInts.size(); i++)
//            printf("[%d,%d)  ", yInts[i].start, yInts[i].end);
//        printf("\n");
//        printf("zInts:\n");
//        for(int i=0; i<zInts.size(); i++)
//            printf("[%d,%d)  ", zInts[i].start, zInts[i].end);
//        printf("\n");

    }
}

/**********************************************************************************
* Linked to verbosity combobox
***********************************************************************************/
void PMain::verbosityChanged(int i)
{
    /**/itm::debug(itm::LEV1, strprintf("i = %d", i).c_str(), __itm__current__function__);

    itm::DEBUG = i;
    iim::DEBUG = i;
//    CSettings::instance()->writeSettings();
}

void PMain::ESbuttonClicked()
{
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

    // check precondition: 3D visualization is active
    CExplorerWindow* curWin = CExplorerWindow::getCurrent();
    if(!curWin)
        return;


    // check precondition: valid volume at the currently displayed resolution
    VirtualVolume* vol = CImport::instance()->getVolume(curWin->volResIndex);
    if(!vol)
        return;


    // determine start condition
    bool start = !isESactive();


    // generate blocks and ask to confirm
    ESblocks.clear();
    if(start)
    {
        // generate blocks
        generateESblocks();

        // interrupt if trying to start ES mode at the lowest resolution
        if(curWin->volResIndex == 0)
        {
            QMessageBox::warning(this, "Warning", "Cannot start the \"Sliding viewer\" mode at the lowest resolution. Please first zoom-in to a higher res.");
            ESblocks.clear();
            return;
        }

        // interrupt if ES mode has to be started with just 1 block
        else if(ESblocks.size() < 2)
        {
            QMessageBox::warning(this, "Warning", "Cannot start the \"Sliding viewer\" mode. At least 2 blocks are needed. Please change volume resolution (too low) "
                                 "or your viewer maximum dimensions (too big)");
            ESblocks.clear();
            return;
        }

        // ask to confirm before proceed
        else if(QMessageBox::Yes != QMessageBox::question(this, "Confirm", strprintf("The whole volume at resolution (%d x %d x %d) has been subdivided into %d %soverlapping 3D blocks, "
                                                        "each sized %d x %d x %d at most.\n\nThroughout the guided scan, all TeraFly's 3D navigation features "
                                                        "will be temporarily disabled.\n\nContinue?", vol->getDIM_H(), vol->getDIM_V(), vol->getDIM_D(), ESblocks.size(),
                                                        ESoverlapSpbox->value() == 0 ? "non" : "", Hdim_sbox->value(), Vdim_sbox->value(), Ddim_sbox->value()).c_str(),
                                                        QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes))
        {
            ESblocks.clear();
            return;
        }
    }


    // "Local viewer" panel widgets
    resolution_cbox->setEnabled(!start);
    Vdim_sbox->setEnabled(!start);
    Hdim_sbox->setEnabled(!start);
    Ddim_sbox->setEnabled(!start);
    Tdim_sbox->setEnabled(!start);


    // "Zoom in/out" panel widgets
    zoom_panel->setEnabled(!start);
    if(start)
    {
        zoomInSens->setValue(zoomInSens->maximum());
        zoomOutSens->setValue(zoomOutSens->minimum());
    }
    else
        resetMultiresControls();


    // "Global coordinates" panel widgets
    V0_sbox->setEnabled(!start);
    V1_sbox->setEnabled(!start);
    H0_sbox->setEnabled(!start);
    H1_sbox->setEnabled(!start);
    D0_sbox->setEnabled(!start);
    D1_sbox->setEnabled(!start);
    to_label_1->setEnabled(!start);
    to_label_2->setEnabled(!start);
    to_label_3->setEnabled(!start);
    if(frameCoord->isEnabled())
    {
        T0_sbox->setEnabled(!start);
        T1_sbox->setEnabled(!start);
        to_label_4->setEnabled(!start);
    }
    traslXlabel->setText(start ? "<font size=\"4\" color=\"gray\">X</font>" : "<font size=\"4\" color=\"red\"><b>X</b></font>");
    traslYlabel->setText(start ? "<font size=\"4\" color=\"gray\">Y</font>" : "<font size=\"4\" color=\"green\"><b>Y</b></font>");
    traslZlabel->setText(start ? "<font size=\"4\" color=\"gray\">Z</font>" : "<font size=\"4\" color=\"blue\"><b>Z</b></font>");
    traslTlabel->setText(start ? "<font size=\"4\" color=\"gray\">t</font>" : "<font size=\"4\" color=\"gray\"><b>t</b></font>");
    traslYneg->setEnabled(!start && curWin->volV0 > 0);
    traslYpos->setEnabled(!start && curWin->volV1 < CImport::instance()->getVolume(curWin->volResIndex)->getDIM_V());
    traslXneg->setEnabled(!start && curWin->volH0 > 0);
    traslXpos->setEnabled(!start && curWin->volH1 < CImport::instance()->getVolume(curWin->volResIndex)->getDIM_H());
    traslZneg->setEnabled(!start && curWin->volD0 > 0);
    traslZpos->setEnabled(!start && curWin->volD1 < CImport::instance()->getVolume(curWin->volResIndex)->getDIM_D());
    traslTneg->setEnabled(!start && curWin->volT0 > 0);
    traslTpos->setEnabled(!start && curWin->volT1 < CImport::instance()->getVolume(curWin->volResIndex)->getDIM_T()-1);
    refSys->setFilled(!start);
    if(start)
        refSys->setZoom(-8.0);
    else
    {
        refSys->setDims(curWin->volH1-curWin->volH0+1, curWin->volV1-curWin->volV0+1, curWin->volD1-curWin->volD0+1);
        refSys->resetZoom();
    }


    // "Sliding viewer" panel
    ESbutton->setText(start ? "Stop" : "Start");
    ESbutton->setIcon(start ? QIcon(":/icons/stop.png") : QIcon(":/icons/start.png"));
    ESoverlapSpbox->setEnabled(!start);
    ESmethodCbox->setEnabled(!start);
    ESblockSpbox->setEnabled(start);
    ESblockSpbox->setSuffix(start ? "/" + QString::number(ESblocks.size()) : "/0");
    ESblockSpbox->setMaximum(start ? ESblocks.size(): 0);
    ESblockSpbox->setMinimum(start ? 1: 0);
    ESblockSpbox->setValue(start ? 1: 0);
}

/**********************************************************************************
* Called when the correspont spin box has changed
***********************************************************************************/
void PMain::ESblockSpboxChanged(int b)
{
    if(b == 0 || !ESblockSpbox->isEnabled())
        return;

    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

    CExplorerWindow* curWin = CExplorerWindow::getCurrent();
    if(curWin)
    {
        // update reference system
        int ROIxS   = ESblocks[b-1].xInt.start;
        int ROIxDim = ESblocks[b-1].xInt.end   - ROIxS;
        int ROIyS   = ESblocks[b-1].yInt.start;
        int ROIyDim = ESblocks[b-1].yInt.end   - ROIyS;
        int ROIzS   = ESblocks[b-1].zInt.start;
        int ROIzDim = ESblocks[b-1].zInt.end   - ROIzS;
        int dimX   = CImport::instance()->getVolume(curWin->volResIndex)->getDIM_H();
        int dimY   = CImport::instance()->getVolume(curWin->volResIndex)->getDIM_V();
        int dimZ   = CImport::instance()->getVolume(curWin->volResIndex)->getDIM_D();
        refSys->setDims(dimX, dimY, dimZ, ROIxDim, ROIyDim, ROIzDim, ROIxS, ROIyS, ROIzS);

        // invoke new view
        curWin->newView(ESblocks[b-1].xInt.end, ESblocks[b-1].yInt.end, ESblocks[b-1].zInt.end, curWin->volResIndex,
                curWin->volT0, curWin->volT1, false, -1, -1, -1, ESblocks[b-1].xInt.start, ESblocks[b-1].yInt.start, ESblocks[b-1].zInt.start, true, false);
    }
}

/**********************************************************************************
* Called when the corresponding QLineEdit has been edited
***********************************************************************************/
void PMain::debugRedirectSTDoutPathEdited(QString s)
{
    if(s.isEmpty())
    {
        itm::DEBUG_TO_FILE = false;
        iim::DEBUG_TO_FILE = false;
    }
    else
    {
        itm::DEBUG_TO_FILE = true;
        itm::DEBUG_FILE_PATH = s.toStdString();
        iim::DEBUG_TO_FILE = true;
        iim::DEBUG_FILE_PATH = s.toStdString();
    }
}


/**********************************************************************************
* <sendProgressBarChanged> event handler
***********************************************************************************/
void PMain::progressBarChanged(int val, int minutes, int seconds, const char* message)
{
    /**/itm::debug(itm::LEV3, strprintf("val = %d, minutes = %d, seconds = %d, message = %s", val, minutes, seconds, message).c_str(), __itm__current__function__);

    progressBar->setMinimum(0);
    progressBar->setMaximum(100);
    progressBar->setValue(val);
    QString remaining_time = QString::number(minutes);
    remaining_time.append(" minutes and ");
    remaining_time.append(QString::number(seconds));
    remaining_time.append(" seconds remaining");
    if(message && strlen(message) != 0)
    {
        statusBar->showMessage(message + QString(": ") + remaining_time);
    }
    else
        statusBar->showMessage(remaining_time);
}
