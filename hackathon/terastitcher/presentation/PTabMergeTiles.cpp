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

#include "PTabMergeTiles.h"
#include "MyException.h"
#include "StackedVolume.h"
#include "PMain.h"
#include "control/CImport.h"
#include "control/CMergeTiles.h"
#include "StackStitcher.h"
#include "S_config.h"

using namespace terastitcher;

/*********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "istance(...)"
**********************************************************************************/
PTabMergeTiles* PTabMergeTiles::uniqueInstance = NULL;
void PTabMergeTiles::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

PTabMergeTiles::PTabMergeTiles(QMyTabWidget* _container, int _tab_index) : QWidget(), container(_container), tab_index(_tab_index)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabMergeTiles created\n", this->thread()->currentThreadId());
    #endif

    //help box
    helpbox = new QLabel("<html><table><tr style=\"vertical-align: middle;\"><td><img src=\":/icons/help.png\"></td>"
                         "<td><p style=\"text-align:justify; margin-left:10px;\"> This step combines tiles using the computed displacements "
                         "to obtain a multistacked or stacked volume which is saved at different resolutions. The overlapping regions are "
                         "substitued by a blended version of them according to the selected blending algorithm.<br><br> "
                         "If the <i>Optimal tiles placement</i> step has NOT been performed, nominal stage coordinates will be used "
                         "for merging. This allows direct stitching provided that nominal coordinates are precise enough.<br><br>"
                         "This step is the most time-consuming one, however memory usage is limited thanks to the slice-based adopted"
                         "approach.</p> </td></tr></table> </html>");
    helpbox->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); margin-top:10px; margin-bottom:10px; padding-top:10px; padding-bottom:10px;");
    helpbox->setWordWrap(true);
    helpbox->setFixedHeight(180);

    //basic panel widgets
    basic_panel = new QGroupBox("Basic settings");
    basic_panel->setStyle(new QWindowsStyle());
    savedir_label = new QLabel("Save stitched volume to:");
    savedir_field = new QLineEdit("Enter or select the EMPTY directory where to save the stitched volume.");
    browse_button = new QPushButton("Browse for dir...");
    resolutions_label = new QLabel("Resolution (V x H x D)");
    resolutions_label->setAlignment(Qt::AlignCenter);
    resolutions_size_label = new QLabel("Size(GVoxels)");
    resolutions_size_label->setAlignment(Qt::AlignCenter);
    resolutions_save_label = new QLabel("Save to disk");
    resolutions_save_label->setAlignment(Qt::AlignCenter);
    resolutions_view_label = new QLabel("Open in Vaa3D");
    resolutions_view_label->setAlignment(Qt::AlignCenter);
    outputs_label      = new QLabel("Outputs:");
    outputs_label->setAlignment(Qt::AlignVCenter);
    resolutions_save_selection = new QButtonGroup();
    resolutions_save_selection->setExclusive(false);
    for(int i=0; i<S_MAX_MULTIRES; i++)
    {
        resolutions_fields[i] = new QLabel("n.a. x n.a. x n.a.");
        resolutions_fields[i]->setAlignment(Qt::AlignCenter);
        resolutions_sizes[i] = new QLabel("n.a.");
        resolutions_sizes[i]->setAlignment(Qt::AlignCenter);
        resolutions_save_cboxs[i] = new QCheckBox("");
        resolutions_save_cboxs[i]->setChecked(true);
        resolutions_save_cboxs[i]->setStyleSheet("::indicator {subcontrol-position: center; subcontrol-origin: padding;}");
        resolutions_save_selection->addButton(resolutions_save_cboxs[i]);
        resolutions_view_cboxs[i] = new QCheckBox("");
        resolutions_view_cboxs[i]->setStyleSheet("::indicator {subcontrol-position: center; subcontrol-origin: padding;}");
    }
    volumeformat_selection  = new QButtonGroup();
    volumeformat_label      = new QLabel("Volume's format:");
    multistack_cbox         = new QCheckBox("multi-stack");
    multistack_cbox->setChecked(true);
    singlestack_cbox        = new QCheckBox("single-stack");
    multistack_cbox->setStyleSheet("margin-left: auto; margin-right:auto; width=70%;");
    singlestack_cbox->setStyleSheet("margin-left: auto; margin-right:auto; width=70%;");
    volumeformat_selection->addButton(multistack_cbox);
    volumeformat_selection->addButton(singlestack_cbox);
    stackdims_label         = new QLabel("Stack dimensions (height x width):");
    stackheight_field       = new QSpinBox();
    stackheight_field->setAlignment(Qt::AlignCenter);
    stackheight_field->setMinimum(256);
    stackheight_field->setMaximum(10000);
    stackheight_field->setValue(1000);
    stackwidth_field        = new QSpinBox();
    stackwidth_field->setAlignment(Qt::AlignCenter);
    stackwidth_field->setMinimum(256);
    stackwidth_field->setMaximum(10000);
    stackwidth_field->setValue(1000);
    memocc_label            = new QLabel("Estimated memory usage:");
    memocc_field            = new QLabel();

    //advanced panel widgets
    advanced_panel = new QGroupBox("Advanced settings");
    advanced_panel->setStyle(new QWindowsStyle());
    volumeportion_label = new QLabel("Portion to be stitched:");
    row0_field = new QSpinBox();
    row0_field->setAlignment(Qt::AlignCenter);
    row0_field->setMinimum(-1);
    row0_field->setValue(-1);
    row1_field = new QSpinBox();
    row1_field->setAlignment(Qt::AlignCenter);
    row1_field->setMinimum(-1);
    row1_field->setValue(-1);
    col0_field = new QSpinBox();
    col0_field->setAlignment(Qt::AlignCenter);
    col0_field->setMinimum(-1);
    col0_field->setValue(-1);
    col1_field = new QSpinBox();
    col1_field->setAlignment(Qt::AlignCenter);
    col1_field->setMinimum(-1);
    col1_field->setValue(-1);
    slice0_field = new QSpinBox();
    slice0_field->setAlignment(Qt::AlignCenter);
    slice0_field->setMinimum(-1);
    slice0_field->setValue(-1);
    slice1_field = new QSpinBox();
    slice1_field->setAlignment(Qt::AlignCenter);
    slice1_field->setMinimum(-1);
    slice1_field->setValue(-1);
    excludenonstitchables_cbox = new QCheckBox();
    blendingalgo_label = new QLabel("Blending algorithm:");
    blendingalbo_cbox = new QComboBox();
    blendingalbo_cbox->insertItem(0, "No Blending");
    blendingalbo_cbox->insertItem(1, "Sinusoidal Blending");
    blendingalbo_cbox->insertItem(2, "No Blending with emphasized stacks borders");
    blendingalbo_cbox->setCurrentIndex(1);
    restoreSPIM_label = new QLabel("SPIM artifacts removal:");
    restoreSPIM_cbox = new QComboBox();
    restoreSPIM_cbox->insertItem(0, "None");
    restoreSPIM_cbox->insertItem(1, "Zebrated pattern along V");
    restoreSPIM_cbox->insertItem(2, "Zebrated pattern along H");
    restoreSPIM_cbox->insertItem(3, "Zebrated pattern along D");
    imgformat_label = new QLabel("Slice image format:");
    imgformat_cbox = new QComboBox();
    imgformat_cbox->insertItem(0, "tif");
    imgformat_cbox->insertItem(1, "tiff");
    imgformat_cbox->insertItem(2, "png");
    imgformat_cbox->insertItem(3, "bmp");
    imgformat_cbox->insertItem(4, "jpeg");
    imgformat_cbox->insertItem(5, "jpg");
    imgformat_cbox->insertItem(6, "dib");
    imgformat_cbox->insertItem(7, "pbm");
    imgformat_cbox->insertItem(8, "pgm");
    imgformat_cbox->insertItem(9, "ppm");
    imgformat_cbox->insertItem(10, "sr");
    imgformat_cbox->insertItem(11, "ras");
    imgdepth_label = new QLabel("depth:");
    imgdepth_cbox = new QComboBox();
    imgdepth_cbox->insertItem(0, "8");
    imgdepth_cbox->insertItem(1, "16");



    /*** LAYOUT SECTIONS ***/
    //basic settings panel
    QGridLayout* basicpanel_layout = new QGridLayout();
    basicpanel_layout->addWidget(savedir_label, 0, 0, 1, 2);
    savedir_label->setFixedWidth(185);
    basicpanel_layout->addWidget(savedir_field, 0, 2, 1, 10);
    basicpanel_layout->addWidget(browse_button, 0, 12, 1, 2);
    QWidget* emptyspace = new QWidget();
    emptyspace->setFixedHeight(15);
    basicpanel_layout->addWidget(emptyspace, 1, 0, 1, 14);
    basicpanel_layout->addWidget(resolutions_label, 2, 2, 1, 2);
    basicpanel_layout->addWidget(resolutions_size_label, 2, 4, 1, 2);
    basicpanel_layout->addWidget(resolutions_save_label, 2, 6, 1, 2);
    basicpanel_layout->addWidget(resolutions_view_label, 2, 8, 1, 2);
    basicpanel_layout->addWidget(outputs_label, 3, 0, S_MAX_MULTIRES, 2);
    for(int i=0; i<S_MAX_MULTIRES; i++)
    {
        basicpanel_layout->addWidget(resolutions_fields[i], 3+i, 2, 1, 2);
        basicpanel_layout->addWidget(resolutions_sizes[i], 3+i, 4, 1, 2);
        basicpanel_layout->addWidget(resolutions_save_cboxs[i], 3+i, 6, 1, 2);
        basicpanel_layout->addWidget(resolutions_view_cboxs[i], 3+i, 8, 1, 2);
    }
    QWidget* emptyspace2 = new QWidget();
    emptyspace2->setFixedHeight(15);
    basicpanel_layout->addWidget(emptyspace2, 3+S_MAX_MULTIRES, 0, 1, 14);
    basicpanel_layout->addWidget(volumeformat_label, 4+S_MAX_MULTIRES, 0, 1, 2);
    basicpanel_layout->addWidget(multistack_cbox, 4+S_MAX_MULTIRES, 2, 1, 2);
    stackheight_label = new QLabel("stacks height:");
    stackheight_label->setAlignment(Qt::AlignCenter);
    by_label = new QLabel("x");
    stackwidth_label = new QLabel("stacks width:");
    stackwidth_label->setAlignment(Qt::AlignCenter);
    basicpanel_layout->addWidget(stackheight_label, 4+S_MAX_MULTIRES, 4, 1, 2);
    basicpanel_layout->addWidget(stackheight_field, 4+S_MAX_MULTIRES, 6, 1, 2);
    basicpanel_layout->addWidget(stackwidth_label, 4+S_MAX_MULTIRES, 8, 1, 2);
    basicpanel_layout->addWidget(stackwidth_field, 4+S_MAX_MULTIRES, 10, 1, 2);
    basicpanel_layout->addWidget(singlestack_cbox, 5+S_MAX_MULTIRES, 2, 1, 2);
    basicpanel_layout->addWidget(memocc_label, 6+S_MAX_MULTIRES, 0, 1, 2);
    basicpanel_layout->addWidget(memocc_field, 6+S_MAX_MULTIRES, 2, 1, 2);
    basic_panel->setLayout(basicpanel_layout);

    //basic settings panel
    QGridLayout* advancedpanel_layout = new QGridLayout();
    QLabel* to_label1 = new QLabel("to");
    to_label1->setAlignment(Qt::AlignCenter);
    to_label1->setFixedWidth(18);
    QLabel* to_label2 = new QLabel("to");
    to_label2->setAlignment(Qt::AlignCenter);
    to_label2->setFixedWidth(18);
    QLabel* to_label3 = new QLabel("to");
    to_label3->setAlignment(Qt::AlignCenter);
    to_label3->setFixedWidth(18);
    QLabel* rows_label = new QLabel("Rows: ");
    rows_label->setLayoutDirection(Qt::RightToLeft);
    rows_label->setAlignment(Qt::AlignCenter);
    QLabel* cols_label = new QLabel("Columns: ");
    cols_label->setLayoutDirection(Qt::RightToLeft);
    cols_label->setAlignment(Qt::AlignCenter);
    QLabel* slices_label = new QLabel("Slices: ");
    slices_label->setLayoutDirection(Qt::RightToLeft);
    slices_label->setAlignment(Qt::AlignCenter);
    volumeportion_label->setFixedWidth(185);
    QWidget* emptyspace3 = new QWidget();
    emptyspace3->setFixedWidth(8);
    QWidget* emptyspace4 = new QWidget();
    emptyspace4->setFixedWidth(8);
    advancedpanel_layout->addWidget(volumeportion_label, 0, 0, 1, 2);
    advancedpanel_layout->addWidget(rows_label, 0, 2, 1, 1);
    advancedpanel_layout->addWidget(row0_field, 0, 3, 1, 1);
    advancedpanel_layout->addWidget(to_label1, 0, 4, 1, 1);
    advancedpanel_layout->addWidget(row1_field, 0, 5, 1, 1);
    advancedpanel_layout->addWidget(emptyspace3, 0, 6, 1, 1);
    advancedpanel_layout->addWidget(cols_label, 0, 7, 1, 1);
    advancedpanel_layout->addWidget(col0_field, 0, 8, 1, 1);
    advancedpanel_layout->addWidget(to_label2, 0, 9, 1, 1);
    advancedpanel_layout->addWidget(col1_field, 0, 10, 1, 1);
    advancedpanel_layout->addWidget(emptyspace4, 0, 11, 1, 1);
    advancedpanel_layout->addWidget(slices_label, 0, 12, 1, 1);
    advancedpanel_layout->addWidget(slice0_field, 0, 13, 1, 1);
    advancedpanel_layout->addWidget(to_label3,    0, 14, 1, 1);
    advancedpanel_layout->addWidget(slice1_field, 0, 15, 1, 1);
    advancedpanel_layout->addWidget(new QLabel("Exclude nonstitchables:"), 1, 0, 1, 2);
    advancedpanel_layout->addWidget(excludenonstitchables_cbox, 1, 2, 1, 1);
    advancedpanel_layout->addWidget(blendingalgo_label, 2, 0, 1, 2);
    advancedpanel_layout->addWidget(blendingalbo_cbox, 2, 2, 1, 4);
    advancedpanel_layout->addWidget(restoreSPIM_label, 3, 0, 1, 2);
    advancedpanel_layout->addWidget(restoreSPIM_cbox, 3, 2, 1, 4);
    advancedpanel_layout->addWidget(imgformat_label, 4, 0, 1, 2);
    advancedpanel_layout->addWidget(imgformat_cbox, 4, 2, 1, 1);
    advancedpanel_layout->addWidget(imgdepth_label, 4, 4, 1, 1);
    advancedpanel_layout->addWidget(imgdepth_cbox, 4, 5, 1, 1);
    advanced_panel->setLayout(advancedpanel_layout);

    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);
    layout->addWidget(helpbox);
    layout->addWidget(basic_panel);
    layout->addWidget(advanced_panel);
    setLayout(layout);

    //wait animated GIF tab icon
    wait_movie = new QMovie(":/icons/wait.gif");
    wait_label = new QLabel(this);
    wait_label->setMovie(wait_movie);

    // signals and slots
    connect(browse_button, SIGNAL(clicked()), this, SLOT(browse_button_clicked()));
    connect(row0_field, SIGNAL(valueChanged(int)), this, SLOT(stacksinterval_changed()));
    connect(row0_field, SIGNAL(valueChanged(int)), this, SLOT(row0_field_changed(int)));
    connect(row1_field, SIGNAL(valueChanged(int)), this, SLOT(stacksinterval_changed()));
    connect(row1_field, SIGNAL(valueChanged(int)), this, SLOT(row1_field_changed(int)));
    connect(col0_field, SIGNAL(valueChanged(int)), this, SLOT(stacksinterval_changed()));
    connect(col0_field, SIGNAL(valueChanged(int)), this, SLOT(col0_field_changed(int)));
    connect(col1_field, SIGNAL(valueChanged(int)), this, SLOT(stacksinterval_changed()));
    connect(col1_field, SIGNAL(valueChanged(int)), this, SLOT(col1_field_changed(int)));
    connect(slice0_field, SIGNAL(valueChanged(int)), this, SLOT(updateContent()));
    connect(slice0_field, SIGNAL(valueChanged(int)), this, SLOT(slice0_field_changed(int)));
    connect(slice1_field, SIGNAL(valueChanged(int)), this, SLOT(updateContent()));
    connect(slice1_field, SIGNAL(valueChanged(int)), this, SLOT(slice1_field_changed(int)));
    connect(excludenonstitchables_cbox, SIGNAL(stateChanged(int)),this, SLOT(excludenonstitchables_changed()));
    connect(multistack_cbox, SIGNAL(stateChanged(int)), this, SLOT(volumeformat_changed()));
    connect(singlestack_cbox, SIGNAL(stateChanged(int)), this, SLOT(volumeformat_changed()));
    for(int i=0; i<S_MAX_MULTIRES; i++)
    {
        connect(resolutions_save_cboxs[i], SIGNAL(stateChanged(int)), this, SLOT(updateContent()));
        connect(resolutions_save_cboxs[i], SIGNAL(stateChanged(int)), this, SLOT(save_changed(int)));
        connect(resolutions_view_cboxs[i], SIGNAL(stateChanged(int)), this, SLOT(viewinVaa3D_changed(int)));
    }
    connect(CMergeTiles::instance(), SIGNAL(sendOperationOutcome(MyException*, Image4DSimple*)), this, SLOT(merging_done(MyException*, Image4DSimple*)), Qt::QueuedConnection);
}


