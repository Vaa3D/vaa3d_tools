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

#include "PTeraStitcher.h"
#include "CImportUnstitched.h"
#include "CPreview.h"
#include "CDisplComp.h"
#include "CMergeTiles.h"

#ifndef WITH_QT
#define WITH_QT
#endif
#include "QProgressSender.h"

using namespace terastitcher;

PTeraStitcher* PTeraStitcher::uniqueInstance = NULL;

PTeraStitcher* PTeraStitcher::instance(QWidget *parent)
{
	if (uniqueInstance == NULL)
		uniqueInstance = new PTeraStitcher(parent);
	return uniqueInstance;
}

void PTeraStitcher::uninstance()
{
    CImportUnstitched::uninstance();
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


PTeraStitcher::PTeraStitcher(QWidget *parent) : QMainWindow(parent)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTeraStitcher created\n", this->thread()->currentThreadId());
    #endif

    //initializing members
    parentWidget = parent;

    fileMenu = menuBar()->addMenu("File");
    closeVolumeAction = new QAction("Close volume", this);
    closeVolumeAction->setIcon(QIcon(":/icons/close.png"));
    closeVolumeAction->setShortcut(QKeySequence("Ctrl+C"));
    connect(closeVolumeAction, SIGNAL(triggered()), this, SLOT(closeVolumeTriggered()));
    fileMenu->addAction(closeVolumeAction);
    exitAction = new QAction("Quit", this);
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    fileMenu->addAction(exitAction);
    connect(exitAction, SIGNAL(triggered()), this, SLOT(exit()));

    optionsMenu = menuBar()->addMenu("Options");
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

    helpMenu = menuBar()->addMenu("Help");
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
#ifdef VAA3D_TERASTITCHER
    tabMergeTiles = PTabMergeTiles::instance(tabs, 5, V3D_env);
#else
	tabMergeTiles = PTabMergeTiles::instance(tabs, 5);
#endif
    tabMergeTiles->setEnabled(false);
    tabs->addTab(tabImport, tr("Import"));
    tabs->addTab(tabDisplComp, tr("Align"));
    tabs->addTab(tabDisplProj, tr("Project"));
    tabs->addTab(tabDisplThres, tr("Threshold"));
    tabs->addTab(tabPlaceTiles, tr("Place"));
    tabs->addTab(tabMergeTiles, tr("Merge"));
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
    statusBar()->showMessage("Ready.");
    //statusBar()->setFont(QFont("",8));
    //helpBox = new QHelpBox(this);
    //helpBox->setDefaultMessage("<b>What's this?</b><br><i>Move the mouse over an object and its description will be displayed here.</i>");

    //layout
    QHBoxLayout* bottomBar = new QHBoxLayout();
    bottomBar->addWidget(progressBar, 1);
    bottomBar->addWidget(startButton);
    bottomBar->addWidget(stopButton);
    //bottomBar->setContentsMargins(5,0,5,5);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(tabs, 1);
    //helpBox->setFixedHeight(100);
    //QGridLayout *helpBoxLayout = new QGridLayout();
   // helpBoxLayout->addWidget(helpBox, 0, 0);
    //helpBoxLayout->setContentsMargins(5,5,5,0);
	//layout->addLayout(helpBoxLayout);
	layout->addLayout(bottomBar, 0);
    //layout->setSpacing(0);
    //layout->setContentsMargins(0,0,0,0);
	QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
	setCentralWidget(centralWidget);

    // signals and slots
    connect(startButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
    //connect(startAllButton, SIGNAL(clicked()), this, SLOT(startAllButtonClicked()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopButtonClicked()));
    connect(ts::QProgressSender::instance(), SIGNAL(sendProgressBarChanged(int, int, int, std::string)), this, SLOT(progressBarChanged(int, int, int, std::string)), Qt::QueuedConnection);
    /*tabImport->installEventFilter(this);
    tabDisplComp->installEventFilter(this);
    tabDisplProj->installEventFilter(this);
    tabDisplThres->installEventFilter(this);
    tabPlaceTiles->installEventFilter(this);
    tabMergeTiles->installEventFilter(this);
    tabImport->slice_spinbox->installEventFilter(this);
    tabImport->preview_button->installEventFilter(this);*/

    //window attributes
    setWindowTitle(tr("TeraStitcher v") + (terastitcher::version + " (with Qt " + ts::qtversion + ")").c_str());
    //setWindowFlags(Qt::WindowStaysOnTopHint);
	//resize(1024, 768);
//    setFixedSize(800, 600);
//    setFixedSize(1024, 768);
    show();
    move(QApplication::desktop()->screen()->rect().center() - rect().center());

    reset();
}

PTeraStitcher::~PTeraStitcher()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTeraStitcher destroyed\n", this->thread()->currentThreadId());
    #endif
}

