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




/*
 * v3dr_control_signal.cpp
 *
 *  Created on: Sep 30, 2008
 *      Author: ruanzongcai
 *
 * last update: 090618: by Hanchuan Peng, add global option to default video card compress, axes display and bounding box display states
 */

#include "v3dr_mainwindow.h"


#ifndef MIN
#define MIN(a, b)  ( ((a)<(b))? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a, b)  ( ((a)>(b))? (a) : (b) )
#endif


QString xCut_altTip(QWidget* parent, int v, int minv, int maxv, int offset)
{
	V3dR_GLWidget* w;
	if (parent && (w = ((V3dR_MainWindow*)parent)->getGLWidget()))
		return w->Cut_altTip(1, v, minv, maxv, offset);
	else
		return "";
}
QString yCut_altTip(QWidget* parent, int v, int minv, int maxv, int offset)
{
	V3dR_GLWidget* w;
	if (parent && (w = ((V3dR_MainWindow*)parent)->getGLWidget()))
		return w->Cut_altTip(2, v, minv, maxv, offset);
	else
		return "";
}
QString zCut_altTip(QWidget* parent, int v, int minv, int maxv, int offset)
{
	V3dR_GLWidget* w;
	if (parent && (w = ((V3dR_MainWindow*)parent)->getGLWidget()))
		return w->Cut_altTip(3, v, minv, maxv, offset);
	else
		return "";
}


