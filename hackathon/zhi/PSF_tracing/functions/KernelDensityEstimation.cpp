#include <mex.h>
#include <matrix.h>
#include <math.h>

#include "KernelDensityEstimation.h"
//#include "FunctionsForMainCode.h"


/*void kernelFunction(double *samples, int dim, double *data, double *weights,
        double *eigenVecs, double *eigenVals, double *normP, int Nsamples, int Ndata, double *BETA,
        double *kernels, double *probs, double *grads, double *hessians, double *loghessians, int nlhs) {*/

void KernelDensityEstimation(double *samples, int dim, int *data, double *weights,
		double *eigenVecs, double *eigenVals, double *normP, int Nsamples, int Ndata, double beta, double *alpha_ij,
		double *kernels, double *probs, double *grads, double *hessians, double *loghessians, int nlhs)
{
    int k, j, i, l, l_;
    double addVal, detLambda_i, innerProd;

	double u_i[3];
    double d_i[3];
    double Hess [3*3];
#if 0	
	double u_i[2];
    double d_i[2];
   // double alpha_ij[2*1*DEFAULT_LOOKUP_WIDTH*DEFAULT_LOOKUP_WIDTH];
	 //double alpha_ij[2*1*15*15];
    double Hess [2*2];
#endif

#if 0
	double u_i[3];
    double d_i[3];
//    double alpha_ij[3*1*DEFAULT_LOOKUP_WIDTH*DEFAULT_LOOKUP_WIDTH*DEFAULT_LOOKUP_WIDTH];
   //double alpha_ij[3*1*15*15*15];
    double Hess [3*3];
#endif

#if 0
	double *u_i = new double [dim];
    double *d_i = new double [dim];
    double *alpha_ij = new double[dim*Ndata];
    double *Hess = new double[dim*dim];
#endif

	for (k=0; k<Nsamples; k++) {
        // get the ith sample and calculate the difference between jth data point
		probs[k] = 0;
		for (l=0; l<dim; l++)
		{					
			grads[l+dim*k] = 0;
			for (l_= l; l_<dim; l_++) 
			{
				hessians[k*dim*dim+l*dim+l_] = 0;
			}
		}
#if 1                    
        for (i=0; i<Ndata; i++) {
            detLambda_i = 1;
            for (l=0; l<dim; l++) { // difference vector
                d_i[l] =  samples[k*dim+l] - ((double)data[i*dim+l]);
            }
            
            addVal = 0;
            for (j=0; j<dim; j++) {
                // get the jth withening vector : q_j/lambda_j^.5
                // and calculate the inner product with d_i
                
                innerProd = 0;
                for (l=0; l<dim; l++) { // difference vector
                    innerProd += eigenVecs[i*dim*dim+j*dim+l]*d_i[l];
                }
                
                alpha_ij[j+dim*i]=innerProd/sqrt(eigenVals[i*dim+j]);
                addVal += pow(alpha_ij[j+dim*i], 2);
            }
            
            kernels[k+i*Nsamples] = weights[i]*normP[i]*exp(-pow(addVal, beta)/2);
            probs[k] += kernels[k+i*Nsamples];
            
            if (nlhs>2) {
                // Contribution from the i th data to the estimation of the gradient of the kth sample
                // calculate u_i
				
                for (l=0; l<dim; l++) {					
                    u_i[l]=0;
                    for (j=0; j<dim; j++) {
                        u_i[l] += alpha_ij[j+dim*i]*eigenVecs[i*dim*dim+j*dim+l]/sqrt(eigenVals[i*dim+j]);
                    }
                    
                    // calculate g_i
                    grads[l+dim*k] -= kernels[k+i*Nsamples]*beta*pow(addVal, beta-1)*u_i[l];
                }
            }

            if (nlhs>3) {
                // Contribution from the i th data to the estimation of the Hessian of the kth sample
                for (l=0; l<dim; l++) {
                    for (l_=0; l_<dim; l_++) {
                        Hess[l*dim+l_] = 0;
                    }
                }
                for (j=0; j<dim; j++) {
                    for (l=0; l<dim; l++) {
                        for (l_=0; l_<dim; l_++) {
                            Hess[l*dim+l_] -= eigenVecs[i*dim*dim+j*dim+l]*eigenVecs[i*dim*dim+j*dim+l_]/(eigenVals[i*dim+j]);
                        }
                    }
                }
                
                for (l=0; l<dim; l++) {
                    for (l_=l; l_<dim; l_++) { // only upper tri half
                        if(beta>=2) {
                            Hess[l*dim+l_] *= addVal;
                            Hess[l*dim+l_] += (beta*pow(addVal, beta) - 2*(beta-1))*u_i[l]*u_i[l_];
                            hessians[k*dim*dim+l*dim+l_] += Hess[l*dim+l_] * beta*pow(addVal, beta-2)*kernels[k+i*Nsamples];
                        }
                        else {
                            Hess[l*dim+l_] += (beta*pow(addVal, beta-1))*u_i[l]*u_i[l_]* beta*pow(addVal, beta-1);
                            hessians[k*dim*dim+l*dim+l_] += Hess[l*dim+l_] * beta*pow(addVal, beta-1)*kernels[k+i*Nsamples];
                        }
                        hessians[k*dim*dim+l_*dim+l] = hessians[k*dim*dim+l*dim+l_];
                    }
                }
            }
        }  // End of data points
        if (nlhs>4) {
            bool Hlogp = true;
            if (Hlogp) {
                for (l=0; l<dim; l++) {
                    for (l_=0; l_<dim; l_++) { // only upper tri half
                        loghessians[k*dim*dim+l*dim+l_] = hessians[k*dim*dim+l*dim+l_]/probs[k] -
                                grads[l+dim*k]*grads[l_+dim*k]/pow(probs[k], 2);
//                         loghessians[k*dim*dim+l_*dim+l] = hessians[k*dim*dim+l*dim+l_];
                    }
                }
            }
        }
#endif
    }  // End of samples to be evaluated
     //delete [] Hess, u_i, d_i, alpha_ij;

}
