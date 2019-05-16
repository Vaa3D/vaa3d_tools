/* Functions for convolution on 3D data
 * Data is linearly indexed: (x,y,z) -> x + y*nx + z*nx*ny
 * Supported border conditions: mirror
 *
 * (c) Francois Aguet, 08/28/2012 (last modified 08/29/2012)
 * */


// adopted by Liya Ding 2019.05.16

void convolveEvenX(const double input[], const double kernel[], const int k, const int nx, const int ny, const int nz, double output[]) {
    int k_1 = k-1;
    int idx = 0;
    int A,B;
    for (int z=0;z<nz;++z) {
        for (int y=0;y<ny;++y) {
            A = y*nx+z*nx*ny;
            B = (y+1)*nx+z*nx*ny-1;
            // "left" border
            for (int x=0;x<k_1;++x) {
                output[idx] = kernel[0]*input[idx];
                for (int i=1;i<=x;++i) {
                    output[idx] += kernel[i]*(input[idx-i]+input[idx+i]);
                }
                for (int i=x+1;i<k;++i) {
                    output[idx] += kernel[i]*(input[2*A-idx+i]+input[idx+i]);
                }
                idx++;
            }
            for (int x=k_1;x<=nx-k;++x) {
                output[idx] = kernel[0]*input[idx];
                for (int i=1;i<k;++i) {
                    output[idx] += kernel[i]*(input[idx-i]+input[idx+i]);
                }
                idx++;
            }
            // "right" border
            for (int x=nx-k_1;x<nx;++x) {
                output[idx] = kernel[0]*input[idx];
                for (int i=1;i<nx-x;++i) {
                    output[idx] += kernel[i]*(input[idx-i]+input[idx+i]);
                }
                for (int i=nx-x;i<k;++i) {
                    output[idx] += kernel[i]*(input[idx-i]+input[2*B-idx-i]);
                }
                idx++;
            }
        }
    }
}


void convolveOddX(const double input[], const double kernel[], const int k, const int nx, const int ny, const int nz, double output[]) {
    int k_1 = k-1;
    int idx = 0;
    int A,B;
    for (int z=0;z<nz;++z) {
        for (int y=0;y<ny;++y) {
            A = y*nx+z*nx*ny;
            B = (y+1)*nx+z*nx*ny-1;
            // "left" border
            for (int x=0;x<k_1;++x) {
                output[idx] = 0.0;
                for (int i=1;i<=x;++i) {
                    output[idx] += kernel[i]*(input[idx-i]-input[idx+i]);
                }
                for (int i=x+1;i<k;++i) {
                    output[idx] += kernel[i]*(input[2*A-idx+i]-input[idx+i]);
                }
                idx++;
            }
            for (int x=k_1;x<=nx-k;++x) {
                output[idx] = 0.0;
                for (int i=1;i<k;++i) {
                    output[idx] += kernel[i]*(input[idx-i]-input[idx+i]);
                }
                idx++;
            }
            // "right" border
            for (int x=nx-k_1;x<nx;++x) {
                output[idx] = 0.0;
                for (int i=1;i<nx-x;++i) {
                    output[idx] += kernel[i]*(input[idx-i]-input[idx+i]);
                }
                for (int i=nx-x;i<k;++i) {
                    output[idx] += kernel[i]*(input[idx-i]-input[2*B-idx-i]);
                }
                idx++;
            }
        }
    }
}


void convolveEvenY(const double input[], const double kernel[], const int k, const int nx, const int ny, const int nz, double output[]) {
    int k_1 = k-1;
    int idx = 0;
    int A,B;
    for (int z=0;z<nz;++z) {
        for (int x=0;x<nx;++x) {
            A = x + z*nx*ny;
            B = x + (ny-1)*nx + z*nx*ny;
            // "left" border
            for (int y=0;y<k_1;++y) {
                idx = x + y*nx + z*nx*ny;
                output[idx] = kernel[0]*input[idx];
                for (int i=1;i<=y;++i) {
                    output[idx] += kernel[i]*(input[idx-i*nx]+input[idx+i*nx]);
                }
                for (int i=y+1;i<k;++i) {
                    output[idx] += kernel[i]*(input[2*A-idx+i*nx]+input[idx+i*nx]);
                }
            }
            for (int y=k_1;y<=ny-k;++y) {
                idx = x + y*nx + z*nx*ny;
                output[idx] = kernel[0]*input[idx];
                for (int i=1;i<k;++i) {
                    output[idx] += kernel[i]*(input[idx-i*nx]+input[idx+i*nx]);
                }
            }
            // "right" border
            for (int y=ny-k_1;y<ny;++y) {
                idx = x + y*nx + z*nx*ny;
                output[idx] = kernel[0]*input[idx];
                for (int i=1;i<ny-y;++i) {
                    output[idx] += kernel[i]*(input[idx-i*nx]+input[idx+i*nx]);
                }
                for (int i=ny-y;i<k;++i) {
                    output[idx] += kernel[i]*(input[idx-i*nx]+input[2*B-idx-i*nx]);
                }
            }
        }
    }
}


