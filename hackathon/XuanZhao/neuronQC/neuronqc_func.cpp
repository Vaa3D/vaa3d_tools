#include "neuronqc_func.h"

#include <omp.h>
#include <sstream>

#include <algorithm>

#include "neuron_format_converter.h"

map<size_t, set<size_t> > seg2SegsMap;
V_NeuronSWC_list segList;
set<vector<size_t> > detectedLoops;
set<set<size_t> > detectedLoopsSet;
set<set<size_t> > finalizedLoopsSet;
set<set<size_t> > nonLoopErrors;
multimap<string, size_t> segEnd2segIDmap;
map<size_t, set<size_t> > segTail2segIDmap;
size_t rcCount;
size_t testCount;


bool judgeSort(const NeuronTree &nt, QString &sortInfo){
    bool isSort = true;
    sortInfo = "continuous";
    if(nt.listNeuron.empty()){
        return isSort;
    }
    int startN = nt.listNeuron[0].n;
    for(V3DLONG i=1; i<nt.listNeuron.size(); ++i){
        if(nt.listNeuron[i].n != startN + i){
            isSort = false;
            break;
        }
    }
    if(!isSort)
        sortInfo = "discontinuous";
    return isSort;
}


vector<NeuronTree> splitNeuronTree(const NeuronTree &nt){
    V3DLONG size = nt.listNeuron.size();
    vector<V3DLONG> rootIndexes;
    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(size,vector<V3DLONG>());
    for(V3DLONG i=0; i<size; ++i){
        V3DLONG prt = nt.listNeuron[i].parent;
        if(prt == -1 || nt.hashNeuron.find(prt) == nt.hashNeuron.end()){
            rootIndexes.push_back(i);
        }else{
            children[nt.hashNeuron.value(prt)].push_back(i);
        }
    }

    vector<NeuronTree> result;
    queue<V3DLONG> indexQ;
    for(int i=0; i<rootIndexes.size(); ++i){
        NeuronTree singleTree = NeuronTree();
        V3DLONG rootIndex = rootIndexes[i];
        if(nt.listNeuron[rootIndex].type != 1){
            NeuronSWC tmp = nt.listNeuron[rootIndex];
            ImageMarker m = ImageMarker(tmp.x,tmp.y,tmp.z);
            m.color.r = colortable[0][0];
            m.color.g = colortable[0][1];
            m.color.b = colortable[0][2];
            errorMarkerList.push_back(m);
        }
        indexQ.push(rootIndex);
        while (!indexQ.empty()) {
            V3DLONG index = indexQ.front();
            indexQ.pop();
            singleTree.listNeuron.push_back(nt.listNeuron[index]);
            for(int j=0; j<children[index].size(); ++j){
                V3DLONG cIndex = children[index][j];
                indexQ.push(cIndex);
            }
        }
        singleTree.hashNeuron.clear();
        for(int j=0; j<singleTree.listNeuron.size(); ++j){
            singleTree.hashNeuron.insert(singleTree.listNeuron[j].n,j);
        }
        result.push_back(singleTree);
    }

    return result;

}

bool judgeSomaType(const NeuronTree &nt, QString &somaTypeInfo){
    int count = 0;
    for(V3DLONG i=0; i<nt.listNeuron.size(); ++i){
        if(nt.listNeuron[i].type == 1){
            count++;
            if(nt.listNeuron[i].parent != -1){
                somaTypeInfo = "The first node with type 1 is not the root node";
                return false;
            }
        }
    }

    if(count == 0){
        somaTypeInfo = "There is no node of type 1";
        return false;
    }else if (count > 1) {
        somaTypeInfo = "There are multiple nodes of type 1";
        return false;
    }else{
        somaTypeInfo = "1";
        return true;
    }
}

int getShortBranchesCount(const NeuronTree &nt, float lengthThres){

    int sCount = 0;

    V3DLONG size = nt.listNeuron.size();
    vector<V3DLONG> tipIndexes;
    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(size,vector<V3DLONG>());
    for(V3DLONG i=0; i<size; ++i){
        V3DLONG prt = nt.listNeuron[i].parent;
        if(prt == -1 || nt.hashNeuron.find(prt) == nt.hashNeuron.end()){
            continue;
        }else{
            children[nt.hashNeuron.value(prt)].push_back(i);
        }
    }
    for(V3DLONG i=0; i<size; ++i){
        if(children[i].size() == 0)
            tipIndexes.push_back(i);
    }

    for(int i=0; i<tipIndexes.size(); ++i){
        V3DLONG tipIndex = tipIndexes[i];
        float tipBranchLength = 0;
        V3DLONG curIndex = tipIndex;
        V3DLONG prt  = nt.listNeuron[tipIndex].parent;
        while(prt != -1 && nt.hashNeuron.find(prt) != nt.hashNeuron.end()){
            V3DLONG prtIndex = nt.hashNeuron.value(prt);
            tipBranchLength += zx_dist(nt.listNeuron[prtIndex],nt.listNeuron[curIndex]);
            curIndex = prtIndex;
            if(children[prtIndex].size()>1){
                break;
            }
            prt = nt.listNeuron[curIndex].parent;
        }
        if(tipBranchLength < lengthThres){
            NeuronSWC tmp = nt.listNeuron[tipIndex];
            ImageMarker m = ImageMarker(tmp.x,tmp.y,tmp.z);
            m.color.r = colortable[4][0];
            m.color.g = colortable[4][1];
            m.color.b = colortable[4][2];
            errorMarkerList.push_back(m);
            sCount++;
        }
    }

    return sCount;
}

bool getMinMaxNodePath(const NeuronTree &nt, float &minPath, float &maxPath, float lengthThres){
    minPath = INT_MAX;
    maxPath = 0;
    bool isNormal = true;
    for(V3DLONG i=0; i<nt.listNeuron.size(); ++i){
        V3DLONG prt = nt.listNeuron[i].parent;
        if(prt != -1 && nt.hashNeuron.find(prt) != nt.hashNeuron.end()){
            V3DLONG prtIndex = nt.hashNeuron.value(prt);
            float length = zx_dist(nt.listNeuron[prtIndex],nt.listNeuron[i]);
            if(length>lengthThres){
                NeuronSWC tmp = nt.listNeuron[i];
                ImageMarker m = ImageMarker(tmp.x,tmp.y,tmp.z);
                m.color.r = colortable[5][0];
                m.color.g = colortable[5][1];
                m.color.b = colortable[5][2];
                errorMarkerList.push_back(m);
                isNormal = false;
                minPath = minPath < length ? minPath : length;
                maxPath = maxPath > length ? maxPath : length;
            }
        }
    }

    return isNormal;
}

