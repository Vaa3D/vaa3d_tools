#include "compute_gmi.h"
void computeGMI(const NeuronTree & nt, double * gmi)
{
	QList<NeuronSWC> list = nt.listNeuron;
	QHash<int, int> LUT = QHash<int, int>();
	for (int i=0;i<list.size();i++)
		LUT.insert(list.at(i).n,i);
	double centerpos[3] = {0,0,0};
	
	int siz = list.size();
	double** b = NULL;
	try {
		b = new double*[siz];
	}
	catch (...)
	{
		fprintf(stderr,"fail to allocate memory");
		if (b) { delete []b; b=NULL;}
		return;
	}
	double avgR = 0;
	for (int i=0;i<siz;i++)
	{
		//here I only kept x,y,z & pn info because others are not used
		b[i] = NULL;
		try
		{
			b[i] = new double[4];
		}
		catch (...)
		{
			fprintf(stderr,"fail to allocate memory");
			if (b[i]) { delete b[i]; b[i]=NULL;}
			return;
		}
		b[i][0] = list.at(i).x; 
		b[i][1] = list.at(i).y; 
		b[i][2] = list.at(i).z; 
		avgR += list.at(i).r;
		if (list.at(i).pn<0) {
			b[i][3] = -1;
		}
		else
			b[i][3] = LUT.value(list.at(i).pn);
		//b[i][4] = list.at(i).r;
	}
	avgR /= siz;
	gmi[13] = avgR;

	double m000 = compute_moments_neuron(b,siz,0,0,0,VOID);
	centerpos[0] = compute_moments_neuron(b,siz,1,0,0,VOID);
	centerpos[1] = compute_moments_neuron(b,siz,0,1,0,VOID);
	centerpos[2] = compute_moments_neuron(b,siz,0,0,1,VOID);

	for (int j=0;j<3;j++)
		centerpos[j] /= m000;


	compute_neuron_GMI(b,siz,centerpos,VOID,gmi);

	for (int i=0;i<siz;i++)
	{
		if (b[i])
		{
			delete b[i];
			b[i] = NULL;
		}
	}
	if (b) {delete b;
		b = NULL; }

		return;
}

