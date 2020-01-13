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

#include <QtGlobal>
#include <QInputDialog>

#include "neuron_sim_scores.h"
#include "v_neuronswc.h"
#include <iostream>
#include "global_feature_compute.h"

V_NeuronSWC get_v_neuron_swc(const NeuronTree *p);
vector<V_NeuronSWC> get_neuron_segments(const NeuronTree *p);
void neuron_branch_tip_count(V3DLONG &n_branch, V3DLONG &n_tip, const vector<V_NeuronSWC> & segment_list);
void neuron_branch_tip_count(V3DLONG &n_branch, V3DLONG &n_tip, const V_NeuronSWC & in_swc);

double d_thres = 2.0;

//round all neuronal node coordinates, and compute the average min distance matches for all places the neurons go through
NeuronDistSimple neuron_score_rounding_nearest_neighbor(const NeuronTree *p1, const NeuronTree *p2,bool bmenu, double d_thres_updated)
{
	NeuronDistSimple ss;

    //===
    if(bmenu)
    {
        bool ok1;

#if defined(USE_Qt5)
        V3DLONG d_thres_new = QInputDialog::getInt(0, "change the default distance threshold",
                                                       "The visible-spatial-distance threshold of two neurons: ", d_thres, 2, 20, 1, &ok1);
#else
        V3DLONG d_thres_new = QInputDialog::getInteger(0, "change the default distance threshold",
                                                       "The visible-spatial-distance threshold of two neurons: ", d_thres, 2, 20, 1, &ok1);
#endif
        if (ok1)
        {
            d_thres = d_thres_new;
        }
    }else
        d_thres = d_thres_updated;
    //===


	if (!p1 || !p2) return ss;
	V3DLONG p1sz = p1->listNeuron.size(), p2sz = p2->listNeuron.size();
    if (p1sz<2 || p2sz<2) {
        cout<<"Input neurons has too few nodes, distance calculation requires at least two nodes." <<endl;
        return ss; //requires two nodes at least
     }

	double sum12, sum21;
	V3DLONG nseg1, nseg2;
	double sum12big, sum21big;
    double maxdist12 = -1, maxdist21 = -1; //set as some big numbers
	V3DLONG nseg1big, nseg2big;
    sum12 = dist_directional_swc_1_2(nseg1, nseg1big, sum12big, p1, p2, maxdist12);
    sum21 = dist_directional_swc_1_2(nseg2, nseg2big, sum21big, p2, p1, maxdist21);

    //qDebug() << "sum12="<<sum12 << "npoints1="<< nseg1 << "sum21="<< sum21 << "npoint2="<< nseg2;
    //qDebug() << "sum12big="<<sum12big << "npoints1big="<< nseg1big << "sum21big="<< sum21big << "npoint2big="<< nseg2big;
    //qDebug() << "maxdist12="<<maxdist12 << "maxdist21="<< maxdist21;


    ss.dist_12_allnodes = sum12/nseg1;
    ss.dist_21_allnodes = sum21/nseg2;

    ss.dist_allnodes = (sum12/nseg1 + sum21/nseg2)/2.0;
	if (nseg1big>0)
	{
		if (nseg2big>0)
			ss.dist_apartnodes = (sum12big/nseg1big + sum21big/nseg2big)/2.0;
		else
			ss.dist_apartnodes = (sum12big/nseg1big);
	}
	else
	{
		if (nseg2big>0)
			ss.dist_apartnodes = (sum21big/nseg2big);
		else
			ss.dist_apartnodes = 0;
	}

    ss.percent_12_apartnodes = double(nseg1big)/nseg1;
    ss.percent_21_apartnodes = double(nseg2big)/nseg2;
	ss.percent_apartnodes = (double(nseg1big)/nseg1 + double(nseg2big)/nseg2)/2.0;

    ss.dist_max = (maxdist12<maxdist21) ? maxdist12 : maxdist21; //this max distance should refelect the meaningful measure.
                                                                 // Becasue the two neurons (tracts) can have different starting and ending locations,
                                                                 // the bigger one of  maxdist12 and maxdist21 could simply reflect the big difference of
                                                                 // of the starting locations of the two tracts. Thus I use the smaller one, which
                                                                // should correspond better to the max distance at the truck part of two tracts. PHC 20140318.
	return ss;
}

