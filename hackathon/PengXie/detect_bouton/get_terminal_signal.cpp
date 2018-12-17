#include "get_terminal_signal.h"
#ifndef SWC_RADIUS
#define SWC_RADIUS 5
#endif

#ifndef BX
#define BX 100
#endif
#ifndef BY
#define BY 100
#endif
#ifndef BZ
#define BZ 20
#endif

#ifndef RX
#define RX 0.2
#endif
#ifndef RY
#define RY 0.2
#endif
#ifndef RZ
#define RZ 1
#endif

#ifndef MIN_DIST
#define MIN_DIST 0.001
#endif

NeuronSWC adjust_size(NeuronSWC node, double ratio_x=RX, double ratio_y=RY, double ratio_z=RZ){
    node.x *= ratio_x;
    node.y *= ratio_y;
    node.z *= ratio_z;
    return node;
}
NeuronSWC readjust_size(NeuronSWC node, double ratio_x=RX, double ratio_y=RY, double ratio_z=RZ){
    node.x /= ratio_x;
    node.y /= ratio_y;
    node.z /= ratio_z;
    return node;
}
MyMarker adjust_size(MyMarker marker, double ratio_x=RX, double ratio_y=RY, double ratio_z=RZ){
    marker.x *= ratio_x;
    marker.y *= ratio_y;
    marker.z *= ratio_z;
    return marker;
}
MyMarker readjust_size(MyMarker marker, double ratio_x=RX, double ratio_y=RY, double ratio_z=RZ){
    marker.x /= ratio_x;
    marker.y /= ratio_y;
    marker.z /= ratio_z;
    return marker;
}
double marker_dist(MyMarker a, MyMarker b, bool scale)
{
    if(scale){
        a.x = a.x*RX;
        a.y = a.y*RY;
        a.z = a.z*RZ;
        b.x = b.x*RX;
        b.y = b.y*RY;
        b.z = b.z*RZ;
    }
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}
double node_dist(NeuronSWC a, NeuronSWC b, bool scale)
{
    if(scale){
        a.x = a.x*RX;
        a.y = a.y*RY;
        a.z = a.z*RZ;
        b.x = b.x*RX;
        b.y = b.y*RY;
        b.z = b.z*RZ;
    }
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}

// Functions for getting signal around swc
bool getMarkersBetween(vector<MyMarker> &allmarkers, MyMarker m1, MyMarker m2)
{
//    printf("welcome to use getMarkersBetween\n");
//    qDebug()<<m1.x<<m1.y<<m1.z;
//    qDebug()<<m2.x<<m2.y<<m2.z;

    double ratio_x = 1;
    double ratio_y = 1;
    double ratio_z = 5;
    m1 = adjust_size(m1, ratio_x, ratio_y, ratio_z);
    m2 = adjust_size(m2, ratio_x, ratio_y, ratio_z);

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

    for(double t = 0.0; t <= marker_dist(m1, m2, false); t += 1.0)
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

                    y1 = y * ctz - x * stz; x1 = x * ctz + y * stz; y = y1; x = x1;
                    x1 = x * cty + z * sty; z1 = z * cty - x * sty; x = x1; z = z1;
                    z1 = z * ctz + y * stz; y1 = y * ctz - z * stz; z = z1; y = y1;
                    x += cx; y += cy; z += cz;
                    marker = MyMarker(x, y, z);
                    marker = readjust_size(marker, ratio_x, ratio_y, ratio_z);
                    marker.x = (int)(marker.x+0.5);
                    marker.y = (int)(marker.y+0.5);
                    marker.z = (int)(marker.z+0.5);
                    marker_set.insert(marker);
                }
            }
        }
    }

    allmarkers.insert(allmarkers.end(), marker_set.begin(), marker_set.end());
//    qDebug()<<"Done getting markers";
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
        if((x<0) || (x>=sz0) || (y<0) || (y>=sz1) || (z<0) || (z>=sz2)){continue;} // out of image border
        int id = z*sz01+y*sz0+x;
//        cout<<id<<"\t";
        if(id<total_sz){
            signal += data1d[id];
            in_block_ct += 1;
        }
//        cout<<"pass"<<endl;
    }
