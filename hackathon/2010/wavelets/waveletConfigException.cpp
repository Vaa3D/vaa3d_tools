#include "waveletConfigException.h"

WaveletConfigException::WaveletConfigException()
 {
 	message = "Invalid wavelet configuration";
 }
 
WaveletConfigException::WaveletConfigException(const char* exceptionMessage)
{
	message = exceptionMessage;
}

const char* WaveletConfigException::WaveletConfigException::what() const throw()
{
    return message;
}