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
 Copyright (C) 2006-2008 Hanchuan Peng. All rights reserved.
 Read the raw image file generated using Hanchuan Peng's program saveStack2Raw.m .

 by Hanchuan Peng
 Feb 25, 2006
 Feb 28, 2006
 v0.3: March 1, 2006: fix the looking glass display bug.
 March 13: add control point definition & display
 060816: add tif reading
 2006-09-24: add raw-2byte-support (the same as the current wano v0.13)
 Oct 9, 2006: begin v1.3, add "save" function by pressing 'ctrl-s', and add the "roiPolygon" to each view
 Oct 11, 2006: add "crop" function by "ctrl-c', 'ctrl-d', and 'ctrl-left mouse'.
 July 16, 2007: correct the bug in RGB gray display. The new version is called v3d 1.4
 Aug, 5/6, 2007 - try to add LSM reading support. Now called version v3d 1.5
 Dec 17, 2007 - try to add a control point move function and then define corresponding matching points of multiple images. unfinished.
 Jan 1, 2008 - continue to develop the landmarking tool
 Jan 7/8, 2008: developing the landmark tools and also add help info
 Jan 9, 2008: add 'b' which equals 'p' keyboard operation
 Jan 10, 2008: add scroll control through a stack
 Jan 11, 2008: add the default file name for the landmark saving function
 Jan 17, 2008: add info of a landmark when double-clicking it
 Jan 21, 2008: change the wheel scrolling up/down direction
 March 02, 2008: v1.63, add display of 12 (16) bit LSM/TIF
 March 17, 2008: add the C. elegans functions including random seeding, MST finding. Now call version 1.64
 March 18, 2008: v1.65, with the tiff series importing functions
 March 19, 2008: v1.66, with the PCA based major axis estimation and rotation
 March 25, 2008: temporarily block the menu of image registration etc
 March 26, 2008: v1.67. add fly 3D registration codes
 April 1, 2008: link laff header
 April 2, 2008: add several functions to support the all-in-one matching and warping
 April 2, 2008: add several more menu items to clean all landmarks or just the relationship graph of landmarks, and do other image processing as well
 April 2, 2008: remove the open file button
 April 5, 2008: add a cubic-spline display function for the backbone of worm
 April 6, 2008: continue debug the cubic-spline functions
 April 8, 2008: the worm straightening func
 April 11, 2008: add thread for registration
 April 14, 2008: add reslicing function
 April 15, 2008: add all three axes resampling (for X,Y the downsampling) functions
 April 16, 2008: add multiple image blending function with a simple interface. More complicated interface (e.g. clickable color) will be added later
 April 16, 2008: add a new crop func, landmark scaling func, and a landmark plus/minus constant func. V1.70
 April 21, 2008: adding masking func
 April 22, 2008: try to add the stitching func
 April 24, 2008: add masking codes for any shape. To be done: a mask-crop function to remove everything out of the selection
 May 3, 2008: add the nearest neighbor interpolation (useful for mask image interpolation)
 May 13: try to activate the BDB minus module
 June 4: coorect the file name overwriting bug in rotating and other processing of images
 June 11, 2008: add histeq for an image
 June 18, 2008: correct the landmark save surfix problem; also add the 5-column printing for the coordinates in the display panel
 July 23, 2008: add v3d_compile_constraints.h
 July 28, 2008: add mask_out function to use a channel to mask out other channel info
 July 30, 2008: add a compiling flag to disable showing the stitching mask
 Aug 8, 2008: remove redundant / unused para for rotate image
 Aug 13, 2008: add short-keys to activate the 3D view
 Aug 19, 2008: update the function to call  detectBestMatchingCpt()
 Aug 19, 2008: add a function to only find the best match for one single specified landmark
 Aug 21, 2008: move the see in 3d button to top
 Aug, 22, 2008: add lobeseg function
 Aug 23, 2008: add cell seg functions
 Aug 28, 2008: add landmark cropping functions
 Sept 4, 2008: try to add outside ROI mask, but not finished yet
 Sept 30, 2008: add an image color channel wrapper around function
 Oct 1, 2008: add a lot of try catch in the main image processing function entry point popupImageProcessingFunc()
 Nov 14, 2008: add zoom in-out of tri-view
 Nov 15, 2008: fix a bug in triview ctrl-click to define rois. also add zoom-1 and zoom-2 support. add v3d_version_info.h
 Nov 23, 2008: add atlas_viewer
 Nov 26, 2008: add memory use threshold th_use_memory
 Nov 27, 2008: add jba_match_landmarks.cpp
 Dec 7, 2008: separate the functions of My4DImage class to my4dimage.cpp, so that the file size is smaller
 Dec 8, 2008: add landmark_property_dialog
 Jan 8, 2009: add GMM fitting for a given location (current focus location)
 Jan 17, 2009: add real time neuron trace & display
 March 18, 2009: add global affine transform based on matching landmarks
 July 6, 2009: separate the central image procesing entry points to v3dimgproc_entry.cpp
 July 31, 2009: RZC: XFormWidget::importGeneralImgSeries extend to 4D time series color image packed time.
 May 20, 2010: add MSVC conditiional compilation
 May 21, 2010: change the upper limit of the amount of memory a system can use. This should become a user preference later!
 Jul 27, 2011: RZC: add code path of ChannelTable to mix many channels to RGB
 **
 ****************************************************************************/

#include <stdio.h>


#if defined (_MSC_VER) //added by PHC, 2010-05-20. do not need to include <strings.h> for VC complier (indeed this file does not even exist)
#else
#include <strings.h>
#endif

#include <math.h>

#include "../3drenderer/v3dr_common.h" // Ensure Glee is loaded first
#include "../vrrenderer/v3dr_gl_vr.h"


#include <QLayout>
#include <QPainter>
#include <QPainterPath>
//#include <QTextEdit>

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
  #include <QtWidgets>
#else
  #include <QtGui>
#endif

#include "v3d_core.h"
#include "mainwindow.h"

#include <fstream> //080107
#include <iostream> //080107
using namespace std;

#include "rotate_image.h"
#include "dialog_rotate.h"
#include "landmark_property_dialog.h"
#include "dialog_keypoint_features.h"
#include "dialog_curve_trace_para.h"
#include "import_images_tool_dialog.h"

#include "dialog_imagecrop_bbox.h"
#include "dialog_imageresample.h"
#include "dialog_maskroi.h"

#include "atlas_viewer.h"

#include "v3d_version_info.h"

#include "../basic_c_fun/volimg_proc.h"
#include "../basic_c_fun/stackutil.h" // For getSuffix

#include "../3drenderer/v3dr_mainwindow.h" //v3d_drawmain-->v3dr_mainwindow, by RZC 20080921
#include "../3drenderer/v3dr_glwidget.h" //090710 by RZC for XFormWidget::doImage3DView
#include "../3drenderer/renderer_gl1.h" //090117 by RZC for My4DImage::update_3drenderer_neuron_view


#include "ChannelTable.h" //110718 RZC, lookup and mix multi-channel's color
#define USE_CHANNEL_TABLE  1
inline bool isIndexColor(ImageDisplayColorType c) { return (c>=colorPseudoMaskColor); }


#include "../multithreadimageIO/v3d_multithreadimageIO.h"

#ifdef USE_FFMPEG
#include "../neuron_annotator/utility/loadV3dFFMpeg.h"
#endif

/////// a global variable to limit the amount of memory use

#if defined (Q_OS_LINUX)
#if defined (__x86_64__)
double TH_USE_MEMORY=64.0; //64bit Linux
#else
double TH_USE_MEMORY=1.5; //32bit Linux
#endif

#else

#if defined (__APPLE__)
#if defined(__MAC_x86_64__)
double TH_USE_MEMORY=64.0; //64bit Mac
#else
double TH_USE_MEMORY=1.1; //32bit Mac
#endif

#else
#if defined (WIN32) || defined (_WIN32)
#if defined (_WIN64)
double TH_USE_MEMORY=64.0; //64bit Windows
#else
double TH_USE_MEMORY=1.5; //32bit Windows
#endif
#endif

#endif

#endif


MyTextBrowser::MyTextBrowser(QWidget * parent) : QTextBrowser(parent)
{
}


// all copyRaw2QPixmap(T****, ?) called by XFormView::internal_only_imgplane_op

template <class T> QPixmap copyRaw2QPixmap(const T ** p2d, V3DLONG sz0, V3DLONG sz1)
{
	QImage tmpimg = QImage(sz0, sz1, QImage::Format_RGB32);
	int tmpval;
	for (V3DLONG j=0;j<sz1;j++)
	{
		for (V3DLONG i=0;i<sz0;i++)
		{
			tmpval = int(p2d[j][i]);
			tmpimg.setPixel(i, j, qRgb(tmpval, tmpval, tmpval));
		}
	}
	return QPixmap::fromImage(tmpimg);
}

template <class T>  QPixmap copyRaw2QPixmap(const T ** p2dRed, const T ** p2dGreen, const T ** p2dBlue, V3DLONG sz0, V3DLONG sz1)
{
	QImage tmpimg = QImage(sz0, sz1, QImage::Format_RGB32);
	int tr,tg,tb;
	for (V3DLONG j=0;j<sz1;j++)
	{
		for (V3DLONG i=0;i<sz0;i++)
		{
			tr = int((p2dRed)?p2dRed[j][i]:0);
			tg = int((p2dGreen)?p2dGreen[j][i]:0);
			tb = int((p2dBlue)?p2dBlue[j][i]:0);
			tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
		}
	}
	return QPixmap::fromImage(tmpimg);
}

template <class T> QPixmap copyRaw2QPixmap_xPlanes(const T **** p4d,
												   V3DLONG sz0,
												   V3DLONG sz1,
												   V3DLONG sz2,
												   V3DLONG sz3,
												   ImageDisplayColorType Ctype,
												   V3DLONG cpos,
												   bool bIntensityRescale,
												   double *p_vmax,
												   double *p_vmin)
{
	QImage tmpimg = QImage(sz2, sz1, QImage::Format_RGB32);
	int tr,tg,tb;

	V3DLONG curpos = (cpos>sz0) ? sz0-1 : cpos-1;
	curpos = (curpos<0)?0:curpos;

	V3DLONG i,j;
	double tmpr,tmpg,tmpb;
	double tmpr_min, tmpg_min, tmpb_min;

	if (sz3>=3)
	{
		tmpb = p_vmax[2]-p_vmin[2]; tmpb = (tmpb==0)?1:tmpb;
		tmpb_min = p_vmin[2];
	}

	if (sz3>=2)
	{
		tmpg = p_vmax[1]-p_vmin[1]; tmpg = (tmpg==0)?1:tmpg;
		tmpg_min = p_vmin[1];
	}

	if (sz3>=1)
	{
		tmpr = p_vmax[0]-p_vmin[0]; tmpr = (tmpr==0)?1:tmpr;
		tmpr_min = p_vmin[0];
	}

	//the wrong Ctype options should be disabled in the interface instead of a complicated logic management here
	switch (Ctype)
	{
		case colorRedOnly:
			tb = tg = 0;
			if (bIntensityRescale==false)
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tr = p4d[0][i][j][curpos];
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			else
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tr = floor((p4d[0][i][j][curpos]-tmpr_min)/tmpr*255.0);
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			break;

		case colorGray: //070716
			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz2;i++)
				{
					double tmpval=0;
					for (int tmpcnt=0;tmpcnt<sz3;tmpcnt++)
					{
						if (tmpcnt==0)
							tmpval += ((bIntensityRescale==false) ? p4d[tmpcnt][i][j][curpos] : floor((p4d[tmpcnt][i][j][curpos]-tmpr_min)/tmpr*255.0));
						else if (tmpcnt==1)
							tmpval += ((bIntensityRescale==false) ? p4d[tmpcnt][i][j][curpos] : floor((p4d[tmpcnt][i][j][curpos]-tmpg_min)/tmpg*255.0));
						else if (tmpcnt==2)
							tmpval += ((bIntensityRescale==false) ? p4d[tmpcnt][i][j][curpos] : floor((p4d[tmpcnt][i][j][curpos]-tmpb_min)/tmpb*255.0));
						else
							tmpval += ((bIntensityRescale==false) ? p4d[tmpcnt][i][j][curpos] : p4d[tmpcnt][i][j][curpos]); //in this case, don't about any channel more than 3
					}
					tmpval /= sz3;
					tmpimg.setPixel(i, j, qRgb(int(tmpval), int(tmpval), int(tmpval)));
				}
			}
			break;

		case colorRed2Gray:
			if (bIntensityRescale==false)
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tg = tb = tr = p4d[0][i][j][curpos];
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			else
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tg = tb = tr = floor((p4d[0][i][j][curpos]-tmpr_min)/tmpr*255.0);
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			break;

		case colorGreenOnly:
			tb = tr = 0;
			if (bIntensityRescale==false)
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tg = p4d[1][i][j][curpos];
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			else
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tg = floor((p4d[1][i][j][curpos]-tmpg_min)/tmpg*255.0);
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			break;

		case colorGreen2Gray:
			if (bIntensityRescale==false)
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tb = tr = tg = p4d[1][i][j][curpos];
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			else
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tb = tr = tg = floor((p4d[1][i][j][curpos]-tmpg_min)/tmpg*255.0);
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			break;

		case colorBlueOnly:
			tg = tr = 0;
			if (bIntensityRescale==false)
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tb = p4d[2][i][j][curpos];
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			else
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tb = floor((p4d[2][i][j][curpos]-tmpb_min)/tmpb*255.0);
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			break;

		case colorBlue2Gray:
			if (bIntensityRescale==false)
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tg = tr = tb = p4d[2][i][j][curpos];
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			else
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tg = tr = tb = floor((p4d[2][i][j][curpos]-tmpb_min)/tmpb*255.0);
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			break;

		case colorRGB:
			if (bIntensityRescale==false)
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tr = p4d[0][i][j][curpos];
						tg = p4d[1][i][j][curpos];
						tb = p4d[2][i][j][curpos];
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			else
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tr = floor((p4d[0][i][j][curpos]-tmpr_min)/tmpr*255.0);
						tg = floor((p4d[1][i][j][curpos]-tmpg_min)/tmpg*255.0);
						tb = floor((p4d[2][i][j][curpos]-tmpb_min)/tmpb*255.0);
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			break;

		case colorRG:
			tb = 0;
			if (bIntensityRescale==false)
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tr = p4d[0][i][j][curpos];
						tg = p4d[1][i][j][curpos];
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			else
			{
				for (j=0;j<sz1;j++)
				{
					for (i=0;i<sz2;i++)
					{
						tr = floor((p4d[0][i][j][curpos]-tmpr_min)/tmpr*255.0);
						tg = floor((p4d[1][i][j][curpos]-tmpg_min)/tmpg*255.0);
						tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
					}
				}
			}
			break;

		case colorUnknown:
		default:

			break;
	}

	return QPixmap::fromImage(tmpimg);
}


