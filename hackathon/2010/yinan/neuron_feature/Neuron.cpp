
#include "Neuron.h"
#include "Limit.h"
#include "Func.h"
#include <fstream>
#include <iostream>
using namespace std;
#include <cstdlib>
#include <string>
#include <stdlib.h>
#include <time.h>
//#include "ParameterList.h"
#include "Random.h"

#include <sstream>
#include <stdio.h>
#include "pca.h"
#define min(a,b) (a)<(b)?(a):(b)
#define max(a,b) (a)>(b)?(a):(b)
#define SIGN(a, b) ( (b) < 0 ? -fabs(a) : fabs(a) )



Neuron::Neuron(){
	setDefaults();
}
;

void Neuron::setDefaults(){

	d[0][0]=0;
	soma =NULL;
	nrSeg=0;
	//lp=NULL;
	setShrinkX(1);
	setShrinkY(1);
	setShrinkZ(1);

	char def[50];
	strcpy(def,"empty");
	name=def ; //strcpy(name,def);
	
	oriented=false;

}
;

void Neuron::init(){
	//findDuplicate(soma);
	buildFathers(soma,soma);
 	renumerateId();
	setOrder(soma);
	
	setParameters(soma);

};

/*
Neuron::Neuron(ParameterList* lps){
	double d, rho,diam,  az, ele,torch;
	int met, id, pid;
	setName("Tamori");
	rnd=new Random();
	
	setDefaults();

	
	lp=lps;

	
	lp->setType(1);
	torch=0;
	met=lp->getParameter(METHOD);
	
	d=lp->getParameter(SOMA_DIAMETER);
	pid=-1;id=1;
	addPolar(id,1,1,0,0,0,d,-1);
	pid=id;
	
	int maxType=lp->getParameter(MAX_TYPE);

	for(int ii=3;ii<=maxType;ii++){
		
		lp->setType(ii);
		d=lp->getParameter(NUM_TREE);
		
		for(int i=0;i<d;i++){
			//first virtual segment long 1
			if(lp->isPresent(CONSLENGTH)==1)
				rho=lp->getParameter(CONSLENGTH);
			if(lp->isPresent(LENGTH)==1)
				rho=lp->getParameter(LENGTH);

			az=lp->getParameter(TREE_AZY);
			az*=PI/180;//az=0;
			ele=lp->getParameter(TREE_ELEV);
			ele*=PI/180;
			diam=lp->getParameter(INIT_DIAM);
			id++;
			id=addPolar(id,3,rho,az,ele,torch,diam,pid);
			switch (met){
			case 0:
				id=growBurke(diam,ii,id);
				break;
			case 1:
				id=growTamori(diam,ii,id);
				break;
			case 2:
				id=growHillman(diam,ii,id);
				break;
			default :
				cerr<<"No Valid method specified!\n";
			}
		}
	}
	if(soma!=NULL){
		soma->polarRelativeToabsolute();
		soma->polarToCartesian();
		init();
	}

};

//grow a virtual neuron based on the list of parameters

void Neuron::OpenPRM(ParameterList* lps){
	double d, rho,diam,  az, ele,torch;
	int met, id, pid;
	
	
	setDefaults();

	setName(lps->getFileName());
	rnd=new Random();



	lp=lps;

	lp->setType(1);
	torch=0;
	met=lp->getParameter(METHOD);
	//create soma
	d=lp->getParameter(SOMA_DIAMETER);
	pid=-1;id=1;
	addPolar(id,1,1,0,0,0,d,-1);
	pid=id;

	int maxType=lp->getParameter(MAX_TYPE);

	for(int ii=3;ii<=maxType;ii++){
		//3:Apical....
		lp->setType(ii);
		d=lp->getParameter(NUM_TREE);
	
		for(int i=0;i<d;i++){
			//first virtual segment long 1
			if(lp->isPresent(CONSLENGTH)==1)
				rho=lp->getParameter(CONSLENGTH);
			if(lp->isPresent(LENGTH)==1)
				rho=lp->getParameter(LENGTH);

			az=lp->getParameter(TREE_AZY);
			az*=PI/180;//az=0;
			ele=lp->getParameter(TREE_ELEV);
			ele*=PI/180;
			diam=lp->getParameter(INIT_DIAM);
			id++;
			id=addPolar(id,3,rho,az,ele,torch,diam,pid);
			switch (met){
			case 0:
				id=growBurke(diam,ii,id);
				break;
			case 1:
				id=growTamori(diam,ii,id);
				break;
			case 2:
				id=growHillman(diam,ii,id);
				break;
			default :
				cerr<<"No Valid method specified!\n";
			}
		}
	}
	if(soma!=NULL){
		soma->polarRelativeToabsolute();
		soma->polarToCartesian();
		init();
	}

};

int Neuron::growBurke(double diameter, int type,int pid){

	int id=pid;

	double length = lp->getParameter(LENGTH);			//Sample_Distribution (dist_bin_length [tree_type]);
	double taper = lp->getParameter(TAPER);				//Sample_Distribution (dist_taper [tree_type]);
	double k1_overlap = lp->getParameter(K1_OVERLAP);	//Sample_Distribution (dist_k1_overlap [tree_type]);
	double k2_overlap = lp->getParameter(K2_OVERLAP);	//Sample_Distribution (dist_k2_overlap [tree_type]);
	double k1_nonoverlap = lp->getParameter(K1_NONOVERLAP);	//Sample_Distribution (dist_k1_nonoverlap [tree_type]);
	double k2_nonoverlap = lp->getParameter(K2_NONOVERLAP);	//Sample_Distribution (dist_k2_nonoverlap [tree_type]);
	double k1_terminate = lp->getParameter(K1_TERMINATE);	//Sample_Distribution (dist_k1_terminate [tree_type]);
	double k2_terminate = lp->getParameter(K2_TERMINATE);	//Sample_Distribution (dist_k2_terminate [tree_type]);
	double pbr, pnonoverlap, ptr;
	 double torch=0;


	diameter += taper*length;
	
	pbr = k1_overlap * exp (k2_overlap * diameter);
	ptr = k1_terminate * exp (k2_terminate * diameter);
	pnonoverlap = k1_nonoverlap * exp (k2_nonoverlap * diameter);
	
	if (pnonoverlap < pbr)
		pbr = pnonoverlap;
		
	// decide whether to branch, terminate or grow 

	if (rnd->rnd01 () < pbr*length) {
		// branch 
		double r1 = lp->getParameter(GAUSS_BRANCH);		// Sample_Distribution (dist_gaussian_branch [tree_type]);
		double r2 = lp->getParameter(GAUSS_BRANCH);		//Sample_Distribution (dist_gaussian_branch [tree_type]);
		double burkea = lp->getParameter(LINEAR_BRANCH);//Sample_Distribution (dist_linear_branch [tree_type]);
		double diam1 = diameter * (r1 + r2*burkea);
		double diam2 = diameter * (r2 + r1*burkea);
		double totbifang = lp->getParameter(BIFAMPLI);	//Sample_Distribution (dist_bifurcating_amplitude_angle [tree_type]);
		double bifang1 = rnd->rnd01 () * totbifang;
		double bifang2 = bifang1 - totbifang;
	
		id=pid+1;
		id=addPolar( id, type, length, bifang1,0 ,torch, diam1, pid);
		
		id=growBurke(diam1,type, id);

		
		id++;
		id=addPolar( id, type, length, bifang2,0 ,torch, diam2, pid);
		
		pid=growBurke(diam2,type, id);

		return pid;

	} else if (rnd->rnd01 () < ptr*length) { // terminate 
		return id;

	} else { // grow a stem 
		double azi=lp->getParameter(EXTEND_AZIMUTH);
		double ele=lp->getParameter(EXTEND_ELE);
	    id=pid+1;
		torch=0;
	    id=addPolar( id, type, length, azi,ele ,torch, diameter, pid);
	
	    id=growBurke(diameter,type, id);

		
	}	
	return pid;
}


int Neuron::growTamori(double diameter, int type, int pid){
	double length, taper;
	length=lp->getParameter(CONSLENGTH);
	taper=lp->getParameter(TAPER);
	diameter*=(1-taper);

	int id=pid;

	if(diameter<lp->getParameter(THRESHOLD)){
		length=lp->getParameter(TERMLENGTH);
		id++;
		id=addPolar( id, type, length, 0,0,0 , diameter, pid);

		//if less than min then terminate
		return id;
	}

	//bifurcate
	double ratio, power, rall, diam1, diam2;
	

	ratio=lp->getParameter(BIFRATIO);
	power=lp->getParameter(BIFPOWER);
	rall = pow ((1 + pow (ratio, power)), -1/power);
	diam2=diameter*rall*pow(lp->getParameter(PK),1/power);
	diam1=ratio*diam2;

	double dimension, eq5_4_n1,eq5_4_n2,eq5_4_den,bifang1,bifang2;
			dimension = 1.0 + rnd->rnd01() * (power-1);

		// Equation 5.4 from Tamori, 1993 
	eq5_4_n1 = pow (1+pow(ratio,power), 2*dimension/power);
	eq5_4_n2 = pow(ratio, 2*dimension);
	eq5_4_den = 2*pow (1+pow(ratio,power), dimension/power);
	bifang1 = acos ((eq5_4_n1 + eq5_4_n2 - 1) / (eq5_4_den * pow (ratio, dimension)));
	
	bifang2 = acos ((eq5_4_n1 - eq5_4_n2 + 1) / (eq5_4_den));
	

	//for display purpose
	if (rnd->rnd01 () < 0.5) {
		bifang1 *= -1;
		bifang2 *= -1;		
	}


	double torch=lp->getParameter(BIFORIENT)*PI/180;
	double azi,ele;



	 id=pid+1;
	
	azi=bifang1;ele=0;
	id=addPolar( id, type, length, azi,ele ,torch, diam1, pid);
	
	id=growTamori(diam1,type, id);

	
	id++;
	
	azi=bifang2;ele=0;
	id=addPolar( id, type, length, azi,ele ,torch, diam2, pid);
	
	id=growTamori(diam2,type, id);

	return id;
}

int Neuron::growHillman(double diameter,int type, int pid){
// model of Hillman
	double length, taper;
	length=lp->getParameter(CONSLENGTH);
	taper=lp->getParameter(TAPER);
	double diamOld=diameter;
	diameter*=(1-taper);
	

	int id=pid;
	
	if(diameter<lp->getParameter(THRESHOLD)){
		length=lp->getParameter(TERMLENGTH);
		id++;
		id=addPolar( id, type, length, 0,0,0 , diameter, pid);

		//if less than min then terminate
		return id;
	}

	//bifurcate
	double ratio, power, rall, diam1, diam2, totbifang;
	

	ratio=lp->getParameter(BIFRATIO);
	power=lp->getParameter(BIFPOWER);
	rall = pow ((1 + pow (ratio, power)), -1/power);
	diam2=diameter*rall*pow(lp->getParameter(PK),1/power);
	diam1=ratio*diam2;

	double bifang1,bifang2;
	
			totbifang = lp->getParameter(BIFAMPLI);
			totbifang*=PI/180;
			bifang1 = rnd->rnd01 () * totbifang;
			bifang2 = bifang1 - totbifang;



	

	double torch=lp->getParameter(BIFORIENT)*PI/180;
	double azi,ele;

	
	azi=bifang1;ele=0;
	 id=pid+1;
	id=addPolar( id, type, length, azi,ele ,torch, diam1, pid);
	
	id=growHillman(diam1,type, id);

	
	id++;
	
	azi=bifang2;ele=0;
	addPolar( id, type, length, azi,ele ,torch, diam2, pid);
	
	pid=growHillman(diam2,type, id);

	return pid;
}
*/


//build a neuron from an input file

Neuron::Neuron(char* c){
	ferror=false;
	foundsoma=false;
	//reset comment field
	d[0][0]=0;
	//neuronalClass=neuroClass;
	soma =NULL;
	strtseg=NULL;
	nrSeg=0;
	setShrinkX(1);
	setShrinkY(1);
	setShrinkZ(1);
	name=c;
	//name_str =c;
	namectr=1;

	oriented=false;
	
	rf=0;
	//flag variable to intiate rf and first segment
	flag=0;ctr=0;
	

	in=new ifstream(c, ios::in | ios::binary );
	//char * ext;
	
	if(strstr(c,".swc")==NULL && strstr(c,".SWC")==NULL)
	{
		cerr<<"You must input swc files."<<"\n";
	}
		
	//computes SWC files
	OpenSWC();
	
	if(soma->getType()!=1 && foundsoma==true)
	{
		addVirtualSoma(soma);
		buildFathers(soma,soma);
		//initializing this vairable for every new neuron sri 03/25/10
		found=0;
		Segment * tmp=rearrangeSWC(soma); 
		soma=tmp;
		tmp=NULL;
		
		soma->setPrev(NULL);			
	}

	char buff[30];

	if(soma==NULL){
		
		cerr<<"Unable to create Neuron:"<<c<<" !\n----------------------------------\n";
		Vector * e=new Vector(0,0,0);
		soma=new Segment(0,1,e,1,-1);
		delete e;
		return;
	}

	in->close();
	delete in;
	in=NULL;

	init();
	

	//soma->setClass(neuronalClass);

}

