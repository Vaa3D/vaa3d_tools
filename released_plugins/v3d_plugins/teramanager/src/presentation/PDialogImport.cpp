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

#include "PDialogImport.h"
#include "PMain.h"
#include "../control/CImport.h"

using namespace teramanager;

/*********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "istance(...)"
**********************************************************************************/
PDialogImport* PDialogImport::uniqueInstance = 0;
void PDialogImport::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

PDialogImport::PDialogImport(QWidget* parent) : QDialog(parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    //import form widgets
    QFont tinyFont = QApplication::font();
    tinyFont.setPointSize(10);
    QFont bigFont = QApplication::font();
    bigFont.setPointSize(14);
    import_form_desc_1 = new QLabel("<html><table><tr style=\"vertical-align: middle;\"><td><img src=\":/icons/help.png\"></td>"
                          "<td><p style=\"text-align:justify; margin-left:10px; margin-right:3px\"> TeraFly was unable to import this volume directly. That could "
                          "be caused by an unsupported volume format or by corrupt or missing <b>metadata</b> files. <br><br>"
                          "Please provide additional information through the form below and then click on the <i>Import volume</i> button. <br><br>If your volume's format "
                          "is not selectable from the list, you might consider to use <b>TeraConverter</b> to produce a format which is supported by TeraFly. </p> </td></tr></table> </html>");
    import_form_desc_1->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); margin-top:10px; margin-bottom:10px; padding-top:10px; padding-bottom:10px;");
    import_form_desc_1->setWordWrap(true);
    import_form_desc_1->setFont(tinyFont);

    inFormatCBox = new QComboBox();
    inFormatCBox->insertItem(0, iim::STACKED_FORMAT.c_str());
    inFormatCBox->insertItem(1, iim::SIMPLE_FORMAT.c_str());
    inFormatCBox->insertItem(2, iim::TILED_TIF3D_FORMAT.c_str());
    inFormatCBox->insertItem(3, iim::TIF3D_FORMAT.c_str());
    inFormatCBox->insertItem(4, iim::RAW_FORMAT.c_str());
    inFormatCBox->insertItem(5, iim::TILED_FORMAT.c_str());
    inFormatCBox->insertItem(6, iim::TILED_MC_FORMAT.c_str());
    inFormatCBox->insertItem(7, iim::SIMPLE_RAW_FORMAT.c_str());
    PMain::setEnabledComboBoxItem(inFormatCBox, 1, false);
    PMain::setEnabledComboBoxItem(inFormatCBox, 2, false);
    PMain::setEnabledComboBoxItem(inFormatCBox, 3, false);
    PMain::setEnabledComboBoxItem(inFormatCBox, 4, false);
    PMain::setEnabledComboBoxItem(inFormatCBox, 7, false);
    inFormatCBox->setEditable(true);
    inFormatCBox->lineEdit()->setReadOnly(true);
    inFormatCBox->lineEdit()->setAlignment(Qt::AlignCenter);
    tsCheckBox = new QCheckBox("5D (time series)");

    first_direction_label = new QLabel("First direction");
    second_direction_label = new QLabel("Second direction");
    third_direction_label = new QLabel("Third direction");
    first_direction_label->setFont(QFont("", 8));
    second_direction_label->setFont(QFont("", 8));
    third_direction_label->setFont(QFont("", 8));
    axes_label = new QLabel("Axes:");
    voxels_dims_label = new QLabel(QString("Voxel's dims (").append(QChar(0x03BC)).append("m):"));

    axs1_field = new QComboBox();
    axs1_field->addItem("X");
    axs1_field->addItem("-X");
    axs1_field->addItem("Y");
    axs1_field->addItem("-Y");
    axs1_field->addItem("Z");
    axs1_field->addItem("-Z");
    axs1_field->setEditable(true);
    axs1_field->lineEdit()->setReadOnly(true);
    axs1_field->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < axs1_field->count(); i++)
        axs1_field->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);

    axs2_field = new QComboBox();
    axs2_field->addItem("X");
    axs2_field->addItem("-X");
    axs2_field->addItem("Y");
    axs2_field->addItem("-Y");
    axs2_field->addItem("Z");
    axs2_field->addItem("-Z");
    axs2_field->setEditable(true);
    axs2_field->lineEdit()->setReadOnly(true);
    axs2_field->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < axs2_field->count(); i++)
        axs2_field->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);

    axs3_field = new QComboBox();
    axs3_field->addItem("X");
    axs3_field->addItem("-X");
    axs3_field->addItem("Y");
    axs3_field->addItem("-Y");
    axs3_field->addItem("Z");
    axs3_field->addItem("-Z");
    axs3_field->setEditable(true);
    axs3_field->lineEdit()->setReadOnly(true);
    axs3_field->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < axs3_field->count(); i++)
        axs3_field->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);

    vxl1_field = new QDoubleSpinBox();
    vxl1_field->setAlignment(Qt::AlignCenter);
    vxl2_field = new QDoubleSpinBox();
    vxl2_field->setAlignment(Qt::AlignCenter);
    vxl3_field = new QDoubleSpinBox();
    vxl3_field->setAlignment(Qt::AlignCenter);

    import_button = new QPushButton(" Import volume");
    import_button->setIcon(QIcon(":/icons/import.png"));
    import_button->setIconSize(QSize(30,30));
    import_button->setFixedHeight(50);
    import_button->setFont(bigFont);
    cancel_button = new QPushButton(" Cancel");
    cancel_button->setIcon(QIcon(":/icons/stop.png"));
    cancel_button->setIconSize(QSize(30,30));
    cancel_button->setFixedHeight(50);
    cancel_button->setFont(bigFont);


    /*** LAYOUT SECTION ***/
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(20,20,20,20);
    /* --------------- volume's format ---------------- */
    QHBoxLayout* volumeFormatLayout = new QHBoxLayout();
