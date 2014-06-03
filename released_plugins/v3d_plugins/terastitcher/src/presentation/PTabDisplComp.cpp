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

#include "PTabDisplComp.h"
#include "PTabDisplProj.h"
#include "PTabDisplThresh.h"
#include "MyException.h"
#include "vmStackedVolume.h"
#include "PMain.h"
#include "src/control/CImport.h"
#include "src/control/CDisplComp.h"

using namespace terastitcher;

/*********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "istance(...)"
**********************************************************************************/
PTabDisplComp* PTabDisplComp::uniqueInstance = NULL;
void PTabDisplComp::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

PTabDisplComp::PTabDisplComp(QMyTabWidget* _container, int _tab_index) : QWidget(), container(_container), tab_index(_tab_index)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplComp created\n", this->thread()->currentThreadId());
    #endif

    //creating Basic panel widgets
    basic_panel = new QWidget();
    saveproj_label = new QLabel("Save project XML to:");
    saveproj_field = new QLineEdit();
    saveproj_field->setFont(QFont("",8));
    browse_button = new QPushButton("...");
    algorithm_label = new QLabel("Algorithm:");
    algorithm_cbox = new QComboBox();
    algorithm_cbox->insertItem(0, "MIP-NCC");
    algorithm_cbox->setEditable(true);
    algorithm_cbox->lineEdit()->setReadOnly(true);
    algorithm_cbox->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < algorithm_cbox->count(); i++)
        algorithm_cbox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    subvoldims_label = new QLabel("Number of slices per layer:");
    subvoldims_sbox = new QSpinBox();
    subvoldims_sbox->setAlignment(Qt::AlignCenter);
    subvoldims_sbox->setMaximum(600);
    subvoldims_sbox->setMinimum(1);
    memocc_label = new QLabel("Estimated memory usage:");
    memocc_field = new QLineEdit();
    memocc_field->setReadOnly(true);
    memocc_field->setAlignment(Qt::AlignCenter);
    channel_selection = new QComboBox();
    channel_selection->addItem("all channels");
    channel_selection->addItem("R");
    channel_selection->addItem("G");
    channel_selection->addItem("B");
    channel_selection->setEditable(true);
    channel_selection->lineEdit()->setReadOnly(true);
    channel_selection->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < channel_selection->count(); i++)
        channel_selection->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    connect(channel_selection, SIGNAL(currentIndexChanged(int)),this, SLOT(channelSelectedChanged(int)));

    showAdvancedButton = new QPushButton(QString("Advanced options ").append(QChar(0x00BB)), this);
    showAdvancedButton->setCheckable(true);

    //creating Advanced panel widgets
    advanced_panel = new QWidget();
    stackrowstbp_label = new QLabel("Stacks rows to process:");
    startrow_sbox = new QSpinBox();
    startrow_sbox->setAlignment(Qt::AlignCenter);
    to_label_1 = new QLabel("to");
    to_label_1->setAlignment(Qt::AlignCenter);
    endrow_sbox = new QSpinBox();
    endrow_sbox->setAlignment(Qt::AlignCenter);
    stackcolstbp_label = new QLabel("Stacks columns to process:");
    startcol_sbox = new QSpinBox();
    startcol_sbox->setAlignment(Qt::AlignCenter);
    to_label_2 = new QLabel("to");
    to_label_2->setAlignment(Qt::AlignCenter);
    endcol_sbox = new QSpinBox();
    endcol_sbox->setAlignment(Qt::AlignCenter);
    searchregion_label = new QLabel("Search Region (voxels):");
    Ysearch_sbox = new QSpinBox();
    Ysearch_sbox->setAlignment(Qt::AlignCenter);
    Ysearch_sbox->setMinimum(5);
    Ysearch_sbox->setValue(20);
    Ysearch_sbox->setSuffix(" (Y)");
    Xsearch_sbox = new QSpinBox();
    Xsearch_sbox->setAlignment(Qt::AlignCenter);
    Xsearch_sbox->setMinimum(5);
    Xsearch_sbox->setValue(20);
    Xsearch_sbox->setSuffix(" (X)");
    Zsearch_sbox = new QSpinBox();
    Zsearch_sbox->setAlignment(Qt::AlignCenter);
    Zsearch_sbox->setMinimum(5);
    Zsearch_sbox->setValue(20);
    Zsearch_sbox->setSuffix(" (Z)");
    for_label_1 = new QLabel(QChar(0x00D7));
    for_label_1->setAlignment(Qt::AlignCenter);
    for_label_2 = new QLabel(QChar(0x00D7));
    for_label_2->setAlignment(Qt::AlignCenter);
    overlap_label = new QLabel("Overlap (voxels):");
    Yoverlap_sbox = new QSpinBox();
    Yoverlap_sbox->setAlignment(Qt::AlignCenter);
    Yoverlap_sbox->setSuffix(" (Y)");
    Xoverlap_sbox = new QSpinBox();
    Xoverlap_sbox->setAlignment(Qt::AlignCenter);
    Xoverlap_sbox->setSuffix(" (X)");
    for_label_3 = new QLabel(QChar(0x00D7));
    for_label_3->setAlignment(Qt::AlignCenter);
    restoreSPIM_label = new QLabel("SPIM artifacts removal:");
    restoreSPIM_cbox = new QCheckBox("Compute stacks profiles to be used in the Merging tiles step");

    /*** LAYOUT SECTIONS ***/
    //basic panel
    QGridLayout* basicpanel_layout = new QGridLayout();
    QHBoxLayout *tmp = new QHBoxLayout();
    basicpanel_layout->addWidget(saveproj_label,     0,0,1,1);
    saveproj_label->setFixedWidth(200);
    browse_button->setFixedWidth(80);
    tmp->addWidget(saveproj_field,1);
    tmp->addWidget(browse_button);
    tmp->setSpacing(6);
    basicpanel_layout->addLayout(tmp,                0,1,1,11);
    basicpanel_layout->addWidget(algorithm_label,    1,0,1,1);
    algorithm_cbox->setFixedWidth(150);
    channel_selection->setFixedWidth(150);
    basicpanel_layout->addWidget(algorithm_cbox,     1,1,1,2);
    basicpanel_layout->addWidget(subvoldims_label,   2,0,1,1);
    basicpanel_layout->addWidget(subvoldims_sbox,    2,1,1,2);
    QLabel* channel_label = new QLabel("Channel selection:");
    channel_label->setFixedWidth(200);
    basicpanel_layout->addWidget(channel_label,       3,0,1,1);
    basicpanel_layout->addWidget(channel_selection,   3,1,1,2);
    basicpanel_layout->addWidget(memocc_label,       4,0,1,1);
    basicpanel_layout->addWidget(memocc_field,       4,1,1,2);
    QWidget* emptyspace3 = new QWidget();
    emptyspace3->setFixedHeight(5);
    QWidget* emptyspace4 = new QWidget();
    emptyspace4->setFixedHeight(1);
    basicpanel_layout->addWidget(emptyspace3, 4+S_MAX_MULTIRES, 0, 1, 12);
    basicpanel_layout->addWidget(showAdvancedButton, 5+S_MAX_MULTIRES, 0, 1, 12);
    basicpanel_layout->addWidget(emptyspace4, 6+S_MAX_MULTIRES, 0, 1, 12);
    basicpanel_layout->setVerticalSpacing(2);
    basic_panel->setLayout(basicpanel_layout);
    basic_panel->setContentsMargins(0,0,0,0);


    //advanced panel
    QGridLayout* advancedpanel_layout = new QGridLayout();
    advancedpanel_layout->addWidget(stackrowstbp_label,     0,0,1,1);
    stackrowstbp_label->setFixedWidth(200);
    startrow_sbox->setFixedWidth(150);
    advancedpanel_layout->addWidget(startrow_sbox,          0,1,1,3);
    advancedpanel_layout->addWidget(to_label_1,             0,4,1,1);
    advancedpanel_layout->addWidget(endrow_sbox,            0,5,1,3);
    advancedpanel_layout->addWidget(stackcolstbp_label,     1,0,1,1);
    advancedpanel_layout->addWidget(startcol_sbox,          1,1,1,3);
    advancedpanel_layout->addWidget(to_label_2,             1,4,1,1);
    advancedpanel_layout->addWidget(endcol_sbox,            1,5,1,3);
    advancedpanel_layout->addWidget(searchregion_label,     2,0,1,1);
    advancedpanel_layout->addWidget(Xsearch_sbox,           2,1,1,3);
    advancedpanel_layout->addWidget(for_label_1,            2,4,1,1);
    advancedpanel_layout->addWidget(Ysearch_sbox,           2,5,1,3);
    advancedpanel_layout->addWidget(for_label_2,            2,8,1,1);
    advancedpanel_layout->addWidget(Zsearch_sbox,           2,9,1,3);
    advancedpanel_layout->addWidget(overlap_label,          3,0,1,1);
    advancedpanel_layout->addWidget(Xoverlap_sbox,          3,1,1,3);
    QLabel* for_label_4 = new QLabel(QChar(0x00D7));
    for_label_4->setAlignment(Qt::AlignCenter);
    advancedpanel_layout->addWidget(for_label_4,            3,4,1,1);
    advancedpanel_layout->addWidget(Yoverlap_sbox,          3,5,1,3);
    advancedpanel_layout->addWidget(restoreSPIM_label,      4,0,1,1);
    advancedpanel_layout->addWidget(restoreSPIM_cbox,       4,1,1,11);
    advancedpanel_layout->setVerticalSpacing(2);
    advanced_panel->setLayout(advancedpanel_layout);
    advanced_panel->setContentsMargins(0,0,0,0);



    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(basic_panel);
    layout->addWidget(advanced_panel);
    layout->addStretch(1);
    layout->setSpacing(0);
    layout->setContentsMargins(10,0,10,0);
    setLayout(layout);

    //wait animated GIF tab icon
    wait_movie = new QMovie(":/icons/wait.gif");
    wait_label = new QLabel(this);
    wait_label->setMovie(wait_movie);

    // signals and slots
    connect(subvoldims_sbox, SIGNAL(valueChanged(int)), this, SLOT(updateMemoryOccupancy(int)));
    connect(startrow_sbox, SIGNAL(valueChanged(int)), this, SLOT(updateMemoryOccupancy(int)));
    connect(endrow_sbox, SIGNAL(valueChanged(int)), this, SLOT(updateMemoryOccupancy(int)));
    connect(startcol_sbox, SIGNAL(valueChanged(int)), this, SLOT(updateMemoryOccupancy(int)));
    connect(endcol_sbox, SIGNAL(valueChanged(int)), this, SLOT(updateMemoryOccupancy(int)));
    connect(browse_button, SIGNAL(clicked()), this, SLOT(browse_button_clicked()));
    connect(CDisplComp::instance(), SIGNAL(sendOperationOutcome(MyException*)), this, SLOT(displcomp_done(MyException*)), Qt::QueuedConnection);
    connect(showAdvancedButton, SIGNAL(toggled(bool)), this, SLOT(showAdvancedChanged(bool)));

    reset();
}