void	Neuron::setFunc(int n)
{
	Func * globalLimit = NULL;
	a = findFunc(n,globalLimit);
}

Func *	Neuron::findFunc(int n,Func *f){
switch(n) {
        case 0: return new Soma_Surface(f);
        case 1: return new N_stems(f);
        case 2: return new N_bifs(f);
        case 3: return new N_branch(f);
        case 4: return new N_tips(f);
        case 5: return new Width(f);
        case 6: return new Height(f);
        case 7: return new Depth(f); 
        case 8: return new Type(f);
        case 9: return new Diameter(f);
		case 10: return new Diameter_pow(f);
        case 11: return new Length(f);
        case 12: return new Surface(f);
		case 13: return new SectionArea(f);
        case 14: return new Volume(f);
        case 15: return new EucDistance(f);
        case 16: return new PathDistance(f);
        case 17: return new Xyz(f);
        case 18: return new Branch_Order(f);
        case 19: return new Terminal_degree(f);
		case 20: return new TerminalSegment(f);
        case 21: return new Taper_1(f);
        case 22: return new Taper_2(f);
        case 23: return new Branch_pathlength(f);
        case 24: return new Contraction(f);
        case 25: return new Fragmentation(f);
        case 26: return new Daughter_Ratio(f);
        case 27: return new Parent_Daughter_Ratio(f);
        case 28: return new Partition_asymmetry(f);
        case 29: return new Rall_Power(f);
        case 30: return new Pk(f);
        case 31: return new Pk_classic(f);
        case 32: return new Pk_2(f);
        case 33: return new Bif_ampl_local(f);
        case 34: return new Bif_ampl_remote(f);
        case 35: return new Bif_tilt_local(f);
        case 36: return new Bif_tilt_remote(f);
        case 37: return new Bif_torque_local(f);
        case 38: return new Bif_torque_remote(f);
        case 39: return new Last_parent_diam(f);
        case 40: return new Diam_threshold(f);
        case 41: return new HillmanThreshold(f);
        case 42: return new Hausdorff(f);
        case 43: return new Helix(f);
		case 44: return new Fractal_Dim(f);
	    case 99: return new N_Class(f);

       }

	return new Zero();
}

	
void	Neuron::doPCA(){
	
	if(oriented==false){
		if(soma->getEnd()->getX()!=0 || soma->getEnd()->getY()!=0 || soma->getEnd()->getZ()!=0)
			translation(soma->getEnd()->getX(),soma->getEnd()->getY(),soma->getEnd()->getZ());
		
			pca();
	
		if(soma->getEnd()->getX()!=0 || soma->getEnd()->getY()!=0 || soma->getEnd()->getZ()!=0)
			translation(soma->getEnd()->getX(),soma->getEnd()->getY(),soma->getEnd()->getZ());
	
 		setOrder(soma);
		setParameters(soma);

		}
	oriented=true;
}

/*
bool Neuron::lookFor(char * string,ifstream* in){
	int ctr=0;
	char c[500];
	while(strncmp(string,c,strlen(string))!=0 && in->peek()!=-1){
		*in>>c;
	//cerr<<"c............... :"<<c<<"\n";
	if(strncmp(string,c,strlen(string))==0){
		//cerr<<"c............... :"<<c<<"  String........: "<<string<<"\n";
		return true;
	}

	}
		return false;
}
double * Neuron::getValues(double * ret){
	char c='y';
	double xSum=0,ySum=0,zSum=0,rSum=0,xSq=0,ySq=0,zSq=0,x,y,z,r;
	int count=0;

	{
		x=-1;y=-1;z=-1;
		while( c!=-1){

			while(c!='(' && c!=-1 &&c!=')'){
				in->peek();
				c=in->get();
			}
			if(c==')')
				break;
		//remove spaces after '('. added on 09/10/2010 by Sridevi. Otherwise the soma points are skipped 
			while(in->peek()==' ' && in->peek()!=-1 ) in->get();
			if((in->peek()>='0' && in->peek()<='9') || in->peek()=='-') {
				*in>>x;
				*in>>y;
				*in>>z;
				*in>>r;
				//cerr<<x<<" "<<y<<" "<<z<<" "<<r<<endl;
				xSum+=x;ySum+=y;zSum+=z;rSum+=r;
				xSq+=x*x;ySq+=y*y;zSq+=z*z;
				count++;
			}
			//go to end line
			while(c!=13 && c!=10){
				in->peek();
				c=in->get();

			}
			
			if(in->peek()==')')
				break;
		}
	
		ret[0]=xSum/count;
		ret[1]=ySum/count;
		ret[2]=zSum/count;
	
		double tmp=0;
		double xvar=0,yvar=0,zvar=0;
		tmp=xSq/count-ret[0]*ret[0];
		if(tmp>0) xvar=sqrt(tmp);
		tmp=ySq/count-ret[1]*ret[1];
		if(tmp>0)  yvar=sqrt(tmp);
		tmp=zSq/count-ret[2]*ret[2];
		if(tmp>0)  zvar=sqrt(tmp);
		double soma=(xvar+yvar+zvar)/3*4;
		ret[3]=soma;
		
		c=in->peek();
	}//else
	return ret;

}

int Neuron::NeurolAdd(int id, int type,double x0,double y0, double z0){

		int biforc[1000]; biforc[0]=1; biforc[1]=1;
		int index=0;int lastindex=0;int lookForCloseParenthesis=0;
		int pid;
        int markctr=0;
		pid=1;
        int endTree=0;
		while( in->peek()!=-1 && endTree==0){	
		int save=0;int foundBar=0;
		index=0;lookForCloseParenthesis=0;save=0;
		char c = 'y';double x,y,z,d;
		
			while(c!='(' && c!='|' && in->peek()!=-1 ){
				in->peek();
				c=in->get();
				if(lookForCloseParenthesis==1 && c==')')	
					lookForCloseParenthesis=2;

				if(c==10 ||c==13){
					index=0;
					lookForCloseParenthesis=1;
				}
				if(c==' ')	index++;
				if(lookForCloseParenthesis==2){
				
					char k[100];
					*in>>k;
		
					if(strcmp(k,"tree")==0){
							index=0;lastindex=0;
							biforc[2]=1;
							pid=1;
							endTree=1;
							break;
					}else if(strcmp(k,"split")==0){
							
							lookForCloseParenthesis=0;
					}
				}
			}
		
			if(c=='|' )	foundBar=1;

		
			//remove spaces after '('
			while(in->peek()==' ' && in->peek()!=-1 ) in->get();

			//if the line does not contains anything then save the previous id
			if(in->peek()==10 ||in->peek()==13)	save=1;
			// when found a '|' correct for pid
			if(lastindex==index && foundBar==1){
					int jj=0;
					foundBar=0;
					pid=biforc[index];
			}
			if(lastindex>index && save==1){
					//after a termination
					pid=biforc[index];
					lastindex=index;
			}	
			if(lastindex<index && save==1){
					//new biforcation
					biforc[index]=id;
					lastindex=index;
			}
			//probable location for error!!
			c=in->peek();
		
			if(c==-1){
					int stop=1;
			}
			// to skip marker blocks. when R is not present the line is skipped.
			int pos;
			pos=in->tellg();
			char tmp[100];		
			in->getline(tmp,100);
			if(strstr(tmp,"Marker")!=NULL || strstr(tmp,"Spines")!=NULL|| strstr(tmp,"marker")!=NULL||strstr(tmp,"spines")!=NULL)
			{
				markctr++;
			
				while(strstr(tmp,"End of markers")==NULL){
						pos=in->tellg();
						in->getline(tmp,100);
							
				}
			
			}
			in->seekg(pos);

				//if a number follow get the segment cooordinates
			if((in->peek()>='0' && in->peek()<='9') || in->peek()=='-') {
					id++;
					//get segment
					c=in->peek();
					x=-1;y=-1;z=-1;
					*in>>x;
					*in>>y;
					*in>>z;
					*in>>d;
					//cerr<<x<<' '<<y<<' '<<z<<' '<<d<<"\n";
					if(soma==0){
						//create a soma
						add(1,type,x-x0-0.001,y-y0-0.001,z-z0-0.001,d,-1);
						id=2; pid=1;
					}
 					add(id,type,x-x0,y-y0,z-z0,d,pid);
					pid=id;
			}
				//go to end line
			while(c!=13 && c!=-1 &&c!=10){
					in->peek();
						c=in->get();

			}
			
		}
		cerr<<"markctr:"<<markctr<<"\n";
	return id;
};

void Neuron::add(int id, int type, double x, double y, double z, double diam, int pid){

	double radius=diam/2;
	if(x==3.189 || y==3.189 ||z==3.189){
		double yy=y+z;
	}
	Vector* a=new Vector(x,y,z);
	Segment* s=new Segment(id,type,a,radius,pid);
	delete a;
	if(soma==NULL) {
		soma=s;			
		soma->setPrevFather(s);
	} else {
		addSegment(s);
	}
};





int Neuron::addPolar(int id, int type, double rho, double azi, double ele, double torch, double diam, int pid){
	if(type!=1){
		

		
		
		//fragmentation & contraction
		int fragment=(int)lp->getParameter(FRAGMENTATION)-1;
		double contract=lp->getParameter(CONTRACTION)*PI/180;
		
		//if fragmentation or contraction are not defined just add the segment
		if(fragment<2 || contract ==-1){
			addPolarSegment(id,type,rho,azi,ele,torch,diam,pid);
			return id;
		}

		double plusminus, alpha;
			if (rnd->rnd01 () < 0.5)
				plusminus = 1;
			else
				plusminus = -1;

		alpha = acos (contract)*plusminus;
		
		rho=rho/fragment;
		fragment=fragment/2;
		//add first segment with all angles right
		addPolarSegment(id,type,rho,azi,ele,torch,diam,pid);
		
		for(int i=0;i<fragment;i++){
			pid=id;id++;
			addPolarSegment(id,type,rho,contract,0,0,diam,pid);
			pid=id;id++;
			addPolarSegment(id,type,rho,-contract,0,0,diam,pid);
		}
		return id;
	}
	
	//else just add one segment
	addPolarSegment(id,type,rho,azi,ele,torch,diam,pid);
	return id;

}


void Neuron::addPolarSegment(int id, int type, double rho, double azi, double ele, double torch, double diam, int pid){
	//Create a Segment for each line
	double radius=diam/2;
	Vector* a=new Vector();
	a->setPolar(rho,azi,ele,torch);
	Segment* s=new Segment(id,type,a,radius,pid);

	if(soma==NULL) {
		soma=s;			
		soma->setPrevFather(s);
	} else {
		addSegment(s);
	}
}


double * Neuron::addSomaClaiborne(double * ret){


	double xSum=0,ySum=0,zSum=0,rSum=0,xSq=0,ySq=0,zSq=0,x,y,z,thick;
	int count=0;
	char  id[100],type[100];

	if(in->fail())
		in->clear();
	in->seekg(0,ios::beg);
	while ( in->peek()!=-1)
	{

		while (in->peek()==32){ 
			in->get();
		}
		if(in->peek()=='/'){
			in->get();
			if(in->peek()=='*'){
				int foundEnd=0;
				while(foundEnd==0){
					in->get();
					if(in->peek()=='*'){
						in->get();
						if(in->peek()=='/'){
							foundEnd=1;
							in->get();
						}
					}
				}
			}
		}

		char c=in->peek();
		*in>>id;
		if(strcmp(id,"794")==0){
			int iii=6;
		}
		*in>>type;

		*in>>x;
		*in>>y;
		*in>>z;
		*in>>thick;
	
		while (in->peek()==32  &&  in->peek()!=-1){ 
			in->get();
		}

		while (in->peek()!=13 && in->peek()!=10 && in->peek()!=-1){ 
			char c=in->get();
			
		}

		while(in->peek()==13 &&  in->peek()!=-1)
			if(in->peek()==13){
				in->get();
				if(in->peek()==10)
					in->get();
			}

		while (in->peek()==32  &&  in->peek()!=-1){ 
			in->get();
		}

		if(strcmp("S",type)==0 || strcmp("P",type)==0) {
			xSum+=x;ySum+=y;zSum+=z;
			xSq+=x*x;ySq+=y*y;zSq+=z*z;
			count++;
		}
	}

	double x0=xSum/count;
	double y0=ySum/count;
	double z0=zSum/count;
	
	double xvar=sqrt(xSq/count-x0*x0);
	double yvar=sqrt(ySq/count-y0*y0);
	double zvar=sqrt(zSq/count-z0*z0);

	double radius=(xvar+yvar+zvar)/3;

	ret[0]=x0;
	ret[1]=y0;
	ret[2]=z0;

	ret[3]=radius*2;
	return ret;

}

void Neuron::processSkippedSegments(Segment* ss){
	Segment* p = findSegment(ss->getId(),soma);
	if(p!=NULL){
		if(ss->getPid()==-100)
			ss->setPid(p->getId());
		addSegment(ss);
		reduceSkippedSegments(ss);
	}
	else if(ss->getNext1()!=NULL)
		processSkippedSegments(ss->getNext1());
}

void Neuron::reduceSkippedSegments(Segment* r){
	Segment* prev = r->getPrev();
	Segment* nxt = r->getNext1();
	if(prev!=NULL && nxt!=NULL){
		r->getPrev()->setNext1(r->getNext1());
		r->getNext1()->setPrev(r->getPrev());
	}
	r = NULL;

}
*/

