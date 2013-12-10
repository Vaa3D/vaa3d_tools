#include <sstream>

#include "zswcbranch.h"
#include "tz_error.h"
#include "tz_geo3d_utils.h"
#include "tz_math.h"
#include "swctreenode.h"

using namespace std;

ZSwcBranch::ZSwcBranch()
{
  m_begin = NULL;
  m_end = NULL;
  m_size = 0;
}

ZSwcBranch::ZSwcBranch(Swc_Tree_Node *begin, Swc_Tree_Node *end)
{
  bool success = false;

  Swc_Tree_Node *root = NULL;

  m_size = 0;
  if (end != NULL) {
    if (begin == NULL) {
      success = true;
    }

    Swc_Tree_Node *tn = end;
    while (tn != NULL) {
      root = tn;
      m_size++;
      if (begin == tn) {
        success = true;
        break;
      }
      tn = tn->parent;
    }
  }

  TZ_ASSERT(success, "Ininalization failed");

  if (success) {
    m_begin = root;
    m_end = end;
  }
}

Swc_Tree_Node* ZSwcBranch::root()
{
  Swc_Tree_Node *root = m_end;
  if (root != NULL) {
    while (root->parent != NULL) {
      root = root->parent;
    }
  }

  return root;
}

vector<Swc_Tree_Node*> ZSwcBranch::branchPointArray()
{
  vector<Swc_Tree_Node*> array;

  int branchPointNumber = 0;
  Swc_Tree_Node *tn = m_end;
  while (tn != m_begin) {
    if (Swc_Tree_Node_Is_Branch_Point(tn)) {
      branchPointNumber++;
    }
    tn = tn->parent;
  }
  if (Swc_Tree_Node_Is_Branch_Point(m_begin)) {
    branchPointNumber++;
  }

  array.resize(branchPointNumber);

  tn = m_end;
  int index = branchPointNumber - 1;
  while (tn != m_begin) {
    if (Swc_Tree_Node_Is_Branch_Point(tn)) {
      array[index--] = tn;
    }
    tn = tn->parent;
  }
  if (Swc_Tree_Node_Is_Branch_Point(m_begin)) {
    array[index] = m_begin;
  }

  return array;
}

vector<Swc_Tree_Node*> ZSwcBranch::toArray()
{
  vector<Swc_Tree_Node*> array;

  updateIterator();

  Swc_Tree_Node *tn = upEnd();
  while (tn != NULL) {
    array.push_back(tn);
    tn = tn->next;
  }

  return array;
}

vector<Swc_Tree_Node*> ZSwcBranch::produceBranchSubTree()
{
  vector<Swc_Tree_Node*> array;

  int branchPointNumber = 0;
  Swc_Tree_Node *tn = m_end;
  while (tn != m_begin) {
    if (Swc_Tree_Node_Is_Branch_Point(tn)) {
      branchPointNumber++;
    }
    tn = tn->parent;
  }
  if (Swc_Tree_Node_Is_Branch_Point(m_begin)) {
    branchPointNumber++;
  }

  array.resize(branchPointNumber);

  tn = m_end;
  int index = branchPointNumber - 1;
  while (tn != m_begin) {
    if (Swc_Tree_Node_Is_Branch_Point(tn)) {
      Swc_Tree_Node *newTreeNode = New_Swc_Tree_Node();
      Swc_Tree_Node_Copy_Property(tn, newTreeNode);
      Swc_Tree_Node *child = tn->first_child;
      while (child != NULL) {
        Swc_Tree_Node *sibling = child->next_sibling;
        if (!contains(child)) {
          Swc_Tree_Node_Set_Parent(child, newTreeNode);
        }
        child = sibling;
      }
      array[index--] = newTreeNode;
    }
    tn = tn->parent;
  }
  if (Swc_Tree_Node_Is_Branch_Point(m_begin)) {
    Swc_Tree_Node *newTreeNode = New_Swc_Tree_Node();
    array[index] = newTreeNode;
    Swc_Tree_Node_Copy_Property(m_begin, newTreeNode);
    Swc_Tree_Node *child = m_begin->first_child;
    while (child != NULL) {
      Swc_Tree_Node *sibling = child->next_sibling;
      if (!contains(child)) {
        Swc_Tree_Node_Set_Parent(child, newTreeNode);
      }
      child = sibling;
    }
  }

  return array;
}

bool ZSwcBranch::contains(Swc_Tree_Node *tn)
{
  bool found = false;

  if (tn == m_begin) {
    found = true;
  } else {
    for (Swc_Tree_Node *iter = m_end; iter != m_begin; iter = iter->parent) {
      if (iter == tn) {
        found = true;
        break;
      }
    }
  }

  return found;
}

int ZSwcBranch::nodeNumber()
{
  TZ_ASSERT(!(m_end == NULL && m_begin != NULL), "Invalid branch.");
  int n = 0;

  Swc_Tree_Node *tn = m_end;
  if (tn != NULL) {
    n = 1;
  }

  while (tn != m_begin) {
    n++;
    tn = tn->parent;
  }

  return n;
}

