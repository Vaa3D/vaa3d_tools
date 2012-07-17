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

#include "PTabImport.h"
#include "MyException.h"
#include "StackedVolume.h"
#include "PMain.h"
#include "control/CImport.h"
#include "control/CPreview.h"
#include "PTabDisplComp.h"
#include "PTabDisplProj.h"
#include "PTabDisplThresh.h"
#include "PTabMergeTiles.h"
#include "PTabPlaceTiles.h"

using namespace terastitcher;

/*********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "istance(...)"
**********************************************************************************/
PTabImport* PTabImport::uniqueInstance = NULL;
void PTabImport::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

PTabImport::PTabImport(QMyTabWidget* _container, int _tab_index) : QWidget(), container(_container), tab_index(_tab_index)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport created\n", this->thread()->currentThreadId());
    #endif

    //help box
    helpbox = new QLabel("<html><table><tr style=\"vertical-align: middle;\"><td><img src=\":/icons/help.png\"></td>"
                                 "<td><p style=\"text-align:justify; margin-left:10px;\"> This step performs <b>volume's import into an organized metadata structure</b> suited for further processing. "
                                 "No data will be loaded in this step. However, importing big volumes could take up to a few minutes to read all slices filenames. <br><br>"
                         "If this is the first time the volume the volume is going to be imported, you have to fill all the fields. Please pay attention that the minus \"-\" sign before an axis is interpreted as a rotation by 180 degrees.</p> </td></tr></table> </html>");
    helpbox->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); margin-top:10px; margin-bottom:10px; padding-top:10px; padding-bottom:10px;");
    helpbox->setWordWrap(true);
    helpbox->setFixedHeight(180);

    //import form widgets
    import_form = new QGroupBox("Import form");
    path_field    = new QLineEdit("Enter the volume's directory or a valid TeraStitcher XML project file path");
    path_field->setMinimumWidth(500);
    voldir_button       = new QPushButton("Browse for dir...");
    projfile_button     = new QPushButton("Browse for XML...");
    reimport_checkbox = new QCheckBox("Re-import (check this to overwrite previous imports)");
    first_direction_label = new QLabel("First direction");
    second_direction_label = new QLabel("Second direction");
    third_direction_label = new QLabel("Third direction");
    axes_label = new QLabel("Axes (1 = Vertical, 2 = Horizontal, 3 = Depth)");
    voxels_dims_label = new QLabel("Voxel's dimensions (micrometers)");
    QRegExp axs_regexp("^-?[123]$");
    QRegExp vxl_regexp("^[0-9]+\\.?[0-9]*$");
    axs1_field = new QLineEdit();
    axs1_field->setAlignment(Qt::AlignCenter);
    axs1_field->setValidator(new QRegExpValidator(axs_regexp, axs1_field));
    axs2_field = new QLineEdit();
    axs2_field->setAlignment(Qt::AlignCenter);
    axs2_field->setValidator(new QRegExpValidator(axs_regexp, axs2_field));
    axs3_field = new QLineEdit();
    axs3_field->setAlignment(Qt::AlignCenter);
    axs3_field->setValidator(new QRegExpValidator(axs_regexp, axs3_field));
    vxl1_field = new QLineEdit();
    vxl1_field->setAlignment(Qt::AlignCenter);
    vxl1_field->setValidator(new QRegExpValidator(vxl_regexp, vxl1_field));
    vxl2_field = new QLineEdit();
    vxl2_field->setAlignment(Qt::AlignCenter);
    vxl2_field->setValidator(new QRegExpValidator(vxl_regexp, vxl2_field));
    vxl3_field = new QLineEdit();
    vxl3_field->setAlignment(Qt::AlignCenter);
    vxl3_field->setValidator(new QRegExpValidator(vxl_regexp, vxl3_field));

    //info panel widgets
    info_panel = new QGroupBox("Volume's informations");
    info_panel->setEnabled(false);
    volumedir_label = new QLabel("Volume's directory");
    volumedir_field = new QLabel();
    volumedir_field->setAlignment(Qt::AlignCenter);
    volumedir_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    volume_dims_label = new QLabel("Volume's dimensions (number of stacks):");
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
    stacks_overlap_label = new QLabel("Stacks overlap (voxels):");
    direction_V_label_5 = new QLabel("(V)");
    direction_H_label_5 = new QLabel("(H)");
    ovp_V_field = new QLabel();
    ovp_V_field->setAlignment(Qt::AlignCenter);
    ovp_V_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    ovp_H_field = new QLabel();
    ovp_H_field->setAlignment(Qt::AlignCenter);
    ovp_H_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");


    //preview panel
    preview_panel = new QGroupBox("Test motorized stages precision");
    preview_panel->setEnabled(false);
    preview_desc = new QLabel("In order to test if the volume has been properly imported and the precision of motorized stages, "
                              "you can select a slice to be stitched using nominal coordinates. The result will be shown into Vaa3D.");
    preview_desc->setWordWrap(true);
    slice_slider = new QSlider(Qt::Horizontal);
    slice_index_field = new QLabel();
    slice_index_field->setAlignment(Qt::AlignCenter);
    slice_index_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    preview_button = new QPushButton(this);
    preview_button->setIcon(QIcon(":/icons/preview.png"));
    preview_button->setText("Get slice");

    /*** LAYOUT SECTIONS ***/
    //import form
    QWidget* container_tmp = new QWidget();
    QHBoxLayout* container_tmp_layout = new QHBoxLayout();
    container_tmp_layout->addWidget(path_field);
    container_tmp_layout->addWidget(voldir_button);
    container_tmp_layout->addWidget(projfile_button);
    container_tmp->setLayout(container_tmp_layout);
    QWidget* container_tmp2 = new QWidget();
    QGridLayout* container_tmp2_layout = new QGridLayout();
    container_tmp2_layout->addWidget(first_direction_label, 0, 2, 1, 1, Qt::AlignHCenter);
    container_tmp2_layout->addWidget(second_direction_label, 0, 3, 1, 1, Qt::AlignHCenter);
    container_tmp2_layout->addWidget(third_direction_label, 0, 4, 1, 1, Qt::AlignHCenter);
    container_tmp2_layout->addWidget(axes_label, 1, 0, 1, 2);
    container_tmp2_layout->addWidget(axs1_field, 1, 2, 1, 1, Qt::AlignHCenter);
    container_tmp2_layout->addWidget(axs2_field, 1, 3, 1, 1, Qt::AlignHCenter);
    container_tmp2_layout->addWidget(axs3_field, 1, 4, 1, 1, Qt::AlignHCenter);
    container_tmp2_layout->addWidget(voxels_dims_label, 2, 0, 1, 2);
    container_tmp2_layout->addWidget(vxl1_field, 2, 2, 1, 1, Qt::AlignHCenter);
    container_tmp2_layout->addWidget(vxl2_field, 2, 3, 1, 1, Qt::AlignHCenter);
    container_tmp2_layout->addWidget(vxl3_field, 2, 4, 1, 1, Qt::AlignHCenter);
    container_tmp2->setLayout(container_tmp2_layout);
    QVBoxLayout* import_form_layout = new QVBoxLayout();
    import_form_layout->addWidget(container_tmp);    
    import_form_layout->addWidget(reimport_checkbox);
    import_form_layout->addWidget(container_tmp2);
    import_form->setLayout(import_form_layout);
    import_form->setStyle(new QWindowsStyle());

    //info panel
    QGridLayout* info_panel_layout = new QGridLayout();
    info_panel_layout->addWidget(volumedir_label,       0,0,1,9);
    info_panel_layout->addWidget(volumedir_field,       0,9,1,10);
    info_panel_layout->addWidget(volume_dims_label,     1,0,1,9);
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
    info_panel_layout->addWidget(stacks_overlap_label,  5,0,1,9);
    info_panel_layout->addWidget(ovp_V_field,           5,9,1,2);
    info_panel_layout->addWidget(direction_V_label_5,   5,11,1,1);
    info_panel_layout->addWidget(ovp_H_field,           5,13,1,2);
    info_panel_layout->addWidget(direction_H_label_5,   5,15,1,1);
    info_panel->setLayout(info_panel_layout);
    info_panel->setStyle(new QWindowsStyle());

    //preview panel
    QWidget* container_tmp3 = new QWidget();
    QGridLayout* container_tmp3_layout = new QGridLayout();
    container_tmp3_layout->addWidget(slice_slider, 0, 0, 1, 9);
    container_tmp3_layout->addWidget(slice_index_field, 0, 10, 1, 2);
    container_tmp3_layout->addWidget(preview_button, 0, 13, 1, 2);
    container_tmp3->setLayout(container_tmp3_layout);
    QVBoxLayout* preview_panel_layout = new QVBoxLayout();
    preview_panel_layout->addWidget(preview_desc);
    preview_panel_layout->addWidget(container_tmp3);
    preview_panel->setLayout(preview_panel_layout);
    preview_panel->setStyle(new QWindowsStyle());

    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(helpbox);
    layout->addWidget(import_form);
    layout->addWidget(info_panel);
    layout->addWidget(preview_panel);
    setLayout(layout);

    //wait animated GIF tab icon
    wait_movie = new QMovie(":/icons/wait.gif");
    wait_label = new QLabel(this);
    wait_label->setMovie(wait_movie);

    // signals and slots
    connect(voldir_button, SIGNAL(clicked()), this, SLOT(voldir_button_clicked()));
    connect(projfile_button, SIGNAL(clicked()), this, SLOT(projfile_button_clicked()));
    connect(slice_slider, SIGNAL(valueChanged(int)), this, SLOT(slice_slider_value_changed(int)));
    connect(preview_button, SIGNAL(clicked()), this, SLOT(preview_button_clicked()));
    connect(CImport::instance(), SIGNAL(sendOperationOutcome(MyException*)), this, SLOT(import_done(MyException*)), Qt::QueuedConnection);
    connect(CPreview::instance(), SIGNAL(sendOperationOutcome(MyException*,Image4DSimple*)), this, SLOT(preview_done(MyException*,Image4DSimple*)), Qt::QueuedConnection);
}