void Neuron::OpenSWC(){
	int id=0,type=0,pid=0,tt=0;
	int commId=0;
	double x=0,y=0,z=0,radius=0;            

	Segment * old=new Segment();

	if(in->fail())
		in->clear();
		
	in->seekg(0,ios::beg);
	
	while ( !in->eof() )
	{

		while (in->peek()==32){
			in->get();
	
       }
		
		while (in->peek()==35 || in->peek()==37){
			in->getline(d[commId],200,'\n');
		
			float x,y,z;
			x=0;y=0;z=0;
			
			if(sscanf(d[commId],"# SHRINKAGE_CORRECTION %e %e %e",&x,&y,&z)==3){
				setShrinkX(x);
				setShrinkY(y);
				setShrinkZ(z);
			}

			while (in->peek()==32 || in->peek()==10 || in->peek()==13) {
				in->get();
			}
			
			if(commId<99) 
			{commId++;d[commId][0]=0;}
		}

	
		id=0;type=0;x=0;y=0;z=0;radius=0;type=0;pid=0;
	
		*in>>id;
	
	
		if(in->eof() || in->peek()==-1 ) {break;}

		read++;
		
		*in>>type;
		
		*in>>x;
	
		*in>>y;
	
		*in>>z;

		*in>>radius;
	
		*in>>pid;
	

		//if after reading something is left output "Maybe error"
		if(in->peek()!=32 && in->peek()!=10 && in->peek()!=13 && in->peek()!=-1){
			cerr<<"Maybe error at Id:"<<id<<"\n";
			char c=in->peek();
		}
		//Create a Segment for each line
		Vector* a=new Vector(x,y,z);
		Segment* s=new Segment(id,type,a,radius,pid);
		delete a;
	
     /* 
		if(soma!=NULL && pid==-1)
		{
			
			s->setPid(1);

		}
		//added this if condition, so that for files that have soma could be rearranged such that soma comes first. 
	*/	
		if(type==1){
		
				foundsoma=true;
		
		}if(soma==NULL){
		
			soma=s;	
		
			soma->setPrevFather(s);
		}else if(old->getId()==pid){
		
			old->addNewChild(s);
		}else{
		
			addSegment(s);
				}
		old=s;
	
	}


}
;
//searching for first soma compartment with type == 1
Segment * Neuron::lookForSoma(Segment *s){
    static Segment * f=NULL;

	if(s->getType()==1){
		if(f==NULL){
			f=s;
		} else {
			//f is a bifurcation and s is either continuation or tip
			if(s->getNext2()==NULL && f->getNext2()!=NULL)
				f=s;
		}

	}
	if(s->getNext1()!=NULL)
		lookForSoma(s->getNext1());
	if(s->getNext2()!=NULL)
		lookForSoma(s->getNext2());

	return f;

}

;
//add virtual soma as the beginning point and traverse the tree from there onwards.
void Neuron::addVirtualSoma(Segment* s){

	if(s->getType()!=1){
		if(s->getNext1()!=NULL)
			addVirtualSoma(s->getNext1());
		if(s->getNext2()!=NULL)
			addVirtualSoma(s->getNext2());
	}
	else{
	
		//we are in the soma
		Segment* virt=s->clone();
		//Ghost segment
		virt->setId(s->getId()+1);
		virt->setPid(-1);
		virt->setType(1);
		virt->setRadius(s->getRadius());
		s->setType(1);
		virt->setPrev(NULL);
		virt->setNext1(NULL);
		virt->setNext2(NULL);
		s->addNewChild(virt);
	}
}
// rearrage the swc file recursively such that the pointers are going out of the soma
Segment * Neuron::rearrangeSWC(Segment* s){
	static Segment * ret=NULL;
	
	
	if(s->getType()==1 && s->getNext1()==NULL && s->getNext2()==NULL && found==0){
		found=1;
		s->setNext1(s->getPrev());
		s->setPrev(s);

		rearrange(s->getPrev(),s);
		 ret= s;

	}
	else{
		if(s->getNext1()!=NULL)
			rearrangeSWC(s->getNext1());
		if(s->getNext2()!=NULL)
			rearrangeSWC(s->getNext2());
	}
	return ret;
}

void Neuron::rearrange(Segment* t, Segment* father){
	static Segment* prev;
	prev=t->getPrev();
	static Segment* next1;
	next1=t->getNext1();
	static Segment* next2;
	next2=t->getNext2();

	
	//if next1 or next2 are fathers then rearrange pointers
	if(next1==father){
		t->setPrev(next1);
		if(prev!=NULL) {
			t->setNext1(prev);
			t->setNext2(next2);
		} else {
			t->setNext1(next2);
			t->setNext2(NULL);
		
		}
	
	}
	else if(next2==father){
	
		t->setPrev(next2);
		t->setNext2(prev);
		t->setNext1(next1);
		

	}

	
	if(t->getNext1()!=NULL)
			rearrange(t->getNext1(),t);
	if(t->getNext2()!=NULL)
			rearrange(t->getNext2(),t);
}


void Neuron::setParameters(){
	setParameters(soma);
}

//set all necessary parameters inside each single segment required by successive Elaborations

void Neuron::setParameters(Segment* s){

	static double dist=0;
	if(s->getPid()==-1){

		
		s->setPathDistance(0);
		
	} else {
		
		s->setPathDistance(s->getPrev()->getPathDistance() +s->getPrev()->getEnd()->distance(s->getEnd()));
		
	}
	

	Vector * v1=s->getEnd();
	
	Vector * v2=soma->getEnd();
	
	dist=s->getEnd()->distance(soma->getEnd());
	s->setSomaDistance(dist);

	s->setNeuronName(name);

	if(s->getPid()==-1){
		s->setLength(0);
	}else{
		s->setLength(s->getEnd()->distance(s->getPrev()->getEnd()));
	}

	//check if terminate then set nrseg=1;
	if(s->isTerminate()) s->setNrTip(1);

	if(s->getNext1()!=NULL){
		setParameters(s->getNext1());
		s->setNrTip(s->getNext1()->getNrTip());
	
	}
	if(s->getNext2()!=NULL){
		setParameters(s->getNext2());

		s->setNrTip(s->getNext1()->getNrTip()+s->getNext2()->getNrTip());
	
	}

}



void Neuron::setOrder(Segment* s){
	if(s->getType()==1 ||s->getId()==1 || s==soma){
		s->setOrder(0);
		
	} else if (s->getPrev()->getNext1()!=NULL && s->getPrev()->getNext2()!=NULL && s->getPrev()->getType()>1){
		
		s->setOrder(s->getPrev()->getOrder()+1);

	} else {

		s->setOrder(s->getPrev()->getOrder());

	}
	if(s->getNext1()!=NULL)
		setOrder(s->getNext1());
	if(s->getNext2()!=NULL)
		setOrder(s->getNext2());

}
// find the right spot to insert s given its pid
void Neuron::addSegment(Segment* s){

	int pid=s->getPid();     

	Segment* p=findSegment(pid,soma);
		
	if(p==NULL){ 
		ferror=true;
		cerr<<"Did not found PARENT!!!!!!!!  "<<s->getId()<<" "<<s->getPid()<<" "<<s->getEnd()->getX()<<" "<<s->getEnd()->getY()<<" "<<s->getEnd()->getZ()<<"\n";
	}
	else{
	
			p->addNewChild(s);
		}
}

void Neuron::addSkippedSegments(Segment* firstseg, Segment * prev, Segment* p){
	
	if(firstseg->getNext1()!=NULL){
		addSkippedSegments(firstseg->getNext1(),firstseg,p);
	}
	else {
		firstseg->setNext1(p);
		firstseg->setPrev(prev);
	}
}

// Return the segment that with the given id starting from Father, NULL if it is missing.
Segment* Neuron::findSegment(int id,Segment* father){
	
Segment* tmp=NULL;

	if(father->getId()==id) 
	{
		tmp=father;
	
}


	if(tmp==NULL && father->getNext1()!=NULL){
		tmp=findSegment(id, father->getNext1());
		
	}
	if (tmp==NULL && father->getNext2()!=NULL){
				tmp=findSegment(id, father->getNext2());
	
	}
	
	return tmp;
}

Segment* Neuron::buildFathers(Segment* start,Segment * prevFather){
	Segment* tmp=NULL;
	Segment* tmp1=NULL;
	Segment* tmp2=NULL;
	
	Segment* tmpNext1=start->getNext1();
	Segment* tmpNext2=start->getNext2();

	start->setPrevFather(prevFather);
	
	if(tmpNext1==NULL && tmpNext2==NULL){
		//termination point acts as father
		return start;
	}

	//Check if start is a Father!!
	if(start->isFather()){ 
		prevFather=start;tmp=start;
		}
	
	//for every child recurse
	if( tmpNext1!=NULL){
		
		tmpNext1->setPrev(start);
		tmp1=buildFathers(tmpNext1,prevFather);
		start->setNextFather1(tmp1);
	}
	if( tmpNext2!=NULL){
		//set prev
		tmpNext2->setPrev(start);
		tmp2=buildFathers(tmpNext2,prevFather);
		start->setNextFather2(tmp2);
	}
	
	if(tmp==NULL && tmp1!=NULL){
		tmp=tmp1;
	}
	if(tmpNext1==NULL && tmpNext2 ==NULL){
		//if terminating point, return it's pointer
		tmp=start;
	}
	return tmp;
}

//Look for segments with same x,y,z and remove them!

void Neuron::findDuplicate(Segment* s){

	if(s->getNext1()==NULL && s->getNext2()==NULL)
		return;
	Segment *tmp;
	double x,y,z;
	x=s->getEnd()->getX();
	y=s->getEnd()->getY();
	z=s->getEnd()->getZ();

	if(s->getNext1()!=NULL && s->getEnd()->getX()==s->getNext1()->getEnd()->getX() && s->getEnd()->getY()==s->getNext1()->getEnd()->getY()&& s->getEnd()->getZ()==s->getNext1()->getEnd()->getZ())
		{	
		
		cerr<<"Found Duplicate type:"<<s->getType()<<" X:"<<s->getEnd()->getX()<< " " <<s->getEnd()->getY()<<" "<<s->getEnd()->getZ()<<" ";
		
		if(s->getNext2()==NULL){
			tmp=s->getNext1();
				s->setNext1( tmp->getNext1()); 

			if(tmp->getNext1()!=NULL)
			tmp->getNext1()->setPrev(s);

			s->setNext2( tmp->getNext2()); 
			if(tmp->getNext2()!=NULL)
				tmp->getNext2()->setPrev(s);

			tmp->~Segment();
			cerr<<"removed!\n";

		} else {
			if(s->getNext1()->getType()>0){
				cerr<<"Triforcation Readjusted!\n";
				s-> adjustTriforc();
			}else {
				cerr<<s->getId();
				cerr<<"virtual segment-> skipped!\n";

			}

		}
	}  
	
	else if( s->getNext2()!=NULL && s->getEnd()->getX()==s->getNext2()->getEnd()->getX()
		&& s->getEnd()->getY()==s->getNext2()->getEnd()->getY()
		&& s->getEnd()->getZ()==s->getNext2()->getEnd()->getZ()  ){




			cerr<<"Found Duplicate type:"<<s->getType()<<" X:"<<s->getEnd()->getX()<< " " <<s->getEnd()->getY()<<" "<<s->getEnd()->getZ()<<" ";
		

			if(s->getNext2()->getType()>0){
				cerr<<"Triforcation Readjusted!\n";
				s->adjustTriforc();
				
			} else {
				cerr<<"virtual segment-> skipped!\n";

			}
	}



	if(s->getNext1()!=NULL){

		findDuplicate(s->getNext1());
	
	}
	if(s->getNext2()!=NULL){

		findDuplicate(s->getNext2());
	}

}


void Neuron::exploreExecute(void*f) {

}


void Neuron::adjustShrinkage(Segment* s){
	static double x,y,z;
	static Vector* a;
	a=s->getEnd();
	x=a->getX()*getShrinkX();
	y=a->getY()*getShrinkY();
	z=a->getZ()*getShrinkZ();
	a->setX(x);
	a->setY(y);
	a->setZ(z);

	if(s->getNext1()!=NULL){
		adjustShrinkage(s->getNext1());
	}
	if(s->getNext2()!=NULL){
		adjustShrinkage(s->getNext2());
	}
}

