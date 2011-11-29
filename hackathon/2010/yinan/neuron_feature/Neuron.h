#ifndef NEURON_H
#define NEURON_H

#include "Segment.h"
//#include "ParameterList.h"
#include "Random.h"
#include <string.h>
#include <fstream>
using namespace std;

class Segment;
class Func;
//class ParameterList;


class Neuron {

	//ParameterList* lp;

	ifstream* in;

	char* name;

	Func * a;
	//char* type;

	//int neuronalClass;

	bool cloned;


	char d[300][300];

	int neuronId;


	Random * rnd;

	bool oriented;

	double maxX,minX,maxY,minY,maxZ,minZ;

	int nrSeg;

	int rf;

	int flag;int ctr;

	//std::string name_str;
	//std::string dir_str;
	//std::string new_dir;
	char * tree_ext;

	char buff[30];
	int namectr;
	

	double kx,ky,kz;

	Segment * lookForSoma(Segment * s);


	void expExe(Segment* t,void* f);

	void renumerateId(Segment* s);

	Segment* findSegment(int id,Segment* father);
	Segment* buildFathers(Segment* start,Segment * prevFather);

	Func* findFunc(int n, Func* f);

	//void printSWC(Segment *s,ofstream & out );

	//void printDXF(Segment *s,ofstream & out);

	//void printDXFint(Segment *s,ofstream & out);

	//void printFatherDXF(Segment *s);

	//void printWRL(Segment *s, ofstream & out );

	//void printFatherLineWRL(Segment *s);

	//void printFatherWRL(Segment *s);

	//void printFatherIV(Segment *s);
	
	//void printLineWRL(Segment *s);
	
	//void printCylWRL(Segment *s);
	
	double* getValues(double * ret);
	
	//int LookForFileType(ifstream* in);
	
	void saveTree(Segment *s,ofstream & treeout);
	bool lookFor(char * string,ifstream* in);
	
	void translation(double x, double y, double z,Segment *s);
	
	void rotate(double x, double y, double z,Segment *s);
	
	void rotateAxis(Vector* a,double theta,Segment *s);
	
	void rotateP(Vector * P,Segment *s);
	
	void OpenSWC();
	
	//void OpenSEG();
	
	//void OpenANAT();
	
	//void OpenAMA();
	
	//void OpenASC();
	
	//void OpenNeuroL();
	
	void moveToMean(Segment * t);
	
	void multiply(Segment *soma,float **mat,float *avg,float *evals);
	
	int pca(Segment * t,float **data);
	
	int pcaRefill(Segment * t,float **data);
	
	void pcaStore(Segment * t,float **data);
	
	int NeurolAdd(int id, int type,double x0,double y0, double z0);
	
	double* addSomaClaiborne(double * ret);
	
	//void OpenClaiborne();
	//void OpenAmira();
	
	//void OpenPRM(ParameterList* lps);
	
	double cursor(Vector *v,Segment *t);
	
	double cursorAbs(Vector *v,Segment *t);
	
	void setParameters(Segment* s);
	
	void maxDim(Segment *t);
	
	//void printPGM(Segment *t,float ** data);
	
	void remove(Segment* s);
	
	Segment * clone(Neuron *n,Segment* s);
	
	void findDuplicate(Segment* s);
	
	Vector * lookNeuronForVirtual(Segment *t);


public:
	
	Neuron();
	
	~Neuron();
	
	Segment* soma;
	
	Segment* strtseg;
	
	int read;
	bool ferror;
	bool foundsoma;
	
	bool found;
	
	Neuron(char* fileName);
	
	//Neuron(ParameterList* lp);

	void setFunc(int funcNum);
	Func * getFunc(){return a;};
	
	void doPCA();
	void LookForTrees(Segment *s,std::string dirname);
	void processSkippedSegments(Segment* ss);
	void reduceSkippedSegments(Segment* r);
	void addSkippedSegments(Segment* firstseg, Segment * prev, Segment* p);
	
	void add(int id, int type, double x, double y, double z, double diam, int pid);

	int addPolar(int id, int type, double rho, double az, double ele, double torch,double diam, int pid);

	void addPolarSegment(int id, int type, double rho, double az, double ele, double torch,double diam, int pid);

	void translation(double x, double y, double z);

	void pca();

	void rotate(double x, double y, double z);

	void rotateP(Vector * P);

	void rotateTo(Vector* a);

	void setCloned(){cloned=1;};



	void addSegment(Segment* s);

	void exploreExecute(void* f);

	void setShrinkX(double i){kx=i;};

	void setShrinkY(double i){ky=i;};

	void setShrinkZ(double i){kz=i;};

	void setOrder(Segment* s);

	void addVirtualSoma(Segment* s);
	
	Segment * rearrangeSWC(Segment* s);
	
	void rearrange(Segment* s, Segment* t);

	void setParameters();

	void setDefaults();

	void renumerateId();

	void orientTrans(Vector* o, Vector *p, Vector *c);

	void orientTrans1(Vector* o, Vector *p, Vector *c);

	//void setNeuronalClass(int x){neuronalClass=x;}

	//int getNeuronalClass(){return neuronalClass;}

	//int growBurke(double diameter,int type, int pid);

	//int growTamori(double diameter,int type, int pid);

	//int growHillman(double diameter,int type, int pid);



	double getShrinkX(){return kx;};

	double getShrinkY(){return ky;};

	double getShrinkZ(){return kz;};

	void adjustShrinkage(Segment* s);

	//void electrotonic(Segment * t,double rm,double ri, int type);

	//void electrotonic(Segment * t,double rm,double ri, int type,double xOff,double yOff, double zOff);

	void setNeuronId(int i){neuronId=i;};

	int getNeuronId(){return neuronId;};

	Segment* getSoma(){return soma;};

	void setName(char* nam){name= new char[100];strcpy(name,nam);};

	char* getName();

	void setType(char* type);

	char* getType();

	//void printSWC();

	//void printDXF();

	//void printDXF(Segment * s, Segment *t, ofstream & out);

	//void printFatherDXF();

	//void printWRL();

	//void printFatherLineWRL();

	//void printFatherWRL();

	//void printFatherIV();

	//void printLineWRL();

	//void printCylWRL();

	//void printPGM();

	void setSoma(Segment * t){ soma=t;};

	Neuron * clone();

	void init();


}

;

#endif
