#include "apply_transform_func.h"
#include "basic_surf_objs.h"
#include <math.h>
#include <iostream>
#include "io_affine_transform.h"


using namespace std;

NeuronTree apply_transform(NeuronTree * nt, Matrix  trans )
{
        NeuronTree result;
	V3DLONG size = nt->listNeuron.size();
	for (V3DLONG i = 0;i < size;i++)		
        {
                NeuronSWC s = nt->listNeuron[i];
		NeuronSWC p = s;
                // apply the 3x4 transform matrix


		p.x = trans(1,1) * s.x + trans(1,2) * s.y + trans(1,3) * s.z + trans(1,4) ;
		p.y = trans(2,1) * s.x + trans(2,2) * s.y + trans(2,3) * s.z + trans(2,4) ;
                p.z = trans(3,1) * s.x + trans(3,2) * s.y + trans(3,3) * s.z + trans(3,4) ;
                
                
		result.listNeuron.push_back(p);
	}
   
	return result;


}

bool apply_transform_to_swc(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	vector<char*>* outlist = NULL;
	vector<char*>* paralist = NULL;

	if(input.size() != 2) 
	{
		printf("Please specify both input file and affine transform file.\n");
		return false;
	}
	paralist = (vector<char*>*)(input.at(1).p);
	if (paralist->size()!=1)
	{
		printf("Please specify the affine transform file.\n");
		return false;
	}
	QString transformFileName = QString(paralist->at(0));

	QString fileOpenName = QString(inlist->at(0));
        
	QString fileSaveName;
	if (output.size()==0)
	{
		printf("No outputfile specified.\n");
		fileSaveName = fileOpenName + "_transformed.swc";
	}
	else if (output.size()==1)
	{
		outlist = (vector<char*>*)(output.at(0).p);
		fileSaveName = QString(outlist->at(0));
	}
	else
	{
		printf("You have specified more than 1 output file.\n");
		return false;
	}

	NeuronTree nt;
	if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
		nt = readSWC_file(fileOpenName);

        Matrix affineMatrix3by4 = readTransform(transformFileName);
        
        /* test with identity matrix
        Matrix affineMatrix3by4(3,4);
        affineMatrix3by4(1,1) = 1 ;
        affineMatrix3by4(1,2) = 0 ;
        affineMatrix3by4(1,3) = 0 ;
        affineMatrix3by4(1,4) = 0 ;
        affineMatrix3by4(2,1) = 0 ;
        affineMatrix3by4(2,2) = 1 ;
        affineMatrix3by4(2,3) = 0 ;
        affineMatrix3by4(2,4) = 0 ;
        affineMatrix3by4(3,1) = 0 ;
        affineMatrix3by4(3,2) = 0 ;
        affineMatrix3by4(3,3) = 1 ;
        affineMatrix3by4(3,4) = 0 ;
        */
	NeuronTree result = apply_transform(&nt,affineMatrix3by4);

	if (writeSWC_file(fileSaveName, result) ){
		return true;
	}
	else {
		return false;
	}
}
