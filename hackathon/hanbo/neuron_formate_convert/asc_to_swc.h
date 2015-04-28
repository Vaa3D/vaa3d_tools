#ifndef ASC_TO_SWC_H
#define ASC_TO_SWC_H

#include <fstream>
#include <v3d_interface.h>

using namespace std;

class asc_to_swc
{
public:
    asc_to_swc(){}

public:
    static void readASC_file(NeuronTree &nt, char* fname_asc);

private:
    static double * getValues(ifstream * in, double * ret);
    static int NeurolAdd(NeuronTree &nt, ifstream * in, int id, int type);
    static void OpenNeuroL(NeuronTree &nt, char* fname_asc);
    static void add(NeuronTree &nt, int id, int type, double x, double y, double z, double radius, int pid);
};

#endif // ASC_TO_SWC_H
