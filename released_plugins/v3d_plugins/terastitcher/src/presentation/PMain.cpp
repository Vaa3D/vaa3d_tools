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
#include "src/control/CImport.h"
#include "src/control/CPreview.h"
#include "src/control/CDisplComp.h"
#include "src/control/CMergeTiles.h"
#include "ProgressBar.h"

using namespace terastitcher;

PMain* PMain::uniqueInstance = NULL;
PMain* PMain::instance(V3DPluginCallback *callback, QWidget *parent)
{
    if (uniqueInstance == NULL)
        uniqueInstance = new PMain(callback, parent);
    return uniqueInstance;
}
void PMain::uninstance()
{
    CImport::uninstance();
    CPreview::uninstance();
    PTabImport::uninstance();
    PTabDisplComp::uninstance();
    PTabDisplProj::uninstance();
    PTabDisplThresh::uninstance();
    PTabPlaceTiles::uninstance();
    PTabMergeTiles::uninstance();
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

PMain::PMain(V3DPluginCallback *callback, QWidget *parent) : QWidget(parent)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PMain created\n", this->thread()->currentThreadId());
    #endif

    //initializing members
    V3D_env = callback;
    parentWidget = parent;

    //initializing menu
    menuBar = new QMenuBar(0);

    fileMenu = menuBar->addMenu("File");
    closeVolumeAction = new QAction("Close volume", this);
    closeVolumeAction->setIcon(QIcon(":/icons/close.png"));
    closeVolumeAction->setShortcut(QKeySequence("Ctrl+C"));
    closeVolumeAction->setWhatsThis("asdddd");
    connect(closeVolumeAction, SIGNAL(triggered()), this, SLOT(closeVolumeTriggered()));
    fileMenu->addAction(closeVolumeAction);
    exitAction = new QAction("Quit", this);
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    fileMenu->addAction(exitAction);
    connect(exitAction, SIGNAL(triggered()), this, SLOT(exit()));

    optionsMenu = menuBar->addMenu("Options");
    modeOptionsMenu = optionsMenu->addMenu("Mode");
    QActionGroup* modeSelectionGroup = new QActionGroup(this);
    modeSelectionGroup->setExclusive(true);
    modeBasicAction = new QAction("Basic", this);
    modeAdvancedAction = new QAction("Advanced", this);
    modeBasicAction->setCheckable(true);
    modeAdvancedAction->setCheckable(true);
    modeAdvancedAction->setChecked(true);
    modeSelectionGroup->addAction(modeBasicAction);
    modeSelectionGroup->addAction(modeAdvancedAction);
    modeOptionsMenu->addAction(modeBasicAction);
    modeOptionsMenu->addAction(modeAdvancedAction);
    connect(modeBasicAction, SIGNAL(changed()), this, SLOT(modeChanged()));
    connect(modeAdvancedAction, SIGNAL(changed()), this, SLOT(modeChanged()));

    helpMenu = menuBar->addMenu("Help");
    aboutAction = new QAction("About", this);
    aboutAction->setIcon(QIcon(":/icons/about.png"));
    helpMenu->addAction(aboutAction);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));


