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
by Hanchuan Peng

Feb, 26 2006
Aug 3, 2006: change to use the new stackutil.h function for 4-byte-size format
Sept 24, 2006: v3d v1.2 add raw-2-byte support
Oct 9, 2006: begin v1.3, add "save" function by pressing 'ctrl-s', and add the "roiPolygon" to each view
Oct 11, 2006: add "crop" function by "ctrl-c', 'ctrl-d', and 'ctrl-left mouse'.
Jan 2/6, 2008: add members to XFormView class "	b_moveCurrentLandmark". "ind_landmarkToBeChanged"
Jan 7, 2008: add new buttons; also separate the ptLocation.h file
Jan 10, 2008: add a wheel event for scrolling through different depth of a stack
March 2, 2008: add the path info for stackutil.h
March 2, 2008: add the function bool convert_data_to_8bit(void * &img, V3DLONG * sz, int datatype);
March 14, 2008: add worm straightening specific codes
March 18, 2008: add two new functions to Image4DSimple class
March 26, 2008: add functions for registration menu
April 2, 2008: add the new menu for all-in-one warping
April 2, 2008: add a button for image processing
April 5, 2008: add a cubic-spline display function for the backbone of worm
April 14, 2008: add xyz rez equalize func
May 3, 2008: change the resampling func
June 8, 2008: add v3d_aboutinfo()
Aug 6, 2008: add a v3d button
Aug 10, 2008: add estimateRoughAmountUsedMemory()
Aug 14, 2008: overload the closeEvent function of XFormWidget() to assume the data get freed
Aug 19, 2008: add proj_alignment_matching_1single_pt()
Aug, 21, 2008: update the iDrawExternalParameter structure
Aug 22, 2008: add a fly brain lobeseg function
Aug 24, 2008: add colormap control which is separated from wano project
Sept 25, 2008: add object files' names in the iDrawExternalParameter()
July 31, 2009: RZC: My4DImage extend to 4D time series color image packed time.
Jan 28-30, 2010: PHC. further adjusting codes for v2.440 and above
**
****************************************************************************/

#ifndef V3d_CORE_H
#define V3d_CORE_H

#include "v3d_compile_constraints.h"

#include "../basic_c_fun/v3d_message.h"

#include "../basic_c_fun/stackutil.h"
#include "../basic_c_fun/color_xyz.h"
#include "../basic_c_fun/basic_4dimage.h"
#include "../basic_c_fun/basic_landmark.h"
#include "../basic_c_fun/basic_surf_objs.h"

#include "../basic_c_fun/img_definition.h"

#include "../neuron_editing/v_neuronswc.h"

#include "colormap.h"

#include "mapview.h"

// // Added by MK, 11/23/2016, for migrating from VS2010/Qt4 to VS2015/Qt5
#include "version_control.h"
#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
#include "../../../Qt/Qt5.6.2/Examples/Qt-5.6/widgets/painting/shared/arthurwidgets.h" 
#else
#include <arthurwidgets.h>
#endif
//////////////////////////////////////////////////////////////////////////////////////////

#include "../cellseg/template_matching_seg.h"
#include "template_matching_cellseg_dialog.h"


#include <QBasicTimer>
#include <QPolygonF>
#include <QTextBrowser>

class HoverPoints;
class QLineEdit;
class QLabel;
class QScrollBar;
class QSpinBox;
class QRadioButton;
class QPushButton;
class QHBoxLayout;
class QGridLayout;
class QVBoxLayout;
class QCheckBox;
class MainWindow;
class XFormView;
class XFormWidget;
class BDB_Minus_ConfigParameter;
struct iDrawExternalParameter;
class iDrawMainWindow;
class V3D_atlas_viewerDialog;

struct CurveTracePara; //in curve_trace_para_dialog.h
struct Options_Rotate;

typedef unsigned short int USHORTINT16;

