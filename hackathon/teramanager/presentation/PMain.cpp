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
#include "control/CLoadSubvolume.h"
#include "PDialogImport.h"

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
    CLoadSubvolume::uninstance();
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

    //help box
    helpbox = new QLabel("<html><table><tr style=\"vertical-align: middle;\"><td><img src=\":/icons/help.png\"></td>"
                          "<td><p style=\"text-align:justify; margin-left:10px;\"> This experimental tools enables the visualization of "
                          "selectable portions of <b>teravoxel-sized datasets</b> with limited memory usage by exploiting the multi-stack "
                          "format, where provided.</p> </td></tr></table> </html>");
    helpbox->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); margin-top:10px; margin-bottom:10px; padding-top:10px; padding-bottom:10px;");
    helpbox->setWordWrap(true);
    helpbox->setFixedHeight(100);

    //import form widgets
    import_form = new QGroupBox("Import form");
    path_field    = new QLineEdit("Enter the volume's directory");
    path_field->setMinimumWidth(200);
    voldir_button       = new QPushButton("Browse for dir...");
    reimport_checkbox = new QCheckBox("Re-import (check it to overwrite previous imports)");
    generate_vmap = new QCheckBox("Enable 3D mode (uncheck it if lower resolutions are not available)");
    generate_vmap->setChecked(true);

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
    vol_height_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    vol_width_field = new QLabel();
    vol_width_field->setAlignment(Qt::AlignCenter);
    vol_width_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    vol_depth_field = new QLabel();
    vol_depth_field->setAlignment(Qt::AlignCenter);
    vol_depth_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    volume_stacks_label = new QLabel("Number of stacks:");
    direction_V_label_1 = new QLabel("(V)");
    direction_H_label_1 = new QLabel("(H)");
    by_label_1 = new QLabel("x");
    nrows_field = new QLabel();
    nrows_field->setAlignment(Qt::AlignCenter);
    nrows_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    ncols_field = new QLabel();
    ncols_field->setAlignment(Qt::AlignCenter);
    ncols_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    stacks_dims_label = new QLabel("Stacks dimensions (voxels):");
    direction_V_label_2 = new QLabel("(V)");
    direction_H_label_2 = new QLabel("(H)");
    direction_D_label_2 = new QLabel("(D)");
    by_label_2 = new QLabel("x");
    by_label_3 = new QLabel("x");
    stack_height_field = new QLabel();
    stack_height_field->setAlignment(Qt::AlignCenter);
    stack_height_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    stack_width_field = new QLabel();
    stack_width_field->setAlignment(Qt::AlignCenter);
    stack_width_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    stack_depth_field = new QLabel();
    stack_depth_field->setAlignment(Qt::AlignCenter);
    stack_depth_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    voxel_dims_label = new QLabel("Voxel's dimensions (microns):");
    direction_V_label_3 = new QLabel("(V)");
    direction_H_label_3 = new QLabel("(H)");
    direction_D_label_3 = new QLabel("(D)");
    by_label_4 = new QLabel("x");
    by_label_5 = new QLabel("x");
    vxl_V_field = new QLabel();
    vxl_V_field->setAlignment(Qt::AlignCenter);
    vxl_V_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    vxl_H_field = new QLabel();
    vxl_H_field->setAlignment(Qt::AlignCenter);
    vxl_H_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    vxl_D_field = new QLabel();
    vxl_D_field->setAlignment(Qt::AlignCenter);
    vxl_D_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    origin_label = new QLabel("Origin (millimeters):");
    direction_V_label_4 = new QLabel("(V)");
    direction_H_label_4 = new QLabel("(H)");
    direction_D_label_4 = new QLabel("(D)");
    org_V_field = new QLabel();
    org_V_field->setAlignment(Qt::AlignCenter);
    org_V_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    org_H_field = new QLabel();
    org_H_field->setAlignment(Qt::AlignCenter);
    org_H_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    org_D_field = new QLabel();
    org_D_field->setAlignment(Qt::AlignCenter);
    org_D_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");

    //other widgets
    volumeportion_label = new QLabel("Subvolume to be loaded:");
    V0_sbox = new QSpinBox();
    V0_sbox->setAlignment(Qt::AlignCenter);
    V0_sbox->setMinimumWidth(80);
    V1_sbox = new QSpinBox();
    V1_sbox->setAlignment(Qt::AlignCenter);
    V1_sbox->setMinimumWidth(80);
    H0_sbox = new QSpinBox();
    H0_sbox->setAlignment(Qt::AlignCenter);
    H0_sbox->setMinimumWidth(80);
    H1_sbox = new QSpinBox();
    H1_sbox->setAlignment(Qt::AlignCenter);
    H1_sbox->setMinimumWidth(80);
    D0_sbox = new QSpinBox();
    D0_sbox->setAlignment(Qt::AlignCenter);
    D0_sbox->setMinimumWidth(80);
    D1_sbox = new QSpinBox();
    D1_sbox->setAlignment(Qt::AlignCenter);
    D1_sbox->setMinimumWidth(80);
    to_label_1 = new QLabel("to");
    to_label_1->setAlignment(Qt::AlignCenter);
    to_label_2 = new QLabel("to");
    to_label_2->setAlignment(Qt::AlignCenter);
    to_label_3 = new QLabel("to");
    to_label_3->setAlignment(Qt::AlignCenter);
    direction_V_label_5 = new QLabel("V:");
    direction_H_label_5 = new QLabel("H:");
    direction_D_label_5 = new QLabel("D:");
    progressBar = new QProgressBar(this);
    loadButton = new QPushButton(this);
    loadButton->setIcon(QIcon(":/icons/load.png"));
    loadButton->setText("Load selected subvolume");
    loadButton->setMinimumHeight(50);
    loadButton->setIconSize(QSize(35,35));
    loadButton->setFont(QFont(loadButton->font().family(), loadButton->font().pointSize()+2));
    statusBar = new QStatusBar();
    statusBar->showMessage("Ready.");

    //****LAYOUT SECTIONS****
    //import form
    QWidget* container_tmp = new QWidget();
    QHBoxLayout* container_tmp_layout = new QHBoxLayout();
    container_tmp_layout->addWidget(path_field);
    container_tmp_layout->addWidget(voldir_button);
    container_tmp->setLayout(container_tmp_layout);
    QVBoxLayout* import_form_layout = new QVBoxLayout();
    import_form_layout->addWidget(container_tmp);
    import_form_layout->addWidget(reimport_checkbox);
    import_form_layout->addWidget(generate_vmap);
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
    subvol_panel = new QWidget();
    QGridLayout* subvol_panel_layout = new QGridLayout();
    subvol_panel_layout->addWidget(volumeportion_label, 0, 0, 1, 2);
    direction_V_label_5->setStyleSheet("margin-left: 15;");
    subvol_panel_layout->addWidget(direction_V_label_5, 0, 2, 1, 1);
    subvol_panel_layout->addWidget(V0_sbox,             0, 3, 1, 2);
    subvol_panel_layout->addWidget(to_label_1,          0, 5, 1, 1);
    subvol_panel_layout->addWidget(V1_sbox,             0, 6, 1, 2);
    direction_H_label_5->setStyleSheet("margin-left: 15;");
    subvol_panel_layout->addWidget(direction_H_label_5, 1, 2, 1, 1);
    subvol_panel_layout->addWidget(H0_sbox,             1, 3, 1, 2);
    subvol_panel_layout->addWidget(to_label_2,          1, 5, 1, 1);
    subvol_panel_layout->addWidget(H1_sbox,             1, 6, 1, 2);
    direction_D_label_5->setStyleSheet("margin-left: 15;");
    subvol_panel_layout->addWidget(direction_D_label_5, 2, 2, 1, 1);
    subvol_panel_layout->addWidget(D0_sbox,             2, 3, 1, 2);
    subvol_panel_layout->addWidget(to_label_3,          2, 5, 1, 1);
    subvol_panel_layout->addWidget(D1_sbox,             2, 6, 1, 2);
    subvol_panel->setLayout(subvol_panel_layout);

    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(helpbox);
    layout->addWidget(import_form);
    layout->addWidget(info_panel);
    layout->addWidget(subvol_panel);
    layout->addWidget(loadButton);
    layout->addWidget(statusBar);
    layout->addWidget(progressBar);
    setLayout(layout);
    setWindowTitle(tr("TeraManager plugin"));
    subvol_panel->setEnabled(false);
    loadButton->setEnabled(false);

    // signals and slots
    connect(voldir_button, SIGNAL(clicked()), this, SLOT(voldir_button_clicked()));
    connect(loadButton, SIGNAL(clicked()), this, SLOT(loadButtonClicked()));
    connect(CImport::instance(), SIGNAL(sendOperationOutcome(MyException*, Image4DSimple*)), this, SLOT(import_done(MyException*, Image4DSimple*)), Qt::QueuedConnection);
    connect(CLoadSubvolume::instance(), SIGNAL(sendOperationOutcome(MyException*)), this, SLOT(loading_done(MyException*)), Qt::QueuedConnection);
    resetGUI();

    //center on screen and set always on top
    setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->size(),qApp->desktop()->availableGeometry()));
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}