bool judgeTypes(const NeuronTree &nt, string& allTypesInfo){
    set<int> allTypesSet;
    bool connect = true, allType = true;
    for(V3DLONG i=0; i<nt.listNeuron.size(); ++i){
        int type = nt.listNeuron[i].type;
        V3DLONG prt = nt.listNeuron[i].parent;
        switch (type) {
        case 1:
            if(prt != -1 && nt.hashNeuron.find(prt) != nt.hashNeuron.end()){
                NeuronSWC tmp = nt.listNeuron[i];
                ImageMarker m = ImageMarker(tmp.x,tmp.y,tmp.z);
                m.color.r = colortable[1][0];
                m.color.g = colortable[1][1];
                m.color.b = colortable[1][2];
                errorMarkerList.push_back(m);
                connect = false;
            }
            break;
        case 2:
            if(prt != -1 && nt.hashNeuron.find(prt) != nt.hashNeuron.end()){
                V3DLONG prtIndex = nt.hashNeuron.value(prt);
                int prtType = nt.listNeuron[prtIndex].type;
                if(prtType != 2 && prtType != 1 && prtType != 3){
                    NeuronSWC tmp = nt.listNeuron[i];
                    ImageMarker m = ImageMarker(tmp.x,tmp.y,tmp.z);
                    m.color.r = colortable[1][0];
                    m.color.g = colortable[1][1];
                    m.color.b = colortable[1][2];
                    errorMarkerList.push_back(m);
                    connect = false;
                }
            }
            break;
        case 3:
            if(prt != -1 && nt.hashNeuron.find(prt) != nt.hashNeuron.end()){
                V3DLONG prtIndex = nt.hashNeuron.value(prt);
                int prtType = nt.listNeuron[prtIndex].type;
                if(prtType != 3 && prtType != 1){
                    NeuronSWC tmp = nt.listNeuron[i];
                    ImageMarker m = ImageMarker(tmp.x,tmp.y,tmp.z);
                    m.color.r = colortable[1][0];
                    m.color.g = colortable[1][1];
                    m.color.b = colortable[1][2];
                    errorMarkerList.push_back(m);
                    connect = false;
                }
            }
            break;
        case 4:
            if(prt != -1 && nt.hashNeuron.find(prt) != nt.hashNeuron.end()){
                V3DLONG prtIndex = nt.hashNeuron.value(prt);
                int prtType = nt.listNeuron[prtIndex].type;
                if(prtType != 4 && prtType != 1){
                    NeuronSWC tmp = nt.listNeuron[i];
                    ImageMarker m = ImageMarker(tmp.x,tmp.y,tmp.z);
                    m.color.r = colortable[1][0];
                    m.color.g = colortable[1][1];
                    m.color.b = colortable[1][2];
                    errorMarkerList.push_back(m);
                    connect = false;
                }
            }
            break;
        default:
            NeuronSWC tmp = nt.listNeuron[i];
            ImageMarker m = ImageMarker(tmp.x,tmp.y,tmp.z);
            m.color.r = colortable[1][0];
            m.color.g = colortable[1][1];
            m.color.b = colortable[1][2];
            errorMarkerList.push_back(m);
            break;
        }
        allTypesSet.insert(type);
    }
    if(allTypesSet.size() == 3){
        if(allTypesSet.find(1) != allTypesSet.end()
                && allTypesSet.find(2) != allTypesSet.end()
                && allTypesSet.find(3) != allTypesSet.end()){
            allType = true;
        }else{
            allType = false;
        }
    }else if(allTypesSet.size() == 4){
        if(allTypesSet.find(1) != allTypesSet.end()
                && allTypesSet.find(2) != allTypesSet.end()
                && allTypesSet.find(3) != allTypesSet.end()
                && allTypesSet.find(4) != allTypesSet.end()){
            allType = true;
        }else{
            allType = false;
        }
    }else{
        allType = false;
    }

    set<int>::iterator typeIterator = allTypesSet.begin();
    for(typeIterator; typeIterator != allTypesSet.end(); typeIterator++){
        allTypesInfo += (to_string(*typeIterator) + " ");
    }
    if(!connect){
        allTypesInfo += "type connect false is existed";
    }

    return connect && allType;
}

int getThreeBifurcationCount(const vector<NeuronSWC> &outputErroneousPoints){
    int count = 0;
    for(int i=0; i<outputErroneousPoints.size(); ++i){
        if(outputErroneousPoints[i].type == 20){
            NeuronSWC tmp = outputErroneousPoints[i];
            ImageMarker m = ImageMarker(tmp.x,tmp.y,tmp.z);
            m.color.r = colortable[3][0];
            m.color.g = colortable[3][1];
            m.color.b = colortable[3][2];
            errorMarkerList.push_back(m);
            count++;
        }
    }
    return count;
}

int getLoopCount(const vector<NeuronSWC> &outputErroneousPoints){
    int count = 0;
    for(int i=0; i<outputErroneousPoints.size(); ++i){
        if(outputErroneousPoints[i].type == 15){
            NeuronSWC tmp = outputErroneousPoints[i];
            ImageMarker m = ImageMarker(tmp.x,tmp.y,tmp.z);
            m.color.r = colortable[2][0];
            m.color.g = colortable[2][1];
            m.color.b = colortable[2][2];
            errorMarkerList.push_back(m);
            count++;
        }
    }
    return count;
}

