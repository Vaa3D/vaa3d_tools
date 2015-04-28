#include <iostream>

#include "Parameter.h"

using namespace std;

char *strlwr( char *string)
{
   char *p = string;
   while(*p)
      {
         *p = tolower(*p);
            p++;
      }
   return string;
}

//add a new value to the list 	
void Parameter::addValue(double value){
	val[nextValue]=value;
	nextValue++;
}
;
double Parameter::getValue(){
	double x;
	if(strcmp(getDist(),"k")==0){
		//CONSTANT
		return val[0];
	}else if(strcmp(getDist(),"u")==0){
		//min,max
		//uniform
		return rnd->rnduniform (val[0], val[1]);
	}else if(strcmp(getDist(),"g")==0){
		//gaussian
		x=-1;
		if(nextValue==2){
			return x = rnd->rndgauss (val[0], val[1]);;
		} else if (nextValue==4){
			
			do {
				//mean,stdev, min, max
				x = rnd->rndgauss (val[0], val[1]);
			} while ((x < val[2]) || (x >val[3]));
		}
		if(x==-1)
			cerr<<"Missing parameter for :"<<name<<endl;
		return x;

	}else if(strcmp(getDist(),"y")==0){
		//gamma
		if(nextValue==3){
			return rnd->rndgamma (val[0], val[1], val[2]);
		} else if(nextValue==5){
			do {
				//alpha, beta, offset
				x = rnd->rndgamma (val[0], val[1], val[2]);  /* SCR 05-15-01 */
			} while ((x < val[3]) || (x > val[4]));
			return x;
		} 
		
		cerr<<"Missing parameter for :"<<name<<endl;
		return -1;
	}else if(strcmp(getDist(),"k")==0){
		return val[0];
	}else if(strcmp(getDist(),"k")==0){
		return val[0];
	}else if(strcmp(getDist(),"k")==0){
		return val[0];
	}else if(strcmp(getDist(),"k")==0){
		return val[0];
	}else if(strcmp(getDist(),"k")==0){
		return val[0];
	}else if(strcmp(getDist(),"k")==0){
		return val[0];
	};

	return -1;
}
;
