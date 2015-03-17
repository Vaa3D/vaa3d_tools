#include "math.h"
#include "EigenDecomposition3.h"

#ifdef MAX
#undef MAX
#endif
#define MAX(a, b) ((a)>(b)?(a):(b))
#define absd(a) ((a)>(-a)?(a):(-a))
#ifndef pow2
#define pow2(a) (a*a)
#endif
#define n 3
#define inv3 0.3333333333333333
#define root3 1.7320508075688772

/* Eigen decomposition code for symmetric 3x3 matrices, copied from the public
 * domain Java Matrix library JAMA. */
static double hypot2(double x, double y) { return sqrt(x*x+y*y); }

/* Symmetric Householder reduction to tridiagonal form. */
static void tred2(double V[n][n], double d[n], double e[n]) {
    
/*  This is derived from the Algol procedures tred2 by */
/*  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for */
/*  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding */
/*  Fortran subroutine in EISPACK. */
    int i, j, k;
    double scale;
    double f, g, h;
    double hh;
    for (j = 0; j < n; j++) {d[j] = V[n-1][j]; }
    
    /* Householder reduction to tridiagonal form. */
    
    for (i = n-1; i > 0; i--) {
        /* Scale to avoid under/overflow. */
        scale = 0.0;
        h = 0.0;
        for (k = 0; k < i; k++) { scale = scale + fabs(d[k]); }
        if (scale == 0.0) {
            e[i] = d[i-1];
            for (j = 0; j < i; j++) { d[j] = V[i-1][j]; V[i][j] = 0.0;  V[j][i] = 0.0; }
        } else {
            
            /* Generate Householder vector. */
            
            for (k = 0; k < i; k++) { d[k] /= scale; h += d[k] * d[k]; }
            f = d[i-1];
            g = sqrt(h);
            if (f > 0) { g = -g; }
            e[i] = scale * g;
            h = h - f * g;
            d[i-1] = f - g;
            for (j = 0; j < i; j++) { e[j] = 0.0; }
            
            /* Apply similarity transformation to remaining columns. */
            
            for (j = 0; j < i; j++) {
                f = d[j];
                V[j][i] = f;
                g = e[j] + V[j][j] * f;
                for (k = j+1; k <= i-1; k++) { g += V[k][j] * d[k]; e[k] += V[k][j] * f; }
                e[j] = g;
            }
            f = 0.0;
            for (j = 0; j < i; j++) { e[j] /= h; f += e[j] * d[j]; }
            hh = f / (h + h);
            for (j = 0; j < i; j++) { e[j] -= hh * d[j]; }
            for (j = 0; j < i; j++) {
                f = d[j]; g = e[j];
                for (k = j; k <= i-1; k++) { V[k][j] -= (f * e[k] + g * d[k]); }
                d[j] = V[i-1][j];
                V[i][j] = 0.0;
            }
        }
        d[i] = h;
    }
    
    /* Accumulate transformations. */
    
    for (i = 0; i < n-1; i++) {
        V[n-1][i] = V[i][i];
        V[i][i] = 1.0;
        h = d[i+1];
        if (h != 0.0) {
            for (k = 0; k <= i; k++) { d[k] = V[k][i+1] / h;}
            for (j = 0; j <= i; j++) {
                g = 0.0;
                for (k = 0; k <= i; k++) { g += V[k][i+1] * V[k][j]; }
                for (k = 0; k <= i; k++) { V[k][j] -= g * d[k]; }
            }
        }
        for (k = 0; k <= i; k++) { V[k][i+1] = 0.0;}
    }
    for (j = 0; j < n; j++) { d[j] = V[n-1][j]; V[n-1][j] = 0.0; }
    V[n-1][n-1] = 1.0;
    e[0] = 0.0;
}

