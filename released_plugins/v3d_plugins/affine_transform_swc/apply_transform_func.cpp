#include "apply_transform_func.h"
#include "basic_surf_objs.h"
#include <math.h>
#include <iostream>
#include "io_affine_transform.h"


using namespace std;

#define PI 3.14159265

Matrix affine_matrix(unit_vector u, double angle, double shift_x, double shift_y, double shift_z)
{


    double radians = angle * PI / 180.0 ;  // from degree to radian


    double c = cos( radians ) ;
    double l_c = 1 - c ;
    double s = sin( radians ) ;

    Matrix affineMatrix3by4(3,4);


    affineMatrix3by4(1,1) = u.x*u.x + (1 - u.x*u.x)*c ;
    affineMatrix3by4(1,2) = u.x*u.y*l_c - u.z*s;
    affineMatrix3by4(1,3) = u.x*u.z*l_c + u.y*s;
    affineMatrix3by4(1,4) = shift_x;
    affineMatrix3by4(2,1) = u.x*u.y*l_c + u.z*s;
    affineMatrix3by4(2,2) = u.y*u.y+(1 - u.y*u.y)*c ;
    affineMatrix3by4(2,3) = u.y*u.z*l_c - u.x*s ;
    affineMatrix3by4(2,4) = shift_y ;
    affineMatrix3by4(3,1) = u.x*u.z*l_c - u.y*s ;
    affineMatrix3by4(3,2) = u.y*u.z*l_c + u.x*s;
    affineMatrix3by4(3,3) = u.z*u.z + (1 - u.z*u.z)*c ;
    affineMatrix3by4(3,4) = shift_z ;


    return affineMatrix3by4;

}



Matrix translate_matrix(double shift_x, double shift_y, double shift_z)
{


    Matrix affineMatrix3by4(3,4);


    affineMatrix3by4(1,1) = 1;
    affineMatrix3by4(1,2) = 0;
    affineMatrix3by4(1,3) = 0;
    affineMatrix3by4(1,4) = shift_x;
    affineMatrix3by4(2,1) = 0;
    affineMatrix3by4(2,2) = 1;
    affineMatrix3by4(2,3) = 0;
    affineMatrix3by4(2,4) = shift_y;
    affineMatrix3by4(3,1) = 0;
    affineMatrix3by4(3,2) = 0;
    affineMatrix3by4(3,3) = 1;
    affineMatrix3by4(3,4) = shift_z;

    return affineMatrix3by4;

}

double get_scale_from_trans(Matrix trans){

   //extract the sale parameter from rotation matrix
    Matrix rMatrix3by3(3,3);
   rMatrix3by3(1,1) = trans(1,1);
   rMatrix3by3(1,2) = trans(1,2);
   rMatrix3by3(1,3) = trans(1,3);

   rMatrix3by3(2,1) = trans(2,1);
   rMatrix3by3(2,2) = trans(2,2);
   rMatrix3by3(2,3) = trans(2,3);

   rMatrix3by3(3,1) = trans(3,1);
   rMatrix3by3(3,2) = trans(3,2);
   rMatrix3by3(3,3) = trans(3,3);


   double det = rMatrix3by3.determinant();

   double s = pow(det, 1.0/3.0);
   return s;
}


NeuronTree apply_transform(NeuronTree * nt, Matrix  trans )
{
    NeuronTree result;
    V3DLONG size = nt->listNeuron.size();
    //cout<<"size="<<size<<endl;

    double scale = get_scale_from_trans(trans);

    for (V3DLONG i = 0;i < size;i++)
    {
        NeuronSWC s = nt->listNeuron[i];
        NeuronSWC p = s;
        // apply the 3x4 transform matrix


        p.x = trans(1,1) * s.x + trans(1,2) * s.y + trans(1,3) * s.z + trans(1,4) ;
        p.y = trans(2,1) * s.x + trans(2,2) * s.y + trans(2,3) * s.z + trans(2,4) ;
        p.z = trans(3,1) * s.x + trans(3,2) * s.y + trans(3,3) * s.z + trans(3,4) ;
      
        p.r = s.r *scale;

                
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