void V3dR_MainWindow::createControlWidgets()
{
	//qDebug("V3dR_MainWindow::createControlWigdets");
	int i;

    toolBtnGroup = new QWidget;
    QHBoxLayout *layout_toolBtnGroup = new QHBoxLayout(toolBtnGroup);

    volumeColormapButton = new QPushButton("Vol Colormap");
    surfobjManagerButton = new QPushButton("Object Manager"); //Pick/Color"); // 090422 RZC
    layout_toolBtnGroup->addWidget(volumeColormapButton);
    layout_toolBtnGroup->addWidget(surfobjManagerButton);


#define __volume_display_option_box__
	//------------------------------------------------------------------------------
	//volume display option box

    QWidget *volDisplayOptGroup = new QWidget;
    //QGroupBox *volDisplayOptGroup = new QGroupBox();
    //volDisplayOptGroup->setTitle("Volume display options");
    QGridLayout *layout_mainDisplayOptGroup = new QGridLayout(volDisplayOptGroup);
    //layout_mainDisplayOptGroup->setContentsMargins(10,1,1,1);

    dispType_maxip = new QRadioButton("MIP", volDisplayOptGroup);
    dispType_minip = new QRadioButton("mIP", volDisplayOptGroup);
    
    dispType_alpha = new QRadioButton("Alpha", volDisplayOptGroup);
    dispType_cs3d = new QRadioButton("X-section", volDisplayOptGroup);
    dispType_maxip->setToolTip("Maximum Intensity Projection");
    dispType_minip->setToolTip("Minimum Intensity Projection");
    dispType_alpha->setToolTip("Alpha Blending Projection\n (affected by background color)"); //110714
    dispType_cs3d->setToolTip("Cross-section");

    //QLabel *labelline = new QLabel; labelline->setFrameStyle(QFrame::HLine | QFrame::Raised); labelline->setLineWidth(1);

    transparentSlider = createTranparentSlider();
    zthicknessBox = createThicknessSpinBox();
    comboBox_channel = createChannelComboBox();
    zthicknessBox->setToolTip("more thick more Slow!");
	comboBox_channel->setToolTip("Set the default channel for pinpointing in 3D image.");

	checkBox_channelR = new QCheckBox("R", volDisplayOptGroup);
	checkBox_channelG = new QCheckBox("G", volDisplayOptGroup);
	checkBox_channelB = new QCheckBox("B", volDisplayOptGroup);
	checkBox_volCompress = new QCheckBox("Compress", volDisplayOptGroup);
	checkBox_channelR->setToolTip("output Red color on screen");
	checkBox_channelG->setToolTip("output Green color on screen");
	checkBox_channelB->setToolTip("output Blue color on screen");
	checkBox_volCompress->setToolTip("Compressed format can improve\n interactive speed of static volume");
			//".\nUncompressed format for very large volume may cause system halt!");

	layout_mainDisplayOptGroup->addWidget(dispType_maxip, 1, 0, 1, 5);
	layout_mainDisplayOptGroup->addWidget(dispType_minip, 1, 5, 1, 5);
	layout_mainDisplayOptGroup->addWidget(dispType_alpha, 1, 5+5, 1, 7);
	layout_mainDisplayOptGroup->addWidget(dispType_cs3d, 1, 5+5+7, 1, 9);

	//layout_mainDisplayOptGroup->addWidget(labelline, 2, 0, 1, 21);

	layout_mainDisplayOptGroup->addWidget(transparentSlider_Label = new QLabel("Transparency"), 2, 0, 1, 9);
	layout_mainDisplayOptGroup->addWidget(transparentSlider,       2, 9-1, 1, 13);

	layout_mainDisplayOptGroup->addWidget(new QLabel("Z-thick"), 3, 0, 1, 8);
	layout_mainDisplayOptGroup->addWidget(zthicknessBox,         3, 5, 1, 7);

	layout_mainDisplayOptGroup->addWidget(new QLabel("M-chan"), 3, 12, 1, 8);
	layout_mainDisplayOptGroup->addWidget(comboBox_channel,    3, 12+5, 1, 5);

	layout_mainDisplayOptGroup->addWidget(checkBox_channelR,      4, 0, 1, 4);
	layout_mainDisplayOptGroup->addWidget(checkBox_channelG,      4, 0+4, 1, 4);
	layout_mainDisplayOptGroup->addWidget(checkBox_channelB,      4, 0+4+4, 1, 4);
	layout_mainDisplayOptGroup->addWidget(checkBox_volCompress, 4, 12+1, 1, 9);


//	layout_mainDisplayOptGroup->setRowStretch( 1, 21 );
//	layout_mainDisplayOptGroup->addWidget(volumeColormapButton, 6, 0, 1, 21);


#define __surface_display_option_box__
    //------------------------------------------------------------------------------
	//surface display option box

	QWidget *surfDisplayOptGroup = new QWidget;
    QGridLayout *surfDisplayOptLayout = new QGridLayout(surfDisplayOptGroup);

	checkBox_displayMarkers = new QCheckBox("Markers");
	checkBox_displaySurf = new QCheckBox("Surfaces");
	checkBox_displayMarkers->setTristate(true);
	checkBox_displaySurf->setTristate(true);  // (off, occlusion-by-image, floating-over-image)
	checkBox_displayMarkers->setToolTip("Tri-state: off, occlusion-by-image, floating-over-image");
	checkBox_displaySurf->setToolTip("Tri-state: off, occlusion-by-image, floating-over-image");

	updateLandmarkButton = new QPushButton("Sync Tri-view Objs");
	loadSaveObjectsButton = new QPushButton("Load/Save Surf >>");

    spinBox_markerSize = createMarkerSizeSpinBox(); //marker size is related with voxel size
    checkBox_markerLabel = new QCheckBox("Label");
    checkBox_surfStretch = new QCheckBox("Stretch with Volume");

    surfDisplayOptLayout->addWidget(checkBox_displayMarkers, 1, 0, 1, 7);
    surfDisplayOptLayout->addWidget(updateLandmarkButton, 1, 7, 1, 20-6);
    surfDisplayOptLayout->addWidget(checkBox_markerLabel, 2, 2, 1, 10);
    surfDisplayOptLayout->addWidget(new QLabel("Size"), 2, 9, 1, 10);
    surfDisplayOptLayout->addWidget(spinBox_markerSize, 2, 12, 1, 21-12);

    surfDisplayOptLayout->addWidget(checkBox_displaySurf, 3, 0, 1, 7);
    surfDisplayOptLayout->addWidget(loadSaveObjectsButton, 3, 7, 1, 20-6);
    surfDisplayOptLayout->addWidget(checkBox_surfStretch, 4, 2, 1, 20-2);


//    surfDisplayOptLayout->setRowStretch( 1, 21 );
//    surfDisplayOptLayout->addWidget(surfobjManagerButton, 6, 0, 1, 21);


#define __others_display_option_box__
    //------------------------------------------------------------------
    //others display option box

    QWidget *miscDisplayOptGroup = new QWidget(); // 090422 RZC
//    QGroupBox *miscDisplayOptGroup = new QGroupBox();
//    miscDisplayOptGroup->setTitle("Other options");
    QGridLayout *layout_miscDisplayOptGroup = new QGridLayout(miscDisplayOptGroup);

	checkBox_displayAxes = new QCheckBox("Axes", miscDisplayOptGroup);
	checkBox_displayBoundingBox = new QCheckBox("Bounding box", miscDisplayOptGroup);
	colorButton = new QPushButton("Color >>", miscDisplayOptGroup);
	brightButton = new QPushButton("Brighten", miscDisplayOptGroup);

	checkBox_OrthoView = new QCheckBox("Parallel (Scale bar)", miscDisplayOptGroup);

    movieSaveButton = new QPushButton("Save Movie", miscDisplayOptGroup);
    animateButton = new QPushButton("Animate >>", miscDisplayOptGroup);
    //reloadDataButton = new QPushButton("Reload", miscDisplayOptGroup);

	layout_miscDisplayOptGroup->addWidget(checkBox_displayAxes, 1, 0, 1, 20);
	layout_miscDisplayOptGroup->addWidget(checkBox_displayBoundingBox, 2, 0, 1, 20);
	layout_miscDisplayOptGroup->addWidget(checkBox_OrthoView, 3, 0, 1, 20);
	layout_miscDisplayOptGroup->addWidget(colorButton, 4, 0, 1, 12);

	layout_miscDisplayOptGroup->addWidget(movieSaveButton, 1, 1+11, 1, 21-12);
	layout_miscDisplayOptGroup->addWidget(animateButton, 2, 1+11, 1, 21-12);
	//layout_miscDisplayOptGroup->addWidget(reloadDataButton, 3, 1+11, 1, 21-12);
	layout_miscDisplayOptGroup->addWidget(brightButton, 4, 1+11, 1, 21-12);


    //==============================================================================
	//volume/surface/misc tab box

	tabOptions = new QTabWidget(); //AutoTabWidget(); //commented by PHC, 090117
	tabOptions->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	i= tabOptions->addTab(volDisplayOptGroup, "Volume");
	i= tabOptions->addTab(surfDisplayOptGroup, "Surf/Object");
	i= tabOptions->addTab(miscDisplayOptGroup, "Others");
	tabOptions->setTabToolTip(0, "Volume Image");
	tabOptions->setTabToolTip(1, "Surface/Object");
	tabOptions->setTabToolTip(2, "Other Options");

	//==============================================================================


#define __rotation_slider_box__
    //------------------------------------------------------------------------------
	//rotation sliders

    QWidget *rotateBarGroup = new QWidget;
    //QGroupBox *rotateBarGroup = new QGroupBox();
    //rotateBarGroup->setTitle("Rotations around axes");
    QGridLayout *rotateBarBoxLayout = new QGridLayout(rotateBarGroup);

    //QFont smallFont;   smallFont.setPixelSize(12); smallFont.setBold(false);
    //rotateBarGroup->setFont(smallFont); // no any use

	xRotSlider = createRotateSlider();
    yRotSlider = createRotateSlider();
    zRotSlider = createRotateSlider();
	  xRotBox = createRotateSpinBox();
	  yRotBox = createRotateSpinBox();
	  zRotBox = createRotateSpinBox();
//	  checkBox_absoluteRot = new QCheckBox("Absolute rotation", rotateBarGroup);
//	  checkBox_absoluteRot->setToolTip("Absolute rotation (around X-Y-Z sequentially from reset pose)");
	rotReset = new QPushButton("Zero", rotateBarGroup);
	  rotAbsolute = new QPushButton("Go", rotateBarGroup);
	  rotAbsolute->setToolTip("Go to rotation pose about X-Y-Z sequentially from Zero pose");
	  rotAbsolutePose = new QPushButton("Freeze", rotateBarGroup);
	  rotAbsolutePose->setToolTip("Absolutize rotation pose about X-Y-Z sequentially for Going back next time");
	  rotVRView = new QPushButton("See in VR", rotateBarGroup);
	  rotVRView->setToolTip("You can see current image in VR environment.");
//    rotateBarBoxLayout->addWidget(xrotateLabel, 1, 0, 1, 5);
//    rotateBarBoxLayout->addWidget(xRotSlider, 1, 6, 1, 15);
//
//    rotateBarBoxLayout->addWidget(yrotateLabel, 2, 0, 1, 5);
//    rotateBarBoxLayout->addWidget(yRotSlider, 2, 6, 1, 15);
//
//    rotateBarBoxLayout->addWidget(zrotateLabel, 3, 0, 1, 5);
//    rotateBarBoxLayout->addWidget(zRotSlider, 3, 6, 1, 15);

	rotateBarBoxLayout->addWidget(new QLabel("X-"), 1, 0, 1, 7);
	rotateBarBoxLayout->addWidget(new QLabel("Y-"), 1, 7, 1, 7);
	rotateBarBoxLayout->addWidget(new QLabel("Z-"), 1, 7+7, 1, 7);

	rotateBarBoxLayout->addWidget(xRotSlider, 1, 0+1, 1, 7);
	rotateBarBoxLayout->addWidget(yRotSlider, 1, 7+1, 1, 7);
	rotateBarBoxLayout->addWidget(zRotSlider, 1, 7+7+1, 1, 7);

	rotateBarBoxLayout->addWidget(xRotBox, 2, 0+1, 1, 7);
	rotateBarBoxLayout->addWidget(yRotBox, 2, 7+1, 1, 7);
	rotateBarBoxLayout->addWidget(zRotBox, 2, 7+7+1, 1, 7);

	rotateBarBoxLayout->addWidget(rotAbsolutePose, 3, 0, 1, 7);
	rotateBarBoxLayout->addWidget(rotAbsolute, 3, 7+1, 1, 7);
	rotateBarBoxLayout->addWidget(rotReset, 3, 14+2, 1, 7);
	rotateBarBoxLayout->addWidget(rotVRView, 4, 7+1, 1, 7);//wwbmark freeze

	//rotateBarBoxLayout->setContentsMargins(0,0,0,0);


#define __zoom_shift_slider_box__
	//------------------------------------------------------------------------
    // zoom & shift slider

    QWidget *zoomBarGroup = new QWidget;
    //QGroupBox *zoomBarGroup = new QGroupBox();
    //zoomBarGroup->setTitle("Zoom && Shift");
    QGridLayout *zoomBarBoxLayout = new QGridLayout(zoomBarGroup);

	zoomSlider = createZoomSlider( Qt::Vertical); //080930
	zoomReset = new QPushButton("Reset", zoomBarGroup);
	xShiftSlider = createShiftSlider();
    yShiftSlider = createShiftSlider(Qt::Vertical); //080930
	  xShiftBox = createShiftSpinBox();
	  yShiftBox = createShiftSpinBox();
	  zoomBox = createZoomSpinBox();

//    zoomBarBoxLayout->addWidget(zoomLabel, 1, 0, 1, 4);
//    zoomBarBoxLayout->addWidget(zoomSlider, 1, 1+4, 1, 11);
//    zoomBarBoxLayout->addWidget(zoomReset, 1, 1+4+11, 1, 20-11-4);
//
//    zoomBarBoxLayout->addWidget(xshiftLabel, 2, 0, 1, 4);
//    zoomBarBoxLayout->addWidget(xShiftSlider, 2, 1+4, 1, 20-4);
//
//    zoomBarBoxLayout->addWidget(yshiftLabel, 3, 0, 1, 4);
//    zoomBarBoxLayout->addWidget(yShiftSlider, 3, 1+4, 1, 20-4);

	QLabel* zoomLabel;
    zoomBarBoxLayout->addWidget(zoomLabel= new QLabel("Zoom"), 1, 0, 1, 5);
	zoomBarBoxLayout->addWidget(zoomSlider, 2, 0, 3, 3);
    zoomBarBoxLayout->addWidget(zoomBox, 2, 3, 1, 6);
    QString zoom_tip = "Note: The zoom factor is NOT linear about display size.";
    zoomLabel->setToolTip(zoom_tip);
    zoomSlider->setToolTip(zoom_tip);
    zoomBox->setToolTip(zoom_tip);

	//zoomBarBoxLayout->addWidget(new QLabel("Hori"), 3, 1+3+3, 1, 3);
	zoomBarBoxLayout->addWidget(xShiftSlider, 4, 3, 1, 16);
	zoomBarBoxLayout->addWidget(xShiftBox, 3, 3+(16-6)/2, 1, 6);

	//zoomBarBoxLayout->addWidget(new QLabel("Vert"), 1, 20-3-3, 1, 3);
	zoomBarBoxLayout->addWidget(yShiftSlider, 1, 21-3, 4, 3);
	zoomBarBoxLayout->addWidget(yShiftBox, 2, 21-3-6, 1, 6);

	zoomBarBoxLayout->addWidget(zoomReset, 1, (21-9)/2, 1, 9);



	//=============================================================================
	// rotation/zoom-shift tab box

	tabRotZoom = new QTabWidget();//tabRotZoom = new AutoTabWidget(); //commented by PHC, 090117
	tabRotZoom->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	i= tabRotZoom->addTab(rotateBarGroup, "Rotation");
    i= tabRotZoom->addTab(zoomBarGroup,   "Zoom && Shift");
	tabRotZoom->setTabToolTip(0, "Rotation");
    tabRotZoom->setTabToolTip(1, "Zoom & Shift");

    //=============================================================================


#define __cut_plane_slider_box__
    //-----------------------------------------------------------------------------
    //cut-plane boxes (1-volume & 2-section shared same place, 3-surface)

	QWidget *cutPlaneGroup[1+3];
	cutPlaneGroup[1] = new QWidget;
	cutPlaneGroup[2] = new QWidget;
	cutPlaneGroup[3] = new QWidget;
//	QGroupBox *cutPlaneGroup[1+3];
//	cutPlaneGroup[1] = new QGroupBox();
//	cutPlaneGroup[2] = new QGroupBox();
//	cutPlaneGroup[3] = new QGroupBox();
//	cutPlaneGroup[1]->setTitle("Volume cut-planes");
//	cutPlaneGroup[2]->setTitle("X-section planes");
//	cutPlaneGroup[3]->setTitle("Surface cut-planes");
	QGridLayout *cutPlaneRgnLayout[1+3];
	for (int i=1; i<=3; i++)
	switch (i)
	{
	case 1: {// volume
		int d1, d2, d3;
		d1 = MAX(0, glWidget->dataDim1()-1);
		d2 = MAX(0, glWidget->dataDim2()-1);
		d3 = MAX(0, glWidget->dataDim3()-1);
		xcminSlider = createCutPlaneSlider(d1);
		xcmaxSlider = createCutPlaneSlider(d1);
		ycminSlider = createCutPlaneSlider(d2);
		ycmaxSlider = createCutPlaneSlider(d2);
		zcminSlider = createCutPlaneSlider(d3);
		zcmaxSlider = createCutPlaneSlider(d3);
		fcutSlider = createRangeSlider(0, CLIP_RANGE);
//		xcLock = new QCheckBox();
//		ycLock = new QCheckBox();
//		zcLock = new QCheckBox();
		xcLock = new QToolButton(); xcLock->setCheckable(true);
		ycLock = new QToolButton(); ycLock->setCheckable(true);
		zcLock = new QToolButton(); zcLock->setCheckable(true);

		cutPlaneRgnLayout[i] = new QGridLayout(cutPlaneGroup[i]);

//		cutPlaneRgnLayout[i]->addWidget(new QLabel("X-min"), 1, 0, 1, 5);
//		cutPlaneRgnLayout[i]->addWidget(xcminSlider, 1, 6, 1, 15);
//
//		cutPlaneRgnLayout[i]->addWidget(new QLabel("X-max"), 2, 0, 1, 5);
//		cutPlaneRgnLayout[i]->addWidget(xcmaxSlider, 2, 6, 1, 15);
//
//		cutPlaneRgnLayout[i]->addWidget(new QLabel("Y-min"), 3, 0, 1, 5);
//		cutPlaneRgnLayout[i]->addWidget(ycminSlider, 3, 6, 1, 15);
//
//		cutPlaneRgnLayout[i]->addWidget(new QLabel("Y-max"), 4, 0, 1, 5);
//		cutPlaneRgnLayout[i]->addWidget(ycmaxSlider, 4, 6, 1, 15);
//
//		cutPlaneRgnLayout[i]->addWidget(new QLabel("Z-min"), 5, 0, 1, 5);
//		cutPlaneRgnLayout[i]->addWidget(zcminSlider, 5, 6, 1, 15);
//
//		cutPlaneRgnLayout[i]->addWidget(new QLabel("Z-max"), 6, 0, 1, 5);
//		cutPlaneRgnLayout[i]->addWidget(zcmaxSlider, 6, 6, 1, 15);

		cutPlaneRgnLayout[i]->addWidget(new QLabel("X-cut"), 1, 0, 1, 5);
		cutPlaneRgnLayout[i]->addWidget(xcminSlider, 1, 6, 1, 15);

		cutPlaneRgnLayout[i]->addWidget(xcLock, 2, 1, 1, 3);
		cutPlaneRgnLayout[i]->addWidget(xcmaxSlider, 2, 6, 1, 15);

		cutPlaneRgnLayout[i]->addWidget(new QLabel("Y-cut"), 3, 0, 1, 5);
		cutPlaneRgnLayout[i]->addWidget(ycminSlider, 3, 6, 1, 15);

		cutPlaneRgnLayout[i]->addWidget(ycLock, 4, 1, 1, 3);
		cutPlaneRgnLayout[i]->addWidget(ycmaxSlider, 4, 6, 1, 15);

		cutPlaneRgnLayout[i]->addWidget(new QLabel("Z-cut"), 5, 0, 1, 5);
		cutPlaneRgnLayout[i]->addWidget(zcminSlider, 5, 6, 1, 15);

		cutPlaneRgnLayout[i]->addWidget(zcLock, 6, 1, 1, 3);
		cutPlaneRgnLayout[i]->addWidget(zcmaxSlider, 6, 6, 1, 15);

		QLabel* front_label=0;
		cutPlaneRgnLayout[i]->addWidget(front_label = new QLabel("Front"), 7, 0, 1, 5);
		cutPlaneRgnLayout[i]->addWidget(fcutSlider, 7, 6, 1, 15);
		front_label->setToolTip("Front-cut-plane linked with the F-Slice in X-Section mode.");

	}break;
	case 2: {// x-section
		int d1, d2, d3;
		d1 = MAX(0, glWidget->dataDim1()-1);
		d2 = MAX(0, glWidget->dataDim2()-1);
		d3 = MAX(0, glWidget->dataDim3()-1);
		xCSSlider = createCutPlaneSlider(d1);
		yCSSlider = createCutPlaneSlider(d2);
		zCSSlider = createCutPlaneSlider(d3);
		fCSSlider = createRangeSlider(0, CLIP_RANGE);

		cutPlaneRgnLayout[i] = new QGridLayout(cutPlaneGroup[i]);

		cutPlaneRgnLayout[i]->addWidget(checkBox_xCS = new QCheckBox("X-slice"), 1, 0, 1, 5);
		cutPlaneRgnLayout[i]->addWidget(xCSSlider, 1, 6, 1, 15);

		cutPlaneRgnLayout[i]->addWidget(checkBox_yCS = new QCheckBox("Y-slice"), 2, 0, 1, 5);
		cutPlaneRgnLayout[i]->addWidget(yCSSlider, 2, 6, 1, 15);

		cutPlaneRgnLayout[i]->addWidget(checkBox_zCS = new QCheckBox("Z-slice"), 3, 0, 1, 5);
		cutPlaneRgnLayout[i]->addWidget(zCSSlider, 3, 6, 1, 15);

		cutPlaneRgnLayout[i]->addWidget(checkBox_fCS = new QCheckBox("F-slice"), 4, 0, 1, 5);
		cutPlaneRgnLayout[i]->addWidget(fCSSlider, 4, 6, 1, 15);

	}break;
	case 3: {// surface
		xSminSlider = createCutPlaneSlider(CLIP_RANGE);
		xSmaxSlider = createCutPlaneSlider(CLIP_RANGE);
		ySminSlider = createCutPlaneSlider(CLIP_RANGE);
		ySmaxSlider = createCutPlaneSlider(CLIP_RANGE);
		zSminSlider = createCutPlaneSlider(CLIP_RANGE);
		zSmaxSlider = createCutPlaneSlider(CLIP_RANGE);

		cutPlaneRgnLayout[i] = new QGridLayout(cutPlaneGroup[i]);

		cutPlaneRgnLayout[i]->addWidget(new QLabel("X-min"), 1, 0, 1, 5);
		cutPlaneRgnLayout[i]->addWidget(xSminSlider, 1, 6, 1, 15);

		cutPlaneRgnLayout[i]->addWidget(new QLabel("X-max"), 2, 0, 1, 5);
		cutPlaneRgnLayout[i]->addWidget(xSmaxSlider, 2, 6, 1, 15);

		cutPlaneRgnLayout[i]->addWidget(new QLabel("Y-min"), 3, 0, 1, 5);
		cutPlaneRgnLayout[i]->addWidget(ySminSlider, 3, 6, 1, 15);

		cutPlaneRgnLayout[i]->addWidget(new QLabel("Y-max"), 4, 0, 1, 5);
		cutPlaneRgnLayout[i]->addWidget(ySmaxSlider, 4, 6, 1, 15);

		cutPlaneRgnLayout[i]->addWidget(new QLabel("Z-min"), 5, 0, 1, 5);
		cutPlaneRgnLayout[i]->addWidget(zSminSlider, 5, 6, 1, 15);

		cutPlaneRgnLayout[i]->addWidget(new QLabel("Z-max"), 6, 0, 1, 5);
		cutPlaneRgnLayout[i]->addWidget(zSmaxSlider, 6, 6, 1, 15);

	}break;
	}

	//========================================================================
	// cut-planes tab box

	tabCutPlane = new QTabWidget(); //new AutoTabWidget(); //commented by PHC, 090117
	tabCutPlane->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//	i= tabCutPlane->insertTab(1, cutPlaneGroup[1], "Volume");    // qDebug("insertTab(1) = %d", i);
//  i= tabCutPlane->insertTab(2, cutPlaneGroup[2], "X-section"); // qDebug("insertTab(2) = %d", i);
//  i= tabCutPlane->insertTab(3, cutPlaneGroup[3], "Surface");   // qDebug("insertTab(3) = %d", i);
	QWidget * stackedVolPage = new QWidget;
	stackedCutPlane = new QStackedLayout(stackedVolPage);
	i= stackedCutPlane->addWidget(cutPlaneGroup[1]);
	i= stackedCutPlane->addWidget(cutPlaneGroup[2]);
	i= tabCutPlane->addTab(stackedVolPage, "Volume Cut");
    i= tabCutPlane->addTab(cutPlaneGroup[3], "Surface Cut");
	tabCutPlane->setTabToolTip(0, "Volume Cut");
    tabCutPlane->setTabToolTip(1, "Surface Cut");

    //========================================================================



/*	////////////////////////////////////////////////////////////
	//movie rotation

	xstep=2; ystep=0; zstep=0; nsteps_rot_movie=180;

	QGroupBox *movieCtrlGroup = new QGroupBox(btnGroup);
    movieCtrlGroup->setTitle("Movie");
    QGridLayout *movieCtrlLayout = new QGridLayout(movieCtrlGroup);

	xRotBox = new QSpinBox;
    xRotBox->setRange(-90, 90);
    xRotBox->setSingleStep(1);
    xRotBox->setValue(xstep);
	QLabel *xRotBoxLabel = new QLabel("X Step", movieCtrlGroup);

	yRotBox = new QSpinBox;
    yRotBox->setRange(-90, 90);
    yRotBox->setSingleStep(1);
    yRotBox->setValue(ystep);
	QLabel *yRotBoxLabel = new QLabel("Y Step", movieCtrlGroup);

	zRotBox = new QSpinBox;
    zRotBox->setRange(-90, 90);
    zRotBox->setSingleStep(1);
    zRotBox->setValue(zstep);
	QLabel *zRotBoxLabel = new QLabel("Z Step", movieCtrlGroup);

	nStepBox = new QSpinBox;
    nStepBox->setRange(1, 500);
    nStepBox->setSingleStep(1);
    nStepBox->setValue(nsteps_rot_movie);
	QLabel *nStepBoxLabel = new QLabel("# Steps", movieCtrlGroup);

    moviePreViewButton = new QPushButton(movieCtrlGroup);
    moviePreViewButton->setText("Preview Movie");

    movieSaveButton = new QPushButton(movieCtrlGroup);
    movieSaveButton->setText("Save Movie Now...");

    movieCtrlLayout->addWidget(xRotBoxLabel, 0, 0, 1, 4);
    movieCtrlLayout->addWidget(xRotBox, 0, 5, 1, 9);

    movieCtrlLayout->addWidget(yRotBoxLabel, 1, 0, 1, 4);
    movieCtrlLayout->addWidget(yRotBox, 1, 5, 1, 9);

    movieCtrlLayout->addWidget(zRotBoxLabel, 2, 0, 1, 4);
    movieCtrlLayout->addWidget(zRotBox, 2, 5, 1, 9);

    movieCtrlLayout->addWidget(nStepBoxLabel, 3, 0, 1, 4);
    movieCtrlLayout->addWidget(nStepBox, 3, 5, 1, 9);

    movieCtrlLayout->addWidget(moviePreViewButton, 4, 0, 1, 13);

    movieCtrlLayout->addWidget(movieSaveButton, 5, 0, 1, 13);

	connect(xRotBox, SIGNAL(valueChanged(int)), this, SLOT(setXRotStep(int)));
	connect(yRotBox, SIGNAL(valueChanged(int)), this, SLOT(setYRotStep(int)));
	connect(zRotBox, SIGNAL(valueChanged(int)), this, SLOT(setZRotStep(int)));
	connect(nStepBox, SIGNAL(valueChanged(int)), this, SLOT(setNSteps(int)));
*/


#define __manage_layout_and_call_connect_signal__
	//===========================================================================================
    //Managing the layout

    QGroupBox* viewGroup = new QGroupBox();
    //QFrame *viewGroup = new QFrame;  viewGroup->setFrameStyle(QFrame::StyledPanel);

    glWidgetArea = new QScrollArea;
    glWidgetArea->setWidgetResizable(true);
    glWidgetArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    glWidgetArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    glWidgetArea->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding); //QSizePolicy::Ignored, QSizePolicy::Ignored);
    glWidgetArea->setMinimumSize(MINVIEW_SIZEX, MINVIEW_SIZEY);
    if (glWidget)  	glWidgetArea->setWidget(glWidget);

    timeSlider = new QScrollBar(Qt::Horizontal);
    timeSlider->setRange(0,0);
    timeSlider->setSingleStep(1);
    timeSlider->setPageStep(10);

    QVBoxLayout *viewLayout = new QVBoxLayout(viewGroup);
    viewLayout->addWidget(glWidgetArea);
    viewLayout->addWidget(timeSlider);
    viewLayout->setContentsMargins(0,0,0,0);

    //------------------------------------------------

    controlGroup = new QGroupBox();
    //QFrame *controlGroup = new QFrame;  controlGroup->setFrameStyle(QFrame::StyledPanel);
    controlGroup->setFixedWidth(CTRL_SIZEX);
    controlGroup->setTitle("Controls");
    controlGroup->setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *controlLayout = new QVBoxLayout(controlGroup);
    controlLayout->addWidget(tabOptions); //mainDisplayOptGroup);
    controlLayout->addWidget(toolBtnGroup); //090712 RZC
    controlLayout->addWidget(tabCutPlane); //cutPlaneGroup);
    controlLayout->addWidget(tabRotZoom); //rotateBarGroup);
    controlLayout->addStretch(0);
    controlLayout->setSpacing(0);
    //------------------------------------------------

    // @ADDED by Alessandro on 2015-05-07 : hide/display controls.
    hideDisplayControlsButton = new QPushButton("");
    hideDisplayControlsButton->setIcon(QIcon(":/pic/arrow_right.png"));
    QFont smallFont;
    smallFont.setPointSize(10);
    hideDisplayControlsButton->setFont(smallFont);
    hideDisplayControlsButton->setContentsMargins(0,0,0,0);
    hideDisplayControlsButton->setMaximumWidth(25);
    hideDisplayControlsButton->setFixedHeight(100);
    connect(hideDisplayControlsButton, SIGNAL(clicked()), this, SLOT(hideDisplayControls()));
    //hideDisplayControlsButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    centralLayout = new QHBoxLayout(this);
    centralLayout->addWidget(viewGroup, 1);
    centralLayout->addWidget(hideDisplayControlsButton, 0);
    centralLayout->addWidget(controlGroup);
    //centralLayout->setContentsMargins(10,10,10,10);
    setLayout(centralLayout);

	////////////////////////////////////////////////////////////////////////////////////
    connectSignal();
    ////////////////////////////////////////////////////////////////////////////////////

    // after connectSignal then set widget's initial values, by RZC 080821
    // 090503 RZC: deferred until glWigdet->initialGL for correct settings, because the renderer is created dynamically
    // initControlValue();


