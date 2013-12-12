#include "zneuronnetwork.h"

#include <iostream>

#include "zstring.h"
#include "tz_utilities.h"
#include "zsynapseannotationanalyzer.h"
#include "zcuboid.h"
#include "tz_darray.h"
#include "zswcnetwork.h"
#include "swctreenode.h"

using namespace FlyEm;
using namespace std;

ZNeuronNetwork::ZNeuronNetwork()
{
  m_analyzer.setDownsampleRate(5, 2);
  m_analyzer.setStackSize(12000, 12000, 1300);
  m_analyzer.setResolution(4, 4, 45);
  m_analyzer.setStartNumber(161);
}

void ZNeuronNetwork::import(const std::string &filePath)
{
    ZString str;
    FILE *fp = fopen(filePath.c_str(), "r");

    ZString dir = ZString(filePath).dirPath();

    while (str.readLine(fp)) {
      str.trim();

      if (!str.isAbsolutePath()) {
        str = str.absolutePath(dir);
      }

      if (fhasext(str.c_str(), "swc")) {
        ZSwcTree *tree = new ZSwcTree;
        tree->load(str.c_str());
        m_swcTreeArray.push_back(tree);
        m_bodyIdArray.push_back(str.lastInteger());
      } else if (fhasext(str.c_str(), "json")) {
        m_synapseArray.loadJson(str.c_str());
      } else if (fhasext(str.c_str(), "cfg")) {
        m_analyzer.loadConfig(str.c_str());
      }
    }

    fclose(fp);
}

