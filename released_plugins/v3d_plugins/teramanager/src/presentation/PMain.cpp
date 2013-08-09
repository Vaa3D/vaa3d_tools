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
#include "../control/CImport.h"
#include "../control/CVolume.h"
#include "../control/CSettings.h"
#include "../control/CExplorerWindow.h"
#include "../control/CAnnotations.h"
#include "../control/V3Dsubclasses.h"
#include "renderer_gl1.h"
#include "v3dr_mainwindow.h"

using namespace teramanager;

string PMain::HTwelcome = "Go to <i>File->Open volume</i> and select the directory of any resolution. The volume will be opened in <b>multiresolution mode</b>. "
                          "To disable multiresolution mode or to change volume import options, go to <i>Options->Import</i>.";
//string PMain::HTbase =    "<u>Navigate through different resolutions by</u>:<br><br>"
//                          "<b>zoom-in</b>: right-click-><i>Zoom-in HighRezImage</i> on image/marker;<br>"
//                          "<b>zoom-out</b>: mouse scroll down;<br>"
//                          "<b>jump to res</b>: select VOI with volume cut scrollbars/spinboxes and choose resolution from pull-down menu.<br><br>";
string PMain::HTbase =    "<b>What's this?</b><br><i>Move the mouse over an object and its description will be displayed here.</i>";
string PMain::HTvoiDim =  "Set the <b>dimensions</b> (in voxels) of the volume of interest (<b>VOI</b>) to be loaded when zoomin-in. "
                          "Please be careful not to set a too big region or you will soon use up your <b>graphic card's memory</b>. ";
string PMain::HTjumpToRes = "Choose from pull-down menu the <b>resolution</b> you want to jump to and the displayed image will be loaded at the resolution selected. "
                            "To load only a volume of interest (<b>VOI</b>) at the selected resolution, you may use the Vaa3D <i>Volume Cut</i> scrollbars "
                            "or the <i>Highest resolution volume's coordinates</i> spinboxes embedded in this plugin.";
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

PMain* PMain::uniqueInstance = NULL;
PMain* PMain::instance(V3DPluginCallback2 *callback, QWidget *parent)
{
    if (uniqueInstance == NULL)
        uniqueInstance = new PMain(callback, parent);
    else
    {
        uniqueInstance->setWindowState(Qt::WindowNoState);
        uniqueInstance->raise();
        uniqueInstance->activateWindow();
        uniqueInstance->show();
        if(CExplorerWindow::current)
        {
            CExplorerWindow::current->window3D->setWindowState(Qt::WindowNoState);
            CExplorerWindow::current->window3D->raise();
            CExplorerWindow::current->window3D->activateWindow();
            CExplorerWindow::current->window3D->show();
            CExplorerWindow::current->alignToLeft(uniqueInstance);
        }
        return uniqueInstance;
    }
}
PMain* PMain::getInstance()
{
    if(uniqueInstance)
        return uniqueInstance;
    else
        QMessageBox::critical(0,QObject::tr("Error"), QObject::tr("TeraFly not yet instantiated"),QObject::tr("Ok"));
}

void PMain::uninstance()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> PMain::uninstance()\n");
    #endif

    CImport::uninstance();
    PDialogImport::uninstance();
    PAbout::uninstance();
    CVolume::uninstance();
    CExplorerWindow::uninstance();
    CSettings::uninstance();
    CAnnotations::uninstance();
    if(uniqueInstance)
        delete uniqueInstance;
    uniqueInstance = 0;
}

PMain::~PMain()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::~PMain()\n");
    printf("--------------------- teramanager plugin [thread *] >> PMain destroyed\n");
    #endif
}

