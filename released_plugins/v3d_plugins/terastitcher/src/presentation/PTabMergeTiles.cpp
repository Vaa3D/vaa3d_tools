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
#include "iomanager.config.h"
#include "vmStackedVolume.h"
#include "vmBlockVolume.h"
#include "PMain.h"
#include "src/control/CImport.h"
#include "src/control/CMergeTiles.h"
#include "StackStitcher.h"
#include "S_config.h"
#include "IOPluginAPI.h"

using namespace terastitcher;

/*********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "istance(...)"
**********************************************************************************/
PTabMergeTiles* PTabMergeTiles::uniqueInstance = 0;
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
    savedir_label = new QLabel("Save to:");
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
    volumeformat_label      = new QLabel("Format:");
    vol_format_cbox = new QComboBox();
    vol_format_cbox->setFont(QFont("", 8));
    vol_format_cbox->setEditable(true);
    vol_format_cbox->lineEdit()->setReadOnly(true);
    vol_format_cbox->lineEdit()->setAlignment(Qt::AlignCenter);
    vol_format_cbox->addItem("--- Volume format ---");
    vol_format_cbox->addItem("2Dseries");
    vol_format_cbox->addItem("3Dseries");
    std::vector <std::string> volformats = vm::VirtualVolumeFactory::registeredPluginsList();
    for(int i=0; i<volformats.size(); i++)
        vol_format_cbox->addItem(volformats[i].c_str());
    for(int i = 0; i < vol_format_cbox->count(); i++)
        vol_format_cbox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    PMain::setEnabledComboBoxItem(vol_format_cbox, 0, false);

    imout_plugin_cbox = new QComboBox();
    imout_plugin_cbox->setFont(QFont("", 8));
    std::vector<std::string> ioplugins = iom::IOPluginFactory::registeredPluginsList();
    imout_plugin_cbox->addItem("--- I/O plugin: ---");
    for(int i=0; i<ioplugins.size(); i++)
        imout_plugin_cbox->addItem(ioplugins[i].c_str());
    imout_plugin_cbox->setEditable(true);
    imout_plugin_cbox->lineEdit()->setReadOnly(true);
    imout_plugin_cbox->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < imout_plugin_cbox->count(); i++)
        imout_plugin_cbox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    PMain::setEnabledComboBoxItem(imout_plugin_cbox, 0, false);

    block_height_field       = new QSpinBox();
    block_height_field->setAlignment(Qt::AlignCenter);
    block_height_field->setMinimum(-1);
    block_height_field->setMaximum(4096);
    block_height_field->setValue(512);
    block_height_field->setSuffix(" (height)");
    block_height_field->setFont(QFont("", 9));
    block_width_field        = new QSpinBox();
    block_width_field->setAlignment(Qt::AlignCenter);
    block_width_field->setMinimum(-1);
    block_width_field->setMaximum(4096);
    block_width_field->setValue(512);
    block_width_field->setSuffix(" (width)");
    block_width_field->setFont(QFont("", 9));
    block_depth_field        = new QSpinBox();
    block_depth_field->setAlignment(Qt::AlignCenter);
    block_depth_field->setMinimum(-1);
    block_depth_field->setMaximum(1024);
    block_depth_field->setValue(256);
    block_depth_field->setSuffix(" (depth)");
    block_depth_field->setFont(QFont("", 9));
    memocc_field = new QLineEdit();
    memocc_field->setReadOnly(true);
    memocc_field->setAlignment(Qt::AlignLeft);
    memocc_field->setFont(QFont("", 9));
    memocc_field->setStyleSheet("background-color: #ACDCA5");
    memocc_field->setTextMargins(5,0,0,0);
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
    row0_field->setPrefix("[");
    row1_field = new QSpinBox();
    row1_field->setAlignment(Qt::AlignCenter);
    row1_field->setMinimum(-1);
    row1_field->setValue(-1);
    row1_field->setFont(QFont("", 9));
    row1_field->setSuffix("]");
    col0_field = new QSpinBox();
    col0_field->setAlignment(Qt::AlignCenter);
    col0_field->setMinimum(-1);
    col0_field->setValue(-1);
    col0_field->setFont(QFont("", 9));
    col0_field->setPrefix("[");
    col1_field = new QSpinBox();
    col1_field->setAlignment(Qt::AlignCenter);
    col1_field->setMinimum(-1);
    col1_field->setValue(-1);
    col1_field->setFont(QFont("", 9));
    col1_field->setSuffix("]");
    slice0_field = new QSpinBox();
    slice0_field->setAlignment(Qt::AlignCenter);
    slice0_field->setMinimum(-1);
    slice0_field->setMaximum(-1);
    slice0_field->setValue(-1);
    slice0_field->setFont(QFont("", 9));
    slice0_field->setPrefix("[");
    slice1_field = new QSpinBox();
    slice1_field->setAlignment(Qt::AlignCenter);
    slice1_field->setMinimum(-1);
    slice1_field->setMaximum(-1);
    slice1_field->setValue(-1);
    slice1_field->setFont(QFont("", 9));
    slice1_field->setSuffix("]");
    excludenonstitchables_cbox = new QCheckBox("stitchables only");
    excludenonstitchables_cbox->setFont(QFont("", 9));
    blendingalgo_label = new QLabel("Blending:");
    blendingalbo_cbox = new QComboBox();
    blendingalbo_cbox->insertItem(0, "No Blending");
    blendingalbo_cbox->insertItem(1, "Sinusoidal Blending");
    blendingalbo_cbox->insertItem(2, "No Blending with emphasized stacks borders");
    blendingalbo_cbox->setEditable(true);
    blendingalbo_cbox->lineEdit()->setReadOnly(true);
    blendingalbo_cbox->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < blendingalbo_cbox->count(); i++)
        blendingalbo_cbox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    blendingalbo_cbox->setCurrentIndex(1);
    blendingalbo_cbox->setFont(QFont("", 9));
    restoreSPIM_label = new QLabel("remove SPIM artifacts: ");
    restoreSPIM_label->setFont(QFont("", 9));
    restoreSPIM_cbox = new QComboBox();
    restoreSPIM_cbox->insertItem(0, "None");
    restoreSPIM_cbox->insertItem(1, "Zebrated pattern (Y)");
    restoreSPIM_cbox->insertItem(2, "Zebrated pattern (X)");
    restoreSPIM_cbox->insertItem(3, "Zebrated pattern (Z)");
    restoreSPIM_cbox->setEditable(true);
    restoreSPIM_cbox->lineEdit()->setReadOnly(true);
    restoreSPIM_cbox->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < restoreSPIM_cbox->count(); i++)
        restoreSPIM_cbox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    restoreSPIM_cbox->setFont(QFont("", 9));
    imgformat_label = new QLabel("");
    img_format_cbox = new QComboBox();
    img_format_cbox->insertItem(0, "tif");
    img_format_cbox->insertItem(1, "tiff");
    img_format_cbox->insertItem(2, "v3draw");
    img_format_cbox->insertItem(3, "png");
    img_format_cbox->insertItem(4, "bmp");
    img_format_cbox->insertItem(5, "jpeg");
    img_format_cbox->insertItem(6, "jpg");
    img_format_cbox->insertItem(7, "dib");
    img_format_cbox->insertItem(8, "pbm");
    img_format_cbox->insertItem(9, "pgm");
    img_format_cbox->insertItem(10, "ppm");
    img_format_cbox->insertItem(11, "sr");
    img_format_cbox->insertItem(12, "ras");
    img_format_cbox->setFont(QFont("", 9));
    img_format_cbox->setEditable(true);
    img_format_cbox->lineEdit()->setReadOnly(true);
    img_format_cbox->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < img_format_cbox->count(); i++)
        img_format_cbox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    imgdepth_cbox = new QComboBox();
    imgdepth_cbox->insertItem(0, "8 bits");
    imgdepth_cbox->insertItem(1, "16 bits");
    imgdepth_cbox->setFont(QFont("", 9));
    imgdepth_cbox->setEditable(true);
    imgdepth_cbox->lineEdit()->setReadOnly(true);
    imgdepth_cbox->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < imgdepth_cbox->count(); i++)
        imgdepth_cbox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    channel_selection = new QComboBox();
    channel_selection->addItem("all channels");
    channel_selection->addItem("channel R");
    channel_selection->addItem("channel G");
    channel_selection->addItem("channel B");
    channel_selection->setFont(QFont("", 8));
    channel_selection->setEditable(true);
    channel_selection->lineEdit()->setReadOnly(true);
    channel_selection->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < channel_selection->count(); i++)
        channel_selection->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    connect(channel_selection, SIGNAL(currentIndexChanged(int)),this, SLOT(channelSelectedChanged(int)));



    /*** LAYOUT SECTIONS ***/
    //basic settings panel
    QVBoxLayout* basicpanel_layout = new QVBoxLayout();
    basicpanel_layout->setContentsMargins(0,0,0,0);
    int left_margin = 80;
    /**/
    QHBoxLayout* basic_panel_row_1 = new QHBoxLayout();
    basic_panel_row_1->setContentsMargins(0,0,0,0);
    basic_panel_row_1->setSpacing(0);
    savedir_label->setFixedWidth(left_margin);
    browse_button->setFixedWidth(80);
    basic_panel_row_1->addWidget(savedir_label);
    basic_panel_row_1->addWidget(savedir_field,1);
    basic_panel_row_1->addWidget(browse_button);
    basicpanel_layout->addLayout(basic_panel_row_1);
    /**/
    basicpanel_layout->addSpacing(10);
    /**/
    QGridLayout* basic_panel_row_2 = new QGridLayout();
    basic_panel_row_2->setContentsMargins(0,0,0,0);
    basic_panel_row_2->setSpacing(0);
    basic_panel_row_2->setVerticalSpacing(0);
    basic_panel_row_2->addWidget(resolutions_label,             0,      1,   1,              6);
    resolutions_label->setFixedWidth(200);
    basic_panel_row_2->addWidget(resolutions_size_label,        0,      7,   1,              3);
    resolutions_size_label->setFixedWidth(120);
    basic_panel_row_2->addWidget(resolutions_save_label,        0,      10,  1,              1);
    basic_panel_row_2->addWidget(resolutions_view_label,        0,      11,  1,              1);
    outputs_label->setFixedWidth(left_margin);
    basic_panel_row_2->addWidget(outputs_label,                 1,      0,   S_MAX_MULTIRES, 1);
    for(int i=0; i<S_MAX_MULTIRES; i++)
    {
        resolutions_fields[i]->setFont(QFont("", 9));
        resolutions_fields[i]->setFixedWidth(200);
        resolutions_sizes[i]->setFont(QFont("", 9));
        resolutions_sizes[i]->setFixedWidth(120);
        basic_panel_row_2->addWidget(resolutions_fields[i],     1+i,    1,  1, 6);
        basic_panel_row_2->addWidget(resolutions_sizes[i],      1+i,    7,  1, 3);
        basic_panel_row_2->addWidget(resolutions_save_cboxs[i], 1+i,    10, 1, 1);
        basic_panel_row_2->addWidget(resolutions_view_cboxs[i], 1+i,    11, 1, 1);
    }
    basicpanel_layout->addLayout(basic_panel_row_2);
    /**/
    basicpanel_layout->addSpacing(10);
    /**/
    QHBoxLayout* basic_panel_row_3 = new QHBoxLayout();
    basic_panel_row_3->setContentsMargins(0,0,0,0);
    basic_panel_row_3->setSpacing(0);
    volumeformat_label->setFixedWidth(left_margin);
    basic_panel_row_3->addWidget(volumeformat_label);
    vol_format_cbox->setFixedWidth(150);
    basic_panel_row_3->addWidget(vol_format_cbox);
    imout_plugin_cbox->setFixedWidth(130);
    basic_panel_row_3->addSpacing(10);
    basic_panel_row_3->addWidget(imout_plugin_cbox);
    basic_panel_row_3->addSpacing(20);
    basic_panel_row_3->addWidget(block_height_field, 1);
    basic_panel_row_3->addSpacing(5);
    basic_panel_row_3->addWidget(block_width_field, 1);
    basic_panel_row_3->addSpacing(5);
    basic_panel_row_3->addWidget(block_depth_field, 1);
    basicpanel_layout->addLayout(basic_panel_row_3);
    /**/
    QHBoxLayout* basic_panel_row_4 = new QHBoxLayout();
    basic_panel_row_4->setContentsMargins(0,0,0,0);
    basic_panel_row_4->setSpacing(0);
    imgformat_label->setFixedWidth(left_margin);
    basic_panel_row_4->addWidget(imgformat_label);
    img_format_cbox->setFixedWidth(80);
    basic_panel_row_4->addWidget(img_format_cbox);
    basic_panel_row_4->addSpacing(5);
    imgdepth_cbox->setFixedWidth(90);
    basic_panel_row_4->addWidget(imgdepth_cbox);
    basic_panel_row_4->addSpacing(5);
    channel_selection->setFixedWidth(110);
    basic_panel_row_4->addWidget(channel_selection);
    basic_panel_row_4->addSpacing(20);
    basic_panel_row_4->addWidget(memocc_field, 1);
    basicpanel_layout->addLayout(basic_panel_row_4);
    basicpanel_layout->addSpacing(5);
    /**/
    basicpanel_layout->addWidget(showAdvancedButton);
    /**/
    basicpanel_layout->setContentsMargins(10,0,10,0);
    basic_panel->setLayout(basicpanel_layout);

    //advanced settings panel
    QVBoxLayout* advancedpanel_layout = new QVBoxLayout();
    /**/
    QHBoxLayout* advancedpanel_row1 = new QHBoxLayout();
    advancedpanel_row1->setSpacing(0);
    advancedpanel_row1->setContentsMargins(0,0,0,0);
    QLabel* selection_label = new QLabel("Selection:");
    selection_label->setFixedWidth(left_margin);
    advancedpanel_row1->addWidget(selection_label);
    QLabel* rowLabel = new QLabel(" (rows)");
    rowLabel->setFont(QFont("", 8));
    QLabel* colLabel = new QLabel(" (cols)");
    colLabel->setFont(QFont("", 8));
    QLabel* sliceLabel = new QLabel(" (slices)");
    sliceLabel->setFont(QFont("", 8));
    rowLabel->setFixedWidth(60);
    colLabel->setFixedWidth(60);
    sliceLabel->setFixedWidth(60);
    row0_field->setFixedWidth(50);
    row1_field->setFixedWidth(50);
    col0_field->setFixedWidth(50);
    col1_field->setFixedWidth(50);
    slice0_field->setFixedWidth(75);
    slice1_field->setFixedWidth(75);
    advancedpanel_row1->addWidget(row0_field);
    advancedpanel_row1->addWidget(row1_field);
    advancedpanel_row1->addWidget(rowLabel);
    advancedpanel_row1->addWidget(col0_field);
    advancedpanel_row1->addWidget(col1_field);
    advancedpanel_row1->addWidget(colLabel);
    advancedpanel_row1->addWidget(slice0_field);
    advancedpanel_row1->addWidget(slice1_field);
    advancedpanel_row1->addWidget(sliceLabel);
    advancedpanel_row1->addStretch(1);
    advancedpanel_row1->addWidget(excludenonstitchables_cbox);
    advancedpanel_layout->addLayout(advancedpanel_row1);
    /**/
    QHBoxLayout* advancedpanel_row2 = new QHBoxLayout();
    advancedpanel_row2->setSpacing(0);
    advancedpanel_row2->setContentsMargins(0,0,0,0);
    blendingalgo_label->setFixedWidth(left_margin);
    advancedpanel_row2->addWidget(blendingalgo_label);
    blendingalbo_cbox->setFixedWidth(260);
    advancedpanel_row2->addWidget(blendingalbo_cbox);
    advancedpanel_row2->addSpacing(60);
    advancedpanel_row2->addWidget(restoreSPIM_label);
    advancedpanel_row2->addWidget(restoreSPIM_cbox);
    advancedpanel_layout->addLayout(advancedpanel_row2);
    /**/
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
    connect(vol_format_cbox, SIGNAL(currentIndexChanged(QString)), this, SLOT(volumeformat_changed(QString)));
    connect(imout_plugin_cbox, SIGNAL(currentIndexChanged(QString)), this, SLOT(imout_plugin_changed(QString)));
    for(int i=0; i<S_MAX_MULTIRES; i++)
    {
        connect(resolutions_save_cboxs[i], SIGNAL(stateChanged(int)), this, SLOT(updateContent()));
        connect(resolutions_save_cboxs[i], SIGNAL(stateChanged(int)), this, SLOT(save_changed(int)));
        connect(resolutions_view_cboxs[i], SIGNAL(stateChanged(int)), this, SLOT(viewinVaa3D_changed(int)));
    }
    connect(CMergeTiles::instance(), SIGNAL(sendOperationOutcome(iom::exception*, Image4DSimple*)), this, SLOT(merging_done(iom::exception*, Image4DSimple*)), Qt::QueuedConnection);
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

    vol_format_cbox->setCurrentIndex(0);
    imout_plugin_cbox->setCurrentIndex(0);