double dist_directional_swc_1_2(V3DLONG & nseg1, V3DLONG & nseg1big, double & sum1big, const NeuronTree *p1, const NeuronTree *p2, double & maxdist)
{
	if (!p1 || !p2) return -1;
	V3DLONG p1sz = p1->listNeuron.size(), p2sz = p2->listNeuron.size();
	if (p1sz<2 || p2sz<2) return -1;

	NeuronSWC *tp1, *tp2;
	V3DLONG i, j;
	double sum1=0;
	nseg1=0;
	nseg1big=0;
	sum1big=0;

	QHash<int, int> h1 = generate_neuron_swc_hash(p1); //generate a hash lookup table from a neuron swc graph

	for (i=0;i<p1->listNeuron.size();i++)
	{
		//first find the two ends of a line seg
        tp1 = (NeuronSWC *)(&(p1->listNeuron.at(i)));
        if (tp1->pn < 0)
			continue;
		tp2 = (NeuronSWC *)(&(p1->listNeuron.at(h1.value(tp1->pn)))); //use hash table
		//qDebug() << "i="<< i << " pn="<<tp1->pn - 1;

		//now produce a series of points for the line seg
		double len=dist_L2(XYZ(tp1->x,tp1->y,tp1->z), XYZ(tp2->x,tp2->y,tp2->z));
		int N = int(1+len+0.5);
		XYZ ptdiff;
		if (N<=1)
		{
            //qDebug() << "detect one very short segment, len=" << len;
			ptdiff = XYZ(0,0,0);
		}
		else
		{
			double N1=1.0/(N-1);
			ptdiff = XYZ(N1,N1,N1) * XYZ(tp2->x-tp1->x, tp2->y-tp1->y, tp2->z-tp1->z);
		}
        //qDebug() << "N="<<N << "len=" <<len << "xd="<<ptdiff.x << " yd=" << ptdiff.y << " zd=" << ptdiff.z << " ";
		for (j=0;j<N;j++)
		{
			XYZ curpt(tp1->x + ptdiff.x*j, tp1->y + ptdiff.y*j, tp1->z + ptdiff.z*j);
			double cur_d = dist_pt_to_swc(curpt, p2);
			sum1 += cur_d;
			nseg1++;

            if (maxdist<0) //use <0 as a condition to check if maxdist has been set
                maxdist = cur_d;
            else
            {
                if (maxdist<cur_d)
                    maxdist = cur_d;
            }

            if (cur_d>=d_thres)
			{
				sum1big += cur_d;
				nseg1big++;
                //qDebug() << "(" << cur_d << ", " << nseg1big << ")";
			}

		}
	}
    //qDebug() << "end directional neuronal distance computing";

	return sum1;
}

double dist_pt_to_swc(const XYZ & pt, const NeuronTree * p_tree)
{
	//first find all the edge end point distances
	if (!p_tree) return -1;
	V3DLONG p_tree_sz = p_tree->listNeuron.size();

	NeuronSWC *tp1, *tp2;
	if (p_tree_sz<2)
	{
		tp1 = (NeuronSWC *)(&(p_tree->listNeuron.at(0)));
		return norm(pt - XYZ(tp1->x, tp1->y, tp1->z));
	}

	QHash<int, int> h = generate_neuron_swc_hash(p_tree); //generate a hash lookup table from a neuron swc graph

	V3DLONG i;
	double min_dist;
	bool b_first=false;
	for (i=0;i<p_tree->listNeuron.size();i++)
	{
		//first find the two ends of a line seg
		tp1 = (NeuronSWC *)(&(p_tree->listNeuron.at(i)));
        if (tp1->pn < 0)
			continue;
		tp2 = (NeuronSWC *)(&(p_tree->listNeuron.at(h.value(tp1->pn)))); //use hash table

		//now compute the distance between the pt and the current segment
		double cur_d = dist_pt_to_line_seg(pt, XYZ(tp1->x,tp1->y,tp1->z), XYZ(tp2->x,tp2->y,tp2->z));

		//now find the min distance
		if (b_first==false) {min_dist = cur_d; b_first=true;}
		else min_dist = (min_dist>cur_d) ? cur_d : min_dist;
	}

	return min_dist;
}

double dist_pt_to_line(const XYZ & p0, const XYZ &  p1, const XYZ &  p2) //p1 and p2 are the two points of the straight line, and p0 the point
{
	if (p1==p2)
		return norm(p0-p1);
	else if (p0==p1 || p0==p2) return 0;

	XYZ d12 = p2-p1;
	XYZ d01 = p1-p0;
	float v01 = dot(d01, d01);
	float v12 = dot(d12, d12);
	float d012 = dot(d12, d01);

	float t = -d012/v12;

	XYZ xpt(p1.x+d12.x*t, p1.y+d12.y*t, p1.z+d12.z*t);
	return dist_L2(xpt, p0);
}

