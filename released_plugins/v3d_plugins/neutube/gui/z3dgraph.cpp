#include <iostream>

#include "z3dgraph.h"
#include "znormcolormap.h"

using namespace std;

Z3DGraphNode::Z3DGraphNode()
{
  Z3DGraphNode(0.0, 0.0, 0.0);
}

Z3DGraphNode::Z3DGraphNode(double x, double y, double z, double r)
{
  m_center.set(x, y, z);
  m_radius = r;
  m_color.setRgb(0, 0, 0);
  m_shape = GRAPH_BALL;
}

Z3DGraphNode::Z3DGraphNode(const Z3DGraphNode &node)
{
  m_center = node.m_center;
  m_radius = node.m_radius;
  m_color = node.m_color;
  m_shape = node.m_shape;
}

void Z3DGraphNode::set(double x, double y, double z, double r)
{
  m_center.set(x, y, z);
  m_radius = r;
}

void Z3DGraphNode::loadJsonObject(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (eqstr(key, "center")) {
      ZJsonArray center;
      center.set(value, false);
      m_center.set(ZJsonParser::numberValue(center.at(0)),
                   ZJsonParser::numberValue(center.at(1)),
                   ZJsonParser::numberValue(center.at(2)));
    } else if (eqstr(key, "radius")) {
      m_radius = ZJsonParser::numberValue(value);
    } else if (eqstr(key, "color")) {
      ZJsonArray color;
      color.set(value, false);
      //cout << "color: " << ZJsonParser::integerValue(color.at(0)) << endl;
      m_color.setRgb(ZJsonParser::integerValue(color.at(0)),
               ZJsonParser::integerValue(color.at(1)),
               ZJsonParser::integerValue(color.at(2)));
      if (color.size() == 4) {
        m_color.setAlpha(ZJsonParser::integerValue(color.at(3)));
      }
    }
  }
}

void Z3DGraphNode::print()
{
  cout << "(" << m_center.x() << ", " << m_center.y() << ", "
       << m_center.z() << "), " << m_radius << ": " << "rgb("
       << m_color.red() << ", " << m_color.green() << ", " << m_color.blue()
       << ", " << m_color.alpha()
       << ")" << endl;
}

Z3DGraphEdge::Z3DGraphEdge() : m_shape(GRAPH_CYLINDER)
{
  set(-1, -1);
}

Z3DGraphEdge::Z3DGraphEdge(int vs, int vt) : m_shape(GRAPH_CYLINDER)
{
  QColor color;
  color.setRgb(0, 0, 255);
  color.setAlphaF(0.5);
  set(vs, vt, 1.0, false, color, color, GRAPH_LINE);
}

Z3DGraphEdge::Z3DGraphEdge(const Z3DGraphEdge &edge)
{
  set(edge.m_vs, edge.m_vt, edge.m_radius, edge.m_usingNodeColor,
      edge.m_startColor, edge.m_endColor, edge.m_shape);
}

void Z3DGraphEdge::set(int vs, int vt, double radius)
{
  m_vs = vs;
  m_vt = vt;
  m_radius = radius;
}

void Z3DGraphEdge::set(int vs, int vt, double radius, bool usingNodeColor,
                       const QColor &startColor, const QColor &endColor,
                       EGraphShape shape)
{
  set(vs, vt, radius);
  m_usingNodeColor = usingNodeColor;
  m_radius = radius;
  m_startColor = startColor;
  m_endColor = endColor;
  m_shape = shape;
}

void Z3DGraphEdge::loadJsonObject(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (eqstr(key, "node")) {
      ZJsonArray node;
      node.set(value, false);
      set(ZJsonParser::integerValue(node.at(0)),
          ZJsonParser::integerValue(node.at(1)));
    } else if (eqstr(key, "radius")) {
      m_radius = ZJsonParser::numberValue(value);
    } else if (eqstr(key, "color1")) {
      ZJsonArray color;
      color.set(value, false);
      m_startColor.setRgb(ZJsonParser::integerValue(color.at(0)),
                          ZJsonParser::integerValue(color.at(1)),
                          ZJsonParser::integerValue(color.at(2)));
      if (color.size() == 4) {
        m_startColor.setAlpha(ZJsonParser::integerValue(color.at(3)));
      }
      m_usingNodeColor = false;
    } else if (eqstr(key, "color2")) {
      ZJsonArray color;
      color.set(value, false);
      m_endColor.setRgb(ZJsonParser::integerValue(color.at(0)),
                          ZJsonParser::integerValue(color.at(1)),
                          ZJsonParser::integerValue(color.at(2)));
      if (color.size() == 4) {
        m_endColor.setAlpha(ZJsonParser::integerValue(color.at(3)));
      }
      m_usingNodeColor = false;
    } else if (eqstr(key, "shape")) {
      const char *shape = ZJsonParser::stringValue(value);
      setShape(shape);
    }
  }
}

