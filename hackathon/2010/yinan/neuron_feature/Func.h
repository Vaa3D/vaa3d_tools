#ifndef FUNC_H
#define FUNC_H

#include "Neuron.h"
#include "pca.h"
#include "Segment.h"
#include "math.h"
#include "stdlib.h"
#include "Limit.h"
#include <assert.h> 
#include <iostream>
using namespace std;

#define VOID 1000000000
#define TH_DIM 0.95
#define PI 3.14159265359
;

class Func{
	protected:
		double preMean;
		Func * pre;
		char* name;
		Func *limit;
	public:
		double virtual computeStep(Segment *t){return VOID;};

		Func(){name="<NoName>";limit=0;pre=this;};
		Func(char* m){
			name=m;limit=0;pre= this;
		}
		Func(char*m, Func* l){name=m;limit=l;}
		Func(char*m, Func* preElaboration, Func* l){name=m;limit=l;pre=preElaboration;}

		void virtual initialize(double preMean1) {preMean=preMean1;};
		virtual Func *  getPreFunc(){ return pre;}

		//a value great than MAX is considered NULL
		double compute(Segment *t){
			//if no limit
			if(limit==0)
				return computeStep(t);
			//otherwise check limit
			if (limit->computeStep(t)!=0 )
				return computeStep(t);

			return VOID;	
		};


		char * getName(){return name;}
		char * getLimit(){if (limit==0) return "";return limit->getName();}
		void setName(char* m){name=m;}
}
;

class Equal:public Func{
	protected:
	public:
		Func * a;
		double value;

		Equal(Func * b, double d,Func* lim=0):Func("Lim",lim){
			a=b;
			value=d;
		};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){
			if(a->computeStep(t)==value)
				return 1;
			return 0;
		}
}
;
class Greater:public Func{
	protected:
	public:
		Func * a;
		double value;

		Greater(Func * b, double d,Func* lim=0):Func("Lim",lim){
			a=b;
			value=d;
		};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){
			if(a->computeStep(t)>value)
				return 1;
			return 0;
		}
}
;
class Lesser:public Func{
	protected:
	public:
		Func * a;
		double value;

		Lesser(Func * b, double d,Func* lim=0):Func("Lim",lim){
			a=b;
			value=d;
		};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){
			if(a->computeStep(t)<value)
				return 1;
			return 0;
		}
}
;
//And: if both function return a value different than zero then they return 1 otherwise 0
class And:public Func{
	protected:
	public:
		Func * a;
		Func *b;

		And(Func * A,Func * B, Func* lim=0):Func("Lim",lim){
			a=A;b=B;
		};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){
			if(a->computeStep(t)!=0 && b->computeStep(t)!=0)
				return 1;
			return 0;
		}
}
;

//Or: if one function return a value different than zero then return 1 otherwise 0
class Or:public Func{
	protected:
	public:
		Func * a;
		Func *b;

		Or(Func * A,Func * B, Func* lim=0):Func("Lim",lim){
			a=A;b=B;
		};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){
			if(a->computeStep(t)!=0 || b->computeStep(t)!=0)
				return 1;
			return 0;
		}
}
;

//store Class Info for Machine Learning Algorithm
class N_Class:public Func{
	int c;
	public:

	N_Class(Func* lim=0):Func("N_Class",lim){c=-1;};
	Func *  getPreFunc(){ return this;}

	double  computeStep(Segment *t){
		if(t->getClass()!=-1){
			Segment * t1=t;
			while(t1->getPrev()!=NULL)
				t1=t1->getPrev();
			c=t1->getClass();
		}
		return c;
	}
}
;
class Diameter:public Func{
	public:

		Diameter(Func* lim=0):Func("Diameter",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){
			if(t==NULL) return VOID;
			return t->getDiam();
		}

}
;
class Diameter_pow:public Func{
	public:


		Diameter_pow(Func* lim=0):Func("Diameter_pow",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){
			if(t==NULL) return VOID;
			return pow(t->getDiam(),1.5);
		}

}
;
class Type:public Func{
	public:

		Type(Func* lim=0):Func("Type",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){
			return t->getType();}

}
;

class N_stems:public Func{
	public:

		N_stems(Func* lim=0):Func("N_stems",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 

			Segment * p=t;

			if(t->getPrev()!=NULL){




				if(t->getType()!=t->getPrev()->getType() && t->getType()>1 && (t->getPrev()->getType()== -1 || t->getPrev()->getType()==1)){

					return 1;
				}
			}

			return VOID;
		}
}
;
class N_branch:public Func{
	public:

		N_branch(Func* lim=0):Func("N_branch",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 

			if(t->isFather() || t->isTerminate())

				return 1;
			return VOID;
		}
}
;
//changing the code for N_tips to TerminalSegment
class TerminalSegment:public Func{
	public:

		TerminalSegment(Func* lim=0):Func("TerminalSegment",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 


			if(t->getNrTip()==1)
				return 1;
			return VOID;
		}
}
;
class Length:public Func{
	public:

		Length(Func* lim=0):Func("Length",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			double lenght=0;
			if( t->getPid()!=-1)
				lenght=t->getEnd()->distance(t->getPrev()->getEnd());

			if(lenght>0) return lenght;
			return VOID;
		}
}
;
class Branch_Order:public Func{
	public:

		Branch_Order(Func* lim=0):Func("Branch_Order",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 

			return t->getOrder();
		}
}
;
class Diam_threshold:public Func{

	public:

		Diam_threshold(Func* lim=0):Func("Diam_threshold",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 

			if(t==NULL ||t->getPrev()==NULL) return VOID;

			if(t->getPrev()->isFather() && t->getNrTip()==1) return t->getDiam();
			return VOID ;
		}
}
;

