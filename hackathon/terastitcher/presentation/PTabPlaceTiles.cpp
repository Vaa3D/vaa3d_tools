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

#include "PTabPlaceTiles.h"
#include "MyException.h"
#include "StackedVolume.h"
#include "PMain.h"
#include "control/CImport.h"
#include "StackStitcher.h"

using namespace terastitcher;

/*********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "istance(...)"
**********************************************************************************/
PTabPlaceTiles* PTabPlaceTiles::uniqueInstance = NULL;
void PTabPlaceTiles::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

PTabPlaceTiles::PTabPlaceTiles(QMyTabWidget* _container, int _tab_index) : QWidget(), container(_container), tab_index(_tab_index)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabPlaceTiles created\n", this->thread()->currentThreadId());
    #endif

    //help box
    helpbox = new QLabel("<html><table><tr style=\"vertical-align: middle;\"><td><img src=\":/icons/help.png\"></td>"
                              "<td><p style=\"text-align:justify; margin-left:10px;\"> This step uses the computed displacements to <b>obtain the optimal tiles placement</b>.<br><br>"
                              "<u>If this step is skipped, tiles will be merged using only nominal stage coordinates.</u><br><br>"
                              "Furthermore, <b>this step needs each pair of adjacent stacks have one and only one displacement. If this is not the case, please run the Displacement Projections step first.</p> </td></tr></table> </html>");
    helpbox->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); margin-top:10px; margin-bottom:10px; padding-top:10px; padding-bottom:10px;");
    helpbox->setWordWrap(true);
    helpbox->setFixedHeight(180);

    //other widgets
    saveproj_label = new QLabel("Save project XML to:");
    saveproj_field = new QLineEdit();
    browse_button = new QPushButton("Save to XML...");
    algo_label = new QLabel("Algorithm");
    algo_cbox = new QComboBox();
    algo_cbox->insertItem(0, "Minimum Spanning Tree");
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
    gridlayout->addWidget(algo_label, 1, 0, 1, 2);
    gridlayout->addWidget(algo_cbox, 1, 2, 1, 6);
    gridlayout->addWidget(per_stack_displ_number_label, 4, 0, 1, 2);
    gridlayout->addWidget(per_stack_displ_number_field, 4, 2, 1, 2);
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


PTabPlaceTiles::~PTabPlaceTiles()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabPlaceTiles destroyed\n", this->thread()->currentThreadId());
    #endif
}

/*********************************************************************************
* Start/Stop methods associated to the current step.
* They are called by the startButtonClicked/stopButtonClicked methods of <PMain>
**********************************************************************************/
void PTabPlaceTiles::start()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabPlaceTiles start() launched\n", this->thread()->currentThreadId());
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
            return;
        }

        //checking that one and only one displacement exist for each pair od adjacent stacks
        if(per_stack_displ_number_field->text().toFloat() != 1.0f)
            throw MyException("One and only one displacement must exist for each pair of adjacent stacks. "
                              "\nTo this aim, the Displacement Projection step has to be performed first.");

        //disabling import form and enabling progress bar animation and tab wait animation
        PMain::instance()->getProgressBar()->setEnabled(true);
        PMain::instance()->getProgressBar()->setMinimum(0);
        PMain::instance()->getProgressBar()->setMaximum(0);
        wait_movie->start();
        container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, wait_label);

        //performing operation
        StackedVolume* volume = CImport::instance()->getVolume();
        StackStitcher stitcher(volume);
        stitcher.computeTilesPlacement(algo_cbox->currentIndex());
        volume->saveXML(0, saveproj_field->text().toStdString().c_str());

        //enabling (and updating) other tabs
        PTabMergeTiles::getInstance()->setEnabled(true);

        stop();
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::instance()->resetGUI();
    }
}

void PTabPlaceTiles::stop()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabPlaceTiles stop() launched\n", this->thread()->currentThreadId());
    #endif

    //re-enabling import form and disabling progress bar and wait animations
    PMain::instance()->resetGUI();
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
}

/**********************************************************************************
* Overrides QWidget's setEnabled(bool).
* If the widget is enabled, its fields are filled with the informations provided by
* the <StackedVolume> object of <CImport> instance.
***********************************************************************************/
void PTabPlaceTiles::setEnabled(bool enabled)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabPlaceTiles setEnabled(%d) called\n", this->thread()->currentThreadId(), enabled);
    #endif

    //first calling super-class implementation
    QWidget::setEnabled(enabled);

    //then filling widget fields
    if(enabled && CImport::instance()->getVolume())
    {
        //saving path
        QString saveproj_path = CImport::instance()->getVolume()->getSTACKS_DIR();
        saveproj_path.append("/xml_merging.xml");
        saveproj_field->setText(saveproj_path);

        //coloring <per_stack_displ_number_field>
        int total;
        float per_stack;
        CImport::instance()->getVolume()->countDisplacements(total, per_stack);
        per_stack_displ_number_field->setText(QString::number(per_stack));
        if(per_stack==1.0f)
            per_stack_displ_number_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: #7fdb73");
        else
            per_stack_displ_number_field->setStyleSheet("border: 1px solid; border-color: gray; background-color: #ff4c4c");
    }
}

/**********************************************************************************
* Opens file dialog to select the XML file to be saved.
* Called when user clicks on "browse_button".
***********************************************************************************/
void PTabPlaceTiles::browse_button_clicked()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabPlaceTiles browse_button_clicked() launched\n", this->thread()->currentThreadId());
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
