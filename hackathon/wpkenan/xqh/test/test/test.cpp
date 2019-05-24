// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include<stdio.h>
#include<math.h>
#include<time.h>
#include<stdlib.h>
#define	PI 3.141592653589		
#define P_num 150         
#define dim 4           
#define iter_num 16     
#define V_max 20          
#define w 0.5    
#define alp 1       
double particle[P_num][dim];           
double particle_loc_best[P_num][dim]; 
double particle_loc_fit[P_num];        
double particle_glo_best[dim];        
double gfit;                         
double particle_v[P_num][dim];        
double particle_fit[P_num];            
double c1;          
double c2;
double fitness(double a[])            
{

	double fitness_faval = 0;
	double  temp1 = 0;
	temp1 = ((a[0] - 1)/7)*((a[0] - 1)/7);

	double  temp2 = 0;
	temp2 = ((a[0] - 8)/7)*((a[0] - 8)/7);

	double  temp3 = 0;
	temp3 = ((a[1] - 1)/7)*((a[1] - 1)/7);

	double  temp4 = 0;
	temp4 = ((a[1] - 8)/7)*((a[1] - 8)/7);

	double  temp5 = 0;
	temp5 = ((a[2] + 3)/6)*((a[2] + 3)/6);

	double  temp6 = 0;
	temp6 = ((a[2] - 3)/6)*((a[2] - 3)/6);

	double  temp7 = 0;
	temp7 = ((a[3] + 21)/26)*((a[3] + 21)/26);

	double  temp8 = 0;
	temp8 = ((a[3] - 5)/26)*((a[3] - 5)/26);


	fitness_faval = fitness_faval + (50.4359*a[0]*a[0] + 200.4335*a[0] + 1200.6485) + (200.55*a[1]*a[1] + 500.746*a[1] + 1857.201) + 3000*(temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8) ;

	return fitness_faval;
}
void initial()
{
	int i,j;
	for(i=0; i<P_num; i++)           
	{
		for(j=0; j<dim; j++)
		{

			particle[i][0] = 1+(8-1)*1.0*rand()/RAND_MAX;  
			particle[i][1] = 1+(8-1)*1.0*rand()/RAND_MAX;  
			particle[i][2] = (-3)+(3 + 3)*1.0*rand()/RAND_MAX;  
			particle[i][3] = (-21)+(5 + 21)*1.0*rand()/RAND_MAX;  

			particle_loc_best[i][j] = particle[i][j];              
			particle_v[i][j] = -V_max+2*V_max*1.0*rand()/RAND_MAX;    
		}
	}
	for(i=0; i<P_num; i++)          
	{
		particle_fit[i] = fitness(particle[i]);
		particle_loc_fit[i] = particle_fit[i];
	}
	gfit = particle_loc_fit[0];     
	j=0;
	for(i=1; i<P_num; i++)
	{
		if(particle_loc_fit[i]<gfit)
		{
			gfit = particle_loc_fit[i];
			j = i;
		}
	}
	for(i=0; i<dim; i++)             
	{
		particle_glo_best[i] = particle_loc_best[j][i];
	}
}
void renew_particle()
{
	int i,j;
	for(i=0; i<P_num; i++)            
	{
		for(j=0; j<dim; j++)
		{
			particle[i][j] +=  alp*particle_v[i][j];


			if(particle[i][0] > 8)
			{
				particle[i][0] = 8;
			}
			if(particle[i][0] < 1)
			{
				particle[i][0] = 1;
			}

			if(particle[i][1] > 8)
			{
				particle[i][1] = 8;
			}
			if(particle[i][1] < 1)
			{
				particle[i][1] = 1;
			}

			if(particle[i][2] > 3)
			{
				particle[i][2] = 3;
			}
			if(particle[i][2] < -3)
			{
				particle[i][2] = -3;
			}

			if(particle[i][3] > 5)
			{
				particle[i][3] = 5;
			}
			if(particle[i][3] < -21)
			{
				particle[i][3] = -21;
			}
		}
	}
}
void renew_var()
{
	int i, j;
	for(i=0; i<P_num; i++)            
	{
		particle_fit[i] = fitness(particle[i]);
		if(particle_fit[i] < particle_loc_fit[i])      
		{
			particle_loc_fit[i] = particle_fit[i];
			for(j=0; j<dim; j++)      
			{
				particle_loc_best[i][j] = particle[i][j];
			}
		}
	}
	for(i=0,j=-1; i<P_num; i++)                  
	{
		if(particle_loc_fit[i]<gfit)
		{
			gfit = particle_loc_fit[i];
			j = i;
		}
	}
	if(j != -1)
	{
		for(i=0; i<dim; i++)            
		{
			particle_glo_best[i] = particle_loc_best[j][i];
		}
	}
	for(i=0; i<P_num; i++)    
	{
		for(j=0; j<dim; j++)
		{
			particle_v[i][j]=w*particle_v[i][j]+
				c1*1.0*rand()/RAND_MAX*(particle_loc_best[i][j]-particle[i][j])+
				c2*1.0*rand()/RAND_MAX*(particle_glo_best[j]-particle[i][j]);
			if(particle_v[i][j] > V_max)
			{
				particle_v[i][j] = V_max;
			}
			if(particle_v[i][j] < -V_max)
			{
				particle_v[i][j] = -V_max;
			}
		}
	}
}
int main()
{

	int i=0;
	int n_max = 16;
	c1 = 1.1;          
	c2 = 0.8;
	srand((unsigned)time(NULL));
	initial();
	printf("粒子个数:%d\n",P_num);
	printf("维度为:%d\n",dim);
	while(i < iter_num)
	{
		renew_particle();
		renew_var();
		i++;
		c1 = 1.1 - i/n_max;
		c2 = 0.8 - i/n_max;
		printf("迭代次数%d 最优值%.10lf\n", i,gfit);
		printf("最优值处决策变量的值%.10lf %.10lf %.10lf %.10lf\n", particle_glo_best[0], particle_glo_best[1],particle_glo_best[2],particle_glo_best[3]);

	}


	printf("最优值为%.10lf\n", gfit);
	printf("最优值处决策变量的值%.10lf %.10lf %.10lf %.10lf\n", particle_glo_best[0], particle_glo_best[1],particle_glo_best[2],particle_glo_best[3]);

	system("pause");
	return 0;
}

