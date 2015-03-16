#ifndef GRAPH_H
#define GRAPH_H

class Path
{
public:
    V3DLONG src_ind;
    V3DLONG dst_ind;
    double w;
public:
    Path(V3DLONG point1,V3DLONG point2,double w)
    {
        this->w = w;
        src_ind = point1 > point2 ? point2 : point1;
        dst_ind = point1 <= point2 ? point2 : point1;
    }
    V3DLONG getDst(V3DLONG ind)
    {
        return src_ind == ind ? dst_ind : dst_ind == ind ? src_ind : -1;
    }
};

template<class GraphNode>
class Graph
{
public:
    GraphNode node;
    QMap<V3DLONG,Path*> connect;//连接集合
public:
    Graph(GraphNode node)
    {
        this->node = node;
    }
    ~Graph()
    {
        qDeleteAll(connect);
    }
};
#endif // GRAPH_H