void Neuron::renumerateId() {
	renumerateId(soma);
}

void Neuron::renumerateId(Segment* s) {
 
	static int id=0;

	 if(s==soma){
		 id=0;

	 
	 };
	 
	if (s->getType()>0 ){
		
		id++;
		s->setId(id);
		
		if(s->getId()>0 && s->getType()>0){

			if(s->getPrev()!=NULL){	
				Segment * tmp=s->getPrev();
			
				s->setPid(tmp->getId());
			} else {
 			s->setPid(-1);
			}
		}
	 
	}
	nrSeg++;
	if(s->getNext1()!=NULL){

		renumerateId(s->getNext1());
	}
	if(s->getNext2()!=NULL){

		renumerateId(s->getNext2());
	}
}

/*
void Neuron::LookForTrees(Segment* tmp,std::string dirname){

	Segment *next1 = tmp->getNext1();
	Segment *next2 = tmp->getNext2();

	if(tmp->getPid()!=-1 && tmp->getPrev()!=NULL){
	
		  if(tmp->getType()!=tmp->getPrev()->getType() && tmp->getType()>1 && (tmp->getPrev()->getType()== -1 || tmp->getPrev()->getType()==1)){
			std::string treename = name_str+"tree"+itoa(namectr,buff,10);

			treename+=tree_ext;

			new_dir = dir_str+dirname+"\\";
			if(_mkdir(new_dir.c_str())==0){

				cerr<<"created a new subdirectory for sub trees!";
			}
			std::string fulltreename = new_dir+treename;

			ofstream treefile(fulltreename.c_str(),ios::binary);
			flag=0;

			saveTree(tmp,treefile);
			namectr++;
		}
	}
	
	if(next1!=NULL){

		LookForTrees(next1,dirname);
	}
	if(next2!=NULL){

		LookForTrees(next2,dirname);
	}
}

//to save the tree relative to its position in a seperate file
void Neuron::saveTree(Segment *newtree,ofstream & treeout){
	double newID=0,newPID=0;

	Segment *firstSegment = newtree;
	if(flag==0){
		ctr=0;
		rf=firstSegment->getId();
		newID = 1;//firstSegment->getPrev()->getId()-rf+1;
		newPID = -1;//firstSegment->getPrev()->getPid()-rf+1;
		if(firstSegment->getPrev()->getPid()==-1)
		cerr<<"virtual id "<<firstSegment->getPrevFather()->getId()<<"pid "<<firstSegment->getPrevFather()->getPid()<<"\n";
		
		treeout<<newID<<" "<<firstSegment->getPrev()->getType()<<" "<<firstSegment->getPrev()->getEnd()->getX()<<" "<<firstSegment->getPrev()->getEnd()->getY()<<" "<<firstSegment->getPrev()->getEnd()->getZ()<<" "<<firstSegment->getPrev()->getRadius()<<" "<<newPID<<"\n";
		flag=1;
	}
	

	newID = firstSegment->getId()-rf+2;
	newPID = firstSegment->getPid()-rf+2;
	if(newPID==0)newPID=-1;	
	if(newID==2)newPID=1;
		treeout<<newID<<" "<<firstSegment->getType()<<" "<<firstSegment->getEnd()->getX()<<" "<<firstSegment->getEnd()->getY()<<" "<<firstSegment->getEnd()->getZ()<<" "<<firstSegment->getRadius()<<" "<<newPID<<"\n";

	if(firstSegment->getNext1()!=NULL){
		
		saveTree(firstSegment->getNext1(),treeout);
	}
	cerr<<"\n";
	if(firstSegment->getNext2()!=NULL){
		
		saveTree(firstSegment->getNext2(),treeout);
	}
			
}

*/

/*
void Neuron::printSWC()
{
  int i=0;
  char nm[300];
  strcpy(nm,name);

  strcat(nm,".swc");
  
  ofstream tfile( nm );//, ios::binary );
 
  while(d[i][0]!=0){
    tfile<<d[i++]<<"\n";

  }
  cout<<"\n";
  
  printSWC( soma, tfile);

  tfile.close();
}


void Neuron::printSWC(Segment * s, ofstream & out){
	if (s->getId()>0 && s->getType()>0)
          out << " " 
              << s->getId() 
              << " "<< s->getType() 
              << " " << s->getEnd()->getX() 
              << " " << s->getEnd()->getY()  
			  << " " << s->getEnd()->getZ()
              << " " << s->getRadius()  
			  << " " << s->getPid()  
			  << "\n";

	if(s->getNext1()!=NULL){
		printSWC(s->getNext1(),out);}
	if(s->getNext2()!=NULL){
		printSWC(s->getNext2(),out);}
}



void Neuron::electrotonic(Segment * t,double rm,double ri, int type){
	electrotonic(t,rm,ri,type,0,0,0);
}



void Neuron::electrotonic(Segment * t,double rm,double ri, int type,double xOffset,double yOffset,double zOffset){

	double xLoc=0,yLoc=0,zLoc=0;
	if(t->getPrev()!=NULL){
		//perfom offset, so if there was any correction the part will have the same direction
		Vector * end=t->getEnd();
		end->setX(end->getX()+xOffset);
		end->setY(end->getY()+yOffset);
		end->setZ(end->getZ()+zOffset);

		
		{
			Vector * prev=t->getPrev()->getEnd();
			double scale=sqrt(t->getDiam()/4.0)*sqrt(rm/ri);
			scale=2;
			
			//rescale in proportion to the amount on each axis
			xLoc= scale*(end->getX()-prev->getX());
			yLoc= scale*(end->getY()-prev->getY());
			zLoc= scale*(end->getZ()-prev->getZ());
			end->setX(prev->getX() + xLoc);
			end->setY(prev->getY() + yLoc);
			end->setZ(prev->getZ() + zLoc);
			
		} 

	}
	//recursion
	if(t->getNext1()!=NULL) electrotonic(t->getNext1(), rm, ri, type,xOffset+xLoc,yOffset+yLoc,zOffset+zLoc);
	if(t->getNext2()!=NULL) electrotonic(t->getNext2(), rm, ri, type,xOffset+xLoc,yOffset+yLoc,zOffset+zLoc);

}
;

void Neuron::translation(double x, double y, double z,Segment *s){
	s->getEnd()->setX(s->getEnd()->getX()-x);
	s->getEnd()->setY(s->getEnd()->getY()-y);
	s->getEnd()->setZ(s->getEnd()->getZ()-z);
	if(s->getNext1()!=NULL)
		translation(x,y,z,s->getNext1());
	if(s->getNext2()!=NULL)
		translation(x,y,z,s->getNext2());

};

void Neuron::translation(double x, double y, double z){
	translation(x,y,z,soma);
};

void Neuron::rotate(double x, double y, double z){
	rotate(x,y,z,soma);
};


void Neuron::rotateTo(Vector * a){
	Vector *b =new Vector();
	Vector *yAxis =new Vector(0,1,0);
	Vector * org=new Vector();
	b->crossProduct(org,yAxis,a);
	double theta;
	theta=org->angle(a,yAxis);

	rotateAxis(b,theta*PI/180,soma);
	

	delete b;
	delete yAxis;
	delete org;
};

void Neuron::rotateAxis(Vector * a, double theta,Segment *s){
	if(s==soma){
		doPCA();
	}


	s->getEnd()->rotateAxis(a,theta);

	if(s->getNext1()!=NULL)
		rotateAxis(a,theta,s->getNext1());
	if(s->getNext2()!=NULL)
		rotateAxis(a,theta,s->getNext2());
	

};*/


void Neuron::rotate(double alpha, double beta, double gamma,Segment *s){
	static double cx=0,sx=0,cy=0,sy=0,cz=0,sz=0;
	if(s==soma){
		alpha=alpha*PI/180;
		beta=beta*PI/180;
		gamma=gamma*PI/180;

		cx=cos(alpha);sx=sin(alpha);
		cy=cos(beta);sy=sin(beta);
		cz=cos(gamma);sz=sin(gamma);
	}
	double x=s->getEnd()->getX();
	double y=s->getEnd()->getY();
	double z=s->getEnd()->getZ();
	double x1,y1,z1;
	x1=x,y1=y;z1=z;

	x1=x*cy*cz+y*cy*sz-z*sy;
	y1=x*(sx*sy*cz-cx*sz)+y*(sx*sy*sz+cx*cz)+z*sx*cy;
	z1=x*(cx*sy*cz+sx*sz)+y*((cx*sy*sz-sx*cz))+z*(cx*cy);
	x;x1;y;y1;z;z1;
	s->getEnd()->setX( x1 );
	s->getEnd()->setY(y1);
	s->getEnd()->setZ(z1);
	if(s->getNext1()!=NULL)
		rotate(alpha,beta,gamma,s->getNext1());
	if(s->getNext2()!=NULL)
		rotate(alpha,beta,gamma,s->getNext2());

};


void Neuron::rotateP(Vector * P){
	rotateP(P,soma);
};

void Neuron::rotateP(Vector *P,Segment *s){
	
	s->getEnd()->rotateP(P);

	if(s->getNext1()!=NULL)
		rotateP(P,s->getNext1());
	if(s->getNext2()!=NULL)
		rotateP(P,s->getNext2());

};



void Neuron::pca(){
	float **data, **symmat, **symmat2,  *evals, *interm;	//create a matrix that can contains all points

	int i,j;

	int n=nrSeg;

	int m=3;
	
	data=matrix(n,m);
	for( i=1;i<=n;i++)
		for( j=1;j<=m;j++){
			if(j%100==0){
				int kk=j;
			}
			data[i][j]=0;
		}

	if(data==NULL)
		exit(-1);

	n=pca(soma,data);
	
    symmat = matrix(m, m);  /* Allocation of correlation (etc.) matrix */
    float *avg=vector1(m);

    covcol1(data, n, m, symmat,avg);



    evals = vector1(m);     /* Storage alloc. for vector of eigenvalues */
    interm = vector1(m);    /* Storage alloc. for 'intermediate' vector */
    symmat2 = matrix(m, m);  /* Duplicate of correlation (etc.) matrix */
    for (i = 1; i <= m; i++) {
     for (int j = 1; j <= m; j++) {
      symmat2[i][j] = symmat[i][j]; /* Needed below for col. projections */
                              }
                             }
    tred2(symmat, m, evals, interm);  /* Triangular decomposition */
    tqli(evals, interm, m, symmat);   /* Reduction of sym. trid. matrix */
 
	//rotate the neuron to the PCA
	multiply(soma,symmat,avg,evals);

		//move soma to 0,0,0
	if(soma->getEnd()->getX()!=0 || soma->getEnd()->getY()!=0 || soma->getEnd()->getZ()!=0)
		translation(soma->getEnd()->getX(),soma->getEnd()->getY(),soma->getEnd()->getZ());

	
	//fill matrix
	pcaRefill(soma,data);



	if(true){
		Vector *e=new Vector();
		
		e->set(1,0,0);
		double xcur=	cursor(e,soma);
		e->set(0,1,0);
		double ycur=	cursor(e,soma);
		e->set(0,0,1);
		double zcur=	cursor(e,soma);
		e->set(1,0,0);
		double absX=cursorAbs(e,soma);
		e->set(0,1,0);
		double absY=cursorAbs(e,soma);
		e->set(0,0,1);
		double absZ=cursorAbs(e,soma);
	
		float * stddev = vector1(m);
		stdDev(data, n, m, stddev);
		double xd=stddev[1],yd=stddev[2],zd=stddev[3];
	
		double perc=absX/absY+absX/absZ+absY/absZ+absY/absX+absZ/absX+absZ/absY;
		//rotate of 90 degree on the major axes
		if(perc>6.5){
			if(absX>absY && absX>absZ){
				if(xcur>0)
					rotate((double)0,0,90);
				if(xcur<0)
					rotate((double)0,0,-90);
			} else 	if(absY>absX && absY>absZ){
				if(ycur>0)
					rotate((double)0,0,0);//ok
				if(ycur<0)
					rotate((double)0,0,180);
			} else if (absZ>absX && absZ>absY){
				if(zcur>0)
 					rotate(90,0,0);//ok
				if(zcur<0)
 					rotate(-90,0,0);//ok
			}
		}else { 
			//rotate depending on stddev
			if(xd>yd && xd>zd){
				if(xcur>0)
					rotate(-90,0,0);
				if(xcur<0)
					rotate(90,0,0);
			} else 	if(yd>xd && yd>zd){
				if(ycur>0)
					rotate(0,0,-90);//ok
				if(ycur<0)
					rotate(0,0,90);
			} else if (zd>xd && zd>yd){
				if(zcur>0)
 					rotate(0,0,90);//ok
				if(zcur<0)
 					rotate(0,0,-90);//ok
			}

		}

	//move soma to 0,0,0
	if(soma->getEnd()->getX()!=0 || soma->getEnd()->getY()!=0 || soma->getEnd()->getZ()!=0)
		translation(soma->getEnd()->getX(),soma->getEnd()->getY(),soma->getEnd()->getZ());


		pcaRefill(soma,data);

		stdDev(data, n, m, stddev);


		e->set(1,0,0);
		 xcur=	cursor(e,soma);
		e->set(0,1,0);
		 ycur=	cursor(e,soma);
		e->set(0,0,1);
		 zcur=	cursor(e,soma);
		e->set(1,0,0);
		 absX=cursorAbs(e,soma);
		e->set(0,1,0);
		 absY=cursorAbs(e,soma);
		e->set(0,0,1);
		 absZ=cursorAbs(e,soma);
		
		 stddev = vector1(m);
		stdDev(data, n, m, stddev);
		 xd=stddev[1],yd=stddev[2],zd=stddev[3];
		
		 perc=absX/absY+absX/absZ+absY/absZ+absY/absX+absZ/absX+absZ/absY;
		
		 if(ycur<0){
		
				 rotate(180,0,0);
		 }

	//move soma to 0,0,0
	if(soma->getEnd()->getX()!=0 || soma->getEnd()->getY()!=0 || soma->getEnd()->getZ()!=0)
		translation(soma->getEnd()->getX(),soma->getEnd()->getY(),soma->getEnd()->getZ());

		//refill matrix and compute std dev
		pcaRefill(soma,data);

		stdDev(data, n, m, stddev);


		e->set(1,0,0);
		 xcur=	cursor(e,soma);
		e->set(0,1,0);
		 ycur=	cursor(e,soma);
		e->set(0,0,1);
		 zcur=	cursor(e,soma);
		e->set(1,0,0);
		 absX=cursorAbs(e,soma);
		e->set(0,1,0);
		 absY=cursorAbs(e,soma);
		e->set(0,0,1);
		 absZ=cursorAbs(e,soma);
		delete e;
		 stddev = vector1(m);
		stdDev(data, n, m, stddev);
		 xd=stddev[1],yd=stddev[2],zd=stddev[3];
		free_vector(stddev, m);
		 perc=absX/absY+absX/absZ+absY/absZ+absY/absX+absZ/absX+absZ/absY;

		 if(perc>7){
			 if(zd>xd || absZ>absX){
				 
				 rotate((double)0,-90,0);
			 }
		 } else {
			 //absCursor too close to each other
			 if(zd>xd){
				 rotate((double)0,0,0);
			 }
		 }

	}



    free_matrix(data, n, m);

    free_matrix(symmat, m, m);
    free_matrix(symmat2, m, m);
    free_vector(evals, m);
    free_vector(avg, m);
    free_vector(interm, m);

};