PTabMergeTiles::~PTabMergeTiles()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabMergeTiles destroyed\n", this->thread()->currentThreadId());
    #endif
}

/*********************************************************************************
* Start/Stop methods associated to the current step.
* They are called by the startButtonClicked/stopButtonClicked methods of <PMain>
**********************************************************************************/
void PTabMergeTiles::start()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabMergeTiles start() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        //first checking that a volume has been properly imported
        if(!CImport::instance()->getVolume())
            throw MyException("A volume must be properly imported first. Please perform the Import step.");

        //verifying that directory is readable
        QDir directory(savedir_field->text());
        if(!directory.isReadable())
            throw MyException(QString("Cannot open directory\n \"").append(savedir_field->text()).append("\"").toStdString().c_str());

        //asking confirmation to continue when saving to a non-empty dir
        QStringList dir_entries = directory.entryList();
        if(dir_entries.size() > 2 && QMessageBox::information(this, "Warning", "The directory you selected is NOT empty. \n\nIf you continue, the merging "
                                               "process could fail if the directories to be created already exist in the given path.", "Continue", "Cancel"))
        {
            PMain::instance()->resetGUI();
            return;
        }

        //disabling import form and enabling progress bar animation and tab wait animation
        PMain::instance()->getProgressBar()->setEnabled(true);
        PMain::instance()->getProgressBar()->setMinimum(0);
        PMain::instance()->getProgressBar()->setMaximum(100);
        wait_movie->start();
        container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, wait_label);

        //propagating options and paramaters and launching task
        CMergeTiles::instance()->setPMergeTiles(this);
        for(int i=0; i<S_MAX_MULTIRES; i++)
        {
            CMergeTiles::instance()->setResolution(i, resolutions_save_cboxs[i]->isChecked());
            if(singlestack_cbox->isChecked() && resolutions_view_cboxs[i]->isChecked())
                CMergeTiles::instance()->setResolutionToShow(i);
        }
        CMergeTiles::instance()->start();
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::instance()->resetGUI();
    }
    catch(...)
    {
        QMessageBox::critical(this,QObject::tr("Error"), "Unknown error has occurred",QObject::tr("Ok"));
        PMain::instance()->resetGUI();
    }
}