PMain::PMain(V3DPluginCallback2 *callback, QWidget *parent) : QWidget(parent)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::PMain()\n");
    #endif

    //initializing members
    V3D_env = callback;
    parentWidget = parent;
    annotationsPathLRU = "";
    marginLeft = 90;

    //creating fonts
    QFont tinyFont = QApplication::font();
    tinyFont.setPointSize(10);
    QFont smallFont = QApplication::font();
    smallFont.setPointSize(11);
    QFont xlabelFont = QApplication::font();
    xlabelFont.setBold(true);
    xlabelFont.setPointSize(11);

    //import form widgets
    import_form = new QWidget();
    reimport_checkbox = new QCheckBox("Re-import");
    enableMultiresMode = new QCheckBox("Enable multiresolution mode");
    enableMultiresMode->setChecked(true);
    volMapWidget = new QWidget();
    regenerateVolMap = new QCheckBox("Regenerate volume map");
    regenerateVolMap->setChecked(false);
    volMapMaxSizeSBox = new QSpinBox();
    volMapMaxSizeSBox->setMaximum(10);
    volMapMaxSizeSBox->setMaximum(1000);
    volMapMaxSizeSBox->setValue(CSettings::instance()->getVolMapSizeLimit());
    volMapMaxSizeSBox->setAlignment(Qt::AlignCenter);

    //initializing menu
    menuBar = new QMenuBar(0);
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
    optionsMenu = menuBar->addMenu("Options");
    importOptionsMenu = optionsMenu->addMenu("Import");
    importOptionsWidget = new QWidgetAction(this);
    importOptionsWidget->setDefaultWidget(import_form);
    importOptionsMenu->addAction(importOptionsWidget);    
    debugMenu = menuBar->addMenu("Debug");
    debugAction1 = new QAction("Action 1", this);
    connect(debugAction1, SIGNAL(triggered()), this, SLOT(debugAction1Triggered()));
    debugMenu->addAction(debugAction1);
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
                           "stop: 0 rgb(150,150,150), stop: 1 rgb(190,190,190));}");

    //multiresolution mode widgets
    multires_panel = new QGroupBox("Multiresolution mode");
    gradientBar = new QGradientBar(this);
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
    by_label_6 = new QLabel(QChar(0x00D7));
    by_label_7 = new QLabel(QChar(0x00D7));
    zoominVoiSize = new QLabel();
    zoominVoiSize->setAlignment(Qt::AlignCenter);
    zoominVoiSize->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    zoominVoiSize->setFont(tinyFont);
    resolution_cbox = new QComboBox();
    resolution_cbox->installEventFilter(this);
    zoomOutSens = new QSlider(Qt::Horizontal, this);
    zoomOutSens->setTickPosition(QSlider::TicksBelow);
    zoomOutSens->setMinimum(-100);
    zoomOutSens->setMaximum(0);
    zoomOutSens->setSingleStep(10);
    zoomOutSens->setPageStep(20);
    zoomOutSens->installEventFilter(this);
    zoomInSens = new QSlider(Qt::Horizontal, this);
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
    traslXpos = new QArrowButton(this, QColor(255,0,0), 15, 6, 0, Qt::LeftToRight);
    traslXneg = new QArrowButton(this, QColor(255,0,0), 15, 6, 0, Qt::RightToLeft);
    traslXlabel = new QLabel("");
    traslYpos = new QArrowButton(this, QColor(0,200,0), 15, 6, 0, Qt::LeftToRight);
    traslYneg = new QArrowButton(this, QColor(0,200,0), 15, 6, 0, Qt::RightToLeft);
    traslYlabel = new QLabel("");
    traslZpos = new QArrowButton(this, QColor(0,0,255), 15, 6, 0, Qt::LeftToRight);
    traslZneg = new QArrowButton(this, QColor(0,0,255), 15, 6, 0, Qt::RightToLeft);
    traslZlabel = new QLabel("");
    traslXpos->installEventFilter(this);
    traslXneg->installEventFilter(this);
    traslYpos->installEventFilter(this);
    traslYneg->installEventFilter(this);
    traslZpos->installEventFilter(this);
    traslZneg->installEventFilter(this);
    controlsResetButton = new QPushButton(this);
    controlsResetButton->setIcon(QIcon(":/icons/reset.png"));
    controlsLineTree = new QLineTree(this, Qt::gray, 0.5, 3, 11);
    zoomInMethod = new QComboBox(this);
    zoomInMethod->addItem("WYSIWYG (10 markers)");
    zoomInMethod->addItem("Foreground (1 marker)");
    zoomInMethod->addItem("Foreground (spread markers)");
    setEnabledComboBoxItem(zoomInMethod, 2, false);
    zoomInMethod->installEventFilter(this);

    #ifndef USE_EXPERIMENTAL_FEATURES
    zoomInMethod->setEnabled(false);    
    zoomInSens->setEnabled(false);
    #else
    zoomInMethod->setCurrentIndex(1);
    #endif


    //info panel widgets
    info_panel = new QGroupBox("Volume's informations");
    vol_size_label = new QLabel();
    vol_size_field = new QLabel();
    vol_size_field->setAlignment(Qt::AlignCenter);
    vol_size_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    vol_size_field->setFont(tinyFont);
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

    //subvol panel widgets
    subvol_panel = new QGroupBox("Highest resolution volume's coordinates");
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
    V0_sbox->installEventFilter(this);
    V1_sbox->installEventFilter(this);
    H0_sbox->installEventFilter(this);
    H1_sbox->installEventFilter(this);
    D0_sbox->installEventFilter(this);
    D1_sbox->installEventFilter(this);
    to_label_1 = new QLabel("to");
    to_label_1->setAlignment(Qt::AlignCenter);
    to_label_2 = new QLabel("to");
    to_label_2->setAlignment(Qt::AlignCenter);
    to_label_3 = new QLabel("to");
    to_label_3->setAlignment(Qt::AlignCenter);
    loadButton = new QPushButton(this);
    loadButton->setIcon(QIcon(":/icons/load.png"));
    loadButton->setText("Load");
    loadButton->setIconSize(QSize(30,30));

    //other widgets
    helpBox = new QHelpBox(this);
    progressBar = new QProgressBar(this);
    statusBar = new QStatusBar();

    //****LAYOUT SECTIONS****
    //import form
    QVBoxLayout* import_form_layout = new QVBoxLayout();
    import_form_layout->addWidget(reimport_checkbox);
    import_form_layout->addWidget(enableMultiresMode);
    QHBoxLayout* volMapSizeRow = new QHBoxLayout();
    volMapMaxSizeSBox->setMaximumWidth(60);
    volMapSizeRow->addWidget(volMapMaxSizeSBox);
    volMapSizeRow->addWidget(new QLabel("MVoxels volume map size limit"));
    QVBoxLayout* volMapWidgetLayout = new QVBoxLayout();
    volMapWidgetLayout->addLayout(volMapSizeRow);
    //volMapWidgetLayout->addWidget(regenerateVolMap);
    volMapWidgetLayout->setMargin(0);
    volMapWidget->setLayout(volMapWidgetLayout);
    volMapWidget->setContentsMargins(20,0,0,0);
    import_form_layout->addWidget(volMapWidget);
    import_form_layout->addWidget(regenerateVolMap);
    import_form->setLayout(import_form_layout);


    //info panel
    QGridLayout* info_panel_layout = new QGridLayout();

    info_panel_layout->addWidget(new QLabel("Size:"),      0,0,1,1);
    info_panel_layout->addWidget(vol_size_field,           0,2,1,2);
    info_panel_layout->addWidget(vol_size_label,           0,4,1,4);

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
    info_panel->setLayout(info_panel_layout);
    info_panel->setStyle(new QWindowsStyle());

    //subvolume selection widgets
    QGridLayout* subvol_panel_layout = new QGridLayout();
    QLabel* xCutLabel = new QLabel("X-cut interval:");
    xCutLabel->setFixedWidth(marginLeft);
    subvol_panel_layout->addWidget(xCutLabel,           0, 0, 1, 1);
    subvol_panel_layout->addWidget(H0_sbox,             0, 2, 1, 3);
    subvol_panel_layout->addWidget(to_label_1,          0, 5, 1, 1);
    subvol_panel_layout->addWidget(H1_sbox,             0, 6, 1, 3);

    subvol_panel_layout->addWidget(new QLabel("Y-cut interval:"), 1, 0, 1, 1);
    subvol_panel_layout->addWidget(V0_sbox,             1, 2, 1, 3);
    subvol_panel_layout->addWidget(to_label_2,          1, 5, 1, 1);
    subvol_panel_layout->addWidget(V1_sbox,             1, 6, 1,3);

    subvol_panel_layout->addWidget(new QLabel("Z-cut interval:"), 2, 0, 1, 1);
    subvol_panel_layout->addWidget(D0_sbox,             2, 2, 1, 3);
    subvol_panel_layout->addWidget(to_label_3,          2, 5, 1, 1);
    subvol_panel_layout->addWidget(D1_sbox,             2, 6, 1, 3);

    subvol_panel_layout->addWidget(loadButton,          3, 0, 1, 13);
    subvol_panel->setLayout(subvol_panel_layout);
    subvol_panel->setStyle(new QWindowsStyle());

    //multiresolution mode widgets
    QGridLayout* multiresModePanelLayout= new QGridLayout();
    QLabel* jumpToResLabel = new QLabel("Jump to res:");
    jumpToResLabel->setFixedWidth(marginLeft);


    multiresModePanelLayout->addWidget(new QLabel("Resolution:"),               0, 0, 1, 1);
    multiresModePanelLayout->addWidget(gradientBar,                             0, 2, 1, 11);
    multiresModePanelLayout->addWidget(jumpToResLabel,                          1, 0, 1, 1);
    multiresModePanelLayout->addWidget(resolution_cbox,                         1, 2, 1, 11);
    multiresModePanelLayout->addWidget(new QLabel("View max dims:"),         2, 0, 1, 1);
    multiresModePanelLayout->addWidget(Hdim_sbox,                               2, 2, 1, 3);
    multiresModePanelLayout->addWidget(by_label_6,                              2, 5, 1, 1);
    multiresModePanelLayout->addWidget(Vdim_sbox,                               2, 6, 1, 3);
    multiresModePanelLayout->addWidget(by_label_7,                              2, 9, 1, 1);
    multiresModePanelLayout->addWidget(Ddim_sbox,                               2, 10, 1, 3);
