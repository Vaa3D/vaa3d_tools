#ifndef DETACHCONNECTEDDOMAIN_H
#define DETACHCONNECTEDDOMAIN_H

#include <iostream>
#include <vector>
#include <queue>
#include <QDebug>
#include "v3d_interface.h"
#include "SpLog.h"
#include "ExternalFunction.h"


using std::vector;
using std::queue;

struct Point
{
    int x;
    int y;
    int z;
    Point(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}
    Point(const Point& p) : x(p.x), y(p.y), z(p.z) {}
};

class Solution
{
public:
    int m;
    int n;
    int k;

    bool isvalid(int i, int j, int l, vector<vector<vector<int>>>& volume, vector<vector<vector<bool>>>& mask)
    {
        return i >= 0 && i < m && j >= 0 && j < n && l >= 0 && l < k && !mask[i][j][l] && volume[i][j][l] !=0;
    }

    void add(int i, int j, int l, vector<vector<vector<int>>>& volume, queue<Point>& q, vector<vector<vector<bool>>>& mask)
    {
        if (isvalid(i, j, l, volume, mask))
        {
            q.push(Point(i, j, l));
            mask[i][j][l] = true;
        }
    }

    vector<vector<Point>> bwlabel(vector<vector<vector<int>>>& volume)
    {
        m = volume.size();
        n = volume[0].size();
        k = volume[0][0].size();
        vector<vector<Point>> res;
        vector<Point> tmp;
        vector<vector<vector<bool>>> mask(m, vector<vector<bool>>(n, vector<bool>(k, false)));
        for (int i = 0; i < m; i++)
        {
            for (int j = 0; j < n; j++)
            {
                for (int l = 0; l < k; l++)
                {
                    if (mask[i][j][l] || volume[i][j][l] == 0)
                        continue;
                    tmp.clear();
                    queue<Point> q;
                    q.push(Point(i, j, l));
                    mask[i][j][l] = true;
                    int count=1;
                    while (!q.empty())
                    {
                        Point t = q.front();
                        q.pop();
                        tmp.push_back(t);
                        add(t.x - 1, t.y, t.z, volume, q, mask);
                        add(t.x + 1, t.y, t.z, volume, q, mask);
                        add(t.x, t.y - 1, t.z, volume, q, mask);
                        add(t.x, t.y + 1, t.z, volume, q, mask);
                        add(t.x, t.y, t.z - 1, volume, q, mask);
                        add(t.x, t.y, t.z + 1, volume, q, mask);
                    }
                    res.push_back(tmp);
                }
            }
        }
        return res;
    }

    QStringList detach(QString name,V3DPluginCallback2 &callback);
    QString seg(QString name,V3DPluginCallback2 &callback);
};
std::vector<std::vector<std::vector<int>>> reshape(unsigned char* data, long width, long height, long depth);
unsigned char* convertVectorToUnsignedChar(const std::vector<std::vector<std::vector<int>>>& data);
std::vector<std::vector<std::vector<int>>> rotateMatrix(const std::vector<std::vector<std::vector<int>>>& matrix);
std::vector<std::vector<std::vector<int>>> flipMatrixZ(const std::vector<std::vector<std::vector<int>>>& matrix);
std::vector<std::vector<std::vector<int>>> flipMatrixX(const std::vector<std::vector<std::vector<int>>>& matrix);
std::vector<std::vector<std::vector<int>>> flipMatrixY(const std::vector<std::vector<std::vector<int>>>& matrix);

#endif // DETACHCONNECTEDDOMAIN_H
