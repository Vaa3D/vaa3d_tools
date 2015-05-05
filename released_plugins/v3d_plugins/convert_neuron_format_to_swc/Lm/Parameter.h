#ifndef PARA_H
#define PARA_H

#include <fstream>
#include <string.h>
#include "Random.h"

#define METHOD "METHOD"
#define SOMA_DIAMETER "SOMADIAM"
#define MAX_TYPE "MAX_TYPE"
#define NUM_TREE "NUM_TREE"
#define CONSLENGTH "CONSLENGTH"
#define TREE_AZY "TREEAZIM"
#define TREE_ELEV "TREEELEV"
#define INIT_DIAM "INITDIAM"
#define CONSLENGTH "CONSLENGTH"
#define TAPER "TAPER"
#define THRESHOLD "THRESHOLD"
#define BIFRATIO "BIFRATIO"
#define BIFORIENT "BIFORIENT"
#define BIFPOWER "BIFPOWER"
#define BIFAMPLI "BIFAMPLI"
#define TERMLENGTH "TERMLENGTH"
#define PK "PK"
#define FRAGMENTATION "FRAGMENTATION"
#define CONTRACTION "CONTRACTION"
using namespace std;
//BURKE
#define LENGTH "LENGTH"
#define K1_OVERLAP "K1_OVERLAP"
#define K2_OVERLAP "K2_OVERLAP"
#define K1_NONOVERLAP "K1_NONOVERLAP"
#define K2_NONOVERLAP "K2_NONOVERLAP"
#define K1_TERMINATE "K1_TERMINATE"
#define K2_TERMINATE "K2_TERMINATE"
#define GAUSS_BRANCH "GAUSS_BRANCH"
#define LINEAR_BRANCH "LINEAR_BRANCH"
#define EXTEND_AZIMUTH "EXE_AZI"
#define EXTEND_ELE "EXE_ELE"



char *strlwr( char *string);

//add always a parameter called BOOLEAN return random between 0 and 1;
class Parameter{
	int type;
	Random * rnd;
	char name[20],distr[20];
	
	double val[20];
	
	int nextValue;
	
	double percent;
	
	Parameter * nextPar;

public:
	
	Parameter(){rnd=new Random();nextValue=0;nextPar=NULL;percent=0;};

	
	void setName(char *parName){
		strcpy(name,parName);
		strlwr(name);
	};

	
	char * getName(){return name;};

	
	void setNext(Parameter * p){ nextPar=p;};
	
 	Parameter * getNext(){return nextPar;};

	
	void setType(int i){type=i;}
	
	int getType(){return type;}
	
	void setPercent(double p){percent=p;}
	
	double getPercent(){return percent;}
	
	void addValue(double value);
	
	double getValue();
	
	
	void setDist(char *dist){strcpy(distr, dist); };
	
	char * getDist(){return distr;};
	
}
;

#endif
