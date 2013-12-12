/**@file tz_swc_tree.h
 * @brief Swc tree
 * @author Ting Zhao
 * @date 16-Jul-2009
 */

#ifndef _TZ_SWC_TREE_H_
#define _TZ_SWC_TREE_H_

#include "tz_cdefs.h"
#include "tz_swc_cell.h"
#include "tz_local_neuroseg.h"
#include "tz_geo3d_point_array.h"
#include "tz_workspace.h"

__BEGIN_DECLS

/**@struct _Swc_Tree_Node tz_swc_tree.h
 *
 * Linked structure of a node in a SWC tree.
 */
typedef struct _Swc_Tree_Node {
  Swc_Node node;                       /**< node for swc file */
  double weight;                       /**< weight to connect to its parent */
  double feature;                      /**< feature of the node */
  struct _Swc_Tree_Node *parent;       /**< parent */
  struct _Swc_Tree_Node *first_child;  /**< first child */
  struct _Swc_Tree_Node *next_sibling; /**< next sibling */
  struct _Swc_Tree_Node *next;         /**< for iteration */
  int index;                           /**< 0-based index in iteration */ 
  int flag;                            /**< a flag for temporary operation */
                                       /*Must reset to 0 after usage*/
} Swc_Tree_Node;

typedef struct _Swc_Tree_Node_Map {
  Swc_Tree_Node *tree_node;
} Swc_Tree_Node_Map;

/**@struct _Swc_Tree tz_swc_tree.h
 *
 * The structure of a SWC tree.
 */
typedef struct _Swc_Tree {
  Swc_Tree_Node *root;         /**< root of the tree */
  Swc_Tree_Node *iterator;     /**< iterator */
  Swc_Tree_Node *begin;        /**< begin of the iterator */
} Swc_Tree;

typedef struct _Swc_Tree_Branch {
  Swc_Tree_Node *end;
  int n;
} Swc_Tree_Branch;

/**@brief New a swc tree node.
 */
Swc_Tree_Node* New_Swc_Tree_Node();

/**@brief Set default attributes of a swc tree node
 *
 * Default values:
 *   All link is set to NULL
 *   weight, feature, flag, x, y, z are set to 0
 *   radius is set to 1
 *   ID is set to 0
 *   type is set to 2 (axon)
 */
void Default_Swc_Tree_Node(Swc_Tree_Node *node);

/**@brief Delete a swc tree node.
 */
void Delete_Swc_Tree_Node(Swc_Tree_Node *tn);

/**@brief Make a swc tree node.
 *
 * The property of the node will be the same as <node>.
 */
Swc_Tree_Node* Make_Swc_Tree_Node(const Swc_Node *node);

/**@brief Make a virtual node.
 *
 * Make_Virtual_Swc_Tree_Node() returns a virtual node.
 */
Swc_Tree_Node* Make_Virtual_Swc_Tree_Node();

/**@brief Kill a swc tree node.
 */
void Kill_Swc_Tree_Node(Swc_Tree_Node *tn);

/**@brief Print a swc tree node.
 */
void Print_Swc_Tree_Node(const Swc_Tree_Node *tn);

/**@brief Copy the property of an SWC node to another.
 *
 * Swc_Tree_Node_Copy_Property() copies the properties of <src> to <dst>. The 
 * linking and iterating information will not be copied.
 */
void Swc_Tree_Node_Copy_Property(const Swc_Tree_Node *src, Swc_Tree_Node *dst);

/**@brief Turn an SWC tree node to virtual.
 */
void Swc_Tree_Node_To_Virtual(Swc_Tree_Node *tn);

/**@brief Clean a subtree.
 *
 * Swc_Tree_Node_Clean_Subtree() Kills all nodes in the sub-tree that has <tn>
 * as its root.
 */
void Swc_Tree_Node_Kill_Subtree(Swc_Tree_Node *tn);

/**@brief Data reference of a swc tree node.
 *
 * Swc_Tree_Node_Data() returns the data pointer of <tn>.
 */
Swc_Node* Swc_Tree_Node_Data(Swc_Tree_Node *tn);

/**@brief Constant data reference of a swc tree node.
 *
 * Swc_Tree_Node_Data() returns the const data pointer of <tn>.
 */
const Swc_Node* Swc_Tree_Node_Const_Data(const Swc_Tree_Node *tn);

/**@brief Id of a swc tree node.
 */
int Swc_Tree_Node_Id(const Swc_Tree_Node *tn);

/**@brief Parent id of a SWC tree node.
 */
int Swc_Tree_Node_Parent_Id(const Swc_Tree_Node *tn);

/**@brief Set id of a swc tree node.
 *
 * Swc_Tree_Node_Set_Id() sets the id of <tn> to <id>.
 */
void Swc_Tree_Node_Set_Id(Swc_Tree_Node *tn, int id);

/**@biref Set parent id of a swc tree node.
 *
 * Swc_Tree_Node_Set_Parent_Id() sets the parent id of <tn> to <id>.
 */
void Swc_Tree_Node_Set_Parent_Id(Swc_Tree_Node *tn, int id);

/**@brief Label of an SWC node.
 *
 * Swc_Tree_Node_Label() returns the label of <tn>.
 */
int Swc_Tree_Node_Label(const Swc_Tree_Node *tn);

/**@brief Set the label of an SWC node.
 *
 * Swc_Tree_Node_Set_Label() set the label of <tn> to <label>.
 */
void Swc_Tree_Node_Set_Label(Swc_Tree_Node *tn, int label);

/**@brief Set label for downstream nodes.
 *
 * Swc_Tree_Node_Set_Label_D() sets the labels of <tn> and its descents to
 * <label>. If <overwrite> is FALSE, <label> is assigned to a node only when
 * its label is 0.
 */
void Swc_Tree_Node_Set_Label_D(Swc_Tree_Node *tn, int label, int overwrite);

/**@brief Radius of an SWC node
 */
double Swc_Tree_Node_Radius(const Swc_Tree_Node *tn);

/**@brief Weight of an SWC node.
 */
double Swc_Tree_Node_Weight(const Swc_Tree_Node *tn);

int Swc_Tree_Node_Type(const Swc_Tree_Node *tn);

/**@brief The position of a swc tree node.
 *
 * Swc_Tree_Node_Pos() stores the position of <tn> into <pos>.
 */
void Swc_Tree_Node_Pos(const Swc_Tree_Node *tn, double *pos);
double Swc_Tree_Node_X(const Swc_Tree_Node *tn);
double Swc_Tree_Node_Y(const Swc_Tree_Node *tn);
double Swc_Tree_Node_Z(const Swc_Tree_Node *tn);

