#include "connectivity_func.h"




bool getBrainRegions(QString imagePath, int flag, float ratio, int thres, V3DPluginCallback2 & callback) {
    cout<<flag<<" "<<ratio<<" "<<thres<<endl;
    unsigned char* dataSource = 0;
    V3DLONG szSource[4] = {0, 0, 0, 0};
    int datatype = 0;
    simple_loadimage_wrapper(callback, imagePath.toStdString().c_str(), dataSource, szSource, datatype);




    V3DLONG sz[4] = {0, 0, 0, 0};
    for (int i = 0; i < 4; ++i) {
       sz[i] = szSource[i];
    }


    V3DLONG tolSZ = sz[0] * sz[1] * sz[2];

    unsigned char* data1d = new unsigned char[tolSZ];
    memcpy(data1d, dataSource, tolSZ* sizeof(unsigned char));

    resample3dimg(data1d, sz, 5, 5, 5, 1);

    qDebug()<<"source: "<<szSource[0]<<" "<<szSource[1]<<" "<<szSource[2];
    qDebug()<<"after: "<<sz[0]<<" "<<sz[1]<<" "<<sz[2];

    tolSZ = sz[0] * sz[1] * sz[2];

    int th = 0;
    if (flag == 1) {
        float imageMean = 0, imageStd = 0;
        mean_and_std(data1d, tolSZ, imageMean, imageStd);
        th = round(imageMean + ratio * imageStd);
    } else if (flag == 2) {
        th = thres;
    }

    qDebug()<<"tolSZ: "<<tolSZ<<" th: "<<th;
    int sz01 = sz[0] * sz[1];
    vector<vector<int> > dataset;
    for (V3DLONG i = 0; i < tolSZ; ++i) {

        if (int(data1d[i]) >= th) {
//            cout<<"-----------------------"<<endl;
            int x = i % sz[0];
            int y = (i / sz[0]) % sz[1];
            int z = (i / sz01) % sz[2];
            vector<int> node = vector<int>();
            node.push_back(x);
            node.push_back(y);
            node.push_back(z);
            node.push_back(data1d[i]);
            dataset.push_back(node);
        }
    }
    qDebug()<<"all valid pixels: "<<dataset.size();

    KMEANS<int> kmeans;
    kmeans.loadData(dataset);
    kmeans.kmeans(12);

    unsigned char* pdata = new unsigned char[tolSZ];
    memset(pdata, 0, tolSZ * sizeof(unsigned char));
    qDebug()<<"tol size: "<<tolSZ;

    for(V3DLONG i = 0; i < dataset.size(); ++i) {
        V3DLONG index = dataset[i][2] * sz01 + dataset[i][1] * sz[0] + dataset[i][0];
        pdata[index] = kmeans.clusterAssment[i].minIndex + 1;
    }

    resample3dimg(pdata, sz, sz[0] / (double)szSource[0], sz[1] / (double)szSource[1], sz[2] / (double)szSource[2], 2);
    qDebug()<<"last: "<<sz[0]<<" "<<sz[1]<<" "<<sz[2];

    QString indexImagePath = imagePath + "_index.v3draw";
    simple_saveimage_wrapper(callback, indexImagePath.toStdString().c_str(), pdata, sz, datatype);

    QList<ImageMarker> markers;
    for (int i = 0; i < 12; ++i) {
        markers.push_back(ImageMarker(0, 0, 0));
    }
    for (V3DLONG i = 0; i < szSource[0] * szSource[1] * szSource[2]; ++i)
    {
        int index = pdata[i] - 1;
        if (index < 0) {
            continue;
        }
        int x = i % sz[0];
        int y = (i / sz[0]) % sz[1];
        int z = (i / sz01) % sz[2];
        markers[index].x += dataSource[i] * x;
        markers[index].y += dataSource[i] * y;
        markers[index].z += dataSource[i] * z;
        markers[index].radius += dataSource[i];
    }

    for (int i = 0; i < 12; ++i) {
        markers[i].x /= markers[i].radius;
        markers[i].y /= markers[i].radius;
        markers[i].z /= markers[i].radius;
    }

    QString indexMarkerPath = imagePath + "_index.marker";
    writeMarker_file(indexMarkerPath, markers);

    if (dataSource) {delete[] dataSource;}
    if (data1d) {delete[] data1d;}
    if (pdata) {delete[] pdata;}
}

