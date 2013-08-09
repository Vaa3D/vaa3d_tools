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
#include "../control/CPlugin.h"
#include "../core/ImageManager/StackedVolume.h"
#include "../control/CVolume.h"
#include "../control/CExplorerWindow.h"
#include "PDialogImport.h"
#include "v3dr_glwidget.h"
#include "QArrowButton.h"
#include "QHelpBox.h"
#include "QGradientBar.h"
#include "QLineTree.h"

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
        V3DPluginCallback2* V3D_env;    //handle of V3D environment
        QWidget *parentWidget;          //handle of parent widget
        string annotationsPathLRU;      //last recently used (LRU) annotations filepath

        //menu widgets
        QMenuBar* menuBar;              //Menu bar
        QMenu* fileMenu;                //"File" menu
        QAction* openVolumeAction;      //"Open volume" menu action
        QAction* closeVolumeAction;     //"Close volume" menu action
        QAction* loadAnnotationsAction; //"Load annotations" menu action
        QAction* saveAnnotationsAction; //"Save annotations" menu action
        QAction* saveAnnotationsAsAction; //"Save annotations as" menu action
        QAction* clearAnnotationsAction;//"Clear annotations" menu action
        QAction* exitAction;            //"Exit" menu action
        QMenu* optionsMenu;             //"Options" menu
        QMenu* importOptionsMenu;       //"Import" menu
        QWidgetAction* importOptionsWidget;
        QMenu* helpMenu;                //"Help" menu
        QAction* aboutAction;           //"About" menu action
        QMenu* debugMenu;               //"Debug" menu for debugging purposes
        QAction* debugAction1;          //debug menu action #1
        QMenu *recentVolumesMenu;
        QAction* clearRecentVolumesAction;

        //toolbar widgets
        QToolBar* toolBar;              //tool bar with buttons
        QToolButton *openVolumeToolButton; //tool button for volume opening

        //import form widgets
        QWidget* import_form;         //import form containing input fields
        QCheckBox *reimport_checkbox;   //checkbox to be used to reimport a volume already imported
        QCheckBox *enableMultiresMode;  //checkbox to be used to generate and show a 3D volume map
        QWidget* volMapWidget;          //widget containing volume map options
        QCheckBox *regenerateVolMap;    //if activated, the volume map will be regenerated
        QSpinBox  *volMapMaxSizeSBox;   //to set the maximum allowed size (in MVoxels) of the volume map

        //info panel widgets, contain informations of the loaded volume
        QGroupBox* info_panel;
        QLabel* vol_size_field;
        QLabel* vol_size_label;
        QLabel* vol_height_mm_field;
        QLabel* vol_width_mm_field;
        QLabel* vol_depth_mm_field;
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
        QPushButton* loadButton;

        //multiresolution mode widgets
        QGroupBox* multires_panel;
        QGradientBar* gradientBar;
        QSpinBox* Vdim_sbox;
        QSpinBox* Hdim_sbox;
        QSpinBox* Ddim_sbox;
        QLabel* by_label_6;
        QLabel* by_label_7;
        QComboBox* resolution_cbox;
        QLabel* zoominVoiSize;
        QSlider* cacheSens;
        QSlider* zoomInSens;
        QSlider* zoomOutSens;
        QArrowButton* traslXpos;
        QLabel* traslXlabel;
        QArrowButton* traslXneg;
        QArrowButton* traslYpos;
        QLabel* traslYlabel;
        QArrowButton* traslYneg;
        QArrowButton* traslZpos;
        QLabel* traslZlabel;
        QArrowButton* traslZneg;
        QLineTree* controlsLineTree;
        QPushButton* controlsResetButton;
        QComboBox* zoomInMethod;

        //other widgets
        QHelpBox* helpBox;              //help box
        QProgressBar* progressBar;      //progress bar
        QStatusBar* statusBar;          //status bar

        //layout
        int marginLeft;                 //width of first column containing labels only

        //very useful (not included in Qt): disables the given item of the given combobox
        void setEnabledComboBoxItem(QComboBox* cbox, int _index, bool enabled);

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PMain* instance(V3DPluginCallback2 *callback, QWidget *parent);
        static PMain* getInstance();
        static void uninstance();
        static bool isInstantiated(){return uniqueInstance != 0;}
        ~PMain();

        //GET and SET methods
        V3DPluginCallback2* getV3D_env(){return V3D_env;}
        QProgressBar* getProgressBar(){return progressBar;}
        QStatusBar* getStatusBar(){return statusBar;}

        //resets progress bar, start/stop buttons and tab bar
        void resetGUI();

        //reset everything
        void reset();

        //overrides closeEvent method of QWidget
        void closeEvent(QCloseEvent *evt);

        /**********************************************************************************
        * Filters events generated by the widgets to which a help message must be associated
        ***********************************************************************************/
        bool eventFilter(QObject *object, QEvent *event);

        /**********************************************************************************
        * Displays tooltip when ToolTip, MouseMove or KeyPress events occur on the widget.
        ***********************************************************************************/
        static void displayToolTip(QWidget* widget, QEvent* event, string msg);

        //<CExplorerWindow> instances need to access to all members of the current class
        friend class CExplorerWindow;

        friend class myV3dR_GLWidget;

        //help texts
        static string HTwelcome;
        static string HTbase;
        static string HTvoiDim;
        static string HTjumpToRes;
        static string HTcacheSens;
        static string HTzoomInThres;
        static string HTzoomOutThres;
        static string HTtraslatePos;
        static string HTtraslateNeg;
        static string HTvolcuts;
        static string HTzoomInMethod;


    public slots:

        /**********************************************************************************
        * Called when "enable3Dmode" state changed.
        * Enables or disables the correspondent panel
        ***********************************************************************************/
        void mode3D_checkbox_changed(int);

        /**********************************************************************************
        * Called when "Open volume" menu action is triggered.
        * If path is not provided, opens a QFileDialog to select volume's path.
        ***********************************************************************************/
        void openVolume(string path = "");

        /**********************************************************************************
        * Called when a path in the "Recent volumes" menu is selected.
        ***********************************************************************************/
        void openVolumeActionTriggered();

        /**********************************************************************************
        * Called when "Clear menu" action in "Recent volumes" menu is triggered.
        ***********************************************************************************/
        void clearRecentVolumesTriggered();

        /**********************************************************************************
        * Called when "Close volume" menu action is triggered.
        * All the memory allocated is released and GUI is reset".
        ***********************************************************************************/
        void closeVolume();

        /**********************************************************************************
        * Called when "Open annotations" menu action is triggered.
        * Opens QFileDialog to select annotation file path.
        ***********************************************************************************/
        void loadAnnotations();

        /**********************************************************************************
        * Called when "Save annotations" or "Save annotations as" menu actions are triggered.
        * If required, opens QFileDialog to select annotation file path.
        ***********************************************************************************/
        void saveAnnotations();
        void saveAnnotationsAs();

        /**********************************************************************************
        * Called when "Clear annotations" menu action is triggered.
        ***********************************************************************************/
        void clearAnnotations();

        /**********************************************************************************
        * Called when "Exit" menu action is triggered or TeraFly window is closed.
        ***********************************************************************************/
        void exit();

        /**********************************************************************************
        * Called when "Help->About" menu action is triggered
        ***********************************************************************************/
        void about();

        /**********************************************************************************
        * Called when controlsResetButton is clicked
        ***********************************************************************************/
        void resetMultiresControls();

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
        void importDone(MyException *ex, Image4DSimple* vmap_image=0);

        /**********************************************************************************
        * Called by <CVolume> when the associated operation has been performed.
        * If an exception has occurred in the <CVolume> thread, it is propagated and
        * managed in the current thread (ex != 0).
        ***********************************************************************************/
        void loadingDone(MyException *ex, void* sourceObject);

        /**********************************************************************************
        * Called when the GUI widgets that control application settings change.
        * This is used to manage persistent platform-independent application settings.
        ***********************************************************************************/
        void settingsChanged(int);

        /**********************************************************************************
        * Linked to resolution combobox
        * This switches to the given resolution index.
        ***********************************************************************************/
        void resolutionIndexChanged(int i);

        /**********************************************************************************
        * Linked to zoom-in VOI spinboxes.
        * This updates the zoom-in VOI size widget.
        ***********************************************************************************/
        void zoomInVoiSizeChanged(int i);

        /**********************************************************************************
        * Linked to highest res VOI's selection spinboxes.
        * This updates the load button text.
        ***********************************************************************************/
        void highestVOISizeChanged(int i);

        /**********************************************************************************
        * Called when the correspont buttons are clicked
        ***********************************************************************************/
        void traslXposClicked();
        void traslXnegClicked();
        void traslYposClicked();
        void traslYnegClicked();
        void traslZposClicked();
        void traslZnegClicked();

        /**********************************************************************************
        * Called when the correspondent slider is changed
        ***********************************************************************************/
        void zoomInSensChanged(int i);

        /**********************************************************************************
        * Called when the correspondent debug actions are triggered
        ***********************************************************************************/
        void debugAction1Triggered();
};

#endif // PMAIN_GUI_H