/**@brief Set the position of a swc tree node.
 *
 * Swc_Tree_Node_Set_Pos() sets the position of <tn>.
 */
void Swc_Tree_Node_Set_Pos(Swc_Tree_Node *tn, const double *pos);

/**@brief Iterate swc tree node.
 *
 * Swc_Tree_Node_Next() returns the node next to <tn> via depth-first search.
 */
Swc_Tree_Node* Swc_Tree_Node_Next(Swc_Tree_Node *tn);

/**@brief Last child of a swc tree node.
 *
 * Swc_Tree_Node_Last_Child() returns the last child of <tn>. It returns NULL
 * if <tn> has no child. 
 */
Swc_Tree_Node* Swc_Tree_Node_Last_Child(const Swc_Tree_Node *tn);

/**@brief Test if a node is the child of the other
 *
 * Swc_Tree_Node_Is_Child() returns TRUE iff <child> is the child of <parent>.
 */
BOOL Swc_Tree_Node_Is_Child(const Swc_Tree_Node *parent, 
			    const Swc_Tree_Node *child);

/**@brief Number of children of an SWC node.
 */
int Swc_Tree_Node_Child_Number(const Swc_Tree_Node *tn);

/**@brief Previous sibling of a swc tree node.
 *
 * Swc_Tree_Node_Prev_Sibling() returns the previous sibling of <tn>. It
 * returns NULL if is has no elder sibling.
 */
Swc_Tree_Node* Swc_Tree_Node_Prev_Sibling(const Swc_Tree_Node *tn);

/**@brief Test if a node is virtual.
 * 
 * A node is virtual if and only if its ID is negative.
 */
BOOL Swc_Tree_Node_Is_Virtual(const Swc_Tree_Node *tn);

/**@brief Test if a node is regular.
 *
 * A node is regular if an only if it is not NULL or virtual.
 */
BOOL Swc_Tree_Node_Is_Regular(const Swc_Tree_Node *tn);

/**@brief Test if a node is a root.
 *
 * A node is a root if and only if it is not NULL or a descendent of a regular 
 * node. 
 */
BOOL Swc_Tree_Node_Is_Root(const Swc_Tree_Node *tn);

/**@brief Test if a node is a regular root.
 *
 * A node is a regular root if and only if it is a root and regular.
 */
BOOL Swc_Tree_Node_Is_Regular_Root(const Swc_Tree_Node *tn);

/**@brief Test if a node is the last child of its parent.
 *
 * A node is the last child if and only if its parent is not NULL and it does
 * not have next sibling.
 */
BOOL Swc_Tree_Node_Is_Last_Child(const Swc_Tree_Node *tn);

/**@brief Test if a node is a leaf.
 *
 * A node is a leaf if and only if:
 *   i) it is regular and
 *   ii) it is not a root and
 *   iii) it has no child
 */
BOOL Swc_Tree_Node_Is_Leaf(const Swc_Tree_Node *tn);

/**@brief Test if a node is a branch point.
 *
 * A node is a branching point if and only if it is regular and has more than
 * one child.
 */
BOOL Swc_Tree_Node_Is_Branch_Point(const Swc_Tree_Node *tn);

/**@brief Test if a node is a structural branch point.
 *
 * A node is a structural branching point if and only if it has more than
 * one child.
 */
BOOL Swc_Tree_Node_Is_Branch_Point_S(const Swc_Tree_Node *tn);

/**@brief Test if a node is a continuation.
 *
 * A node is a continuation if and only if it is regular and is NOT a root,
 * leaf or branch point.
 */
BOOL Swc_Tree_Node_Is_Continuation(const Swc_Tree_Node *tn);

/**@brief Test if a node is a spur.
 *
 * A node is a spur if and only if it is a leaf and its parent is a branch 
 * point 
 */
BOOL Swc_Tree_Node_Is_Spur(const Swc_Tree_Node *tn);

/**@brief Test if two nodes are siblings.
 *
 * Swc_Tree_Node_Is_Sibling() returns TRUE if <tn1> and <tn2> are siblings;
 * otherwise it returns FALSE.
 */
BOOL Swc_Tree_Node_Is_Sibling(const Swc_Tree_Node *tn1, 
			      const Swc_Tree_Node *tn2);

/**@brief Add an child to a swc tree node.
 *
 * Swc_Tree_Node_Add_Child() set <child> as the last child of <tn>. It returns
 * the child added.
 */
Swc_Tree_Node* Swc_Tree_Node_Add_Child(Swc_Tree_Node *tn, Swc_Tree_Node *child);

/**@brief Family size of a node.
 *
 * Swc_Tree_Node_Fsize() returns the family size of <tn>, which is defined as
 * the number of descents of <tn> plus 1.
 */
int Swc_Tree_Node_Fsize(Swc_Tree_Node *tn);

/**@brief Thickest child of a node.
 *
 * Swc_Tree_Node_Thickest_Child() returns the thickest child of <tn>. It returns
 * NULL if either <tn> is NULL or it has no child.
 */
Swc_Tree_Node* Swc_Tree_Node_Thickest_Child(const Swc_Tree_Node *tn);

/**@brief Remove a child of a swc tree node.
 *
 * Swc_Tree_Node_Remove_Child() removes <child> from the chilren of <tn> if
 * it is one of them. It does nothing if <child> is not a child of <tn>.
 */
void Swc_Tree_Node_Remove_Child(Swc_Tree_Node *tn, Swc_Tree_Node *child);

/**@brief Detach a node from its parent.
 *
 * Swc_Tree_Node_Detach_Parent() cuts the link between <tn> and its parent.
 * After the function call, the parent of <tn> will be NULL.
 */
Swc_Tree_Node* Swc_Tree_Node_Detach_Parent(Swc_Tree_Node *tn);

/**@brief Set the parent of a swc tree node.
 *
 * Swc_Tree_Node_Set_Parent() sets the parent of <tn> to <parent>. <tn> becomes
 * the last child of <parent>.
 */
void Swc_Tree_Node_Set_Parent(Swc_Tree_Node *tn, Swc_Tree_Node *parent);

/**@brief Replace a child of a swc tree node.
 *
 * Swc_Tree_Node_Replace_Child() assumes <old_child> is a child of a node x and 
 * replace it with <new_child> so that x has <new_child> take the place of
 * <old_child>.
 */
void Swc_Tree_Node_Replace_Child(Swc_Tree_Node *old_child, 
				 Swc_Tree_Node *new_child);

/**@brief Build a linked list for the subtree rooted from a node.
 */
int Swc_Tree_Node_Build_Downstream_List(Swc_Tree_Node *tn);

