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
 * basic_view3d.h
 *
 *  Created on: Aug 11, 2010
 *      Author: ruanz and hanchuanp
 */
//last change: add vr support 20160615 by PHC

#ifndef BASIC_VIEW3D_H_
#define BASIC_VIEW3D_H_

class View3DControl
{
public:
    virtual ~View3DControl() {};

    // Expose surface cut getters, Dec.01,2013 by Zhi Zhou
    virtual int xClip0() const =0;
    virtual int xClip1() const =0;
    virtual int yClip0() const =0;
    virtual int yClip1() const =0;
    virtual int zClip0() const =0;
    virtual int zClip1() const =0;

    virtual int isShowSurfObjects() const = 0; //Nov. 27, 2013 by Zhi Zhou
	virtual void setState(int state, bool is_enable) = 0; // May 29, 2012 by Hang
	virtual void addStateFunc(int state, void(*mouse_func)(void *)) = 0; // May 29, 2012 by Hang
	virtual void deleteStateFunc(int state) = 0; // May 29, 2012 by Hang

	virtual int renderMode() const =0;
	virtual int dataDim1() const =0;
	virtual int dataDim2() const =0;
	virtual int dataDim3() const =0;
	virtual int dataDim4() const =0;
	virtual int dataDim5() const =0;

	virtual int xRot() const =0;
	virtual int yRot() const =0;
	virtual int zRot() const =0;
    virtual int zoom() const =0;
	virtual int xShift() const =0;
	virtual int yShift() const =0;
	virtual int zShift() const =0;
	virtual bool isAbsoluteRot() const =0;

    // Expose cut getters, 2011 Feb 09 CMB
    virtual int xCut0() const =0;
    virtual int xCut1() const =0;
    virtual int yCut0() const =0;
    virtual int yCut1() const =0;
    virtual int zCut0() const =0;
    virtual int zCut1() const =0;
    virtual int frontCut() const =0;
    virtual int xCS() const =0;
    virtual int yCS() const =0;
    virtual int zCS() const =0;
    virtual bool channelR() =0;
    virtual bool channelG() =0;
    virtual bool channelB() =0;
    // ...and lock setters.  Movie maker needs to turn off locks.
    virtual void setXCutLock(bool) =0;
    virtual void setYCutLock(bool) =0;
    virtual void setZCutLock(bool) =0;

	virtual bool isVolCompress() const =0;
	virtual bool isShowBoundingBox() const =0;
    virtual bool isShowAxes() const =0;

	virtual void hideTool() =0;
	virtual void showTool() =0;
	virtual void updateTool() =0;
	virtual void updateControl() =0;

	virtual int setVolumeTimePoint(int t) =0;
	virtual void incVolumeTimePoint(float step) =0;
	virtual int volumeTimePoint() const =0; // CMB 19Apr2011

	virtual void setRenderMode_Maxip(bool b) =0;
	virtual void setRenderMode_Minip(bool b) =0;
	virtual void setRenderMode_Alpha(bool b) =0;
	virtual void setRenderMode_Cs3d(bool b) =0;

	virtual void setCSTransparent(int) =0;
	virtual void setThickness(double) =0;
	virtual void setCurChannel(int) =0;

	virtual void setChannelR(bool b) =0;
	virtual void setChannelG(bool b) =0;
	virtual void setChannelB(bool b) =0;
	virtual void setVolCompress(bool b) =0;

	virtual void volumeColormapDialog() =0;
	virtual void surfaceSelectDialog(int curTab=0) =0;
	virtual void surfaceSelectTab(int curTab=0) =0;
	virtual void surfaceDialogHide() =0;
	virtual void annotationDialog(int dataClass, int surfaceType, int index) =0;

	virtual void setXRotation(int angle) =0;
	virtual void setYRotation(int angle) =0;
	virtual void setZRotation(int angle) =0;
	virtual void resetRotation(bool b_emit=true) =0;
	virtual void modelRotation(int xRotStep, int yRotStep, int zRotStep) =0;
	virtual void viewRotation(int xRotStep, int yRotStep, int zRotStep) =0;
	virtual void absoluteRotPose() =0;
    virtual void doAbsoluteRot(int xRot, int yRot, int zRot) =0;
	virtual void lookAlong(float xLook, float yLook, float zLook) =0;