/* Symmetric tridiagonal QL algorithm. */
static void tql2(double V[n][n], double d[n], double e[n]) {
    
/*  This is derived from the Algol procedures tql2, by */
/*  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for */
/*  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding */
/*  Fortran subroutine in EISPACK. */
    
    int i, j, k, l, m;
    double f;
    double tst1;
    double eps;
    int iter;
    double g, p, r;
    double dl1, h, c, c2, c3, el1, s, s2;
    
    for (i = 1; i < n; i++) { e[i-1] = e[i]; }
    e[n-1] = 0.0;
    
    f = 0.0;
    tst1 = 0.0;
    eps = pow(2.0, -52.0);
    for (l = 0; l < n; l++) {
        
        /* Find small subdiagonal element */
        
        tst1 = MAX(tst1, fabs(d[l]) + fabs(e[l]));
        m = l;
        while (m < n) {
            if (fabs(e[m]) <= eps*tst1) { break; }
            m++;
        }
        
        /* If m == l, d[l] is an eigenvalue, */
        /* otherwise, iterate. */
        
        if (m > l) {
            iter = 0;
            do {
                iter = iter + 1;  /* (Could check iteration count here.) */
                /* Compute implicit shift */
                g = d[l];
                p = (d[l+1] - g) / (2.0 * e[l]);
                r = hypot2(p, 1.0);
                if (p < 0) { r = -r; }
                d[l] = e[l] / (p + r);
                d[l+1] = e[l] * (p + r);
                dl1 = d[l+1];
                h = g - d[l];
                for (i = l+2; i < n; i++) { d[i] -= h; }
                f = f + h;
                /* Implicit QL transformation. */
                p = d[m]; c = 1.0; c2 = c; c3 = c;
                el1 = e[l+1]; s = 0.0; s2 = 0.0;
                for (i = m-1; i >= l; i--) {
                    c3 = c2;
                    c2 = c;
                    s2 = s;
                    g = c * e[i];
                    h = c * p;
                    r = hypot2(p, e[i]);
                    e[i+1] = s * r;
                    s = e[i] / r;
                    c = p / r;
                    p = c * d[i] - s * g;
                    d[i+1] = h + s * (c * g + s * d[i]);
                    /* Accumulate transformation. */
                    for (k = 0; k < n; k++) {
                        h = V[k][i+1];
                        V[k][i+1] = s * V[k][i] + c * h;
                        V[k][i] = c * V[k][i] - s * h;
                    }
                }
                p = -s * s2 * c3 * el1 * e[l] / dl1;
                e[l] = s * p;
                d[l] = c * p;
                
                /* Check for convergence. */
            } while (fabs(e[l]) > eps*tst1);
        }
        d[l] = d[l] + f;
        e[l] = 0.0;
    }
    
    /* Sort eigenvalues and corresponding vectors. */
    for (i = 0; i < n-1; i++) {
        k = i;
        p = d[i];
        for (j = i+1; j < n; j++) {
            if (d[j] < p) {
                k = j;
                p = d[j];
            }
        }
        if (k != i) {
            d[k] = d[i];
            d[i] = p;
            for (j = 0; j < n; j++) {
                p = V[j][i];
                V[j][i] = V[j][k];
                V[j][k] = p;
            }
        }
    }
}



void roots3(double d[3], double c0,double c1, double c2)
{
    double c2Div3, aDiv3, mbDiv2, q, magnitude, angle, cs, sn;
    
    /* Solve the roots of  y^3 + c2 * y^2 + c1 *y + c0  */
    c2Div3 = -c2*inv3;
    aDiv3 = (c1 + c2*c2Div3)*inv3;
    if (aDiv3 > 0.0) { aDiv3 = 0.0; }
    mbDiv2 = 0.5*(-c0 + c2Div3*(2.0*c2Div3*c2Div3 - c1));
    q = mbDiv2*mbDiv2 + aDiv3*aDiv3*aDiv3;
    if (q > 0.0) { q = 0.0; }
    magnitude = sqrt(-aDiv3);
    angle = atan2(sqrt(-q),mbDiv2)*inv3;
    cs = cos(angle);
    sn = sin(angle);
    d[0] = c2Div3 + 2.0*magnitude*cs;
    d[1] = c2Div3 - magnitude*(cs + root3*sn);
    d[2] = c2Div3 - magnitude*(cs - root3*sn);
}   

