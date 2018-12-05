/*
 * Copyright (c) 2008-2011 Zhang Ming (M. Zhang), zmjerry@163.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 or any later version.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details. A copy of the GNU General Public License is available at:
 * http://www.fsf.org/licensing/licenses
 */


/*****************************************************************************
 *                               svd-impl.h
 *
 * Implementation for SVD class.
 *
 * Zhang Ming, 2010-01 (revised 2010-08), Xi'an Jiaotong University.
 *****************************************************************************/
#include "SVD.h"


/**
 * constructor and destructor
 */
using namespace splab;
template<typename Real>
SVD<Real>::SVD()
{
}

template<typename Real>
SVD<Real>::~SVD()
{
}


/**
 * Making singular decomposition.
 */
template <typename Real>
void SVD<Real>::dec( const Matrix<Real> &A )
{
    int m = A.rows(),
        n = A.cols(),
        p = min( m, n );

    U = Matrix<Real>( m, p );
    V = Matrix<Real>( n, p );
    S = vector<Real>( p );
    if( m >= n )
    {
        Matrix<Real> B(A);
        decomposition( B, U, S, V );
    }
    else
    {
        Matrix<Real> B( trT( A ) );
        decomposition( B, V, S, U );
    }
}


/**
 * Making singular decomposition of m >= n.
 */
