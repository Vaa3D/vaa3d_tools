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



//last edit: 170606. add by PHC to introduce a favorite tracing direction

#ifndef __V3D_CURVETRACEPARA_H__
#define __V3D_CURVETRACEPARA_H__

#include "v3d_basicdatatype.h"

struct CurveTracePara
{
    double  false_th;
	double image_force_weight;
	double grident_force_weight;
	double internal_force_weight;
	double internal_force2_weight;
	double prior_force_weight;
	V3DLONG nloops;
	int channo; //which data channel the trace will be applied to
	int n_points; //# of control points
	bool b_adaptiveCtrlPoints;
	bool b_estRadii;
	bool b_postMergeClosebyBranches;
	bool b_post_trimming;
	bool b_pruneArtifactBranches;

	bool b_deformcurve; //use shortest path or deformable model
	
	bool b_3dcurve_width_from_xyonly; //added 2010 Nov 30. Note this has not been added to the tracing parameter dialog yet (only for command line GD tracing)

	//shortest path parameters
	int sp_num_end_nodes; //(0 for shortest path tree) (1 for shortest path) (>1 for n pair path)
	int sp_graph_connect;    //(0 for 6-connect) (1 for include diagonal connects)
	int sp_graph_background; //0 - full image, 1 - use mean value to threshold image and use foreground only
	int sp_graph_resolution_step;
	int sp_downsample_step;
	int sp_smoothing_win_sz;
    int sp_downsample_method; // 0 for average, 1 for max //Added by Zhi 20170915

	
	double imgTH; //anything <=imgTH should NOT be traced! added by PHC, 2010-Dec-21 for the cmd line v3dneuron tracing program
    double visible_thresh; //2013-02-10

    bool b_use_favorite_direction;//whether or not use the favorite direction to trace. // by PHC 170606
    double favorite_direction[3]; //170606. add by PHC to introduce a favorite tracing direction

	V3DLONG landmark_id_start, landmark_id_end;

	CurveTracePara()
	{
		channo=0; n_points=8; landmark_id_start=0; landmark_id_end=0;
		nloops=1; //change from 100 to 1 for JHS data. 090824
		image_force_weight=1;
		grident_force_weight=1;
		internal_force_weight=0.1;
		internal_force2_weight=0.1;
		prior_force_weight=0.2;

		b_adaptiveCtrlPoints=true;
		b_deformcurve=false;
		
		b_estRadii = true;
		b_postMergeClosebyBranches=true;
		
		b_post_trimming = false; //20101213. by Hanchuan Peng
        b_pruneArtifactBranches = true; //20120402

		sp_num_end_nodes = 1;
		sp_graph_connect=0;
		sp_graph_background=1;
		sp_graph_resolution_step=2;
		sp_downsample_step=2;
		sp_smoothing_win_sz=7;
        sp_downsample_method=0;

		imgTH = 0;
        visible_thresh = 30;
		
		b_3dcurve_width_from_xyonly = false;

                b_use_favorite_direction = false; //default set to false
                favorite_direction[0] = favorite_direction[1] = favorite_direction[2] = 0;
	}
};


#endif


