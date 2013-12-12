#ifndef ZSWCBRANCH_H
#define ZSWCBRANCH_H

#include "tz_swc_tree.h"

#include <vector>
#include <string>

#include "zpoint.h"

class ZSwcBranch
{
public:
  ZSwcBranch();
  ZSwcBranch(Swc_Tree_Node *upEnd, Swc_Tree_Node *downEnd);

  void updateIterator();

  Swc_Tree_Node* root();

  std::vector<Swc_Tree_Node*> branchPointArray();
  std::vector<Swc_Tree_Node*> produceBranchSubTree();
  std::vector<Swc_Tree_Node*> toArray();

  bool contains(Swc_Tree_Node *tn);

  inline Swc_Tree_Node* upEnd() { return m_begin; }
  inline Swc_Tree_Node* downEnd() { return m_end; }

  int nodeNumber();

  void label(int v);
  void setType(int type);
  void addLabel(int dv);

  void updateAccumDistance();

  inline int size() const { return m_size; }

  double computeLength();

  void save(const char *filePath);

  std::vector<ZPoint> sample(double step);

  std::string toString();

  //Note: resample() modifies the intermediate
  void resample(double step);

  void removeSmallEnds(double ratio);

private:
  Swc_Tree_Node *m_begin; //upstream
  Swc_Tree_Node *m_end;   //downstream
  int m_size;
};

#endif // ZSWCBRANCH_H
