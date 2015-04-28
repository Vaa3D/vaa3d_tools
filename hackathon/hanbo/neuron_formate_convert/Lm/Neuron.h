#ifndef NEURON_H
#define NEURON_H

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#define OS_WIN
#endif

#include "Segment.h"
#include <fstream>
#include "ParameterList.h"
#include "Random.h"
#include <string>
using namespace std;

class Segment;
class ParameterList;


class Neuron {

	ParameterList* lp;

	ifstream* in;

	char* name;

	char* type;

	int neuronalClass;
	
	bool cloned;


	char d[300][300];

	ifstream* in_d;

	int neuronId;


	Random * rnd;
	
	bool oriented;

	double maxX,minX,maxY,minY,maxZ,minZ;
	
	int nrSeg;
	
	int rf;
	
	int flag;int ctr;int LookForTreesFlag;
	
	std::string name_str;
	std::string dir_str;
	std::string new_dir;
	std::string new_dir_parent;
	char * tree_ext;
	
	char buff[30];
	int namectr;

	
	double kx,ky,kz;

	Segment * lookForSoma(Segment * s);


	void expExe(Segment* t,void* f);

	void renumerateId(Segment* s);

	Segment* findSegment(int id,Segment* father);
	Segment* buildFathers(Segment* start,Segment * prevFather);
	void printSWC(Segment *s,ofstream & out );

	void printDXF(Segment *s,ofstream & out);

	void printDXFint(Segment *s,ofstream & out);

	void printFatherDXF(Segment *s);

	void printWRL(Segment *s, ofstream & out );

	void printFatherLineWRL(Segment *s);

	void printFatherWRL(Segment *s);

	void printFatherIV(Segment *s);
	
	void printLineWRL(Segment *s);
	
	void printCylWRL(Segment *s);
	
	double* getValues(double * ret);
	
	int LookForFileType(ifstream* in);
	
	void saveTree(Segment *s,ofstream & treeout);
	bool lookFor(char * string,ifstream* in);
	
	void translation(double x, double y, double z,Segment *s);
	
	void rotate(double x, double y, double z,Segment *s);
	
	void rotateAxis(Vector* a,double theta,Segment *s);
	
	void rotateP(Vector * P,Segment *s);
	
	void OpenSWC();
	
	void OpenSEG();
	
	void OpenANAT();
	
	void OpenAMA();
	
	void OpenASC();
	
	void OpenNeuroL();
	
	void moveToMean(Segment * t);
	
	void multiply(Segment *soma,float **mat,float *avg,float *evals,int zres);
	
	int hasDendriteType(Segment *t);

	int pcaDataPoints(Segment * t,float **data,int dendriteOnly);
	
	int pcaRefill(Segment * t,float **data);
	
	void pcaStore(Segment * t,float **data);
	
	int NeurolAdd(int id, int type,double x0,double y0, double z0);
	
	double* addSomaClaiborne(double * ret);
	
	void OpenClaiborne();
	void OpenAmira();
	
	void OpenPRM(ParameterList* lps);
	double zresolutionEstimator(Segment *t);
	//double distFromPointsToCOM(Segment*t, Vector *com, int dendriteOnly);
	int saveZcoords(bool flg, Segment *t, float **datcopy);
	int setZcoords(bool flg, int ncompartments, Segment *t, float **dat);
	
	double cursor(Vector *v,Segment *t,int dendriteOnly);
	
	double cursorAbs(Vector *v,Segment *t,int dendriteOnly);
	
	void setParameters(Segment* s);
	
	void maxDim(Segment *t);
	
	void printPGM(Segment *t,float ** data);
	
	int remove(Segment* s);
	
	Segment * clone(Neuron *n,Segment* s);
	
	void findDuplicate(Segment* s);
	//Adding the new function to avoid recursion
	void findDuplicate1(Segment* s);
	
	Vector * lookNeuronForVirtual(Segment *t);


public:
	
	Neuron();
	
	~Neuron();
	
	Segment* soma;
	
	Segment* strtseg;
	
        bool ferror;
	bool foundsoma;	
	
	bool found;
	
	//Neuron(char* fileName, int neurClass=-1);
	Neuron(char* fileName, int neurClass=-1, bool isDiaPresent=false, char* dia_n=NULL);
	
	Neuron(ParameterList* lp);
	
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

	void zeroing();

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

	void setNeuronalClass(int x){neuronalClass=x;}

	int getNeuronalClass(){return neuronalClass;}
	
	int growBurke(double diameter,int type, int pid);

	int growTamori(double diameter,int type, int pid);

	int growHillman(double diameter,int type, int pid);



	double getShrinkX(){return kx;};

	double getShrinkY(){return ky;};

	double getShrinkZ(){return kz;};

	void adjustShrinkage(Segment* s);
	void computeBranch(Segment* s);
	
	void electrotonic(Segment * t,double rm,double ri, int type);

	void electrotonic(Segment * t,double rm,double ri, int type,double xOff,double yOff, double zOff);

	void setNeuronId(int i){neuronId=i;};

	int getNeuronId(){return neuronId;};

	Segment* getSoma(){return soma;};

	void setName(char* nam){name= new char[100];strcpy(name,nam);};

	char* getName();

	void setType(char* type);

	char* getType();

    void printSWC(char* fname);

	void printSWC();

	void printDXF();

	void printDXF(Segment * s, Segment *t, ofstream & out);

	void printFatherDXF();

	void printWRL();

	void printFatherLineWRL();

	void printFatherWRL();

	void printFatherIV();

	void printLineWRL();

	void printCylWRL();

	void printPGM();

	void setSoma(Segment * t){ soma=t;};

	Neuron * clone();
	
	void init();
	
	double round2Digits(double num);

	char * getFileName(char * filePath);

	void addCylinderSoma(Segment* s);


	char * dia;
	bool isDiaSet;
	void setDia(char * dia_i){
		dia = dia_i;
	}
	void setIsDiaSet(bool diaSet){
		isDiaSet = diaSet;
	}

	void rearrangeArray(double (*connarr), int connarr_size);
	bool idExistsInArray(double (*connarr), int id, int findTill);
	void reversePlaces(double (*connarr), int reverseFrom, int reverseTo);
	void printArray(double(*connarr), int connarr_size);



}

;

#endif