void Z3DGraphEdge::setShape(const string &shape)
{
  if (shape == "cylinder") {
    setShape(GRAPH_CYLINDER);
  } else if (shape == "line") {
    setShape(GRAPH_LINE);
  }
}

void Z3DGraphEdge::print()
{
  cout << m_vs << "--" << m_vt << ": " << m_radius << ", " << "rgb("
       << m_startColor.red() << ", " << m_startColor.green() << ", "
       << m_startColor.blue() << ", " << m_startColor.alpha()
       << ")" << " " << "rgb("
       << m_endColor.red() << ", " << m_endColor.green() << ", "
       << m_endColor.blue() << ", " << m_endColor.alpha()
       << ")" << endl;
}

Z3DGraph::Z3DGraph()
{
}

void Z3DGraph::importPointNetwork(const ZPointNetwork &pointNetwork,
                                  ZNormColorMap *colorMap)
{
  m_nodeArray.clear();
  m_edgeArray.clear();

  for (size_t i = 0; i < pointNetwork.pointNumber(); i++) {
    m_nodeArray.push_back(Z3DGraphNode(pointNetwork.getPoint(i).x(),
                                       pointNetwork.getPoint(i).y(),
                                       pointNetwork.getPoint(i).z(),
                                       pointNetwork.getPoint(i).weight()));
  }

  for (size_t i = 0; i < pointNetwork.edgeNumber(); i++) {
    m_edgeArray.push_back(Z3DGraphEdge(pointNetwork.getEdgeNodeStart(i),
                                       pointNetwork.getEdgeNodeEnd(i)));

    m_nodeArray[m_edgeArray.back().vs()].setColor(QColor(255, 255, 0, 128));
    m_nodeArray[m_edgeArray.back().vt()].setColor(QColor(0, 0, 0, 128));

    if (colorMap != NULL) {
      QColor color = colorMap->mapColor(pointNetwork.getEdgeWeight(i));
      color.setAlphaF(0.5);

#ifdef _DEBUG_
      cout << "weight: " << pointNetwork.getEdgeWeight(i) << endl;
      cout << "color: " << color.red() << " " << color.green() << " "
           << color.blue() << " " << color.alpha() << endl;
#endif

      m_edgeArray.back().setStartColor(color);
      m_edgeArray.back().setEndColor(color);
    } else {
      m_edgeArray.back().useNodeColor();
    }
  }
}

void Z3DGraph::importJsonFile(const string &filePath)
{
  m_nodeArray.clear();
  m_edgeArray.clear();

  ZJsonObject jsonObject;
  jsonObject.load(filePath);

  const char *key;
  json_t *value;

  json_object_foreach(jsonObject.getData(), key, value) {
    if (eqstr(key, "3DGraph")) {
      const char *graphKey;
      json_t *graphObject;
      json_object_foreach(value, graphKey, graphObject) {
        if (eqstr(graphKey, "Node")) {
          ZJsonArray nodeArray;
          nodeArray.set(graphObject, false);
          m_nodeArray.resize(nodeArray.size());
          for (size_t i = 0; i < nodeArray.size(); ++i) {
            m_nodeArray[i].loadJsonObject(nodeArray.at(i));
          }
        } else if (eqstr(graphKey, "Edge")) {
          ZJsonArray edgeArray;
          edgeArray.set(graphObject, false         );
          m_edgeArray.resize(edgeArray.size());
          for (size_t i = 0; i < edgeArray.size(); ++i) {
            m_edgeArray[i].loadJsonObject(edgeArray.at(i));
          }
        }
      }
      break;
    }
  }
}

void Z3DGraph::print()
{
  cout << m_nodeArray.size() << " nodes" << endl;
  for (size_t i = 0; i < m_nodeArray.size(); ++i) {
    m_nodeArray[i].print();
  }

  cout << m_edgeArray.size() << " edges" << endl;
  for (size_t i = 0; i < m_edgeArray.size(); ++i) {
    m_edgeArray[i].print();
  }
}