//    multiresModePanelLayout->addWidget(new QLabel("Zoom-in VOI size:"),         2, 0, 1, 1);
//    multiresModePanelLayout->addWidget(zoominVoiSize,                           2, 2, 1, 3);
//    multiresModePanelLayout->addWidget(new QLabel("MVoxels"),                   2, 5, 1, 9);
    multiresModePanelLayout->addWidget(new QLabel("Zoom-in method:"),           3 , 0, 1, 1);
    multiresModePanelLayout->addWidget(zoomInMethod,                            3, 2, 1, 11);
    multiresModePanelLayout->addWidget(new QLabel("Zoom-in  thres:"),           4 , 0, 1, 1);
    multiresModePanelLayout->addWidget(zoomInSens,                              4, 2, 1, 9);
    multiresModePanelLayout->addWidget(new QLabel("Caching  sens:"),           5 , 0, 1, 1);
    multiresModePanelLayout->addWidget(cacheSens,                               5, 2, 1, 9);
    controlsLineTree->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    multiresModePanelLayout->addWidget(controlsLineTree,                        4, 11, 3, 1);
    multiresModePanelLayout->addWidget(controlsResetButton,                     5, 12, 1, 1);
    multiresModePanelLayout->addWidget(new QLabel("Zoom-out thres:"),           6 , 0, 1, 1);
    multiresModePanelLayout->addWidget(zoomOutSens,                             6, 2, 1, 9);
    traslXneg->setMaximumWidth(25);
    traslXpos->setMaximumWidth(25);
    traslYneg->setMaximumWidth(25);
    traslYpos->setMaximumWidth(25);
    traslZneg->setMaximumWidth(25);
    traslZpos->setMaximumWidth(25);
    multiresModePanelLayout->addWidget(new QLabel("Translate:"),                7, 0, 1, 1);
    multiresModePanelLayout->addWidget(traslXneg,                               7, 2, 1, 1);
    multiresModePanelLayout->addWidget(traslXlabel,                             7, 3, 1, 1);
    multiresModePanelLayout->addWidget(traslXpos,                               7, 4, 1, 1);
    multiresModePanelLayout->addWidget(traslYneg,                               7, 6, 1, 1);
    multiresModePanelLayout->addWidget(traslYlabel,                             7, 7, 1, 1);
    multiresModePanelLayout->addWidget(traslYpos,                               7, 8, 1, 1);
    multiresModePanelLayout->addWidget(traslZneg,                               7, 10, 1, 1);
    multiresModePanelLayout->addWidget(traslZlabel,                             7, 11, 1, 1);
    multiresModePanelLayout->addWidget(traslZpos,                               7, 12, 1, 1);
    multires_panel->setLayout(multiresModePanelLayout);
    multires_panel->setStyle(new QWindowsStyle());

    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    QHBoxLayout* centralLayout = new QHBoxLayout();
    QVBoxLayout* innerLayout = new QVBoxLayout();
    QVBoxLayout* bottomLayout = new QVBoxLayout();
    innerLayout->addWidget(info_panel, 0);
    innerLayout->addWidget(multires_panel, 0);
    innerLayout->addWidget(subvol_panel, 0);
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
    connect(loadButton, SIGNAL(clicked()), this, SLOT(loadButtonClicked()));
    connect(CImport::instance(), SIGNAL(sendOperationOutcome(MyException*, Image4DSimple*)), this, SLOT(importDone(MyException*, Image4DSimple*)), Qt::QueuedConnection);
    connect(CVolume::instance(), SIGNAL(sendOperationOutcome(MyException*,void*)), SLOT(loadingDone(MyException*,void*)), Qt::QueuedConnection);
    connect(enableMultiresMode, SIGNAL(stateChanged(int)), this, SLOT(mode3D_checkbox_changed(int)), Qt::QueuedConnection);
    connect(volMapMaxSizeSBox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)), Qt::QueuedConnection);
    connect(Vdim_sbox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)), Qt::QueuedConnection);
    connect(Hdim_sbox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)), Qt::QueuedConnection);
    connect(Ddim_sbox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)), Qt::QueuedConnection);
    connect(Vdim_sbox, SIGNAL(valueChanged(int)), this, SLOT(zoomInVoiSizeChanged(int)), Qt::QueuedConnection);
    connect(Hdim_sbox, SIGNAL(valueChanged(int)), this, SLOT(zoomInVoiSizeChanged(int)), Qt::QueuedConnection);
    connect(Ddim_sbox, SIGNAL(valueChanged(int)), this, SLOT(zoomInVoiSizeChanged(int)), Qt::QueuedConnection);
    connect(V0_sbox, SIGNAL(valueChanged(int)), this, SLOT(highestVOISizeChanged(int)));
    connect(V1_sbox, SIGNAL(valueChanged(int)), this, SLOT(highestVOISizeChanged(int)));
    connect(H0_sbox, SIGNAL(valueChanged(int)), this, SLOT(highestVOISizeChanged(int)));
    connect(H1_sbox, SIGNAL(valueChanged(int)), this, SLOT(highestVOISizeChanged(int)));
    connect(D0_sbox, SIGNAL(valueChanged(int)), this, SLOT(highestVOISizeChanged(int)));
    connect(D1_sbox, SIGNAL(valueChanged(int)), this, SLOT(highestVOISizeChanged(int)));
    connect(resolution_cbox, SIGNAL(currentIndexChanged(int)), this, SLOT(resolutionIndexChanged(int)), Qt::QueuedConnection);
    connect(traslXpos, SIGNAL(clicked()), this, SLOT(traslXposClicked()));
    connect(traslXneg, SIGNAL(clicked()), this, SLOT(traslXnegClicked()));
    connect(traslYpos, SIGNAL(clicked()), this, SLOT(traslYposClicked()));
    connect(traslYneg, SIGNAL(clicked()), this, SLOT(traslYnegClicked()));
    connect(traslZpos, SIGNAL(clicked()), this, SLOT(traslZposClicked()));
    connect(traslZneg, SIGNAL(clicked()), this, SLOT(traslZnegClicked()));
    connect(controlsResetButton, SIGNAL(clicked()), this, SLOT(resetMultiresControls()));
    connect(zoomInSens, SIGNAL(valueChanged(int)), this, SLOT(zoomInSensChanged(int)));

    //reset widgets
    reset();
    resetMultiresControls();

    //set always on top
    setWindowFlags(Qt::WindowStaysOnTopHint);
    setMaximumSize(this->minimumWidth(), this->minimumHeight());
    show();
    move(QApplication::desktop()->screen()->rect().center() - rect().center());

    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain created\n");
    #endif
}

