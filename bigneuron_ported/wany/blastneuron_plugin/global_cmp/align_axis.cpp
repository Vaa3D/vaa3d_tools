#include "align_axis.h"
#include "pca1.h"
#include "basic_surf_objs.h"

NeuronTree align_axis(NeuronTree input)
{
	NeuronTree result = input;
	V3DLONG siz = input.listNeuron.size();
	printf("size=%d\n", siz);
	
	double * coord = new double[siz * 3];
	for (V3DLONG i=0; i<siz; i++)
	{
		coord[i] = input.listNeuron[i].x;
		coord[siz + i] = input.listNeuron[i].y;
		coord[2*siz + i] = input.listNeuron[i].z;
	}


	double * coord_pca = new double[siz * 3];
	double * PC = new double[siz];
	double V[3];

	pca1(coord, 3, siz, coord_pca, PC, V);

	printf("PCA done\n");

	for (V3DLONG i=0;i<siz; i++)
	{
		result.listNeuron[i].x = coord_pca[i];
		result.listNeuron[i].y = coord_pca[siz + i];
		result.listNeuron[i].z = coord_pca[2*siz + i];
	}

	delete []coord;
	delete []coord_pca;
	delete []PC;
	
	return result;
}
