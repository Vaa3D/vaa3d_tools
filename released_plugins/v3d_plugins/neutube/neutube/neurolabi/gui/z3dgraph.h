#ifndef Z3DGRAPH_H
#define Z3DGRAPH_H

#include <vector>
#include <string>
#include <QColor>
#include "zpoint.h"
#include "zpointnetwork.h"
#include "zjsonparser.h"

class ZNormColorMap;

enum EGraphShape {
  GRAPH_NO_SHAPE, GRAPH_BALL, GRAPH_CYLINDER, GRAPH_LINE
};

class Z3DGraphNode {
public:
  Z3DGraphNode();
  Z3DGraphNode(double x, double y, double z, double r = 3.0);
  Z3DGraphNode(const Z3DGraphNode &node);

  inline ZPoint center() { return m_center; }
  inline double radius() { return m_radius; }
  inline QColor color() { return m_color; }
  inline EGraphShape shape() { return m_shape; }
  inline double x() { return m_center.x(); }
  inline double y() { return m_center.y(); }
  inline double z() { return m_center.z(); }

  inline void setColor(const QColor &color) { m_color = color; }

  void set(double x, double y, double z, double r);

  void loadJsonObject(json_t *obj);

  void print();

private:
  ZPoint m_center;
  double m_radius;
  QColor m_color;
  EGraphShape m_shape;
};

struct Z3DGraphEdge {
public:
  Z3DGraphEdge();
  Z3DGraphEdge(int vs, int vt);
  Z3DGraphEdge(const Z3DGraphEdge &edge);

  inline int vs() { return m_vs; }
  inline int vt() { return m_vt; }
  inline int radius() { return m_radius; }
  inline QColor startColor() { return m_startColor; }
  inline QColor endColor() { return m_endColor; }
  inline EGraphShape shape() { return m_shape; }
  inline bool isValid() { return vs() >= 0 && vt() >= 0; }
  void set(int vs, int vt, double radius = 1.0);
  void set(int vs, int vt, double radius, bool usingNodeColor,
           const QColor &startColor, const QColor &endColor,
           EGraphShape shape);
  inline void setShape(EGraphShape shape) { m_shape = shape; }
  inline void setShape(const std::string &shape);

  inline void setStartColor(const QColor &color) { m_startColor = color; }
  inline void setEndColor(const QColor &color) { m_endColor = color; }
  inline void useNodeColor(bool on = true) { m_usingNodeColor = on; }
  inline bool usingNodeColor() const { return m_usingNodeColor; }

  void loadJsonObject(json_t *obj);

  void print();
private:
  int m_vs;
  int m_vt;
  double m_radius;
  bool m_usingNodeColor;
  QColor m_startColor;
  QColor m_endColor;
  EGraphShape m_shape;
};

class Z3DGraph
{
public:
  Z3DGraph();

  inline size_t getNodeNumber() { return m_nodeArray.size(); }
  inline size_t getEdgeNumber() { return m_edgeArray.size(); }

  inline Z3DGraphNode getNode(size_t index) {
    return m_nodeArray[index];
  }

  inline Z3DGraphEdge getEdge(size_t index) {
    return m_edgeArray[index];
  }

  inline Z3DGraphNode getStartNode(size_t index) {
    return m_nodeArray[m_edgeArray[index].vs()];
  }
  inline Z3DGraphNode getEndNode(size_t index) {
    return m_nodeArray[m_edgeArray[index].vt()];
  }

public:
  void importPointNetwork(const ZPointNetwork &pointNetwork,
                          ZNormColorMap *colorMap = NULL);
  void importJsonFile(const std::string &filePath);

  void print();

private:
  std::vector<Z3DGraphNode> m_nodeArray;
  std::vector<Z3DGraphEdge> m_edgeArray;
};

#endif // Z3DGRAPH_H