double Neuron::cursor(Vector *v,Segment *t){
	static Vector * orig=new Vector(0,0,0);
	static Vector * seg=new Vector();
	double tmp=0;
	if(t->getPrev()!=NULL){

		seg->setX(t->getEnd()->getX());
		seg->setY(t->getEnd()->getY());
		seg->setZ(t->getEnd()->getZ());
		tmp=v->scalar(orig,v,seg);
		tmp=v->angle(orig,v,orig,seg);
		tmp=cos(tmp*PI/180);
	}
	if(t->getNext1()!=NULL)
		tmp+=cursor(v,t->getNext1());
	if(t->getNext2()!=NULL)
		tmp+=cursor(v,t->getNext2());
	

	return tmp;
};

double Neuron::cursorAbs(Vector *v,Segment *t){
	static Vector * orig=new Vector(0,0,0);
	static Vector * seg=new Vector();
	double tmp=0;
	if(t->getPrev()!=NULL){
	
		seg->setX(t->getEnd()->getX());
		seg->setY(t->getEnd()->getY());
		seg->setZ(t->getEnd()->getZ());
		tmp=fabs(v->scalar(orig,v,seg));
		tmp=v->angle(orig,v,orig,seg);
		tmp=fabs(cos(tmp*PI/180));
	}
	if(t->getNext1()!=NULL)
		tmp+=cursorAbs(v,t->getNext1());
	if(t->getNext2()!=NULL)
		tmp+=cursorAbs(v,t->getNext2());
	
	return tmp;
};

void Neuron::multiply(Segment *t,float **mat,float *avg,float *evals){
	float *    interm = vector1(3);
	float *    data = vector1(3);
	float *    out = vector1(3);
 	static int i=0;
	if(t==soma)
		i=1;
	data[1]=t->getEnd()->getX()-avg[1];
	data[2]=t->getEnd()->getY()-avg[2];
	data[3]=t->getEnd()->getZ()-avg[3];

	int m=3;
        for (int k = 1; k <= 3; k++) {
		  out[k]=0;
          for (int  k2 = 1; k2 <= m; k2++) {
            out[k] += data[k2] * mat[k2][m-k+1]; 
		  }
		  if(0==1){
	        if (evals[m-k+1] > 0.0005)   // Guard against zero eigenvalue 
		       out[k] /=(float) sqrt(evals[m-k+1]);   // Rescale 
			else
				out[k] = 0.0;    // Standard kludge 
		  }
		  if(fabs(out[k])<0.000001)
			  out[k]=0;
		}
   

	
	t->getEnd()->setX(out[1]);
	t->getEnd()->setY(out[2]);
	t->getEnd()->setZ(out[3]);
	i++;
	
	if(t->getNext1()!=NULL)
		multiply(t->getNext1(),mat,avg,evals);
	if(t->getNext2()!=NULL)
		multiply(t->getNext2(),mat,avg,evals);

	free_vector(interm,3);
	free_vector(data,3);
	free_vector(out,3);


}


void Neuron::moveToMean(Segment * t){
	static int x=0,y=0,z=0,i=0;
	if(t->getId()>0){
		x += (int)t->getEnd()->getX();
		y += (int)t->getEnd()->getY();
		z += (int)t->getEnd()->getZ();
		i++;
	}
	if(t->getNext1()!=NULL)
		moveToMean(t->getNext1());
	if(t->getNext2()!=NULL)
		moveToMean(t->getNext2());
	if(t==soma)
		translation(x/i,y/i,z/i);
}



int Neuron::pca(Segment * t,float **data){
	static int id=0;
	if(t==soma)
		id=1;
	

	if(t->getType()>2 && t->getType()<6){
		double d;
		d=1;
		if(t->getPathDistance()>300){
			d=0.001;
		}
		
		if(d==0) d=1;
		data[id][1]=t->getEnd()->getX()/d;
		data[id][2]=t->getEnd()->getY()/d;
		data[id][3]=t->getEnd()->getZ()/d;
		id++;
	}
	if(t->getNext1()!=NULL)
		pca(t->getNext1(),data);
	if(t->getNext2()!=NULL)
		pca(t->getNext2(),data);

	return id;

}

;

int Neuron::pcaRefill(Segment * t,float **data){
	static int id=0;
	if(t==soma)
		id=1;
	

	if(t->getType()>2 && t->getType()<6){
		data[id][1]=t->getEnd()->getX();
		data[id][2]=t->getEnd()->getY();
		data[id][3]=t->getEnd()->getZ();
		id++;
	}
	if(t->getNext1()!=NULL)
		pcaRefill(t->getNext1(),data);
	if(t->getNext2()!=NULL)
		pcaRefill(t->getNext2(),data);

	return id;

}

;

void Neuron::pcaStore(Segment * t,float **data){
	//rotate and recenter all points
	static int id=0;
	if(t==soma)
		id=1;
	if(id%610==0){
		int hh=id;
	}
	if(t->getEnd()==NULL)
		return;
	
	t->getEnd()->setX(data[id][1]);
	t->getEnd()->setY(data[id][2]);
	t->getEnd()->setZ(data[id][3]);
	id++;
	if(t->getNext1()!=NULL && t->getNext1()->getEnd()==NULL)
		t->setNext1(NULL);
	if(t->getNext1()!=NULL)
		pcaStore(t->getNext1(),data);
	if(t->getNext2()!=NULL)
		pcaStore(t->getNext2(),data);

}

;

void stdDev(float **data,int n,int m,float *stddev)

{
float eps =(float) 0.005;
float  *mean;// *stddev;//, *vector();
int j,i;

/* Allocate storage for mean and std. dev. vectors */

mean = vector1(m);

/* Determine mean of column vectors of input data matrix */

for (j = 1; j <= m; j++)
    {
    mean[j] = 0.0;
    for (i = 1; i <= n; i++)
        {
        mean[j] += data[i][j];
        }
    mean[j] /= (float)n;

    }



for (j = 1; j <= m; j++)
    {
    stddev[j] = 0.0;
    for (i = 1; i <= n; i++)
        {
        stddev[j] += (   ( data[i][j] - mean[j] ) *
                         ( data[i][j] - mean[j] )  );
        }
        stddev[j] /= (float)n;
        stddev[j] =(float) sqrt(stddev[j]);
       
        if (stddev[j] <= eps) stddev[j] = 1.0;
    }

free_vector(mean,m);
}
;



/**  Correlation matrix: creation  ***********************************/

void corcol(float **data,int n,int m,float ** symmat,float *avg)

{
float eps =(float) 0.005;
float x, *mean, *stddev;//, *vector();
int i, j, j1, j2;

/* Allocate storage for mean and std. dev. vectors */

mean = vector1(m);
stddev = vector1(m);

/* Determine mean of column vectors of input data matrix */

for (j = 1; j <= m; j++)
    {
    mean[j] = 0.0;
    for (i = 1; i <= n; i++)
        {
        mean[j] += data[i][j];
        }
    mean[j] /= (float)n;
	avg[j]=mean[j];
    }


/* Determine standard deviations of column vectors of data matrix. */
stddev = vector1(m);

for (j = 1; j <= m; j++)
    {
    stddev[j] = 0.0;
    for (i = 1; i <= n; i++)
        {
        stddev[j] += (   ( data[i][j] - mean[j] ) *
                         ( data[i][j] - mean[j] )  );
        }
        stddev[j] /= (float)n;
        stddev[j] =(float) sqrt(stddev[j]);
       
        if (stddev[j] <= eps) stddev[j] = 1.0;
    }


/* Center and reduce the column vectors. */

for (i = 1; i <= n; i++)
    {
    for (j = 1; j <= m; j++)
        {
        data[i][j] -= mean[j];
        x = (float) sqrt((float)n);
        x *= stddev[j];
        data[i][j] /= x;
        }
    }

/* Calculate the m * m correlation matrix. */
for (j1 = 1; j1 <= m-1; j1++)
    {
    symmat[j1][j1] = 1.0;
    for (j2 = j1+1; j2 <= m; j2++)
        {
        symmat[j1][j2] = 0.0;
        for (i = 1; i <= n; i++)
            {
            symmat[j1][j2] += ( data[i][j1] * data[i][j2]);
            }
        symmat[j2][j1] = symmat[j1][j2];
        }
    }
    symmat[m][m] = 1.0;
	free_vector(mean,m);
	free_vector(stddev,m);

return;

}


/** Standard Dev */
void stddev(float **data, float *std){

}
/**  Variance-covariance matrix: creation  *****************************/

void covcol1(float **data,int n,int m,float ** symmat,float *avg)

{
float *mean;//, *vector();
int i, j, j1, j2;

/* Allocate storage for mean vector */

mean = vector1(m);

/* Determine mean of column vectors of input data matrix */

for (j = 1; j <= m; j++)
    {
    mean[j] = 0.0;
    for (i = 1; i <= n; i++)
        {
		if(i==809){
			int hh=j;
		}
        mean[j] += data[i][j];
        }
    mean[j] /= (float)n;
	avg[j]=mean[j];
    }




/* Determine standard deviations of column vectors of data matrix. */
float *stddev = vector1(m);

for (j = 1; j <= m; j++)
    {
    stddev[j] = 0.0;
    for (i = 1; i <= n; i++)
        {
        stddev[j] += (   ( data[i][j] - mean[j] ) *
                         ( data[i][j] - mean[j] )  );
        }
        stddev[j] /= (float)n;
        stddev[j] =(float) sqrt(stddev[j]);
       
        if (stddev[j] <= 0.0001) stddev[j] = 1.0;
    }





/* Center the column vectors. */

for (i = 1; i <= n; i++)
    {
    for (j = 1; j <= m; j++)
        {
        data[i][j] -= mean[j];
        }
    }

/* Calculate the m * m covariance matrix. */
for (j1 = 1; j1 <= m; j1++)
    {
    for (j2 = j1; j2 <= m; j2++)
        {
        symmat[j1][j2] = 0.0;
        for (i = 1; i <= n; i++)
            {
            symmat[j1][j2] += data[i][j1] * data[i][j2];
            }
        symmat[j2][j1] = symmat[j1][j2];
        }
    }

return;

}

/**  Sums-of-squares-and-cross-products matrix: creation  **************/

void scpcol(float **data,int n,int m,float ** symmat)

{
int i, j1, j2;

/* Calculate the m * m sums-of-squares-and-cross-products matrix. */

for (j1 = 1; j1 <= m; j1++)
    {
    for (j2 = j1; j2 <= m; j2++)
        {
        symmat[j1][j2] = 0.0;
        for (i = 1; i <= n; i++)
            {
            symmat[j1][j2] += data[i][j1] * data[i][j2];
            }
        symmat[j2][j1] = symmat[j1][j2];
        }
    }

return;

}

