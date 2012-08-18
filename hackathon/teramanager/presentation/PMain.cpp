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
#include "control/CImport.h"
#include "control/CVolume.h"
#include "PDialogImport.h"
#include "renderer_gl1.h"
#include "v3d_imaging_para.h"

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
    CImport::uninstance();
    PDialogImport::uninstance();
    CVolume::uninstance();
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

PMain::PMain(V3DPluginCallback2 *callback, QWidget *parent) : QWidget(parent)
{
    #ifdef TMP_DEBUG
    printf("TeraManager plugin [thread %d] >> PMain created\n", this->thread()->currentThreadId());
    #endif

    //initializing members
    V3D_env = callback;
    parentWidget = parent;
    view3DWidget = 0;
    treeviewWidget = 0;

    //creating fonts
    QFont tinyFont = QApplication::font();
    tinyFont.setPointSize(10);
    QFont smallFont = QApplication::font();
    smallFont.setPointSize(11);

    //help box
    helpbox = new QLabel("<html><table><tr style=\"vertical-align: middle;\"><td><img src=\":/icons/help.png\"></td>"
                          "<td><p style=\"text-align:justify; margin-left:10px;\"> This experimental tools enables the visualization of "
                          "selectable portions of <b>teravoxel-sized datasets</b> with limited memory usage by exploiting the multi-stack "
                          "format, where provided.</p> </td></tr></table> </html>");
    helpbox->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); margin-top:10px; margin-bottom:10px; padding-top:10px; padding-bottom:10px;");
    helpbox->setWordWrap(true);
    helpbox->setFixedHeight(100);
    helpbox->setFont(tinyFont);

    //import form widgets
    import_form = new QGroupBox("Import form");
    path_field    = new QLineEdit("Enter the volume's directory");
    path_field->setMinimumWidth(200);
    voldir_button       = new QPushButton("Browse for dir...");
    reimport_checkbox = new QCheckBox("Overwrite previous imports");
    enableMultiresMode = new QCheckBox("Enable multiresolution mode");
    enableMultiresMode->setChecked(true);
    regenerateVolMap = new QCheckBox("Regenerate volume map");
    regenerateVolMap->setChecked(false);
    volMapMaxSizeSBox = new QSpinBox();
    volMapMaxSizeSBox->setMaximum(10);
    volMapMaxSizeSBox->setMaximum(1000);
    volMapMaxSizeSBox->setValue(200);
    volMapMaxSizeSBox->setAlignment(Qt::AlignCenter);

    //multiresolution mode widgets
    multires_panel = new QGroupBox("Multiresolution mode");
    subvol_dims_label = new QLabel("VOI dimensions:");
    Vdim_sbox = new QSpinBox();
    Vdim_sbox->setAlignment(Qt::AlignCenter);
    Vdim_sbox->setMaximum(1000);
    Vdim_sbox->setValue(200);
    Hdim_sbox = new QSpinBox();
    Hdim_sbox->setAlignment(Qt::AlignCenter);
    Hdim_sbox->setMaximum(1000);
    Hdim_sbox->setValue(200);
    Ddim_sbox = new QSpinBox();
    Ddim_sbox->setAlignment(Qt::AlignCenter);
    Ddim_sbox->setMaximum(1000);
    Ddim_sbox->setValue(100);
    direction_V_label_6 = new QLabel("(V)");
    direction_H_label_6 = new QLabel("(H)");
    direction_D_label_6 = new QLabel("(D)");
    by_label_6 = new QLabel("x");
    by_label_7 = new QLabel("x");


    //info panel widgets
    info_panel = new QGroupBox("Volume's informations");
    info_panel->setEnabled(false);
    volume_dims_label = new QLabel("Dimensions (voxels)");
    direction_V_label_0 = new QLabel("(V)");
    direction_H_label_0 = new QLabel("(H)");
    direction_D_label_0 = new QLabel("(D)");
    by_label_01 = new QLabel("x");
    by_label_02 = new QLabel("x");
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
    direction_V_label_1 = new QLabel("(V)");
    direction_H_label_1 = new QLabel("(H)");
    by_label_1 = new QLabel("x");
    nrows_field = new QLabel();
    nrows_field->setAlignment(Qt::AlignCenter);
    nrows_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    nrows_field->setFont(tinyFont);
    ncols_field = new QLabel();
    ncols_field->setAlignment(Qt::AlignCenter);
    ncols_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); padding-left:5px; padding-right:5px");
    ncols_field->setFont(tinyFont);
    stacks_dims_label = new QLabel("Stacks dimensions (voxels):");
    direction_V_label_2 = new QLabel("(V)");
    direction_H_label_2 = new QLabel("(H)");
    direction_D_label_2 = new QLabel("(D)");
    by_label_2 = new QLabel("x");
    by_label_3 = new QLabel("x");
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
    voxel_dims_label = new QLabel("Voxel's dimensions (microns):");
    direction_V_label_3 = new QLabel("(V)");
    direction_H_label_3 = new QLabel("(H)");
    direction_D_label_3 = new QLabel("(D)");
    by_label_4 = new QLabel("x");
    by_label_5 = new QLabel("x");
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
    origin_label = new QLabel("Origin (millimeters):");
    direction_V_label_4 = new QLabel("(V)");
    direction_H_label_4 = new QLabel("(H)");
    direction_D_label_4 = new QLabel("(D)");
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
    subvol_panel = new QGroupBox("Volume Of Interest's selection:");
    subvol_panel->setEnabled(false);
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
    direction_V_label_5 = new QLabel("V:");
    direction_H_label_5 = new QLabel("H:");
    direction_D_label_5 = new QLabel("D:");
    loadButton = new QPushButton(this);
    loadButton->setIcon(QIcon(":/icons/load.png"));
    loadButton->setText("Load");
    loadButton->setIconSize(QSize(30,30));

    //other widgets
    progressBar = new QProgressBar(this);
    statusBar = new QStatusBar();
    statusBar->showMessage("Ready.");

    //****LAYOUT SECTIONS****
    //import form
    QGridLayout* import_form_layout = new QGridLayout();
    import_form_layout->addWidget(path_field,           0,0,1,4);
    import_form_layout->addWidget(voldir_button,        0,4,1,2);
    import_form_layout->addWidget(reimport_checkbox,    1,0,1,3);
    import_form_layout->addWidget(regenerateVolMap,     1,3,1,3);
    import_form_layout->addWidget(enableMultiresMode,   3,0,1,6);
    import_form->setLayout(import_form_layout);
    import_form->setStyle(new QWindowsStyle());

    //info panel
    QGridLayout* info_panel_layout = new QGridLayout();
    info_panel_layout->addWidget(volume_dims_label,     0,0,1,9);
    info_panel_layout->addWidget(vol_height_field,      0,9,1,2);
    info_panel_layout->addWidget(direction_V_label_0,   0,11,1,1);
    info_panel_layout->addWidget(by_label_01,           0,12,1,1);
    info_panel_layout->addWidget(vol_width_field,       0,13,1,2);
    info_panel_layout->addWidget(direction_H_label_0,   0,15,1,1);
    info_panel_layout->addWidget(by_label_02,           0,16,1,1);
    info_panel_layout->addWidget(vol_depth_field,       0,17,1,2);
    info_panel_layout->addWidget(direction_D_label_0,   0,19,1,1);
    info_panel_layout->addWidget(volume_stacks_label,   1,0,1,9);
    info_panel_layout->addWidget(nrows_field,           1,9,1,2);
    info_panel_layout->addWidget(direction_V_label_1,   1,11,1,1);
    info_panel_layout->addWidget(by_label_1,            1,12,1,1);
    info_panel_layout->addWidget(ncols_field,           1,13,1,2);
    info_panel_layout->addWidget(direction_H_label_1,   1,15,1,1);
    info_panel_layout->addWidget(stacks_dims_label,     2,0,1,9);
    info_panel_layout->addWidget(stack_height_field,    2,9,1,2);
    info_panel_layout->addWidget(direction_V_label_2,   2,11,1,1);
    info_panel_layout->addWidget(by_label_2,            2,12,1,1);
    info_panel_layout->addWidget(stack_width_field,     2,13,1,2);
    info_panel_layout->addWidget(direction_H_label_2,   2,15,1,1);
    info_panel_layout->addWidget(by_label_3,            2,16,1,1);
    info_panel_layout->addWidget(stack_depth_field,     2,17,1,2);
    info_panel_layout->addWidget(direction_D_label_2,   2,19,1,1);
    info_panel_layout->addWidget(voxel_dims_label,      3,0,1,9);
    info_panel_layout->addWidget(vxl_V_field,           3,9,1,2);
    info_panel_layout->addWidget(direction_V_label_3,   3,11,1,1);
    info_panel_layout->addWidget(by_label_4,            3,12,1,1);
    info_panel_layout->addWidget(vxl_H_field,           3,13,1,2);
    info_panel_layout->addWidget(direction_H_label_3,   3,15,1,1);
    info_panel_layout->addWidget(by_label_5,            3,16,1,1);
    info_panel_layout->addWidget(vxl_D_field,           3,17,1,2);
    info_panel_layout->addWidget(direction_D_label_3,   3,19,1,1);
    info_panel_layout->addWidget(origin_label,          4,0,1,9);
    info_panel_layout->addWidget(org_V_field,           4,9,1,2);
    info_panel_layout->addWidget(direction_V_label_4,   4,11,1,1);
    info_panel_layout->addWidget(org_H_field,           4,13,1,2);
    info_panel_layout->addWidget(direction_H_label_4,   4,15,1,1);
    info_panel_layout->addWidget(org_D_field,           4,17,1,2);
    info_panel_layout->addWidget(direction_D_label_4,   4,19,1,1);
    info_panel->setLayout(info_panel_layout);
    info_panel->setStyle(new QWindowsStyle());

    //subvolume selection widgets
    QGridLayout* subvol_panel_layout = new QGridLayout();
    subvol_panel_layout->addWidget(direction_V_label_5, 0, 0, 1, 1);
    subvol_panel_layout->addWidget(V0_sbox,             0, 1, 1, 2);
    subvol_panel_layout->addWidget(to_label_1,          0, 3, 1, 1);
    subvol_panel_layout->addWidget(V1_sbox,             0, 4, 1, 2);
    subvol_panel_layout->addWidget(direction_H_label_5, 0, 6, 1, 1);
    subvol_panel_layout->addWidget(H0_sbox,             0, 7, 1, 2);
    subvol_panel_layout->addWidget(to_label_2,          0, 9, 1, 1);
    subvol_panel_layout->addWidget(H1_sbox,             0, 10, 1, 2);
    subvol_panel_layout->addWidget(direction_D_label_5, 0, 12, 1, 1);
    subvol_panel_layout->addWidget(D0_sbox,             0, 13, 1, 2);
    subvol_panel_layout->addWidget(to_label_3,          0, 15, 1, 1);
    subvol_panel_layout->addWidget(D1_sbox,             0, 16, 1, 2);
    subvol_panel_layout->addWidget(loadButton,          1, 0, 1, 18);
    subvol_panel->setLayout(subvol_panel_layout);
    subvol_panel->setStyle(new QWindowsStyle());

    //multiresolution mode widgets
    QGridLayout* multiresModePanelLayout= new QGridLayout();
    multiresModePanelLayout->addWidget(new QLabel("Volume map size limit:"),    1,0,1,4);
    multiresModePanelLayout->addWidget(volMapMaxSizeSBox,                       1,5,1,2);
    multiresModePanelLayout->addWidget(new QLabel("MVoxels"),                   1,7,1,3);
    multiresModePanelLayout->addWidget(subvol_dims_label,                       2, 0, 1, 4);
    multiresModePanelLayout->addWidget(Vdim_sbox,                               2, 5, 1, 2);
    multiresModePanelLayout->addWidget(direction_V_label_6,                     2, 7, 1, 1);
    multiresModePanelLayout->addWidget(by_label_6,                              2, 8, 1, 1);
    multiresModePanelLayout->addWidget(Hdim_sbox,                               2, 9, 1, 2);
    multiresModePanelLayout->addWidget(direction_H_label_6,                     2, 11, 1, 1);
    multiresModePanelLayout->addWidget(by_label_7,                              2, 12, 1, 1);
    multiresModePanelLayout->addWidget(Ddim_sbox,                               2, 13, 1, 2);
    multiresModePanelLayout->addWidget(direction_D_label_6,                     2, 15, 1, 1);
    multires_panel->setLayout(multiresModePanelLayout);
    multires_panel->setStyle(new QWindowsStyle());

    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(helpbox);
    layout->addWidget(import_form);    
    layout->addWidget(multires_panel);
    layout->addWidget(info_panel);
    layout->addWidget(subvol_panel);
    layout->addWidget(statusBar);
    layout->addWidget(progressBar);
    setLayout(layout);
    setWindowTitle(tr("TeraManager plugin"));    
    this->setFont(tinyFont);
    subvol_panel->setEnabled(false);

    // signals and slots
    connect(voldir_button, SIGNAL(clicked()), this, SLOT(voldir_button_clicked()));
    connect(loadButton, SIGNAL(clicked()), this, SLOT(loadButtonClicked()));
    connect(CImport::instance(), SIGNAL(sendOperationOutcome(MyException*, Image4DSimple*)), this, SLOT(import_done(MyException*, Image4DSimple*)), Qt::QueuedConnection);
    connect(CVolume::instance(), SIGNAL(sendOperationOutcome(MyException*)), this, SLOT(loading_done(MyException*)), Qt::QueuedConnection);
    connect(enableMultiresMode, SIGNAL(stateChanged(int)), this, SLOT(mode3D_checkbox_changed(int)), Qt::QueuedConnection);
    resetGUI();

    //center on screen and set always on top
    setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->size(),qApp->desktop()->availableGeometry()));
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->setMaximumSize(this->minimumWidth(), this->minimumHeight());
}

