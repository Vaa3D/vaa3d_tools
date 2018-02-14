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

/******************
*    CHANGELOG    *
*******************
* 2017-05-03. Giulio.     @ADDED taking into account the isotropic flag in computing resolutions sizes
*/


#include "PTabMergeTiles.h"
#include "iomanager.config.h"
#include "vmStackedVolume.h"
#include "vmBlockVolume.h"
#include "PTeraStitcher.h"
#include "CImportUnstitched.h"
#include "CMergeTiles.h"
#include "StackStitcher.h"
#include "S_config.h"
#include "IOPluginAPI.h"
#include <QButtonGroup>
#include <QVBoxLayout>

using namespace terastitcher;

std::string PTabMergeTiles::metadata_path_null = "Enter or select image metadata .ims file (optional)";
std::string PTabMergeTiles::external_compressor_line_null = "Compressor options (not enabled yet)";
//std::string PTabMergeTiles::external_compressor_line_null = "Compressor options (optional, format: ID[:opt1:opt2:...])";

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

#if defined Q_OS_MAC
	QFont smallFont;
#elif defined Q_OS_WIN
	QFont smallFont;//("", 7);
#else 
	QFont smallFont;//("", 8);
#endif

    //basic panel widgets
    basic_panel = new QWidget();
    savedir_label = new QLabel("Save to:");
    savedir_field = new QLineEdit();
    savedir_field->setFont(smallFont);
	outDirButton = new QPushButton("Browse for dir...", this);

	outFileButton = new QPushButton("Browse for file...", this);
    resolutions_label = new QLabel(QString("Resolution (X ").append(QChar(0x00D7)).append(" Y ").append(QChar(0x00D7)).append(" Z)"));
    resolutions_label->setFont(smallFont);
    resolutions_label->setAlignment(Qt::AlignCenter);
    resolutions_size_label = new QLabel("Size (GVoxels)");
    resolutions_size_label->setFont(smallFont);
    resolutions_size_label->setAlignment(Qt::AlignCenter);
    resolutions_save_label = new QLabel("Save to disk");
    resolutions_save_label->setFont(smallFont);
    resolutions_save_label->setAlignment(Qt::AlignCenter);
    outputs_label      = new QLabel("Outputs:");
    outputs_label->setAlignment(Qt::AlignVCenter);
    resolutions_save_selection = new QButtonGroup();
    resolutions_save_selection->setExclusive(false);
    for(int i=0; i<n_max_resolutions; i++)
    {
        resolutions_fields[i] = new QLabel();
        resolutions_fields[i]->setAlignment(Qt::AlignCenter);
        resolutions_sizes[i] = new QLabel();
        resolutions_sizes[i]->setAlignment(Qt::AlignCenter);
        resolutions_save_cboxs[i] = new QCheckBox("");
        resolutions_save_cboxs[i]->setChecked(i==0);
        resolutions_save_cboxs[i]->setStyleSheet("::indicator {subcontrol-position: center; subcontrol-origin: padding;}");
        resolutions_save_selection->addButton(resolutions_save_cboxs[i]);
    }
    volumeformat_label      = new QLabel("Format:");
    vol_format_cbox = new QComboBox();
    vol_format_cbox->setFont(smallFont);
    vol_format_cbox->setEditable(true);
    vol_format_cbox->lineEdit()->setReadOnly(true);
    vol_format_cbox->lineEdit()->setAlignment(Qt::AlignCenter);
	vol_format_cbox->addItem("--- Select a format ---");
	vol_format_cbox->addItem(iim::SIMPLE_FORMAT.c_str());
	vol_format_cbox->addItem(iim::STACKED_FORMAT.c_str());
	vol_format_cbox->addItem(iim::TILED_TIF3D_FORMAT.c_str());
	vol_format_cbox->addItem(iim::TILED_MC_TIF3D_FORMAT.c_str());
	vol_format_cbox->addItem(iim::TIF3D_FORMAT.c_str());
	vol_format_cbox->addItem(iim::BDV_HDF5_FORMAT.c_str());
	vol_format_cbox->addItem(iim::IMS_HDF5_FORMAT.c_str());
	vol_format_cbox->addItem(iim::SIMPLE_RAW_FORMAT.c_str());
	vol_format_cbox->addItem(iim::STACKED_RAW_FORMAT.c_str());
	vol_format_cbox->addItem(iim::TILED_FORMAT.c_str());
	vol_format_cbox->addItem(iim::TILED_MC_FORMAT.c_str());
	//vol_format_cbox->addItemiim::RAW_FORMAT.c_str());
    //vol_format_cbox->addItem("2Dseries");
    //vol_format_cbox->addItem("3Dseries");

    //std::vector <std::string> volformats = vm::VirtualVolumeFactory::registeredPluginsList();
    //for(int i=0; i<volformats.size(); i++)
    //    vol_format_cbox->addItem(volformats[i].c_str());
    for(int i = 0; i < vol_format_cbox->count(); i++)
        vol_format_cbox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    PTeraStitcher::setEnabledComboBoxItem(vol_format_cbox, 0, false);

    block_height_field       = new QSpinBox();
    block_height_field->setAlignment(Qt::AlignCenter);
    block_height_field->setMinimum(-1);
    block_height_field->setMaximum(4096);
    block_height_field->setValue(512);
    block_height_field->setSuffix(" (height)");
    block_height_field->setFont(smallFont);
    block_width_field        = new QSpinBox();
    block_width_field->setAlignment(Qt::AlignCenter);
    block_width_field->setMinimum(-1);
    block_width_field->setMaximum(4096);
    block_width_field->setValue(512);
    block_width_field->setSuffix(" (width)");
    block_width_field->setFont(smallFont);
    block_depth_field        = new QSpinBox();
    block_depth_field->setAlignment(Qt::AlignCenter);
    block_depth_field->setMinimum(-1);
    block_depth_field->setMaximum(1024);
    block_depth_field->setValue(256);
    block_depth_field->setSuffix(" (depth)");
    block_depth_field->setFont(smallFont);
    memocc_field = new QLineEdit();
    memocc_field->setReadOnly(true);
    memocc_field->setAlignment(Qt::AlignLeft);
    memocc_field->setFont(smallFont);
    memocc_field->setStyleSheet("background-color: #ACDCA5");
    memocc_field->setTextMargins(5,0,0,0);
	libtiff_uncompressed_checkbox = new QCheckBox("uncompressed", this);
	libtiff_uncompressed_checkbox->setToolTip("Disable libTIFF compression");
	libtiff_bigtiff_checkbox = new QCheckBox("bigtiff", this);
	libtiff_bigtiff_checkbox->setToolTip("Force BigTIFF mode for files > 4 GB");
	isotropic_checkbox = new QCheckBox("isotropic", this);
	isotropic_checkbox->setToolTip("Generate lowest resolution with voxels as much isotropic as possible. Use this flag when the high resolution image has highy anistropic voxels");
    isotropic_checkbox->setChecked(true);
	showAdvancedButton = new QPushButton(QString("Advanced options ").append(QChar(0x00BB)), this);
    showAdvancedButton->setCheckable(true);
	mdata_line = new QLineEdit(this);
	mdata_line->setFont(smallFont);
	mdata_browse_button = new QPushButton ("...", this);
	external_compressor_line = new QLineEdit(this);
	external_compressor_line->setFont(smallFont);

    //advanced panel widgets
    advanced_panel = new QWidget();
    y0_field = new QSpinBox();
    y0_field->setAlignment(Qt::AlignCenter);
    y0_field->setFont(smallFont);
    y0_field->setPrefix("[");
    y1_field = new QSpinBox();
    y1_field->setAlignment(Qt::AlignCenter);
    y1_field->setValue(-1);
    y1_field->setFont(smallFont);
    y1_field->setSuffix("]");
    x0_field = new QSpinBox();
    x0_field->setAlignment(Qt::AlignCenter);
    x0_field->setFont(smallFont);
    x0_field->setPrefix("[");
    x1_field = new QSpinBox();
    x1_field->setAlignment(Qt::AlignCenter);
    x1_field->setFont(smallFont);
    x1_field->setSuffix("]");
    z0_field = new QSpinBox();
    z0_field->setAlignment(Qt::AlignCenter);
    z0_field->setFont(smallFont);
    z0_field->setPrefix("[");
    z1_field = new QSpinBox();
    z1_field->setAlignment(Qt::AlignCenter);
    z1_field->setFont(smallFont);
    z1_field->setSuffix("]");
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
    blendingalbo_cbox->setFont(smallFont);
    restoreSPIM_label = new QLabel("Artifacts removal:");
    restoreSPIM_label->setFont(smallFont);
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
    restoreSPIM_cbox->setFont(smallFont);
    /*imgdepth_cbox = new QComboBox();
    imgdepth_cbox->insertItem(0, "8 bits");
    imgdepth_cbox->insertItem(1, "16 bits");
    imgdepth_cbox->setFont(smallFont);
    imgdepth_cbox->setEditable(true);
    imgdepth_cbox->lineEdit()->setReadOnly(true);
    imgdepth_cbox->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < imgdepth_cbox->count(); i++)
        imgdepth_cbox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);*/
    channel_selection = new QComboBox();



    /*** LAYOUT SECTIONS ***/
    //basic settings panel
    QVBoxLayout* basicpanel_layout = new QVBoxLayout();
	basicpanel_layout->setContentsMargins(0,0,0,0);
	basicpanel_layout->setSpacing(5);
    int left_margin = 130;
    /**/
    QHBoxLayout* basic_panel_row_1 = new QHBoxLayout();
    basic_panel_row_1->setContentsMargins(0,0,0,0);
    //basic_panel_row_1->setSpacing(0);
    savedir_label->setFixedWidth(left_margin);
    basic_panel_row_1->addWidget(savedir_label);
	basic_panel_row_1->addWidget(savedir_field,1);
	basic_panel_row_1->addWidget(outDirButton);
	basic_panel_row_1->addWidget(outFileButton);
	basic_panel_row_1->setContentsMargins(0,0,0,0);
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
    outputs_label->setFixedWidth(left_margin);
    basic_panel_row_2->addWidget(outputs_label,                 1,      0,   n_max_resolutions, 1);
    for(int i=0; i<n_max_resolutions; i++)
    {
        resolutions_fields[i]->setFont(smallFont);
        resolutions_fields[i]->setFixedWidth(200);
        resolutions_sizes[i]->setFont(smallFont);
        resolutions_sizes[i]->setFixedWidth(120);
        basic_panel_row_2->addWidget(resolutions_fields[i],     1+i,    1,  1, 6);
        basic_panel_row_2->addWidget(resolutions_sizes[i],      1+i,    7,  1, 3);
        basic_panel_row_2->addWidget(resolutions_save_cboxs[i], 1+i,    10, 1, 1);
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
    vol_format_cbox->setFixedWidth(300);
	basic_panel_row_3->setAlignment(Qt::AlignLeft);
    basic_panel_row_3->addWidget(vol_format_cbox);
    basic_panel_row_3->addSpacing(20);
    basic_panel_row_3->addWidget(block_height_field, 1);
    basic_panel_row_3->addSpacing(5);
    basic_panel_row_3->addWidget(block_width_field, 1);
    basic_panel_row_3->addSpacing(5);
	basic_panel_row_3->addWidget(block_depth_field, 1);
	basic_panel_row_3->addWidget(mdata_line, 1);
	basic_panel_row_3->addSpacing(5);
	basic_panel_row_3->addWidget(mdata_browse_button);
	//basic_panel_row_3->addStretch(1);
    basicpanel_layout->addLayout(basic_panel_row_3);
    /**/
    QHBoxLayout* basic_panel_row_4 = new QHBoxLayout();
	basic_panel_row_4->setAlignment(Qt::AlignLeft);
    basic_panel_row_4->setContentsMargins(0,0,0,0);
    basic_panel_row_4->setSpacing(0);
    basic_panel_row_4->addSpacing(left_margin);
	basic_panel_row_4->addWidget(memocc_field);
	memocc_field->setFixedWidth(300);
	basic_panel_row_4->addSpacing(25);
	basic_panel_row_4->addWidget(external_compressor_line, 1);
	basicpanel_layout->addLayout(basic_panel_row_4);
	basicpanel_layout->addSpacing(5);
    /**/
	/*QHBoxLayout* basic_panel_row_5 = new QHBoxLayout();
	basic_panel_row_5->setContentsMargins(0,0,0,0);
	basic_panel_row_5->setSpacing(0);
	basic_panel_row_5->addSpacing(left_margin);
	libtiff_uncompressed_checkbox->setFixedWidth(140);
	libtiff_bigtiff_checkbox->setFixedWidth(140);
	basic_panel_row_5->addWidget(libtiff_uncompressed_checkbox);
	basic_panel_row_5->setSpacing(10);
	basic_panel_row_5->addWidget(libtiff_bigtiff_checkbox);
	basic_panel_row_5->addStretch(1);
	basicpanel_layout->addLayout(basic_panel_row_5);*/
	/**/
    basicpanel_layout->addWidget(showAdvancedButton);
    /**/
    basicpanel_layout->setContentsMargins(10,0,10,0);
    basic_panel->setLayout(basicpanel_layout);

    //advanced settings panel
    QVBoxLayout* advancedpanel_layout = new QVBoxLayout();
    /**/
    QHBoxLayout* advancedpanel_row1 = new QHBoxLayout();
    advancedpanel_row1->setSpacing(10);
    advancedpanel_row1->setContentsMargins(0,0,0,0);
    QLabel* selection_label = new QLabel("XYZC selection:");
    selection_label->setFixedWidth(left_margin);
    advancedpanel_row1->addWidget(selection_label);
    QLabel* yRangeLabel = new QLabel(" (Y)");
    yRangeLabel->setFont(smallFont);
    QLabel* xRangeLabel = new QLabel(" (X)");
    xRangeLabel->setFont(smallFont);
    QLabel* zRangeLabel = new QLabel(" (Z)");
	zRangeLabel->setFont(smallFont);
	QLabel* cRangeLabel = new QLabel(" (C)");
	cRangeLabel->setFont(smallFont);
    //yRangeLabel->setFixedWidth(60);
   // xRangeLabel->setFixedWidth(60);
    //zRangeLabel->setFixedWidth(60);
    advancedpanel_row1->addWidget(x0_field);
    advancedpanel_row1->addWidget(x1_field);
	advancedpanel_row1->addWidget(xRangeLabel);
	advancedpanel_row1->addSpacing(20);
	advancedpanel_row1->addWidget(y0_field);
	advancedpanel_row1->addWidget(y1_field);
	advancedpanel_row1->addWidget(yRangeLabel);
	advancedpanel_row1->addSpacing(20);
    advancedpanel_row1->addWidget(z0_field);
    advancedpanel_row1->addWidget(z1_field);
	advancedpanel_row1->addWidget(zRangeLabel);
	advancedpanel_row1->addSpacing(20);
	advancedpanel_row1->addWidget(channel_selection, 1);
	advancedpanel_row1->addWidget(cRangeLabel);
    //advancedpanel_row1->addStretch(1);
    advancedpanel_layout->addLayout(advancedpanel_row1);
    /**/
    QHBoxLayout* advancedpanel_row2 = new QHBoxLayout();
    advancedpanel_row2->setSpacing(0);
    advancedpanel_row2->setContentsMargins(0,0,0,0);
    blendingalgo_label->setFixedWidth(left_margin);
    advancedpanel_row2->addWidget(blendingalgo_label);
    //blendingalbo_cbox->setFixedWidth(300);
	advancedpanel_row2->addWidget(blendingalbo_cbox);
	blendingalbo_cbox->setFixedWidth(300);
	advancedpanel_row2->addStretch(1);
	advancedpanel_layout->addLayout(advancedpanel_row2);
	/**/
	QHBoxLayout* advancedpanel_row3 = new QHBoxLayout();
	restoreSPIM_label->setFixedWidth(left_margin);
	advancedpanel_row3->setSpacing(0);
	advancedpanel_row3->setContentsMargins(0,0,0,0);
    advancedpanel_row3->addWidget(restoreSPIM_label);
	advancedpanel_row3->addWidget(restoreSPIM_cbox);
	restoreSPIM_cbox->setFixedWidth(300);
	advancedpanel_row3->addStretch(1);
    advancedpanel_layout->addLayout(advancedpanel_row3);
    /**/
	QHBoxLayout* advancedpanel_row4 = new QHBoxLayout();
	QLabel* other_options_label = new QLabel("Other options:", this);
	other_options_label->setFixedWidth(left_margin);
	advancedpanel_row4->setSpacing(10);
	advancedpanel_row4->addWidget(other_options_label);
	advancedpanel_row4->addWidget(libtiff_uncompressed_checkbox);
	advancedpanel_row4->addWidget(libtiff_bigtiff_checkbox);
	advancedpanel_row4->addWidget(isotropic_checkbox);
	advancedpanel_row4->addStretch(1);
	advancedpanel_layout->addLayout(advancedpanel_row4);
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
	connect(outDirButton, SIGNAL(clicked()), this, SLOT(browse_button_clicked()));
	connect(outFileButton, SIGNAL(clicked()), this, SLOT(browse_button_clicked()));
    connect(y0_field, SIGNAL(valueChanged(int)), this, SLOT(y0_field_changed(int)));
    connect(y1_field, SIGNAL(valueChanged(int)), this, SLOT(y1_field_changed(int)));
    connect(x0_field, SIGNAL(valueChanged(int)), this, SLOT(x0_field_changed(int)));
    connect(x1_field, SIGNAL(valueChanged(int)), this, SLOT(x1_field_changed(int)));
    connect(z0_field, SIGNAL(valueChanged(int)), this, SLOT(updateContent()));
    connect(z0_field, SIGNAL(valueChanged(int)), this, SLOT(z0_field_changed(int)));
    connect(z1_field, SIGNAL(valueChanged(int)), this, SLOT(updateContent()));
    connect(z1_field, SIGNAL(valueChanged(int)), this, SLOT(z1_field_changed(int)));
    connect(vol_format_cbox, SIGNAL(currentIndexChanged(QString)), this, SLOT(volumeformat_changed(QString)));
    for(int i=0; i<n_max_resolutions; i++)
        connect(resolutions_save_cboxs[i], SIGNAL(stateChanged(int)), this, SLOT(updateContent()));
	connect(CMergeTiles::instance(), SIGNAL(sendOperationOutcome(iom::exception*)), this, SLOT(merging_done(iom::exception*)), Qt::QueuedConnection);
	connect(showAdvancedButton, SIGNAL(toggled(bool)), this, SLOT(showAdvancedChanged(bool)));
	connect(qobject_cast<QStandardItemModel *>(channel_selection->model()), SIGNAL(itemChanged(QStandardItem *)), this, SLOT(channelSelectionChanged(QStandardItem *)));
    connect(mdata_browse_button, SIGNAL(clicked()), this, SLOT(metadata_browse_button_clicked()));

	// 2017-05-03. Giulio. Connected a handler to the isotropic check box
    connect(isotropic_checkbox, SIGNAL(stateChanged(int)), this, SLOT(updateContent()));
	
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

    savedir_field->setText("Enter or select the folder/file where to save the stitched volume.");
    for(int i=0; i<n_max_resolutions; i++)
    {
        resolutions_fields[i]->setText(QString("n.a. ").append(QChar(0x00D7)).append(QString(" n.a. ").append(QChar(0x00D7)).append(" n.a.")));
        resolutions_sizes[i]->setText("n.a.");
        resolutions_save_cboxs[i]->setChecked(i==0);
    }

    vol_format_cbox->setCurrentIndex(0);

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

    showAdvancedButton->setChecked(false);
    advanced_panel->setVisible(false);

	volumeformat_changed(0);
    setEnabled(false);

	channel_selection->clear();

	mdata_line->setText(metadata_path_null.c_str());
	external_compressor_line->setText(external_compressor_line_null.c_str());
}

