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

#include "PTabDisplThresh.h"
#include "MyException.h"
#include "StackedVolume.h"
#include "PMain.h"
#include "src/control/CImport.h"
#include "StackStitcher.h"

using namespace terastitcher;

/*********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "istance(...)"
**********************************************************************************/
PTabDisplThresh* PTabDisplThresh::uniqueInstance = NULL;
void PTabDisplThresh::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

PTabDisplThresh::PTabDisplThresh(QMyTabWidget* _container, int _tab_index) : QWidget(), container(_container), tab_index(_tab_index)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplThresh created\n", this->thread()->currentThreadId());
    #endif

    //other widgets
    saveproj_label = new QLabel("Save project XML to:");
    saveproj_field = new QLineEdit();
    saveproj_field->setFont(QFont("",8));
    browse_button = new QPushButton("...");
    reliable_displ_number_label = new QLabel("Reliable displacements:");
    reliable_displ_number_field = new QLineEdit();
    reliable_displ_number_field->setReadOnly(true);
    reliable_displ_number_field->setAlignment(Qt::AlignCenter);
    stitchable_stacks_label = new QLabel("Stitchable stacks:");
    stitchable_stacks_field = new QLineEdit();
    stitchable_stacks_field->setReadOnly(true);
    stitchable_stacks_field->setAlignment(Qt::AlignCenter);
    threshold_label = new QLabel("Reliability threshold:");
    threshold_slider = new QSlider();
    threshold_slider->setOrientation(Qt::Horizontal);
    threshold_field = new QDoubleSpinBox();
    threshold_field->setAlignment(Qt::AlignCenter);
    threshold_field->setSuffix("/1,00");
    per_stack_displ_number_label = new QLabel("Per-pair displacements:");
    per_stack_displ_number_field = new QLineEdit();
    per_stack_displ_number_field->setReadOnly(true);
    per_stack_displ_number_field->setAlignment(Qt::AlignCenter);


    /*** LAYOUT SECTIONS ***/    
    //widgets
    QGridLayout* gridlayout = new QGridLayout();
    saveproj_label->setFixedWidth(200);
    browse_button->setFixedWidth(80);
    gridlayout->addWidget(saveproj_label, 0, 0, 1, 1);
    gridlayout->addWidget(saveproj_field, 0, 1, 1, 10);
    gridlayout->addWidget(browse_button, 0, 11, 1, 1);
    gridlayout->addWidget(threshold_label, 1, 0, 1, 1);
    gridlayout->addWidget(threshold_slider, 1, 1, 1, 8);
    gridlayout->addWidget(threshold_field, 1, 9, 1, 2);
    gridlayout->addWidget(reliable_displ_number_label, 2, 0, 1, 1);
    gridlayout->addWidget(reliable_displ_number_field, 2, 1, 1, 3);
    gridlayout->addWidget(stitchable_stacks_label, 3, 0, 1, 1);
    gridlayout->addWidget(stitchable_stacks_field, 3, 1, 1, 3);
    gridlayout->addWidget(per_stack_displ_number_label, 4, 0, 1, 1);
    gridlayout->addWidget(per_stack_displ_number_field, 4, 1, 1, 3);    
    gridlayout->setContentsMargins(10,0,10,0);
    gridlayout->setVerticalSpacing(2);
    QWidget *container = new QWidget();
    container->setLayout(gridlayout);

    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);
    layout->addWidget(container);
    setLayout(layout);

    //wait animated GIF tab icon
    wait_movie = new QMovie(":/icons/wait.gif");
    wait_label = new QLabel(this);
    wait_label->setMovie(wait_movie);

    // signals and slots
    connect(browse_button, SIGNAL(clicked()), this, SLOT(browse_button_clicked()));
    connect(threshold_slider, SIGNAL(valueChanged(int)), this, SLOT(threshold_slider_changed(int)));
    connect(threshold_field, SIGNAL(valueChanged(double)), this, SLOT(threshold_spinner_changed(double)));

    reset();
}


PTabDisplThresh::~PTabDisplThresh()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplThresh destroyed\n", this->thread()->currentThreadId());
    #endif
}

//reset method
void PTabDisplThresh::reset()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplThresh::reset()\n", this->thread()->currentThreadId());
    #endif

    saveproj_field->setText("");
    per_stack_displ_number_field->setText("");
    threshold_slider->setMinimum(0);
    threshold_slider->setMaximum(100);
    threshold_slider->setValue(75);
    threshold_field->setMinimum(0);
    threshold_field->setMaximum(1);
    threshold_field->setValue(0.75);
    threshold_field->setSingleStep(0.01);
    reliable_displ_number_field->setText("");
    stitchable_stacks_field->setText("");
    per_stack_displ_number_field->setStyleSheet("");
    stitchable_stacks_field->setStyleSheet("");
    reliable_displ_number_field->setStyleSheet("");
    setEnabled(false);
}

