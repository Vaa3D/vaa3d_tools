/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */
/************
                                            ********* LICENSE NOTICE ************
This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.
You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.
1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.
2. You agree to appropriately cite this work in your related studies and publications.
Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )
Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )
3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.
*************/
/****************************************************************************
**
** by Hanchuan Peng
** 2006, 2007, 2008
*
* Jan 7, 2008: add a copy&paste buffer control point list
* March 13, 2008: add an atlas view button & action
* March 18, 2008: add an import image function (for tif series)
* 2008-07-28: add mask_channel menu
 2008-08-24: add cellseg menus
 2008-08-27: try to add the drop function
 2010-06-01: add more plugin interface for global setting and swc curves
**
****************************************************************************/



///////////////////////////////////////////////////////////////////

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#define __v3d_custom_toolbar__

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
  #include <QtWidgets>
#else
  #include <QtGui>
#endif
// These two explicit includes make my IDE work better - CMB 08-Oct-2010
#include <QMainWindow>

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
#include <QMdiArea>
#else
#include <QWorkspace>
#endif
#include "v3d_actions.h"
#include "v3d_compile_constraints.h"
#include "../worm_straighten_c/bdb_minus.h"
#include "../neuron_editing/neuron_format_converter.h"
#include "v3d_global_preference_dialog.h"
//#include "../atlas_builder/pointcloud_atlas_io.h"
#include "v3d_core.h"
#include "../basic_c_fun/basic_thread.h" //YuY Dec-20-2010
#include "v3d_commandlineparser.h"
#include "pluginfunchandler.h"
#ifdef __V3DWSDEVELOP__
#include "../webservice/src/v3dwebservice.hpp" // YuY March-16-2011
class V3DWebService; //110315 YuY
class soappara; //110315 YuY
#endif
class V3d_PluginLoader;
class Image4DSimple;
class My4DImage;
class QAction;
class QMenu;
class MdiChild;
class QSignalMapper;
class XFormWidget;
class V3dR_MainWindow;
class Vaa3DPluginMenu;

//class QList <V3dR_MainWindow *>;
//struct LocationSimple; //080107
//class QList <LocationSimple>; //080107
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();
    XFormWidget ** retrieveAllMdiChild(int & nchild);
	QList <LocationSimple> buffer_landmark_pts; //080107
    // CMB 12-Nov-2010
    // Intercept QFileOpenEvent on Mac - drag-onto-app
    bool eventFilter(QObject *obj, QEvent *event);
#ifdef _ALLOW_WORKMODE_MENU_
    void setV3DDefaultModeCheck(bool checkState);
    void setNeuronAnnotatorModeCheck(bool checkState);