/*********************************************************************************
* Start/Stop methods associated to the current step.
* They are called by the startButtonClicked/stopButtonClicked methods of <PTeraStitcher>
**********************************************************************************/
void PTabMergeTiles::start()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabMergeTiles start() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        // first check that a volume has been properly imported
        if(!CImportUnstitched::instance()->getVolume())
            throw iom::exception("A volume must be properly imported first. Please perform the Import step.");

        // check user input
        if(vol_format_cbox->currentIndex() == 0)
            throw iom::exception("Please select the volume format from the pull-down menu");

		// check at least one channel is enabled
		bool channel_enabled = false;
		QStandardItemModel * ch_model = qobject_cast<QStandardItemModel *>(channel_selection->model());
		for(int i=0; i<ch_model->rowCount() && !channel_enabled; i++)
			if(ch_model->item(i)->checkState() == Qt::Checked)
				channel_enabled = true;
		if(!channel_enabled)
			throw iom::exception("Please select at least one channel from the channel selection pull-down menu");

		if(outDirButton->isEnabled())
		{
			// check that directory is readable
			QDir directory(savedir_field->text());
			if(!directory.isReadable())
				throw iom::exception(QString("Cannot open directory\n \"").append(savedir_field->text()).append("\"").toStdString().c_str());

			// ask confirmation to continue when saving to a non-empty dir
			QStringList dir_entries = directory.entryList();
			if(dir_entries.size() > 2 && QMessageBox::information(this, "Warning", "The directory you selected is NOT empty. \n\nIf you continue, the merging "
												   "process could fail if the directories to be created already exist in the given path.", "Continue", "Cancel"))
			{
				PTeraStitcher::instance()->setToReady();
				return;
			}
		}
		else if(iim::isFile(savedir_field->text().toStdString()))
		{
			// ask confirmation to continue when overwriting an existing file
			if(QMessageBox::information(this, "Warning", "The file already exists and will be overwritten.", "Continue", "Cancel"))
			{
				PTeraStitcher::instance()->setToReady();
				return;
			}
		}

        //disable import form and enable progress bar animation and tab wait animation
        PTeraStitcher::instance()->getProgressBar()->setEnabled(true);
        PTeraStitcher::instance()->getProgressBar()->setMinimum(0);
        PTeraStitcher::instance()->getProgressBar()->setMaximum(100);
        PTeraStitcher::instance()->closeVolumeAction->setEnabled(false);
        PTeraStitcher::instance()->exitAction->setEnabled(false);
        wait_movie->start();
        if(PTeraStitcher::instance()->modeBasicAction->isChecked())
            container->getTabBar()->setTabButton(2, QTabBar::LeftSide, wait_label);
        else
            container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, wait_label);

        // propagate options and parameters
        CMergeTiles::instance()->setPMergeTiles(this);
        for(int i=0; i<n_max_resolutions; i++)
            CMergeTiles::instance()->setResolution(i, resolutions_save_cboxs[i]->isChecked());

		// set active channels
		CMergeTiles::instance()->resetActiveChannels();
		if (ch_model->item(0)->checkState() != Qt::Checked)	// do nothing if 'All channels' is checked
		{
			for(int i=1; i<ch_model->rowCount(); i++)
				if(ch_model->item(i)->checkState() == Qt::Checked)
					CMergeTiles::instance()->setActiveChannel(i-1);
		}

		// perform task in a separate thread
        CMergeTiles::instance()->start();
    }
    catch(iom::exception &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PTeraStitcher::instance()->setToReady();
    }
    catch(...)
    {
        QMessageBox::critical(this,QObject::tr("Error"), "Unknown error has occurred",QObject::tr("Ok"));
        PTeraStitcher::instance()->setToReady();
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
    PTeraStitcher::instance()->setToReady();
    wait_movie->stop();   
    if(PTeraStitcher::instance()->modeBasicAction->isChecked())
        container->getTabBar()->setTabButton(2, QTabBar::LeftSide, 0);
    else
        container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);

    PTeraStitcher::instance()->closeVolumeAction->setEnabled(true);
    PTeraStitcher::instance()->exitAction->setEnabled(true);
}

