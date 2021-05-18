#include "branchtree.h"

#include "algorithm";
#include "fastmarching_tree.h"
#include "neuron_sim_scores.h"

#include "../../../released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h"

#include "swc_convert.h"

bool Branch::get_r_pointsIndex_of_branch(vector<long long> &r_points){
    int tmp = endPointIndex;
    r_points.push_back(tmp);
//    qDebug()<<"in get_r_pointsIndex_of_branch";
    while(tmp != headPointIndex)
    {
        tmp = this->bt->nt.hashNeuron.value(this->bt->nt.listNeuron[tmp].parent);
        r_points.push_back(tmp);
    }
    return true;
}

bool Branch::get_pointsIndex_of_branch(vector<long long> &points){
    this->get_r_pointsIndex_of_branch(points);
    reverse(points.begin(),points.end());
    return true;
}

void Branch::setBranchType(int type, bool isBreak){
    vector<V3DLONG> pointsIndex;
    this->get_pointsIndex_of_branch(pointsIndex);
    int j = 1;
    if(isBreak){
        for(int i=1; i<pointsIndex.size(); ++i){
            V3DLONG index = pointsIndex[i];
            if(this->bt->nt.listNeuron[index].type == BreakType){
                j = i;
                break;
            }
        }
    }
    for(; j<pointsIndex.size(); ++j){
        V3DLONG index = pointsIndex[j];
        this->bt->nt.listNeuron[index].type = type;
    }
}

void Branch::findBranchInflectionPoint(double d, double cosAngleThres){
    qDebug()<<"in findInflectionPoint";

    NeuronTree& nt = this->bt->nt;

    vector<V3DLONG> pointsIndex = vector<V3DLONG>();
    this->get_pointsIndex_of_branch(pointsIndex);
    int pointsSize = pointsIndex.size();
    double* path = new double[pointsSize];
    memset(path,0,sizeof(double)*pointsSize);

    if(pointsSize<2){
        return;
    }

    for(int i=1; i<pointsSize; i++){
        double tmpD = zx_dist(nt.listNeuron[pointsIndex[i]],nt.listNeuron[pointsIndex[i-1]]);
        path[i] = path[i-1] + tmpD;
    }

    XYZ v1,v2;
    XYZ p1,p2;
    p1 = XYZ(nt.listNeuron[pointsIndex[0]].x,
            nt.listNeuron[pointsIndex[0]].y,
            nt.listNeuron[pointsIndex[0]].z);

    double cosAngle;
    int startPoint = 0;
    int i,j,k;

    int ppIndex,ccIndex,curIndex;
    XYZ pp1,pp2;

    for(i=0; i<pointsSize;){
        for(j=i+1;j<pointsSize;j++){
            if(path[j]-path[i]>d){
                p2 = XYZ(nt.listNeuron[pointsIndex[j]].x,
                        nt.listNeuron[pointsIndex[j]].y,
                        nt.listNeuron[pointsIndex[j]].z);
                if(i==startPoint){
                    v1 = p2-p1;
                }else {
                    v2 = p2-p1;
                    cosAngle = dot(normalize(v1),normalize(v2));
                    if(cosAngle<cosAngleThres){
                        for(k=i; k<=j; k++){
                            nt.listNeuron[pointsIndex[k]].type = 5;
                        }
                        double l = path[j]-path[startPoint];
                        double cosAngle = 1;
                        int index = -1;
                        for(k=startPoint+1; k<j; k++){
                            if((path[k]-path[startPoint])<d || (path[j]-path[k])<d){
                                continue;
                            }
                            curIndex = pointsIndex[k];
                            ppIndex = pointsIndex[startPoint];
                            ccIndex = pointsIndex[j];
                            for(int ki=k-1; ki>=startPoint; ki--){
                                if(path[k]-path[ki]>d){
                                    ppIndex = pointsIndex[ki];
                                    break;
                                }
                            }
                            for(int ki=k+1; ki<=j; ki++){
                                if(path[ki]-path[k]>d){
                                    ccIndex = pointsIndex[ki];
                                    break;
                                }
                            }
                            pp1 = XYZ(nt.listNeuron[curIndex].x - nt.listNeuron[ppIndex].x,
                                     nt.listNeuron[curIndex].y - nt.listNeuron[ppIndex].y,
                                     nt.listNeuron[curIndex].z - nt.listNeuron[ppIndex].z);
                            pp2 = XYZ(nt.listNeuron[ccIndex].x - nt.listNeuron[curIndex].x,
                                     nt.listNeuron[ccIndex].y - nt.listNeuron[curIndex].y,
                                     nt.listNeuron[ccIndex].z - nt.listNeuron[curIndex].z);
                            double tmpCosAngle = dot(normalize(pp1),normalize(pp2));
                            if(tmpCosAngle<cosAngle){
                                cosAngle = tmpCosAngle;
                                index = k;
                            }
                        }
                        if(cosAngle>(sqrt(2.0)/2) || index == -1){
                            index = (j+startPoint)/2;
                        }
                        nt.listNeuron[pointsIndex[index]].type = InFlectionType;

                        startPoint = j;
                    }
                }
                p1 = p2;
                break;
            }
        }
        i = j;
    }

    if(path){
        delete[] path;
        path = 0;
    }
}

