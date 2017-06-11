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
** 2008-08-09
** last update : Hanchuan Peng, 090220
** Last update: Hanchuan Peng, 090221, Do NOT add surface obj geometry dialog
****************************************************************************/

#ifndef V3DR_GLWIDGET_H
#define V3DR_GLWIDGET_H

#include "v3dr_common.h"
#include "renderer.h"
#include "../basic_c_fun/basic_view3d.h"
#include <QGLWidget>

class Renderer;
class V3dR_MainWindow;
class V3dr_colormapDialog;
class V3dr_surfaceDialog;
//class SurfaceObjGeometryDialog;
typedef void(*PluginMouseFunc)(QGLWidget*); // May 29, 2012 by Hang

class V3dR_GLWidget : public QGLWidget, public View3DControl
{
    Q_OBJECT;
//	friend class V3dR_MainWindow; //090710 RZC: to delete renderer before ~V3dR_GLWidget()
//	friend class v3dr_surfaceDialog;
//	friend class v3dr_colormapDialog;

public:
    V3dR_GLWidget(iDrawExternalParameter* idep, QWidget* mainWindow=0, QString title="");
    ~V3dR_GLWidget();
    //void makeCurrent() {if (!_in_destructor) QGLWidget::makeCurrent();} //090605 RZC: to override invalid access in qgl_x11.cpp
    virtual void deleteRenderer();  //090710 RZC: to delete renderer before ~V3dR_GLWidget()
    virtual void createRenderer();  //090710 RZC: to create renderer at any time

	void handleKeyPressEvent(QKeyEvent * event); //for hook to MainWindow
	void handleKeyReleaseEvent(QKeyEvent * event); //for hook to MainWindow
	QString Cut_altTip(int dim_i, int v, int minv, int maxv, int offset); //tool tip function for real dimension of image

    iDrawExternalParameter* getiDrawExternalParameter() {return _idep;}
    QWidget* getMainWindow() {return mainwindow;}
	Renderer* getRenderer()   {return renderer;}
        const Renderer* getRenderer() const {return renderer;} // const version CMB
	QString getDataTitle()    {return data_title;}
	void setDataTitle(QString newdt) {data_title = newdt;}
	int getNumKeyHolding()    {for(int i=1;i<=9; i++) if(_holding_num[i]) return i; return -1;}

	bool getStill() 		{return _still;} //used by Renderer::beStill()
	void setStill(bool b) 	{_still = b;}    //used by V3dR_MainWindow::doSaveMovie()
    bool needStillPaint();
	void clearColormapDialog() {colormapDlg = 0;}
    void clearSurfaceDialog()  {surfaceDlg = 0;}
    bool screenShot(QString filename);
    void triggerNeuronShown(QList<int> overlayList)    {emit neuronShown(overlayList);}
    void triggerNeuronShownAll(QList<int> overlayList) {emit neuronShownAll(overlayList);}
    void triggerNeuronClearAll() {emit neuronClearAll();}
    void triggerNeuronIndexChanged(int index) {emit neuronIndexChanged(index);}
    void triggerNeuronClearAllSelections() {emit neuronClearAllSelections();}
    void setNeuronIndex(int index) {neuronIndex = index;}
    int getNeuronIndex() {return neuronIndex;}
    virtual void preparingRenderer();
    
//protected:
	virtual void choiceRenderer();
	virtual void settingRenderer(); // for setting the default renderer state when initialize
     virtual void initializeGL();
	virtual void resizeGL(int width, int height);
    virtual void paintGL();

    virtual void paintEvent(QPaintEvent *event);

    virtual void focusInEvent(QFocusEvent* e);
    virtual void focusOutEvent(QFocusEvent* e);
    virtual void enterEvent(QEvent *e);
    virtual void leaveEvent(QEvent *e);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
	virtual void mouseDoubleClickEvent ( QMouseEvent * event ) {};

