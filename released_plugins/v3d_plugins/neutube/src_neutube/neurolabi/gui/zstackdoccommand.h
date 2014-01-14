#ifndef ZSTACKDOCCOMMAND_H
#define ZSTACKDOCCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include "swctreenode.h"
#include "neutube.h"
#include "zswcpath.h"
#include <QMap>

class ZSwcTree;
class ZLocsegChain;
class ZObject3d;
class ZLocsegChainConn;
class ZPunctum;
class ZProgrogressReporter;
class ZStack;
class ZStroke2d;
class ZStackDoc;

namespace ZStackDocCommand {
namespace SwcEdit {
class TranslateRoot : public QUndoCommand
{
public:
  TranslateRoot(ZStackDoc *doc, double x, double y, double z,
                QUndoCommand *parent = NULL);
  virtual ~TranslateRoot();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  QList<ZSwcTree*> m_swcList;
  double m_x;
  double m_y;
  double m_z;
};

class Rescale : public QUndoCommand
{
public:
  Rescale(ZStackDoc *doc, double scaleX, double scaleY, double scaleZ,
            QUndoCommand *parent = NULL);
  Rescale(ZStackDoc *doc, double srcPixelPerUmXY, double srcPixelPerUmZ,
            double dstPixelPerUmXY, double dstPixelPerUmZ,
            QUndoCommand *parent = 0);
  virtual ~Rescale();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  QList<ZSwcTree*> m_swcList;
  double m_scaleX;
  double m_scaleY;
  double m_scaleZ;
};

class RescaleRadius : public QUndoCommand
{
public:
  RescaleRadius(ZStackDoc *doc, double scale, int startdepth,
                  int enddepth, QUndoCommand *parent = NULL);
  virtual ~RescaleRadius();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  QList<ZSwcTree*> m_swcList;
  double m_scale;
  int m_startdepth;
  int m_enddepth;
};

class ReduceNodeNumber : public QUndoCommand
{
public:
  ReduceNodeNumber(ZStackDoc *doc, double lengthThre, QUndoCommand *parent = NULL);
  virtual ~ReduceNodeNumber();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  QList<ZSwcTree*> m_swcList;
  double m_lengthThre;
};

class CompositeCommand : public QUndoCommand
{
public:
  CompositeCommand(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~CompositeCommand();

  void redo();
  void undo();
protected:
  ZStackDoc *m_doc;
};

class AddSwc : public QUndoCommand
{
public:
  AddSwc(ZStackDoc *doc, ZSwcTree *tree, QUndoCommand *parent = NULL);
  virtual ~AddSwc();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  ZSwcTree *m_tree;
  bool m_isInDoc;
};

class AddSwcNode : public QUndoCommand
{
public:
  AddSwcNode(ZStackDoc *doc, Swc_Tree_Node* tn, QUndoCommand *parent = NULL);
  virtual ~AddSwcNode();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  ZSwcTree* m_tree;
  bool m_treeInDoc;
  static int m_index;  // used to generate unique source for each new swc tree
};

class ExtendSwcNode : public QUndoCommand
{
public:
  ExtendSwcNode(ZStackDoc *doc, Swc_Tree_Node* node, Swc_Tree_Node* pnode,
                QUndoCommand *parent = NULL);
  virtual ~ExtendSwcNode();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  Swc_Tree_Node *m_parentNode;
  bool m_nodeInDoc;
};

class MergeSwcNode : public CompositeCommand
{
public:
  MergeSwcNode(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~MergeSwcNode();
};

class ChangeSwcNodeGeometry : public QUndoCommand
{
public:
  ChangeSwcNodeGeometry(ZStackDoc *doc, Swc_Tree_Node* node, double x, double y,
                        double z, double r, QUndoCommand *parent = NULL);
  virtual ~ChangeSwcNodeGeometry();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  double m_x;
  double m_y;
  double m_z;
  double m_r;
  double m_backupX;
  double m_backupY;
  double m_backupZ;
  double m_backupR;
};

class ChangeSwcNodeZ : public QUndoCommand
{
public:
  ChangeSwcNodeZ(ZStackDoc *doc, Swc_Tree_Node* node, double z,
                QUndoCommand *parent = NULL);
  virtual ~ChangeSwcNodeZ();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  double m_z;
  double m_backup;
};

class ChangeSwcNode : public QUndoCommand
{
public:
  ChangeSwcNode(ZStackDoc *doc, Swc_Tree_Node* node,
                const Swc_Tree_Node &newNode, QUndoCommand *parent = NULL);
  virtual ~ChangeSwcNode();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  Swc_Tree_Node m_backup;
  Swc_Tree_Node m_newNode;
};

class DeleteSwcNode : public QUndoCommand
{
public:
  DeleteSwcNode(ZStackDoc *doc, Swc_Tree_Node* node, Swc_Tree_Node *root,
                QUndoCommand *parent = NULL);
  virtual ~DeleteSwcNode();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  Swc_Tree_Node *m_root;
  Swc_Tree_Node m_backup;
  Swc_Tree_Node *m_prevSibling;
  Swc_Tree_Node *m_lastChild;
  bool m_nodeInDoc;
};

class SetParent : public QUndoCommand
{
public:
  SetParent(ZStackDoc *doc, Swc_Tree_Node *node, Swc_Tree_Node *parentNode,
               QUndoCommand *parent = NULL);
  virtual ~SetParent();

  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  Swc_Tree_Node *m_newParent;
  Swc_Tree_Node *m_oldParent;
  Swc_Tree_Node *m_prevSibling;
};

class SetSwcNodeSeletion : public QUndoCommand
{
public:
  SetSwcNodeSeletion(ZStackDoc *doc, const std::set<Swc_Tree_Node*> nodeSet,
                     QUndoCommand *parent = NULL);
  virtual ~SetSwcNodeSeletion();

  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  std::set<Swc_Tree_Node*> m_nodeSet;
  std::set<Swc_Tree_Node*> m_oldNodeSet;
};

class RemoveSubtree : public CompositeCommand
{
public:
  RemoveSubtree(ZStackDoc *doc, Swc_Tree_Node *node, QUndoCommand *parent = NULL);
  virtual ~RemoveSubtree();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
};

class SwcTreeLabeTraceMask : public QUndoCommand
{
public:
  SwcTreeLabeTraceMask(ZStackDoc *doc, Swc_Tree *tree, QUndoCommand *parent = NULL);
  virtual ~SwcTreeLabeTraceMask();

  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree *m_tree;
};

class SwcPathLabeTraceMask : public QUndoCommand
{
public:
  SwcPathLabeTraceMask(ZStackDoc *doc, const ZSwcPath& branch,
                       QUndoCommand *parent = NULL);
  virtual ~SwcPathLabeTraceMask();

  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  ZSwcPath m_branch;
};

class SetRoot : public CompositeCommand
{
public:
  SetRoot(ZStackDoc *doc, Swc_Tree_Node *tn, QUndoCommand *parent = NULL);
};

class ConnectSwcNode : public CompositeCommand
{
public:
  ConnectSwcNode(ZStackDoc *doc, QUndoCommand *parent = NULL);
};

class RemoveSwc : public QUndoCommand
{
public:
  RemoveSwc(ZStackDoc *doc, ZSwcTree *tree, QUndoCommand *parent = NULL);
  ~RemoveSwc();
  void redo();
  void undo();

private:
  ZStackDoc *m_doc;
  ZSwcTree *m_tree;
  bool m_isInDoc;
};

class RemoveEmptyTree : public QUndoCommand
{
public:
  RemoveEmptyTree(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~RemoveEmptyTree();
  void redo();
  void undo();

private:
  ZStackDoc *m_doc;
  std::set<ZSwcTree*> m_emptyTreeSet;
};

class BreakForest : public CompositeCommand
{
public:
  BreakForest(ZStackDoc *doc, QUndoCommand *parent = NULL);
};

class GroupSwc : public CompositeCommand
{
public:
  GroupSwc(ZStackDoc *doc, QUndoCommand *parent = NULL);
};


#if 0
class TraceSwcBranch : public QUndoCommand
{
public:
  TraceSwcBranch(ZStackDoc *doc, double x, double y, double z, int c,
                 QUndoCommand *parent = NULL);
  virtual ~TraceSwcBranch();

