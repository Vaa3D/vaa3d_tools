

#ifndef SWC_RADIUS
#define SWC_RADIUS 5
#endif
#include "shift_mask2D.h"

#ifndef MIN_DIST
#define MIN_DIST 0.0000001
#endif


double marker_dist(MyMarker a, MyMarker b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}
double node_dist(NeuronSWC a, NeuronSWC b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}

// Functions for getting signal around swc
bool getMarkersBetween(vector<MyMarker> &allmarkers, MyMarker m1, MyMarker m2)
{
//    printf("welcome to use getMarkersBetween\n");

    double A = m2.x - m1.x;
    double B = m2.y - m1.y;
    double C = m2.z - m1.z;
    double R = m2.radius - m1.radius;
    double D = sqrt(A*A + B*B + C*C);
    A = A/D; B = B/D; C = C/D; R = R/D;

    double ctz = A/sqrt(A*A + B*B);
    double stz = B/sqrt(A*A + B*B);

    double cty = C/sqrt(A*A + B*B + C*C);
    double sty = sqrt(A*A + B*B)/sqrt(A*A + B*B + C*C);

    double x0 = m1.x;
    double y0 = m1.y;
    double z0 = m1.z;
    double r0 = m1.radius;

    set<MyMarker> marker_set;

    for(double t = 0.0; t <= marker_dist(m1, m2); t += 1.0)
    {
        MyMarker marker;
        int cx = x0 + A*t + 0.5;
        int cy = y0 + B*t + 0.5;
        int cz = z0 + C*t + 0.5;
        int radius = r0 + R*t + 0.5;
        int radius2 = radius * radius;

        for(int k = -radius; k <= radius; k++)
        {
            for(int j = -radius; j <= radius; j++)
            {
                for(int i = -radius; i <= radius; i++)
                {
                    if(i * i + j * j + k * k > radius2) continue;
                    double x = i, y = j, z = k;
                    double x1, y1, z1;


                    //rotate_coordinate_z_clockwise(ctz, stz, x, y, z);
                    //rotate_along_y_clockwise     (cty, sty, x, y, z);
                    //rotate_coordinate_x_anticlock(ctz, stz, x, y, z);
                    //translate_to(cx, cy, cz, x, y, z);
                    y1 = y * ctz - x * stz; x1 = x * ctz + y * stz; y = y1; x = x1;
                    x1 = x * cty + z * sty; z1 = z * cty - x * sty; x = x1; z = z1;
                    z1 = z * ctz + y * stz; y1 = y * ctz - z * stz; z = z1; y = y1;
                    x += cx; y += cy; z += cz;
                    x = (int)(x+0.5);
                    y = (int)(y+0.5);
                    z = (int)(z+0.5);
                    marker_set.insert(MyMarker(x, y, z));
                }
            }
        }
    }

    allmarkers.insert(allmarkers.end(), marker_set.begin(), marker_set.end());
    return true;
}
double signal_at_markers(vector<MyMarker> allmarkers, unsigned char * data1d, long sz0, long sz1, long sz2)
{
//    printf("welcome to use signal_at_markers\n");
    double signal = 0;
    double in_block_ct = 0;
    long sz01 = sz0 * sz1;
    long total_sz = sz0 * sz1 * sz2;
    unsigned char * outimg1d=0;


    for(int i=0; i<allmarkers.size();i++){
        int x = allmarkers[i].x;
        int y = allmarkers[i].y;
        int z = allmarkers[i].z;
        int id = z*sz01+y*sz0+x;
        if(id<total_sz){
            signal += data1d[id];
            in_block_ct += 1;
        }

    }
    if(in_block_ct>0)
    {
        signal = signal / in_block_ct;
    }
    return signal;
}
double signal_between_markers(unsigned char * data1d, NeuronSWC n1, NeuronSWC n2, long sz0, long sz1, long sz2, long sz3, V3DPluginCallback2 & callback)
{
//    printf("welcome to use signal_between_markers\n");
    // To be implemented: read image as 1d image
    vector<MyMarker> allmarkers;  // allmarkers: pixels within the cylinder specified by n1&n2;
    MyMarker m1 = MyMarker(n1.x, n1.y,n1.z);
    MyMarker m2 = MyMarker(n2.x, n2.y,n2.z);
    m1.radius = SWC_RADIUS;
    m2.radius = SWC_RADIUS;
    getMarkersBetween(allmarkers, m1, m2);
    // Check image signal in the sub-segment
//    unsigned char * outimg1d = 0;
//    save_temp_image(outimg1d, data1d, allmarkers, sz0, sz1, sz2, sz3, callback);
    // Signal: average signal at allmarkers;
    double signal = signal_at_markers(allmarkers, data1d, sz0, sz1, sz2);
    return signal;
}