void PTabMergeTiles::stop()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabMergeTiles stop() launched\n", this->thread()->currentThreadId());
    #endif

    // ----- terminating CMergeTiles's thread is UNSAFE ==> this feature should be disabled or a warning should be displayed ------
    //terminating thread
    try
    {
        CMergeTiles::instance()->terminate();
        CMergeTiles::instance()->wait();
    }
    catch(MyException &ex) {QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));}
    catch(...) {QMessageBox::critical(this,QObject::tr("Error"), QObject::tr("Unable to determine error's type"),QObject::tr("Ok"));}

    //disabling progress bar and wait animations
    PMain::instance()->resetGUI();
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
}

/**********************************************************************************
* Overrides QWidget's setEnabled(bool).
* If the widget is enabled, its fields are filled with the informations provided by
* the <StackedVolume> object of <CImport> instance.
***********************************************************************************/
void PTabMergeTiles::setEnabled(bool enabled)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabMergeTiles setEnabled(%d) called\n", this->thread()->currentThreadId(), enabled);
    #endif

    //first calling super-class implementation
    QWidget::setEnabled(enabled);

    //then filling widget fields
    if(enabled && CImport::instance()->getVolume())
    {
        StackedVolume* volume = CImport::instance()->getVolume();

        //inserting volume dimensions
        row0_field->setMinimum(0);
        row0_field->setMaximum(volume->getN_ROWS()-1);
        row0_field->setValue(0);
        row1_field->setMinimum(0);
        row1_field->setMaximum(volume->getN_ROWS()-1);
        row1_field->setValue(volume->getN_ROWS()-1);
        col0_field->setMinimum(0);
        col0_field->setMaximum(volume->getN_COLS()-1);
        col0_field->setValue(0);
        col1_field->setMinimum(0);
        col1_field->setMaximum(volume->getN_COLS()-1);
        col1_field->setValue(volume->getN_COLS()-1);
        multistack_cbox->setChecked(true);
        volumeformat_changed();

        //updating content
        updateContent();
    }
}