template <class T> QPixmap copyRaw2QPixmap_yPlanes(const T **** p4d,
												   V3DLONG sz0,
												   V3DLONG sz1,
												   V3DLONG sz2,
												   V3DLONG sz3,
												   ImageDisplayColorType Ctype,
												   V3DLONG cpos,
												   bool bIntensityRescale,
												   double *p_vmax,
												   double *p_vmin)
{
	QImage tmpimg = QImage(sz0, sz2, QImage::Format_RGB32);
	int tr,tg,tb;

	V3DLONG curpos = (cpos>sz1) ? sz1-1 : cpos-1;
	curpos = (curpos<0)?0:curpos;

	V3DLONG i,j;
	double tmpr,tmpg,tmpb;
	double tmpr_min, tmpg_min, tmpb_min;

	if (sz3>=3)
	{
		tmpb = p_vmax[2]-p_vmin[2]; tmpb = (tmpb==0)?1:tmpb;
		tmpb_min = p_vmin[2];
	}

	if (sz3>=2)
	{
		tmpg = p_vmax[1]-p_vmin[1]; tmpg = (tmpg==0)?1:tmpg;
		tmpg_min = p_vmin[1];
	}

	if (sz3>=1)
	{
		tmpr = p_vmax[0]-p_vmin[0]; tmpr = (tmpr==0)?1:tmpr;
		tmpr_min = p_vmin[0];
	}

	//the wrong Ctype options should be disabled in the interface instead of a complicated logic management here

	switch (Ctype)
	{
		case colorRedOnly:
			tb = tg = 0;
			for (j=0;j<sz2;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tr = (bIntensityRescale==false) ? p4d[0][j][curpos][i] : floor((p4d[0][j][curpos][i]-tmpr_min)/tmpr*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorGray: //070716
			for (j=0;j<sz2;j++)
			{
				for (i=0;i<sz0;i++)
				{
					double tmpval=0;
					for (int tmpcnt=0;tmpcnt<sz3;tmpcnt++)
					{
						if (tmpcnt==0)
							tmpval += ((bIntensityRescale==false) ? p4d[tmpcnt][j][curpos][i] : floor((p4d[tmpcnt][j][curpos][i]-tmpr_min)/tmpr*255.0));
						else if (tmpcnt==1)
							tmpval += ((bIntensityRescale==false) ? p4d[tmpcnt][j][curpos][i] : floor((p4d[tmpcnt][j][curpos][i]-tmpg_min)/tmpg*255.0));
						else if (tmpcnt==2)
							tmpval += ((bIntensityRescale==false) ? p4d[tmpcnt][j][curpos][i] : floor((p4d[tmpcnt][j][curpos][i]-tmpb_min)/tmpb*255.0));
						else
							tmpval += ((bIntensityRescale==false) ? p4d[tmpcnt][j][curpos][i] : p4d[tmpcnt][j][curpos][i]); //in this case, don't about any channel more than 3
					}
					tmpval /= sz3;
					tmpimg.setPixel(i, j, qRgb(int(tmpval), int(tmpval), int(tmpval)));
				}
			}
			break;

		case colorRed2Gray:
			for (j=0;j<sz2;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tg = tb = tr = (bIntensityRescale==false) ? p4d[0][j][curpos][i] : floor((p4d[0][j][curpos][i]-tmpr_min)/tmpr*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorGreenOnly:
			tb = tr = 0;
			for (j=0;j<sz2;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tg = (bIntensityRescale==false) ? p4d[1][j][curpos][i] : floor((p4d[1][j][curpos][i]-tmpg_min)/tmpg*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorGreen2Gray:
			for (j=0;j<sz2;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tb = tr = tg = (bIntensityRescale==false) ? p4d[1][j][curpos][i] : floor((p4d[1][j][curpos][i]-tmpg_min)/tmpg*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorBlueOnly:
			tg = tr = 0;
			for (j=0;j<sz2;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tb = (bIntensityRescale==false) ? p4d[2][j][curpos][i] : floor((p4d[2][j][curpos][i]-tmpb_min)/tmpb*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorBlue2Gray:
			for (j=0;j<sz2;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tg = tr = tb = (bIntensityRescale==false) ? p4d[2][j][curpos][i] : floor((p4d[2][j][curpos][i]-tmpb_min)/tmpb*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorRGB:
			for (j=0;j<sz2;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tr = (bIntensityRescale==false) ? p4d[0][j][curpos][i] : floor((p4d[0][j][curpos][i]-tmpr_min)/tmpr*255.0);
					tg = (bIntensityRescale==false) ? p4d[1][j][curpos][i] : floor((p4d[1][j][curpos][i]-tmpg_min)/tmpg*255.0);
					tb = (bIntensityRescale==false) ? p4d[2][j][curpos][i] : floor((p4d[2][j][curpos][i]-tmpb_min)/tmpb*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorRG:
			tb = 0;
			for (j=0;j<sz2;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tr = (bIntensityRescale==false) ? p4d[0][j][curpos][i] : floor((p4d[0][j][curpos][i]-tmpr_min)/tmpr*255.0);
					tg = (bIntensityRescale==false) ? p4d[1][j][curpos][i] : floor((p4d[1][j][curpos][i]-tmpg_min)/tmpg*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorUnknown:
		default:
			break;
	}

	return QPixmap::fromImage(tmpimg);
}

template <class T> QPixmap copyRaw2QPixmap_zPlanes(const T **** p4d,
												   V3DLONG sz0,
												   V3DLONG sz1,
												   V3DLONG sz2,
												   V3DLONG sz3,
												   ImageDisplayColorType Ctype,
												   V3DLONG cpos,
												   bool bIntensityRescale,
												   double *p_vmax,
												   double *p_vmin)
{
	QImage tmpimg = QImage(sz0, sz1, QImage::Format_RGB32);
	int tr,tg,tb;

	V3DLONG curpos = (cpos>sz2)?sz2-1:cpos-1;
	curpos = (curpos<0)?0:curpos;

	/*
	 unsigned char **p2dRed, **p2dGreen, **p2dBlue;
	 p2dBlue = (sz3<3) ? 0 : (unsigned char **)(p4d[2][curpos]);
	 p2dGreen = (sz3<2) ? 0 : (unsigned char **)(p4d[1][curpos]);
	 p2dRed = (sz3<1) ? 0 : (unsigned char **)(p4d[0][curpos]);
	 */

	V3DLONG i,j;
	double tmpr,tmpg,tmpb;
	double tmpr_min, tmpg_min, tmpb_min;

	if (sz3>=3)
	{
		tmpb = p_vmax[2]-p_vmin[2]; tmpb = (tmpb==0)?1:tmpb;
		tmpb_min = p_vmin[2];
	}

	if (sz3>=2)
	{
		tmpg = p_vmax[1]-p_vmin[1]; tmpg = (tmpg==0)?1:tmpg;
		tmpg_min = p_vmin[1];
	}

	if (sz3>=1)
	{
		tmpr = p_vmax[0]-p_vmin[0]; tmpr = (tmpr==0)?1:tmpr;
		tmpr_min = p_vmin[0];
	}

	//the wrong Ctype options should be disabled in the interface instead of a complicated logic management here

	switch (Ctype)
	{
		case colorRedOnly:
			tg = tb = 0;
			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tr = (bIntensityRescale==false) ? p4d[0][curpos][j][i] : floor((p4d[0][curpos][j][i]-tmpr_min)/tmpr*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorGray: //070716
			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz0;i++)
				{
					double tmpval=0;
					for (int tmpcnt=0;tmpcnt<sz3;tmpcnt++)
					{
						if (tmpcnt==0)
							tmpval += ((bIntensityRescale==false) ? p4d[tmpcnt][curpos][j][i] : floor((p4d[tmpcnt][curpos][j][i]-tmpr_min)/tmpr*255.0));
						else if (tmpcnt==1)
							tmpval += ((bIntensityRescale==false) ? p4d[tmpcnt][curpos][j][i] : floor((p4d[tmpcnt][curpos][j][i]-tmpg_min)/tmpg*255.0));
						else if (tmpcnt==2)
							tmpval += ((bIntensityRescale==false) ? p4d[tmpcnt][curpos][j][i] : floor((p4d[tmpcnt][curpos][j][i]-tmpb_min)/tmpb*255.0));
						else
							tmpval += ((bIntensityRescale==false) ? p4d[tmpcnt][curpos][j][i] : p4d[tmpcnt][curpos][j][i]); //in this case, don't about any channel more than 3
					}
					tmpval /= sz3;
					tmpimg.setPixel(i, j, qRgb(int(tmpval), int(tmpval), int(tmpval)));
				}
			}
			break;

		case colorRed2Gray:
			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tg = tb = tr = (bIntensityRescale==false) ? p4d[0][curpos][j][i] : floor((p4d[0][curpos][j][i]-tmpr_min)/tmpr*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorGreenOnly:
			tr = tb = 0;
			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tg = (bIntensityRescale==false) ? p4d[1][curpos][j][i] : floor((p4d[1][curpos][j][i]-tmpg_min)/tmpg*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorGreen2Gray:
			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tr = tb = tg = (bIntensityRescale==false) ? p4d[1][curpos][j][i] : floor((p4d[1][curpos][j][i]-tmpg_min)/tmpg*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorBlueOnly:
			tr = tg = 0;
			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tb = (bIntensityRescale==false) ? p4d[2][curpos][j][i] : floor((p4d[2][curpos][j][i]-tmpb_min)/tmpb*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorBlue2Gray:
			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tr = tg = tb = (bIntensityRescale==false) ? p4d[2][curpos][j][i] : floor((p4d[2][curpos][j][i]-tmpb_min)/tmpb*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorRGB:
			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tr = (bIntensityRescale==false) ? p4d[0][curpos][j][i] : floor((p4d[0][curpos][j][i]-tmpr_min)/tmpr*255.0);
					tg = (bIntensityRescale==false) ? p4d[1][curpos][j][i] : floor((p4d[1][curpos][j][i]-tmpg_min)/tmpg*255.0);
					tb = (bIntensityRescale==false) ? p4d[2][curpos][j][i] : floor((p4d[2][curpos][j][i]-tmpb_min)/tmpb*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorRG:
			tb = 0;
			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz0;i++)
				{
					tr = (bIntensityRescale==false) ? p4d[0][curpos][j][i] : floor((p4d[0][curpos][j][i]-tmpr_min)/tmpr*255.0);
					tg = (bIntensityRescale==false) ? p4d[1][curpos][j][i] : floor((p4d[1][curpos][j][i]-tmpg_min)/tmpg*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case colorUnknown:
		default:
			break;
	}
	/*
	 for (j=0;j<sz1;j++)
	 {
	 for (i=0;i<sz0;i++)
	 {
	 tr = (p2dRed) ? ((bIntensityRescale==false) ? p2dRed[j][i] : floor((p2dRed[j][i]-tmpr_min)/tmpr*255.0)) : 0;
	 tg = (p2dGreen) ? ((bIntensityRescale==false) ? p2dGreen[j][i] : floor((p2dGreen[j][i]-tmpg_min)/tmpg*255.0)) : 0;
	 tb = (p2dBlue) ? ((bIntensityRescale==false) ? p2dBlue[j][i] : floor((p2dBlue[j][i]-tmpb_min)/tmpb*255.0)) : 0;
	 tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
	 }
	 }
	 */
	return QPixmap::fromImage(tmpimg);
}

#define __copy_slice_from_volume__
template <class T> QPixmap copyRaw2QPixmap(const T **** p4d, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, ImageDisplayColorType Ctype, V3DLONG cpos, ImagePlaneDisplayType disType, bool bIntensityRescale, double *p_vmax, double *p_vmin)
{
#if ! USE_CHANNEL_TABLE
	switch (disType)
	{
		case imgPlaneX:
			return copyRaw2QPixmap_xPlanes(p4d, sz0, sz1, sz2, sz3, Ctype, cpos, bIntensityRescale, p_vmax, p_vmin);
			break;

		case imgPlaneY:
			return copyRaw2QPixmap_yPlanes(p4d, sz0, sz1, sz2, sz3, Ctype, cpos, bIntensityRescale, p_vmax, p_vmin);
			break;

		case imgPlaneZ:
			return copyRaw2QPixmap_zPlanes(p4d, sz0, sz1, sz2, sz3, Ctype, cpos, bIntensityRescale, p_vmax, p_vmin);
			break;

		default:
			printf("Undefined ImagePlaneDisplayType. Check your code.\n");
			return QPixmap(0,0); //return an empty image for this prohibited case
			break;
	}
#else
	//110718 RZC, test function for 4-channel volume data without indexed colormap
	return copyRaw2QPixmap_Slice(disType, cpos, p4d, sz0, sz1, sz2, sz3, Ctype, bIntensityRescale, p_vmax, p_vmin);
#endif
}

QPixmap copyRaw2QPixmap_colormap(const void **** p4d, ImagePixelType dtype, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cpos, const ColorMap *pc, ImagePlaneDisplayType disType)
{
	/*  if (Ctype!=colorPseudoMaskColor)
	 {
	 printf("This function is designed for colorPseudoMaskColor only. Check your code.\n");
	 return QPixmap(0,0); //return an empty image for this prohibited case
	 }
	 */
	switch (disType)
	{
		case imgPlaneX:
			return copyRaw2QPixmap_xPlanes_colormap(p4d, dtype, sz0, sz1, sz2, sz3, cpos, pc);
			break;

		case imgPlaneY:
			return copyRaw2QPixmap_yPlanes_colormap(p4d, dtype, sz0, sz1, sz2, sz3, cpos, pc);
			break;

		case imgPlaneZ:
			return copyRaw2QPixmap_zPlanes_colormap(p4d, dtype, sz0, sz1, sz2, sz3, cpos, pc);
			break;

		default:
			printf("Undefined ImagePlaneDisplayType. Check your code.\n");
			return QPixmap(0,0); //return an empty image for this prohibited case
			break;
	}
}

QPixmap copyRaw2QPixmap_xPlanes_colormap(const void **** p4d, ImagePixelType dtype, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cpos, const ColorMap *pc)
{
    if (!p4d || !pc)
	{
		printf("The input parameters are invalid in copyRaw2QPixmap_xPlanes_colormap()!\n");
		return QPixmap(sz2, sz1);
	}

	if (sz3<1)
	{
		printf("The number of color channels cannot be smalled than 1. Note that as an indexed color, only the first (0-th) dim is used. \n");
		return QPixmap(sz2, sz1);;
	}

	QImage tmpimg = QImage(sz2, sz1, QImage::Format_RGB32);
	int tr,tg,tb;
	int clen = pc->len;
	V3DLONG ind;

	V3DLONG curpos = (cpos>sz0) ? sz0-1 : cpos-1;
	curpos = (curpos<0)?0:curpos;

	V3DLONG i,j;

	float ***p3d_float32=0;
	USHORTINT16 ***p3d_uint16=0;
	unsigned char ***p3d_uint8=0;

	switch(dtype)
	{
		case V3D_UINT8:
			p3d_uint8 = (unsigned char ***)(p4d[0]);

			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz2;i++)
				{
					ind = V3DLONG(p3d_uint8[i][j][curpos]);
					if (ind>=clen) ind = ind % clen;
					tr = pc->map2d[ind][0];
					tg = pc->map2d[ind][1];
					tb = pc->map2d[ind][2];
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}

			break;

		case V3D_UINT16:
			p3d_uint16 = ((USHORTINT16 ****)p4d)[0];
			//printf("p3d=[%ld] sz1=%d sz2=%d\n", V3DLONG(p3d_uint16), sz1, sz2);

			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz2;i++)
				{
				    //printf("(i=%d,j=%d,curpos=%d) \n", i,j,curpos);
					ind = V3DLONG(p3d_uint16[i][j][curpos]);
					if (ind>=clen) ind = ind % clen;
					tr = pc->map2d[ind][0];
					tg = pc->map2d[ind][1];
					tb = pc->map2d[ind][2];
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}

			break;

		case V3D_FLOAT32:
			p3d_float32 = ((float ****)p4d)[0]; //(float ***)(p4d[0]);

			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz2;i++)
				{
					ind = V3DLONG(fabs(p3d_float32[i][j][curpos]));
					if (ind>=clen) ind = ind % clen;
					tr = pc->map2d[ind][0];
					tg = pc->map2d[ind][1];
					tb = pc->map2d[ind][2];
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}

			break;

		default:
			break;
	}

	return QPixmap::fromImage(tmpimg);
}

QPixmap copyRaw2QPixmap_yPlanes_colormap(const void **** p4d, ImagePixelType dtype, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cpos, const ColorMap *pc)
{
    if (!p4d || !pc)
	{
		printf("The input parameters are invalid in copyRaw2QPixmap_yPlanes_colormap()!\n");
		return QPixmap(sz0, sz2);
	}

	if (sz3<1)
	{
		printf("The number of color channels cannot be smalled than 1. Note that as an indexed color, only the first (0-th) dim is used. \n");
		return QPixmap(sz0, sz2);
	}

	QImage tmpimg = QImage(sz0, sz2, QImage::Format_RGB32);
	int tr,tg,tb;
	int clen = pc->len;
	V3DLONG ind;

	V3DLONG curpos = (cpos>sz1) ? sz1-1 : cpos-1;
	curpos = (curpos<0)?0:curpos;

	V3DLONG i,j;

	float ***p3d_float32;
	USHORTINT16 ***p3d_uint16;
	unsigned char ***p3d_uint8;

	switch (dtype)
	{
		case V3D_UINT8:
			p3d_uint8 = (unsigned char ***)(p4d[0]);

			for (j=0;j<sz2;j++)
			{
				for (i=0;i<sz0;i++)
				{
					ind = V3DLONG(p3d_uint8[j][curpos][i]);

					if (ind>=clen) ind = ind % clen;
					tr = pc->map2d[ind][0];
					tg = pc->map2d[ind][1];
					tb = pc->map2d[ind][2];
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case V3D_UINT16:
			p3d_uint16 = (USHORTINT16 ***)(p4d[0]);

			for (j=0;j<sz2;j++)
			{
				for (i=0;i<sz0;i++)
				{
					ind = V3DLONG(p3d_uint16[j][curpos][i]);

					if (ind>=clen) ind = ind % clen;
					tr = pc->map2d[ind][0];
					tg = pc->map2d[ind][1];
					tb = pc->map2d[ind][2];
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		case V3D_FLOAT32:
			p3d_float32 = (float ***)(p4d[0]);

			for (j=0;j<sz2;j++)
			{
				for (i=0;i<sz0;i++)
				{
					ind = V3DLONG(fabs(p3d_float32[j][curpos][i]));

					if (ind>=clen) ind = ind % clen;
					tr = pc->map2d[ind][0];
					tg = pc->map2d[ind][1];
					tb = pc->map2d[ind][2];
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		default:
			break;
	}

	return QPixmap::fromImage(tmpimg);
}

QPixmap copyRaw2QPixmap_zPlanes_colormap(const void **** p4d, ImagePixelType dtype, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cpos, const ColorMap *pc)
{
    if (!p4d || !pc)
	{
		printf("The input parameters are invalid in copyRaw2QPixmap_zPlanes_colormap()!\n");
		return QPixmap(sz0, sz1);
	}

	if (sz3<1)
	{
		printf("The number of color channels cannot be smalled than 1. Note that as an indexed color, only the first (0-th) dim is used. \n");
		return QPixmap(sz0, sz1);
	}

	QImage tmpimg = QImage(sz0, sz1, QImage::Format_RGB32);
	int tr,tg,tb;
	int clen = pc->len;
	V3DLONG ind;

	V3DLONG curpos = (cpos>sz2)?sz2-1:cpos-1;
	curpos = (curpos<0)?0:curpos;

	V3DLONG i,j;

	float ***p3d_float32;
	USHORTINT16 ***p3d_uint16;
	unsigned char ***p3d_uint8;

	switch (dtype)
	{
		case V3D_UINT8:
			p3d_uint8 = (unsigned char ***)(p4d[0]);

			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz0;i++)
				{
					ind = V3DLONG(p3d_uint8[curpos][j][i]);
					if (ind>=clen) ind = ind % clen;
					tr = pc->map2d[ind][0];
					tg = pc->map2d[ind][1];
					tb = pc->map2d[ind][2];
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));

				}
			}
			break;

		case V3D_UINT16:
			p3d_uint16 = (USHORTINT16 ***)(p4d[0]);

			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz0;i++)
				{
					ind = V3DLONG(p3d_uint16[curpos][j][i]);
					if (ind>=clen) ind = ind % clen;
					tr = pc->map2d[ind][0];
					tg = pc->map2d[ind][1];
					tb = pc->map2d[ind][2];
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));

				}
			}
			break;

		case V3D_FLOAT32:
			p3d_float32 = (float ***)(p4d[0]);

			for (j=0;j<sz1;j++)
			{
				for (i=0;i<sz0;i++)
				{
					ind = V3DLONG(fabs(p3d_float32[curpos][j][i]));
					if (ind>=clen) ind = ind % clen;
					tr = pc->map2d[ind][0];
					tg = pc->map2d[ind][1];
					tb = pc->map2d[ind][2];
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));

				}
			}
			break;

		default:
			break;
	}

	return QPixmap::fromImage(tmpimg);
}


bool getFocusCrossLinePos(int & focusPosInWidth, int & focusPosInHeight, My4DImage * imgData, ImagePlaneDisplayType Ptype)
{
	if (imgData==NULL)
		return false;

	if (imgData->isEmpty()==true)
		return false;

	switch (Ptype)
	{
		case imgPlaneX:
			focusPosInWidth = imgData->curFocusZ;
			focusPosInHeight = imgData->curFocusY;
			break;

		case imgPlaneY:
			focusPosInWidth = imgData->curFocusX;
			focusPosInHeight = imgData->curFocusZ;
			break;

		case imgPlaneZ:
			focusPosInWidth = imgData->curFocusX;
			focusPosInHeight = imgData->curFocusY;
			break;

		default:
			return false;
	}

	return (focusPosInWidth==-1 || focusPosInHeight==-1) ? false : true;
}


//====================================================================
const int alpha = 155;

#define ______XFormView_functions________

XFormView::XFormView(QWidget *parent)
//    : QWidget(parent)
: ArthurFrame(parent)
{

    setAttribute(Qt::WA_MouseTracking);

    Gtype = PixmapType;
    m_scale = 1.0;
	//  m_shear = 0.0;
	//  m_rotation = 0.0;
	myCursor = QCursor(Qt::OpenHandCursor);

    b_displayFocusCrossLine = true;

    Ptype = imgPlaneUndefined; //set -1 as a invalid type
    cur_focus_pos = 1;
	imgData = 0; //make a reference to the actual data
	Ctype = colorRGB; //set how to display RGB color. This setting can be changed later by the setCType() function
	Ctype_glass = colorUnknown;
	_for_index_only = false;

    // set a default map

	pixmap = QPixmap(256, 256);
	pixmap.fill(Qt::red);

    focusPosInWidth = pixmap.width()/2.0;
    focusPosInHeight = pixmap.height()/2.0;

	bMouseCurorIn = false;

    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	curDisplayCenter = QPoint(pixmap.width()/2.0, pixmap.height()/2.0);

	ind_landmarkToBeChanged=-1; //reset it initially
	b_moveCurrentLandmark = false; //reset it initially

	disp_scale = 1;
	disp_width = disp_scale * pixmap.width();
	disp_height = disp_scale * pixmap.height();

}

bool XFormView::internal_only_imgplane_op()
{
	if (!imgData || imgData->getCDim()<1) return false;
    ImagePixelType dtype;
  	unsigned char **** p4d = (unsigned char ****)imgData->getData(dtype);
	if (!p4d) return false;


	//qDebug("XFormView::internal_only_imgplane_op(), colorChanged(%d)", this->Ptype);
#if USE_CHANNEL_TABLE
	bool glass_on = (imgData->getFlagLookingGlass());
	if (this->_for_index_only == false)
	{
		// signal indirectly connected to ChannelTabWidget::updateXFormWidget(int plane)
		{
			emit colorChanged(this->Ptype);  //if isIndexColor(Ctype)), it will iterate back with (_for_index_only=true)
		}
		if (glass_on)
		{
			emit colorChangedGlass(this->Ptype); //if isIndexColor(Ctype_glass)), it will iterate back with (_for_index_only=true)
		}
		return true;   //has painted, skip old code
	}
	//follows only for (_for_index_only)
#endif


	if (dtype==V3D_UINT8)
	{
		if (imgData->getCDim()==1 && (Ctype==colorPseudoMaskColor || Ctype==colorArnimFlyBrainColor || Ctype==colorHanchuanFlyBrainColor))
		{
			if (imgData->colorMap)
			{
				pixmap = copyRaw2QPixmap_colormap(
												  (const void ****) p4d, dtype,
												  imgData->getXDim(),
												  imgData->getYDim(),
												  imgData->getZDim(),
												  imgData->getCDim(),
												  cur_focus_pos,
												  imgData->colorMap, Ptype);
			}
			else
			{
				v3d_msg("The image colormap has not been set yet (XFormView::setImgData()) for UINT8!\n");
				return false;
			}
		}
		else
		{
			pixmap = copyRaw2QPixmap(
									 (const unsigned char ****) p4d,
									 imgData->getXDim(),
									 imgData->getYDim(),
									 imgData->getZDim(),
									 imgData->getCDim(),
									 Ctype, cur_focus_pos, Ptype,
									 imgData->getFlagImgValScaleDisplay(),
									 imgData->p_vmax,
									 imgData->p_vmin);
		}
	}
	else if (dtype==V3D_UINT16)  // this part need to change in the future when I want to scale the UINT16 and display it as grayscale-256
	{
		if (imgData->getCDim()==1)
		{
			if (imgData->colorMap && (Ctype==colorPseudoMaskColor || Ctype==colorArnimFlyBrainColor || Ctype==colorHanchuanFlyBrainColor))
			{
				pixmap = copyRaw2QPixmap_colormap(
												  (const void ****) p4d,
												  dtype,
												  imgData->getXDim(),
												  imgData->getYDim(),
												  imgData->getZDim(),
												  imgData->getCDim(),
												  cur_focus_pos,
												  imgData->colorMap, Ptype);
			}
			else
			{
				pixmap = copyRaw2QPixmap(
										 (const unsigned short int ****) p4d,
										 imgData->getXDim(),
										 imgData->getYDim(),
										 imgData->getZDim(),
										 imgData->getCDim(),
										 Ctype, cur_focus_pos, Ptype,
										 imgData->getFlagImgValScaleDisplay(),
										 imgData->p_vmax,
										 imgData->p_vmin);
			}
		}
		else
		{
			pixmap = copyRaw2QPixmap(
									 (const unsigned short int ****) p4d,
									 imgData->getXDim(),
									 imgData->getYDim(),
									 imgData->getZDim(),
									 imgData->getCDim(),
									 Ctype, cur_focus_pos, Ptype,
									 imgData->getFlagImgValScaleDisplay(),
									 imgData->p_vmax,
									 imgData->p_vmin);
		}
	}
	else if (dtype==V3D_FLOAT32)
	{
		if (imgData->getCDim()==1)
		{
			if (imgData->colorMap && (Ctype==colorPseudoMaskColor || Ctype==colorArnimFlyBrainColor || Ctype==colorHanchuanFlyBrainColor))
			{
				pixmap = copyRaw2QPixmap_colormap(
												  (const void ****) p4d,
												  dtype,
												  imgData->getXDim(),
												  imgData->getYDim(),
												  imgData->getZDim(),
												  imgData->getCDim(),
												  cur_focus_pos,
												  imgData->colorMap, Ptype);
			}
			else
			{
				pixmap = copyRaw2QPixmap(
										 (const float ****) p4d,
										 imgData->getXDim(),
										 imgData->getYDim(),
										 imgData->getZDim(),
										 imgData->getCDim(),
										 Ctype, cur_focus_pos, Ptype,
										 imgData->getFlagImgValScaleDisplay(),
										 imgData->p_vmax,
										 imgData->p_vmin);
			}
		}
		else
		{
			pixmap = copyRaw2QPixmap(
									 (const float ****) p4d,
									 imgData->getXDim(),
									 imgData->getYDim(),
									 imgData->getZDim(),
									 imgData->getCDim(),
									 Ctype, cur_focus_pos, Ptype,
									 imgData->getFlagImgValScaleDisplay(),
									 imgData->p_vmax,
									 imgData->p_vmin);
		}
	}
	else
	{
		v3d_msg("Right now only support UINT8, UINT16, and FLOAT32.\n", 0);
		return false; //do nothing
	}

	return true;
}

void XFormView::setImgData(ImagePlaneDisplayType ptype, My4DImage * pdata, ImageDisplayColorType ctype)
{
	Ptype = ptype; //which type of the display cutting plane the pixmap is going to get from pdata image
    cur_focus_pos = 1;
	imgData = pdata; //make a reference to the actual data
	Ctype = ctype; //set how to display RGB color. This setting can be changed later by the setCType() function

    if (!imgData) //this is used to set the default pictures
	{
		switch (Ptype)
		{
			case imgPlaneX:
				pixmap = QPixmap(":/pic/default_yz_pic.jpg");
				break;
			case imgPlaneY:
				pixmap = QPixmap(":/pic/default_zx_pic.jpg");
				break;
			case imgPlaneZ:
				pixmap = QPixmap(":/pic/default_xy_pic.jpg");
				break;
			default:
				pixmap = QPixmap(":/pic/bg1.jpg");
				break;
		}
		v3d_msg(QString("The pixel map size is %1 %2 (for invalid imagedata)").arg(pixmap.width()).arg(pixmap.height()), 0);

		return;
	}

    ImagePixelType dtype;
  	unsigned char **** p4d = (unsigned char ****)imgData->getData(dtype);
  	if (!p4d)
  	{
	    //printf("invalid pointer address in XFormView::setImgData()\n");
	    switch (Ptype)
	    {
			case imgPlaneX:
				pixmap = QPixmap(":/pic/default_yz_pic.jpg");
				break;
			case imgPlaneY:
				pixmap = QPixmap(":/pic/default_zx_pic.jpg");
				break;
			case imgPlaneZ:
				pixmap = QPixmap(":/pic/default_xy_pic.jpg");
				break;
			default:
				pixmap = QPixmap(":/pic/bg1.jpg");
				break;
	    }

   	    //printf("The pixel map size is %d %d\n", pixmap.width(), pixmap.height());
	}
	else
	{
		internal_only_imgplane_op();
	}
}

QRect XFormView::getRoiBoundingRect()
{
	if (roiPolygon.count()<2) //in case the roiPolygon is not really specified
	{
		//return QRect(0,0, width(), height());
		return QRect(0,0, pixmap.width(), pixmap.height()); //081114
	}
	else
		return roiPolygon.boundingRect();
}

QPointF	 XFormView::mouseEventToImageCoords(const QPoint& p)
{
	return	QPointF(
		double((p.x()-curDisplayCenter.x()))/(disp_scale*m_scale)+pixmap.width()/2.0+0.5,
		double((p.y()-curDisplayCenter.y()))/(disp_scale*m_scale)+pixmap.height()/2.0+0.5
	);
}

void XFormView::mouseDoubleClickEvent(QMouseEvent * e)
{
    if (!imgData)
		return;

    QPoint cp = mouseEventToImageCoords(e->pos()).toPoint();

	int sx,sy,sz; //current selection location's x,y,z

	switch(Ptype)
	{
		case imgPlaneZ:
		    sx = cp.x(); sy = cp.y(); sz = imgData->curFocusZ;
		    break;

		case imgPlaneX:
		    sz = cp.x(); sy = cp.y(); sx = imgData->curFocusX;
		    break;

		case imgPlaneY:
		    sx = cp.x(); sz = cp.y(); sy = imgData->curFocusY;
		    break;

		default:
		    return;
			break;
	}

	int rr = 10;

	double dmin;
	V3DLONG ind_min;

	int tmpx,tmpy,tmpz;
	LocationSimple tmpLocation(0,0,0);
	double dx,dy,dz, dd;
	int b_find = 0;
	for (V3DLONG i=0; i<imgData->listLandmarks.count();i++)
	{
		tmpLocation = imgData->listLandmarks.at(i);
		tmpLocation.getCoord(tmpx,tmpy,tmpz);
		if (fabs(dx=tmpx-sx)>rr || fabs(dy=tmpy-sy)>rr || fabs(dz=tmpz-sz)>rr) //use a simple criterion to filter point first
			continue;
		if ((dd=dx*dx+dy*dy+dz*dz)>rr*rr)
			continue;

		if (b_find==0)
		{
			b_find=1;
			dmin = dd;
			ind_min = i;
		}
		else
		{
			if (dd<dmin)
			{
				dmin = dd;
				ind_min = i;
			}
		}
	}

	if (b_find==1)
	{
		QString tmps("Change Landmark [");
		QString v1,v2,v3;
		tmpLocation = imgData->listLandmarks.at(ind_min);
		tmpLocation.getCoord(tmpx,tmpy,tmpz);
		v1.setNum(tmpx+1); v1.append(",");
		v2.setNum(tmpy+1); v2.append(",");
		v3.setNum(tmpz+1); v3.append("]");
		tmps.append(v1);
		tmps.append(v2);
		tmps.append(v3);

        QMessageBox mb(tmps, //tr("Landmark change: []"),
					   "Change (delete) the selected marker?",
					   QMessageBox::Question,
					   QMessageBox::Yes | QMessageBox::Default,
					   QMessageBox::No,
					   QMessageBox::Cancel | QMessageBox::Escape);
		mb.addButton(QMessageBox::Open);

        mb.setButtonText(QMessageBox::Yes, "Move");
        mb.setButtonText(QMessageBox::No, "Delete");
        mb.setButtonText(QMessageBox::Open, "Edit properties");
        mb.setButtonText(QMessageBox::Cancel, "Do nothing");

        switch(mb.exec()) {
            case QMessageBox::Yes:
				//move the location: 080101
				setMouseTracking(false); //"false" should be the default, but I set it again here to assure the mouseMove event is captured only when one mouse button is pressed
				b_moveCurrentLandmark=true;
				ind_landmarkToBeChanged=ind_min;
				printf("landmark to change=[%ld] start\n", ind_landmarkToBeChanged);

				//imgData->listLandmarks.removeAt(ind_min);
				//imgData->setFocusFeatureViewText();
				break;

			case QMessageBox::No:
				//delete the current landmark
				imgData->listLandmarks.removeAt(ind_min);
				imgData->setFocusFeatureViewText();
				break;

			case QMessageBox::Open:
			{
				LandmarkPropertyDialog *landmarkView = NULL;
				landmarkView = new LandmarkPropertyDialog(&(imgData->listLandmarks), ind_min, imgData);
				if (landmarkView->exec()!=QDialog::Accepted)
				{
					if (landmarkView) {delete landmarkView; landmarkView = NULL;}
					break; //only return true when the results are accepted, which will lead to an update operation below
				}
				landmarkView->fetchData(&(imgData->listLandmarks), ind_min);
				qDebug("edit landmark [%ld]. data fetched [%s][%s][%d]", ind_min,
					   imgData->listLandmarks.at(ind_min).name.c_str(), imgData->listLandmarks.at(ind_min).comments.c_str(),  int(imgData->listLandmarks.at(ind_min).shape));

				//inportant: set the shape of the landmark
				LocationSimple * p_tmp_location = (LocationSimple *) & (imgData->listLandmarks.at(ind_min));
				switch (p_tmp_location->shape)
				{
					case pxSphere:	p_tmp_location->inputProperty = pxLocaUseful; break;
					case pxCube: p_tmp_location->inputProperty = pxLocaNotUseful; break;
					default: p_tmp_location->inputProperty = pxLocaUnsure; break;
				}

				if (landmarkView) {delete landmarkView; landmarkView = NULL;}
			}
				break;

			case QMessageBox::Cancel:
				//do nothing
				break;
		}
	}
}

void XFormView::mousePressEvent(QMouseEvent *e)
{
	switch (e->button())
	{
		case Qt::LeftButton:
			mouseLeftButtonPressEvent(e);
			break;

		case Qt::RightButton:
			mouseRightButtonPressEvent(e);
			break;

		default:
			break;
	}
}


void XFormView::mouseRightButtonPressEvent(QMouseEvent *e)
{
	//#define USE_POPUP_MENU_RIGHT
#ifdef USE_POPUP_MENU_RIGHT
	//printf("  ... [x=%d y=%d]\n", e->x(), e->y());
	if (QApplication::keyboardModifiers()==Qt::ShiftModifier) //080314 to pop up the menu
	{
		popupImageProcessingDialog();
	}
	else
	{
#endif
		QPoint	eImgCoords = mouseEventToImageCoords(e->pos()).toPoint();

		switch (Ptype)
		{
			case imgPlaneX:
				emit focusZChanged(eImgCoords.x());
				emit focusYChanged(eImgCoords.y());
				break;

			case imgPlaneY:
				emit focusXChanged(eImgCoords.x());
				emit focusZChanged(eImgCoords.y());
				break;

			case imgPlaneZ:
				emit focusXChanged(eImgCoords.x());
				emit focusYChanged(eImgCoords.y());
				//qDebug()<<"x="<<e->x()<<"y="<<e->y()<<"disp_scale="<<disp_scale<<"m_scale="<<m_scale<<"curdispcenter.x="<<curDisplayCenter.x()<<"curdispcenter.y="<<curDisplayCenter.y();

				break;

			default: //do nothing
				break;
		}
#ifdef USE_POPUP_MENU_RIGHT
	}
#endif
}

void XFormView::mouseLeftButtonPressEvent(QMouseEvent *e) //080101
{
	//reserved for future editing of the the pop-up menu
	if (QApplication::keyboardModifiers()==Qt::ControlModifier)
	{
		//add zoom-in support. by PHC 20101119
		QPoint	cp = mouseEventToImageCoords(e->pos()).toPoint();

		roiPolygon << cp;
		update();
	}
	else if (QApplication::keyboardModifiers()==Qt::AltModifier) //100816
	{
		if (roiPolygon.count()>0)
		{
			roiPolygon.pop_back();
			update();
		}
	}
	else if (b_moveCurrentLandmark==false && QApplication::keyboardModifiers()==Qt::ShiftModifier) //add to define marker list, by PHC, 20120702
    {
		QPoint cp = mouseEventToImageCoords(e->pos()).toPoint();

		int sx,sy,sz; //current selection location's x,y,z

		switch(Ptype)
		{
			case imgPlaneZ:
				sx = cp.x(); sy = cp.y(); sz = imgData->curFocusZ;
				break;

			case imgPlaneX:
				sz = cp.x(); sy = cp.y(); sx = imgData->curFocusX;
				break;

			case imgPlaneY:
				sx = cp.x(); sz = cp.y(); sy = imgData->curFocusY;
				break;

			default:
				return;
				break;
		}

		//LocationSimple tmpLocation(sx,sy,sz);
		//tmpLocation.inputProperty = imgData->listLandmarks.at(ind_landmarkToBeChanged).inputProperty;
		//tmpLocation.radius = imgData->listLandmarks.at(ind_landmarkToBeChanged).radius;

        //the following 4 lines are suggested by Carlos Becker to replace the above three lines, 111004 to preserve the comments and other info when a marker is moved
        LocationSimple tmpLocation;
        tmpLocation.x = sx;
        tmpLocation.y = sy;
        tmpLocation.z = sz;

        //20141029. add by PHC
        QString tmp_label;
        tmpLocation.name = qPrintable(tmp_label.setNum(imgData->listLandmarks.count()+1).prepend("landmark "));
        tmpLocation.radius = imgData->getXWidget()->getMainControlWindow()->global_setting.default_marker_radius; //add a default landmark size

        imgData->listLandmarks.append(tmpLocation);
		v3d_msg(QString("Add new marker at location %1 %2 %3\n").arg(sx).arg(sy).arg(sz), 0);

		//update();
	    imgData->updateViews();
    }
	else if (b_moveCurrentLandmark==true && ind_landmarkToBeChanged>=0 && QApplication::keyboardModifiers()==Qt::ShiftModifier)
	{
		QPoint cp = mouseEventToImageCoords(e->pos()).toPoint();

		int sx,sy,sz; //current selection location's x,y,z

		switch(Ptype)
		{
			case imgPlaneZ:
				sx = cp.x(); sy = cp.y(); sz = imgData->curFocusZ;
				break;

			case imgPlaneX:
				sz = cp.x(); sy = cp.y(); sx = imgData->curFocusX;
				break;

			case imgPlaneY:
				sx = cp.x(); sz = cp.y(); sy = imgData->curFocusY;
				break;

			default:
				return;
				break;
		}

		//LocationSimple tmpLocation(sx,sy,sz);
		//tmpLocation.inputProperty = imgData->listLandmarks.at(ind_landmarkToBeChanged).inputProperty;
		//tmpLocation.radius = imgData->listLandmarks.at(ind_landmarkToBeChanged).radius;

        //the following 4 lines are suggested by Carlos Becker to replace the above three lines, 111004 to preserve the comments and other info when a marker is moved
        LocationSimple tmpLocation = imgData->listLandmarks.at(ind_landmarkToBeChanged);
        tmpLocation.x = sx;
        tmpLocation.y = sy;
        tmpLocation.z = sz;

        imgData->listLandmarks.replace(ind_landmarkToBeChanged, tmpLocation);
		imgData->setFocusFeatureViewText();

		imgData->b_proj_worm_mst_diameter_set = false; //080318: whenever a landmark's location has been changed, reset the flag of MST diameter existency

		printf("end moving point [%ld].\n", ind_landmarkToBeChanged);
		ind_landmarkToBeChanged=-1; //reset it after the updating
		b_moveCurrentLandmark = false; //reset it after the updating

		//update();
	    imgData->updateViews();
	}
	else if (m_scale>1) //then interpret the press as a drag operation
	{
		dragStartPosition = e->pos();
		curDisplayCenter0 = curDisplayCenter;
	    setCursor(myCursor);
	}
}

void XFormView::mouseMoveEvent (QMouseEvent * e)
{
	//  curMousePos = e->pos() + QPoint(1,1);
	curMousePos = e->pos()/disp_scale;

    //090212. for panning
    if (m_scale>1)
    {
       if ((e->buttons() & Qt::LeftButton))
       {
            {
                 setCursor(myCursor); //maybe repeated set? is this necessary?

                 curDisplayCenter = curDisplayCenter0 + QPointF(curMousePos.x()*disp_scale-dragStartPosition.x(), curMousePos.y()*disp_scale-dragStartPosition.y());
                 //qDebug()<<curDisplayCenter.x()<<" "<<curDisplayCenter.y();

                 if (curDisplayCenter.x() < (2-m_scale)*disp_width/2.0-1)
                      curDisplayCenter.setX((2-m_scale)*disp_width/2.0-1);
                 else if (curDisplayCenter.x() > m_scale*disp_width/2.0)
                      curDisplayCenter.setX(m_scale*disp_width/2.0);

                 if (curDisplayCenter.y() < (2-m_scale)*disp_height/2.0-1)
                      curDisplayCenter.setY((2-m_scale)*disp_height/2.0-1);
                 else if (curDisplayCenter.y() > m_scale*disp_height/2.0)
                      curDisplayCenter.setY(m_scale*disp_height/2.0);
            }
       }
    }

	update();
}

void XFormView::enterEvent (QEvent * e)
{
	bMouseCurorIn = true;
}

void XFormView::leaveEvent (QEvent * e)
{
	bMouseCurorIn = false;
	update();
}

void XFormView::deleteROI()
{
	roiPolygon.clear();
}

/*
 void XFormView::wheelEvent(QWheelEvent *e) //need refine later
 {
 m_scale += e->delta()/120;  //most mouse works 15 degrees * 8
 //m_scale = qMax(1, qMin(4.0, m_scale));
 emit scaleChanged(int(m_scale*1));
 }
 */

void XFormView::paint(QPainter *p)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    switch (Gtype) {
		case VectorType:
			//        drawVectorType(p);
			break;
		case PixmapType:
			drawPixmapType(p);
			break;
		case TextType:
			//        drawTextType(p);
			break;
    }
    p->restore();
}

void XFormView::setPixmapType()
{
    Gtype = PixmapType;
    update();
}


void XFormView::changeScale(int s)
{
    m_scale = double(s/4.0);
	if (m_scale<=1)
		curDisplayCenter = QPointF(disp_width/2.0, disp_height/2.0); //reset the center
    update();
}

void XFormView::changeFocusPlane(int c)
{
    if (!imgData)
		return;

    ImagePixelType dtype;
    unsigned char ****p4d = (unsigned char ****)imgData->getData(dtype);
	if (!p4d)
		return;

    cur_focus_pos = c;

	switch (Ptype)
	{
		case imgPlaneX: imgData->setFocusX(cur_focus_pos); break;
		case imgPlaneY: imgData->setFocusY(cur_focus_pos); break;
		case imgPlaneZ: imgData->setFocusZ(cur_focus_pos); break;
		default: break; //do nothing
	}

	internal_only_imgplane_op(); //110727 RZC, moved to rear of switch(Ptype)


	//update the focus cross lines in other two views
	if (imgData->getFlagLinkFocusViews()==true) //otherwise do not update the two other views
	{
		if (imgData->get_xy_view()!=this)
			imgData->get_xy_view()->update();
		if (imgData->get_yz_view()!=this)
			imgData->get_yz_view()->update();
		if (imgData->get_zx_view()!=this)
			imgData->get_zx_view()->update();
	}


	imgData->setFocusFeatureViewText();

    //update the current view
    update();
}

#define __paint_plane_for_color_type___

void XFormView::changeColorType(ImageDisplayColorType c, bool bGlass) //110803 RZC, add bGlass
{
    if (! bGlass)
    	Ctype = c;
    else
    	Ctype_glass = c;
    if (c==colorUnknown)  return; //110725 RZC, just skip back to continue new code


    if (!imgData)
		return;

    /* the following three sentences are used to assue the display is updated */
    ImagePixelType dtype;
    unsigned char ****p4d = (unsigned char ****)imgData->getData(dtype);

	if (!p4d)
		return;

	//110804 RZC
	QPixmap tmap;
	ImageDisplayColorType tc;
	if (isIndexColor(c))  this->_for_index_only=true;
	if (bGlass && isIndexColor(Ctype_glass)) { tmap=pixmap; tc=Ctype;Ctype=Ctype_glass;}
	{
		internal_only_imgplane_op();
	}
	if (isIndexColor(c))  this->_for_index_only=false;
	if (bGlass && isIndexColor(Ctype_glass)) {pixmap_glass=pixmap;pixmap=tmap; Ctype=tc;}

	update();
}


void XFormView::reset()
{
    emit scaleChanged(4);
	//	emit focusPlaneChanged(1);
}

void XFormView::wheelEvent(QWheelEvent * e) //add this on 2008-01-10
{
    int numDegrees = -e->delta() / 8; //change to -e on 080121
    int numSteps = numDegrees / 15;

	if (!imgData) return;
	if (imgData->isEmpty()) return;

	switch (Ptype)
	{
		case imgPlaneX:
			emit focusXChanged(imgData->curFocusX+1+numSteps); //the first +1 to convert to the range [1, max]. The below is the same.
			break;
		case imgPlaneY:
			emit focusYChanged(imgData->curFocusY+1+numSteps);
			break;
		case imgPlaneZ:
			emit focusZChanged(imgData->curFocusZ+1+numSteps);
			break;
		default: //do nothing
			break;
	}

	return;
}

//void XFormView::dragMoveEvent(QDragMoveEvent * e)
//{
//}

void XFormView::do_keyPressEvent(QKeyEvent * e)  //by PHC, rename to do_keyPressEvent() on 100816 so that avoid conflict with XFormWidget keyPressEvent().
												//it seems no need to keep the left,right,up,down,B.,etc key events, as it is easy to do using mouse
{
    //two temp variables for pop-up dialog
   	QStringList items;
    QString item;

    double stepx = 1, stepy = 1; //default size is 1 pixel by pixel
	//qDebug()<<"init: "<<stepx<<" "<<stepy;

	//printf("[%d]\n",e->modifiers()); //don't know why this cause a crash!!

    //if (e->modifiers()==Qt::ShiftModifier) //note that e->modifiers() does not work!!!

	if (!imgData) return;
	if (imgData->isEmpty()) return;

	switch (e->key())
	{
		case Qt::Key_S:
		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
		    {
				imgData->getXWidget()->saveData();
			}
			break;

		case Qt::Key_Left: //for unknown reason, QT just do not recognize the combination of keymodifier and arrow!. by PHC, 090211.
			if (QApplication::keyboardModifiers()==Qt::ControlModifier) //then scroll page by page
			{
				stepx = pixmap.width()/2.0, stepy = pixmap.height()/2.0;
				qDebug()<<"ctrl pressed: "<<stepx<<" "<<stepy;
			}
			if (m_scale>1)
			{
				curDisplayCenter -= QPointF(stepx, 0);
				if (curDisplayCenter.x() < (2-m_scale)*pixmap.width()/2.0-1)
					curDisplayCenter.setX((2-m_scale)*pixmap.width()/2.0-1);

				update();
		    }
	  		break;

		case Qt::Key_Right:
			if (QApplication::keyboardModifiers()==Qt::ControlModifier) //then scroll page by page
			{
				stepx = pixmap.width()/2.0, stepy = pixmap.height()/2.0;
				qDebug()<<"ctrl pressed: "<<stepx<<" "<<stepy;
			}
			if (m_scale>1)
			{
				curDisplayCenter += QPointF(stepx, 0);
				if (curDisplayCenter.x() > m_scale*pixmap.width()/2.0)
					curDisplayCenter.setX(m_scale*pixmap.width()/2.0);

				update();
		    }
			break;

		case Qt::Key_Up:
			if (QApplication::keyboardModifiers()==Qt::ControlModifier) //then scroll page by page
			{
				stepx = pixmap.width()/2.0, stepy = pixmap.height()/2.0;
				qDebug()<<"ctrl pressed: "<<stepx<<" "<<stepy;
			}
			if (m_scale>1)
			{
				curDisplayCenter -= QPointF(0, stepy);
				if (curDisplayCenter.y() < (2-m_scale)*pixmap.height()/2.0-1)
					curDisplayCenter.setY((2-m_scale)*pixmap.height()/2.0-1);

				update();
		    }
			break;

		case Qt::Key_Down:
			//case Qt::Key_8: //a test to show normal key modifier works!
			if (QApplication::keyboardModifiers()==Qt::ControlModifier) //then scroll page by page
			{
				stepx = pixmap.width()/2.0, stepy = pixmap.height()/2.0;
				qDebug()<<"ctrl pressed: "<<stepx<<" "<<stepy;
			}
			if (m_scale>1)
			{
				//qDebug()<<"real stepx="<<stepx<<" stepy="<<stepy;
				curDisplayCenter += QPointF(0, stepy);
				if (curDisplayCenter.y() > m_scale*pixmap.height()/2.0)
					curDisplayCenter.setY(m_scale*pixmap.height()/2.0);

				update();
		    }
			break;

		case Qt::Key_N:
		case Qt::Key_Period: //080403
			switch (Ptype)
		{
			case imgPlaneX:
				emit focusXChanged(imgData->curFocusX+1+1); //the first +1 to convert to the range [1, max]. The below is the same.
				break;
			case imgPlaneY:
				emit focusYChanged(imgData->curFocusY+1+1);
				break;
			case imgPlaneZ:
				emit focusZChanged(imgData->curFocusZ+1+1);
				break;
			default: //do nothing
				break;
		}
			break;

		case Qt::Key_B: //add 'b' on 080109
		case Qt::Key_Comma: //080403
			switch (Ptype)
		{
			case imgPlaneX:
				emit focusXChanged(imgData->curFocusX+1-1);
				break;
			case imgPlaneY:
				emit focusYChanged(imgData->curFocusY+1-1);
				break;
			case imgPlaneZ:
				emit focusZChanged(imgData->curFocusZ+1-1);
				break;
			default: //do nothing
				break;
		}
			break;

		case Qt::Key_I:
			imgData->getXWidget()->triview_zoomin();
			break;

		case Qt::Key_O:
		{
			bool result = (
						   imgData->getXWidget()->disp_zoom * imgData->getXDim() <= 1 ||
						   imgData->getXWidget()->disp_zoom * imgData->getYDim() <= 1 ||
						   imgData->getXWidget()->disp_zoom * imgData->getZDim() <= 1 );

			if ( result )
			{
				v3d_msg("Cannot zoom-out more, - one of the first 3 dims of the images has been displayed to <=1 pixel on the monitor.");
				break;
			}
			imgData->getXWidget()->triview_zoomout();
			break;
		}

		case Qt::Key_1:
			imgData->getXWidget()->triview_zoom1();
			break;

		case Qt::Key_2:
			imgData->getXWidget()->triview_zoom2();
			break;

			//the following is another way to activate the pop-up menu or point-definition dialog at the pixel location. by PHC, 060312

#if COMPILE_TARGET_LEVEL != 0
/*		case Qt::Key_M:
		{
			//first search if a landmark has been defined at the same location. If yes, modify that one. Otherwise add a new one.

			QList <LocationSimple> * tmplist = (QList <LocationSimple> *) &(imgData->listLandmarks);
			int tmprownum; bool b_landmark_exist=false;
			int cx = imgData->curFocusX+1, cy = imgData->curFocusY+1, cz = imgData->curFocusZ+1;
			for (tmprownum=0;tmprownum<tmplist->count();tmprownum++)
			{
				if (int(tmplist->at(tmprownum).x)==cx && int(tmplist->at(tmprownum).y)==cy && int(tmplist->at(tmprownum).z)==cz)
				{
					b_landmark_exist=true;
					printf("detected existing landmark no=[%d]\n", tmprownum);
					break;
				}
			}

			LandmarkPropertyDialog *landmarkView = NULL;
			if (!landmarkView)
			{
				if (b_landmark_exist)
					landmarkView = new LandmarkPropertyDialog(tmplist, tmprownum, imgData);
				else
				{
					LocationSimple tmp_location(cx, cy, cz);
					//tmp_location.order = imgData->listLandmarks.count()+1;
					QString tmp_label = "";
					tmp_location.name = qPrintable(tmp_label.setNum(imgData->listLandmarks.count()+1).prepend("landmark "));
					tmp_location.radius = imgData->getXWidget()->getMainControlWindow()->global_setting.default_marker_radius; //add a default landmark size
					QList <LocationSimple> tmplist_1;
					tmplist_1.append(tmp_location);
					landmarkView = new LandmarkPropertyDialog(&tmplist_1, 0, imgData);
				}
			}

			int res = landmarkView->exec(); //note that as I request the user must either accept or change the cell property, I set it as a Modal dialog by calling exec() instead of show.
			if (res!=QDialog::Accepted)
			{
				if (landmarkView) {delete landmarkView; landmarkView = NULL;}
				break; //only return true when the results are accepted, which will lead to an update operation below
			}

			//update the current item
			if (!b_landmark_exist)
			{
				LocationSimple tmp_location(cx, cy, cz);
				//QString tmp_label = ""; tmp_location.name = qPrintable(tmp_label.setNum(imgData->listLandmarks.count()+1).prepend("landmark ")); //no need to do again, as the content will be overwritten anyway
				imgData->listLandmarks.append(tmp_location);
				tmprownum = imgData->listLandmarks.count()-1;
			}
			landmarkView->fetchData(&(imgData->listLandmarks), tmprownum);
			qDebug("data fetched [%s][%s] shape=[%d] radius=[%5.3f]",
				   imgData->listLandmarks.at(tmprownum).name.c_str(), imgData->listLandmarks.at(tmprownum).comments.c_str(),  int(imgData->listLandmarks.at(tmprownum).shape),  float(imgData->listLandmarks.at(tmprownum).radius));

			//important: set the shape of the landmark
			LocationSimple * p_tmp_location = (LocationSimple *) & (imgData->listLandmarks.at(tmprownum));
			switch (p_tmp_location->shape)
			{
				case pxSphere:	p_tmp_location->inputProperty = pxLocaUseful; //qDebug("pxsphere");
					break;
				case pxCube: p_tmp_location->inputProperty = pxLocaNotUseful; //qDebug("pxcube");
					break;
				default: p_tmp_location->inputProperty = pxLocaUnsure; //qDebug("%d pxunsure", int(p_tmp_location->shape));
					break;
			}

			if (landmarkView) {delete landmarkView; landmarkView = NULL;}
		}
			break;

		case Qt::Key_H:
		    dispHistogram();//in the future I can add a parameter to indicate the current view-id, so that I can only display the histogram of the current view (slice) instead of the whole stack
			break;

		case Qt::Key_C:
		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
		    {
				popupImageProcessingDialog(tr(" -- crop image using minMax bounding box in 3D (derived from ROIs)"));
            }
			else if (QApplication::keyboardModifiers()==Qt::ShiftModifier)
			{
				if (imgData->getCDim()!=1) break;
				if(imgData->getDatatype()!=V3D_UINT16 && imgData->getDatatype()!=V3D_UINT8) //only work for UINT16/UINT8 1 channel data
					break;
				imgData->getXWidget()->switchMaskColormap();
			}
			break;
*/
		case Qt::Key_R:
		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
		    {
				Options_Rotate tmpopt;
				tmpopt.degree=0.0;
				tmpopt.b_keepSameSize=true;
				tmpopt.fillcolor=0;
				tmpopt.center_x = (imgData->getXDim()-1.0)/2;
				tmpopt.center_y = (imgData->getYDim()-1.0)/2;
				tmpopt.center_z = (imgData->getZDim()-1.0)/2;

				Dialog_Rotate tmpdlg;
				tmpdlg.setContents(tmpopt);

				int dlg_res = tmpdlg.exec();
				if (dlg_res)
				{
					tmpdlg.getContents(tmpopt);
					imgData->rotate(Ptype, tmpopt);
				}
			}
			break;

		case Qt::Key_D: //remove the last pos from roiVertexList
		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
		    {
				//roiPolygon.erase(roiPolygon.end()-1);
				if (roiPolygon.count()>0)
				{
					roiPolygon.pop_back();
					update();
				}
			}
			break;
#endif


#if COMPILE_TARGET_LEVEL == 2
		case Qt::Key_P:
		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
		    {
				popupImageProcessingDialog();
			}
			break;
#endif

#if COMPILE_TARGET_LEVEL != 0
		case Qt::Key_V:
//			if(imgData->getDatatype()!=V3D_UINT8) //only work for UINT8 data
//			{
//				v3d_msg("Your data type is not UINT8 yet, - you will need to convert to UINT8 to see the data in 3D. Go to main menu \"Image/Data\" -> \"Image type\" to convert.");
//				break;
//			}

		    if (QApplication::keyboardModifiers()==Qt::ControlModifier) //launch the full-image 3d view
		    {
				imgData->getXWidget()->doImage3DView(true); //use the maximum display 512x512x256
			}
			else if (QApplication::keyboardModifiers()==Qt::ShiftModifier) //launch the local zoom-in view
			{
				imgData->getXWidget()->doImage3DLocalRoiView();
			}
			else if (QApplication::keyboardModifiers()==(Qt::ShiftModifier | Qt::ControlModifier)) //display the real size of an image. This may crash if not enough memory is available
			{
				if(QMessageBox::question (0, "",
										  "You have just requested displaying 3D view for an image using the full resolution. "
										  "If your machine does not have enough video memory, you may have a crash in the video memory. "
										  "Are you sure you want to continue?",
										  QMessageBox::Yes, QMessageBox::No)
				   == QMessageBox::Yes)
				{
					imgData->getXWidget()->doImage3DView(false);
				}
			}
			break;
#endif

#if COMPILE_TARGET_LEVEL == 2

#ifdef _ALLOW_ATLAS_IMAGE_MENU_
		case Qt::Key_A: //activate the atlas viewer
		case Qt::Key_F: //activate the find/search function
		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
		    {

				imgData->getXWidget()->launchAtlasViewer();
			}
			break;
#endif

#ifdef _ALLOW_NEURONSEG_MENU_
		case Qt::Key_T:
			popupImageProcessingDialog(tr(" -- trace between two locations"));
 			break;

		case Qt::Key_Z: //undo the last tracing step if possible. by PHC, 090120
		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
		    {
		    	if (imgData)
					imgData->getXWidget()->popupImageProcessingDialog(tr(" -- undo last tracing step"));
			}
			break;
#endif

#ifdef _ALLOW_IMGREG_MENU_
		case Qt::Key_W:
			popupImageProcessingDialog(tr(" -- Match one single landmark in another image"));
 			break;

		case Qt::Key_E:
		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
		    {
				bool ok;
				if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod==(int)MATCH_INTENSITY)
					item = tr("MATCH_INTENSITY");
				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod==(int)MATCH_CORRCOEF)
					item = tr("MATCH_CORRCOEF");
				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod==(int)MATCH_MI)
					item = tr("MATCH_MI");
				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod==(int)MATCH_IMOMENT)
					item = tr("MATCH_IMOMENT");
				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod==(int)MATCH_MEANOFCIRCLES)
					item = tr("MATCH_MEANOFCIRCLES");
				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod==(int)MATCH_MULTIPLE_MI_INT_CORR)
					item = tr("MATCH_MULTIPLE_MI_INT_CORR");
				else
					item = tr("Undefined");

				if(QMessageBox::Yes == QMessageBox::question (0, "", tr("Your current landmark matching method is [ ") + item + tr("]<br> Do you change?"), QMessageBox::Yes, QMessageBox::No))
				{
					items << tr("MATCH_MI") << tr("MATCH_MULTIPLE_MI_INT_CORR") << tr("MATCH_INTENSITY") << tr("MATCH_CORRCOEF") << tr("MATCH_IMOMENT") << tr("MATCH_MEANOFCIRCLES");
					item = QInputDialog::getItem(this, tr(""), tr("Please select a landmark matching method"), items, 0, false, &ok);
					if (ok && !item.isEmpty())
					{
						if (item==tr("MATCH_INTENSITY"))
							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod = (int)MATCH_INTENSITY;
						else if (item==tr("MATCH_CORRCOEF"))
							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod = (int)MATCH_CORRCOEF;
						else if (item==tr("MATCH_MI"))
							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod = (int)MATCH_MI;
						else if (item==tr("MATCH_IMOMENT"))
							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod = (int)MATCH_IMOMENT;
						else if (item==tr("MATCH_MEANOFCIRCLES"))
							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod = (int)MATCH_MEANOFCIRCLES;
						else if (item==tr("MATCH_MULTIPLE_MI_INT_CORR"))
							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod = (int)MATCH_MULTIPLE_MI_INT_CORR;
						else
							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod = (int)MATCH_MI;
					}
				}
			}
			else if (QApplication::keyboardModifiers()==Qt::ShiftModifier)
			{
				bool ok;
				if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method==(int)DF_GEN_TPS)
					item = tr("TPS");
				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method==(int)DF_GEN_HIER_B_SPLINE)
					item = tr("Hier-B-Spline");
				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method==(int)DF_GEN_TPS_B_SPLINE)
					item = tr("TPS-B-Spline-interpolation");
				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method==(int)DF_GEN_TPS_LINEAR_INTERP)
					item = tr("TPS-linear-interpolation");
				else
					item = tr("Undefined");

				if(QMessageBox::Yes == QMessageBox::question (0, "", tr("Your current displacement field computing method is [ ") + item + tr("]<br> Do you change?"), QMessageBox::Yes, QMessageBox::No))
				{
					items << tr("TPS-linear-interpolation") <<  tr("TPS-B-Spline-interpolation") << tr("TPS") << tr("Hier-B-Spline");
					item = QInputDialog::getItem(this, tr(""), tr("Please select a displacement filed (DF) computing method"), items, 0, false, &ok);
					if (ok && !item.isEmpty())
					{
						if (item==tr("TPS-linear-interpolation"))
							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method = (int)DF_GEN_TPS_LINEAR_INTERP;
						else if (item==tr("TPS-B-Spline-interpolation"))
							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method = (int)DF_GEN_TPS_B_SPLINE;
						else if (item==tr("TPS"))
							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method = (int)DF_GEN_TPS;
						else if (item==tr("Hier-B-Spline"))
							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method = (int)DF_GEN_HIER_B_SPLINE;
						else
							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method = (int)DF_GEN_TPS;
					}
				}
			}
			break;
#endif

#endif

		default:
			break;
	}

	return;
}


void XFormView::drawPixmapType(QPainter *painter)
{
    int pwid = disp_width; //changed to disp_height/disp_width on 090212
	int phei = disp_height;
    QPointF center(pwid/2.0, phei/2.0);

	QPointF curDisplayCenter_old = curDisplayCenter;

	if (m_scale>1)
		painter->translate(curDisplayCenter - center);
	else
		curDisplayCenter = center;

	//for the un-zommed coordinate
    painter->translate(center);
    //    painter->rotate(m_rotation);
    painter->scale(m_scale, m_scale);
    //    painter->shear(m_shear, m_shear);
    painter->translate(-center);

	//now zoom. 081114
    painter->scale(disp_scale, disp_scale);

	//
    painter->drawPixmap(QPointF(0, 0), pixmap);

    painter->setPen(QPen(QColor(255, 255, 255, alpha), 1, Qt::DotLine, Qt::FlatCap, Qt::BevelJoin));
    painter->setBrush(Qt::NoBrush);
    //painter->drawRect(QRectF(0, 0, pixmap.width(), pixmap.height()).adjusted(-2, -2, 2, 2));

	if(imgData!=NULL && imgData->isEmpty()==false)
		b_displayFocusCrossLine = imgData->getFlagDisplayFocusCross();

	if (b_displayFocusCrossLine)
	{
		int focusPosInWidth, focusPosInHeight;
		if (!getFocusCrossLinePos(focusPosInWidth, focusPosInHeight, imgData, Ptype)) //should be safe to call even imgData is NULL or contains no data
		{
			focusPosInWidth = pwid/2.0;
			focusPosInHeight = phei/2.0;
		}

		painter->drawLine(0, focusPosInHeight, pixmap.width()-1, focusPosInHeight);
		painter->drawLine(focusPosInWidth, 0, focusPosInWidth, pixmap.height()-1);
	}

	if (imgData && !(imgData->isEmpty()))
	{
		if (imgData->getFlagLookingGlass())
		{
			setCursor(Qt::CrossCursor);

			// draw the Looking glass if necessary and possible. Note that when Looking glass is enabled, the m_scale is assumed to be 1
			drawLookingGlassMap(painter, 0); //draw the anchored zoom-in map
			if (bMouseCurorIn)
				drawLookingGlassMap(painter, &curMousePos); //draw the moving-point zoom-in map
		}
		else
		{
			setCursor(Qt::ArrowCursor);
		}
	}

	// draw the defined interesting & non-interesting points
	bool b_displaySelectedLocation=true;
	if (imgData && b_displaySelectedLocation==true)
	{
		drawSelectedLocations(painter, &(imgData->listLandmarks), &(imgData->listLocationRelationship));
	}

	// draw ROI
	drawROI(painter);

	// draw mapview win only for normal data - one level
    bool b_displayMapviewWin = true;
	if (b_displayMapviewWin)
	{
          // transform back
		painter->scale(1.0/disp_scale, 1.0/disp_scale);
		painter->translate(center);
		painter->scale(1.0/m_scale, 1.0/m_scale);
		painter->translate(-center);
		if (m_scale>1)
			painter->translate(center-curDisplayCenter_old);

		// setPen to draw solid lines
		painter->setPen(QPen(QColor(255, 255, 255, alpha), 3, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin));
		painter->setBrush(Qt::NoBrush);
		// determin the size of mapview win
		int mapwinWidth, mapwinHeight;
		mapwinWidth = (int)(disp_width/5.0 + 0.5);
		if (mapwinWidth<10)
		{
			mapwinWidth=10;
		}
		else if (mapwinWidth>100)
		{
			mapwinWidth=100;
		}
		mapwinHeight = (int)(mapwinWidth* ((float)disp_height/disp_width) + 0.5);

          // draw mapview win
		if (m_scale > 1)
		{
               painter->setOpacity(1.0);
               switch(Ptype)
               {
                    case imgPlaneZ: // xy_view
                         painter->drawRect(disp_width-mapwinWidth, disp_height-mapwinHeight, mapwinWidth, mapwinHeight);
                         break;
                    case imgPlaneY: // zx_view
                         painter->drawRect(disp_width-mapwinWidth, 0, mapwinWidth, mapwinHeight);
                         break;
                    case imgPlaneX: // yz_view
                         painter->drawRect(0, disp_height-mapwinHeight, mapwinWidth, mapwinHeight);
                         break;
                    default:
                         break;
               }

		}

		// draw scale bar
          painter->setPen(QPen(QColor(255, 255, 255, alpha), 1, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin));
          painter->setBrush(Qt::NoBrush);
          painter->setOpacity(1.0);

          painter->drawLine(9, 14, 15, 15);
          //painter->drawRect(10, 15, 1, 30);
		painter->drawLine(10, 15, 10, 45);
          painter->drawLine(9, 46, 15, 45);

          // draw scale text: 30/m_scale
          double zoomf = imgData->getXWidget()->disp_zoom; // zoom factor
          float disp_num=30.0/(m_scale * zoomf);
          painter->drawText(12, 33, QString::number(disp_num, 'f', 1));

		// draw the inner navigation window
		int navwinWidth, navwinHeight;
		float navwinScale = m_scale;
		navwinWidth =int(mapwinWidth/navwinScale);
		navwinHeight = int(navwinWidth*mapwinHeight/(float)mapwinWidth + 0.5);

		int navstartX, navstartY;
		QPointF centerMov = (center-curDisplayCenter_old)*mapwinWidth/(float)(disp_width * m_scale);

          switch(Ptype)
		{
			case imgPlaneZ: // xy_view
                    navstartX = int(disp_width - mapwinWidth/2.0 - navwinWidth/2.0 + centerMov.x() + 0.5);
                    navstartY = int(disp_height - mapwinHeight/2.0 - navwinHeight/2.0 + centerMov.y() + 0.5);
				break;
               case imgPlaneY: // zx_view
                    navstartX = int(disp_width - mapwinWidth/2.0 - navwinWidth/2.0 + centerMov.x() + 0.5);
                    navstartY = int(mapwinHeight/2.0 - navwinHeight/2.0 + centerMov.y() + 0.5);
                    break;
               case imgPlaneX: // yz_view
                    navstartX = int(mapwinWidth/2.0 - navwinWidth/2.0 + centerMov.x() + 0.5);
                    navstartY = int(disp_height - mapwinHeight/2.0 - navwinHeight/2.0 + centerMov.y() + 0.5);
                    break;
               default:
                    break;
          }

		painter->setPen(QPen(QColor(0, 0, 255, alpha), 2, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin));
		painter->setBrush(Qt::NoBrush);

		if (m_scale > 1)
		{
               painter->setOpacity(0.7);
               painter->fillRect(navstartX, navstartY, navwinWidth, navwinHeight, QColor(150, 200, 255, alpha) );
               painter->setOpacity(1.0);
               painter->drawRect(navstartX, navstartY, navwinWidth, navwinHeight);

		}
	} // end b_displayMapviewWin

     // display mapview for hraw multi-level image
     // if(mapview_paras.hraw_prefix != "")
     // {

     // }

}


void XFormView::drawSelectedLocations(QPainter *painter, QList <LocationSimple> *curList, QList <PtIndexAndParents> * curRelation)
{
	if (!curList) return;
	V3DLONG NLandmarks = curList->count();
	if (NLandmarks<=0)
	{
		//qDebug("No landmark.");
		return;
	}

	int b_color = 1;
	bool b_disp_polylines = true;
	//bool b_disp_textlabel = true;

	QColor curColor;
	PxLocationMarkerShape curShape;
	QString curStrLabel;

	if (Ctype==colorGray || Ctype==colorRed2Gray || Ctype==colorGreen2Gray || Ctype==colorBlue2Gray)
	{
		b_color=1;
	}
	else
	{
		b_color = 0;
	}

    int cx = imgData->curFocusX,cy = imgData->curFocusY, cz = imgData->curFocusZ;
	int rr = 5; //, rr_real;

	//int tmpx,tmpy,tmpz;
	float tmpx,tmpy,tmpz;
	float twidpos,theipos; //int
	int b_draw = 0;

	LocationSimple tmpLocation(0,0,0);
     QPolygonF polygon;

	for (V3DLONG i=0;i<NLandmarks;i++)
	{
		tmpLocation = curList->at(i);
		tmpLocation.getCoord(tmpx,tmpy,tmpz);
		rr = ceil(tmpLocation.radius); //090109

		b_draw = 0;
		if (tmpLocation.on==false) continue; //do not draw those have been disabled in the landmark manager. 081210
		switch(Ptype)
		{
			case imgPlaneZ:
				if (tmpz<=cz+rr && tmpz>=cz-rr) {twidpos = tmpx; theipos = tmpy; b_draw=1;}
				break;

			case imgPlaneX:
				if (tmpx<=cx+rr && tmpx>=cx-rr) {twidpos = tmpz; theipos = tmpy; b_draw=1;}
				break;

			case imgPlaneY:
				if (tmpy<=cy+rr && tmpy>=cy-rr) {twidpos = tmpx; theipos = tmpz; b_draw=1;}
				break;

			default:
				b_draw=0;
				break;
		}

		if (b_draw==0)
		{
			//qDebug("[%d] out of range not show", i);
			continue;
		}

//20111017: disable this for now
//20120211: BY PHC. enable again for colored display of markers. Note this is not an ultimate solution, as the color-display is different from my original design already
        // thus when the display is gray now, the markers will still be white (and that is bad sometimes!)

		switch(tmpLocation.howUseful())
		{
			case pxLocaUseful:
				curColor = (b_color==1) ? QColor(255, 0, 0, alpha) : QColor(255, 255, 255, alpha);
				curShape = pxSphere; //pxCircle;
				b_draw=1;
				break;

			case pxLocaNotUseful:
				curColor = (b_color==1) ? QColor(0, 255, 0, alpha) : QColor(255, 255, 255, alpha);
				curShape = pxCube; //pxRect;
				b_draw=1;
				break;

			case pxLocaUnsure:
				curColor = (b_color==1) ? QColor(0, 0, 255, alpha) : QColor(255, 255, 255, alpha);
				curShape = pxTriangle;
				b_draw=1;
				break;

			case pxTemp: //080405
				curColor = (b_color==1) ? QColor(155, 155, 0, alpha) : QColor(255, 255, 255, alpha);
				curShape = pxDot;
				b_draw=1;
				break;

			default:
				b_draw=0;
				break;
		}


		//
		curShape = tmpLocation.shape;//use the saved info

		if (b_draw==0)
		{
			//qDebug("[%d] unknown pxtype not show", i);
			continue;
		}

        painter->setPen(QPen(curColor, 1, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin));

		//qDebug("rr_real=%d ",rr_real);
		switch(curShape)
		{
			case pxSphere:
				//painter->drawEllipse(twidpos-rr,theipos-rr, rr+rr+1, rr+rr+1);
				painter->drawEllipse(twidpos-rr-1,theipos-rr-1, rr+rr+1, rr+rr+1); //081210. correct a skewed draw bug
				//painter->drawEllipse(twidpos-rr_real-1,theipos-rr_real-1, rr_real+rr_real+1, rr_real+rr_real+1); //090109. now use the real radius
				break;

			case pxCube:
				//painter->drawRect(twidpos-rr,theipos-rr, rr+rr+1, rr+rr+1);
				painter->drawRect(twidpos-rr-1,theipos-rr-1, rr+rr+1, rr+rr+1); //081210.
				break;

			case pxTriangle:
				polygon.clear();
				//polygon << QPointF(twidpos, theipos-rr) << QPointF(twidpos-rr, theipos+rr) << QPointF(twidpos+rr, theipos+rr);
				polygon << QPointF(twidpos, theipos-rr-1) << QPointF(twidpos-rr-1, theipos+rr) << QPointF(twidpos+rr, theipos+rr); //081210
				painter->drawPolygon(polygon);
				break;

			case pxDot:	//080405
				//painter->drawPoint(twidpos, theipos); //strange- why not work
				//painter->drawLine(twidpos, theipos, twidpos, theipos); //strange- why not work
				painter->drawEllipse(twidpos,theipos, 1, 1);
				break;

			default:
				break;
		}

		if (curShape!=pxDot && imgData->getXWidget()->bDispMarkerLabel ) //update 080405: do not display text for the pxDot shaped locations
		{
			painter->drawText(twidpos+rr/2, theipos-rr/2, curStrLabel.setNum(i+1));
		}
	}

	// draw the polylines and also labels

	if (b_disp_polylines && Ptype==imgPlaneZ && curRelation && curRelation->count()>0)
	{
        painter->setPen(QPen(QColor(255, 255, 0, alpha), 1, Qt::DotLine, Qt::FlatCap, Qt::RoundJoin));

		float tmpx1,tmpy1,tmpz1;
		float tmpx2,tmpy2,tmpz2;
		V3DLONG curNode, curNodeParent;
		for (V3DLONG jr=0; jr<curRelation->count(); jr++)
		{
			curNode = curRelation->at(jr).nodeInd;
			curNodeParent = curRelation->at(jr).nodeParent;

			if (curNode>=NLandmarks || curNode<0 || curNodeParent>=NLandmarks || curNodeParent<0)
			{
				continue; //do not print this line
			}

			tmpLocation = curList->at(curNode);
			tmpLocation.getCoord(tmpx1,tmpy1,tmpz1);

			tmpLocation = curList->at(curNodeParent);
			tmpLocation.getCoord(tmpx2,tmpy2,tmpz2);

			painter->drawLine(QPointF(tmpx1, tmpy1), QPointF(tmpx2, tmpy2));
		}
	}

	//	printf("done \n");
}


void XFormView::drawROI(QPainter *painter)
{
	int b_color = 1;
	QColor curColor;

	if (Ctype==colorGray || Ctype==colorRed2Gray || Ctype==colorGreen2Gray || Ctype==colorBlue2Gray)
	{
		b_color=1;
	}
	else
	{
		b_color = 0;
	}

    curColor = (b_color==1) ? QColor(255, 0, 0, alpha) : QColor(255, 255, 255, alpha);

    double adj_width = 1./imgData->getXWidget()->disp_zoom; //by PHC, zoom-based ROI border line
    if (adj_width<1)
        adj_width = 1;

    painter->setPen(QPen(curColor, adj_width, Qt::DashDotLine, Qt::FlatCap, Qt::BevelJoin)); //use adj_width instead of 1.
	painter->drawPolygon(roiPolygon);
}

void XFormView::drawLookingGlassMap(QPainter *painter, QPoint *curPt)
{
	// draw the Looking glass. Note that when Looking glass is enabled, the m_scale is assumed to be 1
	int glassRadius = imgData->getXWidget()->getMainControlWindow()->global_setting.default_lookglass_size; //5
    int glassZoom = 4; //4, 5,6,8

	if (m_scale!=1)
		return;

	int focusPosInWidth, focusPosInHeight;
	if (!curPt)
	{
		if (!getFocusCrossLinePos(focusPosInWidth, focusPosInHeight, imgData, Ptype))
			return;
	}
	else
	{
		focusPosInWidth = curPt->x();
		focusPosInHeight = curPt->y();
	}

#if USE_CHANNEL_TABLE
	QPixmap& buf = pixmap_glass;
	//if (Ctype>=colorPseudoMaskColor) buf = pixmap; //switch in XFormView::changColorType instead
#else
	QPixmap& buf = pixmap;
#endif
	QPixmap myrgn = buf.copy(QRect(QPoint(qMin(qMax(focusPosInWidth-glassRadius,0), buf.width()-1),
	                                         qMin(qMax(0,focusPosInHeight-glassRadius), buf.height()-1)),
									  QPoint(qMax(qMin(focusPosInWidth+glassRadius,buf.width()-1), 0),
									         qMax(qMin(focusPosInHeight+glassRadius, buf.height()-1), 0))
									  )
								);

	QPointF tmpcenter(focusPosInWidth, focusPosInHeight);
	painter->translate(tmpcenter);
	painter->scale(glassZoom, glassZoom); //looking glass zoom-in 4 times
	painter->translate(-tmpcenter);
	painter->drawPixmap(QPointF(qMax(focusPosInWidth-glassRadius-0.5, 0.0),
	                            qMax(focusPosInHeight-glassRadius-0.5, 0.0)),
						myrgn);

	painter->translate(tmpcenter);
	painter->scale(1.0/glassZoom, 1.0/glassZoom); //looking glass zoom-in 4 times
	painter->translate(-tmpcenter);

    if (!curPt)
		painter->setPen(QPen(QColor(255, 0, 255, alpha), 1, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin));
	else
		painter->setPen(QPen(QColor(255, 255, 0, alpha), 1, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin));

    painter->setBrush(Qt::NoBrush);
    painter->drawRect(QRectF(focusPosInWidth+(-glassRadius-0.5+0.25)*glassZoom, focusPosInHeight+(-glassRadius-0.5+0.25)*glassZoom, (2*glassRadius+1)*glassZoom-1, (2*glassRadius+1)*glassZoom-1));

}

void XFormView::dispHistogram()
{
	//reserved for future edit
}

#define _______XFormWidget_functions________

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
XFormWidget::XFormWidget(QWidget *parent) : QMdiSubWindow(parent)
#else
XFormWidget::XFormWidget(QWidget *parent) : QWidget(parent)
#endif
{
	initialize();
	createGUI();
	connectEventSignals();
	updateDataRelatedGUI();
}

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
XFormWidget::XFormWidget(QWidget *parent, Qt::WidgetAttribute f) : QMdiSubWindow(parent) //added on 080814: this function is for future use. Not really get called now
#else
XFormWidget::XFormWidget(QWidget *parent, Qt::WidgetAttribute f) : QWidget(parent) //added on 080814: this function is for future use. Not really get called now
#endif
{
	setAttribute(f);

	initialize();
	createGUI();
	connectEventSignals();
	updateDataRelatedGUI();
}

void XFormWidget::initialize()
{
    imgData = 0;
    openFileNameLabel = QString(""); //"/Users/hanchuanpeng/work/v3d/test1.raw"

	mypara_3Dview.b_use_512x512x256 = true;
	mypara_3Dview.b_still_open = false;
	mypara_3Dview.image4d = 0;

	Ctype = colorUnknown;
	Ctype_glass = colorUnknown; //110804 RZC

	atlasViewerDlg = 0; //081123

	/* GUI related pointers*/
    bExistGUI = false;
	bLinkFocusViews = false;
	bDisplayFocusCross = false;
	bDispMarkerLabel = true;

    xy_view = NULL;
    yz_view = NULL;
    zx_view = NULL;

    channelTabXView = NULL; //110722 RZC
    channelTabGlass = NULL; //1100801 RZC

	disp_zoom=1; //081114
	b_use_dispzoom=false;

	focusPointFeatureWidget = NULL;

    dataGroup = NULL;
  	viewGroup = NULL;
	infoGroup = NULL;
	mainGroup = NULL;
	coordGroup = NULL;
	scaleGroup = NULL;
	typeGroup = NULL;

	xSlider = NULL;
	ySlider = NULL;
	zSlider = NULL;
	xValueSpinBox = NULL;
	yValueSpinBox = NULL;
	zValueSpinBox = NULL;
	xSliderLabel = NULL;
	ySliderLabel = NULL;
	zSliderLabel = NULL;

	linkFocusCheckBox = NULL;
	displayFocusCrossCheckBox = NULL;

    xScaleSlider = NULL;
	yScaleSlider = NULL;
	zScaleSlider = NULL;
	xScaleSliderLabel = NULL;
	yScaleSliderLabel = NULL;
	zScaleSliderLabel = NULL;
	zoomWholeViewButton = NULL;

    lookingGlassCheckBox = NULL;

    // for mapview. 20120309 ZJL
    xSlider_mapv = NULL;
    ySlider_mapv = NULL;
    zSlider_mapv = NULL;
    zoomSlider_mapv = NULL;

    xValueSpinBox_mapv = NULL;
    yValueSpinBox_mapv = NULL;
    zValueSpinBox_mapv = NULL;
    zoomSpinBox_mapv = NULL;
    mvControlWin = NULL;

    colorRedType = NULL;
	colorGreenType = NULL;
	colorBlueType = NULL;
	colorAllType = NULL;
    colorRed2GrayType = NULL;
	colorGreen2GrayType = NULL;
	colorBlue2GrayType = NULL;
	colorAll2GrayType = NULL;

    imgValScaleDisplayCheckBox = NULL;

	cBox_bSendSignalToExternal = NULL;
	cBox_bAcceptSignalFromExternal = NULL;

    landmarkCopyButton = NULL;
	landmarkPasteButton = NULL;
	landmarkSaveButton = NULL;
	landmarkLoadButton = NULL;
	landmarkManagerButton = NULL;

	//landmarkLabelDispCheckBox = NULL;

    resetButton = NULL;
	openFileNameButton = NULL;
	imgProcessButton = NULL;
	imgV3DButton = NULL;
	//imgV3DROIButton = NULL;
	whatsThisButton = NULL;

	allLayout = NULL;
	dataGroupLayout = NULL;
	xyzViewLayout = NULL;
	infoGroupLayout = NULL;
	coordGroupLayout = NULL;
	scaleGroupLayout = NULL;
	typeGroupLayout = NULL;
	LandmarkGroupLayout = NULL; //080107
	mainGroupLayout = NULL;

	// communication to other windows

	p_mainWindow = NULL;
	bSendSignalToExternal = false;
	bAcceptSignalFromExternal = false;

	//
	bUsingMultithreadedImageIO = true;
}

XFormWidget::~XFormWidget()
{
	qDebug("***v3d: ~XFormWidget");
	cleanData();
}

// for mapview ZJL
void XFormWidget::createMapviewControlWin()
{
    mvControlWin = new QWidget(this);
    mvControlWin->setWindowTitle("Mapview Control");
    mvControlWin->setWindowFlags( Qt::Widget
            | Qt::Tool
            | Qt::CustomizeWindowHint | Qt::WindowTitleHint  //only title bar, disable buttons on title bar
            );

    mvControlWin->setFixedWidth(250);
    mvControlWin->setFixedHeight(150);

    QGridLayout *layout = new QGridLayout(mvControlWin);

    // get X Y Z size
    V3DLONG ts0, ts1, ts2; // block nums
    V3DLONG bs0, bs1, bs2; // block size
    V3DLONG dimx, dimy, dimz;
    mapview.getImageSize(mapview_paras.level, ts0, ts1, ts2, bs0, bs1, bs2);
    dimx = ts0*bs0; dimy = ts1*bs1; dimz = ts2*bs2;

    // zoom range
    int dim_zoom= 10;

    xSlider_mapv = new QScrollBar(Qt::Horizontal);
    xSlider_mapv->setRange(0, dimx-1-mapview_paras.outsz[0]); //need redefine range
    xSlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QLabel* xSliderLabel_mapv = new QLabel("X");

    xValueSpinBox_mapv = new QSpinBox;
    xValueSpinBox_mapv->setRange(0, dimx-1-mapview_paras.outsz[0]);
    xValueSpinBox_mapv->setSingleStep(1);
    xValueSpinBox_mapv->setValue(mapview_paras.origin[0]);

    ySlider_mapv = new QScrollBar(Qt::Horizontal);
    ySlider_mapv->setRange(0, dimy-1-mapview_paras.outsz[1]); //need redefine range
    ySlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QLabel* ySliderLabel_mapv = new QLabel("Y");

    yValueSpinBox_mapv = new QSpinBox;
    yValueSpinBox_mapv->setRange(0, dimy-1-mapview_paras.outsz[1]);
    yValueSpinBox_mapv->setSingleStep(1);
    yValueSpinBox_mapv->setValue(mapview_paras.origin[1]);

    zSlider_mapv = new QScrollBar(Qt::Horizontal);
    zSlider_mapv->setRange(0, dimz-1-mapview_paras.outsz[2]); //need redefine range
    zSlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QLabel* zSliderLabel_mapv = new QLabel("Z");

    zValueSpinBox_mapv = new QSpinBox;
    zValueSpinBox_mapv->setRange(0, dimz-1-mapview_paras.outsz[2]);
    zValueSpinBox_mapv->setSingleStep(1);
    zValueSpinBox_mapv->setValue(mapview_paras.origin[2]);

    // zoom slider
    zoomSlider_mapv = new QScrollBar(Qt::Horizontal);
    zoomSlider_mapv->setRange(0, dim_zoom-1); //need redefine range
    zoomSlider_mapv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QLabel* zoomLabel_mapv = new QLabel("Zoom");

    zoomSpinBox_mapv = new QSpinBox;
    zoomSpinBox_mapv->setRange(0, dim_zoom-1);
    zoomSpinBox_mapv->setSingleStep(1);
    zoomSpinBox_mapv->setValue(mapview_paras.level);

    // layout for mv control window
    layout->addWidget(zSliderLabel_mapv, 0, 0, 1, 1);
    layout->addWidget(zSlider_mapv, 0, 1, 1, 13);
    layout->addWidget(zValueSpinBox_mapv, 0, 14, 1, 6);

    layout->addWidget(xSliderLabel_mapv, 1, 0, 1, 1);
    layout->addWidget(xSlider_mapv, 1, 1, 1, 13);
    layout->addWidget(xValueSpinBox_mapv, 1, 14, 1, 6);

    layout->addWidget(ySliderLabel_mapv, 2, 0, 1, 1);
    layout->addWidget(ySlider_mapv, 2, 1, 1, 13);
    layout->addWidget(yValueSpinBox_mapv, 2, 14, 1, 6);

    layout->addWidget(zoomLabel_mapv, 3, 0, 1, 1);
    layout->addWidget(zoomSlider_mapv, 3, 1, 1, 13);
    layout->addWidget(zoomSpinBox_mapv, 3, 14, 1, 6);

    // setup connections
    connect(xValueSpinBox_mapv, SIGNAL(valueChanged(int)), xSlider_mapv, SLOT(setValue(int)));
    connect(xSlider_mapv, SIGNAL(valueChanged(int)), xValueSpinBox_mapv, SLOT(setValue(int)));

    connect(yValueSpinBox_mapv, SIGNAL(valueChanged(int)), ySlider_mapv, SLOT(setValue(int)));
    connect(ySlider_mapv, SIGNAL(valueChanged(int)), yValueSpinBox_mapv, SLOT(setValue(int)));

    connect(zValueSpinBox_mapv, SIGNAL(valueChanged(int)), zSlider_mapv, SLOT(setValue(int)));
    connect(zSlider_mapv, SIGNAL(valueChanged(int)), zValueSpinBox_mapv, SLOT(setValue(int)));

    connect(zoomSpinBox_mapv, SIGNAL(valueChanged(int)), zoomSlider_mapv, SLOT(setValue(int)));
    connect(zoomSlider_mapv, SIGNAL(valueChanged(int)), zoomSpinBox_mapv, SLOT(setValue(int)));

    connect(xSlider_mapv,    SIGNAL(valueChanged(int)), this, SLOT(changeXOffset_mapv(int)));
    connect(ySlider_mapv,    SIGNAL(valueChanged(int)), this, SLOT(changeYOffset_mapv(int)));
    connect(zSlider_mapv,    SIGNAL(valueChanged(int)), this, SLOT(changeZOffset_mapv(int)));
    connect(zoomSlider_mapv, SIGNAL(valueChanged(int)), this, SLOT(changeLevel_mapv(int)));

    mvControlWin->show();
}

void XFormWidget::updateMapviewControlWin(int level)
{
    mapview_paras.level = level;

    // get X Y Z size
    V3DLONG ts0, ts1, ts2; // block nums
    V3DLONG bs0, bs1, bs2; // block size
    V3DLONG dimx, dimy, dimz;
    mapview.getImageSize(mapview_paras.level, ts0, ts1, ts2, bs0, bs1, bs2);

    dimx = ts0*bs0;   dimy = ts1*bs1;   dimz = ts2*bs2;

    xSlider_mapv->setRange(0, dimx-1-mapview_paras.outsz[0]);
    xValueSpinBox_mapv->setRange(0, dimx-1-mapview_paras.outsz[0]);
    xValueSpinBox_mapv->setValue(mapview_paras.origin[0]);

    ySlider_mapv->setRange(0, dimy-1-mapview_paras.outsz[1]);
    yValueSpinBox_mapv->setRange(0, dimy-1-mapview_paras.outsz[1]);
    yValueSpinBox_mapv->setValue(mapview_paras.origin[1]);

    zSlider_mapv->setRange(0, dimz-1-mapview_paras.outsz[2]);
    zValueSpinBox_mapv->setRange(0, dimz-1-mapview_paras.outsz[2]);
    zValueSpinBox_mapv->setValue(mapview_paras.origin[2]);

}

void XFormWidget::changeXOffset_mapv(int x)
{
    mapview_paras.origin[0] = x;
    updateMapview();
}

void XFormWidget::changeYOffset_mapv(int y)
{
    mapview_paras.origin[1] = y;
    updateMapview();
}


void XFormWidget::changeZOffset_mapv(int z)
{
    mapview_paras.origin[2] = z;
    updateMapview();
}

void XFormWidget::changeLevel_mapv(int level)
{
    mapview_paras.level = level;
    updateMapviewControlWin(level);
    updateMapview();
}

void XFormWidget::updateMapview()
{
     // retrieve image from blocks
     unsigned char * outimg1d = 0;

     mapview.getImage(mapview_paras.level, outimg1d, mapview_paras.origin[0], mapview_paras.origin[1], mapview_paras.origin[2],
                      mapview_paras.outsz[0], mapview_paras.outsz[1], mapview_paras.outsz[2]);

     this->setImageData(outimg1d, mapview_paras.outsz[0], mapview_paras.outsz[1], mapview_paras.outsz[2], mapview_paras.outsz[3], V3D_UINT8);

     this->reset();

}


void XFormWidget::closeEvent(QCloseEvent *event) //080814: this function is specially added to assure the image data will be cleaned; so that have more memory for other stacks.
//note the reason to overload this closeEvent function but not use the QWidget destructor is because seems Qt has a build-in bug in freeing ArthurFrame object in QString freeing
{
	qDebug("***v3d: XFormWidget::closeEvent");

    //clean up the potential 3D viewer links so to avoid potential later crash. PHC 20151116
        mypara_3Dlocalview.image4d = NULL;
        mypara_3Dlocalview.xwidget = NULL;

        mypara_3Dview.image4d = NULL;
        mypara_3Dview.xwidget = NULL;

    //


    if(mvControlWin) mvControlWin->close();

	printf("Now going to free memory for this image or data of this window. .... ");
	cleanData();
	printf("Succeeded in freeing memory.\n");

	//TODO: find solution to really do ~XFormWidget
	//deleteLater(); //090812, 110802 RZC: will cause BAD_ACCESS in XFormView::~XFormView()
	//110804 seems that some paint device resource are leaked
}

void XFormWidget::cleanData()
{
	if (imgData) {delete imgData; imgData = 0;}
	if (atlasViewerDlg) {atlasViewerDlg->deleteLater(); atlasViewerDlg=0;} //081211,090812 deleteLater
	if (channelTabGlass) {channelTabGlass->deleteLater(); channelTabGlass=0;} //110802 RZC
}

void XFormWidget::changeEvent(QEvent* e)
{
	//qDebug() <<"XFormWidget::changeEvent" <<e->type() <<windowTitle();
	if (e->type()==QEvent::ActivationChange && isActiveWindow())  //NO effect to MDI child widget !!!
	{
			qDebug() << QString("XFormWidget::changeEvent, ActivationChange-> %1").arg(windowTitle());

			if (channelTabGlass)  channelTabGlass->show();
	}
	else	if (channelTabGlass)  channelTabGlass->hide();
}
void XFormWidget::hideEvent(QHideEvent* e)
{
	if (channelTabGlass)  channelTabGlass->hide();
}
void XFormWidget::onActivated(QWidget* aw)
{
	//qDebug() <<"XFormWidget::onActivated" <<aw <<"this="<<this <<windowTitle();
	if (aw == this)
	{
		//qDebug() <<"XFormWidget::onActivated" <<this <<"channelTabGlass->show()" <<windowTitle();
		if (channelTabGlass)  channelTabGlass->show();
	}
	else
	{
		//qDebug() <<"XFormWidget::onActivated" <<this <<"channelTabGlass->hide()" <<windowTitle();
		if (channelTabGlass)  channelTabGlass->hide();
	}
}

void XFormWidget::keyPressEvent(QKeyEvent * e)
{
	if (!imgData || !imgData->valid()) return;

	switch (e->key())
	{
		case Qt::Key_Y: //add a timer for some events. This is an on/off switch. When pressing is OFF, then output the time to the last ON pressing
            {
                if (imgData->b_triviewTimerON)
                {
                    QString etime = QString("Elipsed time for triview timer is %1 seconds").arg(double(imgData->triviewTimer.elapsed())/1000);
                    imgData->b_triviewTimerON = false;
                    v3d_msg(etime);
                    if (imgData->p_mainWidget && imgData->p_mainWidget->getMainControlWindow())
                        imgData->p_mainWidget->getMainControlWindow()->statusBar()->showMessage(etime);
                }
                else
                {
                    imgData->triviewTimer.start(); //should I use restart() sometimes as well
                    imgData->b_triviewTimerON = true;
                    QString mymsg = "Timer for the current image starts...";
                    v3d_msg(mymsg + "\n", 0);
                    if (imgData->p_mainWidget && imgData->p_mainWidget->getMainControlWindow())
                        imgData->p_mainWidget->getMainControlWindow()->statusBar()->showMessage(mymsg, 5000);
                }
            }
			break;

		case Qt::Key_S:
		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
		    {
				saveData();
			}
			break;

//		case Qt::Key_Left: //for unknown reason, QT just does not recognize the combination of keymodifier and arrow!. by PHC, 090211.
//			if (QApplication::keyboardModifiers()==Qt::ControlModifier) //then scroll page by page
//			{
//				stepx = pixmap.width()/2.0, stepy = pixmap.height()/2.0;
//				qDebug()<<"ctrl pressed: "<<stepx<<" "<<stepy;
//			}
//			if (m_scale>1)
//			{
//				curDisplayCenter -= QPointF(stepx, 0);
//				if (curDisplayCenter.x() < (2-m_scale)*pixmap.width()/2.0-1)
//					curDisplayCenter.setX((2-m_scale)*pixmap.width()/2.0-1);
//
//				update();
//		    }
//	  		break;
//
//		case Qt::Key_Right:
//			if (QApplication::keyboardModifiers()==Qt::ControlModifier) //then scroll page by page
//			{
//				stepx = pixmap.width()/2.0, stepy = pixmap.height()/2.0;
//				qDebug()<<"ctrl pressed: "<<stepx<<" "<<stepy;
//			}
//			if (m_scale>1)
//			{
//				curDisplayCenter += QPointF(stepx, 0);
//				if (curDisplayCenter.x() > m_scale*pixmap.width()/2.0)
//					curDisplayCenter.setX(m_scale*pixmap.width()/2.0);
//
//				update();
//		    }
//			break;
//
//		case Qt::Key_Up:
//			if (QApplication::keyboardModifiers()==Qt::ControlModifier) //then scroll page by page
//			{
//				stepx = pixmap.width()/2.0, stepy = pixmap.height()/2.0;
//				qDebug()<<"ctrl pressed: "<<stepx<<" "<<stepy;
//			}
//			if (m_scale>1)
//			{
//				curDisplayCenter -= QPointF(0, stepy);
//				if (curDisplayCenter.y() < (2-m_scale)*pixmap.height()/2.0-1)
//					curDisplayCenter.setY((2-m_scale)*pixmap.height()/2.0-1);
//
//				update();
//		    }
//			break;
//
//		case Qt::Key_Down:
//			//case Qt::Key_8: //a test to show normal key modifier works!
//			if (QApplication::keyboardModifiers()==Qt::ControlModifier) //then scroll page by page
//			{
//				stepx = pixmap.width()/2.0, stepy = pixmap.height()/2.0;
//				qDebug()<<"ctrl pressed: "<<stepx<<" "<<stepy;
//			}
//			if (m_scale>1)
//			{
//				//qDebug()<<"real stepx="<<stepx<<" stepy="<<stepy;
//				curDisplayCenter += QPointF(0, stepy);
//				if (curDisplayCenter.y() > m_scale*pixmap.height()/2.0)
//					curDisplayCenter.setY(m_scale*pixmap.height()/2.0);
//
//				update();
//		    }
//			break;
//
//		case Qt::Key_N:
//		case Qt::Key_Period: //080403
//			switch (Ptype)
//		{
//			case imgPlaneX:
//				emit focusXChanged(imgData->curFocusX+1+1); //the first +1 to convert to the range [1, max]. The below is the same.
//				break;
//			case imgPlaneY:
//				emit focusYChanged(imgData->curFocusY+1+1);
//				break;
//			case imgPlaneZ:
//				emit focusZChanged(imgData->curFocusZ+1+1);
//				break;
//			default: //do nothing
//				break;
//		}
//			break;
//
//		case Qt::Key_B: //add 'b' on 080109
//		case Qt::Key_Comma: //080403
//			switch (Ptype)
//		{
//			case imgPlaneX:
//				emit focusXChanged(imgData->curFocusX+1-1);
//				break;
//			case imgPlaneY:
//				emit focusYChanged(imgData->curFocusY+1-1);
//				break;
//			case imgPlaneZ:
//				emit focusZChanged(imgData->curFocusZ+1-1);
//				break;
//			default: //do nothing
//				break;
//		}
//			break;
//
		case Qt::Key_I:
			triview_zoomin();
			break;

		case Qt::Key_O:
		{
			bool result = (
						   disp_zoom * imgData->getXDim() <= 1 ||
						   disp_zoom * imgData->getYDim() <= 1 ||
						   disp_zoom * imgData->getZDim() <= 1 );

			if ( result )
			{
				v3d_msg("Cannot zoom-out more, - one of the first 3 dims of the images has been displayed to <=1 pixel on the monitor.");
				break;
			}
			triview_zoomout();
			break;
		}

		case Qt::Key_1:
			triview_zoom1();
			break;

		case Qt::Key_2:
			triview_zoom2();
			break;

		case Qt::Key_3:
			triview_setzoom(3, false);
			break;

		case Qt::Key_4:
			triview_setzoom(4, false);
			break;

			//the following is another way to activate the pop-up menu or point-definition dialog at the pixel location. by PHC, 060312

#if COMPILE_TARGET_LEVEL != 0
		case Qt::Key_M:
		{
			v3d_msg("xformwidget marker processing... ",0);

			//first search if a landmark has been defined at the same location. If yes, modify that one. Otherwise add a new one.

			QList <LocationSimple> * tmplist = (QList <LocationSimple> *) &(imgData->listLandmarks);
			int tmprownum; bool b_landmark_exist=false;
			int cx = imgData->curFocusX+1, cy = imgData->curFocusY+1, cz = imgData->curFocusZ+1;
			for (tmprownum=0;tmprownum<tmplist->count();tmprownum++)
			{
				if (int(tmplist->at(tmprownum).x)==cx && int(tmplist->at(tmprownum).y)==cy && int(tmplist->at(tmprownum).z)==cz)
				{
					b_landmark_exist=true;
					printf("detected existing landmark no=[%d]\n", tmprownum);
					break;
				}
			}

			LandmarkPropertyDialog *landmarkView = NULL;
			if (!landmarkView)
			{
				if (b_landmark_exist)
					landmarkView = new LandmarkPropertyDialog(tmplist, tmprownum, imgData);
				else
				{
					LocationSimple tmp_location(cx, cy, cz);
					//tmp_location.order = imgData->listLandmarks.count()+1;
					QString tmp_label = "";
					tmp_location.name = qPrintable(tmp_label.setNum(imgData->listLandmarks.count()+1).prepend("landmark "));
					tmp_location.radius = imgData->getXWidget()->getMainControlWindow()->global_setting.default_marker_radius; //add a default landmark size
					QList <LocationSimple> tmplist_1;
					tmplist_1.append(tmp_location);
					landmarkView = new LandmarkPropertyDialog(&tmplist_1, 0, imgData);
				}
			}

			int res = landmarkView->exec(); //note that as I request the user must either accept or change the cell property, I set it as a Modal dialog by calling exec() instead of show.
			if (res!=QDialog::Accepted)
			{
				if (landmarkView) {delete landmarkView; landmarkView = NULL;}
				break; //only return true when the results are accepted, which will lead to an update operation below
			}

			//update the current item
			if (!b_landmark_exist)
			{
				LocationSimple tmp_location(cx, cy, cz);
				//QString tmp_label = ""; tmp_location.name = qPrintable(tmp_label.setNum(imgData->listLandmarks.count()+1).prepend("landmark ")); //no need to do again, as the content will be overwritten anyway
				imgData->listLandmarks.append(tmp_location);
				tmprownum = imgData->listLandmarks.count()-1;
			}
			landmarkView->fetchData(&(imgData->listLandmarks), tmprownum);
			qDebug("data fetched [%s][%s] shape=[%d] radius=[%5.3f]",
				   imgData->listLandmarks.at(tmprownum).name.c_str(), imgData->listLandmarks.at(tmprownum).comments.c_str(),  int(imgData->listLandmarks.at(tmprownum).shape),  float(imgData->listLandmarks.at(tmprownum).radius));

			//important: set the shape of the landmark
			LocationSimple * p_tmp_location = (LocationSimple *) & (imgData->listLandmarks.at(tmprownum));
			switch (p_tmp_location->shape)
			{
				case pxSphere:	p_tmp_location->inputProperty = pxLocaUseful; //qDebug("pxsphere");
					break;
				case pxCube: p_tmp_location->inputProperty = pxLocaNotUseful; //qDebug("pxcube");
					break;
				default: p_tmp_location->inputProperty = pxLocaUnsure; //qDebug("%d pxunsure", int(p_tmp_location->shape));
					break;
			}

			if (landmarkView) {delete landmarkView; landmarkView = NULL;}
		}
			break;

//		case Qt::Key_H:
//		    dispHistogram();//in the future I can add a parameter to indicate the current view-id, so that I can only display the histogram of the current view (slice) instead of the whole stack
//			break;

		case Qt::Key_C:
		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
		    {
				imgData->get_xy_view()->popupImageProcessingDialog(tr(" -- crop image using minMax bounding box in 3D (derived from ROIs)"));
            }
			else if (QApplication::keyboardModifiers()==Qt::ShiftModifier)
			{
				if (imgData->getCDim()!=1) break; //only work for 1 channel data
				if(imgData->getDatatype()!=V3D_UINT16 && imgData->getDatatype()!=V3D_UINT8 && imgData->getDatatype()!=V3D_FLOAT32)
					break;
				switchMaskColormap();
			}
			break;

//		case Qt::Key_R:
//		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
//		    {
//				Options_Rotate tmpopt;
//				tmpopt.degree=0.0;
//				tmpopt.b_keepSameSize=true;
//				tmpopt.fillcolor=0;
//				tmpopt.center_x = (imgData->getXDim()-1.0)/2;
//				tmpopt.center_y = (imgData->getYDim()-1.0)/2;
//				tmpopt.center_z = (imgData->getZDim()-1.0)/2;
//
//				Dialog_Rotate tmpdlg;
//				tmpdlg.setContents(tmpopt);
//
//				int dlg_res = tmpdlg.exec();
//				if (dlg_res)
//				{
//					tmpdlg.getContents(tmpopt);
//					imgData->rotate(Ptype, tmpopt);
//				}
//			}
//			break;
//
//		case Qt::Key_D: //remove the last pos from roiVertexList
//		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
//		    {
//				//roiPolygon.erase(roiPolygon.end()-1);
//				if (roiPolygon.count()>0)
//				{
//					roiPolygon.pop_back();
//					update();
//				}
//			}
//			break;
#endif


#if COMPILE_TARGET_LEVEL == 2
		case Qt::Key_P:
		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
		    {
				popupImageProcessingDialog();
			}
			break;
#endif

#if COMPILE_TARGET_LEVEL != 0
		case Qt::Key_V:
		    if (QApplication::keyboardModifiers()==Qt::ControlModifier) //launch the full-image 3d view
		    {
				doImage3DView(true); //use the maximum display 512x512x256
			}
			else if (QApplication::keyboardModifiers()==Qt::ShiftModifier) //launch the local zoom-in view
			{
				doImage3DLocalRoiView();
			}
			else if (QApplication::keyboardModifiers()==(Qt::ShiftModifier | Qt::ControlModifier)) //display the real size of an image. This may crash if not enough memory is available
			{
				if(QMessageBox::question (0, "",
										  "You have just requested displaying 3D view for an image using the full resolution. "
										  "If your machine does not have enough video memory, you may have a crash in the video memory. "
										  "Are you sure you want to continue?",
										  QMessageBox::Yes, QMessageBox::No)
				   == QMessageBox::Yes)
				{
					doImage3DView(false);
				}
			}
			break;
#endif

#if COMPILE_TARGET_LEVEL == 2

#ifdef _ALLOW_ATLAS_IMAGE_MENU_
		case Qt::Key_A: //activate the atlas viewer
		case Qt::Key_F: //activate the find/search function
		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
		    {
				(e->key()==Qt::Key_F) ? launchAtlasViewer(1) : launchAtlasViewer();
			}
			break;
#endif

#ifdef _ALLOW_NEURONSEG_MENU_
		case Qt::Key_T:
			popupImageProcessingDialog(tr(" -- trace between two locations"));
 			break;

		case Qt::Key_Z: //undo the last tracing step if possible. by PHC, 090120
		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
		    {
		    	if (imgData)
					popupImageProcessingDialog(tr(" -- undo last tracing step"));
			}
			break;
#endif

//#ifdef _ALLOW_IMGREG_MENU_
//		case Qt::Key_W:
//			popupImageProcessingDialog(tr(" -- Match one single landmark in another image"));
// 			break;
//
//		case Qt::Key_E:
//		    if (QApplication::keyboardModifiers()==Qt::ControlModifier)
//		    {
//				bool ok;
//				if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod==(int)MATCH_INTENSITY)
//					item = tr("MATCH_INTENSITY");
//				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod==(int)MATCH_CORRCOEF)
//					item = tr("MATCH_CORRCOEF");
//				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod==(int)MATCH_MI)
//					item = tr("MATCH_MI");
//				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod==(int)MATCH_IMOMENT)
//					item = tr("MATCH_IMOMENT");
//				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod==(int)MATCH_MEANOFCIRCLES)
//					item = tr("MATCH_MEANOFCIRCLES");
//				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod==(int)MATCH_MULTIPLE_MI_INT_CORR)
//					item = tr("MATCH_MULTIPLE_MI_INT_CORR");
//				else
//					item = tr("Undefined");
//
//				if(QMessageBox::Yes == QMessageBox::question (0, "", tr("Your current landmark matching method is [ ") + item + tr("]<br> Do you change?"), QMessageBox::Yes, QMessageBox::No))
//				{
//					items << tr("MATCH_MI") << tr("MATCH_MULTIPLE_MI_INT_CORR") << tr("MATCH_INTENSITY") << tr("MATCH_CORRCOEF") << tr("MATCH_IMOMENT") << tr("MATCH_MEANOFCIRCLES");
//					item = QInputDialog::getItem(this, tr(""), tr("Please select a landmark matching method"), items, 0, false, &ok);
//					if (ok && !item.isEmpty())
//					{
//						if (item==tr("MATCH_INTENSITY"))
//							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod = (int)MATCH_INTENSITY;
//						else if (item==tr("MATCH_CORRCOEF"))
//							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod = (int)MATCH_CORRCOEF;
//						else if (item==tr("MATCH_MI"))
//							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod = (int)MATCH_MI;
//						else if (item==tr("MATCH_IMOMENT"))
//							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod = (int)MATCH_IMOMENT;
//						else if (item==tr("MATCH_MEANOFCIRCLES"))
//							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod = (int)MATCH_MEANOFCIRCLES;
//						else if (item==tr("MATCH_MULTIPLE_MI_INT_CORR"))
//							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod = (int)MATCH_MULTIPLE_MI_INT_CORR;
//						else
//							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_landmarkMatchingMethod = (int)MATCH_MI;
//					}
//				}
//			}
//			else if (QApplication::keyboardModifiers()==Qt::ShiftModifier)
//			{
//				bool ok;
//				if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method==(int)DF_GEN_TPS)
//					item = tr("TPS");
//				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method==(int)DF_GEN_HIER_B_SPLINE)
//					item = tr("Hier-B-Spline");
//				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method==(int)DF_GEN_TPS_B_SPLINE)
//					item = tr("TPS-B-Spline-interpolation");
//				else if (imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method==(int)DF_GEN_TPS_LINEAR_INTERP)
//					item = tr("TPS-linear-interpolation");
//				else
//					item = tr("Undefined");
//
//				if(QMessageBox::Yes == QMessageBox::question (0, "", tr("Your current displacement field computing method is [ ") + item + tr("]<br> Do you change?"), QMessageBox::Yes, QMessageBox::No))
//				{
//					items << tr("TPS-linear-interpolation") <<  tr("TPS-B-Spline-interpolation") << tr("TPS") << tr("Hier-B-Spline");
//					item = QInputDialog::getItem(this, tr(""), tr("Please select a displacement filed (DF) computing method"), items, 0, false, &ok);
//					if (ok && !item.isEmpty())
//					{
//						if (item==tr("TPS-linear-interpolation"))
//							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method = (int)DF_GEN_TPS_LINEAR_INTERP;
//						else if (item==tr("TPS-B-Spline-interpolation"))
//							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method = (int)DF_GEN_TPS_B_SPLINE;
//						else if (item==tr("TPS"))
//							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method = (int)DF_GEN_TPS;
//						else if (item==tr("Hier-B-Spline"))
//							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method = (int)DF_GEN_HIER_B_SPLINE;
//						else
//							imgData->getXWidget()->getMainControlWindow()->global_setting.GPara_df_compute_method = (int)DF_GEN_TPS;
//					}
//				}
//			}
//			break;
//#endif

#endif

		default:
			break;
	}

	return;
}


//void XFormWidget::focusInEvent ( QFocusEvent * event )
//{
//	if (p_mainWindow)
//	{
//		p_mainWindow->updateMenus();
//		printf("Updated mainwindow menu.\n");
//	}
//}

void XFormWidget::updateTriview()
{
	qDebug()<<"triggered in XFormWidget ... ...";
	if (p_mainWindow) p_mainWindow->updateTriviewWindow();
}

void XFormWidget::updateViews()
{
	if (imgData) imgData->updateViews();
}


#define __channel_table_gui__
#if USE_CHANNEL_TABLE // switch code path

void XFormWidget::connectColorGUI()
{
	//110722 RZC, connect signal for ChannelTabWidget::updateXFormWidget(int plane)
	connect(xy_view, SIGNAL(colorChanged(int)), this, SIGNAL(colorChanged(int)));
	connect(yz_view, SIGNAL(colorChanged(int)), this, SIGNAL(colorChanged(int)));
	connect(zx_view, SIGNAL(colorChanged(int)), this, SIGNAL(colorChanged(int)));
	//110803 RZC
	connect(xy_view, SIGNAL(colorChangedGlass(int)), this, SIGNAL(colorChangedGlass(int)));
	connect(yz_view, SIGNAL(colorChangedGlass(int)), this, SIGNAL(colorChangedGlass(int)));
	connect(zx_view, SIGNAL(colorChangedGlass(int)), this, SIGNAL(colorChangedGlass(int)));
}
void XFormWidget::disconnectColorGUI()
{
}
void XFormWidget::setColorGUI()
{
	if (channelTabXView)  channelTabXView->linkXFormWidgetChannel();
}
QWidget* XFormWidget::createColorGUI()
{
	(colorMapDispType = new QRadioButton(this))->hide(); //just for XFormWidget::switchMaskColormap()

	if (channelTabXView = new ChannelTabWidget(this, 3)) //3 tabs for XFormView
	{
		connect(this, SIGNAL(colorChanged(int)), channelTabXView, SLOT(updateXFormWidget(int)));

		channelTabXView->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
		channelTabXView->setFixedHeight(200); //200 is best for 4 rows
	}
	return channelTabXView;;
}

#else // old code

void XFormWidget::connectColorGUI()
{
    connect(colorRedType, SIGNAL(clicked()), this, SLOT(setColorRedType()));
    connect(colorGreenType, SIGNAL(clicked()), this, SLOT(setColorGreenType()));
    connect(colorBlueType, SIGNAL(clicked()), this, SLOT(setColorBlueType()));
    connect(colorAllType, SIGNAL(clicked()), this, SLOT(setColorAllType()));

    connect(colorRed2GrayType, SIGNAL(clicked()), this, SLOT(setColorRed2GrayType()));
    connect(colorGreen2GrayType, SIGNAL(clicked()), this, SLOT(setColorGreen2GrayType()));
    connect(colorBlue2GrayType, SIGNAL(clicked()), this, SLOT(setColorBlue2GrayType()));
    connect(colorAll2GrayType, SIGNAL(clicked()), this, SLOT(setColorAll2GrayType()));

	connect(colorMapDispType, SIGNAL(clicked()), this, SLOT(setColorMapDispType()));
    connect(imgValScaleDisplayCheckBox, SIGNAL(clicked()), this, SLOT(toggleImgValScaleDisplay()));
}
void XFormWidget::disconnectColorGUI()
{
    disconnect(colorRedType, 0, this, 0);
    disconnect(colorGreenType, 0, this, 0);
    disconnect(colorBlueType, 0, this, 0);
    disconnect(colorAllType, 0, this, 0);

    disconnect(colorRed2GrayType, 0, this, 0);
    disconnect(colorGreen2GrayType, 0, this, 0);
    disconnect(colorBlue2GrayType, 0, this, 0);
    disconnect(colorAll2GrayType, 0, this, 0);

    disconnect(colorMapDispType, 0, this, 0);
    disconnect(imgValScaleDisplayCheckBox, 0, this, 0);
}

void XFormWidget::setColorGUI()
{
	if (imgData)
	{
		colorRedType->setEnabled(true);
		colorBlueType->setEnabled(true);
		colorGreenType->setEnabled(true);
		colorAllType->setEnabled(true);
		colorRed2GrayType->setEnabled(true);
		colorGreen2GrayType->setEnabled(true);
		colorBlue2GrayType->setEnabled(true);
		colorAll2GrayType->setEnabled(true);
		colorMapDispType->setEnabled(true);

		imgValScaleDisplayCheckBox->setEnabled(true);
		if (imgData->getDatatype()==V3D_UINT16 || imgData->getDatatype()==V3D_FLOAT32)
			imgValScaleDisplayCheckBox->setCheckState(Qt::Checked); //100814. PHC. set 16/32bit data default to rescale for triview display
		//imgData->setFlagImgValScaleDisplay((imgValScaleDisplayCheckBox->checkState()==Qt::Checked) ? true : false); //100814: PHC. move to here to avoid potential error

		if (imgData->getCDim()>=3) //081124
		{
			setColorAllType();
		}

		if (imgData->getCDim()<3)
		{
			colorBlueType->setEnabled(false);
			colorBlue2GrayType->setEnabled(false);
		}

		if (imgData->getCDim()<2)
		{
			colorGreenType->setEnabled(false);
			colorGreen2GrayType->setEnabled(false);
		}

		colorMapDispType->setEnabled(imgData->getCDim()==1);

		if (imgData->getCDim()<=1) //100815
			colorRed2GrayType->setChecked(true);
		else
			colorAllType->setChecked(true);

	}
	else // no imgData
	{

		colorRedType->setEnabled(false);
		colorBlueType->setEnabled(false);
		colorGreenType->setEnabled(false);
		colorAllType->setEnabled(false);
		colorRed2GrayType->setEnabled(false);
		colorGreen2GrayType->setEnabled(false);
		colorBlue2GrayType->setEnabled(false);
		colorAll2GrayType->setEnabled(false);
		colorMapDispType->setEnabled(false);

		imgValScaleDisplayCheckBox->setEnabled(false);

		colorAllType->setChecked(true);

	}
}

QWidget* XFormWidget::createColorGUI()
{
    QGroupBox *typeGroup = new QGroupBox(this);
    //typeGroup->setAttribute(Qt::WA_ContentsPropagated);
    typeGroup->setTitle("Color Channels");

    colorRedType = new QRadioButton(typeGroup);
    colorGreenType = new QRadioButton(typeGroup);
    colorBlueType = new QRadioButton(typeGroup);
    colorAllType = new QRadioButton(typeGroup);
    colorRed2GrayType = new QRadioButton(typeGroup);
    colorGreen2GrayType = new QRadioButton(typeGroup);
    colorBlue2GrayType = new QRadioButton(typeGroup);
    colorAll2GrayType = new QRadioButton(typeGroup);
	colorMapDispType = new QRadioButton(typeGroup);

    colorRedType->setText("Red (Chan 1)");
    colorGreenType->setText("Green (Chan 2)");
    colorBlueType->setText("Blue (Chan 3)");
    colorAllType->setText("RGB (All)");
    colorRed2GrayType->setText("Red (gray)");
    colorGreen2GrayType->setText("Green (gray)");
    colorBlue2GrayType->setText("Blue (gray)");
    colorAll2GrayType->setText("RGB (gray)");
	colorMapDispType->setText("Colormap (for indexed image)");

	imgValScaleDisplayCheckBox = new QCheckBox("I(Voxel) rescale: m->0, M->255");
	imgValScaleDisplayCheckBox->setCheckState(Qt::Unchecked);


    typeGroupLayout = new QGridLayout(typeGroup);
    typeGroupLayout->addWidget(colorAllType, 0, 0);
    typeGroupLayout->addWidget(colorRedType, 1, 0);
    typeGroupLayout->addWidget(colorGreenType, 2, 0);
    typeGroupLayout->addWidget(colorBlueType, 3, 0);
    typeGroupLayout->addWidget(colorAll2GrayType, 0, 1);
    typeGroupLayout->addWidget(colorRed2GrayType, 1, 1);
    typeGroupLayout->addWidget(colorGreen2GrayType, 2, 1);
    typeGroupLayout->addWidget(colorBlue2GrayType, 3, 1);
    typeGroupLayout->addWidget(colorMapDispType, 4, 0, 1, 2);

    typeGroupLayout->addWidget(imgValScaleDisplayCheckBox, 5, 0, 1, 2);


	//put all in a QWidget
    QWidget* colorForm = new QWidget(this);
    QVBoxLayout* colorFormLayout = new QVBoxLayout(colorForm);
    colorFormLayout->setContentsMargins(0,0,0,0); //remove margins
    colorFormLayout->addWidget(typeGroup);
	return colorForm;
}

#endif


void XFormWidget::createGUI()
{
	if (bExistGUI)
		return;

	bLinkFocusViews = true;
	bDisplayFocusCross = true;

     /* Set up the data related GUI */
     dataGroup = new QGroupBox(this);
     dataGroup->setTitle("Image data");

     viewGroup = new QGroupBox(dataGroup);
     viewGroup->setTitle("Views [XY: upper-left] [ZY: upper-right] [XZ: lower-left]");
     //viewGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

     xy_view = new XFormView(viewGroup);
	xy_view->setImgData(imgPlaneZ, 0, colorRGB); //because the second parameter is 0 (NULL pointer), then just load the default maps for this view
     xy_view->setFixedWidth(xy_view->get_disp_width()*1.5);
     xy_view->setFixedHeight(xy_view->get_disp_height()*1.5);
     xy_view->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
     xy_view->setFocusPolicy(Qt::ClickFocus);

     yz_view = new XFormView(viewGroup);
	yz_view->setImgData(imgPlaneX, 0, colorRGB); //because the second parameter is 0 (NULL pointer), then just load the default maps for this view
     yz_view->setFixedWidth(yz_view->get_disp_width()*1.5);
     yz_view->setFixedHeight(yz_view->get_disp_height()*1.5);
    yz_view->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
     yz_view->setFocusPolicy(Qt::ClickFocus);

     zx_view = new XFormView(viewGroup);
	zx_view->setImgData(imgPlaneY, 0, colorRGB); //because the second parameter is 0 (NULL pointer), then just load the default maps for this view
     zx_view->setFixedWidth(zx_view->get_disp_width()*1.5);
     zx_view->setFixedHeight(zx_view->get_disp_height()*1.5);
    zx_view->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
     zx_view->setFocusPolicy(Qt::ClickFocus);

    //    viewGroup->setFixedWidth(xy_view->frameGeometry().width()+yz_view->frameGeometry().width());

     // information group

     infoGroup = new QGroupBox(dataGroup);
     infoGroup->setTitle("Information of your selections");

     focusPointFeatureWidget = new MyTextBrowser(infoGroup);
	//	focusPointFeatureWidget->setFixedWidth(qMax(200, xy_view->width()+yz_view->width()));
	focusPointFeatureWidget->setFixedWidth(qMax(200, xy_view->get_disp_width()+yz_view->get_disp_width()));
	//focusPointFeatureWidget->setFixedHeight(50);
	//focusPointFeatureWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

     //viewGroup->setFixedWidth(xy_view->width()+yz_view->width()+10);
	//    viewGroup->setMinimumSize(xy_view->width()+yz_view->width(), xy_view->height()+zx_view->height()+50);
	//    viewGroup->setFixedWidth(xy_view->width()+yz_view->width()+10);

	//    dataGroup->setFixedWidth(xy_view->frameGeometry().width()+yz_view->frameGeometry().width()+10);

    // setup the control panel

    mainGroup = new QGroupBox(this);
    mainGroup->setFixedWidth(300);
    mainGroup->setTitle("Options");

    // focus planes group

    coordGroup = new QGroupBox(mainGroup);
    coordGroup->setAttribute(Qt::WA_ContentsPropagated);
    coordGroup->setTitle("Focus Coordinates");

    xSlider = new QScrollBar(Qt::Horizontal, coordGroup);
    xSlider->setRange(1, 1); //need redefine range
    xSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	xSliderLabel = new QLabel("X", coordGroup);

	xValueSpinBox = new QSpinBox;
    xValueSpinBox->setRange(1, 1);
    xValueSpinBox->setSingleStep(1);
    xValueSpinBox->setValue(yz_view->focusPlaneCoord());

    ySlider = new QScrollBar(Qt::Horizontal, coordGroup);
    ySlider->setRange(1, 1); //need redefine range
    ySlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	ySliderLabel = new QLabel("Y", coordGroup);

	yValueSpinBox = new QSpinBox;
    yValueSpinBox->setRange(1, 1);
    yValueSpinBox->setSingleStep(1);
    yValueSpinBox->setValue(zx_view->focusPlaneCoord());

    zSlider = new QScrollBar(Qt::Horizontal, coordGroup);
    zSlider->setRange(1, 1); //need redefine range
    zSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	zSliderLabel = new QLabel("Z", coordGroup);

	zValueSpinBox = new QSpinBox;
    zValueSpinBox->setRange(1, 1);
    zValueSpinBox->setSingleStep(1);
    zValueSpinBox->setValue(xy_view->focusPlaneCoord());

	linkFocusCheckBox = new QCheckBox("Anchor 3 Focal Views");
	linkFocusCheckBox->setCheckState((bLinkFocusViews) ? Qt::Checked : Qt::Unchecked);

	displayFocusCrossCheckBox = new QCheckBox("Focus cross"); //("Display Focus Cross Lines");
	displayFocusCrossCheckBox->setCheckState((bDisplayFocusCross) ? Qt::Checked : Qt::Unchecked);

    // scale factor group

    scaleGroup = new QGroupBox(mainGroup);
    scaleGroup->setAttribute(Qt::WA_ContentsPropagated);
    scaleGroup->setTitle("Zoom (Regular x1/4~x8, Looking glass x4)");

    xScaleSlider = new QScrollBar(Qt::Horizontal, scaleGroup);
    xScaleSlider->setRange(1, 32);
    xScaleSlider->setSingleStep(1);
    xScaleSlider->setValue(4);
    xScaleSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	xScaleSliderLabel = new QLabel("ZY-plane", scaleGroup);

    yScaleSlider = new QScrollBar(Qt::Horizontal, scaleGroup);
    yScaleSlider->setRange(1, 32);
    yScaleSlider->setSingleStep(1);
    yScaleSlider->setValue(4);
    yScaleSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	yScaleSliderLabel = new QLabel("XZ-plane", scaleGroup);

    zScaleSlider = new QScrollBar(Qt::Horizontal, scaleGroup);
    zScaleSlider->setRange(1, 32);
    zScaleSlider->setSingleStep(1);
    zScaleSlider->setValue(4);
    zScaleSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	zScaleSliderLabel = new QLabel("XY-plane", scaleGroup);

	lookingGlassCheckBox = new QCheckBox("Use looking glass");
	lookingGlassCheckBox->setCheckState(Qt::Unchecked);

	resetButton = new QPushButton(scaleGroup);
    resetButton->setText("Reset");

	zoomWholeViewButton = new QPushButton();
	zoomWholeViewButton->setText("TriView zoom=1. Click to set.");

	createMenuOfTriviewZoom();

    // color display group
	//moved to createColorGUI()

    //other control button
	cBox_bSendSignalToExternal = new QCheckBox("Link out");
	cBox_bSendSignalToExternal->setCheckState(Qt::Unchecked);

	cBox_bAcceptSignalFromExternal = new QCheckBox("Linked");
	cBox_bAcceptSignalFromExternal->setCheckState(Qt::Unchecked);

	//the landmark ctrl box

	QGroupBox *landmarkGroup  = new QGroupBox(mainGroup);
	landmarkGroup->setTitle("Landmark controls");

	landmarkCopyButton = new QPushButton(landmarkGroup);
	landmarkCopyButton->setText("Copy");

	landmarkPasteButton = new QPushButton(landmarkGroup);
	landmarkPasteButton->setText("Paste");

	landmarkSaveButton = new QPushButton(landmarkGroup);
	landmarkSaveButton->setText("Save");

	landmarkLoadButton = new QPushButton(landmarkGroup);
	landmarkLoadButton->setText("Load");

	landmarkManagerButton = new QPushButton(landmarkGroup);
	landmarkManagerButton->setText("Landmark/Atlas/Color Manager");

    imgV3DButton = new QPushButton(mainGroup);
    imgV3DButton->setText("See in 3D");

	createMenuOf3DViewer();

    whatsThisButton = new QPushButton(mainGroup);
    whatsThisButton->setText("Help ... ");
    //whatsThisButton->setCheckable(true);

    // All layouts

//    allLayout = new QHBoxLayout(this);
//    allLayout->addWidget(dataGroup);
//    allLayout->addWidget(mainGroup);

    xyzViewLayout = new QGridLayout(viewGroup);
    xyzViewLayout->addWidget(xy_view, 0, 0, 1, 1, Qt::AlignRight | Qt::AlignBottom);
    xyzViewLayout->addWidget(yz_view, 0, 1, 1, 1, Qt::AlignLeft | Qt::AlignBottom);
    xyzViewLayout->addWidget(zx_view, 1, 0, 1, 1, Qt::AlignRight | Qt::AlignTop);
	xyzViewLayout->update();//061014
	//xyzViewLayout->addWidget(focusPointFeatureWidget, 2, 0, 1, 2, Qt::AlignLeft | Qt::AlignBottom);


    infoGroupLayout = new QVBoxLayout(infoGroup);
	infoGroupLayout->addWidget(focusPointFeatureWidget);

    // @ADDED by Alessandro on 2015-05-07 : hide/display controls and infos.
    hideDisplayInfoButton = new QPushButton();
    hideDisplayInfoButton->setIcon(QIcon(":/pic/arrow_down.png"));
    hideDisplayInfoButton->setFixedWidth(100);
    hideDisplayInfoButton->setFixedHeight(25);
    connect(hideDisplayInfoButton, SIGNAL(clicked()), this, SLOT(hideDisplayInfo()));

    dataGroupLayout = new QVBoxLayout(dataGroup);
    dataGroupLayout->addWidget(viewGroup, 1);
    dataGroupLayout->addWidget(hideDisplayInfoButton, 0, Qt::AlignCenter);
    dataGroupLayout->addSpacing(5);
    dataGroupLayout->addWidget(infoGroup);
    dataGroupLayout->addStretch(0);

    // layout for focus planes

	coordGroupLayout = new QGridLayout(coordGroup);
    coordGroupLayout->addWidget(zSliderLabel, 0, 0, 1, 1);
	coordGroupLayout->addWidget(zSlider, 0, 1, 1, 13);
    coordGroupLayout->addWidget(zValueSpinBox, 0, 14, 1, 6);

    coordGroupLayout->addWidget(xSliderLabel, 1, 0, 1, 1);
	coordGroupLayout->addWidget(xSlider, 1, 1, 1, 13);
    coordGroupLayout->addWidget(xValueSpinBox, 1, 14, 1, 6);

    coordGroupLayout->addWidget(ySliderLabel, 2, 0, 1, 1);
	coordGroupLayout->addWidget(ySlider, 2, 1, 1, 13);
    coordGroupLayout->addWidget(yValueSpinBox, 2, 14, 1, 6);

//  coordGroupLayout->addWidget(linkFocusCheckBox, 3, 0, 1, 14);
//	coordGroupLayout->addWidget(displayFocusCrossCheckBox, 4, 0, 1, 14);
//	coordGroupLayout->addWidget(cBox_bSendSignalToExternal, 5, 0, 1, 6);
//	coordGroupLayout->addWidget(cBox_bAcceptSignalFromExternal, 5, 7, 1, 7);

	coordGroupLayout->addWidget(displayFocusCrossCheckBox, 		3, 0, 1, 8);
	coordGroupLayout->addWidget(cBox_bSendSignalToExternal,     3, 8, 1, 6);
	coordGroupLayout->addWidget(cBox_bAcceptSignalFromExternal, 3, 8+6, 1, 6);

	// layout for scaling factors

    scaleGroupLayout = new QGridLayout(scaleGroup);
    scaleGroupLayout->addWidget(zScaleSlider, 0, 0, 1, 13);
    scaleGroupLayout->addWidget(zScaleSliderLabel, 0, 14, 1, 6);

    scaleGroupLayout->addWidget(xScaleSlider, 1, 0, 1, 13);
    scaleGroupLayout->addWidget(xScaleSliderLabel, 1, 14, 1, 6);

    scaleGroupLayout->addWidget(yScaleSlider, 2, 0, 1, 13);
    scaleGroupLayout->addWidget(yScaleSliderLabel, 2, 14, 1, 6);

    scaleGroupLayout->addWidget(lookingGlassCheckBox, 3, 0, 1, 13);
    scaleGroupLayout->addWidget(resetButton, 3, 14, 1, 6);

    scaleGroupLayout->addWidget(zoomWholeViewButton, 4, 0, 1, 20);

    // color display layout
    //moved to createColorGUI();

	//landmark group

	LandmarkGroupLayout = new QGridLayout(landmarkGroup);
	LandmarkGroupLayout->addWidget(landmarkCopyButton, 0, 0, 1, 4);
	LandmarkGroupLayout->addWidget(landmarkPasteButton, 0, 5, 1, 4);
	LandmarkGroupLayout->addWidget(landmarkLoadButton, 0, 10, 1, 4);
	LandmarkGroupLayout->addWidget(landmarkSaveButton, 0, 15, 1, 4);

	//LandmarkGroupLayout->addWidget(landmarkLabelDispCheckBox, 1, 0, 1, 11);


	//110719 RZC for spacing between buttons, because setSpacing(0) between GroupBox
	QWidget* btnArea = new QWidget(this);
	QVBoxLayout* btnLayout = new QVBoxLayout(btnArea);
	//btnLayout->setContentsMargins(0,0,0,0); //remove margins
    btnLayout->addWidget(landmarkManagerButton);
    btnLayout->addWidget(imgV3DButton);
    btnLayout->addStretch(0);
    btnLayout->addWidget(whatsThisButton);

	// main control panel layout ====================================

    mainGroupLayout = new QVBoxLayout(mainGroup);
    mainGroupLayout->setSpacing(0); //remove spacing
    mainGroupLayout->addWidget(coordGroup);
    mainGroupLayout->addWidget(scaleGroup);
    mainGroupLayout->addWidget(createColorGUI()); //typeGroup); //110719 RZC
    mainGroupLayout->addWidget(landmarkGroup); //080107
	mainGroupLayout->addWidget(btnArea); //110719 RZC

	// force layout set
	QLayout *cur_layout=layout();
	printf("cur layout=%ld\n", V3DLONG(cur_layout));

    // @ADDED by Alessandro on 2015-05-07 : hide/display controls and infos.
    hideDisplayControlsButton = new QPushButton();
    hideDisplayControlsButton->setIcon(QIcon(":/pic/arrow_right.png"));
    hideDisplayControlsButton->setFixedHeight(100);
    hideDisplayControlsButton->setFixedWidth(25);

    //hideDisplayControlsButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    connect(hideDisplayControlsButton, SIGNAL(clicked()), this, SLOT(hideDisplayControls()));

    allLayout = new QHBoxLayout(this);
    allLayout->addWidget(dataGroup, 1);
    allLayout->addWidget(hideDisplayControlsButton);
    allLayout->addWidget(mainGroup);
//	setLayout(allLayout);

	// set the flag
	bExistGUI = true;

	setFocusPolicy(Qt::StrongFocus);
	this->setFocus();
	//QTimer::singleShot(500, this, SLOT(Focus()));
}

// @ADDED by Alessandro on 2015-05-07 : hide/display controls.
void XFormWidget::hideDisplayControls()
{
    if(mainGroup->isVisible())
    {
        mainGroup->setVisible(false);
        hideDisplayControlsButton->setIcon(QIcon(":/pic/arrow_left.png"));
    }
    else
    {
        mainGroup->setVisible(true);
        hideDisplayControlsButton->setIcon(QIcon(":/pic/arrow_right.png"));
    }
}
void XFormWidget::hideDisplayInfo()
{
    if(infoGroup->isVisible())
    {
        infoGroup->setVisible(false);
        hideDisplayInfoButton->setIcon(QIcon(":/pic/arrow_up.png"));
    }
    else
    {
        infoGroup->setVisible(true);
        hideDisplayInfoButton->setIcon(QIcon(":/pic/arrow_down.png"));
    }
}

void XFormWidget::updateDataRelatedGUI()
{
	if (imgData)
	{
		// the data of tri-view planes

		xy_view->setImgData(imgPlaneZ, imgData, Ctype);
		if (b_use_dispzoom)
		{
			xy_view->set_disp_width(imgData->getXDim()*disp_zoom);
			xy_view->set_disp_height(imgData->getYDim()*disp_zoom);
			xy_view->set_disp_scale(disp_zoom);
		}
		else
		{
			xy_view->set_disp_width(imgData->getXDim());
			xy_view->set_disp_height(imgData->getYDim());
			xy_view->set_disp_scale(1);
		}
		xy_view->setFixedWidth(xy_view->get_disp_width());
		xy_view->setFixedHeight(xy_view->get_disp_height());
		imgData->set_xy_view(xy_view);

		//
		yz_view->setImgData(imgPlaneX, imgData, Ctype);
		if (b_use_dispzoom)
		{
			yz_view->set_disp_width(imgData->getZDim()*disp_zoom);
			yz_view->set_disp_height(imgData->getYDim()*disp_zoom);
			yz_view->set_disp_scale(disp_zoom);
		}
		else
		{
			yz_view->set_disp_width(imgData->getZDim());
			yz_view->set_disp_height(imgData->getYDim());
			yz_view->set_disp_scale(1);
		}
		yz_view->setFixedWidth(yz_view->get_disp_width());
		yz_view->setFixedHeight(yz_view->get_disp_height());
		imgData->set_yz_view(yz_view);

		//
		zx_view->setImgData(imgPlaneY, imgData, Ctype);
		if (b_use_dispzoom)
		{
			zx_view->set_disp_width(imgData->getXDim()*disp_zoom);
			zx_view->set_disp_height(imgData->getZDim()*disp_zoom);
			zx_view->set_disp_scale(disp_zoom);
		}
		else
		{
			zx_view->set_disp_width(imgData->getXDim());
			zx_view->set_disp_height(imgData->getZDim());
			zx_view->set_disp_scale(1);
		}
		zx_view->setFixedWidth(zx_view->get_disp_width());
		zx_view->setFixedHeight(zx_view->get_disp_height());
		imgData->set_zx_view(zx_view);

		if (b_use_dispzoom)
		{
			focusPointFeatureWidget->setFixedWidth(qMax(200, int(imgData->getXDim()*disp_zoom+imgData->getZDim()*disp_zoom)));
		}
		else
		{
			focusPointFeatureWidget->setFixedWidth(qMax(200, int(imgData->getXDim()+imgData->getZDim())));
		}
		focusPointFeatureWidget->setMinimumHeight(100);
		imgData->setFocusFeatureView((MyTextBrowser*)focusPointFeatureWidget);

		imgData->setMainWidget((XFormWidget *)this);

		//viewGroup->setFixedWidth(imgData->getXDim() + imgData->getZDim() + 20);

		// range of scroll bars of focus planes

		xSlider->setRange(1, imgData->getXDim()); //need redefine range
		xValueSpinBox->setRange(1, imgData->getXDim());
		xSlider->setValue(1);
		imgData->setFocusX(xSlider->value());

		ySlider->setRange(1, imgData->getYDim()); //need redefine range
		yValueSpinBox->setRange(1, imgData->getYDim());
		ySlider->setValue(1);
		imgData->setFocusY(ySlider->value());

		zSlider->setRange(1, imgData->getZDim()); //need redefine range
		zValueSpinBox->setRange(1, imgData->getZDim());
		zSlider->setValue(1);
		imgData->setFocusZ(zSlider->value());

		linkFocusCheckBox->setEnabled(true);
		displayFocusCrossCheckBox->setEnabled(true);

		// external communication

		cBox_bSendSignalToExternal->setEnabled(true);
		cBox_bAcceptSignalFromExternal->setEnabled(true);

		// position of scales

		xScaleSlider->setValue(4);
		yScaleSlider->setValue(4);
		zScaleSlider->setValue(4);

		lookingGlassCheckBox->setEnabled(true);
		lookingGlassCheckBox->setChecked(false);
		toggleLookingGlassCheckBox(); //this is used to set the correct enable for the zoom-in sliders

		zoomWholeViewButton->setText(QString("Tri-view zoom=%1. Click to set.").arg(disp_zoom));


		// update color channel options
		setColorGUI(); //110721 RZC

		//landmarkLabelDispCheckBox->setEnabled(true);

		//imgProcessButton->setCheckable(true); //080402
		imgV3DButton->setEnabled(true); //100816. always enable, but will display a warning when the user click it.
//		if (imgData->getDatatype()==V3D_UINT8)
//		{
//			imgV3DButton->setEnabled(true);
//			//imgV3DROIButton->setEnabled(true);
//		}
//		else
//		{
//			imgV3DButton->setEnabled(false);
//			//imgV3DROIButton->setEnabled(false);
//		}
		//setLayout(viewLayout);

		//resize(minimumSize());

		// main window title
		//setWindowTitle(openFileNameLabel.prepend("v3d: "));
		setWindowTitle(openFileNameLabel); //061011

		//added 081124
		imgData->updateViews();
	}
	else
	{
		// range of scroll bars of focus planes

		xSlider->setRange(1, 1); //need redefine range
		xValueSpinBox->setRange(1, 1);

		ySlider->setRange(1, 1); //need redefine range
		yValueSpinBox->setRange(1, 1);

		zSlider->setRange(1, 1); //need redefine range
		zValueSpinBox->setRange(1, 1);

		linkFocusCheckBox->setEnabled(false);
		displayFocusCrossCheckBox->setEnabled(false);

		// external communication

		cBox_bSendSignalToExternal->setEnabled(false);
		cBox_bAcceptSignalFromExternal->setEnabled(false);

		lookingGlassCheckBox->setEnabled(false);

		zoomWholeViewButton->setText("Set tri-view zoom");


		// color channel options
		setColorGUI(); //110721 RZC


		//landmarkLabelDispCheckBox->setEnabled(true);

		//imgProcessButton->setCheckable(false); //080402
		imgV3DButton->setEnabled(false);
		//imgV3DROIButton->setEnabled(false);

		//setLayout(viewLayout);

		// set main window title
		setWindowTitle("v3d: no data loaded yet - click the Load Stack Image button to load data.");
	}

#if COMPILE_TARGET_LEVEL == 0 //disable the landmark control box if use the V3D lite
	if (landmarkCopyButton) landmarkCopyButton->setEnabled(false);
	if (landmarkPasteButton) landmarkPasteButton->setEnabled(false);
	if (landmarkLoadButton) landmarkLoadButton->setEnabled(false);
	if (landmarkSaveButton) landmarkSaveButton->setEnabled(false);
	if (landmarkManagerButton) landmarkManagerButton->setEnabled(false);
#endif

	allLayout->update();

	//set the default focus location
	if (imgData)
	{
		emit external_focusXChanged(imgData->getXDim()>>1);
		emit external_focusYChanged(imgData->getYDim()>>1);
		emit external_focusZChanged(imgData->getZDim()>>1);
	}

	//110801 RZC, fixed MDI window resize
	{
		QWidget* w = parentWidget(); // MDI child wrapper widget
		if (w)  w->resize(w->size() - QSize(1,1));	//works!
		if (w)  w->adjustSize();					//also need for perfect
	}

//	updateGeometry(); //seems no need
	adjustSize();
	allLayout->update();

	update();
}

void XFormWidget::setWindowTitle_Prefix(const char *prefix)
{
	if (!openFileNameLabel.startsWith(prefix)) //only prepend the prefix if it has not been prepended before
	{
		setWindowTitle(openFileNameLabel.prepend(prefix));
		getImageData()->setFileName((char *)(openFileNameLabel.constData())); //also update the file name
	}
}

void XFormWidget::setWindowTitle_Suffix(const char *sfix)
{
	if (!openFileNameLabel.endsWith(sfix)) //only append suffix if it has not been appended before
	{
		setWindowTitle(openFileNameLabel.append(sfix));
		getImageData()->setFileName((char *)qPrintable(openFileNameLabel)); //also update the file name
	}
}

bool XFormWidget::getFlagImgValScaleDisplay()
{
	if (imgValScaleDisplayCheckBox)
		return (imgValScaleDisplayCheckBox->checkState()==Qt::Checked) ? true : false;
	else
		return false;
}

void XFormWidget::setOpenFileName()
{
    QString tmp = QFileDialog::getOpenFileName(this,
											   tr("Select a stack image file to open ... "),
											   openFileNameLabel,
											   tr("V3D's Raw image stack (*.v3draw; *.raw);TIFF stacks (*.tif; *.tiff);All Files (*)"));
	if (!tmp.isEmpty()) //note that I used isEmpty() instead of isNull, although seems the Cancel operation will return a null string. phc 060422
	{
		openFileNameLabel = tmp;
		loadData();
	}
}

bool XFormWidget::loadFile(QString filename)
{
	if (!filename.isEmpty())
	{
		openFileNameLabel = filename;
		return loadData();
	}
	else
	{
		return false;
	}
}

bool XFormWidget::loadData()
{
	//try to get a rough estimation of available amount of memory
	V3DLONG nbytes = estimateRoughAmountUsedMemory();
    if (nbytes>(unsigned V3DLONG)((double(1024)*1024*1024*TH_USE_MEMORY)))
	{
        printf("machine info: double upper limit =%5.4f V3DLONG upper limit=%ld\n long_n_bytes=%zd",
               (double(1024)*1024*1024*TH_USE_MEMORY), (V3DLONG)(double(1024)*1024*1024*TH_USE_MEMORY), sizeof(V3DLONG));
        v3d_msg(QString("You already used about %1 bytes of memory, which is more than %2 G bytes for your images. "
                        "Please either enlarge the memory threshold (in Preferences) or close some stacks to ensure you have enough memory.\n").arg(nbytes).arg(TH_USE_MEMORY));
		return false;
	}

	// trying to load image using multithreaded method, by YuY, added 20101216
	if(bUsingMultithreadedImageIO)
	{
		//set up parameters
		v3d_multithreadimageio_paras myimgiop;
		myimgiop.qOperation = "HIDDEN-MTIMGIO";
		myimgiop.qFilename = openFileNameLabel;

		//v3d_msg(QString("Test Parameters %1 and %2.\n").arg(myimgiop.qOperation).arg(myimgiop.qFilename.toStdString().c_str()));

		//load image
		bool bloadsuccess = v3d_multithreadimageIO(this, myimgiop);

		if(bloadsuccess)
		{
			v3d_msg("File is loaded successfully by multithreaded image loading engine!\n", 0);
			return true;
		}
	}

	//the following are the original codes

	if (imgData)
	{
		cleanData();
	}

	imgData = new My4DImage;
	if (!imgData)
		return false;
	else {
		imgData->setMainWidget((XFormWidget *)this); //by PHC, added 100904 to ensure imgData can access global setting
	}

	// The previous version of toAscii().data() worked fine on OS X and Linux
	// but barfs on Windows. According to the Qt docs using toAscii and its ilk
	// has a memory leak, which apparently corrupts the pointer on Windows
	QByteArray ba = openFileNameLabel.toUtf8();
	const char* filename = ba.constData();
	const char * curFileSurfix = getSuffix(filename);

	if ( curFileSurfix && strcasecmp(curFileSurfix, "mp4") == 0 )
		loadH264Image( filename );
	else if ( curFileSurfix && strcasecmp(curFileSurfix, "h5j") == 0 )
		loadHDF5( filename );
	else
		imgData->loadImage(filename);  // imgData->loadImage("/Users/hanchuanpeng/work/v3d/test1.raw");
	if (imgData->isEmpty())
	{
		delete imgData; imgData = 0;
		v3d_msg("File open error : Fail to open the image file you specified. This could be due to \n(1) The file does not exist. \n(2) The file format is not supported (V3D only supports TIFF stacks, most Zeiss LSM files, and Hanchuan Peng's RAW files. TIFF stacks are widely used and can be easily generated using other tools such as ImageJ or Hanchuan Peng's Matlab image fileio toolbox). \n(3) Your image file is too big. Since on 32-bit machines, an image is at most 2G bytes, and opening tiff files need extra-space for temporary buffer, thus currently V3D has a limitaton on the size of images: TIFF and LSM files < 900M Bytes, and V3D's RAW file < 1.5G bytes. You can contact Hanchuan Peng to get a special version of V3D to handle very big image files.\n", 0);
		v3d_msg("Fail to open the image file you specified. This could be due to <br><br>"
				"(1) The file does not exist. <br>"
                "(2) The file format is not supported. <br>"
				"(3) Your image file is too big. Since on 32-bit machines, an image is at most 2G bytes, and opening tiff files need extra-space for temporary buffer, thus currently V3D has a limitaton on the size of images: TIFF and LSM files less than 900M Bytes, and Hanchuan's RAW file less than 1.5G bytes. You can contact Hanchuan Peng to get a special version of V3D to handle very big image files.<br>");
		return false;
	}

	v3d_msg(QString("img data size %1 %2 %3 %4\n").arg(imgData->getXDim()).arg(imgData->getYDim()).arg(imgData->getZDim()).arg(imgData->getCDim()), 0);

	setCTypeBasedOnImageData();

	imgData->setFlagLinkFocusViews(bLinkFocusViews);
	imgData->setFlagDisplayFocusCross(bDisplayFocusCross);

	//now set the disp_zoom. 081114

	if (imgData->getXDim()>512 || imgData->getYDim()>512 || imgData->getZDim()>512)
	{
		disp_zoom= double(512) / qMax(imgData->getXDim(), qMax(imgData->getYDim(), imgData->getZDim()));
		b_use_dispzoom=true;
	}

     // save imgData for use in saveDataFor3DViewer() in MainWindow.cpp. ZJL 111021
     mypara_3Dview.image4d = imgData;

	// update the interface
	updateDataRelatedGUI();

    //v3d_msg(QString("The current file has the name [%1]").arg(imgData->getFileName()));

	reset(); //090718. PHC. force to update once, since sometimes the 16bit image does not display correctly (why all black but once click reset button everything correct?)
	return true;

}

void XFormWidget::loadH264Image( char const* filename )
{
#ifdef USE_FFMPEG
    v3d_msg( "start to try H.264 loader", 0 );

    if ( !loadStackFFMpeg(filename, *imgData) )
    {
        v3d_msg( "Error happened in H.264 file reading. Stop. \n", false );
        return;
    }
    imgData->setupData4D();
    imgData->setFileName( filename ); // PHC added 20121213 to fix a bug in the PDB reading.
#endif
}

void XFormWidget::loadHDF5( char const* filename )

{
#ifdef USE_HDF5
    v3d_msg( "start to try HDF loader", 0 );

    if ( !loadStackHDF5(filename, *imgData) )
    {
        v3d_msg( "Error happened in HDF5 file reading. Stop. \n", false );
        return;
    }
    imgData->setupData4D();
    imgData->setFileName( filename );

#endif
}

bool XFormWidget::setCTypeBasedOnImageData() //separate this out on 2010-08-01. by PHC
{
    if (!imgData || !imgData->valid())
	{
		v3d_msg("Invalid data in setCTypeBasedOnImageData()");
		return false;
	}

    if (imgData->getCDim()<1)
	{
		printf("Error in data reading. The number of color channels cannot be smaller than 1!!\n");
		if (imgData) {delete imgData; imgData = 0;}
		return false;
	}

	if (imgData->getDatatype()==V3D_UINT8 ||
		imgData->getDatatype()==V3D_UINT16 ||
		imgData->getDatatype()==V3D_FLOAT32)
	{
		if (imgData->getCDim()>=3)
			Ctype = colorRGB;
		else if (imgData->getCDim()==2)
			Ctype = colorRG;
		else //==1
			Ctype = colorRed2Gray;
	}
//	else if (imgData->getDatatype()==V3D_UINT16) //080824
//	{
//		Ctype = colorPseudoMaskColor;
//	}
	else
	{
		v3d_msg("Seems you load an unknown data which is not supported for display at this moment. -- setCTypeBasedOnImageData()", 0);
		Ctype = colorRed2Gray;
	}

	return true;
}

bool XFormWidget::setCurrentFileName(QString cfilename)
{
	qDebug()<<"XFormWidget::setCurrentFileName: " <<cfilename;
	if (cfilename.isEmpty()) return false;
	openFileNameLabel = cfilename;
	setWindowTitle(openFileNameLabel);
	if (getImageData())
		getImageData()->setFileName((char *)qPrintable(openFileNameLabel));
	update();
	return true;
}

bool XFormWidget::saveData()
{
	if (!imgData) {printf("Image data is empty!\n"); return false;}

	QString outputFile = QFileDialog::getSaveFileName(0,
													  "Choose a filename to save under",
													  //"./",
													  QString(openFileNameLabel)+".v3draw",
                                                      "Save file format (*.v3draw *.v3dpbd *.tif *.raw *.nrrd *.nhdr)");

	while (outputFile.isEmpty()) //note that I used isEmpty() instead of isNull, although seems the Cancel operation will return a null string. phc 060422
	{
    	if(QMessageBox::Yes == QMessageBox::question (0, "", "Are you sure you do NOT want to save?", QMessageBox::Yes, QMessageBox::No))
	    {
		    return false;
		}
		outputFile = QFileDialog::getSaveFileName(0,
												  "Choose a filename to save under",
												  "./",
                                                  "Save file format (*.v3draw *.v3dpbd *.tif *.raw *.nrrd *.nhdr)");
	}

	saveFile(outputFile);

	return true;
}
bool XFormWidget::saveFile(QString filename)
{
	if (!imgData) {printf("Image data is empty!\n"); return false;}
	if (filename.isEmpty()) {printf("The file name to save image is empty!\n"); return false;}

	imgData->saveImage(qPrintable(filename));

	printf("Current image is saved to the file %s\n", qPrintable(filename));
	setCurrentFileName(filename);
	return true;
}



bool XFormWidget::newProcessedImage(QString filename, unsigned char *ndata1d, V3DLONG nsz0, V3DLONG nsz1, V3DLONG nsz2, V3DLONG nsz3, ImagePixelType ndatatype) //080408
{
	if (filename.isEmpty()) return false;
	openFileNameLabel = filename;

	return setImageData(ndata1d, nsz0, nsz1, nsz2, nsz3, ndatatype)
	&& setCurrentFileName(filename);
}

bool XFormWidget::setImageData(unsigned char *ndata1d, V3DLONG nsz0, V3DLONG nsz1, V3DLONG nsz2, V3DLONG nsz3, ImagePixelType ndatatype) //090818 RZC
{

	//	printf("he %p\n", ndata1d);
	if (imgData) { cleanData();	}
	imgData = new My4DImage;
	if (!imgData)  return false;

	//////////////////////////
	updateDataRelatedGUI(); //this should be very important //but strange logic
	//////////////////////////

	printf("now in the function setImageData() line=%d.\n", __LINE__);
	if (!imgData->setNewImageData(ndata1d, nsz0, nsz1, nsz2, nsz3, ndatatype))
	{
		printf("Sth wrong in the function setImageData() line=%d.\n", __LINE__);
		return false;
	}

	imgData->setFileName((char *)qPrintable(openFileNameLabel));

    if (imgData->getCDim()>=3)
	    Ctype = colorRGB;
    else if (imgData->getCDim()==2)
	    Ctype = colorRG;
	else //==1
	    Ctype = colorRed2Gray;

    imgData->setFlagLinkFocusViews(bLinkFocusViews);
    imgData->setFlagDisplayFocusCross(bDisplayFocusCross);

    //imgData->setFlagImgValScaleDisplay((imgValScaleDisplayCheckBox->checkState()==Qt::Checked) ? true : false);


	//now set the disp_zoom. 081114

	if (imgData->getXDim()>512 || imgData->getYDim()>512 || imgData->getZDim()>512)
	{
		disp_zoom= double(512) / qMax(imgData->getXDim(), qMax(imgData->getYDim(), imgData->getZDim()));
		b_use_dispzoom=true;
	}


	////////////////////////////
    updateDataRelatedGUI();
    ////////////////////////////

	v3d_msg("succeed in setting up image data.\n", 0);
	return true;
}

void XFormWidget::doImage3DView()
{
	doImage3DView(true, 0); //0 for entire image
}


void XFormWidget::doImageVRView()//wwbchange
{
	NeuronTree nt;
	nt.listNeuron.clear();
	nt.hashNeuron.clear();
	//doimageVRViewer_v2(nt,0);
	doimageVRViewer(nt);

}
void XFormWidget::doImage3DLocalMarkerView()
{
	if (!imgData)  return;
	doImage3DView(true, 1); //1 for marker
}

void XFormWidget::doImage3DLocalRoiView()
{
	if (!imgData)  return;
	doImage3DView(true, 2); //2 for roi
}

void XFormWidget::doImage3DLocalBBoxView()  //do not have arguments so that can be used as the slot of a timer signal
{
	if (!imgData)  return;
	doImage3DView(true, 3, bbx0, bbx1, bby0, bby1, bbz0, bbz1); //3 for bbox
}

void XFormWidget::doImage3DView(bool tmp_b_use_512x512x256, int b_local, V3DLONG bbx0, V3DLONG bbx1, V3DLONG bby0, V3DLONG bby1, V3DLONG bbz0, V3DLONG bbz1, bool show)
	//b_local==0, use entire image
	//b_local==1, use marker;
	//b_local==2, use roi;
	//b_local==3, use lower and upper bounding box in bbx0, bby0, ....
{
	if (!b_local && mypara_3Dview.b_still_open)
	{
		mypara_3Dview.window3D->raise_and_activate(); // activateWindow();
		return;
	}
	if (b_local && mypara_3Dlocalview.b_still_open)
	{
		//mypara_3Dlocalview.window->raise_and_activate();

		//090723: continue create a new view, wait 1 second for the last local 3D view closed
		mypara_3Dlocalview.window3D->postClose();
		if (b_local==1)
			QTimer::singleShot(1000, this, SLOT(doImage3DLocalMarkerView()));
		else if (b_local==2)
			QTimer::singleShot(1000, this, SLOT(doImage3DLocalRoiView()));
		else if (b_local==3)
			QTimer::singleShot(1000, this, SLOT(doImage3DLocalBBoxView())); //do not have arguments so that can be used as the slot of a timer signal
		else
			v3d_msg("Invalid b_local parameter in doImage3DView();");

		return;
	}

	if (imgData)
	{
		V3DLONG nbytes = estimateRoughAmountUsedMemory();
        if (nbytes>(V3DLONG)((double(1024)*1024*1024*TH_USE_MEMORY)))
		{
            v3d_msg(QString("You already used more than %1G bytes for your images. Please close some stacks to assure you have enough memory.").arg(TH_USE_MEMORY));
			return;
		}

		if (! b_local) //0 for entire image
		{
			//iDrawExternalParameter mypara;
			mypara_3Dview.image4d = imgData;
			mypara_3Dview.b_use_512x512x256 = tmp_b_use_512x512x256;
			mypara_3Dview.xwidget = this; //imgData->listLandmarks;
			mypara_3Dview.V3Dmainwindow = p_mainWindow; //added on 090503
			mypara_3Dview.p_list_3Dview_win = &(p_mainWindow->list_3Dview_win); //081003: always keep an record in the central controller

			mypara_3Dlocalview.b_local = b_local;
		}
		if (b_local==1 || b_local==2 || b_local==3)
		{
			V3DLONG x0, y0, z0, x1, y1, z1;

			switch (b_local)
			{
				case 1: //1 for marker
					if (imgData->listLandmarks.size()>0)
					{
						//get the current marker
						LocationSimple *pt = 0;
						LocationSimple mypt;
						mypt = imgData->listLandmarks.at(imgData->cur_hit_landmark);
						mypt.x-=1; mypt.y-=1; mypt.z-=1;
						pt = &mypt;

						x0 = qBound((V3DLONG)0L, (V3DLONG)((*pt).x-64) , (imgData->getXDim()-1));
						y0 = qBound((V3DLONG)0L, (V3DLONG)((*pt).y-64) , (imgData->getYDim()-1));
						z0 = qBound((V3DLONG)0L, (V3DLONG)((*pt).z-64) , imgData->getZDim()-1);
						x1 = qBound((V3DLONG)0L, (V3DLONG)((*pt).x+63) , imgData->getXDim()-1);
						y1 = qBound((V3DLONG)0L, (V3DLONG)((*pt).y+63) , imgData->getYDim()-1);
						z1 = qBound((V3DLONG)0L, (V3DLONG)((*pt).z+63) , imgData->getZDim()-1);
						//					c0 = 0;
						//					c1 = sz3-1;
					}
					break;

				case 3:
					x0 = qBound((V3DLONG)(bbx0), V3DLONG(0), imgData->getXDim()-1);
					y0 = qBound((V3DLONG)(bby0), V3DLONG(0), imgData->getYDim()-1);
					z0 = qBound((V3DLONG)(bbz0), V3DLONG(0), imgData->getZDim()-1);
					x1 = qBound((V3DLONG)(bbx1), V3DLONG(0), imgData->getXDim()-1);
					y1 = qBound((V3DLONG)(bby1), V3DLONG(0), imgData->getYDim()-1);
					z1 = qBound((V3DLONG)(bbz1), V3DLONG(0), imgData->getZDim()-1);

					break;

				case 2: //2 for roi
				default:
					QRect b_xy = imgData->p_xy_view->getRoiBoundingRect();
					QRect b_yz = imgData->p_yz_view->getRoiBoundingRect();
					QRect b_zx = imgData->p_zx_view->getRoiBoundingRect();

					V3DLONG bpos_x = qBound((V3DLONG)(0), V3DLONG(qMax(b_xy.left(), b_zx.left())), imgData->getXDim()-1),
					bpos_y = qBound((V3DLONG)(0), V3DLONG(qMax(b_xy.top(),  b_yz.top())), imgData->getYDim()-1),
					bpos_z = qBound((V3DLONG)(0), V3DLONG(qMax(b_yz.left(), b_zx.top())), imgData->getZDim()-1),
					bpos_c = 0;
					V3DLONG epos_x = qBound((V3DLONG)(0), V3DLONG(qMin(b_xy.right(), b_zx.right())), imgData->getXDim()-1),
					epos_y = qBound((V3DLONG)(0), V3DLONG(qMin(b_xy.bottom(), b_yz.bottom())), imgData->getYDim()-1),
					epos_z = qBound((V3DLONG)(0), V3DLONG(qMin(b_yz.right(), b_zx.bottom())), imgData->getZDim()-1),
					epos_c = imgData->getCDim()-1;

					if (bpos_x>epos_x || bpos_y>epos_y || bpos_z>epos_z)
					{
						v3d_msg("The roi polygons in three views are not intersecting! No crop is done!\n");
						return;
					}

					x0 = bpos_x;
					y0 = bpos_y;
					z0 = bpos_z;
					x1 = epos_x;
					y1 = epos_y;
					z1 = epos_z;

					break;
			}

			mypara_3Dlocalview.image4d = imgData;
			mypara_3Dlocalview.b_use_512x512x256 = tmp_b_use_512x512x256;
			mypara_3Dlocalview.xwidget = this; //imgData->listLandmarks;
			mypara_3Dlocalview.V3Dmainwindow = p_mainWindow; //added on 090503
			mypara_3Dlocalview.p_list_3Dview_win = &(p_mainWindow->list_3Dview_win); //081003: always keep an record in the central controller

			mypara_3Dlocalview.b_local = b_local;
			mypara_3Dlocalview.local_size = LocationSimple(x1-x0+1, y1-y0+1, z1-z0+1);
			mypara_3Dlocalview.local_start = LocationSimple(x0, y0, z0);

			//			if (mypara_3Dlocalview.localimage4d)
			//				delete mypara_3Dlocalview.localimage4d;
			//			mypara_3Dlocalview.localimage4d = 0;
			//			mypara_3Dlocalview.localimage4d = new My4DImage();
			//			if (mypara_3Dlocalview.localimage4d)
			//				mypara_3Dlocalview.localimage4d->setNewImageData(imgData->getRawData(),
			//						mypara_3Dlocalview.local_size.x,
			//						mypara_3Dlocalview.local_size.y,
			//						mypara_3Dlocalview.local_size.z,
			//						imgData->getCDim(), imgData->getDatatype());
		}

		V3dR_MainWindow *my3dwin = 0;
		try
		{
			if (b_local)
			{
				my3dwin = new V3dR_MainWindow(&mypara_3Dlocalview); //090628 RZC
				mypara_3Dlocalview.window3D = my3dwin;
			}
			else
			{
				my3dwin = new V3dR_MainWindow(&mypara_3Dview); //iDrawMainWindow-->V3dR_MainWindow, by RZC 20080921
				mypara_3Dview.window3D = my3dwin;
			}
			my3dwin->setParent(0);

            // @ADDED by Alessandro on 2015-09-29. Postpone show() if required.
            if(show)
                my3dwin->show();
		}
		catch (...)
		{
			v3d_msg("You fail to open a 3D view window. You may have opened too many stacks (if so please close some first) or "
                    "try to render a too-big 3D view (if so please contact Hanchuan Peng for a 64-bit version of Vaa3D).");
			return;
		}
	}
	else
	{
		v3d_msg("The image data is invalid() in doImage3DView().\n", 0);
		return;
	}
}

void XFormWidget::popupImageProcessingDialog()
{
	popupImageProcessingDialog(QString(""));
}
void XFormWidget::popupImageProcessingDialog(QString item)
{
	if (imgData && xy_view)
	{
		xy_view->popupImageProcessingDialog(item);
		if (item==tr(" -- clear the ROI"))
		{
			zx_view->popupImageProcessingDialog(item);
			yz_view->popupImageProcessingDialog(item);
		}
	}
}

void XFormWidget::aboutInfo()
{
	v3d_aboutinfo();
}

void XFormWidget::connectEventSignals()
{
	connect(xSlider, SIGNAL(valueChanged(int)), yz_view, SLOT(changeFocusPlane(int)));
    connect(ySlider, SIGNAL(valueChanged(int)), zx_view, SLOT(changeFocusPlane(int)));
    connect(zSlider, SIGNAL(valueChanged(int)), xy_view, SLOT(changeFocusPlane(int)));
	//printf("connect status[%d]\n",a);

    connect(xValueSpinBox, SIGNAL(valueChanged(int)), xSlider, SLOT(setValue(int)));
    connect(xSlider, SIGNAL(valueChanged(int)), xValueSpinBox, SLOT(setValue(int)));

    connect(yValueSpinBox, SIGNAL(valueChanged(int)), ySlider, SLOT(setValue(int)));
    connect(ySlider, SIGNAL(valueChanged(int)), yValueSpinBox, SLOT(setValue(int)));

    connect(zValueSpinBox, SIGNAL(valueChanged(int)), zSlider, SLOT(setValue(int)));
    connect(zSlider, SIGNAL(valueChanged(int)), zValueSpinBox, SLOT(setValue(int)));

	//set the navigation event connection

    connect(xy_view, SIGNAL(focusXChanged(int)), xSlider, SLOT(setValue(int)));
    connect(xy_view, SIGNAL(focusYChanged(int)), ySlider, SLOT(setValue(int)));
    connect(xy_view, SIGNAL(focusZChanged(int)), zSlider, SLOT(setValue(int)));

    connect(yz_view, SIGNAL(focusXChanged(int)), xSlider, SLOT(setValue(int)));
    connect(yz_view, SIGNAL(focusYChanged(int)), ySlider, SLOT(setValue(int)));
    connect(yz_view, SIGNAL(focusZChanged(int)), zSlider, SLOT(setValue(int)));

    connect(zx_view, SIGNAL(focusXChanged(int)), xSlider, SLOT(setValue(int)));
    connect(zx_view, SIGNAL(focusYChanged(int)), ySlider, SLOT(setValue(int)));
    connect(zx_view, SIGNAL(focusZChanged(int)), zSlider, SLOT(setValue(int)));

    //set up link to respond to external change of focus

    connect(this, SIGNAL(external_focusXChanged(int)), xValueSpinBox, SLOT(setValue(int)));
    connect(this, SIGNAL(external_focusYChanged(int)), yValueSpinBox, SLOT(setValue(int)));
    connect(this, SIGNAL(external_focusZChanged(int)), zValueSpinBox, SLOT(setValue(int)));

	//set up slot to accept signals passed to external
    connect(xValueSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeFocusXToExternal(int)));
    connect(yValueSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeFocusYToExternal(int)));
    connect(zValueSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeFocusZToExternal(int)));
	//printf("out connect status[%d]\n",a);


    // set up focus link events
    connect(linkFocusCheckBox, SIGNAL(clicked()), this, SLOT(toggleLinkFocusCheckBox()));
    connect(displayFocusCrossCheckBox, SIGNAL(clicked()), this, SLOT(toggleDisplayFocusCrossCheckBox()));

	// external communication

	connect(cBox_bSendSignalToExternal, SIGNAL(clicked()), this, SLOT(toggleCheckBox_bSendSignalToExternal()));
	connect(cBox_bAcceptSignalFromExternal, SIGNAL(clicked()), this, SLOT(toggleCheckBox_bAcceptSignalFromExternal()));

    // set up zoom events

    connect(xScaleSlider, SIGNAL(valueChanged(int)), yz_view, SLOT(changeScale(int)));
    connect(yScaleSlider, SIGNAL(valueChanged(int)), zx_view, SLOT(changeScale(int)));
    connect(zScaleSlider, SIGNAL(valueChanged(int)), xy_view, SLOT(changeScale(int)));

    connect(xy_view, SIGNAL(scaleChanged(int)), zScaleSlider, SLOT(setValue(int)));
    connect(yz_view, SIGNAL(scaleChanged(int)), xScaleSlider, SLOT(setValue(int)));
    connect(zx_view, SIGNAL(scaleChanged(int)), yScaleSlider, SLOT(setValue(int)));

    connect(lookingGlassCheckBox, SIGNAL(clicked()), this, SLOT(toggleLookingGlassCheckBox()));

	connect(zoomWholeViewButton, SIGNAL(clicked()), this, SLOT(doMenuOfTriviewZoom()));


    // set up color mapping events
	connectColorGUI(); //110721 RZC


    connect(landmarkCopyButton, SIGNAL(clicked()), this, SLOT(copyLandmarkToPublicBuffer()));
    connect(landmarkPasteButton, SIGNAL(clicked()), this, SLOT(pasteLandmarkFromPublicBuffer()));
    connect(landmarkLoadButton, SIGNAL(clicked()), this, SLOT(loadLandmarkFromFile()));
    connect(landmarkSaveButton, SIGNAL(clicked()), this, SLOT(saveLandmarkToFile()));
    connect(landmarkManagerButton, SIGNAL(clicked()), this, SLOT(openLandmarkManager()));

    connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));
	//connect(openFileNameButton, SIGNAL(clicked()), this, SLOT(setOpenFileName())); //	remove this button on 080402

    //connect(landmarkLabelDispCheckBox, SIGNAL(clicked()), this, SLOT(toggleLandmarkLabelDisp()));

    //connect(whatsThisButton, SIGNAL(clicked(bool)), xy_view, SLOT(setDescriptionEnabled(bool)));
    //connect(imgProcessButton, SIGNAL(clicked()), this, SLOT(popupImageProcessingDialog()));
    //connect(imgV3DButton, SIGNAL(clicked()), this, SLOT(doImage3DView()));
    connect(imgV3DButton, SIGNAL(clicked()), this, SLOT(doMenuOf3DViewer()));
    //connect(imgV3DROIButton, SIGNAL(clicked()), this, SLOT(doImage3DLocalRoiView()));
    connect(whatsThisButton, SIGNAL(clicked()), this, SLOT(aboutInfo()));
	//    connect(xy_view, SIGNAL(descriptionEnabledChanged(bool)), xy_view->hoverPoints(), SLOT(setDisabled(bool)));
	//    connect(xy_view, SIGNAL(descriptionEnabledChanged(bool)), whatsThisButton, SLOT(setChecked(bool)));

	connect(this, SIGNAL(external_validZSliceChanged(long)), this, SLOT(updateTriview()), Qt::AutoConnection); //, Qt::DirectConnection);

}

