#include "review.h"
#include<math.h>
#include<algorithm>
#include "filter_dialog.h"
#include <iostream>
#define length_threshold 150

//找到整个树的所有孩子节点
bool SWCTreeSeg::initialize(NeuronTree t){//初始化整个树的结构
    nt.deepCopy(t);
    V3DLONG size = nt.listNeuron.size();
    children = vector<vector<V3DLONG> >(size,vector<V3DLONG>());//父子对应关系表，在父亲对应位置记录其孩子节点id
    //如何分段
    for(V3DLONG i=0;i<size;i++){
        V3DLONG pn=nt.listNeuron[i].parent;
        if(pn<0){
            //root=nt.listNeuron[i];
            continue;
        }
        children[nt.hashNeuron.value(pn)].push_back(i);

    }
    return true;


}

//从块中候选点开始进行分段，并将新的候选点（由于切块导致的，本来就不在块内的）返回
bool SWCTreeSeg::block_seg(NeuronTree orig,vector<NeuronSWC> &candidate_point,vector<V3DLONG> &subNeuron,size_t x0,size_t y0,size_t z0,int resolution){//一个块内的分段
    vector<NeuronSWC> new_candidate;
    vector<NeuronSWC> not_block;
    V3DLONG size=candidate_point.size();
    int times = pow(2,(resolution-1));
    qDebug()<<"candidate size"<<size;
    bool flag_length=false;
    while(!candidate_point.empty()){//候选点此时还是绝对坐标
        qDebug()<<"***********************************";
        NeuronSWC tmp = candidate_point.back();
        //转换为此块内的相对坐标，再判断是否在此块内的所有点中
        tmp.x=tmp.x/times-x0;
        tmp.y=tmp.y/times-y0;
        tmp.z=tmp.z/times-z0;
        //select_point tmp1(tmp.n,false);
        if(tmp.parent<0){
            root=tmp;
        }
        if(find(subNeuron.begin(),subNeuron.end(),tmp.n)==subNeuron.end()){//候选点可能有些不在当前块中，在块移动的时候
            not_block.push_back(tmp);//不在当前块的候选起始点
            candidate_point.erase(candidate_point.end()-1);
            continue;
        }
        candidate_point.erase(candidate_point.end()-1);
        qDebug()<<"candidate point size:"<<candidate_point.size();
        qDebug()<<tmp.n;
        //qDebug()<<"child size:"<<children[nt.hashNeuron.value(tmp.n)].size();
        for(V3DLONG i=0;i<children[nt.hashNeuron.value(tmp.n)].size();i++){//从候选点的孩子开始
            qDebug()<<"---------------------";
            segment seg;
            seg.start=tmp;
            seg.points.push_back(tmp);
            seg.length=0;
            seg.segs_index=segments.size(); //暂时不用
            NeuronSWC child = nt.listNeuron[children[nt.hashNeuron.value(seg.start.n)][i]];
            //select_point child_s(child.n,false);
            vector<V3DLONG>::iterator it=find(subNeuron.begin(),subNeuron.end(),child.n);//查看孩子节点是否在当前块的节点内
            if(it==subNeuron.end()){//分叉点的可能存在在块的边缘，导致出现只有一个点的段
                not_block.push_back(orig.listNeuron[orig.hashNeuron.value(child.n)]);
                //qDebug()<<"-----------------------not-block--------------------";
                continue;
            }
            Point_xyz point1(tmp.x,tmp.y,tmp.z);
            Point_xyz point2(child.x,child.y,child.z);
            qDebug()<<"child size in seg:"<<children[nt.hashNeuron.value(child.n)].size();
            while(children[nt.hashNeuron.value(child.n)].size()==1&&it!=subNeuron.end()){//没有分叉，且在当前块中
                //it->select=true;//当前点已经在段中了
                seg.points.push_back(child);//将该段的所有点都保留下来，在块内的点已经是相对坐标
                seg.end=child;
                seg.length+=distance_two_point(point1,point2);
                if(seg.length>=length_threshold){//长度过长的切断阈值暂定40
                    qDebug()<<"------------cut seg length---------";
                    flag_length=true;
                    break;
                }
                child=nt.listNeuron[children[nt.hashNeuron.value(child.n)][0]];
                //select_point child_s(child.n,false);
                point1=point2;
                point2=Point_xyz(child.x,child.y,child.z);
                it=find(subNeuron.begin(),subNeuron.end(),child.n);
            }
            qDebug()<<"child size in seg:"<<children[nt.hashNeuron.value(child.n)].size();
            if(flag_length){//因段的长度过长而被切断,有可能正好到分叉点
                //qDebug()<<"new candidate1"<<child.n;
                candidate_point.push_back(orig.listNeuron[orig.hashNeuron.value(child.n)]);
                segments.push_back(seg);
                //qDebug()<<"end cord1:"<<seg.end.x<<seg.end.y<<seg.end.z;
                flag_length=false;
            }
            else if(it==subNeuron.end()){//到达块的边缘,可能同时在块的边缘，又是分叉点，所以先判断是否到达块的边缘
                new_candidate.push_back(orig.listNeuron[orig.hashNeuron.value(child.n)]);//下一个块的候选点
                segments.push_back(seg);//因为达到边缘而终止，所以它的上一个节点作为本段的终点
                qDebug()<<"cut seg size: "<<seg.points.size();
                //qDebug()<<"seg :"<<seg.start.n<<seg.end.n;
                //qDebug()<<"end cordc:"<<seg.end.x<<seg.end.y<<seg.end.z;
                continue;
            }
            else if(children[nt.hashNeuron.value(child.n)].size()>=2){//因为分叉结束
                //因为child是经过坐标变换的，前面还要再变换一次会造成错误，所以这里只能存放原来该点的绝对坐标
                //找到它原来的位置
                qDebug()<<"new candidate2"<<child.n;
                candidate_point.push_back(orig.listNeuron[orig.hashNeuron.value(child.n)]);//分叉表示分段未结束，导致新的起始点
                seg.points.push_back(child);//分叉点
                seg.end=child;
                segments.push_back(seg);
                //qDebug()<<"end cord2:"<<seg.end.x<<seg.end.y<<seg.end.z;
                continue;
            }
            else if(children[nt.hashNeuron.value(child.n)].size()==0){
                seg.points.push_back(child);
                seg.end=child;
                segments.push_back(seg);
                //qDebug()<<"end cord0:"<<seg.end.x<<seg.end.y<<seg.end.z;
            }
            //有可能达到叶子节点
            //放入分段中，表示已经被分段了
//            qDebug()<<"seg size: "<<seg.points.size();
//            qDebug()<<"seg :"<<seg.start.n<<seg.end.n;

//            //qDebug()<<"seg length:"<<seg.length;

        }

    }
    qDebug()<<"segments size:"<<segments.size();


    //vector<V3DLONG> merge_point;
    candidate_point.insert(candidate_point.end(),not_block.begin(),not_block.end());//不在这个块内的剩余候选点
    candidate_point.insert(candidate_point.end(),new_candidate.begin(),new_candidate.end());//新产生的候选点
    return true;
}

