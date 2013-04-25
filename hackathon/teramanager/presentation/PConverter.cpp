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

#include "PConverter.h"
#include "ProgressBar.h"
#include "control/CConverter.h"
#include "control/CSettings.h"

using namespace teramanager;

PConverter* PConverter::uniqueInstance = NULL;
PConverter* PConverter::instance(V3DPluginCallback *callback, QWidget *parent)
{
    if (uniqueInstance == NULL)
        uniqueInstance = new PConverter(callback, parent);
    return uniqueInstance;
}
void PConverter::uninstance()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> PConverter::uninstance()\n");
    #endif

    CConverter::uninstance();
    CSettings::uninstance();
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

PConverter::PConverter(V3DPluginCallback *callback, QWidget *parent) : QWidget(parent)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PConverter::PConverter()\n", this->thread()->currentThreadId()%10);
    #endif

    //initializing members
    V3D_env = callback;
    parentWidget = parent;

    //main widgets
    QFont tinyFont = QApplication::font();
    tinyFont.setPointSize(10);
    import_panel = new QGroupBox("Step 1: Import volume");
    import_panel->setStyle(new QWindowsStyle());
    conversion_panel = new QGroupBox("Step 2: Convert volume to TeraFly format");
    conversion_panel->setStyle(new QWindowsStyle());
    progressBar = new QProgressBar(this);
    startButton = new QPushButton(this);
    startButton->setIcon(QIcon(":/icons/start.png"));
    startButton->setText("Start");
    stopButton = new QPushButton(this);
    stopButton->setIcon(QIcon(":/icons/stop.png"));
    stopButton->setText("Stop");
    statusBar = new QStatusBar();
    statusBar->showMessage("Ready to import volume.");

    //import form widget
    volformatCombobox = new QComboBox();
    volformatCombobox->insertItem(0, "TeraStitcher");
    volformatCombobox->insertItem(1, "Image series");
    volformatCombobox->insertItem(2, "Vaa3D raw");
    volformatCombobox->setFixedWidth(200);
    volformatCombobox->setEditable(true);
    volformatCombobox->lineEdit()->setReadOnly(true);
    volformatCombobox->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < volformatCombobox->count(); i++)
    {
      volformatCombobox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
      if(volformatCombobox->itemText(i).compare(CSettings::instance()->getVCFormat().c_str()) == 0)
          volformatCombobox->setCurrentIndex(i);
    }
    volformatHelpbox = new QLabel();
    volformatHelpbox->setStyleSheet("border: 1px solid; border-color: rgb(71,127,249); background-color: rgb(245,245,245); margin-top:0px; "
                                    "margin-bottom:0px; margin-left: 27px; padding-top:4px; padding-bottom:4px; padding-left:23px; text-align:justify;");
    volformatHelpbox->setWordWrap(true);
    volformatHelpbox->setFont(tinyFont);
    volformatHelpbox->setFixedWidth(700);
    helpIcon = new QLabel("<html><table><tr style=\"vertical-align: middle;\"><td><img src=\":/icons/help.png\"></td></tr></html>");
    helpIcon->setStyleSheet("border: none; background-color: none;");

    volpathField    = new QLineEdit(QString(CSettings::instance()->getVCInputPath().c_str()));
    volpathField->setFixedWidth(700);
    voldirButton = new QPushButton("Browse for dir...");
    volfileButton = new QPushButton("Browse for file...");
    voldirButton->setFixedWidth(200);
    volfileButton->setFixedWidth(200);

    //conversion form widget
    voloutpathField    = new QLineEdit(QString(CSettings::instance()->getVCOutputPath().c_str()));
    voloutpathField->setFixedWidth(700);
    voloutdirButton = new QPushButton("Browse for dir...");
    voloutdirButton->setFixedWidth(200);
    resolutionsLayout = new QGridLayout();
    resolutionsNumber = -1;
    resolutionsFields = 0;
    resolutionsSizes = 0;
    resolutionsCboxs = 0;
    stacksWidthField = new QSpinBox();
    stacksWidthField->setAlignment(Qt::AlignCenter);
    stacksWidthField->setMinimum(100);
    stacksWidthField->setMaximum(10000);
    stacksWidthField->setValue(CSettings::instance()->getVCStacksWidth());
    stacksHeightField = new QSpinBox();
    stacksHeightField->setAlignment(Qt::AlignCenter);
    stacksHeightField->setMinimum(100);
    stacksHeightField->setMaximum(10000);
    stacksHeightField->setValue(CSettings::instance()->getVCStacksHeight());
    memoryField = new QLabel();
    memoryField->setAlignment(Qt::AlignLeft);

    //import form layout
    QVBoxLayout* importFormLayout = new QVBoxLayout();
    importFormLayout->addWidget(new QLabel("Select the format of the volume to be imported:"));
    QHBoxLayout* formatLayout = new QHBoxLayout();
    formatLayout->addWidget(volformatCombobox);
    QStackedLayout *helpLayout = new QStackedLayout();
    helpLayout->addWidget(volformatHelpbox);
    helpLayout->addWidget(helpIcon);
    helpLayout->setStackingMode(QStackedLayout::StackAll);
    formatLayout->addLayout(helpLayout);
    importFormLayout->addLayout(formatLayout);
    importFormLayout->addSpacing(20);
    importFormLayout->addWidget(new QLabel("Enter the directory or the file path of the volume to be imported:"));
    QHBoxLayout* volPathLayout = new QHBoxLayout();
    volButtonLayout = new QStackedLayout();
    volButtonLayout->addWidget(voldirButton);
    volButtonLayout->addWidget(volfileButton);
    volPathLayout->addLayout(volButtonLayout);
    volPathLayout->addWidget(volpathField);
    importFormLayout->addLayout(volPathLayout);
    import_panel->setLayout(importFormLayout);

    //conversion form layout
    QVBoxLayout* conversionFormLayout = new QVBoxLayout();
    QHBoxLayout* voloutPathLayout = new QHBoxLayout();
    voloutPathLayout->addWidget(voloutdirButton);
    voloutPathLayout->addWidget(voloutpathField);
    conversionFormLayout->addWidget(new QLabel("Destination folder:"));
    conversionFormLayout->addLayout(voloutPathLayout);
    conversionFormLayout->addSpacing(30);
    QHBoxLayout* resolutionLayout = new QHBoxLayout();
    QLabel* outputLabel = new QLabel("Output:");
    outputLabel->setFixedWidth(200);
    resolutionLayout->addWidget(outputLabel);
    QLabel* selectLabel = new QLabel("Select");
    selectLabel->setAlignment(Qt::AlignCenter);
    resolutionsLayout->addWidget(selectLabel, 0, 0, 1, 1);
    QLabel* resolutionLabel = new QLabel(QString("Resolution (X ").append(QChar(0x00D7)).append(" Y ").append(QChar(0x00D7)).append(" Z)"));
    resolutionLabel->setAlignment(Qt::AlignCenter);
    resolutionsLayout->addWidget(resolutionLabel, 0, 1, 1, 2);
    QLabel* sizeLabel = new QLabel("Size (GVoxels)");
    sizeLabel->setAlignment(Qt::AlignCenter);
    resolutionsLayout->addWidget(sizeLabel, 0, 3, 1, 2);
    resolutionLayout->addLayout(resolutionsLayout);
    conversionFormLayout->addLayout(resolutionLayout);
    conversionFormLayout->addSpacing(50);
    QHBoxLayout* stacksWidthLayout = new QHBoxLayout();
    QLabel* stacksWidthLabel = new QLabel("Stacks width:");
    stacksWidthLabel->setFixedWidth(200);
    stacksWidthLayout->addWidget(stacksWidthLabel);
    stacksWidthLayout->addWidget(stacksWidthField);
    stacksWidthLayout->addStretch(1);
    conversionFormLayout->addLayout(stacksWidthLayout);
    QHBoxLayout* stacksHeightLayout = new QHBoxLayout();
    QLabel* stacksHeightLabel = new QLabel("Stacks height:");
    stacksHeightLabel->setFixedWidth(200);
    stacksHeightLayout->addWidget(stacksHeightLabel);
    stacksHeightLayout->addWidget(stacksHeightField);
    stacksHeightLayout->addStretch(1);
    conversionFormLayout->addLayout(stacksHeightLayout);
    QHBoxLayout* ramLayout = new QHBoxLayout();
    QLabel* memoryLabel = new QLabel("Estimated RAM usage:");
    memoryLabel->setFixedWidth(200);
    ramLayout->addWidget(memoryLabel);
    ramLayout->addWidget(memoryField, 0, Qt::AlignLeft);
    ramLayout->addStretch(1);
    conversionFormLayout->addSpacing(10);
    conversionFormLayout->addLayout(ramLayout);
    conversion_panel->setLayout(conversionFormLayout);
    conversion_panel->setEnabled(false);


    //general layout
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(import_panel);
    layout->addWidget(conversion_panel);
    layout->addWidget(statusBar);
    QWidget* container = new QWidget();
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->addWidget(progressBar, 0,0,1,18);
    gridLayout->addWidget(startButton, 0,18,1,1);
    gridLayout->addWidget(stopButton, 0,19,1,1);
    container->setLayout(gridLayout);
    layout->addWidget(container);
    setLayout(layout);
    setWindowTitle(tr("TeraConverter"));
    layout->setSizeConstraint( QLayout::SetFixedSize );

    //signals and slots
    connect(startButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopButtonClicked()));
    connect(this, SIGNAL(sendProgressBarChanged(int, int, int, const char*)), this, SLOT(progressBarChanged(int, int, int, const char*)), Qt::QueuedConnection);
    connect(volformatCombobox, SIGNAL(currentIndexChanged(QString)), this, SLOT(volformatChanged(QString)));
    connect(CConverter::instance(), SIGNAL(sendOperationOutcome(MyException*)), this, SLOT(operationDone(MyException*)), Qt::QueuedConnection);
    connect(voldirButton, SIGNAL(clicked()), this, SLOT(voldirButtonClicked()));
    connect(volfileButton, SIGNAL(clicked()), this, SLOT(volfileButtonClicked()));
    connect(voloutdirButton, SIGNAL(clicked()), this, SLOT(voldiroutButtonClicked()));
    connect(volpathField, SIGNAL(textChanged(QString)), this, SLOT(settingsChanged()));
    connect(volpathField, SIGNAL(editingFinished()), this, SLOT(startButtonClicked()));
    connect(voloutpathField, SIGNAL(textChanged(QString)), this, SLOT(settingsChanged()));
    connect(volformatCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsChanged()));
    connect(stacksWidthField, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
    connect(stacksHeightField, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
    resetGUI();

    //set always on top
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->setFocusPolicy(Qt::StrongFocus);

    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PConverter created\n", this->thread()->currentThreadId()%10);
    #endif
}

PConverter::~PConverter()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PConverter::~PConverter()\n", this->thread()->currentThreadId()%10);
    printf("--------------------- teramanager plugin [thread %d] >> PConverter destroyed\n", this->thread()->currentThreadId()%10);
    #endif
}

