#ifndef ASC_TO_SWC_H
#define ASC_TO_SWC_H

#include <fstream>
#include <v3d_interface.h>

using namespace std;

class asc_to_swc
{
public:
    asc_to_swc(char* fname_asc, char* fname_swc);

private:
    char* name;
    ifstream* in;
    NeuronTree nt;
    double * getValues(double * ret);
    int NeurolAdd(int id, int type);
    void OpenNeuroL();
    void add(int id, int type, double x, double y, double z, double radius, int pid);
};

#endif // ASC_TO_SWC_H