bool Comp_node(NeuronSWC &a,NeuronSWC &b){
    return a.n<b.n;
}

//计算大小为sz0*sz1*sz2块中，每一个段的平均灰度值
bool SWCTreeSeg::seg_intensity(V3DLONG sz0,V3DLONG sz1,V3DLONG sz2,unsigned char* &p1data){
    V3DLONG sz_num=sz0*sz1*sz2;//出现越界问题，坐标不在图像块中？？？？？
    //qDebug()<<"seg_intensity segments size"<<segments.size();
    segment seg;
    for(int i=0;i<segments.size();i++){
        NeuronTree subTree;
        subTree.listNeuron.clear();
        seg=segments[i];
        qDebug()<<"seg size:"<<seg.points.size();
        //除掉一个段上的重复点
//        sort(seg.points.begin(),seg.points.end(),Comp_node);
//        vector<V3DLONG> index;
//        for(int j=0;j<seg.points.size();j++){
//            index.push_back(seg.points[j].n);
//        }
//        vector<V3DLONG>::iterator ite=unique(index.begin(),index.end());
//        for(;ite!=index.end();ite++){
//            for(int ii=seg.points.size()-1;ii>=0;ii--){//为防止删除前面的点过后导致后面点位置发生改变，所以从尾到头开始搜索
//                if(seg.points[ii].n==*ite){
//                    seg.points.erase(seg.points.begin()+ii);//删除重复点
//                    break;
//                }
//            }
//        }
        //---------------------------------
        for(int j=0;j<seg.points.size();j++){
            subTree.listNeuron.push_back(seg.points[j]);
        }
        //计算每一个段的平均灰度
        unsigned char* data1d_mask=0;
        data1d_mask=new unsigned char[sz_num];
        memset(data1d_mask,0,sz_num*sizeof(unsigned char));
        double margin=1;

        //会出现overlapp node,就对它不做处理，属于正常现象
        //为什么有些段上的点很少且mask没有覆盖任何区域？？？？
        ComputemaskImage(subTree,data1d_mask,sz0,sz1,sz2,margin);//给指定段打上mask，计算intenstity
        int count=0;
        //qDebug()<<"sz_num"<<sz_num;
        //qDebug()<<p1data[0];//p1data在这里为空指针
        for(V3DLONG k=0;k<sz_num;k++){
            if(data1d_mask[k]!=0){
                //qDebug()<<"p1data:"<<p1data[k];//为什么都是0？？？？？、

                seg.avg_intensity+=p1data[k];
                count++;
            }
        }
        //qDebug()<<"count:"<<count;//segment 上体素的数目
        seg.avg_intensity/=count;
        qDebug()<<"seg_intensity:"<<seg.avg_intensity;
        qDebug()<<"seg length:"<<seg.length;

        segments[i]=seg;
        qDebug()<<"-------------*------------------";
    }

    return true;
}

