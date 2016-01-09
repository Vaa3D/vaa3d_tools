#ifndef TREE_H
#define TREE_H
#include "basetypes.h"
#include "ineurondataobject.h"
#include <vector>
#include <memory>

//struct NeuronNode{
//    int ID, PID;
//    double x,y,z,r,v;
//};

//struct Neurite{
//    int ID;
//    int headConnect;
//    int tailConnect;
//    std::vector<NeuronNode> m_Node;
//};
//typedef std::shared_ptr<Neurite> NeuritePointer;

//class Tree : public INeuronDataObject
//{
//public:
//    Tree(INeuronProcessObject *p = nullptr);
//    ~Tree();
//    bool IsEmpty()const;
//    Neurite& GetNeurite(int);
//    const Neurite& GetNeurite(int)const;
//    int size()const{return m_Tree.size();}
//    void clear(){m_Tree.clear();}
//    void SetTree(std::vector<VectorVec5d>&);
////    void SetProcessObject(INeuronProcessObject*);
////    void ReleaseProcessObject();
//private:
//    std::vector<Neurite> m_Tree;
//};

//typedef std::shared_ptr<Tree> TreePointer;

class TreeCurve : public INeuronDataObject
{
public:
    TreeCurve(INeuronProcessObject *p = 0);
    ~TreeCurve();
    bool IsEmpty()const;
    //void SetProcessObject(INeuronProcessObject*);
    //void ReleaseProcessObject();
    void Swap(std::vector<VectorVec5d>&);
    void SetCurve(std::vector<VectorVec5d>&);
    void Clear(){m_Curve.clear();}
    int size()const{return m_Curve.size();}
    const std::vector<VectorVec5d>& GetCurve() const{return m_Curve;}
    std::vector<VectorVec5d>& GetCurve(){return m_Curve;}
private:
    std::vector<VectorVec5d> m_Curve;
};
#ifdef _WIN32
#define nullptr 0 
typedef std::tr1::shared_ptr<TreeCurve> TreeCurvePointer;
typedef std::tr1::shared_ptr<const TreeCurve> CTreeCurvePointer;
#else
typedef std::shared_ptr<TreeCurve> TreeCurvePointer;
typedef std::shared_ptr<const TreeCurve> CTreeCurvePointer;
#endif

class TreeConnect : public INeuronDataObject
{
public:
    TreeConnect(INeuronProcessObject *p = nullptr);
    ~TreeConnect();
    bool IsEmpty()const;
    //void SetProcessObject(INeuronProcessObject*);
    //void ReleaseProcessObject();
    void Swap(VectorMat2i&);
    void SetConnect(const VectorMat2i&);
    void Clear(){m_Connect.clear();}
    int size()const{return m_Connect.size();}
    const VectorMat2i& GetConnect() const{return m_Connect;}
private:
    VectorMat2i m_Connect;
};
#ifdef _WIN32
typedef std::tr1::shared_ptr<TreeConnect> TreeConnectPointer;
typedef std::tr1::shared_ptr<const TreeConnect> CTreeConnectPointer;
#else
typedef std::shared_ptr<TreeConnect> TreeConnectPointer;
typedef std::shared_ptr<const TreeConnect> CTreeConnectPointer;
#endif

class SeperateTree : public INeuronDataObject
{
public:
    SeperateTree(INeuronProcessObject *p = nullptr);
    ~SeperateTree();
    bool IsEmpty()const;
    //void SetProcessObject(INeuronProcessObject*);
    //void ReleaseProcessObject();
    void Swap(std::vector<std::vector<VectorVec5d> >&);
    void SetTree(std::vector<std::vector<VectorVec5d> >&);
    void SetTypeList( std::vector<int>&);
    void Clear(){m_Tree.clear();}
    int size()const{return m_Tree.size();}
    const std::vector<std::vector<VectorVec5d> >& GetTree() const{return m_Tree;}
    std::vector<std::vector<VectorVec5d> >& GetTree() {return m_Tree;}
    const std::vector<int>& GetTypeList()const {return m_typeList;}
    std::vector<int>& GetTypeList() {return m_typeList;}
private:
    std::vector<std::vector<VectorVec5d> >  m_Tree;
    std::vector<int> m_typeList;
};
#ifdef _WIN32
typedef std::tr1::shared_ptr<SeperateTree> SeperateTreePointer;
typedef std::tr1::shared_ptr<const SeperateTree> CSeperateTreePointer;
#else
typedef std::shared_ptr<SeperateTree> SeperateTreePointer;
typedef std::shared_ptr<const SeperateTree> CSeperateTreePointer;
#endif

#endif // TREE_H
