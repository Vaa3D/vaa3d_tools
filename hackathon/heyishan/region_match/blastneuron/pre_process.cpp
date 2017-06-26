
#include "pre_process.h"
#include"prune_short_branch.h"
#include"basic_surf_objs.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
NeuronTree pre_process(NeuronTree nt, double step_size, double prune_size)
{
    NeuronTree result;
    NeuronTree pruned;
    if (!prune_branch(nt, pruned,prune_size))
    {
        fprintf(stderr,"Error in prune_short_branch.\n");
        return result;
    }

    printf("Resampling along segments\n");
    NeuronTree resampled = resample(pruned, step_size);


//    printf("Sort \n");
//    NeuronTree sorted = sort(resampled, step_size);


    printf("Aligning PCA axis\n");
    result = align_axis(resampled);

    return result;
}

NeuronTree align_axis(NeuronTree input)
{
    NeuronTree result;
    result.deepCopy(input);
    V3DLONG siz = input.listNeuron.size();
    printf("size=%d\n", siz);

    double * coord = new double[siz * 3];
    for (V3DLONG i=0; i<siz; i++)
    {
        coord[i] = input.listNeuron[i].x;
        coord[siz + i] = input.listNeuron[i].y;
        coord[2*siz + i] = input.listNeuron[i].z;
    }


    double * coord_pca = new double[siz * 3];
    double * PC = new double[siz];
    double V[3];

    pca1(coord, 3, siz, coord_pca, PC, V);

    printf("PCA done\n");

    for (V3DLONG i=0;i<siz; i++)
    {
        result.listNeuron[i].x = coord_pca[i];
        result.listNeuron[i].y = coord_pca[siz + i];
        result.listNeuron[i].z = coord_pca[2*siz + i];
        result.listNeuron[i].type=input.listNeuron[i].type;
    }

    delete []coord;
    delete []coord_pca;
    delete []PC;

    return result;
}


#define ALGO_DEBUG

/*
   �ú�˹�ɶ��£�Householder���任��n��ʵ�Գƾ���Լ��Ϊ�Գ����Խ���
   ��ʿ�. �����㷨���򼯣�C�����������3��. �廪��ѧ������. 2004

   double a[] --- ά��Ϊnxn������n��ʵ�Գƾ���A
   int n      --- ʵ�Գƾ���A�Ľ���
   double q[] --- ά��nxn������Householder�任�ĳ˻�����Q������
                  Tri_Symmetry_Diagonal_Eigenvector()������ã�����
                  Q������Ϊ�ú�����һ������ʱ���ɼ���ʵ�Գ���A������
                  ����Ӧ���������
   double b[] --- ά��Ϊn�����ضԳ����Խ����е����Խ���Ԫ��
   double c[] --- ����n��ǰn��1��Ԫ�ط��ضԳ����Խ����еĴζԽ���Ԫ�ء�
*/
void Householder_Tri_Symetry_Diagonal( double a[], int n, double q[],
                                       double b[], double c[] )
{
    int i, j, k, u;
    double h, f, g, h2;

    for ( i = 0; i <= n-1; i++ )
        for ( j = 0; j <= n-1; j++ )
        {
            u = i * n + j;
            q[u] = a[u];
        }
    for ( i = n-1; i >= 1; i-- )
    {
        h = 0.0;
        if ( i > 1 )
            for ( k = 0; k <= i-1; k++ )
            {
                u = i * n + k;
                h = h + q[u] * q[u];
            }
        if ( h + 1.0 == 1.0 )
        {
            c[i] = 0.0;
            if ( i == 1 ) c[i] = q[i*n+i-1];
            b[i] = 0.0;
        }
        else
        {
            c[i] = sqrt( h );
            u = i * n + i - 1;
            if ( q[u] > 0.0 ) c[i] = -c[i];
            h = h - q[u] * c[i];
            q[u] = q[u] - c[i];
            f = 0.0;
            for ( j = 0; j <= i - 1; j++ )
            {
                q[j*n+i] = q[i*n+j] / h;
                g = 0.0;
                for ( k = 0; k <= j; k++ )
                    g = g + q[j*n+k] * q[i*n+k];
                if ( j + 1 <= i-1 )
                    for ( k = j+1; k <= i-1; k++ )
                        g = g + q[k*n+j] * q[i*n+k];
                    c[j] = g / h;
                    f = f + g * q[j*n+i];
            }
            h2 = f / ( h + h );
            for ( j = 0; j <= i-1; j++ )
            {
                f = q[i*n+j];
                g = c[j] - h2 * f;
                c[j] = g;
                for ( k = 0; k <= j; k++ )
                {
                    u = j * n + k;
                    q[u] = q[u] - f * c[k] - g * q[i*n+k];
                }
            }
            b[i] = h;
        }
    }
    for ( i = 0; i <= n-2; i++ ) c[i] = c[i+1];
    c[n-1] = 0.0;
    b[0] = 0.0;
    for ( i = 0; i <= n-1; i++ )
    {
        if ( ( b[i] != 0.0 ) && ( i-1 >= 0 ) )
            for ( j = 0; j <= i-1; j++ )
            {
                g = 0.0;
                for ( k = 0; k <= i-1; k++ )
                    g = g + q[i*n+k] * q[k*n+j];
                for ( k = 0; k <= i-1; k++ )
                {
                    u = k * n + j;
                    q[u] = q[u] - g * q[k*n+i];
                }
            }
        u = i * n + i;
        b[i] = q[u]; q[u] = 1.0;
        if ( i - 1 >= 0 )
            for ( j = 0; j <= i - 1; j++ )
            {
                q[i*n+j] = 0.0; q[j*n+i] = 0.0;
            }
    }

    return;
}