//以center_point为中心切一个大小为block_sz0*block_sz1*block_sz2大小的块，x0,y0,z0表示该块原点的绝对坐标
bool cut_block(QString input,V3DPluginCallback2 &callback,NeuronTree &nt,V3DLONG block_sz0,V3DLONG block_sz1,V3DLONG block_sz2,size_t &x0,size_t &y0,size_t &z0, NeuronSWC center_point,vector<V3DLONG> &subNeuron,int resolution,unsigned char* &p1data){
    size_t x1=0,y1=0,z1=0;
    x0=IN,y0=IN,z0=IN;
    //获取不同分辨率的图
    QDir all_braindir(input);
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
    //当前选择分辨率下的路径
    qDebug()<<"directory count:"<<dir_count;
    QString current_braindir = list_braindir[dir_count-resolution].absoluteFilePath();
    qDebug()<<current_braindir;
    int times = pow(2,(resolution-1));//当前分辨率下，坐标的缩放
    qDebug()<<"times"<<times;
    //获取块的边界坐标
    x0=(center_point.x-block_sz0/2)/times-10;
    x1=(center_point.x+block_sz0/2)/times+10;
    y0=(center_point.y-block_sz1/2)/times-10;
    y1=(center_point.y+block_sz1/2)/times+10;
    z0=(center_point.z-block_sz2/2)/times-10;
    z1=(center_point.z+block_sz2/2)/times+10;
    x0=(x0<0)?0:x0;
    //x1,y1,z1的边界值目前不知道，暂不考虑，这是为了防止坐标越界
    y0=(y0<0)?0:y0;
    z0=(z0<0)?0:z0;
    qDebug()<<"center point:"<<center_point.n<<center_point.x<<center_point.y<<center_point.z;
    qDebug()<<"x0,y0,z0:"<<x0<<y0<<z0;
    p1data = 0;
    //获取以center_point为中心的,大小为block_size的原图块
    p1data = callback.getSubVolumeTeraFly(current_braindir.toStdString().c_str(),x0,x1,y0,y1,z0,z1);
    qDebug()<<"cut_block p1data:"<<p1data;

    //将该块保存下来，方便后期显示
    V3DLONG sz0=x1-x0;
    V3DLONG sz1=y1-y0;
    V3DLONG sz2=z1-z0;

    QString tiffile = "D://subvolum//"+QString::number(x0)+"_"+QString::number(y0)+"_"+QString::number(z0)+".tif";
    V3DLONG sz[4] = {sz0,sz1,sz2,1};
    simple_saveimage_wrapper(callback,tiffile.toStdString().c_str(),p1data,sz,1);

    //切块，将坐标转换为块内的相对坐标
    V3DLONG size = nt.listNeuron.size();
    for(V3DLONG i=0;i<size;i++){

        if(nt.listNeuron[i].x/times<x0||nt.listNeuron[i].x/times>x1||nt.listNeuron[i].y/times<y0||nt.listNeuron[i].y/times>y1||nt.listNeuron[i].z/times<z0||nt.listNeuron[i].z/times>z1)
            //坐标不在所切块范围内，移动块的位置
          continue;
        else{//在当前所切的块内
            //转换为块内的相对坐标
            nt.listNeuron[i].x=nt.listNeuron[i].x/times-x0;
            nt.listNeuron[i].y=nt.listNeuron[i].y/times-y0;
            nt.listNeuron[i].z=nt.listNeuron[i].z/times-z0;
            //select_point point(nt.listNeuron[i].n,false);
            subNeuron.push_back(nt.listNeuron[i].n);//存放在该块中点的index
            //qDebug()<<nt.listNeuron[i].n;
        }
    }
    return true;
}