//    int index = vol_format_cbox->findText(BlockVolume::id.c_str());
//    if ( index != -1 )
//       vol_format_cbox->setCurrentIndex(index);

//    index = imout_plugin_cbox->findText("tiff3D");
//        if ( index != -1 )
//           imout_plugin_cbox->setCurrentIndex(index);

    block_height_field->setMinimum(-1);
    block_height_field->setMaximum(4096);
    block_height_field->setValue(512);
    block_width_field->setMinimum(-1);
    block_width_field->setMaximum(4096);
    block_width_field->setValue(512);
    block_depth_field->setMinimum(-1);
    block_depth_field->setMaximum(1024);
    block_depth_field->setValue(256);
    memocc_field->setText("Memory usage: ");
    excludenonstitchables_cbox->setChecked(false);

    row0_field->setMinimum(-1);
    row0_field->setValue(-1);
    row1_field->setMinimum(-1);
    row1_field->setValue(-1);
    col0_field->setMinimum(-1);
    col0_field->setValue(-1);
    col1_field->setMinimum(-1);
    col1_field->setValue(-1);
    slice0_field->setMinimum(-1);
    slice0_field->setMaximum(-1);
    slice0_field->setValue(-1);
    slice1_field->setMinimum(-1);
    slice1_field->setMaximum(-1);
    slice1_field->setValue(-1);
    row0_field->setMinimum(-1);
    row0_field->setMaximum(-1);
    row0_field->setValue(-1);
    row1_field->setMinimum(-1);
    row1_field->setMaximum(-1);
    row1_field->setValue(-1);
    col0_field->setMinimum(-1);
    col0_field->setMaximum(-1);
    col0_field->setValue(-1);
    col1_field->setMinimum(-1);
    col1_field->setMaximum(-1);
    col1_field->setValue(-1);

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
            throw iom::exception("A volume must be properly imported first. Please perform the Import step.");

        // check user input
        if(imout_plugin_cbox->currentIndex() == 0)
            throw iom::exception("Please select an image I/O plugin from the combolist");
        if(vol_format_cbox->currentIndex() == 0)
            throw iom::exception("Please select the volume format from the combolist");

        //verifying that directory is readable
        QDir directory(savedir_field->text());
        if(!directory.isReadable())
            throw iom::exception(QString("Cannot open directory\n \"").append(savedir_field->text()).append("\"").toStdString().c_str());

        //asking confirmation to continue when saving to a non-empty dir
        QStringList dir_entries = directory.entryList();
        if(dir_entries.size() > 2 && QMessageBox::information(this, "Warning", "The directory you selected is NOT empty. \n\nIf you continue, the merging "
                                               "process could fail if the directories to be created already exist in the given path.", "Continue", "Cancel"))
        {
            PMain::instance()->setToReady();
            return;
        }

        //if basic mode is active, automatically performing the hidden steps (Projecting, Thresholding, Placing) if necessary