/*
   ����ʵ�Գ����Խ�����ȫ������ֵ����Ӧ�������
   ��ʿ�. �����㷨���򼯣�C�����������3��. �廪��ѧ������. 2004

   int n    --- ʵ�Գ����Խ����Ľ���
   double b --- ����Ϊn������n�׶Գ����Խ��������Խ����ϵĸ�Ԫ�أ�
                ����ʱ����ȫ������ֵ
   double c --- ����Ϊn��ǰn��1��Ԫ�ش���n�׶Գ����Խ����ĴζԽ�
                ���ϵ�Ԫ��
   double q --- ά��Ϊnxn�������ŵ�λ�������򷵻�n��ʵ�Գ����Խ�
                ��T����������飻������Householder_Tri_Symetry_Diagonal()
                ���������ص�һ��ʵ�Գƾ���A�ĺ�˹�ɶ��ñ任�ĳ�
                ������Q���򷵻�ʵ�Գƾ���A����������顣���У�q��
                �ĵ�j��Ϊ������b�е�j������ֵ��Ӧ���������
   double eps --- �������ڵ��������еĿ��ƾ���Ҫ��
   int l    --- Ϊ����һ������ֵ��������������������
   ����ֵ��
   ��ر���С��0����˵��������l�λ�δ����һ������ֵ������fail
   ��Ϣ��������ر��Ǵ���0����˵����������������ȫ������ֵ��һ
   ά����b����������������ɶ�ά����q����
*/
int Tri_Symmetry_Diagonal_Eigenvector( int n, double b[], double c[],
                                       double q[], double eps, int l )
{
    int i, j, k, m, it, u, v;
    double d, f, h, g, p, r, e, s;

    c[n-1] = 0.0; d = 0.0; f = 0.0;
    for ( j = 0; j <= n-1; j++ )
    {
        it = 0;
        h = eps * ( fabs( b[j] ) + fabs( c[j] ) );
        if ( h > d ) d = h;
        m = j;
        while ( ( m <= n-1 ) && ( fabs( c[m] ) > d ) ) m = m+1;
        if ( m != j )
        {
            do
            {
                if ( it == l )
                {
                    #ifdef ALGO_DEBUG
                    printf( "fail\n" );
                    #endif
                    return( -1 );
                }
                it = it + 1;
                g = b[j];
                p = ( b[j+1] - g ) / ( 2.0 * c[j] );
                r = sqrt( p * p + 1.0 );
                if ( p >= 0.0 )
                    b[j] = c[j] / ( p + r );
                else
                    b[j] = c[j] / ( p - r );
                h = g - b[j];
                for ( i = j+1; i <= n-1; i++ )
                    b[i] = b[i] - h;
                f = f + h; p = b[m]; e = 1.0; s = 0.0;
                for ( i = m-1; i >= j; i-- )
                {
                    g = e * c[i]; h = e * p;
                    if ( fabs( p ) >= fabs( c[i] ) )
                    {
                        e = c[i] / p; r = sqrt( e * e + 1.0 );
                        c[i+1] = s * p * r; s = e / r; e = 1.0 / r;
                    }
                    else
                    {
                        e = p / c[i]; r = sqrt( e * e + 1.0 );
                        c[i+1] = s * c[i] * r;
                        s = 1.0 / r; e = e / r;
                    }
                    p = e * b[i] - s * g;
                    b[i+1] = h + s * ( e * g + s * b[i] );
                    for ( k = 0; k <= n-1; k++ )
                    {
                        u = k * n + i + 1; v = u - 1;
                        h = q[u]; q[u] = s * q[v] + e * h;
                        q[v] = e * q[v] - s * h;
                    }
                }
                c[j] = s * p; b[j] = e * p;
            }
            while ( fabs( c[j] ) > d );
        }
        b[j] = b[j] + f;
    }
    for ( i = 0; i <= n-1; i++ )
    {
        k = i; p = b[i];
        if ( i+1 <= n-1 )
        {
            j = i+1;
            while ( ( j <= n-1 ) && ( b[j] <= p ) )
            {
                k = j; p = b[j]; j = j+1;
            }
        }
        if ( k != i )
        {
            b[k] = b[i]; b[i] = p;
            for ( j = 0; j <= n-1; j++ )
            {
                u = j * n + i; v = j * n + k;
                p = q[u]; q[u] = q[v]; q[v] = p;
            }
        }
    }

    return( 1 );
}