void ZSwcBranch::updateAccumDistance()
{
  updateIterator();

  Swc_Tree_Node *iter = m_begin;
  iter->weight = 0.0;
  for (Swc_Tree_Node *iter = m_begin->next; iter != NULL; iter = iter->next) {
    iter->weight = iter->parent->weight + Swc_Tree_Node_Dist(iter, iter->parent);
  }
}

void ZSwcBranch::updateIterator()
{
  m_end->next = NULL;
  for (Swc_Tree_Node *iter = m_end; iter != m_begin; iter = iter->parent) {
    if (iter->parent != NULL) {
      iter->parent->next = iter;
    }
  }
}

double ZSwcBranch::computeLength()
{
  double length = 0.0;
  for (Swc_Tree_Node *iter = m_end; iter != m_begin; iter = iter->parent) {
    length += Swc_Tree_Node_Dist(iter, iter->parent);
  }

  return length;
}

void ZSwcBranch::save(const char *filePath)
{
  updateIterator();

  FILE *fp = fopen(filePath, "w");

  Swc_Tree_Node *tn = m_begin;
  int currentId = 1;
  while (tn != NULL) {
    fprintf(fp, "%d 1 %g %g %g %g %d\n", currentId, tn->node.x, tn->node.y,
            tn->node.z, tn->node.d, currentId + 1);
    currentId++;
    tn = tn->next;
  }

  fclose(fp);
}

string ZSwcBranch::toString()
{
  ostringstream stream;

  updateIterator();

  Swc_Tree_Node *tn = m_begin;

  while (tn != NULL) {
    stream << tn->node.x << ' ' << tn->node.y << ' ' << tn->node.z << ' '
           << tn->node.d << endl;
    tn = tn->next;
  }

  return stream.str();
}

vector<ZPoint> ZSwcBranch::sample(double step)
{
  vector<ZPoint> sampleArray;

  updateIterator();

  double targetLength = step;
  double prevLength = 0.0;
  double curLength = 0.0;

  sampleArray.push_back(
        ZPoint(m_begin->node.x, m_begin->node.y, m_begin->node.z));

  Swc_Tree_Node *tn = m_begin->next;
  while (tn != NULL) {
    double dist = Swc_Tree_Node_Dist(tn, tn->parent);
    curLength += dist;
    while (targetLength <= curLength) {
      double start[3];
      double end[3];
      double inter[3];

      Swc_Tree_Node_Pos(tn->parent, start);
      Swc_Tree_Node_Pos(tn, end);
      double lambda = (targetLength - prevLength) / dist;
      Geo3d_Lineseg_Break(start, end, lambda, inter);
      sampleArray.push_back(ZPoint(inter));
      targetLength += step;
    }

    prevLength = curLength;
    tn = tn->next;
  }

  return sampleArray;
}

void ZSwcBranch::label(int v)
{
  updateIterator();
  Swc_Tree_Node *tn = upEnd();
  while (tn != NULL) {
    Swc_Tree_Node_Set_Label(tn, v);
    tn = tn->next;
  }
}

void ZSwcBranch::addLabel(int dv)
{
  updateIterator();
  Swc_Tree_Node *tn = upEnd();
  while (tn != NULL) {
    Swc_Tree_Node_Set_Label(tn, SwcTreeNode::label(tn) + dv);
    tn = tn->next;
  }
}

void ZSwcBranch::setType(int type)
{
  updateIterator();
  Swc_Tree_Node *tn = upEnd();
  while (tn != NULL) {
    tn->node.type = type;
    tn = tn->next;
  }
}

void ZSwcBranch::resample(double step)
{
  double len = computeLength();

  if (len > 0) {
    int nseg = iround(len / step);
    if (nseg > 1) {
      double realStep = len / nseg;

      vector<ZPoint> pointArray = sample(realStep);

      if (nseg + 1 > nodeNumber()) {
        int n = nseg + 1 - nodeNumber();
        for (int i = 0; i < n; i++) {
          Swc_Tree_Node_Add_Break(m_end, 0.5);
        }
      } else {
        int n = nodeNumber() - nseg - 1;
        for (int i = 0; i < n; i++) {
          Swc_Tree_Node_Merge_To_Parent(m_end->parent);
        }
      }

      Swc_Tree_Node *tn = m_end;

      TZ_ASSERT(pointArray.size() > 1, "Invalide point number");

      size_t startIndex = nseg - 1;

      for (size_t i = startIndex; i > 0; i--) {
        tn = tn->parent;
        double pos[3];
        pointArray[i].toArray(pos);
        Swc_Tree_Node_Set_Pos(tn, pos);
      }
    } else {
      if (m_begin != m_end) {
        while (m_end->parent != m_begin) {
          Swc_Tree_Node_Merge_To_Parent(m_end->parent);
        }
      }
    }
  }
}

void ZSwcBranch::removeSmallEnds(double ratio)
{
  if (size() > 1) {
    if (SwcTreeNode::radius(upEnd()) >
        ratio * SwcTreeNode::radius(upEnd()->first_child)) {

    }
  }
}
