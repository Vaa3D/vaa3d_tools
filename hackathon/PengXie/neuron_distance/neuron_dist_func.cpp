/* neuron_dist_func.cpp
 * The plugin to calculate distance between two neurons. Distance is defined as the average distance among all nearest pairs in two neurons.
 * 2012-05-04 : by Yinan Wan
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "neuron_dist_func.h"
#include "neuron_dist_gui.h"
#include "neuron_sim_scores.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
//#include "../swc_to_maskimage/filter_dialog.h"
//#include "utilities.h"
//#include "pre_processing_main.h"
//#include "NeuronStructExplorer.h"
#include <vector>
#include <iostream>
#include <fstream>


using namespace std;

/* Functions for testing overlapping volumns */
/* Basic functions */
cuboid check_cuboid_order(cuboid cb){
    if(cb.min_corner.x>cb.max_corner.x){
        double tp = cb.min_corner.x;
        cb.min_corner.x = cb.max_corner.x;
        cb.max_corner.x = tp;
    }
    if(cb.min_corner.y>cb.max_corner.y){
        double tp = cb.min_corner.y;
        cb.min_corner.y = cb.max_corner.y;
        cb.max_corner.y = tp;
    }
    if(cb.min_corner.z>cb.max_corner.z){
        double tp = cb.min_corner.z;
        cb.min_corner.z = cb.max_corner.z;
        cb.max_corner.z = tp;
    }
    return cb;
}

myline check_line_order(myline ln){
    if(ln.a>ln.b){
        double tp = ln.a;
        ln.a = ln.b;
        ln.b = tp;
    }
    return ln;
}
double max(double x, double y){
    double res = x > y? x:y;
    return res;
}
double min(double x, double y){
    double res = x < y? x:y;
    return res;
}
bool line_intersect(myline l1, myline l2){
    l1 = check_line_order(l1);
    l2 = check_line_order(l2);
    double overlap = max(0, min(l1.b, l2.b)-max(l1.a, l2.a));
    if(overlap>0){
        return 1;
    }
    return 0;
}
bool cuboid_intersect(cuboid cb1, cuboid cb2){
    myline l1, l2;
    l1.a = cb1.min_corner.x;
    l1.b = cb1.max_corner.x;
    l2.a = cb2.min_corner.x;
    l2.b = cb2.max_corner.x;
    if(!(line_intersect(l1, l2))){
        return false;
    }
    l1.a = cb1.min_corner.y;
    l1.b = cb1.max_corner.y;
    l2.a = cb2.min_corner.y;
    l2.b = cb2.max_corner.y;
    if(!(line_intersect(l1, l2))){
        return false;
    }
    l1.a = cb1.min_corner.z;
    l1.b = cb1.max_corner.z;
    l2.a = cb2.min_corner.z;
    l2.b = cb2.max_corner.z;
    if(!(line_intersect(l1, l2))){
        return false;
    }
    return true;
}
/* Overlaps of two neurons */
cuboid nt_to_cuboid(NeuronTree nt){
    cuboid cb=cuboid();
    cb.min_corner.x = nt.listNeuron.at(0).x;
    cb.min_corner.y = nt.listNeuron.at(0).y;
    cb.min_corner.z = nt.listNeuron.at(0).z;
    cb.max_corner.x = nt.listNeuron.at(0).x;
    cb.max_corner.y = nt.listNeuron.at(0).y;
    cb.max_corner.z = nt.listNeuron.at(0).z;
    for(int i=1; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        cb.min_corner.x = min(cb.min_corner.x, node.x);
        cb.min_corner.y = min(cb.min_corner.y, node.y);
        cb.min_corner.z = min(cb.min_corner.z, node.z);
        cb.max_corner.x = max(cb.max_corner.x, node.x);
        cb.max_corner.y = max(cb.max_corner.y, node.y);
        cb.max_corner.z = max(cb.max_corner.z, node.z);
    }
    return cb;
}
bool cuboid_overlap_nt(NeuronTree nt1, NeuronTree nt2){
    return cuboid_intersect(nt_to_cuboid(nt1), nt_to_cuboid(nt2));
}

