#include "branchtree.h"

#include <queue>

void adjustRalationForSplitBranches(Branch *origin, vector<Branch *> target) {
    target.front()->parent = origin->parent;
    if (origin->parent) {
        auto it = origin->parent->children.begin();
        for (; it != origin->parent->children.end(); ++it) {
            if (*it == origin) {
                origin->parent->children.erase(it);
                break;
            }
        }
        origin->parent->children.push_back(target.front());
    }
    for (int k = 1; k < target.size(); ++k) {
        target[k]->parent = target[k - 1];
        target[k - 1]->children.push_back(target[k]);
    }
    target.back()->children = origin->children;
    for (Branch* child : origin->children) {
        child->parent = target.back();
    }
}

void adjustRalationForRemoveBranch(Branch *origin) {
    if (origin->parent) {
        auto it = origin->parent->children.begin();
        for (; it != origin->parent->children.end(); ++it) {
            if (*it == origin) {
                origin->parent->children.erase(it);
                break;
            }
        }
    }

    for (Branch* child : origin->children) {
        if (origin->parent) {
            origin->parent->children.push_back(child);
        }
        child->parent = origin->parent;
    }
}

void PointFeature::getFeature(unsigned char *pdata, long long *sz, float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;

    double *vec1 = new double[3];
    double *vec2 = new double[3];
    double *vec3 = new double[3];
    double pc1, pc2, pc3;

    int xx = x + 0.5, yy = y + 0.5, zz = z + 0.5;
    xx = xx >= sz[0] ? sz[0] - 1 : xx;
    xx = xx < 0 ? 0 : xx;
    yy = yy >= sz[1] ? sz[1] - 1 : yy;
    yy = yy < 0 ? 0 : yy;
    zz = zz >= sz[2] ? sz[2] - 1 : zz;
    zz = zz < 0 ? 0 : zz;
    this->intensity = pdata[zz * sz[0]* sz[1] + yy * sz[0] + xx];
    computeCubePcaEigVec(pdata, sz, xx, yy, zz, 3, 3, 3, pc1, pc2, pc3, vec1, vec2, vec3);
    this->linearity_3 = pc2 == 0 ? 0 : pc1 / pc2;
    computeCubePcaEigVec(pdata, sz, xx, yy, zz, 5, 5, 5, pc1, pc2, pc3, vec1, vec2, vec3);
    this->linearity_5 = pc2 == 0 ? 0 : pc1 / pc2;
    computeCubePcaEigVec(pdata, sz, xx, yy, zz, 8, 8, 8, pc1, pc2, pc3, vec1, vec2, vec3);
    this->linearity_8 = pc2 == 0 ? 0 : pc1 / pc2;

    if (x < 5 || x > sz[0] -5 ||
            y < 5 || y > sz[1] - 5 ||
            z < 5 || z > sz[2] - 5) {
        this->nearEdge = 1;
    } else {
        this->nearEdge = 0;
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
}


void LineFeature::intial() {
    this->pointsFeature.clear();
    this->pointsFeature = vector<PointFeature>(5);
    this->directions.clear();
    this->directions = vector<XYZ>(5, XYZ());
    this->intensity_mean = this->intensity_std = 0;
    this->intensity_mean_r5 = this->intensity_std_r5 = 0;
    this->linearity_3_mean = this->linearity_5_mean = this->linearity_8_mean = 0;
}


vector<Branch*> Branch::splitByInflectionPoint(float d, float cosAngleThres) {
    vector<Branch*> results;
    int pointSize = this->points.size();
    float* path = new float[pointSize];
    memset(path, 0, sizeof(float) * pointSize);

    if (pointSize < 2) {
        results.push_back(this);
        return results;
    }

    for (int i = 1; i < pointSize; ++i) {
        float tmpD = zx_dist(this->points[i], this->points[i - 1]);
        path[i] = path[i-1] + tmpD;
    }

    XYZ v1,v2;
    XYZ p1,p2;
    p1 = XYZ(this->points[0].x, this->points[0].y, this->points[0].z);

    float cosAngle;
    int startPoint = 0;
    int i,j,k;

    int ppIndex,ccIndex,curIndex;
    XYZ pp1,pp2;

    float inflectionPointCosAngle;
    int inflectionPointIndex = -1;

    for (i = 0; i < pointSize;) {
        for (j = i+1; j < pointSize; ++j) {
            if (path[j] - path[i] > d) {
                p2 = XYZ(this->points[j].x, this->points[j].y, this->points[j].z);
                if (i == startPoint) {
                    v1 = p2 - p1;
                } else {
                    v2 = p2 - p1;
                    cosAngle = dot(normalize(v1), normalize(v2));
                    if (cosAngle < cosAngleThres) {
                        inflectionPointCosAngle = 1;
                        inflectionPointIndex = -1;
                        for (k = startPoint + 1; k < j; ++k) {
                            if ((path[k] - path[startPoint]) < d || (path[j] - path[k]) < d) {
                                continue;
                            }
                            curIndex = k;
                            ppIndex = startPoint;
                            ccIndex = j;
                            for (int ki = k - 1; ki >= startPoint; --ki) {
                                if (path[k] - path[ki] > d) {
                                    ppIndex = ki;
                                    break;
                                }
                            }
                            for (int ki = k + 1; ki <= j; ++ki) {
                                if (path[ki] - path[k] > d) {
                                    ccIndex = ki;
                                    break;
                                }
                            }
                            pp1 = XYZ(this->points[ppIndex].x, this->points[ppIndex].y, this->points[ppIndex].z);
                            pp2 = XYZ(this->points[ccIndex].x, this->points[ccIndex].y, this->points[ccIndex].z);
                            double tmpCosAngle = dot(normalize(pp1), normalize(pp2));
                            if (tmpCosAngle < inflectionPointCosAngle) {
                                inflectionPointCosAngle = tmpCosAngle;
                                inflectionPointIndex = k;
                            }
                        }
                        if(cosAngle > (sqrt(2.0)/2) || inflectionPointIndex == -1){
                            inflectionPointIndex = (j + startPoint) / 2;
                        }
                        Branch* line = new Branch();
                        for (k = startPoint; k <= inflectionPointIndex; ++k) {
                            line->points.push_back(this->points[k]);
                        }
                        results.push_back(line);
                        startPoint = j;
                    }
                }
                p1 = p2;
                break;
            }
        }
        i = j;
    }

    if (startPoint == 0) {
        results.push_back(this);
        return results;
    } else {
        Branch* line = new Branch();
        for (k = inflectionPointIndex; k < pointSize; ++k) {
            line->points.push_back(this->points[k]);
        }
        results.push_back(line);
    }

    adjustRalationForSplitBranches(this, results);

    return results;
}

vector<Branch*> Branch::splitByLength(float l_thres) {
    vector<Branch*> results;
    this->calLength();
    if (this->length < l_thres) {
        results.push_back(this);
        return results;
    }
    int length_mean = this->length / ceil(this->length / l_thres);
    int count_i = 1;
    float path_length = 0;
    int start_index = 0;
    for (int i = 1; i<this->points.size() - 1; ++i) {
        path_length += zx_dist(this->points[i], this->points[i - 1]);
        if (path_length > length_mean * count_i) {
            Branch* branch = new Branch();
            branch->points.insert(branch->points.end(), this->points.begin() + start_index, this->points.begin() + i + 1);
            results.push_back(branch);
            start_index = i;
            count_i++;
        }
    }
    Branch* branch = new Branch();
    branch->points.insert(branch->points.end(), this->points.begin() + start_index, this->points.end());
    results.push_back(branch);

    adjustRalationForSplitBranches(this, results);

    return results;
}

void Branch::removePointsNearSoma(const NeuronSWC &soma, float ratio) {
    if (this->level > 2) {
        return;
    }
    auto it = this->points.begin();
    for (; it != this->points.end(); ) {
        if (zx_dist(*it, soma) < soma.r * ratio) {
            this->points.erase(it);
        } else {
            break;
        }
    }
}

void Branch::removeTerminalPoints(float d) {
    auto it = this->points.begin();
    float length = 0;
    for (; it != this->points.end() - 1; ) {
        if (length < d) {
            length += zx_dist(*it, *(it + 1));
            this->points.erase(it);
        } else {
            break;
        }
    }

    it = this->points.end() - 1;
    length = 0;
    for (; it != this->points.begin(); ) {
        if (length < d) {
            length += zx_dist(*it, *(it - 1));
            this->points.erase(it);
            --it;
        } else {
            break;
        }
    }
}

float Branch::calLength() {
    this->length = 0;
    auto it = this->points.begin();
    for(; it != this->points.end() - 1; ++it) {
        this->length += zx_dist(*it, *(it + 1));
    }
    return this->length;
}

float Branch::calDistance() {
    this->distance = this->points.size() ? zx_dist(this->points.front(), this->points.back()) : 0;
    return this->distance;
}

void Branch::getFeature(unsigned char *pdata, long long *sz) {
    this->line_feature.intial();
    this->calLength();
    this->calDistance();
    int pointSize = this->points.size();

    double *vec1 = new double[3];
    double *vec2 = new double[3];
    double *vec3 = new double[3];
    int x,y,z;
    double pc1,pc2,pc3;

    vector<unsigned char> intensities;

    this->line_feature.pointsFeature[0].getFeature(pdata, sz, this->points.front().x, this->points.front().y, this->points.front().z);
    this->line_feature.intensity_mean += this->line_feature.pointsFeature.front().intensity;
    intensities.push_back(this->line_feature.pointsFeature.front().intensity);
    this->line_feature.linearity_3_mean += this->line_feature.pointsFeature.front().linearity_3;
    this->line_feature.linearity_5_mean += this->line_feature.pointsFeature.front().linearity_5;
    this->line_feature.linearity_8_mean += this->line_feature.pointsFeature.front().linearity_8;
    int point_i = 1;
    float cur_length = 0;

    for (int i = 1; i < pointSize - 1; ++i) {
        cur_length += zx_dist(this->points[i], this->points[i - 1]);
        if (cur_length > this->length / 5 * point_i) {
            this->line_feature.pointsFeature[point_i].getFeature(pdata, sz, this->points[i].x, this->points[i].y, this->points[i].z);
            this->line_feature.intensity_mean += this->line_feature.pointsFeature[point_i].intensity;
            intensities.push_back(this->line_feature.pointsFeature[point_i].intensity);
            this->line_feature.linearity_3_mean += this->line_feature.pointsFeature[point_i].linearity_3;
            this->line_feature.linearity_5_mean += this->line_feature.pointsFeature[point_i].linearity_5;
            this->line_feature.linearity_8_mean += this->line_feature.pointsFeature[point_i].linearity_8;
            point_i++;
        } else {
            x = this->points[i].x + 0.5;
            y = this->points[i].y + 0.5;
            z = this->points[i].z + 0.5;
            x = x >= sz[0] ? sz[0] - 1 : x;
            x = x < 0 ? 0 : x;
            y = y >= sz[1] ? sz[1] - 1 : y;
            y = y < 0 ? 0 : y;
            z = z >= sz[2] ? sz[2] - 1 : z;
            z = z < 0 ? 0 : z;
            this->line_feature.intensity_mean += pdata[z * sz[0] * sz[1] + y * sz[0] + x];
            intensities.push_back(pdata[z * sz[0] * sz[1] + y * sz[0] + x]);
            computeCubePcaEigVec(pdata, sz, x, y, z, 3, 3, 3, pc1, pc2, pc3, vec1, vec2, vec3);
            this->line_feature.linearity_3_mean += (pc2 == 0 ? 0 : pc1 / pc2);
            computeCubePcaEigVec(pdata, sz, x, y, z, 5, 5, 5, pc1, pc2, pc3, vec1, vec2, vec3);
            this->line_feature.linearity_5_mean += (pc2 == 0 ? 0 : pc1 / pc2);
            computeCubePcaEigVec(pdata, sz, x, y, z, 8, 8, 8, pc1, pc2, pc3, vec1, vec2, vec3);
            this->line_feature.linearity_8_mean += (pc2 == 0 ? 0 : pc1 / pc2);
        }
    }

    this->line_feature.pointsFeature[4].getFeature(pdata, sz, this->points.back().x, this->points.back().y, this->points.back().z);
    this->line_feature.intensity_mean += this->line_feature.pointsFeature.back().intensity;
    intensities.push_back(this->line_feature.pointsFeature.front().intensity);
    this->line_feature.linearity_3_mean += this->line_feature.pointsFeature.back().linearity_3;
    this->line_feature.linearity_5_mean += this->line_feature.pointsFeature.back().linearity_5;
    this->line_feature.linearity_8_mean += this->line_feature.pointsFeature.back().linearity_8;

    this->line_feature.intensity_mean /= pointSize;
    this->line_feature.linearity_3_mean /= pointSize;
    this->line_feature.linearity_5_mean /= pointSize;
    this->line_feature.linearity_8_mean /= pointSize;

    for (auto intensity : intensities) {
        this->line_feature.intensity_std += pow((intensity - this->line_feature.intensity_mean), 2);
    }
    this->line_feature.intensity_std = sqrt(this->line_feature.intensity_std / pointSize);

    NeuronTree nt_r5;
    for (NeuronSWC s : this->points) {
        s.radius = 5;
        nt_r5.listNeuron.push_back(s);
    }
    setNeuronTreeHash(nt_r5);
    vector<MyMarker*> markers_r5 = swc_convert(nt_r5);
    unsigned char* mask_r5 = 0;
    swcTomask(mask_r5, markers_r5, sz[0], sz[1], sz[2]);
    V3DLONG total_sz = sz[0] * sz[1] * sz[2];
    intensities.clear();
//    qDebug()<<"origin: "<<"r5_mean: "<<this->line_feature.intensity_mean_r5<<" r5_std: "<<this->line_feature.intensity_std_r5;
//    qDebug()<<"intensity size: "<<intensities.size();
    for (int i = 0; i < total_sz; ++i) {
        if (mask_r5[i] > 0) {
            this->line_feature.intensity_mean_r5 += pdata[i];
            intensities.push_back(pdata[i]);
        }
    }
//    qDebug()<<"after intensity size: "<<intensities.size();
    if (intensities.size() > 0) {
        this->line_feature.intensity_mean_r5 /= intensities.size();
    }
    for (auto intensity : intensities) {
        this->line_feature.intensity_std_r5 += pow((intensity - this->line_feature.intensity_mean_r5), 2);
    }
    if (intensities.size() > 0) {
        this->line_feature.intensity_std_r5 = sqrt(this->line_feature.intensity_std_r5 / intensities.size());
    }
//    qDebug()<<"r5_mean: "<<this->line_feature.intensity_mean_r5<<" r5_std: "<<this->line_feature.intensity_std_r5;

    if (this->line_feature.pointsFeature.front().x > this->line_feature.pointsFeature.back().x) {
        reverse(this->line_feature.pointsFeature.begin(), this->line_feature.pointsFeature.end());
    } else if (this->line_feature.pointsFeature.front().x == this->line_feature.pointsFeature.back().x) {
        if (this->line_feature.pointsFeature.front().y > this->line_feature.pointsFeature.back().y) {
            reverse(this->line_feature.pointsFeature.begin(), this->line_feature.pointsFeature.end());
        } else if (this->line_feature.pointsFeature.front().y == this->line_feature.pointsFeature.back().y){
            if (this->line_feature.pointsFeature.front().z > this->line_feature.pointsFeature.back().z) {
                reverse(this->line_feature.pointsFeature.begin(), this->line_feature.pointsFeature.end());
            }
        }
    }

    this->line_feature.directions[0] = XYZ(this->line_feature.pointsFeature.back().x - this->line_feature.pointsFeature.front().x,
                                           this->line_feature.pointsFeature.back().y - this->line_feature.pointsFeature.front().y,
                                           this->line_feature.pointsFeature.back().z - this->line_feature.pointsFeature.front().z);
    for (int i = 1; i < this->line_feature.pointsFeature.size(); ++i) {
        this->line_feature.directions[i] = XYZ(this->line_feature.pointsFeature[i].x - this->line_feature.pointsFeature[i - 1].x,
                this->line_feature.pointsFeature[i].y - this->line_feature.pointsFeature[i - 1].y,
                this->line_feature.pointsFeature[i].z - this->line_feature.pointsFeature[i - 1].z);
    }
    for (int i = 0; i < 5; ++i) {
        normalize(this->line_feature.directions[i]);
    }
}

bool BranchTree::initialize(NeuronTree &nt) {
    this->branches.clear();

    vector<V3DLONG> rootIndex = vector<V3DLONG>();
    V3DLONG size = nt.listNeuron.size();
    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(size,vector<V3DLONG>());
    for (V3DLONG i = 0; i < size; ++i) {
        V3DLONG prt = nt.listNeuron[i].parent;
        if (nt.hashNeuron.contains(prt)) {
            V3DLONG prtIndex = nt.hashNeuron.value(prt);
            children[prtIndex].push_back(i);
        } else {
            rootIndex.push_back(i);
        }
    }

    queue<V3DLONG> q;
    if (rootIndex.size() != 1) {
        return false;
    } else {
        this->soma = nt.listNeuron[rootIndex.front()];
        q.push(rootIndex.front());
        if (soma.parent == -1) {
            this->hasSoma = true;
        } else {
            this->hasSoma = false;
        }
    }

    while (!q.empty()) {
        V3DLONG tmp = q.front();
        q.pop();
        vector<V3DLONG>& child = children[tmp];
        for(int i = 0; i < child.size(); ++i)
        {
            Branch* branch = new Branch();
            int cIndex = child[i];
            branch->points.push_back(nt.listNeuron[tmp]);

            while(children[cIndex].size() == 1)
            {
                branch->points.push_back(nt.listNeuron[cIndex]);
                cIndex = children[cIndex][0];
            }
            if(children.at(cIndex).size() >= 1)
            {
                q.push(cIndex);
            }
            branch->points.push_back(nt.listNeuron[cIndex]);
            this->branches.push_back(branch);
        }
    }

    //initial parent
    for(int i = 0; i < this->branches.size(); ++i)
    {
        if(this->branches[i]->points.front().parent == this->soma.parent)
        {
            this->branches[i]->parent = nullptr;
        }
        else
        {
            for(int j = 0; j < this->branches.size(); ++j)
            {
                if(this->branches[i]->points.front().n == this->branches[j]->points.back().n)
                {
                    this->branches[i]->parent = this->branches[j];
                    break;
                }
            }
        }
    }

    //initial level
    for(int i = 0; i < this->branches.size(); ++i)
    {
        Branch* branch = this->branches[i];
        int level=0;
        while(branch->parent != nullptr)
        {
            level++;
            branch = branch->parent;
        }
        this->branches[i]->level = level;
    }

    //initial children
    for(int i = 0; i < this->branches.size(); ++i){
        if (this->branches[i]->parent) {
            this->branches[i]->parent->children.push_back(this->branches[i]);
        }
    }
}

void BranchTree::preProcess(float inflection_d, float cosAngleThres,
                            float l_thres_max, float l_thres_min,
                            float t_length, float soma_ratio) {
    vector<Branch*> tmp_results;
    for (Branch* branch : this->branches) {
        vector<Branch*> s_results = branch->splitByInflectionPoint(inflection_d, cosAngleThres);
        tmp_results.insert(tmp_results.end(), s_results.begin(), s_results.end());
    }

    this->branches.clear();
    for (Branch* branch : tmp_results) {
        vector<Branch*> s_results = branch->splitByLength(l_thres_max);
        this->branches.insert(this->branches.end(), s_results.begin(), s_results.end());
    }

    if (this->hasSoma) {
        for (Branch* branch : this->branches) {
            branch->removePointsNearSoma(this->soma, soma_ratio);
        }
    }

    for (Branch* branch : this->branches) {
        branch->removeTerminalPoints(t_length);
    }

    auto it = this->branches.begin();
    for (; it != this->branches.end();) {
        if ((*it)->calLength() < l_thres_min) {
            adjustRalationForRemoveBranch(*it);
            this->branches.erase(it);
        } else {
            ++it;
        }
    }
}


void BranchTree::getFeature(unsigned char *pdata, long long *sz) {
    for (Branch* branch : this->branches) {
        branch->getFeature(pdata, sz);
    }
}



