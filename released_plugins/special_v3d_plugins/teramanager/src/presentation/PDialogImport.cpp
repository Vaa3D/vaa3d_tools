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
#include "../core/ImageManager/MyException.h"
#include "../core/ImageManager/StackedVolume.h"
#include "PMain.h"
#include "../control/CImport.h"

using namespace teramanager;

/*********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "istance(...)"
**********************************************************************************/
PDialogImport* PDialogImport::uniqueInstance = NULL;
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
    #ifdef TSP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PDialogImport::PDialogImport()\n");
    #endif

    //import form widgets
    QFont tinyFont = QApplication::font();
    tinyFont.setPointSize(10);
    QFont bigFont = QApplication::font();
    bigFont.setPointSize(14);
    import_form_desc_1 = new QLabel("<html><table><tr style=\"vertical-align: middle;\"><td><img src=\":/icons/help.png\"></td>"
                          "<td><p style=\"text-align:justify; margin-left:10px;\"> The metadata binary file contains <b>volume descriptors</b> needed "
                          "to manage properly and efficiently all the images contained in the two-leveled hierarchical directory structure. It "
                          "does NOT contain absolute filepath, hence the entire volume can be easily moved or renamed. </p> <p style=\"text-align:justify; margin-left:10px;\">"
                          "The metadata binary file is <b> automatically saved </b> when a volume is imported for the first time or re-imported. "
                          "However, in these cases additional informations are required. </p> </td></tr></table> </html>");
    import_form_desc_1->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(245,245,245); margin-top:10px; margin-bottom:10px; padding-top:10px; padding-bottom:10px;");
    import_form_desc_1->setWordWrap(true);
    import_form_desc_1->setFont(tinyFont);
    import_form_desc_2 = new QLabel("<html><i>Please fill all the fields and pay attention that the minus \"-\" sign before an axis is interpreted as a rotation by 180 degrees.</i></html>");
    import_form_desc_2->setWordWrap(true);
    first_direction_label = new QLabel("First direction");
    second_direction_label = new QLabel("Second direction");
    third_direction_label = new QLabel("Third direction");
    axes_label = new QLabel("Axes (1 = Vertical, 2 = Horizontal, 3 = Depth)");
    voxels_dims_label = new QLabel("Voxel's dimensions (micrometers)");
    QRegExp axs_regexp("^-?[123]$");
    QRegExp vxl_regexp("^[0-9]+\\.?[0-9]*$");
    axs1_field = new QLineEdit();
    axs1_field->setAlignment(Qt::AlignCenter);
    axs1_field->setValidator(new QRegExpValidator(axs_regexp, axs1_field));
    axs2_field = new QLineEdit();
    axs2_field->setAlignment(Qt::AlignCenter);
    axs2_field->setValidator(new QRegExpValidator(axs_regexp, axs2_field));
    axs3_field = new QLineEdit();
    axs3_field->setAlignment(Qt::AlignCenter);
    axs3_field->setValidator(new QRegExpValidator(axs_regexp, axs3_field));
    vxl1_field = new QLineEdit();
    vxl1_field->setAlignment(Qt::AlignCenter);
    vxl1_field->setValidator(new QRegExpValidator(vxl_regexp, vxl1_field));
    vxl2_field = new QLineEdit();
    vxl2_field->setAlignment(Qt::AlignCenter);
    vxl2_field->setValidator(new QRegExpValidator(vxl_regexp, vxl2_field));
    vxl3_field = new QLineEdit();
    vxl3_field->setAlignment(Qt::AlignCenter);
    vxl3_field->setValidator(new QRegExpValidator(vxl_regexp, vxl3_field));
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

    QWidget* container = new QWidget();
    QGridLayout* grid_layout = new QGridLayout();
    grid_layout->addWidget(first_direction_label, 0, 2, 1, 1, Qt::AlignHCenter);
    grid_layout->addWidget(second_direction_label, 0, 3, 1, 1, Qt::AlignHCenter);
    grid_layout->addWidget(third_direction_label, 0, 4, 1, 1, Qt::AlignHCenter);
    grid_layout->addWidget(axes_label, 1, 0, 1, 2);
    grid_layout->addWidget(axs1_field, 1, 2, 1, 1, Qt::AlignHCenter);
    grid_layout->addWidget(axs2_field, 1, 3, 1, 1, Qt::AlignHCenter);
    grid_layout->addWidget(axs3_field, 1, 4, 1, 1, Qt::AlignHCenter);
    grid_layout->addWidget(voxels_dims_label, 2, 0, 1, 2);
    grid_layout->addWidget(vxl1_field, 2, 2, 1, 1, Qt::AlignHCenter);
    grid_layout->addWidget(vxl2_field, 2, 3, 1, 1, Qt::AlignHCenter);
    grid_layout->addWidget(vxl3_field, 2, 4, 1, 1, Qt::AlignHCenter);
    container->setLayout(grid_layout);
    QHBoxLayout* buttons_layout = new QHBoxLayout();
    buttons_layout->addWidget(import_button);
    buttons_layout->addWidget(cancel_button);

    layout->addWidget(import_form_desc_1);
    layout->addWidget(import_form_desc_2);
    layout->addWidget(container);
    layout->addLayout(buttons_layout);

    layout->setSizeConstraint( QLayout::SetFixedSize );
    setLayout(layout);

    //windows flags and title
    char title[IM_STATIC_STRINGS_SIZE];
    sprintf(title, "\"%s\" metadata file not found", IM_METADATA_FILE_NAME);
    this->setWindowTitle(title);
    this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);

    // signals and slots
    connect(import_button, SIGNAL(clicked()), this, SLOT(import_button_clicked()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(close()));

    #ifdef TSP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PDialogImport created\n");
    #endif
}