/**********************************************************************************
* Overrides QWidget's setEnabled(bool).
* If the widget is enabled, its fields are filled with the informations provided by
* the <StackedVolume> object of <CImport> instance.
***********************************************************************************/
void PTabMergeTiles::setEnabled(bool enabled)
{
    /**/ts::debug(ts::LEV_MAX, 0, __tsp__current__function__);

    //then filling widget fields
    if(enabled && CImportUnstitched::instance()->getVolume() && CMergeTiles::instance()->unstitchedVolume())
    {
        UnstitchedVolume* volume = CMergeTiles::instance()->unstitchedVolume();

        //inserting volume dimensions
        QWidget::setEnabled(false);
        y0_field->setMinimum(0);
        y0_field->setMaximum(volume->getDIM_V()-1);
        y0_field->setValue(0);
        y1_field->setMinimum(0);
        y1_field->setMaximum(volume->getDIM_V()-1);
        y1_field->setValue(volume->getDIM_V()-1);
        x0_field->setMinimum(0);
        x0_field->setMaximum(volume->getDIM_H()-1);
        x0_field->setValue(0);
        x1_field->setMinimum(0);
        x1_field->setMaximum(volume->getDIM_H()-1);
        x1_field->setValue(volume->getDIM_H()-1);
        z0_field->setMaximum(volume->getDIM_D()-1);
        z0_field->setMinimum(0);
        z0_field->setValue(0);
        z1_field->setMaximum(volume->getDIM_D()-1);
        z1_field->setMinimum(0);
        z1_field->setValue(volume->getDIM_D()-1);
        volumeformat_changed(vol_format_cbox->currentText());
        QWidget::setEnabled(true);

		// update channel selection
		QStandardItemModel * ch_model = qobject_cast<QStandardItemModel *>(channel_selection->model());
		for(int c=0; c<=volume->getDIM_C(); c++)
		{
			if(c)
				channel_selection->addItem(QString("Channel ") + QString::number(c));
			else
				channel_selection->addItem("All channels");
			ch_model->item(c)->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			ch_model->item(c)->setData(Qt::Checked, Qt::CheckStateRole);
			ch_model->item(c)->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
		}
		channel_selection->setEditable(true);
		channel_selection->lineEdit()->setReadOnly(true);
		channel_selection->lineEdit()->setAlignment(Qt::AlignCenter);

        //updating content
        updateContent();
    }
    else
        QWidget::setEnabled(enabled);
}