//reset everything
void PMain::reset()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::reset()\n");
    #endif

    //resetting menu options and widgets
    openVolumeAction->setEnabled(true);
    openVolumeToolButton->setEnabled(true);
    recentVolumesMenu->setEnabled(true);
    closeVolumeAction->setEnabled(false);
    importOptionsMenu->setEnabled(true);
    importOptionsWidget->setEnabled(true);
    import_form->setEnabled(true);
    aboutAction->setEnabled(true);
    loadAnnotationsAction->setEnabled(false);
    saveAnnotationsAction->setEnabled(false);
    saveAnnotationsAsAction->setEnabled(false);
    clearAnnotationsAction->setEnabled(false);

    //reseting info panel widgets
    info_panel->setEnabled(false);
    vol_size_label->setText("");
    vol_size_field->setText("n.a.");
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
    multires_panel->setEnabled(false);
    Vdim_sbox->setValue(CSettings::instance()->getVOIdimV());
    Hdim_sbox->setValue(CSettings::instance()->getVOIdimH());
    Ddim_sbox->setValue(CSettings::instance()->getVOIdimD());
    zoominVoiSize->setText("n.a.");
    resolution_cbox->setEnabled(false);
    while(resolution_cbox->count())
        resolution_cbox->removeItem(0);
    traslXlabel->setAlignment(Qt::AlignCenter);
    traslXlabel->setTextFormat(Qt::RichText);
    traslXlabel->setText("<font size=\"4\">X</font>");
    traslYlabel->setAlignment(Qt::AlignCenter);
    traslYlabel->setTextFormat(Qt::RichText);
    traslYlabel->setText("<font size=\"4\">Y</font>");
    traslZlabel->setAlignment(Qt::AlignCenter);
    traslZlabel->setTextFormat(Qt::RichText);
    traslZlabel->setText("<font size=\"4\">Z</font>");
    traslXpos->setEnabled(false);
    traslXneg->setEnabled(false);
    traslYpos->setEnabled(false);
    traslYneg->setEnabled(false);
    traslZpos->setEnabled(false);
    traslZneg->setEnabled(false);
    gradientBar->setEnabled(false);
    gradientBar->setNSteps(-1);
    gradientBar->setStep(0);

    //resetting subvol panel widgets    
    loadButton->setVisible(false);
    subvol_panel->setEnabled(false);
    V0_sbox->setValue(0);
    V1_sbox->setValue(0);
    H0_sbox->setValue(0);
    H1_sbox->setValue(0);
    D0_sbox->setValue(0);
    D1_sbox->setValue(0);

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
    progressBar->setEnabled(false);
    progressBar->setMaximum(1);         //needed to stop animation on some operating systems
    statusBar->clearMessage();
    statusBar->showMessage("Ready.");
}