enum ImagePlaneDisplayType {imgPlaneUndefined, imgPlaneX, imgPlaneY, imgPlaneZ};
enum AxisCode {axis_x, axis_y, axis_z, axis_c};
enum ImageResamplingCode {PRS_Z_ONLY, PRS_X_ONLY, PRS_Y_ONLY, PRS_XY_SAME, PRS_XYZ_SAME};
enum ImageMaskingCode {IMC_XYZ_INTERSECT, IMC_XYZ_UNION, IMC_XY, IMC_YZ, IMC_XZ};

template <class T> int new3dpointer_v3d(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, unsigned char * p1d);
template <class T> void delete3dpointer_v3d(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2);

template <class T> int new4dpointer_v3d(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, unsigned char * p1d);
template <class T> void delete4dpointer_v3d(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3);

void v3d_aboutinfo();

template <class T> QPixmap copyRaw2QPixmap(const T ** p2d, V3DLONG sz0, V3DLONG sz1);
template <class T> QPixmap copyRaw2QPixmap(const T ** p2dRed, const unsigned char ** p2dGreen, const unsigned char ** p2dBlue, V3DLONG sz0, V3DLONG sz1);
template <class T> QPixmap copyRaw2QPixmap_xPlanes(const T **** p4d, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, ImageDisplayColorType Ctype, V3DLONG cpos, bool bIntensityRescale, double *p_vmax, double *p_vmin);
template <class T> QPixmap copyRaw2QPixmap_yPlanes(const T **** p4d, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, ImageDisplayColorType Ctype, V3DLONG cpos, bool bIntensityRescale, double *p_vmax, double *p_vmin);
template <class T> QPixmap copyRaw2QPixmap_zPlanes(const T **** p4d, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, ImageDisplayColorType Ctype, V3DLONG cpos, bool bIntensityRescale, double *p_vmax, double *p_vmin);
template <class T> QPixmap copyRaw2QPixmap(const T **** p4d, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, ImageDisplayColorType Ctype, V3DLONG cpos, ImagePlaneDisplayType disType, bool bIntensityRescale, double *p_vmax, double *p_vmin);

QPixmap copyRaw2QPixmap_colormap(const void **** p4d, ImagePixelType dtype, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cpos, const ColorMap *pc, ImagePlaneDisplayType disType);
QPixmap copyRaw2QPixmap_xPlanes_colormap(const void **** p4d, ImagePixelType dtype, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cpos, const ColorMap *pc);
QPixmap copyRaw2QPixmap_yPlanes_colormap(const void **** p4d, ImagePixelType dtype, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cpos, const ColorMap *pc);
QPixmap copyRaw2QPixmap_zPlanes_colormap(const void **** p4d, ImagePixelType dtype, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cpos, const ColorMap *pc);


bool pointInPolygon(double x, double y, const QPolygon & pg);
bool file_exist(const char * filename);

class My4DImage;
bool compute_statistics_objects(My4DImage *grayimg, V3DLONG c, My4DImage * maskimg, LocationSimple * & p_ano, V3DLONG & n_objects);
bool compute_statistics_objects(Vol3DSimple<unsigned char> *grayimg, Vol3DSimple<unsigned short int> * maskimg, LocationSimple * & p_ano, V3DLONG & n_objects);


bool convert_data_to_8bit(void * &img, V3DLONG * sz, int datatype);

QList <LocationSimple> readPosFile(const char * posFile); //080107. obsolete. try not to use. 090725
QList <LocationSimple> readPosFile_usingMarkerCode(const char * posFile);
bool readSingleImageFile(char *imgSrcFile, unsigned char * & data1d, V3DLONG * & sz, ImagePixelType & datatype);

QStringList importSeriesFileList_addnumbersort(const QString & individualFileName, TimePackType & timepacktype);

// @ADDED by Alessandro on 2015-05-09. Method to get the path-based URL from the file-based URL
#ifdef _ENABLE_MACX_DRAG_DROP_FIX_
QString getPathFromYosemiteFileReferenceURL(QUrl);
#endif

