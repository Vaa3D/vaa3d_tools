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
** by Hanchuan Peng
060809
Last update: 080814: move iDrawExternalParameter to v3d_core.h
 081003: add setDatatitle and getDatatitle functions
 081123: separate the autotab.h. By Hanchuan Peng
 090626: by Hanchuan Peng. add a raise_and_activate() slot
****************************************************************************/

#ifndef V3DR_MAINWINDOW_H
#define V3DR_MAINWINDOW_H


#include "v3dr_common.h"

#include "qtr_widget.h"


class V3dR_GLWidget;

class V3dR_MainWindow : public QWidget
{
    Q_OBJECT

public:
    V3dR_MainWindow(iDrawExternalParameter* idep);
    ~V3dR_MainWindow();
	void setDataTitle(QString newdt);

	QString getTitlePrefix() {return title_prefix;}
	QString getDataTitle() {return data_title;}
	V3dR_GLWidget * getGLWidget() {return glWidget;}

protected:
    virtual void closeEvent(QCloseEvent* e);
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dropEvent ( QDropEvent * e);
    virtual void focusInEvent(QFocusEvent* e);
    virtual void focusOutEvent(QFocusEvent* e);
    virtual void enterEvent(QEvent *e);
    virtual void leaveEvent(QEvent *e);
    virtual void changeEvent(QEvent* e);
	virtual void keyPressEvent(QKeyEvent * e);
	virtual void keyReleaseEvent(QKeyEvent * e);

public slots:
	void postClose();

	void setXCutLockIcon(bool);
	void setYCutLockIcon(bool);
	void setZCutLockIcon(bool);

	void initControlValue(); // easy to edit, by RZC 080828, easy to reset 081122
	void initVolumeTimeRange();
	void initVolumeCutRange();  // called by initControlValue
	void initSurfaceCutRange(); // called by initControlValue

	void onlySurfaceObjTab() {if(tabOptions) tabOptions->setCurrentIndex(1); if(tabCutPlane) tabCutPlane->setCurrentIndex(1);}

	void doMenuOfSurfFile();
	void doMenuOfAnimate();

    // previewMovie() also return the rendering benchmark information
	QString previewMovie(QString& loop_scpript, int rotation_frames, int rotation_timepoints);
    void doSaveMovie(QString& loop_scpript, int rotation_frames, int rotation_timepoints);
    void saveMovie();

	void animateOn();
	void animateOff();
    void animateStep();
	QString getAnimateRotType(QString qtitle, bool* ok);
	void setAnimateRotType();
	int getAnimateRotTimePoints(QString qtitle, bool* ok, int v);
	void setAnimateRotTimePoints();
	void setAnimateRotSpeedSec();

	void raise_and_activate() {if (isMinimized()) showNormal(); raise(); activateWindow();} //by PHC. 090626 to bring a 3D viewer to front and activate it

    // @ADDED by Alessandro on 2015-05-07 : hide/display controls.
    void hideDisplayControls();

//	void about();
//    //for movie control
//    void setXRotStep(int t);
//    void setYRotStep(int t);
//    void setZRotStep(int t);
//    void setNSteps(int t);

public:
	iDrawExternalParameter null_idep; //090918: for editing swc
	iDrawExternalParameter* _idep;
	QString title_prefix;
	QString data_title;

    void saveFrameFunc(int i);
	QString outputDir;

	// animate() return total frames to render in one loop_scpript
	V3DLONG animate(QString& loop_scpript, int rotation_time_ms, int rotation_frames,  // FPS = rotation_frames*1000/rotation_time_ms
					int rotation_timepoints=0, bool bSaveFrame=false);
    int sAnimate;  // 0--stop, 1--run once, 2--run loop
    bool bAnimating;
    QTimer animate_timer;
    struct {
    	QStringList loop_list;
    	int frame_time_ms;
    	int rotation_frames;
    	int rotation_timepoints;
    	float frame_timepoints;
    	int iframe_rotation;
    	int irotation;
    	bool bSaveFrame;
    } animate_option;
    QString scriptAnimateRot;
    int FPS;
    int rotationSpeedSec;
    int rotationTimePoints;
    void default_animate_para()
    {
        scriptAnimateRot = "X Y Z";
		FPS = 15; //the best choice: 15 frame/second
        rotationSpeedSec = 12;
		rotationTimePoints = 0;
    }