PTabImport::~PTabImport()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport destroyed\n", this->thread()->currentThreadId());
    #endif
}

/**********************************************************************************
* Called when "voldir_button" has been clicked.
* Opens QFileDialog to select volume's path, which is copied into "path_field".
***********************************************************************************/
void PTabImport::voldir_button_clicked()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport voldir_button_clicked() launched\n", this->thread()->currentThreadId());
    #endif

    //obtaining volume's directory
    path_field->setText(QFileDialog::getExistingDirectory(0, QObject::tr("Select volume's directory"), QDir::currentPath()));
}

/**********************************************************************************
* Called when "projfile_button" has been clicked.
* Opens QFileDialog to select project's XML path, which is copied into "path_field".
***********************************************************************************/
void PTabImport::projfile_button_clicked()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport projfile_button_clicked() launched\n", this->thread()->currentThreadId());
    #endif

    //obtaining TeraStitcher XML project file
    path_field->setText(QFileDialog::getOpenFileName(0, QObject::tr("Select TeraStitcher XML project file"), QDir::currentPath(), tr("XML files (*.xml *.XML)")));
}
/**********************************************************************************
* Called when "preview_button" has been clicked.
* Launches stitching of the selected slice. The result is displayed in Vaa3D.
***********************************************************************************/
void PTabImport::preview_button_clicked()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport preview_button_clicked() launched\n", this->thread()->currentThreadId());
    #endif

    //disabling preview panel and enabling progress bar animation and tab wait animation
    preview_panel->setEnabled(false);
    PMain::instance()->getProgressBar()->setEnabled(true);
    PMain::instance()->getProgressBar()->setMinimum(0);
    PMain::instance()->getProgressBar()->setMaximum(0);
    PMain::instance()->getStatusBar()->showMessage("Stitching slice...");
    PMain::instance()->setStartButtonEnabled(false);
    wait_movie->start();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, wait_label);

    //launching preview thread
    CPreview::instance()->setMembers(CImport::instance()->getVolume(), slice_slider->value());
    CPreview::instance()->start();

}

