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
#include "control/CImport.h"
#include "control/CVolume.h"
#include "control/CSettings.h"
#include "control/CExplorerWindow.h"
#include "control/CAnnotations.h"
#include "renderer_gl1.h"
#include "v3dr_mainwindow.h"

using namespace teramanager;

PMain* PMain::uniqueInstance = NULL;
PMain* PMain::instance(V3DPluginCallback2 *callback, QWidget *parent)
{
    if (uniqueInstance == NULL)
        uniqueInstance = new PMain(callback, parent);
    return uniqueInstance;
}
void PMain::uninstance()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread unknown] >> PMain uninstance\n");
    #endif

    CImport::uninstance();
    PDialogImport::uninstance();
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
    printf("--------------------- teramanager plugin [thread %d] >> PMain destroyed\n", this->thread()->currentThreadId());
    #endif
}

PMain::PMain(V3DPluginCallback2 *callback, QWidget *parent) : QWidget(parent)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PMain created\n", this->thread()->currentThreadId());
    #endif

    //initializing members
    V3D_env = callback;
    parentWidget = parent;

    //creating fonts
    QFont tinyFont = QApplication::font();
    tinyFont.setPointSize(10);
    QFont smallFont = QApplication::font();
    smallFont.setPointSize(11);

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
    menuBar = new QMenuBar(this);
    fileMenu = menuBar->addMenu("File");
    openVolumeAction = new QAction("Open volume", this);
    closeVolumeAction = new QAction("Close volume", this);
    exitAction = new QAction("Exit", this);
    openVolumeAction->setShortcuts(QKeySequence::Open);
    closeVolumeAction->setShortcuts(QKeySequence::Close);
    exitAction->setShortcuts(QKeySequence::Quit);
    connect(openVolumeAction, SIGNAL(triggered()), this, SLOT(openVolume()));
    connect(closeVolumeAction, SIGNAL(triggered()), this, SLOT(closeVolume()));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(exit()));
    fileMenu->addAction(openVolumeAction);
    fileMenu->addAction(closeVolumeAction);
    fileMenu->addAction(exitAction);

    optionsMenu = menuBar->addMenu("Options");
    importOptionsMenu = optionsMenu->addMenu("Import");
    importOptionsWidget = new QWidgetAction(this);
    importOptionsWidget->setDefaultWidget(import_form);
    importOptionsMenu->addAction(importOptionsWidget);

    helpMenu = menuBar->addMenu("Help");
    aboutAction = new QAction("About", this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    helpMenu->addAction(aboutAction);

    //multiresolution mode widgets
    multires_panel = new QGroupBox("Multiresolution mode");
    subvol_dims_label = new QLabel("Zoom-in VOI dimensions:");
    Vdim_sbox = new QSpinBox();
    Vdim_sbox->setAlignment(Qt::AlignCenter);
    Vdim_sbox->setMaximum(1000);
    Vdim_sbox->setValue(CSettings::instance()->getVOIdimV());
    Hdim_sbox = new QSpinBox();
    Hdim_sbox->setAlignment(Qt::AlignCenter);
    Hdim_sbox->setMaximum(1000);
    Hdim_sbox->setValue(CSettings::instance()->getVOIdimH());
    Ddim_sbox = new QSpinBox();
    Ddim_sbox->setAlignment(Qt::AlignCenter);
    Ddim_sbox->setMaximum(1000);
    Ddim_sbox->setValue(CSettings::instance()->getVOIdimD());
    direction_V_label_6 = new QLabel("(Y)");
    direction_H_label_6 = new QLabel("(X)");
    direction_D_label_6 = new QLabel("(Z)");
    by_label_6 = new QLabel(QChar(0x00D7));
    by_label_7 = new QLabel(QChar(0x00D7));
    zoominVoiSize = new QLabel();
    zoominVoiSize->setAlignment(Qt::AlignCenter);
    zoominVoiSize->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    zoominVoiSize->setFont(tinyFont);
    resolution_cbox = new QComboBox();


    //info panel widgets
    info_panel = new QGroupBox("Volume's informations");
    volume_dims_label = new QLabel("Dimensions (voxels)");
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
    stacks_dims_label = new QLabel("Stacks dimensions (voxels):");
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
    voxel_dims_label = new QLabel(QString("Voxel's dimensions (").append(QChar(0x03BC)).append("m)"));
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
    subvol_panel = new QGroupBox("VOI's selection from the highest resolution volume:");
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
    to_label_1 = new QLabel("to");
    to_label_1->setAlignment(Qt::AlignCenter);
    to_label_2 = new QLabel("to");
    to_label_2->setAlignment(Qt::AlignCenter);
    to_label_3 = new QLabel("to");
    to_label_3->setAlignment(Qt::AlignCenter);
    direction_V_label_5 = new QLabel("Y:");
    direction_H_label_5 = new QLabel("X:");
    direction_D_label_5 = new QLabel("Z:");
    loadButton = new QPushButton(this);
    loadButton->setIcon(QIcon(":/icons/load.png"));
    loadButton->setText("Load");
    loadButton->setIconSize(QSize(30,30));

    //other widgets
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
    info_panel_layout->addWidget(volume_dims_label,     0,0,1,9);
    info_panel_layout->addWidget(vol_width_field,       0,9,1,2);
    info_panel_layout->addWidget(direction_H_label_0,   0,11,1,1);
    info_panel_layout->addWidget(by_label_01,           0,12,1,1);
    info_panel_layout->addWidget(vol_height_field,      0,13,1,2);
    info_panel_layout->addWidget(direction_V_label_0,   0,15,1,1);
    info_panel_layout->addWidget(by_label_02,           0,16,1,1);
    info_panel_layout->addWidget(vol_depth_field,       0,17,1,2);
    info_panel_layout->addWidget(direction_D_label_0,   0,19,1,1);
    info_panel_layout->addWidget(volume_stacks_label,   1,0,1,9);
    info_panel_layout->addWidget(ncols_field,           1,9,1,2);
    info_panel_layout->addWidget(direction_H_label_1,   1,11,1,1);
    info_panel_layout->addWidget(by_label_1,            1,12,1,1);
    info_panel_layout->addWidget(nrows_field,           1,13,1,2);
    info_panel_layout->addWidget(direction_V_label_1,   1,15,1,1);
    info_panel_layout->addWidget(stacks_dims_label,     2,0,1,9);
    info_panel_layout->addWidget(stack_width_field,     2,9,1,2);
    info_panel_layout->addWidget(direction_H_label_2,   2,11,1,1);
    info_panel_layout->addWidget(by_label_2,            2,12,1,1);
    info_panel_layout->addWidget(stack_height_field,    2,13,1,2);
    info_panel_layout->addWidget(direction_V_label_2,   2,15,1,1);
    info_panel_layout->addWidget(by_label_3,            2,16,1,1);
    info_panel_layout->addWidget(stack_depth_field,     2,17,1,2);
    info_panel_layout->addWidget(direction_D_label_2,   2,19,1,1);
    info_panel_layout->addWidget(voxel_dims_label,      3,0,1,9);
    info_panel_layout->addWidget(vxl_H_field,           3,9,1,2);
    info_panel_layout->addWidget(direction_H_label_3,   3,11,1,1);
    info_panel_layout->addWidget(by_label_4,            3,12,1,1);
    info_panel_layout->addWidget(vxl_V_field,           3,13,1,2);
    info_panel_layout->addWidget(direction_V_label_3,   3,15,1,1);
    info_panel_layout->addWidget(by_label_5,            3,16,1,1);
    info_panel_layout->addWidget(vxl_D_field,           3,17,1,2);
    info_panel_layout->addWidget(direction_D_label_3,   3,19,1,1);
    info_panel_layout->addWidget(origin_label,          4,0,1,9);
    info_panel_layout->addWidget(org_H_field,           4,9,1,2);
    info_panel_layout->addWidget(direction_H_label_4,   4,11,1,1);
    info_panel_layout->addWidget(org_V_field,           4,13,1,2);
    info_panel_layout->addWidget(direction_V_label_4,   4,15,1,1);
    info_panel_layout->addWidget(org_D_field,           4,17,1,2);
    info_panel_layout->addWidget(direction_D_label_4,   4,19,1,1);
    info_panel->setLayout(info_panel_layout);
    info_panel->setStyle(new QWindowsStyle());

    //subvolume selection widgets
    QGridLayout* subvol_panel_layout = new QGridLayout();
    subvol_panel_layout->addWidget(direction_H_label_5, 0, 0, 1, 1);
    subvol_panel_layout->addWidget(H0_sbox,             0, 1, 1, 2);
    subvol_panel_layout->addWidget(to_label_1,          0, 3, 1, 1);
    subvol_panel_layout->addWidget(H1_sbox,             0, 4, 1, 2);
    subvol_panel_layout->addWidget(direction_V_label_5, 0, 6, 1, 1);
    subvol_panel_layout->addWidget(V0_sbox,             0, 7, 1, 2);
    subvol_panel_layout->addWidget(to_label_2,          0, 9, 1, 1);
    subvol_panel_layout->addWidget(V1_sbox,             0, 10, 1, 2);
    subvol_panel_layout->addWidget(direction_D_label_5, 0, 12, 1, 1);
    subvol_panel_layout->addWidget(D0_sbox,             0, 13, 1, 2);
    subvol_panel_layout->addWidget(to_label_3,          0, 15, 1, 1);
    subvol_panel_layout->addWidget(D1_sbox,             0, 16, 1, 2);
    subvol_panel_layout->addWidget(loadButton,          2, 0, 1, 18);
    subvol_panel->setLayout(subvol_panel_layout);
    subvol_panel->setStyle(new QWindowsStyle());

    //multiresolution mode widgets
    QGridLayout* multiresModePanelLayout= new QGridLayout();
    multiresModePanelLayout->addWidget(new QLabel("Resolution:"),               0,0,1,4);
    multiresModePanelLayout->addWidget(resolution_cbox,                         0,5,1,11);
    multiresModePanelLayout->addWidget(subvol_dims_label,                       1, 0, 1, 4);
    multiresModePanelLayout->addWidget(Hdim_sbox,                               1, 5, 1, 2);
    multiresModePanelLayout->addWidget(direction_H_label_6,                     1, 7, 1, 1);
    multiresModePanelLayout->addWidget(by_label_6,                              1, 8, 1, 1);
    multiresModePanelLayout->addWidget(Vdim_sbox,                               1, 9, 1, 2);
    multiresModePanelLayout->addWidget(direction_V_label_6,                     1, 11, 1, 1);
    multiresModePanelLayout->addWidget(by_label_7,                              1, 12, 1, 1);
    multiresModePanelLayout->addWidget(Ddim_sbox,                               1, 13, 1, 2);
    multiresModePanelLayout->addWidget(direction_D_label_6,                     1, 15, 1, 1);
    multiresModePanelLayout->addWidget(new QLabel("Zoom-in VOI size"),          2, 0, 1, 4);
    multiresModePanelLayout->addWidget(zoominVoiSize,                           2, 5, 1, 2);
    multiresModePanelLayout->addWidget(new QLabel("MVoxels"),                   2, 7, 1, 9);
    multires_panel->setLayout(multiresModePanelLayout);
    multires_panel->setStyle(new QWindowsStyle());

    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(menuBar);
    layout->addWidget(info_panel);
    layout->addWidget(multires_panel);
    layout->addWidget(subvol_panel);
    layout->addWidget(statusBar);
    layout->addWidget(progressBar);
    setLayout(layout);
    setWindowTitle(tr("TeraFly plugin"));
    this->setFont(tinyFont);
    subvol_panel->setEnabled(false);

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
    resetGUI();

    //set always on top
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->setMaximumSize(this->minimumWidth(), this->minimumHeight());
    this->setFocusPolicy(Qt::StrongFocus);

    //reset widgets
    reset();
}