        virtual void keyPressEvent(QKeyEvent * e) {handleKeyPressEvent(e);}
        virtual void keyReleaseEvent(QKeyEvent * e) {handleKeyReleaseEvent(e);}

    virtual void closeEvent(QCloseEvent* e); //for self closing
    virtual bool event(QEvent* e);       //090427 RZC:  for QHelpEvent of ToolTip
    virtual void customEvent(QEvent* e); // for QEvent_OpenFiles, by RZC 081002

    iDrawExternalParameter* _idep;
    QWidget *mainwindow;
	Renderer* renderer;
    QString data_title;
	QString dropUrl;
    static V3dr_colormapDialog* colormapDlg;
    static V3dr_surfaceDialog*  surfaceDlg;
	//static SurfaceObjGeometryDialog *surfaceObjGeoDlg;
    int neuronIndex;

    // @ADDED by Alessandro on 2015-09-30.
    // Used to enable / disable the progress bar when the 3D viewer image data are loading.
    // No progress bar --> faster update (maybe due to processEvents() used to handle progress bar events).
    // Be sure to set to 'true' by default when you subclass (or, simplier, just call the base-constructor).
    bool show_progress_bar;

	int currentPluginState;                              // May 29, 2012 by Hang
	map<int, void(*)(void*)> pluginLeftMouseFuncs;     // May 29, 2012 by Hang

public slots:
   	virtual void stillPaint(); //for deferred full-resolution volume painting, connected to still_timer


#define __view3dcontrol_interface__
public:
	View3DControl * getView3DControl() {return dynamic_cast<View3DControl *>(this);}
//----------------------------------------------------------------------------------------
// begin View3DControl interface
//----------------------------------------------------------------------------------------
public:
    // Expose marker size, Nov. 08, 2016 by Zhi Zhou
    virtual int getMarkerSize() const { return _markerSize; }
    // Expose surface cut getters, Dec.01,2013 by Zhi Zhou
    virtual int xClip0() const { return _xClip0; }
    virtual int xClip1() const { return _xClip1; }
    virtual int yClip0() const { return _yClip0; }
    virtual int yClip1() const { return _yClip1; }
    virtual int zClip0() const { return _zClip0; }
    virtual int zClip1() const { return _zClip1; }


    virtual int isShowSurfObjects() const { return renderer->sShowSurfObjects;} // Nov. 27, 2013 by Zhi Zhou

    virtual void setState(int state, bool is_enable){ if(is_enable) currentPluginState = state; else currentPluginState = -1;}             // May 29, 2012 by Hang
	virtual void addStateFunc(int state, void(*mouse_func)(void *)){pluginLeftMouseFuncs[state] = mouse_func;} // May 29, 2012 by Hang
	virtual void deleteStateFunc(int state){pluginLeftMouseFuncs.erase(state);} // May 29, 2012 by Hang

	virtual int renderMode() const { return _renderMode; }
	virtual int dataDim1() const { return _data_size[0]; }
	virtual int dataDim2() const { return _data_size[1]; }
	virtual int dataDim3() const { return _data_size[2]; }
	virtual int dataDim4() const { return _data_size[3]; }
	virtual int dataDim5() const { return _data_size[4]; }

	virtual int xRot() const { return _xRot; }
	virtual int yRot() const { return _yRot; }
	virtual int zRot() const { return _zRot; }
	virtual int zoom() const { return _zoom; }
	virtual int xShift() const { return _xShift; }
	virtual int yShift() const { return _yShift; }
	virtual int zShift() const { return _zShift; }
	virtual bool isAbsoluteRot()	const { return _absRot; }