//reset GUI method
void PConverter::resetGUI()
{
    progressBar->setEnabled(false);
    progressBar->setMaximum(1);         //needed to stop animation on some operating systems
    startButton->setEnabled(true);
    stopButton->setEnabled(false);    
    statusBar->clearMessage();
    statusBar->showMessage("Ready to import volume.");
    volformatChanged("none");
    import_panel->setEnabled(true);
}

//called when startButton has been clicked
void PConverter::startButtonClicked()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PConverter::startButtonClicked()\n", this->thread()->currentThreadId()%10);
    #endif

    CConverter::instance()->setMembers(this);
    import_panel->setEnabled(false);
    conversion_panel->setEnabled(false);
    progressBar->setEnabled(true);
    startButton->setEnabled(false);
    stopButton->setEnabled(true);
    if(!CConverter::instance()->getConversionMode())
    {
        statusBar->clearMessage();
        statusBar->showMessage("Importing volume...");
        progressBar->setMinimum(0);
        progressBar->setMaximum(0);
    }
    else
    {
        //checking destination dir is empty
        QDir directory(voloutpathField->text());
        QStringList dir_entries = directory.entryList();
        if(dir_entries.size() > 2 && QMessageBox::information(this, "Warning", "The directory you selected is NOT empty. \n\nIf you continue, the conversion "
                                               "process could fail if the directories to be created already exist in the given path.", "Continue", "Cancel"))
        {
            statusBar->clearMessage();
            statusBar->showMessage("Ready to convert volume.");
            progressBar->setEnabled(false);
            progressBar->setMaximum(1);         //needed to stop animation on some operating systems
            startButton->setEnabled(true);
            stopButton->setEnabled(false);
            import_panel->setEnabled(false);
            conversion_panel->setEnabled(true);
            return;
        }
        progressBar->setMinimum(0);
        progressBar->setMaximum(100);
    }
    CConverter::instance()->start();
}