//resets all GUI elements
void PTeraStitcher::reset()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTeraStitcher::reset()\n", this->thread()->currentThreadId());
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
void PTeraStitcher::modeChanged()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTeraStitcher::modeChanged()\n", this->thread()->currentThreadId());
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
        tabs->addTab(tabDisplProj, tr("Project"));
        tabs->addTab(tabDisplThres, tr("Threshold"));
        tabs->addTab(tabPlaceTiles, tr("Place"));
        tabs->addTab(tabMergeTiles, tr("Merge"));
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
void PTeraStitcher::setToReady()
{
    progressBar->setEnabled(false);
    progressBar->setMaximum(1);         //needed to stop animation on some operating systems
    startButton->setEnabled(true);
    stopButton->setEnabled(false);    
    tabs->setEnabled(true);
    statusBar()->clearMessage();
    statusBar()->showMessage("Ready.");
}

//called when startButton has been clicked
void PTeraStitcher::startButtonClicked()
{
    //tabs->setEnabled(false); // DANGEROUS
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
void PTeraStitcher::startAllButtonClicked()
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
void PTeraStitcher::stopButtonClicked()
{
    if(this->tabImport->isVisible())
        tabImport->stop();
    else if(tabDisplComp->isVisible())
    {
        if(QMessageBox::information(this, "Warning", "Terminating this step can be unsafe and cause TeraStitcher to crash. \n"
                                                  "\nPlease save your data first or click on \"Cancel\" and close the "
                                                  "application to terminate safely this process.", "Continue", "Cancel"))
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
        if(QMessageBox::information(this, "Warning", "Terminating this step can be unsafe and cause TeraStitcher to crash. \n"
                                                  "\nPlease save your data first or click on \"Cancel\" and close the "
                                                  "application to terminate safely this process.", "Continue", "Cancel"))
            return;
        else
            tabMergeTiles->stop();
    }
    else;
    setToReady();
}

//overrides closeEvent method of QWidget
void PTeraStitcher::closeEvent(QCloseEvent *evt)
{
    if(progressBar->isEnabled() && QMessageBox::information(this, "Warning", "An operation is still in progress. Terminating it can be unsafe and cause Vaa3D to crash. \n"
                                                                    "\nPlease save your data first.", "Close TeraStitcher", "Cancel"))
    {
        evt->ignore();
    }
    else
    {
        evt->accept();
        PTeraStitcher::uninstance();
    }
}

/**********************************************************************************
* Called when "Close volume" menu action is triggered.
* All the memory allocated is released and GUI is reset".
***********************************************************************************/
void PTeraStitcher::closeVolumeTriggered()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTeraStitcher::closeVolumeTriggered()\n", this->thread()->currentThreadId());
    #endif

    CImportUnstitched::instance()->reset();
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
void PTeraStitcher::exit()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTeraStitcher::exit()\n", this->thread()->currentThreadId());
    #endif

    this->close();
}

/**********************************************************************************
* Called when "Help->About" menu action is triggered
***********************************************************************************/
void PTeraStitcher::about()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTeraStitcher::about()\n", this->thread()->currentThreadId());
    #endif

    QMessageBox msgBox(this);
    QSpacerItem* horizontalSpacer = new QSpacerItem(600, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    msgBox.setText( QString("<html><h1>TeraStitcher v. ").append(terastitcher::version.c_str()).append("</h1>"
                    "<big>A tool for fast automatic 3D-stitching of teravoxel-sized microscopy images</big><br>"
                    "<a href=\"http://www.biomedcentral.com/1471-2105/13/316\">(BMC Bioinformatics 2012, 13:316)</a><br><br>"
                    "<u>Developed by:</u><ul>"
                    "<li><b>Alessandro Bria</b> (email: a.bria@unicas.it)<br>"
                           "Post-doctoral Fellow at University of Cassino</li>"
                    "<li><b>Giulio Iannello</b> (email: g.iannello@unicampus.it)<br>"
                           "Full Professor at University Campus Bio-Medico of Rome</li>"
                    "For further info, please visit our <a href=\"http://abria.github.io/TeraStitcher/\">website</a>.</html>" ));

    QGridLayout* layout = (QGridLayout*)msgBox.layout();
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    msgBox.exec();
}

/**********************************************************************************
* <sendProgressBarChanged> event handler
***********************************************************************************/
void PTeraStitcher::progressBarChanged(int val, int minutes, int seconds, std::string message)
{
    progressBar->setValue(val);
    QString remaining_time = QString::number(minutes);
    remaining_time.append(" minutes and ");
    remaining_time.append(QString::number(seconds));
    remaining_time.append(" seconds remaining");
    if(message.size())
        statusBar()->showMessage(message.c_str());
    else
        statusBar()->showMessage(remaining_time);
}