	// Expose cut getters, 2011 Feb 09 CMB
    virtual int xCut0() const { return _xCut0; }
    virtual int xCut1() const { return _xCut1; }
    virtual int yCut0() const { return _yCut0; }
    virtual int yCut1() const { return _yCut1; }
    virtual int zCut0() const { return _zCut0; }
    virtual int zCut1() const { return _zCut1; }
    virtual int frontCut() const { return _fCut; }
    virtual int xCS() const {return _xCS;}
    virtual int yCS() const {return _yCS;}
    virtual int zCS() const {return _zCS;}
    virtual bool channelR() {return renderer->color_proxy.r != 0;}
    virtual bool channelG() {return renderer->color_proxy.g != 0;}
    virtual bool channelB() {return renderer->color_proxy.b != 0;}
    virtual int volumeTimePoint() const {return _volumeTimePoint;}

	virtual bool isVolCompress() const { return (renderer)? renderer->tryTexCompress :false; }
	virtual bool isShowBoundingBox() const { return (renderer)? renderer->bShowBoundingBox :false; }
	virtual bool isShowAxes() 		const { return (renderer)? renderer->bShowAxes :false; }

	virtual void hideTool();
	virtual void showTool();
	virtual void updateTool();
	virtual void updateControl();

    // @ADDED by Alessandro on 2015-09-30. See 'show_progress_bar'.
    bool         getShowProgressBar(){return show_progress_bar;}
    void         setShowProgressBar(bool val){show_progress_bar = val;}

public slots:
// most of format: set***(type) related to a change***(type)

	virtual int setVolumeTimePoint(int t);
	virtual void incVolumeTimePoint(float step);

	/** if useMin == false => minimum intensity projection **/
	virtual void setRenderMode_Mip(bool b, bool useMin = false);

	virtual void setRenderMode_Maxip(bool b) {
		setRenderMode_Mip(b, false);
	}
	virtual void setRenderMode_Minip(bool b) {
		setRenderMode_Mip(b, true);
	}

	virtual void setRenderMode_Alpha(bool b);
	virtual void setRenderMode_Cs3d(bool b);

	virtual void setCSTransparent(int);
	virtual void setThickness(double);
	virtual void setCurChannel(int);

	virtual void setChannelR(bool b);
	virtual void setChannelG(bool b);
	virtual void setChannelB(bool b);
	virtual void setVolCompress(bool b);

	virtual void volumeColormapDialog();
	virtual void surfaceSelectDialog(int curTab=-1); // 090505 RZC: add curTab.  110713 RZC: change curTab=-1 from 0
	virtual void surfaceSelectTab(int curTab=-1); // 090522 RZC: just switch to curTab, no creating
	virtual void surfaceDialogHide(); //added 090220, by PHC for convenience
	virtual void annotationDialog(int dataClass, int surfaceType, int index);

	virtual void setXRotation(int angle);
	virtual void setYRotation(int angle);
	virtual void setZRotation(int angle);
	virtual void resetRotation(bool b_emit=true);
	virtual void modelRotation(int xRotStep, int yRotStep, int zRotStep);
	virtual void viewRotation(int xRotStep, int yRotStep, int zRotStep);
	virtual void absoluteRotPose();
	virtual void absoluteVRview();
	virtual void doAbsoluteRot(int xRot, int yRot, int zRot);
	virtual void lookAlong(float xLook, float yLook, float zLook); //100812 RZC

	virtual void setZoom(int r);
	virtual void setXShift(int s);
	virtual void setYShift(int s);
	virtual void setZShift(int s);
	virtual void resetZoomShift();

	// Float methods for smoother animation 2011 Feb 07 CMB
    virtual void setXShift(float s);
    virtual void setYShift(float s);
    virtual void setZShift(float s);
    virtual void setZoom(float r);
    virtual void doAbsoluteRot(float xRot, float yRot, float zRot);
    virtual void setXRotation(float angle);
    virtual void setYRotation(float angle);
    virtual void setZRotation(float angle);

	virtual void enableFrontSlice(bool);
	virtual void enableXSlice(bool);
	virtual void enableYSlice(bool);
	virtual void enableZSlice(bool);