/*********************************************************************************
* Start/Stop methods associated to the current step.
* They are called by the startButtonClicked/stopButtonClicked methods of <PMain>
**********************************************************************************/
void PTabDisplThresh::start()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplThresh start() launched\n", this->thread()->currentThreadId());
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
        stitcher.thresholdDisplacements(threshold_field->value());
        volume->saveXML(0, saveproj_field->text().toStdString().c_str());

        //showing operation successful message
        QMessageBox::information(this, "Operation successful", "Step successfully performed!", QMessageBox::Ok);

        //enabling (and updating) other tabs
        this->updateContent();
        PTabPlaceTiles::getInstance()->setEnabled(true);
        PTabMergeTiles::getInstance()->setEnabled(true);

        stop();
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::instance()->setToReady();
    }
}

void PTabDisplThresh::stop()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplThresh stop() launched\n", this->thread()->currentThreadId());
    #endif

    //re-enabling import form and disabling progress bar and wait animations
    PMain::instance()->setToReady();
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
}

/**********************************************************************************
* Overrides QWidget's setEnabled(bool).
* If the widget is enabled, its fields are filled with the informations provided by
* the <StackedVolume> object of <CImport> instance.
***********************************************************************************/
void PTabDisplThresh::setEnabled(bool enabled)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplThresh setEnabled(%d) called\n", this->thread()->currentThreadId(), enabled);
    #endif

    //first calling super-class implementation
    QWidget::setEnabled(enabled);

    //then filling widget fields
    if(enabled && CImport::instance()->getVolume())
    {
        //saving path
        QString saveproj_path = CImport::instance()->getVolume()->getSTACKS_DIR();
        saveproj_path.append("/xml_displthres.xml");
        saveproj_field->setText(saveproj_path);

        //enabling <reliable_displ_number_field> and <stitchable_stacks_field> only if
        //1 and only 1 displacement exists for each pair of stacks
        int total;
        float per_stack;
        CImport::instance()->getVolume()->countDisplacements(total, per_stack);
        per_stack_displ_number_field->setText(QString::number(per_stack));
        if(per_stack==1.0f)
            per_stack_displ_number_field->setStyleSheet("background-color: #7fdb73");
        else
            per_stack_displ_number_field->setStyleSheet("background-color: #ff4c4c");
        threshold_slider->setEnabled(per_stack==1.0f);
        threshold_field->setEnabled(per_stack==1.0f);
        threshold_label->setEnabled(per_stack==1.0f);
        reliable_displ_number_label->setEnabled(per_stack==1.0f);
        reliable_displ_number_field->setEnabled(per_stack==1.0f);
        stitchable_stacks_label->setEnabled(per_stack==1.0f);
        stitchable_stacks_field->setEnabled(per_stack==1.0f);

        updateContent();
    }
}

/**********************************************************************************
* Opens file dialog to select the XML file to be saved.
* Called when user clicks on "browse_button".
***********************************************************************************/
void PTabDisplThresh::browse_button_clicked()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplThresh browse_button_clicked() launched\n", this->thread()->currentThreadId());
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
* Syncronization between threshold's slider and spinner
***********************************************************************************/
void PTabDisplThresh::threshold_slider_changed(int val)
{
    threshold_field->setValue(val/100.0);
    updateContent();
}

void PTabDisplThresh::threshold_spinner_changed(double val)
{
    threshold_slider->setValue(val*100);
    updateContent();
}

/**********************************************************************************
* Updates the <reliable_displ_number_field> and <stitchable_stacks_field>
***********************************************************************************/
void PTabDisplThresh::updateContent()
{
    if(CImport::instance()->getVolume())
    {
        StackedVolume* volume = CImport::instance()->getVolume();
        int total, reliable, stitchables, nstacks=0;
        volume->countReliableSingleDirectionDisplacements(threshold_field->value(), total, reliable);
        stitchables = volume->countStitchableStacks(threshold_field->value());
        nstacks = volume->getN_ROWS() * volume->getN_COLS();

        reliable_displ_number_field->setText(QString::number(reliable).append("/").append(QString::number(total)));
        stitchable_stacks_field->setText(QString::number(stitchables).append("/").append(QString::number(nstacks)));

        if(stitchables == 0)
            stitchable_stacks_field->setStyleSheet("background-color: #ff4c4c");
        else
            stitchable_stacks_field->setStyleSheet("background-color: #7fdb73");

        if(reliable == 0)
            reliable_displ_number_field->setStyleSheet("background-color: #ff4c4c");
        else
            reliable_displ_number_field->setStyleSheet("background-color: #7fdb73");
    }
}
