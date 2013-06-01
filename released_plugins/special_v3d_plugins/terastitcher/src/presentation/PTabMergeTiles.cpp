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
#include "src/control/CImport.h"
#include "src/control/CMergeTiles.h"
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

    //basic panel widgets
    basic_panel = new QWidget();
    savedir_label = new QLabel("Save stitched volume to:");
    savedir_field = new QLineEdit();
    savedir_field->setFont(QFont("",8));
    browse_button = new QPushButton("...");
    resolutions_label = new QLabel(QString("Resolution (X ").append(QChar(0x00D7)).append(" Y ").append(QChar(0x00D7)).append(" Z)"));
    resolutions_label->setFont(QFont("",8));
    resolutions_label->setAlignment(Qt::AlignCenter);
    resolutions_size_label = new QLabel("Size (GVoxels)");
    resolutions_size_label->setFont(QFont("",8));
    resolutions_size_label->setAlignment(Qt::AlignCenter);
    resolutions_save_label = new QLabel("Save to disk");
    resolutions_save_label->setFont(QFont("",8));
    resolutions_save_label->setAlignment(Qt::AlignCenter);
    resolutions_view_label = new QLabel("Open");
    resolutions_view_label->setFont(QFont("",8));
    resolutions_view_label->setAlignment(Qt::AlignCenter);
    outputs_label      = new QLabel("Outputs:");
    outputs_label->setAlignment(Qt::AlignVCenter);
    resolutions_save_selection = new QButtonGroup();
    resolutions_save_selection->setExclusive(false);
    for(int i=0; i<S_MAX_MULTIRES; i++)
    {
        resolutions_fields[i] = new QLabel();
        resolutions_fields[i]->setAlignment(Qt::AlignCenter);
        resolutions_sizes[i] = new QLabel();
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
    multistack_cbox->setFont(QFont("", 9));
    multistack_cbox->setChecked(true);
    singlestack_cbox        = new QCheckBox("single-stack");
    singlestack_cbox->setFont(QFont("", 9));
    volumeformat_selection->addButton(multistack_cbox);
    volumeformat_selection->addButton(singlestack_cbox);
    stackheight_field       = new QSpinBox();
    stackheight_field->setAlignment(Qt::AlignCenter);
    stackheight_field->setMinimum(256);
    stackheight_field->setMaximum(4096);
    stackheight_field->setValue(512);
    stackheight_field->setSuffix(" (height)");
    stackheight_field->setFont(QFont("", 9));
    stackwidth_field        = new QSpinBox();
    stackwidth_field->setAlignment(Qt::AlignCenter);
    stackwidth_field->setMinimum(256);
    stackwidth_field->setMaximum(4096);
    stackwidth_field->setValue(512);
    stackwidth_field->setSuffix(" (width)");
    stackwidth_field->setFont(QFont("", 9));
    memocc_label = new QLabel("Estimated memory usage:");
    memocc_field = new QLineEdit();
    memocc_field->setReadOnly(true);
    memocc_field->setAlignment(Qt::AlignCenter);
    memocc_field->setFont(QFont("", 9));
    showAdvancedButton = new QPushButton(QString("Advanced options ").append(QChar(0x00BB)), this);
    showAdvancedButton->setCheckable(true);

    //advanced panel widgets
    advanced_panel = new QWidget();
    volumeportion_label = new QLabel("Portion to be stitched:");
    row0_field = new QSpinBox();
    row0_field->setAlignment(Qt::AlignCenter);
    row0_field->setMinimum(-1);
    row0_field->setValue(-1);
    row0_field->setFont(QFont("", 9));
    row1_field = new QSpinBox();
    row1_field->setAlignment(Qt::AlignCenter);
    row1_field->setMinimum(-1);
    row1_field->setValue(-1);
    row1_field->setFont(QFont("", 9));
    col0_field = new QSpinBox();
    col0_field->setAlignment(Qt::AlignCenter);
    col0_field->setMinimum(-1);
    col0_field->setValue(-1);
    col0_field->setFont(QFont("", 9));
    col1_field = new QSpinBox();
    col1_field->setAlignment(Qt::AlignCenter);
    col1_field->setMinimum(-1);
    col1_field->setValue(-1);
    col1_field->setFont(QFont("", 9));
    slice0_field = new QSpinBox();
    slice0_field->setAlignment(Qt::AlignCenter);
    slice0_field->setMinimum(-1);
    slice0_field->setValue(-1);
    slice0_field->setFont(QFont("", 9));
    slice1_field = new QSpinBox();
    slice1_field->setAlignment(Qt::AlignCenter);
    slice1_field->setMinimum(-1);
    slice1_field->setValue(-1);
    slice1_field->setFont(QFont("", 9));
    excludenonstitchables_cbox = new QCheckBox();
    blendingalgo_label = new QLabel("Blending algorithm:");
    blendingalbo_cbox = new QComboBox();
    blendingalbo_cbox->insertItem(0, "No Blending");
    blendingalbo_cbox->insertItem(1, "Sinusoidal Blending");
    blendingalbo_cbox->insertItem(2, "No Blending with emphasized stacks borders");
    blendingalbo_cbox->setCurrentIndex(1);
    blendingalbo_cbox->setFont(QFont("", 9));
    restoreSPIM_label = new QLabel("SPIM artifacts removal:");
    restoreSPIM_cbox = new QComboBox();
    restoreSPIM_cbox->insertItem(0, "None");
    restoreSPIM_cbox->insertItem(1, "Zebrated pattern along V");
    restoreSPIM_cbox->insertItem(2, "Zebrated pattern along H");
    restoreSPIM_cbox->insertItem(3, "Zebrated pattern along D");
    restoreSPIM_cbox->setFont(QFont("", 9));
    imgformat_label = new QLabel("Slice image format:");
    imgformat_cbox = new QComboBox();
    imgformat_cbox->setFont(QFont("", 9));
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
    imgdepth_cbox->setFont(QFont("", 9));
    imgdepth_cbox->insertItem(0, "8");
    imgdepth_cbox->insertItem(1, "16");


    /*** LAYOUT SECTIONS ***/
    //basic settings panel
    QGridLayout* basicpanel_layout = new QGridLayout();
    savedir_label->setFixedWidth(200);
    browse_button->setFixedWidth(80);
    basicpanel_layout->addWidget(savedir_label, 0, 0, 1, 1);
    basicpanel_layout->addWidget(savedir_field, 0, 1, 1, 10);
    basicpanel_layout->addWidget(browse_button, 0, 11, 1, 1);
    QWidget* emptyspace = new QWidget();
    emptyspace->setFixedHeight(15);
    basicpanel_layout->addWidget(emptyspace, 1, 0, 1, 11);
    basicpanel_layout->addWidget(resolutions_label, 2, 1, 1, 6);
    resolutions_label->setFixedWidth(200);
    basicpanel_layout->addWidget(resolutions_size_label, 2, 7, 1, 3);
    resolutions_size_label->setFixedWidth(120);
    basicpanel_layout->addWidget(resolutions_save_label, 2, 10, 1, 1);
    basicpanel_layout->addWidget(resolutions_view_label, 2, 11, 1, 1);
    basicpanel_layout->addWidget(outputs_label, 3, 0, S_MAX_MULTIRES, 1);
    for(int i=0; i<S_MAX_MULTIRES; i++)
    {
        resolutions_fields[i]->setFont(QFont("", 9));
        resolutions_fields[i]->setFixedWidth(200);
        resolutions_sizes[i]->setFont(QFont("", 9));
        resolutions_sizes[i]->setFixedWidth(120);
        basicpanel_layout->addWidget(resolutions_fields[i], 3+i, 1, 1, 6);
        basicpanel_layout->addWidget(resolutions_sizes[i], 3+i, 7, 1, 3);
        basicpanel_layout->addWidget(resolutions_save_cboxs[i], 3+i, 10, 1, 1);
        basicpanel_layout->addWidget(resolutions_view_cboxs[i], 3+i, 11, 1, 1);
    }
    QWidget* emptyspace2 = new QWidget();
    emptyspace2->setFixedHeight(15);
    basicpanel_layout->addWidget(emptyspace2, 3+S_MAX_MULTIRES, 0, 1, 11);
    basicpanel_layout->addWidget(volumeformat_label, 4+S_MAX_MULTIRES, 0, 1, 1);
    basicpanel_layout->addWidget(singlestack_cbox, 4+S_MAX_MULTIRES, 1, 1, 3);
    basicpanel_layout->addWidget(multistack_cbox, 4+S_MAX_MULTIRES, 4, 1, 3);
    QHBoxLayout* stacksDimsLayout = new QHBoxLayout();
    stacksDimsLayout->addWidget(stackheight_field, 1);
    byLabel = new QLabel(QChar(0x00D7));
    byLabel->setAlignment(Qt::AlignCenter);
    byLabel->setFixedWidth(10);
    stacksDimsLayout->addWidget(byLabel);
    stacksDimsLayout->addWidget(stackwidth_field, 1);
    stacksDimsLayout->setSpacing(5);
    basicpanel_layout->addLayout(stacksDimsLayout, 4+S_MAX_MULTIRES, 7, 1, 5);
    basicpanel_layout->addWidget(memocc_label, 5+S_MAX_MULTIRES, 0, 1, 1);
    basicpanel_layout->addWidget(memocc_field, 5+S_MAX_MULTIRES, 1, 1, 2);
    QWidget* emptyspace3 = new QWidget();
    emptyspace3->setFixedHeight(5);
    QWidget* emptyspace4 = new QWidget();
    emptyspace4->setFixedHeight(5);
    basicpanel_layout->addWidget(emptyspace3, 6+S_MAX_MULTIRES, 0, 1, 12);
    basicpanel_layout->addWidget(showAdvancedButton, 7+S_MAX_MULTIRES, 0, 1, 12);
    basicpanel_layout->addWidget(emptyspace4, 8+S_MAX_MULTIRES, 0, 1, 12);
    basicpanel_layout->setVerticalSpacing(0);    
    basicpanel_layout->setContentsMargins(10,0,10,0);
    basic_panel->setLayout(basicpanel_layout);

    //advanced settings panel
    QGridLayout* advancedpanel_layout = new QGridLayout();
    volumeportion_label->setFixedWidth(200);
    QLabel* to_label1 = new QLabel(" - ");
    to_label1->setAlignment(Qt::AlignCenter);
    QLabel* to_label2 = new QLabel(" - ");
    to_label2->setAlignment(Qt::AlignCenter);
    QLabel* to_label3 = new QLabel(" - ");
    to_label3->setAlignment(Qt::AlignCenter);
    advancedpanel_layout->addWidget(new QLabel("Exclude nonstitchables:"), 0, 0, 1, 1);
    advancedpanel_layout->addWidget(excludenonstitchables_cbox, 0, 1, 1, 11);
    advancedpanel_layout->addWidget(volumeportion_label, 1, 0, 1, 1);
    QHBoxLayout* subvolLayout = new QHBoxLayout();
    QLabel* rowLabel = new QLabel("rows: ");
    rowLabel->setFont(QFont("", 8));
    subvolLayout->addWidget(rowLabel);
    subvolLayout->addWidget(row0_field);
    subvolLayout->addWidget(to_label1);
    subvolLayout->addWidget(row1_field);
    subvolLayout->addSpacing(12);
    QLabel* colLabel = new QLabel("cols: ");
    colLabel->setFont(QFont("", 8));
    subvolLayout->addWidget(colLabel);
    subvolLayout->addWidget(col0_field);
    subvolLayout->addWidget(to_label2);
    subvolLayout->addWidget(col1_field);
    subvolLayout->addSpacing(12);
    QLabel* sliceLabel = new QLabel("slices: ");
    sliceLabel->setFont(QFont("", 8));
    subvolLayout->addWidget(sliceLabel);
    subvolLayout->addWidget(slice0_field,1);
    subvolLayout->addWidget(to_label3);
    subvolLayout->addWidget(slice1_field,1);
    advancedpanel_layout->addLayout(subvolLayout, 1, 1, 1, 11);
    advancedpanel_layout->addWidget(blendingalgo_label, 2, 0, 1, 1);
    advancedpanel_layout->addWidget(blendingalbo_cbox, 2, 1, 1, 6);
    advancedpanel_layout->addWidget(restoreSPIM_label, 3, 0, 1, 1);
    advancedpanel_layout->addWidget(restoreSPIM_cbox, 3, 1, 1, 6);
    advancedpanel_layout->addWidget(imgformat_label, 4, 0, 1, 1);
    advancedpanel_layout->addWidget(imgformat_cbox, 4, 1, 1, 1);
    advancedpanel_layout->addWidget(imgdepth_label, 4, 2, 1, 1);
    advancedpanel_layout->addWidget(imgdepth_cbox, 4, 3, 1, 1);
    advancedpanel_layout->setVerticalSpacing(2);
    advancedpanel_layout->setContentsMargins(10,0,10,0);
    advanced_panel->setLayout(advancedpanel_layout);

    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);
    layout->addWidget(basic_panel);
    layout->addWidget(advanced_panel);
    layout->setSpacing(0);
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
    connect(showAdvancedButton, SIGNAL(toggled(bool)), this, SLOT(showAdvancedChanged(bool)));

    reset();
}