bool getIndexImageMarkers(QString indexImagePath, QString imagePath, int flag, float ratio, int thres, V3DPluginCallback2 & callback) {
    cout<<flag<<" "<<ratio<<" "<<thres<<endl;
    unsigned char* dataIndex = 0;
    V3DLONG szIndex[4] = {0, 0, 0, 0};
    int datatypeIndex = 0;
    simple_loadimage_wrapper(callback, indexImagePath.toStdString().c_str(), dataIndex, szIndex, datatypeIndex);

    unsigned char* dataImage = 0;
    V3DLONG szImage[4] = {0, 0, 0, 0};
    int datatypeImage = 0;
    simple_loadimage_wrapper(callback, imagePath.toStdString().c_str(), dataImage, szImage, datatypeImage);

    if (szIndex[0] != szImage[0] ||
            szIndex[1] != szImage[1] ||
            szIndex[2] != szImage[2]) {
        if (dataIndex) {delete[] dataIndex;}
        if (dataImage) {delete[] dataImage;}
        return false;
    }

    V3DLONG tolSZ = szIndex[0] * szIndex[1] * szIndex[2];
//    void* indexPtr = (void*)dataIndex;
//    void* imagePtr = (void*)dataImage;

    int th = 0;
    if (flag == 1) {
        float imageMean = 0, imageStd = 0;
        if (datatypeImage == 1) {
            mean_and_std(dataImage, tolSZ, imageMean, imageStd);
        } else if (datatypeImage == 2) {
            mean_and_std((unsigned short*)dataImage, tolSZ, imageMean, imageStd);
        } else if (datatypeImage == 4) {
            mean_and_std((float*)dataImage, tolSZ, imageMean, imageStd);
        }
        th = round(imageMean + ratio * imageStd);
    } else if (flag == 2) {
        th = thres;
    }

    qDebug()<<"th: "<<th;

    int max_index = 0;
    map<int, ImageMarker> markerMap;
    map<int, set<int> > neighborIndexMap;
    int p1[6] = {0, 0, 0, 0, 1, -1};
    int p2[6] = {0, 0, 1, -1, 0, 0};
    int p3[6] = {1, -1, 0, 0, 0, 0};

    for (V3DLONG i = 0; i < tolSZ; ++i) {
        int index = -1;
        if (datatypeIndex == 1) {
            index = dataIndex[i];
        } else if (datatypeIndex == 2) {
            index = ((unsigned short*)dataIndex)[i];
        } else if (datatypeIndex == 4) {
            index = ((float*)dataIndex)[i];
        }
        if (index == 0) {
            continue;
        }

        max_index = max_index > index ? max_index : index;
        int x = i % szIndex[0];
        int y = (i / szIndex[0]) % szIndex[1];
        int z = (i / (szIndex[0] * szIndex[1])) % szIndex[2];

        for (int j = 0; j < 6; ++j) {
            int nx = x + p1[j];
            int ny = y + p2[j];
            int nz = z + p3[j];
            V3DLONG n_index = nz * szIndex[0] * szIndex[1] + ny * szIndex[0] + nx;
            if (n_index < 0 || n_index >= tolSZ) {
                continue;
            }
            int nIndex = -1;
            if (datatypeIndex == 1) {
                nIndex = dataIndex[n_index];
            } else if (datatypeIndex == 2) {
                nIndex = ((unsigned short*)dataIndex)[n_index];
            } else if (datatypeIndex == 4) {
                nIndex = ((float*)dataIndex)[n_index];
            }
            if (nIndex == -1 || nIndex == index) {
                continue;
            }
            if (neighborIndexMap.find(index) == neighborIndexMap.end()) {
                set<int> neighborIndexSet;
                neighborIndexSet.insert(nIndex);
                neighborIndexMap[index] = neighborIndexSet;
            } else {
                neighborIndexMap[index].insert(nIndex);
            }
        }

        int value = -1;
        if (datatypeImage == 1) {
            value = dataImage[i];
        } else if (datatypeImage == 2) {
            value = ((unsigned short*)dataImage)[i];
        } else if (datatypeImage == 4) {
            value = ((float*)dataImage)[i];
        }

        if (value == -1 || index == -1) {
            if (dataIndex) {delete[] dataIndex;}
            if (dataImage) {delete[] dataImage;}
            return false;
        }
        if (markerMap.find(index) == markerMap.end()){
            ImageMarker curIndexMarker = ImageMarker(x * value, y * value, z * value);
            curIndexMarker.radius = value;
            curIndexMarker.n = 1;
            curIndexMarker.type = 0;
            markerMap[index] = curIndexMarker;
        } else {
            markerMap[index].x += x * value;
            markerMap[index].y += y * value;
            markerMap[index].z += z * value;
            markerMap[index].radius += value;
            markerMap[index].n += 1;
        }
    }

    qDebug()<<"max index: "<<max_index;

    QList<ImageMarker> markers;
    map<int, ImageMarker>::iterator it = markerMap.begin();
    while(it != markerMap.end()) {
        ImageMarker& m = it->second;
        if (m.radius / m.n >= th) {
            m.x /= m.radius;
            m.y /= m.radius;
            m.z /= m.radius;
            V3DLONG i = floor(m.z) * szIndex[0] * szIndex[1] + floor(m.y) * szIndex[0] + floor(m.x);
            int index = -1;
            if (datatypeIndex == 1) {
                index = dataIndex[i];
            } else if (datatypeIndex == 2) {
                index = ((unsigned short*)dataIndex)[i];
            } else if (datatypeIndex == 4) {
                index = ((float*)dataIndex)[i];
            }
            if (index == it->first) {
                markers.push_back(m);
                m.type = 1;
            }
        }
        it++;
    }

    map<int, set<int> >::iterator nit = neighborIndexMap.begin();
    while(nit != neighborIndexMap.end()) {
        set<int>& curSet = nit->second;
        qDebug()<<"before: "<<curSet.size();
        for(set<int>::iterator sit=curSet.begin(); sit!=curSet.end();){
            if (markerMap[*sit].type == 0) {
                set<int>::iterator tmpit = sit;
                sit++;
                curSet.erase(tmpit);
            } else {
                sit++;
            }
        }
        qDebug()<<"after: "<<curSet.size();
        nit++;
    }

    qDebug()<<"marker size:"<<markers.size();

    Image4DSimple * p4dImage = callback.loadImage((char*)imagePath.toStdString().c_str());
    p4dImage->convert_to_UINT8();

    it = markerMap.begin();
    while(it != markerMap.end()) {
        int brainRegionIndex = it->first;
        if (it->second.type == 0) {
            it++;
            continue;
        }
        runGDForBrainRegion(brainRegionIndex, markerMap, neighborIndexMap, dataIndex, datatypeIndex, p4dImage, szIndex, imagePath);
        it++;
    }

    QString indexMarkerPath = imagePath + "_index.marker";
    writeMarker_file(indexMarkerPath, markers);

    if (dataIndex) {delete[] dataIndex;}
    if (dataImage) {delete[] dataImage;}
}