//called when stopButton has been clicked
void PConverter::stopButtonClicked()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PConverter::stopButtonClicked()\n", this->thread()->currentThreadId()%10);
    #endif

    if(QMessageBox::information(this, "Warning", "Terminating this step can be unsafe and cause Vaa3D to crash. \n"
                                              "\nPlease save your data first or click on \"Cancel\" and close the "
                                              "plugin to terminate safely this process.", "Continue", "Cancel"))
        return;
    else
    {
        if(!CConverter::instance()->getConversionMode())
            this->resetGUI();
        else
        {
            statusBar->clearMessage();
            statusBar->showMessage("Ready to convert volume.");
            progressBar->setEnabled(false);
            progressBar->setMaximum(1);         //needed to stop animation on some operating systems
            startButton->setEnabled(true);
            stopButton->setEnabled(false);
            import_panel->setEnabled(false);
            conversion_panel->setEnabled(true);
        }
    }
}

void PConverter::voldirButtonClicked()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PConverter::voldirButtonClicked()\n", this->thread()->currentThreadId()%10);
    #endif

    //obtaining volume's directory
    volpathField->setText(QFileDialog::getExistingDirectory(this, QObject::tr("Select volume's directory"), CSettings::instance()->getVCInputPath().c_str()));

    //launching import
    if(volpathField->text().toStdString().compare("") != 0)
        startButtonClicked();
}