PTabDisplComp::~PTabDisplComp()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplComp destroyed\n", this->thread()->currentThreadId());
    #endif
}

//reset method
void PTabDisplComp::reset()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplComp::reset()\n", this->thread()->currentThreadId());
    #endif

    Yoverlap_sbox->setMinimum(0);
    Yoverlap_sbox->setMaximum(0);
    Yoverlap_sbox->setValue(0);
    Xoverlap_sbox->setMinimum(0);
    Xoverlap_sbox->setMaximum(0);
    Xoverlap_sbox->setValue(0);
    startrow_sbox->setMinimum(0);
    startrow_sbox->setMaximum(0);
    startrow_sbox->setValue(0);
    endrow_sbox->setMinimum(0);
    endrow_sbox->setMaximum(0);
    endrow_sbox->setValue(0);
    startcol_sbox->setMinimum(0);
    startcol_sbox->setMaximum(0);
    startcol_sbox->setValue(0);
    endcol_sbox->setMinimum(0);
    endcol_sbox->setMaximum(0);
    endcol_sbox->setValue(0);
    saveproj_field->setText("");
    restoreSPIM_cbox->setChecked(false);
    subvoldims_sbox->setValue(100);
    memocc_field->setText("");

    showAdvancedButton->setChecked(false);
    advanced_panel->setVisible(false);

    setEnabled(false);
}