bool getNeuronFeature(const NeuronTree &nt, neuronQCFeature &nqcf, float sLengthThres, float nodeLengthThres, bool loopThreeBifurcation){
    errorMarkerList.clear();

    //isSort
    QString sortInfo;
    bool isSort = judgeSort(nt,sortInfo);
    if(!isSort){
        nqcf.bFeature.push_back(isSort);
        nqcf.featureInfo.push_back(sortInfo.toStdString());
        string fInfo = "The swc is not sorted";
        for(int i=1; i<featureSize; ++i){
            nqcf.bFeature.push_back(isSort);
            nqcf.featureInfo.push_back(fInfo);
        }
        return false;
    }else {
        nqcf.bFeature.push_back(isSort);
        nqcf.featureInfo.push_back(sortInfo.toStdString());
    }
    //somaType
    QString somaTypeInfo;
    bool somaType = judgeSomaType(nt,somaTypeInfo);
    if(!somaType){
        nqcf.bFeature.push_back(somaType);
        nqcf.featureInfo.push_back(somaTypeInfo.toStdString());
        string fInfo = "The somaType is false";
        for(int i=2; i<featureSize; ++i){
            nqcf.bFeature.push_back(isSort);
            nqcf.featureInfo.push_back(fInfo);
        }
        return false;
    }else{
        nqcf.bFeature.push_back(somaType);
        nqcf.featureInfo.push_back(somaTypeInfo.toStdString());
    }
    //gap
    vector<NeuronTree> trees = splitNeuronTree(nt);
    bool gap = true;
    if(trees.size()>1){
        gap = false;
    }
    string gapInfo = to_string(trees.size()-1);
    nqcf.bFeature.push_back(gap);
    nqcf.featureInfo.push_back(gapInfo);
    //allTypes
    string allTypesInfo;
    bool allTypes = judgeTypes(nt,allTypesInfo);
    nqcf.bFeature.push_back(allTypes);
    nqcf.featureInfo.push_back(allTypesInfo);
    //loop, threeBifurcation
    if(loopThreeBifurcation){
        int loopCount = 0, threeBifurcationCount = 0;
        for(int i=0; i<trees.size(); ++i){
            V_NeuronSWC_list inputSegList = NeuronTree__2__V_NeuronSWC_list(trees[i]);
            vector<NeuronSWC> outputErroneousPoints = loopDetection(inputSegList);
            loopCount += getLoopCount(outputErroneousPoints);
            threeBifurcationCount += getThreeBifurcationCount(outputErroneousPoints);
        }
        nqcf.bFeature.push_back(loopCount == 0);
        nqcf.featureInfo.push_back(to_string(loopCount));
        nqcf.bFeature.push_back((threeBifurcationCount - 1) <= 0);
        nqcf.featureInfo.push_back(to_string(threeBifurcationCount - 1));
    }else{
        nqcf.bFeature.push_back(true);
        nqcf.featureInfo.push_back("the feature is unchecked");
        nqcf.bFeature.push_back(true);
        nqcf.featureInfo.push_back("the feature is unchecked");
    }

    //short Branches
    int sBranchesCount = getShortBranchesCount(nt,sLengthThres);
    nqcf.bFeature.push_back(sBranchesCount == 0);
    nqcf.featureInfo.push_back(to_string(sBranchesCount));
    //nodeLength
    float minPath,maxPath;
    bool nodeLength = getMinMaxNodePath(nt,minPath,maxPath,nodeLengthThres);
    string nodeLengthInfo = "minLength = " + to_string(minPath) + " maxLength = " +to_string(maxPath);
    if(nodeLength)
        nodeLengthInfo = "the node length is normal";
    nqcf.bFeature.push_back(nodeLength);
    nqcf.featureInfo.push_back(nodeLengthInfo);

    return true;
}

bool writeNeuronFeature(vector<neuronQCFeature> nqcfv, vector<string> neuronId, ofstream &csvFile){
    csvFile<<"neuronId"<<',';
    for(int i=0; i<featureSize; ++i){
        csvFile<<featureName[i]<<','<<(featureName[i] + " Info")<<',';
    }
    csvFile<<"isQualified"<<endl;
    for(int i=0; i<nqcfv.size(); ++i){
        bool qualified = true;
        csvFile<<neuronId[i]<<',';
        for(int j=0; j<featureSize; ++j){
            if(!nqcfv[i].bFeature[j] && qualified)
                qualified = false;
            csvFile<<nqcfv[i].bFeature[j]<<','<<nqcfv[i].featureInfo[j]<<',';
        }
        csvFile<<qualified<<endl;
    }
    return true;
}

bool writeErrorMarkerList(QString errorMarkersFile){
    return writeMarker_file(errorMarkersFile,errorMarkerList);
}

bool writeErrorApoList(QString errorMarkersFile){
    QList<CellAPO> apoList;
    for(int i=0; i<errorMarkerList.size(); ++i){
        CellAPO tmp;
        tmp.x = errorMarkerList[i].x;
        tmp.y = errorMarkerList[i].y;
        tmp.z = errorMarkerList[i].z;
        tmp.color.r = errorMarkerList[i].color.r;
        tmp.color.g = errorMarkerList[i].color.g;
        tmp.color.b = errorMarkerList[i].color.b;
        apoList.push_back(tmp);
    }
    return writeAPO_file(errorMarkersFile,apoList);
}

bool getNeuronFeatureForBatch(QString swcDir, float sLengthThres, float nodeLengthThres, bool loopThreeBifurcation, ofstream & csvFile){
    QDir dir(swcDir);
    if(!dir.exists()){
        qDebug()<<"the dir is not exists";
        return false;
    }
    dir.setFilter(QDir::Files|QDir::NoSymLinks);
    QStringList filters;
    filters<<QString("*.swc")<<QString("*.eswc");
    dir.setNameFilters(filters);
    QFileInfoList swcPathInfo = dir.entryInfoList();
    vector<string> neuronId;
    vector<neuronQCFeature> nqcfv;
    for(int i=0; i<swcPathInfo.size(); ++i){
        neuronId.push_back(swcPathInfo[i].completeBaseName().toStdString());
        QString swcPath = swcPathInfo[i].absoluteFilePath();
        NeuronTree nt = readSWC_file(swcPath);
        neuronQCFeature nqcf;
        getNeuronFeature(nt,nqcf,sLengthThres,nodeLengthThres,loopThreeBifurcation);
        nqcfv.push_back(nqcf);

        QString aposPath = swcPath + ".apo";
        writeErrorApoList(aposPath);
        QString anoPath = swcPath + ".ano";
        ofstream ano;
        ano.open(anoPath.toStdString().c_str(),ios::out);
        ano<<"APOFILE="<<(swcPathInfo[i].fileName() + ".apo").toStdString().c_str()<<endl;
        ano<<"SWCFILE="<<(swcPathInfo[i].fileName()).toStdString().c_str()<<endl;
        ano.close();

    }
    writeNeuronFeature(nqcfv,neuronId,csvFile);
    return true;
}