double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2, double xscale=XSCALE, double yscale=YSCALE, double zscale=ZSCALE)
{
    double xx = (s1.x-s2.x)*xscale;
    double yy = (s1.y-s2.y)*yscale;
    double zz = (s1.z-s2.z)*zscale;
    return sqrt(xx*xx+yy*yy+zz*zz);
};


CellAPO get_marker(NeuronSWC node, double vol, double color_r, double color_g, double color_b){
    CellAPO marker;
    marker.x = node.x;
    marker.y = node.y;
    marker.z = node.z;
    marker.volsize = vol;
    marker.color.r = color_r;
    marker.color.g = color_g;
    marker.color.b = color_b;
    return marker;
}
double get_total_length(NeuronTree nt){
    double total_length=0;
    for(int i=0;i<nt.listNeuron.size(); i++){
        NeuronSWC node=nt.listNeuron.at(i);
        if(node.pn != -1){
            NeuronSWC parent=nt.listNeuron.at(nt.hashNeuron.value(node.pn));
            total_length += computeDist2(node, parent);
        }
    }
    return total_length;
}

double overlap_directional_swc_1_2(NeuronTree nt1, NeuronTree nt2, double overlap_thres, QString savemarkerfile)
{
    if (!&nt1 || !&nt2) return -1;

    double overlap_length=0;
    QList<CellAPO> markers;

    /* Test overlap at volumn level */

    if(!cuboid_overlap_nt(nt1, nt2)){
        return 0;
    }

    // Traversal p1 slice by slice, if corresponding slice found in p2. If yes, find the overlapping nodes.

    for (int i=0;i<nt1.listNeuron.size();i++)
    {
        double add_to_overlap_length = 0;
        //first find the two ends of a line seg
        NeuronSWC * tp1 = (NeuronSWC *)(&(nt1.listNeuron.at(i)));
        if (tp1->pn < 0)
            continue;
        NeuronSWC * tp2 = (NeuronSWC *)(&(nt1.listNeuron.at(nt1.hashNeuron.value(tp1->pn)))); //use hash table

        //now produce a series of points for the line seg
        double len=computeDist2(*tp1, *tp2);
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
        for (int j=0;j<N;j++)
        {
            XYZ curpt(tp1->x + ptdiff.x*j, tp1->y + ptdiff.y*j, tp1->z + ptdiff.z*j);
            double cur_d = dist_pt_to_swc(curpt, &nt2);

            if (cur_d<overlap_thres)
            {
                if(add_to_overlap_length==0){
                    overlap_length += len;
                    add_to_overlap_length = len;
                }
                CellAPO marker = get_marker(nt1.listNeuron.at(i), 100, 255,255,255);
                markers.append(marker);
            }
        }

    }
    //qDebug() << "end directional neuronal distance computing";
    if((savemarkerfile.length()>0) && (markers.size()>0)){
        writeAPO_file(qPrintable(savemarkerfile), markers);
    }
    return overlap_length;
}

/* When Xianji's library is ready, complete the following function to improve efficiency */

//double overlap_directional_swc_1_2(NeuronTree nt1, NeuronTree nt2, double overlap_thres, QString savemarkerfile)
//{
//    if (!&nt1 || !&nt2) return -1;
//    V3DLONG N1 = nt1.listNeuron.size(), N2 = nt2.listNeuron.size();

//    double overlap_length=0;
//    QList<CellAPO> markers;

//    // Traversal p1 slice by slice, if corresponding slice found in p2. If yes, find the overlapping nodes.
//    profiledTree pt1 = profiledTree(nt1);
//    qDebug()<<pt1.tree.listNeuron.size();

//    for (i=0;i<p1->listNeuron.size();i++)
//    {
//        //first find the two ends of a line seg
//        tp1 = (NeuronSWC *)(&(p1->listNeuron.at(i)));
//        if (tp1->pn < 0)
//            continue;
//        tp2 = (NeuronSWC *)(&(p1->listNeuron.at(h1.value(tp1->pn)))); //use hash table
//        //qDebug() << "i="<< i << " pn="<<tp1->pn - 1;

