#ifndef __BATCH_COMPUTE_H__
#define __BATHC_COMPUTE_H__
#include "compute_gmi.h"
#include "compute_morph.h"
bool batch_compute(vector<NeuronTree> nt_list, QList<double*> & feature_list, int method_code)
{
	V3DLONG VSIZE = 0;
	switch (method_code)
	{
		case 1:
			VSIZE = 21;
			break;
		case 2:
			VSIZE = 14;
			break;
		default:
			printf("ERROR: feature extraction method code unsupported.\n");
			return false;
	}


	printf("(1) calculate features.\n");
	V3DLONG neuronNum = nt_list.size();
	for (int i=0;i<neuronNum;i++)
	{
		printf("%d of %d\n",i, neuronNum);
		NeuronTree nt = nt_list[i];
		double * features = NULL;
		try 
		{
			features = new double[VSIZE];
		}
		catch (...)
		{
			if (features)  {delete []features; features=NULL;}
			return false;
		}
		if (method_code==1)
			computeFeature(nt, features);
		else if (method_code==2)
			computeGMI(nt, features);
		feature_list.append(features);
	}
	return true;
}

#endif