//reset everything
void PMain::reset()
{
    //resetting menu options and widgets
    openVolumeAction->setEnabled(true);
    closeVolumeAction->setEnabled(false);
    importOptionsMenu->setEnabled(true);
    importOptionsWidget->setEnabled(true);
    aboutAction->setEnabled(true);

    //reseting info panel widgets
    info_panel->setEnabled(false);
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
    for(int i=0; i<resolution_cbox->count(); i++)
        resolution_cbox->removeItem(i);

    //resetting subvol panel widgets
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
    printf("--------------------- teramanager plugin [thread %d] >> PMain loadButtonClicked() called\n", this->thread()->currentThreadId());
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
        //the object that will catch the SIGNAL emitted from CVolume is the last CExplorerWindow if multiresolution mode is enabled
        if(enableMultiresMode->isChecked() && CExplorerWindow::getLast())
            CVolume::instance()->setVoi(CExplorerWindow::getLast(), CImport::instance()->getResolutions()-1, V0_sbox->value()-1, V1_sbox->value()-1,H0_sbox->value()-1, H1_sbox->value()-1, D0_sbox->value()-1, D1_sbox->value()-1);
        else
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
* Called when "Open volume" menu action is triggered.
* Opens QFileDialog to select volume's path, which is copied into "path_field".
***********************************************************************************/
void PMain::openVolume()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PMain openVolume() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        //obtaining volume's directory
        string import_path= QFileDialog::getExistingDirectory(this, QObject::tr("Select volume's directory"), CSettings::instance()->getVolumePathLRU().c_str()).toStdString();

        //first checking that no volume has imported yet
        if(!CImport::instance()->isEmpty())
            throw MyException("A volume has been already imported! Please restart the plugin to import another volume.");

        //checking that the inserted path exists
        if(!StackedVolume::fileExists(import_path.c_str()))
            throw MyException("The inserted path does not exist!");

        //check if additional informations are required
        string mdata_fpath = import_path;
        mdata_fpath.append("/");
        mdata_fpath.append(IM_METADATA_FILE_NAME);
        string vmap_fpath = import_path;
        vmap_fpath.append("/");
        vmap_fpath.append(TMP_VMAP_FNAME);
        if(!StackedVolume::fileExists(mdata_fpath.c_str()) || reimport_checkbox->isChecked()) // ||
          //(!StackedVolume::fileExists(vmap_fpath.c_str()) && enableMultiresMode->isChecked()))    ---- Alessandro 2013-01-06: we do not want converted volumes to be imported again. I don't understand the function of this line of code
            PDialogImport::instance(this)->exec();
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
        PMain::instance()->resetGUI();
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
    printf("--------------------- teramanager plugin [thread %d] >> PMain closeVolume() launched\n", this->thread()->currentThreadId());
    #endif

    CImport::instance()->reset();
    CVolume::instance()->reset();

    PDialogImport::uninstance();
    CExplorerWindow::uninstance();
    CAnnotations::uninstance();
    reset();
}