vector<NeuronSWC> loopDetection(V_NeuronSWC_list inputSegList)
{
#ifdef Q_OS_WIN32
    char* numProcsC;
    numProcsC = getenv("NUMBER_OF_PROCESSORS");
    string numProcsString(numProcsC);
    int numProcs = stoi(numProcsString);
#endif

    vector<NeuronSWC> outputErroneousPoints;
    outputErroneousPoints.clear();

    segList = inputSegList;
    seg2SegsMap.clear();
    segTail2segIDmap.clear();
    map<string, set<size_t> > wholeGrid2segIDmap;
    set<size_t> subtreeSegs;
    size_t segCount = 0;
    for (vector<V_NeuronSWC>::iterator segIt = segList.seg.begin(); segIt != segList.seg.end(); ++segIt)
    {
        for (vector<V_NeuronSWC_unit>::iterator nodeIt = segIt->row.begin(); nodeIt != segIt->row.end(); ++nodeIt)
        {
            int xLabel = nodeIt->x / GRID_LENGTH;
            int yLabel = nodeIt->y / GRID_LENGTH;
            int zLabel = nodeIt->z / GRID_LENGTH;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            wholeGrid2segIDmap[gridKey].insert(size_t(segIt - segList.seg.begin()));
        }
        subtreeSegs.insert(segCount);
        ++segCount;
    }

    for (set<size_t>::iterator it = subtreeSegs.begin(); it != subtreeSegs.end(); ++it)
    {
        //cout << *it << ":";
        set<size_t> connectedSegs;
        connectedSegs.clear();
        if (segList.seg[*it].row.size() <= 1)
        {
            segList.seg[*it].to_be_deleted = true;
            continue;
        }
        else if (segList.seg[*it].to_be_deleted) continue;

        for (vector<V_NeuronSWC_unit>::iterator nodeIt = segList.seg[*it].row.begin(); nodeIt != segList.seg[*it].row.end(); ++nodeIt)
        {
            int xLabel = nodeIt->x / GRID_LENGTH;
            int yLabel = nodeIt->y / GRID_LENGTH;
            int zLabel = nodeIt->z / GRID_LENGTH;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();

            set<size_t> scannedSegs = wholeGrid2segIDmap[gridKey];
            if (!scannedSegs.empty())
            {
                for (set<size_t>::iterator scannedIt = scannedSegs.begin(); scannedIt != scannedSegs.end(); ++scannedIt)
                {
                    if (*scannedIt == *it || segList.seg[*scannedIt].to_be_deleted) continue;
                    if (segList.seg[*scannedIt].row.size() == 1) continue;

                    if (segList.seg[*scannedIt].row.begin()->x == nodeIt->x && segList.seg[*scannedIt].row.begin()->y == nodeIt->y && segList.seg[*scannedIt].row.begin()->z == nodeIt->z)
                    {
                        connectedSegs.insert(*scannedIt);
                        set<size_t> reversed;
                        reversed.insert(*it);
                        if (!seg2SegsMap.insert(pair<size_t, set<size_t> >(*scannedIt, reversed)).second) seg2SegsMap[*scannedIt].insert(*it);
                        if (!segTail2segIDmap.insert(pair<size_t, set<size_t> >(*scannedIt, reversed)).second) segTail2segIDmap[*scannedIt].insert(*it);
                    }
                    else if ((segList.seg[*scannedIt].row.end() - 1)->x == nodeIt->x && (segList.seg[*scannedIt].row.end() - 1)->y == nodeIt->y && (segList.seg[*scannedIt].row.end() - 1)->z == nodeIt->z)
                    {
                        connectedSegs.insert(*scannedIt);
                        set<size_t> reversed;
                        reversed.insert(*it);
                        if (!seg2SegsMap.insert(pair<size_t, set<size_t> >(*scannedIt, reversed)).second) seg2SegsMap[*scannedIt].insert(*it);
                    }
                }
            }
        }
        if (!seg2SegsMap.insert(pair<size_t, set<size_t> >(*it, connectedSegs)).second)
        {
            for (set<size_t>::iterator otherSegIt = connectedSegs.begin(); otherSegIt != connectedSegs.end(); ++otherSegIt)
                seg2SegsMap[*it].insert(*otherSegIt);
        }
    }

//	cout << endl << "Starting loop detection.. " << endl;
    clock_t begin = clock();
    detectedLoopsSet.clear();
    finalizedLoopsSet.clear();
    nonLoopErrors.clear();
    rcCount = 0;
    testCount = 0;
#ifdef Q_OS_WIN32
#pragma omp parallel num_threads(this->numProcs)
    {
#endif
        for (map<size_t, set<size_t> >::iterator it = seg2SegsMap.begin(); it != seg2SegsMap.end(); ++it)
        {
            if (it->second.size() <= 2) continue;

            vector<size_t> loops2ThisSeg;
            loops2ThisSeg.clear();

            rc_loopPathCheck(it->first, loops2ThisSeg);
        }
#ifdef Q_OS_WIN32
    }
#endif
    //cout << "rc count: " << rcCount << endl;
    //cout << "test count: " << testCount << endl;
    clock_t end = clock();
    float elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    cout << "TIME ELAPSED: " << elapsed_secs << " SECS" << endl << endl;

    if (finalizedLoopsSet.empty()) return outputErroneousPoints;
    else
    {
        int loopCount = 0;
        for (set<set<size_t> >::iterator loopIt = finalizedLoopsSet.begin(); loopIt != finalizedLoopsSet.end(); ++loopIt)
        {
            set<size_t> thisLoop = *loopIt;
            int jointCount = 0;
            vector<size_t> thisSet;
            for (set<size_t>::iterator setIt = loopIt->begin(); setIt != loopIt->end(); ++setIt) thisSet.push_back(*setIt);

            V_NeuronSWC_unit jointNode;
            for (vector<V_NeuronSWC_unit>::iterator seg1It = segList.seg.at(thisSet.at(0)).row.begin(); seg1It != segList.seg.at(thisSet.at(1)).row.end(); ++seg1It)
            {
                for (vector<V_NeuronSWC_unit>::iterator seg2It = segList.seg.at(thisSet.at(1)).row.begin(); seg2It != segList.seg.at(thisSet.at(1)).row.end(); ++seg2It)
                {
                    if (seg1It->x == seg2It->x && seg1It->y == seg2It->y && seg1It->z == seg2It->z)
                    {
                        jointNode = *seg1It;
                        jointCount = 2;
                        goto FOUND_JOINT_NODE;
                    }
                }
            }

            ++loopCount;
            for (set<size_t>::iterator it = thisLoop.begin(); it != thisLoop.end(); ++it)
            {
                cout << *it << " ";
                for (vector<V_NeuronSWC_unit>::iterator unitIt = segList.seg[*it].row.begin(); unitIt != segList.seg[*it].row.end(); ++unitIt)
                {
                    unitIt->type = 15;

                    if (unitIt->parent == -1)
                    {
                        NeuronSWC problematicNode;
                        problematicNode.x = unitIt->x;
                        problematicNode.y = unitIt->y;
                        problematicNode.z = unitIt->z;
                        problematicNode.type = 15;
                        problematicNode.parent = unitIt->parent;
                        problematicNode.n = unitIt->n;
                        outputErroneousPoints.push_back(problematicNode);
                    }
                }
            }
            cout << endl << endl;
            continue;

        FOUND_JOINT_NODE:
            for (vector<size_t>::iterator segIt = thisSet.begin() + 2; segIt != thisSet.end(); ++segIt)
            {
                for (vector<V_NeuronSWC_unit>::iterator nodeIt = segList.seg.at(*segIt).row.begin(); nodeIt != segList.seg.at(*segIt).row.end(); ++nodeIt)
                {
                    if (nodeIt->x == jointNode.x && nodeIt->y == jointNode.y && nodeIt->z == jointNode.z) ++jointCount;
                }
            }

            if (jointCount == loopIt->size()) nonLoopErrors.insert(*loopIt);
            else
            {
                ++loopCount;
                for (set<size_t>::iterator it = thisLoop.begin(); it != thisLoop.end(); ++it)
                {
                    //cout << *it << " ";
                    for (vector<V_NeuronSWC_unit>::iterator unitIt = segList.seg[*it].row.begin(); unitIt != segList.seg[*it].row.end(); ++unitIt)
                    {
                        unitIt->type = 15;

                        if (unitIt->parent == -1)
                        {
                            NeuronSWC problematicNode;
                            problematicNode.x = unitIt->x;
                            problematicNode.y = unitIt->y;
                            problematicNode.z = unitIt->z;
                            problematicNode.type = 15;
                            problematicNode.parent = unitIt->parent;
                            problematicNode.n = unitIt->n;
                            outputErroneousPoints.push_back(problematicNode);
                        }
                    }
                }
                //cout << endl << endl;
            }
        }
        cout << "LOOPS NUMBER (set): " << loopCount << endl << endl;

        if (!nonLoopErrors.empty())
        {
            while (1)
            {
                for (set<set<size_t> >::iterator setCheckIt1 = nonLoopErrors.begin(); setCheckIt1 != nonLoopErrors.end(); ++setCheckIt1)
                {
                    for (set<set<size_t> >::iterator setCheckIt2 = nonLoopErrors.begin(); setCheckIt2 != nonLoopErrors.end(); ++setCheckIt2)
                    {
                        if (setCheckIt1 == setCheckIt2) continue;
                        else
                        {
                            int segNum = 0;
                            for (set<size_t>::iterator segCheck1 = setCheckIt1->begin(); segCheck1 != setCheckIt1->end(); ++segCheck1)
                                if (setCheckIt2->find(*segCheck1) != setCheckIt2->end()) ++segNum;

                            if (segNum == setCheckIt1->size())
                            {
                                nonLoopErrors.erase(setCheckIt1);
                                goto SET_ERASED;
                            }
                        }
                    }
                }
                break;

            SET_ERASED:
                continue;
            }

            for (set<set<size_t> >::iterator loopIt = nonLoopErrors.begin(); loopIt != nonLoopErrors.end(); ++loopIt)
            {
                map<string, set<size_t> > headCountMap;
                map<string, set<size_t> > tailCountMap;
                bool head = true, tail = false;

                set<size_t> thisLoop = *loopIt;
                for (set<size_t>::iterator it = thisLoop.begin(); it != thisLoop.end(); ++it)
                {
                    std::ostringstream ssHeadx, ssHeady, ssHeadz;
                    std::ostringstream ssTailx, ssTaily, ssTailz;
                    ssHeadx << (segList.seg[*it].row.end() - 1)->x; string headX(ssHeadx.str());
                    ssHeady << (segList.seg[*it].row.end() - 1)->y; string headY(ssHeady.str());
                    ssHeadz << (segList.seg[*it].row.end() - 1)->z; string headZ(ssHeadz.str());
                    ssTailx << segList.seg[*it].row.begin()->x;     string tailX(ssTailx.str());
                    ssTaily << segList.seg[*it].row.begin()->y;     string tailY(ssTaily.str());
                    ssTailz << segList.seg[*it].row.begin()->z;     string tailZ(ssTailz.str());
                    string headLabel = headX + " " + headY + " " + headZ;
                    string tailLabel = tailX + " " + tailY + " " + tailZ;

                    //string headLabel = to_string((segList.seg[*it].row.end() - 1)->x) + " " + to_string((segList.seg[*it].row.end() - 1)->y) + " " + to_string((segList.seg[*it].row.end() - 1)->z);
                    //string tailLabel = to_string(segList.seg[*it].row.begin()->x) + " " + to_string(segList.seg[*it].row.begin()->y) + " " + to_string(segList.seg[*it].row.begin()->z);
                    headCountMap[headLabel].insert(*it);
                    tailCountMap[tailLabel].insert(*it);
                }

                int overlapCount = 0;
                size_t segNum;
                for (map<string, set<size_t> >::iterator countIt = headCountMap.begin(); countIt != headCountMap.end(); ++countIt)
                {
                    if (countIt->second.size() > overlapCount)
                    {
                        overlapCount = countIt->second.size();
                        segNum = *countIt->second.begin();
                    }
                }
                for (map<string, set<size_t> >::iterator countIt = tailCountMap.begin(); countIt != tailCountMap.end(); ++countIt)
                {
                    if (countIt->second.size() > overlapCount)
                    {
                        overlapCount = countIt->second.size();
                        segNum = *countIt->second.begin();
                        head = false;
                        tail = true;
                    }
                }

                // Zhi: The following identifies the node that has the most duplication in 3 way fork situ. Please make changes you need here.
                if (head)
                {
                    NeuronSWC problematicNode;
                    problematicNode.x = (segList.seg[segNum].row.end() - 1)->x;
                    problematicNode.y = (segList.seg[segNum].row.end() - 1)->y;
                    problematicNode.z = (segList.seg[segNum].row.end() - 1)->z;
                    problematicNode.type = 20;
                    outputErroneousPoints.push_back(problematicNode);
                    //cout << problematicNode.x << " " << problematicNode.y << " " << problematicNode.z << endl;
                }
                else if (tail)
                {
                    NeuronSWC problematicNode;
                    problematicNode.x = segList.seg[segNum].row.begin()->x;
                    problematicNode.y = segList.seg[segNum].row.begin()->y;
                    problematicNode.z = segList.seg[segNum].row.begin()->z;
                    problematicNode.type = 20;
                    outputErroneousPoints.push_back(problematicNode);
                    //cout << problematicNode.x << " " << problematicNode.y << " " << problematicNode.z << endl;
                }
            }
        }
    }

    return outputErroneousPoints;
}

