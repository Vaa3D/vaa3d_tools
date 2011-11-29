#include "Segment.h"
#include <iostream>
using namespace std;

#define VOID 1000000000 

Segment::Segment(){

	end=new Vector();
	id=VOID;
}

Segment::Segment(int ident,int typ, Vector* e, double rad, int pident){
	end= new Vector(e);
	
	radius=rad;
	//cerr<<"radius...in segment.cpp :"<<radius;
	type=typ;
	//cerr<<"type...in segmen.cpp :"<<type;
	id=ident;
	//cerr<<"id...in segmen.cpp :"<<id;
	pid=pident;
	//cerr<<"pid...in segmen.cpp :"<<pid;
	next1=NULL;
	next2=NULL;
	prev=NULL;
	prev2=NULL;
	prevFather=NULL;
	nextFather1=NULL;
	nextFather2=NULL;
	length=0;
	nrTip=0;
	
};

Segment * Segment::clone(){
	Vector * e=new Vector(end->getX(),end->getY(),end->getZ());
	Segment * s=new Segment(id,type,e,radius,pid);
	delete e;
	return s;
};
	

void Segment::polarRelativeToabsolute(){
	if(prev!=NULL){
		//except for soma
		double the=0;
			the=prev->getEnd()->getTheta();
		double phi=0;
			phi=prev->getEnd()->getPhi();
		double torch=0;
			torch=prev->getEnd()->getTorch();

		if(phi!=0)
			double rrr=phi;

		the+=end->getTheta();
		phi+=end->getPhi();
		torch+=end->getTorch();
		
		end->setPhi(phi);
		end->setTheta(the);
		end->setTorch(torch);
	}
class Segment;
	
	if(next1!=NULL){
		next1->polarRelativeToabsolute();
	} 
	if(next2!=NULL){
		next2->polarRelativeToabsolute();
	}

};
void Segment::polarToCartesian(){
	if(prev==NULL){
		end->polarToRelativeCartesian(0,0,0);
	}else{
		end->polarToRelativeCartesian(prev->getEnd()->getX(),prev->getEnd()->getY(),prev->getEnd()->getZ());
	}
	
	if(next1!=NULL){
		next1->polarToCartesian();
	} 
	if(next2!=NULL){
		next2->polarToCartesian();
	}
}


void Segment::addNewChild(Segment* s){

	if(next1==NULL){
		next1=s;
	
		s->setPrev(this);
	} else if(next2==NULL){
		next2=s;
	
		s->setPrev(this);
	} else  {

		Segment * virt;
		if(this->getType()==1){
			
			//we are in the soma
			virt=this->clone();
			//Ghost segment

			virt->setId(1);
			virt->setType(-1);
		
			Segment *tmp;
			
			tmp=this->getNext2();
		
			this->setNext2(virt);
			virt->setNext1(tmp);
			virt->setNext2(s);
						
			return;	
		}

	
		double one,two,three;
		Segment * s1, * s2;

		one=next1->getDiam();
		two=next2->getDiam();
		three=s->getDiam();
			
		Segment* toBeSplit=next1;
		s1=next2;
		
		s2=s;
	
		if(two>one && two>three) {
			
			toBeSplit=next2;
			
			s1=next1;
			
			s2=s;
		
		}
		if(three>two && three>one){
			
			toBeSplit=s;
		
			s1=next1;
		
			s2=next2;
		
		}


		//create a virtual segment to provide enough space for s
		
		virt=toBeSplit->clone();
	
		virt->setNext1(toBeSplit);
		toBeSplit->setPrev(virt);
		virt->setNext2(s1);
		s1->setPrev(virt);
		this->setNext1(virt);
		virt->setPrev(this);
		this->setNext2(s2);
		s2->setPrev(this);

		virt->adjustTriforc();
	}
}

//given a segment cloned due to triforcation adjust its end values!!
void Segment::adjustTriforc(){
	if(getType()<0 || getPid()<0)
		return;

	double x=0,y=0,z=0;
	//Extend father by double the size of the segment
	// get father end point

	Vector* p1=this->getPrev()->getEnd();
	Vector* p2=this->getEnd();
	//get coordinate from s end point
	double x1=p1->getX();
	double y1=p1->getY();
	double z1=p1->getZ();
	


	//get previous of father end point to extract direction of father grow
	// if father is soma (pid==1) then it hasn't' Prev, so just replicate point!!!
	double x2=p2->getX();
	double y2=p2->getY();
	double z2=p2->getZ();
	//adjust end point of s
	
	//evaluate direction vector
	
	double alpha=this->getDiam()*2/p1->distance(p2);
	
	if(alpha>0.5){
		//segment is long 2 diameters then set alpha to 0.5
		alpha=0.5;
	}
	if(p1->distance(p2)!=0 ){
		//interchanged x1,x2 to pick a point close to the p2 (towards the growth direction) not p1. 07/26/07 sri
		x=(x1-x2)*alpha+x2;
		y=(y1-y2)*alpha+y2;
		z=(z1-z2)*alpha+z2;
	} else {
		x=1.0001*x1;
		y=1.0001*y1;
		z=1.0001*z1;
	}
	cerr<<"new triforcation point at:"<<x<<" "<<y<<" "<<z<<endl;

	//set point
	p1=this->getEnd();
	p1->setX( x);
	p1->setY( y);
	p1->setZ( z);
	p1=NULL;p2=NULL;

};

Segment::~Segment(){
	delete (end);

	end=NULL;
	prev=NULL;
	prevFather=NULL;
	next1=NULL;
	next2=NULL;
	nextFather1=NULL;
	nextFather2=NULL;
};
