#include "converge_trees.h"
#include "resample_swc.h"

using namespace std;

/**
 *  @brief  Accumulate values in a range.
 *
 *  Accumulates the values in the range [first,last) using operator+().  The
 *  initial value is @a init.  The values are processed in order.
 *
 *  @param  first  Start of range.
 *  @param  last  End of range.
 *  @param  init  Starting value to add other values to.
 *  @return  The final sum.
 */
//template<typename _InputIterator, typename _Tp>
//  inline _Tp
//  accumulate(_InputIterator __first, _InputIterator __last, _Tp __init)
//  {
//    // concept requirements
//    __glibcxx_function_requires(_InputIteratorConcept<_InputIterator>)
//    __glibcxx_requires_valid_range(__first, __last);

//    for (; __first != __last; ++__first)
//  __init = __init + *__first;
//    return __init;
//  }

///**
// *  @brief  Accumulate values in a range with operation.
// *
// *  Accumulates the values in the range [first,last) using the function
// *  object @a binary_op.  The initial value is @a init.  The values are
// *  processed in order.
// *
// *  @param  first  Start of range.
// *  @param  last  End of range.
// *  @param  init  Starting value to add other values to.
// *  @param  binary_op  Function object to accumulate with.
// *  @return  The final sum.
// */
//template<typename _InputIterator, typename _Tp, typename _BinaryOperation>
//  inline _Tp
//  accumulate(_InputIterator __first, _InputIterator __last, _Tp __init,
//         _BinaryOperation __binary_op)
//  {
//    // concept requirements
//    __glibcxx_function_requires(_InputIteratorConcept<_InputIterator>)
//    __glibcxx_requires_valid_range(__first, __last);

//    for (; __first != __last; ++__first)
//  __init = __binary_op(__init, *__first);
//    return __init;
//  }

//XYZ kd_mean_XYZ(vector<XYZ> points)
//{
//    vector<double> x;
//    vector<double> y;
//    vector<double> z;
//    XYZ averageP;
//    if (points.size() == 0){cout<<"empty point list!" <<endl;}
//    for (int i = 0 ; i < points.size(); i++)
//    {
//        x.push_back(points[i].x);
//        y.push_back(points[i].y);
//        z.push_back(points[i].z);
//    }
//    averageP.x = accumulate( x.begin(), x.end(), 0.0 )/ x.size();
//    averageP.y = accumulate( y.begin(), y.end(), 0.0 )/ y.size();
//    averageP.z = accumulate( z.begin(), z.end(), 0.0 )/ z.size();
//    return averageP;
//}

//double kd_dist_pt_to_line_seg(const XYZ p0, const XYZ p1, const XYZ p2, XYZ & closestPt) //p1 and p2 are the two ends of the line segment, and p0 the point
//{
//    if (p1==p2)
//    {
//        closestPt = p1;
//        return norm(p0-p1);
//    }
//        else if (p0==p1 || p0==p2)
//    {
//        closestPt = p0;
//        return 0.0;
//    }

//    XYZ d12 = p2-p1;
//    XYZ d01 = p1-p0;
//    float v01 = dot(d01, d01);
//    float v12 = dot(d12, d12);
//    float d012 = dot(d12, d01);

//    float t = -d012/v12;
//    if (t<0 || t>1) //then no intersection within the lineseg
//    {
//        double d01 = dist_L2(p0, p1);
//        double d02 = dist_L2(p0, p2);

//        if (d01<d02){
//          closestPt=XYZ(p1.x,p1.y,p1.z);
//          return d01;
//        }
//        else
//        {
//          closestPt=XYZ(p2.x,p2.y,p2.z);
//          return d02;
//        }


//    }
//    else
//    {//intersection
//        XYZ xpt(p1.x+d12.x*t, p1.y+d12.y*t, p1.z+d12.z*t);
//        closestPt=xpt;
//        return dist_L2(xpt, p0);
//    }
//}

//double kd_correspondingPointFromNeuron( XYZ pt, NeuronTree * p_nt, XYZ & closest_p)
//{
//   double min_dist = LONG_MAX;
//   closest_p.x = -1;
//   closest_p.y = -1;
//   closest_p.z = -1;


//   V3DLONG num_nodes = p_nt->listNeuron.size();

//   NeuronSWC *tp1, *tp2;
//   if (num_nodes<2 && num_nodes>0)
//   {
//       tp1 = (NeuronSWC *)(&(p_nt->listNeuron.at(0)));
//       return norm(pt - XYZ(tp1->x, tp1->y, tp1->z));
//   }

//   QHash<int, int> h =p_nt->hashNeuron;
//   V3DLONG i;

//   for (i=0;i<p_nt->listNeuron.size();i++)
//   {
//       //first find the two ends of a line seg
//       tp1 = (NeuronSWC *)(&(p_nt->listNeuron.at(i)));
//       if (tp1->pn < 0 )
//       {
//           double cur_d =dist_L2( XYZ(tp1->x,tp1->y,tp1->z), pt);
//           if (min_dist > cur_d){
//               min_dist = cur_d;
//               closest_p = XYZ(tp1->x,tp1->y,tp1->z);
//           }
//           continue;
//       }
//       tp2 = (NeuronSWC *)(&(p_nt->listNeuron.at(h.value(tp1->pn)))); //use hash table

