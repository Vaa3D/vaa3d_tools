

#include <QtGlobal>
#include <QInputDialog>
#include "neuron_matching_distance.h"
#include "v_neuronswc.h"
#include <iostream>

V_NeuronSWC get_v_neuron_swc(const NeuronTree *p);
vector<V_NeuronSWC> get_neuron_segments(const NeuronTree *p);
void neuron_branch_tip_count(V3DLONG &n_branch, V3DLONG &n_tip, const vector<V_NeuronSWC> & segment_list);
void neuron_branch_tip_count(V3DLONG &n_branch, V3DLONG &n_tip, const V_NeuronSWC & in_swc);

double d_thres = 10.0;

//round all neuronal node coordinates, and compute the average min distance matches for all places the neurons go through
NeuronDist resampled_neuron_matching_distance(const NeuronTree *p1, const NeuronTree *p2,bool bmenu)
{
    NeuronDist ss;

    //===
    if(bmenu)
    {
        bool ok1;
#ifndef USE_Qt5
        V3DLONG d_thres_new = QInputDialog::getInteger(0, "change the default distance threshold",
                                                       "The visible-spatial-distance threshold of two neurons: ", d_thres, 2, 20, 1, &ok1);
#else
        V3DLONG d_thres_new = QInputDialog::getInt(0, "change the default distance threshold",
                                                       "The visible-spatial-distance threshold of two neurons: ", d_thres, 2, 20, 1, &ok1);
#endif
        if (ok1)
        {
            d_thres = d_thres_new;
        }
    }
    //===


    if (!p1 || !p2) return ss;
    V3DLONG p1sz = p1->listNeuron.size(), p2sz = p2->listNeuron.size();
    if (p1sz<1 || p2sz<1) return ss;

    double w_dist_12big = -1, w_dist_21big = -1, sum12=-1, sum21=-1;
    double maxdist12 = -1, maxdist21 = -1; //set as some big numbers
    double big_ratio12 = 0.0,big_ratio21 = 0.0;

    double weighted_dist_12 = resampled_dist_directional_swc_1_2(sum12, w_dist_12big, big_ratio12, p1, p2, maxdist12, false);
    double weighted_dist_21 = resampled_dist_directional_swc_1_2(sum21, w_dist_21big, big_ratio21,p2, p1, maxdist21, false);


    if (weighted_dist_12 < 0 || weighted_dist_21 <0)
    {
        std::cout <<"Error: one of the input neuron probably has less than 2 SWC nodes, cannot computing."<<std::endl;
        ss.matching_total_dist12 = -1;
        ss.matching_total_dist21 = -1;
        ss.matching_total_dist_ave = -1;
        ss.weighted_dist12_allnodes = -1;
        ss.weighted_dist21_allnodes = -1;
        ss.weighted_dist_ave_allnodes = -1;
        ss.dist_max = -1;
        ss.dist_apartnodes = -1;
        ss.percent_apartnodes = -1;
        return ss;
    }


    ss.matching_total_dist12 = sum12;
    ss.matching_total_dist21 = sum21;
    ss.matching_total_dist_ave = (sum21+sum12)/2.0;


    ss.weighted_dist12_allnodes = weighted_dist_12;
    ss.weighted_dist21_allnodes = weighted_dist_21;
    ss.weighted_dist_ave_allnodes = (weighted_dist_12+ weighted_dist_21)/2.0;


    if (w_dist_12big>-1)
    {
        if (w_dist_21big>-1)
            ss.dist_apartnodes = (w_dist_12big + w_dist_21big)/2.0;
        else
            ss.dist_apartnodes = w_dist_12big;
    }
    else
    {
        if (w_dist_21big>-1)
            ss.dist_apartnodes = w_dist_21big;
        else
            ss.dist_apartnodes = 0;
    }


    ss.percent_apartnodes = (big_ratio12 + big_ratio21)/2.0;

    ss.dist_max = (maxdist12<maxdist21) ? maxdist12 : maxdist21; //this max distance should refelect the meaningful measure.
                                                                 // Becasue the two neurons (tracts) can have different starting and ending locations,
                                                                 // the bigger one of  maxdist12 and maxdist21 could simply reflect the big difference of
                                                                 // of the starting locations of the two tracts. Thus I use the smaller one, which
                                                                // should correspond better to the max distance at the truck part of two tracts. PHC 20140318.
    return ss;
}