// for mapview paras. ZJL
struct Mapview_Paras {
    int L, M, N, l, m, n;//block
    int level;           //current level
    V3DLONG outsz[4];    //output size
    V3DLONG origin[3];   //top-left corner pos
    QString hraw_prefix; //prefix of files

    Mapview_Paras()
    {
        hraw_prefix=QString("");
        outsz[0]=outsz[1]=outsz[2]=outsz[3]=0;
        origin[0]=origin[1]=origin[2]=0;
        L = M = N = l = m = n = 0;
        level=0;
    }
};


struct InvidualAtlasFileInfo
{
	int n;			  // index
	QString category; //the name of the GAL4 line, or other categorical info
	QString imgfile;  //the name of the actual image file
	bool exist;       //if the file exist or not
	RGBA8 color;
	bool on;
};

QList <InvidualAtlasFileInfo> readAtlasFormatFile(const char * filename); //081124

struct DataChannelColor
{
	int n;			  // index
	RGBA8 color;
	bool on;
};


class V3dR_GLWidget;
class Renderer_gl1;

int mergeback_mmunits_to_neuron_path(int n_end_nodes, vector< vector<V_NeuronSWC_unit> > & mmUnit, V_NeuronSWC_list & tNeuron);

class MyTextBrowser : public QTextBrowser
{
	Q_OBJECT;

public:
	MyTextBrowser(QWidget * parent=0);
	~MyTextBrowser(){}

public slots:
	void setText2FocusPointFeatureWidget()
	{
		this->setText(focusFeatureViewTextContent);
		this->update();
	}

public:
	void setFocusFeatureViewTextContent(QString text)
	{
		focusFeatureViewTextContent = text;
	}

	QString getFocusFeatureViewTextContent()
	{
		return focusFeatureViewTextContent;
	}

private:
	QString focusFeatureViewTextContent;

};

class My4DImage : public QObject, public Image4DSimple
{
	Q_OBJECT;

public:
        double at(int x, int y, int z, int c=0) const; //return a double number because it can always be converted back to UINT8 and UINT16 without information loss
	void **** getData(ImagePixelType & dtype);
	void **** getData() {return data4d_virtual;}
	bool isEmpty() {return (!data4d_virtual) ?  true : false; }

	bool valid() {
    return (!data4d_virtual || !(this->Image4DSimple::valid()) ) ?  false : true;
    }

	void loadImage(const char* filename);
	void setupData4D();
	void setupDefaultColorChannelMapping();
	bool updateminmaxvalues();
	void loadImage(V3DLONG imgsz0, V3DLONG imgsz1, V3DLONG imgsz2, V3DLONG imgsz3, int imgdatatype); //an overloaded function to create a blank image

	void setFocusX(V3DLONG x) {curFocusX = (x>=1 && x <= this->getXDim()) ? x-1 : -1;}
	void setFocusY(V3DLONG y) {curFocusY = (y>=1 && y <= this->getYDim()) ? y-1 : -1;}
	void setFocusZ(V3DLONG z) {curFocusZ = (z>=1 && z <= this->getZDim()) ? z-1 : -1;}
	V3DLONG getFocusX() {return curFocusX;}
	V3DLONG getFocusY() {return curFocusY;}
	V3DLONG getFocusZ() {return curFocusZ;}

	My4DImage();
	~My4DImage();

	float **** data4d_float32;
	USHORTINT16 **** data4d_uint16;
	unsigned char **** data4d_uint8;
	void **** data4d_virtual;

	void updateViews();

	bool reshape(V3DLONG rsz0, V3DLONG rsz1, V3DLONG rsz2, V3DLONG rsz3);
	bool permute(V3DLONG dimorder[4]);

	double * p_vmax, * p_vmin; //whole volume max/min values. Use pointer to handle multiple channels separately
	double getChannalMinIntensity(V3DLONG channo);
	double getChannalMaxIntensity(V3DLONG channo);

	bool bLinkFocusViews;
	bool bDisplayFocusCross;
	bool bLookingGlass;

	ColorMap *colorMap;