PMain::~PMain()
{
    #ifdef TMP_DEBUG
    printf("TeraManager plugin [thread %d] >> PMain destroyed\n", this->thread()->currentThreadId());
    #endif
}

//reset GUI method
void PMain::resetGUI()
{
    progressBar->setEnabled(false);
    progressBar->setMaximum(1);         //needed to stop animation on some operating systems
    statusBar->clearMessage();
    statusBar->showMessage("Ready.");
}

//called when loadButton has been clicked
void PMain::loadButtonClicked()
{ 
    #ifdef TMP_DEBUG
    printf("TeraManager plugin [thread %d] >> loadButtonClicked() called\n", this->thread()->currentThreadId());
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
        CLoadSubvolume::instance()->setVOI(V0_sbox->value(), V1_sbox->value(),H0_sbox->value(), H1_sbox->value(), D0_sbox->value(), D1_sbox->value());
        CLoadSubvolume::instance()->start();
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
    printf("teramanager plugin [thread %d] >> PDialogImport voldir_button_clicked() launched\n", this->thread()->currentThreadId());
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
          (!StackedVolume::fileExists(vmap_fpath.c_str()) && generate_vmap->isChecked()))
            PDialogImport::instance()->exec();
        CImport::instance()->setPath(import_path);
        CImport::instance()->setReimport(reimport_checkbox->isChecked());
        CImport::instance()->setGenerateMap(generate_vmap->isChecked());

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
        vxl_V_field->setText(QString::number(CImport::instance()->getVolume()->getVXL_V()));
        vxl_H_field->setText(QString::number(CImport::instance()->getVolume()->getVXL_H()));
        vxl_D_field->setText(QString::number(CImport::instance()->getVolume()->getVXL_D()));
        org_V_field->setText(QString::number(CImport::instance()->getVolume()->getORG_V()));
        org_H_field->setText(QString::number(CImport::instance()->getVolume()->getORG_H()));
        org_D_field->setText(QString::number(CImport::instance()->getVolume()->getORG_D()));

        //and settings subvol widgets limits
        V0_sbox->setMinimum(0);
        V0_sbox->setMaximum(volume->getDIM_V());
        V0_sbox->setValue(volume->getDIM_V()/2 - 256);
        V1_sbox->setMinimum(0);
        V1_sbox->setMaximum(volume->getDIM_V());
        V1_sbox->setValue(volume->getDIM_V()/2 + 256);
        H0_sbox->setMinimum(0);
        H0_sbox->setMaximum(volume->getDIM_H());
        H0_sbox->setValue(volume->getDIM_H()/2 - 256);
        H1_sbox->setMinimum(0);
        H1_sbox->setMaximum(volume->getDIM_H());
        H1_sbox->setValue(volume->getDIM_H()/2 + 256);
        D0_sbox->setMinimum(0);
        D0_sbox->setMaximum(volume->getDIM_D());
        D0_sbox->setValue(volume->getDIM_D()/2 - 256);
        D1_sbox->setMinimum(0);
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

            //installing event filter on 3D renderer
            View3DControl *view3DControl =  V3D_env->getView3DControl(new_win);
            view3DWidget = (V3dR_GLWidget*)view3DControl;
            view3DWidget->installEventFilter(this);
        }
    }

    //resetting some widgets
    resetGUI();
    CImport::instance()->reset();
}

