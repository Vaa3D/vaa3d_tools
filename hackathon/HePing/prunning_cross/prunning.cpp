#include "prunning.h"
#include <algorithm>
#include <math.h>
#include "filter_dialog.h"

bool Comp(const int &a,const int &b)
{
    return a>b;
}

//获得segment上所有点的逆序，从尾节点到起始点
bool Branch::get_r_points_of_branch(vector<NeuronSWC> &r_points, NeuronTree &nt)
{
    NeuronSWC tmp=end_point;
    r_points.push_back(end_point);
    while(tmp.n!=head_point.n)
    {
        tmp=nt.listNeuron[nt.hashNeuron.value(tmp.parent)];
        r_points.push_back(tmp);
    }
    //r_points.push_back(head_point);
    return true;
}

//获得segment上的所有点
bool Branch::get_points_of_branch(vector<NeuronSWC> &points, NeuronTree &nt)
{
    vector<NeuronSWC> r_points;
    this->get_r_points_of_branch(r_points,nt);
    while(!r_points.empty())
    {
        NeuronSWC tmp=r_points.back();
        r_points.pop_back();
        points.push_back(tmp);
    }

    return true;
}

bool Branch::get_meanstd(QString braindir, V3DPluginCallback2 &callback, NeuronTree &nt, double &branchmean, double &branchstd, int resolution)
{
    QDir all_braindir(braindir);
    QFileInfoList list_braindir = all_braindir.entryInfoList(QStringList(),QDir::Dirs|QDir::NoDotAndDotDot);
    int dir_count = list_braindir.size();

    map<int,int> sizemap;

    for(int i=0; i<dir_count; ++i)
    {
        qDebug()<<i<<": "<<list_braindir[i].absoluteFilePath();
        QString t = list_braindir[i].baseName();
        QStringList ts =t.split('x');
        int resolution = ts[1].toInt();
        sizemap[resolution] = i;
    }

    QVector<QFileInfo> vtmp;
    map<int,int>::iterator it =sizemap.begin();

    while(it!=sizemap.end())
    {
        int index = it->second;
        vtmp.push_back(list_braindir[index]);
        it++;
    }

    list_braindir.clear();
    for(int i=0; i<dir_count; ++i)
    {
        list_braindir.push_back(vtmp[i]);
        //qDebug()<<list_braindir[i].absoluteFilePath();
    }
    //qDebug()<<dir_count-resolution<<"----------------";
    QString current_braindir = list_braindir[dir_count-resolution].absoluteFilePath();

    qDebug()<<current_braindir;

    vector<NeuronSWC> points;
    this->get_points_of_branch(points,nt);

    int times = pow(2,(resolution-1));

    for(int i=0; i<points.size(); ++i)
    {
        points[i].x /= times;
        points[i].y /= times;
        points[i].z /= times;
    }
    size_t x0 = IN, x1 = 0, y0 = IN, y1 = 0, z0 = IN, z1 = 0;
    for(V3DLONG j=0; j<points.size(); ++j)
    {
        x0 = (x0>points[j].x) ? points[j].x : x0;
        x1 = (x1<points[j].x) ? points[j].x : x1;
        y0 = (y0>points[j].y) ? points[j].y : y0;
        y1 = (y1<points[j].y) ? points[j].y : y1;
        z0 = (z0>points[j].z) ? points[j].z : z0;
        z1 = (z1<points[j].z) ? points[j].z : z1;
    }
    x0 -= 10, x1 += 10, y0 -= 10, y1 += 10, z0 -= 10, z1 += 10;

    V3DLONG sz0 = x1 - x0, sz1 = y1 - y0, sz2 = z1 - z0;

    unsigned char* pdata = 0;

    pdata = callback.getSubVolumeTeraFly(current_braindir.toStdString().c_str(),x0,x1,y0,y1,z0,z1);

    for(V3DLONG j=0; j<points.size(); ++j)
    {
        points[j].x -= x0;
        points[j].y -= y0;
        points[j].z -= z0;
    }

    NeuronTree orig_nt;
    for(int i=0; i<points.size(); ++i)
    {
        orig_nt.listNeuron.push_back(points[i]);
    }

    unsigned char* data1d_mask = 0;
    V3DLONG sz_num = sz0*sz1*sz2;
    data1d_mask = new unsigned char[sz_num];
    memset(data1d_mask,0,sz_num*sizeof(unsigned char));
    double margin = 1;
    ComputemaskImage(orig_nt,data1d_mask,sz0,sz1,sz2,margin);

    int count = 0;

    for(V3DLONG i=0; i<sz_num; ++i)
    {
        if(data1d_mask[i]!=0)
        {
            branchmean += pdata[i];
            count++;
        }
    }
    branchmean /= count;

    for(V3DLONG i=0; i<sz_num; ++i)
    {
        if(data1d_mask[i]!=0)
        {
            branchstd += (pdata[i]-branchmean)*(pdata[i]-branchmean);
        }
    }

    branchstd = sqrt(branchstd)/count;
    qDebug()<<"end mean std!";
    return true;


}