/**********************************************************************************
* Called when "Exit" menu action is triggered or TeraFly window is closed.
***********************************************************************************/
void PMain::exit()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PMain exit() launched\n", this->thread()->currentThreadId());
    #endif

    this->close();
}

/**********************************************************************************
* Called when "Help->About" menu action is triggered
***********************************************************************************/
void PMain::about()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PMain about() launched\n", this->thread()->currentThreadId());
    #endif

    QMessageBox* msgBox = new QMessageBox(this);
    QSpacerItem* horizontalSpacer = new QSpacerItem(800, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    msgBox->setText( QString("<html><h1>TeraFly plugin v. ").append(CPlugin::getMajorVersion().c_str()).append("</h1>"
                    "<big>An experimental tool designed for Teravoxel-sized datasets 3D navigation into Vaa3D.</big><br><br>"
                    "<u>Developed by:</u><ul>"
                    "<li><b>Alessandro Bria</b> (email: a.bria@unicas.it)<br>"
                           "Ph.D. Student at University of Cassino</li>"
                    "<li><b>Giulio Iannello</b> (email: g.iannello@unicampus.it)<br>"
                           "Full Professor at University Campus Bio-Medico of Rome</li></ul><br>"
                    "<u>Features:</u><ul>"
                    "<li>user can select a subvolume to be shown into Vaa3D</li>"
                    "<li>low memory requirements (2 GB) for multi-stacked datasets</li></ul><br>"
                    "<u>Supported input formats:</u><ul>"
                    "<li>two-level directory structure with each tile containing a series of image slices (see documentation for further information)</li>"
                    "<li>supported formats for image slices are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF</li>"
                    "<li>no restriction on the bit depth</li>"
                    "<li>no restriction on the number of channels</li></ul></html>" ));

    QGridLayout* layout = (QGridLayout*)msgBox->layout();
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
//    msgBox->setWindowFlags(Qt::WindowStaysOnTopHint);
//    msgBox->setFocusPolicy(Qt::StrongFocus);
    msgBox->exec();
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
    printf("--------------------- teramanager plugin [thread %d] >> PMain import_done(%s) launched\n", this->thread()->currentThreadId(), (ex? "ex" : "NULL"));
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
        V0_sbox->setValue(volume->getDIM_V()/2 - 256);
        V1_sbox->setMinimum(1);
        V1_sbox->setMaximum(volume->getDIM_V());
        V1_sbox->setValue(volume->getDIM_V()/2 + 256);
        H0_sbox->setMinimum(1);
        H0_sbox->setMaximum(volume->getDIM_H());
        H0_sbox->setValue(volume->getDIM_H()/2 - 256);
        H1_sbox->setMinimum(1);
        H1_sbox->setMaximum(volume->getDIM_H());
        H1_sbox->setValue(volume->getDIM_H()/2 + 256);
        D0_sbox->setMinimum(1);
        D0_sbox->setMaximum(volume->getDIM_D());
        D0_sbox->setValue(volume->getDIM_D()/2 - 256);
        D1_sbox->setMinimum(1);
        D1_sbox->setMaximum(volume->getDIM_D());
        D1_sbox->setValue(volume->getDIM_D()/2 + 256);
        import_form->setEnabled(false);
        subvol_panel->setEnabled(true);
        loadButton->setEnabled(true);
        highestVOISizeChanged(0);

        //updating menu items
        openVolumeAction->setEnabled(false);
        importOptionsMenu->setEnabled(false);
        closeVolumeAction->setEnabled(true);

        //if multiresulution mode is enabled
        if(enableMultiresMode->isChecked())
        {
            //enabling multiresolution panel and hiding volume map options
            this->multires_panel->setEnabled(true);
            this->volMapWidget->setVisible(false);

            //updating zoom-in VOI size
            zoomInVoiSizeChanged(0);

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

            //instantiating CAnnotations
            CAnnotations::instance(volume->getDIM_V(), volume->getDIM_H(), volume->getDIM_D());

            //starting 3D exploration
            new CExplorerWindow(V3D_env, CImport::instance()->getVMapResIndex(), CImport::instance()->getVMap(),
                                0, CImport::instance()->getVMapHeight(), 0, CImport::instance()->getVMapWidth(),
                                0, CImport::instance()->getVMapDepth(), CImport::instance()->getNChannels(), 0);
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
    printf("--------------------- teramanager plugin [thread %d] >> PMain loading_done(%s) launched\n", this->thread()->currentThreadId(), (ex? "ex" : "NULL"));
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
    printf("--------------------- teramanager plugin [thread %d] >> PMain closeEvent() launched\n", this->thread()->currentThreadId());
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

//overrides focusInEvent method of QWidget
void PMain::focusInEvent ( QFocusEvent * event )
{

/* Added on January 29th by Alessandro: in my original intentions this lets the user quickly recover the renderer window
   after it is automatically hidden by Vaa3D when interacting with its menus. It does work, but the "Qt:Tool" flag also
   causes an unwanted change in the renderer title bar (minimize and full screen buttor are removed). The "standard" so-
   lution to recover a minimized / or hidden window is to use setWindowState method, but it doesn't work here.*/
//    if(CExplorerWindow::getLast())
//    {
//        event->accept();
//        CExplorerWindow::getLast()->getWindow3D()->setWindowFlags(Qt::Tool);
//        CExplorerWindow::getLast()->getWindow3D()->show();
//    }
//    else
        event->ignore();
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
    if(CExplorerWindow::getLast() && i != CExplorerWindow::getLast()->getResIndex())
    {
        QMessageBox::information(this, "Warning", "Operation not yet implemented!");
        resolution_cbox->setCurrentIndex(CExplorerWindow::getLast()->getResIndex());
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