/**********************************************************************************
* Called when "slice_slider" value has changed.
* Updates value of "slice_index_field".
***********************************************************************************/
void PTabImport::slice_slider_value_changed(int new_val)
{
    #if TSP_DEBUG > 2
    printf("TeraStitcher plugin [thread %d] >> PTabImport slice_slider_value_changed() launched\n", this->thread()->currentThreadId());
    #endif
    slice_index_field->setText(QString::number(new_val));
}

/*********************************************************************************
* Start/Stop methods associated to the current step.
* They are called by the startButtonClicked/stopButtonClicked methods of <PMain>
**********************************************************************************/
void PTabImport::start()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport start() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        //first checking that no volume has imported yet
        if(CImport::instance()->getVolume())
            throw MyException("A volume has been already imported! Please restart the plugin to import another volume.");

        //checking that the inserted path exists
        string import_path = path_field->text().toStdString();
        if(!StackedVolume::fileExists(import_path.c_str()))
            throw MyException("The inserted path does not exist!");

        //if volume's directory has been selected, one should check if additional informations are required
        if(import_path.find(".xml")==std::string::npos && import_path.find(".XML")==std::string::npos)
        {
            string mdata_fpath = import_path;
            mdata_fpath.append("/");
            mdata_fpath.append(VM_BIN_METADATA_FILE_NAME);

            //if this is the first time the volume is going to be imported (metadata binary file doesn't exist)
            //or "Re-import" checkbox is selected, further informations are required
            if(!StackedVolume::fileExists(mdata_fpath.c_str()) || reimport_checkbox->isChecked())
            {
                int pos=0;
                QString tbv = axs1_field->text();
                if(axs1_field->validator()->validate(tbv,pos) != QValidator::Acceptable)
                {
                    axs1_field->setFocus();
                    throw MyException("One or more fields not properly filled");
                }
                tbv = axs2_field->text();
                if(axs2_field->validator()->validate(tbv,pos) != QValidator::Acceptable)
                {
                    axs2_field->setFocus();
                    throw MyException("One or more fields not properly filled");
                }
                tbv = axs3_field->text();
                if(axs3_field->validator()->validate(tbv,pos) != QValidator::Acceptable)
                {
                    axs3_field->setFocus();
                    throw MyException("One or more fields not properly filled");
                }
                tbv = vxl1_field->text();
                if(vxl1_field->validator()->validate(tbv,pos) != QValidator::Acceptable)
                {
                    vxl1_field->setFocus();
                    throw MyException("One or more fields not properly filled");
                }
                tbv = vxl2_field->text();
                if(vxl2_field->validator()->validate(tbv,pos) != QValidator::Acceptable)
                {
                    vxl2_field->setFocus();
                    throw MyException("One or more fields not properly filled");
                }
                tbv = vxl3_field->text();
                if(vxl3_field->validator()->validate(tbv,pos) != QValidator::Acceptable)
                {
                    vxl3_field->setFocus();
                    throw MyException("One or more fields not properly filled");
                }
                CImport::instance()->setAxes(axs1_field->text().toStdString().c_str(),
                                             axs2_field->text().toStdString().c_str(),
                                             axs3_field->text().toStdString().c_str());
                CImport::instance()->setVoxels(vxl1_field->text().toStdString().c_str(),
                                               vxl2_field->text().toStdString().c_str(),
                                               vxl3_field->text().toStdString().c_str());
                CImport::instance()->setReimport(reimport_checkbox->isChecked());
            }
        }
        CImport::instance()->setPath(import_path);

        //disabling import form and enabling progress bar animation and tab wait animation
        import_form->setEnabled(false);
        PMain::instance()->getProgressBar()->setEnabled(true);
        PMain::instance()->getProgressBar()->setMinimum(0);
        PMain::instance()->getProgressBar()->setMaximum(0);
        PMain::instance()->getStatusBar()->showMessage("Importing volume...");
        wait_movie->start();
        container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, wait_label);

        //starting
        CImport::instance()->start();
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::instance()->resetGUI();
        CImport::instance()->reset();
    }
}
void PTabImport::stop()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport stop() launched\n", this->thread()->currentThreadId());
    #endif

    //terminating thread
    try
    {
        CImport::instance()->terminate();
        CImport::instance()->wait();
        CImport::instance()->reset();
    }
    catch(MyException &ex) {QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));}
    catch(...) {QMessageBox::critical(this,QObject::tr("Error"), QObject::tr("Unable to determine error's type"),QObject::tr("Ok"));}

    //re-enabling import form and disabling progress bar and wait animations
    import_form->setEnabled(true);
    PMain::instance()->resetGUI();
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
}