/**********************************************************************************
* Called when the checkboxes of the channel QComboBox's QStandardItemModel changed.
***********************************************************************************/
void PTabMergeTiles::channelSelectionChanged(QStandardItem * item)
{
	QStandardItemModel * ch_model = qobject_cast<QStandardItemModel *>(channel_selection->model());
	if (ch_model->item(0)->checkState() == Qt::Checked)
	{
		for(int i=1; i<ch_model->rowCount(); i++)
			ch_model->item(i)->setCheckState(Qt::Checked);
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

	if(QObject::sender() == outDirButton)
	{
		//obtaining volume's directory
		QFileDialog dialog(this);
		dialog.setFileMode(QFileDialog::DirectoryOnly);
		dialog.setAcceptMode(QFileDialog::AcceptOpen);
		//dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
		dialog.setDirectory(CImportUnstitched::instance()->getVolume()->getSTACKS_DIR());
		dialog.setWindowTitle("Select an EMPTY directory");
		if (dialog.exec())
		{
			QStringList fileNames = dialog.selectedFiles();
			QString folder_path = fileNames.first();

			QDir directory(folder_path);
			QStringList dir_entries = directory.entryList();
			if(dir_entries.size() <= 2)
				savedir_field->setText(folder_path);
			else
			{
				if(!QMessageBox::information(this, "Warning", "The directory you selected is NOT empty. \n\nIf you continue, the merging "
												   "process could fail if the directories to be created already exist in the given path.", "Continue", "Cancel"))
					savedir_field->setText(folder_path);
			}
		}
	}
	else
	{
		QFileDialog dialog(this);
		dialog.setFileMode(QFileDialog::AnyFile);
		dialog.setWindowTitle("Save as");
		if(vol_format_cbox->currentText().toStdString() == iim::BDV_HDF5_FORMAT)
			dialog.setNameFilter( tr("BigDataViewer HDF5 files (*.h5 *.H5)") );
		else if(vol_format_cbox->currentText().toStdString() == iim::IMS_HDF5_FORMAT)
			dialog.setNameFilter( tr("Imaris HDF5 files (*.ims *.IMS)") );
		dialog.setAcceptMode(QFileDialog::AcceptSave);
		dialog.setDirectory(CImportUnstitched::instance()->getVolume()->getSTACKS_DIR());
		if (dialog.exec())
		{
			QStringList fileNames = dialog.selectedFiles();
			QString file_path = fileNames.first();
			if(!file_path.endsWith(".h5", Qt::CaseInsensitive) && vol_format_cbox->currentText().toStdString() == iim::BDV_HDF5_FORMAT)
				file_path.append(".h5");
			else if(!file_path.endsWith(".ims", Qt::CaseInsensitive) && vol_format_cbox->currentText().toStdString() == iim::IMS_HDF5_FORMAT)
				file_path.append(".ims");
			savedir_field->setText(file_path);
		}
	}
}

/**********************************************************************************
* Updates widgets contents
***********************************************************************************/
void PTabMergeTiles::updateContent()
{
    /**/ts::debug(ts::LEV_MAX, 0, __tsp__current__function__);

    try
    {
        if(this->isEnabled() && CImportUnstitched::instance()->getVolume() && CMergeTiles::instance()->unstitchedVolume())
        {
            UnstitchedVolume* volume = CMergeTiles::instance()->unstitchedVolume();

			//2017-05-03. Giulio. set advanced halving rules
			int halve_pow2[N_MAX_RESOLUTIONS];
			
			if ( isotropic_checkbox->isChecked() && vol_format_cbox->currentText().toStdString() != iim::BDV_HDF5_FORMAT && vol_format_cbox->currentText().toStdString() != iim::IMS_HDF5_FORMAT ) {
				// an isotropic image must be generated
				float vxlsz_Vx2 = 2*(CImportUnstitched::instance()->getVolume()->getVXL_V() > 0 ? CImportUnstitched::instance()->getVolume()->getVXL_V() : -(CImportUnstitched::instance()->getVolume()->getVXL_V()));
				float vxlsz_Hx2 = 2*(CImportUnstitched::instance()->getVolume()->getVXL_H() > 0 ? CImportUnstitched::instance()->getVolume()->getVXL_H() : -(CImportUnstitched::instance()->getVolume()->getVXL_H()));
				float vxlsz_D = CImportUnstitched::instance()->getVolume()->getVXL_D();
				halve_pow2[0] = 0;
				for ( int i=1; i<n_max_resolutions; i++ ) {
					halve_pow2[i] = halve_pow2[i-1];
					if ( vxlsz_D < std::max<float>(vxlsz_Vx2,vxlsz_Hx2) ) {
						halve_pow2[i]++;
						vxlsz_D   *= 2;
					}
					vxlsz_Vx2 *= 2;
					vxlsz_Hx2 *= 2;
				}
			}
			else {
				// halving along D dimension must be always performed
				for ( int i=0; i<n_max_resolutions; i++ )
					halve_pow2[i] = i;
			}

            int max_res = 0;
			for(int i=0; i<n_max_resolutions; i++)
			{
				int height = (y1_field->value()-y0_field->value()+1)/pow(2.0f, i);
                int width  = (x1_field->value()-x0_field->value()+1)/pow(2.0f, i);
                int depth  = (z1_field->value()-z0_field->value()+1)/pow(2.0f, halve_pow2[i]); // 2017-05-03. Giulio. advanced halving rules
                float GVoxels = (height/1024.0f)*(width/1024.0f)*(depth/1024.0f);
                resolutions_fields[i]->setText(QString::number(width).append(" ").append(QChar(0x00D7)).append(" ").append(QString::number(height)).append(" ").append(QChar(0x00D7)).append(" ").append(QString::number(depth)));
                resolutions_sizes[i]->setText(QString::number(GVoxels,'f',3));

				resolutions_fields[i]->setVisible(height && width && depth);
				resolutions_sizes[i]->setVisible(height && width && depth);
				resolutions_save_cboxs[i]->setVisible(height && width && depth);

				if(resolutions_save_cboxs[i]->isChecked())
					max_res = std::max(max_res, i);
            }

            //updating RAM usage estimation
			int layer_height = (y1_field->value()-y0_field->value()+1);
			int layer_width  = (x1_field->value()-x0_field->value()+1);
            int layer_depth = pow(2.0f, max_res);
            float MBytes = (layer_height/1024.0f)*(layer_width/1024.0f)*layer_depth*4;
            memocc_field->setText(QString("Memory usage: ")+QString::number(MBytes, 'f', 0).append(" MB"));

            // update ranges
           /* z0_field->setValue(0);
            z1_field->setValue(volume->getDIM_D()-1);
            y0_field->setValue(0);
            y1_field->setValue(volume->getDIM_V()-1);
            x0_field->setValue(0);
            x1_field->setValue(volume->getDIM_H()-1);*/
        }
    }
    catch(iom::exception &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), strprintf("An error occurred while preparing the stitcher for the Merge step: \n\n\"%s\"\n\nPlease check the previous steps before you can perform the Merge step.", ex.what()).c_str(),QObject::tr("Ok"));
        this->setEnabled(false);
	}
	catch(iim::IOException &ex)
	{
		QMessageBox::critical(this,QObject::tr("Error"), strprintf("An error occurred while preparing the stitcher for the Merge step: \n\n\"%s\"\n\nPlease check the previous steps before you can perform the Merge step.", ex.what()).c_str(),QObject::tr("Ok"));
		this->setEnabled(false);
	}
}