#define __tip_filter_and_button_menu__
	//===========================================================================================

	SliderTipFilter *TsliderTip = new SliderTipFilter(this, "time = ", "", 0);
	timeSlider->installEventFilter(TsliderTip);

	//SliderTipFilter *VsliderTip = new SliderTipFilter(this, "", "", 1);
	SliderTipFilter *xVsliderTip = new SliderTipFilter(this, "", "", 1, xCut_altTip);
	SliderTipFilter *yVsliderTip = new SliderTipFilter(this, "", "", 1, yCut_altTip);
	SliderTipFilter *zVsliderTip = new SliderTipFilter(this, "", "", 1, zCut_altTip);

	xcminSlider->installEventFilter(xVsliderTip);
	xcmaxSlider->installEventFilter(xVsliderTip);
	ycminSlider->installEventFilter(yVsliderTip);
	ycmaxSlider->installEventFilter(yVsliderTip);
	zcminSlider->installEventFilter(zVsliderTip);
	zcmaxSlider->installEventFilter(zVsliderTip);

	xCSSlider->installEventFilter(xVsliderTip);
	yCSSlider->installEventFilter(yVsliderTip);
	zCSSlider->installEventFilter(zVsliderTip);

	SliderTipFilter *CsliderTip = new SliderTipFilter(this, "", "%", 0);
	transparentSlider->installEventFilter(CsliderTip);

	SliderTipFilter *SsliderTip = new SliderTipFilter(this, "", "%", -100);
	fcutSlider->installEventFilter(SsliderTip);
	fCSSlider->installEventFilter(SsliderTip);
	xSminSlider->installEventFilter(SsliderTip);
	xSmaxSlider->installEventFilter(SsliderTip);
	ySminSlider->installEventFilter(SsliderTip);
	ySmaxSlider->installEventFilter(SsliderTip);
	zSminSlider->installEventFilter(SsliderTip);
	zSmaxSlider->installEventFilter(SsliderTip);


	createMenuOfAnimate();  // Animation menu
	createMenuOfSurfFile(); // Load/Save Surface menu
}