void XFormWidget::disconnectEventSignals()
{
	disconnect(xSlider, 0, yz_view, 0);
    disconnect(ySlider, 0, zx_view, 0);
    disconnect(zSlider, 0, xy_view, 0);

    disconnect(xValueSpinBox, 0, xSlider, 0);
    disconnect(xSlider, 0, xValueSpinBox, 0);

    disconnect(yValueSpinBox, 0, ySlider, 0);
    disconnect(ySlider, 0, yValueSpinBox, 0);

    disconnect(zValueSpinBox, 0, zSlider, 0);
    disconnect(zSlider, 0, zValueSpinBox, 0);

    disconnect(linkFocusCheckBox, 0, this, 0);
    disconnect(displayFocusCrossCheckBox, 0, this, 0);

    disconnect(this, 0, xValueSpinBox, 0);
    disconnect(this, 0, yValueSpinBox, 0);
    disconnect(this, 0, zValueSpinBox, 0);

    disconnect(xy_view, 0, zSlider, 0);
    disconnect(yz_view, 0, xSlider, 0);
    disconnect(zx_view, 0, ySlider, 0);

    disconnect(xy_view, 0, zScaleSlider, 0);
    disconnect(yz_view, 0, xScaleSlider, 0);
    disconnect(zx_view, 0, yScaleSlider, 0);

    disconnect(lookingGlassCheckBox, 0, this, 0);

    disconnectColorGUI(); //110721 RZC

	disconnect(landmarkCopyButton, 0, this, 0);
	disconnect(landmarkPasteButton, 0, this, 0);
	disconnect(landmarkLoadButton, 0, this, 0);
	disconnect(landmarkSaveButton, 0, this, 0);
	disconnect(landmarkManagerButton, 0, this, 0);

    //disconnect(landmarkLabelDispCheckBox, 0, this, 0);

    disconnect(resetButton, 0, this, 0);
	//disconnect(openFileNameButton, 0, this, 0);

    //disconnect(imgProcessButton, 0, this, 0);
    disconnect(imgV3DButton, 0, this, 0);
	// disconnect(imgV3DROIButton, 0, this, 0);
    disconnect(whatsThisButton, 0, this, 0);
	//    disconnect(whatsThisButton, 0, xy_view, 0);
	//    disconnect(xy_view, 0, xy_view->hoverPoints(), 0);
	//    disconnect(xy_view, 0, whatsThisButton, 0);
}