//    qDebug()<<"Done collecting pixels";
    if(in_block_ct>0)
    {
        signal = signal / in_block_ct;
    }
    return signal;
}
double signal_between_markers(unsigned char * data1d, NeuronSWC n1, NeuronSWC n2, long sz0, long sz1, long sz2, long sz3, V3DPluginCallback2 & callback)
{
//    qDebug()<<"welcome to use signal_between_markers\n";
    // To be implemented: read image as 1d image
    vector<MyMarker> allmarkers;  // allmarkers: pixels within the cylinder specified by n1&n2;
    MyMarker m1 = MyMarker(n1.x, n1.y,n1.z);
    MyMarker m2 = MyMarker(n2.x, n2.y,n2.z);
    m1.radius = SWC_RADIUS;
    m2.radius = SWC_RADIUS;
    getMarkersBetween(allmarkers, m1, m2);
    // Signal: average signal at allmarkers;
//    qDebug()<<"size of marker list:"<<allmarkers.size();
    double signal = signal_at_markers(allmarkers, data1d, sz0, sz1, sz2);
    return signal;
}

// Functions for visualization
template<class T> bool save_temp_image(T * &outimg1d, unsigned char * inimg1d, vector<MyMarker> allmarkers, QString output_file,
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
        if((x<0) || (y<0) || (z<0)){continue;}
        int id = z*sz01+y*sz0+x;
        if(id<tol_sz){
            outimg1d[id] = inimg1d[id];
        }
    }
    V3DLONG out_sz[4] = {sz0, sz1, sz2, 1};
    const char * char_file = Qstring_to_char(output_file);
//    cout<<"before saveimage:"<<char_file<<"end"<<endl;
    simple_saveimage_wrapper(callback, char_file, outimg1d, out_sz, datatype);
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
    MyMarker center = MyMarker(BX/2, BY/2, BZ/2);
    for(int i=0; i<nt.listNeuron.size(); i++){
        MyMarker node=MyMarker(nt.listNeuron.at(i).x, nt.listNeuron.at(i).y, nt.listNeuron.at(i).z);
        if(marker_dist(center, node, true)<MIN_DIST){
            return i;
        }
    }
    printf("No tip found!\n");
    return -1;
}
QList<bool> find_branch(NeuronTree nt){
    QList<NeuronSWC> n_list = nt.listNeuron;
    QHash<int, int> n_hash = nt.hashNeuron;
    QList<bool> is_branch;
    QList<int> n_child;
    QList<int> n_parent;
    for(int i=0; i<n_list.size(); i++){
        n_child.append(0);
        n_parent.append(0);
    }
    for(int i=0; i<n_list.size(); i++)
    {
        NeuronSWC node = n_list.at(i);
        int pn_id = n_hash.value(node.pn, -1);
//        qDebug()<<i<<node.n<<node.pn<<pn_id;
        if(pn_id>=0){
            n_child[pn_id]++;
            n_parent[i]++;
        }
    }
    for(int i=0; i<n_list.size(); i++){
        is_branch.append((n_child.at(i)+n_parent.at(i))>2);
//        qDebug()<<n_child.at(i)<<n_parent.at(i)<<n_list.at(i).n<<n_list.at(i).x<<n_list.at(i).y<<n_list.at(i).z;
//        if((n_child.at(i)+n_parent.at(i))>2){
//            qDebug()<<"reached branch"<<n_list.at(i).n<<n_list.at(i).x<<n_list.at(i).y<<n_list.at(i).z<<n_parent.at(i);
//        }
    }
    return is_branch;
}
double tip_to_branch(NeuronTree nt, int tip_id)
{
    QList<NeuronSWC> n_list = nt.listNeuron;
    QHash<int, int> n_hash = nt.hashNeuron;
    QList<bool> is_branch = find_branch(nt);
    if((tip_id<0) || (tip_id>=nt.listNeuron.size()))
    {
        return -1;
    }
    double dist = 0;
    NeuronSWC node = nt.listNeuron.at(tip_id);
    while(node.pn != -1)
    {
        int pn_id = n_hash.value(node.pn);
        if((pn_id<0) || (pn_id>=nt.listNeuron.size())){break;}
        NeuronSWC pn_node = n_list.at(pn_id);
        dist += node_dist(node, pn_node, true);
        if(is_branch.at(pn_id)){
            qDebug()<<"reached branch"<<pn_node.n<<pn_node.x<<pn_node.y<<pn_node.z;
            break;
        }  // End condition: meet branch point
        node = pn_node;
    }
    return dist;
}