/**********************************************************************************
* Filters events generated by the widgets to which a help message must be associated
***********************************************************************************/
std::string defaultMsg   = "<b>What's this?</b><br><i>Move the mouse over an object and its description will be displayed here.</i>";
std::string tabImportMsg = "The <i>Import</i> step imports the <b>acquisition files</b> and previously computed <b>stitching metadata</b> "
                      "(alignments, tiles placement, etc.) if any. This takes up to a few minutes the first time an acquisition is "
                      "imported (all files are scanned) whereas it is instant when importing through a valid <b>TeraStitcher XML project</b> file.";
std::string previewMsg =   "Select a <b>slice</b> to be <b>stitched</b> using <b>nominal stage coordinates</b> and the result will be shown "
                      "into <b>Vaa3D</b>. This may be useful to test whether the volume has been properly imported and the precision of "
                      "<b>motorized stages</b>.";
std::string tabDCompMsg  = "The <i>Align</i> step computes all <b>pairwise stacks displacements</b> and saves them in the selected XML project file. "
                      "The volume will be processed a layer at the time, whose thickness can be specified by the user to control <b>memory occupancy</b>.";
std::string tabDProjMsg  = "The <i>Project</i> step generates <b>the most reliable displacement</b> between each pair of adjacent stacks "
                      "by combining the multiple displacements previously computed along D (one for each layer)";
std::string tabDThresMsg = "The <i>Threshold</i> step substitutes all the displacements whose <b>reliability</b> is below the selected threshold "
                      "with the default displacement corresponding to the <b>nominal stage coordinate</b>. In addition, the subset of reliable displacements "
                      "is used to mark stacks as <b>stitchables</b> or <b>nonstitchables</b>, that can be later excluded from <i>Merge</i>.";
std::string tabPlaceMsg =  "The <i>Place</i> step uses the computed displacements to obtain the <b>optimal tiles placement</b>. If this step is skipped, "
                      "tiles will be merged using only nominal stage coordinates.";
std::string tabMergeMsg =  "The <i>Merge</i> step <b>combines</b> tiles into a volume which is saved at <b>different resolutions</b>. The "
                      "overlapping regions are substituted with a <b>blended</b> version of them. If the previous step has NOT been performed, <b>nominal</b> stage "
                      "coordinates will be used for merging. This allows <b>direct stitching</b> in the case that nominal coordinates are precise enough.";
//bool PTeraStitcher::eventFilter(QObject *object, QEvent *event)
//{
//    if((object == tabImport->slice_spinbox || object == tabImport->preview_button) && tabImport->isVisible())
//    {
//        if(event->type() == QEvent::Enter)
//            helpBox->setText(previewMsg);
//        else if(event->type() == QEvent::Leave)
//            helpBox->setText(tabImportMsg);
//    }
//    else if (object == tabImport)
//    {
//        if(event->type() == QEvent::Enter)
//            helpBox->setText(tabImportMsg);
//        else if(event->type() == QEvent::Leave)
//            helpBox->setText(defaultMsg);
//    }
//    else if(object == tabDisplComp)
//    {
//        if(event->type() == QEvent::Enter)
//            helpBox->setText(tabDCompMsg);
//        else if(event->type() == QEvent::Leave)
//            helpBox->setText(defaultMsg);
//    }
//    else if(object == tabDisplProj)
//    {
//        if(event->type() == QEvent::Enter)
//            helpBox->setText(tabDProjMsg);
//        else if(event->type() == QEvent::Leave)
//            helpBox->setText(defaultMsg);
//    }
//    else if(object == tabDisplThres)
//    {
//        if(event->type() == QEvent::Enter)
//            helpBox->setText(tabDThresMsg);
//        else if(event->type() == QEvent::Leave)
//            helpBox->setText(defaultMsg);
//    }
//    else if(object == tabPlaceTiles)
//    {
//        if(event->type() == QEvent::Enter)
//            helpBox->setText(tabPlaceMsg);
//        else if(event->type() == QEvent::Leave)
//            helpBox->setText(defaultMsg);
//    }
//    else if(object == tabMergeTiles)
//    {
//        if(event->type() == QEvent::Enter)
//            helpBox->setText(tabMergeMsg);
//        else if(event->type() == QEvent::Leave)
//            helpBox->setText(defaultMsg);
//    }
//    return false;
//}

//very useful (not included in Qt): disables the given item of the given combobox
void PTeraStitcher::setEnabledComboBoxItem(QComboBox* cbox, int _index, bool enabled)
{
    // Get the index of the value to disable
    QModelIndex index = cbox->model()->index(_index,0);

    // These are the effective 'disable/enable' flags
    QVariant v1(Qt::NoItemFlags);
    QVariant v2(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    //the magic
    cbox->model()->setData( index, enabled ? v2 : v1, Qt::UserRole -1);
}