//       //now compute the distance between the pt and the current segment
//       XYZ c_p;
//       double cur_d = kd_dist_pt_to_line_seg(pt, XYZ(tp1->x,tp1->y,tp1->z), XYZ(tp2->x,tp2->y,tp2->z),c_p);

//       //now find the min distance
//       if (min_dist > cur_d){
//           min_dist = cur_d;
//           closest_p = c_p;
//       }

////       if (min_dist <= 0.5){
////        //definitely a match already, skip the search
////           continue;
////       }

//   }

//   return min_dist;
//}

//void  kd_resample_neurons(vector<NeuronTree> nt_list,  vector<NeuronTree> * nt_list_resampled){

//    cout<<"Resampling..."<<endl;
//    for (int i = 0; i < nt_list.size(); i++)
//    {
//        NeuronTree nt = nt_list[i];
//        if (nt.listNeuron.size()>0)
//        {
//            //resample with step size 1
//            NeuronTree resampled = resample(nt, 1.0);
//            if (resampled.listNeuron.size()>0){
//                resampled.file = nt.file;
//                nt_list_resampled->push_back(resampled);
//            }
//        }
//    }
//     return;
// }

void nt_to_kdt(NeuronTree nt, ANNpointArray *kd_pts, int nPts)
{
    //add all points from the neuron tree to the ANN point array
    for (int m = 0; m < nPts; ++m) {
        //hard-coded for 3D
        (*kd_pts)[m][0] = nt.listNeuron[m].x;
        (*kd_pts)[m][1] = nt.listNeuron[m].y;
        (*kd_pts)[m][2] = nt.listNeuron[m].z;
    }
}

//double kd_match_and_center(vector<NeuronTree> nt_list, vector<ANNkd_tree *>kd_list, vector<ANNpointArray> kd_pts_list, int input_neuron_id,  double distance_threshold, NeuronTree & adjusted_neuron)
//{
//    if(  input_neuron_id > (nt_list.size() -1) )
//    {
//        cout<<"error in match_and_center(): input neuron id is wrong."<<endl;
//        return false;
//    }
//    double total_editing_dis = 0.0;
//    NeuronTree input_neuron = nt_list[input_neuron_id]; //the neuron tree
//    adjusted_neuron.deepCopy(input_neuron);
//    vector<XYZ> cluster;

//    for (int i = 0; i <input_neuron.listNeuron.size(); i++) //for each neuron in the tree
//    {
//        NeuronSWC s = input_neuron.listNeuron.at(i);
//        XYZ cur;
//        cur.x = s.x;
//        cur.y = s.y;
//        cur.z = s.z;

//        int dim = 3;
//        ANNpoint queryPt = annAllocPt(dim);
//        queryPt[0] = s.x;
//        queryPt[1] = s.y;
//        queryPt[2] = s.z;

//        cluster.clear();
//        //include itself into the cluster first
//        cluster.push_back(cur);

//        //int match_nodes = 0;

//        for (int j = 0; j < nt_list.size(); j++) //look at all other trees
//        {

//            if (j == input_neuron_id)
//            {// skip matching with itself,
//                continue;
//            }

//            XYZ closest_p;
//            //find closest swc node from resampled tree j -- actually finds closest point
//            //double min_dis = kd_correspondingPointFromNeuron(cur, &nt_list.at(j), closest_p);

//            //instead use kdtree search to find the closest node
//            int k = 1; //num of neighbors to look for

//            ANNidxArray nIdx = new ANNidx[k];
//            ANNdistArray dist = new ANNdist[k];

//            kd_list[j]->annkSearch(queryPt, k, nIdx, dist, 0);
//            double min_dis = sqrt(dist[0]);

//            ANNidx id = nIdx[0];
//            closest_p.x = kd_pts_list[j][id][0];
//            closest_p.y = kd_pts_list[j][id][1];
//            closest_p.z = kd_pts_list[j][id][2];

//            delete [] nIdx;
//            delete [] dist;

//            //need to convert point to XYZ for closest_p


//            if (min_dis < distance_threshold)
//            {
//                cluster.push_back(closest_p);
//                //match_nodes += 1;
//            }

//        }

//        //cout << "node found " << match_nodes << " matches " << endl;

//        //average over the clustered location p
//        if (cluster.size() > 0)
//        {
//           XYZ average_p =  mean_XYZ(cluster);

//           adjusted_neuron.listNeuron[i].x = average_p.x;
//           adjusted_neuron.listNeuron[i].y = average_p.y;
//           adjusted_neuron.listNeuron[i].z = average_p.z;
//           adjusted_neuron.listNeuron[i].r = cluster.size();
//           adjusted_neuron.listNeuron[i].type = -1;// not merged, used in the merge step

//           total_editing_dis += dist_L2(average_p, cur);