void rc_loopPathCheck(size_t inputSegID, vector<size_t> curPathWalk)
{
    //++rcCount;

    if (seg2SegsMap[inputSegID].size() < 2)	return;

    //cout << "  input seg num: " << inputSegID << " ";
    curPathWalk.push_back(inputSegID);
    /*for (vector<size_t>::iterator curPathIt = curPathWalk.begin(); curPathIt != curPathWalk.end(); ++curPathIt)
    cout << *curPathIt << " ";
    cout << endl << endl;*/

    for (set<size_t>::iterator it = seg2SegsMap[inputSegID].begin(); it != seg2SegsMap[inputSegID].end(); ++it)
    {
        if (segTail2segIDmap.find(*it) == segTail2segIDmap.end())
        {
            //++testCount;
            continue;
        }

        if (curPathWalk.size() >= 2 && *it == *(curPathWalk.end() - 2))
        {
            V_NeuronSWC_unit headUnit = *(segList.seg[*it].row.end() - 1);
            V_NeuronSWC_unit tailUnit = *segList.seg[*it].row.begin();

            bool headCheck = false, tailCheck = false;
            for (vector<V_NeuronSWC_unit>::iterator it = segList.seg[*(curPathWalk.end() - 1)].row.begin(); it != segList.seg[*(curPathWalk.end() - 1)].row.end(); ++it)
            {
                if (it->x == headUnit.x && it->y == headUnit.y && it->z == headUnit.z) headCheck = true;
                if (it->x == tailUnit.x && it->y == tailUnit.y && it->z == tailUnit.z) tailCheck = true;
            }

            if (headCheck == true && tailCheck == true)
            {
                set<size_t> detectedLoopPathSet;
                detectedLoopPathSet.clear();
                for (vector<size_t>::iterator loopIt = find(curPathWalk.begin(), curPathWalk.end(), *it); loopIt != curPathWalk.end(); ++loopIt)
                    detectedLoopPathSet.insert(*loopIt);

                if (detectedLoopsSet.insert(detectedLoopPathSet).second)
                {
                    nonLoopErrors.insert(detectedLoopPathSet);
                    continue;
                }
                else return;
            }
            else continue;
        }

        if (find(curPathWalk.begin(), curPathWalk.end(), *it) == curPathWalk.end())
        {
            rc_loopPathCheck(*it, curPathWalk);
        }
        else
        {
            // a loop is found
            set<size_t> detectedLoopPathSet;
            detectedLoopPathSet.clear();
            for (vector<size_t>::iterator loopIt = find(curPathWalk.begin(), curPathWalk.end(), *it); loopIt != curPathWalk.end(); ++loopIt)
                detectedLoopPathSet.insert(*loopIt);

            if (detectedLoopsSet.insert(detectedLoopPathSet).second)
            {
                // pusedoloop by fork intersection check
                //cout << "pusedoloop check.." << endl;

                if (*(curPathWalk.end() - 3) == *it)
                {
                    if (seg2SegsMap[*(curPathWalk.end() - 2)].find(*it) != seg2SegsMap[*(curPathWalk.end() - 2)].end())
                    {
                        vector<V_NeuronSWC_unit> forkCheck;
                        forkCheck.push_back(*(segList.seg[*(curPathWalk.end() - 1)].row.end() - 1));
                        forkCheck.push_back(*segList.seg[*(curPathWalk.end() - 1)].row.begin());
                        forkCheck.push_back(*(segList.seg[*(curPathWalk.end() - 2)].row.end() - 1));
                        forkCheck.push_back(*segList.seg[*(curPathWalk.end() - 2)].row.begin());

                        int headConnectedCount = 0;
                        for (vector<V_NeuronSWC_unit>::iterator checkIt = forkCheck.begin(); checkIt != forkCheck.end(); ++checkIt)
                        {
                            if (checkIt->x == (segList.seg[*it].row.end() - 1)->x && checkIt->y == (segList.seg[*it].row.end() - 1)->y && checkIt->z == (segList.seg[*it].row.end() - 1)->z)
                                ++headConnectedCount;
                        }

                        int tailConnectedCount = 0;
                        for (vector<V_NeuronSWC_unit>::iterator checkIt = forkCheck.begin(); checkIt != forkCheck.end(); ++checkIt)
                        {
                            if (checkIt->x == segList.seg[*it].row.begin()->x && checkIt->y == segList.seg[*it].row.begin()->y && checkIt->z == segList.seg[*it].row.begin()->z)
                                ++tailConnectedCount;
                        }

                        if (!(headConnectedCount == 1 && tailConnectedCount == 1))
                        {
                            cout << "  -> 3 seg intersection detected, exluded from loop candidates. (" << *it << ") ";
                            for (set<size_t>::iterator thisLoopIt = detectedLoopPathSet.begin(); thisLoopIt != detectedLoopPathSet.end(); ++thisLoopIt)
                                cout << *thisLoopIt << " ";
                            cout << endl;
                            continue;
                        }
                        else
                        {
                            finalizedLoopsSet.insert(detectedLoopPathSet);
                            cout << "  Loop from 3 way detected ----> (" << *it << ") ";
                            for (set<size_t>::iterator thisLoopIt = detectedLoopPathSet.begin(); thisLoopIt != detectedLoopPathSet.end(); ++thisLoopIt)
                                cout << *thisLoopIt << " ";
                            cout << endl;
                            return;
                        }
                    }
                }
                else if (curPathWalk.size() == 4)
                {
                    if ((*curPathWalk.end() - 4) == *it)
                    {
                        if (seg2SegsMap[*(curPathWalk.end() - 2)].find(*it) != seg2SegsMap[*(curPathWalk.end() - 2)].end() &&
                            seg2SegsMap[*(curPathWalk.end() - 3)].find(*it) != seg2SegsMap[*(curPathWalk.end() - 3)].end())
                        {
                            if (seg2SegsMap[*(curPathWalk.end() - 2)].find(*(curPathWalk.end() - 1)) != seg2SegsMap[*(curPathWalk.end() - 2)].end() &&
                                seg2SegsMap[*(curPathWalk.end() - 3)].find(*(curPathWalk.end() - 2)) != seg2SegsMap[*(curPathWalk.end() - 3)].end() &&
                                seg2SegsMap[*(curPathWalk.end() - 4)].find(*(curPathWalk.end() - 3)) != seg2SegsMap[*(curPathWalk.end() - 4)].end())
                            {
                                nonLoopErrors.insert(detectedLoopPathSet);
                                cout << "  -> 4 way intersection detected ----> (" << *it << ") ";
                                for (set<size_t>::iterator thisLoopIt = detectedLoopPathSet.begin(); thisLoopIt != detectedLoopPathSet.end(); ++thisLoopIt)
                                    cout << *thisLoopIt << " ";
                                cout << endl << endl;
                            }
                        }
                    }
                }
                else
                {
                    finalizedLoopsSet.insert(detectedLoopPathSet);
                    cout << "  Topological loop identified ----> (" << *it << ") ";
                    for (set<size_t>::iterator thisLoopIt = detectedLoopPathSet.begin(); thisLoopIt != detectedLoopPathSet.end(); ++thisLoopIt)
                        cout << *thisLoopIt << " ";
                    cout << endl << endl;

                    while (1)
                    {
                        for (set<set<size_t> >::iterator setCheckIt1 = finalizedLoopsSet.begin(); setCheckIt1 != finalizedLoopsSet.end(); ++setCheckIt1)
                        {
                            for (set<set<size_t> >::iterator setCheckIt2 = finalizedLoopsSet.begin(); setCheckIt2 != finalizedLoopsSet.end(); ++setCheckIt2)
                            {
                                if (setCheckIt1 == setCheckIt2) continue;
                                else
                                {
                                    int segNum = 0;
                                    for (set<size_t>::iterator segCheck1 = setCheckIt1->begin(); segCheck1 != setCheckIt1->end(); ++segCheck1)
                                        if (setCheckIt2->find(*segCheck1) != setCheckIt2->end()) ++segNum;

                                    if (segNum == setCheckIt1->size())
                                    {
                                        finalizedLoopsSet.erase(setCheckIt1);
                                        goto SET_ERASED;
                                    }
                                }
                            }
                        }
                        break;

                    SET_ERASED:
                        continue;
                    }
                }
            }
            else return;
        }
    }

    curPathWalk.pop_back();
    //cout << endl;
}