/*********************************************************************************
* Start/Stop methods associated to the current step.
* They are called by the startButtonClicked/stopButtonClicked methods of <PMain>
**********************************************************************************/
void PTabDisplComp::start()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplComp start() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        //first checking that a volume has been properly imported
        if(!CImport::instance()->getVolume())
            throw MyException("A volume must be properly imported first. Please perform the Import step.");

        //asking confirmation to continue when overwriting existing XML file
        if( StackedVolume::fileExists(saveproj_field->text().toStdString().c_str()) &&
              QMessageBox::information(this, "Warning", "An XML file with the same name was found and it will be overwritten.", "Continue", "Cancel"))
        {
            PMain::instance()->setToReady();
            CDisplComp::instance()->reset();
            return;
        }

        //disabling import form and enabling progress bar animation and tab wait animation
        PMain::instance()->getProgressBar()->setEnabled(true);
        PMain::instance()->getProgressBar()->setMinimum(0);
        PMain::instance()->getProgressBar()->setMaximum(100);
        PMain::instance()->closeVolumeAction->setEnabled(false);
        PMain::instance()->exitAction->setEnabled(false);
        wait_movie->start();
        container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, wait_label);

        //propagating options and paramaters and launching task
        CDisplComp::instance()->setProjPath(saveproj_field->text().toStdString());
        CDisplComp::instance()->setAlgorithm(algorithm_cbox->currentIndex());
        CDisplComp::instance()->setSubvolDim(subvoldims_sbox->value());
        CDisplComp::instance()->setStacksIntervals(startrow_sbox->value(), endrow_sbox->value(), startcol_sbox->value(), endcol_sbox->value());
        CDisplComp::instance()->setSearchRadius(Ysearch_sbox->value(), Xsearch_sbox->value(), Zsearch_sbox->value());
        CDisplComp::instance()->setOverlap(Yoverlap_sbox->value(), Xoverlap_sbox->value());
        CDisplComp::instance()->setRestoreSPIM(restoreSPIM_cbox->isChecked());
        CDisplComp::instance()->start();
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::instance()->setToReady();
        CDisplComp::instance()->reset();
    }
}
void PTabDisplComp::stop()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplComp stop() launched\n", this->thread()->currentThreadId());
    #endif

    // ----- terminating CDisplComp's thread is UNSAFE ==> this feature should be disabled or a warning should be displayed ------
    try
    {
        CDisplComp::instance()->terminate();
        CDisplComp::instance()->wait();
        CDisplComp::instance()->reset();
    }
    catch(MyException &ex) {QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));}
    catch(...) {QMessageBox::critical(this,QObject::tr("Error"), QObject::tr("Unable to determine error's type"),QObject::tr("Ok"));}

    //disabling progress bar and wait animations
    PMain::instance()->setToReady();
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);

    PMain::instance()->closeVolumeAction->setEnabled(true);
    PMain::instance()->exitAction->setEnabled(true);
}