//    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    //creating widgets
    tabs = new QMyTabWidget();
    tabImport = PTabImport::instance(tabs, 0);
    tabDisplComp = PTabDisplComp::instance(tabs, 1);
    tabDisplComp->setEnabled(false);
    tabDisplProj = PTabDisplProj::instance(tabs, 2);
    tabDisplProj->setEnabled(false);
    tabDisplThres = PTabDisplThresh::instance(tabs, 3);
    tabDisplThres->setEnabled(false);
    tabPlaceTiles = PTabPlaceTiles::instance(tabs, 4);
    tabPlaceTiles->setEnabled(false);
    tabMergeTiles = PTabMergeTiles::instance(tabs, 5);
    tabMergeTiles->setEnabled(false);
    tabs->addTab(tabImport, tr("Importing"));
    tabs->addTab(tabDisplComp, tr("Aligning"));
    tabs->addTab(tabDisplProj, tr("Projecting"));
    tabs->addTab(tabDisplThres, tr("Thresholding"));
    tabs->addTab(tabPlaceTiles, tr("Placing"));
    tabs->addTab(tabMergeTiles, tr("Merging"));
    tabs->setTabIcon(0, QIcon(":/icons/number1.png"));
    tabs->setTabIcon(1, QIcon(":/icons/number2.png"));
    tabs->setTabIcon(2, QIcon(":/icons/number3.png"));
    tabs->setTabIcon(3, QIcon(":/icons/number4.png"));
    tabs->setTabIcon(4, QIcon(":/icons/number5.png"));
    tabs->setTabIcon(5, QIcon(":/icons/number6.png"));
    progressBar = new QProgressBar(this);
    startButton = new QPushButton(this);
    startButton->setIcon(QIcon(":/icons/start.png"));
    startButton->setText("Start");
    /*startAllButton = new QPushButton(this);
    startAllButton->setIcon(QIcon(":/icons/startall.png"));
    startAllButton->setText("Start all");*/
    stopButton = new QPushButton(this);
    stopButton->setIcon(QIcon(":/icons/stop.png"));
    stopButton->setText("Stop");
    statusBar = new QStatusBar();
    statusBar->showMessage("Ready.");
    statusBar->setFont(QFont("",8));
    helpBox = new QHelpBox(this);
    helpBox->setDefaultMessage("<b>What's this?</b><br><i>Move the mouse over an object and its description will be displayed here.</i>");

    //layout
    QGridLayout* bottomBar = new QGridLayout();
    bottomBar->addWidget(statusBar, 0,0,1,20);
    bottomBar->addWidget(progressBar, 1,0,1,18);
    bottomBar->addWidget(startButton, 1,18,1,1);
    bottomBar->addWidget(stopButton, 1,19,1,1);
    bottomBar->setContentsMargins(5,0,5,5);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(menuBar, 0);
    layout->addWidget(tabs, 1);
    helpBox->setFixedHeight(55);
    QGridLayout *helpBoxLayout = new QGridLayout();
    helpBoxLayout->addWidget(helpBox, 0, 0);
    helpBoxLayout->setContentsMargins(5,5,5,0);
    layout->addLayout(helpBoxLayout);
    layout->addLayout(bottomBar, 0);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

    // signals and slots
    connect(startButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
    //connect(startAllButton, SIGNAL(clicked()), this, SLOT(startAllButtonClicked()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopButtonClicked()));
    connect(this, SIGNAL(sendProgressBarChanged(int, int, int, const char*)), this, SLOT(progressBarChanged(int, int, int, const char*)), Qt::QueuedConnection);
    tabImport->installEventFilter(this);
    tabDisplComp->installEventFilter(this);
    tabDisplProj->installEventFilter(this);
    tabDisplThres->installEventFilter(this);
    tabPlaceTiles->installEventFilter(this);
    tabMergeTiles->installEventFilter(this);
    tabImport->slice_spinbox->installEventFilter(this);
    tabImport->preview_button->installEventFilter(this);

    //window attributes
    setWindowTitle(tr("TeraStitcher plugin v") + terastitcher::version.c_str());
    setWindowFlags(Qt::WindowStaysOnTopHint);
    setFixedSize(800, 600);
    show();
    move(QApplication::desktop()->screen()->rect().center() - rect().center());

    reset();
}

PMain::~PMain()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PMain destroyed\n", this->thread()->currentThreadId());
    #endif
}

//resets all GUI elements
void PMain::reset()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PMain::reset()\n", this->thread()->currentThreadId());
    #endif

    //resetting menu widgets
    closeVolumeAction->setEnabled(false);
    modeBasicAction->setChecked(true);

    //set GUI to ready state
    setToReady();
}

/**********************************************************************************
* Called when "Options->Mode" menu action has changed
***********************************************************************************/
void PMain::modeChanged()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PMain::modeChanged()\n", this->thread()->currentThreadId());
    #endif

    if(modeBasicAction->isChecked())
    {
        tabs->removeTab(2);
        tabs->removeTab(3);
        tabs->removeTab(4);
        tabs->setTabIcon(2, QIcon(":/icons/number3.png"));
        tabDisplComp->showAdvancedButton->setChecked(false);
        tabMergeTiles->showAdvancedButton->setChecked(false);
    }
    else
    {
        tabs->removeTab(3);
        tabs->addTab(tabDisplProj, tr("Projecting"));
        tabs->addTab(tabDisplThres, tr("Thresholding"));
        tabs->addTab(tabPlaceTiles, tr("Placing"));
        tabs->addTab(tabMergeTiles, tr("Merging"));
        tabs->setTabIcon(2, QIcon(":/icons/number3.png"));
        tabs->setTabIcon(3, QIcon(":/icons/number4.png"));
        tabs->setTabIcon(4, QIcon(":/icons/number5.png"));
        tabs->setTabIcon(5, QIcon(":/icons/number6.png"));
        if(tabs->currentIndex() == 2)
            tabs->setCurrentIndex(5);
        tabDisplComp->showAdvancedButton->setChecked(true);
        tabMergeTiles->showAdvancedButton->setChecked(true);
    }
}

//reset GUI method
void PMain::setToReady()
{
    progressBar->setEnabled(false);
    progressBar->setMaximum(1);         //needed to stop animation on some operating systems
    startButton->setEnabled(true);
    stopButton->setEnabled(false);    
    tabs->setEnabled(true);
    statusBar->clearMessage();
    statusBar->showMessage("Ready.");
}