bool Branch::checkBranchInflectionPoint(unsigned char *inimg1d, long long *sz){
    qDebug()<<"in checkInflectionPoint";
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];

    NeuronTree& nt = this->bt->nt;
    vector<Branch>& branches = this->bt->branches;
    V3DLONG somaIndex = this->bt->somaIndex;

    vector<V3DLONG> pointsIndex = vector<V3DLONG>();
    this->get_pointsIndex_of_branch(pointsIndex);
    int pointsSize = pointsIndex.size();
    double* path = new double[pointsSize];
    memset(path,0,sizeof(double)*pointsSize);

    if(pointsSize<2 || zx_dist(nt.listNeuron[somaIndex],nt.listNeuron[pointsIndex[0]])<nt.listNeuron[somaIndex].radius){
        return true;
    }

    for(int i=1; i<pointsSize; i++){
        double tmpD = zx_dist(nt.listNeuron[pointsIndex[i]],nt.listNeuron[pointsIndex[i-1]]);
        path[i] = path[i-1] + tmpD;
    }

    if(path[pointsSize-1]<20){
        return true;
    }

    for(int i=1; i<pointsSize; i++){
        int index = pointsIndex[i];
        if(nt.listNeuron[index].type == InFlectionType){
            qDebug()<<"there is a inflectionPoint";
            int startI = 0;
            int endI = pointsSize-1;
            for(int j=i-1; j>=0; j--){
                if((path[i]-path[j])>10){
                    startI = j;
                    break;
                }
            }
            for(int j=i+1; j<pointsSize; j++){
                if((path[j] - path[i])>10){
                    endI = j;
                    break;
                }
            }
            qDebug()<<"startI: "<<startI<<" endI: "<<endI;
            MyMarker root = MyMarker(nt.listNeuron[pointsIndex[endI]].x,
                                     nt.listNeuron[pointsIndex[endI]].y,
                                     nt.listNeuron[pointsIndex[endI]].z);
            vector<MyMarker*> outLine;
            fastmarching_tree_constraint(root,inimg1d,outLine,sz[0],sz[1],sz[2],3,0,false,25);
            qDebug()<<"end fastmartching_tree";
            XYZ p1 = XYZ(nt.listNeuron[index].x - root.x,
                         nt.listNeuron[index].y - root.y,
                         nt.listNeuron[index].z - root.z);
            XYZ p2 = XYZ(outLine[outLine.size()/2]->x - root.x,
                    outLine[outLine.size()/2]->y - root.y,
                    outLine[outLine.size()/2]->z - root.z);
            qDebug()<<"outLine size: "<<outLine.size();
            double tmpCosAngle = dot(normalize(p1),normalize(p2));

            qDebug()<<"tmpCosAngle: "<<tmpCosAngle;

//            NeuronTree outswcBefore = swc_convert(outLine);
//            QString savePathBefore = QString("F:\\manual\\") + QString::number(this->endPointIndex) + "fastmartching_before.swc";
//            writeSWC_file(savePathBefore,outswcBefore);

            if(tmpCosAngle<0.5){
                vector<MyMarker*> maskMarkers;
                maskMarkers.insert(maskMarkers.begin(),outLine.begin(),outLine.begin()+outLine.size()/2);
                unsigned char* maskFlag = 0;
                for(int j=0; j<maskMarkers.size(); ++j){
                    maskMarkers[j]->radius = 8;
                }
                swc2mask(maskFlag,maskMarkers,sz[0],sz[1],sz[2]);
                qDebug()<<"mask end";

                for(long j=0; j<tolSZ; j++){
                    if(maskFlag[j] == (unsigned char)255){
                        maskFlag[j] = 0;
                    }else{
                        maskFlag[j] = inimg1d[j];
                    }
                }
                outLine.clear();
                fastmarching_tree_constraint(root,maskFlag,outLine,sz[0],sz[1],sz[2],3,0,false,25);
                qDebug()<<"end fastmartching_tree 2";
                if(maskFlag){
                    delete[] maskFlag;
                    maskFlag = 0;
                }
//                NeuronTree outswc = swc_convert(outLine);
//                QString savePath = QString("F:\\manual\\") + QString::number(this->endPointIndex) + "fastmartching.swc";
//                writeSWC_file(savePath,outswc);
            }

            NeuronTree* origin = new NeuronTree();
            for(int j=1; j<=endI; ++j){
                NeuronSWC s = nt.listNeuron[pointsIndex[j]];
//                if(j == 0){
//                    s.parent = -1;
//                }
                origin->listNeuron.push_back(s);
            }
            int parentIndex = this->parentIndex;
            int count = 0;
            if(parentIndex != -1){
                vector<V3DLONG> tmpPointsIndex;
                tmpPointsIndex.clear();
                branches[parentIndex].get_pointsIndex_of_branch(tmpPointsIndex);
                for(int j=0; j<tmpPointsIndex.size(); ++j){
                    NeuronSWC s = nt.listNeuron[tmpPointsIndex[j]];
                    if(j == 0){
                        s.parent = -1;
                    }
                    origin->listNeuron.push_back(s);
                }
                queue<int> branchQ;
                for(int j=0; j<branches[parentIndex].childrenIndex.size(); j++){
                    int cIndex = branches[parentIndex].childrenIndex[j];
                    tmpPointsIndex.clear();
                    branches[cIndex].get_pointsIndex_of_branch(tmpPointsIndex);
                    if(tmpPointsIndex.size()>1 && tmpPointsIndex[1] == pointsIndex[1]){
                        continue;
                    }
                    branchQ.push(cIndex);
                }


                while (!branchQ.empty() && count<5) {
                    int tmpBranchIndex = branchQ.front();
                    count++;
                    branchQ.pop();
                    tmpPointsIndex.clear();
                    branches[tmpBranchIndex].get_pointsIndex_of_branch(tmpPointsIndex);
                    for(int j=1; j<tmpPointsIndex.size(); ++j){
                        NeuronSWC s = nt.listNeuron[tmpPointsIndex[j]];
                        origin->listNeuron.push_back(s);
                    }
                    for(int j=0; j<branches[tmpBranchIndex].childrenIndex.size(); ++j){
                        int cIndex = branches[tmpBranchIndex].childrenIndex[j];
                        branchQ.push(cIndex);
                    }
                }
            }




            qDebug()<<"origin size: "<<origin->listNeuron.size();
            if(outLine.size()<1)
                return true;


            XYZ pt;
            double targetD = 0;
            count = 0;
            for(int j=0; j<outLine.size()/2; ++j){
                pt = XYZ(outLine[j]->x,outLine[j]->y,outLine[j]->z);
                targetD += dist_pt_to_swc(pt,origin);
                count++;
            }
            if(count>0){
                targetD /= count;
            }

            qDebug()<<"targetD: "<<targetD;
            if(targetD>2){
                for(int j=0; j<pointsSize; j++){
                    if(j <= i)
                        continue;
                    nt.listNeuron[pointsIndex[j]].type = ToDeleteType;
                }
                if(origin)
                    delete origin;
                return false;
            }
            if(origin)
                delete origin;
        }
    }
    return true;
}

bool Branch::calAngle(){
    if(this->childrenIndex.size()<2){
        return false;
    }
    XYZ v1,v2,v3;
    double cosAngle1,cosAngle2,cosAngle3;
    double angle1, angle2, angle3;


    int d = 5;

    int cIndex1 = this->childrenIndex[0];
    int cIndex2 = this->childrenIndex[1];

    vector<V3DLONG> pointsIndex = vector<V3DLONG>();
    this->get_r_pointsIndex_of_branch(pointsIndex);
    v1 = this->bt->getBranchLocalVector(pointsIndex,d);

    pointsIndex.clear();
    this->bt->branches[cIndex1].get_pointsIndex_of_branch(pointsIndex);
    v2 = this->bt->getBranchLocalVector(pointsIndex,d);

    pointsIndex.clear();
    this->bt->branches[cIndex2].get_pointsIndex_of_branch(pointsIndex);
    v3 = this->bt->getBranchLocalVector(pointsIndex,d);

    cosAngle1 = dot(normalize(v1),normalize(v2));
    cosAngle2 = dot(normalize(v1),normalize(v3));
    cosAngle3 = dot(normalize(v2),normalize(v3));

    angle1 = (acos(cosAngle1)/PI)*180;
    angle2 = (acos(cosAngle2)/PI)*180;
    angle3 = (acos(cosAngle3)/PI)*180;

    this->localAngle1 = angle1;
    this->localAngle2 = angle2;
    this->localAngle3 = angle3;

    return true;
}

float Branch::getBranchIntensity(unsigned char *pdata, long long *sz){
    vector<V3DLONG> pointsIndex;
    this->get_pointsIndex_of_branch(pointsIndex);
    int size = pointsIndex.size();
    intensityMean = intensityStd = 0;
    if(size == 0){
        return intensityMean;
    }
    int x,y,z;
    V3DLONG index;
    for(int i=0; i<size; ++i){
        index = pointsIndex[i];
        x = this->bt->nt.listNeuron[index].x + 0.5;
        if(x>=sz[0]) x = sz[0]-1;
        y = this->bt->nt.listNeuron[index].y + 0.5;
        if(y>=sz[1]) y = sz[1]-1;
        z = this->bt->nt.listNeuron[index].z + 0.5;
        if(z>=sz[2]) z = sz[2]-1;
        intensityMean += pdata[z*sz[0]*sz[1]+y*sz[0]+x];
    }
    intensityMean /= size;

    for(int i=0; i<size; ++i){
        index = pointsIndex[i];
        x = this->bt->nt.listNeuron[index].x + 0.5;
        if(x>=sz[0]) x = sz[0]-1;
        y = this->bt->nt.listNeuron[index].y + 0.5;
        if(y>=sz[1]) y = sz[1]-1;
        z = this->bt->nt.listNeuron[index].z + 0.5;
        if(z>=sz[2]) z = sz[2]-1;
        intensityStd += pow((pdata[z*sz[0]*sz[1]+y*sz[0]+x] - intensityMean),2);
    }

    intensityStd = sqrt(intensityMean/size);

    return intensityMean;

}

