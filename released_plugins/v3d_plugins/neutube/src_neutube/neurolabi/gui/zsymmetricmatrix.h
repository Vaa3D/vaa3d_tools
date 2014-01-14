#ifndef ZSYMMETRICMATRIX_H
#define ZSYMMETRICMATRIX_H

#include "zsquarematrix.h"

class ZSymmetricMatrix : public ZSquareMatrix
{
public:
  enum EDefiniteness {
    POSITIVE_DEFINITE, SEMI_POSITIVE_DEFINITE, NEGATIVE_DEFINITE,
    SEMI_NEGATIVE_DEFINITE, FREE_DEFINITE
  };

public:
  ZSymmetricMatrix();

  inline void setDefiniteness();

public:
  EDefiniteness m_definiteness;
};

#endif // ZSYMMETRICMATIX_H