# define EPS    0.000001   /* ���㾫�� */
# define Iteration    60   /* ��ȡ������������������� */

/*
   ����ʵ�Գ�����ȫ������ֵ����Ӧ�������
   int n              --- ʵ�Գ����Ľ���
   double * CovMatrix --- ά��Ϊnxn������n�׶Գ����ĸ�Ԫ�أ�
   double * Eigen     --- ����Ϊn��Ϊn������ֵ
   double * EigenVector --- ά��Ϊnxn������n��ʵ�Գ�����������������У�
                            EigenVector�е�j��Ϊ������Eigen�е�j������ֵ
                            ��Ӧ���������
   ����ֵ��
   ��ر���С��0����˵��������Iteration�λ�δ����һ������ֵ��
   ��ر��Ǵ���0����˵����������������ȫ������ֵ��һ
   ά����Eigen����������������ɶ�ά����EigenVector����
*/
int SymmetricRealMatrix_Eigen( double CovMatrix[], int n,
                               double Eigen[], double EigenVector[] )
{
    int k;
    double * subDiagonal;

    subDiagonal = ( double * )malloc( sizeof( double ) * n );
    Householder_Tri_Symetry_Diagonal( CovMatrix, n, EigenVector, Eigen, subDiagonal );
    k = Tri_Symmetry_Diagonal_Eigenvector( n, Eigen, subDiagonal, EigenVector, EPS, Iteration );
    free( subDiagonal );

    return( k );
}

