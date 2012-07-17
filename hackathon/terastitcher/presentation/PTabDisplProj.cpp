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

#include "PTabDisplProj.h"
#include "PTabDisplThresh.h"
#include "PTabMergeTiles.h"
#include "PTabPlaceTiles.h"
#include "control/CImport.h"
#include "MyException.h"
#include "StackedVolume.h"
#include "PMain.h"
#include "StackStitcher.h"

using namespace terastitcher;

/*********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "istance(...)"
**********************************************************************************/
PTabDisplProj* PTabDisplProj::uniqueInstance = NULL;
void PTabDisplProj::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

PTabDisplProj::PTabDisplProj(QMyTabWidget* _container, int _tab_index) : QWidget(), container(_container), tab_index(_tab_index)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplProj created\n", this->thread()->currentThreadId());
    #endif

    //help box
    helpbox = new QLabel("<html><table><tr style=\"vertical-align: middle;\"><td><img src=\":/icons/help.png\"></td>"
                              "<td><p style=\"text-align:justify; margin-left:10px;\"> This step generates <b>the most reliable displacement</b> between each pair of adjacent stacks "
                              "by exploiting the redundancy produced by the computation of multiple displacements along D (one for each layer).<br><br>"
                              "Where no computed displacement is available, a displacement is generated using nominal stage coordinates.<br><br>"
                              "Only metadata are processed in this step, so it will take no more than just a few seconds.</p> </td></tr></table> </html>");
    helpbox->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); margin-top:10px; margin-bottom:10px; padding-top:10px; padding-bottom:10px;");
    helpbox->setWordWrap(true);
    helpbox->setFixedHeight(180);

    //other widgets
    saveproj_label = new QLabel("Save project XML to:");
    saveproj_field = new QLineEdit();
    browse_button = new QPushButton("Save to XML...");
    total_displ_number_label = new QLabel("# of displacements:");
    total_displ_number_field = new QLabel("");
    total_displ_number_field->setAlignment(Qt::AlignCenter);
    total_displ_number_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");
    per_stack_displ_number_label = new QLabel("# of displacements per stacks pair:");
    per_stack_displ_number_field = new QLabel("");
    per_stack_displ_number_field->setAlignment(Qt::AlignCenter);
    per_stack_displ_number_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245)");

    /*** LAYOUT SECTIONS ***/

    //widgets
    QGridLayout* gridlayout = new QGridLayout();
    gridlayout->addWidget(saveproj_label, 0, 0, 1, 2);
    gridlayout->addWidget(saveproj_field, 0, 2, 1, 10);
    gridlayout->addWidget(browse_button, 0, 12, 1, 1);
    gridlayout->addWidget(total_displ_number_label, 1, 0, 1, 2);
    gridlayout->addWidget(total_displ_number_field, 1, 2, 1, 2);
    gridlayout->addWidget(per_stack_displ_number_label, 2, 0, 1, 2);
    gridlayout->addWidget(per_stack_displ_number_field, 2, 2, 1, 2);
    QWidget *container = new QWidget();
    container->setLayout(gridlayout);

    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);
    layout->addWidget(helpbox);
    layout->addWidget(container);
    setLayout(layout);

    //wait animated GIF tab icon
    wait_movie = new QMovie(":/icons/wait.gif");
    wait_label = new QLabel(this);
    wait_label->setMovie(wait_movie);

    // signals and slots    
    connect(browse_button, SIGNAL(clicked()), this, SLOT(browse_button_clicked()));
}


PTabDisplProj::~PTabDisplProj()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplProj destroyed\n", this->thread()->currentThreadId());
    #endif
}

/*********************************************************************************
* Start/Stop methods associated to the current step.
* They are called by the startButtonClicked/stopButtonClicked methods of <PMain>
**********************************************************************************/
void PTabDisplProj::start()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplProj start() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        //first checking that a volume has been properly imported
        if(!CImport::instance()->getVolume())
            throw MyException("A volume must be properly imported first. Please perform the Import step.");

        //asking confirmation to continue if no displacements were found
        if(total_displ_number_field->text().toInt() == 0 &&
           QMessageBox::information(this, "Warning", "No computed displacements found. \n\nDisplacements will be generated using nominal stage coordinates.", "Continue", "Cancel"))
        {
            PMain::instance()->resetGUI();
            return;
        }

        //asking confirmation to continue when overwriting existing XML file
        if( StackedVolume::fileExists(saveproj_field->text().toStdString().c_str()) &&
              QMessageBox::information(this, "Warning", "An XML file with the same name was found and it will be overwritten.", "Continue", "Cancel"))
        {
            PMain::instance()->resetGUI();
            return;
        }

        //disabling import form and enabling progress bar animation and tab wait animation
        PMain::instance()->getProgressBar()->setEnabled(true);
        PMain::instance()->getProgressBar()->setMinimum(0);
        PMain::instance()->getProgressBar()->setMaximum(0);
        wait_movie->start();
        container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, wait_label);

        //performing operation
        StackedVolume* volume = CImport::instance()->getVolume();
        if(!volume)
            throw MyException("Unable to start this step. A volume must be properly imported first.");
        StackStitcher stitcher(volume);
        stitcher.projectDisplacements();
        volume->saveXML(0, saveproj_field->text().toStdString().c_str());

        //enabling (and updating) other tabs
        this->setEnabled(true);
        PTabDisplThresh::getInstance()->setEnabled(true);
        PTabPlaceTiles::getInstance()->setEnabled(true);
        PTabMergeTiles::getInstance()->setEnabled(true);

        stop();
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::instance()->resetGUI();
    }
}

void PTabDisplProj::stop()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplProj stop() launched\n", this->thread()->currentThreadId());
    #endif

    //re-enabling import form and disabling progress bar and wait animations
    PMain::instance()->resetGUI();
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
}

/**********************************************************************************
* Opens file dialog to select the XML file to be saved.
* Called when user clicks on "browse_button".
***********************************************************************************/
void PTabDisplProj::browse_button_clicked()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplProj browse_button_clicked() launched\n", this->thread()->currentThreadId());
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
* Overrides QWidget's setEnabled(bool).
* If the widget is enabled, its fields are filled with the informations provided by
* the <StackedVolume> object of <CImport> instance.
***********************************************************************************/
void PTabDisplProj::setEnabled(bool enabled)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplProj setEnabled(%d) called\n", this->thread()->currentThreadId(), enabled);
    #endif

    //first calling super-class implementation
    QWidget::setEnabled(enabled);

    //then filling widget fields
    if(enabled && CImport::instance()->getVolume())
    {
        int total;
        float per_stack;
        CImport::instance()->getVolume()->countDisplacements(total, per_stack);
        total_displ_number_field->setText(QString::number(total));
        per_stack_displ_number_field->setText(QString::number(per_stack));

        QString saveproj_path = CImport::instance()->getVolume()->getSTACKS_DIR();
        saveproj_path.append("/xml_displproj.xml");
        saveproj_field->setText(saveproj_path);
    }
}