/*********************************************************************************
* Called by <CImport> when the associated operation has been performed.
* If an exception has occurred in the <CImport> thread,  it is propagated and man-
* aged in the current thread (ex != 0). Otherwise, volume information are imported
* in the GUI by the <StackedVolume> handle of <CImport>.
**********************************************************************************/
void PTabImport::import_done(MyException *ex)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport import_done(%s) launched\n", this->thread()->currentThreadId(), (ex? "ex" : "NULL"));
    #endif

    //if an exception has occurred, showing a message error and re-enabling import form
    if(ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
        import_form->setEnabled(true);
    }
    else
    {
        //otherwise inserting volume's informations...
        info_panel->setEnabled(true);
        preview_panel->setEnabled(true);
        volumedir_field->setText(CImport::instance()->getVolume()->getSTACKS_DIR());
        nrows_field->setText(QString::number(CImport::instance()->getVolume()->getN_ROWS()));
        ncols_field->setText(QString::number(CImport::instance()->getVolume()->getN_COLS()));
        stack_height_field->setText(QString::number(CImport::instance()->getVolume()->getStacksHeight()));
        stack_width_field->setText(QString::number(CImport::instance()->getVolume()->getStacksWidth()));
        stack_depth_field->setText(QString::number(CImport::instance()->getVolume()->getN_SLICES()));
        vxl_V_field->setText(QString::number(CImport::instance()->getVolume()->getVXL_V()));
        vxl_H_field->setText(QString::number(CImport::instance()->getVolume()->getVXL_H()));
        vxl_D_field->setText(QString::number(CImport::instance()->getVolume()->getVXL_D()));
        org_V_field->setText(QString::number(CImport::instance()->getVolume()->getORG_V()));
        org_H_field->setText(QString::number(CImport::instance()->getVolume()->getORG_H()));
        org_D_field->setText(QString::number(CImport::instance()->getVolume()->getORG_D()));
        ovp_V_field->setText(QString::number(CImport::instance()->getVolume()->getOVERLAP_V()));
        ovp_H_field->setText(QString::number(CImport::instance()->getVolume()->getOVERLAP_H()));
        slice_slider->setMinimum(0);
        slice_slider->setMaximum(CImport::instance()->getVolume()->getN_SLICES()-1);
        slice_slider->setValue(slice_slider->maximum()/2);
        slice_index_field->setText(QString::number(slice_slider->value()));

        //...and enabling (ed updating) all other tabs
        PTabDisplComp::getInstance()->setEnabled(true);
        PTabDisplProj::getInstance()->setEnabled(true);
        PTabDisplThresh::getInstance()->setEnabled(true);
        PTabPlaceTiles::getInstance()->setEnabled(true);
        PTabMergeTiles::getInstance()->setEnabled(true);
    }

    //resetting some widgets
    PMain::instance()->resetGUI();
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
    CImport::instance()->reset();
}