void XFormWidget::toggleLinkFocusCheckBox()
{
    bLinkFocusViews = (linkFocusCheckBox->checkState()==Qt::Checked) ? true : false;
	if (imgData!=NULL)
	{
		if (imgData->isEmpty()==false)
		{
			imgData->setFlagLinkFocusViews(bLinkFocusViews);
			//printf("display cross %d\n", int(imgData->getFlagLinkFocusViews()));
		}
	}
    update();
}

void XFormWidget::toggleDisplayFocusCrossCheckBox()
{
    bDisplayFocusCross = (displayFocusCrossCheckBox->checkState()==Qt::Checked) ? true : false;
	if (imgData!=NULL)
	{
		if (imgData->isEmpty()==false)
		{
			imgData->setFlagDisplayFocusCross(bDisplayFocusCross);
			//printf("display cross %d\n", int(imgData->getFlagDisplayFocusCross()));
		}
	}
    update();
}

void XFormWidget::toggleImgValScaleDisplay()
{
	if (imgData!=NULL)
	{
		if (imgData->isEmpty()==false)
		{
			//imgData->setFlagImgValScaleDisplay((imgValScaleDisplayCheckBox->checkState()==Qt::Checked) ? true : false); //disable 100814
			//printf("display cross %d\n", int(imgData->getFlagDisplayFocusCross()));
		}
	}

    //use color change to force the update of 3 views

	xy_view->changeColorType(Ctype);
	yz_view->changeColorType(Ctype);
	zx_view->changeColorType(Ctype);
    update();
}