/**********************************************************************************
* Called when the corresponding spinboxes changed.
* New maximum/minimum values are set according to the status of spinboxes.
***********************************************************************************/
void PTabMergeTiles::y0_field_changed(int val){y1_field->setMinimum(val); updateContent();}
void PTabMergeTiles::y1_field_changed(int val){y0_field->setMaximum(val); updateContent();}
void PTabMergeTiles::x0_field_changed(int val){x1_field->setMinimum(val); updateContent();}
void PTabMergeTiles::x1_field_changed(int val){x0_field->setMaximum(val); updateContent();}
void PTabMergeTiles::z0_field_changed(int val){z1_field->setMinimum(val); updateContent();}
void PTabMergeTiles::z1_field_changed(int val){z0_field->setMaximum(val); updateContent();}

/**********************************************************************************
* Called when <multistack_cbox> or <signlestack_cbox> state changed.
***********************************************************************************/
void PTabMergeTiles::volumeformat_changed(QString str)
{
   std::string stdstr = str.toStdString();
   if(
		stdstr.compare(iim::STACKED_RAW_FORMAT) == 0 || 
		stdstr.compare(iim::STACKED_FORMAT) == 0)
    {
		block_height_field->setEnabled(true);
		block_height_field->setVisible(true);
		block_width_field->setEnabled(true);
		block_width_field->setVisible(true);
		block_depth_field->setEnabled(false);
		block_depth_field->setVisible(false);
		block_height_field->setValue(512);
		block_width_field->setValue(512);
		block_depth_field->setValue(-1);
    }
	else if(
		stdstr.compare(iim::TILED_FORMAT) == 0 || 
		stdstr.compare(iim::TILED_MC_FORMAT) == 0 || 
		stdstr.compare(iim::TILED_TIF3D_FORMAT) == 0 || 
		stdstr.compare(iim::TILED_MC_TIF3D_FORMAT) == 0)
    {
		block_height_field->setEnabled(true);
		block_height_field->setVisible(true);
		block_width_field->setEnabled(true);
		block_width_field->setVisible(true);
		block_depth_field->setEnabled(true);
		block_depth_field->setVisible(true);
		block_height_field->setValue(512);
		block_width_field->setValue(512);
		block_depth_field->setValue(512);
    }
	else
	{
		block_height_field->setEnabled(false);
		block_height_field->setVisible(false);
		block_width_field->setEnabled(false);
		block_width_field->setVisible(false);
		block_depth_field->setEnabled(false);
		block_depth_field->setVisible(false);
		block_height_field->setValue(-1);
		block_width_field->setValue(-1);
		block_depth_field->setValue(-1);
	}

	if( stdstr.compare(iim::BDV_HDF5_FORMAT) == 0 || 
		stdstr.compare(iim::IMS_HDF5_FORMAT) == 0)
	{
		outDirButton->setVisible(false);
		outDirButton->setEnabled(false);
		outFileButton->setVisible(true);
		outFileButton->setEnabled(true);
	}
	else
	{
		outDirButton->setVisible(true);
		outDirButton->setEnabled(true);
		outFileButton->setVisible(false);
		outFileButton->setEnabled(false);
	}

	if(stdstr.compare(iim::IMS_HDF5_FORMAT) == 0)
	{
		mdata_line->setVisible(true);
		mdata_line->setEnabled(true);
		mdata_browse_button->setVisible(true);
		mdata_browse_button->setEnabled(true);
		external_compressor_line->setVisible(true);
		external_compressor_line->setEnabled(true);
	}
	else
	{
		mdata_line->setVisible(false);
		mdata_line->setEnabled(false);
		mdata_browse_button->setVisible(false);
		mdata_browse_button->setEnabled(false);
		external_compressor_line->setVisible(false);
		external_compressor_line->setEnabled(false);
	}

	libtiff_bigtiff_checkbox->setEnabled(stdstr.find("TIFF") != std::string::npos);
	libtiff_uncompressed_checkbox->setEnabled(stdstr.find("TIFF") != std::string::npos);
	
	// 2017-05-03. Giulio. if output format change the sizes of the resolutions could change and must be updated
	updateContent();
}


