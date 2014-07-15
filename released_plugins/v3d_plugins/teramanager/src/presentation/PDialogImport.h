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

#ifndef PDialogImport_H
#define PDialogImport_H

#include <QWidget>
#include <QtGui>
#include "../control/CPlugin.h"

class teramanager::PDialogImport : public QDialog
{
    Q_OBJECT

    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PDialogImport* uniqueInstance;
        PDialogImport(QWidget* parent);

        //widgets
        QLabel* import_form_desc_1;     //contains text describing import form usage
        QComboBox* inFormatCBox;        //combobox for volume's input format selection
        QCheckBox* tsCheckBox;          //checkbox to mark the volume to be imported as a time series
        QLabel* first_direction_label;  //label "First direction"
        QLabel* second_direction_label; //label "Second direction"
        QLabel* third_direction_label;  //label "Third direction"
        QLabel* axes_label;             //label "Axes"
        QComboBox *axs1_field;          //field for first direction axis
        QComboBox *axs2_field;          //field for second direction axis
        QComboBox *axs3_field;          //field for third direction axis
        QLabel* voxels_dims_label;      //label "Voxel dimensions"
        QDoubleSpinBox *vxl1_field;     //field for voxel dimension along first direction
        QDoubleSpinBox *vxl2_field;     //field for voxel dimension along second direction
        QDoubleSpinBox *vxl3_field;     //field for voxel dimension along third direction
        QPushButton* import_button;     //ok button
        QPushButton* cancel_button;     //cancel button

    public:

        /**********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        ***********************************************************************************/
        static PDialogImport* instance(QWidget* parent)
        {
            if (uniqueInstance == 0)
                uniqueInstance = new PDialogImport(parent);
            return uniqueInstance;
        }
        static PDialogImport* getInstance(){return uniqueInstance;}
        static void uninstance();
        ~PDialogImport();

        //reset method
        void reset();

    public slots:

        /**********************************************************************************
        * Called when <import_button> emits <click()> signal.
        ***********************************************************************************/
        void import_button_clicked();
};

#endif // PDialogImport_H