QList<NeuronSWC> resample_straight_line(NeuronSWC n1, NeuronSWC n2, double step_size, int new_node_name)
{
//    qDebug()<<"resample";
    // Adjust step by voxel size
    n1 = adjust_size(n1);
    n2 = adjust_size(n2);

    // n1 is the child node, n2 is parent node
    QList<NeuronSWC> n_list;
    if(n1.pn != n2.n){
        printf("Error: resample_straight_line. Input nodes not child-parent relationship!\n");
        return n_list;
    }

    double D = node_dist(n1, n2, false);  // Unit: micron
    double step_x = (n2.x - n1.x)/D*step_size;
    double step_y = (n2.y - n1.y)/D*step_size;
    double step_z = (n2.z - n1.z)/D*step_size;
    int total_steps = int(D/step_size-0.5);
//    qDebug()<<n1.n<<n2.n;
    NeuronSWC cur_node = n1;
    for(int i=0; i<total_steps; i++){
        cur_node.pn = new_node_name;
//        qDebug()<<cur_node.n<<cur_node.parent;
        n_list.append(readjust_size(cur_node));
        cur_node.x += step_x;
        cur_node.y += step_y;
        cur_node.z += step_z;
        cur_node.n = new_node_name;
        new_node_name++;
    }
    cur_node.pn = -1;
    n_list.append(readjust_size(cur_node));
    return n_list;
}
QList<NeuronSWC> trace_back(NeuronTree nt, int tip_id)
{
    QList<NeuronSWC> n_list;
    double step_size = 0.2; // Unit: micron
    NeuronSWC node = nt.listNeuron.at(tip_id);

    // 1. Get a single branch
    while(node.pn != -1)
    {
        // End condition: when reach the end of a branch
        int pn_id = nt.hashNeuron.value(node.pn);
        if((pn_id>=nt.listNeuron.size()) || (pn_id<0)){break;}
        n_list.append(node);
        node = nt.listNeuron.at(pn_id);
    }
    node.pn=-1;
    n_list.append(node);

    // 2. Resample along this branch
    QList<NeuronSWC> resample_n_list;
    // name of newly added nodes
    int new_node_name = 0;
    for(int i=0; i<n_list.size(); i++){
        if(new_node_name<=n_list.at(i).n){
            new_node_name = n_list.at(i).n + 1;
        }
    }
    node = n_list.at(0);
    nt = neuronlist_2_neurontree(n_list);

    while(node.pn != -1)
    {
        int pn_id = nt.hashNeuron.value(node.pn);
//        qDebug()<<pn_id;
        QList<NeuronSWC> cur_list = resample_straight_line(node, n_list.at(pn_id), step_size, new_node_name);
        new_node_name = (cur_list.size()>1)? (cur_list.last().n+1):new_node_name;
        if(resample_n_list.empty()){
            resample_n_list = cur_list;
        }
        else{
            resample_n_list.last().pn = cur_list.first().n;
            resample_n_list.append(cur_list);
        }
        node = n_list.at(pn_id);
    }
    return resample_n_list;
}
QList<NeuronSWC> proceed(NeuronTree nt, int tip_id, int n_steps)
{
    QList<NeuronSWC> n_list = nt.listNeuron;
    if(n_list.empty()){return n_list;}
    int new_node_name = 0;
    for(int i=0; i<n_list.size(); i++){
        if(new_node_name<=n_list.at(i).n){
            new_node_name = n_list.at(i).n + 1;
        }
    }

    NeuronSWC n1 = n_list.at(tip_id);
    if(nt.hashNeuron.value(n1.pn)<0){  // Tip does not have a valid parent node

        return n_list;
    }
    NeuronSWC n2 = n_list.at(nt.hashNeuron.value(n1.pn));
    n1 = adjust_size(n1);
    n2 = adjust_size(n2);
    double D = node_dist(n1, n2, false);  // Unit: micron
    double step_size = 0.2;   // Unit: micron
    double step_x = (n1.x - n2.x)/D * step_size;
    double step_y = (n1.y - n2.y)/D * step_size;
    double step_z = (n1.z - n2.z)/D * step_size;
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
        n_list.append(readjust_size(new_node));
//        n_list.append(new_node);
    }
    return n_list;
}

