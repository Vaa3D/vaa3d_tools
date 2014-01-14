#ifndef SWCTREENODE_H
#define SWCTREENODE_H

#include <string>
#include <vector>
#include <set>
#include <map>

#include "tz_swc_tree.h"
#include "zpoint.h"
#include "zweightedpointarray.h"
#include "neutube.h"
#include "zcuboid.h"

typedef int (*Swc_Tree_Node_Compare) (Swc_Tree_Node* lhs, Swc_Tree_Node *rhs);
typedef bool (*Swc_Tree_Node_Compare_B) (Swc_Tree_Node* lhs, Swc_Tree_Node *rhs);

static std::vector<std::string> SwcTreeNodeClipboard;

//A wrapper for Swc_Tree_Node
namespace SwcTreeNode {

//Constructors

/*!
 * \brief Create a regular swc node
 * \return A pointer of Swc_Tree_Node, which must be freed by kill()
 */
Swc_Tree_Node* makePointer();

/*!
 * \brief Create a pointer of Swc_Tree_Node
 * \param id ID of the node
 * \param type Type of the node
 * \param pos Center position of the node
 * \param radius Radius of the node
 * \param parentId Parent ID of the node
 * \return A pointer of Swc_Tree_Node, which must be freed by kill()
 */
Swc_Tree_Node* makePointer(int id, int type, const ZPoint &pos, double radius,
                           int parentId);

/*!
 * \brief Create a pointer of Swc_Tree_Node
 * \param id ID of the node
 * \param type Type of the node
 * \param x X coordinate of the node center
 * \param y Y coordinate of the node cebter
 * \param z Z coordinate of the node center
 * \param radius Radius of the node
 * \param parentId Parent ID of the node
 * \return A pointer of Swc_Tree_Node, which must be freed by kill()
 */
Swc_Tree_Node* makePointer(int id, int type, double x, double y, double z,
                           double radius, int parentId);

/*!
 * \brief Create a pointer of Swc_Tree_Node with default id and type
 * \param x X coordinate of the node center
 * \param y Y coordinate of the node cebter
 * \param z Z coordinate of the node center
 * \param radius Radius of the node
 * \return A pointer of Swc_Tree_Node, which must be freed by kill()
 */
Swc_Tree_Node* makePointer(double x, double y, double z, double radius);

/*!
 * \brief Create a pointer of Swc_Tree_Node with default id and type
 * \param pos Center position of the node
 * \param radius Radius of the node
 * \return A pointer of Swc_Tree_Node, which must be freed by kill()
 */
Swc_Tree_Node* makePointer(const ZPoint &pos, double radius);

/*!
 * \brief Make a virtual node
 * \return Return the pointer of a virual Swc_Tree_Node
 */
Swc_Tree_Node* makeVirtualNode();

/*!
 * \brief Set a node to default properties
 * \param tn Input node.
 */
void setDefault(Swc_Tree_Node *tn);

//Attributes
double x(const Swc_Tree_Node *tn);
double y(const Swc_Tree_Node *tn);
double z(const Swc_Tree_Node *tn);
ZPoint pos(const Swc_Tree_Node *tn);
double radius(const Swc_Tree_Node *tn);
int id(const Swc_Tree_Node *tn);
int parentId(const Swc_Tree_Node *tn);
inline int type(const Swc_Tree_Node *tn) { return tn->node.type; }
int label(const Swc_Tree_Node *tn);
int index(const Swc_Tree_Node *tn);

/*!
 * \brief Get the lenght of a node
 *
 * The length of \a tn is defined as its central distance to its parent.
 * It returns 0.0 if it is a root.
 */
double length(const Swc_Tree_Node *tn);

inline double weight(const Swc_Tree_Node *tn) {
  return tn->weight;
}
inline void setWeight(Swc_Tree_Node *tn, double w) {
  if (tn != NULL) tn->weight = w;
}

inline void addWeight(Swc_Tree_Node *tn, double dw) {
  if (tn != NULL) tn->weight += dw;
}


inline double feature(const Swc_Tree_Node *tn) {
  return tn->feature;
}
inline void setFeature(Swc_Tree_Node *tn, double v) {
  tn->feature = v;
}

//Properties
bool isParentIdConsistent(const Swc_Tree_Node *tn);
bool isRegular(const Swc_Tree_Node *tn);
bool isVirtual(const Swc_Tree_Node *tn);

ZCuboid boundBox(const Swc_Tree_Node *tn);

//Structure properties
inline Swc_Tree_Node* firstChild(const Swc_Tree_Node *tn) {
  return (tn == NULL) ? NULL : tn->first_child;
}
inline Swc_Tree_Node* parent(const Swc_Tree_Node *tn) {
  return (tn == NULL) ? NULL : tn->parent;
}

//! Topological type of a node
enum ETopologicalType{
  BRANCH_POINT, /*!< Branch point*/
  TERMINAL, /*!< Terminal*/
  ROOT, /*!< Root*/
  LEAF, /*!< Leaf*/
  CONTINUATION /*!< A node that has exactly two neighbors*/
};

/*!
 * \brief Test a node if has a child
 * \param tn Input node.
 * \return true if \a tn has a child
 */
bool hasChild(const Swc_Tree_Node *tn);

/*!
 * \brief childNumber Child number of a node
 * \param tn Input node.
 * \return The number of children of \a tn.
 */
int childNumber(const Swc_Tree_Node *tn);

/*!
 * \brief minChildLabel Minimal child lable of a node
 * \param tn Input node.
 * \return The minimal label of the children of \a tn. 0 if \a tn has no child.
 */
int minChildLabel(const Swc_Tree_Node *tn);

bool isBranchPoint(const Swc_Tree_Node *tn);
bool isLeaf(const Swc_Tree_Node *tn);

/*!
 * \brief Test if a node is a terminal
 * \a tn is a terminal if
 *   1. it is regular;
 *   2. it is a leaf or
 *   3. it is a root and it has no more than one child
 */
bool isTerminal(const Swc_Tree_Node *tn);

int downstreamSize(Swc_Tree_Node *tn);
int downstreamSize(Swc_Tree_Node *tn, Swc_Tree_Node_Compare compfunc);
int singleTreeSize(Swc_Tree_Node *tn);

inline Swc_Tree_Node *nextSibling(Swc_Tree_Node *tn) {
  return (tn == NULL) ? NULL : tn->next_sibling;
}
Swc_Tree_Node *prevSibling(Swc_Tree_Node *tn);
Swc_Tree_Node *lastChild(Swc_Tree_Node *tn);
bool isContinuation(const Swc_Tree_Node *tn);

bool isRegularRoot(const Swc_Tree_Node *tn);
//return true iff tn is virtual or regular root
bool isRoot(const Swc_Tree_Node *tn);

Swc_Tree_Node* regularRoot(Swc_Tree_Node *tn);
Swc_Tree_Node* root(Swc_Tree_Node *tn);
Swc_Tree_Node* commonAncestor(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2);
const Swc_Tree_Node* commonAncestor(const Swc_Tree_Node *tn1,
                                    const Swc_Tree_Node *tn2);
bool isAncestor(const Swc_Tree_Node *ancestor, const Swc_Tree_Node *tn);

Swc_Tree_Node *thickestChild(Swc_Tree_Node *tn);

/*!
 * \brief Continuous descendent
 *
 * The target node is the first descendent >= minDist away
 * on the continuous path of \a tn.
 *
 * \param tn The source node.
 * \param minDist The minimal path distance of the target node
 * \return The descendent of the node or NULL if no node satifies the conditions.
 */
Swc_Tree_Node* continuousDescendent(const Swc_Tree_Node *tn, double minDist);

/*!
 * \brief Continuous ancestor
 *
 * The target node is the first ancestor >= minDist away
 * on the continuous path of \a tn.
 *
 * \param tn The source node.
 * \param minDist The minimal path distance of the target node
 * \return The ancestor of the node or NULL if no node satifies the conditions.
 */
Swc_Tree_Node* continuousAncestor(const Swc_Tree_Node *tn, double minDist);

//Returns true iff one node is the parent of the other.
//It returns false if either <tn1> or <tn2> is NULL.
bool isConnected(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2);

//Attribute modifiers
void setPos(Swc_Tree_Node *tn, double x, double y, double z);
inline void setX(Swc_Tree_Node *tn, double x) {
  tn->node.x = x;
}
inline void setY(Swc_Tree_Node *tn, double y) {
  tn->node.y = y;
}
inline void setZ(Swc_Tree_Node *tn, double z) {
  tn->node.z = z;
}

/*!
 * \brief Set radius of the node.
 *
 * The radius is set to 0 if \a < 0.
 */
void setRadius(Swc_Tree_Node *tn, double r);

/*!
 * \brief Change the radius of the node
 *
 * new radius = radius * \a scale + \a dr. There is a minimal radius constraint
 * on the change. The radius is set to \a MinimalRadius if the calculation
 * results in a smaller value than \a MinimalRadius.
 */
void changeRadius(Swc_Tree_Node *tn, double dr, double scale);

void setId(Swc_Tree_Node *tn, int id);
void setParentId(Swc_Tree_Node *tn, int parentId);
inline void setType(Swc_Tree_Node *tn, int type) { tn->node.type = type; }
inline void addType(Swc_Tree_Node *tn, int type) { tn->node.type += type; }
void setLabel(Swc_Tree_Node *tn, int label);
void addLabel(Swc_Tree_Node *tn, int label);
void copyProperty(const Swc_Tree_Node *src, Swc_Tree_Node *dst);
void toVirtual(Swc_Tree_Node *tn);
void setDownstreamType(Swc_Tree_Node *tn, int type);
void setUpstreamType(Swc_Tree_Node *tn, int type, Swc_Tree_Node *stop = NULL);
void translate(Swc_Tree_Node *tn, double dx, double dy, double dz);
void rotate(Swc_Tree_Node *tn, double theta, double psi, const ZPoint &center);
void rotate(Swc_Tree_Node *tn, double theta, double psi);

void interpolate(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2, double lambda,
                double *x, double *y, double *z, double *r);

//Elementary operations (do not validate structures)
enum EKnowingLink {
  FIRST_CHILD, PARENT, NEXT_SIBLING
};

/*!
 * \brief Set the link between two nodes
 *
 * setLink() with FIRST_CHILD link option set the first child field of \
 *
 * \param tn Source node
 * \param target Target node
 * \param link Linking option.
 */
void setLink(Swc_Tree_Node *tn, Swc_Tree_Node *target, EKnowingLink link);

//Structure modifiers
/*!
 * \brief Set a node as a root
 *
 * setAsRoot() sets \a tn to a root. If \a tn is regular, it becomes a regular
 * root after the function call. The parent of \a tn is the virtual node of
 * host tree of \a tn if applicable.
 *
 * \param Input node
 */
void setAsRoot(Swc_Tree_Node *tn);
void setParent(Swc_Tree_Node *tn, Swc_Tree_Node *parent);
void detachParent(Swc_Tree_Node *tn);
void adoptChildren(Swc_Tree_Node *newParent, Swc_Tree_Node *oldParent);
void mergeToParent(Swc_Tree_Node *tn);

//Destructors
//Delete <tn> and all its descendents
void killSubtree(Swc_Tree_Node *tn);

std::string toString(const Swc_Tree_Node *tn);

//Path routines
void setPathType(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2, int type);
void addPathType(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2, int type);
void addPathLabel(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2, int type);
double pathLength(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2);
/*!
 * \brief length of project path on the XY plane
 */
double planePathLength(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2);

enum EDistanceType {
  GEODESIC, EUCLIDEAN, EUCLIDEAN_SURFACE
};

/*!
 * \brief Distance between two swc nodes
 *
 * Calculates distance between \a tn1 and \a tn2. There are three definitions
 * of the distance specified by \a distType:
 *   SwcTreeNode::EUCLIDEAN - euclidean distance, defined as distance between
 *     the centers
 *   SwcTreeNode::EUCLIDEAN_SURFACE - surface distance, defined as euclidean
 *     distance minus the sum of radii of \a tn1 and \a tn2. It will be negative
 *     when \a tn1 and \a tn2 overlap.
 *   SwcTreeNode::GEODESIC - geodesic distance, defined as the length of the
 *     skeleton path from \a tn1 to \a tn2. It returns Infinity if there is no
 *     path between \a tn1 and \a tn2.
 *
 * Returns 0 if \a tn1 or \a tn2 is not a regular node.
 */
double distance(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2,
                EDistanceType distType = EUCLIDEAN);

/*!
 * \brief Distance between a node an point
 *
 * It supports two distance options:
 *   SwcTreeNode::EUCLIDEAN - distance from (\a x, \a y, \a z) to the center of
 *     \a tn.
 *   SwcTreeNode::EUCLIDEAN_SURFACE - distance from (\a x, \a y, \a z) to the
 *     surface of \a tn.
 *
 * It returns 0 for other options. It returns 0 when \a tn is not a regular
 *   node.
 */
double distance(const Swc_Tree_Node *tn, double x, double y, double z,
                EDistanceType distType = EUCLIDEAN);

Swc_Tree_Node*
furthestNode(Swc_Tree_Node *tn, EDistanceType distType = EUCLIDEAN);

int labelDifference(Swc_Tree_Node *lhs, Swc_Tree_Node *rhs);
ZPoint upStreamDirection(Swc_Tree_Node *tn, int n = 0);
ZPoint localDirection(const Swc_Tree_Node *tn, int extend = 1);
double localRadius(const Swc_Tree_Node *tn, int extend = 1);
ZWeightedPointArray localSegment(const Swc_Tree_Node *tn, int extend = 1);

double estimateRadius(const Swc_Tree_Node *tn, const Stack *stack,
                      NeuTube::EImageBackground bg);
bool fitSignal(Swc_Tree_Node *tn, const Stack *stack,
               NeuTube::EImageBackground bg);

//Node set
bool connect(const std::vector<Swc_Tree_Node*> &nodeArray);
bool connect(const std::set<Swc_Tree_Node*> &nodeSet);
Swc_Tree_Node* findClosestNode(const std::set<Swc_Tree_Node*> &nodeSet,
                               const Swc_Tree_Node *seeker);

/*!
 * \brief Get neighbors of a node
 *
 * Only regular nodes are included.
 */
std::vector<Swc_Tree_Node*> neighborArray(const Swc_Tree_Node *tn);

ZPoint centroid(const std::set<Swc_Tree_Node*> &nodeSet);
double maxRadius(const std::set<Swc_Tree_Node*> &nodeSet);
ZCuboid boundBox(const std::set<Swc_Tree_Node*> &nodeSet);

template<class InputIterator>
ZPoint centroid(InputIterator first, InputIterator last);

template<class InputIterator>
ZCuboid boundBox(InputIterator first, InputIterator last);

template<class InputIterator>
void setType(InputIterator first, InputIterator last, int type);

Swc_Tree_Node* merge(const std::set<Swc_Tree_Node*> &nodeSet);
void kill(std::set<Swc_Tree_Node*> &nodeSet);
void kill(Swc_Tree_Node *tn);

/*!
 * \brief The overall length of the segments formed by a set of nodes
 *
 * Calculates the length of all the segments connecting nodes in \a nodeset. It
 * returns 0 if \a nodeSet is empty or all nodes in the set is isolated.
 */
double segmentLength(std::set<Swc_Tree_Node*> &nodeSet);

//clipboard
void clearClipboard();
void addToClipboard(const Swc_Tree_Node* tn);
void paste(Swc_Tree_Node *tn, size_t index = 0);
inline std::vector<std::string>& clipboard() {
  return SwcTreeNodeClipboard;
}

//Relations
bool hasOverlap(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2);

/*!
 * \brief Test if two nodes have significant overlap
 *
 * \return true iff the center of \a tn1 is in \a tn2 or \a tn2 is in \a tn1.
 *   It always returns false if \a tn1 or \a tn2 is not a regular node.
 */
bool hasSignificantOverlap(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2);

bool isTurn(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2,
            const Swc_Tree_Node *tn3);

/*!
 * \brief Normalized dot product between two node edges
 *
 * The normalized dot product between tn1->tn2 and tn2->tn3.
 */
double normalizedDot(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2,
                     const Swc_Tree_Node *tn3);

/*!
 * \brief The curvature presented by three nodes
 *
 * Curvature of tn1 - tn2 - tn3
 *
 * \return 0 if any of the node is not regular
 */
double computeCurvature(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2,
                        const Swc_Tree_Node *tn3);

/*!
 * \return true iff \a tn1 is completely within \a tn2. It returns false if
 *         either \a tn1 or \a tn2 is not regular.
 */
bool isWithin(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2);

//Comparison functions
int compareZ(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2);

//A virtual node is always less than a regular node
//Always returns false for two virtual nodes
bool lessThanZ(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2);


bool lessThanWeight(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2);
bool biggerThanWeight(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2);

//Interaction
void average(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2,
             Swc_Tree_Node *out);

/*!
 * \brief interpolate
 *
 * (x, y, z, r) [\a out] = (x, y, z, r) [\a tn1] * lambda + (x, y, z, r) [\a tn2] * (1 - lambda)
 * if both \a tn1 and \a tn2 are regular. Other attributes of \a out is not
 * changed. \a out remains virtual if it is virtual.
 */
void interpolate(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2,
                 double lambda, Swc_Tree_Node *out);

/*!
 * \brief Match all neighbors of a node to find any plausible crossover
 * \param center The center node
 * \param maxAngle Two nodes cannot form a cross if their angle (with \a center)
 *                 is bigger than maxAngle.
 * \return The pair of crossover nodes
 */
std::map<Swc_Tree_Node*, Swc_Tree_Node*> crossoverMatch(
    const Swc_Tree_Node *center, double maxAngle = 0.0);


//I/O
void dump(const Swc_Tree_Node *tn, std::ostream &stream);

static const double MinimalRadius = 0.1;
}

template<class InputIterator>
ZCuboid SwcTreeNode::boundBox(InputIterator first, InputIterator last)
{
  ZCuboid bound = boundBox(*first);
  ++first;

  while (first != last) {
    bound.bind(boundBox(*first));
    ++first;
  }

  return bound;
}

template<class InputIterator>
void SwcTreeNode::setType(InputIterator first, InputIterator last, int type)
{
  while (first != last) {
    setType(*first, type);
    ++first;
  }
}

template<class InputIterator>
ZPoint SwcTreeNode::centroid(InputIterator first, InputIterator last)
{
  ZPoint pt(0.0, 0.0, 0.0);
  double weight = 0.0;
  int count = 0;

  while (first != last) {
    pt += pos(*first) * radius(*first);
    weight += radius(*first);
    ++first;
    ++count;
  }

  if (weight > 0.0) {
    pt /= weight;
  } else {
    pt /= count;
  }

  return pt;
}

#endif // SWCTREENODE_H
