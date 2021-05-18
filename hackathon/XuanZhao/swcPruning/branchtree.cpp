#include "branchtree.h"

#include "algorithm";

#include "neuron_sim_scores.h"

#include "fastmarching_tree.h"

#include "../../../released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h"

bool Branch::get_r_pointsIndex_of_branch(vector<V3DLONG> &r_points, NeuronTree &nt)
{
    int tmp = endPointIndex;
    r_points.push_back(tmp);
//    qDebug()<<"in get_r_pointsIndex_of_branch";
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

bool Branch::operator <(const Branch& other) const{
    if(this->level<other.level){
        return true;
    }else {
        return false;
    }
}

void Branch::calculateMidPointsLocalAngle(NeuronTree &nt, ofstream &csvFile, double toBifurcationD){
    vector<V3DLONG> pointsIndex = vector<V3DLONG>();
    this->get_pointsIndex_of_branch(pointsIndex,nt);
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

    int ppIndex,ccIndex,curIndex;
    XYZ p1,p2;

    double cosAngle;

    for(int i=1; i<pointsSize-1; i++){
        if(path[i]>toBifurcationD && (path[pointsSize-1]-path[i])>toBifurcationD){
            curIndex = pointsIndex[i];
            ppIndex = pointsIndex.front();
            ccIndex = pointsIndex.back();
            for(int ii=i-1; ii>=0; ii--){
                if(path[i]-path[ii]>toBifurcationD){
                    ppIndex = pointsIndex[ii];
                    break;
                }
            }
            for(int ii=i+1; ii<pointsSize; ii++){
                if(path[ii]-path[i]>toBifurcationD){
                    ccIndex = pointsIndex[ii];
                    break;
                }
            }
            p1 = XYZ(nt.listNeuron[curIndex].x - nt.listNeuron[ppIndex].x,
                     nt.listNeuron[curIndex].y - nt.listNeuron[ppIndex].y,
                     nt.listNeuron[curIndex].z - nt.listNeuron[ppIndex].z);
            p2 = XYZ(nt.listNeuron[ccIndex].x - nt.listNeuron[curIndex].x,
                     nt.listNeuron[ccIndex].y - nt.listNeuron[curIndex].y,
                     nt.listNeuron[ccIndex].z - nt.listNeuron[curIndex].z);
            cosAngle =  dot(normalize(p1),normalize(p2));
            csvFile<<this->level<<','<<this->rLevel<<','<<cosAngle<<endl;
        }
    }

    if(path){
        delete[] path;
        path = 0;
    }

}

void Branch::findInflectionPoint(NeuronTree &nt, double d, double cosAngleThres, ofstream &csvFile){

    qDebug()<<"in findInflectionPoint";

    vector<V3DLONG> pointsIndex = vector<V3DLONG>();
    this->get_pointsIndex_of_branch(pointsIndex,nt);
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
                        nt.listNeuron[pointsIndex[index]].type = 6;
                        inflectionNum++;

                        csvFile<<this->level<<','<<this->rLevel<<','<<l<<','<<cosAngle<<endl;
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

void Branch::checkInflectionPoint(NeuronTree &nt, unsigned char* inimg1d, long* sz){
    qDebug()<<"in checkInflectionPoint";
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];

    vector<V3DLONG> pointsIndex = vector<V3DLONG>();
    this->get_r_pointsIndex_of_branch(pointsIndex,nt);
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

    if(path[pointsSize-1]<20){
        return;
    }

    for(int i=1; i<pointsSize; i++){
        int index = pointsIndex[i];
        if(nt.listNeuron[index].type == 6){
            qDebug()<<"there is a inflectionPoint";
            int startI = 0;
            int endI = pointsSize-1;
            for(int j=0; j<i; j++){
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
            fastmarching_tree_constraint(root,inimg1d,outLine,sz[0],sz[1],sz[2],3,1,false,25);
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
            if(tmpCosAngle<0.5){
                vector<MyMarker*> maskMarkers;
                maskMarkers.insert(maskMarkers.begin(),outLine.begin(),outLine.begin()+outLine.size()/2);
                unsigned char* maskFlag = 0;
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
                fastmarching_tree_constraint(root,maskFlag,outLine,sz[0],sz[1],sz[2],3,1,false,25);
                qDebug()<<"end fastmartching_tree 2";
                if(maskFlag){
                    delete[] maskFlag;
                    maskFlag = 0;
                }
            }

            NeuronTree* origin = new NeuronTree();
            for(int j=0; j<=endI; j++){
                NeuronSWC s = nt.listNeuron[pointsIndex[j]];
                if(j == 0){
                    s.parent = -1;
                }
                origin->listNeuron.push_back(s);
            }
            qDebug()<<"origin size: "<<origin->listNeuron.size();
            if(outLine.size()<1)
                return;
            XYZ pt = XYZ(outLine[0]->x,outLine[0]->y,outLine[0]->z);
            double targetD = dist_pt_to_swc(pt,origin);
            qDebug()<<"targetD: "<<targetD;
            if(targetD>2){
                for(int j=0; j<pointsSize; j++){
                    if(j == i)
                        continue;
                    nt.listNeuron[pointsIndex[j]].type = 7;
                }
                return;
            }
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
            branches[i].parent = 0;
        }
        else
        {
            for(int j=0; j<branches.size(); ++j)
            {
                if(branches[i].headPointIndex == branches[j].endPointIndex)
                {
                    branches[i].parent = &branches[j];
                }
            }
        }
    }

    //initial level
    for(int i=0; i<branches.size(); ++i)
    {
        Branch* tmp;
        tmp = &branches[i];
        int level=0;
        while(tmp->parent != 0)
        {
            level++;
            tmp = tmp->parent;
        }
        branches[i].level=level;
    }

    //initial rlevel
    map<Branch*,int> branchMap = map<Branch*,int>();
    for(int i=0; i<branches.size(); ++i){
        branchMap[&(branches[i])] = i;
    }

    vector<vector<int> > branchChildren = vector<vector<int> >(branches.size(),vector<int>());
    for(int i=0; i<branches.size(); ++i){
        if(branches[i].parent != 0){
            branchChildren[branchMap[branches[i].parent]].push_back(i);
        }
    }

    for(int i=0;i<branches.size(); ++i){
        if(branchChildren[i].size() == 0){
            int rLevel = 0;
            Branch* tmp;
            tmp = &branches[i];
            tmp->rLevel = rLevel;
            while(tmp->parent != 0){
                rLevel++;
                tmp = tmp->parent;
                if(tmp->rLevel<rLevel){
                    tmp->rLevel = rLevel;
                }
            }

        }
    }

    //initial lengthToSoma,wieght
    for(int i=0; i<branches.size(); i++){
        Branch* tmp;
        tmp = &branches[i];
        float lengthToSoma = tmp->length;
        while(tmp->parent != 0){
            tmp = tmp->parent;
            lengthToSoma += tmp->length;
        }
        branches[i].lengthToSoma = lengthToSoma;
    }

    int maxLevel = this->get_max_level();

    vector<int> levelBranches = vector<int>();

    while(maxLevel>=0){
//        qDebug()<<"level: "<<maxLevel;
        for(int i=0; i<branches.size(); i++){
            if(branches[i].level == maxLevel){
                levelBranches.push_back(i);
            }
        }
//        qDebug()<<"levelBranches size: "<<levelBranches.size();

        for(int i=0; i<levelBranches.size(); i++){
            int tmp = levelBranches[i];
            if(branchChildren[tmp].size() == 0){
                branches[tmp].weight = 0;
            }else {
                for(int j=0; j<branchChildren[tmp].size(); j++){
                    int cIndex = branchChildren[tmp][j];
                    branches[tmp].weight += (branches[cIndex].length + branches[cIndex].weight);
                }
            }
        }
        levelBranches.clear();
        maxLevel--;
    }


    for(int i=0; i<branches.size(); i++){
        if(branches[i].weight>maxWeight){
            maxWeight = branches[i].weight;
        }
    }

    for(int i=0; i<branches.size(); i++){
        branches[i].sWeight = branches[i].weight*100/maxWeight;
    }



    cout<<"--------------------initialize end--------------";

    return true;
}

bool BranchTree::get_level_index(vector<int> &level_index, int level){
    for(int i=0; i<branches.size(); i++){
        if(branches[i].level == level){
            level_index.push_back(i);
        }
    }
    return true;
}

int BranchTree::get_max_level(){
    int max = 0;
    for(int i=0; i<branches.size(); i++){
        if(branches[i].level > max){
            max = branches[i].level;
        }
    }
    return max;
}

void BranchTree::calculateBranchMidPointsLocalAngle(ofstream &csvFile, double toBifurcationD){
    for(int i=0; i<branches.size(); i++){
        branches[i].calculateMidPointsLocalAngle(nt,csvFile,toBifurcationD);
    }
}

void BranchTree::findBranchInflectionPoint(ofstream &csvFile, double d, double cosAngleThres){
    for(int i=0; i<branches.size(); i++){
        branches[i].findInflectionPoint(nt,d,cosAngleThres,csvFile);
    }
}

void BranchTree::checkBranchInflectionPoint(unsigned char *inimg1d, long *sz){
    for(int i=0; i<branches.size(); i++){
        branches[i].checkInflectionPoint(nt,inimg1d,sz);
    }
}

void BranchTree::groupBifurcationPoint(ofstream &csvFile, double d){
    map<Branch*,int> branchMap = map<Branch*,int>();
    for(int i=0; i<branches.size(); ++i){
        branchMap[&(branches[i])] = i;
    }

    vector<vector<int> > branchChildren = vector<vector<int> >(branches.size(),vector<int>());
    for(int i=0; i<branches.size(); ++i){
        if(branches[i].parent != 0){
            branchChildren[branchMap[branches[i].parent]].push_back(i);
        }
    }
    for(int i=0; i<branches.size(); ++i){
        if(branchChildren[i].size()>2){
            nt.listNeuron[branches[i].endPointIndex].type = 7;
            branches[i].isNormalY = 0;
        }else if (branchChildren[i].size() == 2) {
            qDebug()<<i<<" branchChildren[i].size() == 2";
            XYZ v1,v2,v3;
            double cosAngle1,cosAngle2,cosAngle3;
            double angle1, angle2, angle3;

            XYZ v11,v22,v33;
            double cosAngle11,cosAngle22,cosAngle33;
            double angle11, angle22, angle33;
//            XYZ p1,p2;
            int cIndex1 = branchChildren[i][0];
            int cIndex2 = branchChildren[i][1];
//            if(branches[i].length<d || branches[cIndex1].length<d || branches[cIndex2].length<d){
//                nt.listNeuron[branches[i].endPointIndex].type = 8;
//                continue;
//            }
            vector<V3DLONG> pointsIndex = vector<V3DLONG>();
            branches[i].get_r_pointsIndex_of_branch(pointsIndex,nt);
            v1 = getBranchLocalVector(pointsIndex,d);
            v11 = getBranchGlobalVector(pointsIndex);

            pointsIndex.clear();
            branches[cIndex1].get_pointsIndex_of_branch(pointsIndex,nt);
            v2 = getBranchLocalVector(pointsIndex,d);
            v22 = getBranchGlobalVector(pointsIndex);

            pointsIndex.clear();
            branches[cIndex2].get_pointsIndex_of_branch(pointsIndex,nt);
            v3 = getBranchLocalVector(pointsIndex,d);
            v33 = getBranchGlobalVector(pointsIndex);

            cosAngle1 = dot(normalize(v1),normalize(v2));
            cosAngle2 = dot(normalize(v1),normalize(v3));
            cosAngle3 = dot(normalize(v2),normalize(v3));

            cosAngle11 = dot(normalize(v11),normalize(v22));
            cosAngle22 = dot(normalize(v11),normalize(v33));
            cosAngle33 = dot(normalize(v22),normalize(v33));

            angle1 = (acos(cosAngle1)/PI)*180;
            angle2 = (acos(cosAngle2)/PI)*180;
            angle3 = (acos(cosAngle3)/PI)*180;

            angle11 = (acos(cosAngle11)/PI)*180;
            angle22 = (acos(cosAngle22)/PI)*180;
            angle33 = (acos(cosAngle33)/PI)*180;

//            vector<double> angles = vector<double>();
//            angles.push_back(angle1);
//            angles.push_back(angle2);
//            angles.push_back(angle3);
            double angles[3] = {angle1,angle2,angle3};
            sort(angles,angles+3);

            if(angle3 == angles[0]){
                branches[i].isNormalY = 1;
            }else {
                branches[i].isNormalY = 0;
            }

            for(int j=0; j<3; j++){
                qDebug()<<"angles: "<<angles[j];
            }

            double flag = 0;

            double angleDiff1 = angles[2] - angles[1];
            double angleDiff2 = angles[1] - angles[0];
            if(angleDiff2<angleDiff1 && angleDiff2<15 && angles[2]>150){
                flag = 1;
            }

            if(flag == 0){
                if(angle3 == angles[0]){
                    nt.listNeuron[branches[i].endPointIndex].type = 9;
                }else{
                    nt.listNeuron[branches[i].endPointIndex].type = 10;
                }
            }else if (flag == 1) {
                if(angle3 == angles[2]){
                    nt.listNeuron[branches[i].endPointIndex].type = 11;
                }else {
                    nt.listNeuron[branches[i].endPointIndex].type = 12;
                }
            }
            branches[i].localAngle1 = angle1;
            branches[i].localAngle2 = angle2;
            branches[i].localAngle3 = angle3;
            branches[i].globalAngle1 = angle11;
            branches[i].globalAngle2 = angle22;
            branches[i].globalAngle3 = angle33;



        }
    }

    for(int i=0; i<branches.size(); i++){
        int flag = 0;
//        if(branches[i].isNormalY == 0){
//            for(int j=0; j<branchChildren[i].size(); j++){
//                int cIndex = branchChildren[i][j];
//                if(branches[cIndex].isNormalY == 0){
//                    flag = 1;

//                    nt.listNeuron[branches[i].endPointIndex].type = 13;
//                    break;
//                }
//            }
//        }

        csvFile<<branches[i].level<<','<<branches[i].rLevel<<','
              <<branches[i].length<<','<<branches[i].distance<<','
              <<branches[i].lengthToSoma<<','<<branches[i].weight<<','
             <<branches[i].sWeight<<','
            <<branches[i].localAngle1<<','<<branches[i].localAngle2<<','
           <<branches[i].localAngle3<<','
           <<branches[i].globalAngle1<<','<<branches[i].globalAngle2
          <<','<<branches[i].globalAngle3<<','
          <<flag<<endl;
    }

}

void BranchTree::groupBifurcationPoint2(ofstream &csvFile, double d){
    map<Branch*,int> branchMap = map<Branch*,int>();
    for(int i=0; i<branches.size(); ++i){
        branchMap[&(branches[i])] = i;
    }

    vector<vector<int> > branchChildren = vector<vector<int> >(branches.size(),vector<int>());
    for(int i=0; i<branches.size(); ++i){
        if(branches[i].parent != 0){
            branchChildren[branchMap[branches[i].parent]].push_back(i);
        }
    }

    for(int i=0; i<branches.size(); i++){
        if(branchChildren[i].size() == 0 || branches[i].isNeighbor == 1){
            continue;
        }
        if(branchChildren[i].size()>2){
            branches[i].isNormalY = 0;
            for(int j=0; j<branchChildren[i].size(); j++){
                int cbIndex = branchChildren[i][j];
                if(branches[cbIndex].length<d){
                    branches[i].isNeighbor = 1;
                    branches[i].isNeighbor = 1;
                    branches[cbIndex].isNormalY = 0;
                }
            }
        }else {
            judgeIsNormalY(i,branchChildren,d);
            for(int j=0; j<2; j++){
                int cbIndex = branchChildren[i][j];
                if(branches[cbIndex].length<d){
                    branches[i].isNeighbor = 1;
                    branches[cbIndex].isNeighbor = 1;
                    if(branches[i].isNormalY == 0){
                        branches[cbIndex].isNormalY = 0;
                    }else {
                        judgeIsNormalY(cbIndex,branchChildren,d);
                        if(branches[cbIndex].isNormalY == 0){
                            branches[i].isNormalY = 0;
                        }
                    }
                }
            }
        }
    }

    for(int i=0; i<branches.size(); i++){
        vector<V3DLONG> pointsIndex = vector<V3DLONG>();
        branches[i].get_pointsIndex_of_branch(pointsIndex,nt);
        if(branches[i].isNormalY == 0){
            nt.listNeuron[branches[i].endPointIndex].type = 4;
            if(branches[i].isNeighbor == 1 && branches[i].length<d)
                continue;
            if(branches[i].parent != 0 && branches[i].parent->isNormalY == 0){
                for(int j=1; j<pointsIndex.size()-1; j++){
                    if(nt.listNeuron[pointsIndex[j]].type != 3)
                        continue;
                    nt.listNeuron[pointsIndex[j]].type = 2;
                }
            }else if (branches[i].inflectionNum>0) {
                for(int j=pointsIndex.size()-1; j>0; j--){
                    if(nt.listNeuron[pointsIndex[j]].type == 6){
                        break;
                    }
                    nt.listNeuron[pointsIndex[j]].type = 2;
                }
            }
        }else if (branchChildren[i].size() == 0) {
            if(branches[i].parent != 0 && branches[i].parent->isNormalY == 0){
                for(int j=1; j<pointsIndex.size()-1; j++){
                    if(nt.listNeuron[pointsIndex[j]].type != 3)
                        continue;
                    nt.listNeuron[pointsIndex[j]].type = 2;
                }
            }else{
                if(branches[i].inflectionNum>0){
                    for(int j=pointsIndex.size()-1; j>0; j--){
                        if(nt.listNeuron[pointsIndex[j]].type == 6){
                            break;
                        }
                        nt.listNeuron[pointsIndex[j]].type = 2;
                    }
                }
            }
        }
    }
}

XYZ BranchTree::getBranchVector(vector<V3DLONG> pointsIndex, double d){
    XYZ p1,p2;
    int pointsSize = pointsIndex.size();
    double* path = new double[pointsSize];
    memset(path,0,sizeof(double)*pointsSize);
    for(int i=1; i<pointsSize; i++){
        double tmpD = zx_dist(nt.listNeuron[pointsIndex[i]],nt.listNeuron[pointsIndex[i-1]]);
        path[i] = path[i-1] + tmpD;
    }
    int startPoint = -1;
    for(int i=1; i<pointsSize; i++){
        if(path[i]>d){
            p1 = XYZ(nt.listNeuron[pointsIndex[i]].x,
                    nt.listNeuron[pointsIndex[i]].y,
                    nt.listNeuron[pointsIndex[i]].z);
            startPoint = i;
            break;
        }
    }
    int ppIndex,ccIndex,curIndex;
    XYZ pp1,pp2;
    for(int i= startPoint+1; i<pointsSize; i++){
        curIndex = pointsIndex[i];
        ppIndex = pointsIndex.front();
        ccIndex = pointsIndex.back();
        for(int ii=i-1; ii>=0; ii--){
            if(path[i]-path[ii]>d){
                ppIndex = pointsIndex[ii];
                break;
            }
        }
        for(int ii=i+1; ii<pointsSize; ii++){
            if(path[ii]-path[i]>d){
                ccIndex = pointsIndex[ii];
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
        if(tmpCosAngle<(sqrt(2.0)/2)){
            p2 = XYZ(nt.listNeuron[pointsIndex[i]].x,
                    nt.listNeuron[pointsIndex[i]].y,
                    nt.listNeuron[pointsIndex[i]].z);
        }
    }


    if(path){
        delete[] path;
        path = 0;
    }

    return p2-p1;
}

XYZ BranchTree::getBranchLocalVector(vector<V3DLONG> pointsIndex, double d){
    XYZ p1,p2;
    int pointsSize = pointsIndex.size();
//    double* path = new double[pointsSize];
//    memset(path,0,sizeof(double)*pointsSize);
    double length = 0;
    for(int i=1; i<pointsSize; i++){
        double tmpD = zx_dist(nt.listNeuron[pointsIndex[i]],nt.listNeuron[pointsIndex[i-1]]);
//        path[i] = path[i-1] + tmpD;
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
            return XYZ(0,0,0);
        }
        return p2-p1;
    }else {
        p1 = XYZ(nt.listNeuron[pointsIndex[0]].x,
                nt.listNeuron[pointsIndex[0]].y,
                nt.listNeuron[pointsIndex[0]].z);
        int count =0;
        for(int i=1; i<pointsSize; i++){
//            if(path[i]<d){
//                continue;
//            }
            count++;
            p2.x += nt.listNeuron[pointsIndex[i]].x;
            p2.y += nt.listNeuron[pointsIndex[i]].y;
            p2.z += nt.listNeuron[pointsIndex[i]].z;
            if(nt.listNeuron[pointsIndex[i]].type == 6){
                break;
            }
        }
        if(count>0){
            p2.x /= (double)count;
            p2.y /= (double)count;
            p2.z /= (double)count;
        }else {
            return XYZ(0,0,0);
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

void BranchTree::saveMarkerFlag(QString markerPath){
    QList<ImageMarker> markers = QList<ImageMarker>();
    for(int i=0; i<nt.listNeuron.size(); i++){
        int type = nt.listNeuron[i].type;
        if(type != 6 /*&& type != 7
                && type != 8 && type != 9
                && type != 10 && type != 11 && type != 12 && type!= 13*/){
            continue;
        }
        ImageMarker m = ImageMarker(nt.listNeuron[i].x+1,nt.listNeuron[i].y+1,nt.listNeuron[i].z+1);
        switch (type) {
        case 6:
            m.color = XYZW(255,0,0,0);
            break;
//        case 7:
//            m.color = XYZW(0,255,0,0);
//            break;
//        case 8:
//            m.color = XYZW(0,0,255,0);
//            break;
//        case 9:
//            m.color = XYZW(255,255,0,0);
//            break;
//        case 10:
//            m.color = XYZW(255,0,255,0);
//            break;
//        case 11:
//            m.color = XYZW(0,255,255,0);
//            break;
//        case 12:
//            m.color = XYZW(128,128,0,0);
//            break;
        case 13:
            m.color = XYZW(128,0,0,0);
        default:
            break;
        }
        markers.push_back(m);
    }
    writeMarker_file(markerPath,markers);
}

void BranchTree::refineBifurcationPoint(){
    NeuronTree nt_new;
    int size = nt.listNeuron.size();
    int* flag = new int[size];
    memset(flag,0,sizeof(int)*size);

    map<Branch*,int> branchMap = map<Branch*,int>();
    for(int i=0; i<branches.size(); ++i){
        branchMap[&(branches[i])] = i;
    }

    vector<vector<int> > branchChildren = vector<vector<int> >(branches.size(),vector<int>());
    for(int i=0; i<branches.size(); ++i){
        if(branches[i].parent != 0){
            branchChildren[branchMap[branches[i].parent]].push_back(i);
        }
    }
    double d = 10;
    map<V3DLONG,V3DLONG> parentIndexMap = map<V3DLONG,V3DLONG>();

    for(int i=0; i<branches.size(); i++){
        if(branchChildren[i].size()>1){
            vector<V3DLONG> pointsIndex = vector<V3DLONG>();
            branches[i].get_r_pointsIndex_of_branch(pointsIndex,nt);
            XYZ p1,p2;
            p1 = XYZ(nt.listNeuron[pointsIndex[1]].x,
                    nt.listNeuron[pointsIndex[1]].y,
                    nt.listNeuron[pointsIndex[1]].z);
            p2 = XYZ(nt.listNeuron[pointsIndex[0]].x,
                    nt.listNeuron[pointsIndex[0]].y,
                    nt.listNeuron[pointsIndex[0]].z);
            XYZ v1 = p2 - p1;
            double cosAngleMax = -1;
            int cosAngleIndex = -1;

            for(int j=0; j<branchChildren[i].size(); j++){
                int cIndex = branchChildren[i][j];
                pointsIndex.clear();
                branches[cIndex].get_pointsIndex_of_branch(pointsIndex,nt);
                p1 = XYZ(nt.listNeuron[pointsIndex[1]].x,
                        nt.listNeuron[pointsIndex[1]].y,
                        nt.listNeuron[pointsIndex[1]].z);
                p2 = XYZ(nt.listNeuron[pointsIndex[0]].x,
                        nt.listNeuron[pointsIndex[0]].y,
                        nt.listNeuron[pointsIndex[0]].z);
                XYZ v2 = p1 - p2;
                double cosAngleTmp = dot(normalize(v1),normalize(v2));
                if(cosAngleTmp>cosAngleMax){
                    cosAngleMax = cosAngleTmp;
                    cosAngleIndex = j;
                }
            }
            pointsIndex.clear();
            branches[i].get_pointsIndex_of_branch(pointsIndex,nt);
            qDebug()<<"i size:"<<pointsIndex.size();
            int paretBranchPointSize = pointsIndex.size();
            pointsIndex.pop_back();
            p1 = XYZ(nt.listNeuron[pointsIndex.back()].x,
                    nt.listNeuron[pointsIndex.back()].y,
                    nt.listNeuron[pointsIndex.back()].z);
            branches[branchChildren[i][cosAngleIndex]].get_pointsIndex_of_branch(pointsIndex,nt);
            qDebug()<<"i size:"<<pointsIndex.size();
            p2 = XYZ(nt.listNeuron[pointsIndex[paretBranchPointSize]].x,
                    nt.listNeuron[pointsIndex[paretBranchPointSize]].y,
                    nt.listNeuron[pointsIndex[paretBranchPointSize]].z);
            v1 = p2 - p1;

            NeuronTree tmpnt;
            tmpnt.listNeuron.clear();
            for(int j=0; j<pointsIndex.size(); j++){
                tmpnt.listNeuron.push_back(nt.listNeuron[pointsIndex[j]]);
            }

            for(int j=0; j<branchChildren[i].size(); j++){
                int cIndex = branchChildren[i][j];

                if(j == cosAngleIndex){
                    continue;
                }
                qDebug()<<"branch j:"<<cIndex;

                vector<V3DLONG> pointsIndex2 = vector<V3DLONG>();
                branches[cIndex].get_pointsIndex_of_branch(pointsIndex2,nt);


                int count = 0;
                int bi = -1;
                for(int k=2; k<pointsIndex2.size(); k++){
                    p1 = XYZ(nt.listNeuron[pointsIndex2[1]].x,
                            nt.listNeuron[pointsIndex2[1]].y,
                            nt.listNeuron[pointsIndex2[1]].z);
                    p2 = XYZ(nt.listNeuron[pointsIndex2[k]].x,
                            nt.listNeuron[pointsIndex2[k]].y,
                            nt.listNeuron[pointsIndex2[k]].z);
                    XYZ v2 = p2 - p1;
                    double cosAngleTmp = abs(dot(normalize(v1),normalize(v2)));
                    if(cosAngleTmp>0.92 && dist_pt_to_swc(p2,&tmpnt)<3){
                        count++;
                        bi = k;
                        qDebug()<<cosAngleTmp;
                    }else {
                        bi = k;
                        break;
                    }

//                    double minD = dist_pt_to_swc(tmp,&tmpnt);
//                    if(minD<3){
//                        count++;
//                        bi = k;
//                        qDebug()<<minD;
//                    }else {
//                        break;
//                    }
                }
                if(count>1){
                    qDebug()<<"count: "<<count;
                    for(int k=1; k<bi;k++){
                        flag[pointsIndex2[k]] = 1;
                    }
                    V3DLONG bIndex = pointsIndex2[bi];
                    double minD = INT_MAX;
                    int minIndex = -1;
                    for(int ki=0; ki<pointsIndex.size(); ki++){
                        double tmpD = zx_dist(nt.listNeuron[pointsIndex[ki]],nt.listNeuron[bIndex]);
                        qDebug()<<ki<<" : "<<tmpD;
                        if(tmpD<minD){
                            minD = tmpD;
                            minIndex = pointsIndex[ki];
                        }
                    }
                    qDebug()<<"minD: "<<minD;
//                    nt.listNeuron[bIndex].parent = nt.listNeuron[minIndex].n;
                    parentIndexMap[bIndex] = minIndex;
                    nt.listNeuron[bIndex].type = 7;

                }

            }
        }
    }

    for(int i=0; i<nt.listNeuron.size(); i++){
        if(nt.listNeuron[i].type == 7){
            qDebug()<<"new bifurcation"<<i;
            nt.listNeuron[i].parent = nt.listNeuron[parentIndexMap[i]].n;
        }
    }

    qDebug()<<"start to make nt_new";

    for(int i=0; i<size; i++){
        if(flag[i] == 0){
            nt_new.listNeuron.push_back(nt.listNeuron[i]);
        }
    }

    for(int i=0; i<nt_new.listNeuron.size(); i++){
        nt_new.hashNeuron.insert(nt_new.listNeuron[i].n,i);
    }

    this->branches.clear();
    this->maxWeight = 0;
    this->initialize(nt_new);

    if(flag){
        delete[] flag;
        flag = 0;
    }
}

void BranchTree::judgeIsNormalY(int i, vector<vector<int> > branchChildren, double d){
    XYZ v1,v2,v3;
    double cosAngle1,cosAngle2,cosAngle3;
    double angle1, angle2, angle3;

    int cbIndex1 = branchChildren[i][0];
    int cbIndex2 = branchChildren[i][1];

    vector<V3DLONG> pointsIndex = vector<V3DLONG>();
    branches[i].get_r_pointsIndex_of_branch(pointsIndex,nt);
    v1 = getBranchLocalVector(pointsIndex,d);

    pointsIndex.clear();
    branches[cbIndex1].get_pointsIndex_of_branch(pointsIndex,nt);
    v2 = getBranchLocalVector(pointsIndex,d);

    pointsIndex.clear();
    branches[cbIndex2].get_pointsIndex_of_branch(pointsIndex,nt);
    v3 = getBranchLocalVector(pointsIndex,d);

    cosAngle1 = dot(normalize(v1),normalize(v2));
    cosAngle2 = dot(normalize(v1),normalize(v3));
    cosAngle3 = dot(normalize(v2),normalize(v3));

    angle1 = (acos(cosAngle1)/PI)*180;
    angle2 = (acos(cosAngle2)/PI)*180;
    angle3 = (acos(cosAngle3)/PI)*180;

    double angles[3] = {angle1,angle2,angle3};
    sort(angles,angles+3);

    if(angle3 == angles[0]){
        branches[i].isNormalY = 1;
    }else {
        branches[i].isNormalY = 0;
    }
}

void BranchTree::groupBifurcationPoint3(ofstream &csvFile, double d){
    map<Branch*,int> branchMap = map<Branch*,int>();
    for(int i=0; i<branches.size(); ++i){
        branchMap[&(branches[i])] = i;
    }

    vector<vector<int> > branchChildren = vector<vector<int> >(branches.size(),vector<int>());
    for(int i=0; i<branches.size(); ++i){
        if(branches[i].parent != 0){
            branchChildren[branchMap[branches[i].parent]].push_back(i);
        }
    }

    for(int i=0 ; i<branches.size(); i++){
        if(branches[i].length<d && branchChildren[i].size()>0 && branches[i].parent != 0){
            nt.listNeuron[branches[i].headPointIndex].type = 2;
            nt.listNeuron[branches[i].endPointIndex].type = 2;

            vector<int> cIndexs = vector<int>();

            for(int j=0; j<branchChildren[i].size(); j++){
                int cIndex = branchChildren[i][j];
                if(branches[cIndex].length<d){
                    cIndexs.push_back(cIndex);
                }
            }

            XYZ v1,v2,v3;
            double cosAngle1,cosAngle2,cosAngle3;
            double angle1, angle2, angle3;
            XYZ v11,v22,v33;
            double cosAngle11,cosAngle22,cosAngle33;
            double angle11, angle22, angle33;
            vector<V3DLONG> pointsIndex = vector<V3DLONG>();
            if(cIndexs.size() == 0){
                int bpIndex = branchMap[branches[i].parent];
                int cIndex1 = branchChildren[bpIndex][0];
                int cIndex2 = branchChildren[bpIndex][1];

                branches[i].parent->get_r_pointsIndex_of_branch(pointsIndex,nt);
                v1 = getBranchLocalVector(pointsIndex,d);
                v11 = getBranchGlobalVector(pointsIndex);

                pointsIndex.clear();
                branches[cIndex1].get_pointsIndex_of_branch(pointsIndex,nt);
                v2 = getBranchLocalVector(pointsIndex,d);
                v22 = getBranchGlobalVector(pointsIndex);

                pointsIndex.clear();
                branches[cIndex2].get_pointsIndex_of_branch(pointsIndex,nt);
                v3 = getBranchLocalVector(pointsIndex,d);
                v33 = getBranchGlobalVector(pointsIndex);

                cosAngle1 = dot(normalize(v1),normalize(v2));
                cosAngle2 = dot(normalize(v1),normalize(v3));
                cosAngle3 = dot(normalize(v2),normalize(v3));

                cosAngle11 = dot(normalize(v11),normalize(v22));
                cosAngle22 = dot(normalize(v11),normalize(v33));
                cosAngle33 = dot(normalize(v22),normalize(v33));

                angle1 = (acos(cosAngle1)/PI)*180;
                angle2 = (acos(cosAngle2)/PI)*180;
                angle3 = (acos(cosAngle3)/PI)*180;

                angle11 = (acos(cosAngle11)/PI)*180;
                angle22 = (acos(cosAngle22)/PI)*180;
                angle33 = (acos(cosAngle33)/PI)*180;

                pointsIndex.clear();
                branches[i].get_pointsIndex_of_branch(pointsIndex,nt);
                XYZ v = getBranchGlobalVector(pointsIndex);
                double cosAngle = dot(normalize(v),normalize(v1));
                double angle = (acos(cosAngle)/PI)*180;

                csvFile<<branches[i].level<<','<<branches[i].rLevel<<','<<branches[i].length<<','<<cIndexs.size()
                      <<','<<angle<<','<<angle1<<','<<angle2<<','<<angle3<<','<<angle11<<','<<angle22<<','<<angle33;

                bpIndex = i;
                cIndex1 = branchChildren[bpIndex][0];
                cIndex2 = branchChildren[bpIndex][1];

                pointsIndex.clear();
                branches[i].get_r_pointsIndex_of_branch(pointsIndex,nt);
                v1 = getBranchLocalVector(pointsIndex,d);
                v11 = getBranchGlobalVector(pointsIndex);

                pointsIndex.clear();
                branches[cIndex1].get_pointsIndex_of_branch(pointsIndex,nt);
                v2 = getBranchLocalVector(pointsIndex,d);
                v22 = getBranchGlobalVector(pointsIndex);

                pointsIndex.clear();
                branches[cIndex2].get_pointsIndex_of_branch(pointsIndex,nt);
                v3 = getBranchLocalVector(pointsIndex,d);
                v33 = getBranchGlobalVector(pointsIndex);

                cosAngle1 = dot(normalize(v1),normalize(v2));
                cosAngle2 = dot(normalize(v1),normalize(v3));
                cosAngle3 = dot(normalize(v2),normalize(v3));

                cosAngle11 = dot(normalize(v11),normalize(v22));
                cosAngle22 = dot(normalize(v11),normalize(v33));
                cosAngle33 = dot(normalize(v22),normalize(v33));

                angle1 = (acos(cosAngle1)/PI)*180;
                angle2 = (acos(cosAngle2)/PI)*180;
                angle3 = (acos(cosAngle3)/PI)*180;

                angle11 = (acos(cosAngle11)/PI)*180;
                angle22 = (acos(cosAngle22)/PI)*180;
                angle33 = (acos(cosAngle33)/PI)*180;

                csvFile<<','<<angle1<<','<<angle2<<','<<angle3<<','<<angle11<<','<<angle22<<','<<angle33<<endl;


            }else {
                csvFile<<branches[i].level<<','<<branches[i].rLevel<<','<<branches[i].length<<','<<cIndexs.size()<<endl;
            }
        }
    }
}

void BranchTree::pruningCross(double d){
    map<Branch*,int> branchMap = map<Branch*,int>();
    for(int i=0; i<branches.size(); ++i){
        branchMap[&(branches[i])] = i;
    }

    vector<vector<int> > branchChildren = vector<vector<int> >(branches.size(),vector<int>());
    for(int i=0; i<branches.size(); ++i){
        if(branches[i].parent != 0){
            branchChildren[branchMap[branches[i].parent]].push_back(i);
        }
    }

    int* branchFlag = new int[branches.size()];
    memset(branchFlag,0,sizeof(int)*branches.size());

    for(int i=0; i<branches.size(); i++){
        vector<int> cIndexs = vector<int>();
        XYZ v1,v2;
        vector<V3DLONG> pointsIndex = vector<V3DLONG>();
        branches[i].get_r_pointsIndex_of_branch(pointsIndex,nt);
        v1 = getBranchLocalVector(pointsIndex,d);
        double cosAngle;
        for(int j=0; j<branchChildren[i].size(); j++){
            int cIndex = branchChildren[i][j];
            if(branches[cIndex].length<d){
                branchFlag[cIndex] = 0;
            }else{
                pointsIndex.clear();
                branches[cIndex].get_pointsIndex_of_branch(pointsIndex,nt);
                v2 = getBranchLocalVector(pointsIndex,d);
                cosAngle = dot(normalize(v1),normalize(v2));
                if(cosAngle>0){
                    branchFlag[cIndex] = 1;
                    for(int k=1; k<pointsIndex.size()-1; k++){
                        nt.listNeuron[pointsIndex[k]].type = 0;
                    }
                }else {
                    branchFlag[cIndex] = 0;
                }
            }
        }
    }

    for(int i=0; i<branches.size(); i++){
        if(branchChildren[i].size() == 0 || branches[i].isNeighbor == 1){
            continue;
        }
        if(branchChildren[i].size()>2){
            branches[i].isNormalY = 0;
            for(int j=0; j<branchChildren[i].size(); j++){
                int cbIndex = branchChildren[i][j];
                if(branches[cbIndex].length<d){
                    branches[i].isNeighbor = 1;
                    branches[i].isNeighbor = 1;
                    branches[cbIndex].isNormalY = 0;
                }
            }
        }else {
            judgeIsNormalY(i,branchChildren,d);
            for(int j=0; j<2; j++){
                int cbIndex = branchChildren[i][j];
                if(branches[cbIndex].length<d){
                    branches[i].isNeighbor = 1;
                    branches[cbIndex].isNeighbor = 1;
                    if(branches[i].isNormalY == 0){
                        branches[cbIndex].isNormalY = 0;
                    }else {
                        judgeIsNormalY(cbIndex,branchChildren,d);
                        if(branches[cbIndex].isNormalY == 0){
                            branches[i].isNormalY = 0;
                        }
                    }
                }
            }
        }
    }

    for(int i=0; i<branches.size(); i++){
        vector<V3DLONG> pointsIndex = vector<V3DLONG>();
        branches[i].get_pointsIndex_of_branch(pointsIndex,nt);
        if(branchFlag[i] == 1 && branches[i].isNormalY == 0){
            for(int j=1; j<pointsIndex.size()-1; j++){
                nt.listNeuron[pointsIndex[j]].type = 2;
            }
        }else if (branches[i].inflectionNum>0 && branches[i].isNormalY == 0) {
            for(int j=pointsIndex.size()-1; j>0; j--){
                if(nt.listNeuron[pointsIndex[j]].type == 6){
                    break;
                }
                nt.listNeuron[pointsIndex[j]].type = 2;
            }
        }
    }

    if(branchFlag){
        delete[] branchFlag;
        branchFlag = 0;
    }
}

void BranchTree::calculateChildrenBranchAngle(ofstream &csvFile, double d){
    map<Branch*,int> branchMap = map<Branch*,int>();
    for(int i=0; i<branches.size(); ++i){
        branchMap[&(branches[i])] = i;
    }

    vector<vector<int> > branchChildren = vector<vector<int> >(branches.size(),vector<int>());
    for(int i=0; i<branches.size(); ++i){
        if(branches[i].parent != 0){
            branchChildren[branchMap[branches[i].parent]].push_back(i);
        }
    }

    for(int i=0; i<branches.size(); i++){
        if(branchChildren[i].size() == 0)
            continue;
        XYZ v1,v2;
        vector<V3DLONG> pointsIndex = vector<V3DLONG>();
        int cbIndex1 = branchChildren[i][0];
        int cbIndex2 = branchChildren[i][1];

        branches[cbIndex1].get_pointsIndex_of_branch(pointsIndex,nt);
        v1 = getBranchLocalVector(pointsIndex,d);

        pointsIndex.clear();
        branches[cbIndex2].get_pointsIndex_of_branch(pointsIndex,nt);
        v2 = getBranchLocalVector(pointsIndex,d);

        double cosAngle = dot(normalize(v1),normalize(v2));

        double angle = (acos(cosAngle)/PI)*180;

        csvFile<<branches[i].level<<','<<branches[i].rLevel<<','
              <<branches[i].length<<','<<branches[i].inflectionNum<<','<<angle<<endl;

    }
}

void BranchTree::calculateChildrenBranchGlobalAngle(ofstream &csvFile, double d){
    map<Branch*,int> branchMap = map<Branch*,int>();
    for(int i=0; i<branches.size(); ++i){
        branchMap[&(branches[i])] = i;
    }

    vector<vector<int> > branchChildren = vector<vector<int> >(branches.size(),vector<int>());
    for(int i=0; i<branches.size(); ++i){
        if(branches[i].parent != 0){
            branchChildren[branchMap[branches[i].parent]].push_back(i);
        }
    }

    vector<int> masterBranchesIndex = vector<int>();
    vector<int> level0Index = vector<int>();
    this->get_level_index(level0Index,0);

    for(int i=0; i<level0Index.size(); i++){
        int index = level0Index[i];
        if(branches[index].length<2*d){
            for(int j=0; j<branchChildren[index].size(); j++){
                int cbIndex = branchChildren[index][j];
                masterBranchesIndex.push_back(cbIndex);
            }
        }else {
            masterBranchesIndex.push_back(index);
        }
    }

    for(int i=0; i<masterBranchesIndex.size(); i++){
        int index = masterBranchesIndex[i];

        XYZ v1, v2;
        vector<V3DLONG> pointsIndex = vector<V3DLONG>();
        branches[index].get_r_pointsIndex_of_branch(pointsIndex,nt);
        v1 = getBranchLocalVector(pointsIndex,d);
        v1 = XYZ(0,0,0) - v1;

        vector<int> queue = vector<int>();
        for(int j=0; j<branchChildren[index].size(); j++){
            queue.push_back(branchChildren[index][j]);
        }

        while (!queue.empty()) {
            int tmpIndex = queue.front();
            queue.erase(queue.begin());

            pointsIndex.clear();
            branches[tmpIndex].get_pointsIndex_of_branch(pointsIndex,nt);
            v2 = getBranchGlobalVector(pointsIndex);

            double cosAngle = dot(normalize(v1),normalize(v2));
            double angle = (acos(cosAngle)/PI)*180;

            csvFile<<branches[tmpIndex].level<<','<<branches[tmpIndex].rLevel<<','
                  <<branches[tmpIndex].length<<','<<branches[tmpIndex].inflectionNum<<','<<angle<<endl;

            for(int j=0; j<branchChildren[tmpIndex].size(); j++){
                queue.push_back(branchChildren[tmpIndex][j]);
            }
        }

    }

}













