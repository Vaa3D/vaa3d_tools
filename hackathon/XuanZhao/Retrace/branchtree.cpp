#include "branchtree.h"

#include "swc2mask.h"
#include "swc_convert.h"

#include <iostream>

using namespace std;

bool Branch::get_r_pointsIndex_of_branch(vector<V3DLONG> &r_points, NeuronTree &nt)
{
//    qDebug()<<"headPointIndex: "<<headPointIndex<<" endPointIndex: "<<endPointIndex;
    int tmp = endPointIndex;
    r_points.push_back(tmp);
    while(tmp != headPointIndex)
    {
//        qDebug()<<"tmp: "<<tmp;
        tmp = nt.hashNeuron.value(nt.listNeuron[tmp].parent);
        r_points.push_back(tmp);
    }
    return true;
}

bool Branch::get_pointsIndex_of_branch(vector<V3DLONG> &points, NeuronTree &nt)
{
    vector<V3DLONG> r_points;
    this->get_r_pointsIndex_of_branch(r_points,nt);
    while(!r_points.empty())
    {
        int tmp=r_points.back();
        r_points.pop_back();
        points.push_back(tmp);
    }

    return true;
}

void Branch::getBranchFeature(unsigned char *pdata, V3DLONG *sz, NeuronTree &nt){
    vector<V3DLONG> pointsIndex = vector<V3DLONG>();
    this->get_pointsIndex_of_branch(pointsIndex,nt);

    int pointNum = pointsIndex.size();
    NeuronTree maskTree = NeuronTree();
    vector<bool> isInterPoints = vector<bool>(pointNum,true);
    vector<float> lengthList = vector<float>(pointNum,0);

    for(int i=1; i<pointNum; i++){
        MyMarker p1(nt.listNeuron[pointsIndex[i-1]].x,nt.listNeuron[pointsIndex[i-1]].y,nt.listNeuron[pointsIndex[i-1]].z);
        MyMarker p2(nt.listNeuron[pointsIndex[i]].x,nt.listNeuron[pointsIndex[i]].y,nt.listNeuron[pointsIndex[i]].z);
        lengthList[i] = lengthList[i-1] + dist(p1,p2);
    }
//    qDebug()<<"----------cal length end-----------";
    for(int i=0; i<pointNum; i++){
        if(lengthList[i]<nt.listNeuron[headPointIndex].r){
            isInterPoints[i] = false;
        }else {
            break;
        }
    }
    for(int i=pointNum-1; i>=0; i--){
        if((lengthList[pointNum-1] - lengthList[i])<nt.listNeuron[endPointIndex].r){
            isInterPoints[i] = false;
        }else {
            break;
        }
    }
//    qDebug()<<"------------get interPoint end--------------";
    for(int i=0; i<pointNum; i++){
        if(isInterPoints[i]){
            maskTree.listNeuron.push_back(nt.listNeuron[pointsIndex[i]]);
        }
    }

    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];

    vector<MyMarker*> interMarkers = swc_convert(maskTree);

//    qDebug()<<"------------swc_convert end--------------";
    for(int j=0; j<interMarkers.size(); j++){
        interMarkers[j]->radius = 1;
    }
    unsigned char* maskR1 = 0;
    swcTomask(maskR1,interMarkers,sz[0],sz[1],sz[2]);

    for(int j=0; j<interMarkers.size(); j++){
        interMarkers[j]->radius = 2;
    }
    unsigned char* maskR2 = 0;
    swcTomask(maskR2,interMarkers,sz[0],sz[1],sz[2]);

    for(int j=0; j<interMarkers.size(); j++){
        interMarkers[j]->radius = 3;
    }
    unsigned char* maskR3 = 0;
    swcTomask(maskR3,interMarkers,sz[0],sz[1],sz[2]);

    float intensityR1 = 0, intensityR2 = 0, intensityR3 = 0;
    int countR1 = 0, countR2 = 0, countR3 = 0;

    for(int j=0; j<tolSZ; j++){
        if(maskR1[j] > 0){
            intensityR1 += pdata[j];
            countR1++;
        }
        if(maskR2[j] > 0){
            intensityR2 += pdata[j];
            countR2++;
        }
        if(maskR3[j] > 0){
            intensityR3 += pdata[j];
            countR3++;
        }
    }

//    qDebug()<<"------------ mask end-----------------";

    if(maskR1){
        delete[] maskR1;
        maskR1 = 0;
    }
    if(maskR2){
        delete[] maskR2;
        maskR2 = 0;
    }
    if(maskR3){
        delete[] maskR3;
        maskR3 = 0;
    }

//    qDebug()<<"------------ delete mask end-----------------";



    if(countR1>0)
        intensityR1 /= countR1;
    if(countR2>0)
        intensityR2 /= countR2;
    if(countR3>0)
        intensityR3 /= countR3;