//modulated from pengh's compute_neuron_GMI.m
void compute_neuron_GMI(double **b, int siz,  double* centerpos, double radius_thres, double * gmi)
{
	if (centerpos[0]!=0 || centerpos[1]!=0 || centerpos[2]!=0)
		for (int i=0;i<siz;i++)
			for (int j=0;j<3;j++)
				b[i][j] -= centerpos[j];

	double c000 = compute_moments_neuron(b,siz,0,0,0, radius_thres); 

	double c200 = compute_moments_neuron(b,siz, 2,0,0, radius_thres); 
	double c020 = compute_moments_neuron(b,siz, 0,2,0, radius_thres); 
	double c002 = compute_moments_neuron(b,siz, 0,0,2, radius_thres); 
	double c110 = compute_moments_neuron(b,siz, 1,1,0, radius_thres); 
	double c101 = compute_moments_neuron(b,siz, 1,0,1, radius_thres); 
	double c011 = compute_moments_neuron(b,siz, 0,1,1, radius_thres); 

	double c300 = compute_moments_neuron(b,siz, 3,0,0, radius_thres); 
	double c030 = compute_moments_neuron(b,siz, 0,3,0, radius_thres); 
	double c003 = compute_moments_neuron(b,siz, 0,0,3, radius_thres); 
	double c120 = compute_moments_neuron(b,siz, 1,2,0, radius_thres); 
	double c102 = compute_moments_neuron(b,siz, 1,0,2, radius_thres); 
	double c210 = compute_moments_neuron(b,siz, 2,1,0, radius_thres); 
	double c201 = compute_moments_neuron(b,siz, 2,0,1, radius_thres); 
	double c012 = compute_moments_neuron(b,siz, 0,1,2, radius_thres); 
	double c021 = compute_moments_neuron(b,siz, 0,2,1, radius_thres); 
	double c111 = compute_moments_neuron(b,siz, 1,1,1, radius_thres); 
	/*
	   gmi[0] = (c200+c020+c002)/c000;
	   gmi[1] = (c200*c020+c020*c002+c002*c200-c101*c101-c011*c011-c110*c110)/(c000*c000);
	   gmi[2] = (c200*c020*c002+2*c110*c101*c011-c200*c011*c011-c020*c101*c101-c002*c110*c110)/(c000*c000*c000);
	   gmi[3] = (c003*c003+6*c012*c012+6*c021*c021+c030*c030+6*c102*c102+15*c111*c111-3*c102*c120+6*c120*c120-3*c021*c201+6*c102*c102-3*c003*(c021+c201)-3*c030*c210+6*c210*c210-3*c012*(c030+c210)-3*c102*c300-3*c120*c300+c300*c300)/(c000*c000);
	 */
	//cout<<"c000:"<<c000<<"\tc200:"<<c200<<"\tc020:"<<c020<<"\tc002:"<<c002<<"\tc110:"<<c110<<"\tc101:"<<c101<<"\tc011:"<<c011<<endl;
	//cout<<"c300:"<<c300<<"\tc030:"<<c030<<"\tc003:"<<c003<<"\tc120:"<<c120<<"\tc102:"<<c102<<"\tc210:"<<c210<<"\tc201:"<<c201<<"\tc012:"<<c012<<"\tc021:"<<c021<<"\tc111:"<<c111<<endl;
	//feaVec(:,1) = size(b,1); 

	gmi[0] = c000;
	gmi[1] = c200+c020+c002;
	gmi[2] = c200*c020+c020*c002+c002*c200-c101*c101-c011*c011-c110*c110; 
	gmi[3] = c200*c020*c002-c002*c110*c110+2*c110*c101*c011-c020*c101*c101-c200*c011*c011; 

	double spi = sqrt(PI);

	complex<double> v_0_0 ((2*spi/3)*(c200+c020+c002),0);

	complex<double> v_2_2 (c200-c020,2*c110);
	v_2_2 *= spi*sqrt(2.0/15);
	complex<double> v_2_1 (-2*c101,-2*c011);
	v_2_1 *= spi*sqrt(2.0/15); 
	complex<double> v_2_0 (2*c002-c200-c020,0);
	v_2_0 *= spi*sqrt(4.0/45);
	complex<double> v_2_m1 (2*c101,-2*c011);
	v_2_m1 *= spi*sqrt(2.0/15); 
	complex<double> v_2_m2 (c200-c020,-2*c110);
	v_2_m2 *= spi*sqrt(2.0/15);

	complex<double> v_3_3 ((-c300+3*c120) , (c030-3*c210));
	v_3_3 *= spi*sqrt(1.0/35);
	complex<double> v_3_2 ((c201-c021), 2*c111);
	v_3_2 *= spi*sqrt(6.0/35);
	complex<double> v_3_1 ((c300+c120-4*c102), (c030+c210-4*c012));
	v_3_1 *= spi*sqrt(3.0/175);
	complex<double> v_3_0 (2*c003 - 3*c201 - 3*c021,0);
	v_3_0 *= spi*sqrt(4.0/175);
	complex<double> v_3_m1 ((-c300-c120+4*c102) , (c030+c210-4*c012));
	v_3_m1 *=  spi*sqrt(3.0/175);
	complex<double> v_3_m2 ((c201-c021) , -2*c111);
	v_3_m2 *=  spi*sqrt(6.0/35);
	complex<double> v_3_m3 ((c300-3*c120) , (c030-3*c210));
	v_3_m3 *= spi*sqrt(1.0/35); 

	complex<double> v_1_1 ((-c300-c120-c102), -(c030+c210+c012));
	v_1_1 *= spi*sqrt(6.0/25);
	complex<double> v_1_0 (c003+c201+c021,0);
	v_1_0 *= spi*sqrt(12.0/25);
	complex<double> v_1_m1 ((c300+c120+c102) , -(c030+c210+c012));
	v_1_m1 *= spi*sqrt(6.0/25);

	complex<double> v_g33_2_2 = sqrt(10.0/21)*v_3_3*v_3_m1 - sqrt(20.0/21)*v_3_2*v_3_0 + sqrt(2.0/7)*v_3_1*v_3_1;
	complex<double> v_g33_2_1 = sqrt(25.0/21)*v_3_3*v_3_m2 - sqrt(5.0/7)*v_3_2*v_3_m1 + sqrt(2.0/21)*v_3_1*v_3_0;
	complex<double> v_g33_2_0 = sqrt(25.0/21)*v_3_3*v_3_m3 - sqrt(3.0/7)*v_3_1*v_3_m1 + sqrt(4.0/21)*v_0_0*v_0_0;
	complex<double> v_g33_2_m1 = sqrt(25.0/21)*v_3_m3*v_3_2 - sqrt(5.0/7)*v_3_m2*v_3_1 + sqrt(2.0/21)*v_3_m1*v_3_0;
	complex<double> v_g33_2_m2 = sqrt(10.0/21)*v_3_m3*v_3_1 - sqrt(20.0/21)*v_3_m2*v_3_0 + sqrt(2.0/7)*v_3_m1*v_3_m1;

	complex<double> v_g31_2_2 = -sqrt(1.0/105)*v_3_2*v_1_0 + sqrt(1.0/35)*v_3_3*v_1_m1 + sqrt(1.0/525)*v_3_1*v_1_1;
	complex<double> v_g31_2_1 = sqrt(2.0/105)*v_3_2*v_1_m1 + sqrt(1.0/175)*v_3_0*v_1_1 - sqrt(4.0/525)*v_3_1*v_1_0;
	complex<double> v_g31_2_0 = -sqrt(3.0/175)*v_3_0*v_1_0 + sqrt(2.0/175)*v_3_1*v_1_m1 + sqrt(2.0/175)*v_3_m1*v_1_1;
	complex<double> v_g31_2_m1 = sqrt(2.0/105)*v_3_m2*v_1_1 + sqrt(1.0/175)*v_3_0*v_1_m1 -sqrt(4.0/525)*v_3_m1*v_1_0;
	complex<double> v_g31_2_m2 = -sqrt(1.0/105)*v_3_m2*v_1_0 + sqrt(1.0/35)*v_3_m3*v_1_1 + sqrt(1.0/525)*v_3_m1*v_1_m1;

	complex<double> v_g11_2_2 = 0.2*v_1_1*v_1_1;
	complex<double> v_g11_2_1 = sqrt(2.0/25)*v_1_0*v_1_1;
	complex<double> v_g11_2_0 = sqrt(2.0/75)*(v_1_0*v_1_0 + v_1_1*v_1_m1);
	complex<double> v_g11_2_m1 = sqrt(2.0/25)*v_1_0*v_1_m1;
	complex<double> v_g11_2_m2 = 0.2*v_1_m1*v_1_m1;

	complex<double> tmp;
	tmp = pow(v_0_0,(12.0/5));

	gmi[4] = real((1/sqrt(7.0)) * (v_3_3*v_3_m3*2.0 - v_3_2*v_3_m2*2.0 + v_3_1*v_3_m1*2.0 - v_3_0*v_3_0) / tmp); 
	gmi[5] = real((1/sqrt(3.0))* (v_1_1*v_1_m1*2.0 - v_1_0*v_1_0) / tmp); 

	tmp = pow(v_0_0,(24.0/5));
	gmi[6] = real((1/sqrt(5.0)) * (v_g33_2_m2*v_g33_2_2*2.0 - v_g33_2_m1*v_g33_2_1*2.0 + v_g33_2_0*v_g33_2_0) / tmp);
	gmi[7] = real((1/sqrt(5.0)) * (v_g31_2_m2*v_g31_2_2*2.0 - v_g31_2_m1*v_g31_2_1*2.0 + v_g31_2_0*v_g31_2_0) / tmp);
	gmi[8] = real((1/sqrt(5.0)) * (v_g33_2_m2*v_g31_2_2 - v_g33_2_m1*v_g31_2_1 + v_g33_2_0*v_g31_2_0 - v_g33_2_1*v_g31_2_m1 + v_g33_2_2*v_g31_2_m2) / tmp);
	gmi[9] = real((1/sqrt(5.0)) * (v_g31_2_m2*v_g11_2_2 - v_g31_2_m1*v_g11_2_1 + v_g31_2_0*v_g11_2_0 - v_g31_2_1*v_g11_2_m1 + v_g31_2_1*v_g11_2_m2) / tmp);

	tmp = pow(v_0_0,(17.0/5));
	gmi[10] = real((1/sqrt(5.0)) * (v_g33_2_m2*v_2_2 - v_g33_2_m2*v_2_1 + v_g33_2_0*v_2_0 - v_g33_2_1*v_2_m1 + v_g33_2_2*v_2_m2) / tmp);
	gmi[11] = real((1/sqrt(5.0)) * (v_g31_2_m2*v_2_2 - v_g31_2_m2*v_2_1 + v_g31_2_0*v_2_0 - v_g31_2_1*v_2_m1 + v_g31_2_2*v_2_m2) / tmp);
	gmi[12] = real((1/sqrt(5.0)) * (v_g11_2_m2*v_2_2 - v_g11_2_m2*v_2_1 + v_g11_2_0*v_2_0 - v_g11_2_1*v_2_m1 + v_g11_2_2*v_2_m2) / tmp);

}