//        //now produce a series of points for the line seg
//        double len=dist_L2(XYZ(tp1->x,tp1->y,tp1->z), XYZ(tp2->x,tp2->y,tp2->z));
//        int N = int(1+len+0.5);
//        XYZ ptdiff;
//        if (N<=1)
//        {
//            //qDebug() << "detect one very short segment, len=" << len;
//            ptdiff = XYZ(0,0,0);
//        }
//        else
//        {
//            double N1=1.0/(N-1);
//            ptdiff = XYZ(N1,N1,N1) * XYZ(tp2->x-tp1->x, tp2->y-tp1->y, tp2->z-tp1->z);
//        }
//        //qDebug() << "N="<<N << "len=" <<len << "xd="<<ptdiff.x << " yd=" << ptdiff.y << " zd=" << ptdiff.z << " ";
//        for (j=0;j<N;j++)
//        {
//            XYZ curpt(tp1->x + ptdiff.x*j, tp1->y + ptdiff.y*j, tp1->z + ptdiff.z*j);
//            double cur_d = dist_pt_to_swc(curpt, p2);
//            sum1 += cur_d;
//            nseg1++;

//            if (mindist<0) //use <0 as a condition to check if maxdist has been set
//                mindist = cur_d;
//            else
//            {
//                if (mindist>cur_d)
//                    mindist = cur_d;
//            }

//            if (cur_d<overlap_thres)
//            {
//                sum1small += cur_d;
//                nseg1small++;
//                CellAPO marker = get_marker(p1->listNeuron.at(i), 100, 255,255,255);
//                markers.append(marker);
//                //qDebug() << "(" << cur_d << ", " << nseg1big << ")";
//            }
//        }

//    }
//    //qDebug() << "end directional neuronal distance computing";
//    if(savemarkerfile.length()>0){
//        writeAPO_file(qPrintable(savemarkerfile), markers);
//    }
//    return overlap_length;
//}

NeuronOverlapSimple overlapping_structure(const NeuronTree *p1, const NeuronTree *p2, bool bmenu, double overlap_thres, QString savemarkerfile)
{
    NeuronOverlapSimple ss;

//    //===
//    if(bmenu)
//    {
//        bool ok1;

//#if not defined(USE_Qt5_VS2015_Win7_81) && not defined(USE_Qt5_VS2015_Win10_10_14393)
//        V3DLONG d_thres_new = QInputDialog::getInteger(0, "change the default distance threshold",
//                                                       "The visible-spatial-distance threshold of two neurons: ", d_thres, 2, 20, 1, &ok1);
//#else
//        V3DLONG d_thres_new = QInputDialog::getInt(0, "change the default distance threshold",
//                                                       "The visible-spatial-distance threshold of two neurons: ", d_thres, 2, 20, 1, &ok1);
//#endif
//        if (ok1)
//        {
//            d_thres = d_thres_new;
//        }
//    }else
//        d_thres = d_thres_updated;
//    //===


    if (!p1 || !p2) return ss;
    V3DLONG p1sz = p1->listNeuron.size(), p2sz = p2->listNeuron.size();
    if (p1sz<2 || p2sz<2) {
        cout<<"Input neurons has too few nodes, distance calculation requires at least two nodes." <<endl;
        return ss; //requires two nodes at least
     }

    ss.nt1_length = get_total_length(*p1);
    ss.nt2_length = get_total_length(*p2);
    ss.overlap_length = overlap_directional_swc_1_2(*p1, *p2, overlap_thres, savemarkerfile);

    return ss;
}

