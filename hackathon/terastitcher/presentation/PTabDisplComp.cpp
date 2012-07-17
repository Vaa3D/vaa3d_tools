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
#include "StackedVolume.h"
#include "PMain.h"
#include "control/CImport.h"
#include "control/CDisplComp.h"

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

    //help box
    helpbox = new QLabel("<html><table><tr style=\"vertical-align: middle;\"><td><img src=\":/icons/help.png\"></td>"
                              "<td><p style=\"text-align:justify; margin-left:10px;\"> This step computes all <b>pairwise stacks displacements</b> which are automatically saved in a XML project file at the selected path.<br><br>"
                              "The volume will be processed a layer at the time, each composed by <i>Number of slices per layer</b> slices at most</i>. "
                              "This allows <b>direct control over memory occupancy</b>. "
                              "You can also select which algorithm to be used for each pairwise stacks displacements computation.</p> </td></tr></table> </html>");
    helpbox->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); margin-top:10px; margin-bottom:10px; padding-top:10px; padding-bottom:10px;");
    helpbox->setWordWrap(true);
    helpbox->setFixedHeight(180);

    //creating Basic panel widgets
    basic_panel = new QGroupBox("Basic settings");
    saveproj_label = new QLabel("Save project XML to:");
    saveproj_field = new QLineEdit();
    browse_button = new QPushButton("Save to XML...");
    algorithm_label = new QLabel("Algorithm:");
    algorithm_cbox = new QComboBox();
    algorithm_cbox->insertItem(0, "Maximum Intensity Projection - Normalized Cross Correlation (MIP-NCC)");
    subvoldims_label = new QLabel("Number of slices per layer:");
    subvoldims_sbox = new QSpinBox();
    subvoldims_sbox->setAlignment(Qt::AlignCenter);
    subvoldims_sbox->setMaximum(600);
    subvoldims_sbox->setMinimum(50);
    subvoldims_sbox->setValue(100);
    memocc_label = new QLabel("Estimated memory usage:");
    memocc_label->setFixedWidth(310);
    memocc_field = new QLabel();
    memocc_field->setMaximumHeight(30);

    //creating Advanced panel widgets
    advanced_panel = new QGroupBox("Advanced settings");
    stackrowstbp_label = new QLabel("Stacks rows to process:");
    startrow_sbox = new QSpinBox();
    startrow_sbox->setAlignment(Qt::AlignCenter);
    to_label_1 = new QLabel("to");
    endrow_sbox = new QSpinBox();
    endrow_sbox->setAlignment(Qt::AlignCenter);
    stackcolstbp_label = new QLabel("Stacks columns to process:");
    startcol_sbox = new QSpinBox();
    startcol_sbox->setAlignment(Qt::AlignCenter);
    to_label_2 = new QLabel("to");
    endcol_sbox = new QSpinBox();
    endcol_sbox->setAlignment(Qt::AlignCenter);
    searchregion_label = new QLabel("Search Region (voxels):");
    Vsearch_sbox = new QSpinBox();
    Vsearch_sbox->setAlignment(Qt::AlignCenter);
    Vsearch_sbox->setMinimum(5);
    Vsearch_sbox->setValue(20);
    Hsearch_sbox = new QSpinBox();
    Hsearch_sbox->setAlignment(Qt::AlignCenter);
    Hsearch_sbox->setMinimum(5);
    Hsearch_sbox->setValue(20);
    Dsearch_sbox = new QSpinBox();
    Dsearch_sbox->setAlignment(Qt::AlignCenter);
    Dsearch_sbox->setMinimum(5);
    Dsearch_sbox->setValue(20);
    directionV_label_2 = new QLabel("(V)");
    directionH_label_2 = new QLabel("(H)");
    directionD_label_2 = new QLabel("(D)");
    for_label_1 = new QLabel("x");
    for_label_2 = new QLabel("x");
    overlap_label = new QLabel("Overlap (voxels):");
    Voverlap_sbox = new QSpinBox();
    Voverlap_sbox->setAlignment(Qt::AlignCenter);
    Hoverlap_sbox = new QSpinBox();
    Hoverlap_sbox->setAlignment(Qt::AlignCenter);
    directionV_label_3 = new QLabel("(V)");
    directionH_label_3 = new QLabel("(H)");
    for_label_3 = new QLabel("x");
    restoreSPIM_label = new QLabel("SPIM artifacts removal:");
    restoreSPIM_cbox = new QCheckBox("Compute stacks profiles to be used in the Merging tiles step");

    /*** LAYOUT SECTIONS ***/
    //basic panel
    QGridLayout* basicpanel_layout = new QGridLayout();
    basicpanel_layout->addWidget(saveproj_label,     0,0,1,9);
    basicpanel_layout->addWidget(saveproj_field,     0,9,1,9);
    basicpanel_layout->addWidget(browse_button,      0,18,1,2);
    basicpanel_layout->addWidget(algorithm_label,    1,0,1,9);
    basicpanel_layout->addWidget(algorithm_cbox,     1,9,1,5);
    basicpanel_layout->addWidget(subvoldims_label,   2,0,1,9);
    basicpanel_layout->addWidget(subvoldims_sbox,    2,9,1,2);
    basicpanel_layout->addWidget(memocc_label,       3,0,1,9);
    basicpanel_layout->addWidget(memocc_field,       3,9,1,2);
    basic_panel->setLayout(basicpanel_layout);
    basic_panel->setStyle(new QWindowsStyle());


    //advanced panel
    QGridLayout* advancedpanel_layout = new QGridLayout();
    advancedpanel_layout->addWidget(stackrowstbp_label,     0,0,1,5);
    advancedpanel_layout->addWidget(startrow_sbox,          0,5,1,2);
    advancedpanel_layout->addWidget(to_label_1,             0,8,1,1);
    advancedpanel_layout->addWidget(endrow_sbox,            0,9,1,2);
    advancedpanel_layout->addWidget(stackcolstbp_label,     1,0,1,5);
    advancedpanel_layout->addWidget(startcol_sbox,          1,5,1,2);
    advancedpanel_layout->addWidget(to_label_2,             1,8,1,1);
    advancedpanel_layout->addWidget(endcol_sbox,            1,9,1,2);
    advancedpanel_layout->addWidget(searchregion_label,     2,0,1,5);
    advancedpanel_layout->addWidget(Vsearch_sbox,           2,5,1,2);
    advancedpanel_layout->addWidget(directionV_label_2,     2,7,1,1);
    advancedpanel_layout->addWidget(for_label_1,            2,8,1,1);
    advancedpanel_layout->addWidget(Hsearch_sbox,           2,9,1,2);
    advancedpanel_layout->addWidget(directionH_label_2,     2,11,1,1);
    advancedpanel_layout->addWidget(for_label_2,            2,12,1,1);
    advancedpanel_layout->addWidget(Dsearch_sbox,           2,13,1,2);
    advancedpanel_layout->addWidget(directionD_label_2,     2,15,1,1);
    advancedpanel_layout->addWidget(overlap_label,          3,0,1,5);
    advancedpanel_layout->addWidget(Voverlap_sbox,          3,5,1,2);
    advancedpanel_layout->addWidget(directionV_label_3,     3,7,1,1);
    advancedpanel_layout->addWidget(Hoverlap_sbox,          3,9,1,2);
    advancedpanel_layout->addWidget(directionH_label_3,     3,11,1,1);
    advancedpanel_layout->addWidget(restoreSPIM_label,      4,0,1,5);
    advancedpanel_layout->addWidget(restoreSPIM_cbox,       4,5,1,11);
    advanced_panel->setLayout(advancedpanel_layout);
    advanced_panel->setStyle(new QWindowsStyle());



    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(helpbox);
    layout->addWidget(basic_panel);
    layout->addWidget(advanced_panel);
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
}