bool runGDForBrainRegion(int brainRegionIndex, map<int, ImageMarker>& markerMap, map<int, set<int> >& neighborIndexMap, unsigned char* dataIndex, int datatypeIndex, Image4DSimple * p4dImage, V3DLONG* sz, QString imageDir){
    set<int> neighborIndexes = neighborIndexMap[brainRegionIndex];
    if (neighborIndexes.empty()) {
        return false;
    }

    int minX = INT32_MAX, maxX = 0, minY = INT32_MAX, maxY = 0, minZ = INT32_MAX, maxZ = 0;
    V3DLONG tolSZ = sz[0] * sz[1] * sz[2];
    for (V3DLONG i = 0; i < tolSZ; ++i) {
        int x = i % sz[0];
        int y = (i / sz[0]) % sz[1];
        int z = (i / (sz[0] * sz[1])) % sz[2];
        int index = -1;
        if (datatypeIndex == 1) {
            index = dataIndex[i];
        } else if (datatypeIndex == 2) {
            index = ((unsigned short*)dataIndex)[i];
        } else if (datatypeIndex == 4) {
            index = ((float*)dataIndex)[i];
        }
        if (index == brainRegionIndex || neighborIndexes.find(index) != neighborIndexes.end()) {
            minX = minX <= x ? minX : x;
            maxX = maxX >= x ? maxX : x;
            minY = minY <= y ? minY : y;
            maxY = maxY >= y ? maxY : y;
            minZ = minZ <= z ? minZ : z;
            maxZ = maxZ >= z ? maxZ : z;
        }
    }
    qDebug()<<"min max xyz: "<<minX<<" "<<maxX<<" "<<minY<<" "<<maxY<<" "<<minZ<<" "<<maxZ;
    qDebug()<<"p4dimage: "<<p4dImage->getXDim()<<" "<<p4dImage->getYDim()<<" "<<p4dImage->getZDim();

    Image4DSimple* p4dCropImage = new Image4DSimple();
    if (!p4dCropImage->createImage(maxX - minX + 1, maxY - minY + 1, maxZ - minZ + 1, 1, V3D_UINT8)) {
        qDebug()<<"create false";
        return false;
    }

    qDebug()<<"create ";
    if(!subvolumecopy(p4dCropImage,
                  p4dImage,
                  minX, maxX - minX + 1,
                  minY, maxY - minY + 1,
                  minZ, maxZ - minZ + 1, 0, 1)){
        qDebug()<<"subvolume false";
        return false;
    }

    qDebug()<<"subvolume copy: ";

    unsigned char ****p4d = 0;
    unsigned char * indata1d = p4dCropImage->getRawDataAtChannel(0);

    V3DLONG sz_tracing[4] = {maxX - minX + 1, maxY - minY + 1, maxZ - minZ + 1, 1};
    new4dpointer(p4d, sz_tracing[0], sz_tracing[1], sz_tracing[2], sz_tracing[3], indata1d);

    qDebug()<<"new4dpointer: ";

    ImageMarker m = markerMap[brainRegionIndex];
    LocationSimple p0 = LocationSimple(m.x - minX, m.y - minY, m.z - minZ);
    vector<LocationSimple> pp;
    NeuronTree nt;
    set<int>::iterator sit = neighborIndexes.begin();
    while(sit != neighborIndexes.end()){
        qDebug()<<"*sit: "<<*sit;
        ImageMarker nm = markerMap[*sit];
        LocationSimple p = LocationSimple(nm.x - minX, nm.y - minY, nm.z - minZ);
        pp.push_back(p);
        sit++;
    }

    qDebug()<<"parameter end ";

    double weight_xy_z=1.0;
    bool b_mergeCloseBranches = false;
    bool b_usedshortestpathonly = false;
    bool b_postTrim = true;
    bool b_pruneArtifactBranches = true;
    int ds_step = 1;

    CurveTracePara trace_para;
    trace_para.channo = 0;
    trace_para.sp_graph_resolution_step = ds_step;
    trace_para.b_deformcurve = b_usedshortestpathonly;
    trace_para.b_postMergeClosebyBranches = b_mergeCloseBranches;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_post_trimming = b_postTrim;
    trace_para.b_pruneArtifactBranches = b_pruneArtifactBranches;

    nt = v3dneuron_GD_tracing(p4d, sz_tracing, p0, pp, trace_para, weight_xy_z);

    qDebug()<<"gd end ";

    for(int i = 0; i < nt.listNeuron.size(); ++i){
        nt.listNeuron[i].x += minX;
        nt.listNeuron[i].y += minY;
        nt.listNeuron[i].z += minZ;
    }

    QString swcPath = imageDir + QString::number(brainRegionIndex) + ".swc";
    writeSWC_file(swcPath, nt);

    if (p4d) {delete4dpointer(p4d, sz_tracing[0], sz_tracing[1], sz_tracing[2], sz_tracing[3]);}
    if (p4dCropImage) {delete p4dCropImage;}
}