bool neuron_overlap_io(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to neuron_dist_io"<<endl;
    vector<char*> * inlist = (vector<char*>*)(input.at(0).p);
    if (inlist->size()!=2)
    {
        cerr<<"plese specify only 2 input neurons for distance computing"<<endl;
        return false;
    }

    vector<char*> inparas;
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    double  d_thres = (inparas.size() >= 1) ? atof(inparas[0]) : 2;
    QString savemarkerfile = (inparas.size() >= 2) ? QString(inparas[1]) : QString();
    qDebug()<<savemarkerfile<<endl;
    bool bmenu = 0;
	QString name_nt1(inlist->at(0));
	QString name_nt2(inlist->at(1));
	NeuronTree nt1 = readSWC_file(name_nt1);
	NeuronTree nt2 = readSWC_file(name_nt2);
//    double sum12, sum21;
//    V3DLONG nseg1, nseg2;
//    double sum12small, sum21small;
//    double mindist12 = -1, mindist21 = -1; //set as some big numbers
//    V3DLONG nseg12small, nseg21bsmall;
//    overlap_directional_swc_1_2(nseg1, nseg12small, sum12small, &nt1, &nt2, mindist12, d_thres);

    NeuronOverlapSimple ss = overlapping_structure(&nt1,&nt2,0,d_thres, savemarkerfile);
    ss.nt1 = name_nt1;
    ss.nt2 = name_nt2;

    cout<<"\nOverlap between neuron 1 "<<qPrintable(ss.nt1)<<" and neuron 2 "<<qPrintable(ss.nt2)<<" is: "<<endl;
    cout<<"Total overlapping length:\t" << ss.overlap_length<<endl;
    cout<<"Total length (neuron 1):\t" << ss.nt1_length<<endl;
    cout<<"Total length (neuron 2):\t" << ss.nt2_length<<endl;

    if (output.size() == 1)
//    if ((output.size() == 1) && (ss.overlap_length>0))
    {
        char *outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

        ofstream myfile;
        myfile.open (outimg_file);
        myfile << "input1\t";
        myfile << name_nt1.toStdString().c_str()  ;
        myfile << "\nintput2\t";
        myfile << name_nt2.toStdString().c_str();
        myfile << "\nTotal_length(input1)\t";
        myfile << ss.nt1_length;
        myfile << "\nTotal_length(input2)\t";
        myfile << ss.nt2_length;
        myfile << "\nTotal_length(overlap)\t";
        myfile << ss.overlap_length;
        myfile << "\n";
        myfile.close();
    }
	return true;
}

//bool neuron_dist_toolbox(const V3DPluginArgList & input, V3DPluginCallback2 & callback)
//{
//	vaa3d_neurontoolbox_paras * paras = (vaa3d_neurontoolbox_paras *)(input.at(0).p);
//	V3dR_MainWindow * win = paras->win;
//	QList<NeuronTree> * nt_list = callback.getHandleNeuronTrees_Any3DViewer(win);
//	NeuronTree nt = paras->nt;
//	if (nt_list->size()<=1)
//	{
//		v3d_msg("You should have at least 2 neurons in the current 3D Viewer");
//		return false;
//	}

//	QString message;
//	int cur_idx = 0;

//	for (V3DLONG i=0;i<nt_list->size();i++)
//	{
//		NeuronTree curr_nt = nt_list->at(i);
//		if (curr_nt.file == nt.file) {cur_idx = i; continue;}
//        NeuronDistSimple tmp_score = neuron_score_rounding_nearest_neighbor(&nt, &curr_nt,1);
//        message += QString("\nneuron #%1:\n%2\n").arg(i+1).arg(curr_nt.file);
//        message += QString("entire-structure-average (from neuron 1 to 2)= %1\n").arg(tmp_score.dist_12_allnodes);
//        message += QString("entire-structure-average (from neuron 2 to 1)= %1\n").arg(tmp_score.dist_21_allnodes);
//        message += QString("average of bi-directional entire-structure-averages = %1\n").arg(tmp_score.dist_allnodes);
//		message += QString("differen-structure-average = %1\n").arg(tmp_score.dist_apartnodes);
//		message += QString("percent of different-structure = %1\n").arg(tmp_score.percent_apartnodes);
//	}
//	message = QString("Distance between current neuron #%1 and\n").arg(cur_idx+1) + message;


//	v3d_msg(message);

//	return true;

//}

//int neuron_dist_mask(V3DPluginCallback2 &callback, QWidget *parent)
//{
//    SelectNeuronDlg * selectDlg = new SelectNeuronDlg(parent);
//    selectDlg->exec();

//    NeuronTree nt1 = selectDlg->nt1;
//    NeuronTree nt2 = selectDlg->nt2;

//    float dilate_ratio = QInputDialog::getDouble(parent, "dilate_ratio",
//                                 "Enter dialate ratio:",
//                                 3.0, 1.0, 100.0);
//    for(V3DLONG i = 0; i <nt1.listNeuron.size(); i++)
//        nt1.listNeuron[i].r = dilate_ratio;
//    for(V3DLONG i = 0; i <nt2.listNeuron.size(); i++)
//        nt2.listNeuron[i].r = dilate_ratio;