	virtual void setFrontCut(int s);
	virtual void setXCut0(int s);
	virtual void setYCut0(int s);
	virtual void setZCut0(int s);
	virtual void setXCut1(int s);
	virtual void setYCut1(int s);
	virtual void setZCut1(int s);
	virtual void setXCS(int s);
	virtual void setYCS(int s);
	virtual void setZCS(int s);
	virtual void setXClip0(int s);
	virtual void setYClip0(int s);
	virtual void setZClip0(int s);
	virtual void setXClip1(int s);
	virtual void setYClip1(int s);
	virtual void setZClip1(int s);

	virtual void setXCutLock(bool);
	virtual void setYCutLock(bool);
	virtual void setZCutLock(bool);

	virtual void enableShowAxes(bool b);
    virtual void enableShowBoundingBox(bool b);
    virtual void enableClipBoundingBox(bool b);  //141013 Hanbo Chen
	virtual void enableOrthoView(bool b);
	virtual void setBackgroundColor();
	virtual void setBright();

	virtual void setShowMarkers(int s);
	virtual void setShowSurfObjects(int s);
	virtual void enableMarkerLabel(bool);
	virtual void setMarkerSize(int s);
	virtual void enableSurfStretch(bool);
	virtual void toggleCellName();
	virtual void toggleMarkerName();// by Lei Qu, 110425

	virtual void createSurfCurrentR();
	virtual void createSurfCurrentG();
	virtual void createSurfCurrentB();
	virtual void loadObjectFromFile(QString url="");
	virtual void loadObjectListFromFile();
	virtual void saveSurfFile();

	virtual void togglePolygonMode();
	virtual void toggleLineType();
	virtual void toggleObjShader();

    //defined for Katie's need to export the local 3D viewer starting and local locations //140811
    virtual int getLocalStartPosX();
    virtual int getLocalStartPosY();
    virtual int getLocalStartPosZ();
    virtual int getLocalEndPosX();
    virtual int getLocalEndPosY();
    virtual int getLocalEndPosZ();

    //

     virtual void toggleNStrokeCurveDrawing(); // For n-right-strokes curve shortcut ZJL 110920

     virtual void setDragWinSize(int csize); // ZJL 110927

	virtual void changeLineOption();
	virtual void changeVolShadingOption();
	virtual void changeObjShadingOption();

	virtual void toggleTexFilter();
	virtual void toggleTex2D3D();
	virtual void toggleTexCompression();
	virtual void toggleTexStream();
	virtual void toggleShader();
	virtual void showGLinfo();

	virtual void updateWithTriView();
    virtual void updateLandmark();
    virtual void updateImageData();
	virtual void reloadData();
	virtual void cancelSelect();

//----------------------------------------------------------------------------------------
// end View3DControl interface
//----------------------------------------------------------------------------------------

signals:
// most of format: change***(type)
// most of format: changeEnable***(bool)
// most of format: signal***()

	void signalInitControlValue();
	void signalVolumeCutRange();
	void signalOnlySurfaceObj();

	void changeVolumeTimePoint(int);

	void changeDispType_maxip(bool);
	void changeDispType_minip(bool);
	void changeDispType_alpha(bool);
	void changeDispType_cs3d(bool);

	void changeVolCompress(bool);
	void changeEnableVolCompress(bool);
	void changeEnableVolColormap(bool);

	void changeTransparentSliderLabel(const QString&);
	void changeEnableTransparentSlider(bool);
	void changeMarkerSize(int);

	void xRotationChanged(int);
    void yRotationChanged(int);
    void zRotationChanged(int);

    void zoomChanged(int);
    void xShiftChanged(int);
    void yShiftChanged(int);
    void zShiftChanged(int);

	void changeEnableCut0Slider(bool);
    void changeEnableCut1Slider(bool);
    void changeCurrentTabCutPlane(int);
    void changeEnableTabCutPlane(int,bool);

    void changeXCSSlider(int s);
    void changeYCSSlider(int s);
    void changeZCSSlider(int s);
    void changeFrontCut(int s);

    void changeXCut0(int s);
	void changeXCut1(int s);
    void changeYCut0(int s);
	void changeYCut1(int s);
    void changeZCut0(int s);
	void changeZCut1(int s);

