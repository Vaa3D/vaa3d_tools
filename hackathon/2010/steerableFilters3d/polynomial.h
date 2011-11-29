/////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU General Public License         //
// version 2 as published by the Free Software Foundation.             //
//                                                                     //
// This program is distributed in the hope that it will be useful, but //
// WITHOUT ANY WARRANTY; without even the implied warranty of          //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// General Public License for more details.                            //
//                                                                     //
// Written and (C) by German Gonzale and Aurelien Lucchi               //
// Contact <german.gonzale@epfl.ch> or <aurelien.lucchi@gmail.com>     //
// for comments & bug reports                                          //
/////////////////////////////////////////////////////////////////////////

#ifndef POLYNOMIAL_H_
#define POLYNOMIAL_H_

#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

class Polynomial
{

public:

  vector< double > coeffs;

  Polynomial(){
    coeffs.resize(0);
  }

  ~Polynomial(){
    delete &coeffs;
  }

  Polynomial(vector< double > coeffs)
  { this->coeffs = coeffs;}

  Polynomial* derivative(){
    if(coeffs.size() <= 1)
      return new Polynomial();
    vector<double> new_coeffs(coeffs.size()-1);
    for(int i = 0; i < (int)new_coeffs.size(); i++)
      new_coeffs[i] = coeffs[i+1]*(i+1);
    return new Polynomial(new_coeffs);
  }

  Polynomial* add_polynomial(Polynomial* p1)
  {
    Polynomial* p_higher;
    Polynomial* p_lower;
    if(p1->coeffs.size() > this->coeffs.size()){
      p_higher  = p1;
      p_lower   = this;
    } else{
      p_higher  = this;
      p_lower   = p1;
    }
    vector<double> new_coeffs = p_higher->coeffs;
    for(int i = 0; i < (int)p_lower->coeffs.size(); i++)
      new_coeffs[i] += p_lower->coeffs[i];

    return new Polynomial(new_coeffs);
  }

  Polynomial* substract_polynomial(Polynomial* p1)
  {
    Polynomial* p_higher;
    Polynomial* p_lower;
    if(p1->coeffs.size() > this->coeffs.size()){
      p_higher  = p1;
      p_lower   = this;
    } else{
      p_higher  = this;
      p_lower   = p1;
    }
    vector<double> new_coeffs = p_higher->coeffs;
    for(int i = 0; i < (int)p_lower->coeffs.size(); i++)
      new_coeffs[i] -= p_lower->coeffs[i];

    return new Polynomial(new_coeffs);
  }

  Polynomial* multiply_polynomial(Polynomial* p1)
  {
    vector<double> new_coeffs(p1->coeffs.size() + coeffs.size() -1);
    for(int i = 0; i < (int)coeffs.size(); i++)
      for(int j = 0; j < (int)p1->coeffs.size(); j++)
        new_coeffs[i+j] += coeffs[i]*p1->coeffs[j];
    return new Polynomial(new_coeffs);
  }

  void print(){
    for(int i = 0; i < (int)coeffs.size(); i++)
      if(coeffs[i] < 0)
        printf("%.03fx^%i ", coeffs[i],i);
      else
        printf(" %.03fx^%i ", coeffs[i],i);
    printf("\n");
  }

  double evaluate(double x){
    if(coeffs.size()==0)
      return 0;
    double toReturn = coeffs[0];
    double val = x;
    for(int i = 1; i < (int)coeffs.size(); i++){
      toReturn += val*coeffs[i];
      val *= x;
    }
    return toReturn;
  }
};


#endif