// Functions for visualization
template<class T> bool save_temp_image(T * &outimg1d, unsigned char * inimg1d, vector<MyMarker> allmarkers,
                                       long sz0, long sz1, long sz2, int datatype, V3DPluginCallback2 & callback)
{
//    printf("welcome to use save_temp_image\n");

    long tol_sz = sz0 * sz1 * sz2;
    long sz01 = sz0 * sz1;
    outimg1d = new T[tol_sz];
    for(long i = 0; i < tol_sz; i++) outimg1d[i] = 0;

    for(int i=0; i<allmarkers.size();i++){
        int x = allmarkers[i].x-1;
        int y = allmarkers[i].y-1;
        int z = allmarkers[i].z-1;
        int id = z*sz01+y*sz0+x;
        if(id<tol_sz){
            outimg1d[id] = inimg1d[id];
        }
    }
    V3DLONG out_sz[4] = {sz0, sz1, sz2, 1};
    simple_saveimage_wrapper(callback, "test.nrrd", outimg1d, out_sz, datatype);
    return true;
}
// Functions for processing swc files
vector<MyMarker> swc2marker(NeuronTree nt, int tip_id)
{
    vector<MyMarker> allmarkers;
    NeuronSWC node = nt.listNeuron.at(tip_id);

    while(node.pn != -1)
    {
        // End condition: when reach the end of a branch
        int pn_id = nt.hashNeuron.value(node.pn);
        if((pn_id>=nt.listNeuron.size()) || (pn_id<0)){break;}
        NeuronSWC n1 = node;
        NeuronSWC n2 = nt.listNeuron.at(pn_id);
        MyMarker m1 = MyMarker(n1.x, n1.y,n1.z);
        MyMarker m2 = MyMarker(n2.x, n2.y,n2.z);
        m1.radius = SWC_RADIUS;
        m2.radius = SWC_RADIUS;
        vector<MyMarker> cur_markers;
        getMarkersBetween(cur_markers, m1, m2);
        allmarkers.insert(allmarkers.end(), cur_markers.begin(), cur_markers.end());
        node = nt.listNeuron.at(pn_id);
    }
    return allmarkers;
}
int find_tip(NeuronTree nt, long sz0, long sz1, long sz2)
{
    // Return the node at center of the image as tip node
    MyMarker center = MyMarker((sz0-1)/2, (sz1-1)/2, (sz2-1)/2);
    for(int i=0; i<nt.listNeuron.size(); i++){
        MyMarker node=MyMarker(nt.listNeuron.at(i).x, nt.listNeuron.at(i).y, nt.listNeuron.at(i).z);
        if(marker_dist(center, node)<MIN_DIST){
            return i;
        }
    }
    printf("No tip found!\n");
    return 0;
}
QList<NeuronSWC> resample_straight_line(NeuronSWC n1, NeuronSWC n2, double step_size, int new_node_name)
{
    qDebug()<<"resample";
    // n1 is the child node, n2 is parent node
    QList<NeuronSWC> n_list;
    if(n1.pn != n2.n){
        printf("Error: resample_straight_line. Input nodes not child-parent relationship!\n");
        return n_list;
    }

    double D = node_dist(n1, n2);
    double step_x = (n2.x - n1.x)/D*step_size;
    double step_y = (n2.y - n1.y)/D*step_size;
    double step_z = (n2.z - n1.z)/D*step_size;
    int total_steps = int(D/step_size-0.5);
    qDebug()<<n1.n<<n2.n;
    NeuronSWC cur_node = n1;
    for(int i=0; i<total_steps; i++){
        cur_node.pn = new_node_name;
        qDebug()<<cur_node.n<<cur_node.parent;
        n_list.append(cur_node);
        cur_node.x += step_x;
        cur_node.y += step_y;
        cur_node.z += step_z;
        cur_node.n = new_node_name;
        new_node_name++;
    }
    cur_node.pn = -1;
    n_list.append(cur_node);
    return n_list;
}
struct add_num_info{
    QList<NeuronSWC> result;
    int add_num;
};
bool resample_two_nodes(NeuronSWC first,NeuronSWC second,add_num_info result_stru)
{
    NeuronSWC addnode1,addnode2,addnode3,addnode4;
    add_num_info resample_tree;
    bool rootor=1;
    if(first.pn==-1) rootor=0;
    if(!rootor) first=second;
    double dis=node_dist(second,third);
    resample_tree.add_num=floor(dis)-1;
    int num;
    if(3<dis<5) {
        num=1;
        get_middle_node(first,second,addnode1,0,1);
        revise_last_node_n_pn(second,num);
        result_stru.result.append(first);
        result_stru.result.append(addnode1);
        result_stru.result.append(second);
    }
    if(5<dis<10) {
        num=3;
        get_middle_node(first,second,addnode2,1,2);
        get_middle_node(first,addnode2,addnode1,0,1);
        get_middle_node(addnode2,second,addnode3,0,1);
        result_stru.result.append(first);
        result_stru.result.append(addnode1);
        result_stru.result.append(addnode2);
        result_stru.result.append(addnode3);
        revise_last_node_n_pn(second,num);
        result_stru.result.append(second);
    }
//    if(8<dis<16) {
//        num=7;
//        QList<NeuronSWC> result1,result2;
//        get_middle_node(first,second,addnode4);
//        resample_two_nodes(first,addnode4,result1);
//        result1.removeLast();
//        result_stru.result=result1;result_stru.result.insert(result_stru.result.end(),result1.begin(),result1.end());
//        resample_two_nodes(addnode4,second,result2);
//        result2.removeLast();
//        result_stru.result.insert(result_stru.result.end(),result2.begin(),result2.end());
//        revise_last_node_n_pn(second,num);
//        result_stru.result.append(second);
//    }
    result_stru.add_num=num;
}
void get_middle_node(NeuronSWC first,NeuronSWC second,NeuronSWC middle,int addnum,int addnum1){

    middle.x=(second.x+first.x)/2;
    middle.y=(second.y+first.y)/2;
    middle.z=(second.z+first.z)/2;
    middle.type=first.type;
    middle.radius=(second.radius+first.radius)/2;
    middle.pn=first.n+addnum;
    middle.n=first.n+addnum1;

}
void revise_last_node_n_pn(NeuronSWC input_last_end,int num){
    NeuronSWC s;
    s.x=input_last_end.x;
    s.y=input_last_end.y;
    s.z=input_last_end.z;
    s.type=input_last_end.type;
    s.radius=input_last_end.radius;
    s.pn=input_last_end.pn+num;
    s.n=input_last_end.n+num;
    input_last_end=s;
}
QList<NeuronSWC> trace_back(NeuronTree nt, int tip_id)
{
    QList<NeuronSWC> n_list;
    add_num_info new_list;
    //0.get child list
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            V3DLONG par = nt.listNeuron[i].pn;
            if (par<0) continue;
            childs[nt.hashNeuron.value(par)].push_back(i);
        }

    // 1. Get a single branch
    NeuronSWC node = nt.listNeuron.at(tip_id);
    NeuronSWC tipnode = nt.listNeuron.at(tip_id);
    NeuronSWC node1=node;
    while(node.pn != -1)
    {
        // End condition: when reach the end of a branch
        int pn_id = nt.hashNeuron.value(node.pn);
        if((pn_id>=nt.listNeuron.size()) || (pn_id<0)){break;}
        n_list.append(node);
        node = nt.listNeuron.at(pn_id);
    }
    node.pn=-1;
    n_list.append(node);//now this node have been the parent node

    // 2. Resample along this branch
    int child_id=nt.hashNeuron.value(node.n);
    QList<NeuronSWC> resample_n_list;
    while(childs[child_id].size()!=0)
    {
        //int child_id=nt.hashNeuron.value(node.n);
        NeuronSWC child_swc=nt.listNeuron.at(child_id);
        NeuronSWC child_swc1=nt.listNeuron.at(child_id);
        resample_two_nodes(node1,child_swc,new_list);
        node=child_swc1;
        node1=child_swc;
        child_id=nt.hashNeuron.value(node.n);
        resample_n_list.insert(resample_n_list.end(),new_list.result.begin(),new_list.result.end());
    }
    nt = neuronlist_2_neurontree(resample_n_list);
    return resample_n_list;
    writeSWC_file();
}



