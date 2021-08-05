//checkMachineEndian.cpp
//by Hanchuan Peng
//2008-01-04: copy and paste the code in the stackutil.cpp so that everywhere is consistent
//

#include <mex.h>

char checkMachineEndian()
{
    char e='N'; //for unknown endianness
    
    long int a=0x44332211;
    unsigned char * p = (unsigned char *)&a;
    if ((*p==0x11) && (*(p+1)==0x22) && (*(p+2)==0x33) && (*(p+3)==0x44))
        e = 'L';
    else if ((*p==0x44) && (*(p+1)==0x33) && (*(p+2)==0x22) && (*(p+3)==0x11))
        e = 'B';
    else if ((*p==0x22) && (*(p+1)==0x11) && (*(p+2)==0x44) && (*(p+3)==0x33))
        e = 'M';
    else
        e = 'N';
	
    //printf("[%c] \n", e);
    return e;
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if (nrhs>=1) mexErrMsgTxt("Usage: endianCode = checkMachineEndian()");
  
  char output_buf[2]; 
  output_buf[1]='\0';
  output_buf[0]=checkMachineEndian();
  plhs[0] = mxCreateString(output_buf);
}    
