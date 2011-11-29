#ifndef __WAVELETCONFIGEXCEPTION_H__
#define __WAVELETCONFIGEXCEPTION_H__

//! Exception returned when B3 spline wavelets configuration is invalid
/*!
 *
 * @author Nicolas Chenouard nicolas.chenouard@epfl.ch
 * @author Fabrice de Chaumont
 * @author Ihor Smal
 *
 * @version 1.0
 * @date 8/5/2010
 */


#include <stdlib.h>

class WaveletConfigException
{
	public:
	//! message describing why the wavelet configuration is invalid
  	const char* message;
  	//! default constructor such that the exception message is standard
  	/**
  	 * The default message is "Invalid wavelet configuration".
  	 */
  	WaveletConfigException();
  	//! constructor which specifies the exception message
  	/**
  	 * The input argument is the message which should describe why the exception is invalid.
  	 */
  	WaveletConfigException(const char* exceptionMessage);
  	//! Returns the exception description
  	/**
  	 * Explains why the wavelet configuration is invalid.
  	 */
  	const char* what() const throw();
};

#endif