float Branch::getBranchPcaValue(unsigned char *pdata, long long *sz, int r){

    double *vec1 = new double[3];
    double *vec2 = new double[3];
    double *vec3 = new double[3];

    vector<V3DLONG> pointsIndex;
    this->get_pointsIndex_of_branch(pointsIndex);
    int size = pointsIndex.size();

    if(size<3)
        return -1;

    float sum = 0, minS = INT_MAX, maxS = 0;

    int x,y,z;
    V3DLONG index;
    double pc1,pc2,pc3;

    for(int i=0; i<size; ++i){
        index = pointsIndex[i];
        x = this->bt->nt.listNeuron[index].x + 0.5;
        if(x>=sz[0]) x = sz[0]-1;
        y = this->bt->nt.listNeuron[index].y + 0.5;
        if(y>=sz[1]) y = sz[1]-1;
        z = this->bt->nt.listNeuron[index].z + 0.5;
        if(z>=sz[2]) z = sz[2]-1;
        computeCubePcaEigVec(pdata,sz,x,y,z,r,r,r,pc1,pc2,pc3,vec1,vec2,vec3);
        float tmp = pc1/pc2;
        maxS = tmp>maxS ? tmp : maxS;
        minS = tmp<minS ? tmp : minS;
        sum += tmp;
    }


    if(vec1){
        delete[] vec1; vec1 = 0;
    }
    if(vec2){
        delete[] vec2; vec2 = 0;
    }
    if(vec3){
        delete[] vec3; vec3 = 0;
    }

    return (sum-minS-maxS)/(size-2);

}

double Branch::distToNeuronTree(NeuronTree* otherTree){
    NeuronTree& nt = this->bt->nt;
    vector<V3DLONG> pointsIndex;
    this->get_pointsIndex_of_branch(pointsIndex);
    double dToTree = 0;
    for(int i=1; i<pointsIndex.size()-1; ++i){
        V3DLONG index = pointsIndex[i];
        XYZ pt = XYZ(nt.listNeuron[index].x,nt.listNeuron[index].y,nt.listNeuron[index].z);
        double tmp = dist_pt_to_swc(pt,otherTree);
        dToTree += tmp;
    }
    if(pointsIndex.size()>2){
        return dToTree/(pointsIndex.size()-2);
    }else{
        return 0;
    }
}

int Branch::distToNeuronTree2(NeuronTree *otherTree, double dTh){
    NeuronTree& nt = this->bt->nt;
    vector<V3DLONG> pointsIndex;
    this->get_pointsIndex_of_branch(pointsIndex);
    double dToTree = 0;
    for(int i=1; i<pointsIndex.size()-1; ++i){
        V3DLONG index = pointsIndex[i];
        XYZ pt = XYZ(nt.listNeuron[index].x,nt.listNeuron[index].y,nt.listNeuron[index].z);
        double tmp = dist_pt_to_swc(pt,otherTree);
        dToTree += tmp;
    }
    if(pointsIndex.size()>2){
        double d = dToTree/(pointsIndex.size()-2);
        if(d<dTh){
            return 0;
        }else{
            for(int i=0; i<pointsIndex.size()-1; ++i){
                V3DLONG index = pointsIndex[i];
                XYZ pt = XYZ(nt.listNeuron[index].x,nt.listNeuron[index].y,nt.listNeuron[index].z);
                double tmp = dist_pt_to_swc(pt,otherTree);
                if(tmp>dTh*1.5){
                    if(zx_dist(nt.listNeuron[index],nt.listNeuron[pointsIndex[0]])>10)
                        return index;
                    else
                        return -1;
                }
            }
            return pointsIndex[pointsIndex.size()-1];
        }
    }else{
        return 0;

    }
}

int Branch::distToNeuronTree3(BranchTree otherbt, NeuronTree* otherTree, double dTh, int& pmbi){
    NeuronTree& nt = this->bt->nt;
    vector<V3DLONG> pointsIndex;
    this->get_pointsIndex_of_branch(pointsIndex);
    double dToTree = 0;

    vector<NeuronTree*> otherSegments;
    vector<V3DLONG> tmpPointsIndex;
    for(int i=0; i<otherbt.branches.size(); ++i){
        NeuronTree* segment = new NeuronTree();
        tmpPointsIndex.clear();
        otherbt.branches[i].get_pointsIndex_of_branch(tmpPointsIndex);
        for(int j=0; j<tmpPointsIndex.size(); ++j){
            NeuronSWC s = otherbt.nt.listNeuron[tmpPointsIndex[j]];
            if(j == 0)
                s.parent = -1;
            segment->listNeuron.push_back(s);
        }
        otherSegments.push_back(segment);
    }
//    qDebug()<<"start compare";

    map<int,int,greater<int>> segmentMap;
    for(int i=1; i<pointsIndex.size()-1; ++i){
        V3DLONG index = pointsIndex[i];
        XYZ pt = XYZ(nt.listNeuron[index].x,nt.listNeuron[index].y,nt.listNeuron[index].z);

        float dmin = 1000000;
        int indexMin = -1;
        for(int j=0; j<otherSegments.size(); ++j){
            float dtmp = dist_pt_to_swc(pt,otherSegments[j]);
            if(dtmp<dmin){
                dmin = dtmp;
                indexMin = j;
            }
        }
        if(segmentMap.find(indexMin) == segmentMap.end()){
            segmentMap[indexMin] = 1;
        }else {
            segmentMap[indexMin]++;
        }
        dToTree += dmin;
    }

//    qDebug()<<"compare end";

    for(int i=0; i<otherSegments.size(); ++i){
        delete otherSegments[i];
    }

    if(pointsIndex.size()>2){
        double d = dToTree/(pointsIndex.size()-2);
//        qDebug()<<nt.listNeuron[endPointIndex].n<<" d: "<<d;
        if(d < dTh){
            if(parentIndex == -1 || segmentMap.empty()){
                return 0;
            }
//            qDebug()<<"segment map size:"<<segmentMap.size();



            map<int,int>::iterator it = segmentMap.begin();
            int segmentIndex = it->first;
            int countMax = it->second;
            while (it != segmentMap.end()) {
//                qDebug()<<"it: "<<it->first<<" "<<it->second;
                if(it->second>countMax){
                    segmentIndex = it->first;
                    countMax = it->second;
                }
                it++;
            }
            if(pmbi == segmentIndex){
                return 0;
            }
            pmbi = segmentIndex;

//            qDebug()<<"segmentIndex: "<<segmentIndex<<" "<<countMax;

            NeuronTree* parentTree = new NeuronTree();
            int otherParentIndex = otherbt.branches[segmentIndex].parentIndex;
            if(this->length > otherbt.branches[segmentIndex].length*0.5)
                otherParentIndex = otherbt.branches[segmentIndex].parentIndex;
            while(otherParentIndex != -1){
                tmpPointsIndex.clear();
                otherbt.branches[otherParentIndex].get_pointsIndex_of_branch(tmpPointsIndex);
                for(int j=0; j<tmpPointsIndex.size(); ++j){
                    NeuronSWC s = otherbt.nt.listNeuron[tmpPointsIndex[j]];
                    parentTree->listNeuron.push_back(s);
                }
                otherParentIndex = otherbt.branches[otherParentIndex].parentIndex;
            }
//            qDebug()<<"parent Tree size: "<<parentTree->listNeuron.size();
            if(parentTree->listNeuron.size()<2){
                delete parentTree;
                return 0;
            }

            float dp = 0;
            tmpPointsIndex.clear();
            this->bt->branches[parentIndex].get_pointsIndex_of_branch(tmpPointsIndex);
            for(int j=0; j<tmpPointsIndex.size(); ++j){
                V3DLONG index = tmpPointsIndex[j];
                XYZ pt = XYZ(nt.listNeuron[index].x,nt.listNeuron[index].y,nt.listNeuron[index].z);
                float dptmp = dist_pt_to_swc(pt,parentTree);
                dp += dptmp;
            }
//            qDebug()<<"tmpPoints size: "<<tmpPointsIndex.size();
            dp /= tmpPointsIndex.size();
//            qDebug()<<"dp: "<<dp;
            if(parentTree)
                delete parentTree;
            if(dp < dTh)
                return 0;
            else
                return -1;

        }else{
            for(int i=0; i<pointsIndex.size()-1; ++i){
                V3DLONG index = pointsIndex[i];
                XYZ pt = XYZ(nt.listNeuron[index].x,nt.listNeuron[index].y,nt.listNeuron[index].z);
                double tmp = dist_pt_to_swc(pt,otherTree);
                if(tmp>dTh*1.5){
                    if(zx_dist(nt.listNeuron[index],nt.listNeuron[pointsIndex[0]])>10)
                        return index;
                    else
                        return -1;
                }
            }
            return pointsIndex[pointsIndex.size()-1];
        }
    }else{
        return 0;
    }

}