/**********************************************************************************
* Overrides QWidget's setEnabled(bool).
* If the widget is enabled, its fields are filled with the informations provided by
* the <StackedVolume> object of <CImport> instance.
***********************************************************************************/
void PTabDisplComp::setEnabled(bool enabled)
{
    /**/tsp::debug(tsp::LEV_MAX, strprintf("enabled = %s", enabled ? "true" : "false").c_str(), __tsp__current__function__);

    //first calling super-class implementation
    QWidget::setEnabled(enabled);

    //then filling widget fields
    if(enabled && CImport::instance()->getVolume())
    {
        Yoverlap_sbox->setMinimum(50);
        Yoverlap_sbox->setMaximum(CImport::instance()->getVolume()->getStacksWidth()*0.5);
        Yoverlap_sbox->setValue(CImport::instance()->getVolume()->getOVERLAP_V());
        Xoverlap_sbox->setMinimum(50);
        Xoverlap_sbox->setMaximum(CImport::instance()->getVolume()->getStacksWidth()*0.5);
        Xoverlap_sbox->setValue(CImport::instance()->getVolume()->getOVERLAP_H());
        startrow_sbox->setMinimum(0);
        startrow_sbox->setMaximum(CImport::instance()->getVolume()->getN_ROWS()-1);
        startrow_sbox->setValue(0);
        endrow_sbox->setMinimum(0);
        endrow_sbox->setMaximum(CImport::instance()->getVolume()->getN_ROWS()-1);
        endrow_sbox->setValue(CImport::instance()->getVolume()->getN_ROWS()-1);
        startcol_sbox->setMinimum(0);
        startcol_sbox->setMaximum(CImport::instance()->getVolume()->getN_COLS()-1);
        startcol_sbox->setValue(0);
        endcol_sbox->setMinimum(0);
        endcol_sbox->setMaximum(CImport::instance()->getVolume()->getN_COLS()-1);
        endcol_sbox->setValue(CImport::instance()->getVolume()->getN_COLS()-1);
        QString saveproj_path = CImport::instance()->getVolume()->getSTACKS_DIR();
        saveproj_path.append("/xml_displcomp.xml");
        saveproj_field->setText(saveproj_path);
        updateMemoryOccupancy(0);
    }
}

