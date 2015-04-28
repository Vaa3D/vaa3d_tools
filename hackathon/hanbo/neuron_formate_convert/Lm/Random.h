#ifndef RANDOM_H
#define RANDOM_H

#include "time.h"


class Random{

	 int iset ;
	
	 double gset ;

public:
	
	Random(){
		iset=0;
		gset=0;
		int seed=1;
		
		setSeed( seed);
	}
	
	static double rnd01();

	
	void setSeed (int seed);

	
	double rnduniform (double min, double max);
	
	double expdev();
	
	double gaussdev ();
	
	double rndgauss (double mean, double stdev);

	
	double gamdev(double a,double lambda);
	
	double rndgamma(double alpha, double lambda, double offset);

}
;
#endif