void PConverter::volfileButtonClicked()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PConverter::volfileButtonClicked()\n", this->thread()->currentThreadId()%10);
    #endif

    //obtaining volume's filepath
    volpathField->setText(QFileDialog::getOpenFileName(this, QObject::tr("Select Vaa3D raw file"), CSettings::instance()->getVCInputPath().c_str(), "V3D raw files (*.raw *.RAW *.v3draw *.V3DRAW)"));

    //launching import
    if(volpathField->text().toStdString().compare("") != 0)
        startButtonClicked();
}

void PConverter::voldiroutButtonClicked()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PConverter::voldiroutButtonClicked()\n", this->thread()->currentThreadId()%10);
    #endif

    //obtaining volume's directory
    voloutpathField->setText(QFileDialog::getExistingDirectory(this, QObject::tr("Select volume's directory"), CSettings::instance()->getVCOutputPath().c_str()));
}

/**********************************************************************************
* Called when the GUI widgets that control application settings change.
* This is used to manage persistent platform-independent application settings.
***********************************************************************************/
void PConverter::settingsChanged()
{
    CSettings::instance()->setVCInputPath(volpathField->text().toStdString());
    CSettings::instance()->setVCOutputPath(voloutpathField->text().toStdString());
    CSettings::instance()->setVCFormat(volformatCombobox->currentText().toStdString());
    CSettings::instance()->setVCStacksWidth(stacksWidthField->value());
    CSettings::instance()->setVCStacksHeight(stacksHeightField->value());
    CSettings::instance()->writeSettings();
}