//获取segment上所有点+上采样点的intensity的均值和标准差
bool Branch::get_mean_std(QString input_path, V3DPluginCallback2 &callback, NeuronTree &nt, double &mean, double &std){
    vector<NeuronSWC> points;
    this->get_points_of_branch(points,nt);
    size_t x0=IN,x1=0,y0=IN,y1=0,z0=IN,z1=0;

//    for(int i=0;i<points.size();++i)
//    {
//        points[i].x/=2;
//        points[i].y/=2;
//        points[i].z/=2;
//    }

    for(int i=0;i<points.size();i++){//swc的三维坐标的最大值和最小值
        x0 = (x0<points[i].x)?x0:points[i].x;
        x1 = (x1>points[i].x)?x1:points[i].x;
        y0 = (y0<points[i].y)?y0:points[i].y;
        y1 = (y1>points[i].y)?y1:points[i].y;
        z0 = (z0<points[i].z)?z0:points[i].z;
        z1 = (z1>points[i].z)?z1:points[i].z;
    }
    x0-=20,x1+=20,y0-=20,y1+=20,z0-=20,z1+=20;

    qDebug()<<"x0: "<<x0<<" x1: "<<x1<<" y0: "<<y0<<" y1 "<<y1<<" z0 "<<z0<<" z1 "<<z1<<endl;

    V3DLONG sz0 = x1-x0;
    V3DLONG sz1 = y1-y0;
    V3DLONG sz2 = z1-z0;
    unsigned char* p1data = 0;
    p1data = callback.getSubVolumeTeraFly(input_path.toStdString(),x0,x1,y0,y1,z0,z1);//切包含该swc的原图块
    for(int j=0;j<points.size();j++){//相对坐标
        points[j].x-=x0;
        points[j].y-=y0;
        points[j].z-=z0;
    }

    vector<Point_xyz> v_xyz;
    int size =points.size();
    for(int k=0;k<size-1;k++){
        //坐标取整
        Point_xyz tmp0((int)(points[k].x+0.5),(int)(points[k].y+0.5),(int)(points[k].z+0.5));
        v_xyz.push_back(tmp0);
        //上采样，将两个相邻点间的中点作为新的点
        Point_xyz tmp1((int)((points[k+1].x+points[k].x)/2+0.5),(int)((points[k+1].y+points[k].y)/2+0.5),(int)((points[k+1].z+points[k].z)/2+0.5));
        v_xyz.push_back(tmp1);
    }
    //最后一个点单独放入
    Point_xyz tmp((int)(points[size-1].x+0.5),(int)(points[size-1].y+0.5),(int)(points[size-1].z+0.5));
    v_xyz.push_back(tmp);

    //坐标换算，将三维坐标转换为存储的一维坐标
    for(int i=0;i<v_xyz.size();i++){
        V3DLONG index=v_xyz[i].z*(sz0*sz1)+v_xyz[i].y*sz0+v_xyz[i].x;
        mean+=p1data[index];
    }
    mean/=v_xyz.size();

    for(int i=0;i<v_xyz.size();i++){
        V3DLONG index=v_xyz[i].z*(sz0*sz1)+v_xyz[i].y*sz0+v_xyz[i].x;
        std+=(p1data[index]-mean)*(p1data[index]-mean);
    }
    std=sqrt(std)/v_xyz.size();

    //标准差大于5的块保存下来
    if(std>5)
    {
        QString tiffile = "D://subvolum//"+QString::number(x0)+"_"+QString::number(y0)+"_"+QString::number(z0)+".tif";
        QString eswcfile = "D://subvolum//"+QString::number(x0)+"_"+QString::number(y0)+"_"+QString::number(z0)+".eswc";

        V3DLONG sz[4] = {sz0,sz1,sz2,1};
        simple_saveimage_wrapper(callback,tiffile.toStdString().c_str(),p1data,sz,1);
        NeuronTree t0;
        for(int i=0;i<points.size();++i)
        {
            t0.listNeuron.push_back(points[i]);
        }
        writeESWC_file(eswcfile,t0);
    }



    if(p1data) delete[]p1data;


    return true;

}



