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
#include "QGLRefSys.h"

class teramanager::PMain : public QWidget
{
    Q_OBJECT

    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PMain* uniqueInstance;
        PMain(){    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);   }
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
        QMenu* importOptionsMenu;       //"Import" menu level 2
        QMenu* threeDMenu;              //"3D" menu level 2
        QMenu* curvesMenu;              //"Curves" menu level 3
        QMenu* curveDimsMenu;           //"Curve dims" menu level 4
        QWidgetAction* curveDimsWidget; //"Curve dims" menu action widget
        QSpinBox* curveDimsSpinBox;     //"Curve dims" spinbox
        QMenu* curveAspectMenu;         //"Curve aspect" menu level 4
        QAction* curveAspectTube;       //"Tube" action
        QAction* curveAspectSkeleton;   //"Skeleton" action

        QMenu* DirectionalShiftsMenu;   //"Directional shifts" menu level 2
        QMenu* xShiftMenu;              // x-shift entry
        QWidgetAction* xShiftWidget;    // x-shift action
        QSpinBox *xShiftSBox;           // x-shift widget (a spinbox)
        QMenu* yShiftMenu;              // y-shift entry
        QWidgetAction* yShiftWidget;    // y-shift action
        QSpinBox *yShiftSBox;           // y-shift widget (a spinbox)
        QMenu* zShiftMenu;              // z-shift entry
        QWidgetAction* zShiftWidget;    // z-shift action
        QSpinBox *zShiftSBox;           // z-shift widget (a spinbox)
        QMenu* tShiftMenu;              // t-shift entry
        QWidgetAction* tShiftWidget;    // t-shift action
        QSpinBox *tShiftSBox;           // t-shift widget (a spinbox)

        QMenu* helpMenu;                //"Help" menu
        QAction* aboutAction;           //"About" menu action
        QMenu *recentVolumesMenu;
        QAction* clearRecentVolumesAction;

        //debug menu widgets
        QMenu* debugMenu;               //"Debug" menu for debugging purposes
        QAction* debugAction1;          //debug menu action #1
        QAction* debugShowLogAction;    //debug menu action "Show log"
        QMenu* debugStreamingStepsMenu;                    // streaming steps entry
        QWidgetAction* debugStreamingStepsActionWidget;    // streaming steps action
        QSpinBox *debugStreamingStepsSBox;                 // streaming steps widget (a spinbox)
        QMenu* debugVerbosityMenu;                         // verbosity entry
        QWidgetAction* debugVerbosityActionWidget;         // verbosity action
        QComboBox *debugVerbosityCBox;                     // verbosity widget (a combobox)
        QMenu* debugRedirectSTDoutMenu;                    // redirect stdout entry
        QWidgetAction* debugRedirectSTDoutActionWidget;    // redirect stdout action
        QLineEdit *debugRedirectSTDoutPath;                // redirect stdout widget (a line edit)
        QAction* addGaussianNoiseToTimeSeries;             // add gaussian noise to time series action

        //toolbar widgets
        QToolBar* toolBar;                                  //tool bar with buttons
        QToolButton *openVolumeToolButton;                  //tool button for volume opening

        //import form widgets
        QAction *regenMData_cAction;                        // if active, metadata will be regenerated
        QAction *regenVMap_cAction;                         // if active, volume map is regenerated
        QMenu* volMapSizeMenu;                              // volume map size entry
        QWidgetAction* volMapSizeWidget;                    // volume map size action
        QSpinBox *volMapSizeSBox;                           // volume map size widget (a spinbox)

        QTabWidget *tabs;               //tab widget
        //Page "Volume's info": contains informations of the loaded volume
        QWidget* info_page;
        QLabel* vol_size_files_field;
        QLabel* vol_size_files_label;
        QLabel* vol_size_voxel_field;
        QLabel* vol_size_voxel_label;
        QLabel* vol_size_bytes_field;
        QLabel* vol_size_bytes_label;
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

        //Page "Controls": contains navigation controls
        QWidget* controls_page;
        /* ------- local viewer panel widgets ------- */
        QGroupBox* localViewer_panel;
        QGradientBar* gradientBar;
        QSpinBox* Vdim_sbox;
        QSpinBox* Hdim_sbox;
        QSpinBox* Ddim_sbox;
        QSpinBox* Tdim_sbox;
        QComboBox* resolution_cbox;
        /* ------- zoom options panel widgets ------- */
        QGroupBox* zoom_panel;
        QSlider* cacheSens;
        QSlider* zoomInSens;
        QSlider* zoomOutSens;
        QPushButton* controlsResetButton;
        QComboBox* zoomInMethod;
        /* ------- global coord panel widgets ------- */
        QGroupBox* globalCoord_panel;
        QArrowButton* traslXpos;
        QLabel* traslXlabel;
        QArrowButton* traslXneg;
        QArrowButton* traslYpos;
        QLabel* traslYlabel;
        QArrowButton* traslYneg;
        QArrowButton* traslZpos;
        QLabel* traslZlabel;
        QArrowButton* traslZneg;
        QArrowButton* traslTpos;
        QLabel* traslTlabel;
        QArrowButton* traslTneg;
        QSpinBox* V0_sbox;
        QSpinBox* V1_sbox;
        QSpinBox* H0_sbox;
        QSpinBox* H1_sbox;
        QSpinBox* D0_sbox;
        QSpinBox* D1_sbox;
        QLineEdit* T0_sbox;
        QLineEdit* T1_sbox;
        QLabel* to_label_1;
        QLabel* to_label_2;
        QLabel* to_label_3;
        QLabel* to_label_4;
        QGLRefSys* refSys;              //interactive 3D-based reference system
        QLineEdit* frameCoord;
        /* ------- exhaustive scan (ES) panel widgets ------- */
        QGroupBox* ESPanel;
        QPushButton* ESbutton;
        QSpinBox* ESblockSpbox;
        QSpinBox* ESoverlapSpbox;
        QComboBox* ESmethodCbox;
        std::vector<itm::block_t> ESblocks; //list of blocks for exhaustive scan

        //other widgets
        QHelpBox* helpBox;              //help box
        QProgressBar* progressBar;      //progress bar
        QStatusBar* statusBar;          //status bar

        //layout
        int marginLeft;                 //width of first column containing labels only

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

        //enables / disables directional shift controls
        void setEnabledDirectionalShifts(bool enabled);

        //generate blocks for exhaustive scan
        void generateESblocks() throw (itm::RuntimeException);
        bool isESactive(){return ESbutton->text().compare("Stop") == 0;}

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
        static string HTrefsys;
        static string HTresolution;


    public slots:

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
        * Called by <CImport> when the associated operation has been performed.
        * If an exception has occurred in the <CImport> thread,  it is propagated and man-
        * aged in the current thread (ex != 0). Otherwise, volume information are imported
        * in the GUI by the <StackedVolume> handle of <CImport>.
        ***********************************************************************************/
        void importDone(itm::RuntimeException *ex, qint64 elapsed_time = 0);

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
        * Called when the corresponding buttons are clicked
        ***********************************************************************************/
        void traslXposClicked();
        void traslXnegClicked();
        void traslYposClicked();
        void traslYnegClicked();
        void traslZposClicked();
        void traslZnegClicked();
        void traslTposClicked();
        void traslTnegClicked();

        /**********************************************************************************
        * Called when the corresponding debug actions are triggered
        ***********************************************************************************/
        void debugAction1Triggered();
        void addGaussianNoiseTriggered();
        void showLogTriggered();

        /**********************************************************************************
        * Called when the corresponding Options->3D->Curve actions are triggered
        ***********************************************************************************/
        void curveDimsChanged(int dim);
        void curveAspectChanged();

        /**********************************************************************************
        * Linked to verbosity combobox
        ***********************************************************************************/
        void verbosityChanged(int i);

        //very useful (not included in Qt): disables the given item of the given combobox
        static void setEnabledComboBoxItem(QComboBox* cbox, int _index, bool enabled);

        /**********************************************************************************
        * Called when the corresponding buttons are clicked
        ***********************************************************************************/
        void ESbuttonClicked();

        /**********************************************************************************
        * Called when the corresponding spin box has changed
        ***********************************************************************************/
        void ESblockSpboxChanged(int b);

        /**********************************************************************************
        * Called when the corresponding QLineEdit has been edited
        ***********************************************************************************/
        void debugRedirectSTDoutPathEdited(QString s);


};

#endif // PMAIN_GUI_H