bool BranchTree::initialize(NeuronTree t){
    branches.clear();
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
    if(ori.size() != 1){
        return false;
    }else{
        somaIndex = ori[0];
        queue.push_back(somaIndex);
    }
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
            Branch branch = Branch();
            branch.headPointIndex = tmp;
            int cIndex = child.at(i);
            branch.length += zx_dist(nt.listNeuron[cIndex],nt.listNeuron[tmp]);

            while(children.at(cIndex).size() == 1)
            {
                int curIndex = cIndex;
                cIndex = children.at(curIndex).at(0);
                branch.length += zx_dist(nt.listNeuron[cIndex],nt.listNeuron[curIndex]);
            }
            if(children.at(cIndex).size() >= 1)
            {
                queue.push_back(cIndex);
            }
            branch.endPointIndex = cIndex;
            branch.distance = zx_dist(nt.listNeuron[branch.headPointIndex],nt.listNeuron[branch.endPointIndex]);
            branch.bt = this;
            branches.push_back(branch);
        }
    }

    //initial parent
    cout<<"initial parent"<<endl;
    cout<<"branch size: "<<branches.size()<<endl;
    for(int i=0;i<branches.size();++i)
    {
//        cout<<i<<endl;
        if(nt.listNeuron.at(branches[i].headPointIndex).parent < 0)
        {
            branches[i].parentIndex = -1;
        }
        else
        {
            for(int j=0; j<branches.size(); ++j)
            {
                if(branches[i].headPointIndex == branches[j].endPointIndex)
                {
                    branches[i].parentIndex = j;
                    break;
                }
            }
        }
    }

    //initial level
    for(int i=0; i<branches.size(); ++i)
    {
        int tmp = i;
        int level=0;
        while(branches[tmp].parentIndex != -1)
        {
            level++;
            tmp = branches[tmp].parentIndex;
        }
        branches[i].level=level;
    }

    //initial children rlevel


    for(int i=0; i<branches.size(); ++i){
        int prtIndex = branches[i].parentIndex;
//        cout<<"i: "<<i<<" prtIndex: "<<prtIndex<<endl;
        if(prtIndex != -1){
            branches[prtIndex].childrenIndex.push_back(i);
        }
    }

    for(int i=0;i<branches.size(); ++i){
        if(branches[i].childrenIndex.size() == 0){
            int rLevel = 0;
            int tmp = i;
            branches[tmp].rLevel = rLevel;
            while(branches[tmp].parentIndex != -1){
                rLevel++;
                tmp = branches[tmp].parentIndex;
                if(branches[tmp].rLevel<rLevel){
                    branches[tmp].rLevel = rLevel;
                }
            }

        }
    }

    cout<<"--------------------initialize end--------------"<<endl;

    return true;
}

XYZ BranchTree::getBranchLocalVector(vector<long long> pointsIndex, double d){
    XYZ p1,p2;
    int pointsSize = pointsIndex.size();
//    qDebug()<<"n: "<<nt.listNeuron[pointsIndex[0]].n;
    double* path = new double[pointsSize];
    memset(path,0,sizeof(double)*pointsSize);
    double length = 0;
    for(int i=1; i<pointsSize; i++){
        double tmpD = zx_dist(nt.listNeuron[pointsIndex[i]],nt.listNeuron[pointsIndex[i-1]]);
        path[i] = path[i-1] + tmpD;
        length += tmpD;
    }

    if(length<d){
        p1 = XYZ(nt.listNeuron[pointsIndex[0]].x,
                nt.listNeuron[pointsIndex[0]].y,
                nt.listNeuron[pointsIndex[0]].z);
        p2 = XYZ(0,0,0);
        for(int i=1; i<pointsSize; i++){
            p2.x += nt.listNeuron[pointsIndex[i]].x;
            p2.y += nt.listNeuron[pointsIndex[i]].y;
            p2.z += nt.listNeuron[pointsIndex[i]].z;
        }
        if(pointsSize-1>0){
            p2.x /= (double)(pointsSize-1);
            p2.y /= (double)(pointsSize-1);
            p2.z /= (double)(pointsSize-1);
        }else{
            if(path){
                delete[] path;
                path = 0;
            }
            return XYZ(0,0,0);
        }
        if(path){
            delete[] path;
            path = 0;
        }
        return p2-p1;
    }else {
        p1 = XYZ(nt.listNeuron[pointsIndex[0]].x,
                nt.listNeuron[pointsIndex[0]].y,
                nt.listNeuron[pointsIndex[0]].z);
        int count =0;
        p2 = XYZ(0,0,0);
        for(int i=1; i<pointsSize; i++){
            if(path[i]<d){
                continue;
            }
            count++;
            p2.x += nt.listNeuron[pointsIndex[i]].x;
            p2.y += nt.listNeuron[pointsIndex[i]].y;
            p2.z += nt.listNeuron[pointsIndex[i]].z;
            if(nt.listNeuron[pointsIndex[i]].type == InFlectionType && path[i]>d){
//                qDebug()<<"path[i]: "<<path[i];
                break;
            }
        }

        if(count>0){
            p2.x /= (double)count;
            p2.y /= (double)count;
            p2.z /= (double)count;
        }else {
            if(path){
                delete[] path;
                path = 0;
            }
            return XYZ(0,0,0);
        }
//        qDebug()<<"p2 x y z"<<p2.x<<" "<<p2.y<<" "<<p2.z;
        if(path){
            delete[] path;
            path = 0;
        }
        return p2-p1;
    }

}

XYZ BranchTree::getBranchGlobalVector(vector<long long> pointsIndex){
    XYZ p1,p2;
    int pointsSize = pointsIndex.size();

    p1 = XYZ(nt.listNeuron[pointsIndex[0]].x,
            nt.listNeuron[pointsIndex[0]].y,
            nt.listNeuron[pointsIndex[0]].z);
    p2 = XYZ(nt.listNeuron[pointsIndex[pointsSize-1]].x,
            nt.listNeuron[pointsIndex[pointsSize-1]].y,
            nt.listNeuron[pointsIndex[pointsSize-1]].z);
    return p2-p1;
}

bool BranchTree::setChildenBranchType(int branchIndex, int type){
    queue<int> branchDQ;
    for(int j=0; j<branches[branchIndex].childrenIndex.size(); ++j){
        int cIndex = branches[branchIndex].childrenIndex[j];
        branchDQ.push(cIndex);
    }
    while (!branchDQ.empty()) {
        int branchDIndex = branchDQ.front();
//        qDebug()<<"branchDIndex: "<<branchDIndex;
        branchDQ.pop();
        branches[branchDIndex].setBranchType(type);
        if(type == ToDeleteType){
            branches[branchDIndex].rLevel = -1;
        }
        for(int j=0; j<branches[branchDIndex].childrenIndex.size(); ++j){
            int cIndex = branches[branchDIndex].childrenIndex[j];
            branchDQ.push(cIndex);
        }
    }
    return true;
}

