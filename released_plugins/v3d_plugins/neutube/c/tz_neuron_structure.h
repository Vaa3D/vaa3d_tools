/**@file tz_neuron_structure.h
 * @brief neuron structure
 * @author Ting Zhao
 * @date 02-Sep-2008
 */

#ifndef _TZ_NEURON_STRUCTURE_H_
#define _TZ_NEURON_STRUCTURE_H_

#include "tz_cdefs.h"
#include "tz_graph.h"
#include "tz_neuron_component_arraylist.h"
#include "tz_neurocomp_conn.h"
#include "tz_swc_cell.h"
#include "tz_swc_tree.h"

__BEGIN_DECLS

/**@addtogroup neuron_struct_ Neuron structure
 * @{
 */

/**@brief Neuron structure
 * 
 * This data structure specifies the graph structure of neuron components. It has
 * three parts, <graph>, <conn> and <comp>. <comp> is an array of the components
 * in the structure. <graph> specifies the connection relationship among the
 * components. <conn> specifies how the component are connected. Each element of
 * <conn> corresponds to each edge in <graph> with the same indexing rule.
 */
typedef struct _Neuron_Structure {
  Graph *graph;
  Neurocomp_Conn *conn;
  Neuron_Component *comp;
} Neuron_Structure;

Neuron_Structure* New_Neuron_Structure();
void Delete_Neuron_Structure(Neuron_Structure *ns);

Neuron_Structure* Make_Neuron_Structure(int ncomp);
void Clean_Neuron_Structure(Neuron_Structure *ns);
void Kill_Neuron_Structure(Neuron_Structure *ns);

void Print_Neuron_Structure(const Neuron_Structure *ns);

#define NEURON_STRUCTURE_LINK_NUMBER(ns) ((ns)->graph->nedge)
#define NEURON_STRUCTURE_COMPONENT_NUMBER(ns) ((ns)->graph->nvertex)

int Neuron_Structure_Link_Number(const Neuron_Structure *ns);
int Neuron_Structure_Component_Number(const Neuron_Structure *ns);

Neurocomp_Conn* Neuron_Structure_Get_Conn(const Neuron_Structure *ns, 
					  int index);

void Neuron_Structure_Add_Conn(Neuron_Structure *ns,
			       int id1, int id2, const Neurocomp_Conn *conn);

/**@brief Remove a connection.
 *
 * Neuron_Structure_Remove_Conn() removes the connection from <id1> to <id2>
 * in <ns> if the connection exists. It does not affect the connection from
 * <id2> to <id1>.
 */
void Neuron_Structure_Remove_Conn(Neuron_Structure *ns, int id1, int id2);

void Neuron_Structure_Set_Component_Array(Neuron_Structure *ns,
					  Neuron_Component *comp, int n);

/**@brief Set a neuron component at a certain index.
 *
 * Neuron_Structure_Set_Component(() sets the component at <index> in <ns> to 
 * the content of <comp>. If <index> is out of the range of <ns>, the function
 * will expand the number of components in <ns> to <index>+1. The internal 
 * data of comp will be associated with <ns>. So when <ns> is killed, <comp> 
 * is cleaned too.
 */
void Neuron_Structure_Set_Component(Neuron_Structure *ns, int index, 
				    const Neuron_Component *comp);

Neuron_Component* Neuron_Structure_Get_Component(const Neuron_Structure *ns, 
						 int index);

void Process_Neuron_Structure(Neuron_Structure *ns);

int Neuron_Structure_Find_Root_Circle(const Neuron_Structure *ns,
				      double *root_pos);

void Neuron_Structure_To_Swc_File(const Neuron_Structure *ns, 
				  const char *file_path);
void Neuron_Structure_To_Swc_File_Circle(const Neuron_Structure *ns, 
					 const char *file_path);
void Neuron_Structure_To_Swc_File_Circle_Z(const Neuron_Structure *ns, 
					   const char *file_path,
					   double z_scale, double *root_pos);
Swc_Tree* Neuron_Structure_To_Swc_Tree_Circle_Z(const Neuron_Structure *ns, 
						double z_scale, 
						double *root_pos);

Neuron_Structure* Neuron_Structure_From_Swc_File(const char *filepath);

void Neuron_Structure_Weight_Graph(Neuron_Structure *ns);
void Neuron_Structure_Unweight_Graph(Neuron_Structure *ns);

void Neuron_Structure_Main_Graph(Neuron_Structure *ns);
void Neuron_Structure_To_Tree(Neuron_Structure *ns);

Neuron_Component_Arraylist* Neuron_Structure_Branch_Point(Neuron_Structure *ns);

void Neuron_Structure_Crossover_Test(Neuron_Structure *ns, double z_scale);

/**@brief Adjust the weights of link connections.
 *
 * Neuron_Structure_Adjust_Link() multiplies the connection cost of each link
 * connection with <factor>.
 */
void Neuron_Structure_Adjust_Link(Neuron_Structure *ns, double factor);

/**@brief angle of a connection
 *
 * Neuron_Structure_Conn_Angle() returns the angle of a connection in <ns>. 
 * <index> specifies the connection index. The result is between [0, PI].
 * But if the angle cannot be calculated, it returns -1.0.
 */
double Neuron_Structure_Conn_Angle(Neuron_Structure *ns, int index, 
				   double z_scale);

/**@}*/

/**@addtogroup neuron_struct_spec_ Specific component structure routines*/
/**@{*/

Neuron_Structure* 
Neuron_Structure_Locseg_Chain_To_Circle(const Neuron_Structure *ns);

Neuron_Structure* 
Neuron_Structure_Locseg_Chain_To_Circle_S(const Neuron_Structure *ns,
					  double xy_scale, double z_scale);

/**@brief Merge straight chains.
 *
 * <ns> must contain a tree structure.
 */
void Neuron_Structure_Merge_Locseg_Chain(const Neuron_Structure *ns);

/**@}*/

int Neuron_Structure_Conn_Index(const Neuron_Structure *ns, int comp_index);

Graph* Neuron_Structure_Import_Xml_Graph(const char *file_path);

/**@brief Initialize a neuron structure based on roots.
 *
 * Neuron_Structure_Load_Root() initializes <ns> so that the tubes covering
 * different roots are connected. <n> is the number of roots and <roots>
 * contains the coordinates. The function assumes that no pair of roots are on 
 * the same chain. It returns the number of edges that are actually added.
 */
int Neuron_Structure_Load_Root(Neuron_Structure *ns, coordinate_3d_t *roots,
			       int n);

/**@brief Break components in a neuron structure based on roots.
 *
 * Neuron_Structure_Break_Root() breaks the components in <ns> so that no pair
 * of roots is from the same component. This function will not update 
 * connections.
 */
void Neuron_Structure_Break_Root(Neuron_Structure *ns, coordinate_3d_t *roots,
				 int n);

/**@brief Remove negative connections.
 *
 * Neuron_Structure_Remove_Negative_Conn() removes connections that have
 * negative cost.
 */
void Neuron_Structure_Remove_Negative_Conn(Neuron_Structure *ns);

__END_DECLS

#endif
