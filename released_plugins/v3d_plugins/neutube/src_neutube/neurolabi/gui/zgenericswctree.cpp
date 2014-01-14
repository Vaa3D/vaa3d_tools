#include "zgenericswctree.h"

#include <stdlib.h>

#include "zswctreenode.h"

using namespace std;

ZGenericSwcTree::ZGenericSwcTree(void)
{
}

ZGenericSwcTree::ZGenericSwcTree(ZTreeNode<ZSwcNode> *root)
{
    m_root = root;
}

ZGenericSwcTree::~ZGenericSwcTree(void)
{
}

#if 0
ZTreeNode<ZSwcNode>* ZGenericSwcTree::query(
    int id //Id of the node to query
    )
{
    ZTreeNode<ZSwcNode> *tn = NULL;

    while ((tn = next()) != NULL) {
        if (tn->dataRef()->id() == id) {
            break;
        }
    }

    return tn;
}
#endif

bool ZGenericSwcTree::readSwcFile(
    const char *filePath  //The path of the swc file to read
    )
{
    bool isSuccess = true;

    //Open file stream
    ifstream stream(filePath, std::ios::in);

    isSuccess = !stream.fail();

    if (isSuccess) {
        clear();
        string line;
        vector<ZTreeNode<ZSwcNode>*> nodeMap(1);
        vector<int> parentIdArray(1);

        m_root = new ZTreeNode<ZSwcNode>; //A virtual node
        nodeMap[0] = m_root;
        parentIdArray[0] = -1;

        //For each line in the stream
        while (!std::getline(stream, line).eof()) {
            //Trim leading spaces
            line.erase(0, line.find_first_not_of(" \t\n"));
            if (!line.empty()) {
                //If it is not a comment
                if (line[0] != '#') {
                    //Turn the line into a swc node
                    char *pEnd;
                    int id = (int) strtod(line.c_str(), &pEnd);
                    if (id >= 0) {
                      ZTreeNode<ZSwcNode> *pNode =
                          static_cast<ZTreeNode<ZSwcNode>*>(new ZSwcTreeNode);
                        pNode->dataRef()->setId(id);
                        pNode->dataRef()->setType(
                            static_cast<ZSwcNode::EType>((int) strtod(pEnd, &pEnd)));
                        pNode->dataRef()->setX(strtod(pEnd, &pEnd));
                        pNode->dataRef()->setY(strtod(pEnd, &pEnd));
                        pNode->dataRef()->setZ(strtod(pEnd, &pEnd));
                        pNode->dataRef()->setR(strtod(pEnd, &pEnd));
                        int parentId = (int) strtod(pEnd, NULL);

                        //Add the node to the node map
                        if (id + 1 >= (int) nodeMap.size()) {
                            int newSize = id + 2;
                            nodeMap.resize(newSize, NULL);
                            parentIdArray.resize(newSize, -1);
                        }
                        nodeMap[id + 1] = pNode;
                        parentIdArray[id + 1] = parentId;
                    }
                }
            }
        }


        //For each node in the node map
        for (unsigned int i = 1; i < nodeMap.size(); ++i) {
            //Set the parent of the node
            if (nodeMap[i] != NULL) {
                int parentId = parentIdArray[nodeMap[i]->id() + 1];
                ZTreeNode<ZSwcNode> *parent = nodeMap[parentId + 1];
                nodeMap[i]->setParent(parent);
            }
        }
    }

    return isSuccess;
}

bool ZGenericSwcTree::exportSwcFile(
    const char *filePath  //The path of the swc file to export
    )
{

    ofstream output(filePath, ios_base::out);

    bool isSuccess = !output.fail();

    if (isSuccess) {
      #if 0
        if (!isEmpty()) {
            updateIterator(ITERATOR_BREADTH_FIRST, false);
            ZTreeNode<ZSwcNode> *tn = NULL;
            while ((tn = next()) != NULL) {
                if (tn->isRegular()) {
                    ZSwcTreeNode* swcNode = dynamic_cast<ZSwcTreeNode*>(tn);
                    output << swcNode->id() << " " << swcNode->type() << " " <<
                        swcNode->x() << " " << swcNode->y() << " " << swcNode->z() <<
                        " " << swcNode->r() << " " << swcNode->parentId() << endl;
                }
            }
        }
#endif
        output.close();
    }


    return isSuccess;
}