//        }
//    }
//    return total_editing_dis;
//}

//void kd_run_match_center(vector<NeuronTree> & nt_list, int max_num_iters, double cluster_distance_threshold){
//     //DEBUG
//     //  QFileInfo info(nt_list[0].file);
//     //  QString anofilename = info.path()+"/max_iter_centered.ano";
//     //  QFile file(anofilename);
//     //  if (!file.open(QFile::WriteOnly|QFile::Truncate))
//     //  {
//     //      cout <<"Error opening the file "<<"./test_adjusted.ano" << endl;
//     //      return false;
//     //  }
//     //QTextStream  stream_ano (&file);
//     //END

//       //identify nearest neighbothood, and find the average location
//      vector<NeuronTree> shift_nt_list;
//      NeuronTree nt;

//       cout<<"\n\nMatch and center:"<<endl;
//       for (int k = 0 ; k<max_num_iters; k++)
//       { // iterate multiple times, neurons will converge to center locations
//           cout<<"Iteration " <<k<<":"<<endl;
//           shift_nt_list.clear();
//           double total_editing_dis = 0.0;
//           int total_nodes = 0;

//           vector<ANNpointArray> tree_points;
//           vector<ANNkd_tree *> kd_nt_list;
//           int dim = 3;

//           //builds a vector of all kd trees
//           for (int k = 0; k < nt_list.size(); k++) //for each neuron tree
//           {
//               int n_nodes = nt_list[k].listNeuron.size();
//               //initialize points vector
//               tree_points.push_back(annAllocPts(n_nodes, dim));

//               nt_to_kdt(nt_list[k], &tree_points[k], n_nodes);
//               kd_nt_list.push_back(new ANNkd_tree(tree_points[k], n_nodes, dim));
//           }

//           //cout << "printing location of each kdtree\n";
//           //cout << &kd_nt_list[0] << endl;
//           cout << &kd_nt_list[1] << endl;
//           cout << &kd_nt_list[2] << endl;

//           cout << "printing first node in each point array" << endl;
//           cout << "x: " << tree_points[0][0][0] << " y: " << tree_points[0][0][1] << " z: " << tree_points[0][0][2] << endl;
//           cout << "x: " << tree_points[1][0][0] << " y: " << tree_points[1][0][1] << " z: " << tree_points[1][0][2] << endl;
//           cout << "x: " << tree_points[2][0][0] << " y: " << tree_points[2][0][1] << " z: " << tree_points[2][0][2] << endl;

//           for (int i = 0; i < nt_list.size(); i++) //for each neuron tree
//           {
//               total_nodes += nt_list[i].listNeuron.size();
//               nt.listNeuron.clear();
//               nt.hashNeuron.clear();
//               int idx = i;
//               double editing_dis = kd_match_and_center(nt_list, kd_nt_list, tree_points, idx, cluster_distance_threshold, nt);
//               total_editing_dis += editing_dis;
//               if (editing_dis > 0)
//               {
//                   shift_nt_list.push_back(nt);
//      //DEBUG
//                   //if ( k == (max_num_iters -1))
//                   //{
//                   //    char * filename = new char [1000];
//                   //    sprintf( filename, "%s_adjusted%d.swc", nt_list[i].file.toStdString().c_str(),k);

//                   //    export_listNeuron_2swc( nt.listNeuron, filename);
//                   //    stream_ano<< "SWCFILE="<<QString(filename)<<"\n";
//                   //    //cout<<"print to ano file: SWCFILE="<<filename<<endl;
//                   //    delete [] filename;
//                   //}
//      //END
//               }

//           }

//           nt_list.clear();//for the next iteration
//           for (int j =0 ; j < shift_nt_list.size(); j++)
//           {
//               nt_list.push_back(shift_nt_list[j]);
//           }

//           for (int m = 0; m < kd_nt_list.size(); m++)
//           {
//               delete kd_nt_list[m]; //deleting all the kd trees
//           }
//           kd_nt_list.clear();
//           tree_points.clear();

//           //converage
//           cout<<"Total matching distance is :" <<total_editing_dis<<endl;
//           cout<<"Avearge node editing distance is :" <<total_editing_dis/total_nodes<<endl;

//           if ( (total_editing_dis / total_nodes) < 0.2)
//           {
//             cout<<"Converged: stop the iterations.\n";
//             break;
//           }


//       }

//       annClose();
//     //DEBUG
//     //      file.close();
//     //END

//       // at the end of the iteration, resample nodes before merging
//       vector<NeuronTree> nt_list_resampled;
//       kd_resample_neurons(nt_list, &nt_list_resampled);
//       nt_list = nt_list_resampled;


//       //DEBUG
//       //for (int i = 0; i < nt_list.size(); i++){
//           //char * filename = new char [1000];
//           //sprintf( filename, "%s_before_merge.swc", nt_list[i].file.toStdString().c_str());
//           //export_listNeuron_2swc( nt_list[i].listNeuron, filename);
//           //delete [] filename;
//       //}
//       //END

//    annClose();
//    return;
//}