/**@brief Build a linked list for the subtree rooted from a node with an exclusion.
 *
 * Swc_Tree_Node_Build_Downstream_List_E() builds an intrinsic linked list for
 * the subtree rooted from <tn>. The node <excluded> serves a blocker for the
 * iteration. No downstream node of <excluded>, including <excluded> itself,
 * will be in the list.
 */
int Swc_Tree_Node_Build_Downstream_List_E(Swc_Tree_Node *tn, 
    Swc_Tree_Node *excluded);

int Swc_Tree_Node_Build_Downstream_List_L(Swc_Tree_Node *tn, 
    int label);

/**@brief Find the downstream leaf that is the furthest away.
 *
 * Swc_Tree_Node_Furthest_Leaf() return a leaf that is a descent of <tn> and
 * further away than any other descent leaves.
 */
Swc_Tree_Node* Swc_Tree_Node_Furthest_Leaf(Swc_Tree_Node *tn);

/**@brief Find the downstream leaf that is the furthest away with exclusion.
 *
 * Swc_Tree_Node_Furthest_Leaf() return a leaf that is a descent of <tn> and
 * further away than any other descent leaves. Any leaf desending from
 * <excluded> will be ignored.
 */
Swc_Tree_Node* Swc_Tree_Node_Furthest_Leaf_E(Swc_Tree_Node *tn, 
    Swc_Tree_Node *excluded);

/**@brief Find the downstream leaf that is the furthest away.
 *
 * Swc_Tree_Node_Furthest_Leaf() return a leaf that is a descent of <tn> and
 * further away than any other descent leaves. Any leaf desending from
 * the nodes with labels other than <label> will be ignored.
 */
Swc_Tree_Node* Swc_Tree_Node_Furthest_Leaf_L(Swc_Tree_Node *tn, 
    int label);

/**@brief Relace a branch of a tree.
 *
 * Swc_Tree_Node_Graft() replace the branch <r1>-><t1> with <r2>-><t2>.
 */
void Swc_Tree_Node_Graft(Swc_Tree_Node *r1, Swc_Tree_Node *t1,
			 Swc_Tree_Node *r2, Swc_Tree_Node *t2);

/**@brief Insert a swc tree node.
 *
 * Swc_Tree_Node_Insert_Before() inserts <tn2> between <tn> and its parent.
 * After insertion, <tn2> takes the place of <tn> in the tree and <tn> 
 * becomes the first child of <tn2>. Both <tn> and <tn2> must be regular 
 * nodes. 
 */
void Swc_Tree_Node_Insert_Before(Swc_Tree_Node *tn, Swc_Tree_Node *tn2);

/**@brief Set a node as the first child of its parent
 */
void Swc_Tree_Node_Set_As_First_Child(Swc_Tree_Node *tn);

/**@brief Add a internal to an edge.
 *
 * Swc_Tree_Node_Add_Break() adds a node between <tn> and its parent. The
 * position of the node is determined by <lambda>, which is the breaking
 * ratio. The node is closer to <tn> when <lambda> is larger. It returns the
 * new node.
 */
Swc_Tree_Node* Swc_Tree_Node_Add_Break(Swc_Tree_Node *tn, double lambda);

/**@brief Merge a node and one of its siblings.
 *
 * Swc_Tree_Node_Merge_Sibling() merges <tn2> to <tn1> if <tn2> is a sibling of
 * <tn1>. Otherwise it does not affect <tn1> or <tn2>.
 */
void Swc_Tree_Node_Merge_Sibling(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2);

/**@brief Merge a node to its parent.
 *
 * Swc_Tree_Node_Merge_To_Parent() merges <tn> with its parent. If <tn> has no
 * parent, it will be either deleted if it has a child or become a virtual node.
 * The function returns NULL. If <tn> has a parent, the children of <tn> 
 * are inserted to the children list of its parent at the original position of 
 * <tn>. The children <tn> itself will be deleted. The function returns the 
 * parent of <tn>.
 */
Swc_Tree_Node* Swc_Tree_Node_Merge_To_Parent(Swc_Tree_Node *tn);

/**@brief Set a node as a root.
 *
 * Swc_Tree_Node_Set_Root() makes <tn> as the root of the tree to which it
 * blongs. For any edge that is reversed, the parent becomes the last child
 * of its child.
 */
void Swc_Tree_Node_Set_Root(Swc_Tree_Node *tn);

/**@brief Set a node as a pointer root.
 *
 * Swc_Tree_Node_Set_Root() makes <tn> as the pointer root of the tree to which
 * it blongs. For any edge that is reversed, the parent becomes the last child
 * of its child. A node is a pointer root if its parent is NULL.
 */
void Swc_Tree_Node_Set_Root_A(Swc_Tree_Node *tn);

/**@brief Test if one node is at the upstream of the other.
 *
 * It returns TRUE iff <tn1> is from the upstream of <tn2>. It returns FALSE if
 * either of the inputs is NULL.
 */
BOOL Swc_Tree_Node_Is_Upstream(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2);

/**@brief The distance from a node to its parent.
 *
 * Swc_Tree_Node_Length() returns the length of the edge that connnects <tn> and
 * its parent. It returns 0 if <tn> has no parent.
 */
double Swc_Tree_Node_Length(const Swc_Tree_Node *tn);

/**@brief The distance between two nodes.
 *
 * Swc_Tree_Node_Dist() returns the distance between <tn1> and <tn2>.
 */
double Swc_Tree_Node_Dist(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2);

/**@brief The weight accumulation of tracing back to an anscester branch point.
 *
 * Swc_Tree_Node_Backtrace_Length() returns the accumulated weights along the
 * path when <tn> back to a branch point. The tracing stops when it encounters 
 * n branch points. But if n is -1, it traces back to the root.
 */
double Swc_Tree_Node_Backtrace_Weight(const Swc_Tree_Node *tn, int n);

/**@brief The direction of a node.
 *
 * Swc_Tree_Node_Direction() returns the direction of the edge connecting from
 * the parent of <tn> to <tn>.
 */
void Swc_Tree_Node_Direction(const Swc_Tree_Node *tn, double *vec);

/**@brief Common ancestor of two nodes.
 *
 * Swc_Tree_Node_Common_Ancestor() return the first common ancestor of <tn1>
 * and <tn2>. It returns NULL if <tn1> and <tn2> has no common ancestor,
 * including the case that either <tn1> or <tn2> is NULL.
 */
Swc_Tree_Node* Swc_Tree_Node_Common_Ancestor(const Swc_Tree_Node *tn1,
    const Swc_Tree_Node *tn2);