vector<V_NeuronSWC_list> showConnectedSegs(const V_NeuronSWC_list& inputSegList)
{
    V_NeuronSWC_list inputList = inputSegList;
    vector<V_NeuronSWC_list> outputTreeList;
    set<size_t> singleTreeSegs;
    vector<size_t> singleTreeSegsVec;

    while (inputList.seg.size() > 0)
    {
        singleTreeSegsVec.clear();
        singleTreeSegs.clear();
        singleTreeSegs.insert(0);

        segEnd2segIDmap.clear();
        for (vector<V_NeuronSWC>::iterator it = inputList.seg.begin(); it != inputList.seg.end(); ++it)
        {
            double xLabelTail = it->row.begin()->x;
            double yLabelTail = it->row.begin()->y;
            double zLabelTail = it->row.begin()->z;
            double xLabelHead = (it->row.end() - 1)->x;
            double yLabelHead = (it->row.end() - 1)->y;
            double zLabelHead = (it->row.end() - 1)->z;
            QString key1Q = QString::number(xLabelTail) + "_" + QString::number(yLabelTail) + "_" + QString::number(zLabelTail);
            string key1 = key1Q.toStdString();
            QString key2Q = QString::number(xLabelHead) + "_" + QString::number(yLabelHead) + "_" + QString::number(zLabelHead);
            string key2 = key2Q.toStdString();

            segEnd2segIDmap.insert(pair<string, size_t>(key1, size_t(it - inputList.seg.begin())));
            segEnd2segIDmap.insert(pair<string, size_t>(key2, size_t(it - inputList.seg.begin())));
        }

        rc_findConnectedSegs(inputList, singleTreeSegs, 0, segEnd2segIDmap);
        singleTreeSegsVec.insert(singleTreeSegsVec.begin(), singleTreeSegs.begin(), singleTreeSegs.end());
        sort(singleTreeSegsVec.rbegin(), singleTreeSegsVec.rend());
        V_NeuronSWC_list currTreeSegs;
        for (vector<size_t>::iterator segIt = singleTreeSegsVec.begin(); segIt != singleTreeSegsVec.end(); ++segIt)
        {
            vector<V_NeuronSWC_unit> currSegUnits;
            for (vector<V_NeuronSWC_unit>::iterator unitIt = inputList.seg[*segIt].row.begin(); unitIt != inputList.seg[*segIt].row.end(); ++unitIt)
                currSegUnits.push_back(*unitIt);
            V_NeuronSWC currSeg;
            currSeg.row = currSegUnits;
            currTreeSegs.seg.push_back(currSeg);
            inputList.seg.erase(inputList.seg.begin() + *segIt);
        }

        outputTreeList.push_back(currTreeSegs);
        cout << "number of segs in this tree: " << currTreeSegs.seg.size() << endl;
    }

    return outputTreeList;
}