/**********************************************************************************
* Updates memory occupancy's estimation field.
* Called when either "subvoldims_sbox" or stacks intervals fields changed.
***********************************************************************************/
void PTabDisplComp::updateMemoryOccupancy(int)
{
    StackedVolume* vol = CImport::instance()->getVolume();
    if(vol)
    {
        int nstack_at_time = std::min(endrow_sbox->value()-startrow_sbox->value(),endcol_sbox->value()-startcol_sbox->value()) + 2;
        float MBytes = nstack_at_time*subvoldims_sbox->value()*(vol->getStacksHeight()/1024.0f)*(vol->getStacksWidth()/1024.0f)*4;
        memocc_field->setText(QString::number(MBytes).append(" MB"));
    }
}

/**********************************************************************************
* Opens file dialog to select the XML file to be saved.
* Called when user clicks on "browse_button".
***********************************************************************************/
void PTabDisplComp::browse_button_clicked()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplComp browse_button_clicked() launched\n", this->thread()->currentThreadId());
    #endif

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter( tr("XML files (*.xml *.XML)") );
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDirectory(CImport::instance()->getVolume()->getSTACKS_DIR());
    if (dialog.exec())
    {
        QStringList fileNames = dialog.selectedFiles();
        QString xmlpath = fileNames.first();
        if(!xmlpath.endsWith(".xml", Qt::CaseInsensitive))
            xmlpath.append(".xml");
        saveproj_field->setText(xmlpath);
    }
}

/**********************************************************************************
* Called by <CDisplComp> when the associated operation has been performed.
* If an exception has occurred in the <CDisplComp> thread,it is propagated and man-
* aged in the current thread (ex != 0). Otherwise, the other tabs are updated.
***********************************************************************************/
void PTabDisplComp::displcomp_done(MyException *ex)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplComp displcomp_done(%s) launched\n", this->thread()->currentThreadId(), (ex? "ex" : "NULL"));
    #endif

    //if an exception has occurred, showing a message error
    if(ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
    }
    else
    {
        //showing operation successful message
        QMessageBox::information(this, "Operation successful", "Aligning step successfully performed!", QMessageBox::Ok);

        PTabDisplProj::getInstance()->setEnabled(true);
        PTabDisplThresh::getInstance()->setEnabled(true);

        // basic mode: automatically performing "Projecting", "Thresholding" and "Placing" steps
        if(PMain::instance()->modeBasicAction->isChecked())
        {
            PTabDisplProj::getInstance()->start();
            PTabDisplThresh::getInstance()->start();
            PTabPlaceTiles::getInstance()->setEnabled(true);
            PTabPlaceTiles::getInstance()->start();
        }
    }

    //resetting some widgets
    PMain::instance()->closeVolumeAction->setEnabled(true);
    PMain::instance()->exitAction->setEnabled(true);
    PMain::instance()->setToReady();
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
    CDisplComp::instance()->reset();
}

/**********************************************************************************
* Called when <showAdvancedButton> status changed
***********************************************************************************/
void PTabDisplComp::showAdvancedChanged(bool status)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplComp::showAdvancedChanged(%s)\n", this->thread()->currentThreadId(), (status? "true" : "false"));
    #endif

    advanced_panel->setVisible(status);
}

/**********************************************************************************
* Called when "channel_selection" state has changed.
***********************************************************************************/
void PTabDisplComp::channelSelectedChanged(int c)
{
    iom::CHANNEL_SELECTION = c;
}