//将整个swc进行切块，并根据候选点移动块的位置进行下一步的追踪排序
bool move_block(QString braindir, V3DPluginCallback2 &callback,NeuronTree orig,SWCTreeSeg &segs, long long block_sz0, long long block_sz1, long long block_sz2,vector<NeuronSWC> &candidate_point){
    //vector<NeuronSWC> candidate_point;
    size_t x0=IN,y0=IN,z0=IN;
    V3DLONG size = segs.nt.listNeuron.size();
    unsigned char *p1data=0;
    //V3DLONG block_size = 500;//暂定值
    int resolution = 1;//暂定所有最高分辨率
    vector<V3DLONG> subNeuron;
    for(V3DLONG i=0;i<size;i++){  
        V3DLONG pn = segs.nt.listNeuron[i].parent;
        if(pn<0){//以root为中心点切块

            candidate_point.push_back(segs.nt.listNeuron[i]);//将候选起始追踪点放入栈中
        }

    }
    qDebug()<<"first candidate size:"<<candidate_point.size();
    cut_block(braindir,callback,segs.nt,block_sz0,block_sz1,block_sz2,x0,y0,z0,candidate_point.front(),subNeuron,resolution,p1data);
    qDebug()<<"subNeuron SIZE:"<<subNeuron.size();
    qDebug()<<"test p1data:"<<p1data[1];
    segs.block_seg(orig,candidate_point,subNeuron,x0,y0,z0,resolution);//从候选起始点开始分段
    qDebug()<<"test2 p1data:"<<p1data[2];
    segs.seg_intensity(block_sz0,block_sz1,block_sz2,p1data);//计算当前块中所有段的平均信号强度
   if(!candidate_point.empty()){
    //继续递归调用上述的切块分段等操作，下次调用分块操作时应根据上次分块后段的顺序最后一个段的位置决定下一次分块的位置
    }
    return true;
}

//排序规则
bool Comp(segment &a,segment &b){
    return a.avg_intensity>b.avg_intensity;
}


//将每个段的信息都保存到相关的结构中，然后后期一起写入文件
bool write_swc(NeuronTree orig,NeuronTree &nt,QList<ImageMarker> &markers,segment seg,V3DLONG &index){
        qDebug()<<"segment index:"<<index;
        int size=seg.points.size();//还是相对坐标，没有转换,所以通过index找到它原来在树中的坐标值，之前不能改成绝对坐标，因为计算mask需要相对坐标
        qDebug()<<"seg avg intensity:"<<seg.avg_intensity;
        //只在段尾添加marker，避免出现重复
        //CellAPO *marker_end;
        ImageMarker *marker_end;
        NeuronSWC start,end;

//        start=orig.listNeuron[orig.hashNeuron.value(seg.start.n)];//找到绝对坐标
//        end=orig.listNeuron[orig.hashNeuron.value(seg.end.n)];
        start=seg.start;
        end=seg.end;//相对坐标
        qDebug()<<"start,end:"<<start.n<<end.n;

        marker_end=new ImageMarker();
        //marker_end=new CellAPO();
        marker_end->name=QString::number(index);
        marker_end->color.r=255;
        marker_end->color.g=0;
        marker_end->color.b=0;
        marker_end->x=end.x+1;//marker起始点是(1,1,1)
        marker_end->y=end.y+1;
        marker_end->z=end.z+1;
        markers.push_back(*marker_end);

        NeuronSWC point;
        for(int j=1;j<size;j++){//防止分叉点重复出现
            //point=orig.listNeuron[orig.hashNeuron.value(seg.points[j].n)];//绝对坐标
            point=seg.points[j];//相对坐标
            point.type=index%14;//记录顺序信息
            nt.listNeuron.push_back(point);
            //std::cout<<point.n<<" ";
        }
        index++;
    return true;
}