bool SWCTree::initialize(NeuronTree t){//分段，计算每段的头角度、尾角度，段之间的父子关系
    nt.deepCopy(t);

    NeuronSWC orig;
    V3DLONG size = nt.listNeuron.size();
    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(size,vector<V3DLONG>());//segment的所有孩子节点index

    for(V3DLONG i=0;i<size;i++){
        V3DLONG par=nt.listNeuron[i].parent;
        if(par<0){
            orig=nt.listNeuron[i];
            continue;
        }
        children[nt.hashNeuron.value(par)].push_back(i);
    }


    vector<NeuronSWC> queue;
    queue.push_back(orig);//seed

    qDebug()<<"initial branch \n";

    while(!queue.empty()){//从soma开始遍历所有的segment，存放到branchs
        NeuronSWC tmp=queue.front();
        queue.erase(queue.begin());

        for(int i=0;i<children[nt.hashNeuron.value(tmp.n)].size();i++){//从每一个点的孩子节点往下广度遍历
            Branch seg;
            seg.head_point=tmp;
            NeuronSWC par;
            NeuronSWC child=nt.listNeuron[children[nt.hashNeuron.value(seg.head_point.n)][i]];
            seg.length+=distance_two_point(tmp,child);
            Angle near_point_angle=Angle(child.x-tmp.x,child.y-tmp.y,child.z-tmp.z);
            near_point_angle.norm_angle();
            double sum_angle=0;
            bool flag=false;
            int sign=1;
            while(children[nt.hashNeuron.value(child.n)].size()==1){//没有分叉点就不断往下，如果出现角度偏差过大则分段

                par=child;

                child=nt.listNeuron[children[nt.hashNeuron.value(par.n)][0]];
                Angle tmp;
//                if(sign%min_step!=0){
//                    sign++;
//                    cos_angle = 1;
//                    tmp = near_point_angle;
//                }
//                else{
//                    tmp = Angle(child.x-par.x,child.y-par.y,child.z-par.z);
//                    tmp.norm_angle();
//                    cos_angle = near_point_angle*tmp;//两个相邻点组成的段，相邻段之间的夹角余弦值
//                }
                tmp = Angle(child.x-par.x,child.y-par.y,child.z-par.z);
                tmp.norm_angle();
                sum_angle += acos(near_point_angle*tmp);//累加角度变化
                seg.length+=distance_two_point(par,child);
                seg.end_point=child;
                seg.distance=seg.get_distance();
                qDebug()<<seg.distance<<endl;

                qDebug()<<"sum_angle/distance:"<<sum_angle/seg.distance;

                //segment出现角度偏离过大,不想分段过短，但是也不能过长，否则它的角度变化可能又不满足条件,最小长度的设置很重要如何选定一个合适的值
                if(sum_angle/seg.distance>0.098&&seg.length>6 ){
                    //seg.length+=distance_two_point(par,child);
                    flag=true;
                    break;
                }

                else{
                    //seg.length+=distance_two_point(par,child);
                    near_point_angle=tmp;
                }

            }

            if(flag==false){
                if(children[nt.hashNeuron.value(child.n)].size()>=2){//分叉点
                    queue.push_back(child);
                }
//                seg.end_point=child;
//                seg.sum_angle = sum_angle;
//                seg.distance=seg.get_distance();
//                if(seg.head_point.parent<0){
//                    seg.parent=0;

//                }
//                else{
//                    if(seg.head_point.n==branchs.end()->head_point.n){
//                        seg.parent=branchs.end()->parent;
//                    }
//                    else{
//                        seg.parent=branchs.end();
//                    }
//                }
//                branchs.push_back(seg);

            }
            else{//角度变化过大而被截断，可能存在因截断而导致只有两个点组成的段存在
                queue.push_back(child);

            }

            seg.end_point=child;
            seg.distance=seg.get_distance();
            seg.sum_angle = sum_angle;
            branchs.push_back(seg);


        }
    }
    qDebug()<<"end branch\n";

    qDebug()<<"branch size: "<<branchs.size();

    for(int i=0;i<branchs.size();i++){
        //qDebug()<<"____________________  "<<i;
        vector<NeuronSWC> points;

        branchs[i].get_points_of_branch(points,nt);//获取该段的所有点
        //qDebug()<<"points size: "<<points.size();

        double length=0;
        NeuronSWC par1=points[0];

        for(int j=1;j<points.size();j++){//计算当前段的头角度（长度>5的距离的两点之间的夹角）
            NeuronSWC child1=points[j];
            length+=distance_two_point(par1,child1);

            if(length>8){
                branchs[i].head_angle=Angle(child1.x-points[0].x,child1.y-points[0].y,child1.z-points[0].z);
                branchs[i].head_angle.norm_angle();
                break;
            }
            par1=child1;
        }

        if(length<=8){//长度不足5，该段是否丢弃？？？？
            qDebug()<<"segment i length not 8:"<<i;
            branchs[i].head_angle=Angle(par1.x-points[0].x,par1.y-points[0].y,par1.z-points[0].z);
            branchs[i].head_angle.norm_angle();
        }

        length=0;
        points.clear();


        branchs[i].get_r_points_of_branch(points,nt);
        NeuronSWC child2=points[0];
        for(int j=1;j<points.size();j++){//计算当前段的尾角度
            NeuronSWC par2=points[j];
            length+=distance_two_point(par2,child2);
            if(length>8){
                branchs[i].end_angle=Angle(points[0].x-par2.x,points[0].y-par2.y,points[0].z-par2.z);
                branchs[i].end_angle.norm_angle();
                break;
            }
            child2=par2;
        }

        if(length<=8){
            qDebug()<<"segment i length not 8:"<<i;
            branchs[i].end_angle=Angle(points[0].x-child2.x,points[0].y-child2.y,points[0].z-child2.z);
            branchs[i].end_angle.norm_angle();
        }
    }


    qDebug()<<"initial parent segment\n";
    for(int i=0;i<branchs.size();i++){//找到每个段的父段

        if(branchs[i].head_point.parent<0){
            branchs[i].parent=0;
        }
        else{
            for(int j=0;j<branchs.size();j++){
                if(branchs[i].head_point==branchs[j].end_point){
                    branchs[i].parent=&branchs[j];
                    break;
                }
            }

        }
    }

    for(int i=0;i<branchs.size();i++){
        if(branchs[i].length<=6){
            vector<NeuronSWC> points1,points2;
            branchs[i].parent->get_r_points_of_branch(points1,nt);
            branchs[i].get_points_of_branch(points2,nt);
            branchs[i].parent->length += branchs[i].length;
            branchs[i].parent->end_point=branchs[i].end_point;
            branchs[i].parent->end_angle=branchs[i].end_angle;
            branchs[i].parent->sum_angle += branchs[i].sum_angle;
            branchs[i].parent->distance=distance_two_point(branchs[i].parent->head_point,branchs[i].parent->end_point);
            branchs[i].parent->distance_to_soma+=branchs[i].length;
            NeuronSWC node1=points2.back();//获取本段的最后一个节点
            for(int j=0;j<children[node1.parent].size();j++){//遍历
                qDebug()<< children[node1.parent][j];
                for(int k=0;k<branchs[i];k++){
                    if(branchs[k].head_point==children[node1.parent][j]){
                        branchs[k].parent=branchs[i].parent;//将本段的子段的父段改为本段的父段
                    }
                }
            }
            branchs.erase(branchs[i]);
        }
    }


    return true;
}