QList<NeuronSWC> proceed(NeuronTree nt, int tip_id, int n_steps)
{
    QList<NeuronSWC> n_list = nt.listNeuron;
    int new_node_name = 0;
    for(int i=0; i<n_list.size(); i++){
        if(new_node_name<=n_list.at(i).n){
            new_node_name = n_list.at(i).n + 1;
        }
    }
    NeuronSWC n1 = n_list.at(tip_id);
    NeuronSWC n2 = n_list.at(nt.hashNeuron.value(n1.pn));
    double D = node_dist(n1, n2);
    double step_x = (n1.x - n2.x)/D;
    double step_y = (n1.y - n2.y)/D;
    double step_z = (n1.z - n2.z)/D;
    int cur_pn = n1.n;
    for(int i=1; i<=n_steps; i++)
    {
        NeuronSWC new_node;
        new_node.x = n1.x + step_x*i;
        new_node.y = n1.y + step_y*i;
        new_node.z = n1.z + step_z*i;
        new_node.n = new_node_name;
        new_node_name++;
        new_node.pn = cur_pn;
        cur_pn = new_node.n;
        n_list.append(new_node);
    }
    return n_list;
}
NeuronTree neuronlist_2_neurontree(QList<NeuronSWC> neuronlist){
    NeuronTree new_tree;
    QList<NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
//    qDebug()<<"creating new neuronlist";
    for (int i = 0; i < neuronlist.size(); i++)
    {
        NeuronSWC node=neuronlist.at(i);
        NeuronSWC S;
        S.n 	= node.n;
        S.type 	= node.type;
        S.x 	= node.x;
        S.y 	= node.y;
        S.z 	= node.z;
        S.r 	= node.r;
        S.pn 	= node.pn;
        listNeuron.append(S);
        hashNeuron.insert(S.n, i);
    }

    new_tree.listNeuron = listNeuron;
    new_tree.hashNeuron = hashNeuron;
    return new_tree;
}