double dist_pt_to_line_seg(const XYZ & p0, const XYZ &  p1, const XYZ &  p2) //p1 and p2 are the two ends of the line segment, and p0 the point
{
	if (p1==p2)
		return norm(p0-p1);
	else if (p0==p1 || p0==p2) return 0;

	XYZ d12 = p2-p1;
	XYZ d01 = p1-p0;
	float v01 = dot(d01, d01);
	float v12 = dot(d12, d12);
	float d012 = dot(d12, d01);

	float t = -d012/v12;
	if (t<0 || t>1) //then no intersection within the lineseg
	{
		double d01 = dist_L2(p0, p1);
		double d02 = dist_L2(p0, p2);
		return (d01<d02)?d01:d02;
	}
	else
	{
		XYZ xpt(p1.x+d12.x*t, p1.y+d12.y*t, p1.z+d12.z*t);
		return dist_L2(xpt, p0);
	}
}

QHash<int, int> generate_neuron_swc_hash(const NeuronTree * p_tree) //generate a hash lookup table from a neuron swc graph
{
	QHash<int, int> h;
	if (!p_tree || p_tree->listNeuron.size()<=0) return h;

	NeuronSWC *tp;
	for (int i=0;i<p_tree->listNeuron.size(); i++)
	{
		tp = (NeuronSWC *)(&(p_tree->listNeuron.at(i)));
		h.insert(tp->n, i); //make the key is neuron's id, and the value is the current order
	}
	return h;
}

NeuronMorphoInfo neuron_morpho_features(const NeuronTree *p) //collect the morphological features of a neuron
{
	NeuronMorphoInfo m;
	if (!p) return m;

	//create some reference names so that easier to write the code
	double & total_length = m.total_length;
	V3DLONG & n_node = m.n_node;
	V3DLONG & n_segment = m.n_segment; //091009 RZC
	V3DLONG & n_branch = m.n_branch;
	V3DLONG & n_tip = m.n_tip;
	double & bbox_xmin = m.bbox_xmin, bbox_xmax=m.bbox_xmax, bbox_ymin=m.bbox_ymin, bbox_ymax=m.bbox_ymax, bbox_zmin=m.bbox_zmin, bbox_zmax=m.bbox_zmax;
	double * moments = m.moments;

	//
	NeuronSWC *tp1, *tp2;
	V3DLONG i, j;

	QHash<int, int> h = generate_neuron_swc_hash(p); //generate a hash lookup table from a neuron swc graph
	n_node = h.size();

	unsigned char *nchildren = new unsigned char [n_node]; //track the # of children each node has
	for (i=0;i<n_node;i++) nchildren[i]=0;

	total_length = 0;
	for (i=0, total_length=0.0;i<p->listNeuron.size();i++)
	{
		//first find the two ends of a line seg
		tp1 = (NeuronSWC *)(&(p->listNeuron.at(i)));
        if (tp1->pn < 0)
			continue;                              
		tp2 = (NeuronSWC *)(&(p->listNeuron.at(h.value(tp1->pn)))); //use hash table

		nchildren[h.value(tp1->pn)]++; //update the parent node's children number

		//qDebug() << "i="<< i << " pn="<<tp1->pn - 1;

		//compute the length
		double len=dist_L2(XYZ(tp1->x,tp1->y,tp1->z), XYZ(tp2->x,tp2->y,tp2->z));
		total_length += len;

		//compute the bbox
		if (i==0)
		{
			bbox_xmin = bbox_xmax = tp1->x;
			bbox_ymin = bbox_ymax = tp1->y;
			bbox_zmin = bbox_zmax = tp1->z;
		}
		else
		{
			if (bbox_xmin>tp1->x) bbox_xmin = tp1->x; if (bbox_xmax<tp1->x) bbox_xmax = tp1->x;
			if (bbox_ymin>tp1->y) bbox_ymin = tp1->y; if (bbox_ymax<tp1->y) bbox_ymax = tp1->y;
			if (bbox_zmin>tp1->z) bbox_zmin = tp1->z; if (bbox_zmax<tp1->z) bbox_zmax = tp1->z;
		}

//		//now produce a series of points for the line seg
//		int N = int(1+len+0.5);
//		XYZ ptdiff;
//		if (N<=1)
//		{
//			qDebug() << "detect one very short segment, len=" << len;
//			ptdiff = XYZ(0,0,0);
//		}
//		else
//		{
//			double N1=1.0/(N-1);
//			ptdiff = XYZ(N1,N1,N1) * XYZ(tp2->x-tp1->x, tp2->y-tp1->y, tp2->z-tp1->z);
//		}
//		qDebug() << "N="<<N << "len=" <<len << "xd="<<ptdiff.x << " yd=" << ptdiff.y << " zd=" << ptdiff.z << " ";
//		for (j=0;j<N;j++)
//		{
//			XYZ curpt(tp1->x + ptdiff.x*j, tp1->y + ptdiff.y*j, tp1->z + ptdiff.z*j);
//			double cur_d = dist_pt_to_swc(curpt, p2);
//			sum1 += cur_d;
//			nseg1++;
//		}
	}

//	for (i=0, n_branch=0;i<n_node;i++) //find the n_branches
//		if (nchildren[i]>1) n_branch++;

	if (nchildren) {delete nchildren; nchildren=0;}


	//////////////////////////////////////////////////////////
	// 091009 RZC: count after splitting to simple segments
	//////////////////////////////////////////////////////////

	vector <V_NeuronSWC> seg_vec = get_neuron_segments(p);
	n_segment = seg_vec.size();
	//neuron_branch_tip_count(n_branch, n_tip, seg_vec);

	// 091212 RZC: count changed using link_map
    V_NeuronSWC v_neuron = join_V_NeuronSWC_vec(seg_vec);
    //V_NeuronSWC v_neuron = get_v_neuron_swc(p);
    neuron_branch_tip_count(n_branch, n_tip, v_neuron);

    //141006 CHB: above may cause error when there is overlapping point in the file, corrrected:
    vector<int> num_child (p->listNeuron.size(),0);
    for(int i=0; i<p->listNeuron.size(); i++){
        V3DLONG pn=p->listNeuron.at(i).pn, pid=-1;
        if(p->hashNeuron.contains(pn))
            pid=p->hashNeuron[pn];
        else
            continue;
        num_child[pid]++;
    }
    n_branch=0;
    n_tip=0;
    for(int i=0; i<num_child.size(); i++){
        if(p->listNeuron.at(i).pn<0) //skip root
            continue;
        if(num_child[i]==0) //tips
            n_tip++;
        if(num_child[i]>1) //branch points
            n_branch++;
    }

	return m;
}