#define __open_looking_glass__
void XFormWidget::toggleLookingGlassCheckBox()
{
	if (imgData!=NULL)
	{
		if (imgData->isEmpty()==false)
		{
			if (lookingGlassCheckBox->checkState()==Qt::Checked)
			{
				xScaleSlider->setValue(4);
				xScaleSlider->setEnabled(false);

				yScaleSlider->setValue(4);
				yScaleSlider->setEnabled(false);

				zScaleSlider->setValue(4);
				zScaleSlider->setEnabled(false);

#if USE_CHANNEL_TABLE
				if (channelTabGlass==NULL)
				{
					if (channelTabGlass = new ChannelTabWidget(this, 2, true)) //2 tabs for Looking glass
					{
						connect(this, SIGNAL(colorChangedGlass(int)), channelTabGlass, SLOT(updateXFormWidget(int)));

						channelTabGlass->setFixedWidth(270); //270 is same width as channelTabXView
						channelTabGlass->setFixedHeight(200); //200 is best for 4 rows
						channelTabGlass->setWindowFlags( Qt::Widget
								| Qt::Tool
								| Qt::CustomizeWindowHint | Qt::WindowTitleHint  //only title bar, disable buttons on title bar
								//| Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint  //only close buttons on title bar
								);
						channelTabGlass->setWindowTitle("For Looking Glass");

						emit colorChangedGlass(-1); //110804
					}
					channelTabGlass->move(QCursor::pos() + QPoint(10,10));
				}
				channelTabGlass->show();
#endif
			}
			else
			{
				xScaleSlider->setEnabled(true);
				yScaleSlider->setEnabled(true);
				zScaleSlider->setEnabled(true);
				//qDebug("hide channelTabGlass  in XFormWidget::toggleLookingGlassCheckBox()");
				if (channelTabGlass)  channelTabGlass->hide();
			}

			imgData->setFlagLookingGlass((lookingGlassCheckBox->checkState()==Qt::Checked) ? true : false);

			//		xy_view->changeScale(4);
			//		yz_view->changeScale(4);
			//		zx_view->changeScale(4);
			//printf("display cross %d\n", int(imgData->getFlagDisplayFocusCross()));
		}
	}
    update();
}

