
#ifndef LIMIT_H
#define LIMIT_H
#include "RString.h"
#include "Segment.h"


class Func;
;

//determine how to limit the scope of funzion

class Limit{
protected:

	char* name;
	
	Limit * prev;

public:
	
  Limit();
	
  Limit(char* m,Limit* pr);
  
	
  int virtual okSegment(Segment *t);
	
  char *  print();

	
  virtual char * getName();

 

  double virtual getValue();

  char * getParameters();

  int checkLimit(Segment *t);

} ;



class OrderLimit: public Limit
{
  
public:

  int order;

	OrderLimit( int ciao, Limit * prev = NULL )
	{	 
	 Limit( "Order", prev);
	 order=ciao;
	}
  

  double getValue();

  int okSegment(Segment *t);

  char * getName();
	
  char * getParameters();

};



class TipLimit: public Limit{
public:

	TipLimit(int i=0,Limit * prev=0):Limit("TipLimit",prev){}


	int okSegment(Segment *t){
		
		if(t->getNext1()==0 && t->getNext2()==0){
			Segment *tmp=t;
			return 1;
		}
		return 0;
	}

	char * getName(){return (new RString(name,getParameters()))->print();}

	char * getParameters(){	
		return "";
	}
}
;

class ConsolidatedFatherLimit: public Limit{

public:

	ConsolidatedFatherLimit(Limit * prev=0):Limit("Order",prev){}

	int okSegment(Segment *t){

		if(t->getType()==3 && ( t->getNrTip()>1 ) &&t->isFather())
			return 1;
		return 0;
	}

	char * getName(){return (new RString(name,getParameters()))->print();}

	char * getParameters(){	
		return "";
	}
}
;

class TypeMinLimit: public Limit{
protected:
	
	int pid2;
public:
	
	TypeMinLimit(int j,Limit * prev=0):Limit("TypeMin",prev){pid2=j;}

	
	int okSegment(Segment *t){
		if(t->getType()>=pid2)
			return 1;
		return 0;
	}	

	
	char * getName(){return (new RString(name,getParameters()))->print();}
	
	char * getParameters(){	
		
		return (new RString(pid2))->print();
	}
}
;

class TypeFatherMinLimit: public Limit{
protected:
	
	int pid2;
public:
	
	TypeFatherMinLimit(int j,Limit * prev=0):Limit("TypeMin",prev){pid2=j;}

	
	int okSegment(Segment *t){
		if(!(t->isFather())) return 0;
		if(t->getType()>=pid2)
			return 1;
		return 0;
	}	

	
	char * getName(){return (new RString(name,getParameters()))->print();}
	
	char * getParameters(){	
		return (new RString(pid2))->print();
	}
}
;


class TypeLimit: public Limit{
protected:
	
	int pid1;
	
	int pid2;
	
public:
	
	TypeLimit(int i=0, Limit * prev=0):Limit("Type",prev){;pid1=i;pid2=0;}

	
	int okSegment(Segment *t){
		if(pid1==0 && pid2==0) return 1;
		
		if(pid1!=0){
			if(t->getPrev()!=NULL)
				if(t->getPrev()->getType()==pid1)
					return 1;
		}
		return 0;
		
	}	

	
	char * getName(){return (new RString(name,getParameters()))->print();}
	
	char * getParameters(){	
		RString* a1=new RString(pid1);
		a1->add(":");
		a1->add(pid2);
		return a1->print();
	}
}
;

class TypeFatherLimit: public Limit{
protected:

	int pid1;

	int pid2;

public:
	
	TypeFatherLimit(int i=0, Limit* prev=0):Limit("Type",prev){;pid1=i;pid2=0;}


	int okSegment(Segment *t){

		if(!(t->isFather())) return 0;
		if(pid1==0 && pid2==0) return 1;
		if(t->getType()==pid2){
			if(pid1!=0){
				if(t->getPrev()!=NULL)
					if(t->getPrev()->getType()==pid1)
						return 1;
				return 0;
			}
			return 1;
		}
		return 0;
	}	


	char * getName(){return (new RString(name,getParameters()))->print();}

	char * getParameters(){	
		double a=pid1+(pid2/10.0);
		return "";
	}
}
;

#endif
