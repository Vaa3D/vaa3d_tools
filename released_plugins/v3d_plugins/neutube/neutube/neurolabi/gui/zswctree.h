/**@file zswctree.h
 * @brief Swc tree
 * @author Ting Zhao
 */

#ifndef _ZSWCTREE_H_
#define _ZSWCTREE_H_

#include "zqtheader.h"

#include <vector>
#include <string>
#include <set>

#include "tz_swc_tree.h"
#include "zdocumentable.h"
#include "zstackdrawable.h"
#include "zswcexportable.h"
#include "zpoint.h"
#include "zswcpath.h"
#include "zcuboid.h"

class ZSwcForest;
class ZSwcBranch;
class ZSwcTrunkAnalyzer;

//! SWC tree class
/*!
 *It is a c++ wrapper of the Swc_Tree structure.
 *
 *The definition of the SWC file format can be found at
 *  <a href="http://research.mssm.edu/cnic/swc.html">CNIC website</a>
 *
 *Briefly, the basic element of an SWC tree is a node with the following
 *properties: ID, type, x, y, z, radius, parent ID. ZSwcTree provides interfaces
 *for iterating through the nodes with several modes.
 *In neuTube, any node with negative ID will be treated as virtual. A virtual
 *node is not considered as a part of the tree model. It is introduced to
 *facilitate hosting multiple trees or handling specific data structure such as
 *binary trees. All normal operations should assume that only the root node can
 *be virtual. The children of a virtual root are called regular roots, because
 *each of them is the root of a real SWC tree.
*/