PMain::~PMain()
{
    #ifdef TMP_DEBUG
    printf("TeraManager plugin [thread %d] >> PMain destroyed\n", this->thread()->currentThreadId());
    #endif
    if(treeviewWidget)
        treeviewWidget->close();
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
    this->multires_panel->setEnabled(enableMultiresMode->isChecked());
}

//called when loadButton has been clicked
void PMain::loadButtonClicked()
{ 
    #ifdef TMP_DEBUG
    printf("TeraManager plugin [thread %d] >> PMain loadButtonClicked() called\n", this->thread()->currentThreadId());
    #endif

    try
    {
        //first checking that a volume has been properly imported
        if(!CImport::instance()->getVolume())
            throw MyException("A volume should be imported first.");

        //disabling import form and enabling progress bar animation and tab wait animation
        progressBar->setEnabled(true);
        progressBar->setMinimum(0);
        progressBar->setMaximum(0);
        loadButton->setEnabled(false);
        subvol_panel->setEnabled(false);
        statusBar->showMessage("Loading selected subvolume volume...");

        //starting operation
        CVolume::instance()->setVOI(V0_sbox->value()-1, V1_sbox->value()-1,H0_sbox->value()-1, H1_sbox->value()-1, D0_sbox->value()-1, D1_sbox->value()-1);        
        CVolume::instance()->setShowInNewWindow(true);
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
* Called when "voldir_button" has been clicked.
* Opens QFileDialog to select volume's path, which is copied into "path_field".
***********************************************************************************/
void PMain::voldir_button_clicked()
{
    #ifdef TMP_DEBUG
    printf("teramanager plugin [thread %d] >> PMain voldir_button_clicked() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        //obtaining volume's directory
        path_field->setText(QFileDialog::getExistingDirectory(0, QObject::tr("Select volume's directory"), QDir::currentPath()));

        //first checking that no volume has imported yet
        if(CImport::instance()->getVolume())
            throw MyException("A volume has been already imported! Please restart the plugin to import another volume.");

        //checking that the inserted path exists
        string import_path = path_field->text().toStdString();
        if(!StackedVolume::fileExists(import_path.c_str()))
            throw MyException("The inserted path does not exist!");

        //check if additional informations are required
        string mdata_fpath = import_path;
        mdata_fpath.append("/");
        mdata_fpath.append(IM_METADATA_FILE_NAME);
        string vmap_fpath = import_path;
        vmap_fpath.append("/");
        vmap_fpath.append(TMP_VMAP_FNAME);
        if(!StackedVolume::fileExists(mdata_fpath.c_str()) || reimport_checkbox->isChecked() ||
          (!StackedVolume::fileExists(vmap_fpath.c_str()) && enableMultiresMode->isChecked()))
            PDialogImport::instance()->exec();
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


/*********************************************************************************
* Called by <CImport> when the associated operation has been performed.
* If an exception has occurred in the <CImport> thread,  it is propagated and man-
* aged in the current thread (ex != 0). Otherwise, volume information are imported
* in the GUI by the <StackedVolume> handle of <CImport>.
**********************************************************************************/
void PMain::import_done(MyException *ex, Image4DSimple* vmap_image)
{
    #ifdef TMP_DEBUG
    printf("teramanager plugin [thread %d] >> PMain import_done(%s) launched\n", this->thread()->currentThreadId(), (ex? "ex" : "NULL"));
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
        StackedVolume* volume = CImport::instance()->getVolume();
        info_panel->setEnabled(true);
        vol_height_field->setText(QString::number(CImport::instance()->getVolume()->getDIM_V()));
        vol_width_field->setText(QString::number(CImport::instance()->getVolume()->getDIM_H()));
        vol_depth_field->setText(QString::number(CImport::instance()->getVolume()->getDIM_D()));
        nrows_field->setText(QString::number(CImport::instance()->getVolume()->getN_ROWS()));
        ncols_field->setText(QString::number(CImport::instance()->getVolume()->getN_COLS()));
        stack_height_field->setText(QString::number(CImport::instance()->getVolume()->getStacksHeight()));
        stack_width_field->setText(QString::number(CImport::instance()->getVolume()->getStacksWidth()));
        stack_depth_field->setText(QString::number(CImport::instance()->getVolume()->getDIM_D()));
        vxl_V_field->setText(QString::number(CImport::instance()->getVolume()->getVXL_V(), 'f', 2));
        vxl_H_field->setText(QString::number(CImport::instance()->getVolume()->getVXL_H(), 'f', 2));
        vxl_D_field->setText(QString::number(CImport::instance()->getVolume()->getVXL_D(), 'f', 2));
        org_V_field->setText(QString::number(CImport::instance()->getVolume()->getORG_V(), 'f', 2));
        org_H_field->setText(QString::number(CImport::instance()->getVolume()->getORG_H(), 'f', 2));
        org_D_field->setText(QString::number(CImport::instance()->getVolume()->getORG_D(), 'f', 2));

        //and settings subvol widgets limits
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

        //if vmap_image is available, showing it in a 3D renderer
        if(vmap_image)
        {
            v3dhandle new_win = V3D_env->newImageWindow(vmap_image->getFileName());
            V3D_env->setImage(new_win, vmap_image);
            V3D_env->open3DWindow(new_win);
            treeviewWidget = (XFormWidget*)new_win;
            treeviewWidget->setVisible(false);          //hiding the treeview window

            //installing event filter on 3D renderer
            View3DControl *view3DControl =  V3D_env->getView3DControl(new_win);
            view3DWidget = (V3dR_GLWidget*)view3DControl;
            view3DWidget->installEventFilter(this);

            //registering slots for 3D renderer signals
            connect(view3DWidget, SIGNAL(changeXCut0(int)), this, SLOT(Vaa3D_changeXCut0(int)));
            connect(view3DWidget, SIGNAL(changeXCut1(int)), this, SLOT(Vaa3D_changeXCut1(int)));
            connect(view3DWidget, SIGNAL(changeYCut0(int)), this, SLOT(Vaa3D_changeYCut0(int)));
            connect(view3DWidget, SIGNAL(changeYCut1(int)), this, SLOT(Vaa3D_changeYCut1(int)));
            connect(view3DWidget, SIGNAL(changeZCut0(int)), this, SLOT(Vaa3D_changeZCut0(int)));
            connect(view3DWidget, SIGNAL(changeZCut1(int)), this, SLOT(Vaa3D_changeZCut1(int)));
        }
    }

    //resetting some widgets
    resetGUI();
}

/**********************************************************************************
* Called by <CVolume> when the associated operation has been performed.
* If an exception has occurred in the <CVolume> thread, it is propagated and
* managed in the current thread (ex != 0).
***********************************************************************************/
void PMain::loading_done(MyException *ex)
{
    #ifdef TMP_DEBUG
    printf("TeraManager plugin [thread %d] >> PMain loading_done(%s) launched\n", this->thread()->currentThreadId(), (ex? "ex" : "NULL"));
    #endif

    CVolume* cVolume = CVolume::instance();

    //if an exception has occurred, showing a message error
    if(ex)
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
    else if(!enableMultiresMode->isChecked())
    {
        Image4DSimple* img = new Image4DSimple();
        img->setFileName(CImport::instance()->getVolume()->getSTACKS_DIR());
        img->setData(cVolume->getVOI_Data(), cVolume->getH1()-cVolume->getH0(),
                     cVolume->getV1()-cVolume->getV0(), cVolume->getD1()-cVolume->getD0(), 1, V3D_UINT8);
        v3dhandle new_win = V3D_env->newImageWindow(img->getFileName());
        V3D_env->setImage(new_win, img);
    }
    else if(view3DWidget)
    {
        if(CVolume::instance()->getShowInNewWindow())
        {
            char window_name[IM_STATIC_STRINGS_SIZE];
            sprintf(window_name, "subvol_X[%d-%d]_Y[%d-%d]_Z[%d-%d]", cVolume->getH0(), cVolume->getH1(),
                                                                      cVolume->getV0(), cVolume->getV1(),
                                                                      cVolume->getD0(), cVolume->getD1());
            Image4DSimple* img = new Image4DSimple();
            img->setData(cVolume->getVOI_Data(), cVolume->getH1()-cVolume->getH0(),
                         cVolume->getV1()-cVolume->getV0(), cVolume->getD1()-cVolume->getD0(), 1, V3D_UINT8);
            v3dhandle new_win = V3D_env->newImageWindow(window_name);
            V3D_env->setImage(new_win, img);
            V3D_env->open3DWindow(new_win);
            cVolume->resetVOI_Data();
            view3DWidget->setCursor(Qt::ArrowCursor);
        }
        else
        {
            /* Updating renderer content with the loaded subvolume */
            view3DWidget->getiDrawExternalParameter()->image4d->setRawDataPointerToNull();
            view3DWidget->getiDrawExternalParameter()->image4d->setData(cVolume->getVOI_Data(),
                                                                        cVolume->getH1()-cVolume->getH0(),
                                                                        cVolume->getV1()-cVolume->getV0(),
                                                                        cVolume->getD1()-cVolume->getD0(), 1, V3D_UINT8);
            view3DWidget->updateImageData();
            view3DWidget->setCursor(Qt::ArrowCursor);
            view3DWidget->setZoom(view3DWidget->zoom()/CImport::instance()->getMapZoominRatio());
        }
    }

    //resetting some widgets
    resetGUI();
    subvol_panel->setEnabled(true);
    loadButton->setEnabled(true);
}

//overrides closeEvent method of QWidget
void PMain::closeEvent(QCloseEvent *evt)
{
    if(progressBar->isEnabled() && QMessageBox::information(this, "Warning", "An operation is still in progress. Terminating it can be unsafe and cause Vaa3D to crash. \n"
                                                                    "\nPlease save your data first.", "Close TeraManager plugin", "Cancel"))
    {
        evt->ignore();
    }
    else
    {
        evt->accept();
        PMain::uninstance();
    }
}

/**********************************************************************************
* Filters events generated by the 3D rendering window
* We're interested to intercept these events to provide many useful ways to explore
* the 3D volume at different resolutions without changing Vaa3D code.
***********************************************************************************/
bool PMain::eventFilter(QObject *object, QEvent *event)
{
    try
    {
        if (object == view3DWidget && event->type() == QEvent::Wheel)
        {
            LandmarkList markers =  V3D_env->getLandmark(treeviewWidget);

            if(view3DWidget->zoom() > 30 && !CVolume::instance()->getVOI_Data() && markers.size() == 1)
            {
                printf("Zooming-in at %.0f %.0f %.0f\n", markers.first().y, markers.first().x, markers.first().z);
                view3DWidget->setCursor(Qt::WaitCursor);
                progressBar->setEnabled(true);
                progressBar->setMinimum(0);
                progressBar->setMaximum(0);
                loadButton->setEnabled(false);
                import_form->setEnabled(false);
                statusBar->showMessage("Zooming in to the highest resolution...");
                float zratio = CImport::instance()->getMapZoominRatio();
                CVolume::instance()->setVOI(markers.first().y*zratio-Vdim_sbox->value()/2, markers.first().y*zratio+Vdim_sbox->value()/2,
                                                   markers.first().x*zratio-Hdim_sbox->value()/2, markers.first().x*zratio+Hdim_sbox->value()/2,
                                                   markers.first().z*zratio-Ddim_sbox->value()/2, markers.first().z*zratio+Ddim_sbox->value()/2);
                markers.clear();
                V3D_env->setLandmark(treeviewWidget, markers);
                view3DWidget->getRenderer()->updateLandmark();
                CVolume::instance()->setShowInNewWindow(false);
                CVolume::instance()->start();
            }
            else if(view3DWidget->zoom() < 0 && CVolume::instance()->getVOI_Data())
            {
                view3DWidget->setCursor(Qt::WaitCursor);
                view3DWidget->getiDrawExternalParameter()->image4d->setRawDataPointerToNull();
                view3DWidget->getiDrawExternalParameter()->image4d->setData(CImport::instance()->getVMap(),
                                                                            CImport::instance()->getVMapWidth(),
                                                                            CImport::instance()->getVMapHeight(),
                                                                            CImport::instance()->getVMapDepth(), 1, V3D_UINT8);
                view3DWidget->updateImageData();
                CVolume::instance()->reset();
                view3DWidget->setCursor(Qt::ArrowCursor);
                view3DWidget->getView3DControl()->setZoom(CImport::instance()->getMapZoominRatio()*10);
            }
        }
        /*else if(event->type() == QMouseEvent::MouseButtonPress)
        {
            printf("Intercepted MouseButtonPress event at %d,%d\n", ((QMouseEvent*)event)->x(), ((QMouseEvent*)event)->y());
        }*/
        return false;
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        return false;
    }
}

/**********************************************************************************
* Linked to volume cut scrollbars of Vaa3D widget containing the 3D renderer.
* This implements the syncronization Vaa3D-->TeraManager of subvolume selection.
***********************************************************************************/
void PMain::Vaa3D_changeYCut0(int s)
{V0_sbox->setValue(s*(CImport::instance()->getVolume()->getDIM_V()-1.0)/(CImport::instance()->getVMapHeight()-1)+1);}
void PMain::Vaa3D_changeYCut1(int s)
{V1_sbox->setValue(s*(CImport::instance()->getVolume()->getDIM_V()-1.0)/(CImport::instance()->getVMapHeight()-1)+1);}
void PMain::Vaa3D_changeXCut0(int s)
{H0_sbox->setValue(s*(CImport::instance()->getVolume()->getDIM_H()-1.0)/(CImport::instance()->getVMapWidth()-1)+1);}
void PMain::Vaa3D_changeXCut1(int s)
{H1_sbox->setValue(s*(CImport::instance()->getVolume()->getDIM_H()-1.0)/(CImport::instance()->getVMapWidth()-1)+1);}
void PMain::Vaa3D_changeZCut0(int s)
{D0_sbox->setValue(s*(CImport::instance()->getVolume()->getDIM_D()-1.0)/(CImport::instance()->getVMapDepth()-1)+1);}
void PMain::Vaa3D_changeZCut1(int s)
{D1_sbox->setValue(s*(CImport::instance()->getVolume()->getDIM_D()-1.0)/(CImport::instance()->getVMapDepth()-1)+1);}

/**********************************************************************************
* Linked to rightStrokeROI and rightClickROI right-menu entries of the 3D renderer.
* This implements the selection of a ROI in the 3D renderer.
***********************************************************************************/
void PMain::Vaa3D_selectedROI()
{
    #ifdef TMP_DEBUG
    printf("TeraManager plugin [thread %d] >> PMain Vaa3D_selectedROI() called\n", this->thread()->currentThreadId());
    #endif

    if(view3DWidget)
    {
        float zratio = CImport::instance()->getMapZoominRatio();
        v3d_imaging_paras* roi = (v3d_imaging_paras*) view3DWidget->getiDrawExternalParameter()->image4d->getCustomStructPointer();
        view3DWidget->setCursor(Qt::WaitCursor);
        progressBar->setEnabled(true);
        progressBar->setMinimum(0);
        progressBar->setMaximum(0);
        loadButton->setEnabled(false);
        import_form->setEnabled(false);
        statusBar->showMessage("Zooming in to the highest resolution...");
        CVolume::instance()->setVOI(roi->ys*zratio, roi->ye*zratio, roi->ys*zratio, roi->ye*zratio, roi->zs*zratio, roi->ze*zratio);
        CVolume::instance()->setShowInNewWindow(true);
        CVolume::instance()->start();
    }
}