/**********************************************************************************
* Called by <CPreview> when the associated operation has been performed.
* If an exception has occurred in the <CPreview> thread,  it is propagated and man-
* aged in the current thread (ex != 0). Otherwise, the preview which was saved back
* onto the disk is loaded and shown in Vaa3D.
***********************************************************************************/
void PTabImport::preview_done(MyException *ex, Image4DSimple* img)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport preview_done(%s) launched\n", this->thread()->currentThreadId(), (ex? "ex" : "NULL"));
    #endif

    //resetting some widgets
    preview_panel->setEnabled(true);
    PMain::instance()->resetGUI();
    PMain::instance()->setStartButtonEnabled(true);
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
    CImport::instance()->reset();

    //if an exception has occurred, showing a message error. Otherwise showing the computed preview
    if(ex)
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));    
    else
    {
        v3dhandle new_win = PMain::instance()->getV3D_env()->newImageWindow(QString("Slice ").append(slice_index_field->text()));
        //Image4DSimple* img = new Image4DSimple();
        //char path[VM_STATIC_STRINGS_SIZE];
        //sprintf(path, "%s/test_middle_slice.tif", CImport::instance()->getVolume()->getSTACKS_DIR());
        //img->loadImage(path);
        PMain::instance()->getV3D_env()->setImage(new_win, img);
    }
}