//Export all contents of the network
void ZNeuronNetwork::exportSwcFile(const std::string &filePath, int bodyId,
                                ESwcExportOption option)
{
    static const int maxTypeNumber = 14;

    ZSwcTree tree;
    Swc_Tree *subtree = NULL;

    /*
    ZCuboid b0;
    vector<ZCuboid> boxArray;
    vector<ZCuboid> originalBoxArray;
    vector<ZPoint> offsetArray;

    int type = 1;

    for (size_t i = 0; i < m_swcTreeArray.size(); i++) {
        subtree = m_swcTreeArray[i].cloneData();
        double corner[6];
        Swc_Tree_Bound_Box(subtree, corner);
        if (m_bodyIdArray[i] == bodyId) {
            b0.set(corner[0], corner[1], corner[2], corner[3], corner[4],
                   corner[5]);
        } else {
            boxArray.push_back(ZCuboid(corner[0], corner[1], corner[2],
                                       corner[3], corner[4],corner[5]));
            originalBoxArray.push_back(ZCuboid(corner[0], corner[1], corner[2],
                                               corner[3], corner[4],corner[5]));
        }
        Kill_Swc_Tree(subtree);
    }
    */

    //Layout the trees
    //b0.layout(&boxArray, 10.0);

    layoutSwc();

    //int index = 0;
    int type = 1;
    for (size_t i = 0; i < m_swcTreeArray.size(); i++) {
      subtree = m_swcTreeArray[i]->cloneData();
      Swc_Tree_Translate(subtree, m_offsetArray[i].x(),
                         m_offsetArray[i].y(), m_offsetArray[i].z());
      /*
      if (option == EXPORT_ALL) {
        if (m_bodyIdArray[i] != bodyId) {
          ZPoint offset(boxArray[index][0] - originalBoxArray[index][0],
                        boxArray[index][1] - originalBoxArray[index][1],
                        boxArray[index][2] - originalBoxArray[index][2]);
          Swc_Tree_Translate(subtree, offset.x(), offset.y(), offset.z());
          offsetArray.push_back(offset);
          index++;
        } else {
          offsetArray.push_back(ZPoint(0, 0, 0));
        }
      }
      */

      int validType = type % maxTypeNumber;

      if (validType == 1 || validType == 6) {
        type++;
        validType = type % maxTypeNumber;
      }

      Swc_Tree_Set_Type(subtree, validType);
      type++;
      tree.merge(subtree, true);
    }

    if (option == EXPORT_ALL) {
      FlyEm::SynapseDisplayConfig displayConfig;
      displayConfig.mode = FlyEm::SynapseDisplayConfig::SYNAPSE_PAIR;
      displayConfig.bodyId = bodyId;

      m_synapseArray.exportSwcFile("/tmp/zswcnetwork_exportswcfile_tmp.swc",
                                   m_analyzer.config(),
                                   FlyEm::SynapseLocation::ADJUSTED_SWC_SPACE,
                                   displayConfig);

      Swc_Tree *connect_tree =
          Read_Swc_Tree_E("/tmp/zswcnetwork_exportswcfile_tmp.swc");

      if (connect_tree != NULL) {
        Swc_Tree_Iterator_Start(connect_tree,
                                SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

        Swc_Tree_Node *tn = NULL;
        while ((tn = Swc_Tree_Next(connect_tree)) != NULL) {
          if (Swc_Tree_Node_Is_Regular(tn)) {
            if (Swc_Tree_Node_Is_Root(tn)) {
              tn->node.type = 6;
            } else {
              tn->node.type = 1;
            }

            bool found = false;
            for (size_t i = 0; i < m_bodyIdArray.size(); i++) {
              if (tn->node.label == m_bodyIdArray[i]) {
                m_swcTreeArray[i]->moveToSurface(
                      &(tn->node.x), &(tn->node.y), &(tn->node.z));
                Swc_Tree_Node_Translate(tn, m_offsetArray[i].x(),
                                        m_offsetArray[i].y(),
                                        m_offsetArray[i].z());
                found = true;
                break;
              }
            }

            if (!found) {
              Swc_Tree_Node_Merge_To_Parent(tn);
            }
          }
        }

        //Write_Swc_Tree("/Users/zhaot/Work/neutube/neurolabi/data/test2.swc", connect_tree);

        Swc_Tree_Iterator_Start(connect_tree,
                                SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

        while ((tn = Swc_Tree_Next(connect_tree)) != NULL) {
          if (!Swc_Tree_Node_Is_Root(tn)) {
            Swc_Tree_Node *btn = Swc_Tree_Node_Add_Break(tn, 0.5);
            btn->node.type = 6;
          }
        }

        tree.merge(connect_tree, true);
      }
    }

    tree.resortId();
    tree.save(filePath.c_str());
}

void ZNeuronNetwork::addSwcTree(ZSwcTree *tree)
{
  m_swcTreeArray.push_back(tree);
}

int ZNeuronNetwork::getSwcTreeIndex(int bodyId)
{
  size_t index = -1;
  for (size_t i = 0; i < m_bodyIdArray.size(); i++) {
    if (bodyId == m_bodyIdArray[i]) {
      index = i;
      break;
    }
  }

  return index;
}

ZSwcTree* ZNeuronNetwork::getSwcTree(int bodyId)
{
  ZSwcTree *tree = NULL;

  int index = getSwcTreeIndex(bodyId);
  if (index >= 0) {
    tree = m_swcTreeArray[index];
  }

  return tree;
}

void ZNeuronNetwork::addSynapse(const ZPoint &pt1, int id1,
                             const ZPoint &pt2, int id2)
{
  ZSynapseAnnotation synapse;
  synapse.setTBar(pt1.x(), pt1.y(), pt1.z(), id1, 1.0, "");
  synapse.addPartner(pt2.x(), pt2.y(), pt2.z(), id2, 1.0, "");
  m_synapseArray.push_back(synapse);
}

Graph* ZNeuronNetwork::toGraph()
{
  Graph *graph = New_Graph();

  graph->nvertex = m_swcTreeArray.size();

  map<int, int> bodyIdMap;
  for (size_t i = 0; i < m_bodyIdArray.size(); i++) {
    bodyIdMap[m_bodyIdArray[i]] = i;
  }

  for (size_t i = 0; i < m_synapseArray.size(); i++) {
    vector<SynapseLocation> *partnerArray =
        m_synapseArray[i].getPartnerArrayRef();
    SynapseLocation *tBar = m_synapseArray[i].getTBarRef();
    for (size_t j = 0; j < partnerArray->size(); j++) {
      if (bodyIdMap.count(tBar->bodyId()) > 0 &&
          bodyIdMap.count((*partnerArray)[j].bodyId()) > 0) {
        if (tBar->bodyId() != (*partnerArray)[j].bodyId()) {
          Graph_Add_Edge(graph, bodyIdMap[tBar->bodyId()],
                         bodyIdMap[(*partnerArray)[j].bodyId()]);
        } else {
          cout << "Same body tbar-psd found: " << tBar->bodyId() << endl;
        }
      }
    }
  }

  Graph_Normalize_Edge(graph);
  Graph_Remove_Duplicated_Edge(graph);

  return graph;
}

void ZNeuronNetwork::layoutSwc()
{
  Graph *graph = toGraph();

  vector<double> xArray(graph->nvertex);
  vector<double> yArray(graph->nvertex);

  /* Initialize the positions */
  for (size_t i = 0; i < m_swcTreeArray.size(); i++) {
    double z;
    Swc_Tree_Centroid(m_swcTreeArray[i]->data(), &(xArray[i]), &(yArray[i]), &z);
  }

  Graph_Workspace *gw = New_Graph_Workspace();

  vector<double> cxArray = xArray;
  vector<double> cyArray = yArray;

  Graph_Layout(graph, &(xArray[0]), &(yArray[0]), gw);


  m_offsetArray.resize(xArray.size());

  vector<ZCuboid> boundBoxArray(m_swcTreeArray.size());
  //vector<ZCuboid> originalBoundBoxArray(m_swcTreeArray.size());

  double minx = 0.0;
  double miny = 0.0;

  for (size_t i = 0; i < m_swcTreeArray.size(); i++) {
    double corner[6];

    m_swcTreeArray[i]->boundBox(corner);
    //originalBoundBoxArray[i].set(corner);

    corner[0] -= cxArray[i] - xArray[i];
    corner[1] -= cyArray[i] - yArray[i];
    corner[3] -= cxArray[i] - xArray[i];
    corner[4] -= cyArray[i] - yArray[i];

    if (corner[0] < minx) {
      minx = corner[0];
    }
    if (corner[1] < miny) {
      miny = corner[1];
    }
    boundBoxArray[i].set(corner);

#ifdef _DEBUG_
    boundBoxArray[i].print();
#endif
  }

  if (minx < 0) {
    for (size_t i = 0; i < xArray.size(); i++) {
      xArray[i] -= minx;
      boundBoxArray[i][0] -= minx;
      boundBoxArray[i][3] -= minx;
    }
  }

  if (miny < 0) {
    for (size_t i = 0; i < yArray.size(); i++) {
      yArray[i] -= miny;
      boundBoxArray[i][1] -= miny;
      boundBoxArray[i][4] -= miny;
    }
  }

  //Calculate scale
  double bestScale = 0.0;

  //For each pair of bounding boxes of swcs
  for (size_t i = 0; i < boundBoxArray.size(); i++) {
    for (size_t j = i + 1; j < boundBoxArray.size(); j++) {
      /*
      ZPoint movingVec(0, 0, 0);
      movingVec.setX(xArray[i] - xArray[j]);
      movingVec.setY(yArray[i] - yArray[j]);
*/
      //Calculate the minimal scale of separation
      double scale =
          boundBoxArray[i].estimateSeparateScale(boundBoxArray[j],
                                                 ZPoint(1.0, 1.0, 1.0));
      //set scale to the current largest one
      bestScale += scale;
      /*
      if (scale > bestScale) {
        bestScale = scale;
      }
      */
    }
  }

  bestScale /= boundBoxArray.size() * (boundBoxArray.size() - 1) / 2;

  for (size_t i = 0; i < m_swcTreeArray.size(); i++) {
    m_offsetArray[i].set(xArray[i] * bestScale - cxArray[i],
                         yArray[i] * bestScale - cyArray[i],
                         0.0);
    /*
    m_offsetArray[i].set(boundBoxArray[i][0] * bestScale -
        originalBoundBoxArray[i][0], boundBoxArray[i][1] * bestScale -
        originalBoundBoxArray[i][1], 0.0);
        */
    //m_swcTreeArray[i].translate(offsetArray[i].x(), offsetArray[i].y(), 0.0);
  }

/*
  cout << xArray[0] << ' ' << xArray[1] << ' ' <<
          xArray[2] << ' ' << endl;
  cout << yArray[0] << ' ' << yArray[1] << ' ' <<
          yArray[2] << ' ' << endl;
*/
  Kill_Graph_Workspace(gw);
  Kill_Graph(graph);

#if 0
  static const int maxTypeNumber = 14;

  ZSwcTree tree;
  Swc_Tree *subtree = NULL;

  ZCuboid b0;
  vector<ZCuboid> boxArray;
  vector<ZCuboid> originalBoxArray;
  vector<ZPoint> offsetArray;

  int type = 1;
  for (size_t i = 0; i < m_swcTreeArray.size(); i++) {
    subtree = m_swcTreeArray[i].data();
    double corner[6];
    Swc_Tree_Bound_Box(subtree, corner);
    if (m_bodyIdArray[i] == bodyId) {
      b0.set(corner[0], corner[1], corner[2], corner[3], corner[4],
             corner[5]);
    } else {
      boxArray.push_back(ZCuboid(corner[0], corner[1], corner[2],
                                 corner[3], corner[4],corner[5]));
      originalBoxArray.push_back(ZCuboid(corner[0], corner[1], corner[2],
                                         corner[3], corner[4],corner[5]));
    }
  }

  //Layout the trees
  b0.layout(&boxArray, 10.0);
  int index = 0;
  for (size_t i = 0; i < m_swcTreeArray.size(); i++) {
    subtree = m_swcTreeArray[i].cloneData();
    if (m_bodyIdArray[i] != bodyId) {
      ZPoint offset(boxArray[index][0] - originalBoxArray[index][0],
                    boxArray[index][1] - originalBoxArray[index][1],
                    boxArray[index][2] - originalBoxArray[index][2]);
      Swc_Tree_Translate(subtree, offset.x(), offset.y(), offset.z());
      offsetArray.push_back(offset);
      index++;
    } else {
      offsetArray.push_back(ZPoint(0, 0, 0));
    }

    int validType = type % maxTypeNumber;

    if (validType == 1 || validType == 6) {
      type++;
      validType = type % maxTypeNumber;
    }
    Swc_Tree_Set_Type(subtree, validType);
    type++;
    tree.merge(subtree, true);
  }

  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(connect_tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (Swc_Tree_Node_Is_Root(tn)) {
        tn->node.type = 6;
      } else {
        tn->node.type = 1;
      }

      bool found = false;
      for (size_t i = 0; i < m_bodyIdArray.size(); i++) {
        if (tn->node.label == m_bodyIdArray[i]) {
          m_swcTreeArray[i].moveToSurface(
                &(tn->node.x), &(tn->node.y), &(tn->node.z));
          Swc_Tree_Node_Translate(tn, offsetArray[i].x(),
                                  offsetArray[i].y(), offsetArray[i].z());
          found = true;
          break;
        }
      }

      if (!found) {
        Swc_Tree_Node_Merge_To_Parent(tn);
      }
    }
  }
#endif
}

ZSwcNetwork* ZNeuronNetwork::toSwcNetwork()
{
  ZSwcNetwork *network = new ZSwcNetwork;

  //Add all trees to swc network
  for (size_t i = 0; i < m_swcTreeArray.size(); i++) {
    network->addSwcTree(m_swcTreeArray[i], ZPoint(0, 0, 0));
  }

  //Add all connections to swc network

  //For each synapse annotation
  for (ZSynapseAnnotationArray::iterator synapseIter = m_synapseArray.begin();
       synapseIter != m_synapseArray.end(); ++synapseIter) {
    //Map the tbar to the swc space
    ZSynapseAnnotation synapse = *synapseIter;
    ZPoint pt = synapse.getTBarRef()->mapPosition(m_analyzer.config(),
                    SynapseLocation::ADJUSTED_SWC_SPACE);

    //Find the closest node
    int preTreeIndex = getSwcTreeIndex(synapse.getTBarRef()->bodyId());
    if (preTreeIndex >= 0) {
      ZSwcTree *tree = m_swcTreeArray[preTreeIndex];
      Swc_Tree_Node *preNode = NULL;
      if (tree != NULL) {
        preNode = tree->queryNode(pt);
      }


      if (preNode != NULL) {
        ZPoint prePos = SwcTreeNode::pos(preNode);
        tree->moveToSurface(&prePos);
        ZPoint offset1 = prePos - SwcTreeNode::pos(preNode);

        //For each post-synaptic partner
        vector<SynapseLocation> *partnerArray = synapse.getPartnerArrayRef();

        for (vector<SynapseLocation>::iterator partnerIter = partnerArray->begin();
             partnerIter != partnerArray->end(); ++partnerIter) {
          //Map it to the swc space
          SynapseLocation partner = *(partnerIter);
          ZPoint pt = partner.mapPosition(m_analyzer.config(),
                                          SynapseLocation::ADJUSTED_SWC_SPACE);
          int postTreeIndex = getSwcTreeIndex(partner.bodyId());
          if (postTreeIndex >= 0) {
            tree = m_swcTreeArray[postTreeIndex];

            //Find the closest node
            Swc_Tree_Node *postNode = NULL;
            if (tree != NULL) {
              //Add the connection to swc network
              postNode = tree->queryNode(pt);
              if (postNode != NULL) {
                ZPoint postPos = SwcTreeNode::pos(postNode);
                tree->moveToSurface(&postPos);
                ZPoint offset2 = postPos - SwcTreeNode::pos(postNode);
                network->addConnection(preNode, offset1, preTreeIndex,
                                       postNode, offset2, postTreeIndex, .5);
              }
            }
          }
        }
      }
    }
  }

  for (size_t i = 0; i < m_swcTreeArray.size(); i++) {
    if (m_swcTreeArray.size() == m_offsetArray.size()) {
      m_swcTreeArray[i]->translate(m_offsetArray[i].x(),
                                   m_offsetArray[i].y(),
                                   m_offsetArray[i].z());
    }
  }

  return network;
}