class Soma_Surface:public Func{

	public:

		Soma_Surface(Func* lim=0):Func("Soma_Surface",lim){};



		int nrSomaSeg(Segment *t){
			int tmp=0;
			if(t->getNext1()!=NULL)
				tmp+=nrSomaSeg(t->getNext1());
			if(t->getNext2()!=NULL)
				tmp+=nrSomaSeg(t->getNext2());
			if (t->getType()==1 && t->getId()>0) tmp++;
			return tmp;
		}

		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			static int notSingle=0;	
			if(t->getId()<0) return VOID;


			if( t->getPid()==-1 ){

				notSingle=nrSomaSeg(t);


				if(notSingle ==1){

					return (3.14*t->getDiam()*t->getDiam());
				}	

				return VOID;

			}


			if(notSingle>0){

				if (t->getPrev()!=NULL && t->getType()==1 &&t->getId()>0) 
					return (t->getEnd()->distance(t->getPrev()->getEnd()))*3.14*t->getPrev()->getDiam();
			}
			return VOID;
		}
}
;

class EucDistance:public Func{
	public:

		EucDistance(Func* lim=0):Func("EucDistance",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			return t->getSomaDistance();
		}
}
;
class PathDistance:public Func{
	public:

		PathDistance(Func* lim=0):Func("PathDistance",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			return t->getPathDistance();
		}
}
;

//Burke taper
//##ModelId=3F6DA246032A
class Taper_1:public Func{

	Segment * tmp;
	public:


	Taper_1(Func* lim=0):Func("Taper_1",lim){};

	Func *  getPreFunc(){ return this;}

	double  computeStep(Segment *t){ 

		if(t->getPrevFather()->getId() < 2 || ( t->isFather() == NULL ) ) 
			return VOID;
		tmp=t;

		double length=tmp->getLength();
		tmp=tmp->getPrev();
		while(!tmp->isFather()){

			length+=tmp->getLength();
			tmp=tmp->getPrev();
		}

		double taper=(t->getDiam()-t->getPrevFather()->getDiam())/length;
		return taper;
	}
}
;



class Rall_Power:public Func{
	double d,d1,d2,min,max;
	double step(double m){ 
		double a1=pow(d,m);
		double a2=pow(d1,m)+pow(d2,m);

		double a3=fabs( (a1-a2));
		return a3;
	}
	public:

	Rall_Power(Func* lim=0):Func("Rall_Power",lim){};
	Func *  getPreFunc(){ return this;}
	double  computeStep(Segment *t){ 
		if(t==NULL) return VOID;

		if(t->isFather() && t->getType()!=1) {
			max=5;min=0;
			d=t->getDiam();
			if(t->getNext2()==NULL) return VOID;
			d1=t->getNext1()->getDiam();
			d2=t->getNext2()->getDiam();

			if(d==d1 ||d==d2){
				return VOID;
			}

			//skip biforcation than get bigger
			if(d<d1 ||d<d2) {
				return VOID;
			}
			//compute manually
			double start=min;
			double steps=1000;

			double const delta=(max-min)/steps;
			//look for a local minima
			double t1,t2,t3,in=0;
			double minima;
			minima=0;
			while (start<max){
				t1=step(start);
				in++;
				if (in>1000){
					Segment *s=t;

				}

				t2=step(start+delta);

				t3=step(start+delta+delta);

				start+=delta;

				if(t1>t2 && t3>t2) {

					minima=start;
				}
			}

			if (minima==0 || minima==max){

				return VOID;


			}

			return minima;
		}
		return VOID;
	}
}
;
class Rall1:public Func{
	double d,d1,d2,f1,f2,f3,min,max,med;
	double step(double m){ 
		double a1=pow(d,m);
		double a2=pow(d1,m)+pow(d2,m);
		double a3=fabs( (a1-a2)/a1);
		return a3;
	}
	public:

	Rall1(Func* lim=0):Func("Rall1",lim){};
	Func *  getPreFunc(){ return this;}
	double  computeStep(Segment *t){ 



		if(t->isFather() && t->getType()!=1) {
			max=5;min=0.5;med=(min+max)/2;
			d=t->getDiam();
			if(t->getNext2()==NULL) return VOID;
			d1=t->getNext1()->getDiam();
			d2=t->getNext2()->getDiam();

			if(d==d1 ||d==d2){
				return VOID;
			}
			//skip biforcation than get bigger
			if(d<d1 ||d<d2) return VOID;
			for(int i=0;i<100;i++){
				f1=step(min);
				f2=step(med);
				f3=step(max);
				if(f1<f2) max=med;
				if(f2>f3) min=med;
				//if in a minima choose the less max)
				if(f1>f2 && f2<f3){
					if(f1<f3) {
						max=med;
					} else {
						min=med;
					}
				}
				med=(min+max)/2;
			}

			return med;
		}
		return VOID;
	}
}
;
class Parent_Daughter_Ratio:public Func{
	public:

		Parent_Daughter_Ratio(Func* lim=0):Func("Parent_Daughter_Ratio",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			//if prev is father return father/segment ratio else VOID
			if(t->getPrev()!=NULL)
				if(t->getPrev()->isFather()){
					double a,b;
					a=t->getPrev()->getDiam();
					b=t->getDiam();

					return b/a;
				}
			return VOID;
		}
}
;

class Last_parent_diam:public Func{
	public:

		Last_parent_diam(Func* lim=0):Func("Last_parent_diam",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 

			if( t->isFather() && t->getNrTip()==2)
				return t->getDiam();
			return VOID;
		}
}
;
class Partition_asymmetry:public Func{
	public:

		Partition_asymmetry(Func* lim=0):Func("Partition_asymmetry",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			if(t->isFather()){
				if(t->getNext2()==NULL) return VOID;
				int n1=t->getNext1()->getNrTip();
				int n2=t->getNext2()->getNrTip();
				if(n1==1 && n2==1) return 0;

				return abs(n1-n2)/(n1+n2-2.0);
			}
			return VOID;
		}
}
;
class Daughter_Ratio:public Func{
	public:

		Daughter_Ratio(Func* lim=0):Func("Daughter_Ratio",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 

			if(!(t->isFather())) return VOID;
			if(t->getNext2()==NULL) return VOID;
			double d1=t->getNext1()->getDiam();
			double d2=t->getNext2()->getDiam();
			if(d2>d1) return d2/d1;
			return d1/d2;
		}
}
;

class Taper_2:public Func{
	public:

		Taper_2(Func* lim=0):Func("Taper_2",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 

			if(t->getId()<2)
				return VOID;

			if(t->isFather()||(t->isTerminate()&&t->getType()>1)){

				if(t->getPrevFather()->getDiam()!=0)

					return (t->getPrevFather()->getDiam()-t->getDiam())/t->getPrevFather()->getDiam();
			}

			return VOID;
		}
}
;



class Branch_pathlength:public Func{
	public:

		Branch_pathlength(Func* lim=0):Func("Branch_pathlength",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 

			if(t->getId()<2)
				return VOID;

			if(t->isFather() ||(t->isTerminate() && t->getType()>1)){

				return (t->getPathDistance() - t->getPrevFather()->getPathDistance());
			}
			return VOID; 
		}
}
;

class Bif_ampl_local:public Func{

	public:

		Bif_ampl_local(Func* lim=0):Func("Bif_ampl_local",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			if(t->getNext2()==NULL) return VOID;
			if(t->isFather())
				return (t->getEnd()->angle(t->getNext1()->getEnd(),t->getNext2()->getEnd()));
			return VOID;
		}
}
;
class Bif_ampl_remote:public Func{

	public:

		Bif_ampl_remote(Func* lim=0):Func("Bif_ampl_remote",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			if(t->getNextFather2()==NULL) return VOID;
			if(t->isFather())
				return (t->getEnd()->angle(t->getNextFather1()->getEnd(),t->getNextFather2()->getEnd()));
			return VOID;
		}
}
;
class Bif_tilt_local:public Func{
	public:

		Bif_tilt_local(Func* lim=0):Func("Bif_tilt_local",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			if(t->getNext2()==NULL) return VOID;
			if(t->isFather() && t->getPrev()!=NULL && t->getPrev()->getId()!=1){

				double a1,a2;
				a1=t->getEnd()->angle(t->getPrev()->getEnd(),t->getNext1()->getEnd());
				a2=t->getEnd()->angle(t->getPrev()->getEnd(),t->getNext2()->getEnd());
				if(a1>a2) return a2;
				return a1;
			}
			return VOID;
		}
}
;
class Bif_tilt_remote:public Func{
	public:

		Bif_tilt_remote(Func* lim=0):Func("Bif_tilt_remote",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			if(t->isFather() && t->getPrevFather()!=NULL && t->getPrevFather()->getId()!=1){

				double a1,a2;
				a1=t->getEnd()->angle(t->getPrevFather()->getEnd(),t->getNextFather1()->getEnd());
				a2=t->getEnd()->angle(t->getPrevFather()->getEnd(),t->getNextFather2()->getEnd());
				if(a1>a2) return a2;
				return a1;
			}
			return VOID;
		}
}
;
class Surface:public Func{
	public:

		Surface(Func* lim=0):Func("Surface",lim){};
		Func *  getPreFunc(){ return this;}

		double  computeStep(Segment *t){ 
			if(t->getPrev()==NULL) return VOID;

			double t1=t->getEnd()->distance(t->getPrev()->getEnd());
			double t2=t->getLength();
			return PI*t->getDiam()*(t->getLength());

		}
}
;

class SectionArea:public Func{
	public:


		SectionArea(Func* lim=0):Func("SectionArea",lim){};

		Func *  getPreFunc(){ return this;}


		double  computeStep(Segment *t){ 
			if(t->getPrev()==NULL) return VOID;
			double rad=t->getRadius();

			if(rad==0) return VOID;
			double sa=PI*rad*rad;

			if(sa==0) return VOID;
			return sa;

		}
}
;
class Volume:public Func{
	public:

		Volume(Func* lim=0):Func("Volume",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			if(t->getPrev()==NULL) return VOID; 

			return PI*t->getRadius()*t->getRadius()*(t->getLength());

		}
}
;


class Contraction:public Func{

	public:

		Contraction(Func* lim=0):Func("Contraction",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			if((t->isFather()||(t->isTerminate()&&t->getType()>1)) && t->getPrevFather()->getId()>-1){

				if(t->getPrevFather()==NULL) return VOID;
				double path=0,eucli=0;
				path= t->getPathDistance() - t->getPrevFather()->getPathDistance() ;
				eucli=t->getPrevFather()->getEnd()->distance(t->getEnd());
				return eucli/path;
			}
			return VOID;
		}
}
;