double resampled_dist_directional_swc_1_2(double & sum_dist, double & w_dist_12big, double & difference_ratio12, const NeuronTree *p1,
                                         const NeuronTree *p2, double & maxdist, bool USE_WEIGHT)
{
    if (!p1 || !p2) return -1;
    V3DLONG p1sz = p1->listNeuron.size(), p2sz = p2->listNeuron.size();
    if (p1sz<2 || p2sz<2)
    {
        return -1;
    }
    NeuronSWC *tp1, *tp2;
    V3DLONG i, j;
    double sum_weights=0.0;
    double sum_weights_big=0.0;
    w_dist_12big=0;
    difference_ratio12 = 0.0;
    V3DLONG nseg=0;
    V3DLONG nsegbig=0;

    QHash<int, int> h1 = generate_neuron_swc_hash(p1); //generate a hash lookup table from a neuron swc graph

    std::cout << "calculating directional weighted average distance"<< std::endl;
    // collect the weights
    vector<double> fea_values;
    NeuronSWC t1 = p1->listNeuron.at(0);
    NeuronSWC t2 = p2->listNeuron.at(0);

    if (USE_WEIGHT){
        if (t1.fea_val.size()>0 )
        {
            std::cout << "Use neuron 1 feature values as the weights."<<std::endl;
            for (int ii=0;ii<p1->listNeuron.size();ii++){
                NeuronSWC t = p1->listNeuron.at(ii);
                fea_values.push_back( t.fea_val[0]);
            }

        }
        else{
            std::cout << "No weights are provided."<<std::endl;
            return -1;

        }
    }
    else
    {
        for (int ii=0;ii<p1->listNeuron.size();ii++){
            fea_values.push_back( 1.0);
        }
    }


    //assuming the input swcs are resampled
    //no need to interpolate between nodes
    for (i=0;i<p1->listNeuron.size();i++)
    {
        double weight = fea_values[i];
        tp1 = (NeuronSWC *)(&(p1->listNeuron.at(i)));


        if (tp1->pn < 0 || tp1->pn >= p1sz)
            continue;

        XYZ curpt(tp1->x, tp1->y , tp1->z);
        double cur_d = dist_pt_to_swc(curpt, p2);
        sum_dist += cur_d*weight;
        sum_weights += weight;
        nseg++;

        if (maxdist<0) //use <0 as a condition to check if maxdist has been set
            maxdist = cur_d;
        else
        {
            if (maxdist<cur_d)
                maxdist = cur_d;
        }

        if (cur_d>=d_thres)
        {
            w_dist_12big += cur_d *weight;
            sum_weights_big +=  weight;
            nsegbig++;
            //qDebug() << "(" << cur_d << ", " << w_dist_nseg1big << ")";
        }


    }
    //qDebug() << "end directional neuronal distance computing";

    if( nseg > 0 ){
        difference_ratio12 = double(nsegbig)/double(nseg);
    }

    if (nsegbig == 0 )
    {
        w_dist_12big = -1;
    }
    else
    {
        w_dist_12big = w_dist_12big/sum_weights_big;
    }


    if (sum_weights ==0 )
    {
        w_dist_12big = -1;
        return -1;
    }
    return sum_dist/sum_weights;

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
        if (tp1->pn < 0 || tp1->pn >= p_tree_sz)
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
        if (tp1->pn < 0 || tp1->pn >= n_node)
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
    NeuronMorphoInfo tmp_info = neuron_morpho_features(p);
    tmpstr += "total length = "; ts2.setNum(tmp_info.total_length); tmpstr += ts2 + "<br>";
    tmpstr += "total nodes = "; ts2.setNum(tmp_info.n_node); tmpstr += ts2 + "<br>";
    tmpstr += "total segments = "; ts2.setNum(tmp_info.n_segment); tmpstr += ts2 + "<br>";
    tmpstr += "total branch points = "; ts2.setNum(tmp_info.n_branch); tmpstr += ts2 + "<br>";
    tmpstr += "total tips = "; ts2.setNum(tmp_info.n_tip); tmpstr += ts2 + "<br>";
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

    qDebug("all:%d/link:%d/0:%d/1:%d/2:%d/3+:%d",in_swc.row.size(),link_map.size(),n_single,n_tip,n_path,n_branch);
}
