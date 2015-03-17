#include "tree.h"

//Tree::Tree(INeuronProcessObject *p)
//{
//    m_ProcessObject =  p;
//    identifyName =  std::string("Tree");
//}

//Tree::~Tree()
//{

//}

//bool Tree::IsEmpty() const
//{
//    return m_Tree.empty();
//}

//Neurite& Tree::GetNeurite(int idx)
//{
//    return m_Tree[idx];
//}

//const Neurite &Tree::GetNeurite(int idx) const
//{
//    return m_Tree[idx];
//}

//void Tree::SetTree(std::vector<VectorVec5d> &treenode)
//{
//    m_Tree.clear();
//    Neurite tmpNeurite;
//    NeuronNode node;
//    for(int i = 0; i < treenode.size(); ++i){
//        tmpNeurite.ID = i;
//        tmpNeurite.m_Node.clear();
//        for(int j = 0; j < treenode[i].size(); ++j){
//            const Vec5d& cur = treenode[i][j];
//            node.ID = j;
//            node.PID = j - 1;
//            node.x = cur(0);
//            node.y = cur(1);
//            node.z = cur(2);
//            node.r = cur(3);
//            node.v = cur(4);
//            tmpNeurite.m_Node.push_back(node);
//        }
//        m_Tree.push_back(tmpNeurite);
//    }
//}


TreeConnect::TreeConnect(INeuronProcessObject *p)
{
    m_ProcessObject =  p;
    identifyName =  std::string("TreeConnect");
}

TreeConnect::~TreeConnect()
{

}

bool TreeConnect::IsEmpty() const
{
    return m_Connect.empty();
}

void TreeConnect::Swap(VectorMat2i &arg)
{
    m_Connect.swap(arg);
}

void TreeConnect::SetConnect(const VectorMat2i &arg)
{
    m_Connect = arg;
}


TreeCurve::TreeCurve(INeuronProcessObject *p)
{
    m_ProcessObject =  p;
    identifyName =  std::string("TreeCurve");
}

TreeCurve::~TreeCurve()
{

}

bool TreeCurve::IsEmpty() const
{
    return m_Curve.empty();
}

void TreeCurve::Swap(std::vector<VectorVec5d> &arg)
{
    m_Curve.swap(arg);
}

void TreeCurve::SetCurve(std::vector<VectorVec5d> &arg)
{
    m_Curve = arg;
}


SeperateTree::SeperateTree(INeuronProcessObject *p)
{
    m_ProcessObject = p;
}

SeperateTree::~SeperateTree()
{

}

bool SeperateTree::IsEmpty() const
{
    return m_Tree.empty();
}

void SeperateTree::Swap(std::vector<std::vector<VectorVec5d> > &arg)
{
    m_Tree.swap(arg);
}

void SeperateTree::SetTree(std::vector<std::vector<VectorVec5d> > &arg)
{
    m_Tree = arg;
}

void SeperateTree::SetTypeList( std::vector<int>& arg)
{
    m_typeList = arg;
}