void XFormWidget::toggleCheckBox_bSendSignalToExternal()
{
	bSendSignalToExternal = (cBox_bSendSignalToExternal->checkState()==Qt::Checked) ? true : false;

	//the follwoing simple logic prevent the dead-loop of messages passwd among the window. My approach to only allow one-directional signal channel
	//is not the best, but probably is very easy to use and understand. Logically the best one should just detect and eleiminate any message-cycle
	// in the windows, but it probably difficult to use in practise. Of course it may also cause unnecessary complexity in the program.

	if (bSendSignalToExternal==true)
	{
		bAcceptSignalFromExternal = false;
		cBox_bAcceptSignalFromExternal->setChecked(bAcceptSignalFromExternal);
	}
}

void XFormWidget::toggleCheckBox_bAcceptSignalFromExternal()
{
	bAcceptSignalFromExternal = (cBox_bAcceptSignalFromExternal->checkState()==Qt::Checked) ? true : false;

	//the follwoing simple logic prevent the dead-loop of messages passwd among the window. My approach to only allow one-directional signal channel
	//is not the best, but probably is very easy to use and understand. Logically the best one should just detect and eliminate any message-cycle
	// in the windows, but it probably difficult to use in practise. Of course it may also cause unnecessary complexity in the program.

	if (bAcceptSignalFromExternal==true)
	{
		bSendSignalToExternal = false;
		cBox_bSendSignalToExternal->setChecked(bSendSignalToExternal);
	}
}