	void createColorMap(int len, ImageDisplayColorType c=colorPseudoMaskColor);
	void switchColorMap(int len, ImageDisplayColorType c);
	void getColorMapInfo(int &len, ImageDisplayColorType &c);

	V3DLONG curFocusX, curFocusY, curFocusZ;
	XFormView *p_xy_view, *p_yz_view, *p_zx_view;

	MyTextBrowser *p_focusPointFeatureWidget;

	XFormWidget *p_mainWidget;

	void cleanExistData_butKeepFileName();
	void cleanExistData();
	void cleanExistData_only4Dpointers();
	bool setNewImageData(unsigned char *ndata1d, V3DLONG nsz0, V3DLONG nsz1, V3DLONG nsz2, V3DLONG nsz3, ImagePixelType ndatatype, V3DLONG nszt=-1, TimePackType tpk=TIME_PACK_NONE);//nszt==-1 will not reset the ntimepoints and pack type

	void set_xy_view(XFormView *p) {p_xy_view = p;}
	void set_yz_view(XFormView *p) {p_yz_view = p;}
	void set_zx_view(XFormView *p) {p_zx_view = p;}
	XFormView * get_xy_view() {return p_xy_view;}
	XFormView * get_yz_view() {return p_yz_view;}
	XFormView * get_zx_view() {return p_zx_view;}

	void setFocusFeatureView(MyTextBrowser *p) {p_focusPointFeatureWidget = p;}
	QTextBrowser *getFocusFeatureView() {return p_focusPointFeatureWidget;}
	QString setFocusFeatureViewText();

	void setMainWidget(XFormWidget *p) {p_mainWidget=p;}
	XFormWidget * getXWidget() {return p_mainWidget;}

	void setFlagLinkFocusViews(bool t) {bLinkFocusViews = t;}
	bool getFlagLinkFocusViews() {return bLinkFocusViews;}
	void setFlagDisplayFocusCross(bool t) {bDisplayFocusCross = t;}
	bool getFlagDisplayFocusCross() {return bDisplayFocusCross;}

	//void setFlagImgValScaleDisplay(bool t) {bImgValScaleDisplay=t;}
	bool getFlagImgValScaleDisplay();

	void setFlagLookingGlass(bool t) {bLookingGlass=t;}
	bool getFlagLookingGlass() {return bLookingGlass;}

	void recordFocusProperty(PxLocationUsefulness t);
	V3DLONG find_closest_control_pt(int sx, int sy, int sz, double & dmin);
	V3DLONG find_closest_control_pt_thres(int sx, int sy, int sz, double rr, double & dmin);

	QList <DataChannelColor> listChannels; //100824
	QList <LocationSimple> listLandmarks;
	QList <PtIndexAndParents> listLocationRelationship;
	QList <InvidualAtlasFileInfo> listAtlasFiles;
	int atlasColorBlendChannel;
	bool bUseFirstImgAsMask;
	QString curSearchText;

    //timer //20120702
    QTime triviewTimer;  //added on 20120702.
    bool b_triviewTimerON;

	bool compute_rgn_stat(LocationSimple & pt, int channo);
	void loadLandmarkFromFile();
	void saveLandmarkToFile();
	void exportLandmarkToPointCloudAPOFile();
	void exportLandmarkandRelationToSWCFile();
	void exportNeuronToSWCFile();

	void computePointNeighborMoment(int x, int y, int z, int c, double & curptval, double & ave, double & sdev, double & skew, double & curt);
	void computePointNeighborMoment(LocationSimple & L, int c); //overload for convenience

	bool saveVANO_data();
	bool saveMovie();
	bool saveFile();
    bool saveFile(char filename[]);
    bool saveFile(QString outputFile);
	void crop(int landmark_crop_opt);
	void crop(V3DLONG bpos_x, V3DLONG epos_x, V3DLONG bpos_y, V3DLONG epos_y, V3DLONG bpos_z, V3DLONG epos_z, V3DLONG bpos_c, V3DLONG epos_c, int landmark_crop_opt);
	bool maskBW_roi_bbox(unsigned char tval, V3DLONG c_min, V3DLONG c_max, ImageMaskingCode my_maskcode, bool b_inside);
	bool maskBW_roi(unsigned char tval, V3DLONG c_min, V3DLONG c_max, ImageMaskingCode my_maskcode, bool b_inside);
	bool maskBW_channel(V3DLONG mask_channel_no);

