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




//by Hanchuan Peng
// this defines some functions to computer the similarity scores of neuronal structures
//090306
//090430
//091008

#ifndef __NEURON_SIM_SCORES_H__
#define __NEURON_SIM_SCORES_H__

#include "../basic_c_fun/basic_surf_objs.h"

struct NeuronDistSimple
{
        double dist_12_allnodes;//the average distance of all interpolated points: from neuron 1  to neurno 2
        double dist_21_allnodes;//the average distance of all interpolated points: from neuron 2 to neuron 1
        double dist_allnodes; //the average distance of all interpolated points, the average of both directions
	double dist_apartnodes; //the average distance of interpolated points that have significant distance (>= 2 pixels)
	double percent_apartnodes; //the percentage of interpolated points that have significant distance (>= 2 pixels)
    double percent_12_apartnodes; //the percentage of interpolated points that have significant distance (>= 2 pixels):from neuron 1  to neurno 2
    double percent_21_apartnodes; //the percentage of interpolated points that have significant distance (>= 2 pixels):: from neuron 2 to neuron 1
    double dist_max; //the maximal distance between two neurons, which is defined the smaller one of the two one-directional max distances
    NeuronDistSimple() {dist_12_allnodes = dist_21_allnodes = dist_allnodes = dist_apartnodes = percent_apartnodes = dist_max = -1; }
};

//round all neuronal node coordinates, and compute the average min distance matches for all places the neurons go through
NeuronDistSimple neuron_score_rounding_nearest_neighbor(const NeuronTree *p1, const NeuronTree *p2, bool menu, double d_thres_updated = 2.0);
double dist_directional_swc_1_2(V3DLONG & nseg1, V3DLONG & nseg1big, double & sum1big, const NeuronTree *p1, const NeuronTree *p2, double &maxdist);
double dist_pt_to_swc(const XYZ & pt, const NeuronTree * p2);
double dist_pt_to_line(const XYZ & p0, const XYZ &  p1, const XYZ &  p2); //p1 and p2 define a straight line, and p0 the point
double dist_pt_to_line_seg(const XYZ & p0, const XYZ &  p1, const XYZ &  p2); //p1 and p2 are the two ends of the line segment, and p0 the point

QHash<int, int> generate_neuron_swc_hash(const NeuronTree * p_tree); //generate a hash lookup table from a neuron swc graph


struct NeuronMorphoInfo
{
	bool b_valid;
	double total_length;
	V3DLONG n_node;
	V3DLONG n_segment; //091009 RZC
	V3DLONG n_branch;
	V3DLONG n_tip;  //091027 RZC
	double bbox_xmin, bbox_xmax, bbox_ymin, bbox_ymax, bbox_zmin, bbox_zmax;
	double moments[13];
	NeuronMorphoInfo()	{b_valid=false;}
	bool isValid() {return b_valid;}
};

NeuronMorphoInfo neuron_morpho_features(const NeuronTree *p); //collect the morphological features of a neuron
QString get_neuron_morpho_features_str(const NeuronTree *p);

#endif
