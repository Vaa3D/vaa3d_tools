#ifndef ZSWCPATH_H
#define ZSWCPATH_H

#include <vector>
#include "zpoint.h"
#include "tz_swc_tree.h"
#include "c_stack.h"

class ZSwcTree;

//A replace for ZSwcBranch
//The path does not have to be continuous.

class ZSwcPath : public std::vector<Swc_Tree_Node*>
{
public:
  ZSwcPath();
  ZSwcPath(Swc_Tree_Node *beginTn, Swc_Tree_Node *endTn);

  void label(int v);
  void setType(int type);
  void addLabel(int dv);

  //void updateAccumDistance();

  bool isEmpty() const;

  //It returns false if the path is empty.
  bool isContinuous() const;

  void append(Swc_Tree_Node *tn);

  //double computeLength();

  void save(const char *filePath);

  //std::vector<ZPoint> sample(double step);

  std::string toString();

  //Note: resample() modifies the intermediate
  //void resample(double step);

  void reverse();

  void print();

  inline ZSwcTree* getHostTree() {
    return m_hostTree;
  }

  inline void setHostTree(ZSwcTree *host) {
    m_hostTree = host;
  }

  void smoothZ();
  void smoothRadius(bool excludingBranchPoint = false);

  void labelStack(Stack *stack, int value);
  void labelStackAcrossZ(Stack *stack, int value);

  //will be moved to ZSwcPositionEstimator
  void resetPositionFromStack(const Stack *stack);

  /*!
   * \brief Overall length of the path
   */
  double getLength();

private:
  ZSwcTree *m_hostTree;
};

#endif // ZSWCPATH_H