void rc_findConnectedSegs(V_NeuronSWC_list& inputSegList, set<size_t>& singleTreeSegs, size_t inputSegID, multimap<string, size_t>& segEnd2segIDmap)
{
    size_t curSegNum = singleTreeSegs.size();

    /* --------- Find segments that are connected in the middle of input segment --------- */
    if (inputSegList.seg[inputSegID].row.size() > 2)
    {
        for (vector<V_NeuronSWC_unit>::iterator unitIt = inputSegList.seg[inputSegID].row.begin() + 1; unitIt != inputSegList.seg[inputSegID].row.end() - 1; ++unitIt)
        {
            double middleX = unitIt->x;
            double middleY = unitIt->y;
            double middleZ = unitIt->z;
            QString middleNodeKeyQ = QString::number(middleX) + "_" + QString::number(middleY) + "_" + QString::number(middleZ);
            string middleNodeKey = middleNodeKeyQ.toStdString();

            pair<multimap<string, size_t>::iterator, multimap<string, size_t>::iterator> middleRange = segEnd2segIDmap.equal_range(middleNodeKey);
            for (multimap<string, size_t>::iterator middleIt = middleRange.first; middleIt != middleRange.second; ++middleIt)
            {
                if (middleIt->second == inputSegID) continue;
                else if (singleTreeSegs.find(middleIt->second) != singleTreeSegs.end())
                {
                    //cout << "  --> already picked, move to the next." << endl;
                    continue;
                }
                else if (middleIt->first == middleNodeKey)
                {
                    //cout << "  Found a segment in the middle of the route, adding it to the recursive searching process:" << middleNodeKey << " " << middleIt->second << endl;
                    if (inputSegList.seg[middleIt->second].to_be_deleted) continue;
                    singleTreeSegs.insert(middleIt->second);
                    rc_findConnectedSegs(inputSegList, singleTreeSegs, middleIt->second, segEnd2segIDmap);
                }
            }
        }
    }
    /* ------- END of [Find segments that are connected in the middle of input segment] ------- */

    /* --------- Find segments that are connected to the head or tail of input segment --------- */
    set<size_t> curSegEndRegionSegs;
    curSegEndRegionSegs.clear();
    curSegEndRegionSegs = segEndRegionCheck(inputSegList, inputSegID);
    //cout << curSegEndRegionSegs.size() << endl;
    if (!curSegEndRegionSegs.empty())
    {
        for (set<size_t>::iterator regionSegIt = curSegEndRegionSegs.begin(); regionSegIt != curSegEndRegionSegs.end(); ++regionSegIt)
        {
            if (*regionSegIt == inputSegID) continue;
            else if (singleTreeSegs.find(*regionSegIt) != singleTreeSegs.end())
            {
                //cout << "  --> already picked, move to the next." << endl;
                continue;
            }
            else
            {
                //cout << "    ==> segs at the end region added:" << *regionSegIt << endl;
                if (inputSegList.seg[*regionSegIt].to_be_deleted) continue;

                singleTreeSegs.insert(*regionSegIt);
                rc_findConnectedSegs(inputSegList, singleTreeSegs, *regionSegIt, segEnd2segIDmap);
            }
        }
    }
    /* ------- END of [Find segments that are connected to the head or tail of input segment] ------- */

    if (singleTreeSegs.size() == curSegNum) return;
}

