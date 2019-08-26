/* Functions for convolution on 3D data
 * Data is linearly indexed: (x,y,z) -> x + y*nx + z*nx*ny
 * Supported border conditions: mirror
 *
 * (c) Francois Aguet, 08/28/2012 (last modified 08/29/2012)
 * */

// adopted by Liya Ding 2019.05.16

void convolveEvenX(const double input[], const double kernel[], const int k, const int nx, const int ny, const int nz, double output[]); 
void convolveOddX(const double input[], const double kernel[], const int k, const int nx, const int ny, const int nz, double output[]); 
void convolveEvenY(const double input[], const double kernel[], const int k, const int nx, const int ny, const int nz, double output[]);
void convolveOddY(const double input[], const double kernel[], const int k, const int nx, const int ny, const int nz, double output[]);
void convolveEvenZ(const double input[], const double kernel[], const int k, const int nx, const int ny, const int nz, double output[]);
void convolveOddZ(const double input[], const double kernel[], const int k, const int nx, const int ny, const int nz, double output[]);