/*
  PCA1: Perform PCA using covariance.
  data     --- MxN matrix of input data ( M dimensions, N trials )
               ����Ϊԭʼ����ά����ÿ������Ϊһ������
  signal  --- MxN matrix of projected data
  PC       --- each column is a PC
  V        --- Mx1 matrix of variances
  row = M dimensions, col = N trials
*/
int pca1( double * data, int row, int col,
          double * signal, double * PC, double * V )
{
    int x, y, k;
    double * mean;
    double * data1, * tPC, * tV;
    double * covariance, temp;
    int rvalue, * no, tp;

    if ( row <= 1 || col <= 1 ) return( -1 );
    /* subtract off the mean for each dimension */
    mean = ( double * )malloc( sizeof( double ) * row );
    data1 = ( double * )malloc( sizeof( double ) * row * col );
    for ( y = 0; y < row; y++ ) /* calculate mean */
    {
        mean[y] = 0;
        for ( x = 0; x < col; x++ )
            mean[y] += data[y*col+x];
    }
    for ( y = 0; y < row; y++ ) mean[y] = mean[y]/col;
    for ( y = 0; y < row; y++ ) /* subtract mean */
        for ( x = 0; x < col; x++ )
        {
            data1[y*col+x] = data[y*col+x] - mean[y];
        }
    free( mean );
    /* calculate the covariance matrix */
    covariance = ( double * )malloc( sizeof( double ) * row * row );
    for ( y = 0; y < row; y++ )
        for ( x = 0; x < row; x++ )
        {
            temp = 0;
            for ( k = 0; k < col; k++ )
                temp += data1[y*col+k] * data1[x*col+k];
            temp = temp / ( col-1 );
            covariance[x*row+y] = temp;
        }
    /* find the eigenvectors and eigenvalues */
    rvalue = SymmetricRealMatrix_Eigen( covariance, row, V, PC );
    free( covariance );
    /* sort the variances in decreasing order */
    no = ( int * )malloc( sizeof( int ) * row );
    for ( x = 0; x < row; x++ ) no[x] = x;
    for ( x = 0; x < row-1; x++ )
    {
        temp = V[x];
        for ( k = x; k < row; k++ )
            if ( temp < V[k] )
            {
                tp = no[k];
                no[k] = no[x];
                no[x] = tp;
                temp = V[k];
            }
    }
    /* exchange eigen value and vector in decreasing order */
    tV = ( double * )malloc( sizeof( double ) * row );
    tPC = ( double * )malloc( sizeof( double ) * row * row );
    for ( x = 0; x < row; x++ ) tV[x] = V[x];
    for ( x = 0; x < row; x++ )
        for ( y = 0; y < row; y++ )
            tPC[x*row+y] = PC[x*row+y];
    for ( x = 0; x < row; x++ )
    {
        if ( no[x] != x )
        {
            for ( y = 0; y < row; y++ )
                PC[y*row+x] = tPC[y*row+no[x]];
            V[x] = tV[no[x]];
        }
    }
    free( no );
    free( tV );
    free( tPC );

    /* project the original data: signal = PC' * data; */
    for ( y = 0; y < row; y++ )
        for ( x = 0; x < col; x++ )
        {
            signal[y*col+x] = 0;
            for ( k = 0; k < row; k++ )
                signal[y*col+x] += PC[k*row+y] * data1[k*col+x];
        }
    free( data1 );

    return( rvalue );
}

/*
  ͶӰ����Ԫ�������
  double * newdata --- ҪͶӰ����Ԫ�ط���ϵ����ݾ���
  int row, col     --- ���ݾ�����ά��
                       row��������Ϊ��Ԫ���ά��������������
                       col��������Ϊ������
  double * PC      --- ��Ԫ����������������
  double * newsignal --- ͶӰ�����õ��źŵ�ϵ����row*col��
  double * ShiftValue --- ����һ����row��Ԫ�ص�ƫ�����������ֵΪNULL
                          ��������ֵ������ȥ��ֵ��������ȥ��ֵ
*/
int project2PCA( double * newdata, int row, int col,
                 double * PC, double * newsignal, double * ShiftValue )
{
    int x, y, k;
    double * mean, * data1;

    /* subtract off the mean for each dimension */
    data1 = ( double * )malloc( sizeof( double ) * row * col );
    /* if ShiftValue <> NULL */
    if ( ShiftValue != NULL )
    {
        for ( y = 0; y < row; y++ ) /* subtract ShiftValue */
            for ( x = 0; x < col; x++ )
            {
                data1[y*col+x] = newdata[y*col+x] - ShiftValue[y];
            }
    }
    else
    {
        mean = ( double * )malloc( sizeof( double ) * row );
        for ( y = 0; y < row; y++ ) /* calculate mean */
        {
            mean[y] = 0;
            for ( x = 0; x < col; x++ )
                mean[y] += newdata[y*col+x];
        }
        if ( col <= 1 )
            for ( y = 0; y < row; y++ ) mean[y] = 0;
        else
            for ( y = 0; y < row; y++ ) mean[y] = mean[y]/col;
        for ( y = 0; y < row; y++ ) /* subtract mean */
            for ( x = 0; x < col; x++ )
            {
                data1[y*col+x] = newdata[y*col+x] - mean[y];
            }
        free( mean );
    }
    /* project the original data: signal = PC' * data; */
    for ( y = 0; y < row; y++ )
        for ( x = 0; x < col; x++ )
        {
            newsignal[y*col+x] = 0;
            for ( k = 0; k < row; k++ )
                newsignal[y*col+x] += PC[k*row+y] * data1[k*col+x];
        }
    free( data1 );

    return( 1 );
}