    QMenu menuAnimate;
    QMenu menuSurfFile;
	void createMenuOfSurfFile();
	void createMenuOfAnimate();

	QAbstractSlider *createCutPlaneSlider(int maxval, Qt::Orientation hv = Qt::Horizontal);
	QAbstractSlider *createRotateSlider(Qt::Orientation hv = Qt::Horizontal);
	QAbstractSlider *createShiftSlider(Qt::Orientation hv = Qt::Horizontal);
	QAbstractSlider *createZoomSlider(Qt::Orientation hv = Qt::Horizontal);
	QAbstractSlider *createRangeSlider(int znear, int zfar, Qt::Orientation hv = Qt::Horizontal);
	QAbstractSlider *createTranparentSlider(Qt::Orientation hv = Qt::Horizontal);
	QAbstractSlider *createThicknessSlider(Qt::Orientation hv = Qt::Horizontal);
	QDoubleSpinBox *createThicknessSpinBox();
	QComboBox *createChannelComboBox();
	QSpinBox *createMarkerSizeSpinBox();
	QSpinBox *createRotateSpinBox();
	QSpinBox *createShiftSpinBox();
	QSpinBox *createZoomSpinBox();

	//void createActions();
    //void createMenus();
	//QSize getSize();

    //widget controls////////////////////////////////////////////////////////////////

    //QPointer<V3dR_GLWidget> glWidget; // guarded pointers to QObjects, by RZC 080924
    V3dR_GLWidget * glWidget; // normal pointers to QObjects
    static QWidget* lastActive; //for glWidget->updateTool()


    //QWidget *centralWidget;
    QScrollArea *glWidgetArea, *pixmapLabelArea;
    QScrollBar* timeSlider;

    // volume display control
	QRadioButton *dispType_maxip, *dispType_minip, *dispType_alpha, *dispType_cs3d;
	QLabel *thicknessSlider_Label, *transparentSlider_Label; //for disable, by RZC 080822
	QDoubleSpinBox *zthicknessBox; //by PHC, 090215
	QComboBox *comboBox_channel;
    QAbstractSlider *thicknessSlider, *transparentSlider;
    QCheckBox *checkBox_channelR, *checkBox_channelG, *checkBox_channelB, *checkBox_channelA, *checkBox_volCompress;
    QPushButton *volumeColormapButton;

    // surface display control
    QCheckBox *checkBox_displayMarkers, *checkBox_displaySurf, *checkBox_markerLabel, *checkBox_surfStretch;
    QSpinBox * spinBox_markerSize; // 090422 RZC
    QPushButton *updateLandmarkButton, *loadSaveObjectsButton;
    QPushButton *surfobjManagerButton; //or Object Manager button

    // other control
	QCheckBox *checkBox_displayAxes, *checkBox_displayBoundingBox, *checkBox_OrthoView;
	QPushButton *colorButton, *brightButton, *reloadDataButton;
    QPushButton *movieSaveButton, *animateButton;
    QComboBox * comboRotType;

    // @ADDED by Alessandro on 2015-05-07 : hide/display controls.
    QGroupBox *controlGroup;
    QPushButton *hideDisplayControlsButton;
    bool displayControlsHidden;
    QWidget *toolBtnGroup;

    QTabWidget *tabOptions;

    // rotation, zoom, shift control
    QAbstractSlider *xRotSlider, *yRotSlider, *zRotSlider;
	QAbstractSlider *zoomSlider, *xShiftSlider, *yShiftSlider;
    QSpinBox *xRotBox, *yRotBox, *zRotBox, *zoomBox, *xShiftBox, *yShiftBox;
    QCheckBox *checkBox_absoluteRot;
    QPushButton *zoomReset, *rotReset, *rotAbsolute, *rotAbsolutePose,*rotVRView;//wwbmark freeze