	bool invertcolor(int channo); //channo < 0 will invert all channels. Only works for uint8
	bool scaleintensity(int channo, double lower_th, double higher_th, double target_min, double target_max); //map the value linear from [lower_th, higher_th] to [target_min, target_max].
	bool thresholdintensity(int channo, double th); //anything < th will be 0, others unchanged
	bool binarizeintensity(int channo, double th); //anything < th will be 0, others will be 1

	bool rotate(ImagePlaneDisplayType ptype, const Options_Rotate & r_opt);
	bool flip(AxisCode my_axiscode);

	bool b_proj_worm_mst_diameter_set;

    V_NeuronSWC_list tracedNeuron, tracedNeuron_old;//add tracedNeuron_old by PHC, 20150523

	V3DLONG last_hit_landmark, cur_hit_landmark;
	BoundingBox trace_bounding_box;
	float trace_z_thickness;

	bool proj_trace_deformablepath_one_point(V3DLONG startmark_id); // 090603 RZC: output a shortest path tree
	bool proj_trace_deformablepath_one_point_to_allotherpoints(V3DLONG startmark_id); //090609: phc. one pt to all other points
	bool proj_trace_deformablepath_two_points(V3DLONG startmark_id, V3DLONG endmark_id, bool b_select_para=false, int method_code=0); //method code = 0 for shortest path and 1 for deformable model only
	bool proj_trace_deformablepath_two_points(V3DLONG startmark_id, V3DLONG endmark_id, int npoints, bool b_select_para, bool b_fitradius, int method_code); //overloading function provided for convenience
	bool proj_trace_deformablepath_two_points_basic(LocationSimple & p1, LocationSimple & p2, CurveTracePara & trace_para);
	bool proj_trace_deformablepath_two_points_adaptive(LocationSimple & p1, LocationSimple & p2, CurveTracePara & trace_para);
	int proj_trace_deformablepath_two_points_shortestdist(LocationSimple & p1, LocationSimple & p2, CurveTracePara & trace_para);
	int proj_trace_deformablepath_all_points_shortestdist(LocationSimple & p0, vector<LocationSimple> & pp, CurveTracePara & trace_para) ;
	bool proj_trace_smooth_downsample_last_traced_neuron(CurveTracePara & trace_para, int seg_begin, int seg_end);
	bool proj_trace_shortestpath_rgnaroundcurve(CurveTracePara & trace_para, int seg_begin, int seg_end);
	bool proj_trace_compute_radius_of_last_traced_neuron(CurveTracePara & trace_para, int seg_begin, int seg_end, float myzthickness);
//<<<<<<< HEAD
	bool proj_trace_add_curve_segment(vector<XYZ> &loc_list, int chno, double default_type=3);
    NeuronTree proj_trace_add_curve_segment_append_to_a_neuron(vector<XYZ> &mCoord, int chno, NeuronTree & neuronEdited, double default_type=3); //150523
//>>>>>>> master
	bool proj_trace_deformNeuronSeg(V3DLONG node_id, NeuronTree *p_tree, bool b_select_para=true);
	bool proj_trace_profileNeuronSeg(V3DLONG node_id, NeuronTree *p_tree, bool b_dispfig);

	bool proj_trace_mergeOneClosebyNeuronSeg(V3DLONG node_id, NeuronTree *p_tree);
	bool proj_trace_mergeAllClosebyNeuronNodes(NeuronTree *p_tree);
	bool proj_trace_mergeAllClosebyNeuronNodes();