// @ADDED by Alessandro on 2015-05-07 : hide/display controls.
void V3dR_MainWindow::hideDisplayControls()
{
    if(!displayControlsHidden)
    {
        controlGroup->setVisible(false);
        hideDisplayControlsButton->setIcon(QIcon(":/pic/arrow_left.png"));
        displayControlsHidden = true;
    }
    else
    {
        controlGroup->setVisible(true);
        hideDisplayControlsButton->setIcon(QIcon(":/pic/arrow_right.png"));
        displayControlsHidden = false;
    }
}

void V3dR_MainWindow::connectSignal()
{
	qDebug("V3dR_MainWindow::connectSignal with V3dR_GLWidget");
	if (!glWidget)	return;


#define __connect_volume__
	//volume display option box ///////////////////////////////////

	if (timeSlider) {
		connect(glWidget, SIGNAL(changeVolumeTimePoint(int)), timeSlider, SLOT(setValue(int)));
		connect(timeSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setVolumeTimePoint(int)));
	}

	if (dispType_maxip) {
		connect(glWidget, SIGNAL(changeDispType_maxip(bool)), dispType_maxip, SLOT(setChecked(bool)));
		connect(dispType_maxip, SIGNAL(toggled(bool)), glWidget, SLOT(setRenderMode_Maxip(bool)));
	}
	
	if (dispType_minip) {
		connect(glWidget, SIGNAL(changeDispType_minip(bool)), dispType_minip, SLOT(setChecked(bool)));
		connect(dispType_minip, SIGNAL(toggled(bool)), glWidget, SLOT(setRenderMode_Minip(bool)));
	}
	
	if (dispType_alpha) {
		connect(glWidget, SIGNAL(changeDispType_alpha(bool)), dispType_alpha, SLOT(setChecked(bool)));
		connect(dispType_alpha, SIGNAL(toggled(bool)), glWidget, SLOT(setRenderMode_Alpha(bool)));
	}
	if (dispType_cs3d) {
		connect(glWidget, SIGNAL(changeDispType_cs3d(bool)), dispType_cs3d, SLOT(setChecked(bool)));
		connect(dispType_cs3d, SIGNAL(toggled(bool)), glWidget, SLOT(setRenderMode_Cs3d(bool)));
	}

	if (transparentSlider) {
		connect(glWidget, SIGNAL(changeTransparentSliderLabel(const QString&)), transparentSlider_Label, SLOT(setText(const QString&)));
		connect(transparentSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setCSTransparent(int)));
		//connect(glWidget, SIGNAL(enableTransparentSlider(bool)), transparentSlider, SLOT(setEnabled(bool)));
	}
	if (thicknessSlider) {
		connect(thicknessSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setThickness(int)));
		//connect(glWidget, SIGNAL(enableThicknessSlider(bool)), thicknessSlider_Label, SLOT(setEnabled(bool)));
		//connect(glWidget, SIGNAL(enableThicknessSlider(bool)), thicknessSlider, SLOT(setEnabled(bool)));
	}
	if (comboBox_channel) {
		connect(comboBox_channel, SIGNAL(currentIndexChanged(int)), glWidget, SLOT(setCurChannel(int)));
	}
	if (zthicknessBox) {
		connect(zthicknessBox, SIGNAL(valueChanged(double)), glWidget, SLOT(setThickness(double)));
	}
	if (checkBox_channelR) {
		connect(checkBox_channelR, SIGNAL(toggled(bool)), glWidget, SLOT(setChannelR(bool)));
	}
	if (checkBox_channelG) {
		connect(checkBox_channelG, SIGNAL(toggled(bool)), glWidget, SLOT(setChannelG(bool)));
	}
	if (checkBox_channelB) {
		connect(checkBox_channelB, SIGNAL(toggled(bool)), glWidget, SLOT(setChannelB(bool)));
	}

	if (checkBox_volCompress){
		connect(checkBox_volCompress, SIGNAL(toggled(bool)), glWidget, SLOT(setVolCompress(bool)));
		connect(glWidget, SIGNAL(changeVolCompress(bool)), checkBox_volCompress, SLOT(setChecked(bool)));
		connect(glWidget, SIGNAL(changeEnableVolCompress(bool)), checkBox_volCompress, SLOT(setEnabled(bool)));
		checkBox_volCompress->setEnabled(false);
	}
	if (volumeColormapButton)
	{
		connect(volumeColormapButton, SIGNAL(clicked()), glWidget, SLOT(volumeColormapDialog()));
		connect(glWidget, SIGNAL(changeEnableVolColormap(bool)), volumeColormapButton, SLOT(setEnabled(bool)));
		volumeColormapButton->setEnabled(false);
	}