//Called when "enable3Dmode" state changed.
void PMain::mode3D_checkbox_changed(int)
{
    volMapWidget->setVisible(enableMultiresMode->isChecked());
}

//called when loadButton has been clicked
void PMain::loadButtonClicked()
{ 
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain loadButtonClicked() called\n");
    #endif

    try
    {
        //first checking that a volume has been properly imported
        if(!CImport::instance()->getHighestResVolume())
            throw MyException("A volume should be imported first.");

        //disabling import form and enabling progress bar animation and tab wait animation
        progressBar->setEnabled(true);
        progressBar->setMinimum(0);
        progressBar->setMaximum(0);
        loadButton->setEnabled(false);
        subvol_panel->setEnabled(false);
        statusBar->showMessage("Loading selected subvolume...");

        //starting operation
        CVolume::instance()->setVoi(this, CImport::instance()->getResolutions()-1, V0_sbox->value()-1, V1_sbox->value()-1,H0_sbox->value()-1, H1_sbox->value()-1, D0_sbox->value()-1, D1_sbox->value()-1);
        CVolume::instance()->start();
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        resetGUI();
        subvol_panel->setEnabled(true);
        loadButton->setEnabled(true);
    }
}

/**********************************************************************************
* Called when a path in the "Recent volumes" menu is selected.
***********************************************************************************/
void PMain::openVolumeActionTriggered()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::openVolumeActionTriggered() launched\n");
    #endif

    this->openVolume(qobject_cast<QAction*>(sender())->text().toStdString());
}

/**********************************************************************************
* Called when "Clear menu" action in "Recent volumes" menu is triggered.
***********************************************************************************/
void PMain::clearRecentVolumesTriggered()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::clearRecentVolumesTriggered() launched\n");
    #endif

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
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::openVolume(path = \"%s\")\n", path.c_str());
    #endif

    try
    {
        string import_path = path;

        if(import_path.empty())
        {
            //---- Alessandro 2013-05-20: obtaining volume's directory with QFileDialog instead of platform native file dialogs
            //                            since a strange behaviour has been shown by native file dialogs on MacOS X.
            QFileDialog dialog(0);
            dialog.setFileMode(QFileDialog::DirectoryOnly);
            dialog.setViewMode(QFileDialog::Detail);
            dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
            dialog.setWindowTitle("Select volume's directory");
            dialog.setDirectory(CSettings::instance()->getVolumePathLRU().c_str());
            if(dialog.exec())
            {
                printf("dialog.directory().absolutePath() = \"%s\"\n", dialog.directory().absolutePath().toStdString().c_str());
                printf("dialog.directory().dirName() = \"%s\"\n", dialog.directory().dirName().toStdString().c_str());
                printf("dialog.directory().currentPath() = \"%s\"\n", dialog.directory().currentPath().toStdString().c_str());
                printf("dialog.directory().tempPath() = \"%s\"\n", dialog.directory().tempPath().toStdString().c_str());
                import_path = dialog.directory().absolutePath().toStdString();
            }

            //if no directory has been selected, terminating this method
            if(import_path.empty())
                return;
        }

        //first checking that no volume has imported yet
        if(!CImport::instance()->isEmpty())
            throw MyException("A volume has been already imported! Please close the current volume first.");

        //checking that the inserted path exists
        if(!StackedVolume::fileExists(import_path.c_str()))
            throw MyException("The inserted path does not exist!");

        //storing the path into CSettings
        CSettings::instance()->setVolumePathLRU(import_path);
        CSettings::instance()->addVolumePathToHistory(import_path);
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
        string mdata_fpath = import_path;
        mdata_fpath.append("/");
        mdata_fpath.append(IM_METADATA_FILE_NAME);
        string vmap_fpath = import_path;
        vmap_fpath.append("/");
        vmap_fpath.append(TMP_VMAP_FNAME);
        if(!StackedVolume::fileExists(mdata_fpath.c_str()) || reimport_checkbox->isChecked()) // ||
          //(!StackedVolume::fileExists(vmap_fpath.c_str()) && enableMultiresMode->isChecked()))    ---- Alessandro 2013-01-06: we do not want converted volumes to be imported again. I don't understand the function of this line of code
            if(PDialogImport::instance(this)->exec() == QDialog::Rejected)
                return;
        CImport::instance()->setPath(import_path);
        CImport::instance()->setReimport(reimport_checkbox->isChecked());
        CImport::instance()->setMultiresMode(enableMultiresMode->isChecked());
        CImport::instance()->setRegenerateVolumeMap(regenerateVolMap->isChecked());
        CImport::instance()->setVolMapMaxSize(volMapMaxSizeSBox->value());

        //disabling import form and enabling progress bar animation
        progressBar->setEnabled(true);
        progressBar->setMinimum(0);
        progressBar->setMaximum(0);
        loadButton->setEnabled(false);
        import_form->setEnabled(false);
        statusBar->showMessage("Importing volume...");

        //starting import
        CImport::instance()->start();
    }
    catch(MyException &ex)
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
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::closeVolume()\n");
    #endif

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
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::loadAnnotations()\n");
    #endif

    try
    {
        if(CExplorerWindow::getCurrent())
        {
            //obtaining path
            QDir dir(CImport::instance()->getPath().c_str());
            dir.cdUp();

            //---- Alessandro 2013-05-20: obtaining volume's directory with QFileDialog instead of platform native file dialogs
            //                            since a strange behaviour has been shown by native file dialogs on MacOS X.
            string path = "";
            QFileDialog dialog(0);
            dialog.setFileMode(QFileDialog::ExistingFile);
            dialog.setViewMode(QFileDialog::Detail);
            dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
            dialog.setWindowTitle("Open annotation file");
            dialog.setNameFilter(tr("annotation files (*.ano)"));
            dialog.setDirectory(dir.absolutePath().toStdString().c_str());
            if(dialog.exec())
                if(!dialog.selectedFiles().empty())
                    path = dialog.selectedFiles().front().toStdString();

            if(strcmp(path.c_str(), "") == 0)
                return;
            annotationsPathLRU = path;

            CAnnotations::getInstance()->load(annotationsPathLRU.c_str());
            CExplorerWindow::getCurrent()->loadAnnotations();
            saveAnnotationsAction->setEnabled(true);

        }
    }
    catch(MyException &ex)
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
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::saveAnnotations()\n");
    #endif

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
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}
void PMain::saveAnnotationsAs()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::saveAnnotationsAs()\n");
    #endif

    try
    {
        if(CExplorerWindow::getCurrent())
        {
            QDir dir(CImport::instance()->getPath().c_str());
            dir.cdUp();

            //---- Alessandro 2013-05-20: obtaining volume's directory with QFileDialog instead of platform native file dialogs
            //                            since a strange behaviour has been shown by native file dialogs on MacOS X.
            string path = "";
            QFileDialog dialog(0);
            dialog.setFileMode(QFileDialog::AnyFile);
            dialog.setViewMode(QFileDialog::Detail);
            dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
            dialog.setWindowTitle("Save annotation file as");
            dialog.setNameFilter(tr("annotation files (*.ano)"));
            dialog.setDirectory(dir.absolutePath().toStdString().c_str());
            dialog.setAcceptMode(QFileDialog::AcceptSave);
            if(dialog.exec())
                if(!dialog.selectedFiles().empty())
                    path = dialog.selectedFiles().front().toStdString();

            if(strcmp(path.c_str(), "") == 0)
                return;
            annotationsPathLRU = path;

            if(annotationsPathLRU.find(".ano") == string::npos)
                annotationsPathLRU.append(".ano");


            CExplorerWindow::getCurrent()->storeAnnotations();
            CAnnotations::getInstance()->save(annotationsPathLRU.c_str());
            saveAnnotationsAction->setEnabled(true);
        }
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

/**********************************************************************************
* Called when "Clear annotations" menu action is triggered.
***********************************************************************************/
void PMain::clearAnnotations()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::clearAnnotations()\n");
    #endif

    try
    {
        if(CExplorerWindow::getCurrent())
        {
            CAnnotations::getInstance()->clear();
            CExplorerWindow::getCurrent()->loadAnnotations();
        }
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

/**********************************************************************************
* Called when "Exit" menu action is triggered or TeraFly window is closed.
***********************************************************************************/
void PMain::exit()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::exit()\n");
    #endif

    this->close();
}

/**********************************************************************************
* Called when "Help->About" menu action is triggered
***********************************************************************************/
void PMain::about()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::about()\n");
    #endif 

    PAbout::instance(this)->exec();
}