enum { SWC_TREE_NODE_FORWARD, SWC_TREE_NODE_BACKWARD, SWC_TREE_NODE_BIDIRECT };

/**@brief Turn a node to a vector.
 *
 * Swc_Tree_Node_Vector() calculates the vector starting from <tn>. The end of
 * vector depends on <direction>, which has three possible values, 
 * SWC_TREE_NODE_FORWARD, SWC_TREE_NODE_BACKWARD, SWC_TREE_NODE_BIDIRECT.    
 * SWC_TREE_NODE_FORWARD is the direction from <tn> to its first child.
 * SWC_TREE_NODE_BACKWARD is the direction from the parent of <tn> to <tn>.
 * SWC_TREE_NODE_BIDIRECT is the direction from the parent of <tn> to the
 * child of <tn>. If <tn> has no child, <tn> itself takes the place. If <tn> is 
 * a root and has more than one child, the second child takes the place of the
 * start position.
 */
void Swc_Tree_Node_Vector(const Swc_Tree_Node *tn, int direction,
			  double *vec);

/**@brief Label a branch.
 *
 * Swc_Tree_Node_Label_Branch() labels the branch of that contains <tn> with
 * <label>. Both ends (branch point or terminal) are also labeled.
 */
void Swc_Tree_Node_Label_Branch(Swc_Tree_Node *tn, int label);

/**@brief Label an upstream branch.
 *
 * Swc_Tree_Node_Label_Branch_U() labels the nodes from <tn> till its first 
 * ancester branch point node, which will not be labeled.
 */
void Swc_Tree_Node_Label_Branch_U(Swc_Tree_Node *tn, int label);

double Swc_Tree_Node_Dot(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2,
    const Swc_Tree_Node *tn3);

/**@brief Test if three nodes form a turn.
 *
 * It returns TRUE iff 
 * 1) <tn1>--><tn2>--><tn3> or <tn3>--><tn2>--><tn1>; and
 * 2) the vectors of <tn1>--><tn2> and <tn2>--><tn3> form an 
 *    angle greater than 180 degrees.
 */
BOOL Swc_Tree_Node_Forming_Turn(const Swc_Tree_Node *tn1, 
    const Swc_Tree_Node *tn2, const Swc_Tree_Node *tn3);

/**@brief Test if a node is a turning node.
 *
 * It returns TRUE iff
 * 1) <tn> is a continuation; and
 * 2) the parent of <tn>, <tn>, the child of <tn> form a turn.
 */
BOOL Swc_Tree_Node_Is_Turn(Swc_Tree_Node *tn);

/**@brief Test if a node is an overshoot.
 *
 * It returns TRUE iff
 * 1) <tn> is a turn; and
 * 2) One neighbor node of <tn> is a branch point and the other is not.
 */
BOOL Swc_Tree_Node_Is_Overshoot(Swc_Tree_Node *tn);

/**@brief Set weight of a node.
 *
 * Swc_Tree_Node_Set_Weight() sets the weight of <tn> to <weight>.
 */
void Swc_Tree_Node_Set_Weight(Swc_Tree_Node *tn, double weight);

/**@brief Translate a node
 */
void Swc_Tree_Node_Translate(Swc_Tree_Node *tn, double dx, double dy,
    double dz);

/**@brief Rotate a node
 */
void Swc_Tree_Node_Rotate(Swc_Tree_Node *tn, double theta, double psi,
    double cx, double cy, double cz);

/**@brief Shrink a node.
 *
 * Swc_Tree_Node_Radial_Shrink() shrinks back <tn> according to the size of 
 * <tn>.
 */
void Swc_Tree_Node_Radial_Shrink(Swc_Tree_Node *tn);

/**@brief Tune forks of a tree.
 * 
 * Swc_Tree_Node_Tune_Fork() assumes <tn> is a branch point and adjusts the
 * children of <tn> to remove turns.
 */
void Swc_Tree_Node_Tune_Fork(Swc_Tree_Node *tn);

/**@brief Tune a branch point.
 *
 * Swc_Tree_Node_Tune_Branch() assumes <tn> is not a branch point and the
 * parent of <tn> is a branch point. It checks the grandparent of <tn> and the
 * siblings of <tn> to find the closest node that does not form a turn and 
 * change the parent of <tn> to it if it is closer than the original parent.  
 * If the child of <tn> is a branch point, it will be adjusted too. <tn> will
 * search the closest node among the children of of fc(tn) and make it a new
 * branch point.
 */
void Swc_Tree_Node_Tune_Branch(Swc_Tree_Node *tn);

/**@brief Connect two SWC tree nodes.
 *
 * Swc_Tree_Node_Connect() connects <tn> and <tn2> by adding intermidiate nodes.
 * <tn2> becomes a descent of <tn> after the connection.
 */
void Swc_Tree_Node_Connect(Swc_Tree_Node *tn, Swc_Tree_Node *tn2);

/**@brief Turn an SWC tree node to a locseg.
 *
 * Swc_Tree_Node_To_Locseg() returns a locseg that is based on <tn> and its 
 * parent. Both <tn> and its parent has to be regular nodes. Otherwise the
 * function returns NULL. If <locseg> is not NULL, it is the same as the
 * returned pointer.
 */
Local_Neuroseg* Swc_Tree_Node_To_Locseg(const Swc_Tree_Node *tn,
					Local_Neuroseg *locseg);

/**@brief Turn an SWC tree node to a locseg on the plane at z = 0
 */
Local_Neuroseg* Swc_Tree_Node_To_Locseg_P(const Swc_Tree_Node *tn,
    Local_Neuroseg *locseg);

/**@brief Swc tree node hit test.
 *
 * Swc_Tree_Node_Hit_Test() returns TRUE if (<x>, <y>, <z>) hits the segment
 * <tn> and its parent. If <tn> has no regular parent, the ball of <tn> is used.
 */
BOOL Swc_Tree_Node_Hit_Test(Swc_Tree_Node *tn, double x, double y, double z);

/**@brief Plane hit test for swc tree node.
 *
 * Swc_Tree_Node_Hit_Test_P() returns TRUE iff (<x>, <y>) hits the projection of
 * <tn> on the X-Y plane.
 */
BOOL Swc_Tree_Node_Hit_Test_P(Swc_Tree_Node *tn, double x, double y);

typedef enum {
  SWC_TREE_LABEL_NODE, SWC_TREE_LABEL_CONNECTION, SWC_TREE_LABEL_ALL
} Swc_Tree_Node_Label_Mode_t;

typedef struct _Swc_Tree_Node_Label_Workspace {
  Stack_Draw_Workspace sdw;
  Swc_Tree_Node_Label_Mode_t label_mode;
  double offset[3];
  BOOL z_proj;
} Swc_Tree_Node_Label_Workspace;