#endif
protected:
    void closeEvent(QCloseEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
public slots:
    void newFile();
    void open();
    void openWebUrl(); // By CMB 08-Oct-2010
#ifdef _ALLOW_TERAFLY_MENU_
	void func_open_terafly();
    void func_open_teraconverter();
#endif
#ifdef __v3d_custom_toolbar__
	void addCustomToolbar(); // By Hang 06-Aug-2011
#endif
    void finishedLoadingWebImage(QUrl url, QString fileName, bool b_cacheFile, bool b_forceopen3dviewer); // By CMB 08-Oct-2010
    void checkForUpdates(bool b_informOnNoUpdate = true); // CMB Oct-22-2010
    void generateVersionInfo(); //PHC 20130830
    void atlasView();
    void save();
    void saveAs();
    void cut();
    void copy();
    void paste();
    // void about();
	void openRecentFile();
    void updateMenus();
    void updatePluginMenu(); //PHC 20130904
#ifdef _ALLOW_WORKMODE_MENU_
    void updateModeMenu();
#endif
    void updateWindowMenu();
    void exit();
	void updateProcessingMenu();
//    MdiChild *createMdiChild();
    XFormWidget * createMdiChild();
//    MdiChild *activeMdiChild();
//    MdiChild *findMdiChild(const QString &fileName);
	XFormWidget *activeMdiChild();
	XFormWidget *findMdiChild(const QString &fileName);
	void loadV3DFile(QString filename, bool b_putinrecentfilelist=true, bool b_forceopen3dviewer=false);
       void loadV3DUrl(QUrl url, bool b_cacheLocalFile=true, bool b_forceopen3dviewer=false); // by CMB Oct-14-2010
	//the following are public slot processing functions
	void func_procLandmarkManager();
	void func_procAtlasViewer();
	void func_proc3DViewer();
	void func_procVRViewer();//wwbchange
	void func_proc3DLocalRoiViewer();
	void func_procSettings();
	void func_proc_plugin_manager();
	void func_procIO_export_to_vano_format();
	void func_procIO_export_to_movie();
    void func_procIO_export_landmark_to_pointcloud();
	void func_procIO_export_landmark_to_swc();
	void func_procIO_export_tracedneuron_to_swc();
    void import_GeneralImageFile();
    void import_Leica();
	void func_procIO_import_atlas_imgfolder();
//	void func_procIO_import_atlas_apofolder();
//	void func_procIO_import_atlas_apofolder(apoAtlasLinkerInfoAll & apoinfo);
//	void func_procPC_Atlas_edit_atlaslinkerfile();
//	void func_procPC_Atlas_create_atlaslinkerfile();
//	void func_procPC_Atlas_view_atlas();
	void func_procPC_Atlas_view_atlas_computeVanoObjStat();
	void func_procGeneral_rotate_paxis();
	void func_procGeneral_rotate_angle();
	void func_procGeneral_flip();
	void func_procGeneral_clear_all_landmark();
	void func_procGeneral_toggle_landmark_label();
	void func_procGeneral_clear_connectmap();
	void func_procGeneral_rescale_landmarks_only();
	void func_procGeneral_automarker_entireimg();
	void func_procGeneral_automarker_roi();
	void func_procGeneral_split_channels();
	void func_procGeneral_extract_a_channel();
	void func_procGeneral_crop_image_minMaxBox();
	void func_procGeneral_crop_bbox_roi();
	void func_procGeneral_mask_roi();
	void func_procGeneral_mask_nonroi_xy();
	void func_procGeneral_mask_channel();
	void func_procGeneral_clear_roi();
	void func_procGeneral_resample_image();
	void func_procGeneral_projection_max();
	void func_procGeneral_blend_image();
	void func_procGeneral_stitch_image();
	void func_procGeneral_display_histogram();
	void func_procGeneral_linear_adjustment();
	void func_procGeneral_histogram_equalization();
	void func_procGeneral_intensity_rescale();
	void func_procGeneral_intensity_threshold();
	void func_procGeneral_intensity_binarize();
	void func_procGeneral_intensity_updateminmax();
	void func_procGeneral_color_invert();
	void func_procGeneral_16bit_to_8bit();
	void func_procGeneral_32bit_to_8bit();
	void func_procGeneral_scaleandconvert28bit();
	void func_procGeneral_scaleandconvert28bit_1percent();
	void func_procGeneral_indexedimg2rgb();
	//void func_procGeneral_open_image_in_windows();
	void func_procGeneral_save_image();
	void func_procElongated_randomSeeding();
	void func_procElongated_minSpanTree();
	void func_procElongated_mstDiameter();
	void func_procElongated_genCuttingPlaneLocations();
	void func_procElongated_restackingCuttingPlanes();
	void func_procElongated_bdbminus();
	void func_procReg_gridSeeding();
	void func_procReg_randomSeeding();
	void func_procReg_bigGradient_edge_Seeding();
	void func_procReg_big_curvature_corner_Seeding();
	void func_procReg_fileSeeding();
	void func_procReg_global_align();
	void func_procReg_affine_markers_align();
	void func_procReg_flybrain_lobeseg();
	void func_procReg_detect_matching();
	void func_procReg_detect_matching_1pt();
	void func_procReg_warp_using_landmarks();
	void func_procReg_all_in_one_warp();
 void func_procCellSeg_localTemplate();
 void func_procCellSeg_cellcounting();
 void func_procCellSeg_watershed();
 void func_procCellSeg_levelset();
 void func_procCellSeg_Gaussian_fit_1_spot_1_Gauss();
 void func_procCellSeg_Gaussian_fit_1_spot_N_Gauss();
 void func_procCellSeg_Gaussian_partition();
 void func_procCellSeg_manualCorrect();
	void func_procTracing_topdownSkeleton();
	void func_procTracing_bottomupSearch();
	void func_procTracing_glocal_combine();
	void func_procTracing_manualCorrect();
	void func_procTracing_APP2auto();
	void func_procTracing_one2others();
	void func_procTracing_trace_a_curve();
	void func_procTracing_undo_laststep();
	void func_procTracing_redo_laststep();
	void func_procTracing_clear();
	void func_procTracing_update3Dview();
	void func_procTracing_save();
#ifdef _ALLOW_WORKMODE_MENU_
        // Mode
        void func_procModeDefault();
        void func_procModeNeuronAnnotator();
#endif


// Dec-20-2010 YuY
signals:
	void triviewUpdateTriggered();
	void imageLoaded2Plugin();
public slots:
	void transactionStart();
	void allTransactionsDone();
	void updateTriview();
	void updateTriviewWindow(); // trigger a signal triviewUpdateTriggered
	void updateRunPlugin();

    void handleCoordinatedCloseEvent_real(); //move down here, PHC 20151117
    void handleCoordinatedCloseEvent(QCloseEvent *event); //move down here, PHC 20151117


// April-26-2011 YuY
public:
	void setBooleanCLplugin(bool cl_plugininput);
	void setPluginName(char *pluginnameinput);
	void setPluginMethod(char *pluginmethodinput);
	void setPluginFunc(char *pluginfuncinput);
	char *getPluginName();
	char *getPluginMethod();
	char *getPluginFunc();
	void triggerRunPlugin();
    V3D_CL_INTERFACE v3dclp;
private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
	void setCurrentFile(const QString &fileName);
	void updateRecentFileActions();
	QString strippedName(const QString &fullFileName);
	void addTransaction(Transaction *transact); // Dec-20-2010 YuY
	TransactionThread sub_thread; // Dec-20-2010 YuY
	bool cl_plugin; // command line call a plugin
	char *pluginname;
	char *pluginmethod;
	char *pluginfunc;
	QString curFile;
	XFormWidget * curHiddenSelectedXWidget;
     // save data for 3d view quick display. ZJL 111020
     //void saveDataFor3DViewer(iDrawExternalParameter* _idep);
#ifdef __v3d_custom_toolbar__
public :
#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
	QMdiArea *workspace;
#else
	QWorkspace *workspace;
#endif
private:
#else
#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
	QMdiArea *workspace;
#else
	QWorkspace *workspace;
#endif
#endif
    QSignalMapper *windowMapper;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *windowMenu;
#ifdef _ALLOW_WORKMODE_MENU_
    QMenu *modeMenu;
#endif
    QMenu *helpMenu;
	QMenu *basicProcMenu, *advancedProcMenu, *pipelineProcMenu, *visualizeProcMenu;
	QMenu *proc_export_menu;
	QMenu *proc_import_menu;
	QMenu *proc_datatype_menu;
	QMenu *proc_geometry_transform_menu;
	QMenu *proc_intensity_transform_menu;
	QMenu *proc_colorchannel_transform_menu;
	QMenu *proc_landmark_control_menu;
	QMenu *proc_general_menu;
	QMenu *proc_segmentation_menu;
    QMenu *proc_tracing_menu;
	QMenu *proc_registration_menu;
	QMenu *proc_standarization_menu;
	QMenu *proc_pointcloud_atlas_menu;
	QMenu *proc_image_atlas_menu;
	QMenu *proc_automarker_menu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QAction *newAct;
    QAction *openAct;
    QAction *openWebUrlAct; // for web loading CMB
#ifdef __v3d_custom_toolbar__
	QAction *customToolbarAct;
#endif
    QAction *atlasViewAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *separator_RecentFiles_Act;
    QAction *exitAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *closeAct;
    QAction *closeAllAct;
    QAction *tileAct;
    QAction *cascadeAct;
    QAction *arrangeAct;
    QAction *nextAct;
    QAction *previousAct;
    QAction *separator_ImgWindows_Act;
    QAction *checkForUpdatesAct; // CMB Oct-22-2010
    QAction *generateVersionInfoAct;

    v3d::ShowV3dAboutDialogAction *aboutAct;
//    QAction *aboutQtAct;
	//all the image processing menu items are listed below
	QAction * procLandmarkManager;
	QAction * procAtlasViewer;
	QAction * proc3DViewer;
	QAction * procVRViewer;//2017-06-11 by Yimin Wang
	QAction * proc3DLocalRoiViewer;
	QAction * procSettings;
	QAction * proc_plugin_manager;
	QAction * procIO_export_to_vano_format;
	QAction * procIO_export_to_movie;
	QAction * procIO_export_landmark_to_pointcloud;
	QAction * procIO_export_landmark_to_swc;
	QAction * procIO_export_tracedneuron_to_swc;
    QAction * import_GeneralImageFileAct;
	QAction * import_LeicaAct;
	QAction * procIO_import_atlas_imgfolder;
	QAction * procIO_import_atlas_apofolder;
	QAction * procPC_Atlas_edit_atlaslinkerfile;
	QAction * procPC_Atlas_create_atlaslinkerfile;
	QAction * procPC_Atlas_view_atlas;
	QAction * procPC_Atlas_view_atlas_computeVanoObjStat;
	QAction * procGeneral_rotate_paxis;
	QAction * procGeneral_rotate_angle;
	QAction * procGeneral_flip;
	QAction * procGeneral_clear_all_landmark;
	QAction * procGeneral_toggle_landmark_label;
	QAction * procGeneral_clear_connectmap;
	QAction * procGeneral_rescale_landmarks_only;
	QAction * procGeneral_automarker_entireimg;
	QAction * procGeneral_automarker_roi;
	QAction * procGeneral_split_channels;
	QAction * procGeneral_extract_a_channel;
	QAction * procGeneral_crop_image_minMaxBox;
	QAction * procGeneral_crop_bbox_roi;
	QAction * procGeneral_mask_roi;
	QAction * procGeneral_mask_nonroi_xy;
	QAction * procGeneral_mask_channel;
	QAction * procGeneral_clear_roi;
	QAction * procGeneral_resample_image;
	QAction * procGeneral_projection_max;
	QAction * procGeneral_blend_image;
	QAction * procGeneral_stitch_image;
	QAction * procGeneral_display_histogram;
	QAction * procGeneral_linear_adjustment;
	QAction * procGeneral_histogram_equalization;
	QAction * procGeneral_intensity_rescale;
	QAction * procGeneral_intensity_threshold;
	QAction * procGeneral_intensity_binarize;
	QAction * procGeneral_intensity_updateminmax;
	QAction * procGeneral_color_invert;
	QAction * procGeneral_16bit_to_8bit;
	QAction * procGeneral_32bit_to_8bit;
	QAction * procGeneral_scaleandconvert28bit;
	QAction * procGeneral_scaleandconvert28bit_1percent;
	QAction * procGeneral_indexedimg2rgb;
	//QAction * procGeneral_open_image_in_windows;
	QAction * procGeneral_save_image;
	QAction * procElongated_randomSeeding;
	QAction * procElongated_minSpanTree;
	QAction * procElongated_mstDiameter;
	QAction * procElongated_genCuttingPlaneLocations;
	QAction * procElongated_restackingCuttingPlanes;
	QAction * procElongated_bdbminus;
	QAction * procReg_gridSeeding;
	QAction * procReg_randomSeeding;
	QAction * procReg_bigGradient_edge_Seeding;
	QAction * procReg_big_curvature_corner_Seeding;
	QAction * procReg_fileSeeding;
	QAction * procReg_global_align;
	QAction * procReg_affine_markers_align;
	QAction * procReg_flybrain_lobeseg;
	QAction * procReg_detect_matching;
	QAction * procReg_detect_matching_1pt;
	QAction * procReg_warp_using_landmarks;
	QAction * procReg_all_in_one_warp;
	QAction * procTracing_topdownSkeleton;
	QAction * procTracing_bottomupSearch;
	QAction * procTracing_glocal_combine;
	QAction * procTracing_manualCorrect;
	QAction * procTracing_APP2auto;
	QAction * procTracing_one2others;
	QAction * procTracing_trace_a_curve;
	QAction * procTracing_undo_laststep;
	QAction * procTracing_redo_laststep;
	QAction * procTracing_clear;
	QAction * procTracing_update3Dview;
	QAction * procTracing_save;
	QAction * procCellSeg_localTemplate;
	QAction * procCellSeg_cellcounting;
	QAction * procCellSeg_watershed;
	QAction * procCellSeg_levelset;
    QAction * procCellSeg_Gaussian_fit_1_spot_1_Gauss;
    QAction * procCellSeg_Gaussian_fit_1_spot_N_Gauss;
	QAction * procCellSeg_Gaussian_partition;
        QAction * procCellSeg_manualCorrect;
#ifdef _ALLOW_WORKMODE_MENU_
	// Mode
	QAction * procModeDefault;
	QAction * procModeNeuronAnnotator;
#endif
#if defined(__V3DWSDEVELOP__)
private:
	soappara *pSoapPara;
	V3DWebService *v3dws;
public slots:
	void webserviceResponse();
	void initWebService(V3DWebService *pws);
	void quitWebService(V3DWebService *pws);
	void setSoapPara(soappara *pSoapParaInput);
	void do3dfunc();
	void switch3dviewercontrol(V3dR_MainWindow *existing_3dviewer);
#endif
public: //for image processing, some of the parameters should be globally set
	//080822
	BDB_Minus_ConfigParameter flybrain_lobeseg_para;
	void setup_global_imgproc_parameter_default();
	//080901
//	DFComputeMethodType GPara_df_compute_method; //the DF computing method
//	PointMatchMethodType GPara_landmarkMatchingMethod; //the landmark matching method.   should be PointMatchMethodType but I will force to convert to that type in the code
	V3D_GlobalSetting global_setting;
	enum { MaxRecentFiles = 20 };
	QAction *recentFileActs[MaxRecentFiles];
	QList <V3dR_MainWindow *> list_3Dview_win; //need to free later in the destructor //081002
	V3dR_MainWindow * find3DViewer(QString fileName);
//    QMdiArea *get_workspace_handle() {return workspace;}
	void updateWorkspace()
	{
		if (workspace)
		{
			workspace->update();

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
			foreach (QWidget* w, workspace->subWindowList()) w->update();
#else
			foreach (QWidget* w, workspace->windowList()) w->update();
#endif
		}
	}
	void cascadeWindows()
	{

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
		if (workspace) workspace->cascadeSubWindows();
#else
		if (workspace) workspace->cascade();
#endif
	}
#define __used_by_v3d_interface__
    //Vaa3DPluginMenu *pluginProcMenu;
    QMenu *pluginProcMenu; // for dynamically hooking plug-in menu
	V3d_PluginLoader *pluginLoader;
public:	//2009-2010: used by V3D_PluginLoader 	// in mainwindow_interface.cpp
	XFormWidget* currentImageWindow();
	My4DImage* currentImage();
	XFormWidget* curHiddenSelectedWindow() {return curHiddenSelectedXWidget;}
	bool setCurHiddenSelectedWindow( XFormWidget* a);
	bool setCurHiddenSelectedWindow_withoutcheckwinlist( XFormWidget* a); // added by YuY, Dec 16, 2010.
	QList<void*> allWindowList();
	XFormWidget* validateImageWindow(void* window);
	QString getWindowName(void* window);
	QStringList allWindowNameList();
	XFormWidget* newImageWindow(const QString &name);
	XFormWidget* updateImageWindow(void* window);
	XFormWidget* updateImageWindow(void* window, bool b_forceUpdateChannelMinMaxValues); //20120412
	XFormWidget* setImageName(void* window, const QString &name);
	My4DImage* getImage(void* window);
	bool setImage(void* window, Image4DSimple *image);
	QList<LocationSimple> getLandmark(void* window);
	bool setLandmark(void* window, QList<LocationSimple>& landmark_list);
	QList<QPolygon> getROI(void* window);
	bool setROI(void* window, QList<QPolygon>& roi_list);
	NeuronTree getSWC(void* window);
	bool setSWC(void* window, NeuronTree & nt);
	V3D_GlobalSetting getGlobalSetting();
	bool setGlobalSetting( V3D_GlobalSetting &gs );
};
#endif