//modulated from pengh's compute_moments_neuron.m
double compute_moments_neuron(double ** a, int siz, double p, double q, double r, double radius_thres)
{
	double m = 0;
	double step0=0.1;
	double b1[4],b2[4];

	for (int i=0;i<siz;i++)
	{
		if (a[i][3]<0) continue;
		double sum = 0;
		for (int j=0;j<3;j++)
		{
			b1[j] = a[i][j];
			b2[j] = a[int(a[i][3])][j];
			sum += (b1[j]-b2[j])*(b1[j]-b2[j]);
		}

		double len = sqrt(sum);
		int K = floor(len/step0)+1;

		double xstep,ystep,zstep;//,rstep;
		xstep = (b2[0]-b1[0])/K;
		ystep = (b2[1]-b1[1])/K;
		zstep = (b2[2]-b1[2])/K;
		//rstep = (b2[4]-b1[4])/K;

		double x,y,z,d;//,radius;
		for (int k=1;k<=K;k++)
		{
			x = b1[0]+k*xstep;
			y = b1[1]+k*ystep;
			z = b1[2]+k*zstep;
			//radius = b1[4]+k*rstep;
			d = sqrt(x*x+y*y+z*z);
			if (d>radius_thres) { cout<<"invalid VOID!!"<<endl; break;}

			m += pow(x,p) * pow(y,q) * pow(z,r);
		}

	}
	return m;
}
