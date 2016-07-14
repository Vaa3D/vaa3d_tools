#include "kd-tree.h"
#include "color_xyz.h"

using namespace std;

ANNkd_tree *nt_to_kdt(NeuronTree nt, int nPts)
{

    const int dim = 3;

    ANNpointArray kd_pts = annAllocPts(nPts, dim);

    //add all points from the neuron tree to the ANN point array
    for (int m = 0; m < nPts; ++m) {
    	//hard-coded for 3D
        kd_pts[m][0] = nt.listNeuron[m].x;
        kd_pts[m][1] = nt.listNeuron[m].y;
        kd_pts[m][2] = nt.listNeuron[m].z;
    }

    return new ANNkd_tree(kd_pts, nPts, dim);
}

double kd_correspondingPointFromNeuron( XYZ pt, NeuronTree * p_nt, ANNkd_tree *kd, XYZ & closest_p)
{
    double min_dist = LONG_MAX;
    closest_p.x = -1;
    closest_p.y = -1;
    closest_p.z = -1;

    const int dim = 3;
    ANNpoint queryPt = annAllocPt(dim);
    queryPt[0] = pt.x;
    queryPt[1] = pt.y;
    queryPt[2] = pt.z;

    //if tree only has one point, just return that point
    if (kd->nPoints() < 2) {
        closest_p = p_nt->listNeuron.at(0);
        return dist_L2(closest_p, pt);
    }

    int k = 2; //neighbors to find
    ANNidxArray nIdx = new ANNidx[k];
    ANNdistArray dist = new ANNdist[k];

    kd->annkSearch(queryPt, k, nIdx, dist, 0);

    int closest1 = nIdx[0];
    int closest2 = nIdx[1];

    QHash<int, int> h =p_nt->hashNeuron;

    NeuronSWC *tp1, *tp2;
    //check c_node1 and parent
    tp1 = (NeuronSWC *)(&(p_nt->listNeuron.at(closest1)));
    if (tp1->pn < 0 ) //if root
    {
        min_dist =dist_L2( XYZ(tp1->x,tp1->y,tp1->z), pt);
        closest_p = XYZ(tp1->x,tp1->y,tp1->z);
    } else {
        tp2 = (NeuronSWC *)(&(p_nt->listNeuron.at(h.value(tp1->pn)))); //use hash table
        XYZ c_p;
        min_dist = dist_pt_to_line_seg(pt, XYZ(tp1->x,tp1->y,tp1->z), XYZ(tp2->x,tp2->y,tp2->z),c_p);
        closest_p = c_p;
    }

    //check if c_node2 and parent are closer
    tp1 = (NeuronSWC *)(&(p_nt->listNeuron.at(closest2)));
    if (tp1->pn < 0 ) //if root
    {
        double sec_dist =dist_L2( XYZ(tp1->x,tp1->y,tp1->z), pt);
        if (sec_dist < min_dist) {
            min_dist = sec_dist;
            closest_p = XYZ(tp1->x,tp1->y,tp1->z);
        }
    } else {
        tp2 = (NeuronSWC *)(&(p_nt->listNeuron.at(h.value(tp1->pn)))); //use hash table

        //now compute the distance between the pt and the current segment
        XYZ c_p;
        double cur_d = dist_pt_to_line_seg(pt, XYZ(tp1->x,tp1->y,tp1->z), XYZ(tp2->x,tp2->y,tp2->z),c_p);

        //now find the min distance
        if (min_dist > cur_d){
            min_dist = cur_d;
            closest_p = c_p;
        }
    }

    delete [] nIdx;
    delete [] dist;

    return min_dist;
}


double kd_correspondingNodeFromNeuron(XYZ pt, QList<NeuronSWC> listNodes, int &closestNodeIdx, ANNkd_tree * kd, int TYPE_MERGED)
{

    double min_dist = LONG_MAX;
    closestNodeIdx = -1;

    const int dim = 3;
    ANNpoint queryPt = annAllocPt(dim);
    queryPt[0] = pt.x;
    queryPt[1] = pt.y;
    queryPt[2] = pt.z;

    NeuronSWC tp;
    bool found = false;
    int k = 4;
    int cur = 0;

    while (!found && (k <= listNodes.size())) {

        ANNidxArray nIdx = new ANNidx[k];
        ANNdistArray dist = new ANNdist[k];

        kd->annkSearch(queryPt, k, nIdx, dist, 0);

        int id;
        //are any of the nodes found not merged
        while (cur < k) {
            id = nIdx[cur];
            tp = listNodes.at(id);
            if (tp.type != TYPE_MERGED) {
                found = true;
                min_dist = sqrt(dist[cur]);
                //cout << "setting node idx to " << id << endl;
                closestNodeIdx = id;
                break;
            }
            //if merged, check distance for cutoff point - since sorted can exit when reach a node further
            if (sqrt(dist[cur]) > 1.0) {
                found = true;
                break;
            }

            cur++;
        }

        k = 2 * k; //num of neighbors to look for

        delete [] nIdx;
        delete [] dist;
    }

    return min_dist;
 }

double dist_pt_to_line_seg(const XYZ p0, const XYZ p1, const XYZ p2, XYZ & closestPt) //p1 and p2 are the two ends of the line segment, and p0 the point
{
    if (p1==p2)
    {
        closestPt = p1;
        return norm(p0-p1);
    }
        else if (p0==p1 || p0==p2)
    {
        closestPt = p0;
        return 0.0;
    }

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

        if (d01<d02){
          closestPt=XYZ(p1.x,p1.y,p1.z);
          return d01;
        }
        else
        {
          closestPt=XYZ(p2.x,p2.y,p2.z);
          return d02;
        }


    }
    else
    {//intersection
        XYZ xpt(p1.x+d12.x*t, p1.y+d12.y*t, p1.z+d12.z*t);
        closestPt=xpt;
        return dist_L2(xpt, p0);
    }
}