#define __connect_surface__
	//surface display option box ///////////////////////////////////////

	if (checkBox_displayMarkers)
		connect(checkBox_displayMarkers, SIGNAL(stateChanged(int)), glWidget, SLOT(setShowMarkers(int)));
	if (checkBox_displaySurf)
		connect(checkBox_displaySurf, SIGNAL(stateChanged(int)), glWidget, SLOT(setShowSurfObjects(int)));
	if (updateLandmarkButton)
		connect(updateLandmarkButton, SIGNAL(clicked()), glWidget, SLOT(updateWithTriView()));

	if (loadSaveObjectsButton)
	{
		connect(loadSaveObjectsButton, SIGNAL(clicked()), this, SLOT(doMenuOfSurfFile()));
	}

	if (checkBox_markerLabel)
	{
		connect(checkBox_markerLabel, SIGNAL(toggled(bool)), glWidget, SLOT(enableMarkerLabel(bool)));
	}
	if (spinBox_markerSize)
	{
		connect(spinBox_markerSize, SIGNAL(valueChanged(int)), glWidget, SLOT(setMarkerSize(int)));
		connect(glWidget, SIGNAL(changeMarkerSize(int)), spinBox_markerSize, SLOT(setValue(int)));
	}
	if (checkBox_surfStretch)
	{
		connect(checkBox_surfStretch, SIGNAL(toggled(bool)), glWidget, SLOT(enableSurfStretch(bool)));
	}

	if (surfobjManagerButton)
	{
		connect(surfobjManagerButton, SIGNAL(clicked()), glWidget, SLOT(surfaceSelectDialog()));
		//surfobjManagerButton->setEnabled(false);
	}

#define __connect_rotation__
	//rotation group //////////////////////////////////////////////////

	if (xRotSlider) {
		connect(xRotSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setXRotation(int)));
		connect(glWidget, SIGNAL(xRotationChanged(int)), xRotSlider, SLOT(setValue(int)));
		if (xRotBox) {
			connect(xRotSlider, SIGNAL(valueChanged(int)), xRotBox, SLOT(setValue(int)));
			connect(xRotBox, SIGNAL(valueChanged(int)), glWidget, SLOT(setXRotation(int)));
		}
	}
	if (yRotSlider) {
		connect(yRotSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setYRotation(int)));
		connect(glWidget, SIGNAL(yRotationChanged(int)), yRotSlider, SLOT(setValue(int)));
		if (yRotBox) {
			connect(yRotSlider, SIGNAL(valueChanged(int)), yRotBox, SLOT(setValue(int)));
			connect(yRotBox, SIGNAL(valueChanged(int)), glWidget, SLOT(setYRotation(int)));
		}
	}
	if (zRotSlider) {
		connect(zRotSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setZRotation(int)));
		connect(glWidget, SIGNAL(zRotationChanged(int)), zRotSlider, SLOT(setValue(int)));
		if (zRotBox) {
			connect(zRotSlider, SIGNAL(valueChanged(int)), zRotBox, SLOT(setValue(int)));
			connect(zRotBox, SIGNAL(valueChanged(int)), glWidget, SLOT(setZRotation(int)));
		}
	}
//	if (checkBox_absoluteRot){
//		connect(checkBox_absoluteRot, SIGNAL(toggled(bool)), glWidget, SLOT(enableAbsoluteRot(bool)));
//	}
//	if (rotAbsolute){
//		connect(rotAbsolute, SIGNAL(clicked()), glWidget, SLOT(doAbsoluteRot()));
//	}
	if (rotAbsolutePose){
		connect(rotAbsolutePose, SIGNAL(clicked()), glWidget, SLOT(absoluteRotPose()));
	}
	if(rotVRView){
		connect(rotVRView, SIGNAL(clicked()), glWidget, SLOT(absoluteVRview()));
	}