class Fractal_Dim:public Func{
	public:
		Fractal_Dim(Func* lim=0):Func("Fractal_Dim",lim){};
		Func* getPreFunc(){return this;}
		double computeStep(Segment *t){
			static double *eucx= new double[12000];
			static double *pathy=new double[12000];

			int ctr=0;
			if(((t->isFather() && t->getPrev()!=NULL)||(t->isTerminate() && t->getType()>1)) && t->getPrevFather()->getId()>-1){
				Segment* prevtot = t->getPrev();
				Segment* prevfather = t->getPrevFather();

				if(t==prevfather|| prevtot==prevfather || prevtot->getPrev()==prevfather) return VOID;

				while(prevtot->getEnd()!=prevfather->getEnd()){

					eucx[ctr]=prevtot->getEnd()->distance(t->getEnd());
					pathy[ctr]=t->getPathDistance()-prevtot->getPathDistance();

					ctr++;
					prevtot=prevtot->getPrev();
				}
				eucx[ctr]=prevtot->getEnd()->distance(t->getEnd());
				pathy[ctr]=t->getPathDistance()-prevtot->getPathDistance();

				double D = computeD(eucx,pathy,ctr);

				return D;
			}
		}

		double computeD(double x[], double y[],int size){
			double prodofxy=0,sumofx=0,sumofy=0,sumofx2=0,num=0,den=0,sumofy2=0;

			for(int i=size;i>=0;i--){

				if(x[i]==0 || y[i]==0){

					i--;

				}

				prodofxy+=log(x[i])*log(y[i]);
				sumofx+=log(x[i]);
				sumofy+=log(y[i]);

				sumofy2+=log(y[i])*log(y[i]);	
			}

			num = (size+1)*prodofxy-sumofx*sumofy;
			den = (size+1)*sumofy2-sumofy*sumofy;
			return fabs(num/den);
		}
}
;

class Fragmentation:public Func{

	public:

		Fragmentation(Func * lim=0):Func("Fragmentation",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			if((t->isFather() || t->isTerminate()) && t->getPrevFather()->getId()>1) {
				int seg=1;
				Segment * tmp=t->getPrev();
				while(1){
					if(tmp==NULL) break;
					if(tmp->isFather()) break;
					tmp=tmp->getPrev();
					seg++;
				}

				return seg;
			}
			return VOID;
		}
}
;
class Bif_torque_remote:public Func{


	public:

		Bif_torque_remote(Func* lim=0):Func("Bif_torque_remote",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			if(t->getPid()==-1) return VOID;
			if(!(t->isFather() && t->getPrevFather()->isFather())) return VOID;

			if( t->getNextFather1()==NULL || t->getNextFather2()==NULL ) return VOID;

			//continue only if the actual segment is father and it has a previous one. 
			//PS: Pay attention previous father CAN BE outside the LIMIT SET!!!!
			Segment *prev=t->getPrevFather();
			if( prev->getNextFather1()==NULL || prev->getNextFather2()==NULL ) return VOID;
			Vector * v1=new Vector();
			//evaluate cross product of previous father
			v1->crossProduct(prev->getEnd(),prev->getNextFather1()->getEnd(),prev->getNextFather2()->getEnd());
			Vector * v2=new Vector();
			//evaluate cross product of actual father
			v2->crossProduct(t->getEnd(),t->getNextFather1()->getEnd(),t->getNextFather2()->getEnd());
			//exctract angle betwwen v1 and v2


			Vector* oo=new Vector();
			double angle=oo->angle(v1,v2);

			return  angle;
		}
}
;
class Bif_torque_local:public Func{
	//evaluate the angle between two consecutive bifurcations: the first biforc consider segments
	// the sencond one compartments
	//NB: Up to now the angle is always between 0 and PI/2
	//    It means not daughter order is assumed!!!

	public:

		Bif_torque_local(Func* lim=0):Func("Bif_torque_local",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			if(t->getPid()==-1) return VOID;
			//still in soma
			if(t->getId()==t->getPrevFather()->getId()) return VOID;
			if(!(t->isFather() && t->getPrevFather()->isFather()) ) return VOID;
			if( t->getPrevFather()==NULL || t->getNext1()==NULL || t->getNext2()==NULL ) return VOID;
			//continue only if the actual segment is father and it has a previous one. 
			//PS: Pay attention previous father CAN BE outside the LIMIT SET!!!!
			Segment *prev=t->getPrevFather();
			if( prev==NULL || prev->getNext1()==NULL || prev->getNext2()==NULL ) return VOID;

			Vector * v1=new Vector();
			//evaluate cross product of previous father
			v1->crossProduct(prev->getEnd(),prev->getNext1()->getEnd(),prev->getNext2()->getEnd());


			Vector * v2=new Vector();
			//evaluate cross product of actual father
			v2->crossProduct(t->getEnd(),t->getNext1()->getEnd(),t->getNext2()->getEnd());


			Vector* oo=new Vector();
			double angle=oo->angle(v1,v2);

			return  angle;
		}
}
;
class PlaneAngleTotal:public Func{
	//evaluate the angle between two consecutive bifurcations
	//NB: Up to now the angle is always between 0 and PI ... first daughter with bigger diameter
	//    

	public:

		PlaneAngleTotal(Func* lim=0):Func("PlaneAngleTotal",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			if(!(t->isFather() && t->getPrevFather()->isFather()) && t->getPid()!=-1) return VOID;
			if( t->getNextFather1()==NULL || t->getNextFather2()==NULL ) return VOID;
			//continue only if the actual segment is father and it has a previous one. 
			//PS: Pay attention previous father CAN BE outside the LIMIT SET!!!!
			Segment *prev=t->getPrevFather();
			if( prev->getNextFather1()==NULL || prev->getNextFather2()==NULL ) return VOID;
			Vector * v1=new Vector();
			//evaluate cross product of previous father
			if(prev->getNextFather1()->getDiam()>prev->getNextFather2()->getDiam()){
				v1->crossProduct(prev->getEnd(),prev->getNextFather1()->getEnd(),prev->getNextFather2()->getEnd());
			} else {
				v1->crossProduct(prev->getEnd(),prev->getNextFather2()->getEnd(),prev->getNextFather1()->getEnd());

			}
			Vector * v2=new Vector();
			//evaluate cross product of actual father
			if ( t->getNextFather1()->getDiam()>t->getNextFather2()->getDiam()){
				v2->crossProduct(t->getEnd(),t->getNextFather1()->getEnd(),t->getNextFather2()->getEnd());
			}else {
				v2->crossProduct(t->getEnd(),t->getNextFather2()->getEnd(),t->getNextFather1()->getEnd());

			}
			//exctract angle betwwen v1 and v2


			double angle=t->getEnd()->angle(new Vector(),v1,new Vector(),v2);

			return  angle;
		}
}
;
class N_bifs:public Func{
	public:
		N_bifs(Func* lim=0):Func("N_bifs",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 

			if (t->getNext1()!=NULL && t->getNext2()!=NULL && t->getType()>1)//t->isFather()) 
				return 1;
			return VOID;
		}
}
;
class Pk_2:public Func{
	public:

		Pk_2(Func* lim=0):Func("Pk_2", lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			if(t->getPid()==-1){
				Segment * tt=t;
			}
			double k=2;

			if(t->isFather() && t->getType()!=1){
				double p=pow(t->getDiam(),k);
				if(t->getNext2()==NULL) return VOID;
				double d1=pow(t->getNext1()->getDiam(),k);
				double d2=pow(t->getNext2()->getDiam(),k);
				if(p==0){
					Segment* p=t;
				}
				return (d1+d2)/p;
			}
			return VOID;

		}
}
;
class Pk_classic:public Func{
	public:

		Pk_classic(Func* lim=0):Func("Pk_classic", lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			if(t->getPid()==-1){
				Segment * tt=t;
			}
			double k=1.5;

			if(t->isFather() && t->getType()!=1){
				double p=pow(t->getDiam(),k);
				if(t->getNext2()==NULL) return VOID;
				double d1=pow(t->getNext1()->getDiam(),k);
				double d2=pow(t->getNext2()->getDiam(),k);
				if(p==0){
					Segment* p=t;
				}
				return (d1+d2)/p;
			}
			return VOID;

		}
}
;
// Use Next Bifurcation diameter


class Pk:public Func{
	public:
		Pk(Func* lim=0):Func("Pk",new Rall1(), lim){};
		Func *  getPreFunc(){ return new Rall1();}
		double  computeStep(Segment *t){ 
			if(t->getPid()==-1){
				Segment * tt=t;
			}
			double k=preMean;

			if(t->isFather() && t->getType()!=1){
				double p=pow(t->getDiam(),k);
				if(t->getNext2()==NULL) return VOID;
				double d1=pow(t->getNext1()->getDiam(),k);
				double d2=pow(t->getNext2()->getDiam(),k);
				if(p==0){
					Segment* p=t;
				}
				return (d1+d2)/p;
			}
			return VOID;

		}
};

class Xyz:public Func{
	public:

		Xyz(Func* lim=0):Func("Xyz",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 

			cout<<t->getEnd()->getX()<<"\t"<<t->getEnd()->getY()<<"\t"<<t->getEnd()->getZ()<<"\n";
			return VOID;
		}
}
;

class Zero:public Func{
	public:

		Zero(Func* lim=0):Func("Zero",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 

			return VOID;
		}
}
;


class N_tips:public Func{
	public:

		N_tips(Func* lim=0):Func("N_tips",lim){};
		Func *  getPreFunc(){ return this;}
		double  computeStep(Segment *t){ 
			if(t->isTerminate())
				return 1;
			return VOID;
		}
}
;


class Terminal_degree:public Func{
	public:


		Terminal_degree(Func* lim=0):Func("Terminal_degree",lim){};

		Func *  getPreFunc(){ return this;}

		double  computeStep(Segment *t){ 

			return t->getNrTip();
		}
}
;


class Helix:public Func{

	public:

		Helix(Func* lim=0):Func("Helix",lim){};

		double computeHelix(Segment * pre3,Segment * pre2,Segment * pre,Segment * curr)
		{
			Vector * orig = new Vector(0,0,0);
			Vector *v =new Vector();
			Vector *c = new Vector();
			c->crossProduct(pre3->getEnd(),pre2->getEnd(),pre2->getEnd(),pre->getEnd());
			double numerator = v->scalar(pre->getEnd(),curr->getEnd(),orig,c);
			double denomenator = 3*pre3->getEnd()->distance(pre2->getEnd())*
				pre2->getEnd()->distance(pre->getEnd())*
				pre->getEnd()->distance(curr->getEnd());
			return numerator/denomenator;
		}



		Func * getPreFunc(){ return this;}

		double computeStep(Segment *t){
			Segment * prev3 = NULL;
			Segment * prev2 = NULL;
			Segment * prev = NULL;

			prev = t->getPrev();
			if(prev == NULL)
				return VOID;
			prev2 = prev->getPrev();
			if(prev2 == NULL)
				return VOID;
			prev3 = prev2->getPrev();
			if(prev3 == NULL)
				return VOID;

			return computeHelix(prev3,prev2,prev,t);


		}

}
;


class Dim : public Func
{
	protected:

		double percent;
		int x,y,z;
		bool xflg ;
		bool yflg ;
		bool zflg ;
	public:

		Func *  getPreFunc();


		int countSegment(Segment *t);

		void fill(Segment * t,float **data);

		double  computeStep(Segment *t);


		void swap(int min, int i,float **data);


		void SelectionSort(float** data, int left, int right);

		int Partition( float ** d, int left, int right);

		void Quicksort( float** d, int left, int right);

}
;

;
;
class Width:public Dim{
	public:

		//added setName for w,h,d functions to print the name of respective functions when using specificity. sri 05/10/2010.
		Width(Func* lim=0){	
			limit=lim; x=1;y=0;z=0;percent=TH_DIM;setName("Width");
		};
};
class Height:public Dim{
	public:
		Height(Func* lim=0){ 
			limit=lim; x=0;y=1;z=0;percent=TH_DIM;setName("Height");
		};
};
class Depth:public Dim{
	public:
		Depth(Func* lim=0){ 
			limit=lim; x=0;y=0;z=1;percent=TH_DIM;setName("Depth");
		};
};

class X:public Func{
	public:

		X(Func* lim=0):Func("X",lim){};

		Func *  getPreFunc(){ return this;}

		double  computeStep(Segment *t){ 

			return t->getEnd()->getX();
		}
}
;

class HillmanThreshold:public Func{
	public:

		HillmanThreshold(Func* lim=0):Func("HillmanThreshold",new Branch_pathlength(), lim){};

		Func *  getPreFunc(){ return new Branch_pathlength();}

		double  computeStep(Segment *t){ 
			if(t->getPid()==-1){
				Segment * tt=t;
			}


			if(t->isFather() && t->getNrTip()==2){
				double th=0.5*t->getDiam();

				double path=t->getPathDistance()+preMean;
				Segment * s=t->getNext1();
				if(s==NULL) return VOID;
				while(s->getPathDistance()<path){
					if(s->getNext1()!=NULL) {
						s=s->getNext1();
					} else {break;}
				}
				th+=0.25*s->getDiam();


				s=t->getNext2();
				if(s==NULL) return VOID;
				while(s->getPathDistance()<path){
					if(s->getNext1()!=NULL) {
						s=s->getNext1();
					} else {break;}
				}
				th+=0.25*s->getDiam();

				return th;

			}
			return VOID;

		}
};