bool resample3DImage(QString inImagePath, QString outImagePath, int sz_x, int sz_y, int sz_z, int flag, V3DPluginCallback2 & callback) {
    unsigned char* dataImage = 0;
    V3DLONG szImage[4] = {0, 0, 0, 0};
    int datatypeImage = 0;
    simple_loadimage_wrapper(callback, inImagePath.toStdString().c_str(), dataImage, szImage, datatypeImage);

    qDebug()<<"before: "<<szImage[0]<<" "<<szImage[1]<<" "<<szImage[2];
    if (datatypeImage == 1) {
        resample3dimg(dataImage, szImage, szImage[0] / (double)sz_x, szImage[1] / (double)sz_y, szImage[2] / (double)sz_z, flag);
    } else if (datatypeImage == 2) {
        resample3dimg((unsigned short*&)dataImage, szImage, szImage[0] / (double)sz_x, szImage[1] / (double)sz_y, szImage[2] / (double)sz_z, flag);
    } else if (datatypeImage == 4) {
        resample3dimg((float*&)dataImage, szImage, szImage[0] / (double)sz_x, szImage[1] / (double)sz_y, szImage[2] / (double)sz_z, flag);
    }


    qDebug()<<"after: "<<szImage[0]<<" "<<szImage[1]<<" "<<szImage[2];

    qDebug()<<"datatype: "<<datatypeImage;
    simple_saveimage_wrapper(callback, outImagePath.toStdString().c_str(), dataImage, szImage, datatypeImage);

    if (dataImage) {delete[] dataImage;}
}