void XFormWidget::setColorRedType()
{
    Ctype = colorRedOnly;
	xy_view->changeColorType(Ctype);
	yz_view->changeColorType(Ctype);
	zx_view->changeColorType(Ctype);
    update();
}

void XFormWidget::setColorRed2GrayType()
{
    Ctype = colorRed2Gray;
	xy_view->changeColorType(Ctype);
	yz_view->changeColorType(Ctype);
	zx_view->changeColorType(Ctype);
    update();
}

void XFormWidget::setColorGreenType()
{
    if (!imgData)
		return;

    if (imgData->getCDim()<2)
		return;

    Ctype = colorGreenOnly;
  	xy_view->changeColorType(Ctype);
	yz_view->changeColorType(Ctype);
	zx_view->changeColorType(Ctype);
	update();
}

void XFormWidget::setColorGreen2GrayType()
{
    if (!imgData)
		return;

    if (imgData->getCDim()<2)
		return;

    Ctype = colorGreen2Gray;
	xy_view->changeColorType(Ctype);
	yz_view->changeColorType(Ctype);
	zx_view->changeColorType(Ctype);
    update();
}

void XFormWidget::setColorBlueType()
{
    if (!imgData)
		return;

    if (imgData->getCDim()<3)
		return;

    Ctype = colorBlueOnly;
	xy_view->changeColorType(Ctype);
	yz_view->changeColorType(Ctype);
	zx_view->changeColorType(Ctype);
    update();
}

void XFormWidget::setColorBlue2GrayType()
{
    if (!imgData)
		return;

    if (imgData->getCDim()<3)
		return;

    Ctype = colorBlue2Gray;
	xy_view->changeColorType(Ctype);
	yz_view->changeColorType(Ctype);
	zx_view->changeColorType(Ctype);
    update();
}

void XFormWidget::setColorAllType()
{
    if (!imgData)
		return;

    int cdim = imgData->getCDim();
    if (cdim>=3)
		Ctype = colorRGB;
	else if (cdim==2)
		Ctype = colorRG;
	else
		Ctype = colorGray;

	xy_view->changeColorType(Ctype);
	yz_view->changeColorType(Ctype);
	zx_view->changeColorType(Ctype);
    update();
}

void XFormWidget::setColorAll2GrayType()
{
    Ctype = colorGray;
	xy_view->changeColorType(Ctype);
	yz_view->changeColorType(Ctype);
	zx_view->changeColorType(Ctype);
    update();
}

void XFormWidget::syncChannelTabWidgets(ChannelTabWidget* sender) //110803 RZC
{
	if (channelTabXView && channelTabXView != sender && sender)
	{
		channelTabXView->syncSharedData(sender->getChannelSharedData());
	}
//	if (channelTabGlass && channelTabGlass != sender && sender)
//	{
//		channelTabGlass->syncSharedData(sender->getChannelSharedData());
//	}
	if (atlasViewerDlg && atlasViewerDlg->getChannelTabWidget() && (atlasViewerDlg->getChannelTabWidget() != sender) && sender)
	{
		atlasViewerDlg->getChannelTabWidget()->syncSharedData(sender->getChannelSharedData());
	}
}

#define __update_for_index_mode__

void XFormWidget::setColorMapDispType(ImageDisplayColorType ctype, bool bGlass) //default Ctype=colorPseudoMaskColor, bGlass=false
{
	//110725 RZC, for switching colorPseudoMaskColor to colorUnknown by ChannelTabWidget::updateXFormWidget
	if (! bGlass)
		this->Ctype = ctype;
	else
		this->Ctype_glass = ctype;

    //Ctype = colorPseudoMaskColor; //set as default of Ctype
	xy_view->changeColorType(ctype, bGlass);
	yz_view->changeColorType(ctype, bGlass);
	zx_view->changeColorType(ctype, bGlass);
    update();
}

void XFormWidget::switchMaskColormap() //080824
{
	if (!imgData)
		return;

	if (colorMapDispType && colorMapDispType->isChecked()) //switch color will only be valid is the colorMapDispType is on
	{

		int clen;  ImageDisplayColorType cc;

		imgData->getColorMapInfo(clen, cc);

		if (cc==colorPseudoMaskColor) cc=colorHanchuanFlyBrainColor;
		else if (cc==colorHanchuanFlyBrainColor) cc=colorArnimFlyBrainColor;
		else if (cc==colorArnimFlyBrainColor) cc=colorPseudoMaskColor;
		else cc=colorPseudoMaskColor;

		imgData->switchColorMap(clen, cc);

		//update
//		xy_view->changeColorType(cc);
//		yz_view->changeColorType(cc);
//		zx_view->changeColorType(cc);
		//qDebug("Ctype=%d Ctype_glass=%d colormap->Ctype=%d", Ctype, Ctype_glass, cc);
		if (isIndexColor(Ctype))  		setColorMapDispType(Ctype, false); //110804 RZC, copyRaw2QPixmap_colormap only use imgData->ColorMap->Ctype
#if USE_CHANNEL_TABLE
		if (isIndexColor(Ctype_glass))	setColorMapDispType(Ctype_glass, true);
#endif

		update();
	}
}



void XFormWidget::reset()
{
    if (imgData)
	{
		if (imgData->getCDim()>=2)
		{
			Ctype = colorRGB;
			setColorAllType();
		}
		else //==1
		{
			Ctype = colorRed2Gray;
			setColorAll2GrayType();
		}
	}

	if (xy_view) xy_view->reset();
	if (yz_view) yz_view->reset();
	if (zx_view) zx_view->reset();
}



void XFormWidget::changeFocusXToExternal(int c)
{
	changeFocusToExternal(c, -1, -1); // -1 means the parameter won't be used
}

void XFormWidget::changeFocusYToExternal(int c)
{
	changeFocusToExternal(-1, c, -1); // -1 means the parameter won't be used
}

void XFormWidget::changeFocusZToExternal(int c)
{
	changeFocusToExternal(-1, -1, c); // -1 means the parameter won't be used
}

void XFormWidget::changeFocusToExternal(int newx, int newy, int newz) // this is the function to call other image-view's changeFocusFromExternal() function
{
	if (bSendSignalToExternal==false || !p_mainWindow)
		return;

	int listlen;
	XFormWidget **list = p_mainWindow->retrieveAllMdiChild(listlen);
	//printf("Found %d children windows. \n", listlen);

	if (listlen>1 && list) //otherwise the only possible window is itself
	{
		//if there is invalid flag like -1, then replace it using valid focus info
		if (newx<=0) newx = imgData->curFocusX+1; //correction on 03/31/2006 so that the focus planes of master/slave images are the same
		if (newy<=0) newy = imgData->curFocusY+1;
		if (newz<=0) newz = imgData->curFocusZ+1;

		//broadcast signal

		for (int i=0;i<listlen;i++)
		{
			if (list[i]==this) //of course no need to affect this view itself
				continue;
			else
			{
				list[i]->changeFocusFromExternal(newx, newy, newz);
				//printf("\t%d window set new xyz %d %d %d\n", V3DLONG(list[i]), newx, newy, newz);
			}
		}
	}

	if (list) {delete []list; list=0;} //because list is a newly created pointer, should delete after used it
	//probably a better logic is to store this list somewhere temporarily, -- but need to manage the case that
	// some windows may be closed.
	return;
}

void XFormWidget::changeFocusFromExternal(int x, int y, int z) //this should be called from external. When no cross-image communication is needed, should not use this.
{
	if (bAcceptSignalFromExternal==true)
	{
		if (x>0) //as I used -1 as invalid signal, then here I only allow valid signal to be passed
			emit external_focusXChanged(x);

		if (y>0)
			emit external_focusYChanged(y);

		if (z>0)
			emit external_focusZChanged(z);
	}
	//update();
}

void XFormWidget::forceToChangeFocus(int x, int y, int z) //this is called by other controller
{
	{
		if (x>0) //as I used -1 as invalid signal, then here I only allow valid signal to be passed
			emit external_focusXChanged(x);

		if (y>0)
			emit external_focusYChanged(y);

		if (z>0)
			emit external_focusZChanged(z);
	}
	//update();
}


My4DImage * XFormWidget::selectSubjectImage()
{
	if (!p_mainWindow)
		return 0;

	int listlen;
	XFormWidget **list = p_mainWindow->retrieveAllMdiChild(listlen);
	if (listlen<=1)
		return 0;

	int i,k; int * indTable = new int [listlen];
	if (!indTable)
	{
		printf("Fail to allocate memory for index table in selectSubjectImage();\n");
		return 0;
	}

	QStringList items;
	if (listlen>=2 && list) //otherwise the only possible window is itself
	{
		for (i=0, k=0;i<listlen;i++)
		{
			if (list[i]==this)
			{
				continue;
			}
			else
			{
				items << tr(list[i]->getImageData()->getFileName());
				indTable[k++] = i;
			}
		}
	}

	QString item;
	bool ok;
	item = QInputDialog::getItem(this, tr("Subject image list"),
								 tr("Please select one image as the *subject* image"), items, 0, false, &ok);

	int iSelected=-1;
	for (k=0;k<listlen;k++)
	{
		if (item==items.at(k))
		{
			iSelected=indTable[k];
			break;
		}
	}

	if (iSelected<0) return 0;

	My4DImage * pSelected = list[iSelected]->getImageData();
	if (indTable) {delete []indTable; indTable=0;}
	if (list) {delete []list; list=0;}
	return pSelected;
}

