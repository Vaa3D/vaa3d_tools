
#include "Segment.h"
#include <iostream>
using namespace std;
Segment::Segment(){

	//cerr<<"before defining vector";
	end=new Vector();
	//cerr<<"after defining vector";
	id=10;
}


double Segment::test()
{
	return 10.0;
}


Segment::Segment(int ident,int typ, Vector* e, double rad, int pident){
	end= new Vector(e);
	
	radius=rad;
	//cout<<"radius...in segment.cpp :"<<radius;
	type=typ;
	//cout<<"type...in segmen.cpp :"<<type;
	id=ident;
	//cout<<"id...in segmen.cpp :"<<id;
	pid=pident;
	//cout<<"pid...in segmen.cpp :"<<pid;
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

void Segment::addNewChild(Segment* s) {

	if(s->getType() == 1){
		cerr<<"adding type 1";
	}

	if (next1 == NULL) {
		next1 = s;

		s->setPrev(this);
	} else if (next2 == NULL) {
		next2 = s;

		s->setPrev(this);
	} else {

		Segment * virt;
		//s->getPid()!=-1 avoids adding yet another virtual segment to add a virtual segment. sridevi 12/9/2010. This condition allows only for multiple soma points to be added, if present.
		if (this->getType() == 1 && s->getPid() != -1) {
		//if (this->getType() == 1) {
			cerr << "addNewChild:" << this->getEnd()->getX() << ","
					<< this->getEnd()->getY() << "," << this->getEnd()->getZ()
					<< "+\n";
			cerr << s->getEnd()->getX() << "," << s->getEnd()->getY() << ","
					<< s->getEnd()->getZ() << "\n";

			if(s->getType() == 1){

				//we are in the soma
				virt = this->clone();
				//Ghost segment

				virt->setId(1);
				virt->setType(-1);

				//Add new code to bring the soma on the top.
				Segment *tmp1;
				Segment *tmp2;

				tmp1 = this->getNext1();
				tmp2 = this->getNext2();

				next1 = s;
				s->setPrev(this);
				next2 = virt;
				virt->setPrev(this);

				virt->setNext1(tmp1);
				tmp1->setPrev(virt);
				virt->setNext2(tmp2);
				tmp2->setPrev(virt);

				//New piece of code added to fix the problem of adding new somAS,
			}else{

				Segment *joinPoint = this;

				while(joinPoint->getNext2() != NULL){

					if(joinPoint->getNext2()->getType() != -1){
						break;
					}

					joinPoint = joinPoint->getNext2();
				}

				if(joinPoint->getNext2() == NULL){
					joinPoint->setNext2(s);
				}else{

					virt = this->clone();
					virt->setId(1);
					virt->setType(-1);

					Segment *tmp = joinPoint->getNext2();
					virt->setNext1(tmp);
					virt->setNext2(s);
					joinPoint->setNext2(virt);

				}
			}


//			tmp = this->getNext2();
//
//			this->setNext2(virt);
//			virt->setNext1(tmp);
//
//			//SG code added to ensure that the soma point is always added on the next 1.
//			if(s->getType() == 1){
//				virt->setNext2(this->getNext1());
//				this->setNext1(s);
//			}else{
//				virt->setNext2(s);
//			}
//
//			if(virt->getNext2()->getId() == 1){
//				tmp = virt->getNext1();
//				virt->setNext1(virt->getNext2());
//				virt->setNext2(tmp);
//			}
			//End of SG code



			return;
		}

		//Found  TRIFORCATION :
		// Given the three segments clone the thicker one
		// if next1 is bigger split it otherwise next2
		double one, two, three;
		Segment * s1, *s2;

		one = next1->getDiam();
		two = next2->getDiam();
		three = s->getDiam();
		//assuming that one is the largest diameter. Sridevi 12/10/2010
		Segment* toBeSplit = next1;
		s1 = next2;

		s2 = s;

		if (two > one && two > three) {
			//if the biggest is next1 ... otherwise it will be next2
			toBeSplit = next2;

			s1 = next1;

			s2 = s;

		}
		if (three > two && three > one) {
			//if s is the biggest switch and rename it next2
			toBeSplit = s;

			s1 = next1;

			s2 = next2;

		}

		//create a virtual segment to provide enough space for s

		virt = toBeSplit->clone();

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


//void Segment::addNewChild(Segment* s){
//
//	if(next1==NULL){
//		next1=s;
//
//		s->setPrev(this);
//	} else if(next2==NULL){
//		next2=s;
//
//		s->setPrev(this);
//	} else  {
//
//		Segment * virt;
//		if(this->getType()==1){
//
//			//we are in the soma
//			virt=this->clone();
//			//Ghost segment
//
//			virt->setId(1);
//			virt->setType(-1);
//
//			Segment *tmp;
//
//			tmp=this->getNext2();
//
//			this->setNext2(virt);
//			virt->setNext1(tmp);
//
//			//SG code added to ensure that the soma point is always added on the next 1.
//			if (s->getType() == 1) {
//				virt->setNext2(this->getNext1());
//				this->setNext1(s);
//			} else {
//				virt->setNext2(s);
//			}
//
//			if (virt->getNext2()->getId() == 1) {
//				tmp = virt->getNext1();
//				virt->setNext1(virt->getNext2());
//				virt->setNext2(tmp);
//			}
//			//End of SG code
//
//			return;
//		}
//
//
//		double one,two,three;
//		Segment * s1, * s2;
//
//		one=next1->getDiam();
//		two=next2->getDiam();
//		three=s->getDiam();
//
//		Segment* toBeSplit=next1;
//		s1=next2;
//
//		s2=s;
//
//		if(two>one && two>three) {
//
//			toBeSplit=next2;
//
//			s1=next1;
//
//			s2=s;
//
//		}
//		if(three>two && three>one){
//
//			toBeSplit=s;
//
//			s1=next1;
//
//			s2=next2;
//
//		}
//
//
//		//create a virtual segment to provide enough space for s
//
//		virt=toBeSplit->clone();
//
//		virt->setNext1(toBeSplit);
//		toBeSplit->setPrev(virt);
//		virt->setNext2(s1);
//		s1->setPrev(virt);
//		this->setNext1(virt);
//		virt->setPrev(this);
//		this->setNext2(s2);
//		s2->setPrev(this);
//
//		virt->adjustTriforc();
//	}
//}

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
	 alpha =0.5;
	}
	if(p1->distance(p2)!=0){
		//interchanged x1,x2 to pick a point close to the p2 (towards the growth direction) not p1. 07/26/07 sri
		x=(x1-x2)*alpha+x2;
		y=(y1-y2)*alpha+y2;
		z=(z1-z2)*alpha+z2;
	}else{
		x=1.0001*x1;
		y=1.0001*y1;
		z=1.0001*z1;
	}
	//cerr<<"new triforcation point at:"<<x<<" "<<y<<" "<<z<<endl;

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

////adding extra soma compartment to convert sphere to cylinder assumption
//		void Segment::addNewSomaChild(Segment* s){
//			//cerr<<s->getId()<<" "<<s->getEnd()->getX()<<" "<<s->getEnd()->getY()<<" "<<s->getEnd()->getZ()<<" "<<s->getPid()<<"\n";
//			if(next1==NULL){
//				next1=s;
//				s->setPrev(this);
//			} else if(next2==NULL){
//				next2=s;
//				s->setPrev(this);
//			} else  {
//				if(this->getType() ==1){
//
//					Segment * s1, * s2;
//					Segment * virt;
//
//					//next2 should be s.
//					//Segment* toBeSplit=next1;
//					//s1=next2;
//					//s2=s;
//
//					Segment* toBeSplit=this;
//					s1=next1;
//					s2=next2;
//
//				//create a virtual segment to provide enough space for s.
//				//make s as next1 and virt as next2 to soma and move next1 and next2 to virt. This arrangement will ensure that the new cylinder soma id is next to soma. sri 05/05/2011.
//					//this->setNext1(s2 );
//					this->setNext1(s);
//					//s2->setPrev(this);
//					s->setPrev(this);
//					virt=toBeSplit->clone();
//					virt->setType(-1);
//					//virt->setNext1(toBeSplit);
//					virt->setNext1(s1);
//					//toBeSplit->setPrev(virt);
//					s1->setPrev(virt);
//					//virt->setNext2(s1);
//					virt->setNext2(s2);
//					//s1->setPrev(virt);
//					s2->setPrev(virt);
//					this->setNext2(virt);
//					virt->setPrev(this);
//					//this->setNext2(s2);
//					//s2->setPrev(this);
//					//virt->adjustTriforc();
//				}
//			}
//		};


//SG version of add soma child.

//
//void Segment::addNewSomaChild(Segment* s) {
//	if (next1 == NULL) {
//		next1 = s;
//		s->setPrev(this);
//	} else if (next2 == NULL) {
//		next2 = s;
//		s->setPrev(this);
//	} else {
//
//		int toBeSplitChildNo = 0;
//
//		if(next1->getType() == 1 && next1->getType() != -1){
//
//			toBeSplitChildNo = 1;
//
//		}else{
//
//			toBeSplitChildNo = 2;
//		}
//
//		Segment* toBeSplit=this;
//		Segment * virt;
//		virt=toBeSplit->clone();
//		virt->setType(-1);
//
//		Segment * newChild1;
//		Segment * newChild2;
//
//		if(toBeSplitChildNo == 1){
//
//			newChild1 = toBeSplit->getNext1();
//			newChild2 = s;
//			toBeSplit->setNext1(virt);
//
//		}else if(toBeSplitChildNo == 2){
//
//			newChild2 = toBeSplit->getNext2();
//			newChild1 = s;
//			toBeSplit->setNext2(virt);
//		}
//
//		virt->setNext1(newChild1);
//		virt->setNext2(newChild2);
//		virt->setPrev(this);
//
//	}
//}

void Segment::addNewSomaChild(Segment* s) {

	if (next1 == NULL) {
		next1 = s;
		s->setPrev(this);
	} else if (next2 == NULL) {
		next2 = next1;
		next1 = s;
		s->setPrev(this);
	} else {

		Segment * virt = this->clone();
		virt->setType(-1);
		Segment * tempSeg;

		if(this->getNext1()->getType() == 1){
			tempSeg = this->getNext2();
			virt->setNext1(s);
			s->setPrev(virt);
			virt->setNext2(tempSeg);
			tempSeg->setPrev(virt);


		}else{
			tempSeg = this->getNext1();
			virt->setNext1(tempSeg);
			tempSeg->setPrev(virt);
			virt->setNext2(this->getNext2());
			this->getNext2()->setPrev(virt);
			this->setNext1(s);
			s->setPrev(this);

		}
		this->setNext2(virt);
		virt->setPrev(this);
	}
}