//        if(PMain::instance()->modeBasicAction->isChecked())
//        {
//            //asking confirmation to continue if no displacements were found
//            if(PMain::instance()->tabDisplProj->total_displ_number_field->text().toInt() == 0 &&
//               QMessageBox::information(this, "Warning", "No computed displacements found. \n\nDisplacements will be generated using nominal stage coordinates.", "Continue", "Cancel"))
//            {
//                PMain::instance()->setToReady();
//                return;
//            }

//            //performing operation
//            StackedVolume* volume = CImport::instance()->getVolume();
//            if(!volume)
//                throw iom::exception("Unable to start this step. A volume must be properly imported first.");
//            //Alessandro 2013-07-08: this causes crash and it is not needed (nominal stage coordinates are already ready for merging)
////            StackStitcher stitcher(volume);
////            stitcher.projectDisplacements();
////            stitcher.thresholdDisplacements(PMain::instance()->tabDisplThres->threshold_field->value());
////            stitcher.computeTilesPlacement(PMain::instance()->tabPlaceTiles->algo_cbox->currentIndex());

//            //enabling (and updating) other tabs
//            PTabDisplProj::getInstance()->setEnabled(true);
//            PTabDisplThresh::getInstance()->setEnabled(true);
//            PTabPlaceTiles::getInstance()->setEnabled(true);
//            PTabMergeTiles::getInstance()->setEnabled(true);
//        }

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

        //propagating options and parameters and launching task
        CMergeTiles::instance()->setPMergeTiles(this);
        for(int i=0; i<S_MAX_MULTIRES; i++)
        {
            CMergeTiles::instance()->setResolution(i, resolutions_save_cboxs[i]->isChecked());
            if(vol_format_cbox->currentIndex()==0 && resolutions_view_cboxs[i]->isChecked())
                CMergeTiles::instance()->setResolutionToShow(i);
        }
        CMergeTiles::instance()->start();
    }
    catch(iom::exception &ex)
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
    catch(iom::exception &ex) {QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));}
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
    /**/tsp::debug(tsp::LEV_MAX, 0, __tsp__current__function__);

    //then filling widget fields
    if(enabled && CImport::instance()->getVolume())
    {
        vm::VirtualVolume* volume = CImport::instance()->getVolume();

        //inserting volume dimensions
        QWidget::setEnabled(false);
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
        slice0_field->setMaximum(volume->getN_SLICES()-1);
        slice0_field->setMinimum(0);
        slice0_field->setValue(0);
        slice1_field->setMaximum(volume->getN_SLICES()-1);
        slice1_field->setMinimum(0);
        slice1_field->setValue(volume->getN_SLICES()-1);
        volumeformat_changed(vol_format_cbox->currentText());
        QWidget::setEnabled(true);

        //updating content
        updateContent();
    }
    else
        QWidget::setEnabled(enabled);
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
        if(this->isEnabled() && CImport::instance()->getVolume())
            updateContent();
    }
    catch(iom::exception &ex)
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
        if(this->isEnabled() && CImport::instance()->getVolume())
            updateContent();
    }
    catch(iom::exception &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

/**********************************************************************************
* Updates widgets contents
***********************************************************************************/
void PTabMergeTiles::updateContent()
{
    /**/tsp::debug(tsp::LEV_MAX, 0, __tsp__current__function__);

    try
    {
        if(this->isEnabled() && CImport::instance()->getVolume())
        {
            vm::VirtualVolume* volume = CImport::instance()->getVolume();

            StackStitcher stitcher(volume);
            stitcher.computeVolumeDims(excludenonstitchables_cbox->isChecked(), row0_field->value(), row1_field->value(),
                                       col0_field->value(), col1_field->value(), slice0_field->value(), slice1_field->value()+1);

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
            memocc_field->setText(QString("Memory usage: ")+QString::number(MBytes, 'f', 0).append(" MB"));

            // update ranges
            slice0_field->setValue(stitcher.getD0());
            slice1_field->setValue(stitcher.getD1()-1);
            row0_field->setValue(stitcher.getROW0());
            row1_field->setValue(stitcher.getROW1());
            col0_field->setValue(stitcher.getCOL0());
            col1_field->setValue(stitcher.getCOL1());
        }
    }
    catch(iom::exception &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), strprintf("An error occurred while preparing the stitcher for the Merging step: \n\n\"%s\"\n\nPlease check the previous steps before you can perform the Merging step.", ex.what()).c_str(),QObject::tr("Ok"));
        this->setEnabled(false);
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
void PTabMergeTiles::volumeformat_changed(QString str)
{
    if(str.compare("2Dseries") == 0)
    {
        vm::VOLUME_OUTPUT_FORMAT_PLUGIN = StackedVolume::id;
        block_height_field->setEnabled(false);
        block_height_field->setValue(-1);
        block_width_field->setEnabled(false);
        block_width_field->setValue(-1);
        block_depth_field->setEnabled(false);
        block_depth_field->setValue(-1);
    }
    else if(str.compare("3Dseries") == 0)
    {
        vm::VOLUME_OUTPUT_FORMAT_PLUGIN = BlockVolume::id;
        block_height_field->setEnabled(false);
        block_height_field->setValue(-1);
        block_width_field->setEnabled(false);
        block_width_field->setValue(-1);
        block_depth_field->setEnabled(true);
        block_depth_field->setValue(256);
    }
    else if(str.compare(StackedVolume::id.c_str()) == 0)
    {
        vm::VOLUME_OUTPUT_FORMAT_PLUGIN = StackedVolume::id;
        block_height_field->setEnabled(true);
        block_height_field->setValue(512);
        block_width_field->setEnabled(true);
        block_width_field->setValue(512);
        block_depth_field->setEnabled(false);
        block_depth_field->setValue(-1);
    }
    else if(str.compare(BlockVolume::id.c_str()) == 0)
    {
        vm::VOLUME_OUTPUT_FORMAT_PLUGIN = BlockVolume::id;
        block_height_field->setEnabled(true);
        block_height_field->setValue(512);
        block_width_field->setEnabled(true);
        block_width_field->setValue(512);
        block_depth_field->setEnabled(true);
        block_depth_field->setValue(256);
    }

    for(int i=0; i<S_MAX_MULTIRES; i++)
        resolutions_view_cboxs[i]->setEnabled(str.compare(StackedVolume::id.c_str()) == 0);

//    PMain::setEnabledComboBoxItem(img_format_cbox, 2, i == 2);
//    PMain::setEnabledComboBoxItem(img_format_cbox, 3, i != 2);
//    PMain::setEnabledComboBoxItem(img_format_cbox, 4, i != 2);
//    PMain::setEnabledComboBoxItem(img_format_cbox, 5, i != 2);
//    PMain::setEnabledComboBoxItem(img_format_cbox, 6, i != 2);
//    PMain::setEnabledComboBoxItem(img_format_cbox, 7, i != 2);
//    PMain::setEnabledComboBoxItem(img_format_cbox, 8, i != 2);
//    PMain::setEnabledComboBoxItem(img_format_cbox, 9, i != 2);
//    PMain::setEnabledComboBoxItem(img_format_cbox, 10, i != 2);
//    PMain::setEnabledComboBoxItem(img_format_cbox, 11, i != 2);
//    PMain::setEnabledComboBoxItem(img_format_cbox, 12, i != 2);

//    if(i == 2 && img_format_cbox->currentIndex() >= 3)
//        img_format_cbox->setCurrentIndex(0);
//    else if(i < 2 && img_format_cbox->currentIndex() == 2)
//        img_format_cbox->setCurrentIndex(0);


}

/**********************************************************************************
* Called when <imout_plugin_cbox> state changed
***********************************************************************************/
void PTabMergeTiles::imout_plugin_changed(QString str)
{
    iom::IMOUT_PLUGIN = str.toStdString();
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
void PTabMergeTiles::merging_done(iom::exception *ex, Image4DSimple* img)
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

/**********************************************************************************
* Called when "channel_selection" state has changed.
***********************************************************************************/
void PTabMergeTiles::channelSelectedChanged(int c)
{
    iom::CHANS = iom::channel(c);
}
