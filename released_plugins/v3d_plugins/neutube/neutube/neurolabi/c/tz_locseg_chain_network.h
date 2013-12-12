/**@file tz_locseg_chain_network.h
 * @brief network of locseg chain
 * @author Ting Zhao
 * @date 18-Dec-2008
 */

#ifndef _TZ_LOCSEG_CHAIN_NETWORK_H_
#define _TZ_LOCSEG_CHAIN_NETWORK_H_

#include "tz_cdefs.h"
#include "tz_locseg_chain.h"
#include "tz_neuron_structure.h"

__BEGIN_DECLS

typedef struct _Locseg_Chain_Network {
  Neuron_Structure *ns;
  Locseg_Chain *chain_array;
} Locseg_Chain_Network;


void Locseg_Chain_Network_Simlify(Locseg_Chain_Network *net, Graph *graph,
				  Int_Arraylist *cidx, Int_Arraylist *sidx);

Locseg_Chain* Locseg_Chain_Network_Find_Branch(const Neuron_Structure *net);

__END_DECLS

#endif