/***
 * 先从soma开始，将与soma直接相连的段作为起始段，将起始段按照平均灰度值进行排序，
然后按照排序结果，依次选择一个段假设为A作为一个新的开始，将该段的所有子段按照灰度值排序，
然后对排序结果进行处理，将子段中存在父子关系的段调整位置，保证父段在子段的前面
***/
bool sequence_rule(QString inputpath,SWCTreeSeg &swcTree,NeuronTree orig,vector<vector<V3DLONG> > children){
    QFileInfo eswcfileinfo;
    eswcfileinfo=QFileInfo(inputpath);
    QString eswcfile=eswcfileinfo.fileName();
    eswcfile.mid(0,eswcfile.indexOf("."));
    NeuronTree nt1;
    //QList<CellAPO> markers;
    QList<ImageMarker> markers;
    vector<segment> segs = swcTree.segments;
    V3DLONG size=segs.size();
    //qDebug()<<"swctree segments size:"<<segs.size();
    //qDebug()<<segs[0].avg_intensity;
    V3DLONG index=0;

    //找到每个段的父段,子段
    for(V3DLONG i=0;i<size;i++){
        if(segs[i].start.parent<0){//没有父段
            segs[i].parent_seg=0;

        }
        V3DLONG j;
        for(j=0;j<size;j++){
            if(i==j){
                continue;
            }
            if(segs[i].end.n==segs[j].start.n){
               segs[j].parent_seg=&segs[i];//每个段只有一个父段
               segs[i].child_seg.push_back(&segs[j]);//一个段存在两个子段
            }
         }

        if(j==size&&segs[i].child_seg.size()==0){//没有子段，叶子节点或者到达块的边缘,防止有孩子的也push0
                segs[i].child_seg.push_back(0);

            }
        //qDebug()<<"seg"<<i<<"child size:"<<segs[i].child_seg.size();
    }


    //按照灰度值进行从大到小排序,存在问题(由于一些段较短所以平均灰度反而很大先被重建)暂时不用
    //sort(segs.begin(),segs.end(),Comp);

    vector<segment> init_seg;
    //获取所有从soma开始的段
    vector<segment>::iterator ite;
    for(ite=segs.begin();ite<segs.end();ite++){
        segment seg=*ite;
        if(seg.parent_seg==0){//从soma开始的段
            init_seg.push_back(seg);
            //segs.erase(ite);//对将要重建的数据在候选子集中删除
        }
    }

    //对初始段按照平均灰度值排序
    sort(init_seg.begin(),init_seg.end(),Comp);

    for(int ii=0;ii<init_seg.size();ii++){
        qDebug()<<"--------------------------------------------";
        qDebug()<<"write swc segment:"<<ii;

        write_swc(orig,nt1,markers,init_seg[ii],index);

        //开始排序每一个初始段的所有子段
        vector<segment*> child_seg = init_seg[ii].child_seg;
        vector<segment> child_segs;//所有子段
        vector<segment> bfs_seg;
        //获取所有子段,广度优先遍历
        //qDebug()<<"first child size:" <<child_seg.size();
        while(child_seg.front()!=0&&!child_seg.empty()){//取出初始段的左右孩子
            bfs_seg.push_back(*child_seg.front());
            //qDebug()<< child_seg.front();
            child_seg.erase(child_seg.begin());
        }
        qDebug()<<"---------111---------";
        //qDebug()<<"first bfs size:"<<bfs_seg.size();
        while(!bfs_seg.empty()){
            segment seg=bfs_seg.front();
            bfs_seg.erase(bfs_seg.begin());
            child_segs.push_back(seg);
            child_seg=seg.child_seg;
            //qDebug()<<"-------1---1-------";
            while(child_seg.front()!=0&&!child_seg.empty()){//左右孩子入队
                bfs_seg.push_back(*child_seg.front());
                //qDebug()<< child_seg.front();
                child_seg.erase(child_seg.begin());
            }
            //qDebug()<<bfs_seg.size();
        }
        qDebug()<<"---------222---------";
        //按照平均灰度从大到小排序
        sort(child_segs.begin(),child_segs.end(),Comp);
        qDebug()<<"child segs size:"<<child_segs.size();
        vector<segment> new_child_segs;
        //将父段插入子段的前面
        V3DLONG child_size=child_segs.size();
        for(V3DLONG k=0;k<child_size;k++){
            vector<segment> new_child;
            qDebug()<<"child segs index:"<<k;
            segment seg1=child_segs[k];
            new_child.push_back(seg1);
            qDebug()<<seg1.parent_seg->start.n;
            segment* pn_seg=seg1.parent_seg;
            int count=0;
            //qDebug<<"pn seg start:"<<pn_seg->start.n;
            while(pn_seg!=0){//递归往上找到所有父段,除掉初始段
                //qDebug()<<"count:"<<count;
                vector<segment>::iterator it;
                it=find(child_segs.begin(),child_segs.end(),*pn_seg);
                //new_it=find(new_child_segs.begin(),new_child_segs.end(),*pn_seg);
//                if(new_it!=new_child_segs.end()){//已经被排序过了，所以它的所有父段都在已经排序的新序列中
//                    break;
//                }
                if(it==child_segs.end()){//父段不在其中,已经被排过序了
                    break;
                }
                qDebug()<<"pn seg index:"<<distance(child_segs.begin(),it);
                if(distance(child_segs.begin(),it)>k){//distance 算两个迭代器之间的距离，包含多少个元素
                    child_segs.erase(it);//先移除父段
                    child_size=child_segs.size();//删除后vector的长度发生了改变
                    new_child.push_back(*pn_seg);
                    //new_child_segs.push_back(*pn_seg);
                    //child_segs.insert(child_segs.begin()+k-count,*pn_seg);//插入
                    pn_seg=pn_seg->parent_seg;
                    count++;
                }
                else{
                    break;
                }

            }
            reverse(new_child.begin(),new_child.end());//反转,父子关系列表，让祖先放在前面，孩子放在后面
            qDebug()<<"size:"<<new_child.size();
            new_child_segs.insert(new_child_segs.end(),new_child.begin(),new_child.end());
        }
        child_segs=new_child_segs;
        qDebug()<<"------------*********------------";
        //写入初始段的子段
        for(V3DLONG jj=0;jj<child_segs.size();jj++){
            write_swc(orig,nt1,markers,child_segs[jj],index);
        }
        qDebug()<<"------------*********------------";

    }

    //按照排序后的段开始进行子父段排序
//    for(V3DLONG j=0;j<size;j++){
//        segment seg1=segs[j];
//        if(seg1.parent_seg!=0){//有父段
//            segment* pn_seg=seg1.parent_seg;
//            vector<segment>::iterator it;
//            it=find(segs.begin(),segs.end(),*pn_seg);//找出父段在segments的位置
//            if( (it-segs.begin()) > j){//父段在子段的后面
//                segs.erase(it);//先移除原来父段，这样子段的位置不变
//                segs.insert(segs.begin()+j,*pn_seg);//将父段插入到子段的前面
//            }


//        }
//    }


    nt1.listNeuron.push_back(swcTree.root);
    qDebug()<<"segments size:"<<index;
    //nt1.listNeuron.push_back(orig.listNeuron[orig.hashNeuron.value(swcTree.root.n)]);
    writeESWC_file("C://Users//penglab//Desktop//17302-00001//review_test1//"+eswcfile+".eswc",nt1);
    //writeAPO_file("C://Users//penglab//Desktop//17302-00001//review_test1//marker.apo",markers);
    writeMarker_file("C://Users//penglab//Desktop//17302-00001//review_test1//"+eswcfile+".marker",markers);
    return true;
}