set<size_t> segEndRegionCheck(V_NeuronSWC_list& inputSegList, size_t inputSegID)
{
    set<size_t> otherConnectedSegs;
    otherConnectedSegs.clear();

    map<string, set<size_t> > wholeGrid2segIDmap;
    set<size_t> subtreeSegs;
    size_t segCount = 0;
    for (vector<V_NeuronSWC>::iterator segIt = inputSegList.seg.begin(); segIt != inputSegList.seg.end(); ++segIt)
    {
        for (vector<V_NeuronSWC_unit>::iterator nodeIt = segIt->row.begin(); nodeIt != segIt->row.end(); ++nodeIt)
        {
            int xLabel = nodeIt->x / GRID_LENGTH;
            int yLabel = nodeIt->y / GRID_LENGTH;
            int zLabel = nodeIt->z / GRID_LENGTH;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            wholeGrid2segIDmap[gridKey].insert(size_t(segIt - inputSegList.seg.begin()));
        }
        subtreeSegs.insert(segCount);
        ++segCount;
    }

    int xHead = (inputSegList.seg[inputSegID].row.end() - 1)->x / GRID_LENGTH;
    int yHead = (inputSegList.seg[inputSegID].row.end() - 1)->y / GRID_LENGTH;
    int zHead = (inputSegList.seg[inputSegID].row.end() - 1)->z / GRID_LENGTH;
    int xTail = inputSegList.seg[inputSegID].row.begin()->x / GRID_LENGTH;
    int yTail = inputSegList.seg[inputSegID].row.begin()->y / GRID_LENGTH;
    int zTail = inputSegList.seg[inputSegID].row.begin()->z / GRID_LENGTH;
    QString gridKeyHeadQ = QString::number(xHead) + "_" + QString::number(yHead) + "_" + QString::number(zHead);
    string gridKeyHead = gridKeyHeadQ.toStdString();
    QString gridKeyTailQ = QString::number(xTail) + "_" + QString::number(yTail) + "_" + QString::number(zTail);
    string gridKeyTail = gridKeyTailQ.toStdString();

    set<size_t> headRegionSegs = wholeGrid2segIDmap[gridKeyHead];
    set<size_t> tailRegionSegs = wholeGrid2segIDmap[gridKeyTail];

    //cout << " Head region segs:";
    for (set<size_t>::iterator headIt = headRegionSegs.begin(); headIt != headRegionSegs.end(); ++headIt)
    {
        if (*headIt == inputSegID || inputSegList.seg[*headIt].to_be_deleted) continue;
        //cout << *headIt << " ";
        for (vector<V_NeuronSWC_unit>::iterator nodeIt = inputSegList.seg[*headIt].row.begin(); nodeIt != inputSegList.seg[*headIt].row.end(); ++nodeIt)
        {
            if (nodeIt->x == (inputSegList.seg[inputSegID].row.end() - 1)->x && nodeIt->y == (inputSegList.seg[inputSegID].row.end() - 1)->y && nodeIt->z == (inputSegList.seg[inputSegID].row.end() - 1)->z)
                otherConnectedSegs.insert(*headIt);
        }
    }
    //cout << endl << " Tail region segs:";
    for (set<size_t>::iterator tailIt = tailRegionSegs.begin(); tailIt != tailRegionSegs.end(); ++tailIt)
    {
        if (*tailIt == inputSegID || inputSegList.seg[*tailIt].to_be_deleted) continue;
        //cout << *tailIt << " ";
        for (vector<V_NeuronSWC_unit>::iterator nodeIt = inputSegList.seg[*tailIt].row.begin(); nodeIt != inputSegList.seg[*tailIt].row.end(); ++nodeIt)
        {
            if (nodeIt->x == inputSegList.seg[inputSegID].row.begin()->x && nodeIt->y == inputSegList.seg[inputSegID].row.begin()->y && nodeIt->z == inputSegList.seg[inputSegID].row.begin()->z)
                otherConnectedSegs.insert(*tailIt);
        }
    }
    //cout << endl;

    return otherConnectedSegs;
}














