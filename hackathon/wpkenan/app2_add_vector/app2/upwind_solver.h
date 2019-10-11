#ifndef __UPWIND_SOLVER_H__
#define __UPWIND_SOLVER_H__

#include <cstdlib>
#include <vector>

using namespace std;

// solve (u - U1)^2 + (u - U2)^2 + (u - U3)^2 = p^2
// assume U1 >= U2 >= U3
// return u

#ifndef SWAP
#define SWAP(x,y) {x=x+y;y=x-y;x=x-y;}
#endif

template<class T> T upwind_solver(T u2, T u3, T p)
{
	if(u2 < u3) SWAP(u2,u3);
	
	T a = 2;
	T b = -2 * (u2 + u3);
	T c = u2*u2 + u3*u3 - p*p;
	T delta = b*b - 4*a*c;
	if(delta >= 0.0)
	{
		T x1 = -b/(2*a);
		T x2 = sqrt(delta)/(2*a);
		T u = x2 > 0 ? (x1 + x2):(x1-x2);
		if(u > u2) return u;
		else return (u3 + abs(p));
	}
	else return (u3 + abs(p));
}

template<class T> T upwind_solver(T u1, T u2, T u3, T p)
{
	if(u1 < u2)  SWAP(u1,u2);
	if(u1 < u3) SWAP(u1,u3);
	if(u2 < u3) SWAP (u2,u3);
	
	T a = 3;
	T b = -2 * (u1 + u2 + u3);
	T c = u1*u1 + u2*u2 + u3*u3 - p*p;
	T delta = b*b - 4*a*c;
	if(delta >= 0.0) 
	{
		T x1 = -b /(2*a);
		T x2 = sqrt(delta)/(2*a);
		T u = x2 > 0 ? (x1 + x2):(x1-x2);
		if(u > u1) return u;
		else 
			return upwind_solver(u2,u3,p);
	}
	else 
		return upwind_solver(u2,u3,p);
	
}

template<class T> T upwind_solver(vector<T> &parameters, T p=1.0)
{
	if(parameters.size() == 3) return upwind_solver(parameters[0], parameters[1], parameters[2], p);
	else if(parameters.size() == 2) return upwind_solver(parameters[0], parameters[1], p);
	else if(parameters.size() == 1) return parameters[0] + abs(p);
	else 
	{
		cerr<<"upwind_solver : parameters with zero element"<<endl;
		return 0.0;
	}
}

#endif