/*********************************************************************************
* Called by <CImport> when the associated operation has been performed.
* If an exception has occurred in the <CImport> thread,  it is propagated and man-
* aged in the current thread (ex != 0). Otherwise, volume information are imported
* in the GUI by the <StackedVolume> handle of <CImport>.
**********************************************************************************/
void PMain::importDone(MyException *ex, Image4DSimple* vmap_image)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::import_done(%s)\n",  (ex? "error" : ""));
    #endif

    //if an exception has occurred, showing a message error and re-enabling import form
    if(ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
        import_form->setEnabled(true);
    }
    else
    {
        //otherwise inserting volume's informations
        StackedVolume* volume = CImport::instance()->getHighestResVolume();
        info_panel->setEnabled(true);

        double GVoxels = (volume->getDIM_V()/1000.0f)*(volume->getDIM_H()/1000.0f)*(volume->getDIM_D()/1000.0f);
        double TVoxels = GVoxels/1000.0;
        if(TVoxels < 0.1)
        {
            vol_size_field->setText(QString("<b>").append(QString::number(GVoxels, 'f', 1).append("</b>")));
            vol_size_label->setText("GVoxels");
        }
        else
        {
            vol_size_field->setText(QString("<b>").append(QString::number(TVoxels, 'f', 1).append("</b>")));
            vol_size_label->setText("TVoxels");
        }

        vol_height_mm_field->setText(QString::number(fabs(volume->getDIM_V()*volume->getVXL_V()/1000.0f), 'f', 2));
        vol_width_mm_field->setText(QString::number(fabs(volume->getDIM_H()*volume->getVXL_H()/1000.0f), 'f', 2));
        vol_depth_mm_field->setText(QString::number(fabs(volume->getDIM_D()*volume->getVXL_D()/1000.0f), 'f', 2));
        vol_height_field->setText(QString::number(volume->getDIM_V()));
        vol_width_field->setText(QString::number(volume->getDIM_H()));
        vol_depth_field->setText(QString::number(volume->getDIM_D()));
        nrows_field->setText(QString::number(volume->getN_ROWS()));
        ncols_field->setText(QString::number(volume->getN_COLS()));
        stack_height_field->setText(QString::number(volume->getStacksHeight()));
        stack_width_field->setText(QString::number(volume->getStacksWidth()));
        stack_depth_field->setText(QString::number(volume->getDIM_D()));
        vxl_V_field->setText(QString::number(volume->getVXL_V(), 'f', 2));
        vxl_H_field->setText(QString::number(volume->getVXL_H(), 'f', 2));
        vxl_D_field->setText(QString::number(volume->getVXL_D(), 'f', 2));
        org_V_field->setText(QString::number(volume->getORG_V(), 'f', 2));
        org_H_field->setText(QString::number(volume->getORG_H(), 'f', 2));
        org_D_field->setText(QString::number(volume->getORG_D(), 'f', 2));

        //and setting subvol widgets limits
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
        import_form->setEnabled(false);
        subvol_panel->setEnabled(true);
        loadButton->setEnabled(true);
        highestVOISizeChanged(0);

        //updating menu items
        openVolumeAction->setEnabled(false);
        recentVolumesMenu->setEnabled(false);
        openVolumeToolButton->setEnabled(false);
        importOptionsMenu->setEnabled(false);
        closeVolumeAction->setEnabled(true);
        clearAnnotationsAction->setEnabled(true);

        //if multiresulution mode is enabled
        if(enableMultiresMode->isChecked())
        {
            //enabling multiresolution panel and hiding volume map options
            this->multires_panel->setEnabled(true);
            this->volMapWidget->setVisible(false);

            //enabling menu actions
            loadAnnotationsAction->setEnabled(true);
            saveAnnotationsAsAction->setEnabled(true);

            //updating zoom-in VOI size
            zoomInVoiSizeChanged(0);

            //updating gradient bar widget
            gradientBar->setEnabled(true);
            gradientBar->setNSteps(CImport::instance()->getResolutions());

            //inserting available resolutions
            for(int i=0; i<CImport::instance()->getResolutions(); i++)
            {
                QString option = "";
                StackedVolume* vol = CImport::instance()->getVolume(i);
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


            //instantiating CAnnotations
            CAnnotations::instance(volume->getDIM_V(), volume->getDIM_H(), volume->getDIM_D());

            //starting 3D exploration
            new CExplorerWindow(V3D_env, CImport::instance()->getVMapResIndex(), CImport::instance()->getVMap(),
                                0, CImport::instance()->getVMapHeight(), 0, CImport::instance()->getVMapWidth(),
                                0, CImport::instance()->getVMapDepth(), CImport::instance()->getNChannels(), 0);

            helpBox->setText(HTbase);
        }
        else
        {
            loadButton->setVisible(true);
        }

        //finally storing in application settings the path of the opened volume
        CSettings::instance()->setVolumePathLRU(CImport::instance()->getPath());
    }

    //resetting some widgets
    resetGUI();
}

