#include "branchtree.h"

//#include "../../../released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h"
//#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/swc_convert.h"

#include "swc2mask.h"
#include "swc_convert.h"

#include "marker_radius.h"
#include "basic_memory.cpp"
#include "compute_win_pca.h"

bool Branch::get_r_pointsIndex_of_branch(vector<V3DLONG> &r_points, NeuronTree &nt)
{
    int tmp = endPointIndex;
    r_points.push_back(tmp);
    while(tmp != headPointIndex)
    {
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

bool Branch::caculateFeature(unsigned char *data1d, V3DLONG *sz, NeuronTree& nt){

    cout<<"------------in calculateFeature---------------"<<endl;

    unsigned char *** data3d = 0;
    if(!new3dpointer(data3d,sz[0],sz[1],sz[2],data1d)){
        cout<<"Fail to allocate memory"<<endl;
        return false;
    }

    QList<NeuronSWC>& listNeuron = nt.listNeuron;
    distance = dis(listNeuron.at(headPointIndex),listNeuron.at(endPointIndex));
    intensityMean = 0;
    intensityStd = 0;
    length = 0;
    angleChangeMean = 0;
    gradientMean = 0;
    sigma12 = 0;
    sigma13 = 0;

    vector<V3DLONG> pointsIndex = vector<V3DLONG>();
    this->get_pointsIndex_of_branch(pointsIndex,nt);

//    cout<<"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"<<endl;

    int count = 0;

    int x,y,z,index;
    x = (int) (listNeuron.at(headPointIndex).x + 0.5);
    y = (int) (listNeuron.at(headPointIndex).y + 0.5);
    z = (int) (listNeuron.at(headPointIndex).z + 0.5);
    if(x >= sz[0]) x = sz[0] - 1;
    if(x < 0) x = 0;
    if(y >= sz[1]) y = sz[1] - 1;
    if(y < 0) y = 0;
    if(z >= sz[2]) z = sz[2] - 1;
    if(z < 0) z = 0;

    index = z*sz[0]*sz[1] + y*sz[0] + x;
    int lastIntensity = data1d[index], curIntensity;

    intensityMean += lastIntensity;

    Angle lastAngle = Angle(), curAngle = Angle();

    double pc1 = 0, pc2 = 0, pc3 = 0;
    int rx = 5, ry = 5, rz =5;
    int pcaCount = 0;
    if(compute_win3d_pca(data3d,sz[0],sz[1],sz[2],x,y,z,rx,ry,rz,pc1,pc2,pc3,1,false)){
        sigma12 += pc1/pc2;
        pcaCount++;
    }

    count++;

    V3DLONG p1 = headPointIndex;

    for(int i=1; i<pointsIndex.size(); i++){

//        cout<<"i: "<<i<<endl;

        V3DLONG p2 = pointsIndex.at(i);
        if(i == 1){
            lastAngle.setXYZ(listNeuron.at(p2).x - listNeuron.at(p1).x, listNeuron.at(p2).y - listNeuron.at(p1).y, listNeuron.at(p2).z - listNeuron.at(p1).z);
            lastAngle.normAngle();
        }else {
            curAngle.setXYZ(listNeuron.at(p2).x - listNeuron.at(p1).x, listNeuron.at(p2).y - listNeuron.at(p1).y, listNeuron.at(p2).z - listNeuron.at(p1).z);
            curAngle.normAngle();
            angleChangeMean += acos(lastAngle.dot(curAngle));
            lastAngle.setXYZ(curAngle);
        }
        length += dis(listNeuron.at(p1),listNeuron.at(p2));
        if(dis(listNeuron.at(p1),listNeuron.at(p2)) > 1){
            x = (int) ((listNeuron.at(p1).x + listNeuron.at(p2).x)/2 + 0.5);
            y = (int) ((listNeuron.at(p1).y + listNeuron.at(p2).y)/2 + 0.5);
            z = (int) ((listNeuron.at(p1).z + listNeuron.at(p2).z)/2 + 0.5);
            if(x >= sz[0]) x = sz[0] - 1;
            if(x < 0) x = 0;
            if(y >= sz[1]) y = sz[1] - 1;
            if(y < 0) y = 0;
            if(z >= sz[2]) z = sz[2] - 1;
            if(z < 0) z = 0;
            index = z*sz[0]*sz[1] + y*sz[0] + x;
            curIntensity = data1d[index];
            intensityMean += curIntensity;
            gradientMean += abs(curIntensity - lastIntensity);
            lastIntensity = curIntensity;
            count++;
        }
        x = (int) (listNeuron.at(p2).x + 0.5);
        y = (int) (listNeuron.at(p2).y + 0.5);
        z = (int) (listNeuron.at(p2).z + 0.5);
        if(x >= sz[0]) x = sz[0] - 1;
        if(x < 0) x = 0;
        if(y >= sz[1]) y = sz[1] - 1;
        if(y < 0) y = 0;
        if(z >= sz[2]) z = sz[2] - 1;
        if(z < 0) z = 0;
        index = z*sz[0]*sz[1] + y*sz[0] + x;
        curIntensity = data1d[index];
        intensityMean += curIntensity;
        gradientMean += abs(curIntensity - lastIntensity);
        lastIntensity = curIntensity;
        count++;

        if(compute_win3d_pca(data3d,sz[0],sz[1],sz[2],x,y,z,rx,ry,rz,pc1,pc2,pc3,1,false)){
            sigma12 += pc1/pc2;
            pcaCount++;
        }

        p1 = p2;
    }

//    cout<<"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"<<endl;

    if(count>0)
        intensityMean /= (double) count;
    if(length>0)
        angleChangeMean /= length;
    if(count - 1>0)
        gradientMean /= (double) (count-1);

    if(pcaCount>0){
        sigma12 /= (double) pcaCount;
        sigma13 /= (double) pcaCount;
    }

//    cout<<"ccccccccccccccccccccccccccccccccccccccccccccc"<<endl;

    x = (int) (listNeuron.at(headPointIndex).x + 0.5);
    y = (int) (listNeuron.at(headPointIndex).y + 0.5);
    z = (int) (listNeuron.at(headPointIndex).z + 0.5);
    if(x >= sz[0]) x = sz[0] - 1;
    if(x < 0) x = 0;
    if(y >= sz[1]) y = sz[1] - 1;
    if(y < 0) y = 0;
    if(z >= sz[2]) z = sz[2] - 1;
    if(z < 0) z = 0;
    index = z*sz[0]*sz[1] + y*sz[0] + x;
    intensityStd += pow((data1d[index]-intensityMean),2);

    p1 = headPointIndex;

    for(int i=1; i<pointsIndex.size(); i++){
        V3DLONG p2 = pointsIndex.at(i);
        if(dis(listNeuron.at(p1),listNeuron.at(p2)) > 1){
            x = (int) ((listNeuron.at(p1).x + listNeuron.at(p2).x)/2 + 0.5);
            y = (int) ((listNeuron.at(p1).y + listNeuron.at(p2).y)/2 + 0.5);
            z = (int) ((listNeuron.at(p1).z + listNeuron.at(p2).z)/2 + 0.5);
            if(x >= sz[0]) x = sz[0] - 1;
            if(x < 0) x = 0;
            if(y >= sz[1]) y = sz[1] - 1;
            if(y < 0) y = 0;
            if(z >= sz[2]) z = sz[2] - 1;
            if(z < 0) z = 0;
            index = z*sz[0]*sz[1] + y*sz[0] + x;
            intensityStd += pow((data1d[index]-intensityMean),2);
        }
        x = (int) (listNeuron.at(p2).x + 0.5);
        y = (int) (listNeuron.at(p2).y + 0.5);
        z = (int) (listNeuron.at(p2).z + 0.5);
        if(x >= sz[0]) x = sz[0] - 1;
        if(x < 0) x = 0;
        if(y >= sz[1]) y = sz[1] - 1;
        if(y < 0) y = 0;
        if(z >= sz[2]) z = sz[2] - 1;
        if(z < 0) z = 0;
        index = z*sz[0]*sz[1] + y*sz[0] + x;
        intensityStd += pow((data1d[index]-intensityMean),2);
        p1 = p2;
    }
    if(count>0)
        intensityStd = sqrt(intensityStd/count);

//    cout<<"dddddddddddddddddddddddddddddddddddddddddddddddddddd"<<endl;

    float imageMean = 0 , imageStd = 0;
    V3DLONG totalSz = sz[0]*sz[1]*sz[2];
    for(V3DLONG i=0; i<totalSz; i++){
        imageMean += data1d[i];
    }
    if(totalSz>0)
        imageMean /= (double) totalSz;

    for(V3DLONG i=0; i<totalSz; i++){
        imageStd += (data1d[i] - imageMean) * (data1d[i] - imageMean);
    }

    if(totalSz>0)
        imageStd = sqrt(imageStd/(double) totalSz);

    if(imageMean>0){
        intensityRatioToGlobal = intensityMean/imageMean;
    }else {
        intensityRatioToGlobal = 0;
    }

//    cout<<"eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"<<endl;

    NeuronTree b1 = NeuronTree();
    for(int i=0; i<pointsIndex.size(); i++){
        NeuronSWC tmp = listNeuron.at(pointsIndex.at(i));
//        tmp.r = 1;
        b1.listNeuron.push_back(tmp);
        b1.hashNeuron.insert(tmp.n,i);
    }
//    cout<<"1111111111111111111111111111111"<<endl;

    cout<<"imagemean: "<<imageMean<<" imagestd: "<<imageStd<<endl;

    vector<MyMarker*> markers1 = swc_convert(b1);
    double thres = 40;
    thres = (thres>(imageMean+0.5*imageStd))?thres:(imageMean+0.5*imageStd);
    for(int i=0; i<markers1.size(); i++){
        markers1[i]->radius = markerRadius(data1d,sz,*(markers1[i]),thres);
//        cout<<i<<" : "<<markers1[i]->radius<<endl;
    }

//    cout<<"2222222222222222222222222222222"<<endl;

    unsigned char* maskR1 = 0;
    swcTomask(maskR1,markers1,sz[0],sz[1],sz[2]);

//    cout<<"*******************************************************"<<endl;

    NeuronTree b5 = NeuronTree();
    for(int i=0; i<pointsIndex.size(); i++){
        NeuronSWC tmp = b1.listNeuron.at(i);
        tmp.r = 5;
        b5.listNeuron.push_back(tmp);
        b5.hashNeuron.insert(tmp.n,i);
    }
    vector<MyMarker*> markers5 = swc_convert(b5);
    unsigned char* maskR5 = 0;
    swcTomask(maskR5,markers5,sz[0],sz[1],sz[2]);

    float intensityR1 = 0, intensityR5 = 0;
    int countR1 = 0, countR5 = 0;
    for(int i=0; i<totalSz; i++){
        if(maskR1[i] > 0){
            intensityR1 += data1d[i];
            countR1++;
        }
        if(maskR5[i] > 0){
            intensityR5 += data1d[i];
            countR5++;
        }
    }
    if(maskR1)
        delete[] maskR1;
    if(maskR5)
        delete[] maskR5;

    if(countR1>0)
        intensityR1 /= countR1;
    if(countR5>0)
        intensityR5 /= countR5;

    if(intensityR5>0){
        intensityRationToLocal = intensityR1/intensityR5;
    }else {
        intensityRationToLocal = 0;
    }

    delete3dpointer(data3d,sz[0],sz[1],sz[2]);

//    cout<<"ffffffffffffffffffffffffffffffffffffffffffffffffff"<<endl;

    return true;

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
        cout<<"child size: "<<child.size()<<endl;
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