//    qDebug()<<"------------ cal intensity end-----------------";

    MyMarker h = MyMarker(nt.listNeuron[headPointIndex].x,nt.listNeuron[headPointIndex].y,nt.listNeuron[headPointIndex].z);
    MyMarker t = MyMarker(nt.listNeuron[endPointIndex].x,nt.listNeuron[endPointIndex].y,nt.listNeuron[endPointIndex].z);
    double dis = dist(h,t);

    cout<<"n: "<<nt.listNeuron[endPointIndex].n<<" parent: ";
    if(parent){
        cout<<nt.listNeuron[parent->endPointIndex].n;;
    }else {
        cout<<"-1";
    }

    cout<<" level: "<<level
           <<" x: "<<nt.listNeuron[endPointIndex].x<<" y: "<<nt.listNeuron[endPointIndex].y<<" z: "<<nt.listNeuron[endPointIndex].z
          <<" r: "<<nt.listNeuron[endPointIndex].r<<" distance: "<<dis<<" length: "<<lengthList[pointNum-1]
         <<" intensityR1: "<<intensityR1<<" intensityR2: "<<intensityR2<<" intensityR3: "<<intensityR3
        <<" R1/R2: "<<intensityR1/intensityR2<<" R1/R3: "<<intensityR1/intensityR3<<"\n";

    if(intensityR1/intensityR3>1){
        for(int i=0; i<pointNum; i++){
            nt.listNeuron[pointsIndex[i]].type = 2;
        }
    }else {
        for(int i=0; i<pointNum; i++){
            nt.listNeuron[pointsIndex[i]].type = 3;
        }
    }

}

bool BranchTree::initialize(NeuronTree t)
{
    nt.deepCopy(t);

    vector<V3DLONG> ori = vector<V3DLONG>();

    V3DLONG num_p=nt.listNeuron.size();
    vector<vector<V3DLONG> > children=vector<vector<V3DLONG> >(num_p,vector<V3DLONG>());
    for(V3DLONG i=0;i<num_p;++i)
    {
        V3DLONG prt=nt.listNeuron[i].parent;
        if(prt != -1){
            V3DLONG prtIndex = nt.hashNeuron.value(prt);
            children.at(prtIndex).push_back(i);
        }else {
            ori.push_back(i);
        }
    }

    vector<V3DLONG> queue = vector<V3DLONG>();
    queue.insert(queue.begin(),ori.begin(),ori.end());
    cout<<"queue size: "<<queue.size()<<endl;


    cout<<"initial head_point,end_point"<<endl;
    while(!queue.empty())
    {
        V3DLONG tmp=queue.front();
        queue.erase(queue.begin());
        vector<V3DLONG>& child = children.at(tmp);
//        cout<<"child size: "<<child.size()<<endl;
        for(int i=0;i<child.size();++i)
        {
            Branch branch;
            branch.headPointIndex = tmp;
            int cIndex = child.at(i);

            while(children.at(cIndex).size() == 1)
            {
                cIndex = children.at(cIndex).at(0);
            }
            if(children.at(cIndex).size() >= 1)
            {
                queue.push_back(cIndex);
            }
            branch.endPointIndex = cIndex;
            branchs.push_back(branch);
        }
    }

    //initial parent
    cout<<"initial parent"<<endl;
    cout<<"branch size: "<<branchs.size()<<endl;
    for(int i=0;i<branchs.size();++i)
    {
        cout<<i<<endl;
        if(nt.listNeuron.at(branchs[i].headPointIndex).parent < 0)
        {
            branchs[i].parent = 0;
        }
        else
        {
            for(int j=0; j<branchs.size(); ++j)
            {
                if(branchs[i].headPointIndex == branchs[j].endPointIndex)
                {
                    branchs[i].parent = &branchs[j];
                }
            }
        }
    }

    //initial level
    for(int i=0; i<branchs.size(); ++i)
    {
        Branch* tmp;
        tmp = &branchs[i];
        int level=0;
        while(tmp->parent != 0)
        {
            level++;
            tmp = tmp->parent;
        }
        branchs[i].level=level;
    }

    cout<<"--------------------initialize end--------------";

    return true;
}

bool BranchTree::get_level_index(vector<int> &level_index, int level){
    for(int i=0; i<branchs.size(); i++){
        if(branchs[i].level == level){
            level_index.push_back(i);
        }
    }
    return true;
}

int BranchTree::get_max_level(){
    int max = 0;
    for(int i=0; i<branchs.size(); i++){
        if(branchs[i].level > max){
            max = branchs[i].level;
        }
    }
    return max;
}

void BranchTree::getBranchFeature(unsigned char *pdata, long long *sz){
    for(int i=0; i<branchs.size(); i++){
        branchs[i].getBranchFeature(pdata,sz,nt);
    }
}