PTabDisplComp::~PTabDisplComp()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplComp destroyed\n", this->thread()->currentThreadId());
    #endif
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
            PMain::instance()->resetGUI();
            CDisplComp::instance()->reset();
            return;
        }

        //disabling import form and enabling progress bar animation and tab wait animation
        PMain::instance()->getProgressBar()->setEnabled(true);
        PMain::instance()->getProgressBar()->setMinimum(0);
        PMain::instance()->getProgressBar()->setMaximum(100);
        wait_movie->start();
        container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, wait_label);

        //propagating options and paramaters and launching task
        CDisplComp::instance()->setProjPath(saveproj_field->text().toStdString());
        CDisplComp::instance()->setAlgorithm(algorithm_cbox->currentIndex());
        CDisplComp::instance()->setSubvolDim(subvoldims_sbox->value());
        CDisplComp::instance()->setStacksIntervals(startrow_sbox->value(), endrow_sbox->value(), startcol_sbox->value(), endcol_sbox->value());
        CDisplComp::instance()->setSearchRadius(Vsearch_sbox->value(), Hsearch_sbox->value(), Dsearch_sbox->value());
        CDisplComp::instance()->setOverlap(Voverlap_sbox->value(), Hoverlap_sbox->value());
        CDisplComp::instance()->setRestoreSPIM(restoreSPIM_cbox->isChecked());
        CDisplComp::instance()->start();
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::instance()->resetGUI();
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
    PMain::instance()->resetGUI();
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
}

/**********************************************************************************
* Overrides QWidget's setEnabled(bool).
* If the widget is enabled, its fields are filled with the informations provided by
* the <StackedVolume> object of <CImport> instance.
***********************************************************************************/
void PTabDisplComp::setEnabled(bool enabled)
{
    //first calling super-class implementation
    QWidget::setEnabled(enabled);

    //then filling widget fields
    if(enabled && CImport::instance()->getVolume())
    {
        Voverlap_sbox->setMinimum(50);
        Voverlap_sbox->setMaximum(CImport::instance()->getVolume()->getStacksWidth()*0.5);
        Voverlap_sbox->setValue(CImport::instance()->getVolume()->getOVERLAP_V());
        Hoverlap_sbox->setMinimum(50);
        Hoverlap_sbox->setMaximum(CImport::instance()->getVolume()->getStacksWidth()*0.5);
        Hoverlap_sbox->setValue(CImport::instance()->getVolume()->getOVERLAP_H());
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
        //otherwise updating other tabs
        PTabDisplProj::getInstance()->setEnabled(true);
        PTabDisplThresh::getInstance()->setEnabled(true);
    }

    //resetting some widgets
    PMain::instance()->resetGUI();
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
    CDisplComp::instance()->reset();
}