void get_terminal_signal(QString input_swc, QString input_image, QString output_image,V3DPluginCallback2 & callback)
{
    // 1. load input_image: to be implemented
    unsigned char * data1d = 0;
    V3DLONG img_sz[4];
    int datatype;
    simple_loadimage_wrapper(callback, qPrintable(input_image), data1d, img_sz, datatype);
    qDebug()<<img_sz[0]<<img_sz[1]<<img_sz[2]<<img_sz[3]<<img_sz[0]*img_sz[1]*img_sz[2];

    // 2. load swc file
    NeuronTree nt=readSWC_file(input_swc);
    // resample: to be implemented
    // Find tip node: tip node must be at the center of image
    int tip_id = find_tip(nt, img_sz[0], img_sz[1], img_sz[2]);
    QList<NeuronSWC> n_list = trace_back(nt, tip_id);
    nt.deepCopy(neuronlist_2_neurontree(n_list));
    n_list = proceed(nt, 0, 25);
    nt.deepCopy(neuronlist_2_neurontree(n_list));
    writeSWC_file("test.swc", nt);
    tip_id = nt.listNeuron.size()-1;

    // Optional: visualize the image signals along SWC
    vector<MyMarker> allmarkers = swc2marker(nt, tip_id);
    unsigned char * outimg1d;
    save_temp_image(outimg1d, data1d, allmarkers, img_sz[0], img_sz[1], img_sz[2], img_sz[3], callback);


    // Get signal at each sub-segment
    NeuronSWC node = nt.listNeuron.at(tip_id);
    while(node.pn != -1)
    {
        // End condition: when reach the end of a branch
        int pn_id = nt.hashNeuron.value(node.pn);
        if((pn_id>=nt.listNeuron.size()) || (pn_id<0)){break;}
        double cur_sig = signal_between_markers(data1d, node, nt.listNeuron.at(pn_id),
                                                img_sz[0], img_sz[1], img_sz[2], img_sz[3], callback);
        qDebug()<<cur_sig;
        node = nt.listNeuron.at(pn_id);

    }
    return;
}