/**********************************************************************************
* Called by <CVolume> when the associated operation has been performed.
* If an exception has occurred in the <CVolume> thread, it is propagated and
* managed in the current thread (ex != 0).
***********************************************************************************/
void PMain::loadingDone(MyException *ex, void* sourceObject)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::loading_done(%s)\n",  (ex? "error" : ""));
    #endif

    CVolume* cVolume = CVolume::instance();

    //if an exception has occurred, showing a message error
    if(ex)
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
    else if(sourceObject == this)
    {
        Image4DSimple* img = new Image4DSimple();
        img->setFileName(CImport::instance()->getHighestResVolume()->getROOT_DIR());
        img->setData(cVolume->getVoiData(), cVolume->getVoiH1()-cVolume->getVoiH0(),
                     cVolume->getVoiV1()-cVolume->getVoiV0(), cVolume->getVoiD1()-cVolume->getVoiD0(), cVolume->getNChannels(), V3D_UINT8);
        v3dhandle new_win = V3D_env->newImageWindow(img->getFileName());
        V3D_env->setImage(new_win, img);
    }

    //resetting some widgets
    resetGUI();
    subvol_panel->setEnabled(true);
    loadButton->setEnabled(true);
}

//overrides closeEvent method of QWidget
void PMain::closeEvent(QCloseEvent *evt)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::closeEvent()\n");
    #endif

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
    CSettings::instance()->setVolMapSizeLimit(volMapMaxSizeSBox->value());
    CSettings::instance()->setVOIdimV(Vdim_sbox->value());
    CSettings::instance()->setVOIdimH(Hdim_sbox->value());
    CSettings::instance()->setVOIdimD(Ddim_sbox->value());
    CSettings::instance()->writeSettings();
}

/**********************************************************************************
* Linked to resolution combobox
* This switches to the given resolution index.
***********************************************************************************/
void PMain::resolutionIndexChanged(int i)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::resolutionIndexChanged()\n");
    #endif

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
            float MVoxels = ((voiV1-voiV0+1)/1024.0f)*((voiH1-voiH0+1)/1024.0f)*(voiD1-voiD0+1);
            if(QMessageBox::Yes == QMessageBox::question(this, "Confirm", QString("The volume to be loaded is ").append(QString::number(MVoxels, 'f', 1)).append(" MVoxels big.\n\nDo you confirm?"), QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes))
            {
                //voi set
                CVolume::instance()->setVoi(CExplorerWindow::getCurrent(), i, voiV0, voiV1+1, voiH0, voiH1+1, voiD0, voiD1+1);

                //disabling import form and enabling progress bar animation and tab wait animation
                progressBar->setEnabled(true);
                progressBar->setMinimum(0);
                progressBar->setMaximum(0);
                loadButton->setEnabled(false);
                subvol_panel->setEnabled(false);
                statusBar->showMessage("Loading selected subvolume...");

                //saving state of subvol spinboxes
                CExplorerWindow::getCurrent()->saveSubvolSpinboxState();

                //launch operation
                CVolume::instance()->start();
            }
            else
                resolution_cbox->setCurrentIndex(CExplorerWindow::getCurrent()->getResIndex());
        }
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        resetGUI();
        subvol_panel->setEnabled(true);
        loadButton->setEnabled(true);
        resolution_cbox->setCurrentIndex(CExplorerWindow::getCurrent()->getResIndex());
    }
}

/**********************************************************************************
* Linked to zoom-in VOI spinboxes.
* This updates the zoom-in VOI size widget.
***********************************************************************************/
void PMain::zoomInVoiSizeChanged(int i)
{
    float MVoxels = (Vdim_sbox->value()/1024.0f)*(Hdim_sbox->value()/1024.0f)*Ddim_sbox->value();
    zoominVoiSize->setText(QString::number(MVoxels, 'f', 1));
}

