#include "kgraph.h"

#ifdef _MSC_VER
#pragma warning(disable: 4661)
#endif

// Instantiations: <captype, tcaptype, flowtype>
// IMPORTANT: 
//    flowtype should be 'larger' than tcaptype 
//    tcaptype should be 'larger' than captype

template class maxflow::Graph<int,int,int>;
template class maxflow::Graph<short,int,int>;
template class maxflow::Graph<float,float,float>;
template class maxflow::Graph<double,double,double>;

