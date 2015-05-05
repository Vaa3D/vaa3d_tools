#ifndef PARALIST_H
#define PARALIST_H


#include "Parameter.h"


class ParameterList {
private:

	char fileName[100];

	Parameter * par;

	int indexPar;

	int type;

	int readNext(ifstream in,char * tmp);

public:

	ParameterList(){
		indexPar=0; par=NULL;
	}

	void readParameter(char * fileName);
	
	double getParameter(char * paramName);

	
	char * getFileName(){return fileName;}
	
	void setType(int i){type=i;}
	
	int getType(){return type;}
	
	int isPresent(char * paramName);
	
	void addParameter(Parameter *  p){
		if(par==NULL){
			par=p;
			return;
		}
		Parameter * tmp;
		tmp=par;
		while (tmp->getNext()!=NULL)
			tmp=tmp->getNext();
		tmp->setNext(p);
	}
	
}

;


#endif