    QTabWidget *tabRotZoom;

    // cut plane control
    QAbstractSlider *xcminSlider, *xcmaxSlider, *ycminSlider, *ycmaxSlider, *zcminSlider, *zcmaxSlider, *fcutSlider;
    QAbstractButton *xcLock, *ycLock, *zcLock;
    QCheckBox *checkBox_xCS, *checkBox_yCS, *checkBox_zCS, *checkBox_fCS;
    QAbstractSlider *xCSSlider, *yCSSlider, *zCSSlider, *fCSSlider;
    QAbstractSlider *xSminSlider, *xSmaxSlider, *ySminSlider, *ySmaxSlider, *zSminSlider, *zSmaxSlider;
    QStackedLayout *stackedCutPlane;

    QTabWidget *tabCutPlane;

    // @ADDED by Alessandro on 2015-09-30: giving access to layout allows later changes (e.g. addition of new elements)
    QHBoxLayout *centralLayout;

	void init_members() // more clear putting them here, by RZC 080828
	{
		default_animate_para();
	    sAnimate = 0;
		bAnimating = false;
	    connect(&animate_timer, SIGNAL(timeout()), this, SLOT(animateStep())); //only connect once
	    animate_timer.stop();


		//centralWidget=0;
	    glWidgetArea=pixmapLabelArea=0;
	    glWidget=0;
	    timeSlider=0;

	    dispType_minip=dispType_maxip=dispType_alpha=dispType_cs3d=0;
		thicknessSlider_Label=transparentSlider_Label=0;
		transparentSlider=0;
	    thicknessSlider=0; zthicknessBox=0;
	    comboBox_channel=0;
		checkBox_channelR=checkBox_channelG=checkBox_channelB=checkBox_channelA=checkBox_volCompress=0;
	    volumeColormapButton=0;

	    checkBox_displayMarkers=checkBox_displaySurf=checkBox_markerLabel=checkBox_surfStretch=0;
	    spinBox_markerSize=0;
	    updateLandmarkButton=loadSaveObjectsButton=0;
	    surfobjManagerButton=0;

		checkBox_displayAxes=checkBox_displayBoundingBox=checkBox_OrthoView=0;
		colorButton=brightButton=reloadDataButton=0;
		movieSaveButton=animateButton=0;
		comboRotType=0;

	    tabOptions=0;

        // @ADDED by Alessandro on 2015-05-07 : hide/display controls.
        controlGroup = 0;
        hideDisplayControlsButton = 0;
        displayControlsHidden = 0;

		xRotSlider=yRotSlider=zRotSlider=0;
		zoomSlider=xShiftSlider=yShiftSlider=0;
	    xRotBox=yRotBox=zRotBox=zoomBox=xShiftBox=yShiftBox=0;
	    checkBox_absoluteRot=0;
		zoomReset=rotReset=rotAbsolute=rotAbsolutePose=rotVRView=0;

		tabRotZoom=0;

		xcminSlider=xcmaxSlider=ycminSlider=ycmaxSlider=zcminSlider=zcmaxSlider=fcutSlider=0;
		xcLock=ycLock=zcLock=0;
		checkBox_xCS=checkBox_yCS=checkBox_zCS=checkBox_fCS=0;
	    xCSSlider=yCSSlider=zCSSlider=fCSSlider=0;
	    xSminSlider=xSmaxSlider=ySminSlider=ySmaxSlider=zSminSlider=zSmaxSlider=0;
	    stackedCutPlane = 0;

	    tabCutPlane = 0;
	}
	void createControlWidgets();
	void connectSignal();
	//void disconnectSignal(); // no need to disconnect

//    QMenu *fileMenu;
//    QMenu *helpMenu;
//    QAction *grabFrameBufferAct;
//    QAction *renderIntoPixmapAct;
//    QAction *clearPixmapAct;
//    QAction *exitAct;
//    QAction *aboutAct;
//    QAction *aboutQtAct;
};


#endif