  void redo();
  void undo();

private:
  ZStackDoc *m_doc;
  double m_x;
  double m_y;
  double m_z;
  int m_c;
  ZSwcTree *m_tree;
  bool m_isTreeInDoc;
};
#endif

}

namespace ObjectEdit {
class RemoveSelected : public QUndoCommand
{
public:
  RemoveSelected(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~RemoveSelected();

  void undo();
  void redo();

private:
  ZStackDoc *doc;
  QList<ZLocsegChain*> m_chainList;
  QList<ZSwcTree*> m_swcList;
  QList<ZLocsegChainConn*> m_connList;
  QList<ZObject3d*> m_obj3dList;
  QList<ZPunctum*> m_punctaList;
  QList<ZStroke2d*> m_strokeList;
  QMap<Swc_Tree_Node*, Swc_Tree_Node*> m_deletedNodes;
  QMap<Swc_Tree_Node*, Swc_Tree_Node*> m_childrenOfDeletedNodes;
  QList<ZSwcTree*> m_newTreesAfterDeleteNodes;
  QList<ZSwcTree*> m_emptyTreeAfterDeleteNodes;
};

class MoveSelected : public QUndoCommand
{
  ZStackDoc *m_doc;
  std::set<ZSwcTree*> m_swcList;
  std::set<ZPunctum*> m_punctaList;
  std::set<Swc_Tree_Node*> m_swcNodeList;
  double m_x;
  double m_y;
  double m_z;
  bool m_swcMoved;
  bool m_punctaMoved;
  double m_swcScaleX;
  double m_swcScaleY;
  double m_swcScaleZ;
  double m_punctaScaleX;
  double m_punctaScaleY;
  double m_punctaScaleZ;
public:
  MoveSelected(ZStackDoc *doc, double x, double y,
               double z, QUndoCommand *parent = NULL);
  virtual ~MoveSelected();
  void setSwcCoordScale(double x, double y, double z);
  void setPunctaCoordScale(double x, double y, double z);
  virtual int id() const { return 1; }
  virtual bool mergeWith(const QUndoCommand *other);
  void undo();
  void redo();
};

class AddObject : public QUndoCommand
{
public:
  AddObject(ZStackDoc *doc, ZDocumentable *obj, NeuTube::EDocumentableType type,
            QUndoCommand *parent = NULL);
  ~AddObject();
  void redo();
  void undo();

private:
  ZStackDoc *m_doc;
  ZDocumentable *m_obj;
  NeuTube::EDocumentableType m_type;
  bool m_isInDoc;
};
}

namespace TubeEdit {
class RemoveSmall : public QUndoCommand
{
public:
  RemoveSmall(ZStackDoc *doc, double thre, QUndoCommand *parent = NULL);
  virtual ~RemoveSmall();

