#include "neuron_distance.h"
#include <math.h>

#define DIST(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define MIN(a,b) (a)<(b)?(a):(b)
#define MAX_DOUBLE 1e308

double neuron_distance(NeuronTree & nt1, NeuronTree & nt2)
{
	V3DLONG sz1 = nt1.listNeuron.size();
	V3DLONG sz2 = nt2.listNeuron.size();

	double dist = 0;

	for (V3DLONG i=0;i<sz1;i++)
	{
		double cur_dist = MAX_DOUBLE;
		for (V3DLONG j=0;j<sz2;j++)
			cur_dist = MIN(cur_dist, DIST(nt1.listNeuron.at(i), nt2.listNeuron.at(j)));
		dist += cur_dist;
	}
	
	for (V3DLONG i=0;i<sz2;i++)
	{
		double cur_dist = MAX_DOUBLE;
		for (V3DLONG j=0;j<sz1;j++)
			cur_dist = MIN(cur_dist, DIST(nt2.listNeuron.at(i), nt1.listNeuron.at(j)));
		dist += cur_dist;
	}

	dist /= (sz1 + sz2);
}