/**********************************************************************************
* Opens the dialog to select the directory where the stitched volume has to be saved.
* Called when user clicks on "browse_button".
***********************************************************************************/
void PTabMergeTiles::browse_button_clicked()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabMergeTiles browse_button_clicked() launched\n", this->thread()->currentThreadId());
    #endif

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDirectory(CImport::instance()->getVolume()->getSTACKS_DIR());
    dialog.setWindowTitle("Please select an EMPTY directory");
    if (dialog.exec())
    {
        QStringList fileNames = dialog.selectedFiles();
        QString xmlpath = fileNames.first();

        QDir directory(xmlpath);
        QStringList dir_entries = directory.entryList();
        if(dir_entries.size() <= 2)
            savedir_field->setText(xmlpath);
        else
        {
            if(!QMessageBox::information(this, "Warning", "The directory you selected is NOT empty. \n\nIf you continue, the merging "
                                               "process could fail if the directories to be created already exist in the given path.", "Continue", "Cancel"))
                savedir_field->setText(xmlpath);
        }
    }
}

/**********************************************************************************
* Called when <excludenonstitchables_cbox> combobox state changed.
* Inferior and superior limits of spinboxes are recomputed.
***********************************************************************************/
void PTabMergeTiles::excludenonstitchables_changed()
{
    try
    {
        if(CImport::instance()->getVolume())
        {
            StackedVolume* volume = CImport::instance()->getVolume();

            StackStitcher stitcher(volume);
            stitcher.computeVolumeDims(excludenonstitchables_cbox->isChecked());
            row0_field->setMinimum(stitcher.getROW0());
            row0_field->setMaximum(stitcher.getROW1());
            row0_field->setValue(stitcher.getROW0());
            row1_field->setMinimum(stitcher.getROW0());
            row1_field->setMaximum(stitcher.getROW1());
            row1_field->setValue(stitcher.getROW1());
            col0_field->setMinimum(stitcher.getCOL0());
            col0_field->setMaximum(stitcher.getCOL1());
            col0_field->setValue(stitcher.getCOL0());
            col1_field->setMinimum(stitcher.getCOL0());
            col1_field->setMaximum(stitcher.getCOL1());
            col1_field->setValue(stitcher.getCOL1());
            slice0_field->setMinimum(stitcher.getD0());
            slice0_field->setMaximum(stitcher.getD1());
            slice0_field->setValue(stitcher.getD0());
            slice1_field->setMinimum(stitcher.getD0());
            slice1_field->setMaximum(stitcher.getD1());
            slice1_field->setValue(stitcher.getD1());

            //updating content
            updateContent();

        }
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}


/**********************************************************************************
* Called when <row0_field>, <row1_field>, <col0_field> or <col1_field> changed.
* Inferior and superior limits of <slice_[]_cbox> spinboxes are recomputed.
***********************************************************************************/
void PTabMergeTiles::stacksinterval_changed()
{
    try
    {
        if(CImport::instance()->getVolume())
        {
            StackedVolume* volume = CImport::instance()->getVolume();

            StackStitcher stitcher(volume);
            stitcher.computeVolumeDims(excludenonstitchables_cbox->isChecked(), row0_field->value(), row1_field->value(),
                                       col0_field->value(), col1_field->value());

            slice0_field->setMinimum(stitcher.getD0());
            slice0_field->setMaximum(stitcher.getD1());
            slice0_field->setValue(stitcher.getD0());
            slice1_field->setMinimum(stitcher.getD0());
            slice1_field->setMaximum(stitcher.getD1());
            slice1_field->setValue(stitcher.getD1());

            //updating content
            updateContent();

        }
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

/**********************************************************************************
* Updates widgets contents
***********************************************************************************/
void PTabMergeTiles::updateContent()
{
    try
    {
        if(CImport::instance()->getVolume())
        {
            StackedVolume* volume = CImport::instance()->getVolume();

            StackStitcher stitcher(volume);
            stitcher.computeVolumeDims(excludenonstitchables_cbox->isChecked(), row0_field->value(), row1_field->value(),
                                       col0_field->value(), col1_field->value(), slice0_field->value(), slice1_field->value());

            int max_res = 0;
            for(int i=0; i<S_MAX_MULTIRES; i++)
            {
                int height = (stitcher.getV1()-stitcher.getV0())/pow(2.0f, i);
                int width = (stitcher.getH1()-stitcher.getH0())/pow(2.0f, i);
                int depth = (stitcher.getD1()-stitcher.getD0())/pow(2.0f, i);
                float GVoxels = (height/1024.0f)*(width/1024.0f)*(depth/1024.0f);
                resolutions_fields[i]->setText(QString::number(height).append(" x ").append(QString::number(width)).append(" x ").append(QString::number(depth)));
                resolutions_sizes[i]->setText(QString::number(GVoxels,'f',3));
                if(resolutions_save_cboxs[i]->isChecked())
                    max_res = std::max(max_res, i);
            }

            //updating RAM usage estimation
            int layer_height = stitcher.getV1()-stitcher.getV0();
            int layer_width = stitcher.getH1()-stitcher.getH0();
            int layer_depth = pow(2.0f, max_res);
            float MBytes = (layer_height/1024.0f)*(layer_width/1024.0f)*layer_depth*4;
            memocc_field->setText(QString::number(MBytes, 'f', 0).append(" MB"));
        }
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

/**********************************************************************************
* Called when the corresponding spinboxes changed.
* New maximum/minimum values are set according to the status of spinboxes.
***********************************************************************************/
void PTabMergeTiles::row0_field_changed(int val){row1_field->setMinimum(val);}
void PTabMergeTiles::row1_field_changed(int val){row0_field->setMaximum(val);}
void PTabMergeTiles::col0_field_changed(int val){col1_field->setMinimum(val);}
void PTabMergeTiles::col1_field_changed(int val){col0_field->setMaximum(val);}
void PTabMergeTiles::slice0_field_changed(int val){slice1_field->setMinimum(val);}
void PTabMergeTiles::slice1_field_changed(int val){slice0_field->setMaximum(val);}

/**********************************************************************************
* Called when <multistack_cbox> or <multistack_cbox> state changed.
***********************************************************************************/
void PTabMergeTiles::volumeformat_changed()
{
    stackheight_label->setEnabled(multistack_cbox->isChecked());
    stackheight_field->setEnabled(multistack_cbox->isChecked());
    stackwidth_label->setEnabled(multistack_cbox->isChecked());
    stackwidth_field->setEnabled(multistack_cbox->isChecked());
    by_label->setEnabled(multistack_cbox->isChecked());

    for(int i=0; i<S_MAX_MULTIRES; i++)
        resolutions_view_cboxs[i]->setEnabled(!multistack_cbox->isChecked());
}

/**********************************************************************************
* Called when <resolutions_view_cboxs[i]> changed
***********************************************************************************/
void PTabMergeTiles::viewinVaa3D_changed(int checked)
{
    QCheckBox* sender = (QCheckBox*) QObject::sender();

    //unchecking other checkboxes
    if(checked)
        for(int i=0; i<S_MAX_MULTIRES; i++)
            if(resolutions_view_cboxs[i]->isChecked())
            {
                if(resolutions_view_cboxs[i] == sender)
                    resolutions_save_cboxs[i]->setChecked(true);
                else
                    resolutions_view_cboxs[i]->setChecked(false);
            }
}

/**********************************************************************************
* Called when <resolutions_save_cboxs[i]> changed
***********************************************************************************/
void PTabMergeTiles::save_changed(int checked)
{
    //unchecking other checkboxes
    if(!checked)
        for(int i=0; i<S_MAX_MULTIRES; i++)
            if(!resolutions_save_cboxs[i]->isChecked())
                resolutions_view_cboxs[i]->setChecked(false);
}

/**********************************************************************************
* Called by <CMergeTiles> when the associated operation has been performed.
* If an exception has occurred in the <CMergeTiles> thread,it is propagated and man-
* aged in the current thread (ex != 0). Otherwise, if a valid  3D image  is passed,
* it is shown in Vaa3D.
***********************************************************************************/
void PTabMergeTiles::merging_done(MyException *ex, Image4DSimple* img)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabMergeTiles merging_done(%s) launched\n", this->thread()->currentThreadId(), (ex? "ex" : "NULL"));
    #endif

    //if an exception has occurred, showing a message error
    if(ex)
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
    else if(img)
    {
        v3dhandle new_win = PMain::instance()->getV3D_env()->newImageWindow(img->getFileName());
        PMain::instance()->getV3D_env()->setImage(new_win, img);
    }

    //resetting some widgets
    PMain::instance()->resetGUI();
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
}