void PConverter::volformatChanged ( const QString & text )
{
    if(volformatCombobox->currentText().compare("TeraStitcher", Qt::CaseInsensitive) == 0)
    {
        volformatHelpbox->setText("<html><p style=\"text-align:justify;\"> Two-leveled folder structure where each 2nd-level folder contains a "
                                  "stack of the corresponding row and column of the matrix of stacks (possibly composed by 1 single stack). Each "
                                  "image stack is stored as a series of 2D images. Supported formats for single 2D images are BMP, DIB, JPEG, "
                                  "JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF. </p></html>");
        volButtonLayout->setCurrentWidget(voldirButton);
    }
    else if(volformatCombobox->currentText().compare("Image series", Qt::CaseInsensitive) == 0)
    {
        volformatHelpbox->setText("<html><p style=\"text-align:justify;\"> A folder containing a series of 2D images. Supported formats for single "
                                  "2D images are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF. </p></html>");
        volButtonLayout->setCurrentWidget(voldirButton);
    }
    else if(volformatCombobox->currentText().compare("Vaa3D raw", Qt::CaseInsensitive) == 0)
    {
        volformatHelpbox->setText("<html><p style=\"text-align:justify;\"> Vaa3D raw 4D format. Supported suffixes are: .raw, .RAW, .v3draw, .V3DRAW </p></html>");
        volButtonLayout->setCurrentWidget(volfileButton);
    }
    else ;
}

//overrides closeEvent method of QWidget
void PConverter::closeEvent(QCloseEvent *evt)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PConverter::closeEvent()\n", this->thread()->currentThreadId()%10);
    #endif

    if(progressBar->isEnabled() && QMessageBox::information(this, "Warning", "An operation is still in progress. Terminating it can be unsafe and cause Vaa3D to crash. \n"
                                                                    "\nPlease save your data first.", "Close TeraStitcher plugin", "Cancel"))
    {
        evt->ignore();
    }
    else
    {
        evt->accept();
        PConverter::uninstance();
    }
}