//    volumeFormatLayout->setContentsMargins(5,5,5,5);
    QLabel* volumeFormatLabel = new QLabel("Volume's format:");
    volumeFormatLabel->setFixedWidth(200);
    volumeFormatLayout->addWidget(volumeFormatLabel);
    volumeFormatLayout->addWidget(inFormatCBox, 1);
    volumeFormatLayout->addWidget(tsCheckBox);
    /* -------------- metadata row #1 ----------------- */
    QHBoxLayout* mdataRow1Layout = new QHBoxLayout();
//    mdataRow1Layout->setContentsMargins(5,5,5,5);
    mdataRow1Layout->addSpacing(200);
    first_direction_label->setAlignment(Qt::AlignCenter);
    second_direction_label->setAlignment(Qt::AlignCenter);
    third_direction_label->setAlignment(Qt::AlignCenter);
    mdataRow1Layout->addWidget(first_direction_label, 1);
    mdataRow1Layout->addWidget(second_direction_label, 1);
    mdataRow1Layout->addWidget(third_direction_label, 1);
    /* -------------- metadata row #2 ----------------- */
    QHBoxLayout* mdataRow2Layout = new QHBoxLayout();
//    mdataRow2Layout->setContentsMargins(5,5,5,5);
    axes_label->setFixedWidth(200);
    mdataRow2Layout->addWidget(axes_label);
    mdataRow2Layout->addWidget(axs1_field, 1);
    mdataRow2Layout->addWidget(axs2_field, 1);
    mdataRow2Layout->addWidget(axs3_field, 1);
    /* -------------- metadata row #3 ----------------- */
    QHBoxLayout* mdataRow3Layout = new QHBoxLayout();
//    mdataRow3Layout->setContentsMargins(5,5,5,5);
    voxels_dims_label->setFixedWidth(200);
    mdataRow3Layout->addWidget(voxels_dims_label);
    mdataRow3Layout->addWidget(vxl1_field, 1);
    mdataRow3Layout->addWidget(vxl2_field, 1);
    mdataRow3Layout->addWidget(vxl3_field, 1);
    /* ------------------ buttons --------------------- */
    QHBoxLayout* buttons_layout = new QHBoxLayout();
//    buttons_layout->setContentsMargins(5,5,5,5);
    buttons_layout->addWidget(import_button);
    buttons_layout->addWidget(cancel_button);
    /* ------------------ OVERALL --------------------- */
    layout->addWidget(import_form_desc_1);
    layout->addLayout(volumeFormatLayout);
    layout->addSpacing(20);
    layout->addLayout(mdataRow1Layout);
    layout->addLayout(mdataRow2Layout);
    layout->addLayout(mdataRow3Layout);
    layout->addSpacing(30);
    layout->addLayout(buttons_layout);
    layout->setSizeConstraint( QLayout::SetFixedSize );
    setLayout(layout);

    //windows flags and title
    char title[1024];
    sprintf(title, "Unable to import this volume");
    this->setWindowTitle(title);
    this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);

    // signals and slots
    connect(import_button, SIGNAL(clicked()), this, SLOT(import_button_clicked()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(close()));

//    setFixedSize(800, 600);
    reset();

    /**/itm::debug(itm::LEV1, "object successfully constructed", __itm__current__function__);
}


PDialogImport::~PDialogImport()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);
}

//reset method
void PDialogImport::reset()
{
    axs1_field->setCurrentIndex(0);
    axs2_field->setCurrentIndex(2);
    axs3_field->setCurrentIndex(4);
    vxl1_field->setMinimum(0.1);
    vxl1_field->setMaximum(1000.0);
    vxl1_field->setSingleStep(0.1);
    vxl1_field->setValue(1.0);
    vxl2_field->setMinimum(0.1);
    vxl2_field->setMaximum(1000.0);
    vxl2_field->setSingleStep(0.1);
    vxl2_field->setValue(1.0);
    vxl3_field->setMinimum(0.1);
    vxl3_field->setMaximum(1000.0);
    vxl3_field->setSingleStep(0.1);
    vxl3_field->setValue(1.0);
}

/**********************************************************************************
* Called when <import_button> emits <click()> signal.
***********************************************************************************/
void PDialogImport::import_button_clicked()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    try
    {
        CImport::instance()->setAxes(axs1_field->currentText().toStdString(),
                                     axs2_field->currentText().toStdString(),
                                     axs3_field->currentText().toStdString());
        CImport::instance()->setVoxels(static_cast<float>(vxl1_field->value()),
                                       static_cast<float>(vxl2_field->value()),
                                       static_cast<float>(vxl3_field->value()));
        CImport::instance()->setFormat(inFormatCBox->currentText().toStdString());
        CImport::instance()->setTimeSeries(tsCheckBox->isChecked());
        accept();
        hide();
        reset();
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}