//called when startButton has been clicked
void PMain::startButtonClicked()
{
    tabs->setEnabled(false);
    progressBar->setEnabled(true);
    startButton->setEnabled(false);
    stopButton->setEnabled(true);
    if(this->tabImport->isVisible())
        tabImport->start();
    else if(tabDisplComp->isVisible())
        tabDisplComp->start();
    else if(tabDisplProj->isVisible())
        tabDisplProj->start();
    else if(tabDisplThres->isVisible())
        tabDisplThres->start();
    else if(tabPlaceTiles->isVisible())
        tabPlaceTiles->start();
    else if(tabMergeTiles->isVisible())
        tabMergeTiles->start();
    else;
}

//called when startAllButton has been clicked
void PMain::startAllButtonClicked()
{
    //TODO

    /*tabs->setEnabled(false);
    progressBar->setEnabled(true);
    startButton->setEnabled(false);
    stopButton->setEnabled(false);
    tabImport->start();
    CImport::instance()->wait();
    tabDisplComp->start();
    CDisplComp::instance()->wait();
    tabDisplProj->start();
    tabDisplThres->start();
    tabPlaceTiles->start();
    tabMergeTiles->start();*/
}

//called when stopButton has been clicked
void PMain::stopButtonClicked()
{
    if(this->tabImport->isVisible())
        tabImport->stop();
    else if(tabDisplComp->isVisible())
    {
        if(QMessageBox::information(this, "Warning", "Terminating this step can be unsafe and cause Vaa3D to crash. \n"
                                                  "\nPlease save your data first or click on \"Cancel\" and close the "
                                                  "plugin to terminate safely this process.", "Continue", "Cancel"))
            return;
        else
            tabDisplComp->stop();
    }
    else if(tabDisplProj->isVisible())
        tabDisplProj->stop();
    else if(tabDisplThres->isVisible())
        tabDisplThres->stop();
    else if(tabPlaceTiles->isVisible())
        tabPlaceTiles->stop();
    else if(tabMergeTiles->isVisible())
    {
        if(QMessageBox::information(this, "Warning", "Terminating this step can be unsafe and cause Vaa3D to crash. \n"
                                                  "\nPlease save your data first or click on \"Cancel\" and close the "
                                                  "plugin to terminate safely this process.", "Continue", "Cancel"))
            return;
        else
            tabMergeTiles->stop();
    }
    else;
    setToReady();
}

//overrides closeEvent method of QWidget
void PMain::closeEvent(QCloseEvent *evt)
{
    if(progressBar->isEnabled() && QMessageBox::information(this, "Warning", "An operation is still in progress. Terminating it can be unsafe and cause Vaa3D to crash. \n"
                                                                    "\nPlease save your data first.", "Close TeraStitcher plugin", "Cancel"))
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
* Called when "Close volume" menu action is triggered.
* All the memory allocated is released and GUI is reset".
***********************************************************************************/
void PMain::closeVolumeTriggered()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PMain::closeVolumeTriggered()\n", this->thread()->currentThreadId());
    #endif

    CImport::instance()->reset();
    CDisplComp::instance()->reset();
    CPreview::instance()->reset();
    CMergeTiles::instance()->reset();
    this->reset();
    this->tabImport->reset();
    this->tabDisplComp->reset();
    this->tabDisplProj->reset();
    this->tabDisplThres->reset();
    this->tabPlaceTiles->reset();
    this->tabMergeTiles->reset();
}

/**********************************************************************************
* Called when "Exit" menu action is triggered or TeraFly window is closed.
***********************************************************************************/
void PMain::exit()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PMain::exit()\n", this->thread()->currentThreadId());
    #endif

    this->close();
}

/**********************************************************************************
* Called when "Help->About" menu action is triggered
***********************************************************************************/
void PMain::about()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PMain::about()\n", this->thread()->currentThreadId());
    #endif

    QMessageBox msgBox(this);
    QSpacerItem* horizontalSpacer = new QSpacerItem(800, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    msgBox.setText( QString("<html><h1>TeraStitcher plugin v. ").append(terastitcher::version.c_str()).append("</h1>"
                    "<big>A fully automated 3D Stitching tool designed for Teravoxel-sized datasets.</big><br><br>"
                    "<u>Developed by:</u><ul>"
                    "<li><b>Alessandro Bria</b> (email: a.bria@unicas.it)<br>"
                           "Ph.D. Student at University of Cassino</li>"
                    "<li><b>Giulio Iannello</b> (email: g.iannello@unicampus.it)<br>"
                           "Full Professor at University Campus Bio-Medico of Rome</li></ul><br>"
                    "<u>Features:</u><ul>"
                    "<li>fast and reliable stitching based on a multi-MIP approach</li>"
                    "<li>typical memory requirement of ~2 Gigabytes for 1 Teravoxel-sized dataset</li>"
                    "<li>full control over memory requirements and stitching parameters</li>"
                    "<li>user can select just a subvolume to be stitched only</li>"
                    "<li>SPIM artifacts removal option (experimental)</li></ul><br>"
                    "<u>Supported input formats:</u><ul>"
                    "<li>two-level directory structure with each tile containing a series of image slices (see documentation for further information)</li>"
                    "<li>supported formats for image slices are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF</li>"
                    "<li>no restriction on the bit depth</li>"
                    "<li>no restriction on the number of channels</li></ul><br>"
                    "<u>Supported output formats:</u><ul>"
                    "<li>both grid of non-overlapped image stacks or single image stack (which can be directly <b>shown into Vaa3D</b>)</li>"
                    "<li>stacks can be saved at different resolutions</li>"
                    "<li>supported formats for image slices are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF</li>"
                    "<li>no restriction on the bit depth</li>"
                    "<li>only single-channel</li></ul></html>" ));

    QGridLayout* layout = (QGridLayout*)msgBox.layout();
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    msgBox.exec();
}

