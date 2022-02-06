#include "op_neurontree.h"
#include <queue>

void setNeuronTreeHash(NeuronTree &nt) {
    nt.hashNeuron.clear();
    for (V3DLONG i = 0; i < nt.listNeuron.size(); ++i) {
        nt.hashNeuron.insert(nt.listNeuron[i].n,i);
    }
}

NeuronTree shiftAndScaleNeuronTree(const NeuronTree &nt, const XYZ &d_xyz, const float scale) {
    NeuronTree result;
    result.deepCopy(nt);
    for (NeuronSWC& s : result.listNeuron) {
        s.x = (s.x / scale) - d_xyz.x;
        s.y = (s.y / scale) - d_xyz.y;
        s.z = (s.z / scale) - d_xyz.z;
    }
    return result;
}

NeuronTree shiftAndScaleNeuronTree(const NeuronTree &nt, const XYZ &center, const XYZ &box, const float scale) {
    XYZ d_xyz = center - box / XYZ(2,2,2);
    return shiftAndScaleNeuronTree(nt,d_xyz,scale);
}

NeuronTree cropNeuronTree(const NeuronTree &nt, const XYZ &min_xyz, const XYZ &max_xyz) {
    NeuronTree result;
    for (NeuronSWC s : nt.listNeuron) {
        if (s.x < min_xyz.x || s.y < min_xyz.y || s.z < min_xyz.z
                || s.x > max_xyz.x || s.y > max_xyz.y || s.z > max_xyz.z){
            continue;
        }
        result.listNeuron.push_back(s);
    }
    setNeuronTreeHash(result);
    return result;
}

NeuronTree refineNeuronTree(const NeuronTree &nt, unsigned char *pdata, long long *sz) {
    int step = 3;
    double sumMax = 0;
    int size = nt.listNeuron.size();
    int iMax = 0, jMax = 0, kMax = 0;
    for (int i = -step; i <= step; ++i) {
        for (int j = -step; j <= step; ++j) {
            for (int k = -step; k <= step; ++k) {
                int count = 0;
                int c = 0;
                double sum = 0;
                for (int s = 0; s < size; ++s) {
                    V3DLONG x = nt.listNeuron[s].x + i + 0.5;
                    V3DLONG y = nt.listNeuron[s].y + j + 0.5;
                    V3DLONG z = nt.listNeuron[s].z + k + 0.5;
                    if (x >= 0 && x < sz[0]
                            && y >= 0 && y < sz[1]
                            && z >= 0 && z < sz[2]) {
                        count++;
                        sum += pdata[z*sz[0]*sz[1]+y*sz[0]+x];
                    }
                }
                sum /= count;
                if (sum>sumMax) {
                    sumMax = sum;
                    iMax = i;
                    jMax = j;
                    kMax = k;
                }
            }
        }
    }

    NeuronTree result;
    result.copy(nt);
    for (NeuronSWC& s : result.listNeuron) {
        s.x += iMax;
        s.y += jMax;
        s.z += kMax;
    }
    return result;
}

vector<NeuronTree> splitNeuronTree(const NeuronTree &nt) {
    vector<V3DLONG> rootIndexes;
    V3DLONG size = nt.listNeuron.size();
    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(size,vector<V3DLONG>());
    for (V3DLONG i = 0; i < size; ++i) {
        V3DLONG prt = nt.listNeuron[i].parent;
        if (nt.hashNeuron.contains(prt)) {
            V3DLONG prtIndex = nt.hashNeuron.value(prt);
            children[prtIndex].push_back(i);
        } else {
            rootIndexes.push_back(i);
        }
    }

    vector<NeuronTree> results;
    for (V3DLONG rootIndex : rootIndexes) {
        NeuronTree result;
        queue<V3DLONG> q;
        q.push(rootIndex);
        while (!q.empty()) {
            V3DLONG tmp = q.front();
            result.listNeuron.push_back(nt.listNeuron[tmp]);
            q.pop();
            vector<V3DLONG>& child = children[tmp];
            for (V3DLONG childIndex : child) {
                q.push(childIndex);
            }
        }
        setNeuronTreeHash(result);
        results.push_back(result);
    }
    return results;
}