bool BranchTree::setChildenBranchDeleteType(int branchIndex){
    return setChildenBranchType(branchIndex,ToDeleteType);
}

bool BranchTree::setParentBranchRLevel(int branchIndex){
    int branchParentIndex = branches[branchIndex].parentIndex;
    while (branchParentIndex != -1) {
        int maxRLevel = -1;
        for(int j=0; j<branches[branchParentIndex].childrenIndex.size(); ++j){
            int cIndex = branches[branchParentIndex].childrenIndex[j];
            maxRLevel = maxRLevel>branches[cIndex].rLevel ? maxRLevel : branches[cIndex].rLevel;
        }
        branches[branchParentIndex].rLevel = maxRLevel + 1;
        branchParentIndex = branches[branchParentIndex].parentIndex;
    }
    return true;
}

bool BranchTree::getLevelIndex(vector<int> &levelIndex, int level){
    for(int i=0; i<branches.size(); ++i){
        if(branches[i].level == level){
            levelIndex.push_back(i);
        }
    }
    return true;
}

bool BranchTree::pruningByLength(unsigned char* pdata, V3DLONG* sz, int length, double linearityTh){
    bool isCompleted = false;
    while(!isCompleted){
        isCompleted = true;
        for(int i=0; i<branches.size(); ++i){
//            qDebug()<<i<<" : "<<branches[i].length;
            if(branches[i].rLevel == 0 && branches[i].length<length){
                isCompleted = false;
                branches[i].rLevel = -1;
                this->setParentBranchRLevel(i);
                branches[i].setBranchType(ToDeleteType);
            }
        }
    }
    this->update();
    for(int i=0; i<branches.size(); ++i){
//        qDebug()<<i<<" : "<<branches[i].length;
        if(branches[i].rLevel == 0 && branches[i].length<80){
            float pc12 = branches[i].getBranchPcaValue(pdata,sz,10);
            int prtIndex = branches[i].parentIndex;
            if(pc12<linearityTh || (prtIndex != -1 && branches[prtIndex].length<length)){
                branches[i].rLevel = -1;
                this->setParentBranchRLevel(i);
                branches[i].setBranchType(ToDeleteType);
            }
        }
    }
    this->update();
    isCompleted = false;
    while(!isCompleted){
        isCompleted = true;
        for(int i=0; i<branches.size(); ++i){
//            qDebug()<<i<<" : "<<branches[i].length;
            if(branches[i].rLevel == 0 && branches[i].length<length){
                isCompleted = false;
                branches[i].rLevel = -1;
                this->setParentBranchRLevel(i);
                branches[i].setBranchType(ToDeleteType);
            }
        }
    }
    return isCompleted;
}

bool BranchTree::pruningSoma(double times){
    double somaR = nt.listNeuron[somaIndex].r;
    bool isCompleted = false;
    while (!isCompleted) {
        isCompleted = true;
        for(int i=0; i<branches.size(); ++i){
            V3DLONG leafIndex = branches[i].endPointIndex;
            V3DLONG headIndex = branches[i].headPointIndex;
            double ratio = branches[i].length/branches[i].distance;
            if( branches[i].rLevel == 0 && ratio<1.2
                    && zx_dist(nt.listNeuron[headIndex],nt.listNeuron[somaIndex])<times*somaR/2
                    && zx_dist(nt.listNeuron[leafIndex],nt.listNeuron[somaIndex])<times*somaR){
                isCompleted = false;
                branches[i].rLevel = -1;
                this->setParentBranchRLevel(i);
                branches[i].setBranchType(ToDeleteType);
            }
        }
    }
    return isCompleted;
}

bool BranchTree::pruningAdjacentSoma(double somaRTh){
    qDebug()<<"------pruningAdjacentSoma------";
    vector<int> level0Index;
    this->getLevelIndex(level0Index,0);
    queue<int> branchQ;
    for(int i=0; i<level0Index.size(); ++i){
        int branchIndex = level0Index[i];
        if(branches[branchIndex].rLevel != -1){
            branchQ.push(branchIndex);
        }
    }

    double somaR = nt.listNeuron[somaIndex].r;

    vector<vector<int> > branchesIndexVector;

    while (!branchQ.empty()) {
        int branchIndex = branchQ.front();
        branchQ.pop();
        if(branches[branchIndex].rLevel == -1){
            continue;
        }
        vector<V3DLONG> pointsIndex;
        branches[branchIndex].get_pointsIndex_of_branch(pointsIndex);
        bool meetOtherSoma = false;
        for(int i=0; i<pointsIndex.size(); ++i){
            V3DLONG curIndex = pointsIndex[i];
            if(nt.listNeuron[curIndex].r>somaRTh &&
                    zx_dist(nt.listNeuron[somaIndex],nt.listNeuron[curIndex])>somaR*2){
                nt.listNeuron[curIndex].type = BreakType;
                branches[branchIndex].setBranchType(ToDeleteType,true);
                nt.listNeuron[curIndex].type = AdjacentSomaType;
                meetOtherSoma = true;
                vector<int> branchesIndex;
                branchesIndex.push_back(branchIndex);
                while (branches[branchIndex].parentIndex != -1) {
                    branchIndex = branches[branchIndex].parentIndex;
                    branchesIndex.push_back(branchIndex);
                }
                reverse(branchesIndex.begin(),branchesIndex.end());
                branchesIndexVector.push_back(branchesIndex);
                break;
            }
        }

        if(meetOtherSoma){
//            this->setChildenBranchDeleteType(branchIndex);
        }else{
            for(int j=0; j<branches[branchIndex].childrenIndex.size(); ++j){
                int cIndex = branches[branchIndex].childrenIndex[j];
                if(branches[cIndex].rLevel != -1){
                    branchQ.push(cIndex);
                }
            }
        }
    }

    qDebug()<<"other soma size: "<<branchesIndexVector.size();

    for(int i=0; i<branchesIndexVector.size(); ++i){
        int size = branchesIndexVector[i].size();
//        qDebug()<<"i: "<<i;
        vector<bool> flag = vector<bool>(size,false);
        for(int j=0; j<size-1; ++j){
            int branchIndex = branchesIndexVector[i][j];
            int childrenSize = branches[branchIndex].childrenIndex.size();
            branches[branchIndex].calAngle();



            if(childrenSize>2){
                flag[j] = false;
            }else{
                int cIndex1 = branches[branchIndex].childrenIndex[0];
                if(cIndex1 == branchesIndexVector[i][j+1]){
                    if(branches[branchIndex].localAngle2>branches[branchIndex].localAngle3){
                        flag[j] = true;
                    }else{
                        flag[j] = false;
                    }
                }else{
                    if(branches[branchIndex].localAngle1>branches[branchIndex].localAngle3){
                        flag[j] = true;
                    }else{
                        flag[j] = false;
                    }
                }
            }

//            qDebug()<<"j: "<<j<<" "<<branchIndex<<branches[branchIndex].length<<" "<<childrenSize<<" "<<flag[j];
        }

        for(int j=0; j<size-1; ++j){
            if((flag[j] == false && flag[j+1] == false) || j == size-2){
                int branchIndex = branchesIndexVector[i][j];
//                qDebug()<<"branchIndex: "<<branchIndex;
                branches[branchIndex].rLevel = 0;
                this->setChildenBranchDeleteType(branchIndex);
                this->setParentBranchRLevel(branchIndex);
                break;
            }
        }
//        for(int j=0; j<size-1; ++j){
//            qDebug()<<"flag j: "<<flag[j];
//            int branchIndex = branchesIndexVector[i][j];
//            branches[branchIndex].setBranchType(4);
//        }
    }

    return true;
}