	virtual void setZoom(int r) =0;
	virtual void setXShift(int s) =0;
	virtual void setYShift(int s) =0;
	virtual void setZShift(int s) =0;
	virtual void resetZoomShift() =0;

	// Float methods for smoother animation 2011 Feb 07 CMB
	virtual void setXShift(float s) =0;
    virtual void setYShift(float s) =0;
    virtual void setZShift(float s) =0;
    virtual void setZoom(float r) =0;
    virtual void doAbsoluteRot(float xRot, float yRot, float zRot) =0;
    virtual void setXRotation(float angle) =0;
    virtual void setYRotation(float angle) =0;
    virtual void setZRotation(float angle) =0;

	virtual void enableFrontSlice(bool) =0;
	virtual void enableXSlice(bool) =0;
	virtual void enableYSlice(bool) =0;
	virtual void enableZSlice(bool) =0;

	virtual void setFrontCut(int s) =0;
	virtual void setXCut0(int s) =0;
	virtual void setYCut0(int s) =0;
	virtual void setZCut0(int s) =0;
	virtual void setXCut1(int s) =0;
	virtual void setYCut1(int s) =0;
	virtual void setZCut1(int s) =0;
	virtual void setXCS(int s) =0;
	virtual void setYCS(int s) =0;
	virtual void setZCS(int s) =0;
	virtual void setXClip0(int s) =0;
	virtual void setYClip0(int s) =0;
	virtual void setZClip0(int s) =0;
	virtual void setXClip1(int s) =0;
	virtual void setYClip1(int s) =0;
	virtual void setZClip1(int s) =0;

	virtual void enableShowAxes(bool b) =0;
	virtual void enableShowBoundingBox(bool b) =0;
    virtual void enableClipBoundingBox(bool b) =0;
	virtual void enableOrthoView(bool b) =0;
	virtual void setBackgroundColor() =0;
	virtual void setBright() =0;

	virtual void setShowMarkers(int s) =0;
	virtual void setShowSurfObjects(int s) =0;
	virtual void enableMarkerLabel(bool) =0;
	virtual void setMarkerSize(int s) =0;
	virtual void enableSurfStretch(bool) =0;
    virtual void enableSurfZLock(bool) =0;
	virtual void toggleCellName() =0;

	virtual void createSurfCurrentR() =0;
	virtual void createSurfCurrentG() =0;
	virtual void createSurfCurrentB() =0;
	virtual void loadObjectFromFile(QString url="") =0;
	virtual void loadObjectListFromFile() =0;
	virtual void saveSurfFile() =0;

	virtual void togglePolygonMode() =0;
	virtual void toggleLineType() =0;
	virtual void toggleObjShader() =0;

     virtual void toggleNStrokeCurveDrawing()=0; // For n-right-strokes curve shortcut ZJL 110920
     virtual void setDragWinSize(int csize)=0; // ZJL 110927

	virtual void changeLineOption() =0;
	virtual void changeVolShadingOption() =0;
	virtual void changeObjShadingOption() =0;

	virtual void toggleTexFilter() =0;
	virtual void toggleTex2D3D() =0;
	virtual void toggleTexCompression() =0;
	virtual void toggleTexStream() =0;
	virtual void toggleShader() =0;
	virtual void showGLinfo() =0;

	virtual void updateWithTriView() =0;
    virtual void updateLandmark() = 0;
        virtual void updateImageData() =0;
	virtual void reloadData() =0;
	virtual void cancelSelect() =0;

    //defined for Katie's need to export the local 3D viewer starting and local locations //140811
    virtual int getLocalStartPosX()=0;
    virtual int getLocalStartPosY()=0;
    virtual int getLocalStartPosZ()=0;
    virtual int getLocalEndPosX()=0;
    virtual int getLocalEndPosY()=0;
    virtual int getLocalEndPosZ()=0;

    //get marker size, Nov. 08, 2016 by Zhi Zhou
    virtual int getMarkerSize() const =0;
};

#endif /* BASIC_VIEW3D_H_ */