QString get_neuron_morpho_features_str(const NeuronTree *p)
{
    QString tmpstr, ts2;
    double * features = new double[22];
    computeFeature(*p,features);

    tmpstr += "number of nodes = "; ts2.setNum(features[0]); tmpstr += ts2 + "<br>";
    tmpstr += "soma surface = "; ts2.setNum(features[1]); tmpstr += ts2 + "<br>";
    tmpstr += "number of stems = "; ts2.setNum(features[2]); tmpstr += ts2 + "<br>";
    tmpstr += "number of bifurcations = "; ts2.setNum(features[3]); tmpstr += ts2 + "<br>";
    tmpstr += "number of branches = "; ts2.setNum(features[4]); tmpstr += ts2 + "<br>";
    tmpstr += "number of tips = "; ts2.setNum(features[5]); tmpstr += ts2 + "<br>";
    tmpstr += "overall width = "; ts2.setNum(features[6]); tmpstr += ts2 + "<br>";
    tmpstr += "overall height = "; ts2.setNum(features[7]); tmpstr += ts2 + "<br>";
    tmpstr += "overall depth = "; ts2.setNum(features[8]); tmpstr += ts2 + "<br>";
    tmpstr += "average diameter = "; ts2.setNum(features[9]); tmpstr += ts2 + "<br>";
    tmpstr += "total length = "; ts2.setNum(features[10]); tmpstr += ts2 + "<br>";
    tmpstr += "total surface = "; ts2.setNum(features[11]); tmpstr += ts2 + "<br>";
    tmpstr += "total volume = "; ts2.setNum(features[12]); tmpstr += ts2 + "<br>";
    tmpstr += "max euclidean distance = "; ts2.setNum(features[13]); tmpstr += ts2 + "<br>";
    tmpstr += "max path distance = "; ts2.setNum(features[14]); tmpstr += ts2 + "<br>";
    tmpstr += "max branch order = "; ts2.setNum(features[15]); tmpstr += ts2 + "<br>";
    tmpstr += "average contraction = "; ts2.setNum(features[16]); tmpstr += ts2 + "<br>";
    tmpstr += "average fragmentation = "; ts2.setNum(features[17]); tmpstr += ts2 + "<br>";
    tmpstr += "average parent-daughter ratio = "; ts2.setNum(features[18]); tmpstr += ts2 + "<br>";
    tmpstr += "average bifurcation angle local = "; ts2.setNum(features[19]); tmpstr += ts2 + "<br>";
    tmpstr += "average bifurcation angle remote = "; ts2.setNum(features[20]); tmpstr += ts2 + "<br>";
    tmpstr += "Hausdorff dimension = "; ts2.setNum(features[21]); tmpstr += ts2 + "<br>";
    if(features) {delete []features; features=0;}

    return tmpstr;
}