My4DImage * XFormWidget::selectImage()
{
	if (!p_mainWindow)
		return 0;

	int listlen;
	XFormWidget **list = p_mainWindow->retrieveAllMdiChild(listlen);
	if (listlen<=1)
		return 0;

	int i,k; int * indTable = new int [listlen];
	if (!indTable)
	{
		printf("Fail to allocate memory for index table in selectSubjectImage();\n");
		return 0;
	}

	QStringList items;
	if (listlen>=1 && list) //otherwise the only possible window is itself
	{
		for (i=0, k=0;i<listlen;i++)
		{
			items << tr(list[i]->getImageData()->getFileName());
			indTable[k++] = i;
		}
	}

	QString item;
	bool ok;
	item = QInputDialog::getItem(this, tr("image list"),
								 tr("Please select one image for processing"), items, 0, false, &ok);

	int iSelected=-1;
	for (k=0;k<listlen;k++)
	{
		if (item==items.at(k))
		{
			iSelected=indTable[k];
			break;
		}
	}

	if (iSelected<0) return 0;

	My4DImage * pSelected = list[iSelected]->getImageData();
	if (indTable) {delete []indTable; indTable=0;}
	if (list) {delete []list; list=0;}
	return pSelected;
}

V3DLONG XFormWidget::estimateRoughAmountUsedMemory()
{
	if (!p_mainWindow)
		return 0;

	int listlen;
	XFormWidget **list = p_mainWindow->retrieveAllMdiChild(listlen);
	if (listlen<=1)
		return 0;

	int i;

	V3DLONG nbytes=0;
	if (listlen>=1 && list) //otherwise the only possible window is itself
	{
		for (i=0;i<listlen;i++)
		{
			nbytes += list[i]->getImageData()->getTotalBytes();
		}
	}

	printf("=== You have used [%ld] bytes. If you are using 32-bit system, you may only use maximum 2G bytes. === \n", nbytes);

	if (list) {delete []list; list=0;}
	return nbytes;
}


QList <BlendingImageInfo> XFormWidget::selectBlendingImages()
{
	QList <BlendingImageInfo> bList;
	BlendingImageInfo curInfo;
	My4DImage * tmp_pimg;

	if (!p_mainWindow)
		return bList;

	//get the image list
	int listlen;
	XFormWidget **list = p_mainWindow->retrieveAllMdiChild(listlen);
	if (listlen<=0) return bList; //allow blending even there is only one image, because this blending func can be used to change its coloring scheme

	int i,k; int * indTable = new int [listlen];
	if (!indTable)
	{
		printf("Fail to allocate memory for index table in selectBlendingImages();\n");
		return bList;
	}

	QStringList items;
	for (i=0, k=0;i<listlen;i++)
	{
		items << tr(list[i]->getImageData()->getFileName());
		indTable[k++] = i;
	}

	int BLEND_MAXVAL=511;
	int nBlend=0; V3DLONG sz0_first, sz1_first, sz2_first;
	do
	{
		//first select an image
		QString item;
		bool ok;
		item = QInputDialog::getItem(this, tr("image list"), tr("Please select one image to blend"), items, 0, false, &ok);

		int iSelected=-1;
		for (k=0;k<listlen;k++)
		{
			if (item==items.at(k))
			{
				iSelected=indTable[k];
				break;
			}
		}
		if (iSelected<0)
		{
			printf("Should never see this in selectBlendingImages(). Check codes.\n");
			break;
		}

		tmp_pimg = list[iSelected]->getImageData();
		if (nBlend==0)
		{
			sz0_first = tmp_pimg->getXDim(); sz1_first = tmp_pimg->getYDim(); sz2_first = tmp_pimg->getZDim();
			curInfo.pimg = tmp_pimg;

			//then select the channel of this image

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
			curInfo.channo = QInputDialog::getInt(this, tr("channel"), tr("Please select one channel of the last image to blend"), 2, 1, curInfo.pimg->getCDim(), 1, &ok) - 1;
#else
			curInfo.channo = QInputDialog::getInteger(this, tr("channel"), tr("Please select one channel of the last image to blend"), 2, 1, curInfo.pimg->getCDim(), 1, &ok) - 1;
#endif

			//then select RGB info
			int rgbVal;

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
			rgbVal = QInputDialog::getInt(this, tr("Red component"), tr("Red component"), 0, 0, BLEND_MAXVAL, 50, &ok); //set to 511 instead of 255 so that intensity can be increased as well
			curInfo.rr = rgbVal/255.0;
			rgbVal = QInputDialog::getInt(this, tr("Green component"), tr("Green component"), 0, 0, BLEND_MAXVAL, 50, &ok);
			curInfo.gg = rgbVal/255.0;
			rgbVal = QInputDialog::getInt(this, tr("Blue component"), tr("Blue component"), 0, 0, BLEND_MAXVAL, 50, &ok);
			curInfo.bb = rgbVal/255.0;
#else
			rgbVal = QInputDialog::getInteger(this, tr("Red component"), tr("Red component"), 0, 0, BLEND_MAXVAL, 50, &ok); //set to 511 instead of 255 so that intensity can be increased as well
			curInfo.rr = rgbVal/255.0;
			rgbVal = QInputDialog::getInteger(this, tr("Green component"), tr("Green component"), 0, 0, BLEND_MAXVAL, 50, &ok);
			curInfo.gg = rgbVal/255.0;
			rgbVal = QInputDialog::getInteger(this, tr("Blue component"), tr("Blue component"), 0, 0, BLEND_MAXVAL, 50, &ok);
			curInfo.bb = rgbVal/255.0;
#endif

			//add the selected info to the bList
			bList.append(curInfo);

			//check if want to select more images
			if(QMessageBox::No == QMessageBox::question (0, "Continue?", "Continue select another image/channel for blending?", QMessageBox::Yes, QMessageBox::No))
				break;
		}
		else
		{
			if (sz0_first == tmp_pimg->getXDim() && sz1_first == tmp_pimg->getYDim() && sz2_first == tmp_pimg->getZDim())
			{
				curInfo.pimg = tmp_pimg;

				//then select the channel of this image

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
				curInfo.channo = QInputDialog::getInt(this, tr("channel"), tr("Please select one channel of the last image to blend"), 2, 1, curInfo.pimg->getCDim(), 1, &ok) - 1;
#else
				curInfo.channo = QInputDialog::getInteger(this, tr("channel"), tr("Please select one channel of the last image to blend"), 2, 1, curInfo.pimg->getCDim(), 1, &ok) - 1;
#endif

				//then select RGB info
				int rgbVal;

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
				rgbVal = QInputDialog::getInt(this, tr("Red component"), tr("Red component"), 0, 0, BLEND_MAXVAL, 50, &ok);
				curInfo.rr = rgbVal/255.0;
				rgbVal = QInputDialog::getInt(this, tr("Green component"), tr("Green component"), 0, 0, BLEND_MAXVAL, 50, &ok);
				curInfo.gg = rgbVal/255.0;
				rgbVal = QInputDialog::getInt(this, tr("Blue component"), tr("Blue component"), 0, 0, BLEND_MAXVAL, 50, &ok);
				curInfo.bb = rgbVal/255.0;
#else
				rgbVal = QInputDialog::getInteger(this, tr("Red component"), tr("Red component"), 0, 0, BLEND_MAXVAL, 50, &ok);
				curInfo.rr = rgbVal/255.0;
				rgbVal = QInputDialog::getInteger(this, tr("Green component"), tr("Green component"), 0, 0, BLEND_MAXVAL, 50, &ok);
				curInfo.gg = rgbVal/255.0;
				rgbVal = QInputDialog::getInteger(this, tr("Blue component"), tr("Blue component"), 0, 0, BLEND_MAXVAL, 50, &ok);
				curInfo.bb = rgbVal/255.0;
#endif

				//add the selected info to the bList
				bList.append(curInfo);

				//check if want to select more images
				if(QMessageBox::No == QMessageBox::question (0, "Continue?", "Continue select another image/channel for blending?", QMessageBox::Yes, QMessageBox::No))
					break;
			}
			else
			{
				if(QMessageBox::No == QMessageBox::question (0, "Continue?", "You just select an image with different dimensions (x,y,z sizes) with the first one. This one will NOT be used. Continue to select another image?", QMessageBox::Yes, QMessageBox::No))
					break;
			}
		}

		nBlend++;
	} while (1);

	if (indTable) {delete []indTable; indTable=0;}
	if (list) {delete []list; list=0;}

	return bList;
}

void XFormWidget::copyLandmarkToPublicBuffer() //080107
{
	if (imgData && imgData->listLandmarks.count()>0)
	{
		p_mainWindow->buffer_landmark_pts.erase(p_mainWindow->buffer_landmark_pts.begin(), p_mainWindow->buffer_landmark_pts.end());
		for (int i=0;i<imgData->listLandmarks.count(); i++)
		{
			p_mainWindow->buffer_landmark_pts.append(imgData->listLandmarks.at(i));
		}
	}
}

void XFormWidget::pasteLandmarkFromPublicBuffer() //080107
{
	if (imgData && p_mainWindow->buffer_landmark_pts.count()>0)
	{
		//imgData->listLandmarks.erase(imgData->listLandmarks.begin(), imgData->listLandmarks.end());
		imgData->listLandmarks.clear();
		for (int i=0;i<p_mainWindow->buffer_landmark_pts.count(); i++)
		{
			imgData->listLandmarks.append(p_mainWindow->buffer_landmark_pts.at(i));
		}

		//update the related views
		imgData->updateViews();
	}
}

void XFormWidget::saveLandmarkToFile() //080107, 080111
{
	if (!imgData || imgData->listLandmarks.count()<=0)
	{
		v3d_msg("You don't have any landmark defined yet. Do nothing.");
		return;
	}

	imgData->saveLandmarkToFile();
}

void XFormWidget::loadLandmarkFromFile() //080107
{
	if (!imgData)
	{
		v3d_msg("You don't have the image data ready, - thus unable to import landmark. Do nothing.");
		return;
	}

	imgData->loadLandmarkFromFile();

	//update the related views
	imgData->updateViews();
}

void XFormWidget::openLandmarkManager()
{
	if (!imgData)
	{
		v3d_msg("You don't have the image data yet. Do nothing.");
		return;
	}

	launchAtlasViewer(1);

	//update the related views
	imgData->updateViews();
}


void XFormWidget::launchAtlasViewer(int curTab)
{
	if (atlasViewerDlg)
	{
		atlasViewerDlg->reCreateTables(this);
		atlasViewerDlg->show();
	}
	else
	{
		atlasViewerDlg = new V3D_atlas_viewerDialog(this);
		atlasViewerDlg->show();
	}
	atlasViewerDlg->setCurTab(curTab); // 090504 RZC
}

void XFormWidget::createMenuOfTriviewZoom()
{
    QAction* Act;

    Act = new QAction(tr("Zoom &In"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(triview_zoomin()));
    menuTriviewZoom.addAction(Act);

    Act = new QAction(tr("Zoom &Out"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(triview_zoomout()));
    menuTriviewZoom.addAction(Act);

    Act = new QAction(tr("x&1"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(triview_zoom1()));
    menuTriviewZoom.addAction(Act);

    Act = new QAction(tr("x&2"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(triview_zoom2()));
    menuTriviewZoom.addAction(Act);

    Act = new QAction(tr("x&0.5"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(triview_zoomhalf()));
    menuTriviewZoom.addAction(Act);
}

void XFormWidget::doMenuOfTriviewZoom()
{
	try
	{
		menuTriviewZoom.exec(QCursor::pos());
	}
	catch (...)
	{
		v3d_msg("Fail to run the XFormWidget::doMenuOfTriviewZoom() function.\n", 0);
	}
}

void XFormWidget::triview_zoomin()
{
	triview_setzoom(2, true);
}
void XFormWidget::triview_zoomout()
{
	triview_setzoom(0.5, true);
}
void XFormWidget::triview_zoom1()
{
	triview_setzoom(1, false);
}
void XFormWidget::triview_zoom2()
{
	triview_setzoom(2, false);
}
void XFormWidget::triview_zoomhalf()
{
	triview_setzoom(0.5, false);
}

void XFormWidget::triview_setzoom(double z, bool b_multiply) //b_multiply determine is z is the target zoom, or the target zoom should be product of the current zoom multply z
{
	if (b_multiply)
		disp_zoom *= z;
	else
		disp_zoom = z;

	b_use_dispzoom = (fabs(disp_zoom-1)>0.01) ? true : false;
	updateDataRelatedGUI();

	//110805: by PHC. Now this should not be needed any more
	//QTimer::singleShot(200, this, SLOT(cascadeWindows())); //this is very important to ensure the events propogate through. 2010-01-29
}

void XFormWidget::cascadeWindows()
{
	getMainControlWindow()->cascadeWindows();
}

void XFormWidget::createMenuOf3DViewer()
{
    QAction* Act;

    Act = new QAction(tr("Entire image"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(doImage3DView()));
    menu3DViewer.addAction(Act);

    Act = new QAction(tr("Region of Interest"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(doImage3DLocalRoiView()));
    menu3DViewer.addAction(Act);
}

void XFormWidget::doMenuOf3DViewer()
{
	if (!imgData || !imgData->valid())
	{
		v3d_msg("Invalid 3D image data in doMenuOf3DViewer().");
		return;
	}

//	if (imgData->getDatatype() != V3D_UINT8)
//	{
//		v3d_msg("Your data type is not UINT8 yet, - you will need to convert to UINT8 to see the data in 3D. Go to main menu \"Image/Data\" -> \"Image type\" to convert.");
//		return;
//	}

	try
	{
		menu3DViewer.exec(QCursor::pos());
	}
	catch (...)
	{
		v3d_msg("Fail to run the XFormWidget::doMenuOf3DViewer() function.\n", 0);
	}
}

QList <LocationSimple> My4DImage::autoMarkerFromImg(V3DLONG chno, BoundingBox bbox, float zthickness)
{
	QList <LocationSimple> ql;
	if (!valid() || this->getDatatype() != V3D_UINT8 || chno < 0 || chno >= getCDim() )
	{
		v3d_msg("Now only support UINT8 type of data. Your data is not this type, or your channel info is not correct. Do nothing.\n");
		return ql;
	}

	//prepare input and output data

	Vol3DSimple <unsigned char> * tmp_inimg = 0;
	Vol3DSimple <USHORTINT16> * tmp_outimg = 0;

	V3DLONG xb,yb,zb,xe,ye,ze;
	if (bbox.x0<bbox.x1) {xb=bbox.x0; xe=bbox.x1;} else {xb=bbox.x1; xe=bbox.x0;}
	if (bbox.y0<bbox.y1) {yb=bbox.y0; ye=bbox.y1;} else {yb=bbox.y1; ye=bbox.y0;}
	if (bbox.z0<bbox.z1) {zb=bbox.z0; ze=bbox.z1;} else {zb=bbox.z1; ze=bbox.z0;}
	xb = qMin(qMax(xb,V3DLONG(0)), getXDim()-1);
	xe = qMin(qMax(xe,V3DLONG(0)), getXDim()-1);
	yb = qMin(qMax(yb,V3DLONG(0)), getYDim()-1);
	ye = qMin(qMax(ye,V3DLONG(0)), getYDim()-1);
	zb = qMin(qMax(zb,V3DLONG(0)), getZDim()-1);
	ze = qMin(qMax(ze,V3DLONG(0)), getZDim()-1);
	V3DLONG vsz0=xe-xb+1, vsz1=ye-yb+1, vsz2=ze-zb+1;

	try
	{
		tmp_inimg = new Vol3DSimple <unsigned char> (vsz0, vsz1, vsz2);
		tmp_outimg = new Vol3DSimple <USHORTINT16> (vsz0, vsz1, vsz2);
	}
	catch (...)
	{
		v3d_msg("Unable to allocate memory for processing. Do nothing.\n");
		if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;}
		if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
		return ql;
	}

	//copy data
	unsigned char *** tmp_inimg3d = tmp_inimg->getData3dHandle();
	unsigned char *** pCur3d = ((unsigned char ****)getData())[chno /*segpara.channo*/]; //does not allow changing the channo #
	{
		for (V3DLONG k=zb;k<=ze;k++)
			for (V3DLONG j=yb;j<=ye;j++)
				for (V3DLONG i=xb;i<=xe;i++)
					tmp_inimg3d[k-zb][j-yb][i-xb] = pCur3d[k][j][i];
	}

	//define the template matching parameters
	para_template_matching_cellseg segpara;
	segpara.channo = chno;
	segpara.szx=13;
	segpara.szy=13;
	segpara.szz=5; //dim
	segpara.stdx=4.0;
	segpara.stdy=4.0;
	segpara.stdz=3.0; //std
	segpara.t_pixval=150;
	segpara.t_rgnval=20;
	segpara.t_corrcoef=0.30; //thresholds
	segpara.merge_radius = 20;

	//now set the two thresholds adaptively
	double mean_val=0, std_val=0;
	data_array_mean_and_std(tmp_inimg->getData1dHandle(), tmp_inimg->getTotalElementNumber(), mean_val, std_val);
	segpara.t_pixval=qMax(double(150.0), mean_val+3.0*std_val);
	segpara.t_rgnval=qMax(double(20.0), mean_val+1.0*std_val);

	//use dialog to select seg parameters
	para_template_matching_cellseg_dialog *p_mydlg=0;
	if (!p_mydlg) p_mydlg = new para_template_matching_cellseg_dialog(getCDim(), &segpara);
	p_mydlg->setEnabledChannelField(false); //does not allow to change the channel no any more

	int res = p_mydlg->exec();
	if (res!=QDialog::Accepted)
	{
		if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;}
		if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
		return ql;
	}
	else
		p_mydlg->fetchData(&segpara);
	if (p_mydlg) {delete p_mydlg; p_mydlg=0;}

	//do computation
	bool b_res = template_matching_seg(tmp_inimg, tmp_outimg, segpara);
	if (!b_res)
	{
		v3d_msg("Fail to do the cell segmentation().\n");
	}
	else
	{
		USHORTINT16 * tmpImg_d1d = (USHORTINT16 *)(tmp_outimg->getData1dHandle());

		//display new images
		LocationSimple * p_ano = 0;
		V3DLONG n_objects = 0;
		if (!compute_statistics_objects(tmp_inimg, tmp_outimg, p_ano, n_objects))
		{
			v3d_msg("Some errors happen during the computation of image objects' statistics. The annotation is not generated.");
			return ql;
		}

		for (V3DLONG i=1;i<n_objects;i++) //do not process 0 values, as it is background. Thus starts from 1
		{
			p_ano[i].x += xb;
			p_ano[i].y += yb;
			p_ano[i].z += zb;
			ql << p_ano[i];
			//			fprintf(f_ano, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
			//					i,i,"","", int(p_ano[i].z+0.5), int(p_ano[i].x+0.5), int(p_ano[i].y+0.5),
			//					p_ano[i].pixmax, p_ano[i].ave, p_ano[i].sdev, p_ano[i].size, p_ano[i].mass);
		}
		//finally save to image and mask and linker file
		if (p_ano) {delete []p_ano; p_ano=0;}
	}

	//free unneeded variables
	if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;}
	if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}

	return ql;
}

QList <LocationSimple> My4DImage::autoMarkerFromImg(V3DLONG chno)
{
	BoundingBox bbox(0, 0, 0, getXDim()-1, getYDim()-1, getZDim()-1);
	float zthickness = 1.0;
	return 	autoMarkerFromImg(chno, bbox, zthickness);
}



//try to solve a strange linking issue for msvc 2008

#if defined (_MSC_VER)

LandmarkPropertyDialog::LandmarkPropertyDialog(const QList <LocationSimple> *p_anoTable, int curRowNum, My4DImage *p_imgdata=0)
{
	if (!p_anoTable)
		return;
	else
		updateContent(p_anoTable, curRowNum, p_imgdata);
}

void LandmarkPropertyDialog::updateContent(const QList <LocationSimple> *p_anoTable, int curRowNum, My4DImage *p_imgdata=0)
{
	if (!p_anoTable)
	{
		printf("anoTable is not valid in LandmarkPropertyDialog::fetchData().\n");
		return;
	}

	if (curRowNum>=p_anoTable->count())
	{
		printf("The index [=%d] is bigger than the size of the list [=%d].\n", curRow, anoTable->count());
		return;
	}


	imgdata = p_imgdata;
	anoTable = (QList <LocationSimple> *)p_anoTable;
	curRow = curRowNum;
	create();

	QString tmp;

	const LocationSimple *p_landmark = &(anoTable->at(curRow));

	//landmark name/comments/annoattions

	if (curRow==0 && anoTable->size()==1)
		order->setText(tmp.setNum(curRow+1).prepend("No.") + " or new landmark"); //curRow
	else
		order->setText(tmp.setNum(curRow+1)); //curRow

	name->setText(p_landmark->name.c_str());
	comment->setText(p_landmark->comments.c_str());

	//landmark geometry

	coord_z->setText(tmp.setNum(int(p_landmark->z)));
	coord_x->setText(tmp.setNum(int(p_landmark->x)));
	coord_y->setText(tmp.setNum(int(p_landmark->y)));
	radius->setText(tmp.setNum(int(p_landmark->radius)));

	if (int(p_landmark->shape)>=landmark_shape->count())
	{
		qDebug("Warning: your landmark shape type is not compatible with the combobox list. Unset it.\n");
		landmark_shape->setCurrentIndex(0); //the first one is call "unset"
	}
	else
		landmark_shape->setCurrentIndex(int(p_landmark->shape));


	//pixel intensity

	if (imgdata)
	{
		int nc = imgdata->getCDim();
		pix_val_red->setText(tmp.setNum(int(imgdata->at(int(p_landmark->x-1), int(p_landmark->y-1), int(p_landmark->z-1), 0))));
		if (nc>=2)
			pix_val_green->setText(tmp.setNum(int(imgdata->at(int(p_landmark->x-1), int(p_landmark->y-1), int(p_landmark->z-1), 1))));
		else
			pix_val_green->setText(tmp.setNum(0));
		if (nc>=3)
			pix_val_blue->setText(tmp.setNum(int(imgdata->at(int(p_landmark->x-1), int(p_landmark->y-1), int(p_landmark->z-1), 2))));
		else
			pix_val_blue->setText(tmp.setNum(0));
		if (nc>=4)
			pix_val_ch4->setText(tmp.setNum(int(imgdata->at(int(p_landmark->x-1), int(p_landmark->y-1), int(p_landmark->z-1), 3))));
		else
			pix_val_ch4->setText(tmp.setNum(0));
		if (nc>=5)
			pix_val_ch5->setText(tmp.setNum(int(imgdata->at(int(p_landmark->x-1), int(p_landmark->y-1), int(p_landmark->z-1), 4))));
		else
			pix_val_ch5->setText(tmp.setNum(0));

		//landmark surrounding area statistics

		statistics_channel->setRange(1, nc);
		//compute the stat of surrounding rgn

		int tmp_vv =int(imgdata->at(int(p_landmark->x-1), int(p_landmark->y-1), int(p_landmark->z-1), 0));

		val_peak->setText(tmp.setNum(tmp_vv));
		val_mean->setText(tmp.setNum(tmp_vv));
		val_stddev->setText(tmp.setNum(0));
		val_size->setText(tmp.setNum(1));
		val_mass->setText(tmp.setNum(tmp_vv));

		//now do computation
		compute_rgn_stat();
	}
	else
	{
		pix_val_red->setText("Unset");
		pix_val_green->setText("Unset");
		pix_val_blue->setText("Unset");
		pix_val_ch4->setText("Unset");
		pix_val_ch5->setText("Unset");

		//landmark surrounding area statistics

		statistics_channel->setRange(0, 0);
		//compute the stat of surrounding rgn
		val_peak->setText("Unset");
		val_mean->setText("Unset");
		val_stddev->setText("Unset");
		val_size->setText("Unset");
		val_mass->setText("Unset");
	}

	//set read/write property

	order->setReadOnly(true);
	coord_z->setReadOnly(true);
	coord_x->setReadOnly(true);
	coord_y->setReadOnly(true);
	pix_val_red->setReadOnly(true);
	pix_val_green->setReadOnly(true);
	pix_val_blue->setReadOnly(true);
	pix_val_ch4->setReadOnly(true);
	pix_val_ch5->setReadOnly(true);
	val_peak->setReadOnly(true);
	val_mean->setReadOnly(true);
	val_stddev->setReadOnly(true);
	val_size->setReadOnly(true);
	val_mass->setReadOnly(true);
}

void LandmarkPropertyDialog::fetchData(QList <LocationSimple>  *anoTable, int curRow)
{
	if (!anoTable)
	{
		printf("anoTable is not valid in LandmarkPropertyDialog::fetchData().\n");
		return;
	}

	if (curRow>=anoTable->count())
	{
		printf("The index [=%d] is bigger than the size of the list [=%d].\n", curRow, anoTable->count());
		return;
	}

	LocationSimple *p_landmark = (LocationSimple *) &(anoTable->at(curRow));

	//landmark name/comments/annoattions

	//order->setText(tmp.setNum(curRow));
	p_landmark->name = qPrintable(name->text());
	p_landmark->comments = qPrintable(comment->text());

	//landmark geometry

	p_landmark->x = coord_x->text().toDouble();
	p_landmark->y = coord_y->text().toDouble();
	p_landmark->z = coord_z->text().toDouble();
	p_landmark->radius = (radius->text().toDouble()>=0)?radius->text().toDouble():0;
	p_landmark->shape = PxLocationMarkerShape(landmark_shape->currentIndex());
}

void LandmarkPropertyDialog::create()
{
	setupUi(this);

	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	//	radius->setInputMask(tr("999")); //only allow three degits, each is 0~9
	connect(radius, SIGNAL(editingFinished()), this, SLOT(compute_rgn_stat()));
	connect(landmark_shape, SIGNAL(currentIndexChanged(int)), this, SLOT(compute_rgn_stat(int)));

	connect(statistics_channel, SIGNAL(valueChanged(int)), this, SLOT(compute_rgn_stat(int)));
}

void LandmarkPropertyDialog::compute_rgn_stat(int c) //overload for convenience
{
	compute_rgn_stat();
}

void LandmarkPropertyDialog::compute_rgn_stat()
{
	if (!imgdata || !imgdata->valid()) return;

	LocationSimple pt;

	pt.x = coord_x->text().toInt()-1;
	pt.y = coord_y->text().toInt()-1;
	pt.z = coord_z->text().toInt()-1;
	V3DLONG cc = statistics_channel->value()-1; if (cc<0) cc=0; if (cc>=imgdata->getCDim()) cc=imgdata->getCDim()-1;
	pt.radius = (radius->text().toDouble()>=0)?radius->text().toDouble():0;
	pt.shape = PxLocationMarkerShape(landmark_shape->currentIndex());

	//now do the computation
	if (imgdata->compute_rgn_stat(pt, cc)==true)
	{
		//now update the value of the respective
		QString tmp;
		val_peak->setText(tmp.setNum(pt.pixmax));
		val_mean->setText(tmp.setNum(pt.ave));
		val_stddev->setText(tmp.setNum(pt.sdev));
		val_size->setText(tmp.setNum(pt.size));
		val_mass->setText(tmp.setNum(pt.mass));

		if (pt.ev_pc1==VAL_INVALID && pt.ev_pc2==VAL_INVALID && pt.ev_pc3==VAL_INVALID)
		{
			val_pc1_d->setText("uncomputed");
			val_pc2_d->setText("uncomputed");
			val_pc3_d->setText("uncomputed");
		}
		else
		{
			val_pc1_d->setText(tmp.setNum(sqrt(pt.ev_pc1)));
			val_pc2_d->setText(tmp.setNum(sqrt(pt.ev_pc2)));
			val_pc3_d->setText(tmp.setNum(sqrt(pt.ev_pc3)));
		}
	}
}


#endif


/////////////////////////////////////////////////////////////
// put here instead of in my4diamge.cpp because to share the same #include with XFormWidget::doImage3DView
#define LOAD_traced_neuron   load_merged_neuron

void load_segment_neuron(My4DImage* curImg, Renderer_gl1* curRen) // 090622 RZC: load segment neuron, used by algorithm test
{
	V_NeuronSWC null_neuron;
	for (int i=0; i<curImg->tracedNeuron.last_seg_num; i++)	{
	null_neuron.name = qPrintable(QString("%1").arg(i+1)); curRen->updateNeuronTree(null_neuron);}
	for (int i=0; i<curImg->tracedNeuron.seg.size(); i++)
		curRen->updateNeuronTree(curImg->tracedNeuron.seg[i]);
}
void load_merged_neuron(My4DImage* curImg, Renderer_gl1* curRen)
{
	V_NeuronSWC merged_neuron = merge_V_NeuronSWC_list(curImg->tracedNeuron);
	merged_neuron.name = curImg->tracedNeuron.name;
	merged_neuron.file = curImg->tracedNeuron.file;
    curRen->updateNeuronTree(merged_neuron);
}

void My4DImage::
update_3drenderer_neuron_view(V3dR_GLWidget* glwidget, Renderer_gl1* renderer)
{
    LOAD_traced_neuron(this, renderer);
    glwidget->updateTool();
}

void My4DImage::update_3drenderer_neuron_view()
{
	XFormWidget* xwidget = getXWidget();
	if (! xwidget) return;

	if (xwidget->mypara_3Dview.b_still_open && xwidget->mypara_3Dview.window3D)
	{
		Renderer_gl1 * cur_renderer = (Renderer_gl1 *)(xwidget->mypara_3Dview.window3D->getGLWidget()->getRenderer());

		LOAD_traced_neuron(this, cur_renderer);

		xwidget->mypara_3Dview.window3D->getGLWidget()->updateTool(); // 090622 RZC
	}

	//also update local view
	if (xwidget->mypara_3Dlocalview.b_still_open && xwidget->mypara_3Dlocalview.window3D)
	{
		Renderer_gl1 * cur_renderer = (Renderer_gl1 *)(xwidget->mypara_3Dlocalview.window3D->getGLWidget()->getRenderer());

		LOAD_traced_neuron(this, cur_renderer);

		xwidget->mypara_3Dlocalview.window3D->getGLWidget()->updateTool();
	}
}