#define __connnect_zoom_shift__
	// zoom & shift group

	if (zoomSlider) {
		connect(zoomSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setZoom(int)));
		connect(glWidget, SIGNAL(zoomChanged(int)), zoomSlider, SLOT(setValue(int)));
		if (zoomBox) {
			connect(zoomSlider, SIGNAL(valueChanged(int)), zoomBox, SLOT(setValue(int)));
			connect(zoomBox, SIGNAL(valueChanged(int)), zoomSlider, SLOT(setValue(int)));
		}
	}
	if (xShiftSlider) {
		connect(xShiftSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setXShift(int)));
		connect(glWidget, SIGNAL(xShiftChanged(int)), xShiftSlider, SLOT(setValue(int)));
		if (xShiftBox) {
			connect(xShiftSlider, SIGNAL(valueChanged(int)), xShiftBox, SLOT(setValue(int)));
			connect(xShiftBox, SIGNAL(valueChanged(int)), xShiftSlider, SLOT(setValue(int)));
		}
	}
	if (yShiftSlider) {
		connect(yShiftSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setYShift(int)));
		connect(glWidget, SIGNAL(yShiftChanged(int)), yShiftSlider, SLOT(setValue(int)));
		if (yShiftBox) {
			connect(yShiftSlider, SIGNAL(valueChanged(int)), yShiftBox, SLOT(setValue(int)));
			connect(yShiftBox, SIGNAL(valueChanged(int)), yShiftSlider, SLOT(setValue(int)));
		}
	}

	if (rotReset)
	{
		connect(rotReset, SIGNAL(clicked()), glWidget, SLOT(resetRotation()));
	}
	if (zoomReset)
	{
		connect(zoomReset, SIGNAL(clicked()), glWidget, SLOT(resetZoomShift()));
	}

#define __connect_cut_slice__
	// cut-plane group

	if (xcminSlider) {
		connect(glWidget, SIGNAL(changeXCut0(int)), xcminSlider, SLOT(setValue(int)));
		connect(xcminSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setXCut0(int)));
	}
	if (xcmaxSlider) {
		connect(glWidget, SIGNAL(changeXCut1(int)), xcmaxSlider, SLOT(setValue(int)));
		connect(xcmaxSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setXCut1(int)));
	}
	if (ycminSlider) {
		connect(glWidget, SIGNAL(changeYCut0(int)), ycminSlider, SLOT(setValue(int)));
		connect(ycminSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setYCut0(int)));
	}
	if (ycmaxSlider) {
		connect(glWidget, SIGNAL(changeYCut1(int)), ycmaxSlider, SLOT(setValue(int)));
		connect(ycmaxSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setYCut1(int)));
	}
	if (zcminSlider) {
		connect(glWidget, SIGNAL(changeZCut0(int)), zcminSlider, SLOT(setValue(int)));
		connect(zcminSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setZCut0(int)));
	}
	if (zcmaxSlider) {
		connect(glWidget, SIGNAL(changeZCut1(int)), zcmaxSlider, SLOT(setValue(int)));
		connect(zcmaxSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setZCut1(int)));
	}

	if (xcLock) {
		connect(xcLock, SIGNAL(toggled(bool)), glWidget, SLOT(setXCutLock(bool)));
		connect(xcLock, SIGNAL(toggled(bool)), this, SLOT(setXCutLockIcon(bool))); setXCutLockIcon(false);
	}
	if (ycLock) {
		connect(ycLock, SIGNAL(toggled(bool)), glWidget, SLOT(setYCutLock(bool)));
		connect(ycLock, SIGNAL(toggled(bool)), this, SLOT(setYCutLockIcon(bool))); setYCutLockIcon(false);
	}
	if (zcLock) {
		connect(zcLock, SIGNAL(toggled(bool)), glWidget, SLOT(setZCutLock(bool)));
		connect(zcLock, SIGNAL(toggled(bool)), this, SLOT(setZCutLockIcon(bool))); setZCutLockIcon(false);
	}

	///////////////////////////////
	if (fcutSlider) {
		connect(glWidget, SIGNAL(changeFrontCut(int)), fcutSlider, SLOT(setValue(int)));
		connect(fcutSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setFrontCut(int)));
	}
	if (fCSSlider) {
		connect(glWidget, SIGNAL(changeFrontCut(int)), fCSSlider, SLOT(setValue(int)));
		connect(fCSSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setFrontCut(int)));
	}

	if (checkBox_fCS)
		connect(checkBox_fCS, SIGNAL(toggled(bool)), glWidget, SLOT(enableFrontSlice(bool)));
	if (checkBox_xCS)
		connect(checkBox_xCS, SIGNAL(toggled(bool)), glWidget, SLOT(enableXSlice(bool)));
	if (checkBox_yCS)
		connect(checkBox_yCS, SIGNAL(toggled(bool)), glWidget, SLOT(enableYSlice(bool)));
	if (checkBox_zCS)
		connect(checkBox_zCS, SIGNAL(toggled(bool)), glWidget, SLOT(enableZSlice(bool)));

	///////////////////////////////
	if (xCSSlider) {
		connect(glWidget, SIGNAL(changeXCSSlider(int)), xCSSlider, SLOT(setValue(int)));
		connect(xCSSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setXCS(int)));
	}
	if (yCSSlider) {
		connect(glWidget, SIGNAL(changeYCSSlider(int)), yCSSlider, SLOT(setValue(int)));
		connect(yCSSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setYCS(int)));
	}
	if (zCSSlider) {
		connect(glWidget, SIGNAL(changeZCSSlider(int)), zCSSlider, SLOT(setValue(int)));
		connect(zCSSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setZCS(int)));
	}

	///////////////////////////////
	if (xSminSlider) {
		connect(glWidget, SIGNAL(changeXClip0(int)), xSminSlider, SLOT(setValue(int)));
		connect(xSminSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setXClip0(int)));
	}
	if (xSmaxSlider) {
		connect(glWidget, SIGNAL(changeXClip1(int)), xSmaxSlider, SLOT(setValue(int)));
		connect(xSmaxSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setXClip1(int)));
	}
	if (ySminSlider) {
		connect(glWidget, SIGNAL(changeYClip0(int)), ySminSlider, SLOT(setValue(int)));
		connect(ySminSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setYClip0(int)));
	}
	if (ySmaxSlider) {
		connect(glWidget, SIGNAL(changeYClip1(int)), ySmaxSlider, SLOT(setValue(int)));
		connect(ySmaxSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setYClip1(int)));
	}
	if (zSminSlider) {
		connect(glWidget, SIGNAL(changeZClip0(int)), zSminSlider, SLOT(setValue(int)));
		connect(zSminSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setZClip0(int)));
	}
	if (zSmaxSlider) {
		connect(glWidget, SIGNAL(changeZClip1(int)), zSmaxSlider, SLOT(setValue(int)));
		connect(zSmaxSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setZClip1(int)));
	}

//	if (tabCutPlane) {
//		connect(glWidget, SIGNAL(switchTabCutPlane(int)), tabCutPlane, SLOT(setCurrentIndex(int)));
//		connect(glWidget, SIGNAL(enableTabCutPlane(int,bool)), tabCutPlane, SLOT(setTabEnabled(int, bool)));
//	}
	if (stackedCutPlane) {
		connect(glWidget, SIGNAL(changeCurrentTabCutPlane(int)), stackedCutPlane, SLOT(setCurrentIndex(int)));
//		connect(glWidget, SIGNAL(enableTabCutPlane(int,bool)), stackedCutPlane, SLOT(setTabEnabled(int, bool)));
	}

#define __connect_others__
	//others display option box

	if (checkBox_displayAxes)
		connect(checkBox_displayAxes, SIGNAL(toggled(bool)), glWidget, SLOT(enableShowAxes(bool)));
	if (checkBox_displayBoundingBox)
		connect(checkBox_displayBoundingBox, SIGNAL(toggled(bool)), glWidget, SLOT(enableShowBoundingBox(bool)));
	if (checkBox_OrthoView)
	{
		connect(checkBox_OrthoView, SIGNAL(toggled(bool)), glWidget, SLOT(enableOrthoView(bool)));
		connect(glWidget, SIGNAL(changeOrthoView(bool)), checkBox_OrthoView, SLOT(setChecked(bool)));
	}

	if (colorButton)
		connect(colorButton, SIGNAL(clicked()), glWidget, SLOT(setBackgroundColor()));
	if (brightButton)
		connect(brightButton, SIGNAL(clicked()), glWidget, SLOT(setBright()));
	if (reloadDataButton)
		connect(reloadDataButton, SIGNAL(clicked()), glWidget, SLOT(reloadData()));

	if (movieSaveButton)
		connect(movieSaveButton, SIGNAL(clicked()), this, SLOT(saveMovie()));

	if (animateButton)
	{
		connect(animateButton, SIGNAL(clicked()), this, SLOT(doMenuOfAnimate()));
	}


	connect(glWidget, SIGNAL(signalVolumeCutRange()), this, SLOT(initVolumeCutRange())); // 081122
	connect(glWidget, SIGNAL(signalInitControlValue()), this, SLOT(initControlValue())); // 081122
	connect(glWidget, SIGNAL(signalOnlySurfaceObj()), this, SLOT(onlySurfaceObjTab()));  // 110809

	return;
}