/**********************************************************************************
* <sendProgressBarChanged> event handler
***********************************************************************************/
void PConverter::progressBarChanged(int val, int minutes, int seconds, const char* message)
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
* Called by <CConverter> when the associated operation has been performed.
* If an exception has occurred in the <CConverter> thread, it is propagated and
* managed in the current thread (ex != 0).
***********************************************************************************/
void PConverter::operationDone(MyException *ex)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PConverter::operationDone(%s) launched\n", this->thread()->currentThreadId()%10, (ex? "error" : ""));
    #endif


    //if an exception has occurred, showing a message error
    if(ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
        if(!CConverter::instance()->getConversionMode())
            this->resetGUI();
        else
        {
            statusBar->clearMessage();
            statusBar->showMessage("Ready to convert volume.");
            progressBar->setEnabled(false);
            progressBar->setMaximum(1);         //needed to stop animation on some operating systems
            startButton->setEnabled(true);
            stopButton->setEnabled(false);
            import_panel->setEnabled(false);
            conversion_panel->setEnabled(true);
        }
    }
    else if(CConverter::instance()->getConversionMode())
    {
        statusBar->clearMessage();
        statusBar->showMessage("Conversion successfully performed!");
        progressBar->setEnabled(false);
        progressBar->setMaximum(1);         //needed to stop animation on some operating systems
        startButton->setEnabled(false);
        stopButton->setEnabled(false);
        QMessageBox::information(this, "Success!", "Conversion successful!");
        close();
    }
    else
    {
        statusBar->clearMessage();
        statusBar->showMessage("Ready to convert volume.");
        progressBar->setEnabled(false);
        progressBar->setMaximum(1);         //needed to stop animation on some operating systems
        startButton->setEnabled(true);
        stopButton->setEnabled(false);
        conversion_panel->setEnabled(true);

        try
        {
            //generating at runtime the needed resolutions, which depend on the current volume size
            VolumeConverter *vc = CConverter::instance()->getVolumeConverter(); //throws an exception if volume has not been imported yet!
            resolutionsNumber = 0;
            bool deepest_resolution_reached = false;
            while(!deepest_resolution_reached)
            {
                int height = (vc->getV1()-vc->getV0())/pow(2.0f, resolutionsNumber);
                int width = (vc->getH1()-vc->getH0())/pow(2.0f, resolutionsNumber);
                int depth = (vc->getD1()-vc->getD0())/pow(2.0f, resolutionsNumber);
                float MVoxels = (height/1024.0f)*(width/1024.0f)*depth;
                if(MVoxels < CSettings::instance()->getVolMapSizeLimit() || resolutionsNumber >= S_MAX_MULTIRES)
                    deepest_resolution_reached = true;
                resolutionsNumber++;
            }
            resolutionsFields = new QLabel*[resolutionsNumber];
            resolutionsSizes = new QLabel*[resolutionsNumber];
            resolutionsCboxs = new QCheckBox*[resolutionsNumber];
            for(int i=0; i<resolutionsNumber; i++)
            {
                int height = (vc->getV1()-vc->getV0())/pow(2.0f, i);
                int width = (vc->getH1()-vc->getH0())/pow(2.0f, i);
                int depth = (vc->getD1()-vc->getD0())/pow(2.0f, i);
                float GBytes = (height/1024.0f)*(width/1024.0f)*(depth/1024.0f);

                resolutionsFields[i] = new QLabel(QString::number(height).append(" ").append(QChar(0x00D7)).append(" ").append(QString::number(width)).append(" ").append(QChar(0x00D7)).append(" ").append(QString::number(depth)));
                resolutionsFields[i]->setAlignment(Qt::AlignCenter);
                resolutionsSizes[i] = new QLabel(QString::number(GBytes,'f',3));
                resolutionsSizes[i]->setAlignment(Qt::AlignCenter);
                resolutionsCboxs[i] = new QCheckBox();
                resolutionsCboxs[i]->setStyleSheet("::indicator {subcontrol-position: center; subcontrol-origin: padding;}");
                resolutionsCboxs[i]->setChecked(true);
                resolutionsLayout->addWidget(resolutionsCboxs[i],  i+1, 0, 1, 1);
                resolutionsLayout->addWidget(resolutionsFields[i], i+1, 1, 1, 2);
                resolutionsLayout->addWidget(resolutionsSizes[i],  i+1, 3, 1, 2);

                connect(resolutionsCboxs[i], SIGNAL(stateChanged(int)), this, SLOT(updateContent()));

            }

            //updating content
            updateContent();
        }
        catch(MyException &ex) {QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));}
    }
}

/**********************************************************************************
* Updates widgets contents
***********************************************************************************/
void PConverter::updateContent()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread %d] >> PConverter::updateContent()\n", this->thread()->currentThreadId()%10);
    #endif

    try
    {
        //checking that at least one resolution has been selected
        bool selected = false;
        for(int i=0; i<resolutionsNumber; i++)
            selected = selected | resolutionsCboxs[i]->isChecked();
        if(!selected)
        {
            QMessageBox::warning(this,QObject::tr("Warning"), "At least one resolution must be selected",QObject::tr("Ok"));
            resolutionsCboxs[0]->setChecked(true);
        }

        //determining the maximum resolution selected
        VolumeConverter *vc = CConverter::instance()->getVolumeConverter(); //throws an exception if volume has not been imported yet!
        int max_res = 0;
        for(int i=0; i<resolutionsNumber; i++)
        {
            if(resolutionsCboxs[i]->isChecked())
                max_res = std::max(max_res, i);
        }

        //updating RAM usage estimation
        int layer_height = vc->getV1()-vc->getV0();
        int layer_width = vc->getH1()-vc->getH0();
        int layer_depth = pow(2.0f, max_res);
        float GBytes = (layer_height/1024.0f)*(layer_width/1024.0f)*(layer_depth/1024.0f)*4;
        memoryField->setText(QString::number(GBytes, 'f', 3).append(" GB"));
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}
