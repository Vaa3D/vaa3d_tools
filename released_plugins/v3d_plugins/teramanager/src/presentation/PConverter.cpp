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
#include "PMain.h"
#include "../core/ImageManager/ProgressBar.h"
#include "../control/CConverter.h"
#include "../control/CSettings.h"

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
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    CConverter::uninstance();
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = 0;
    }
}

PConverter::PConverter(V3DPluginCallback *callback, QWidget *parent) : QWidget(parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    //initializing members
    V3D_env = callback;
    parentWidget = parent;

    //main widgets
    QFont tinyFont = QApplication::font();
    tinyFont.setPointSize(9);
    helpBox = new QHelpBox(this);
    helpBox->setFixedHeight(60);
    helpBox->setIconSize(35, 35);
    import_panel = new QGroupBox("Step 1: Import volume from:");
    import_panel->setStyle(new QWindowsStyle());
    conversion_panel = new QGroupBox("Step 2: Convert volume to:");
    conversion_panel->setStyle(new QWindowsStyle());
    progressBar = new QProgressBar(this);
    startButton = new QPushButton(this);
    startButton->setIcon(QIcon(":/icons/start.png"));
    startButton->setText("Start");
    stopButton = new QPushButton(this);
    stopButton->setIcon(QIcon(":/icons/stop.png"));
    stopButton->setText("Stop");
    statusBar = new QStatusBar();
    statusBar->setFont(QFont("",8));

    //import form widget
    inFormatCBox = new QComboBox();
    inFormatCBox->insertItem(0, "Image series (tiled)");
    inFormatCBox->insertItem(1, "Image series (nontiled)");
    inFormatCBox->insertItem(2, "3D TIFF (tiled)");
    inFormatCBox->insertItem(3, "3D TIFF (nontiled)");
    inFormatCBox->insertItem(4, "Vaa3D raw");
    inFormatCBox->insertItem(5, "Vaa3D raw (tiled, RGB)");
    inFormatCBox->insertItem(6, "Vaa3D raw (tiled, 4D)");
    inFormatCBox->insertItem(7, "Vaa3D raw (series)");
    PMain::setEnabledComboBoxItem(inFormatCBox, 2, false);
    PMain::setEnabledComboBoxItem(inFormatCBox, 3, false);
    inFormatCBox->setEditable(true);
    inFormatCBox->lineEdit()->setReadOnly(true);
    inFormatCBox->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < inFormatCBox->count(); i++)
    {
      inFormatCBox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
      if(inFormatCBox->itemText(i).compare(CSettings::instance()->getVCInputFormat().c_str()) == 0)
          inFormatCBox->setCurrentIndex(i);
    }
    inPathField    = new QLineEdit(QString(CSettings::instance()->getVCInputPath().c_str()));
    inDirButton = new QPushButton("Browse for dir...");
    inFileButton = new QPushButton("Browse for file...");

    //import form layout
    inButtonLayout = new QStackedLayout();
    inButtonLayout->addWidget(inDirButton);
    inButtonLayout->addWidget(inFileButton);
    QHBoxLayout* importFormLayout = new QHBoxLayout();
    importFormLayout->addWidget(inFormatCBox);
    inFormatCBox->setFixedWidth(220);
    importFormLayout->addWidget(inPathField, 1);
    importFormLayout->addLayout(inButtonLayout);
    import_panel->setLayout(importFormLayout);

    //conversion form widget
    outFormatCBox = new QComboBox();
    outFormatCBox->insertItem(0, "Image series (tiled)");
    outFormatCBox->insertItem(1, "Image series (nontiled)");
    outFormatCBox->insertItem(2, "3D TIFF (tiled)");
    outFormatCBox->insertItem(3, "3D TIFF (nontiled)");
    outFormatCBox->insertItem(4, "Vaa3D raw");
    outFormatCBox->insertItem(5, "Vaa3D raw (tiled, RGB)");
    outFormatCBox->insertItem(6, "Vaa3D raw (tiled, 4D)");
    outFormatCBox->insertItem(7, "Vaa3D raw (series)");
    PMain::setEnabledComboBoxItem(outFormatCBox, 1, false);
    PMain::setEnabledComboBoxItem(outFormatCBox, 2, false);
    PMain::setEnabledComboBoxItem(outFormatCBox, 3, false);
    PMain::setEnabledComboBoxItem(outFormatCBox, 4, false);
    PMain::setEnabledComboBoxItem(outFormatCBox, 7, false);
    outFormatCBox->setEditable(true);
    outFormatCBox->lineEdit()->setReadOnly(true);
    outFormatCBox->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < outFormatCBox->count(); i++)
    {
      outFormatCBox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
      if(outFormatCBox->itemText(i).compare(CSettings::instance()->getVCOutputFormat().c_str()) == 0)
          outFormatCBox->setCurrentIndex(i);
    }
    outPathField    = new QLineEdit(QString(CSettings::instance()->getVCOutputPath().c_str()));
    outDirButton = new QPushButton("Browse for dir...");
    outFileButton = new QPushButton("Browse for file...");
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
    stacksWidthField->setSuffix(" (X)");
    stacksHeightField = new QSpinBox();
    stacksHeightField->setAlignment(Qt::AlignCenter);
    stacksHeightField->setMinimum(100);
    stacksHeightField->setMaximum(10000);
    stacksHeightField->setSuffix(" (Y)");
    stacksHeightField->setValue(CSettings::instance()->getVCStacksHeight());    
    stacksDepthField = new QSpinBox();
    stacksDepthField->setAlignment(Qt::AlignCenter);
    stacksDepthField->setMinimum(100);
    stacksDepthField->setMaximum(10000);
    stacksDepthField->setSuffix(" (Z)");
    stacksDepthField->setValue(CSettings::instance()->getVCStacksDepth());
    memoryField = new QLabel();
    memoryField->setAlignment(Qt::AlignLeft);
    downsamplingCbox = new QComboBox(this);
    downsamplingCbox->addItem(QString("Mean (2").append(QChar(0x00D7)).append("2").append(QChar(0x00D7)).append("2)"));
    downsamplingCbox->addItem(QString("Max  (2").append(QChar(0x00D7)).append("2").append(QChar(0x00D7)).append("2)"));

    //conversion form layout
    QVBoxLayout* conversionFormLayout = new QVBoxLayout();
    outButtonLayout = new QStackedLayout();
    outButtonLayout->addWidget(outDirButton);
    outButtonLayout->addWidget(outFileButton);
    QHBoxLayout* outputFormLayout = new QHBoxLayout();
    outputFormLayout->addWidget(outFormatCBox);
    outFormatCBox->setFixedWidth(220);
    outputFormLayout->addWidget(outPathField, 1);
    outputFormLayout->addLayout(outButtonLayout);
    conversionFormLayout->addLayout(outputFormLayout);
    conversionFormLayout->addSpacing(30);
    QHBoxLayout* resolutionLayout = new QHBoxLayout();
    QLabel* outputLabel = new QLabel("Output:");
    outputLabel->setFixedWidth(220);
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
    conversionFormLayout->addSpacing(30);

    QHBoxLayout* stacksDimLayout = new QHBoxLayout();
    QLabel* stacksDimLabel = new QLabel("Stacks dims:");
    stacksDimLabel->setFixedWidth(220);
    stacksDimLayout->addWidget(stacksDimLabel);
    stacksDimLayout->addWidget(stacksWidthField);
    stacksWidthField->setFixedWidth(160);
    stacksDimLayout->addStretch(1);
    stacksDimLayout->addWidget(stacksHeightField);
    stacksHeightField->setFixedWidth(160);
    stacksDimLayout->addStretch(1);
    stacksDimLayout->addWidget(stacksDepthField);
    stacksDepthField->setFixedWidth(160);
    conversionFormLayout->addLayout(stacksDimLayout);

    QHBoxLayout* downSampleMethLayout = new QHBoxLayout();
    QLabel* downSampleMethLabel = new QLabel("Downsampling method:");
    downSampleMethLabel->setFixedWidth(220);
    downSampleMethLayout->addWidget(downSampleMethLabel);
    downSampleMethLayout->addWidget(downsamplingCbox, 0, Qt::AlignLeft);
    downsamplingCbox->setFixedWidth(160);
    downSampleMethLayout->addStretch(1);

    QHBoxLayout* ramLayout = new QHBoxLayout();
    QLabel* memoryLabel = new QLabel("Estimated RAM usage:");
    memoryLabel->setFixedWidth(220);
    ramLayout->addWidget(memoryLabel);
    ramLayout->addWidget(memoryField, 0, Qt::AlignLeft);
    ramLayout->addStretch(1);
    conversionFormLayout->addSpacing(10);
    conversionFormLayout->addLayout(downSampleMethLayout);
    conversionFormLayout->addSpacing(10);
    conversionFormLayout->addLayout(ramLayout);
    conversion_panel->setLayout(conversionFormLayout);
    conversion_panel->setEnabled(false);


    //overall layout
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(import_panel);
    layout->addSpacing(10);
    layout->addWidget(conversion_panel);
    layout->addSpacing(5);
    layout->addWidget(helpBox);
    layout->addStretch(1);
    layout->addWidget(statusBar);
    QWidget* container = new QWidget();
    QGridLayout* bottomBar = new QGridLayout();
    bottomBar->addWidget(progressBar, 0,0,1,18);
    bottomBar->addWidget(startButton, 0,18,1,1);
    bottomBar->addWidget(stopButton, 0,19,1,1);
    bottomBar->setContentsMargins(0,0,0,5);
    container->setLayout(bottomBar);
    layout->addWidget(container);    
    layout->setSpacing(0);
    layout->setContentsMargins(10,5,10,0);
    setLayout(layout);
    setWindowTitle(tr("TeraConverter"));
    this->setFixedSize(800,600);

    //signals and slots
    connect(startButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopButtonClicked()));
    connect(this, SIGNAL(sendProgressBarChanged(int, int, int, const char*)), this, SLOT(progressBarChanged(int, int, int, const char*)), Qt::QueuedConnection);
    connect(inFormatCBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(volformatChanged(QString)));
    connect(outFormatCBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(volformatChanged(QString)));
    connect(CConverter::instance(), SIGNAL(sendOperationOutcome(MyException*)), this, SLOT(operationDone(MyException*)), Qt::QueuedConnection);
    connect(inDirButton, SIGNAL(clicked()), this, SLOT(inDirButtonClicked()));
    connect(inFileButton, SIGNAL(clicked()), this, SLOT(inFileButtonClicked()));
    connect(outDirButton, SIGNAL(clicked()), this, SLOT(outDirButtonClicked()));
    connect(inPathField, SIGNAL(textChanged(QString)), this, SLOT(settingsChanged()));
    connect(inPathField, SIGNAL(editingFinished()), this, SLOT(startButtonClicked()));
    connect(outPathField, SIGNAL(textChanged(QString)), this, SLOT(settingsChanged()));
    connect(inFormatCBox, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsChanged()));
    connect(stacksWidthField, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
    connect(stacksHeightField, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
    resetGUI();

    //set always on top
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->setFocusPolicy(Qt::StrongFocus);

    /**/itm::debug(itm::LEV1, "object successfully created", __itm__current__function__);
}

PConverter::~PConverter()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);
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
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    try
    {
        CConverter::instance()->setMembers(this);
        import_panel->setEnabled(false);
        conversion_panel->setEnabled(false);
        progressBar->setEnabled(true);
        startButton->setEnabled(false);
        stopButton->setEnabled(true);
        if(!CConverter::instance()->isConversionModeEnabled())
        {
            statusBar->clearMessage();
            statusBar->showMessage("Importing volume...");
            progressBar->setMinimum(0);
            progressBar->setMaximum(0);
        }
        else
        {
            //if destination dir does not exist, asking for creation
            if(!QFile::exists(outPathField->text()))
            {
                if(!QMessageBox::information(this, "Create dir?", "The directory you selected does not exist. Create it?", "Yes", "Cancel"))
                    QDir().mkdir(outPathField->text());
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
                    return;
                }
            }


            //checking destination dir is empty
            QDir directory(outPathField->text());
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
    catch(MyException &ex) {QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));}
}

