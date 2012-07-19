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

#ifndef PMAIN_GUI_H
#define PMAIN_GUI_H

#include <QtGui>
#include <v3d_interface.h>
#include "control/CPlugin.h"
#include "PDialogImport.h"
#include "StackedVolume.h"
#include "control/CLoadSubvolume.h"
#include "v3dr_glwidget.h"

class teramanager::PMain : public QWidget
{
    Q_OBJECT

    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PMain* uniqueInstance;
        PMain(){}
        PMain(V3DPluginCallback2 *callback, QWidget *parent);

        //members
        V3DPluginCallback2* V3D_env;     //handle of V3D environment
        QWidget *parentWidget;           //handle of parent widget
        V3dR_GLWidget* view3DWidget;     //handle of 3D renderer widget
        XFormWidget* treeviewWidget;     //handle of tree-view widget

        //helpbox
        QLabel* helpbox;

        //import form widgets
        QGroupBox* import_form;         //import form containing input fields
        QLineEdit *path_field;          //field for either volume's dir or project XML path
        QPushButton *voldir_button;     //browse for volume's directory button
        QCheckBox *reimport_checkbox;   //checkbox to be used to reimport a volume already imported
        QCheckBox *enable3Dmode;       //checkbox to be used to generate and show a 3D volume map

        //info panel widgets, contain informations of the loaded volume
        QGroupBox* info_panel;
        QLabel* volume_dims_label;
        QLabel* direction_V_label_0;
        QLabel* direction_H_label_0;
        QLabel* direction_D_label_0;
        QLabel* by_label_01;
        QLabel* by_label_02;
        QLabel* vol_height_field;
        QLabel* vol_width_field;
        QLabel* vol_depth_field;
        QLabel* volume_stacks_label;
        QLabel* direction_V_label_1;
        QLabel* direction_H_label_1;
        QLabel* by_label_1;
        QLabel* nrows_field;
        QLabel* ncols_field;
        QLabel* stacks_dims_label;
        QLabel* direction_V_label_2;
        QLabel* direction_H_label_2;
        QLabel* direction_D_label_2;
        QLabel* by_label_2;
        QLabel* by_label_3;
        QLabel* stack_height_field;
        QLabel* stack_width_field;
        QLabel* stack_depth_field;
        QLabel* voxel_dims_label;
        QLabel* direction_V_label_3;
        QLabel* direction_H_label_3;
        QLabel* direction_D_label_3;
        QLabel* by_label_4;
        QLabel* by_label_5;
        QLabel* vxl_V_field;
        QLabel* vxl_H_field;
        QLabel* vxl_D_field;
        QLabel* origin_label;
        QLabel* direction_V_label_4;
        QLabel* direction_H_label_4;
        QLabel* direction_D_label_4;
        QLabel* org_V_field;
        QLabel* org_H_field;
        QLabel* org_D_field;

        //subvol panel widgets
        QGroupBox* subvol_panel;
        QSpinBox* V0_sbox;
        QSpinBox* V1_sbox;
        QSpinBox* H0_sbox;
        QSpinBox* H1_sbox;
        QSpinBox* D0_sbox;
        QSpinBox* D1_sbox;
        QLabel* to_label_1;
        QLabel* to_label_2;
        QLabel* to_label_3;
        QLabel* direction_V_label_5;
        QLabel* direction_H_label_5;
        QLabel* direction_D_label_5;
        QPushButton* loadButton;

        //3d mode widgets
        QGroupBox* mode3D_panel;
        QLabel* subvol_dims_label;
        QSpinBox* Vdim_sbox;
        QSpinBox* Hdim_sbox;
        QSpinBox* Ddim_sbox;
        QLabel* direction_V_label_6;
        QLabel* direction_H_label_6;
        QLabel* direction_D_label_6;
        QLabel* by_label_6;
        QLabel* by_label_7;

        //other widgets
        QProgressBar* progressBar;      //progress bar
        QStatusBar* statusBar;          //status bar

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PMain* instance(V3DPluginCallback2 *callback=0, QWidget *parent=0);
        static void uninstance();
        ~PMain();

        //GET and SET methods
        V3DPluginCallback2* getV3D_env(){return V3D_env;}
        QProgressBar* getProgressBar(){return progressBar;}
        QStatusBar* getStatusBar(){return statusBar;}

        //resets progress bar, start/stop buttons and tab bar
        void resetGUI();

        //overrides closeEvent method of QWidget
        void closeEvent(QCloseEvent *evt);

        /**********************************************************************************
        * Filters events generated by the 3D rendering window
        * We're interested to intercept the mouse wheel event in order to enable a Google-
        * Earth like feature.
        ***********************************************************************************/
        bool eventFilter(QObject *object, QEvent *event);

    public slots:

        /**********************************************************************************
        * Called when "enable3Dmode" state changed.
        * Enables or disables the correspondent panel
        ***********************************************************************************/
        void mode3D_checkbox_changed(int);

        /**********************************************************************************
        * Called when "voldir_button" has been clicked.
        * Opens QFileDialog to select volume's path, which is copied into "path_field".
        ***********************************************************************************/
        void voldir_button_clicked();

        /**********************************************************************************
        * Called when "import_button" has been clicked.
        * Opens <PDialogImport> if additional informations are needed. Then the import op-
        * eration is performed in a separate thread by <CImport>
        ***********************************************************************************/
        void import_button_clicked();

        /**********************************************************************************
        * Called when "loadButton" has been clicked.
        * The selected subvolume is loaded and shown into Vaa3D.
        ***********************************************************************************/
        void loadButtonClicked();

        /**********************************************************************************
        * Called by <CImport> when the associated operation has been performed.
        * If an exception has occurred in the <CImport> thread,  it is propagated and man-
        * aged in the current thread (ex != 0). Otherwise, volume information are imported
        * in the GUI by the <StackedVolume> handle of <CImport>.
        ***********************************************************************************/
        void import_done(MyException *ex, Image4DSimple* vmap_image=0);

        /**********************************************************************************
        * Called by <CLoadSubvolume> when the associated operation has been performed.
        * If an exception has occurred in the <CLoadSubvolume> thread, it is propagated and
        * managed in the current thread (ex != 0).
        ***********************************************************************************/
        void loading_done(MyException *ex);
};

#endif // PMAIN_GUI_H
