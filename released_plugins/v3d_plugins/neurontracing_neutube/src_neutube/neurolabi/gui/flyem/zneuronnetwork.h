#ifndef ZFLYEMSWCNETWORK_H
#define ZFLYEMSWCNETWORK_H

#include <vector>
#include <string>

#include "tz_graph.h"
#include "zswctree.h"
#include "zsynapseannotationarray.h"
#include "zsynapseannotationanalyzer.h"

class ZSwcNetwork;

namespace FlyEm {

class ZNeuronNetwork
{
public:
    ZNeuronNetwork();

    enum ESwcExportOption {
      EXPORT_ALL, EXPORT_SWC_TREE_ONLY
    };

    void import(const std::string &filePath);
    void exportSwcFile(const std::string &filePath, int bodyId,
                       ESwcExportOption option = EXPORT_ALL);

    void layoutSwc();

    int getSwcTreeIndex(int bodyId);
    ZSwcTree *getSwcTree(int bodyId);

    void addSwcTree(ZSwcTree *tree);
    void addSynapse(const ZPoint &pt1, int id1, const ZPoint &pt2, int id2);

    Graph* toGraph();

    ZSwcNetwork* toSwcNetwork();

private:
    std::vector<ZSwcTree*> m_swcTreeArray;
    std::vector<int> m_bodyIdArray;
    ZSynapseAnnotationArray m_synapseArray;
    ZSynapseAnnotationAnalyzer m_analyzer;
    std::vector<ZPoint> m_offsetArray;
};

}

#endif // ZFLYEMSWCNETWORK_H