/**********************************************************************************
* <sendProgressBarChanged> event handler
***********************************************************************************/
void PMain::progressBarChanged(int val, int minutes, int seconds, const char* message)
{
    progressBar->setValue(val);
    QString remaining_time = QString::number(minutes);
    remaining_time.append(" minutes and ");
    remaining_time.append(QString::number(seconds));
    remaining_time.append(" seconds remaining");
    if(message)
        statusBar->showMessage(message);
    else
        statusBar->showMessage(remaining_time);
}

/**********************************************************************************
* Filters events generated by the widgets to which a help message must be associated
***********************************************************************************/
string defaultMsg   = "<b>What's this?</b><br><i>Move the mouse over an object and its description will be displayed here.</i>";
string tabImportMsg = "The <i>Importing</i> step imports the <b>acquisition files</b> and previously computed <b>stitching metadata</b> "
                      "(alignments, tiles placement, etc.) if any. This takes up to a few minutes the first time an acquisition is "
                      "imported (all files are scanned) whereas it is instant when importing through a valid <b>TeraStitcher XML project</b> file.";
string previewMsg =   "Select a <b>slice</b> to be <b>stitched</b> using <b>nominal stage coordinates</b> and the result will be shown "
                      "into <b>Vaa3D</b>. This may be useful to test whether the volume has been properly imported and the precision of "
                      "<b>motorized stages</b>.";
string tabDCompMsg  = "The <i>Aligning</i> step computes all <b>pairwise stacks displacements</b> and saves them in the selected XML project file. "
                      "The volume will be processed a layer at the time, whose thickness can be specified by the user to control <b>memory occupancy</b>.";
string tabDProjMsg  = "The <i>Projecting</i> step generates <b>the most reliable displacement</b> between each pair of adjacent stacks "
                      "by combining the multiple displacements previously computed along D (one for each layer)";
string tabDThresMsg = "The <i>Thresholding</i> step substitutes all the displacements whose <b>reliability</b> is below the selected threshold "
                      "with the default displacement corresponding to the <b>nominal stage coordinate</b>. In addition, the subset of reliable displacements "
                      "is used to mark stacks as <b>stitchables</b> or <b>nonstitchables</b>, that can be later excluded from <i>Merging</i>.";
string tabPlaceMsg =  "The <i>Placing</i> step uses the computed displacements to obtain the <b>optimal tiles placement</b>. If this step is skipped, "
                      "tiles will be merged using only nominal stage coordinates.";
string tabMergeMsg =  "The <i>Merging</i> step <b>combines</b> tiles into a volume which is saved at <b>different resolutions</b>. The "
                      "overlapping regions are substituted with a <b>blended</b> version of them. If the previous step has NOT been performed, <b>nominal</b> stage "
                      "coordinates will be used for merging. This allows <b>direct stitching</b> in the case that nominal coordinates are precise enough.";
bool PMain::eventFilter(QObject *object, QEvent *event)
{
    if((object == tabImport->slice_spinbox || object == tabImport->preview_button) && tabImport->isVisible())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(previewMsg);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(tabImportMsg);
    }
    else if (object == tabImport)
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(tabImportMsg);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(defaultMsg);
    }
    else if(object == tabDisplComp)
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(tabDCompMsg);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(defaultMsg);
    }
    else if(object == tabDisplProj)
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(tabDProjMsg);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(defaultMsg);
    }
    else if(object == tabDisplThres)
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(tabDThresMsg);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(defaultMsg);
    }
    else if(object == tabPlaceTiles)
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(tabPlaceMsg);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(defaultMsg);
    }
    else if(object == tabMergeTiles)
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(tabMergeMsg);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(defaultMsg);
    }
    return false;
}