bool BranchTree::pruningAdjacentSoma2(const QString &multiMarkerPath){
    qDebug()<<"------pruningAdjacentSoma2------";

    QList<ImageMarker> markers = readMarker_file(multiMarkerPath);
    if(markers.size()<=1)
        return true;

    vector<int> level0Index;
    this->getLevelIndex(level0Index,0);
    queue<int> branchQ;
    for(int i=0; i<level0Index.size(); ++i){
        int branchIndex = level0Index[i];
        if(branches[branchIndex].rLevel != -1){
            branchQ.push(branchIndex);
        }
    }

    double somaR = nt.listNeuron[somaIndex].r;

    vector<vector<int> > branchesIndexVector;

    while (!branchQ.empty()) {
        int branchIndex = branchQ.front();
        branchQ.pop();
        if(branches[branchIndex].rLevel == -1){
            continue;
        }
        vector<V3DLONG> pointsIndex;
        branches[branchIndex].get_pointsIndex_of_branch(pointsIndex);
        bool meetOtherSoma = false;
        for(int i=0; i<pointsIndex.size(); ++i){
            V3DLONG curIndex = pointsIndex[i];
            bool isNearSoma = false;
            for(int j=0; j<markers.size(); j++){
                if(zx_dist(markers[j],nt.listNeuron[curIndex])<10){
                    isNearSoma = true;
                    break;
                }
            }
            if(isNearSoma && zx_dist(nt.listNeuron[somaIndex],nt.listNeuron[curIndex])>somaR*2){
                nt.listNeuron[curIndex].type = BreakType;
                branches[branchIndex].setBranchType(ToDeleteType,true);
                nt.listNeuron[curIndex].type = AdjacentSomaType;
                meetOtherSoma = true;
                vector<int> branchesIndex;
                branchesIndex.push_back(branchIndex);
                while (branches[branchIndex].parentIndex != -1) {
                    branchIndex = branches[branchIndex].parentIndex;
                    branchesIndex.push_back(branchIndex);
                }
                reverse(branchesIndex.begin(),branchesIndex.end());
                branchesIndexVector.push_back(branchesIndex);
                break;
            }
        }

        if(meetOtherSoma){
//            this->setChildenBranchDeleteType(branchIndex);
        }else{
            for(int j=0; j<branches[branchIndex].childrenIndex.size(); ++j){
                int cIndex = branches[branchIndex].childrenIndex[j];
                if(branches[cIndex].rLevel != -1){
                    branchQ.push(cIndex);
                }
            }
        }
    }

    qDebug()<<"other soma size: "<<branchesIndexVector.size();

    for(int i=0; i<branchesIndexVector.size(); ++i){
        int size = branchesIndexVector[i].size();
//        qDebug()<<"i: "<<i;
        vector<bool> flag = vector<bool>(size,false);
        for(int j=0; j<size-1; ++j){
            int branchIndex = branchesIndexVector[i][j];
            int childrenSize = branches[branchIndex].childrenIndex.size();
            branches[branchIndex].calAngle();



            if(childrenSize>2){
                flag[j] = false;
            }else{
                int cIndex1 = branches[branchIndex].childrenIndex[0];
                if(cIndex1 == branchesIndexVector[i][j+1]){
                    if(branches[branchIndex].localAngle2>branches[branchIndex].localAngle3){
                        flag[j] = true;
                    }else{
                        flag[j] = false;
                    }
                }else{
                    if(branches[branchIndex].localAngle1>branches[branchIndex].localAngle3){
                        flag[j] = true;
                    }else{
                        flag[j] = false;
                    }
                }
            }

//            qDebug()<<"j: "<<j<<" "<<branchIndex<<branches[branchIndex].length<<" "<<childrenSize<<" "<<flag[j];
        }

        for(int j=0; j<size-1; ++j){
            if((flag[j] == false && flag[j+1] == false) || j == size-2){
                int branchIndex = branchesIndexVector[i][j];
//                qDebug()<<"branchIndex: "<<branchIndex;
                branches[branchIndex].rLevel = 0;
                this->setChildenBranchDeleteType(branchIndex);
                this->setParentBranchRLevel(branchIndex);
                break;
            }
        }
//        for(int j=0; j<size-1; ++j){
//            qDebug()<<"flag j: "<<flag[j];
//            int branchIndex = branchesIndexVector[i][j];
//            branches[branchIndex].setBranchType(4);
//        }
    }

    return true;
}