	QList <V_NeuronSWC_list> tracedNeuron_historylist;
	static const int MAX_history = 30;
	int cur_history;
	void proj_trace_history_append(V_NeuronSWC_list & tNeuron);
	void proj_trace_history_append();
	void proj_trace_history_undo(V_NeuronSWC_list & tNeuron);
	void proj_trace_history_undo();
	void proj_trace_history_redo(V_NeuronSWC_list & tNeuron);
	void proj_trace_history_redo();

	bool proj_trace_changeNeuronSegType(V3DLONG node_id, NeuronTree *p_tree);
	bool proj_trace_changeNeuronSegRadius(V3DLONG node_id, NeuronTree *p_tree);
	bool proj_trace_reverseNeuronSeg(V3DLONG node_id, NeuronTree *p_tree);
	bool proj_trace_deleteNeuronSeg(V3DLONG node_id, NeuronTree *p_tree);
	double proj_trace_measureLengthNeuronSeg(V3DLONG node_id, NeuronTree *p_tree);
	bool proj_trace_breakNeuronSeg(V3DLONG node_id, NeuronTree *p_tree);
	bool proj_trace_joinNearbyNeuronSegs_pathclick(V3DLONG node_id, NeuronTree *p_tree);
	bool proj_trace_joinNearbyNeuronSegs_markclick(V3DLONG node_id, NeuronTree *p_tree);
	bool proj_trace_joinAllNeuronSegs(V3DLONG node_id, NeuronTree *p_tree);
	// load traced neuron to 3D view
	void update_3drenderer_neuron_view();
	void update_3drenderer_neuron_view(V3dR_GLWidget* glwidget, Renderer_gl1* renderer);

	bool proj_general_principal_axis(ImagePlaneDisplayType ptype);
	bool proj_general_resampling(ImageResamplingCode mycode, double target_rez, double cur_rez, int interp_method);
	bool proj_general_resampling_landmark_only(ImageResamplingCode mycode, double target_rez, double cur_rez);
	bool proj_general_landmark_plusminus_constant(ImageResamplingCode mycode, double cval);
	bool proj_general_projection(AxisCode myaxis, V3DLONG mincoord, V3DLONG maxcoord);
	bool proj_general_blend_channel_real(My4DImage * pBlendDstImg, My4DImage * pBlendSrcImg, V3DLONG chnoBlendSrcImg, double rr, double gg, double bb, bool b_assignVal_NoComparison);
	bool proj_general_blend_channels();
	bool proj_general_blend_atlasfiles();
	bool proj_general_split_channels(bool b_keepallchannels, int chno);
	bool proj_general_hist_display();
	bool proj_general_linear_adjustment();
	bool proj_general_hist_equalization(unsigned char lowerbound, unsigned char higherbound);
	bool proj_general_convertIndexedImg2RGB();
	bool proj_general_scaleandconvert28bit(int lb, int ub); //lb, ub: lower bound, upper bound
	bool proj_general_scaleandconvert28bit_1percentage(double apercent); //apercent: percentage of saturation, typiecally 0.01
	bool proj_general_convert16bit_to_8bit(int shiftnbits);
	bool proj_general_convert32bit_to_8bit(int shiftnbits);
	bool proj_general_maskBlue2Zero();


	QList <LocationSimple> autoMarkerFromImg(V3DLONG chno);
	QList <LocationSimple> autoMarkerFromImg(V3DLONG chno, BoundingBox bbox, float zthickness);

signals:
	void focusFeatureViewTextUpdated();
	void signal_trace_history_append(); //20170801 RZC add

public slots:
	void setText2FocusPointFeatureWidget();

};


bool getFocusCrossLinePos(int & focusPosInWidth, int & focusPosInHeight, My4DImage * imgData, ImagePlaneDisplayType Ptype);


class XFormView : public ArthurFrame //class XFormView : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double scale READ scale WRITE changeScale)