class ZSwcTree :
    public ZDocumentable, public ZStackDrawable, public ZSwcExportable {
public:
  //! Action of cleaning the existing data
  enum ESetDataOption {
    CLEAN_ALL, /*!< Clean all associated memory */
    FREE_WRAPPER, /*!< Free the wrapper pointer only */
    LEAVE_ALONE /*!< Don nothing */
  };

  //! Selection mode
  enum ESelectionMode {
    SWC_NODE, /*!< Single node */
    WHOLE_TREE, /*!< Whole tree */
    SWC_BRANCH /*!< Single branch */
  };

  /** @name Constructors
   */
  ///@{

  /*!
   * \brief Copy constructor.
   *
   * It performs deep copy of a tree, i.e. there is no memory sharing between
   * the constructed object and \src . Intermediate results will not be copied.
   *
   * \param src Original object.
   */
  ZSwcTree(const ZSwcTree &src);

  /*!
   * \brief Default constructor.
   */
  ZSwcTree();
  ///@}

  /*!
   * \brief Deconstructor.
   */
  ~ZSwcTree();

  virtual const std::string& className() const;

  friend void swap(ZSwcTree& first, ZSwcTree& second);
  ZSwcTree& operator=(const ZSwcTree &other);

public:
  /** @name Set data.
   * Load tree data into an object.
   */
  ///@{
  /*!
   * \brief Set the data to \a tree.
   *
   * \a tree will be owned by the object.
   *
   * \param tree The data.
   * \param option Optioni of handling existing data.
   */
  void setData(Swc_Tree *tree, ESetDataOption option = CLEAN_ALL);

  /*!
   * \brief Set the data from a node.
   *
   *
   *
   * \param tree
   * \param option
   */
  void setDataFromNode(Swc_Tree_Node *tree, ESetDataOption option = CLEAN_ALL);
  void setDataFromNodeRoot(Swc_Tree_Node *tree,
                           ESetDataOption option = CLEAN_ALL);
  ///@}

  ZSwcTree *clone() const;
  Swc_Tree *cloneData() const;
  //Swc_Tree* copyData() const;  //copy from m_tree

  inline bool hasData() const { return m_tree != NULL; }
  inline Swc_Tree* data() const { return m_tree; }
  inline Swc_Tree_Node* root() const {
    return (m_tree != NULL) ? m_tree->root : NULL;
  }

  /*!
   * \brief Test if a tree is empty.
   * \return true iff the tree has no regular root.
   */
  bool isEmpty() const;

  bool hasRegularNode();

  bool isValid();

public:

  virtual void display(QPainter &painter, int z = 0, Display_Style option = NORMAL) const;

  /*!
   * \brief save Save swc
   * \param filePath
   */
  virtual void save(const char *filePath);

  virtual void load(const char *filePath);

  void save(const std::string &filePath);
  void load(const std::string &filePath);

  virtual int swcFprint(FILE *fp, int start_id = 0, int parent_id = -1,
                        double z_scale = 1.0);
  virtual void swcExport(const char *filePath);

  void print(int iterOption = SWC_TREE_ITERATOR_DEPTH_FIRST);

  inline std::string source() const { return m_source; }

  // convert swc to locsegchains..., return next .tb file idx
  int saveAsLocsegChains(const char *prefix, int startNum);

  bool contains(Swc_Tree_Node *tn) const;

  int regularRootNumber();
  void addRegularRoot(Swc_Tree_Node *tn);

  Swc_Tree_Node *maxLabelNode();

  inline void setSource(std::string source) { m_source = source; }

  enum EComponent {
    DEPTH_FIRST_ARRAY, BREADTH_FIRST_ARRAY, LEAF_ARRAY, TERMINAL_ARRAY,
    BRANCH_POINT_ARRAY, Z_SORTED_ARRAY, ALL_COMPONENT
  };

  bool isDeprecated(EComponent component) const;
  void deprecateDependent(EComponent component);
  void deprecate(EComponent component);

public:
  int size();
  int size(Swc_Tree_Node *start);

  /** @name SWC iterator routines
   *
   *  It is necessary to call updateIterator first before using
   *  begin() and next() to perform iteration. The iterating options are:
   *   SWC_TREE_ITERATOR_DEPTH_FIRST: depth first iteration
   *   SWC_TREE_ITERATOR_BREADTH_FIRST: breadth first iteration
   *   SWC_TREE_ITERATOR_LEAF: iterate through leaves
   *   SWC_TREE_ITERATOR_BRANCH_POINT: iterate through branch points
   *   SWC_TREE_ITERATOR_NO_UPDATE : no update, reset the internal iterator
   *     point to the begin
   *   SWC_TREE_ITERATOR_VOID: no update and no change on the current iterator
   *   SWC_TREE_ITERATOR_REVERSE: reverse the current iterator
   *
   *  An example of iterating through nodes:
   *  @code
   *      ZSwcTree tree;
   *      ... //Suppose the tree is loaded with some real nodes
   *      tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST); //Depth first
   *      for (Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = next()) {
   *          std::cout << "Swc node: " << SwcTreeNode::id(tn) << std::endl;
   *      }
   *  @endcode
   */
  ///@{
  /*!
   * \brief Update iterator
   * \param option Iterator option.
   * \param indexing Indexing the nodes or not.
   * \return
   */
  int updateIterator(int option = SWC_TREE_ITERATOR_DEPTH_FIRST,
                     BOOL indexing = FALSE) const;
  int updateIterator(int option, const std::set<Swc_Tree_Node*> &blocker,
                     BOOL indexing = FALSE) const;
  /*!
   * \brief Update iterator with a starting node.
   *
   * When \a start is specified, the iteration will be on the subtree started
   * from \a start.
   *
   * \param option Iterator option.
   * \param start The starting node.
   * \param indexing Indexing the nodes or not.
   * \return
   */
  int updateIterator(int option, Swc_Tree_Node *start, BOOL indexing) const;

  /*!
   * \brief Update iterator with a start and blocked branches.
   *
   * When the iterator enouncters a node in <blocker>, it would stop going
   * forward along the corresponding pathway.
   * It does not matter whether <start> is in <blocker> or not.
   *
   * \param option Iterator option.
   * \param start The starting node.
   * \param blocker The set of blocking nodes.
   * \param indexing Add indices to the nodes or not.
   * \return The number of nodes.
   */
  int updateIterator(int option, Swc_Tree_Node *start,
                     const std::set<Swc_Tree_Node*> &blocker,
                     BOOL indexing = FALSE) const;

  inline Swc_Tree_Node* begin() {
    return const_cast<const ZSwcTree*>(this)->begin();
  }

  inline Swc_Tree_Node* begin() const {
    if (m_tree == NULL) {
      return NULL;
    }

    if (m_tree->begin != NULL) {
      m_tree->iterator = m_tree->begin->next;
    }

    return m_tree->begin;
  }

  inline Swc_Tree_Node* end() { return NULL; }
  inline Swc_Tree_Node* end() const { return NULL; }

  inline Swc_Tree_Node* next() { return Swc_Tree_Next(m_tree); }
  inline Swc_Tree_Node* next() const { return Swc_Tree_Next(m_tree); }
  ///@}

  bool labelBranch(double x, double y, double z, double thre);
  void labelBranchLevel(int rootType = -1);
  void labelBranchLevelFromLeaf();

  /*!
   * \brief Get bound box of the tree
   *
   * \param corner The array to store the bound box. The first corner is
   *        (\a corner[0], \a corner[1], corner[2]) and the last corner is
   *        (\a corner[3], \a corner[4], corner[5]).
   */
  void boundBox(double *corner) const;

  /*!
   * \brief Get bound box of the tree
   *
   * \return The bound box.
   */
  ZCuboid boundBox() const;

  static ZSwcTree* createCuboidSwc(const ZCuboid &box);
  ZSwcTree* createBoundBoxSwc(double margin = 0.0);

  Swc_Tree_Node* hitTest(double x, double y, double z);

  /*!
   * \brief Plane hit test
   *
   * It tests if a point (\a x, \a y) hits the X-Y projection of the tree.
   *
   * \param x X coodinate of the hit point
   * \param y Y coodinate of the hit point
   * \return  Returns the closest node to (\a x, \a y) if there is hit.
   *          If there is no hit, it returns NULL.
   */
  Swc_Tree_Node* hitTest(double x, double y);

  void toSvgFile(const char *filePath);

  // move soma (first root) to new location
  void translateRootTo(double x, double y, double z);
  // rescale location and radius
  void rescale(double scaleX, double scaleY, double scaleZ);
  void rescale(double srcPixelPerUmXY, double srcPixelPerUmZ,
               double dstPixelPerUmXY, double dstPixelPerUmZ);
  // rescale radius of nodes in certain depth range, startdepth <= depth of node < enddepth
  // 0 is the depth of roots. enddepth can be -1 which means max depth
  void rescaleRadius(double scale, int startdepth, int enddepth);

  //r2 = r * scale + dr
  void changeRadius(double dr, double scale);

  int swcNodeDepth(Swc_Tree_Node *tn);
  // reduce node number, similar to Swc_Tree_Merge_Close_Node, but only merge Continuation node
  void reduceNodeNumber(double lengthThre);

  /*!
   * \brief Compute distance between two trees
   *
   * It computes shortest distance between the terminal centers of the object
   * and the surface of \a tree.
   *
   * \param tree The partner tree for distance calculation
   * \param source Source node closest to \a tree
   * \param A node in \a tree closest to the tree
   */
  double distanceTo(ZSwcTree *tree, Swc_Tree_Node **source = NULL,
                    Swc_Tree_Node **target = NULL);

  double distanceTo(double x, double y, double z, double zScale, Swc_Tree_Node **node = NULL) const;

  ZSwcForest* toSwcTreeArray();

  void resortId();

  void flipY(double height);

  void removeRedundantBranch(double redundacyThreshold);
  double computeRedundancy(Swc_Tree_Node *leaf);
  void removeBranch(Swc_Tree_Node *tn);

  std::vector<int> shollAnalysis(double rStart, double rEnd, double rStep,
                                 ZPoint center);

  Swc_Tree_Node *firstRegularRoot() const;
  Swc_Tree_Node *firstLeaf();

  std::vector<Swc_Tree_Node*> terminalArray();

  ZSwcBranch *extractBranch(int beginId, int endId);
  ZSwcBranch *extractBranch(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2);
  ZSwcBranch *extractBranch(int setLabel);
  ZSwcBranch *extractLongestBranch();
  ZSwcBranch *extractFurthestBranch();

  std::vector<Swc_Tree_Node*> extractLeaf(Swc_Tree_Node *start);

  Swc_Tree_Node* queryNode(int id,
                           int iterOption = SWC_TREE_ITERATOR_DEPTH_FIRST);
  Swc_Tree_Node* queryNode(const ZPoint &pt);

  ZPoint somaCenter();

  std::vector<double> computeAllContinuationAngle(bool rotating = false);
  std::vector<double> computeAllBranchingAngle();

  void merge(Swc_Tree *tree, bool freeInput = false);
  void merge(ZSwcTree *tree, bool freeInput = false);

  void setLabel(int v);
  void setType(int type);

  void translate(const ZPoint& offset);
  void translate(double x, double y, double z);
  void scale(double x, double y, double z);
  //Rotate swc tree around a point
  void rotate(double theta, double psi, const ZPoint& center);

  void resample(double step);

  Swc_Tree_Node* removeRandomBranch();

  /*!
   * \brief Label a subtree
   * \param tn the root of the subtree. It must be a node in the current tree,
   * otherwise the behavior is undefined.
   */
  void labelSubtree(Swc_Tree_Node *tn, int label);

  /*!
   * \brief Add a label to the subtree
   * the new label is the current label plus \a label.
   */
  void addLabelSubtree(Swc_Tree_Node *tn, int label);

  void labelTrunk(int flag, int setLabel, Swc_Tree_Node *start);
  void labelTrunkLevel(ZSwcTrunkAnalyzer *trunkAnalyzer);

  int regularDepth();

  ZSwcPath mainTrunk(ZSwcTrunkAnalyzer *trunkAnalyzer);
  ZSwcPath subTrunk(Swc_Tree_Node *start, int setLabel);
  ZSwcPath subTrunk(Swc_Tree_Node *start,
                    const std::set<Swc_Tree_Node*> &blocker,
                    ZSwcTrunkAnalyzer *trunkAnalyzer);

  std::vector<ZSwcPath> getBranchArray();

  void labelBusyLevel();

  void setTypeByLabel();
  void moveToSurface(double *x, double *y, double *z);
  void moveToSurface(ZPoint *pt);

  inline void deactivateIterator() { m_iteratorReady = true; }
  inline void activateIterator() { m_iteratorReady = false; }

  void forceVirtualRoot();

  void setBranchSizeWeight();

  double length();
  double length(int type);

  /*!
   * \brief Compute back trace length and store the result as node weight
   *
   * \return overall length
   */
  double computeBackTraceLength();

  std::set<int> typeSet();

  bool hasGoodSourceName();

  std::vector<Swc_Tree_Node*> toSwcTreeNodeArray(bool includingVirtual = true);

  enum EIteratorOption {
    DEPTH_FIRST_ITERATOR, BREADTH_FIRST_ITERATOR, LEAF_ITERATOR, TERMINAL_ITERATOR,
    BRANCH_POINT_ITERATOR, Z_SORT_ITERATOR
  };

  const std::vector<Swc_Tree_Node*>& getSwcTreeNodeArray(
      EIteratorOption iteratorOption = DEPTH_FIRST_ITERATOR) const;

  void labelStack(Stack *stack);

public: //static functions
  static std::vector<ZSwcTree*> loadTreeArray(std::string dirPath);
  static Swc_Tree_Node* makeArrow(const ZPoint &startPos, double startSize,
                                  int startType,
                                  const ZPoint &endPos, double endSize,
                                  int endType, bool addBreak = true);

  static ZSwcTree* generateRandomSwcTree(int n, double branchingProb,
                                         double contAngleMu,
                                         double contAngleSigma,
                                         double branchAngleMu,
                                         double branchAngleSigma);

private:
  Swc_Tree *m_tree;
  std::string m_source;
  mutable bool m_iteratorReady; /* When this option is on, any iterator option changing
                           internal linked list
                           is turned off and SWC_TREE_ITERATOR_NO_UPDATE is
                           applied instead */

  mutable std::vector<Swc_Tree_Node*> m_depthFirstArray;
  mutable std::vector<Swc_Tree_Node*> m_breadthFirstArray;
  mutable std::vector<Swc_Tree_Node*> m_leafArray;
  mutable std::vector<Swc_Tree_Node*> m_terminalArray;
  mutable std::vector<Swc_Tree_Node*> m_branchPointArray;
  mutable std::vector<Swc_Tree_Node*> m_zSortedArray;
};

#endif /* _ZSWCTREE_H_ */