void Default_Swc_Tree_Node_Label_Workspace(Swc_Tree_Node_Label_Workspace *ws);

void Swc_Tree_Node_Label_Stack(const Swc_Tree_Node *tn, Stack *stack,
    Swc_Tree_Node_Label_Workspace *ws);

/**@brief A line segment from an SWC node
 *
 * Swc_Tree_Node_Edge() retrieves the line segment from the parent of <tn> and
 * <tn> itself. The start point and end point are stored in <start> and <end>
 * respectively.  It returns TRUE if the line segment exists, i.e. <tn> must
 * not be a root or virtual; Otherwise it returns false and the values in
 * <start> and <end> are not valid.
 */
BOOL Swc_Tree_Node_Edge(const Swc_Tree_Node *tn, double *start, double *end);

/**@brief Average thickness of a branch.
 *
 * Swc_Tree_Node_Average_Thickness() returns the average thickness of a branch.
 * The branch is from <start> to <end>. The caller needs to guarantee that
 * <start> is NULL or an ancestor of <end>. When <start> is NULL, it calculates
 * the average thickness from <end> to the root. <end> cannot be NULL or virtual
 * node.
 */
double Swc_Tree_Node_Average_Thickness(const Swc_Tree_Node *start,
				       const Swc_Tree_Node *end);

/**@brief Get a branch.
 * 
 * Swc_Tree_Get_Branch() retrieves the branch that traces back from <tn> to the
 * nearest branch point. If <include_bp> is TRUE, the branch point will also
 * included in <tb>; Otherwise <tb> does not contain the branch point.
 */
void Swc_Tree_Get_Branch(Swc_Tree_Node *tn, BOOL include_bp, 
    Swc_Tree_Branch *tb);

/**** routines for swc tree branch */
void Print_Swc_Tree_Branch(const Swc_Tree_Branch *tb);

/**@brief Set an SWC branch 
 * 
 * Set_Swc_Tree_Branch() sets the end of <tb> to <tn>. The number of nodes in
 * the branch is set to min(<n>, possible maximum number) if <n> is greater
 * than 0. If <n> is 0, the branch node number is set to the possible maximum
 * number, i.e. the branch is the one from <end> to the root. It is undefined
 * for negative <n>.
 */
void Set_Swc_Tree_Branch(Swc_Tree_Branch *tb, Swc_Tree_Node *tn, int n);

double Swc_Tree_Branch_Length(const Swc_Tree_Branch *tb);

/**@brief Surface area of a segment.
 *
 * Swc_Tree_Branch_Surface_Area() returns the surface area of the cylinder
 * formed by <tb> and its parent.
 */
double Swc_Tree_Branch_Surface_Area(const Swc_Tree_Branch *tb);

/* routines for swc tree */
Swc_Tree* New_Swc_Tree();

void Default_Swc_Tree(Swc_Tree *tree);
void Clean_Swc_Tree(Swc_Tree *tree);
void Kill_Swc_Tree(Swc_Tree *tree);

Swc_Tree* Read_Swc_Tree(const char *file_path);
Swc_Tree* Read_Swc_Tree_E(const char *file_path);
BOOL Write_Swc_Tree(const char *file_path, Swc_Tree *tree);
BOOL Write_Swc_Tree_E(const char *file_path, Swc_Tree *tree);

void Print_Swc_Tree(Swc_Tree *tree);

/**@brief Copy an SWC tree.
 *
 * Copy_Swc_Tree() returns a copy of <tree>. The copy duplicates all 
 * information from <tree> except its iterator and indexing, which may be
 * modified after function call. The copy does not share any memory with <tree>.
 */
Swc_Tree* Copy_Swc_Tree(Swc_Tree *tree);

/* Options for iterating. */
enum { 
  SWC_TREE_ITERATOR_REVERSE = -1,
  SWC_TREE_ITERATOR_NO_UPDATE = 0, 
  SWC_TREE_ITERATOR_DEPTH_FIRST, 
  SWC_TREE_ITERATOR_BREADTH_FIRST,
  SWC_TREE_ITERATOR_LEAF,
  SWC_TREE_ITERATOR_BRANCH_POINT,
  SWC_TREE_ITERATOR_VOID
};

/**@brief Swc tree iteration preparation.
 *
 * Swc_Tree_Iterator_Start() initializes the iterator of <tree> with one of the
 * following optinos: 
 *   SWC_TREE_ITERATOR_NO_UPDATE : no update, reset the internal iterator 
 *     point to the begin
 *   SWC_TREE_ITERATOR_VOID: no update and no change on the current iterator 
 *   SWC_TREE_ITERATOR_REVERSE: reverse the current iterator
 *   SWC_TREE_ITERATOR_DEPTH_FIRST: depth first iteration
 *   SWC_TREE_ITERATOR_BREADTH_FIRST: breadth first iteration
 *   SWC_TREE_ITERATOR_LEAF: iterate through leaves
 *   SWC_TREE_ITERATOR_BRANCH_POINT: iterate through branch points
 * If <indexing> is TRUE, each node will be indexed based on the iteration 
 * order; otherwise the indices are kept intact. Only the two options 
 * SWC_TREE_ITERATOR_NO_UPDATE and SWC_TREE_ITERATOR_VOID has no effect on the 
 * 'next' field. Therefore these two options are inactive, while other options
 * are called active iterators.
 *
 * It returns the number of nodes in the tree except in one of the following 
 * situations:
 *   i) <option> is SWC_TREE_ITEARTOR_NO_UPDATE and <indexing> is FALSE;
 *   ii) the iteration failed.
 */
int Swc_Tree_Iterator_Start(Swc_Tree *tree, int option, BOOL indexing);

BOOL Swc_Tree_Iterator_Is_Active(int option);

/**@brief Build an iterator for a path.
 *
 * Swc_Tree_Iterator_Path() initialize the iterator of <tree> of iterating the
 * path from <begin> to <end>.
 */
void Swc_Tree_Iterator_Path(Swc_Tree *tree, Swc_Tree_Node *begin,
			    Swc_Tree_Node *end);

/**@brief Build an iterator for leaves of an SWC tree.
 * 
 * Swc_Tree_Iterator_Leaf() iterates the leaves of <tree>.
 */
void Swc_Tree_Iterator_Leaf(Swc_Tree *tree);

/**@brief Regular root of a tree.
 *
 * Swc_Tree_Regular_Root() returns the first regular root of <tree>.
 */
Swc_Tree_Node* Swc_Tree_Regular_Root(Swc_Tree *tree);