PDialogImport::~PDialogImport()
{
    #ifdef TSP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PDialogImport::~PDialogImport()\n");
    printf("--------------------- teramanager plugin [thread *] >> PDialogImport destroyed\n");
    #endif
}

//reset method
void PDialogImport::reset()
{
    axs1_field->setText("");
    axs2_field->setText("");
    axs3_field->setText("");
    vxl1_field->setText("");
    vxl2_field->setText("");
    vxl3_field->setText("");
}

/**********************************************************************************
* Called when <import_button> emits <click()> signal.
***********************************************************************************/
void PDialogImport::import_button_clicked()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PDialogImport::import_button_clicked()\n");
    #endif

    try
    {
        int pos=0;
        QString tbv = axs1_field->text();
        if(axs1_field->validator()->validate(tbv,pos) != QValidator::Acceptable)
        {
            axs1_field->setFocus();
            throw MyException("One or more fields not properly filled");
        }
        tbv = axs2_field->text();
        if(axs2_field->validator()->validate(tbv,pos) != QValidator::Acceptable)
        {
            axs2_field->setFocus();
            throw MyException("One or more fields not properly filled");
        }
        tbv = axs3_field->text();
        if(axs3_field->validator()->validate(tbv,pos) != QValidator::Acceptable)
        {
            axs3_field->setFocus();
            throw MyException("One or more fields not properly filled");
        }
        tbv = vxl1_field->text();
        if(vxl1_field->validator()->validate(tbv,pos) != QValidator::Acceptable)
        {
            vxl1_field->setFocus();
            throw MyException("One or more fields not properly filled");
        }
        tbv = vxl2_field->text();
        if(vxl2_field->validator()->validate(tbv,pos) != QValidator::Acceptable)
        {
            vxl2_field->setFocus();
            throw MyException("One or more fields not properly filled");
        }
        tbv = vxl3_field->text();
        if(vxl3_field->validator()->validate(tbv,pos) != QValidator::Acceptable)
        {
            vxl3_field->setFocus();
            throw MyException("One or more fields not properly filled");
        }
        CImport::instance()->setAxes(axs1_field->text().toStdString().c_str(),
                                     axs2_field->text().toStdString().c_str(),
                                     axs3_field->text().toStdString().c_str());
        CImport::instance()->setVoxels(vxl1_field->text().toStdString().c_str(),
                                       vxl2_field->text().toStdString().c_str(),
                                       vxl3_field->text().toStdString().c_str());
        accept();
        hide();
        reset();
    }
    catch(MyException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}