/**********************************************************************************
* Linked to highest res VOI's selection spinboxes.
* This updates the load button text.
***********************************************************************************/
void PMain::highestVOISizeChanged(int i)
{
    float MVoxels = ((V1_sbox->value()-V0_sbox->value())/1024.0f)*((H1_sbox->value()-H0_sbox->value())/1024.0f)*(D1_sbox->value()-D0_sbox->value());
    loadButton->setText(QString("Load (").append(QString::number(MVoxels, 'f', 0)).append(" MVoxels)"));
}

/**********************************************************************************
* Called when the correspondent buttons are clicked
***********************************************************************************/
void PMain::traslXposClicked()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::traslXposClicked()\n");
    #endif

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl)
        expl->newView((expl->volH1-expl->volH0)/2 + (expl->volH1-expl->volH0)*CSettings::instance()->getTraslX()/100.0f,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2, expl->volResIndex, false);
}
void PMain::traslXnegClicked()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::traslXnegClicked()\n");
    #endif

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl)
        expl->newView((expl->volH1-expl->volH0)/2 - (expl->volH1-expl->volH0)*CSettings::instance()->getTraslX()/100.0f,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2, expl->volResIndex, false);
}
void PMain::traslYposClicked()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::traslYposClicked()\n");
    #endif

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl)
        expl->newView((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2 + (expl->volV1-expl->volV0)*CSettings::instance()->getTraslY()/100.0f,
                      (expl->volD1-expl->volD0)/2, expl->volResIndex, false);
}
void PMain::traslYnegClicked()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::traslYnegClicked()\n");
    #endif

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl)
        expl->newView((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2 - (expl->volV1-expl->volV0)*CSettings::instance()->getTraslY()/100.0f,
                      (expl->volD1-expl->volD0)/2, expl->volResIndex, false);
}
void PMain::traslZposClicked()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::traslZposClicked()\n");
    #endif

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl)
        expl->newView((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2 + (expl->volD1-expl->volD0)*CSettings::instance()->getTraslZ()/100.0f, expl->volResIndex, false);
}
void PMain::traslZnegClicked()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::traslZnegClicked()\n");
    #endif

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl)
        expl->newView((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2 - (expl->volD1-expl->volD0)*CSettings::instance()->getTraslZ()/100.0f, expl->volResIndex, false);
}

/**********************************************************************************
* Filters events generated by the widgets to which a help message must be associated
***********************************************************************************/
bool PMain::eventFilter(QObject *object, QEvent *event)
{
    if ((object == Vdim_sbox || object == Hdim_sbox || object == Ddim_sbox) && multires_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTvoiDim);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if((object == resolution_cbox) && multires_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTjumpToRes);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if((object == zoomOutSens) && multires_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTzoomOutThres);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);

        displayToolTip(zoomOutSens, event, QString::number(zoomOutSens->value()).toStdString());
    }
    else if((object == zoomInSens) && multires_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTzoomInThres);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);

        displayToolTip(zoomInSens, event, QString::number(zoomInSens->value()).toStdString());
    }
    else if((object == zoomInMethod) && multires_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTzoomInMethod);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if((object == cacheSens) && multires_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTcacheSens);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);

        displayToolTip(cacheSens, event, QString::number(cacheSens->value()).append("%").toStdString());

    }
    else if ((object == traslXpos || object == traslYpos || object == traslZpos) && multires_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTtraslatePos);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if ((object == traslXneg || object == traslYneg || object == traslZneg) && multires_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTtraslateNeg);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if ((object == V0_sbox || object == V1_sbox || object == H0_sbox || object == H1_sbox || object == D0_sbox || object == D1_sbox) && subvol_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTvolcuts);
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
* Called when the correspondent slider is changed
***********************************************************************************/
void PMain::zoomInSensChanged(int i)
{
//    zoomInThreshold = i;
}

/**********************************************************************************
* Called when the correspondent debug actions are triggered
***********************************************************************************/
void PMain::debugAction1Triggered()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PMain::debugAction1Triggered()\n");
    #endif

    #ifdef USE_EXPERIMENTAL_FEATURES
    CExplorerWindow* cur_win = CExplorerWindow::getCurrent();
    if(cur_win)
    {
        int dimx = cur_win->volH1 - cur_win->volH0;
        int dimy = cur_win->volV1 - cur_win->volV0;
        int dimz = cur_win->volD1 - cur_win->volD0;
        int cx = (dimx)/2;
        int cy = (dimy)/2;
        int cz = (dimz)/2;
        int r = 50;
        int side = 2*r;
        float vol = pow(2.0f*r, 3);
        int count = 0;

        static bool first_time = true;
        static bool restore_data = false;
        static uint8* data_saved = new uint8[static_cast<int>(vol)];
        uint8* data = cur_win->view3DWidget->getiDrawExternalParameter()->image4d->getRawData();
        if(first_time)
        {
            first_time = false;
            for(int k = cz - r; k < cz + r; k++)
                for(int i = cy - r; i < cy + r; i++)
                    for(int j = cx - r; j < cx + r; j++, count++)
                        data_saved[(k-cz+r)*side*side +(i-cy+r)*side + (j-cx+r)] = data[k*dimy*dimx + i*dimx + j];
        }

        if(restore_data)
        {
            for(int k = cz - r; k < cz + r; k++)
                for(int i = cy - r; i < cy + r; i++)
                    for(int j = cx - r; j < cx + r; j++, count++)
                        data[k*dimy*dimx + i*dimx + j] = data_saved[(k-cz+r)*side*side +(i-cy+r)*side + (j-cx+r)];
        }
        else
        {
            for(int k = cz - r; k < cz + r; k++)
                for(int i = cy - r; i < cy + r; i++)
                    for(int j = cx - r; j < cx + r; j++, count++)
                        data[k*dimy*dimx + i*dimx + j] = (count / vol) *255;
        }
        restore_data = !restore_data;

        myV3dR_GLWidget::cast(cur_win->view3DWidget)->updateImageDataFast();
    }
    #endif
}