template <typename Real>
void SVD<Real>::decomposition( Matrix<Real> &B, Matrix<Real> &U,
                               vector<Real> &S, Matrix<Real> &V )
{
    int m = B.rows(),
        n = B.cols();

    vector<Real> e(n);
    vector<Real> work(m);

    // boolean
    int wantu = 1;
    int wantv = 1;

    // Reduce A to bidiagonal form, storing the diagonal elements
    // in s and the super-diagonal elements in e.
    int nct = min( m-1, n );
    int nrt = max( 0, n-2 );
    int i=0,
        j=0,
        k=0;

    for( k=0; k<max(nct,nrt); ++k )
    {
        if( k < nct )
        {
            // Compute the transformation for the k-th column and
            // place the k-th diagonal in s[k].
            // Compute 2-norm of k-th column without under/overflow.
            S[k] = 0;
            for( i=k; i<m; ++i )
                 S[k] = hypot( S[k], B[i][k] );

            if( S[k] != 0 )
            {
                if( B[k][k] < 0 )
                    S[k] = -S[k];

                for( i=k; i<m; ++i )
                    B[i][k] /= S[k];
                B[k][k] += 1;
            }
            S[k] = -S[k];
        }

        for( j=k+1; j<n; ++j )
        {
            if( (k < nct) && ( S[k] != 0 ) )
            {
                // apply the transformation
                Real t = 0;
                for( i=k; i<m; ++i )
                    t += B[i][k] * B[i][j];

                t = -t / B[k][k];
                for( i=k; i<m; ++i )
                    B[i][j] += t*B[i][k];
            }
            e[j] = B[k][j];
        }

        // Place the transformation in U for subsequent back
        // multiplication.
        if( wantu & (k < nct) )
            for( i=k; i<m; ++i )
                U[i][k] = B[i][k];

        if( k < nrt )
        {
            // Compute the k-th row transformation and place the
            // k-th super-diagonal in e[k].
            // Compute 2-norm without under/overflow.
            e[k] = 0;
            for( i=k+1; i<n; ++i )
                e[k] = hypot( e[k], e[i] );

            if( e[k] != 0 )
            {
                if( e[k+1] < 0 )
                    e[k] = -e[k];

                for( i=k+1; i<n; ++i )
                    e[i] /= e[k];
                e[k+1] += 1;
            }
            e[k] = -e[k];

            if( (k+1 < m) && ( e[k] != 0 ) )
            {
                // apply the transformation
                for( i=k+1; i<m; ++i )
                    work[i] = 0;

                for( j=k+1; j<n; ++j )
                    for( i=k+1; i<m; ++i )
                        work[i] += e[j] * B[i][j];

                for( j=k+1; j<n; ++j )
                {
                    Real t = -e[j]/e[k+1];
                    for( i=k+1; i<m; ++i )
                        B[i][j] += t * work[i];
                }
            }

            // Place the transformation in V for subsequent
            // back multiplication.
            if( wantv )
                for( i=k+1; i<n; ++i )
                    V[i][k] = e[i];
        }
    }

    // Set up the final bidiagonal matrix or order p.
    int p = n;

    if( nct < n )
        S[nct] = B[nct][nct];
    if( m < p )
        S[p-1] = 0;

    if( nrt+1 < p )
        e[nrt] = B[nrt][p-1];
    e[p-1] = 0;

    // if required, generate U
    if( wantu )
    {
        for( j=nct; j<n; ++j )
        {
            for( i=0; i<m; ++i )
                U[i][j] = 0;
            U[j][j] = 1;
        }

        for( k=nct-1; k>=0; --k )
            if( S[k] != 0 )
            {
                for( j=k+1; j<n; ++j )
                {
                    Real t = 0;
                    for( i=k; i<m; ++i )
                        t += U[i][k] * U[i][j];
                    t = -t / U[k][k];

                    for( i=k; i<m; ++i )
                        U[i][j] += t * U[i][k];
                }

                for( i=k; i<m; ++i )
                    U[i][k] = -U[i][k];
                U[k][k] = 1 + U[k][k];

                for( i=0; i<k-1; ++i )
                    U[i][k] = 0;
            }
            else
            {
                for( i=0; i<m; ++i )
                    U[i][k] = 0;
                U[k][k] = 1;
            }
    }

    // if required, generate V
    if( wantv )
        for( k=n-1; k>=0; --k )
        {
            if( (k < nrt) && ( e[k] != 0 ) )
                for( j=k+1; j<n; ++j )
                {
                    Real t = 0;
                    for( i=k+1; i<n; ++i )
                        t += V[i][k] * V[i][j];
                    t = -t / V[k+1][k];

                    for( i=k+1; i<n; ++i )
                        V[i][j] += t * V[i][k];
                }

            for( i=0; i<n; ++i )
                V[i][k] = 0;
            V[k][k] = 1;
        }

    // main iteration loop for the singular values
    int pp = p-1;
    int iter = 0;
    double eps = pow( 2.0, -52.0 );

    while( p > 0 )
    {
        int k = 0;
        int kase = 0;

        // Here is where a test for too many iterations would go.
        // This section of the program inspects for negligible
        // elements in the s and e arrays. On completion the
        // variables kase and k are set as follows.
        // kase = 1     if s(p) and e[k-1] are negligible and k<p
        // kase = 2     if s(k) is negligible and k<p
        // kase = 3     if e[k-1] is negligible, k<p, and
        //				s(k), ..., s(p) are not negligible
        // kase = 4     if e(p-1) is negligible (convergence).
        for( k=p-2; k>=-1; --k )
        {
            if( k == -1 )
                break;

            if( abs(e[k]) <= eps*( abs(S[k])+abs(S[k+1]) ) )
            {
                e[k] = 0;
                break;
            }
        }

        if( k == p-2 )
            kase = 4;
        else
        {
            int ks;
            for( ks=p-1; ks>=k; --ks )
            {
                if( ks == k )
                    break;

                Real t = ( (ks != p) ? abs(e[ks]) : 0 ) +
                         ( (ks != k+1) ? abs(e[ks-1]) : 0 );

                if( abs(S[ks]) <= eps*t )
                {
                    S[ks] = 0;
                    break;
                }
            }

            if( ks == k )
                kase = 3;
            else if( ks == p-1 )
                kase = 1;
            else
            {
                kase = 2;
                k = ks;
            }
        }
        k++;

        // Perform the task indicated by kase.
        switch( kase )
        {
            // deflate negligible s(p)
            case 1:
            {
                Real f = e[p-2];
                e[p-2] = 0;

                for( j=p-2; j>=k; --j )
                {
                    Real t = hypot( S[j], f );
                    Real cs = S[j] / t;
                    Real sn = f / t;
                    S[j] = t;

                    if( j != k )
                    {
                        f = -sn * e[j-1];
                        e[j-1] = cs * e[j-1];
                    }

                    if( wantv )
                        for( i=0; i<n; ++i )
                        {
                            t = cs*V[i][j] + sn*V[i][p-1];
                            V[i][p-1] = -sn*V[i][j] + cs*V[i][p-1];
                            V[i][j] = t;
                        }
                }
            }
            break;

            // split at negligible s(k)
            case 2:
            {
                Real f = e[k-1];
                e[k-1] = 0;

                for( j=k; j<p; ++j )
                {
                    Real t = hypot( S[j], f );
                    Real cs = S[j] / t;
                    Real sn = f / t;
                    S[j] = t;
                    f = -sn * e[j];
                    e[j] = cs * e[j];

                    if( wantu )
                        for( i=0; i<m; ++i )
                        {
                            t = cs*U[i][j] + sn*U[i][k-1];
                            U[i][k-1] = -sn*U[i][j] + cs*U[i][k-1];
                            U[i][j] = t;
                        }
                }
            }
            break;

            // perform one qr step
            case 3:
            {
                // calculate the shift
                Real scale = max( max( max( max(
                             abs(S[p-1]), abs(S[p-2]) ), abs(e[p-2]) ),
                             abs(S[k]) ), abs(e[k]) );
                Real sp = S[p-1] / scale;
                Real spm1 = S[p-2] / scale;
                Real epm1 = e[p-2] / scale;
                Real sk = S[k] / scale;
                Real ek = e[k] / scale;
                Real b = ( (spm1+sp)*(spm1-sp) + epm1*epm1 ) / 2.0;
                Real c = (sp*epm1) * (sp*epm1);
                Real shift = 0;

                if( ( b != 0 ) || ( c != 0 ) )
                {
                    shift = sqrt( b*b+c );
                    if( b < 0 )
                        shift = -shift;
                    shift = c / ( b+shift );
                }
                Real f = (sk+sp)*(sk-sp) + shift;
                Real g = sk * ek;

                // chase zeros
                for( j=k; j<p-1; ++j )
                {
                    Real t = hypot( f, g );
                    Real cs = f / t;
                    Real sn = g / t;
                    if( j != k )
                        e[j-1] = t;

                    f = cs*S[j] + sn*e[j];
                    e[j] = cs*e[j] - sn*S[j];
                    g = sn * S[j+1];
                    S[j+1] = cs * S[j+1];

                    if( wantv )
                        for( i=0; i<n; ++i )
                        {
                            t = cs*V[i][j] + sn*V[i][j+1];
                            V[i][j+1] = -sn*V[i][j] + cs*V[i][j+1];
                            V[i][j] = t;
                        }

                    t = hypot( f, g );
                    cs = f / t;
                    sn = g / t;
                    S[j] = t;
                    f = cs*e[j] + sn*S[j+1];
                    S[j+1] = -sn*e[j] + cs*S[j+1];
                    g = sn * e[j+1];
                    e[j+1] = cs * e[j+1];

                    if( wantu && ( j < m-1 ) )
                        for( i=0; i<m; ++i )
                        {
                            t = cs*U[i][j] + sn*U[i][j+1];
                            U[i][j+1] = -sn*U[i][j] + cs*U[i][j+1];
                            U[i][j] = t;
                        }
                }
                e[p-2] = f;
                iter = iter + 1;
            }
            break;

            // convergence
            case 4:
            {
                // Make the singular values positive.
                if( S[k] <= 0 )
                {
                    S[k] = ( S[k] < 0 ) ? -S[k] : 0;
                    if( wantv )
                        for( i=0; i<=pp; ++i )
                            V[i][k] = -V[i][k];
                }

                // Order the singular values.
                while( k < pp )
                {
                    if( S[k] >= S[k+1] )
                        break;

                    Real t = S[k];
                    S[k] = S[k+1];
                    S[k+1] = t;

                    if( wantv && ( k < n-1 ) )
                        for( i=0; i<n; ++i )
                            swap( V[i][k], V[i][k+1] );

                    if( wantu && ( k < m-1 ) )
                        for( i=0; i<m; ++i )
                            swap( U[i][k], U[i][k+1] );
                    k++;
                }
                iter = 0;
                p--;
            }
            break;
        }
    }
}