//called when stopButton has been clicked
void PConverter::stopButtonClicked()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    if(QMessageBox::information(this, "Warning", "Terminating this step can be unsafe and cause Vaa3D to crash. \n"
                                              "\nPlease save your data first or click on \"Cancel\" and close the "
                                              "plugin to terminate safely this process.", "Continue", "Cancel"))
        return;
    else
    {
        if(!CConverter::instance()->isConversionModeEnabled())
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

void PConverter::inDirButtonClicked()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    #ifdef _USE_QT_DIALOGS
    QString path;
    QFileDialog dialog(0);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    dialog.setWindowTitle("Select volume's folder");
    dialog.setDirectory(CSettings::instance()->getVCInputPath().c_str());
    if(dialog.exec())
        path = dialog.directory().absolutePath();

    #else
    QString path = QFileDialog::getExistingDirectory(this, "Select volume's folder", CSettings::instance()->getVCInputPath().c_str(), QFileDialog::ShowDirsOnly);
    #endif

    if(!path.isEmpty())
    {
        inPathField->setText(path);

        //launching import
        startButtonClicked();
    }
}

void PConverter::inFileButtonClicked()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    #ifdef _USE_QT_DIALOGS
    QString path = "";
    QFileDialog dialog(0);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    dialog.setWindowTitle("Select volume's file");
    dialog.setNameFilter(tr("V3D raw files (*.raw *.RAW *.v3draw *.V3DRAW)"));
    dialog.setDirectory(CSettings::instance()->getVCInputPath().c_str());
    if(dialog.exec())
       if(!dialog.selectedFiles().empty())
           path = dialog.selectedFiles().front();

    #else
    QString path = QFileDialog::getOpenFileName(this, "Select volume's file", CSettings::instance()->getVCInputPath().c_str(), tr("V3D raw files (*.raw *.RAW *.v3draw *.V3DRAW)"));
    #endif

    if(!path.isEmpty())
    {
        inPathField->setText(path);

        //launching import
        startButtonClicked();
    }
}