int fast_eigen3x3(double A[3][3], double V[3][3], double d[3])
{
    const double smallv=1e-12;
    double c0, c1,c2;
    int check;
	double l1, l2, l3;
    double t;
    double a1, a2, a3, b1, b2;
    double da[3];
    check=(absd(A[0][1])<smallv)+(absd(A[0][2])<smallv)+(absd(A[1][2])<smallv);
    if(check>1) { return 0; }

    /* 0 = - det (A - yI) = y^3 + c2 * y^2 + c1 *y + c0 */
    c0 = -(A[0][0]*A[1][1]*A[2][2] + 2*A[0][1]*A[0][2]*A[1][2] - A[0][0] * pow2(A[1][2]) - A[1][1]*pow2(A[0][2]) - A[2][2]*pow2(A[0][1]));
    c1 = A[0][0]*A[1][1] - pow2(A[0][1]) + A[0][0]*A[2][2] -pow2(A[0][2]) + A[1][1]*A[2][2] - pow2(A[1][2]);
    c2 = - (A[0][0] + A[1][1] + A[2][2]);

    /* Solve the roots of  y^3 + c2 * y^2 + c1 *y + c0  */
    roots3(d, c0, c1, c2);

    da[0]=absd(d[0]); da[1]=absd(d[1]); da[2]=absd(d[2]);
    /* Sort eigenvalues */
    if(da[0]>=da[1])
    {
        if(da[0]>da[2])
        {
            t=d[0]; d[0]=d[2]; d[2]=t; 
            t=da[0]; da[0]=da[2]; da[2]=t; 
        }
    }
    else if(da[1]>da[2])
    {
        t=d[1]; d[1]=d[2]; d[2]=t; 
        t=da[1]; da[1]=da[2]; da[2]=t; 

    }
    
    if(da[0]>=da[1])
    {
        t=d[0]; d[0]=d[1]; d[1]=t; 
        t=da[0]; da[0]=da[1]; da[1]=t; 
    }

    if((da[1]-da[0])<smallv) { return 0; }
    if((da[2]-da[1])<smallv) { return 0; }
    
    /* Calculate eigen vectors */
    a1=A[0][1]*A[1][2]; a2=A[0][1]*A[0][2]; a3=pow2(A[0][1]);

    b1=A[0][0]-d[0]; b2=A[1][1]-d[0];
    V[0][0]=a1-A[0][2]*b2; V[1][0]=a2-A[1][2]*b1; V[2][0]=b1*b2-a3;

    b1=A[0][0]-d[1]; b2=A[1][1]-d[1];
    V[0][1]=a1-A[0][2]*b2; V[1][1]=a2-A[1][2]*b1; V[2][1]=b1*b2-a3;

    b1=A[0][0]-d[2]; b2=A[1][1]-d[2];
    V[0][2]=a1-A[0][2]*b2; V[1][2]=a2-A[1][2]*b1; V[2][2]=b1*b2-a3;


    /* Eigen vector normalization */
    l1=sqrt(pow2(V[0][0])+ pow2(V[1][0]) + pow2(V[2][0]));
    l2=sqrt(pow2(V[0][1])+ pow2(V[1][1]) + pow2(V[2][1]));
    l3=sqrt(pow2(V[0][2])+ pow2(V[1][2]) + pow2(V[2][2]));

    /* Detect fail : eigenvectors with only zeros */
    if(l1<smallv) { return 0; }
    if(l2<smallv) {	return 0; }
    if(l3<smallv) { return 0; }

    V[0][0]/=l1; V[0][1]/=l2; V[0][2]/=l3;
    V[1][0]/=l1; V[1][1]/=l2; V[1][2]/=l3;
    V[2][0]/=l1; V[2][1]/=l2; V[2][2]/=l3;
    
    /* Succes    */
    return 1;
}

void eigen_decomposition(double A[n][n], double V[n][n], double d[n]) {
    double e[n];
    double da[3];
    double dt, dat;
    double vet[3];
    int i, j;
    
    if(fast_eigen3x3(A, V, d)) { return; }
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            V[i][j] = A[i][j];
        }
    }
    tred2(V, d, e);
    tql2(V, d, e);
    
    /* Sort the eigen values and vectors by abs eigen value */
    da[0]=absd(d[0]); da[1]=absd(d[1]); da[2]=absd(d[2]);
    if((da[0]>=da[1])&&(da[0]>da[2]))
    {
        dt=d[2];   dat=da[2];    vet[0]=V[0][2];    vet[1]=V[1][2];    vet[2]=V[2][2];
        d[2]=d[0]; da[2]=da[0];  V[0][2] = V[0][0]; V[1][2] = V[1][0]; V[2][2] = V[2][0];
        d[0]=dt;   da[0]=dat;    V[0][0] = vet[0];  V[1][0] = vet[1];  V[2][0] = vet[2]; 
    }
    else if((da[1]>=da[0])&&(da[1]>da[2]))  
    {
        dt=d[2];   dat=da[2];    vet[0]=V[0][2];    vet[1]=V[1][2];    vet[2]=V[2][2];
        d[2]=d[1]; da[2]=da[1];  V[0][2] = V[0][1]; V[1][2] = V[1][1]; V[2][2] = V[2][1];
        d[1]=dt;   da[1]=dat;    V[0][1] = vet[0];  V[1][1] = vet[1];  V[2][1] = vet[2]; 
    }
    if(da[0]>da[1])
    {
        dt=d[1];   dat=da[1];    vet[0]=V[0][1];    vet[1]=V[1][1];    vet[2]=V[2][1];
        d[1]=d[0]; da[1]=da[0];  V[0][1] = V[0][0]; V[1][1] = V[1][0]; V[2][1] = V[2][0];
        d[0]=dt;   da[0]=dat;    V[0][0] = vet[0];  V[1][0] = vet[1];  V[2][0] = vet[2]; 
    }
}