/**  Error handler  **************************************************/

void erhand(char *err_msg)

/* Error handler */
{
    fprintf(stderr,"Run-time error:\n");
    fprintf(stderr,"%s\n", err_msg);
    fprintf(stderr,"Exiting to system.\n");
    exit(1);
}

/**  Allocation of vector storage  ***********************************/
/* Allocates a float vector with range [1..n]. */

float *vector1(int n){
   float *v;

    v = (float *) malloc ((unsigned) n*sizeof(float));
    if (!v) erhand("Allocation failure in vector().");
    return v-1;

}

/**  Allocation of float matrix storage  *****************************/
/* Allocate a float matrix with range [1..n][1..m]. */

float **matrix(int n,int m)
{
    int i;
    float **mat;

    /* Allocate pointers to rows. */
    mat = (float **) malloc((unsigned) (n)*sizeof(float*));
    if (!mat) erhand("Allocation failure 1 in matrix().");
    mat -= 1;

    /* Allocate rows and set pointers to them. */
    for (i = 1; i <= n; i++)
        {
        mat[i] = (float *) malloc((unsigned) (m)*sizeof(float));
        if (!mat[i]) erhand("Allocation failure 2 in matrix().");
        mat[i] -= 1;
        }

     /* Return pointer to array of pointers to rows. */
     return mat;

}

/**  Deallocate vector storage  *********************************/

void free_vector(float *v,int n)

/* Free a float vector allocated by vector(). */
{
   free((char*) (v+1));
}

/**  Deallocate float matrix storage  ***************************/

void free_matrix(float **mat,int n,int m)

/* Free a float matrix allocated by matrix(). */
{
   int i;

   for (i = n; i >= 1; i--)
       {
       free ((char*) (mat[i]+1));
       }
   free ((char*) (mat+1));
}

/**  Reduce a real, symmetric matrix to a symmetric, tridiag. matrix. */

void tred2(float **a,int  n, float *d, float *e)



/* Householder reduction of matrix a to tridiagonal form.
   Algorithm: Martin et al., Num. Math. 11, 181-195, 1968.
   Ref: Smith et al., Matrix Eigensystem Routines -- EISPACK Guide
        Springer-Verlag, 1976, pp. 489-494.
        W H Press et al., Numerical Recipes in C, Cambridge U P,
        1988, pp. 373-374.  */
{
int l, k, j, i;
float scale, hh, h, g, f;

for (i = n; i >= 2; i--)
    {
    l = i - 1;
    h = scale = 0.0;
    if (l > 1)
       {
       for (k = 1; k <= l; k++)
           scale +=(float) fabs(a[i][k]);
       if (scale == 0.0)
          e[i] = a[i][l];
       else
          {
          for (k = 1; k <= l; k++)
              {
              a[i][k] /= scale;
              h += a[i][k] * a[i][k];
              }
          f = a[i][l];
          g =(float)( f>0 ? -sqrt(h) : sqrt(h));
          e[i] = scale * g;
          h -= f * g;
          a[i][l] = f - g;
          f = 0.0;
          for (j = 1; j <= l; j++)
              {
              a[j][i] = a[i][j]/h;
              g = 0.0;
              for (k = 1; k <= j; k++)
                  g += a[j][k] * a[i][k];
              for (k = j+1; k <= l; k++)
                  g += a[k][j] * a[i][k];
              e[j] = g / h;
              f += e[j] * a[i][j];
              }
          hh = f / (h + h);
          for (j = 1; j <= l; j++)
              {
              f = a[i][j];
              e[j] = g = e[j] - hh * f;
              for (k = 1; k <= j; k++)
                  a[j][k] -= (f * e[k] + g * a[i][k]);
              }
         }
    }
    else
        e[i] = a[i][l];
    d[i] = h;
    }
d[1] = 0.0;
e[1] = 0.0;
for (i = 1; i <= n; i++)
    {
    l = i - 1;
    if (d[i])
       {
       for (j = 1; j <= l; j++)
           {
           g = 0.0;
           for (k = 1; k <= l; k++)
               g += a[i][k] * a[k][j];
           for (k = 1; k <= l; k++)
               a[k][j] -= g * a[k][i];
           }
       }
       d[i] = a[i][i];
       a[i][i] = 1.0;
       for (j = 1; j <= l; j++)
           a[j][i] = a[i][j] = 0.0;
    }
}

/**  Tridiagonal QL algorithm -- Implicit  **********************/

void tqli(float d[],float e[],int n,float **z)

{
int m, l, iter, i, k;
float s, r, p, g, f, dd, c, b;


for (i = 2; i <= n; i++)
    e[i-1] = e[i];
e[n] = 0.0;
for (l = 1; l <= n; l++)
    {
    iter = 0;
    do
      {
      for (m = l; m <= n-1; m++)
          {
          dd =(float)( fabs(d[m]) + fabs(d[m+1]));
          if (fabs(e[m]) + dd == dd) break;
          }
          if (m != l)
             {
             if (iter++ == 30) erhand("No convergence in TLQI.");
             g =(float) ((d[l+1] - d[l]) / (2.0 * e[l]));
             r =(float) (sqrt((g * g) + 1.0));
             g =(float) (d[m] - d[l] + e[l] / (g + SIGN(r, g)));
             s = c = 1.0;
             p = 0.0;
             for (i = m-1; i >= l; i--)
                 {
                 f = s * e[i];
                 b = c * e[i];
                 if (fabs(f) >= fabs(g))
                    {
                    c = g / f;
                    r =(float) sqrt((c * c) + 1.0);
                    e[i+1] = f * r;
                    c =(float) (c*(s = (float) 1.0/r));
                    }
                 else
                    {
                    s = f / g;
                    r =(float) sqrt((s * s) + 1.0);
                    e[i+1] = g * r;
                    s =(float)( s*(c =(float) 1.0/r));
                    }
                 g = d[i+1] - p;
                 r = (float)((d[i] - g) * s + 2.0 * c * b);
                 p = s * r;
                 d[i+1] = g + p;
                 g = c * r - b;
                 for (k = 1; k <= n; k++)
                     {
                     f = z[k][i+1];
                     z[k][i+1] = s * z[k][i] + c * f;
                     z[k][i] = c * z[k][i] - s * f;
                     }
                 }
                 d[l] = d[l] - p;
                 e[l] = g;
                 e[m] = 0.0;
             }
          }  while (m != l);
      }
 }
;
/*
//modified this method to print the output into a sperate .wrl file instead of Lmout.txt. Sri 08/16/2010
void Neuron::printWRL(){
  int i=0;
  char nm[300];
  strcpy(nm,name);
  strcat(nm,".wrl");
  
  ofstream tfile( nm );//, ios::binary );
 
  cout<<"\n";
	tfile<<"#VRML V2.0 utf8\n";
	
	printWRL(soma, tfile);
	
}
;

void Neuron::printWRL(Segment *t, ofstream & out){
	if (t->getId()>0  && t->getPrev()!=NULL){
		//DIV transform um in mm
		static int RATIO=2, DIV=1000;
		double x,y,z,rad;
		out<<"\n# Segment #:"<<t->getId()<<"\n";
		out<<"Shape {\n";
		out<<"  appearance Appearance {\n";
		out<<"    material Material {\n";
		out<<"      diffuseColor 1 1 1\n";
		out<<"    } \n";// # end material\n";
		out<<"  } \n";// # end appearance\n";

		out<<"  geometry IndexedFaceSet {\n";

		out<<"    solid TRUE\n";
		out<<"    coord Coordinate {\n";
		out<<"      point [\n";

		x=t->getPrev()->getEnd()->getX()/DIV;
		y=t->getPrev()->getEnd()->getY()/DIV;
		z=t->getPrev()->getEnd()->getZ()/DIV;
		rad=t->getRadius()/DIV;
		rad=0.001;
		out<< x+rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";
		out<< x+rad*RATIO<<" "<<y<<" "<<z-rad*RATIO<<",\n";
		out<< x-rad*RATIO<<" "<<y<<" "<<z-rad*RATIO<<",\n";
		out<< x-rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";

		x=t->getEnd()->getX()/DIV;
		y=t->getEnd()->getY()/DIV;
		z=t->getEnd()->getZ()/DIV;

		out<< x+rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";
		out<< x+rad*RATIO<<" "<<y<<" "<<z-rad*RATIO<<",\n";
		out<< x-rad*RATIO<<" "<<y<<" "<<z-rad*RATIO<<",\n";
		out<< x-rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";

		out<<"        ] \n";//  # end point\n";//
		out<<"    } \n";// # end coord\n";
		out<<"    coordIndex [ \n";

		out<<"0, 4, 7, 3, -1,\n";
		out<<"2, 3, 7, 6, -1,\n";
 		out<<"1, 2, 6, 5, -1,\n";
 		out<<"1, 0, 4, 5, -1,\n";
     
		out<<"    ] \n";// # end coordIndex\n";

		out<<"  } \n";// # end geometry\n";
		out<<"} \n";// # end shape\n";
	}
	if(t->getNext1()!=NULL){
		printWRL(t->getNext1(), out);}
	if(t->getNext2()!=NULL){
		printWRL(t->getNext2(), out);}
}
;void Neuron::printLineWRL(){

	cout<<"#VRML V2.0 utf8\n";
	printLineWRL(soma);
	cout<<"#";
}
;

void Neuron::printLineWRL(Segment *t){
	if (t->getId()>0  && t->getPrev()!=NULL){

		static int RATIO=1, DIV=1000;
		double x,y,z,rad;
		cout<<"\n# Segment #:"<<t->getId()<<"\n";
		cout<<"Shape {\n";
		cout<<"  appearance Appearance {\n";
		cout<<"    material Material {\n";
		cout<<"      diffuseColor 1 1 1\n";
		cout<<"    } \n";// # end material\n";
		cout<<"  } \n";// # end appearance\n";

		cout<<"  geometry IndexedLineSet  {\n";

		cout<<"    solid TRUE\n";
		cout<<"    coord Coordinate {\n";
		cout<<"      point [\n";


		x=t->getPrev()->getEnd()->getX()/DIV;
		y=t->getPrev()->getEnd()->getY()/DIV;
		z=t->getPrev()->getEnd()->getZ()/DIV;
		rad=t->getRadius()/DIV;
		cout<< x+rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";

		x=t->getEnd()->getX()/DIV;
		y=t->getEnd()->getY()/DIV;
		z=t->getEnd()->getZ()/DIV;
	
		cout<< x+rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";

		cout<<"        ] \n";//  # end point\n";//
		cout<<"    } \n";// # end coord\n";
		cout<<"    coordIndex [ \n";

		cout<<"0, 1, -1,\n";
     
		cout<<"    ] \n";// # end coordIndex\n";

		cout<<"  } \n";// # end geometry\n";
		cout<<"} \n";// # end shape\n";
	}
	if(t->getNext1()!=NULL){
		printLineWRL(t->getNext1());}
	if(t->getNext2()!=NULL){
		printLineWRL(t->getNext2());}
}
;;void Neuron::printFatherLineWRL(){

	cout<<"#VRML V2.0 utf8\n";
	printFatherLineWRL(soma);
	cout<<"#";
}
;

void Neuron::printFatherLineWRL(Segment *t){
	if (t->getId()>0  && t->getType()>0 && t->getPrevFather()!=NULL){

		static int RATIO=1, DIV=1000;
		double x,y,z,rad;
		x=t->getPrevFather()->getEnd()->getX()/DIV;
		y=t->getPrevFather()->getEnd()->getY()/DIV;
		z=t->getPrevFather()->getEnd()->getZ()/DIV;
		cout<<"\n# Segment #:"<<t->getId()<<"\n";
		cout<<"Shape {\n";
		cout<<"  appearance Appearance {\n";
		cout<<"    material Material {\n";
		cout<<"      diffuseColor 1 1 1\n";
		cout<<"    } \n";// # end material\n";
		cout<<"  } \n";// # end appearance\n";

		cout<<"  geometry IndexedLineSet  {\n";

		cout<<"    solid TRUE\n";
		cout<<"    coord Coordinate {\n";
		cout<<"      point [\n";


		rad=t->getRadius()/DIV;
		cout<< x+rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";

		x=t->getEnd()->getX()/DIV;
		y=t->getEnd()->getY()/DIV;
		z=t->getEnd()->getZ()/DIV;
	
		cout<< x+rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";

		cout<<"        ] \n";//  # end point\n";//
		cout<<"    } \n";// # end coord\n";
		cout<<"    coordIndex [ \n";

		cout<<"0, 1, -1,\n";
    
		cout<<"    ] \n";// # end coordIndex\n";

		cout<<"  } \n";// # end geometry\n";
		cout<<"} \n";// # end shape\n";


	}
	if(t->getNextFather1()!=NULL){
		printFatherLineWRL(t->getNextFather1());}
	if(t->getNextFather2()!=NULL){
		printFatherLineWRL(t->getNextFather2());}
}
;

void Neuron::printFatherWRL(){

	cout<<"#VRML V2.0 utf8\n";
	printFatherWRL(soma);
	cout<<"#";
}
;

void Neuron::printFatherWRL(Segment *t){
	if (t->getId()>0  && t->getPrevFather()!=NULL){
	
		static int RATIO=2, DIV=1000,found=0;
		double x,y,z,rad;
		x=t->getPrevFather()->getEnd()->getX()/DIV;
		y=t->getPrevFather()->getEnd()->getY()/DIV;
		z=t->getPrevFather()->getEnd()->getZ()/DIV;
		rad=t->getRadius()/DIV;


		cout<<"\n# Segment #:"<<t->getId()<<"\n";
		cout<<"Shape {\n";
		cout<<"  appearance Appearance {\n";
		cout<<"    material Material {\n";
		cout<<"      diffuseColor 1 1 1\n";
		cout<<"    } \n";// # end material\n";
		cout<<"  } \n";// # end appearance\n";

		cout<<"  geometry IndexedFaceSet {\n";

		cout<<"    solid TRUE\n";
		cout<<"    coord Coordinate {\n";
		cout<<"      point [\n";

		
		cout<< x+rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";
		cout<< x+rad*RATIO<<" "<<y<<" "<<z-rad*RATIO<<",\n";
		cout<< x-rad*RATIO<<" "<<y<<" "<<z-rad*RATIO<<",\n";
		cout<< x-rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";

		x=t->getEnd()->getX()/DIV;
		y=t->getEnd()->getY()/DIV;
		z=t->getEnd()->getZ()/DIV;
		
		cout<< x+rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";
		cout<< x+rad*RATIO<<" "<<y<<" "<<z-rad*RATIO<<",\n";
		cout<< x-rad*RATIO<<" "<<y<<" "<<z-rad*RATIO<<",\n";
		cout<< x-rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";

		cout<<"        ] \n";//  # end point\n";//
		cout<<"    } \n";// # end coord\n";
		cout<<"    coordIndex [ \n";

		cout<<"0, 4, 7, 3, -1,\n";
		cout<<"2, 3, 7, 6, -1,\n";
 		cout<<"1, 2, 6, 5, -1,\n";
 		cout<<"1, 0, 4, 5, -1,\n";
      
		cout<<"    ] \n";// # end coordIndex\n";

		cout<<"  } \n";// # end geometry\n";
		cout<<"} \n";// # end shape\n";
	}
	if(t->getNextFather1()!=NULL){
		printFatherWRL(t->getNextFather1());}
	if(t->getNextFather2()!=NULL){
		printFatherWRL(t->getNextFather2());}
}
;




void Neuron::printFatherIV(){

	cout<<"#Inventor V2.0 ascii\n";
	printFatherIV(soma);
	cout<<"#";
}
;

void Neuron::printFatherIV(Segment *t){
	if (t->getId()>3  && t->getPrevFather()!=NULL){

		static int RATIO=2, DIV=1000;
		double x,y,z,rad;



		cout<<"\n# Segment #:"<<t->getId()<<"\n";

		cout<<"DEF object Separator {\n";
		cout<<"Coordinate3 {\n";
		cout<<"point  [\n";

		


		x=t->getPrevFather()->getEnd()->getX()/DIV;
		y=t->getPrevFather()->getEnd()->getY()/DIV;
		z=t->getPrevFather()->getEnd()->getZ()/DIV;
		rad=t->getRadius()/DIV;
		cout<< x+rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";
		cout<< x+rad*RATIO<<" "<<y<<" "<<z-rad*RATIO<<",\n";
		cout<< x-rad*RATIO<<" "<<y<<" "<<z-rad*RATIO<<",\n";
		cout<< x-rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";

		x=t->getEnd()->getX()/DIV;
		y=t->getEnd()->getY()/DIV;
		z=t->getEnd()->getZ()/DIV;

		cout<< x+rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<",\n";
		cout<< x+rad*RATIO<<" "<<y<<" "<<z-rad*RATIO<<",\n";
		cout<< x-rad*RATIO<<" "<<y<<" "<<z-rad*RATIO<<",\n";
		cout<< x-rad*RATIO<<" "<<y<<" "<<z+rad*RATIO<<"\n";

		cout<<"]}\n";
		cout<<"IndexedFaceSet {\n";
		cout<<"coordIndex [\n";

		cout<<"0, 4, 7, 3, -1,\n";
		cout<<"2, 3, 7, 6, -1,\n";
 		cout<<"1, 2, 6, 5, -1,\n";
 		cout<<"1, 0, 4, 5, -1\n";

	    cout<<"]\n";
  cout<<"}\n";
cout<<"}\n";

	}
	if(t->getNextFather1()!=NULL){
		printFatherIV(t->getNextFather1());}
	if(t->getNextFather2()!=NULL){
		printFatherIV(t->getNextFather2());}
}
;

void Neuron::printCylWRL(){
	printCylWRL(soma);
}
;

void Neuron::printCylWRL(Segment *t){
	if (t->getId()>0 && t->getPrev()!=NULL){
		static int RATIO=1000;
		cout<<"Transform{\n";
		cout<<"	translation "<<t->getEnd()->getX()/RATIO<<" "<<t->getEnd()->getY()/RATIO<<" "<<t->getEnd()->getZ()/RATIO<<"\n";

		cout<<"children [\n";
		cout<<"Shape { \n";
		cout<<"\n";
		cout<<"appearance Appearance { \n";
		cout<<"material  Material { \n";
		cout<<"ambientIntensity 0.4 \n";
		cout<<"diffuseColor  1.0 0.2 1.0\n";
		cout<<"}\n";
		cout<<"} \n";
		cout<<"geometry Cylinder { \n";
		cout<<"radius "<< t->getRadius()/RATIO<<"\n";
		cout<<"height "<< t->getPrev()->getEnd()->distance(t->getEnd())/RATIO<<"\n";
		cout<<"bottom TRUE \n";
		cout<<"top  TRUE\n";
		cout<<"}\n";
		cout<<"}\n";
		cout<<"]\n";
		cout<<"}\n";
	}
	if(t->getNext1()!=NULL){
		printCylWRL(t->getNext1());}
	if(t->getNext2()!=NULL){
		printCylWRL(t->getNext2());}
}
;
*/


