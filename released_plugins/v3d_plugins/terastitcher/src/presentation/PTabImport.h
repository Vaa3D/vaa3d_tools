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

#ifndef PTABIMPORT_H
#define PTABIMPORT_H

#include <QWidget>
#include <QtGui>
#include "src/control/CPlugin.h"
#include "QMyTabWidget.h"
#include "QPrefixSuffixLineEdit.h"

class terastitcher::PTabImport : public QWidget
{
    Q_OBJECT

    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PTabImport* uniqueInstance;
        PTabImport(QMyTabWidget* _container, int _tab_index);

        //members
        QMyTabWidget* container;        //tabs container
        int tab_index;                  //tab index

        //import form widgets
        QGroupBox* import_form;         //import form containing input fields
        QLabel* import_form_desc_1;     //contains text describing import form usage
        QLabel* import_form_desc_2;     //contains text describing import form usage
        QLineEdit *path_field;          //field for either volume's dir or project XML path
        QPushButton *voldir_button;     //browse for volume's directory button
        QPushButton *projfile_button;   //browse for volume's XML project file button
        QCheckBox *reimport_checkbox;   //checkbox to be used to reimport a volume already imported
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
        QLineEdit *regex_field;         //field for image filter regex
        QLabel *regex_label;

        //info panel widgets, contain informations of the loaded volume
        QGroupBox* info_panel;
        QLabel* volumedir_label;
        QLineEdit* volumedir_field;
        QLabel* volume_dims_label;
        QLabel* by_label_1;
        QLineEdit* nrows_field;
        QLineEdit* ncols_field;
        QLabel* stacks_dims_label;
        QLabel* by_label_2;
        QLabel* by_label_3;
        QLineEdit* stack_height_field;
        QLineEdit* stack_width_field;
        QLineEdit* stack_depth_field;
        QLabel* voxel_dims_label;
        QLabel* by_label_4;
        QLabel* by_label_5;
        QLineEdit* vxl_Y_field;
        QLineEdit* vxl_X_field;
        QLineEdit* vxl_Z_field;
        QLabel* origin_label;
        QLineEdit* org_Y_field;
        QLineEdit* org_X_field;
        QLineEdit* org_Z_field;
        QLabel* stacks_overlap_label;
        QLineEdit* ovp_Y_field;
        QLineEdit* ovp_X_field;
        QSpinBox* slice_spinbox;
        QPushButton* preview_button;
        QComboBox* channel_selection;

        //other widgets
        QMovie *wait_movie;             //animated wait GIF icon
        QLabel *wait_label;             //label containing animated wait GIF icon

    public:

        /**********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        ***********************************************************************************/
        static PTabImport* instance(QMyTabWidget* _container, int _tab_index)
        {
            if (uniqueInstance == NULL)
                uniqueInstance = new PTabImport(_container, _tab_index);
            return uniqueInstance;
        }
        static PTabImport* getInstance(){return uniqueInstance;}
        static void uninstance();
        ~PTabImport();

        /**********************************************************************************
        * Start/Stop methods associated to the current step.
        * They are called by the startButtonClicked/stopButtonClicked methods of <PMain>
        ***********************************************************************************/
        void start();
        void stop();

        //reset method
        void reset();

        //gives PMain instances public access to this class members
        friend class PMain;

    public slots:

        /**********************************************************************************
        * Called when "voldir_button" has been clicked.
        * Opens QFileDialog to select volume's path, which is copied into "path_field".
        ***********************************************************************************/
        void voldir_button_clicked();

        /**********************************************************************************
        * Called when "projfile_button" has been clicked.
        * Opens QFileDialog to select project's XML path, which is copied into "path_field".
        ***********************************************************************************/
        void projfile_button_clicked();

        /**********************************************************************************
        * Called when "preview_button" has been clicked.
        * Launches stitching of the selected slice. The result is displayed in Vaa3D.
        ***********************************************************************************/
        void preview_button_clicked();


        /**********************************************************************************
        * Called by <CImport> when the associated operation has been performed.
        * If an exception has occurred in the <CImport> thread,  it is propagated and man-
        * aged in the current thread (ex != 0). Otherwise, volume information are imported
        * in the GUI by the <StackedVolume> handle of <CImport>.
        ***********************************************************************************/
        void import_done(MyException *ex);

        /**********************************************************************************
        * Called by <CPreview> when the associated operation has been performed.
        * If an exception has occurred in the <CPreview> thread,  it is propagated and man-
        * aged in the current thread (ex != 0). Otherwise, the preview which was saved back
        * onto the disk is loaded and shown in Vaa3D.
        ***********************************************************************************/
        void preview_done(MyException *ex, Image4DSimple* img);

        /**********************************************************************************
        * Called when "path_field" value has changed.
        ***********************************************************************************/
        void volumePathChanged(QString path);

        /**********************************************************************************
        * Called when "reimport_chheckbox" state has changed.
        ***********************************************************************************/
        void reimportCheckboxChanged(int);

        /**********************************************************************************
        * Called when "channel_selection" state has changed.
        ***********************************************************************************/
        void channelSelectedChanged(int);

        /**********************************************************************************
        * Called when "regex_field" state has changed.
        ***********************************************************************************/
        void regexFieldChanged();

};

#endif // PTABIMPORT_H