void convolveOddY(const double input[], const double kernel[], const int k, const int nx, const int ny, const int nz, double output[]) {
    int k_1 = k-1;
    int idx = 0;
    int A,B;
    for (int z=0;z<nz;++z) {
        for (int x=0;x<nx;++x) {
            A = x + z*nx*ny;
            B = x + (ny-1)*nx + z*nx*ny;
            // "left" border
            for (int y=0;y<k_1;++y) {
                idx = x + y*nx + z*nx*ny;
                output[idx] = 0.0;
                for (int i=1;i<=y;++i) {
                    output[idx] += kernel[i]*(input[idx-i*nx]-input[idx+i*nx]);
                }
                for (int i=y+1;i<k;++i) {
                    output[idx] += kernel[i]*(input[2*A-idx+i*nx]-input[idx+i*nx]);
                }
            }
            for (int y=k_1;y<=ny-k;++y) {
                idx = x + y*nx + z*nx*ny;
                output[idx] = 0.0;
                for (int i=1;i<k;++i) {
                    output[idx] += kernel[i]*(input[idx-i*nx]-input[idx+i*nx]);
                }
            }
            // "right" border
            for (int y=ny-k_1;y<ny;++y) {
                idx = x + y*nx + z*nx*ny;
                output[idx] = 0.0;
                for (int i=1;i<ny-y;++i) {
                    output[idx] += kernel[i]*(input[idx-i*nx]-input[idx+i*nx]);
                }
                for (int i=ny-y;i<k;++i) {
                    output[idx] += kernel[i]*(input[idx-i*nx]-input[2*B-idx-i*nx]);
                }
            }
        }
    }
}


void convolveEvenZ(const double input[], const double kernel[], const int k, const int nx, const int ny, const int nz, double output[]) {
    int k_1 = k-1;
    int idx = 0;
    int A,B;
    int N = nx*ny;
    for (int y=0;y<ny;++y) {
        for (int x=0;x<nx;++x) {
            A = x + y*nx;
            B = x + y*nx + (nz-1)*nx*ny;
            // "left" border
            for (int z=0;z<k_1;++z) {
                idx = x + y*nx + z*nx*ny;
                output[idx] = kernel[0]*input[idx];
                for (int i=1;i<=z;++i) {
                    output[idx] += kernel[i]*(input[idx-i*N]+input[idx+i*N]);
                }
                for (int i=z+1;i<k;++i) {
                    output[idx] += kernel[i]*(input[2*A-idx+i*N]+input[idx+i*N]);
                }
            }
            for (int z=k_1;z<=nz-k;++z) {
                idx = x + y*nx + z*nx*ny;
                output[idx] = kernel[0]*input[idx];
                for (int i=1;i<k;++i) {
                    output[idx] += kernel[i]*(input[idx-i*N]+input[idx+i*N]);
                }
            }
            // "right" border
            for (int z=nz-k_1;z<nz;++z) {
                idx = x + y*nx + z*nx*ny;
                output[idx] = kernel[0]*input[idx];
                for (int i=1;i<nz-z;++i) {
                    output[idx] += kernel[i]*(input[idx-i*N]+input[idx+i*N]);
                }
                for (int i=nz-z;i<k;++i) {
                    output[idx] += kernel[i]*(input[idx-i*N]+input[2*B-idx-i*N]);
                }
            }
        }
    }
}


void convolveOddZ(const double input[], const double kernel[], const int k, const int nx, const int ny, const int nz, double output[]) {
    int k_1 = k-1;
    int idx = 0;
    int A,B;
    int N = nx*ny;
    for (int y=0;y<ny;++y) {
        for (int x=0;x<nx;++x) {
            A = x + y*nx;
            B = x + y*nx + (nz-1)*nx*ny;
            // "left" border
            for (int z=0;z<k_1;++z) {
                idx = x + y*nx + z*nx*ny;
                output[idx] = 0.0;
                for (int i=1;i<=z;++i) {
                    output[idx] += kernel[i]*(input[idx-i*N]-input[idx+i*N]);
                }
                for (int i=z+1;i<k;++i) {
                    output[idx] += kernel[i]*(input[2*A-idx+i*N]-input[idx+i*N]);
                }
            }
            for (int z=k_1;z<=nz-k;++z) {
                idx = x + y*nx + z*nx*ny;
                output[idx] = 0.0;
                for (int i=1;i<k;++i) {
                    output[idx] += kernel[i]*(input[idx-i*N]-input[idx+i*N]);
                }
            }
            // "right" border
            for (int z=nz-k_1;z<nz;++z) {
                idx = x + y*nx + z*nx*ny;
                output[idx] = 0.0;
                for (int i=1;i<nz-z;++i) {
                    output[idx] += kernel[i]*(input[idx-i*N]-input[idx+i*N]);
                }
                for (int i=nz-z;i<k;++i) {
                    output[idx] += kernel[i]*(input[idx-i*N]-input[2*B-idx-i*N]);
                }
            }
        }
    }
}