PTabMergeTiles::~PTabMergeTiles()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabMergeTiles destroyed\n", this->thread()->currentThreadId());
    #endif
}

//reset method
void PTabMergeTiles::reset()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabMergeTiles::reset()\n", this->thread()->currentThreadId());
    #endif

    savedir_field->setText("Enter or select the directory where to save the stitched volume.");
    for(int i=0; i<S_MAX_MULTIRES; i++)
    {
        resolutions_fields[i]->setText(QString("n.a. ").append(QChar(0x00D7)).append(QString(" n.a. ").append(QChar(0x00D7)).append(" n.a.")));
        resolutions_sizes[i]->setText("n.a.");
        resolutions_save_cboxs[i]->setChecked(true);
        resolutions_view_cboxs[i]->setChecked(false);
    }
    multistack_cbox->setChecked(true);
    stackheight_field->setMinimum(256);
    stackheight_field->setMaximum(4096);
    stackheight_field->setValue(512);
    stackwidth_field->setMinimum(256);
    stackwidth_field->setMaximum(4096);
    stackwidth_field->setValue(512);
    memocc_field->setText("");
    excludenonstitchables_cbox->setChecked(false);

    row0_field->setMinimum(0);
    row0_field->setValue(0);
    row1_field->setMinimum(0);
    row1_field->setValue(0);
    col0_field->setMinimum(0);
    col0_field->setValue(0);
    col1_field->setMinimum(0);
    col1_field->setValue(0);
    slice0_field->setMinimum(0);
    slice0_field->setValue(0);
    slice1_field->setMinimum(0);
    slice1_field->setValue(0);
    row0_field->setMinimum(0);
    row0_field->setMaximum(0);
    row0_field->setValue(0);
    row1_field->setMinimum(0);
    row1_field->setMaximum(0);
    row1_field->setValue(0);
    col0_field->setMinimum(0);
    col0_field->setMaximum(0);
    col0_field->setValue(0);
    col1_field->setMinimum(0);
    col1_field->setMaximum(0);
    col1_field->setValue(0);
    multistack_cbox->setChecked(true);

    showAdvancedButton->setChecked(false);
    advanced_panel->setVisible(false);

    setEnabled(false);
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
            PMain::instance()->setToReady();
            return;
        }

        //if basic mode is active, automatically performing the hidden steps (Projecting, Thresholding, Placing) if necessary
        if(PMain::instance()->modeBasicAction->isChecked())
        {
            //asking confirmation to continue if no displacements were found
            if(PMain::instance()->tabDisplProj->total_displ_number_field->text().toInt() == 0 &&
               QMessageBox::information(this, "Warning", "No computed displacements found. \n\nDisplacements will be generated using nominal stage coordinates.", "Continue", "Cancel"))
            {
                PMain::instance()->setToReady();
                return;
            }

            //performing operation
            StackedVolume* volume = CImport::instance()->getVolume();
            if(!volume)
                throw MyException("Unable to start this step. A volume must be properly imported first.");
            StackStitcher stitcher(volume);
            stitcher.projectDisplacements();
            stitcher.thresholdDisplacements(PMain::instance()->tabDisplThres->threshold_field->value());
            stitcher.computeTilesPlacement(PMain::instance()->tabPlaceTiles->algo_cbox->currentIndex());

            //enabling (and updating) other tabs
            PTabDisplProj::getInstance()->setEnabled(true);
            PTabDisplThresh::getInstance()->setEnabled(true);
            PTabPlaceTiles::getInstance()->setEnabled(true);
            PTabMergeTiles::getInstance()->setEnabled(true);
        }

        //disabling import form and enabling progress bar animation and tab wait animation
        PMain::instance()->getProgressBar()->setEnabled(true);
        PMain::instance()->getProgressBar()->setMinimum(0);
        PMain::instance()->getProgressBar()->setMaximum(100);
        PMain::instance()->closeVolumeAction->setEnabled(false);
        PMain::instance()->exitAction->setEnabled(false);
        wait_movie->start();
        if(PMain::instance()->modeBasicAction->isChecked())
            container->getTabBar()->setTabButton(2, QTabBar::LeftSide, wait_label);
        else
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
        PMain::instance()->setToReady();
    }
    catch(...)
    {
        QMessageBox::critical(this,QObject::tr("Error"), "Unknown error has occurred",QObject::tr("Ok"));
        PMain::instance()->setToReady();
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
    PMain::instance()->setToReady();
    wait_movie->stop();   
    if(PMain::instance()->modeBasicAction->isChecked())
        container->getTabBar()->setTabButton(2, QTabBar::LeftSide, 0);
    else
        container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);

    PMain::instance()->closeVolumeAction->setEnabled(true);
    PMain::instance()->exitAction->setEnabled(true);
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

    //obtaining volume's directory
    QFileDialog dialog(0);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
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
                resolutions_fields[i]->setText(QString::number(width).append(" ").append(QChar(0x00D7)).append(" ").append(QString::number(height)).append(" ").append(QChar(0x00D7)).append(" ").append(QString::number(depth)));
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
* Called when <multistack_cbox> or <signlestack_cbox> state changed.
***********************************************************************************/
void PTabMergeTiles::volumeformat_changed()
{
    stackheight_field->setVisible(multistack_cbox->isChecked());
    stackwidth_field->setVisible(multistack_cbox->isChecked());
    byLabel->setVisible(multistack_cbox->isChecked());

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
    else
    {
        if(img)
        {
            v3dhandle new_win = PMain::instance()->getV3D_env()->newImageWindow(img->getFileName());
            PMain::instance()->getV3D_env()->setImage(new_win, img);

            //showing operation successful message
            QMessageBox::information(this, "Operation successful", "Step successfully performed!", QMessageBox::Ok);
        }
    }


    //resetting some widgets
    PMain::instance()->closeVolumeAction->setEnabled(true);
    PMain::instance()->exitAction->setEnabled(true);
    PMain::instance()->setToReady();
    wait_movie->stop();
    if(PMain::instance()->modeBasicAction->isChecked())
        container->getTabBar()->setTabButton(2, QTabBar::LeftSide, 0);
    else
        container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
}

/**********************************************************************************
* Called when <showAdvancedButton> status changed
***********************************************************************************/
void PTabMergeTiles::showAdvancedChanged(bool status)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabMergeTiles::showAdvancedChanged(%s)\n", this->thread()->currentThreadId(), (status? "true" : "false"));
    #endif

    advanced_panel->setVisible(status);
}