bool SWCTree::cut_cross(QString input_path, V3DPluginCallback2 &callback){//剪枝cross-by false-positive
    vector<int> false_seg;
    const int threshold_distance=20;
    int size=branchs.size();
    vector<vector<int>> children_b=vector<vector<int> >(size,vector<int>());//每个segment的子段
    vector<int> queue;
    map<Branch,int> mapbranch;
    for(int i=0;i<branchs.size();i++){
        mapbranch[branchs[i]]=i;
    }

    for(int j=0;j<size;j++){
        if(branchs[j].parent==0){
            queue.push_back(j);
            continue;
        }
        children_b[mapbranch[*(branchs[j].parent)]].push_back(j);

    }
    while(!queue.empty()){
        int index_branch = queue.front();
        queue.erase(queue.begin());
        //每个segment到soma的距离
        if(branchs[index_branch].parent==0)
        {
            branchs[index_branch].distance_to_soma = 0;
        }
        else
        {
            branchs[index_branch].distance_to_soma = branchs[index_branch].parent->distance_to_soma+branchs[index_branch].parent->length;
        }
        if(branchs[index_branch].distance_to_soma<threshold_distance){//离soma太近忽略
            for(int i=0;i<children_b[index_branch].size();i++){//将继续往下寻找直到离soma距离大于阈值
                queue.push_back(children_b[index_branch][i]);
            }
        }
        else{
            double cos_angle=branchs[index_branch].head_angle*branchs[index_branch].parent->end_angle;//该段的头角度与父段的尾角度的点积
            double mean,std;
            double pn_mean,pn_std;
            double cd_mean=0.0,cd_std=0.0;
            qDebug()<<"start to get mean_std..."<<endl;
            branchs[index_branch].get_meanstd(input_path,callback,nt,mean,std,1);
            branchs[index_branch].parent->get_meanstd(input_path,callback,nt,pn_mean,pn_std,1);
            if(children_b[index_branch].size()>=1){
                branchs[children_b[index_branch][0]].get_meanstd(input_path,callback,nt,cd_mean,cd_std,1);//先暂定只计算一个孩子的
            }

            qDebug()<<"end get mean_std"<<endl;
            double angle_thres,len_thres,mean_thres,std_thres;

            //分层采用不同的阈值剪枝，离soma距离近的段和离soma超过一定距离的段，距离是从soma开始到该段累加的段的长度，不是直接的欧式距离
            //离soma距离在70以内
            if(branchs[index_branch].distance_to_soma<70){
                mean_thres=0.6*pn_mean;
                std_thres=1.8*(0.3*pn_std+0.7*cd_std);
                qDebug()<<" near to soma ,mean_threshold: "<<mean_thres<<"std_threshold: "<<std_thres;
            }
            else{//离soma距离大于70的阈值
                mean_thres=0.4*mean;
                if(cd_std!=0.0){
                    std_thres=1.0*(0.6*pn_std+0.4*cd_std);
                }
                else{
                    std_thres=1.0*pn_std;
                }
                qDebug()<<" far to soma ,mean_threshold: "<<mean_thres<<"std_threshold: "<<std_thres;
            }
            if(cos_angle<(sqrt(2.0)/2)||branchs[index_branch].sum_angle/branchs[index_branch].distance>0.8){//值待定
                if(cd_mean!=0.0){
                    if(std>std_thres||(abs(pn_mean-mean)>mean_thres||abs(mean-cd_mean)>mean_thres))//考虑前后段的灰度变化均值
                    {
                        false_seg.push_back(index_branch);
                        qDebug()<<"std: "<<std<<" mean_d: "<<abs(pn_mean-mean)<<" "<<abs(mean-cd_mean)<<endl;
                    }

                }
                else{//暂不处理，可能对于弱信号较小的分支
                    if(std>pn_std*3||abs(pn_mean-mean)>20){
                        false_seg.push_back(index_branch);
                    }

                }
            }
            else{
                for(int i=0;i<children_b[index_branch].size();i++){
                    queue.push_back(children_b[index_branch][i]);
                }
            }
        }
    }


    //找到所有错误段的所有子段
    vector<int> delete_seg;
    delete_seg.assign(false_seg.begin(),false_seg.end());
    while(!delete_seg.empty()){
        int delete_index=delete_seg.front();
        delete_seg.erase(delete_seg.begin());
        for(int i=0;i<children_b[delete_index].size();++i)
        {
            false_seg.push_back(children_b[delete_index][i]);
            delete_seg.push_back(children_b[delete_index][i]);
        }
    }



    //将所有delete_seg的颜色改变，用于显示
    NeuronTree nt1;
    for(int i=0;i<false_seg.size();i++){
        vector<NeuronSWC> nodes;
        branchs[false_seg[i]].get_points_of_branch(nodes,nt);
        for(int j=0;j<nodes.size();j++){
            nodes[j].type=2;

            nt1.listNeuron.push_back(nodes[j]);
        }
    }
    writeESWC_file("D://delete_seg.eswc",nt1);
    qDebug()<<"delete seg swc finish";


    //将原始的swc删除错误段，便于显示
    qDebug()<<"deleting.....\n";
//    sort(delete_seg.begin(),delete_seg.end());
//    vector<int>::iterator iter_r=unique(delete_seg.begin(),delete_seg.end());
//    delete_seg.erase(iter_r,delete_seg.end());
    sort(false_seg.begin(),false_seg.end(),Comp);//从大到小排序，否则先删除前面的段会造成后面段的index发生改变，无法删除正确的段
    qDebug()<<"before delete:"<<branchs.size()<<"\n";
    for(int i=0;i<false_seg.size();i++){
        vector<Branch>::iterator iter=branchs.begin();
        iter+=false_seg[i];
        branchs.erase(iter);
    }
    qDebug()<<"after delete:"<<branchs.size()<<"\n";
    NeuronTree nt2;
    for(int i=0;i<branchs.size();i++){
        vector<NeuronSWC> nodes;
        branchs[i].get_points_of_branch(nodes,nt);
        for(int j=0;j<nodes.size();j++){
            nodes[j].type=i%4+3;//不同的段显示不同的颜色，便于观察分段位置
            nt2.listNeuron.push_back(nodes[j]);
        }
    }
    writeESWC_file("D://new_swc_file.eswc",nt2);
    qDebug()<<"finish delete !";



    return true;

}