bool BranchTree::pruningCross(double angleTh, double lengthTh){

    if(this->branches.size()>BigBranchNumber){
        qDebug()<<"the branch number is too big, do nothing!";
        return false;
    }

    vector<int> level0Index;
    this->getLevelIndex(level0Index,0);
    queue<int> branchQ;
    for(int i=0; i<level0Index.size(); ++i){
        int branchIndex = level0Index[i];
        if(branches[branchIndex].rLevel != -1){
            branchQ.push(branchIndex);
        }
    }

    qDebug()<<"branchQ size: "<<branchQ.size();

    qDebug()<<"solve + cross";
    while (!branchQ.empty()) {
        int branchIndex = branchQ.front();
        branchQ.pop();
        if(branches[branchIndex].rLevel == -1){
            continue;
        }
        bool isTrivial= true;
        for(int j=0; j<branches[branchIndex].childrenIndex.size(); ++j){
            int cIndex = branches[branchIndex].childrenIndex[j];
            if(branches[cIndex].rLevel>2){
                isTrivial = false;
            }
        }
        if(isTrivial && branches[branchIndex].rLevel>0
                && branches[branchIndex].calAngle() && branches[branchIndex].localAngle3>angleTh){
            if(branches[branchIndex].length<lengthTh){
                this->setChildenBranchDeleteType(branchIndex);
                branches[branchIndex].setBranchType(ToDeleteType);
                branches[branchIndex].rLevel = -1;
                this->setParentBranchRLevel(branchIndex);
            }else{
                this->setChildenBranchDeleteType(branchIndex);
                branches[branchIndex].rLevel = 0;
                this->setParentBranchRLevel(branchIndex);
            }
        }else{
            for(int j=0; j<branches[branchIndex].childrenIndex.size(); ++j){
                int cIndex = branches[branchIndex].childrenIndex[j];
                if(branches[cIndex].rLevel != -1){
                    branchQ.push(cIndex);
                }
            }
        }
    }

    this->update();
    qDebug()<<"solve X cross";

    level0Index.clear();
    this->getLevelIndex(level0Index,0);

    set<int> branchSet;

    for(int i=0; i<level0Index.size(); ++i){
        int branchIndex = level0Index[i];
        if(branches[branchIndex].rLevel != -1){
            branchQ.push(branchIndex);
            branchSet.insert(branchIndex);
        }
    }
    qDebug()<<"branchQ size: "<<branchQ.size();

    int d = 20;

    while (!branchQ.empty()) {
        int branchIndex = branchQ.front();
        branchQ.pop();
        branchSet.erase(branchIndex);
        if(branches[branchIndex].rLevel <= 0){
            continue;
        }
        if(branches[branchIndex].level == 0){
            for(int j=0; j<branches[branchIndex].childrenIndex.size(); ++j){
                int cIndex = branches[branchIndex].childrenIndex[j];
                if(branches[cIndex].rLevel != -1){
                    if(branchSet.find(cIndex) == branchSet.end()){
                        branchSet.insert(cIndex);
                        branchQ.push(cIndex);
                    }
                }
            }
            continue;
        }
        qDebug()<<branches[branchIndex].rLevel<<" "<<branches[branchIndex].length;
        if(branches[branchIndex].length<lengthTh*2){
            vector<int> candidateBranchIndex;
            vector<int> allBranchIndex;
            vector<XYZ> candidateBranchVector;
            int branchParentIndex = branches[branchIndex].parentIndex;
            for(int i=0; i<branches[branchParentIndex].childrenIndex.size(); ++i){
                int tmpBranchIndex = branches[branchParentIndex].childrenIndex[i];
                allBranchIndex.push_back(tmpBranchIndex);
                if(tmpBranchIndex != branchIndex && branches[tmpBranchIndex].rLevel<=2){
                    candidateBranchIndex.push_back(tmpBranchIndex);
                }
            }
            for(int i=0; i<branches[branchIndex].childrenIndex.size(); ++i){
                int tmpBranchIndex = branches[branchIndex].childrenIndex[i];
                allBranchIndex.push_back(tmpBranchIndex);
                if(branches[tmpBranchIndex].rLevel>2)
                    continue;
                candidateBranchIndex.push_back(tmpBranchIndex);
            }

            vector<V3DLONG> pointsIndex;
//            branches[branchParentIndex].get_r_pointsIndex_of_branch(pointsIndex);
//            XYZ vp = getBranchLocalVector(pointsIndex,d);

            for(int i=0; i<candidateBranchIndex.size(); ++i){
                pointsIndex.clear();
                branches[candidateBranchIndex[i]].findBranchInflectionPoint(d,0);
                branches[candidateBranchIndex[i]].get_pointsIndex_of_branch(pointsIndex);
                XYZ v = getBranchLocalVector(pointsIndex,d);
                candidateBranchVector.push_back(v);
            }

            pair<int,int> maxVectorIndex;
            double maxAngle = 0;
            for(int i=0; i<candidateBranchVector.size(); ++i){
                for(int j=i+1; j<candidateBranchVector.size(); ++j){
                    XYZ p1 = candidateBranchVector[i];
                    XYZ p2 = candidateBranchVector[j];
                    double tmpAngle = (acos(dot(normalize(p1),normalize(p2)))/PI)*180;
                    qDebug()<<i<<" "<<j<<" : "<<tmpAngle;
                    if(tmpAngle>maxAngle){
                        maxAngle = tmpAngle;
                        maxVectorIndex.first = candidateBranchIndex[i];
                        maxVectorIndex.second = candidateBranchIndex[j];
                    }
                }
            }

            if(maxAngle>angleTh){
                for(int i=0; i<allBranchIndex.size(); ++i){
                    int tmpBranchIndex = allBranchIndex[i];
                    if(tmpBranchIndex==maxVectorIndex.first || tmpBranchIndex==maxVectorIndex.second){
                        this->setChildenBranchDeleteType(tmpBranchIndex);
                        branches[tmpBranchIndex].setBranchType(ToDeleteType);
                        branches[tmpBranchIndex].rLevel = -1;
                        this->setParentBranchRLevel(tmpBranchIndex);
                    }else{
                        for(int j=0; j<branches[tmpBranchIndex].childrenIndex.size(); ++j){
                            int cIndex = branches[tmpBranchIndex].childrenIndex[j];
                            if(branchSet.find(cIndex) == branchSet.end()){
                                branchSet.insert(cIndex);
                                branchQ.push(cIndex);
                            }
                        }
                    }
                }
            }else{
                for(int i=0; i<allBranchIndex.size(); ++i){
                    int tmpBranchIndex = allBranchIndex[i];
                    for(int j=0; j<branches[tmpBranchIndex].childrenIndex.size(); ++j){
                        int cIndex = branches[tmpBranchIndex].childrenIndex[j];
                        if(branchSet.find(cIndex) == branchSet.end()){
                            branchSet.insert(cIndex);
                            branchQ.push(cIndex);
                        }
                    }
                }
            }


        }else{
            for(int j=0; j<branches[branchIndex].childrenIndex.size(); ++j){
                int cIndex = branches[branchIndex].childrenIndex[j];
                if(branches[cIndex].rLevel != -1){
                    if(branchSet.find(cIndex) == branchSet.end()){
                        branchSet.insert(cIndex);
                        branchQ.push(cIndex);
                    }
                }
            }
        }

    }
    return true;

}

bool BranchTree::pruningInflectionPoints(unsigned char *inimg1d, long long *sz, double d, double cosAngleThres){

    if(this->branches.size()>BigBranchNumber){
        qDebug()<<"the branch number is too big, do nothing!";
        return false;
    }

    for(int i=0; i<branches.size(); ++i){
        if(branches[i].rLevel != 0)
            continue;
        branches[i].findBranchInflectionPoint(d,cosAngleThres);
        if(!branches[i].checkBranchInflectionPoint(inimg1d,sz)){
            branches[i].rLevel = 0;
            this->setParentBranchRLevel(i);
        }

    }

//    vector<int> level0Index;
//    this->getLevelIndex(level0Index,0);
//    queue<int> branchQ;
//    for(int i=0; i<level0Index.size(); ++i){
//        int branchIndex = level0Index[i];
//        if(branches[branchIndex].rLevel != -1){
//            branchQ.push(branchIndex);
//        }
//    }
//    while (!branchQ.empty()) {
//        int branchIndex = branchQ.front();
//        branches[branchIndex].findBranchInflectionPoint(d,cosAngleThres);
//        branchQ.pop();
//        if(!branches[branchIndex].checkBranchInflectionPoint(inimg1d,sz)){
//            this->setChildenBranchDeleteType(branchIndex);
//            branches[branchIndex].rLevel = 0;
//            this->setParentBranchRLevel(branchIndex);
//        }else{
//            for(int j=0; j<branches[branchIndex].childrenIndex.size(); ++j){
//                int cIndex = branches[branchIndex].childrenIndex[j];
//                if(branches[cIndex].rLevel != -1){
//                    branchQ.push(cIndex);
//                }
//            }
//        }
//    }
    return true;
}