void PConverter::outDirButtonClicked()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    #ifdef _USE_QT_DIALOGS
    QString path;
    QFileDialog dialog(0);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    dialog.setWindowTitle("Select the directory where the converted volume has to be stored");
    dialog.setDirectory(CSettings::instance()->getVCOutputPath().c_str());
    if(dialog.exec())
        path = dialog.directory().absolutePath();

    #else
    QString path = QFileDialog::getExistingDirectory(this, "Select the directory where the converted volume has to be stored", CSettings::instance()->getVCOutputPath().c_str(), QFileDialog::ShowDirsOnly);
    #endif

    if(!path.isEmpty())
        outPathField->setText(path);
}

/**********************************************************************************
* Called when the GUI widgets that control application settings change.
* This is used to manage persistent platform-independent application settings.
***********************************************************************************/
void PConverter::settingsChanged()
{
    CSettings::instance()->setVCInputPath(inPathField->text().toStdString());
    CSettings::instance()->setVCOutputPath(outPathField->text().toStdString());
    CSettings::instance()->setVCInputFormat(inFormatCBox->currentText().toStdString());
    CSettings::instance()->setVCOutputFormat(outFormatCBox->currentText().toStdString());
    CSettings::instance()->setVCStacksWidth(stacksWidthField->value());
    CSettings::instance()->setVCStacksHeight(stacksHeightField->value());
    CSettings::instance()->setVCStacksDepth(stacksDepthField->value());
    CSettings::instance()->writeSettings();
}