void Neuron::maxDim(Segment *t){
	static double x,y,z;
	x=t->getEnd()->getX();
	y=t->getEnd()->getY();
	z=t->getEnd()->getZ();
	if(t->getId()==1){
		minX=maxX=x;
		minY=maxY=y;
		minZ=maxZ=z;
	}
	if(x>maxX)
		maxX=x;
	if(x<minX)
		minX=x;

	if(y>maxY)
		maxY=y;
	if(y<minY)
		minY=y;

	if(z>maxZ)
		maxZ=z;
	if(z<minZ)
		minZ=z;
	if(t->getNext1()!=NULL){
		maxDim(t->getNext1());}
	if(t->getNext2()!=NULL){
		maxDim(t->getNext2());}

}
;
/*
#define WIDTH 1000
#define HEIGTH 500 

void Neuron::printPGM(){
	
	float** data;//[800];//[600];

	data=matrix(WIDTH,HEIGTH);
        int i, j;


	for(i=1;i<=WIDTH;i++)
		for(int j=1;j<=HEIGTH;j++){
			if(j%100==0){
				int kk=j;
			}
			data[i][j]=9;
		}


	maxDim(soma);

	printPGM(soma,data);



	char nm[100];
	strcpy(nm,name);
	strcat(nm,".pgm");
   ofstream tfile( nm , ios::binary );

   tfile<<"P2\n# L-Measure\n"<<(WIDTH)<<" "<<(HEIGTH)<<"\n9\n";
	for(j=HEIGTH;j>0;j--){
		for( i=1;i<=WIDTH;i++){
			tfile<<(int)data[i][j]<<" ";
		}
		tfile<<"\n";
   }
	tfile.close();
	
	free_matrix(data,WIDTH,HEIGTH);


}
;


void Neuron::printPGM(Segment *t,float** data){
	static double x,y,x1,y1,m,b,ka=0,kb=0,kc=0,kd=0;
	static int i,j;
	static double partWIDTH=WIDTH/3.0;
	if(t->getEnd()!=NULL ){

		if(t->getId()==893){
			int ii=0;
		}
		for(int axis=0;axis<3;axis++){
			if(axis==0){
				x=t->getEnd()->getX();
				y=t->getEnd()->getY();
			} else
			if(axis==1){
				x=t->getEnd()->getZ();
				y=t->getEnd()->getY();
			} else
			if(axis==2){
				x=t->getEnd()->getX();
				y=t->getEnd()->getZ();
			} 
			if(ka==0 &&kb==0){
				//compute all multipling factor 
				ka=(WIDTH/3-1)/(maxX-minX);
				kb=(HEIGTH-1)/(maxY-minY);
				kc=(WIDTH/3-1)/(maxZ-minZ);
				kd=(HEIGTH-1)/(maxZ-minZ);
				//choose the the min
				double mx=min(ka,kb);
				mx=min(mx,kc);
				mx=min(mx,kd);
				ka=mx;
			}
			
				i = (int)( (x-minX)*ka+1 );
				j = (int)( (y-minY)*ka+1 );
				i = (int)( i+(int)axis*partWIDTH );
				
					if(i>WIDTH) 
						i=WIDTH;
					if(j>HEIGTH) 
						j=HEIGTH;
					if(i<1) 
						i=1;
					if(j<1) 
						j=1;
				data[i][j]=0;
			
			if( t->getNext1()!=NULL){
				if(axis==0){
 					x1=t->getNext1()->getEnd()->getX();
					y1=t->getNext1()->getEnd()->getY();
				} else if(axis==1){
					x1=t->getNext1()->getEnd()->getZ();
					y1=t->getNext1()->getEnd()->getY();
				} else if(axis==2){
					x1=t->getNext1()->getEnd()->getX();
					y1=t->getNext1()->getEnd()->getZ();
				} 
				
				m=(y1-y)/(x1-x);
				b=y1-m*x1;
				int st=10;
				double step=abs(x1-x)/(st+0.0);
				for(int h=0;(h<st && m>0);h++){
					x+=step;
					y=m*x+b;
					i = (int)( (x-minX)*ka+1 );
					j = (int)( (y-minY)*ka+1 );
					i = (int)( i+(int)axis*partWIDTH );
					if(i>WIDTH) 
						i=WIDTH;
					if(j>HEIGTH) 
						j=HEIGTH;
					if(i<1) 
						i=1;
					if(j<1) 
						j=1;
					data[i][j]=0;
				}
void Neuron::printDXF(Segment*s, Segment*prev, ofstream & out){
	char* color="62\n100\n";;

	double RATIO=1000;
	int ord=s->getOrder();
	if (s->getPid()>0){
		switch (ord) {

		case 2: color="62\n100\n";break;

		}
	}

		static double fac=1;
		double rad=prev->getRadius();
		double length=prev->getEnd()->distance(s->getEnd());
		double x,y,z,x1,y1,z1;
		x=(prev->getEnd()->getX());
		y=(prev->getEnd()->getY());
		z=(prev->getEnd()->getZ());
		x1=(s->getEnd()->getX());
		y1=(s->getEnd()->getY());
		z1=(s->getEnd()->getZ());
		double xref,yref,zref;
		xref=x1-x;
		yref=y1-y;
		zref=z1-z;
		double tm=sqrt(xref*xref+yref*yref+zref*zref);
		xref/=tm;yref/=tm;zref/=tm;

		Vector * ref, *point, *xcur,*ycur,*zcur, *xdir, *ydir,*org;
		//direction fo extrusion for obtaining the cylinder
		ref=new Vector(xref,yref,zref);
		org=new Vector(0,0,0);
		point=new Vector(x,y,z);

		xcur=new Vector(1,0,0);
		ycur=new Vector(0,1,0);
		zcur=new Vector(0,0,1);
		xdir=new Vector();
		ydir=new Vector();

		Vector *a, *b;
		double aa,gg,u;
		u=PI/2;
		a=new Vector(1,0,0);
		a->rotate(PI/4,0,0);
		a=new Vector(1234,-123,20);
		b=new Vector(a->getX(),a->getY(),0);
		aa=org->angleR(b,xcur);

		a->rotate(0,0,-aa);
	
		b=new Vector(a->getX(),0,a->getZ());
		gg=org->angleR(b,xcur);
		a->rotate(0,-gg,0);



		//arbitrary axis algorithm
		if(xref<(1/64) && yref<(1/64)){
			xdir->crossProduct(org,ycur,ref);
		} else{
			xdir->crossProduct(org,zcur,ref);

		}
		ydir->crossProduct(org,ref,xdir);

		double alpha,beta, gamma;

		gamma=org->angleR(new Vector(ycur->getX(),ycur->getY(),0),ydir);
		xcur->rotate(0,0,-gamma);
		ycur->rotate(0,0,-gamma);
		alpha=org->angleR(ycur,ydir);
		ref->rotate(0,-alpha,0);
		beta=org->angleR(zcur,ref)-PI/2;
		
		point->rotate(alpha,beta,gamma);
		x=point->getX();
		y=point->getY();
		z=point->getZ();

		if(false){

			out<<"CIRCLE\n";
			out<<color;
			
			//X
			out<<"10\n"<<x/RATIO<<"\n";
			//Y
			out<<"20\n"<<y/RATIO<<"\n";
			//Z
			out<<"30\n"<<z/RATIO<<"\n";
			//length
			out<<"39\n"<<length/RATIO<<"\n";


			//radius
			out<<"40\n"<<rad/RATIO<<"\n";

			out<<"210\n"<<xref/RATIO<<"\n";
			//Y1
			out<<"220\n"<<yref/RATIO<<"\n";
			//Z1
			out<<"230\n"<<zref/RATIO<<"\n0\n";

			
		}
		if(true){

			out<<"3DFACE\n8\n2\n";
			out<<color;
			//X1
			out<<"10\n"<<(s->getEnd()->getX())/RATIO<<"\n";
			//Y1
			out<<"20\n"<<(s->getEnd()->getY()-fac*rad)/RATIO<<"\n";
			//Z1
			out<<"30\n"<<(s->getEnd()->getZ()+fac*rad)/RATIO<<"\n";

			//X2
			out<<"11\n"<<(s->getEnd()->getX())/RATIO<<"\n";
			//Y2
			out<<"21\n"<<(s->getEnd()->getY()+fac*rad)/RATIO<<"\n";
			//Z2
			out<<"31\n"<<(s->getEnd()->getZ()+fac*rad)/RATIO<<"\n";

			//X3
			out<<"12\n"<<(prev->getEnd()->getX())/RATIO<<"\n";
			//Y3
			out<<"22\n"<<(prev->getEnd()->getY()+fac*rad)/RATIO<<"\n";
			//Z3
			out<<"32\n"<<(prev->getEnd()->getZ()+fac*rad)/RATIO<<"\n";

			//X4
			out<<"13\n"<<(prev->getEnd()->getX())/RATIO<<"\n";
			//Y4
			out<<"23\n"<<(prev->getEnd()->getY()-fac*rad)/RATIO<<"\n";
			//Z4
			out<<"33\n"<<(prev->getEnd()->getZ()+fac*rad)/RATIO<<"\n0\n";
			//---------------------------------------------------------------------------
			//second face 
			out<<"3DFACE\n8\n2\n";
			out<<color;
			//X1
			out<<"10\n"<<(s->getEnd()->getX())/RATIO<<"\n";
			//Y1
			out<<"20\n"<<(s->getEnd()->getY()-fac*rad)/RATIO<<"\n";
			//Z1
			out<<"30\n"<<(s->getEnd()->getZ()-fac*rad)/RATIO<<"\n";

			//X2
			out<<"11\n"<<(s->getEnd()->getX())/RATIO<<"\n";
			//Y2
			out<<"21\n"<<(s->getEnd()->getY()+fac*rad)/RATIO<<"\n";
			//Z2
			out<<"31\n"<<(s->getEnd()->getZ()-fac*rad)/RATIO<<"\n";

			//X3
			out<<"12\n"<<(prev->getEnd()->getX())/RATIO<<"\n";
			//Y3
			out<<"22\n"<<(prev->getEnd()->getY()+fac*rad)/RATIO<<"\n";
			//Z3
			out<<"32\n"<<(prev->getEnd()->getZ()-fac*rad)/RATIO<<"\n";

			//X4
			out<<"13\n"<<(prev->getEnd()->getX())/RATIO<<"\n";
			//Y4
			out<<"23\n"<<(prev->getEnd()->getY()-fac*rad)/RATIO<<"\n";
			//Z4
			out<<"33\n"<<(prev->getEnd()->getZ()-fac*rad)/RATIO<<"\n0\n";
			//---------------------------------------------------------------------------
			//third face 
			out<<"3DFACE\n8\n2\n";
			out<<color;
			//X1
			out<<"10\n"<<(s->getEnd()->getX())/RATIO<<"\n";
			//Y1
			out<<"20\n"<<(s->getEnd()->getY()+fac*rad)/RATIO<<"\n";
			//Z1
			out<<"30\n"<<(s->getEnd()->getZ()+fac*rad)/RATIO<<"\n";

			//X2
			out<<"11\n"<<(s->getEnd()->getX())/RATIO<<"\n";
			//Y2
			out<<"21\n"<<(s->getEnd()->getY()+fac*rad)/RATIO<<"\n";
			//Z2
			out<<"31\n"<<(s->getEnd()->getZ()-fac*rad)/RATIO<<"\n";

			//X3
			out<<"12\n"<<(prev->getEnd()->getX())/RATIO<<"\n";
			//Y3
			out<<"22\n"<<(prev->getEnd()->getY()+fac*rad)/RATIO<<"\n";
			//Z3
			out<<"32\n"<<(prev->getEnd()->getZ()-fac*rad)/RATIO<<"\n";

			//X4
			out<<"13\n"<<(prev->getEnd()->getX())/RATIO<<"\n";
			//Y4
			out<<"23\n"<<(prev->getEnd()->getY()+fac*rad)/RATIO<<"\n";
			//Z4
			out<<"33\n"<<(prev->getEnd()->getZ()+fac*rad)/RATIO<<"\n0\n";
			//---------------------------------------------------------------------------
			//fourth face 
			out<<"3DFACE\n8\n2\n";
			out<<color;
			//X1
			out<<"10\n"<<(s->getEnd()->getX())/RATIO<<"\n";
			//Y1
			out<<"20\n"<<(s->getEnd()->getY()-fac*rad)/RATIO<<"\n";
			//Z1
			out<<"30\n"<<(s->getEnd()->getZ()+fac*rad)/RATIO<<"\n";

			//X2
			out<<"11\n"<<(s->getEnd()->getX())/RATIO<<"\n";
			//Y2
			out<<"21\n"<<(s->getEnd()->getY()-fac*rad)/RATIO<<"\n";
			//Z2
			out<<"31\n"<<(s->getEnd()->getZ()-fac*rad)/RATIO<<"\n";

			//X3
			out<<"12\n"<<(prev->getEnd()->getX())/RATIO<<"\n";
			//Y3
			out<<"22\n"<<(prev->getEnd()->getY()-fac*rad)/RATIO<<"\n";
			//Z3
			out<<"32\n"<<(prev->getEnd()->getZ()-fac*rad)/RATIO<<"\n";

			//X4
			out<<"13\n"<<(prev->getEnd()->getX())/RATIO<<"\n";
			//Y4
			out<<"23\n"<<(prev->getEnd()->getY()-fac*rad)/RATIO<<"\n";
			//Z4
			out<<"33\n"<<(prev->getEnd()->getZ()+fac*rad)/RATIO<<"\n0\n";
			//---------------------------------------------------------------------------
			//fifth face : closing the box
			out<<"3DFACE\n8\n2\n";
			out<<color;
			//X1
			out<<"10\n"<<(s->getEnd()->getX())/RATIO<<"\n";
			//Y1
			out<<"20\n"<<(s->getEnd()->getY()-fac*rad)/RATIO<<"\n";
			//Z1
			out<<"30\n"<<(s->getEnd()->getZ()+fac*rad)/RATIO<<"\n";

			//X2
			out<<"11\n"<<(s->getEnd()->getX())/RATIO<<"\n";
			//Y2
			out<<"21\n"<<(s->getEnd()->getY()-fac*rad)/RATIO<<"\n";
			//Z2
			out<<"31\n"<<(s->getEnd()->getZ()-fac*rad)/RATIO<<"\n";

			//X3
			out<<"12\n"<<(s->getEnd()->getX())/RATIO<<"\n";
			//Y3
			out<<"22\n"<<(s->getEnd()->getY()+fac*rad)/RATIO<<"\n";
			//Z3
			out<<"32\n"<<(s->getEnd()->getZ()-fac*rad)/RATIO<<"\n";

			//X4
			out<<"13\n"<<(s->getEnd()->getX())/RATIO<<"\n";
			//Y4
			out<<"23\n"<<(s->getEnd()->getY()+fac*rad)/RATIO<<"\n";
			//Z4
			out<<"33\n"<<(s->getEnd()->getZ()+fac*rad)/RATIO<<"\n0\n";
			//---------------------------------------------------------------------------
			//six face 
			out<<"3DFACE\n8\n2\n";
			out<<color;
			//X1
			out<<"10\n"<<(prev->getEnd()->getX())/RATIO<<"\n";
			//Y1
			out<<"20\n"<<(prev->getEnd()->getY()-fac*rad)/RATIO<<"\n";
			//Z1
			out<<"30\n"<<(prev->getEnd()->getZ()+fac*rad)/RATIO<<"\n";

			//X2
			out<<"11\n"<<(prev->getEnd()->getX())/RATIO<<"\n";
			//Y2
			out<<"21\n"<<(prev->getEnd()->getY()-fac*rad)/RATIO<<"\n";
			//Z2
			out<<"31\n"<<(prev->getEnd()->getZ()-fac*rad)/RATIO<<"\n";

			//X3
			out<<"12\n"<<(prev->getEnd()->getX())/RATIO<<"\n";
			//Y3
			out<<"22\n"<<(prev->getEnd()->getY()+fac*rad)/RATIO<<"\n";
			//Z3
			out<<"32\n"<<(prev->getEnd()->getZ()-fac*rad)/RATIO<<"\n";

			//X4
			out<<"13\n"<<(prev->getEnd()->getX())/RATIO<<"\n";
			//Y4
			out<<"23\n"<<(prev->getEnd()->getY()+fac*rad)/RATIO<<"\n";
			//Z4
			out<<"33\n"<<(prev->getEnd()->getZ()+fac*rad)/RATIO<<"\n0\n";
			//---------------------------------------------------------------------------
	}


}			}
		}
	}
	if(t->getNext1()!=NULL){
		printPGM(t->getNext1(),data);}
	if(t->getNext2()!=NULL){
		printPGM(t->getNext2(),data);}
}
*/


