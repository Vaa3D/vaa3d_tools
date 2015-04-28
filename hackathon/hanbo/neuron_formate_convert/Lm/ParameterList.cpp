#include "ParameterList.h"
#include <string.h>
#include <stdlib.h>
#include "Random.h"
#include <iostream>


using namespace std;

void ParameterList::readParameter(char * fileNam){
	int i,index,type;
	char  tmp[100],distType[20],paramName[50];
	double value[50];
	double percent;
	strcpy(fileName,fileNam);


	//stoer max type of tree
	int maxType=-1;
	//
	
	ifstream in(fileName);

    if (!in) {
		cerr << "Could not open:"<<fileName << endl;
		return;
    } 
    
	//until EOF
	while(in.peek()!=-1){
		//for every line
		//beginning of a line
		while ((in.peek()==10 || in.peek()==13 ))
			in.get();
		while (!(in.peek()==10 || in.peek()==13 || in.peek()==-1)){
			//reset value[50]
			for( i=0;i<50;i++)
				value[i]=0;
			index=0;
			//default value of percent
			percent=100;

			// read type (1:soma, 3:apical...)

			// GUIDO i=readNext(in, tmp);
			if(i==-1) break;
			// look it is an int
			type=atoi(tmp);
			
			// read Param Name
			// GUIDO i=readNext(in, paramName);
			if(i==-1) break;
			strlwr(paramName);

			//read Dist Type
			// GUIDO i=readNext(in, distType);
			if(i==-1) break;

			//read Param
			index=0;
			while(i!=-1){
                           // GUIDO i=readNext(in, tmp);
				//if eol or comment encounter before one value then go next line
				if(i==-1) break;

				//look if is a percent;
				if(strchr(tmp,37)!=NULL){
					percent=atof(tmp);
				}else {
					//otherwise store the value
					value[index]=atof(tmp);
					index++;
				}
			}
			// at least one parameter value is required!
			if(index<1) break;

			// create Parameter Object
			Parameter * p=new Parameter();
			p->setType(type);
			p->setName(paramName);
			p->setDist(distType);
			p->setPercent(percent);
			for(i=0;i<index;i++)
				p->addValue(value[i]);

			addParameter(p);
			if(type>maxType)
				maxType=type;
			
		}
	}
	//add MAXTYPE parameter
	//MAXTYPE: max value of the dendritic tree type in the all parameter file
	Parameter * p=new Parameter();
	p->setName(MAX_TYPE);
	p->setType(1);
	p->setDist("k");
	p->setPercent(100);
	p->addValue(maxType);
	addParameter(p);

}

int ParameterList::readNext(ifstream in,char * tmp){
	int tmp_lenght;
	//remove all leading spaces
	while(in.peek()==32) in.get();
	//check if end of line
	if((in.peek()==10 || in.peek()==13)) 
		return-1;

	in>>tmp;
	
	if(in.eof()) return -1;
	tmp_lenght=strlen(tmp);
	// look for comment sign
	if((int)strcspn(tmp,"#")<tmp_lenght){
		//found comment: read till the end and go to next line
		while(!(in.peek()==10 || in.peek()==13)) in.get();
		//remove end of line
		while((in.peek()==10 || in.peek()==13)) in.get();
		return -1;
	}
	return 1;


}
;


//return the value from the asked parameter
//
/// the file has to be in the form 
//treeType name distrType val1 val2 val3.... (anywhere in the val list there can be the % of distribution
// like
// 1 SOMA_DIAM k 10   #the % is assumed 100% if missing
// if the total % is more the 100% a error is reported

double ParameterList::getParameter(char * paramName){
	char *c,name[50];
	strcpy(name,paramName);

	strlwr(name);

	Parameter * tmp=par;
	Random * r;
	double rnd;
	rnd=r->rnd01();
	rnd*=100;
	double percent=0;
	double value=-1;
	int find=0;

	do{
		c=tmp->getName();
		//first check if same type
		if(tmp->getType()==type)
			if(strcmp(name,c)==0){
				//check 
				percent+=tmp->getPercent();
				if(rnd<percent && find==0){
					value= tmp->getValue();
					find=1;
				}
			}
		tmp=tmp->getNext();
	}while(tmp!=NULL);
	//check last one

	if(find==1 && percent<=100)
		return value;
	if(percent>0){
		//parameter found but total sum of mixed distribution less then 100%
		cerr<<"Param:"<<name<<" ERROR: mixed distribution of "<<percent<<"%! \n";
	}else {

		cerr<<"Param:"<<name<<" not FOUND! type:"<<type<<"\n";
	}
	return -1;
}
;

//look for parameterName and return 1 if found
int ParameterList::isPresent(char * paramName){
	char *c,name[50];
	strcpy(name,paramName);

	strlwr(name);

	Parameter * tmp=par;

	do{
		c=tmp->getName();
		//first check if same type
		if(tmp->getType()==type)
			if(strcmp(name,c)==0){
				return 1;
			}
			
		tmp=tmp->getNext();
	}while(tmp!=NULL);
	//check last one

	return 0;
}
;