QList<double> get_terminal_signal(QString input_swc, QString input_image, V3DPluginCallback2 & callback)
{

    qDebug()<<"welcome to use get_terminal_signal";

    int meta_len = 2;
    int upstream_len = 50;
    int downstream_len = 25;
    int result_len = meta_len + upstream_len + downstream_len;


    // result:
    // <0> Type of tip;
    // <1> Distance to 1st branch;
    // <meta_len : (meta_len + upstream_len - 1)> Upstream signal;
    // <(meta_len + upstream_len - 1) : end> Downstream signal;
    QList<double> result;
    for(int i=0; i<result_len; i++){result.append(0);}

    // 1. load input_image:
    qDebug()<<"Load image";
    unsigned char * data1d = 0;
    V3DLONG img_sz[4];
    int datatype;
    simple_loadimage_wrapper(callback, qPrintable(input_image), data1d, img_sz, datatype);
    qDebug()<<img_sz[0]<<img_sz[1]<<img_sz[2]<<img_sz[3]<<img_sz[0]*img_sz[1]*img_sz[2]<<data1d[img_sz[0]*img_sz[1]*img_sz[2]-1];

    // 2. load swc file
    qDebug()<<"Load swc";
    NeuronTree nt=readSWC_file(input_swc);
    if(nt.listNeuron.size()<10){return result;}

    // 2.1 Find tip node: tip node must be at the center of image
    int tip_id = find_tip(nt, img_sz[0], img_sz[1], img_sz[2]);
    if(tip_id<0){return result;}  // Return if no tip found.
    double dist_branch_point = tip_to_branch(nt, tip_id);
    int tip_type = nt.listNeuron.at(tip_id).type;

    // 2.2 Move upstream and resample.
    qDebug()<<"trace_back";
    QList<NeuronSWC> n_list = trace_back(nt, tip_id);
    if(n_list.size()<1){return result;}
    nt.deepCopy(neuronlist_2_neurontree(n_list));

    // 2.3 Extend downstream.
    qDebug()<<"proceed";
    n_list = proceed(nt, 0, downstream_len);
    qDebug()<<n_list.size() - nt.listNeuron.size();
    if((n_list.size() - nt.listNeuron.size())!= downstream_len)  // Return if extension is not successful
    {
        return result;
    }
    nt.deepCopy(neuronlist_2_neurontree(n_list));
    tip_id = nt.listNeuron.size()-1;  // New tip position

    // 3. Get signal at each sub-segment
    qDebug()<<"Get signal";
    NeuronSWC node = nt.listNeuron.at(tip_id);
    result[0] = tip_type;
    result[1] = dist_branch_point;
    int ct = result_len - 1;
    while(node.pn != -1)
    {
        // End condition: when reach the end of a branch
        int pn_id = nt.hashNeuron.value(node.pn);
        if((pn_id>=nt.listNeuron.size()) || (pn_id<0)){break;}
//        qDebug()<<ct;
        double cur_sig = signal_between_markers(data1d, node, nt.listNeuron.at(pn_id),
                                                img_sz[0], img_sz[1], img_sz[2], img_sz[3], callback);
//        qDebug()<<cur_sig;
        result[ct]=cur_sig;
        ct--;
//        if(ct==(downstream_len+upstream_len)){break;}
        if(ct<meta_len){break;}
        node = nt.listNeuron.at(pn_id);
    }

    // Optional: visualize the image signals along SWC
    bool temp_save = true;
    QString temp_dir = "mask/";
    QString temp_swc = temp_dir + input_swc.right(input_swc.size()-input_swc.lastIndexOf("/")-1);
    temp_swc.replace(".swc", ".mask.swc");
    QString temp_img = temp_dir + input_image.right(input_image.size()-input_image.lastIndexOf("/")-1);
    temp_img.replace(".nrrd", ".mask.nrrd");
    if(temp_save){
        qDebug()<<"save temp swc"<<temp_swc;
        writeSWC_file(temp_swc, nt);
        qDebug()<<"save temp nrrd"<<temp_img;
        vector<MyMarker> allmarkers = swc2marker(nt, tip_id);
        unsigned char * outimg1d;
        save_temp_image(outimg1d, data1d, allmarkers, temp_img,
                        img_sz[0], img_sz[1], img_sz[2], img_sz[3], callback);
    }

    return result;
}

void get_all_terminal_signal(QString input_folder, QString out_file, V3DPluginCallback2 & callback)
{
    input_folder = input_folder.endsWith("/")? input_folder:(input_folder+"/");
    QList<QString> files;
    read_directory(input_folder, files);
    int ct=0;
    FILE * fp=0;
    fp = fopen((char *)qPrintable(out_file), "wt");
    if(!fp){printf("Error: cannot write to file", qPrintable(out_file)); return;}
    for(int i=0; i<files.size(); i++){
        QString file = files.at(i);
        if(file.endsWith(".swc"))
        {
            QString cell_name = file.left(file.indexOf("."));
            QString input_swc = input_folder+cell_name+".swc";
            QString input_img = input_folder+cell_name+".nrrd";
            qDebug()<<input_swc<<input_img;
//            if(exists_file(Qstring_to_char(input_img))) // To be implemented
            if(true)
            {
                QList<double> cur_sigvector = get_terminal_signal(input_swc, input_img, callback);
                if(cur_sigvector.size()>0){
                    fprintf(fp, qPrintable(cell_name));
                    for(int j=0; j<cur_sigvector.size(); j++){
                        fprintf(fp, "\t");
                        fprintf(fp, qPrintable(QString::number(cur_sigvector.at(j))));
                    }
                    fprintf(fp, "\n");
                    ct++;
//                    if(ct>10000){break;}
                }
            }
        }
    }
    fclose(fp);
    return;
}