V_NeuronSWC get_v_neuron_swc(const NeuronTree *p)
{
	V_NeuronSWC cur_seg;	cur_seg.clear();
	const QList<NeuronSWC> & qlist = p->listNeuron;

	for (V3DLONG i=0;i<qlist.size();i++)
	{
		V_NeuronSWC_unit v;
		v.n		= qlist[i].n;
		v.type	= qlist[i].type;
		v.x 	= qlist[i].x;
		v.y 	= qlist[i].y;
		v.z 	= qlist[i].z;
		v.r 	= qlist[i].r;
		v.parent = qlist[i].pn;

		cur_seg.append(v);
		//qDebug("%d ", cur_seg.nnodes());
	}
	cur_seg.name = qPrintable(QString("%1").arg(1));
	cur_seg.b_linegraph=true; //donot forget to do this
	return cur_seg;
}
vector<V_NeuronSWC> get_neuron_segments(const NeuronTree *p)
{
	V_NeuronSWC cur_seg = get_v_neuron_swc(p);
	vector<V_NeuronSWC> seg_list;
	seg_list = cur_seg.decompose();
	return seg_list;
}

void neuron_branch_tip_count(V3DLONG &n_branch, V3DLONG &n_tip, const vector<V_NeuronSWC> & segment_list)
{
	QMap <V_NeuronSWC_coord, V_NeuronSWC_unit> map;
	map.clear();

	for (V3DLONG i=0; i<segment_list.size(); i++)
	{
		const V_NeuronSWC & seg = segment_list.at(i);
		for (V3DLONG j=0; j<seg.row.size(); j++)
		{
			V_NeuronSWC_unit node = seg.row.at(j);
			V_NeuronSWC_coord key = node;
			map.insertMulti(key, node);
		}
	}

	n_branch = 0;
	n_tip = 2*segment_list.size();

	QList <V_NeuronSWC_coord> keys = map.uniqueKeys();
	for (V3DLONG i=0; i<keys.size(); i++)
	{
		V_NeuronSWC_coord key = keys.at(i);
		int nkey = map.count(key);
		if (nkey >= 3)
		{
			n_branch ++;
		}
		if (nkey >= 2)
		{
			n_tip -= nkey;
		}
	}
}

//091212 RZC
void neuron_branch_tip_count(V3DLONG &n_branch, V3DLONG &n_tip, const V_NeuronSWC & in_swc)
{
	// map swc's index --> vector's index & in/out link
	Link_Map link_map = get_link_map(in_swc);
	//qDebug("link_map created.");

	n_branch = 0;
	n_tip = 0;

    V3DLONG n_path=0, n_single=0; //for test

	Link_Map_Iter it;
	for (it=link_map.begin(); it!=link_map.end(); it++)
	{
//		const V_NeuronSWC_unit & cur_node = in_swc.row.at(i);
//		Node_Link & nodelink = link_map[V3DLONG(cur_node.n)];
		Node_Link & nodelink = (*it).second;

        if(nodelink.nlink == 0) n_single++;

		if (nodelink.nlink == 1)
			n_tip ++;

        if (nodelink.nlink == 2) n_path++;// path point

		if (nodelink.nlink >= 3)
		{
			n_branch ++;
			//qDebug("branch #%d (%g %g %g) %d", V3DLONG(cur_node.n),cur_node.x,cur_node.y,cur_node.z, nodelink.nlink);
		}
	}

    qDebug("cojoc: all:%d/link:%d/0:%d/1:%d/2:%d/3+:%d",in_swc.row.size(),link_map.size(),n_single,n_tip,n_path,n_branch);
}
