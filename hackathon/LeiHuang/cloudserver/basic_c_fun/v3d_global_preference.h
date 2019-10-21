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




//v3d_global_preference.h
//by Hanchuan Peng
// 2010-06-01
// separated from v3d_global_preference_dialog.h, by PHC, 100601.
// 2010-09-04: add b_UseMylibTiff

#ifndef __V3D_GLOBAL_PREFERENCE_H__
#define __V3D_GLOBAL_PREFERENCE_H__

/*!
 * General settings for the V3D program.
 */
struct V3D_GlobalSetting
{
	//triview tab
	bool b_yaxis_up;
	bool b_autoConvert2_8bit;
	int default_rightshift_bits;
	bool b_autoRescale16bitDisplay;
	int default_lookglass_size;
	int default_marker_radius;
	bool b_UseMylibTiff;
    bool b_BlendColor;

	//3D viewer tab
	bool b_scrollupZoomin;
	bool b_autoOpenImg3DViewer;
	bool b_autoDispXYZAxes;
	bool b_autoDispBoundingBox;
	int marker_disp_amplifying_factor;
	bool b_autoSWCLineMode;
	bool b_autoVideoCardCompress;
	bool b_autoVideoCard3DTex;
	bool b_autoVideoCardNPTTex;
	int autoVideoCardStreamMode;

	//image analysis tab
	int GPara_landmarkMatchingMethod; //PointMatchMethodType
	int GPara_df_compute_method;      //DFComputeMethodType
	bool b_3dcurve_autodeform;
	bool b_3dcurve_autowidth;
	bool b_3dcurve_autoconnecttips;
	bool b_3dcurve_inertia;
	bool b_3dcurve_width_from_xyonly;
	
	//default option for image analysis plugins
	int iChannel_for_plugin; //-1 for all channels, and 0, 1,2, ... for valid image color channel for plugin-based image processing
	bool b_plugin_dispResInNewWindow; //if display the plugin-based image processing result in a new window (otherwise re-use in the existing window)
	bool b_plugin_dispParameterDialog; //if display a dialog to ask a user to supply a plugin's parameters (e.g. image processing parameters)
	bool b_plugin_outputImgRescale; //if rescale the output of plugin's processing result between [0, 255]
	bool b_plugin_outputImgConvert2UINT8; //if yes then convert a plugin's output to UINT8 type (rounding to it in most cases); if no, then keep as float (32bit)

	//default preferences
	V3D_GlobalSetting()
	{
		b_yaxis_up = false; //so that y-axis positive direction is from top to bottom
		b_autoConvert2_8bit = false;
		default_rightshift_bits=-1; //-1 means do not use the auto-shift, instead should pop-up a dialog let user select how many bits to shift
		b_autoRescale16bitDisplay = false;
		default_lookglass_size = 7;
		default_marker_radius = 5;
        b_BlendColor = true;

		//3D viewer tab
		b_autoOpenImg3DViewer = false;
		b_autoDispXYZAxes = true;
		b_autoDispBoundingBox = true;
		marker_disp_amplifying_factor = 15;
		b_autoSWCLineMode = false;
		b_autoVideoCardCompress = true;
		b_autoVideoCard3DTex = false;
		b_autoVideoCardNPTTex = false;
		autoVideoCardStreamMode = 0;//1 for adaptive stream mode, 0 for 512x512x256 downsample. for others see dialog info
        b_UseMylibTiff = false;

		//image analysis tab
		GPara_landmarkMatchingMethod = 0; //MATCH_MI; //(PointMatchMethodType)0;
		GPara_df_compute_method = 0; // DF_GEN_TPS_LINEAR_INTERP; //(DFComputeMethodType)0;
		
		b_3dcurve_autodeform=false;
		b_3dcurve_autowidth=false;
		b_3dcurve_autoconnecttips=true;
		b_3dcurve_inertia=true;
		b_3dcurve_width_from_xyonly = false;
		
		//plugin options
		iChannel_for_plugin = -1; //-1 for all channels, and 0, 1,2, ... for valid image color channel for plugin-based image processing
		b_plugin_dispResInNewWindow = true; //if display the plugin-based image processing result in a new window (otherwise re-use in the existing window)
		b_plugin_dispParameterDialog = true; //if display a dialog to ask a user to supply a plugin's parameters (e.g. image processing parameters)
		b_plugin_outputImgRescale = false; //if rescale the output of plugin's processing result between [0, 255]
		b_plugin_outputImgConvert2UINT8 = true; // if yes then convert a plugin's output to UINT8 type (rounding to it in most cases); if no, then keep as float (32bit)
	}
};


#endif