void V3dR_MainWindow::initControlValue()
{
	qDebug("V3dR_MainWindow::initControlValue");


	// cut tab
	//////////////////////////////////////////////////////////////
	initVolumeTimeRange(); /// dynamic change volume time point, 090801
	initVolumeCutRange();  /// dynamic change volume cut range, 081122
    initSurfaceCutRange();

	if (checkBox_fCS)	{
		checkBox_fCS->setChecked(true); //100729 default turn on
	}
	if (checkBox_xCS)	{
		checkBox_xCS->setChecked(true);
	}
	if (checkBox_yCS)	{
		checkBox_yCS->setChecked(true);
	}
	if (checkBox_zCS)	{
		checkBox_zCS->setChecked(true);
	}

    if (xRotSlider) xRotSlider->setValue(ANGLE_X0 * ANGLE_TICK);
    if (yRotSlider) yRotSlider->setValue(ANGLE_Y0 * ANGLE_TICK);
    if (zRotSlider) zRotSlider->setValue(ANGLE_Z0 * ANGLE_TICK);


	// volume tab
	///////////////////////////////////////////////////////////
	if (dispType_maxip) dispType_maxip->setChecked(true);
	if (transparentSlider) transparentSlider->setValue(0); //10. max=100
	//if (thicknessSlider) thicknessSlider->setValue(1);    //1, max=10
	if (zthicknessBox)
	{
		float curthickness=1.0;
#ifndef test_main_cpp
		if (_idep && _idep->image4d)
		{
			float tmpthickness = 2.0*_idep->image4d->getRezZ()/(_idep->image4d->getRezX()+_idep->image4d->getRezY());
			curthickness = CLAMP(1, ZTHICK_RANGE, tmpthickness);
			//v3d_msg(QString("the clamped default z thickness is %1\n").arg(curthickness),0);
		}
#endif
		zthicknessBox->setValue(curthickness); //1
		//v3d_msg(QString("set the default 3d viewer widget zthickness %1\n").arg(zthicknessBox->value()), 0);
	}

	if (checkBox_channelR)	{
		checkBox_channelR->setChecked(true);
	}
	if (checkBox_channelG)	{
		checkBox_channelG->setChecked(true);
	}
	if (checkBox_channelB)	{
		checkBox_channelB->setChecked(true);
	}
	if (checkBox_volCompress && glWidget)	{
		checkBox_volCompress->setChecked(glWidget->isVolCompress());
	}


	// surf/obj tab
	////////////////////////////////////////////////////////////////////////////////////
	if (checkBox_displayMarkers) {
		//checkBox_displayMarkers->setCheckState(Qt::PartiallyChecked);
		checkBox_displayMarkers->setCheckState(Qt::Checked);
		//checkBox_displayMarkers->setCheckState(Qt::Unchecked);
	}
	if (checkBox_displaySurf) {
		//checkBox_displaySurf->setCheckState(Qt::PartiallyChecked);
		checkBox_displaySurf->setCheckState(Qt::Checked);
		//checkBox_displaySurf->setCheckState(Qt::Unchecked);
	}
    if (checkBox_markerLabel) {
    	checkBox_markerLabel->setChecked(true);
    }
    if (checkBox_surfStretch) {
    	checkBox_surfStretch->setChecked(true);
    }

    if (spinBox_markerSize)
	{
#ifndef test_main_cpp
		if (_idep && _idep->V3Dmainwindow)
			spinBox_markerSize->setValue(_idep->V3Dmainwindow->global_setting.marker_disp_amplifying_factor);
		else
#endif
			spinBox_markerSize->setValue(25);
   	}


	//others tab
    ///////////////////////////////////////////////////////////////////////////////
	if (checkBox_displayAxes && glWidget)	{
		checkBox_displayAxes->setChecked(glWidget->isShowAxes());
	}
	if (checkBox_displayBoundingBox && glWidget)	{
		checkBox_displayBoundingBox->setChecked(glWidget->isShowBoundingBox());
	}
	if (checkBox_OrthoView)	{
		checkBox_OrthoView->setChecked(false);
		//emit checkBox_OrthoView->toggled(false); //it's protected member, must use self-defined signal
	}

	//////////////////////////////////////////////////////////////////////////
	// updateGL here immediately to prevent a delay flash due to following Renderer::init when using POST_updateGL, by RZC 080927
	if (glWidget) glWidget->update();  // 081122, CAUTION: call updateGL from initializeGL will cause infinite loop call
	//////////////////////////////////////////////////////////////////////////
}

void V3dR_MainWindow::setXCutLockIcon(bool b)
{
	if (! xcLock)  return;
	if (b)
		xcLock->setIcon(QIcon(":/pic/Lockon.png"));
	else
		xcLock->setIcon(QIcon(":/pic/Lockoff.png"));
}
void V3dR_MainWindow::setYCutLockIcon(bool b)
{
	if (! ycLock)  return;
	if (b)
		ycLock->setIcon(QIcon(":/pic/Lockon.png"));
	else
		ycLock->setIcon(QIcon(":/pic/Lockoff.png"));
}
void V3dR_MainWindow::setZCutLockIcon(bool b)
{
	if (! zcLock)  return;
	if (b)
		zcLock->setIcon(QIcon(":/pic/Lockon.png"));
	else
		zcLock->setIcon(QIcon(":/pic/Lockoff.png"));
}


void V3dR_MainWindow::initVolumeTimeRange()
{
	if (! glWidget) return;

	int t;
	t = MAX(0, glWidget->dataDim5()-1);

	if (timeSlider) {
		timeSlider->setMaximum(t);
		timeSlider->setValue(0);
	}
}

void V3dR_MainWindow::initVolumeCutRange()
{
	if (! glWidget) return;

	int d1, d2, d3;
	d1 = MAX(0, glWidget->dataDim1()-1);
	d2 = MAX(0, glWidget->dataDim2()-1);
	d3 = MAX(0, glWidget->dataDim3()-1);

	if (comboBox_channel) {  //100802 //110811 combo-box from spin-box
		int dc = CLAMP(0,3, glWidget->dataDim4());
		comboBox_channel->setEnabled(dc>0); //090922
		comboBox_channel->clear();
		comboBox_channel->addItem("all (only for markers)"); //index=0
		for (int i=1; i<=dc; i++)
		{
			comboBox_channel->addItem(QString("%1").arg(i));
		}
		comboBox_channel->setCurrentIndex(0);
	}

	if (fcutSlider) {
		fcutSlider->setEnabled(glWidget->dataDim4()>0); //090922
	}
	if (fCSSlider) {
		fCSSlider->setEnabled(glWidget->dataDim4()>0); //090922
	}

	if (xcminSlider) {
		xcminSlider->setMaximum(d1);
		xcminSlider->setValue(xcminSlider->minimum());
	}
	if (xcmaxSlider) {
		xcmaxSlider->setMaximum(d1);
		xcmaxSlider->setValue(xcmaxSlider->maximum());
	}
	if (ycminSlider) {
		ycminSlider->setMaximum(d2);
		ycminSlider->setValue(ycminSlider->minimum());
	}
	if (ycmaxSlider) {
		ycmaxSlider->setMaximum(d2);
		ycmaxSlider->setValue(ycmaxSlider->maximum());
	}
	if (zcminSlider) {
		zcminSlider->setMaximum(d3);
		zcminSlider->setValue(zcminSlider->minimum());
	}
	if (zcmaxSlider) {
		zcmaxSlider->setMaximum(d3);
		zcmaxSlider->setValue(zcmaxSlider->maximum());
	}

	if (xCSSlider) {
		xCSSlider->setMaximum(d1);
		xCSSlider->setValue((xCSSlider->minimum()+xCSSlider->maximum())/2);
	}
	if (yCSSlider) {
		yCSSlider->setMaximum(d2);
		yCSSlider->setValue((yCSSlider->minimum()+yCSSlider->maximum())/2);
	}
	if (zCSSlider) {
		zCSSlider->setMaximum(d3);
		zCSSlider->setValue((zCSSlider->minimum()+zCSSlider->maximum())/2);
	}
}

void V3dR_MainWindow::initSurfaceCutRange()
{
	// set surface cut range
	if (xSminSlider) xSminSlider->setValue(xSminSlider->minimum());
	if (xSmaxSlider) xSmaxSlider->setValue(xSmaxSlider->maximum());
	if (ySminSlider) ySminSlider->setValue(ySminSlider->minimum());
	if (ySmaxSlider) ySmaxSlider->setValue(ySmaxSlider->maximum());
	if (zSminSlider) zSminSlider->setValue(zSminSlider->minimum());
	if (zSmaxSlider) zSmaxSlider->setValue(zSmaxSlider->maximum());
}