void PConverter::volformatChanged ( const QString & text )
{
    QComboBox* sender = static_cast<QComboBox*>(QObject::sender());
    QStackedLayout *buttonLayout = 0;
    QPushButton *dirButton = 0;
    QPushButton *fileButton = 0;
    if(sender == outFormatCBox)
    {
        buttonLayout = outButtonLayout;
        dirButton    = outDirButton;
        fileButton   = outFileButton;
    }
    else
    {
        sender = inFormatCBox;
        buttonLayout = inButtonLayout;
        dirButton    = inDirButton;
        fileButton   = inFileButton;
    }

    if(sender->currentText().compare("Image series (tiled)", Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("Two-leveled folder structure (see <a href=\"http://code.google.com/p/terastitcher/wiki/SupportedFormats\">here</a>) with each tile composed "
                                  "by a series of 2D images.");
        buttonLayout->setCurrentWidget(dirButton);

        if(sender == outFormatCBox)
            stacksDepthField->setVisible(false);
    }
    else if(sender->currentText().compare("Image series (nontiled)", Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("A folder containing a series of 2D images. Supported formats for single "
                                  "2D images are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF.");
        buttonLayout->setCurrentWidget(dirButton);

        if(sender == outFormatCBox)
            stacksDepthField->setVisible(false);
    }
    else if(sender->currentText().compare("Vaa3D raw (tiled, RGB)", Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("Two-leveled folder structure (see <a href=\"http://code.google.com/p/terastitcher/wiki/SupportedFormats\">here</a>) with each tile composed "
                                  "by a series of 3D blocks stored into Vaa3D raw files containing up to 3 channels (RGB).");
        buttonLayout->setCurrentWidget(dirButton);

        if(sender == outFormatCBox)
            stacksDepthField->setVisible(true);
    }
    else if(sender->currentText().compare("Vaa3D raw (tiled, 4D)", Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("Three-leveled folder structure (first level for channels, other two levels for tiling X vs Y) with each tile composed "
                                  "by a series of 3D blocks stored into Vaa3D raw files containing a single channel.");
        buttonLayout->setCurrentWidget(dirButton);

        if(sender == outFormatCBox)
            stacksDepthField->setVisible(true);
    }
    else if(sender->currentText().compare("Vaa3D raw", Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("Vaa3D raw 4D format (single file). Supported suffixes are: .raw, .RAW, .v3draw, .V3DRAW");
        buttonLayout->setCurrentWidget(fileButton);

        if(sender == outFormatCBox)
            stacksDepthField->setVisible(false);
    }
    else if(sender->currentText().compare("Vaa3D raw (series)", Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("A folder containing a series of Vaa3D raw files. Supported suffixes are: .raw, .RAW, .v3draw, .V3DRAW");
        buttonLayout->setCurrentWidget(dirButton);

        if(sender == outFormatCBox)
            stacksDepthField->setVisible(false);
    }
    else
        helpBox->setText("<html><p style=\"text-align:justify;\"> Format not yet supported. </p></html>");
}

//overrides closeEvent method of QWidget
void PConverter::closeEvent(QCloseEvent *evt)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    if(progressBar->isEnabled() && QMessageBox::information(this, "Warning", "An operation is still in progress. Terminating it can be unsafe and cause Vaa3D to crash. \n"
                                                                    "\nPlease save your data first.", "Close TeraConverter plugin", "Cancel"))
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
    /**/itm::debug(itm::LEV1, strprintf("ex = %s", (ex? "error" : "0")).c_str(), __itm__current__function__);


    //if an exception has occurred, showing a message error
    if(ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
        if(!CConverter::instance()->isConversionModeEnabled())
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
    else if(CConverter::instance()->isConversionModeEnabled())
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
                if(height != 0 && width != 0 && depth != 0)
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
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

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
        float GBytes = (layer_height/1024.0f)*(layer_width/1024.0f)*(layer_depth/1024.0f)*vc->getVolume()->getCHANS()*vc->getVolume()->getBYTESxCHAN();
        memoryField->setText(QString::number(GBytes, 'f', 3).append(" GB"));
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}