/**********************************************************************************
* Called by <CMergeTiles> when the associated operation has been performed.
* If an exception has occurred in the <CMergeTiles> thread,it is propagated and man-
* aged in the current thread (ex != 0). Otherwise, if a valid  3D image  is passed,
* it is shown in Vaa3D.
***********************************************************************************/
void PTabMergeTiles::merging_done(iom::exception *ex)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabMergeTiles merging_done(%s) launched\n", this->thread()->currentThreadId(), (ex? "ex" : "NULL"));
    #endif

    //if an exception has occurred, showing a message error
    if(ex)
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
    else
    {
		//showing operation successful message
		QMessageBox::information(this, "Operation successful", "Merge step successfully performed!", QMessageBox::Ok);
    }


    //resetting some widgets
    PTeraStitcher::instance()->closeVolumeAction->setEnabled(true);
    PTeraStitcher::instance()->exitAction->setEnabled(true);
    PTeraStitcher::instance()->setToReady();
    wait_movie->stop();
    if(PTeraStitcher::instance()->modeBasicAction->isChecked())
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
* Opens the dialog to select the image metadata file
* Called when user clicks on "mdata_browse_button".
***********************************************************************************/
void PTabMergeTiles::metadata_browse_button_clicked()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	//dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
	dialog.setWindowTitle("Select image metadata file");
	dialog.setDirectory(QDir::currentPath());
	dialog.setNameFilter(tr("Imaris files (*.ims *.IMS)"));
	if(dialog.exec())
		if(!dialog.selectedFiles().empty())
			mdata_line->setText(dialog.selectedFiles().front());
}