public:
    XFormView(QWidget *parent);
	void setImgData(ImagePlaneDisplayType ptype, My4DImage * pdata, ImageDisplayColorType ctype);

    void paint(QPainter *);
    void drawPixmapType(QPainter *painter);
    void drawLookingGlassMap(QPainter *painter, QPoint *curPt);
	void drawSelectedLocations(QPainter *painter, QList <LocationSimple> *curList, QList <PtIndexAndParents> * curRelation);

    void drawROI(QPainter *painter);

	int disp_width, disp_height;
	double disp_scale;
	int get_disp_width() {return disp_width;}
	int get_disp_height() {return disp_height;}
	double get_disp_scale() {return disp_scale;}
	void set_disp_width(int a) {disp_width = a;}
	void set_disp_height(int a) {disp_height = a;}
	void set_disp_scale(double a) {disp_scale = a; }

	// Converts point from mouse event coordinates to image coordinates
	QPointF mouseEventToImageCoords(const QPoint& p);

    void mousePressEvent(QMouseEvent *e);
    void mouseLeftButtonPressEvent(QMouseEvent *e);
    void mouseRightButtonPressEvent(QMouseEvent *e);

    void mouseDoubleClickEvent(QMouseEvent * e);

	void mouseMoveEvent (QMouseEvent * e);
    void enterEvent (QEvent * e);
    void leaveEvent (QEvent * e);

	void wheelEvent(QWheelEvent * e);

    void dispHistogram();
	QRect getRoiBoundingRect();
	void deleteROI();
	const QPolygon & getRoi() {return roiPolygon;}

    double scale() const { return m_scale; }

    int focusPlaneCoord() {return cur_focus_pos;}
	ImagePlaneDisplayType focusPlaneType() {return Ptype;}

public slots:
	void setPixmap(const QPixmap& pxm, bool bGlass)  //110722 RZC, for directly update pixmap of 3view. //110803 RZC, add bGlass
	{
		if (! bGlass)
			this->pixmap = pxm;
		else
			this->pixmap_glass = pxm;
		update();
	}
	void updateViewPlane() {changeFocusPlane(cur_focus_pos);} // 090504 RZC: make FocusPlane image updated from image stack

    void changeScale(int scale);
    void changeFocusPlane(int c);
    void changeColorType(ImageDisplayColorType c, bool bGlass=false); //110803 RZC
    void setPixmapType();
    void reset();

	void popupImageProcessingDialog();
	void popupImageProcessingDialog(QString item); //added on 080613

signals:
	void colorChanged(int); //110722 RZC, connected to XFormWidget::colorChanged(int)
	void colorChangedGlass(int); //110803 RZC, connected to XFormWidget::colorChangedGlass(int)

	void scaleChanged(int scale);
//    void colorTypeChanged(ImageDisplayColorType c); //probably should be deleted later
    void focusXChanged(int x_new);
    void focusYChanged(int y_new);
    void focusZChanged(int z_new);

protected:
    void do_keyPressEvent ( QKeyEvent * e);

public:
	QPolygon roiPolygon;//061009

private:
    enum XFormType { VectorType, PixmapType, TextType};

    double m_scale;
	QPointF curDisplayCenter;
	QPointF curDisplayCenter0;

	QPoint curMousePos;
	bool bMouseCurorIn;

	QPoint dragStartPosition;

	QCursor myCursor;

	//QPoint curMouseShiftPressPos; //080102

    XFormType Gtype;
    ImagePlaneDisplayType Ptype;

    ImageDisplayColorType Ctype;
    ImageDisplayColorType Ctype_glass; //110803 RZC
    QPixmap pixmap; 		//image buffer of plane
    QPixmap pixmap_glass; 	//image buffer of looking glass
    bool _for_index_only;			//110803 RZC

    int cur_focus_pos;

	My4DImage * imgData; //a reference to the real data stored in the XFormWidget

	bool b_displayFocusCrossLine;

	int focusPosInWidth, focusPosInHeight;

	bool b_moveCurrentLandmark;
	V3DLONG ind_landmarkToBeChanged;

private:
	bool internal_only_imgplane_op(); //100814. by PHC. added for better modulization in data display
};

struct BlendingImageInfo
{
	My4DImage * pimg;
	int channo;
	double rr, gg, bb;
};


#include "xformwidget.h"

#endif // v3d_core.h