    void changeXClip0(int s);
	void changeXClip1(int s);
    void changeYClip0(int s);
	void changeYClip1(int s);
    void changeZClip0(int s);
	void changeZClip1(int s);

	void changeOrthoView(bool b);
        void neuronShown(const QList<int> overlayList); // view neuron in Neuron Annotator
        void neuronShownAll(const QList<int> overlayList);
        void neuronClearAll();
        void neuronIndexChanged(int index);
        void neuronClearAllSelections();

public:
	bool _still, _stillpaint_need, _stillpaint_pending;
    QTimer still_timer;
    static const int still_timer_interval = 1000; //1000 is safe enough

	int t_mouseclick_left, mouse_held, mouse_in_view;

    bool _in_destructor; //for makeCurrent when valid context
    bool _isSoftwareGL; //for choiceRenderer

	int _renderMode;
	//unsigned char * data;
	int _data_size[5];

	char tipBuf[1000];
	bool _holding_num[10];

	int viewW, viewH;
	GLdouble mRot[16];
	static const int flip_X= +1, flip_Y= -1, flip_Z= -1; // make y-axis downward conformed with image coordinate
	QPoint lastPos;

	float _xRot, _yRot, _zRot, dxRot, dyRot, dzRot;
	// int _zoom, _xShift, _yShift, _zShift, dxShift, dyShift, dzShift;
    float _zoom, _xShift, _yShift, _zShift, dxShift, dyShift, dzShift; // CMB 2011 Feb 07
	int _xCut0, _xCut1, _yCut0, _yCut1, _zCut0, _zCut1, _fCut;
	int dxCut, dyCut, dzCut, lockX, lockY, lockZ;
	int _xCS, _yCS, _zCS;
	int _xClip0, _xClip1, _yClip0, _yClip1, _zClip0, _zClip1;
	int _CStransparency, _markerSize, _curChannel;
	float _thickness;
	int _Bright, _Contrast, sUpdate_bright, sUpdate_track;
    bool _showAxes, _showBoundingBox, _absRot, _orthoView, _clipBoxEnable;
	bool _volCompress, _volFilter;

	int _volumeTimePoint; float volumeTimPoint_fraction;

	void init_members()
	{
		_still = _stillpaint_need = _stillpaint_pending = false;
	    connect(&still_timer, SIGNAL(timeout()), this, SLOT(stillPaint())); //only connect once
	    still_timer.start(still_timer_interval);

	    t_mouseclick_left = mouse_held = mouse_in_view = 0;

	    _in_destructor =false;
		_isSoftwareGL =false;

		_renderMode = 0;
		for (int i=0; i<5; i++)	_data_size[i] = 0;

		for (int i=0; i<10; i++) _holding_num[i] = false;

		viewW=viewH=0;
		for (int i=0; i<4; i++)
			for (int j=0; j<4; j++)
				mRot[i*4 +j] = (i==j)? 1 : 0; // Identity matrix

		_xRot=_yRot=_zRot= dxRot=dyRot=dzRot=
		_zoom=_xShift=_yShift=_zShift= dxShift=dyShift=dzShift=
		_xCut0=_xCut1=_yCut0=_yCut1=_zCut0=_zCut1=_fCut=
		dxCut=dyCut=dzCut= lockX=lockY=lockZ=
		_xCS=_yCS=_zCS=
		_xClip0=_xClip1=_yClip0=_yClip1=_zClip0=_zClip1 =0;
		_thickness =1;
		_CStransparency=0; _markerSize=1, _curChannel=-1;

		_Bright=_Contrast=sUpdate_bright=sUpdate_track=0;
		_showAxes = _showBoundingBox = _absRot = _orthoView =false;
        _clipBoxEnable = _volCompress = _volFilter =true;

		_volumeTimePoint=0;
		volumeTimPoint_fraction=0;
	}
};

#endif