/**********************************************************************************
* Called by <CLoadSubvolume> when the associated operation has been performed.
* If an exception has occurred in the <CLoadSubvolume> thread, it is propagated and
* managed in the current thread (ex != 0).
***********************************************************************************/
void PMain::loading_done(MyException *ex)
{
    #ifdef TMP_DEBUG
    printf("TeraManager plugin [thread %d] >> PMain loading_done(%s) launched\n", this->thread()->currentThreadId(), (ex? "ex" : "NULL"));
    #endif

    CLoadSubvolume* cLoadSubvolume = CLoadSubvolume::instance();

    //if an exception has occurred, showing a message error
    if(ex)
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
    else if(!generate_vmap->isChecked())
    {
        Image4DSimple* img = new Image4DSimple();
        img->setFileName(CImport::instance()->getVolume()->getSTACKS_DIR());
        img->setData(cLoadSubvolume->getVOI_Data(), cLoadSubvolume->getH1()-cLoadSubvolume->getH0(),
                     cLoadSubvolume->getV1()-cLoadSubvolume->getV0(), cLoadSubvolume->getD1()-cLoadSubvolume->getD0(), 1, V3D_UINT8);
        v3dhandle new_win = V3D_env->newImageWindow(img->getFileName());
        V3D_env->setImage(new_win, img);
    }
    else if(view3DWidget)
    {
        /* Updating renderer content with the loaded subvolume */

        //---- THE SAFE WAY, BUT a confirm dialog is shown and existing data is released ------
        /*view3DWidget->getiDrawExternalParameter()->image4d->setData(cLoadSubvolume->getVOI_Data(),
                                                                    cLoadSubvolume->getH1()-cLoadSubvolume->getH0(),
                                                                    cLoadSubvolume->getV1()-cLoadSubvolume->getV0(),
                                                                    cLoadSubvolume->getD1()-cLoadSubvolume->getD0(), 1, V3D_UINT8);

        view3DWidget->reloadData();*/

        //----- ALTERNATIVE WAY: no confirm dialog, no memory release, BUT is it safe? ------
        view3DWidget->getiDrawExternalParameter()->image4d->setRawDataPointerToNull();
        view3DWidget->getiDrawExternalParameter()->image4d->setData(cLoadSubvolume->getVOI_Data(),
                                                                    cLoadSubvolume->getH1()-cLoadSubvolume->getH0(),
                                                                    cLoadSubvolume->getV1()-cLoadSubvolume->getV0(),
                                                                    cLoadSubvolume->getD1()-cLoadSubvolume->getD0(), 1, V3D_UINT8);
        view3DWidget->getRenderer()->setupData(view3DWidget->getiDrawExternalParameter());
        view3DWidget->getRenderer()->initialize(1);
        view3DWidget->updateTool();
        view3DWidget->setCursor(Qt::ArrowCursor);
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

//filters events generated by the 3D rendering window
bool PMain::eventFilter(QObject *object, QEvent *event)
{
    //we're only interest to mouse wheel event, which generates a change in the zoom
    if (event->type() == QEvent::Wheel)
    {
        QWheelEvent *mouseEvent = static_cast<QWheelEvent *>(event);

        printf("zoom = %d\n", view3DWidget->zoom());
        if(view3DWidget->zoom() > 20 && !CLoadSubvolume::instance()->getVOI_Data())
        {
            view3DWidget->setCursor(Qt::WaitCursor);
            progressBar->setEnabled(true);
            progressBar->setMinimum(0);
            progressBar->setMaximum(0);
            loadButton->setEnabled(false);
            import_form->setEnabled(false);
            statusBar->showMessage("Zooming in to the highest resolution...");
            CLoadSubvolume::instance()->setVOI(500, 800, 300, 600, 100, 300);
            CLoadSubvolume::instance()->start();
        }
        else if(view3DWidget->zoom() < 0 && CLoadSubvolume::instance()->getVOI_Data())
        {
            //----- ALTERNATIVE WAY: no confirm dialog, no memory release, BUT is it safe? ------
            view3DWidget->setCursor(Qt::WaitCursor);
            view3DWidget->getiDrawExternalParameter()->image4d->setRawDataPointerToNull();
            view3DWidget->getiDrawExternalParameter()->image4d->setData(CImport::instance()->getVMapData(),
                                                                        CImport::instance()->getVMapWidth(),
                                                                        CImport::instance()->getVMapHeight(),
                                                                        CImport::instance()->getVMapDepth(), 1, V3D_UINT8);
            view3DWidget->getRenderer()->setupData(view3DWidget->getiDrawExternalParameter());
            view3DWidget->getRenderer()->initialize(1);
            view3DWidget->updateTool();
            CLoadSubvolume::instance()->reset();
            view3DWidget->setCursor(Qt::ArrowCursor);
        }

        //uint8* mydata = new uint8[100*100*100];
        /*uint8* mydata = CImport::instance()->getVolume()->loadSubvolume_to_UINT8(400,600,400,600,200,400);
        view3DWidget->getiDrawExternalParameter()->image4d->setData(mydata, 200, 200, 200, 1, V3D_UINT8);
        view3DWidget->reloadData();*/

        //trying to set my own data
        //view3DWidget->getRenderer()->cleanData();
        /*My4DImage* my4Dimg = new My4DImage();

        uint8* mydata = new uint8[100*100*100];
        my4Dimg->setData(mydata, 100, 100, 100, 1, V3D_UINT8);
        iDrawExternalParameter* data = new iDrawExternalParameter();
        data->image4d = my4Dimg;
        view3DWidget->getRenderer()->setupData(data);*/

        //view3DWidget->getRenderer()-

        /*if (mouseWheelEvent->)
        {
            // Special tab handling
            V3dR_GLWidget* view3DWidget = (V3dR_GLWidget*)view3DControl;
            return true;
        } else
            return false;*/
    }
    return false;
}