  void undo();    // conn information may be lost after undo
  void redo();

private:
  ZStackDoc *m_doc;
  double m_thre;
  QList<ZLocsegChain*> m_chainList;
};

class RemoveSelected : public QUndoCommand
{
public:
  RemoveSelected(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~RemoveSelected();

  void undo();    // conn information may be lost after undo
  void redo();

private:
  ZStackDoc *m_doc;
  QList<ZLocsegChain*> m_chainList;
};

class Trace : public QUndoCommand
{
public:
  Trace(ZStackDoc *doc, int x, int y, int z, QUndoCommand *parent = NULL);
  Trace(ZStackDoc *doc, int x, int y, int z, int c, QUndoCommand *parent = NULL);
  void undo();
  void redo();
private:
  ZStackDoc *m_doc;
  int m_x;
  int m_y;
  int m_z;
  int m_c;
  ZLocsegChain* m_chain;
};

class CutSegment : public QUndoCommand
{
public:
  CutSegment(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~CutSegment();

  void undo();
  void redo();
private:
  ZStackDoc *m_doc;
  QList<ZLocsegChain*> m_oldChainList;
  QList<ZLocsegChain*> m_newChainList;
};

class BreakChain : public QUndoCommand
{
public:
  BreakChain(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~BreakChain();

  void undo();
  void redo();
private:
  ZStackDoc *m_doc;
  QList<ZLocsegChain*> m_oldChainList;
  QList<ZLocsegChain*> m_newChainList;
};

class AutoTrace : public QUndoCommand
{
public:
  AutoTrace(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~AutoTrace();

  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  QList<ZLocsegChain*> m_chainList;
  QList<ZSwcTree*> m_swcList;
  QList<ZLocsegChainConn*> m_connList;
  QList<ZObject3d*> m_obj3dList;
  QList<ZPunctum*> m_punctaList;
};

class AutoTraceAxon : public QUndoCommand
{
public:
  AutoTraceAxon(ZStackDoc *m_doc, QUndoCommand *parent = NULL);
  virtual ~AutoTraceAxon();

  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  QList<ZLocsegChain*> m_chainList;
  QList<ZSwcTree*> m_swcList;
  QList<ZLocsegChainConn*> m_connList;
  QList<ZObject3d*> m_obj3dList;
  QList<ZPunctum*> m_punctaList;
};
}

namespace StrokeEdit {
class AddStroke : public QUndoCommand
{
public:
  AddStroke(ZStackDoc *doc, ZStroke2d *stroke, QUndoCommand *parent = NULL);
  virtual ~AddStroke();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  ZStroke2d *m_stroke;
  bool m_isInDoc;
};

class RemoveTopStroke : public QUndoCommand
{
public:
  RemoveTopStroke(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~RemoveTopStroke();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  ZStroke2d *m_stroke;
  bool m_isInDoc;
};
}

namespace StackProcess {
class Binarize : public QUndoCommand
{
  ZStackDoc *doc;
  ZStack *zstack;
  int thre;
  bool success;
public:
  Binarize(ZStackDoc *doc, int thre, QUndoCommand *parent = NULL);
  virtual ~Binarize();
  void undo();
  void redo();
};

class BwSolid : public QUndoCommand
{
  ZStackDoc *doc;
  ZStack *zstack;
  bool success;
public:
  BwSolid(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~BwSolid();
  void undo();
  void redo();
};

class EnhanceLine : public QUndoCommand
{
  ZStackDoc *doc;
  ZStack *zstack;
  bool success;
public:
  EnhanceLine(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~EnhanceLine();
  void undo();
  void redo();
};

class Watershed : public QUndoCommand
{
  ZStackDoc *doc;
  ZStack *zstack;
  bool success;
public:
  Watershed(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~Watershed();
  void undo();
  void redo();
};
}

}

#endif // ZSTACKDOCCOMMAND_H