//    double x_min,x_max,y_min,y_max,z_min,z_max;
//    x_min=x_max=y_min=y_max=z_min=z_max=0;
//    V3DLONG sx,sy,sz;
//    unsigned char *pImMask_nt1 = 0;
//    BoundNeuronCoordinates(nt1,x_min,x_max,y_min,y_max,z_min,z_max);
//    sx=x_max;
//    sy=y_max;
//    sz=z_max;
//    V3DLONG stacksz = sx*sy*sz;
//    pImMask_nt1 = new unsigned char [stacksz];
//    memset(pImMask_nt1,0,stacksz*sizeof(unsigned char));
//    ComputemaskImage(nt1, pImMask_nt1, sx, sy, sz);

//    double x_min_2,x_max_2,y_min_2,y_max_2,z_min_2,z_max_2;
//    x_min_2=x_max_2=y_min_2=y_max_2=z_min_2=z_max_2=0;
//    V3DLONG sx_2,sy_2,sz_2;

//    unsigned char *pImMask_nt2 = 0;
//    BoundNeuronCoordinates(nt2,x_min_2,x_max_2,y_min_2,y_max_2,z_min_2,z_max_2);
//    sx_2=x_max_2;
//    sy_2=y_max_2;
//    sz_2=z_max_2;
//    V3DLONG stacksz_2 = sx_2*sy_2*sz_2;
//    pImMask_nt2 = new unsigned char [stacksz_2];
//    memset(pImMask_nt2,0,stacksz_2*sizeof(unsigned char));
//    ComputemaskImage(nt2, pImMask_nt2, sx_2, sy_2, sz_2);

//    unsigned int nx=sx, ny=sy, nz=sz;
//    if(sx_2 > nx) nx = sx_2;
//    if(sy_2 > ny) ny = sy_2;
//    if(sz_2 > nz) nz = sz_2;


//    unsigned char *pData = new unsigned char[nx*ny*nz];
//    memset(pData,0,nx*ny*nz*sizeof(unsigned char));

//    for (V3DLONG k1 = 0; k1 < sz; k1++){
//       for(V3DLONG j1 = 0; j1 < sy; j1++){
//           for(V3DLONG i1 = 0; i1 < sx; i1++){
//               if(pImMask_nt1[k1*sx*sy + j1*sx +i1] == 255)
//                    pData[k1 * nx*ny + j1*nx + i1] = 127;
//           }
//       }
//    }

//    for (V3DLONG k1 = 0; k1 < sz_2; k1++){
//       for(V3DLONG j1 = 0; j1 < sy_2; j1++){
//           for(V3DLONG i1 = 0; i1 < sx_2; i1++){
//               if(pImMask_nt2[k1*sx_2*sy_2 + j1*sx_2 +i1] == 255)
//                    pData[k1 * nx*ny + j1*nx + i1] += 127;
//           }
//       }
//    }


//    V3DLONG AandB = 0, AorB = 0;
//    for(V3DLONG i = 0; i < nx*ny*nz; i++)
//    {
//        if(pData[i] > 0) AorB++;
//        if(pData[i] == 254) AandB++;

//    }

//    if(pImMask_nt1) {delete []pImMask_nt1; pImMask_nt1 = 0;}
//    if(pImMask_nt2) {delete []pImMask_nt2; pImMask_nt2 = 0;}


//    v3d_msg(QString("score is %1, %2").arg(AandB).arg(AorB));
//    Image4DSimple tmp;
//    tmp.setData(pData, nx, ny, nz, 1, V3D_UINT8);
//    v3dhandle newwin = callback.newImageWindow();
//    callback.setImage(newwin, &tmp);
//    callback.setImageName(newwin, QString("Output_swc_mask"));
//    callback.updateImageWindow(newwin);
//    callback.open3DWindow(newwin);


//    return 1;
//}

void printHelp()
{
	cout<<"\nNeuron Distance: compute the distance between two neurons. distance is defined as the average distance among all nearest point pairs. 2012-05-04 by Yinan Wan"<<endl;
    cout<<"Usage: v3d -x neuron_overlap -f overlap -i <input_filename1> <input_filename2> -p <dist_thres> <overlap_apo> -o <output_file>"<<endl;
	cout<<"Parameters:"<<endl;
	cout<<"\t-i <input_filename1> <input_filename2>: input neuron structure file (*.swc *.eswc)"<<endl;
	cout<<"Distance result will be printed on the screen\n"<<endl;
}