bool BranchTree::pruningSuspectedBranch(unsigned char *inimg1d, long long *sz,double angleTh, double lengthTh){
    qDebug()<<"in pruningSuspectedBranch";
    int d = 20;
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];

    for(int i=0; i<branches.size(); ++i){
        int parentIndex = branches[i].parentIndex;
        if(parentIndex == -1)
            continue;
        if(branches[i].rLevel == 0)
            qDebug()<<"parent length:"<<branches[parentIndex].length;
        if(branches[i].rLevel != 0 || branches[parentIndex].length<lengthTh)
            continue;
        XYZ v_curLocal,v_curGlobal,v_p;
        vector<V3DLONG> pointsIndex;
        pointsIndex.clear();
        branches[i].get_pointsIndex_of_branch(pointsIndex);
        v_curLocal = getBranchLocalVector(pointsIndex,d);
        v_curGlobal = getBranchGlobalVector(pointsIndex);

        pointsIndex.clear();
        branches[i].get_r_pointsIndex_of_branch(pointsIndex);
        v_p = XYZ(0,0,0) - getBranchLocalVector(pointsIndex,d);

        double tmpAngle1 = (acos(dot(normalize(v_curLocal),normalize(v_p)))/PI)*180;
        double tmpAngle2 = (acos(dot(normalize(v_curGlobal),normalize(v_p)))/PI)*180;
        qDebug()<<"tmpAngle1: "<<tmpAngle1<<" tmpAngle2: "<<tmpAngle2;
        if(tmpAngle1>angleTh || tmpAngle2>angleTh){

            pointsIndex.clear();
            branches[i].get_pointsIndex_of_branch(pointsIndex);

            int startIndex = pointsIndex.size() - 1;
            double sLength = 0;
            for(int j=1; j<pointsIndex.size(); ++j){
                sLength += zx_dist(nt.listNeuron[pointsIndex[j]],nt.listNeuron[pointsIndex[j-1]]);
                if(sLength>lengthTh){
                    startIndex = j;
                    break;
                }
            }

            MyMarker root = MyMarker(nt.listNeuron[pointsIndex[startIndex]].x,
                    nt.listNeuron[pointsIndex[startIndex]].y,
                    nt.listNeuron[pointsIndex[startIndex]].z);
            vector<MyMarker*> outLine;
            double l = lengthTh + 10;
            qDebug()<<"l: "<<l;
            fastmarching_tree_constraint(root,inimg1d,outLine,sz[0],sz[1],sz[2],3,0,false,l);
            qDebug()<<"end fastmartching_tree";

            int sIndex = 0, eIndex = outLine.size()-1;
            sLength = 0;
            for(int j=pointsIndex.size()-2; j>=0; --j){
                sLength += zx_dist(nt.listNeuron[pointsIndex[j]],nt.listNeuron[pointsIndex[j+1]]);
                if(sLength>lengthTh){
                    sIndex = j;
                    break;
                }
            }
            sLength = 0;
            for(int j=1; j<outLine.size(); ++j){
                sLength += zx_dist(*(outLine[j]),*(outLine[j-1]));
                if(sLength>lengthTh){
                    eIndex = j;
                    break;
                }
            }
            XYZ p1 = XYZ(nt.listNeuron[pointsIndex[sIndex]].x - root.x,
                         nt.listNeuron[pointsIndex[sIndex]].y - root.y,
                         nt.listNeuron[pointsIndex[sIndex]].z - root.z);
            XYZ p2 = XYZ(outLine[eIndex]->x - root.x,
                    outLine[eIndex]->y - root.y,
                    outLine[eIndex]->z - root.z);
            qDebug()<<"outLine size: "<<outLine.size();
            double tmpCosAngle = dot(normalize(p1),normalize(p2));

            qDebug()<<"tmpCosAngle: "<<tmpCosAngle;

//            NeuronTree outswcBefore = swc_convert(outLine);
//            QString savePathBefore = QString("F:\\manual\\") + QString::number(branches[i].endPointIndex) + "fastmartching_before.swc";
//            writeSWC_file(savePathBefore,outswcBefore);

            if(tmpCosAngle<0.5){
                vector<MyMarker*> maskMarkers;
                maskMarkers.insert(maskMarkers.begin(),outLine.begin(),outLine.begin()+outLine.size()/2);
                unsigned char* maskFlag = 0;
                for(int j=0; j<maskMarkers.size(); ++j){
                    maskMarkers[j]->radius = 8;
                }
                swc2mask(maskFlag,maskMarkers,sz[0],sz[1],sz[2]);
                qDebug()<<"mask end";
                for(long j=0; j<tolSZ; j++){
                    if(maskFlag[j] == (unsigned char)255){
                        maskFlag[j] = 0;
                    }else{
                        maskFlag[j] = inimg1d[j];
                    }
                }
                qDebug()<<"end maskFlag";
                outLine.clear();
                qDebug()<<"root xyz: "<<root.x<<" "<<root.y<<" "<<root.z;
                fastmarching_tree_constraint(root,maskFlag,outLine,sz[0],sz[1],sz[2],3,0,false,l);
                qDebug()<<"end fastmartching_tree 2";
                if(maskFlag){
                    delete[] maskFlag;
                    maskFlag = 0;
                }
//                NeuronTree outswc = swc_convert(outLine);
//                QString savePath = QString("F:\\manual\\") + QString::number(branches[i].endPointIndex) + "fastmartching.swc";
//                writeSWC_file(savePath,outswc);
            }

            if(outLine.size()<1)
                continue;

            NeuronTree* origin = new NeuronTree();
            for(int j=pointsIndex.size()-1; j>=0; --j){
                NeuronSWC s = nt.listNeuron[pointsIndex[j]];
                if(j==pointsIndex.size()-1)
                    s.parent = -1;
                origin->listNeuron.push_back(s);
            }
            queue<int> branchQ;
            for(int j=0; j<branches[parentIndex].childrenIndex.size(); ++j){
                int cIndex = branches[parentIndex].childrenIndex[j];
                if(cIndex != i)
                    branchQ.push(cIndex);
            }
            int count = 0;
            while (!branchQ.empty() && count<5) {
                count++;
                int tmpBranchIndex = branchQ.front();
                branchQ.pop();
                pointsIndex.clear();
                branches[tmpBranchIndex].get_pointsIndex_of_branch(pointsIndex);
                for(int j=1; j<pointsIndex.size(); ++j){
                    NeuronSWC s = nt.listNeuron[pointsIndex[j]];
                    origin->listNeuron.push_back(s);
                }
                for(int j=0; j<branches[tmpBranchIndex].childrenIndex.size(); ++j){
                    int cIndex = branches[tmpBranchIndex].childrenIndex[j];
                    branchQ.push(cIndex);
                }
            }


            XYZ pt;
            double targetD = 0;
            count = 0;
            for(int j=0; j<outLine.size()/2; ++j){
                pt = XYZ(outLine[j]->x,outLine[j]->y,outLine[j]->z);
                targetD += dist_pt_to_swc(pt,origin);
                count++;
            }
            if(count>0){
                targetD /= count;
            }
            qDebug()<<"targetD: "<<targetD;
            if(targetD>2){
                branches[i].setBranchType(ToDeleteType);
            }
            if(origin){
                delete origin;
            }

        }

    }
}

bool BranchTree::saveNeuronTree(QString path){
    return writeSWC_file(path,nt);
}

bool BranchTree::savePrunedNeuronTree(QString path){
    NeuronTree prunedTree;
    for(int i=0; i<nt.listNeuron.size(); ++i){
        NeuronSWC s = nt.listNeuron[i];
        if(s.type != ToDeleteType){
            prunedTree.listNeuron.push_back(s);
        }
    }

    return writeSWC_file(path,prunedTree);
}

bool BranchTree::update(){
    NeuronTree prunedTree;
    for(int i=0; i<nt.listNeuron.size(); ++i){
        NeuronSWC s = nt.listNeuron[i];
        if(s.type != ToDeleteType){
            prunedTree.listNeuron.push_back(s);
        }
    }
    prunedTree.hashNeuron.clear();
    for(int i=0; i<prunedTree.listNeuron.size(); ++i){
        NeuronSWC s = prunedTree.listNeuron[i];
        prunedTree.hashNeuron.insert(s.n,i);
    }
    this->initialize(prunedTree);
    return true;
}


void BranchTree::show(unsigned char* pdata, V3DLONG* sz){
    for(int i=0; i<branches.size(); ++i){
        branches[i].getBranchIntensity(pdata,sz);
    }

    for(int i=0; i<branches.size(); ++i){
        cout<<"childrensize: "<<branches[i].childrenIndex.size()<<endl;
        cout<<branches[i].level<<" "<<branches[i].rLevel<<" "
               <<branches[i].intensityMean<<" "<<branches[i].intensityStd<<" ";

        for(int j=0; j<branches[i].childrenIndex.size(); j++){
            int cindex = branches[i].childrenIndex[j];
            cout<<"cIndex: "<<cindex<<" ";
            cout<<branches[cindex].intensityMean<<" "<<branches[cindex].intensityStd<<" ";
        }
        cout<<endl;
    }
}

void BranchTree::calRlevel0Branches(unsigned char *pdata, long long *sz, ofstream& csvFile){
    for(int i=0; i<branches.size(); ++i){
        if(branches[i].rLevel == 0 && branches[i].parentIndex != -1){
            int prtIndex = branches[i].parentIndex;
            float res = branches[i].getBranchPcaValue(pdata,sz,10);
            csvFile<<branches[prtIndex].length<<','<<res<<','<<branches[i].length<<endl;
        }
    }
}

void BranchTree::calBifurcationLocalAngle(ofstream &csvFile){
    for(int i=0; i<branches.size(); ++i){
        branches[i].findBranchInflectionPoint(10,0);
    }
    for(int i=0; i<branches.size(); ++i){
        if(branches[i].rLevel != 0){
            branches[i].calAngle();
            csvFile<<branches[i].localAngle3<<','
                  <<(180 - branches[i].localAngle1)<<','
                 <<(180 - branches[i].localAngle2)<<','<<endl;
        }
    }
}