/**@brief Iterating routine for swc tree.
 *
 * Swc_Tree_Next() increments the iterator of <tree>. It returns the node at the
 * current position.
 */
Swc_Tree_Node* Swc_Tree_Next(Swc_Tree *tree);

/**@brief The number of trees.
 *
 * Swc_Tree_Number() returns the number of trees.
 */
int Swc_Tree_Number(Swc_Tree *tree);

/**@brief Test if an swc tree has a branch
 *
 * It returns TRUE iff <tree> has at least two connected regular nodes.
 */
BOOL Swc_Tree_Has_Branch(Swc_Tree *tree);

/**@brief Find a node with a certain ID.
 *
 * Swc_Tree_Query_Node() returns the first node that has the ID <id>. It returns
 * NULL if no matched node found. The query is based on the current iterator of
 * <tree>. The routine itself will not initialize the iterator.
 */
Swc_Tree_Node* Swc_Tree_Query_Node(Swc_Tree *tree, int id, int iter_option);

/**@brief Find a node with a certain ID.
 *
 * Swc_Tree_Query_Node_C() is similar to Swc_Tree_Query_Node(), but it searches
 * the whole tree.
 */
Swc_Tree_Node* Swc_Tree_Query_Node_C(Swc_Tree *tree, int id);

/* [left, top, front, right, bottom, back] */
void Swc_Tree_Bound_Box(Swc_Tree *tree, double *corner);

/**@brief The thickest node of a tree.
 *
 * Swc_Tree_Thickest_Node() returns the thickest node of <tree>.
 */
Swc_Tree_Node* Swc_Tree_Thickest_Node(Swc_Tree *tree);

/**@brief Save an swc tree as a dot file.
 *
 * Swc_Tree_To_Dot_File() saves <tree> as a dot file, which is specified by
 * <file_path>.
 */
void Swc_Tree_To_Dot_File(Swc_Tree *tree, const char *file_path);

/**@breif Sort Ids of a tree.
 *
 * Swc_Tree_Resort_Id() sorts the Ids of all nodes of <tree> so that they 
 * conform to the SWC format.
 */
void Swc_Tree_Resort_Id(Swc_Tree *tree);

/**@brief Remove zigzags in a tree.
 *
 * Swc_Tree_Remove_Zigzag() removes all zigzags in <tree>.
 */
void Swc_Tree_Remove_Zigzag(Swc_Tree *tree);

/**@brief Remove overshoots in a tree.
 *
 * Swc_Tree_Remove_Zigzag() removes all overshoots in <tree>.
 */
void Swc_Tree_Remove_Overshoot(Swc_Tree *tree);

/**@brief The distance between a tree and a point.
 *
 * Swc_Tree_Point_Dist() returns the distance between the surface of <tree> and
 * the point (<x>, <y>, <z>). The closest surface point is stored in <pt> if
 * <pt> is not NULL. The node that provides the surface point with its parent 
 * is stored in <ctn> if <ctn> is not NULL.
 */
double Swc_Tree_Point_Dist(Swc_Tree *tree, double x, double y, double z,
			   double *pt, Swc_Tree_Node **ctn);

/**@brief The distance between a tree and a point.
 *
 * Swc_Tree_Point_Dist() returns the shortest distance between the surfaces of 
 * the nodes of <tree> and the point (<x>, <y>, <z>). The closest node is 
 * stored in <ctn>. The returned value can be negative if the point is inside
 * the node.
 */
double Swc_Tree_Point_Dist_N(Swc_Tree *tree, double x, double y, double z,
    Swc_Tree_Node **ctn);

/**@brief The physical distance between a tree and a point.
 */
double Swc_Tree_Point_Dist_N_Z(Swc_Tree *tree, double x, double y, double z,
    double z_scale, Swc_Tree_Node **ctn);

/**@brief Shrink leaves of a tree.
 *
 * Swc_Tree_Leaf_Shrink() shrinks all leaves of <tree> based on the radii of the
 * leaves.
 */
void Swc_Tree_Leaf_Shrink(Swc_Tree *tree);

/**@brief Tune forks of a tree.
 */
void Swc_Tree_Tune_Fork(Swc_Tree *tree);
void Swc_Tree_Tune_Branch(Swc_Tree *tree);
void Swc_Tree_Remove_Spur(Swc_Tree *tree);

/**@brief Find the shortest terminal branch in a <tree>.
 *
 * Swc_Tree_Find_Shortest_Terminal_Branch() returns the innermost node of the
 * shortest terminal branch. If <blen> is not NULL, it will store the length of 
 * the branch.
 */
Swc_Tree_Node* Swc_Tree_Find_Shortest_Terminal_Branch(Swc_Tree *tree, 
						      double *blen);

/**@brief Remove short branches.
 *
 * Swc_Tree_Remove_Terminal_Branch() remove terminal branches that are 
 * shorter than <thre> and not connect to a root directly.
 */
 void Swc_Tree_Remove_Terminal_Branch(Swc_Tree *tree, double thre);

/**@brief Connect a node to the closest node in a tree.
 *
 */
void Swc_Tree_Connect_Node(Swc_Tree *tree, Swc_Tree_Node *tn);

/**@brief Connect a branch to the closest node in a tree.
 *
 * The branch is defined as the nodes between <start> and its last first 
 * descent. It returns the node that connects to <tree> directly.
 */
Swc_Tree_Node* Swc_Tree_Connect_Branch(Swc_Tree *tree, Swc_Tree_Node *start);

void Swc_Tree_Clean_Root(Swc_Tree *tree);

void Swc_Tree_Merge_Close_Node(Swc_Tree *tree, double thre);

void Swc_Tree_Merge_Node_As_Root(Swc_Tree *tree, Swc_Tree_Node *tn);

Swc_Tree_Node* Swc_Tree_Closest_Node(Swc_Tree *tree, const double *pos);
Swc_Tree_Node* Swc_Tree_Closest_Node_I(Swc_Tree *tree, const double *pos,
    int iterator_option);

Swc_Tree_Node* Swc_Tree_Largest_Node(Swc_Tree *tree);

/**@brief The outmost leaf of a tree.
 *
 * Swc_Tree_Outmost_leaf() returns the outmost leaf of a tree. Here the outmost
 * leaf is defined as the leaf that is furthest away from the root.
 */
Swc_Tree_Node* Swc_Tree_Outmost_Leaf(Swc_Tree *tree);

/**@brief Reduce a tree to exclude all continuation nodes.
 *
 * Swc_Tree_Reduce() removes all continuation nodes in <tree>. The weight of
 * each node is set to the accumulated length to its parent in the original
 * tree.
 */
