#ifndef _TREE_SEARCH_H__
#define	 _TREE_SEARCH_H__

#include <QtGlobal>
#include <vector>
#include "v3d_message.h"
#include <v3d_interface.h>
#include "neuron_retrieve.h"
#include "compute_gmi.h"
#include "compute_morph.h"

using namespace std;
//void printHelp_global_retrieve();
//int global_retrieve_main(const V3DPluginArgList & input, V3DPluginArgList & output);
int tree_retrieval(const NeuronTree & nt_query, const QList<NeuronTree> & nt_list, vector<V3DLONG> & retrieved_id, const int & num_simstructs);
#endif