void Neuron::orientTrans1(Vector* Trans, Vector *Perp, Vector *centerMass){
	rotateTo(Perp);

	double x,y,z;
	x=Trans->getX();
	y=Trans->getY(); 
	z=Trans->getZ();
	if(fabs(x)>100){
		int hh=0;
	}

	//look for virtual segment with id=-99... to extract end point
 	Vector * zv=lookNeuronForVirtual(soma);
	double angle=0;
	if(zv!=NULL){
		Vector * cc=new Vector(0,0,0);

		angle=cc->anglePlane(zv,centerMass,Perp);


		 delete cc;

	}
	if(angle!=0){
		rotateAxis(Perp,-angle*PI/180,soma);
	}

	x*=1000;
	y*=1000;
	z*=1000;
	translation(-x,-y,-z);
};


Vector * Neuron::lookNeuronForVirtual(Segment *t){
	static int found=0;
	Vector *x1,*x2;x1=NULL;x2=NULL;
	if(t->getType()==-99){
		found=1;
		return t->getEnd();
	}
	if(t->getNext1()!=NULL && found==0){
		 x1=lookNeuronForVirtual(t->getNext1());
	}
	if(t->getNext2()!=NULL && found==0){
		 x2=lookNeuronForVirtual(t->getNext2());
	}
	if(x1!=NULL)
		return x1;
	if(x2!=NULL)
		return x2;
	return NULL;
}
;

Neuron::~Neuron(){

	remove(soma);
	name=NULL;
	//type=NULL;
	in=NULL;
	soma=NULL;

	
};

void Neuron::remove(Segment * t){
	if(t->getNext1()!=NULL){
		remove(t->getNext1());

	}

		if (t->getNext2()!= NULL)
          {
            remove(t->getNext2());

          }


	if(t->getNrTip()==1 &&t->getId()<16){
		int hh=0;
	}
	if(t==soma){
		int tt=00;
		return;
	}
	
	delete t;
	t=NULL;
	

	
	
};

Neuron * Neuron::clone(){
	Neuron *n;
	//create empty neuron;
	n=new Neuron();
	n->setCloned();
	n->setName(name);
	//fill with segments
	Segment *s=clone(n,soma);
	n->setSoma(s);
	n->init();
	s=NULL;
	return n;
};

Segment * Neuron::clone(Neuron * n,Segment * t){
	Segment * newSeg,*t1,*t2;
	t1=NULL;t2=NULL;

	if(t->getNext1()!=NULL){
		t1=clone(n,t->getNext1());
	}
	if(t->getNext2()!=NULL ){
		t2=clone(n,t->getNext2());
	}
	newSeg=t->clone();

	
	if(t1!=NULL){
		newSeg->setNext1(t1);
		t1->setPrev(newSeg);
	}
	if(t2!=NULL){
		newSeg->setNext2(t2);
		t2->setPrev(newSeg);
	}
	t1=NULL;t2=NULL;	
	return newSeg;
};
