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
#include "control/CPreview.h"
#include "control/CDisplComp.h"
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
    tabs->addTab(tabDisplProj, tr("Projecting displacements"));
    tabs->addTab(tabDisplThres, tr("Thresholding displacements"));
    tabs->addTab(tabPlaceTiles, tr("Placing tiles"));
    tabs->addTab(tabMergeTiles, tr("Merging tiles"));
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

    //layout
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSizeConstraint( QLayout::SetNoConstraint);
    gridLayout->addWidget(tabs, 0,0,9,20);
    gridLayout->addWidget(statusBar, 9,0,1,20);
    gridLayout->addWidget(progressBar, 10,0,1,18);
    //gridLayout->addWidget(startAllButton, 10,17,1,1);
    gridLayout->addWidget(startButton, 10,18,1,1);
    gridLayout->addWidget(stopButton, 10,19,1,1);
    setLayout(gridLayout);
    setWindowTitle(tr("TeraStitcher plugin"));
    layout()->setSizeConstraint( QLayout::SetFixedSize );

    // signals and slots
    connect(startButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
    //connect(startAllButton, SIGNAL(clicked()), this, SLOT(startAllButtonClicked()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopButtonClicked()));
    connect(this, SIGNAL(sendProgressBarChanged(int, int, int, const char*)), this, SLOT(progressBarChanged(int, int, int, const char*)), Qt::QueuedConnection);

    resetGUI();

    //center on screen and set always on top
    setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->size(),qApp->desktop()->availableGeometry()));
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}

PMain::~PMain()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PMain destroyed\n", this->thread()->currentThreadId());
    #endif
}

//reset GUI method
void PMain::resetGUI()
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
    resetGUI();
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