/**
 * Get the left singular vectors.
 */
template<typename Real>
inline Matrix<Real> SVD<Real>::getU() const
{
    return U;
}


/**
 * Get the singular values matrix.
 */
template<typename Real>
inline Matrix<Real> SVD<Real>::getSM()
{
    int N = S.size();
    Matrix<Real> tmp( N, N );
    for( int i=0; i<N; ++i )
        tmp[i][i] = S[i];

    return tmp;
}


/**
 * Get the singular values vector.
 */
template<typename Real>
inline Vector<Real> SVD<Real>::getSV() const
{
    return S;
}


/**
 * Get the right singular vectors.
 */
template<typename Real>
inline Matrix<Real> SVD<Real>::getV() const
{
    return V;
}


/**
 * Two norm (max(S)).
 */
template <typename Real>
inline Real SVD<Real>::norm2() const
{
    return S[0];
}


/**
 * Two norm of condition number (max(S)/min(S)).
 */
template <typename Real>
inline Real SVD<Real>::cond() const
{
    return ( S[0] / S(S.size()) );
}


/**
 * Effective numerical matrix rank.
 */
template <typename Real>
int SVD<Real>::rank()
{
    int N = S.size();
    double tol = N * S[0] * EPS;
    int r = 0;

    for( int i=0; i<N; ++i )
        if( S[i] > tol )
            r++;

    return r;
}