/*
class HausdorffOld:public Func{

	short **cell;
	public:

	HausdorffOld(Func* lim=0):Func("HausdorffOld",0){cell=NULL;};

	Func *  getPreFunc(){ return new Zero();}



	int fillArray(Segment * t,short** r1, short** r2){
		static int id=1;
		static double scale=1;

		if(t->getPid()==-1)
			id=1;
		if(t->getPid()==-1 || (t->getType()>2 && t->getType()<6)){
			double d;
			d=1;

			Segment *s=t->getPrev();
			if(s==NULL) s=t;
			int cst=1;
			r2[0][id]=(short)s->getEnd()->getX()/scale+cst;
			r2[1][id]=(short)s->getEnd()->getY()/scale+cst;
			r2[2][id]=(short)s->getEnd()->getZ()/scale+1;
			r1[0][id]=(short)t->getEnd()->getX()/scale+cst;
			r1[1][id]=(short)t->getEnd()->getY()/scale+cst;
			r1[2][id]=(short)t->getEnd()->getZ()/scale+1;

			id++;
		}
		if(t->getNext1()!=NULL)
			fillArray(t->getNext1(),r1,r2);
		if(t->getNext2()!=NULL)
			fillArray(t->getNext2(),r1,r2);

		return id;

	}


	short **matrix(int n,int m)
	{
		int i;
		short **mat;

		// Allocate pointers to rows.
		mat = (short **) malloc((unsigned) (n)*sizeof(short*));
		if (!mat) erhand("Allocation failure 1 in matrix().");
		mat -= 1;

		// Allocate rows and set pointers to them.
		for (i = 0; i < n; i++)
		{
			mat[i] = (short *) malloc((unsigned) (m)*sizeof(short));
			if (!mat[i]) erhand("Allocation failure 2 in matrix().");
			mat[i] -= 1;
		}

		// Return pointer to array of pointers to rows.
		return mat;

	}

	void free_matrix(short **mat,int n,int m)

		// Free a float matrix allocated by matrix(). 
	{
		int i;

		for (i = n-1; i >= 0; i--)
		{
			free ((short*) (mat[i]+1));
		}
		free ((short*) (mat+1));
	}




	double  computeStep(Segment *t){ 
		if(t->getPid()>0)
			return VOID;


#define N 30000            // max# sections in each file 
		int prt=0, trace=0, lmin=0, lmax=0;
#define LMINMAX 2
#define LMAXMIN 1
#define NCELL 50000        // max# nonempty lattice cells in Hausdorff analysis
		cerr<<"HS";



		int n;



		short **	r1, **r2;

		r1=matrix(3,N);
		r2=matrix(3,N);
		for(int j=1;j<3;j++)
			for(int u=1;u<N;u++){
				r1[j][u]=0;
				r2[j][u]=0;
			}

		n=fillArray(t, r1,  r2);
		r1[0][0]=6;r2[0][0]=625;
		r1[1][0]=0;r2[1][0]=625;
		r1[2][0]=0;r2[2][0]=0;

		int i=0, k=0, k1=0, l=0, m=0, cnt=0, dl=0;
		short r[3], rr[3];

		int scale=0;
		float dr[3], rt[3], total=0;
		float hd=0, measure[15], length=0;

		auto int ii=1<<30;

		if (cell==NULL)
		{
			cell=(short **)calloc(3*NCELL, sizeof(short));
			assert(cell!=NULL);
			for (i=0; i<NCELL; i++) cell[i]=(short *)calloc(3,sizeof(short));
			assert(cell[i-1]!=NULL);
		}

		length=0;


		short rMin[3];
		short rMax[3];
		for (k=0; k<3; k++){
			rMin[k]=r1[k][0];
			rMax[k]=r1[k][0];
		}

		for (i=1; i<n; i++) for (k=0; k<3; k++)
		{
			lmin += abs(r1[k][i]-r2[k][i]);
			if(rMin[k]<r1[k][i]) rMin[k]=r1[k][i];
			if(rMax[k]>r1[k][i]) rMax[k]=r1[k][i];
			if (lmax<abs(r1[k][i])) lmax=abs(r1[k][i]);
			if (lmax<abs(r2[k][i])) lmax=abs(r2[k][i]);
		}
		lmin /= LMINMAX*n;
		lmax /=3;


		if (lmin<1) lmin=1;
		else if (lmin>1)
		{
			lmax /= lmin;
			for (i=1; i<n; i++) for (k=0; k<3; k++)
			{
				r1[k][i] /= lmin;
				r2[k][i] /= lmin;
			}
		}
		if (lmax<=1) return(0.0);
		scale=lmin;
		cnt=0;
		//-------------------main loop----------
		while (lmax>LMAXMIN)
		{
			m=1;
			for (k=0; k<3; k++) cell[0][k]=r1[k][1]; //-add root to the marked pool-
			for (i=1; i<n; i++) if ((r1[0][i]!=r2[0][i]) ||
					(r1[1][i]!=r2[1][i]) ||
					(r1[2][i]!=r2[2][i]))
			{
				//----this fragment is to be removed----------
				dl=0;
				for (k=0; k<3; k++) dl += (r1[k][i]-r2[k][i])*(r1[k][i]-r2[k][i]);
				length += sqrt(1.0*dl);

				//-------------------------tracing link-------
				total=0.0;
				for (k=0; k<3; k++) total += abs(r2[k][i]-r1[k][i]);
				for (k=0; k<3; k++) 
				{
					r[k]=r1[k][i];
					dr[k]=(r2[k][i]-r[k])/total;
					rt[k]=dr[k];
				}
				m=mark(m, r); //----------this line is not necessary if each link has a parent--
				while((r[0]!=r2[0][i]) ||
						(r[1]!=r2[1][i]) ||
						(r[2]!=r2[2][i]))
				{

					l=0;
					k1=-1;
					for (k=0; k<3; k++) rr[k]=r2[k][i]-r[k];
					for (k=0; k<3; k++) 
					{
						if ((rt[k]*rr[k]>0) && (abs(l)<abs(rr[k])))
						{
							l=rr[k];
							k1=k;
						}
					}

					assert((k1>=0) && (l!=0));
					if (l>0) 
					{
						r[k1]++;

					}
					else 
					{
						r[k1]--;

					}
					for (k=0; k<3; k++) rt[k] += dr[k];
					m=mark(m, r);
					if (m>=NCELL) cerr<<"maximal cell number reached"<<endl;

					if (m>=NCELL) exit(1);

				}
			}
			//---------------- output-------------
			measure[cnt]=m;
			cerr<< " "<<m;

			cnt++;
				if (prt)
				{
				printf("\ncnt=%d  r1= %d %d %d  r= %d %d %d  r2= %d %d %d", 
				cnt, r1[0][i], r1[1][i], r1[2][i], r[0], r[1], r[2], r2[0][i], r2[1][i], r2[2][i]);
				printf("\nm=%d  i=%d  n=%d  lmin=%d  lmax=%d  length=%f\n", 
				m, i, n, lmin, lmax, length);
				}

			for (i=1; i<n; i++) for (k=0; k<3; k++)
			{
				r1[k][i] /= 2;
				r2[k][i] /= 2;
			}
			lmax /= 2;
			scale *=2;
		}
		//-------------------computing Hausdorff dimension---------
		hd=0;
		for (i=0; i<cnt; i++) hd += (i-0.5*(cnt-1))*log(measure[i]);
		hd *= -12.0/(cnt*(cnt*cnt-1.0))/log(2.0);
		//-------------------file output---------------------------

		return(hd);
	}

	int mark(int m, short * r){
		int i, j, k;
		for (i=0; i<m; i++) 
		{
			if (cell[i][0]==r[0] &&
					cell[i][1]==r[1] &&
					cell[i][2]==r[2]) return(m); 			if (cell[i][0]<=r[0] &&
					cell[i][1]<=r[1] &&
					cell[i][2]<r[2]) break; 
		}
		if (i<m) for (j=m; j>i; j--) for (k=0; k<3; k++) cell[j][k]=cell[j-1][k];
		for (k=0; k<3; k++) for (k=0; k<3; k++) cell[i][k]=r[k];
		return(m+1);
	}

}
;*/



class Hausdorff:public Func{

	short **cell;
	public:

	Hausdorff(Func* lim=0):Func("Hausdorff",0){cell=NULL;};

	Func *  getPreFunc(){ return new Zero();}


	int fillArray(Segment * t,short** r1, short** r2){
		static int id=1;
		static double scale=1;

		if(t->getPid()==-1)
			id=1;
		if(t->getPid()==-1 || (t->getType()>2 && t->getType()<6)){

			Segment *s=t->getPrev();
			if(s==NULL) s=t;
			int cst=1;
			r2[0][id]=(short)s->getEnd()->getX()/scale+cst;
			r2[1][id]=(short)s->getEnd()->getY()/scale+cst;
			r2[2][id]=(short)s->getEnd()->getZ()/scale+1;
			r1[0][id]=(short)t->getEnd()->getX()/scale+cst;
			r1[1][id]=(short)t->getEnd()->getY()/scale+cst;
			r1[2][id]=(short)t->getEnd()->getZ()/scale+1;

			id++;
		}
		if(t->getNext1()!=NULL)
			fillArray(t->getNext1(),r1,r2);
		if(t->getNext2()!=NULL)
			fillArray(t->getNext2(),r1,r2);

		return id;

	}


