#include "zswcsizefeatureanalyzer.h"

#include "tz_error.h"
#include "tz_unipointer_linked_list.h"
#include "swctreenode.h"

using namespace std;

ZSwcSizeFeatureAnalyzer::ZSwcSizeFeatureAnalyzer()
{
  m_excludedLabel = -1;
  m_includedLabel = 0;
}

void ZSwcSizeFeatureAnalyzer::setParameter(const std::vector<double> &parameterArray)
{
  TZ_ASSERT(parameterArray.size() >= 2, "Invalid parameter number");

  m_excludedLabel = parameterArray[0];
  m_includedLabel = parameterArray[0];
}

std::vector<double> ZSwcSizeFeatureAnalyzer::computeFeature(Swc_Tree_Node *tn)
{
  std::vector<double> featureArray(1, 0);

  if (tn != NULL) {
    int n = 1;

    Swc_Tree_Node *pointer = tn;
    Unipointer_List *p = NULL;

    do {
      Swc_Tree_Node *child = pointer->first_child;

      while (child != NULL) {
        if (Swc_Tree_Node_Label(child) != m_excludedLabel) {
          if (m_includedLabel < 0 ||
              SwcTreeNode::label(child) == m_includedLabel) {
            Unipointer_Stack_Push(&p, child);
            n++;
          }
        }
        child = child->next_sibling;
      }
      pointer = (Swc_Tree_Node*) Unipointer_Stack_Pop(&p);
    } while (pointer != NULL);

    featureArray[0] = n;
  }

  return featureArray;
}

double ZSwcSizeFeatureAnalyzer::computeFeatureSimilarity(
    const vector<double> &featureArray1, const vector<double> &featureArray2)
{
  //double maxSize = 10000;

  double s2 = max(featureArray1[0], featureArray2[0]);
  double s1 = min(featureArray1[0], featureArray2[0]);

  TZ_ASSERT(s2 > 0.0, "Invalid number");

  return sqrt(s1) * s1 / s2;
}
