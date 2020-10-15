#include "pruning.h"


#include "hierarchy_prune.h"
#include "my_surf_objs.h"
#include "../app3/swc_convert.h"

void pruningCross(NeuronTree& nt){
    int pointSize = nt.listNeuron.size();
    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(pointSize,vector<V3DLONG>());
    QStack<int> stack = QStack<int>();
    nt.hashNeuron.clear();
    for(V3DLONG i=0; i<pointSize; i++){
        nt.hashNeuron.insert(nt.listNeuron[i].n,i);
    }

    int* isRemain = new int[pointSize];
    V3DLONG ori;
    for(V3DLONG i=0; i<pointSize; i++){
        isRemain[i] = 0;
        if(nt.listNeuron[i].parent == -1){
            ori = i;
            stack.push(ori);
            continue;
        }
        V3DLONG prtIndex = nt.hashNeuron.value(nt.listNeuron[i].parent);
        children[prtIndex].push_back(i);
    }

    double somaR = nt.listNeuron[ori].r;
    vector<V3DLONG> pointIndexs = vector<V3DLONG>();

    V3DLONG t,tmp;
    while (!stack.empty()) {
        t = stack.pop();
        pointIndexs.push_back(t);
        vector<V3DLONG> child = children[t];
        for(int i=0; i<child.size(); i++){
            tmp = child[i];
            while(children[tmp].size() == 1){
                pointIndexs.push_back(tmp);
                V3DLONG ch = children[tmp][0];
                tmp = ch;
            }
            int chsz = children[tmp].size();
            if(chsz>1){
                stack.push(tmp);
            }else {
                pointIndexs.push_back(tmp);
            }

        }
    }

    //pruning X,T structure
    for(V3DLONG i=0; i<pointSize; i++){
        t = pointIndexs[i];

        if(zx_dist(nt.listNeuron[t],nt.listNeuron[ori]) < somaR*2 || isRemain[t] != 0){
            if(isRemain[t] == 0){
                isRemain[t] = 1;
            }
        }else if(children[t].size()>1){
            XYZ p1,p2;
            V3DLONG prtIndex = nt.hashNeuron.value(nt.listNeuron[t].parent);
            double length = 0;
            int count = 0;
            XYZ ptp = XYZ(nt.listNeuron[prtIndex].x,nt.listNeuron[prtIndex].y,nt.listNeuron[prtIndex].z);
            XYZ ptpp = XYZ(0,0,0);

            while(length<10){
                if(prtIndex > 0){
                    tmp = prtIndex;
                    prtIndex = nt.hashNeuron.value(nt.listNeuron[tmp].parent);
                    ptpp.x += nt.listNeuron[prtIndex].x;
                    ptpp.y += nt.listNeuron[prtIndex].y;
                    ptpp.z += nt.listNeuron[prtIndex].z;
                    count++;
                    length += zx_dist(nt.listNeuron[tmp],nt.listNeuron[prtIndex]);
                }else {
                    break;
                }
            }
            if(count>0){
                ptpp.x /= (double)count;
                ptpp.y /= (double)count;
                ptpp.z /= (double)count;
            }
            p1 = ptp - ptpp;

            vector<V3DLONG> child = children[t];
            vector<vector<V3DLONG> > cbs;

            for(int j=0; j<child.size(); j++){
                vector<V3DLONG> cb = vector<V3DLONG>();
                tmp = child[j];
                cb.push_back(tmp);
                while(children[tmp].size() == 1){
                    tmp = children[tmp][0];
                    cb.push_back(tmp);
                }
                cbs.push_back(cb);
            }

            qDebug()<<"cbs size: "<<cbs.size();

            if(cbs.size()>2){
                int maxIndex1, maxIndex2;
                double maxD = -1;
                vector<int> index1 = vector<int>();
                vector<int> index2 = vector<int>();
                vector<double> indexD = vector<double>();
                for(int j=0; j<cbs.size(); j++){
                    if(j != 0){
                        tmp = cbs[j-1][0];
                        ptp = XYZ(0,0,0);
                        ptpp = XYZ(0,0,0);
                        int pCount = 0;
                        int ppCount = 0;
                        length = 0;
                        while (children[tmp].size() != 0) {
                            prtIndex = tmp;
                            tmp = children[tmp][0];
                            length += zx_dist(nt.listNeuron[prtIndex],nt.listNeuron[tmp]);
                            if(length>5 && length<10){
                                ptp.x += nt.listNeuron[tmp].x;
                                ptp.y += nt.listNeuron[tmp].y;
                                ptp.z += nt.listNeuron[tmp].z;
                                pCount++;
                            }else if(length>=10 && length<20){
                                ptpp.x += nt.listNeuron[tmp].x;
                                ptpp.y += nt.listNeuron[tmp].y;
                                ptpp.z += nt.listNeuron[tmp].z;
                                ppCount++;
                            }else if(length>=20){
                                break;
                            }
                        }
                        if(pCount>0){
                            ptp.x /= (double)pCount;
                            ptp.y /= (double)pCount;
                            ptp.z /= (double)pCount;
                        }
                        if(ppCount){
                            ptpp.x /= (double)ppCount;
                            ptpp.y /= (double)ppCount;
                            ptpp.z /= (double)ppCount;
                        }

                        p1 = ptp - ptpp;
                    }
                    for(int k=j; k<cbs.size(); k++){
                        tmp = cbs[k][0];
                        ptp = XYZ(0,0,0);
                        length = 0;
                        count = 0;
                        while(children[tmp].size() != 0){
                            prtIndex = tmp;
                            tmp = children[tmp][0];
                            length += zx_dist(nt.listNeuron[prtIndex],nt.listNeuron[tmp]);
                            if(length>5 && length<15){
                                ptp.x += nt.listNeuron[tmp].x;
                                ptp.y += nt.listNeuron[tmp].y;
                                ptp.z += nt.listNeuron[tmp].z;
                                count++;
                            }else if(length>=15){
                                break;
                            }
                        }
                        if(count>0){
                            ptp.x /= (double)count;
                            ptp.y /= (double)count;
                            ptp.z /= (double)count;
                        }

                        p2 = ptp - ptpp;
                        double tmpD = dot(normalize(p1),normalize(p2));

                        qDebug()<<"tow line id: "<<(j-1)<<" "<<k<<" tmpD: "<<tmpD;
                        index1.push_back(j-1);
                        index2.push_back(k);
                        indexD.push_back(tmpD);
                        if(tmpD>maxD){
                            maxD = tmpD;
                            maxIndex1 = j-1;
                            maxIndex2 = k;
                        }
                    }
                }
                qDebug()<<"maxD: "<<maxD<<" maxIndex: "<<maxIndex1<<" "<<maxIndex2;
                isRemain[t] = 1;
                if(maxIndex1 == -1){
                    for(int j=0; j<indexD.size(); j++){
                        if(index1[j] != maxIndex1 && index1[j] != maxIndex2
                                && index2[j] != maxIndex1 && index2[j] != maxIndex2){
                            if(indexD[j]>0.9){
                                for(int k=0; k<cbs.size(); k++){
                                    if(k == maxIndex2){
                                        for(int kk=0; kk<cbs[k].size(); kk++){
                                            isRemain[cbs[k][kk]] = 1;
                                        }
                                    }else {
                                        for(int kk=0; kk<cbs[k].size(); kk++){
                                            isRemain[cbs[k][kk]] = 2;
                                        }
                                        QStack<int> tmpStack = QStack<int>();
                                        if(children[cbs[k].back()].size()>1){
                                            tmpStack.push_back(cbs[k].back());
                                        }
                                        while (!tmpStack.isEmpty()) {
                                            tmp = tmpStack.pop();
                                            isRemain[tmp] = 2;
                                            for(int kk=0; kk<children[tmp].size(); kk++){
                                                V3DLONG tmp1 = children[tmp][kk];
                                                while(children[tmp1].size() == 1){
                                                    isRemain[tmp1] = 2;
                                                    tmp1 = children[tmp1][0];
                                                }
                                                if(children[tmp1].size() == 0){
                                                    isRemain[tmp1] = 2;
                                                }else {
                                                    tmpStack.push_back(tmp1);
                                                }
                                            }
                                        }
                                    }
                                }
                            }else{
                                for(int k=0; k<cbs.size(); k++){
                                    if(k == maxIndex2){
                                        for(int kk=0; kk<cbs[k].size(); kk++){
                                            isRemain[cbs[k][kk]] = 1;
                                        }
                                    }else {
                                        for(int kk=0; kk<cbs[k].size(); kk++){
                                            isRemain[cbs[k][kk]] = 0;
                                        }
                                    }
                                }
                            }
                            break;
                        }
                    }

                }else{
                    for(int j=0; j<cbs.size(); j++){
                        if(j != maxIndex1 && j != maxIndex2){
                            for(int k=0; k<cbs[j].size(); k++){
                                isRemain[cbs[j][k]] = 1;
                            }
                        }else{
                            for(int k=0; k<cbs[j].size(); k++){
                                isRemain[cbs[j][k]] = 2;
                            }
                            QStack<int> tmpStack = QStack<int>();
                            if(children[cbs[j].back()].size()>1){
                                tmpStack.push_back(cbs[j].back());
                            }
                            while (!tmpStack.isEmpty()) {
                                tmp = tmpStack.pop();
                                isRemain[tmp] = 2;
                                for(int k=0; k<children[tmp].size(); k++){
                                    V3DLONG tmp1 = children[tmp][k];
                                    while(children[tmp1].size() == 1){
                                        isRemain[tmp1] = 2;
                                        tmp1 = children[tmp1][0];
                                    }
                                    if(children[tmp1].size() == 0){
                                        isRemain[tmp1] = 2;
                                    }else {
                                        tmpStack.push_back(tmp1);
                                    }
                                }
                            }
                        }
                    }
                }
                continue;
            }else{
                double minLength = INT_MAX;
                int minIndex = 0;

                double bifurcationD = 10;

                for(int j=0; j<cbs.size(); j++){
                    double tmpLength = zx_dist(nt.listNeuron[cbs[j].front()],nt.listNeuron[cbs[j].back()]);
                    if(tmpLength<minLength){
                        minLength = cbs[j].size();
                        minIndex = j;
                    }
                }
                if(minLength<bifurcationD){

                    V3DLONG next_cur = cbs[minIndex].back();
                    qDebug()<<"children nex_cur size: "<<children[next_cur].size();
                    if(children[next_cur].size()>1){
                        child = children[next_cur];
                        for(int j=0; j<child.size(); j++){
                            vector<V3DLONG> cb = vector<V3DLONG>();
                            tmp = child[j];
                            cb.push_back(tmp);
                            while(children[tmp].size() == 1){
                                tmp = children[tmp][0];
                                cb.push_back(tmp);
                            }
                            cbs.push_back(cb);
                        }
                        int maxIndex1, maxIndex2;
                        double maxD = -1;
                        vector<int> index1 = vector<int>();
                        vector<int> index2 = vector<int>();
                        vector<double> indexD = vector<double>();

                        for(int j=0; j<cbs.size(); j++){
                            if(j-1 == minIndex)
                                continue;
                            if(j != 0){
                                tmp = cbs[j-1][0];
                                ptp = XYZ(0,0,0);
                                ptpp = XYZ(0,0,0);
                                int pCount = 0;
                                int ppCount = 0;
                                length = 0;
                                while (children[tmp].size() != 0) {
                                    prtIndex = tmp;
                                    tmp = children[tmp][0];
                                    length += zx_dist(nt.listNeuron[prtIndex],nt.listNeuron[tmp]);
                                    if(length>5 && length<10){
                                        ptp.x += nt.listNeuron[tmp].x;
                                        ptp.y += nt.listNeuron[tmp].y;
                                        ptp.z += nt.listNeuron[tmp].z;
                                        pCount++;
                                    }else if(length>=10 && length<20){
                                        ptpp.x += nt.listNeuron[tmp].x;
                                        ptpp.y += nt.listNeuron[tmp].y;
                                        ptpp.z += nt.listNeuron[tmp].z;
                                        ppCount++;
                                    }else if(length>=20){
                                        break;
                                    }
                                }
                                if(pCount>0){
                                    ptp.x /= (double)pCount;
                                    ptp.y /= (double)pCount;
                                    ptp.z /= (double)pCount;
                                }
                                if(ppCount){
                                    ptpp.x /= (double)ppCount;
                                    ptpp.y /= (double)ppCount;
                                    ptpp.z /= (double)ppCount;
                                }

                                p1 = ptp - ptpp;
                            }
                            for(int k=j; k<cbs.size(); k++){
                                if(k == minIndex)
                                    continue;
                                tmp = cbs[k][0];
                                ptp = XYZ(0,0,0);
                                count = 0;
                                while(children[tmp].size() != 0){
                                    prtIndex = tmp;
                                    tmp = children[tmp][0];
                                    length += zx_dist(nt.listNeuron[prtIndex],nt.listNeuron[tmp]);
                                    if(length>5 && length<15){
                                        ptp.x += nt.listNeuron[tmp].x;
                                        ptp.y += nt.listNeuron[tmp].y;
                                        ptp.z += nt.listNeuron[tmp].z;
                                        count++;
                                    }else if(length>=15){
                                        break;
                                    }
                                }
                                if(count>0){
                                    ptp.x /= (double)count;
                                    ptp.y /= (double)count;
                                    ptp.z /= (double)count;
                                }

                                p2 = ptp - ptpp;
                                double tmpD = dot(normalize(p1),normalize(p2));

                                qDebug()<<"tow line id: "<<(j-1)<<" "<<k<<" tmpD: "<<tmpD;
                                index1.push_back(j-1);
                                index2.push_back(k);
                                indexD.push_back(tmpD);
                                if(tmpD>maxD){
                                    maxD = tmpD;
                                    maxIndex1 = j-1;
                                    maxIndex2 = k;
                                }
                            }
                        }
                        qDebug()<<"maxD: "<<maxD<<" maxIndex: "<<maxIndex1<<" "<<maxIndex2;
                        isRemain[t] = 1;
                        if(maxIndex1 == -1){
                            for(int j=0; j<indexD.size(); j++){
                                if(index1[j] != maxIndex1 && index1[j] != maxIndex2
                                        && index2[j] != maxIndex1 && index2[j] != maxIndex2){
                                    if(indexD[j]>0.9){
                                        for(int k=0; k<cbs.size(); k++){
                                            if(k == maxIndex2 || k == minIndex){
                                                for(int kk=0; kk<cbs[k].size(); kk++){
                                                    isRemain[cbs[k][kk]] = 1;
                                                }
                                            }else {
                                                for(int kk=0; kk<cbs[k].size(); kk++){
                                                    isRemain[cbs[k][kk]] = 2;
                                                }
                                                QStack<int> tmpStack = QStack<int>();
                                                if(children[cbs[k].back()].size()>1){
                                                    tmpStack.push_back(cbs[k].back());
                                                }
                                                while (!tmpStack.isEmpty()) {
                                                    tmp = tmpStack.pop();
                                                    isRemain[tmp] = 2;
                                                    for(int kk=0; kk<children[tmp].size(); kk++){
                                                        V3DLONG tmp1 = children[tmp][kk];
                                                        while(children[tmp1].size() == 1){
                                                            isRemain[tmp1] = 2;
                                                            tmp1 = children[tmp1][0];
                                                        }
                                                        if(children[tmp1].size() == 0){
                                                            isRemain[tmp1] = 2;
                                                        }else {
                                                            tmpStack.push_back(tmp1);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }else{
                                        for(int k=0; k<cbs.size(); k++){
                                            if(k == maxIndex2 || k == minIndex){
                                                for(int kk=0; kk<cbs[k].size(); kk++){
                                                    isRemain[cbs[k][kk]] = 1;
                                                }
                                            }else {
                                                for(int kk=0; kk<cbs[k].size(); kk++){
                                                    isRemain[cbs[k][kk]] = 0;
                                                }
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }else{
                            for(int j=0; j<cbs.size(); j++){
                                if(j != maxIndex1 && j != maxIndex2){
                                    for(int k=0; k<cbs[j].size(); k++){
                                        isRemain[cbs[j][k]] = 1;
                                    }
                                }else{
                                    for(int k=0; k<cbs[j].size(); k++){
                                        isRemain[cbs[j][k]] = 2;
                                    }
                                    QStack<int> tmpStack = QStack<int>();
                                    if(children[cbs[j].back()].size()>1){
                                        tmpStack.push_back(cbs[j].back());
                                    }
                                    while (!tmpStack.isEmpty()) {
                                        tmp = tmpStack.pop();
                                        qDebug()<<"tmp: "<<tmp;
                                        isRemain[tmp] = 2;
                                        for(int k=0; k<children[tmp].size(); k++){
                                            V3DLONG tmp1 = children[tmp][k];
                                            while(children[tmp1].size() == 1){
                                                isRemain[tmp1] = 2;
                                                tmp1 = children[tmp1][0];
                                            }
                                            if(children[tmp1].size() == 0){
                                                isRemain[tmp1] = 2;
                                            }else {
                                                tmpStack.push_back(tmp1);
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        continue;
                    }
                }else {
                    tmp =cbs[0][0];
                    ptp = XYZ(0,0,0);
                    ptpp = XYZ(0,0,0);
                    int pCount = 0;
                    int ppCount = 0;
                    length = 0;

                    while (children[tmp].size() != 0) {
                        prtIndex = tmp;
                        tmp = children[tmp][0];
                        length += zx_dist(nt.listNeuron[prtIndex],nt.listNeuron[tmp]);
                        if(length>10 && length<15){
                            ptp.x += nt.listNeuron[tmp].x;
                            ptp.y += nt.listNeuron[tmp].y;
                            ptp.z += nt.listNeuron[tmp].z;
                            pCount++;
                        }else if(length>=15 && length<25){
                            ptpp.x += nt.listNeuron[tmp].x;
                            ptpp.y += nt.listNeuron[tmp].y;
                            ptpp.z += nt.listNeuron[tmp].z;
                            ppCount++;
                        }else if(length>=25){
                            break;
                        }
                    }
                    if(pCount>0){
                        ptp.x /= (double)pCount;
                        ptp.y /= (double)pCount;
                        ptp.z /= (double)pCount;
                    }
                    if(ppCount){
                        ptpp.x /= (double)ppCount;
                        ptpp.y /= (double)ppCount;
                        ptpp.z /= (double)ppCount;
                    }

                    p1 = ptp - ptpp;

                    tmp = cbs[1][0];
                    ptp = XYZ(0,0,0);
                    count = 0;
                    while(children[tmp].size() != 0){
                        prtIndex = tmp;
                        tmp = children[tmp][0];
                        length += zx_dist(nt.listNeuron[prtIndex],nt.listNeuron[tmp]);
                        if(length>20 && length<40){
                            ptp.x += nt.listNeuron[tmp].x;
                            ptp.y += nt.listNeuron[tmp].y;
                            ptp.z += nt.listNeuron[tmp].z;
                            count++;
                        }else if(length>=40){
                            break;
                        }
                    }
                    if(count>0){
                        ptp.x /= (double)count;
                        ptp.y /= (double)count;
                        ptp.z /= (double)count;
                    }

                    p2 = ptp - ptpp;
//                    double lt = norm(p2)/norm(p1);
                    double tmpD = dot(normalize(p1),normalize(p2));

                    qDebug()<<"tmpD: "<<tmpD;
                    if(tmpD>0.9){
                        for(int k=0; k<cbs.size(); k++){
                            for(int kk=0; kk<cbs[k].size(); kk++){
                                isRemain[cbs[k][kk]] = 2;
                            }
                            QStack<int> tmpStack = QStack<int>();
                            if(children[cbs[k].back()].size()>1){
                                tmpStack.push_back(cbs[k].back());
                            }
                            while (!tmpStack.isEmpty()) {
                                tmp = tmpStack.pop();
                                isRemain[tmp] = 2;
                                for(int kk=0; kk<children[tmp].size(); kk++){
                                    V3DLONG tmp1 = children[tmp][kk];
                                    while(children[tmp1].size() == 1){
                                        isRemain[tmp1] = 2;
                                        tmp1 = children[tmp1][0];
                                    }
                                    if(children[tmp1].size() == 0){
                                        isRemain[tmp1] = 2;
                                    }else {
                                        tmpStack.push_back(tmp1);
                                    }
                                }
                            }
                        }
                    }

                }
            }
        }

    }

    for(V3DLONG i=0; i<pointSize; i++){
        if(isRemain[i] == 0){
            nt.listNeuron[i].type = 3;
        }else if (isRemain[i] == 1) {
            nt.listNeuron[i].type = 7;
        }else if(isRemain[i] == 2){
            nt.listNeuron[i].type = 2;
        }
    }

    if(isRemain){
        delete[] isRemain;
        isRemain = 0;
    }

}


NeuronTree pruningSoma(NeuronTree& nt, double times){
    vector<MyMarker*> inswc = swc_convert(nt);
    vector<HierarchySegment*> topo_segs;
    swc2topo_segs<unsigned char>(inswc, topo_segs, 1);
    vector<HierarchySegment*> filter_segs;

    MyMarker* root;
    for(V3DLONG i=0; i<inswc.size(); i++){
        if(inswc[i]->parent == 0){
            root = inswc[i];
            break;
        }
    }
    double somaR = root->radius;

    qDebug()<<"topo size:"<<topo_segs.size();
    for(V3DLONG i=0; i<topo_segs.size(); i++){
        MyMarker* leaf_marker = topo_segs[i]->leaf_marker;
        MyMarker* root_marker = topo_segs[i]->root_marker;
        if(dist(*leaf_marker,*root) < times*somaR && dist(*root_marker,*root) < times*somaR/2){
            double ed = dist(*(topo_segs[i]->root_marker),*(topo_segs[i]->leaf_marker));
            double ratio = topo_segs[i]->length/ed;
            qDebug()<<"somaR: "<<somaR<<" somaR*(times+1): "<<somaR*(times+1)
                   <<" length: "<<topo_segs[i]->length<<" ed: "<<ed
                  <<"ratio: "<<ratio
                 <<" level: "<<topo_segs[i]->level;
            if((ratio >= 1.2 && topo_segs[i]->length>5) || topo_segs[i]->level > 1){
                filter_segs.push_back(topo_segs[i]);
            }
        }else {
            filter_segs.push_back(topo_segs[i]);
        }
    }
    qDebug()<<"filter size:"<<filter_segs.size();
    vector<MyMarker*> outswc;
    topo_segs2swc(filter_segs,outswc,1);
    qDebug()<<"inswc size:"<<inswc.size()<<" outswc size: "<<outswc.size();

    NeuronTree outnt = swc_convert(outswc);
    for(V3DLONG i=0; i<inswc.size(); i++){
        delete inswc[i];
    }
    qDebug()<<"nt size: "<<nt.listNeuron.size()<<" outnt size: "<<outnt.listNeuron.size();

    return outnt;
}

NeuronTree pruningByLength(NeuronTree& nt, double lengthT){
    vector<MyMarker*> inswc = swc_convert(nt);
    vector<HierarchySegment*> topo_segs;
    swc2topo_segs<unsigned char>(inswc, topo_segs, 1);
    vector<HierarchySegment*> filter_segs;

    for(V3DLONG i=0; i<topo_segs.size(); i++){
        if(topo_segs[i]->length >= lengthT){
            filter_segs.push_back(topo_segs[i]);
        }
    }
    vector<MyMarker*> outswc;
    topo_segs2swc(filter_segs,outswc,1);
    NeuronTree outnt = swc_convert(outswc);
    for(V3DLONG i=0; i<inswc.size(); i++){
        delete inswc[i];
    }

    return outnt;
}


void pruningCross2(NeuronTree& nt, double bifurcationD){
    int pointSize = nt.listNeuron.size();
    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(pointSize,vector<V3DLONG>());
    QStack<int> stack = QStack<int>();
    nt.hashNeuron.clear();
    for(V3DLONG i=0; i<pointSize; i++){
        nt.hashNeuron.insert(nt.listNeuron[i].n,i);
    }

    int* isRemain = new int[pointSize];
    V3DLONG ori;
    for(V3DLONG i=0; i<pointSize; i++){
        isRemain[i] = 0;
        if(nt.listNeuron[i].parent == -1){
            ori = i;
            stack.push(ori);
            continue;
        }
        V3DLONG prtIndex = nt.hashNeuron.value(nt.listNeuron[i].parent);
        children[prtIndex].push_back(i);
    }

    double somaR = nt.listNeuron[ori].r;
    vector<V3DLONG> pointIndexs = vector<V3DLONG>();

    V3DLONG t,tmp;
    while (!stack.empty()) {
        t = stack.pop();
        pointIndexs.push_back(t);
        vector<V3DLONG> child = children[t];
        for(int i=0; i<child.size(); i++){
            tmp = child[i];
            while(children[tmp].size() == 1){
                pointIndexs.push_back(tmp);
                V3DLONG ch = children[tmp][0];
                tmp = ch;
            }
            int chsz = children[tmp].size();
            if(chsz>1){
                stack.push(tmp);
            }else {
                pointIndexs.push_back(tmp);
            }

        }
    }

    //pruning X,T structure
    for(V3DLONG i=0; i<pointSize; i++){
        t = pointIndexs[i];

        if(zx_dist(nt.listNeuron[t],nt.listNeuron[ori]) < somaR*2 || isRemain[t] != 0){
            if(isRemain[t] == 0){
                isRemain[t] = 1;
            }
        }else if(children[t].size()>1){
            XYZ p1,p2;
            XYZ ptpp = XYZ(nt.listNeuron[t].x,nt.listNeuron[t].y,nt.listNeuron[t].z);
            XYZ ptp;

            V3DLONG prtIndex;

            vector<vector<V3DLONG> > cbs = vector<vector<V3DLONG> >();

            getAdjacentChildSegment(nt,children,cbs,t,bifurcationD);
            qDebug()<<"cbs size: "<<cbs.size();


            vector<int> index1 = vector<int>();
            vector<int> index2 = vector<int>();
            vector<double> indexD = vector<double>();
            int maxIndex1, maxIndex2;
            double minD = 1;

            for(int j=0; j<cbs.size(); j++){
                tmp = cbs[j].back();
                prtIndex = nt.hashNeuron.value(nt.listNeuron[cbs[j].front()].parent);
                ptp = XYZ(nt.listNeuron[tmp].x,nt.listNeuron[tmp].y,nt.listNeuron[tmp].z);
                p1 = ptp - ptpp;
                for(int k=j+1; k<cbs.size(); k++){
                    tmp = cbs[k].back();
                    if(prtIndex == nt.hashNeuron.value(nt.listNeuron[cbs[k].front()].parent)){
                        XYZ prtp = XYZ(nt.listNeuron[prtIndex].x,nt.listNeuron[prtIndex].y,nt.listNeuron[prtIndex].z);
                        p1 = ptp - prtp;
                        ptp = XYZ(nt.listNeuron[tmp].x,nt.listNeuron[tmp].y,nt.listNeuron[tmp].z);
                        p2 = ptp - prtp;
                    }else {
                        ptp = XYZ(nt.listNeuron[tmp].x,nt.listNeuron[tmp].y,nt.listNeuron[tmp].z);
                        p2 = ptp - ptpp;
                    }
                    double tmpD = dot(normalize(p1),normalize(p2));
                    qDebug()<<"tow line id: "<<j<<" "<<k<<" tmpD: "<<tmpD;
                    index1.push_back(j);
                    index2.push_back(k);
                    indexD.push_back(tmpD);
                    if(tmpD<minD){
                        minD = tmpD;
                        maxIndex1 = j;
                        maxIndex2 = k;
                    }
                }
            }

            if(minD<-0.9){
                for(int j=0; j<cbs.size(); j++){
                    if(j != maxIndex1 && j != maxIndex2){
                        for(int k=0; k<cbs[j].size(); k++){
                            isRemain[cbs[j][k]] = 1;
                        }
                    }else{
                        for(int k=0; k<cbs[j].size(); k++){
                            isRemain[cbs[j][k]] = 2;
                        }
                        QStack<int> tmpStack = QStack<int>();
                        tmp = cbs[j].back();
                        while(children[tmp].size() == 1){
                            tmp = children[tmp][0];
                            isRemain[tmp] = 2;
                        }
                        if(children[tmp].size()>1){
                            tmpStack.push(tmp);
                        }
                        while (!tmpStack.isEmpty()) {
                            tmp = tmpStack.pop();
                            isRemain[tmp] = 2;
                            for(int k=0; k<children[tmp].size(); k++){
                                V3DLONG tmp1 = children[tmp][k];
                                while(children[tmp1].size() == 1){
                                    isRemain[tmp1] = 2;
                                    tmp1 = children[tmp1][0];
                                }
                                if(children[tmp1].size() == 0){
                                    isRemain[tmp1] = 2;
                                }else {
                                    tmpStack.push_back(tmp1);
                                }
                            }
                        }
                    }
                }
            }



        }
    }

    for(V3DLONG i=0; i<pointSize; i++){
        if(isRemain[i] == 0){
            nt.listNeuron[i].type = 3;
        }else if (isRemain[i] == 1) {
            nt.listNeuron[i].type = 7;
        }else if(isRemain[i] == 2){
            nt.listNeuron[i].type = 2;
        }
    }

    if(isRemain){
        delete[] isRemain;
        isRemain = 0;
    }
}


void getAdjacentChildSegment(NeuronTree& nt, vector<vector<V3DLONG> > children, vector<vector<V3DLONG> > &cbs, V3DLONG t, double bifurcationD){
    vector<V3DLONG> child = children[t];

    V3DLONG tmp;
    double length;
    V3DLONG prtIndex,pprtIndex;
    XYZ p1,p2;
    double tmpD;

    for(int j=0; j<child.size(); j++){
        vector<V3DLONG> cb = vector<V3DLONG>();
        tmp = child[j];
        cb.push_back(tmp);
        length = 0;
        while(children[tmp].size() == 1){
            prtIndex = tmp;
            tmp = children[tmp][0];
            length += zx_dist(nt.listNeuron[tmp],nt.listNeuron[prtIndex]);
            if(length<bifurcationD){
                cb.push_back(tmp);
            }else {
                p1 = XYZ(nt.listNeuron[tmp].x-nt.listNeuron[prtIndex].x,
                         nt.listNeuron[tmp].y-nt.listNeuron[prtIndex].y,
                         nt.listNeuron[tmp].z-nt.listNeuron[prtIndex].z);
                pprtIndex = nt.hashNeuron.value(nt.listNeuron[prtIndex].parent);
                p2 = XYZ(nt.listNeuron[prtIndex].x-nt.listNeuron[pprtIndex].x,
                         nt.listNeuron[prtIndex].y-nt.listNeuron[pprtIndex].y,
                         nt.listNeuron[prtIndex].z-nt.listNeuron[pprtIndex].z);
                tmpD = dot(normalize(p1),normalize(p2));
                if(tmpD>sqrt(3.0)/2){
                    cb.push_back(tmp);
                }else {
                    break;
                }
            }
        }

        if(length<bifurcationD){
            getAdjacentChildSegment(nt,children,cbs,cb.back(),bifurcationD);
        }else {
            cbs.push_back(cb);
        }

    }
}


NeuronTree pruningByType(NeuronTree& nt, int type){
    NeuronTree outnt = NeuronTree();
    for(V3DLONG i=0; i<nt.listNeuron.size(); i++){
        if(nt.listNeuron[i].type != type){
            outnt.listNeuron.push_back(nt.listNeuron[i]);
        }
    }
    outnt.hashNeuron.clear();
    for(V3DLONG i=0; i<outnt.listNeuron.size(); i++){
        outnt.hashNeuron.insert(outnt.listNeuron[i].n,i);
    }
    return outnt;
}

void pruningCross3(NeuronTree& nt, double bifurcationD){
    int pointSize = nt.listNeuron.size();
    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(pointSize,vector<V3DLONG>());
    QStack<int> stack = QStack<int>();
    nt.hashNeuron.clear();
    for(V3DLONG i=0; i<pointSize; i++){
        nt.hashNeuron.insert(nt.listNeuron[i].n,i);
    }

    int* isRemain = new int[pointSize];
    V3DLONG ori;
    for(V3DLONG i=0; i<pointSize; i++){
        isRemain[i] = 0;
        if(nt.listNeuron[i].parent == -1){
            ori = i;
            stack.push(ori);
            continue;
        }
        V3DLONG prtIndex = nt.hashNeuron.value(nt.listNeuron[i].parent);
        children[prtIndex].push_back(i);
    }

    double somaR = nt.listNeuron[ori].r;
    vector<V3DLONG> pointIndexs = vector<V3DLONG>();

    V3DLONG t,tmp;
    while (!stack.empty()) {
        t = stack.pop();
        pointIndexs.push_back(t);
        vector<V3DLONG> child = children[t];
        for(int i=0; i<child.size(); i++){
            tmp = child[i];
            while(children[tmp].size() == 1){
                pointIndexs.push_back(tmp);
                V3DLONG ch = children[tmp][0];
                tmp = ch;
            }
            int chsz = children[tmp].size();
            if(chsz>1){
                stack.push(tmp);
            }else {
                pointIndexs.push_back(tmp);
            }

        }
    }

    //pruning T structure

    for(V3DLONG i=0; i<pointSize; i++){
        t = pointIndexs[i];

        if(zx_dist(nt.listNeuron[t],nt.listNeuron[ori]) < somaR*2 || isRemain[t] != 0){
            if(isRemain[t] == 0){
                isRemain[t] = 1;
            }
        }else if(children[t].size()>1){
            vector<vector<V3DLONG> > cbs = vector<vector<V3DLONG> >();
            getChildSegment(nt,children,cbs,t,bifurcationD);
            qDebug()<<"T structure cbs size: "<<cbs.size();
            if(cbs.size()<2){
                continue;
            }
            pruningSegmentByAngle(nt,children,cbs,t,bifurcationD,isRemain);
            qDebug()<<"---end---";

        }
    }

    //pruning X structure
    for(V3DLONG i=0; i<pointSize; i++){
        t = pointIndexs[i];

        if(zx_dist(nt.listNeuron[t],nt.listNeuron[ori]) < somaR*2 || isRemain[t] != 0){
            if(isRemain[t] == 0){
                isRemain[t] = 1;
            }
        }else if(children[t].size()>1){
            vector<vector<V3DLONG> > cbs = vector<vector<V3DLONG> >();
            getAdjacentChildSegment(nt,children,cbs,t,bifurcationD,isRemain);
            qDebug()<<"X structure cbs size: "<<cbs.size();
            if(cbs.size()<3){
                continue;
            }
            pruningSegmentByAngle(nt,children,cbs,t,bifurcationD,isRemain);
            qDebug()<<"---end---";
        }
    }

    for(V3DLONG i=0; i<pointSize; i++){
        if(isRemain[i] == 0){
            nt.listNeuron[i].type = 3;
        }else if (isRemain[i] == 1) {
            nt.listNeuron[i].type = 7;
        }else if(isRemain[i] == 2){
            nt.listNeuron[i].type = 2;
        }
    }

    if(isRemain){
        delete[] isRemain;
        isRemain = 0;
    }
}

void getChildSegment(NeuronTree& nt, vector<vector<V3DLONG> > children, vector<vector<V3DLONG> > &cbs, V3DLONG t, double bifurcationD){
    vector<V3DLONG> child = children[t];

    V3DLONG tmp;
    V3DLONG prtIndex,ppIndex,ccIndex;
    XYZ p1,p2;
    double tmpD;
    for(int j=0; j<child.size(); j++){
        vector<V3DLONG> cb = vector<V3DLONG>();
        tmp = child[j];
        cb.push_back(tmp);
        vector<double> length = vector<double>();
        length.push_back(0.0);
        while(children[tmp].size() == 1){
            prtIndex = tmp;
            tmp = children[tmp][0];
            cb.push_back(tmp);
            double l = zx_dist(nt.listNeuron[tmp],nt.listNeuron[prtIndex]) + length.back();
            length.push_back(l);
        }

        if(length.back()<bifurcationD){
            length.clear();
            cb.clear();
            continue;
        }else {
            vector<V3DLONG> cb0 = vector<V3DLONG>();
            for(int k=0; k<cb.size(); k++){
                tmp = cb[k];
                if(length[k]<bifurcationD*2){
                    cb0.push_back(tmp);
                }else {
                    ppIndex = cb.front();
                    ccIndex = cb.back();
                    for(int ki=k-1; ki>=0; ki--){
                        if(length[k]-length[ki]>bifurcationD){
                            ppIndex = cb[ki];
                            break;
                        }
                    }
                    for(int ki=k+1; ki<cb.size(); ki++){
                        if(length[ki]-length[k]>bifurcationD){
                            ccIndex = cb[ki];
                            break;
                        }
                    }
                    p1 = XYZ(nt.listNeuron[tmp].x-nt.listNeuron[ppIndex].x,
                             nt.listNeuron[tmp].y-nt.listNeuron[ppIndex].y,
                             nt.listNeuron[tmp].z-nt.listNeuron[ppIndex].z);
                    p2 = XYZ(nt.listNeuron[ccIndex].x-nt.listNeuron[tmp].x,
                             nt.listNeuron[ccIndex].y-nt.listNeuron[tmp].y,
                             nt.listNeuron[ccIndex].z-nt.listNeuron[tmp].z);
                    tmpD = dot(normalize(p1),normalize(p2));
                    if(tmpD>sqrt(3.0)/2){
                        cb0.push_back(tmp);
                    }else {
                        break;
                    }
                }
            }
            cbs.push_back(cb0);
        }

    }
}

void getAdjacentChildSegment(NeuronTree& nt, vector<vector<V3DLONG> > children, vector<vector<V3DLONG> > &cbs, V3DLONG t, double bifurcationD, int* isRemain){
    vector<V3DLONG> child = children[t];

    V3DLONG tmp;
    V3DLONG prtIndex,ppIndex,ccIndex;
    XYZ p1,p2;
    double tmpD;

    for(int j=0; j<child.size(); j++){
        vector<V3DLONG> cb = vector<V3DLONG>();
        tmp = child[j];
        cb.push_back(tmp);
        vector<double> length = vector<double>();
        length.push_back(0.0);
        while(children[tmp].size() == 1){
            prtIndex = tmp;
            tmp = children[tmp][0];
            cb.push_back(tmp);
            double l = zx_dist(nt.listNeuron[tmp],nt.listNeuron[prtIndex]) + length.back();
            length.push_back(l);
        }

        if(length.back()<bifurcationD){
            tmp = cb.back();
            length.clear();
            cb.clear();
            if(isRemain[tmp] != 0 || children[tmp].size() < 2){
                continue;
            }else{
                getAdjacentChildSegment(nt,children,cbs,tmp,bifurcationD,isRemain);
            }
        }else {
            vector<V3DLONG> cb0 = vector<V3DLONG>();
            for(int k=0; k<cb.size(); k++){
                tmp = cb[k];
                if(length[k]<bifurcationD*2){
                    cb0.push_back(tmp);
                }else {
                    ppIndex = cb.front();
                    ccIndex = cb.back();
                    for(int ki=k-1; ki>=0; ki--){
                        if(length[k]-length[ki]>bifurcationD){
                            ppIndex = cb[ki];
                            break;
                        }
                    }
                    for(int ki=k+1; ki<cb.size(); ki++){
                        if(length[ki]-length[k]>bifurcationD){
                            ccIndex = cb[ki];
                            break;
                        }
                    }
                    p1 = XYZ(nt.listNeuron[tmp].x-nt.listNeuron[ppIndex].x,
                             nt.listNeuron[tmp].y-nt.listNeuron[ppIndex].y,
                             nt.listNeuron[tmp].z-nt.listNeuron[ppIndex].z);
                    p2 = XYZ(nt.listNeuron[ccIndex].x-nt.listNeuron[tmp].x,
                             nt.listNeuron[ccIndex].y-nt.listNeuron[tmp].y,
                             nt.listNeuron[ccIndex].z-nt.listNeuron[tmp].z);
                    tmpD = dot(normalize(p1),normalize(p2));
                    if(tmpD>sqrt(3.0)/2){
                        cb0.push_back(tmp);
                    }else {
                        break;
                    }
                }
            }
            cbs.push_back(cb0);
        }

    }
}

void pruningSegmentByAngle(NeuronTree& nt, vector<vector<V3DLONG> > children, vector<vector<V3DLONG> > &cbs, V3DLONG t, double bifurcationD, int* isRemain){
    int minIndex1, minIndex2;
    double minD = 1;

    XYZ p1,p2;
    XYZ ptpp = XYZ(nt.listNeuron[t].x,nt.listNeuron[t].y,nt.listNeuron[t].z);
    XYZ ptp;

    V3DLONG prtIndex,prtIndexJ,prtIndexK;
    V3DLONG tmp,tmpj,tmpk;

    for(int j=0; j<cbs.size(); j++){
        tmpj = cbs[j].back();
        prtIndexJ = nt.hashNeuron.value(nt.listNeuron[cbs[j].front()].parent);

        for(int k=j+1; k<cbs.size(); k++){
            tmpk = cbs[k].back();
            prtIndexK = nt.hashNeuron.value(nt.listNeuron[cbs[k].front()].parent);
            if(zx_dist(nt.listNeuron[prtIndexJ],nt.listNeuron[prtIndexK])>bifurcationD){
                continue;
            }
            if(prtIndexJ != t || prtIndexK != t){
                if(prtIndexJ != t){
                    prtIndex = prtIndexJ;
                }else {
                    prtIndex = prtIndexK;
                }
                XYZ prtp = XYZ(nt.listNeuron[prtIndex].x,nt.listNeuron[prtIndex].y,nt.listNeuron[prtIndex].z);
                ptp = XYZ(nt.listNeuron[tmpj].x,nt.listNeuron[tmpj].y,nt.listNeuron[tmpj].z);
                p1 = ptp - prtp;
                ptp = XYZ(nt.listNeuron[tmpk].x,nt.listNeuron[tmpk].y,nt.listNeuron[tmpk].z);
                p2 = ptp - prtp;
            }else {
                ptp = XYZ(nt.listNeuron[tmpj].x,nt.listNeuron[tmpj].y,nt.listNeuron[tmpj].z);
                p1 = ptp - ptpp;
                ptp = XYZ(nt.listNeuron[tmpk].x,nt.listNeuron[tmpk].y,nt.listNeuron[tmpk].z);
                p2 = ptp - ptpp;
            }

//            if(prtIndex == nt.hashNeuron.value(nt.listNeuron[cbs[k].front()].parent)){
//                XYZ prtp = XYZ(nt.listNeuron[prtIndex].x,nt.listNeuron[prtIndex].y,nt.listNeuron[prtIndex].z);
//                p1 = ptp - prtp;
//                ptp = XYZ(nt.listNeuron[tmp].x,nt.listNeuron[tmp].y,nt.listNeuron[tmp].z);
//                p2 = ptp - prtp;
//            }else {
//                ptp = XYZ(nt.listNeuron[tmp].x,nt.listNeuron[tmp].y,nt.listNeuron[tmp].z);
//                p2 = ptp - ptpp;
//            }
            double tmpD = dot(normalize(p1),normalize(p2));
            qDebug()<<"tow line id: "<<j<<" "<<k<<" tmpD: "<<tmpD;
            if(tmpD<minD){
                minD = tmpD;
                minIndex1 = j;
                minIndex2 = k;
            }
        }
    }

    if(minD<-0.9){
        for(int j=0; j<cbs.size(); j++){
            if(j != minIndex1 && j != minIndex2){
                for(int k=0; k<cbs[j].size()-1; k++){
                    isRemain[cbs[j][k]] = 1;
                }
            }else{
                prtIndex = nt.hashNeuron.value(nt.listNeuron[cbs[j].front()].parent);
                isRemain[prtIndex] = 1;
                for(int k=0; k<cbs[j].size(); k++){
                    isRemain[cbs[j][k]] = 2;
                }
                QStack<int> tmpStack = QStack<int>();
                tmp = cbs[j].back();
                while(children[tmp].size() == 1){
                    tmp = children[tmp][0];
                    isRemain[tmp] = 2;
                }
                if(children[tmp].size()>1){
                    tmpStack.push(tmp);
                }
                while (!tmpStack.isEmpty()) {
                    tmp = tmpStack.pop();
                    isRemain[tmp] = 2;
                    for(int k=0; k<children[tmp].size(); k++){
                        V3DLONG tmp1 = children[tmp][k];
                        while(children[tmp1].size() == 1){
                            isRemain[tmp1] = 2;
                            tmp1 = children[tmp1][0];
                        }
                        if(children[tmp1].size() == 0){
                            isRemain[tmp1] = 2;
                        }else {
                            tmpStack.push_back(tmp1);
                        }
                    }
                }
            }
        }
    }else{
        for(int j=0; j<cbs.size(); j++){
            for(int k=0; k<cbs[j].size()-1; k++){
                isRemain[cbs[j][k]] = 1;
            }
        }
    }

}

NeuronTree pruningInit(NeuronTree& nt, unsigned char* pdata, V3DLONG* sz, double bifurcationD, double somaTimes){
    vector<MyMarker*> inswc = swc_convert(nt);
    vector<MyMarker*> outswc;
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    double imgMean,imgStd;
    mean_and_std(pdata,tolSZ,imgMean,imgStd);
    double th = imgMean + 0.5*imgStd;
//    hierarchy_prune2(inswc,outswc,pdata,sz[0],sz[1],sz[2],th,10,false);

    happ(inswc,outswc,pdata,sz[0],sz[1],sz[2],th,2);

    NeuronTree outnt = swc_convert(outswc);
//    if(somaTimes>0){
//        NeuronTree tmpnt = pruningSoma(outnt,somaTimes);
//        outnt.listNeuron.clear();
//        outnt.hashNeuron.clear();
//        outnt.deepCopy(tmpnt);
//    }
//    pruningCross3(outnt,bifurcationD);
//    NeuronTree outnt1 = pruningByType(nt,2);
//    NeuronTree resultnt = pruningByLength(outnt1,10);


    for(V3DLONG i=0; i<inswc.size(); i++){
        delete inswc[i];
    }

//    return resultnt;
    return outnt;
}

void getHierarchySegmentLength(NeuronTree& nt, ofstream& csvFile, unsigned char* pdata, V3DLONG* sz){
    vector<MyMarker*> inswc = swc_convert(nt);
    vector<HierarchySegment*> topo_segs;
    swc2topo_segs(inswc,topo_segs,0,pdata,sz[0],sz[1],sz[2]);

    double path;
    vector<MyMarker*> markers = vector<MyMarker*>();
    for(V3DLONG i=0; i<topo_segs.size(); i++){
        markers.clear();
        path = 0;
        topo_segs[i]->get_markers(markers);
        for(int j=markers.size()-1; j>0; j--){
            path += dist(*(markers[j]),*(markers[j]->parent));
        }

        csvFile<<path<<','<<topo_segs[i]->length<<endl;
    }

}