void Swc_Tree_Reduce(Swc_Tree *tree);

BOOL Swc_Tree_Label_Branch(Swc_Tree *tree, int label,
			   const double *pos, double thre);

/**@brief Label all branches with distinguishable values in a tree.
 *
 * Swc_Tree_Label_Branch_All() assigns each branch in <tree> a unique value with
 * the depth-first order. It returns the maximum label, which is the same as 
 * the number of branches. Each branch point has the same label as its upstream
 * branch. The regular root is treated the same as a branch point. 
 * The label starts from 1. The virtual node will be labeled too.
 */
int Swc_Tree_Label_Branch_All(Swc_Tree *tree);

int Swc_Tree_Label_Branch_Order(Swc_Tree *tree);

void Swc_Tree_Svg_Fprint(Swc_Tree *tree, double scale, FILE *fp);

void Swc_Tree_To_Svg_File(Swc_Tree *tree, int max_vx, int max_vy, 
    const char *path);
/*
void Swc_Tree_To_Svg_File_P(Swc_Tree *tree, Geo3d_Scalar_Field *puncta,
    int max_vx, int max_vy, const char *file);
*/
void Swc_Tree_To_Svg_File_W(Swc_Tree *tree, const char *file,
    Swc_Tree_Svg_Workspace *ws);

/**@brief Merge two trees.
 *
 * Swc_Tree_Merge() merges <tree> and <prey>. After merging, all regular trees
 * in <prey> are moved to <tree>. But all the regular structures are kept
 * unchanged.
 */
void Swc_Tree_Merge(Swc_Tree *tree, Swc_Tree *prey);


/**@brief Glue an SWC node to a tree.
 *
 * Swc_Tree_Glue() attaches <tn> to the closest node in <tree>. The parent of
 * <tn> becomes a child of <tn> if it exists.
 */
void Swc_Tree_Glue(Swc_Tree *tree, Swc_Tree_Node *tn);


/**@brief Set the feature of each node in a tree.
 *
 * Swc_Tree_Set_Feature() assigns each node of <tree> an element of <feature>. 
 * The correspondence between a node and a feature is determined by the index 
 * of the node, i.e. the index of the node is the same as the index of the
 * assigned value in <feature>.
 *
 * Note: <tree> must be indexed before being passed in.
 */
void Swc_Tree_Set_Feature(Swc_Tree *tree, double *feature);

/**@brief Number of leaves of an SWC tree.
 *
 * Swc_Tree_Leaf_Number() returns the number of leaves of <tree>.
 */
int Swc_Tree_Leaf_Number(Swc_Tree *tree);

/**@brief Number of branches of an SWC tree.
 *
 * Swc_Tree_Branch_Number() returns the number of continuous branches of
 * <tree>.
 */
int Swc_Tree_Branch_Number(Swc_Tree *tree);

/**@brief Overall length of a tree.
 *
 * Swc_Tree_Overall_Length() returns the overall length of <tree>, which is the
 * sum of the length of all branches.
 */
double Swc_Tree_Overall_Length(Swc_Tree *tree);

/**@brief Overall surface area of a tree.
 *
 */
double Swc_Tree_Surface_Area(Swc_Tree *tree);

/**@brief Accumulated length of each node in a tree.
 *
 * Swc_Tree_Accm_Length() calculates the accumulated length from each node to
 * the root of <tree>. The result is returned as a double array. If <length> is
 * not NULL, it is the same as the returned pointer. <tree> is iterated and 
 * indexed after function call.
 *
 * Note: <tree> must be indexed before passing in.
 */
double* Swc_Tree_Accm_Length(Swc_Tree *tree, double *length);

/**@brief Accumulated thickness of each node in a tree.
 *
 * Swc_Tree_Accm_Length() calculates the accumulated thickness from each node to
 * the root of <tree>. The result is returned as a double array. If <th> is
 * not NULL, it is the same as the returned pointer. <tree> is iterated and 
 * indexed after function call.
 *
 * Note: <tree> must be indexed before passing in.
 */
double* Swc_Tree_Accm_Thickness(Swc_Tree *tree, double *th);

/**@brief Regularize a tree.
 *
 * Swc_Tree_Regularize() removes all virtual nodes except the root in a <tree> 
 * by merging to their parents. See sm037 for more details.
 */
void Swc_Tree_Regularize(Swc_Tree *tree);

/**@brief Canonicalize a tree.
 * 
 * Swc_Tree_Canonicalize() make <tree> canonical. A canonical tree is the
 * simplest representation of the set of logically identical trees.
 */
void Swc_Tree_Canonicalize(Swc_Tree *tree);

/**@brief Pull a tree.
 *
 * Swc_Tree_Pull_R() pulls a tree out of <tree>. The tree is the one that is 
 * closest to the point <root>. The root of the new tree will be set to be as
 * close as possible to <root>.
 */
Swc_Tree* Swc_Tree_Pull_R(Swc_Tree *tree, const double *root);

/**@brief Swc tree hit test.
 *
 * Swc_Tree_Hit_Test() returns TRUE if the point (<x>, <y>, <z>) hits <tree>
 * with the iteration option <iter>.
 */
BOOL Swc_Tree_Hit_Test(Swc_Tree *tree, int iter, 
		       double x, double y, double z);

/**@brief Get the node hit by a point.
 *
 * Swc_Tree_Hit_Node() returns an SWC node that contains the point (<x>, <y>,
 * <z>). If there are more than one node hit by the point, the one with the 
 * smallest normalized distance (d / (r + regulizer)) is returned. It returns NULL if no node
 * is hit.
 */
Swc_Tree_Node* Swc_Tree_Hit_Node(Swc_Tree *tree, double x, double y, double z);

/**@brief Get the node hit by a vertical line.
 *
 * Swc_Tree_Hit_Node() returns an SWC node that hits the vertical line 
 * (along Z) passing (<x>, <y>).
 * If there are more than one node hit by the line, the one with the 
 * smallest normalized distance (d / (r + regulizer)) is returned. 
 * It returns NULL if no node is hit.
 */
Swc_Tree_Node* Swc_Tree_Hit_Node_P(Swc_Tree *tree, double x, double y);

/**brief Cut a node in a tree.
 *
 * Swc_Tree_Cut_Node() assumes <tn> is a <node> in a <tree> and cuts it off. All
 * descendents of <tn> are put under the vitual root of <tree>. The memory of
 * <tn> will be freed.
 */
void Swc_Tree_Cut_Node(Swc_Tree *tree, Swc_Tree_Node *tn);

/**@brief Swc tree subtraction.
 *
 * Swc_Tree_Subtract() removes any node that is in <tree1> and hits <tree2>.
 */
