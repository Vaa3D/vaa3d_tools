#ifndef ZSWCNETWORK_H
#define ZSWCNETWORK_H

#include <vector>
#include <map>
#include "zswctree.h"
#include "tz_graph.h"

class ZPointNetwork;

struct SwcNodeConnection {
  Swc_Tree_Node *first;
  Swc_Tree_Node *second;
  ZPoint first_offset;
  ZPoint second_offset;
  double weight;

  SwcNodeConnection(Swc_Tree_Node *v1, Swc_Tree_Node *v2, double w) :
    first(v1), second(v2), weight(w) {}

  SwcNodeConnection(Swc_Tree_Node *v1, const ZPoint &offset1,
                    Swc_Tree_Node *v2, const ZPoint &offset2, double w) :
    first(v1), second(v2), first_offset(offset1), second_offset(offset2),
    weight(w) {}

};

class ZSwcNetwork
{
public:
  ZSwcNetwork();

  enum ESwcExportOption {
    EXPORT_ALL, EXPORT_SWC_TREE_ONLY
  };

  void importTxtFile(const std::string &filePath);
  void exportTxtFile(const std::string &filePath);
  void exportSwcFile(const std::string &filePath, int bodyId,
                     ESwcExportOption option = EXPORT_ALL);

  void addSwcTree(ZSwcTree *tree, const ZPoint &offset = ZPoint(0, 0, 0));
  void addConnection(int tree_index1, int node_id1, int tree_index2,
                     int node_id2, double weight = 1.0);
  void addConnection(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2,
                     double weight);
  void addConnection(Swc_Tree_Node *tn1, const ZPoint &offset1,
                     Swc_Tree_Node *tn2, const ZPoint &offset2, double weight);
  void addConnection(Swc_Tree_Node *tn1, const ZPoint &offset1, int tree_index1,
                     Swc_Tree_Node *tn2, const ZPoint &offset2, int tree_index2,
                     double weight);

  void layoutSwc();
  Graph* toGraph();

  ZPointNetwork* toPointNetwork();

  void boundBox(double *corner);

  ZSwcTree* getTree(int index);

  inline size_t treeNumber() const { return m_swcTreeArray.size(); }

  void merge(const ZSwcNetwork &network);

private:
  std::vector<ZSwcTree*> m_swcTreeArray;
  std::vector<SwcNodeConnection> m_connectionArray;
  std::vector<ZPoint> m_offsetArray;
  std::map<Swc_Tree_Node*, int> m_treeIndexMap;
};

#endif // ZSWCNETWORK_H