void V3dR_MainWindow::createMenuOfAnimate()
{
    QAction* Act;

    Act = new QAction(tr("&Animation On"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(animateOn()));
    menuAnimate.addAction(Act);

    Act = new QAction(tr("&Animation Off"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(animateOff()));
    menuAnimate.addAction(Act);

    Act = new QAction(tr("&Set Rotation Type"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(setAnimateRotType()));
    menuAnimate.addAction(Act);

    Act = new QAction(tr("&Set Rotation Speed"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(setAnimateRotSpeedSec()));
    menuAnimate.addAction(Act);

    Act = new QAction(tr("&Set Time-points per Rotation"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(setAnimateRotTimePoints()));
    menuAnimate.addAction(Act);
}

void V3dR_MainWindow::doMenuOfAnimate()
{
	try //080927
	{
		menuAnimate.exec(QCursor::pos());
	}
	catch (...)
	{
		printf("Fail to run the V3dR_MainWindow::doMenuOfAnimate() function.\n");
	}
}


static const char* i2strRGB[] = //{"R", "G", "B"};
					{"1", "2", "3"}; //081230

void V3dR_MainWindow::createMenuOfSurfFile()
{
    QAction* Act;
    if (! glWidget)
	{
		printf("Fail to run the V3dR_MainWindow::createMenuOfSurfFile() function.\n");
		return;
	}

    Act = new QAction(tr("&Load/Create Surface From File"), this);
    connect(Act, SIGNAL(triggered()), glWidget, SLOT(loadObjectFromFile()));
    menuSurfFile.addAction(Act);

    QAction* ActRGB[3];
    ActRGB[0] = Act = new QAction(tr("&Create Surface From Associated Image Channel %1").arg(i2strRGB[0]), this);
    connect(Act, SIGNAL(triggered()), glWidget, SLOT(createSurfCurrentR()));
    menuSurfFile.addAction(Act);

    ActRGB[1] = Act = new QAction(tr("&Create Surface From Associated Image Channel %1").arg(i2strRGB[1]), this);
    connect(Act, SIGNAL(triggered()), glWidget, SLOT(createSurfCurrentG()));
    menuSurfFile.addAction(Act);

    ActRGB[2] = Act = new QAction(tr("&Create Surface From Associated Image Channel %1").arg(i2strRGB[2]), this);
    connect(Act, SIGNAL(triggered()), glWidget, SLOT(createSurfCurrentB()));
    menuSurfFile.addAction(Act);

    Act = new QAction(tr("&Save Created Surface To File"), this);
    connect(Act, SIGNAL(triggered()), glWidget, SLOT(saveSurfFile()));
    menuSurfFile.addAction(Act);

#ifndef test_main_cpp
    if (_idep)
    {
		iDrawExternalParameter* ep = _idep;
		My4DImage* image4d = (My4DImage*) ep->image4d;
		int ch_num = 0;
		if (image4d && (image4d->valid()))
		{
			ch_num = image4d->getCDim();
		}
		for (int i=0; i<3; i++)
		{
			ActRGB[i]->setEnabled( i<ch_num );

		}
    }
#endif
}

void V3dR_MainWindow::doMenuOfSurfFile()
{
	try //080927
	{
		menuSurfFile.exec(QCursor::pos());
	}
	catch (...)
	{
		printf("Fail to run the V3dR_MainWindow::doMenuOfSurfFile() function.\n");
	}
}


QAbstractSlider *V3dR_MainWindow::createCutPlaneSlider(int maxval, Qt::Orientation hv)
{
//    QSlider *slider = new QSlider(hv);
//    slider->setRange(0, maxval);
//    slider->setSingleStep(1);
//    slider->setPageStep(10);
//    slider->setTickInterval(10);
//    slider->setTickPosition(QSlider::TicksRight);
    QScrollBar *slider = new QScrollBar(hv);
    slider->setRange(0, maxval);
    slider->setSingleStep(1);
    slider->setPageStep(10);

    //slider->setValue(0);
    return slider;
}

QAbstractSlider *V3dR_MainWindow::createRotateSlider(Qt::Orientation hv)
{
//    QSlider *slider = new QSlider(hv); //060903
//    slider->setRange(0, 360 * ANGLE_TICK);
//    slider->setSingleStep(1 * ANGLE_TICK);
//    slider->setPageStep(10 * ANGLE_TICK);
//    slider->setTickInterval(10 * ANGLE_TICK);
//    slider->setTickPosition(QSlider::TicksRight);
    QDial *slider = new HighlightDial(); //081028 QDial, 081117 HighlightDail
	slider->setRange(0, 360 * ANGLE_TICK);
	slider->setSingleStep(1 * ANGLE_TICK);
	slider->setPageStep(10 * ANGLE_TICK);
	slider->setWrapping(true);
	slider->setNotchesVisible(true);

	//slider->setValue(0);
    return slider;
}
QSpinBox *V3dR_MainWindow::createRotateSpinBox()
{
	QSpinBox *box = new QSpinBox;
	box->setRange(0, 360);
	box->setSuffix("\xB0");
	return box;
}

QAbstractSlider *V3dR_MainWindow::createShiftSlider(Qt::Orientation hv)
{
    QSlider *slider = new QSlider(hv);
    slider->setRange(-SHIFT_RANGE, SHIFT_RANGE);
    slider->setSingleStep(1);
    slider->setPageStep(SHIFT_RANGE/10);
    slider->setTickInterval(SHIFT_RANGE/10);
    slider->setTickPosition(QSlider::TicksRight);

    //slider->setValue(0);
    return slider;
}
QSpinBox *V3dR_MainWindow::createShiftSpinBox()
{
	QSpinBox *box = new QSpinBox;
	box->setRange(-SHIFT_RANGE, SHIFT_RANGE);
	//box->setSuffix(" %o");
	return box;
}

QAbstractSlider *V3dR_MainWindow::createZoomSlider(Qt::Orientation hv) // near,far is keyword in DOS & Windows, by RZC 080912
{
	int znear=-ZOOM_RANGE;
	int zfar=ZOOM_RANGE;
    QSlider *slider = new QSlider(hv);
    slider->setRange(znear, zfar);
    slider->setSingleStep(1);
    slider->setPageStep((zfar-znear)/10);
    slider->setTickInterval((zfar-znear)/10);
    slider->setTickPosition(QSlider::TicksRight);

    //slider->setValue(0);
    return slider;
}
QSpinBox *V3dR_MainWindow::createZoomSpinBox()
{
	QSpinBox *box = new QSpinBox;
	box->setRange(-ZOOM_RANGE, ZOOM_RANGE);
	//box->setSuffix(" %o");
	return box;
}

QAbstractSlider *V3dR_MainWindow::createRangeSlider(int znear, int zfar, Qt::Orientation hv) // near,far is keyword in DOS & Windows, by RZC 080912
{
    QSlider *slider = new QSlider(hv);
    slider->setRange(znear, zfar);
    slider->setSingleStep(1);
    slider->setPageStep((zfar-znear)/10);
    slider->setTickInterval((zfar-znear)/10);
    slider->setTickPosition(QSlider::TicksRight);

    //slider->setValue(0);
    return slider;
}

QAbstractSlider *V3dR_MainWindow::createTranparentSlider(Qt::Orientation hv)
{
    QScrollBar *slider = new QScrollBar(hv);
    slider->setRange(0,TRANSPARENT_RANGE); ///// min=0
    slider->setSingleStep(1);
    slider->setPageStep(10);

    //slider->setValue(0);
    return slider;
}

QAbstractSlider *V3dR_MainWindow::createThicknessSlider(Qt::Orientation hv)
{
//    QSlider *slider = new QSlider(hv);
//    slider->setRange(1,ZTHICK_RANGE);
//    slider->setSingleStep(1);
//    slider->setPageStep(ZTHICK_RANGE/10);
//    slider->setTickInterval(ZTHICK_RANGE/10);
//    slider->setTickPosition(QSlider::TicksRight);
    QScrollBar *slider = new QScrollBar(hv);
    slider->setRange(1,ZTHICK_RANGE); ////////// min=1
    slider->setSingleStep(1);
    slider->setPageStep(1); //ZTHICK_RANGE/10);

    //slider->setValue(0);
    return slider;
}

QDoubleSpinBox *V3dR_MainWindow::createThicknessSpinBox()
{
	QDoubleSpinBox *box = new QDoubleSpinBox;
	box->setRange(1, ZTHICK_RANGE);
    box->setSingleStep(1.0);
    box->setValue(1.0);
	box->setPrefix("x");
	return box;
}

QSpinBox *V3dR_MainWindow::createMarkerSizeSpinBox()
{
	QSpinBox *box = new QSpinBox;
	box->setRange(1, 100);
    //box->setSingleStep(1.0);
    box->setValue(10);
	box->setPrefix("x");
	//box->setSuffix(" %o");
	box->setToolTip("Marker size is related to 1/1000 of bounding box's size");
	return box;
}

QComboBox *V3dR_MainWindow::createChannelComboBox()
{
	QComboBox *box = new QComboBox;
	//box->setRange(1, 3);
    //box->setSingleStep(1.0);
    //box->setValue(1);
	//box->setSuffix(" %o");
	return box;
}