void Swc_Tree_Subtract(Swc_Tree *tree1, Swc_Tree *tree2);

/**@brief Get 2D layout of a branch.
  *
  * Swc_Tree_Branch_2d_Layout() returns a point array that represents the 2D
  * line layout of the branch that has ends <start_id> and <end_id>. The ends
  * of the line are specified by (<x1>, <y1>) and (<x2>, <y2>). <start_id> must
  * be an upstream node of end_id.
  */
Geo3d_Point_Array* Swc_Tree_Branch_2d_Layout(Swc_Tree *tree, int start_id,
    int end_id, double x1, double y1, double x2, double y2, double z_offset,
    double z_scale);

/**@brief Merge all connected nodes with the the same label.
 *
 * Swc_Tree_Merge_Root_Group() merges the set of nodes that satisfy the
 * following conditions:
 *   1. One of them is root;
 *   2. All of them form a connected graph;
 *   3. They have the same label.
 */
void Swc_Tree_Merge_Root_Group(Swc_Tree *tree, int label);

/**@brief Node array of an SWC tree.
 *
 * Swc_Tree_Iterator_Array() returns an array of the iterator of <tree>.The
 * number of array elements is store in <n>. It returns NULL if the iterator is
 * not available.
 */
Swc_Tree_Node** Swc_Tree_Iterator_Array(Swc_Tree *tree, int *n);

Swc_Tree *Swc_Tree_From_Object_3d(const Object_3d *obj);

/**@brief Translate an SWC tree.
 *
 */
void Swc_Tree_Translate(Swc_Tree *tree, double x, double y, double z);

/**@brief Rotate an SWC tree.
 */
void Swc_Tree_Rotate(Swc_Tree *tree, double theta, double psi, 
    double cx, double cy, double cz);

/**@brief Resize an SWC tree.
 *
 */
void Swc_Tree_Resize(Swc_Tree *tree, double x_scale, double y_scale, 
    double z_scale, BOOL change_node_size);

void Swc_Tree_Label_Stack(Swc_Tree *tree, Stack *stack,
    Swc_Tree_Node_Label_Workspace *ws);

/***** Routines for GRASP data analysis *****/
void Swc_Tree_Label_Main_Trunk(Swc_Tree *tree, int label);

void Swc_Tree_Label_Main_Trunk_L(Swc_Tree *tree, int label, 
				 double length_lower_thre, 
				 double length_upper_thre);

void Swc_Tree_Label_Soma(Swc_Tree *tree, int label);
Swc_Tree_Node* Swc_Tree_Grow_Soma(Swc_Tree *tree, int label);
void Swc_Tree_Label_Apical(Swc_Tree *tree, int label);
void Swc_Tree_Resort_Pyramidal(Swc_Tree *tree, BOOL label_soma, 
    BOOL merge_soma);
double Swc_Tree_Node_Apical_Score(Swc_Tree_Node *tn);

void Swc_Tree_Set_Type(Swc_Tree *tree, int type);
void Swc_Tree_Set_Type_As_Label(Swc_Tree *tree);

void Swc_Tree_Set_Label(Swc_Tree *tree, int label);

void Swc_Tree_Identify_Puncta(Swc_Tree *tree, Geo3d_Scalar_Field *puncta, 
    int label, BOOL *on_label);

/**@brief Make all main trunck nodes as the first child.
 *
 * Swc_Tree_Resort_Main_Trunk() turns every main trunck node in <tree> into the
 * first child of its parent.
 */
void Swc_Tree_Resort_Main_Trunk(Swc_Tree *tree);

/* Note: the tree will be changed after the function call. */
void Swc_Tree_To_Analysis_File(Swc_Tree *tree, Geo3d_Scalar_Field *puncta,
    const char *branch_file, const char *puncta_file);

double Swc_Tree_Node_Intensity_Distribution_E(Swc_Tree_Node *tn, Stack *signal, 
    Stack *mask, double margin);
double Swc_Tree_Node_Intensity_Distribution_Ec(Swc_Tree_Node *tn, 
    Stack *signal, Stack *mask, double margin);

void Swc_Tree_Intensity_Feature_E(Swc_Tree *tree, Stack *signal, Stack *mask,
    double margin);
double Swc_Tree_Node_Intensity_Distribution(Swc_Tree_Node *tn, Stack *signal, 
    Stack *mask);

/* SWC tree node features */
void Swc_Tree_Intensity_Feature(Swc_Tree *tree, Stack *signal, Stack *mask);
void Swc_Tree_Intensity_Feature_E(Swc_Tree *tree, Stack *signal, Stack *mask,
    double margin);
void Swc_Tree_Perimeter_Feature(Swc_Tree *tree);
void Swc_Tree_Z_Feature(Swc_Tree *tree);
void Swc_Tree_Puncta_Feature(Swc_Tree *tree, Geo3d_Scalar_Field *puncta);
void Swc_Tree_Feature_Range(Swc_Tree *tree, double *fmin, double *fmax);

void Swc_Tree_Feature_To_Weight(Swc_Tree *tree);

void Swc_Tree_To_Branch_Feature_File(Swc_Tree *tree, const char *filepath);

//Swc_Tree* Swc_Tree_Resample(Swc_Tree *tree, double step);

/* Interaction between SWC tree and stack */
/**@brief Erase an stack by a tree
 */
void Swc_Tree_Erase_Stack(Swc_Tree *tree, Stack *stack, double scale, 
    double delta);
    
/* Cross talk */
double* Swc_Tree_Cross_Talk_Score(Swc_Tree *tree, Swc_Tree *source_tree, 
    int *n);

void Swc_Tree_Remove_Labeled(Swc_Tree *tree, int label);

void Swc_Tree_Centroid(Swc_Tree *tree, double *x, double *y, double *z);

Geo3d_Scalar_Field* Swc_Tree_Sub_To_Scalar_Field(Swc_Tree *tree, int order,
    Geo3d_Scalar_Field *field);

int Swc_Tree_Label_Forest(Swc_Tree *tree);

void Swc_Tree_Reconnect(Swc_Tree *tree, double z_scale, double distThre);

/* Evidence variables for the Bayesian network */
int Swc_Tree_Sub_Degree(Swc_Tree *tree, int order);
int Swc_Tree_Sub_Size(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Length(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Width(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Height(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Depth(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Box_Volume(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Max_Distance(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Min_Distance(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Max_Length(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Min_Length(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Average_Angle(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Max_Angle(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Average_Branching_Angle(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Max_Branching_Angle(Swc_Tree *tree, int order);

__END_DECLS

#endif