	short **matrix(int n,int m)
	{
		int i;
		short **mat = new short*[n];
		for (int i=0;i<n;i++)
		{
			mat[i] = new short[m];
			for (int j=0;j<m;j++)
				mat[i][j] = 0;
		}
		/* Return pointer to array of pointers to rows. */
		return mat;

	}

	void free_matrix(short **mat,int n,int m)

		/* Free a float matrix allocated by matrix(). */
	{
		int i;

		for (i = 0; i<n ; i++)
		{
			delete(mat[i]);
		}
		delete(mat);
	}




	double  computeStep(Segment *t){
		if(t->getPid()>0||t->getType()<0){
			return VOID;
		}


#define N 30000            /* max# sections in each file */

#define LMINMAX 2
#define LMAXMIN 1
#define NCELL 50000        /* max# nonempty lattice cells in Hausdorff analysis */

		int n;

		short **	r1, **r2;

		r1=matrix(3,N);
		r2=matrix(3,N);
		for(int j=1;j<3;j++)
			for(int u=1;u<N;u++){
				r1[j][u]=0;
				r2[j][u]=0;
			}
		n=fillArray(t, r1,  r2);
		//r1[0][0]=6;r2[0][0]=625;
		//r1[1][0]=0;r2[1][0]=625;
		//r1[2][0]=0;r2[2][0]=0;



		int i,  k, k1, l, m, cnt, dl, lmin, lmax;
		short r[3], rr[3], **cell;

		int scale;
		float dr[3], rt[3], total;
		float hd, measure[25], length;

		length=0;
		lmin=0;
		lmax=0;
		for (i=1; i<n; i++) for (k=0; k<3; k++)
		{
			lmin += abs(r1[k][i]-r2[k][i]);
			if (lmax<abs(r2[k][i]-r1[k][1])) lmax=abs(r2[k][i]-r1[k][1]);
		}
		lmin /= LMINMAX*n;
		lmax /= 2;
		/*------------start with lattice cell >= lmin ------------*/
		if (lmin<1) lmin=1;
		else if (lmin>1)
		{
			lmax /= lmin;
			for (i=1; i<n; i++) for (k=0; k<3; k++)
			{
				r1[k][i] /= lmin;
				r2[k][i] /= lmin;
			}
		}
		if (lmax<=1) return(0.0);
		scale=lmin;
		cnt=0;

		cell = matrix(NCELL,3);
		/*-----------------------------------------------------main loop begin----------------------*/
		while (lmax>LMAXMIN)
		{
			for (k=0; k<3; k++) r[k]=r1[k][1];
			m=mark(0, r, cell, scale);
			for (i=1; i<n; i++) if ((r1[0][i]!=r2[0][i]) ||
					(r1[1][i]!=r2[1][i]) ||
					(r1[2][i]!=r2[2][i]))
			{
				/*-------------------------tracing link-------*/
				total=0.0;
				for (k=0; k<3; k++) total += abs(r2[k][i]-r1[k][i]);
				for (k=0; k<3; k++) 
				{
					r[k]=r1[k][i];
					dr[k]=(r2[k][i]-r[k])/total;
					rt[k]=dr[k];
				}
				m=mark(m, r, cell, scale);
				while((r[0]!=r2[0][i]) ||
						(r[1]!=r2[1][i]) ||
						(r[2]!=r2[2][i]))
				{
					l=0;
					k1=-1;
					for (k=0; k<3; k++) rr[k]=r2[k][i]-r[k];
					for (k=0; k<3; k++)
					{
						if ((rt[k]*rr[k]>0) && (abs(l)<abs(rr[k])))
						{
							l=rr[k];
							k1=k;
						}
					}
					assert((k1>=0) && (l!=0));
					if (l>0) 
					{
						r[k1]++;
						rt[k1]--;
					}
					else
					{
						r[k1]--;
						rt[k1]++;
					}
					for (k=0; k<3; k++) rt[k] += dr[k];
					m=mark(m, r, cell, scale);
					if (m>=NCELL) cerr<<"maximal cell number reached"<<endl;;
					if (m>=NCELL) exit(1);
				}

			}
			measure[cnt]=m;
			cnt++;
		
			for (i=1; i<n; i++) for (k=0; k<3; k++)
			{
				r1[k][i] /= 2;
				r2[k][i] /= 2;
			}
			lmax /= 2;
			scale *=2;
		}
		/*-----------------------------main loop end-------------------------*/
		free_matrix(r1,3,N);
		free_matrix(r2,3,N);
		free_matrix(cell,NCELL,3);
		/*-----------------------------computing Hausdorff dimension---------*/
		hd=0;
		for (i=0; i<cnt; i++) hd += (i-0.5*(cnt-1))*log(measure[i]);
		hd *= -12.0/(cnt*(cnt*cnt-1.0))/log(2.0);
		return(hd);
	}

	/*********************** mark lattice cell r, keep marked set ordered */
	int mark(int m, short r[3], short ** c, int scale)
	{
		int i, j, k;
		if (m<=0)
			for (k=0; k<3; k++) c[0][k]=r[k]; /*--initialize the pool of marked cells--*/
		else
		{
			for (i=0; i<m; i++) 
			{
				if (c[i][0]==r[0] &&
						c[i][1]==r[1] &&
						c[i][2]==r[2]) return(m); /*--- already marked ---*/
				if (c[i][0]>=r[0] &&
						c[i][1]>=r[1] &&
						c[i][2]>r[2]) break; /*--- insert into ordered set ---*/
			}
			if (i<m) for (j=m; j>i; j--) for (k=0; k<3; k++) c[j][k]=c[j-1][k];
			for (k=0; k<3; k++) c[i][k]=r[k];
		}

		return(m+1);

	}

}
;




#endif
