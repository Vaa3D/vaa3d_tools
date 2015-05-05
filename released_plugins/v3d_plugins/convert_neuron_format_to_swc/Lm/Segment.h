
#ifndef SEGMENT_H
#define SEGMENT_H

#include "Vector.h"
#include "iostream"
#include "Neuron.h"
#include <cstdlib>
using namespace std;
;

class Segment {

	
	Vector* end;

	double radius;
	char* neuronname;


	double length;

	double somaDistance;

	double pathDistance;

	int order;

	int type;

	int id;

	int pid;

	int nClass;

	int nrTip;

	
	Segment* prev;

	Segment* prev2;

	Segment* prevFather;

	Segment* next1;

	Segment* next2;

	Segment* nextFather1;

	Segment* nextFather2;

public:

	Segment();

	~Segment();

	Segment(int ident,int typ, Vector* e, double radius, int pident);
	void setNeuronName(char* name) {neuronname = name;}
	
	void setEnd(Vector a){	end=new Vector(a);};

	void setRadius(double ra){	radius=ra;};



	void setNrTip(int t){nrTip=t;}

	void setType(int t){type=t;}

	void setId(int i){id=i;}

	void setPid(int p){pid=p;}

	void setOrder(int p){order=p;}

	void setLength(double p){length=p;}

	void setSomaDistance(double p){somaDistance=p;}

	void setPathDistance(double p){pathDistance=p;}

	void setClass(int i){nClass=i;}

	int getClass(){return nClass;}

	int getNrTip(){return nrTip;}

	Vector* getEnd(){return end;};

	double getDiam(){return (2*radius);};

	double getRadius(){return radius;};

	int getType(){return type;}

 	int getId(){return id;}
	char* getNeuronName() {return neuronname;}
	
	int getPid(){return pid;}

	int isFather(){
		
		if(getPid()==-1)
			return 1;
		return (next1!=NULL && next2!=NULL); 
	};

	int isTerminate(){return (next1==NULL && next2==NULL); };

	double getTaper();

	double getLength(){return length;}

	double getSomaDistance(){return somaDistance;}

	double getPathDistance(){return pathDistance;}

	int getOrder(){return order;};
	
	void polarToCartesian();

	void polarRelativeToabsolute();

	//Recursion flag used to avoid recursion
	int avoidRecursiveId;

	double test();

	void addNewChild(Segment* s);

    void adjustTriforc();

	Segment* getPrev(){return prev;};

	Segment* getPrev2(){return prev2;};
	
	Segment* getPrevFather(){return prevFather;};
	
	Segment* getNextFather1(){return nextFather1;};
	
	Segment* getNextFather2(){return nextFather2;};
	
	void setPrev(Segment* s){prev=s;};
	
	void setPrev2(Segment* s){prev2=s;};
	
	void setPrevFather(Segment* s){
		prevFather=s;
		
	};
	
	void setNext1(Segment* s){next1=s;};
	
	void setNext2(Segment* s){next2=s;};
	
	void setNextFather1(Segment* s){nextFather1=s;};
	
	void setNextFather2(Segment* s){nextFather2=s;};
	
	Segment* getNext1(){ return next1;};
	
	Segment* getNext2(){ return next2;};
	
	Segment* getNonVirtualNext1(){

		if(next1->getType() != -1){
			return next1;
		}else if(next1->getNext1()->getType() != -1)
			return next1->getNext1();
		else return next1->getNext2();

	}

	Segment* getNonVirtualNext2(){

		if(next2->getType() != -1){
			return next2;
		}else if(next2->getNext1()->getType() != -1)
			return next2->getNext1();
		else return next2->getNext2();

	}

	Segment* clone(); 

	void addNewSomaChild(Segment* s);
	
	void setRecurseId(int i){
		id=i;
		if(next1==this)
			next1=NULL;
		if(next1!=NULL)
			next1->setRecurseId(i);
		if(next2!=NULL)
			next2->setRecurseId(i);
		
		if(prev!=NULL){
			if(prev->getId()!=id){
		
				Segment * t=prev;
				while( t->getPrev()!=NULL)
					t=t->getPrev();
				t->setRecurseId(i);
			}
		}
	}
}

;
#endif

