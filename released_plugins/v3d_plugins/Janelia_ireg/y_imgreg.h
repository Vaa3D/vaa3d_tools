/* y_imgreg.h
 * created by Yang Yu, Dec 20, 2011
 * update version 0.5 by Yang Yu, Feb 20, 2012
 * yuy@janelia.hhmi.org
 */

//-------------------------------------------------------------------------------
#ifndef __Y_IMGREG_H__
#define __Y_IMGREG_H__
//-------------------------------------------------------------------------------

//
/// The y_imgreg.h includes
//  1) affine transform matrix file (Insight Transform File V1.0) IO
//  2) image registration class
//

#include "y_img.h"

// image types
typedef Y_IMAGE<REAL, LONG64> Y_IMG_REAL;
typedef Y_IMAGE<unsigned char, LONG64> Y_IMG_UINT8;
typedef Y_IMAGE<unsigned short, LONG64> Y_IMG_UINT16;
typedef Y_MAT<REAL, LONG64> Y_MAT_REAL;

// registraton types
typedef enum { UNKNOWNRT, TRANSLATION, RIGID, AFFINE, NONRIGID } RegistrationType;
typedef enum { UNKNOWNIT, NearestNeighbor, TriLinear, BSpline, ThinPlateSpline } InterpolationType;
typedef enum { UNKNOWNOT, GradientDescent, GaussNewton, LevenbergMarquardt, PowellDogLeg, DownhillSimplex } OptimizationType;
typedef enum { UNKNOWNST, SSD, CC, MI } SimilarityType;
typedef enum { UNKNOWNTD, FORWARD, INVERSE } TransformDirection;
typedef enum { UNKNOWNRS, IMCENTER, MASSCENTER, CENTER, SAMPLE, SAMPLENN, PADPOST, PADPRE } ResizeType;

//-------------------------------------------------------------------------------
/// hierarchical (multiscale) structure
//-------------------------------------------------------------------------------
template <class Tdata, class Tidx>
class Scale
{
public:
    Scale(){sampleratio=NULL; dim=0; iters=0;}
    ~Scale(){} //dim=0; iters=0; y_del<Tdata>(sampleratio);

public:
    void setSampleRatio(Tidx dimension, Tdata *sr)
    {
        dim = dimension;

        y_new<Tdata, Tidx>(sampleratio, dim);

        for(Tidx i=0; i<dim; i++)
        {
            sampleratio[i] = sr[i];
        }
    }

    void setSampleRatio3D(Tdata sx, Tdata sy, Tdata sz)
    {
        dim = 3;

        y_new<Tdata, Tidx>(sampleratio, dim);

        sampleratio[0] = sx;
        sampleratio[1] = sy;
        sampleratio[2] = sz;
    }

    void setIterations(Tidx iterations)
    {
        iters = iterations;
    }

public:
    Tidx dim;
    Tdata *sampleratio;
    Tidx iters;
};

template <class Tdata, class Tidx>
class Pyramid
{
public:
    Pyramid(){}
    ~Pyramid(){}

public:
    void setScales( vector< Scale<Tdata, Tidx> > multipleresolution)
    {
        s=multipleresolution;
    }

public:
    vector< Scale<Tdata, Tidx> > s;
};

//-------------------------------------------------------------------------------
/// registration attributes structure
//-------------------------------------------------------------------------------
typedef struct
{
    RegistrationType   regtype;
    OptimizationType   opttype;
    SimilarityType     smltype;
    InterpolationType  inttype;
    TransformDirection transdir;
}RegistrationOption;

//-------------------------------------------------------------------------------
/// affine transform class
//-------------------------------------------------------------------------------
template <class Tdata, class Tidx>
class Y_AFFINETRANSFORM
{
public:
    Y_AFFINETRANSFORM(){pTransform=NULL; sz=0; dims=0;}
    ~Y_AFFINETRANSFORM(){}

public:

    //
    void clean() {y_del<Tdata>(pTransform); sz=0; dims=0;}

    //
    void setTransform(Tdata* matrix, Tidx dim)
    {
        if(!matrix)
        {
            cout<<"input transform matrix is not valid"<<endl;
            return;
        }

        dims = dim;

        Tidx xdim, ydim;

        switch(dims)
        {
        case 2:
            xdim = ydim = 3;
            break;
        case 3:
            xdim = ydim = 4;
            break;
        default:
            cout<<"dimensions "<<dims<<" not supported"<<endl;
            return;
        }

        sz = xdim*ydim;

        try
        {
            pTransform=new Tdata [sz];

            for(Tidx i=0; i<sz; i++)
            {
                pTransform[i] = matrix[i];
            }
        }
        catch(...)
        {
            printf("Fail to allocate memory.\n");
            y_del<Tdata>(pTransform);
            return;
        }
    }

    //
    void keepRotations()
    {
        if(!pTransform)
        {
            printf("The matrix is null!\n");
            return;
        }

        if(dims==2)
        {
            // for 2d image 3x3
            // 0 1 2
            // 3 4 5
            // 6 7 8

            if(sz<9)
            {
                printf("Dimension is inconsistent!\n");
            }

            pTransform[2]=0;
            pTransform[5]=0;

            pTransform[6]=0;
            pTransform[7]=0;

            pTransform[8]=1;

        }
        else if(dims==3)
        {
            // for 3d image 4x4
            // 0  1  2  3
            // 4  5  6  7
            // 8  9  10 11
            // 12 13 14 15

            if(sz<16)
            {
                printf("Dimension is inconsistent!\n");
            }

            pTransform[15]=1;

            pTransform[3]=0;
            pTransform[7]=0;
            pTransform[11]=0;

            pTransform[12]=0;
            pTransform[13]=0;
            pTransform[14]=0;

        }
        else
        {
            printf("Unsupported dimensions!\n");
            return;
        }

        return;
    }

    void eulerAngles2RotationMatrix(Tdata angleX, Tdata angleY, Tdata angleZ)
    {
        // converts rotation angles (in degrees) into rotation matrix
        // 3D case

        if(dims==3)
        {
            // for 3d image 4x4
            // 0  1  2  3
            // 4  5  6  7
            // 8  9  10 11
            // 12 13 14 15

            if(sz<16)
            {
                printf("Dimension is inconsistent!\n");
            }

            pTransform[15]=1;

            pTransform[3]=0;
            pTransform[7]=0;
            pTransform[11]=0;

            pTransform[12]=0;
            pTransform[13]=0;
            pTransform[14]=0;

            //
            psi = angleX*PI/180;
            theta = angleY*PI/180;
            phi = angleZ*PI/180;

            //
            pTransform[0] = cos(theta)*cos(phi);
            pTransform[1] = sin(psi)*sin(theta)*cos(phi)-cos(psi)*sin(phi);
            pTransform[2] = cos(psi)*sin(theta)*cos(phi)+sin(psi)*sin(phi);

            pTransform[4] = cos(theta)*sin(phi);
            pTransform[5] = sin(psi)*sin(theta)*sin(phi)+cos(psi)*cos(phi);
            pTransform[6] = cos(psi)*sin(theta)*sin(phi)-sin(psi)*cos(phi);

            pTransform[8] = -sin(theta);
            pTransform[9] = sin(psi)*cos(theta);
            pTransform[10] = cos(psi)*cos(theta);

            //
            psi = angleX;
            theta = angleY;
            phi = angleZ;
        }
        else
        {
            cout<<"Not supported"<<endl;
            return;
        }

        //
        return;

    }

    void rotationMatrix2EulerAngles()
    {
        //
        if(!pTransform)
        {
            cout<<"Matrix is NULL"<<endl;
            return;
        }
        else
        {
            if(dims==3)
            {
                // for 3d image 4x4
                // 0  1  2  3
                // 4  5  6  7
                // 8  9  10 11
                // 12 13 14 15

                if(sz<16)
                {
                    printf("Dimension is inconsistent!\n");
                }

                //
                if( y_abs<Tdata>(pTransform[2]) != 1 )
                {
                    Tdata theta1 = -sin(pTransform[2]);
                    Tdata theta2 = PI - theta1;

                    Tdata psi1 = atan2(pTransform[6]/cos(theta1), pTransform[10]/cos(theta1));
                    Tdata psi2 = atan2(pTransform[6]/cos(theta2), pTransform[10]/cos(theta2));
                    Tdata pfi1 = atan2(pTransform[1]/cos(theta1), pTransform[0]/cos(theta1));
                    Tdata pfi2 = atan2(pTransform[1]/cos(theta2), pTransform[0]/cos(theta2));
                    theta = theta1; // could be any one of the two
                    psi = psi1;
                    phi = pfi1;
                }
                else
                {
                    phi = 0;
                    Tdata delta = atan2(pTransform[4], pTransform[8]);

                    if( pTransform[2]==-1 )
                    {
                        theta = PI/2;
                        psi = phi + delta;
                    }
                    else
                    {
                        theta = -PI/2;
                        psi = -phi + delta;
                    }
                }

                // in degrees
                psi = psi*180/PI;
                theta = theta*180/PI;
                phi = phi*180/PI;
            }
            else
            {
                cout<<"Not supported"<<endl;
                return;
            }
        }

        //
        return;
    }

    //
    void initTransform(Tidx dim)
    {
        dims = dim;

        Tidx xdim, ydim;

        switch(dims)
        {
        case 2:
            xdim = ydim = 3;
            break;
        case 3:
            xdim = ydim = 4;
            break;
        default:
            cout<<"dimensions "<<dims<<" not supported"<<endl;
            return;
        }

        sz = xdim*ydim;

        try
        {
            pTransform=new Tdata [sz];

            for(int j=0; j<ydim; j++)
            {
                for(int i=0; i<xdim; i++)
                {
                    if(i==j)
                    {
                        pTransform[j*xdim+i]=1.0;
                    }
                    else
                    {
                        pTransform[j*xdim+i]=0.0;
                    }

                }
            }
        }
        catch(...)
        {
            printf("Fail to allocate memory.\n");
            y_del<Tdata>(pTransform);
            return;
        }
    }

    // affine transform matrix is represented in "Insight Transform File V1.0"
    // 1.   The transform file must have an empty line after the fixed parameters
    // 2.   The Fixed Parameters are the coordinates of the center of rotation
    // 3.   The Parameters are written in the following format:
    //        The first 9 numbers represent a linear transformation matrix in column-major order (where the column index varies the fastest)
    //        The last 3 are a translation.
    // 4.   A transform file may contain multiple transforms. The module will apply all of them.


    std::string trim(std::string const& source, char const* delims = " \t\r\n")
    {
        std::string result(source);
        std::string::size_type index = result.find_last_not_of(delims);
        if(index != std::string::npos)
        {
            result.erase(++index);
        }

        index = result.find_first_not_of(delims);
        if(index != std::string::npos)
        {
            result.erase(0, index);
        }
        else
        {
            result.erase();
        }
        return result;
    }

    bool read(const char* fn)
    {
        filename = const_cast<char *>(fn);

        std::ifstream in;

        in.open (fn, std::ios::in | std::ios::binary);
        if ( in.fail() )
        {
            in.close();
            std::cerr<< "The file could not be opened for read access "<< std::endl << "Filename: " << fn <<endl;
        }

        std::ostringstream InData;

        //
        std::filebuf *pbuf;
        pbuf = in.rdbuf();

        // get file size using buffer's members
        int size = pbuf->pubseekoff (0, std::ios::end, std::ios::in);
        pbuf->pubseekpos (0, std::ios::in);

        // allocate memory to contain file data
        char *buffer = new char[size + 1];

        // get file data
        pbuf->sgetn (buffer, size);
        buffer[size] = '\0';
        InData << buffer;

        y_del<char>(buffer);
        std::string data = InData.str();
        in.close();

        // Read line by line
        std::string::size_type position = 0;

        // check for line end convention
        std::string line_end("\n");

        if ( data.find('\n') == std::string::npos )
        {
            if ( data.find('\r') == std::string::npos )
            {
                std::cerr << "No line ending character found, not a valid ITK Transform TXT file"<<endl;
            }
            line_end = "\r";
        }

        Tidx count;
        while ( position != std::string::npos && position < data.size() )
        {
            // Find the next string
            std::string::size_type end = data.find (line_end, position);

            if( end == std::string::npos )
            {
                std::cerr << "Couldn't find end of line in " << data <<endl;
            }

            std::string line = trim ( data.substr (position, end - position) );
            position = end + 1;

            if ( line.length() == 0 )
            {
                continue;
            }
            if ( line[0] == '#' || std::string::npos == line.find_first_not_of (" \t") )
            {
                // Skip lines beginning with #, or blank lines
                continue;
            }

            // Get the name
            end = line.find (":");
            if ( end == std::string::npos )
            {
                // Throw an error
                std::cerr<< "Tags must be delimited by :" << endl;
            }
            std::string Name = trim ( line.substr (0, end) );
            std::string Value = trim ( line.substr ( end + 1, line.length() ) );

            // Push back
            std::istringstream parse (Value);
            if ( Name == "Transform" )
            {
                string str;
                parse >> str;

                //int last = str.end()-1;
                //dims = atoi(str.at(last));

                string::iterator it=str.end()-1;
                char last = *it;
                const char *clast = reinterpret_cast<const char*>(&last);

                dims = atoi(clast);
                sz = (dims+1)*(dims+1);
                count = sz - dims - 1;
            }
            else if ( Name == "ComponentTransformFile" )
            {
                std::cerr << "Not supported" << endl;
                return false;
            }
            else if ( Name == "Parameters" )
            {
                // e.g. 4x4 affine matrix for 3D images
                // ITK AFFINE MATRIX: a11 a12 a13 a21 a22 a23 a31 a32 a33 t1 t2 t3
                // Y AFFINE MATRIX: a11 a12 a13 a14 (t1); a21 a22 a23 a24 (t2); a31 a32 a33 a34 (t3)
                for(Tidx i=0; i<dims; i++)
                {
                    for(Tidx j=0; j<dims; j++)
                    {
                        parse >> pTransform[i*(dims+1) + j];
                    }
                }
                for(Tidx j=0; j<dims; j++)
                {
                    parse >> pTransform[(j+1)*(dims+1) - 1]; // translation
                }
            }
            else if ( Name == "FixedParameters" )
            {
                for(Tidx i=count; i<sz-1; i++)
                {
                    parse >> pTransform[i];
                }
                pTransform[sz-1] = 1.0;
            }
        }

        //
        return true;
    }

    //
    bool write(const char* fn)
    {
        filename = const_cast<char*>(fn);

        //
        std::ofstream out(fn, std::ios_base::out | std::ios_base::app);

        //
        if(out.is_open())
        {
            out.seekp (0, ios::beg);

            out << "#Insight Transform File V1.0" << std::endl;
            out << "#Transform " << 0 << std::endl;

            string transformType = "AffineTransform_double_";
            switch(dims)
            {
            case 2:
                transformType.append("2_2");
                break;
            case 3:
                transformType.append("3_3");
                break;
            default:
                std::cerr<<"dimensions "<<dims<<" not supported"<<endl;
                return false;
            }
            out << "Transform: " << transformType << std::endl;

            Tidx count = sz - dims - 1;
            out << "Parameters: ";
            for(Tidx i=0; i<dims; i++)
            {
                for(Tidx j=0; j<dims; j++)
                {
                    out << pTransform[i*(dims+1) + j] << " ";
                }
            }
            for(Tidx j=0; j<dims; j++)
            {
                out << pTransform[(j+1)*(dims+1) - 1] << " "; // translation
            }
            out << std::endl;
            out << "FixedParameters: ";
            for(Tidx i=count; i<sz-1; i++)
            {
                out << pTransform[i] << " ";
            }
            out << std::endl << std::endl;

            //
            out.close();
        }
        else
        {
            cout<<"Unable to open the output file"<<endl;
            return false;
        }

        //
        return true;
    }

public:
    Tdata *pTransform;
    Tidx dims; // dims=2(D) 3x3 matrix or dims=3(D) 4x4 matrix
    Tidx sz;
    Tdata psi, theta, phi; // Euler Angles in 3D

    char* filename;
};

typedef Y_AFFINETRANSFORM<REAL, LONG64> Y_AFFINETRANSFORM_REAL;

//-------------------------------------------------------------------------------
/// transform functions
//-------------------------------------------------------------------------------

template <class Y_IMG_D, class Y_MAT_D, class Tidx>
void principalaxistransform(Y_IMG_D pIn, Y_MAT_D &S, Y_MAT_D &V)
{
    //
    // 3D principal axis transform
    //
    // N. M. Alpret, J. F. Bradshaw, D. Kennedy, and J. A. Correia, “The principal axes transformation
    // — A method for image registration,” J. Nucl. Med., vol. 31, pp. 1717–1722, 1990.
    //
    // I1 = V1*I*V1'
    // I2 = V2*I*V2'
    //
    // I2 = V2*V1'*I1*V1*V2'
    //

    //
    if(!pIn.pImg || !pIn.dims)
    {
        cout<<"Invalid inputs for Principal Axis Transform function"<<endl;
        return;
    }

    //
    pIn.getMassCenter();
    REAL Ixx=0, Iyy=0, Izz=0, Ixy=0, Ixz=0, Iyz=0, w=0, dz, dy, dx;
    foreach(pIn.dims[2], z)
    {
        dz = z - pIn.centroid[2];
        foreach(pIn.dims[1], y)
        {
            dy = y - pIn.centroid[1];
            foreach(pIn.dims[0], x)
            {
                dx = x - pIn.centroid[0];

                //
                REAL v = pIn.val4d(0, z, y, x);

                Ixx += v*dx*dx;
                Ixy += v*dx*dy;
                Ixz += v*dx*dz;
                Iyy += v*dy*dy;
                Iyz += v*dy*dz;
                Izz += v*dz*dz;

                w += v;
            }
        }
    }

    Ixx /= w;
    Ixy /= w;
    Ixz /= w;
    Iyy /= w;
    Iyz /= w;
    Izz /= w;

    Y_MAT<REAL, Tidx> Mcov;
    Mcov.init(3, 3, 0);

    Mcov.v[0][0] = Ixx;
    Mcov.v[1][1] = Iyy;
    Mcov.v[2][2] = Izz;

    Mcov.v[0][1] = Mcov.v[1][0] = Ixy;
    Mcov.v[0][2] = Mcov.v[2][0] = Ixz;
    Mcov.v[1][2] = Mcov.v[2][1] = Iyz;

    //
    Mcov.svd(Mcov, S, V);

    //
    return;
}

template <class Tdata, class Tidx>
double scaleEstimating(Tdata *pS, Tdata *pT, Tidx totalplxs)
{
    //
    // estimate scale ratio using volumes
    //
    // inputs: pS and pT are segmented images (background 0, foreground 1)
    //

    if(!pS || !pT)
    {
        cout<<"Invalid inputs for scale estimating function"<<endl;
        return -1;
    }

    // volumes
    double volS=0, volT=0;

    foreach(totalplxs, i)
    {
        //if(pS[i]!=0) volS+=1;
        //if(pT[i]!=0) volT+=1;

        volS += pS[i];
        volT += pT[i];
    }

    printf("test vols ... %lf %lf \n",volS,volT);

    return ( pow(volS/volT, 1.0/3.0) );
}

template <class Y_IMG_S, class Y_IMG_T, class Y_MAT_D>
void absoluteOrientation(Y_IMG_S f, Y_IMG_T g, Y_MAT_D &R)
{
    //
    // 3D absolute orientation
    //
    // Horn, Berthold KP. "Closed-form solution of absolute orientation using unit quaternions." JOSA A 4.4 (1987): 629-642.
    //

    //
    if(!f.pImg || !f.dims || !g.pImg || !g.dims)
    {
        cout<<"Invalid inputs for Absolute Orientation function"<<endl;
        return;
    }

    //
    f.getMassCenter();
    g.getMassCenter();

    qDebug()<<"center of sub"<<f.centroid[0]<<f.centroid[1]<<f.centroid[2];
    qDebug()<<"center of tar"<<g.centroid[0]<<g.centroid[1]<<g.centroid[2];

    //
    REAL Sxx=0, Syy=0, Szz=0, Sxy=0, Syx=0, Sxz=0, Szx=0, Syz=0, Szy=0, vf=0, vg=0, w=0, dzf, dyf, dxf, dzg, dyg, dxg;
    foreach(f.dims[2], z)
    {
        dzf = z - f.centroid[2];
        dzg = z - g.centroid[2];
        foreach(f.dims[1], y)
        {
            dyf = y - f.centroid[1];
            dyg = y - g.centroid[1];
            foreach(f.dims[0], x)
            {
                dxf = x - f.centroid[0];
                dxg = x - g.centroid[0];

                //
                vf = f.val4d(0, z, y, x);
                vg = g.val4d(0, z, y, x);

                //                Sxx += vf*dxf*vg*dxg;
                //                Sxy += vf*dxf*vg*dyg;
                //                Syx += vf*dyf*vg*dxg;
                //                Sxz += vf*dxf*vg*dzg;
                //                Szx += vf*dzf*vg*dxg;
                //                Syy += vf*dyf*vg*dyg;
                //                Syz += vf*dyf*vg*dzg;
                //                Szy += vf*dzf*vg*dyg;
                //                Szz += vf*dzf*vg*dzg;

                //                w += vf*vg;

                Sxx += dxf*dxg*dxf*dxg;
                Sxy += dxf*dyg*dxf*dyg;
                Syx += dyf*dxg*dyf*dxg;
                Sxz += dxf*dzg*dxf*dzg;
                Szx += dzf*dxg*dzf*dxg;
                Syy += dyf*dyg*dyf*dyg;
                Syz += dyf*dzg*dyf*dzg;
                Szy += dzf*dyg*dzf*dyg;
                Szz += dzf*dzg*dzf*dzg;

                w++;
            }
        }
    }
    w += EPS;

    Sxx /= w;
    Sxy /= w;
    Syx /= w;
    Sxz /= w;
    Szx /= w;
    Syy /= w;
    Syz /= w;
    Szy /= w;
    Szz /= w;

    // 3D
    R.init(4,4,0);

    Y_MAT_D S,V;
    S.init(4, 4, 0);
    V.init(4, 4, 0);

    ////
    ////
    ///
    Sxx=-164;
    Sxy=-188;
    Sxz=124;
    Syx=-164;
    Syy=-188;
    Szx=460;
    Szy=724;
    Szz=-20;
    Syz=124;
    ///
    ///
    ///
    ///
    ///

    //
    R.v[0][0] = Sxx + Syy + Szz;
    R.v[1][1] = Sxx - Syy - Szz;
    R.v[2][2] = -Sxx + Syy - Szz;
    R.v[3][3] = -Sxx - Syy + Szz;

    R.v[0][1] = R.v[1][0] = Syz - Szy;
    R.v[0][2] = R.v[2][0] = Szx - Sxz;
    R.v[0][3] = R.v[3][0] = Sxy - Syx;

    R.v[1][2] = R.v[2][1] = Sxy + Syx;
    R.v[1][3] = R.v[3][1] = Szx + Sxz;

    R.v[2][3] = R.v[3][2] = Syz + Szy;


    //R.transpose();

    ///
    ///
    cout<<"R ..."<<endl;
    for(V3DLONG i=0; i<R.column; i++)
    {
        for(V3DLONG j=0; j<R.row; j++)
        {
            cout<<" "<<R.v[i][j];
        }
        cout<<endl;
    }
    cout<<endl;
    ///
    ///
    ///
    ///

    //
    R.svd(R, S, V);


    ///
    ///
    cout<<"V ... pre ..."<<endl;
    for(V3DLONG i=0; i<V.column; i++)
    {
        for(V3DLONG j=0; j<V.row; j++)
        {
            cout<<" "<<V.v[i][j];
        }
        cout<<endl;
    }
    cout<<endl;
    ///
    ///
    cout<<"R ... pre ..."<<endl;
    for(V3DLONG i=0; i<R.column; i++)
    {
        for(V3DLONG j=0; j<R.row; j++)
        {
            cout<<" "<<R.v[i][j];
        }
        cout<<endl;
    }
    cout<<endl;
    ///
    ///
    cout<<"S ... pre ..."<<endl;
    for(V3DLONG i=0; i<S.column; i++)
    {
        for(V3DLONG j=0; j<S.row; j++)
        {
            cout<<" "<<S.v[i][j];
        }
        cout<<endl;
    }
    cout<<endl;
    ///
    ///

    // find unit quaternion
    REAL q0, qx, qy, qz;

    V.sortEigenVectors(S); // sorting in descending order

    REAL sgn = y_sign<REAL>( y_max<REAL> (V.v[0][0], y_max<REAL>(V.v[1][0], y_max<REAL>(V.v[2][0], V.v[3][0]))));

    q0 = sgn * V.v[0][0];
    qx = sgn * V.v[1][0];
    qy = sgn * V.v[2][0];
    qz = sgn * V.v[3][0];

    REAL nrm = sqrt(q0*q0 + qx*qx + qy*qy + qz*qz) + EPS;

    q0 /= nrm;
    qx /= nrm;
    qy /= nrm;
    qz /= nrm;

    Y_MAT_D A,At;
    A.init(3,1,0);
    At.init(1,3,0);

    At.v[0][0] = A.v[0][0] = qx;
    At.v[1][0] = A.v[0][1] = qy;
    At.v[2][0] = A.v[0][2] = qz;

    A.prod(At);


    ///
    ///
    cout<<"A ..."<<endl;
    for(V3DLONG i=0; i<A.column; i++)
    {
        for(V3DLONG j=0; j<A.row; j++)
        {
            cout<<" "<<A.v[i][j];
        }
        cout<<endl;
    }
    cout<<endl;
    ///
    ///
    ///
    ///

    //
    V.clean();
    V.init(3,3,0);

    V.v[0][0] =  q0; V.v[0][1] = -qz; V.v[0][2] =  qy;
    V.v[1][0] =  qz; V.v[1][1] =  q0; V.v[1][2] = -qx;
    V.v[2][0] = -qy; V.v[2][1] =  qx; V.v[2][2] =  q0;

    V.prod(V);

    V.add(A);

    cout<<"V ..."<<endl;
    for(V3DLONG i=0; i<V.column; i++)
    {
        for(V3DLONG j=0; j<V.row; j++)
        {
            cout<<" "<<V.v[i][j];
        }
        cout<<endl;
    }
    cout<<endl;

    //
    R.unit();

    foreach(V.column, j)
        foreach(V.row, i)
            R.v[j][i] = V.v[j][i];

    R.v[0][3] = g.centroid[0] - f.centroid[0];
    R.v[1][3] = g.centroid[1] - f.centroid[1];
    R.v[2][3] = g.centroid[2] - f.centroid[2];

    //de-alloc
    V.clean();
    S.clean();
    A.clean();
    At.clean();

    //
    return;
}


template <class Tdata, class Tidx>
double mutualinfo(Tdata *pS, Tdata *pT, Tidx totalplxs)
{
    //
    // compute the mutual information (max)
    //

    if(!pS || !pT)
    {
        cout<<"Invalid inputs for ssd computation"<<endl;
        return -INF;
    }

    // ???
    //normalizing<Tdata, Tidx>(pS, totalplxs, 0, 255);
    //normalizing<Tdata, Tidx>(pT, totalplxs, 0, 255);

    // joint histogram
    double **jointHistogram = NULL;
    double *img1Hist=NULL;
    double *img2Hist=NULL;

    V3DLONG szHist = 256;
    V3DLONG denomHist = szHist*szHist;

    try
    {
        jointHistogram = new double * [szHist];
        foreach(szHist, i)
        {
            jointHistogram[i] = new double [szHist];
            memset(jointHistogram[i], 0, sizeof(double)*szHist);
        }

        y_new<double, Tidx>(img1Hist, szHist);
        y_new<double, Tidx>(img2Hist, szHist);

        memset(img1Hist, 0, sizeof(double)*szHist);
        memset(img2Hist, 0, sizeof(double)*szHist);
    }
    catch(...)
    {
        cout<<"Fail to allocate memory for joint histogram!"<<endl;
        return -1;
    }

    //
    for(Tidx i=0; i<totalplxs; i++)
    {
        jointHistogram[ (Tidx)pS[i] ][ (Tidx)pT[i] ] ++;
    }

    double jointEntropy=0, img1Entropy=0, img2Entropy=0;

    // normalized joint histogram
    for(Tidx i=0; i<szHist; i++)
    {
        for(Tidx j=0; j<szHist; j++)
        {
            jointHistogram[i][j] /= (double)(denomHist);

            double val = jointHistogram[i][j]?jointHistogram[i][j]:1;

            jointEntropy += val * log2(val);
        }
    }

    // marginal histogram
    for(Tidx i=0; i<szHist; i++)
    {
        for(Tidx j=0; j<szHist; j++)
        {
            img1Hist[i] += jointHistogram[i][j];
            img2Hist[i] += jointHistogram[j][i];
        }
    }

    for(Tidx i=0; i<szHist; i++)
    {
        double val1 = img1Hist[i]?img1Hist[i]:1;
        double val2 = img2Hist[i]?img2Hist[i]:1;

        img1Entropy += val1 * log2(val1);
        img2Entropy += val2 * log2(val2);
    }

    // de-alloc
    y_del2d<double, Tidx>(jointHistogram,szHist);
    y_del<double>(img1Hist);
    y_del<double>(img2Hist);

    // MI
    return (jointEntropy - img1Entropy - img2Entropy);

}

template <class Tdata, class Tidx>
double nmi(Tdata *pS, Tdata *pT, Tidx totalplxs)
{
    //
    // compute the normalized mutual information [0,1] (max)
    //

    // assuming inputs are 8-bit image

    if(!pS || !pT)
    {
        cout<<"Invalid inputs for ssd computation"<<endl;
        return -1;
    }

    // joint histogram
    double **jointHistogram = NULL;
    double *img1Hist=NULL;
    double *img2Hist=NULL;

    V3DLONG szHist = 256;
    V3DLONG denomHist = szHist*szHist;

    try
    {
        jointHistogram = new double * [szHist];
        foreach(szHist, i)
        {
            jointHistogram[i] = new double [szHist];
            memset(jointHistogram[i], 0, sizeof(double)*szHist);
        }

        y_new<double, Tidx>(img1Hist, szHist);
        y_new<double, Tidx>(img2Hist, szHist);

        memset(img1Hist, 0, sizeof(double)*szHist);
        memset(img2Hist, 0, sizeof(double)*szHist);
    }
    catch(...)
    {
        cout<<"Fail to allocate memory for joint histogram!"<<endl;
        return -1;
    }

    //
    for(Tidx i=0; i<totalplxs; i++)
    {
        jointHistogram[ (Tidx)pS[i] ][ (Tidx)pT[i] ] ++;
    }

    double jointEntropy=0, img1Entropy=0, img2Entropy=0;

    // normalized joint histogram
    for(Tidx i=0; i<szHist; i++)
    {
        for(Tidx j=0; j<szHist; j++)
        {
            jointHistogram[i][j] /= (double)(denomHist);

            double val = jointHistogram[i][j]?jointHistogram[i][j]:1;

            jointEntropy += val * log2(val);
        }
    }

    // marginal histogram
    for(Tidx i=0; i<szHist; i++)
    {
        for(Tidx j=0; j<szHist; j++)
        {
            img1Hist[i] += jointHistogram[i][j];
            img2Hist[i] += jointHistogram[j][i];
        }
    }

    for(Tidx i=0; i<szHist; i++)
    {
        double val1 = img1Hist[i]?img1Hist[i]:1;
        double val2 = img2Hist[i]?img2Hist[i]:1;

        img1Entropy += val1 * log2(val1);
        img2Entropy += val2 * log2(val2);
    }

    qDebug()<<"entropy ... "<<img1Entropy<<img1Entropy<<jointEntropy;

    // de-alloc
    y_del2d<double, Tidx>(jointHistogram,szHist);
    y_del<double>(img1Hist);
    y_del<double>(img2Hist);

    // MI
    double mi = (img1Entropy + img2Entropy) - jointEntropy;
    double d = sqrt(img1Entropy * img2Entropy);

    qDebug()<<"mi ... "<<mi<<" ... "<<d;

    // NMI 0.5*[0,2]
    if(d)
    {
        return (0.5*mi/d);
    }
    else
    {
        cout<<"Error in computing sqrt(H(x)H(y))"<<endl;
        return -1;
    }
}

template <class Tdata, class Tidx>
double ssd(Tdata *pS, Tdata *pT, Tidx totalplxs)
{
    //
    // compute the sum of squared differences SSD (min)
    //

    if(!pS || !pT)
    {
        cout<<"Invalid inputs for SSD computation"<<endl;
        return -INF;
    }

    double ssdST=0;
    foreach(totalplxs, i)
    {
        double val = (double)(pS[i]) - (double)(pT[i]);
        ssdST += val*val;
    }

    //
    return ssdST;
}

template <class Tdata, class Tidx>
double ncc(Tdata *pS, Tdata *pT, Tidx totalplxs)
{
    //
    // compute the normalized cross correlation NCC [-1, 1]
    //

    if(!pS || !pT)
    {
        cout<<"Invalid inputs for NCC computation"<<endl;
        return -INF;
    }

    double stdS=0, stdT=0, meanS=0, meanT=0, jointST=0;
    foreach(totalplxs, i)
    {
        meanS += (double)(pS[i]);
        meanT += (double)(pT[i]);
    }
    meanS /= totalplxs;
    meanT /= totalplxs;

    foreach(totalplxs, i)
    {
        double valS = (double)(pS[i]) - meanS;
        double valT = (double)(pT[i]) - meanT;

        jointST += valS*valT;
        stdS += valS*valS;
        stdT += valT*valT;
    }

    stdS = sqrt(stdS);
    stdT = sqrt(stdT);

    // NCC 0.5*([-1,1]+1)
    if(stdS && stdT)
    {
        return (jointST/(stdS*stdT));
    }
    else
    {
        cout<<"Error in computing NCC"<<endl;
        return -INF;
    }
}

template <class Tdata, class Tidx>
double nccwmask(Tdata *pS, Tdata *pT, Tdata *pM, Tidx totalplxs)
{
    //
    // compute the normalized cross correlation NCC [-1, 1]
    //

    if(!pS || !pT)
    {
        cout<<"Invalid inputs for NCC computation"<<endl;
        return -INF;
    }

    double stdS=0, stdT=0, meanS=0, meanT=0, jointST=0, cnt=0;
    foreach(totalplxs, i)
    {
        if(pM[i])
        {
            meanS += (double)(pS[i]);
            meanT += (double)(pT[i]);
            cnt    = cnt+1;
        }
    }
    meanS /= cnt;
    meanT /= cnt;

    cout<<"test ncc ... "<<(Tidx)cnt<<" "<<totalplxs<<" "<<meanS<<" "<<meanT<<endl;

    foreach(totalplxs, i)
    {
        if(pM[i])
        {
            double valS = (double)(pS[i]) - meanS;
            double valT = (double)(pT[i]) - meanT;

            jointST += valS*valT;
            stdS += valS*valS;
            stdT += valT*valT;
        }
    }

    stdS = sqrt(stdS);
    stdT = sqrt(stdT);

    // NCC [-1,1]
    if(stdS && stdT)
    {
        return (jointST/(stdS*stdT));
    }
    else
    {
        cout<<"Error in computing NCC"<<endl;
        return -INF;
    }
}

template <class Tdata, class Tidx, class Y_MAT_D>
int rigidregbf(Y_MAT_D &Mrigid, Tdata *pS, Tdata *pT, Tidx sx, Tidx sy, Tidx sz, REAL theta, REAL thetastep, REAL translate, REAL translatestep)
{
    //
    /// rigid registration using brute force greedy searching
    //

    // theta in degrees

    // output Mrigid is 4x4 affine matrix
    Mrigid.unit();

    // error check
    if(!pT || !pS || sx<=0 || sy<=0 || sz<=0 || theta<0 || thetastep<0 || translate<0 || translatestep<0)
    {
        cout<<"Invalid inputs"<<endl;
        return -1;
    }

    // init
    double mutualinfoST = 0, mimax = -INF;

    //
    Tdata *pWarp = NULL;
    Tidx pagesz = sx*sy;
    Tidx volsz = pagesz*sz;
    y_new<Tdata, Tidx>(pWarp, volsz);

    // Rotation Matrix
    Y_MAT_D Rx, Ry, Rz;
    Rx.init(4,4,1);
    Ry.init(4,4,1);
    Rz.init(4,4,1);

    //
    REAL *index=NULL, *position=NULL;
    y_new<REAL,Tidx>(index,4);
    y_new<REAL,Tidx>(position,4);

    // rotation center
    REAL cz, cy, cx;
    cz = (REAL)(sz-1) / 2.0;
    cy = (REAL)(sy-1) / 2.0;
    cx = (REAL)(sx-1) / 2.0;

    // greedy searching
    Tidx iter = 0;
    size_t start_t = clock();

    REAL tx = 0, ty = 0, tz = 0;

    for(REAL rz=-theta; rz<theta; rz+=thetastep )
    {
        for(REAL ry=-theta; ry<theta; ry+=thetastep )
        {
            for(REAL rx=-theta; rx<theta; rx+=thetastep )
            {
                // for(REAL tz=-translate; tz<translate; tz+=translatestep )
                {
                    //for(REAL ty=-translate; ty<translate; ty+=translatestep )
                    {
                        //for(REAL tx=-translate; tx<translate; tx+=translatestep )
                        {

                            // rotations
                            REAL rotx = rx/180*PI;
                            REAL roty = ry/180*PI;
                            REAL rotz = rz/180*PI;

                            Rx.unit();
                            Ry.unit();
                            Rz.unit();

                            Rx.v[1][1] = cos(rotx);
                            Rx.v[1][2] = sin(rotx);
                            Rx.v[2][1] = -sin(rotx);
                            Rx.v[2][2] = cos(rotx);

                            Ry.v[0][0] = cos(roty);
                            Ry.v[0][2] = sin(roty);
                            Ry.v[2][0] = -sin(roty);
                            Ry.v[2][2] = cos(roty);

                            Rz.v[0][0] = cos(rotz);
                            Rz.v[0][1] = sin(rotz);
                            Rz.v[1][0] = -sin(rotz);
                            Rz.v[1][1] = cos(rotz);

                            //
                            Ry.prod(Rz);
                            Rx.prod(Ry);

                            //
                            /// warp
                            //

                            index[3] = 1.0;
                            position[3] = 1.0;
                            for(Tidx z=0; z<sz; z++)
                            {
                                index[2] = (REAL)z - cz;
                                Tidx offset_z = z*pagesz;
                                for(Tidx y=0; y<sy; y++)
                                {
                                    index[1] = (REAL)y - cy;
                                    Tidx offset_y = offset_z + y*sx;
                                    for(Tidx x=0; x<sx; x++)
                                    {
                                        index[0] = (REAL)x - cx;
                                        Rx.vectorMul(index, position);

                                        Tidx idx = offset_y + x;

                                        REAL proj_x, proj_y, proj_z;

                                        proj_x = position[0]/position[3] + cx + tx; // x
                                        proj_y = position[1]/position[3] + cy + ty; // y
                                        proj_z = position[2]/position[3] + cz + tz; // z

                                        if(proj_x<0 || proj_x>=sx || proj_y<0 || proj_y>=sy || proj_z<0 || proj_z>=sz)
                                        {
                                            pWarp[idx] = 0;
                                            continue;
                                        }

                                        // interpolation
                                        Tidx x_s,x_e,y_s,y_e,z_s,z_e;
                                        x_s=floor(proj_x);  x_e=ceil(proj_x);
                                        y_s=floor(proj_y);  y_e=ceil(proj_y);
                                        z_s=floor(proj_z);  z_e=ceil(proj_z);

                                        x_s=x_s<0?0:x_s;
                                        y_s=y_s<0?0:y_s;
                                        z_s=z_s<0?0:z_s;

                                        x_e=x_e>=sx?sx-1:x_e;
                                        y_e=y_e>=sy?sy-1:y_e;
                                        z_e=z_e>=sz?sz-1:z_e;

                                        REAL l,r,u,d,f,b;
                                        l = 1.0 - (proj_x-(REAL)x_s); r = 1.0 -l;
                                        u = 1.0 - (proj_y-(REAL)y_s); d = 1.0 -u;
                                        f = 1.0 - (proj_z-(REAL)z_s); b = 1.0 -f;

                                        REAL i1, i2, j1, j2, w1, w2;

                                        Tidx zs = z_s*pagesz;
                                        Tidx ze = z_e*pagesz;
                                        Tidx ys = y_s*sx;
                                        Tidx ye = y_e*sx;

                                        i1 = (REAL)(pS[zs + ys + x_s])*f + (REAL)(pS[ze + ys + x_s])*b;
                                        i2 = (REAL)(pS[zs + ye + x_s])*f + (REAL)(pS[ze + ye + x_s])*b;
                                        j1 = (REAL)(pS[zs + ys + x_e])*f + (REAL)(pS[ze + ys + x_e])*b;
                                        j2 = (REAL)(pS[zs + ye + x_e])*f + (REAL)(pS[ze + ye + x_e])*b;

                                        w1 = i1 * u + i2 * d;
                                        w2 = j1 * u + j2 * d;

                                        REAL val = w1 * l + w2 * r;

                                        pWarp[idx]=Tdata(val+0.5);

                                    }
                                }
                            }

                            // mi
                            mutualinfoST = mutualinfo<Tdata, Tidx>(pWarp, pT, volsz);

                            if(mimax < mutualinfoST)
                            {
                                mimax = mutualinfoST;

                                for(Tidx i=0; i<Mrigid.column; i++)
                                {
                                    for(Tidx j=0; j<Mrigid.row; j++)
                                    {
                                        Mrigid.v[i][j] = Rx.v[i][j];
                                    }
                                }
                                Mrigid.v[0][3] = tx;
                                Mrigid.v[1][3] = ty;
                                Mrigid.v[2][3] = tz;
                            }

                            cout<<"iter ... "<<++iter<<" time elapsed ... "<< clock()-start_t <<endl;

                        }
                    }
                }
            }
        }
    }

    //de-alloc
    y_del<Tdata>(pWarp);
    y_del2<REAL, REAL>(index,position);


    //
    return 0;
}

template <class Tdata, class Tidx, class Y_MAT_D>
int rotregbf(Y_MAT_D &Mrigid, Tdata *pS, Tdata *pT, Tidx sx, Tidx sy, Tidx sz, REAL thx, REAL thy, REAL thz, REAL thetastep, REAL translate, REAL translatestep)
{
    //
    /// rigid registration using brute force greedy searching
    //

    // theta in degrees

    // output Mrigid is 4x4 affine matrix
    Mrigid.unit();

    // error check
    if(!pT || !pS || sx<=0 || sy<=0 || sz<=0 || thx<0 || thy<0 || thz<0 || thetastep<0 || translate<0 || translatestep<0)
    {
        cout<<"Invalid inputs"<<endl;
        return -1;
    }

    // init
    double mutualinfoST = 0, mimax = -INF;

    //
    Tdata *pWarp = NULL;
    Tidx pagesz = sx*sy;
    Tidx volsz = pagesz*sz;
    y_new<Tdata, Tidx>(pWarp, volsz);

    // Rotation Matrix
    Y_MAT_D Rx, Ry, Rz;
    Rx.init(4,4,1);
    Ry.init(4,4,1);
    Rz.init(4,4,1);

    //
    REAL *index=NULL, *position=NULL;
    y_new<REAL,Tidx>(index,4);
    y_new<REAL,Tidx>(position,4);

    // rotation center
    REAL cz, cy, cx;
    cz = (REAL)(sz-1) / 2.0;
    cy = (REAL)(sy-1) / 2.0;
    cx = (REAL)(sx-1) / 2.0;

    // greedy searching
    Tidx iter = 0;
    size_t start_t = clock();

    REAL tx = 0, ty = 0, tz = 0;

    for(REAL rz=-thz; rz<thz; rz+=thetastep )
    {
        for(REAL ry=-thy; ry<thy; ry+=thetastep )
        {
            for(REAL rx=-thx; rx<thx; rx+=thetastep )
            {
                // for(REAL tz=-translate; tz<translate; tz+=translatestep )
                {
                    //for(REAL ty=-translate; ty<translate; ty+=translatestep )
                    {
                        //for(REAL tx=-translate; tx<translate; tx+=translatestep )
                        {

                            // rotations
                            REAL rotx = rx/180*PI;
                            REAL roty = ry/180*PI;
                            REAL rotz = rz/180*PI;

                            Rx.unit();
                            Ry.unit();
                            Rz.unit();

                            Rx.v[1][1] = cos(rotx);
                            Rx.v[1][2] = sin(rotx);
                            Rx.v[2][1] = -sin(rotx);
                            Rx.v[2][2] = cos(rotx);

                            Ry.v[0][0] = cos(roty);
                            Ry.v[0][2] = sin(roty);
                            Ry.v[2][0] = -sin(roty);
                            Ry.v[2][2] = cos(roty);

                            Rz.v[0][0] = cos(rotz);
                            Rz.v[0][1] = sin(rotz);
                            Rz.v[1][0] = -sin(rotz);
                            Rz.v[1][1] = cos(rotz);

                            //
                            Ry.prod(Rz);
                            Rx.prod(Ry);

                            //
                            /// warp
                            //

                            index[3] = 1.0;
                            position[3] = 1.0;
                            for(Tidx z=0; z<sz; z++)
                            {
                                index[2] = (REAL)z - cz;
                                Tidx offset_z = z*pagesz;
                                for(Tidx y=0; y<sy; y++)
                                {
                                    index[1] = (REAL)y - cy;
                                    Tidx offset_y = offset_z + y*sx;
                                    for(Tidx x=0; x<sx; x++)
                                    {
                                        index[0] = (REAL)x - cx;
                                        Rx.vectorMul(index, position);

                                        Tidx idx = offset_y + x;

                                        REAL proj_x, proj_y, proj_z;

                                        proj_x = position[0]/position[3] + cx + tx; // x
                                        proj_y = position[1]/position[3] + cy + ty; // y
                                        proj_z = position[2]/position[3] + cz + tz; // z

                                        if(proj_x<0 || proj_x>=sx || proj_y<0 || proj_y>=sy || proj_z<0 || proj_z>=sz)
                                        {
                                            pWarp[idx] = 0;
                                            continue;
                                        }

                                        // interpolation
                                        Tidx x_s,x_e,y_s,y_e,z_s,z_e;
                                        x_s=floor(proj_x);  x_e=ceil(proj_x);
                                        y_s=floor(proj_y);  y_e=ceil(proj_y);
                                        z_s=floor(proj_z);  z_e=ceil(proj_z);

                                        x_s=x_s<0?0:x_s;
                                        y_s=y_s<0?0:y_s;
                                        z_s=z_s<0?0:z_s;

                                        x_e=x_e>=sx?sx-1:x_e;
                                        y_e=y_e>=sy?sy-1:y_e;
                                        z_e=z_e>=sz?sz-1:z_e;

                                        REAL l,r,u,d,f,b;
                                        l = 1.0 - (proj_x-(REAL)x_s); r = 1.0 -l;
                                        u = 1.0 - (proj_y-(REAL)y_s); d = 1.0 -u;
                                        f = 1.0 - (proj_z-(REAL)z_s); b = 1.0 -f;

                                        REAL i1, i2, j1, j2, w1, w2;

                                        Tidx zs = z_s*pagesz;
                                        Tidx ze = z_e*pagesz;
                                        Tidx ys = y_s*sx;
                                        Tidx ye = y_e*sx;

                                        i1 = (REAL)(pS[zs + ys + x_s])*f + (REAL)(pS[ze + ys + x_s])*b;
                                        i2 = (REAL)(pS[zs + ye + x_s])*f + (REAL)(pS[ze + ye + x_s])*b;
                                        j1 = (REAL)(pS[zs + ys + x_e])*f + (REAL)(pS[ze + ys + x_e])*b;
                                        j2 = (REAL)(pS[zs + ye + x_e])*f + (REAL)(pS[ze + ye + x_e])*b;

                                        w1 = i1 * u + i2 * d;
                                        w2 = j1 * u + j2 * d;

                                        REAL val = w1 * l + w2 * r;

                                        pWarp[idx]=Tdata(val+0.5);

                                    }
                                }
                            }

                            // mi
                            mutualinfoST = mutualinfo<Tdata, Tidx>(pWarp, pT, volsz);

                            if(mimax < mutualinfoST)
                            {
                                mimax = mutualinfoST;

                                for(Tidx i=0; i<Mrigid.column; i++)
                                {
                                    for(Tidx j=0; j<Mrigid.row; j++)
                                    {
                                        Mrigid.v[i][j] = Rx.v[i][j];
                                    }
                                }
                                Mrigid.v[0][3] = tx;
                                Mrigid.v[1][3] = ty;
                                Mrigid.v[2][3] = tz;
                            }

                            cout<<"iter ... "<<++iter<<" time elapsed ... "<< clock()-start_t <<endl;

                        }
                    }
                }
            }
        }
    }

    //de-alloc
    y_del<Tdata>(pWarp);
    y_del2<REAL, REAL>(index,position);


    //
    return 0;
}

template <class Tdata, class Tidx, class Y_IMG_D, class Y_MAT_D>
void flipcorrectSSD(Y_IMG_D pS, Y_IMG_D pT, Y_MAT_D &Mflip)
{
    //
    // 3D flip correction
    //

    //
    if(!pS.pImg || !pS.dims || !pT.pImg || !pT.dims)
    {
        cout<<"Invalid inputs for flip correction function"<<endl;
        return;
    }

    //
    double ssdST=0, ssdFSTx=0, ssdFSTy=0, ssdFSTz=0;

    Y_IMG_D pSfliped;
    pSfliped.initImage(pS.dims, 4);

    Mflip.unit();

    ssdST = ssd<Tdata, Tidx>(pS.pImg, pT.pImg, pS.totalplxs); // ssd

    // step 1: find the possible flip from x, y, z
    bool flipfound = false;
    bool flipx=false, flipy=false, flipz=false;

    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
    ssdFSTx = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
    ssdFSTy = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
    ssdFSTz = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

    if(ssdFSTx<ssdST || ssdFSTy<ssdST || ssdFSTz<ssdST)
    {
        flipfound = true;

        if(ssdFSTx<ssdFSTy)
        {
            if(ssdFSTx<ssdFSTz)
            {
                flipx=true;
            }
            else
            {
                flipz=true;
            }
        }
        else
        {
            if(ssdFSTy<ssdFSTz)
            {
                flipy=true;
            }
            else
            {
                flipz=true;
            }
        }
    }

    // step 2: find possible flip from the rest axes
    if(flipfound)
    {
        flipfound=false;

        if(flipx)
        {
            cout<<"flip along x axis"<<endl;

            Mflip.v[0][0] *= -1; // x

            ssdST = ssdFSTx;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
            foreach(pSfliped.totalplxs, i)
                pS.pImg[i] = pSfliped.pImg[i]; // image copy

            ssdFSTy=0;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
            ssdFSTy = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

            ssdFSTz=0;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
            ssdFSTz = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

            //
            if(ssdFSTy<ssdST || ssdFSTz<ssdST)
            {
                flipfound = true;

                if(ssdFSTy<ssdFSTz)
                {
                    flipy=true;
                }
                else
                {
                    flipz=true;
                }
            }

            //
            if(flipfound)
            {
                flipfound=false;

                if(flipy)
                {
                    cout<<"flip along y axis"<<endl;

                    Mflip.v[1][1] *= -1; // y

                    ssdST = ssdFSTy;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
                    foreach(pSfliped.totalplxs, i)
                        pS.pImg[i] = pSfliped.pImg[i]; // image copy

                    ssdFSTz=0;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
                    ssdFSTz = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

                    if(ssdFSTz<ssdST)
                    {
                        flipfound = true;

                        cout<<"flip along z axis"<<endl;
                        flipz = true;
                        Mflip.v[2][2] *= -1; // z

                        ssdST = ssdFSTz;
                        flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
                        foreach(pSfliped.totalplxs, i)
                            pS.pImg[i] = pSfliped.pImg[i]; // image copy
                    }
                }

                if(flipz)
                {
                    cout<<"flip along z axis"<<endl;

                    Mflip.v[2][2] *= -1; // z

                    ssdST = ssdFSTz;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
                    foreach(pSfliped.totalplxs, i)
                        pS.pImg[i] = pSfliped.pImg[i]; // image copy

                    ssdFSTy=0;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
                    ssdFSTy = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

                    if(ssdFSTy<ssdST)
                    {
                        flipfound = true;

                        cout<<"flip along y axis"<<endl;
                        flipy = true;
                        Mflip.v[1][1] *= -1; // y

                        ssdST = ssdFSTy;
                        flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
                        foreach(pSfliped.totalplxs, i)
                            pS.pImg[i] = pSfliped.pImg[i]; // image copy
                    }
                }
            }
        }

        if(flipy)
        {
            cout<<"flip along y axis"<<endl;

            Mflip.v[1][1] *= -1; // y

            ssdST = ssdFSTy;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
            foreach(pSfliped.totalplxs, i)
                pS.pImg[i] = pSfliped.pImg[i]; // image copy

            ssdFSTx=0;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
            ssdFSTx = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

            ssdFSTz=0;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
            ssdFSTz = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

            //
            if(ssdFSTx<ssdST || ssdFSTz<ssdST)
            {
                flipfound = true;

                if(ssdFSTx<ssdFSTz)
                {
                    flipx=true;
                }
                else
                {
                    flipz=true;
                }
            }

            //
            if(flipfound)
            {
                flipfound=false;

                if(flipx)
                {
                    cout<<"flip along x axis"<<endl;

                    Mflip.v[0][0] *= -1; // x

                    ssdST = ssdFSTx;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
                    foreach(pSfliped.totalplxs, i)
                        pS.pImg[i] = pSfliped.pImg[i]; // image copy

                    ssdFSTz=0;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
                    ssdFSTz = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

                    if(ssdFSTz<ssdST)
                    {
                        flipfound = true;

                        cout<<"flip along z axis"<<endl;
                        flipz = true;
                        Mflip.v[2][2] *= -1; // z

                        ssdST = ssdFSTz;
                        flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
                        foreach(pSfliped.totalplxs, i)
                            pS.pImg[i] = pSfliped.pImg[i]; // image copy
                    }
                }

                if(flipz)
                {
                    cout<<"flip along z axis"<<endl;

                    Mflip.v[2][2] *= -1; // z

                    ssdST = ssdFSTz;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
                    foreach(pSfliped.totalplxs, i)
                        pS.pImg[i] = pSfliped.pImg[i]; // image copy

                    ssdFSTx=0;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
                    ssdFSTx = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

                    if(ssdFSTx<ssdST)
                    {
                        flipfound = true;

                        cout<<"flip along x axis"<<endl;
                        flipx = true;
                        Mflip.v[0][0] *= -1; // x

                        ssdST = ssdFSTx;
                        flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
                        foreach(pSfliped.totalplxs, i)
                            pS.pImg[i] = pSfliped.pImg[i]; // image copy
                    }
                }
            }
        }

        if(flipz)
        {
            cout<<"flip along z axis"<<endl;

            Mflip.v[2][2] *= -1; // z

            ssdST = ssdFSTz;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
            foreach(pSfliped.totalplxs, i)
                pS.pImg[i] = pSfliped.pImg[i]; // image copy

            ssdFSTy=0;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
            ssdFSTy = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

            ssdFSTx=0;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
            ssdFSTx = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

            //
            if(ssdFSTy<ssdST || ssdFSTx<ssdST)
            {
                flipfound = true;

                if(ssdFSTy<ssdFSTx)
                {
                    flipy=true;
                }
                else
                {
                    flipx=true;
                }
            }

            //
            if(flipfound)
            {
                flipfound=false;

                if(flipy)
                {
                    cout<<"flip along y axis"<<endl;

                    Mflip.v[1][1] *= -1; // y

                    ssdST = ssdFSTy;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
                    foreach(pSfliped.totalplxs, i)
                        pS.pImg[i] = pSfliped.pImg[i]; // image copy

                    ssdFSTx=0;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
                    ssdFSTx = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

                    if(ssdFSTx<ssdST)
                    {
                        flipfound = true;

                        cout<<"flip along x axis"<<endl;
                        flipz = true;
                        Mflip.v[0][0] *= -1; // x

                        ssdST = ssdFSTx;
                        flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
                        foreach(pSfliped.totalplxs, i)
                            pS.pImg[i] = pSfliped.pImg[i]; // image copy
                    }
                }

                if(flipx)
                {
                    cout<<"flip along x axis"<<endl;

                    Mflip.v[0][0] *= -1; // x

                    ssdST = ssdFSTx;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
                    foreach(pSfliped.totalplxs, i)
                        pS.pImg[i] = pSfliped.pImg[i]; // image copy

                    ssdFSTy=0;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
                    ssdFSTy = ssd<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // ssd

                    if(ssdFSTy<ssdST)
                    {
                        flipfound = true;

                        cout<<"flip along y axis"<<endl;
                        flipy = true;
                        Mflip.v[1][1] *= -1; // y

                        ssdST = ssdFSTy;
                        flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
                        foreach(pSfliped.totalplxs, i)
                            pS.pImg[i] = pSfliped.pImg[i]; // image copy
                    }
                }
            }
        }
    }

    //
    pSfliped.clean();

    //
    return;
}

template <class Tdata, class Tidx, class Y_IMG_D, class Y_MAT_D>
void flipcorrectMI(Y_IMG_D pS, Y_IMG_D pT, Y_MAT_D &Mflip)
{
    //
    // 3D flip correction using MI
    //

    //
    if(!pS.pImg || !pS.dims || !pT.pImg || !pT.dims)
    {
        cout<<"Invalid inputs for flip correction function"<<endl;
        return;
    }

    //
    double mutualinfoST=0, mutualinfoFSTx=0, mutualinfoFSTy=0, mutualinfoFSTz=0;

    Y_IMG_D pSfliped;
    pSfliped.initImage(pS.dims, 4);

    Mflip.unit();

    mutualinfoST = mutualinfo<Tdata, Tidx>(pS.pImg, pT.pImg, pS.totalplxs); // mutualinfo

    // step 1: find the possible flip from x, y, z
    bool flipfound = false;
    bool flipx=false, flipy=false, flipz=false;

    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
    mutualinfoFSTx = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
    mutualinfoFSTy = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
    mutualinfoFSTz = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

    if(mutualinfoFSTx>mutualinfoST || mutualinfoFSTy>mutualinfoST || mutualinfoFSTz>mutualinfoST)
    {
        flipfound = true;

        if(mutualinfoFSTx>mutualinfoFSTy)
        {
            if(mutualinfoFSTx>mutualinfoFSTz)
            {
                flipx=true;
            }
            else
            {
                flipz=true;
            }
        }
        else
        {
            if(mutualinfoFSTy>mutualinfoFSTz)
            {
                flipy=true;
            }
            else
            {
                flipz=true;
            }
        }
    }

    // step 2: find possible flip from the rest axes
    if(flipfound)
    {
        flipfound=false;

        if(flipx)
        {
            cout<<"flip along x axis"<<endl;

            Mflip.v[0][0] *= -1; // x

            mutualinfoST = mutualinfoFSTx;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
            foreach(pSfliped.totalplxs, i)
                pS.pImg[i] = pSfliped.pImg[i]; // image copy

            mutualinfoFSTy=0;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
            mutualinfoFSTy = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

            mutualinfoFSTz=0;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
            mutualinfoFSTz = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

            //
            if(mutualinfoFSTy>mutualinfoST || mutualinfoFSTz>mutualinfoST)
            {
                flipfound = true;

                if(mutualinfoFSTy>mutualinfoFSTz)
                {
                    flipy=true;
                }
                else
                {
                    flipz=true;
                }
            }

            //
            if(flipfound)
            {
                flipfound=false;

                if(flipy)
                {
                    cout<<"flip along y axis"<<endl;

                    Mflip.v[1][1] *= -1; // y

                    mutualinfoST = mutualinfoFSTy;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
                    foreach(pSfliped.totalplxs, i)
                        pS.pImg[i] = pSfliped.pImg[i]; // image copy

                    mutualinfoFSTz=0;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
                    mutualinfoFSTz = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

                    if(mutualinfoFSTz>mutualinfoST)
                    {
                        flipfound = true;

                        cout<<"flip along z axis"<<endl;
                        flipz = true;
                        Mflip.v[2][2] *= -1; // z

                        mutualinfoST = mutualinfoFSTz;
                        flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
                        foreach(pSfliped.totalplxs, i)
                            pS.pImg[i] = pSfliped.pImg[i]; // image copy
                    }
                }

                if(flipz)
                {
                    cout<<"flip along z axis"<<endl;

                    Mflip.v[2][2] *= -1; // z

                    mutualinfoST = mutualinfoFSTz;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
                    foreach(pSfliped.totalplxs, i)
                        pS.pImg[i] = pSfliped.pImg[i]; // image copy

                    mutualinfoFSTy=0;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
                    mutualinfoFSTy = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

                    if(mutualinfoFSTy>mutualinfoST)
                    {
                        flipfound = true;

                        cout<<"flip along y axis"<<endl;
                        flipy = true;
                        Mflip.v[1][1] *= -1; // y

                        mutualinfoST = mutualinfoFSTy;
                        flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
                        foreach(pSfliped.totalplxs, i)
                            pS.pImg[i] = pSfliped.pImg[i]; // image copy
                    }
                }
            }
        }

        if(flipy)
        {
            cout<<"flip along y axis"<<endl;

            Mflip.v[1][1] *= -1; // y

            mutualinfoST = mutualinfoFSTy;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
            foreach(pSfliped.totalplxs, i)
                pS.pImg[i] = pSfliped.pImg[i]; // image copy

            mutualinfoFSTx=0;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
            mutualinfoFSTx = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

            mutualinfoFSTz=0;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
            mutualinfoFSTz = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

            //
            if(mutualinfoFSTx>mutualinfoST || mutualinfoFSTz>mutualinfoST)
            {
                flipfound = true;

                if(mutualinfoFSTx>mutualinfoFSTz)
                {
                    flipx=true;
                }
                else
                {
                    flipz=true;
                }
            }

            //
            if(flipfound)
            {
                flipfound=false;

                if(flipx)
                {
                    cout<<"flip along x axis"<<endl;

                    Mflip.v[0][0] *= -1; // x

                    mutualinfoST = mutualinfoFSTx;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
                    foreach(pSfliped.totalplxs, i)
                        pS.pImg[i] = pSfliped.pImg[i]; // image copy

                    mutualinfoFSTz=0;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
                    mutualinfoFSTz = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

                    if(mutualinfoFSTz>mutualinfoST)
                    {
                        flipfound = true;

                        cout<<"flip along z axis"<<endl;
                        flipz = true;
                        Mflip.v[2][2] *= -1; // z

                        mutualinfoST = mutualinfoFSTz;
                        flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
                        foreach(pSfliped.totalplxs, i)
                            pS.pImg[i] = pSfliped.pImg[i]; // image copy
                    }
                }

                if(flipz)
                {
                    cout<<"flip along z axis"<<endl;

                    Mflip.v[2][2] *= -1; // z

                    mutualinfoST = mutualinfoFSTz;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
                    foreach(pSfliped.totalplxs, i)
                        pS.pImg[i] = pSfliped.pImg[i]; // image copy

                    mutualinfoFSTx=0;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
                    mutualinfoFSTx = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

                    if(mutualinfoFSTx>mutualinfoST)
                    {
                        flipfound = true;

                        cout<<"flip along x axis"<<endl;
                        flipx = true;
                        Mflip.v[0][0] *= -1; // x

                        mutualinfoST = mutualinfoFSTx;
                        flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
                        foreach(pSfliped.totalplxs, i)
                            pS.pImg[i] = pSfliped.pImg[i]; // image copy
                    }
                }
            }
        }

        if(flipz)
        {
            cout<<"flip along z axis"<<endl;

            Mflip.v[2][2] *= -1; // z

            mutualinfoST = mutualinfoFSTz;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AZ); // z
            foreach(pSfliped.totalplxs, i)
                pS.pImg[i] = pSfliped.pImg[i]; // image copy

            mutualinfoFSTy=0;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
            mutualinfoFSTy = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

            mutualinfoFSTx=0;
            flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
            mutualinfoFSTx = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

            //
            if(mutualinfoFSTy>mutualinfoST || mutualinfoFSTx>mutualinfoST)
            {
                flipfound = true;

                if(mutualinfoFSTy>mutualinfoFSTx)
                {
                    flipy=true;
                }
                else
                {
                    flipx=true;
                }
            }

            //
            if(flipfound)
            {
                flipfound=false;

                if(flipy)
                {
                    cout<<"flip along y axis"<<endl;

                    Mflip.v[1][1] *= -1; // y

                    mutualinfoST = mutualinfoFSTy;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
                    foreach(pSfliped.totalplxs, i)
                        pS.pImg[i] = pSfliped.pImg[i]; // image copy

                    mutualinfoFSTx=0;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
                    mutualinfoFSTx = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

                    if(mutualinfoFSTx>mutualinfoST)
                    {
                        flipfound = true;

                        cout<<"flip along x axis"<<endl;
                        flipz = true;
                        Mflip.v[0][0] *= -1; // x

                        mutualinfoST = mutualinfoFSTx;
                        flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
                        foreach(pSfliped.totalplxs, i)
                            pS.pImg[i] = pSfliped.pImg[i]; // image copy
                    }
                }

                if(flipx)
                {
                    cout<<"flip along x axis"<<endl;

                    Mflip.v[0][0] *= -1; // x

                    mutualinfoST = mutualinfoFSTx;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AX); // x
                    foreach(pSfliped.totalplxs, i)
                        pS.pImg[i] = pSfliped.pImg[i]; // image copy

                    mutualinfoFSTy=0;
                    flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
                    mutualinfoFSTy = mutualinfo<Tdata, Tidx>(pSfliped.pImg, pT.pImg, pT.totalplxs); // mutualinfo

                    if(mutualinfoFSTy>mutualinfoST)
                    {
                        flipfound = true;

                        cout<<"flip along y axis"<<endl;
                        flipy = true;
                        Mflip.v[1][1] *= -1; // y

                        mutualinfoST = mutualinfoFSTy;
                        flip<Tdata, Tidx>(pSfliped.pImg, pS.pImg, pS.dims, AY); // y
                        foreach(pSfliped.totalplxs, i)
                            pS.pImg[i] = pSfliped.pImg[i]; // image copy
                    }
                }
            }
        }
    }

    //
    pSfliped.clean();

    //
    return;
}

template <class Tidx, class Y_IMG_IN, class Y_IMG_OUT>
void resize(Y_IMG_OUT &pOut, Y_IMG_IN pIn, ResizeType rstype)
{
    //
    Tidx sx, sy, sz;
    sx = pIn.dims[0];
    sy = pIn.dims[1];
    sz = pIn.dims[2];

    Tidx tx, ty, tz;
    tx = pOut.dims[0];
    ty = pOut.dims[1];
    tz = pOut.dims[2];

    // no need to resize
    //    if(sx==tx && sy==ty && sz==tz)
    //    {
    //        foreach(pIn.totalplxs, i)
    //            pOut.pImg[i] = pIn.pImg[i];
    //        return;
    //    }

    //
    Tidx ofzin = pIn.of2;
    Tidx ofzout = pOut.of2;

    //
    if(rstype == PADPOST)
    {
        for(Tidx k=0; k<sz; k++)
        {
            Tidx offset_k = k*tx*ty;
            Tidx offsets_k = k*sx*sy;
            for(Tidx j=0; j<sy; j++)
            {
                Tidx offset_j = offset_k + j*tx;
                Tidx offsets = offsets_k + j*sx;
                for(Tidx i=0; i<sx; i++)
                {
                    pOut.pImg[offset_j + i] = pIn.pImg[offsets + i];
                }
            }
        }
    }
    else if(rstype == IMCENTER || rstype == MASSCENTER || rstype == CENTER)
    {
        /// recenter
        Tidx centerx, centery, centerz;
        Tidx ncenterx, ncentery, ncenterz;

        if(rstype == IMCENTER)
        {
            centerx = sx/2;
            centery = sy/2;
            centerz = sz/2;

            ncenterx = tx/2;
            ncentery = ty/2;
            ncenterz = tz/2;
        }
        else if(rstype == MASSCENTER)
        {
            pIn.getMassCenter();
            centerx = pIn.centroid[0];
            centery = pIn.centroid[1];
            centerz = pIn.centroid[2];

            pOut.getMassCenter();
            ncenterx = pOut.centroid[0];
            ncentery = pOut.centroid[1];
            ncenterz = pOut.centroid[2];

            pOut.zeros();
        }
        else if(rstype == CENTER)
        {
            pIn.getMassCenter();
            centerx = pIn.centroid[0];
            centery = pIn.centroid[1];
            centerz = pIn.centroid[2];

            ncenterx = tx/2;
            ncentery = ty/2;
            ncenterz = tz/2;

            cout<<"centers : "<<centerx<<" "<<centery<<" "<<centerz<<" -- "<<ncenterx<<" "<<ncentery<<" "<<ncenterz<<endl;
        }

        //shift
        Tidx leftx = fabs(ncenterx-centerx);
        Tidx rightx = fabs(sx + leftx);
        if(sx>tx)
        {
            rightx = fabs(tx + leftx);
            rightx = rightx>sx?sx:rightx;
        }
        else
        {
            rightx = rightx>tx?tx:rightx;
        }

        Tidx lefty = fabs(ncentery - centery);
        Tidx righty = fabs(sy + lefty);
        if(sy>ty)
        {
            righty = fabs(ty + lefty);
            righty = righty>sy?sy:righty;
        }
        else
        {
            righty = righty>ty?ty:righty;
        }

        Tidx leftz = fabs(ncenterz - centerz);
        Tidx rightz = fabs(sz + leftz);
        if(sz>tz)
        {
            rightz = fabs(tz + leftz);
            rightz = rightz>sz?sz:rightz;
        }
        else
        {
            rightz = rightz>tz?tz:rightz;
        }

        //simple 8 cases
        if(tx<=sx)
        {
            if(ty<=sy)
            {
                if(tz<=sz)
                {
                    //case 1
                    for(Tidx k=leftz; k<rightz; k++)
                    {
                        Tidx offsetk =  k*ofzin;
                        Tidx offsetnk = (k-leftz)*ofzout;
                        for(Tidx j=lefty; j<righty; j++)
                        {
                            Tidx offsetj = offsetk + j*sx;
                            Tidx offsetnj = offsetnk + (j-lefty)*tx;
                            for(Tidx i=leftx; i<rightx; i++)
                            {
                                pOut.pImg[offsetnj + (i-leftx)] = pIn.pImg[offsetj + i];
                            }
                        }
                    }

                }
                else
                {
                    //case 2
                    for(Tidx k=leftz; k<rightz; k++)
                    {
                        Tidx offsetk = (k-leftz)*ofzin;
                        Tidx offsetnk = k*ofzout;
                        for(Tidx j=lefty; j<righty; j++)
                        {
                            Tidx offsetj = offsetk + j*sx;
                            Tidx offsetnj = offsetnk + (j-lefty)*tx;
                            for(Tidx i=leftx; i<rightx; i++)
                            {
                                pOut.pImg[offsetnj + (i-leftx)] = pIn.pImg[offsetj + i];
                            }
                        }
                    }
                }
            }
            else
            {
                if(tz<=sz)
                {
                    //case 3
                    for(Tidx k=leftz; k<rightz; k++)
                    {
                        Tidx offsetk = k*ofzin;
                        Tidx offsetnk = (k-leftz)*ofzout;
                        for(Tidx j=lefty; j<righty; j++)
                        {
                            Tidx offsetj = offsetk + (j-lefty)*sx;
                            Tidx offsetnj = offsetnk + j*tx;
                            for(Tidx i=leftx; i<rightx; i++)
                            {
                                pOut.pImg[offsetnj + (i-leftx)] = pIn.pImg[offsetj + i];
                            }
                        }
                    }
                }
                else
                {
                    //case 4
                    for(Tidx k=leftz; k<rightz; k++)
                    {
                        Tidx offsetk = (k-leftz)*ofzin;
                        Tidx offsetnk = k*ofzout;
                        for(Tidx j=lefty; j<righty; j++)
                        {
                            Tidx offsetj = offsetk + (j-lefty)*sx;
                            Tidx offsetnj = offsetnk + j*tx;
                            for(Tidx i=leftx; i<rightx; i++)
                            {
                                pOut.pImg[offsetnj + (i-leftx)] = pIn.pImg[offsetj + i];
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if(ty<=sy)
            {
                if(tz<=sz)
                {
                    //case 5
                    for(Tidx k=leftz; k<rightz; k++)
                    {
                        Tidx offsetk = k*ofzin;
                        Tidx offsetnk = (k-leftz)*ofzout;
                        for(Tidx j=lefty; j<righty; j++)
                        {
                            Tidx offsetj = offsetk + j*sx;
                            Tidx offsetnj = offsetnk + (j-lefty)*tx;
                            for(Tidx i=leftx; i<rightx; i++)
                            {
                                pOut.pImg[offsetnj + i] = pIn.pImg[offsetj + (i-leftx)];
                            }
                        }
                    }
                }
                else
                {
                    //case 6
                    for(Tidx k=leftz; k<rightz; k++)
                    {
                        Tidx offsetk = (k-leftz)*ofzin;
                        Tidx offsetnk = k*ofzout;
                        for(Tidx j=lefty; j<righty; j++)
                        {
                            Tidx offsetj = offsetk + j*sx;
                            Tidx offsetnj = offsetnk + (j-lefty)*tx;
                            for(Tidx i=leftx; i<rightx; i++)
                            {
                                pOut.pImg[offsetnj + i] = pIn.pImg[offsetj + (i-leftx)];
                            }
                        }
                    }
                }
            }
            else
            {
                if(tz<=sz)
                {
                    //case 7
                    for(Tidx k=leftz; k<rightz; k++)
                    {
                        Tidx offsetk = k*ofzin;
                        Tidx offsetnk = (k-leftz)*ofzout;
                        for(Tidx j=lefty; j<righty; j++)
                        {
                            Tidx offsetj = offsetk + (j-lefty)*sx;
                            Tidx offsetnj = offsetnk + j*tx;
                            for(Tidx i=leftx; i<rightx; i++)
                            {
                                pOut.pImg[offsetnj + i] = pIn.pImg[offsetj + (i-leftx)];
                            }
                        }
                    }
                }
                else
                {
                    //case 8
                    for(Tidx k=leftz; k<rightz; k++)
                    {
                        Tidx offsetk = (k-leftz)*ofzin;
                        Tidx offsetnk = k*ofzout;
                        for(Tidx j=lefty; j<righty; j++)
                        {
                            Tidx offsetj = offsetk + (j-lefty)*sx;
                            Tidx offsetnj = offsetnk + j*tx;
                            for(Tidx i=leftx; i<rightx; i++)
                            {
                                pOut.pImg[offsetnj + i] = pIn.pImg[offsetj + (i-leftx)];
                            }
                        }
                    }
                }
            }
        }
    }
    else if(rstype == SAMPLE)
    {
        /// sampling
        // linear interpolation
        REAL scaleratio[3];
        scaleratio[0]=(REAL)(pOut.dims[0])/(REAL)(pIn.dims[0]);
        scaleratio[1]=(REAL)(pOut.dims[1])/(REAL)(pIn.dims[1]);
        scaleratio[2]=(REAL)(pOut.dims[2])/(REAL)(pIn.dims[2]);

        bool b_isint=true;
        if(pOut.dt==4) b_isint = false;

        //        Tdata dt = 1.1;
        //        if(dt-1.0>EPS) b_isint = false;

        //
        for(Tidx z=0;z<pOut.dims[2];z++)
        {
            for(Tidx y=0;y<pOut.dims[1];y++)
            {
                for(Tidx x=0;x<pOut.dims[0];x++)
                {
                    Tidx idx = pOut.idx4d(0,z,y,x);

                    REAL proj_x, proj_y, proj_z;
                    proj_x=x/scaleratio[0];
                    proj_y=y/scaleratio[1];
                    proj_z=z/scaleratio[2];

                    if(y_abs<REAL>(proj_x)<EPS)	proj_x=0.0;
                    if(y_abs<REAL>(proj_y)<EPS)	proj_y=0.0;
                    if(y_abs<REAL>(proj_z)<EPS)	proj_z=0.0;
                    if(proj_x<0 || proj_x>pIn.dims[0]-1 || proj_y<0 || proj_y>pIn.dims[1]-1 || proj_z<0 || proj_z>pIn.dims[2]-1)
                    {
                        pOut.pImg[idx] = 0;
                        continue;
                    }

                    Tidx x_s,x_e,y_s,y_e,z_s,z_e;
                    x_s=floor(proj_x); x_e=ceil(proj_x);
                    y_s=floor(proj_y); y_e=ceil(proj_y);
                    z_s=floor(proj_z); z_e=ceil(proj_z);

                    x_s=x_s<0?0:x_s;
                    y_s=y_s<0?0:y_s;
                    z_s=z_s<0?0:z_s;

                    x_e=x_e>=pIn.dims[0]?pIn.dims[0]-1:x_e;
                    y_e=y_e>=pIn.dims[1]?pIn.dims[1]-1:y_e;
                    z_e=z_e>=pIn.dims[2]?pIn.dims[2]-1:z_e;

                    REAL l,r,u,d,f,b;
                    l = 1.0 - (proj_x-x_s); r = 1.0 -l;
                    u = 1.0 - (proj_y-y_s); d = 1.0 -u;
                    f = 1.0 - (proj_z-z_s); b = 1.0 -f;

                    REAL i1, i2, j1, j2, w1, w2;

                    i1 = (REAL)(pIn.val4d(0, z_s, y_s, x_s)) * f + (REAL)(pIn.val4d(0, z_e, y_s, x_s)) * b;
                    i2 = (REAL)(pIn.val4d(0, z_s, y_e, x_s)) * f + (REAL)(pIn.val4d(0, z_e, y_e, x_s)) * b;
                    j1 = (REAL)(pIn.val4d(0, z_s, y_s, x_e)) * f + (REAL)(pIn.val4d(0, z_e, y_s, x_e)) * b;
                    j2 = (REAL)(pIn.val4d(0, z_s, y_e, x_e)) * f + (REAL)(pIn.val4d(0, z_e, y_e, x_e)) * b;

                    w1 = i1 * u + i2 * d;
                    w2 = j1 * u + j2 * d;

                    REAL val = w1 * l + w2 * r;

                    if(b_isint)
                        pOut.pImg[idx]=val+0.5;
                    else
                        pOut.pImg[idx]=val;
                }
            }
        }
    }
    else if(rstype == SAMPLENN)
    {
        /// sampling
        // nearest neighbor interpolation
        REAL scaleratio[3];
        scaleratio[0]=(REAL)(pOut.dims[0])/(REAL)(pIn.dims[0]);
        scaleratio[1]=(REAL)(pOut.dims[1])/(REAL)(pIn.dims[1]);
        scaleratio[2]=(REAL)(pOut.dims[2])/(REAL)(pIn.dims[2]);

        bool b_isint=true;
        if(pOut.dt==4) b_isint = false;

        //
        for(Tidx z=0;z<pOut.dims[2];z++)
        {
            for(Tidx y=0;y<pOut.dims[1];y++)
            {
                for(Tidx x=0;x<pOut.dims[0];x++)
                {
                    Tidx idx = pOut.idx4d(0,z,y,x);

                    REAL proj_x, proj_y, proj_z;
                    proj_x=x/scaleratio[0];
                    proj_y=y/scaleratio[1];
                    proj_z=z/scaleratio[2];

                    if(y_abs<REAL>(proj_x)<EPS)	proj_x=0.0;
                    if(y_abs<REAL>(proj_y)<EPS)	proj_y=0.0;
                    if(y_abs<REAL>(proj_z)<EPS)	proj_z=0.0;
                    if(proj_x<0 || proj_x>pIn.dims[0]-1 || proj_y<0 || proj_y>pIn.dims[1]-1 || proj_z<0 || proj_z>pIn.dims[2]-1)
                    {
                        pOut.pImg[idx] = 0;
                        continue;
                    }

                    Tidx x_s,x_e,y_s,y_e,z_s,z_e;
                    x_s=floor(proj_x); x_e=ceil(proj_x);
                    y_s=floor(proj_y); y_e=ceil(proj_y);
                    z_s=floor(proj_z); z_e=ceil(proj_z);

                    x_s=x_s<0?0:x_s;
                    y_s=y_s<0?0:y_s;
                    z_s=z_s<0?0:z_s;

                    x_e=x_e>=pIn.dims[0]?pIn.dims[0]-1:x_e;
                    y_e=y_e>=pIn.dims[1]?pIn.dims[1]-1:y_e;
                    z_e=z_e>=pIn.dims[2]?pIn.dims[2]-1:z_e;

                    Tidx xnn, ynn, znn;
                    REAL we, ws;

                    we = (x_e-proj_x);
                    ws = (proj_x-x_s);
                    xnn = (ws<=we) ? x_s : x_e;

                    we = (y_e-proj_y);
                    ws = (proj_y-y_s);
                    ynn = (ws<=we) ? y_s : y_e;

                    we = (z_e-proj_z);
                    ws = (proj_z-z_s);
                    znn = (ws<=we) ? z_s : z_e;

                    if(b_isint)
                        pOut.pImg[idx]=pIn.val4d(0, znn, ynn, xnn)+0.5;
                    else
                        pOut.pImg[idx]=pIn.val4d(0, znn, ynn, xnn);
                }
            }
        }
    }
    else
    {
        cout<<"Invalid resize type"<<endl;
    }

    return;
}

template <class Tidx, class Y_IMG_IN, class Y_IMG_OUT>
void resizeMC(Y_IMG_OUT &pOut, Y_IMG_IN pIn, Tidx refno)
{
    // resize image with multiple color channels
    // input: pIn, reference channel number
    // output: pOut

    //
    Tidx sx, sy, sz, sc;
    sx = pIn.dims[0];
    sy = pIn.dims[1];
    sz = pIn.dims[2];
    sc = pIn.dims[3];

    Tidx tx, ty, tz, tc;
    tx = pOut.dims[0];
    ty = pOut.dims[1];
    tz = pOut.dims[2];
    tc = pOut.dims[3];

    if(sc!=tc)
    {
        cout<<"Color dimensions are not match!"<<endl;
        return;
    }

    // no need to resize
    if(sx==tx && sy==ty && sz==tz)
    {
        foreach(pIn.totalplxs, i)
            pOut.pImg[i] = pIn.pImg[i];

        return;
    }

    //    int signResize = 0;
    //    if(sx<tx)
    //    {
    //        signResize++;
    //    }
    //    else
    //    {
    //        signResize--;
    //    }

    //    if(sy<ty)
    //    {
    //        signResize++;
    //    }
    //    else
    //    {
    //        signResize--;
    //    }

    //    if(sz<tz)
    //    {
    //        signResize++;
    //    }
    //    else
    //    {
    //        signResize--;
    //    }

    //
    Tidx pgszin = pIn.of3;
    Tidx pgszout = pOut.of3;

    Tidx ofzin = pIn.of2;
    Tidx ofzout = pOut.of2;

    //
    /// recenter
    Tidx centerx, centery, centerz;
    Tidx ncenterx, ncentery, ncenterz;

    //    if(signResize>0)
    //    {
    //        pIn.setRefChn(refno);
    //        pIn.getMassCenter();
    //        centerx = (Tidx)(pIn.centroid[0]);
    //        centery = (Tidx)(pIn.centroid[1]);
    //        centerz = (Tidx)(pIn.centroid[2]);
    //    }
    //    else
    //    {
    //        centerx = sx/2;
    //        centery = sy/2;
    //        centerz = sz/2;
    //    }

    centerx = sx/2;
    centery = sy/2;
    centerz = sz/2;

    ncenterx = tx/2;
    ncentery = ty/2;
    ncenterz = tz/2;

    //shift
    Tidx leftx = fabs(ncenterx-centerx);
    Tidx rightx = fabs(sx + leftx);
    if(sx>tx)
    {
        rightx = fabs(tx + leftx);
        rightx = rightx>sx?sx:rightx;
    }
    else
    {
        rightx = rightx>tx?tx:rightx;
    }

    Tidx lefty = fabs(ncentery - centery);
    Tidx righty = fabs(sy + lefty);
    if(sy>ty)
    {
        righty = fabs(ty + lefty);
        righty = righty>sy?sy:righty;
    }
    else
    {
        righty = righty>ty?ty:righty;
    }

    Tidx leftz = fabs(ncenterz - centerz);
    Tidx rightz = fabs(sz + leftz);
    if(sz>tz)
    {
        rightz = fabs(tz + leftz);
        rightz = rightz>sz?sz:rightz;
    }
    else
    {
        rightz = rightz>tz?tz:rightz;
    }

    qDebug()<<"test ... "<<leftx<<rightx<<" ... "<<lefty<<righty<<" ... "<<leftz<<rightz;
    qDebug()<<"test ... "<<sx<<sy<<sz<<" ... "<<tx<<ty<<tz;

    //simple 8 cases
    if(tx<=sx)
    {
        if(ty<=sy)
        {
            if(tz<=sz)
            {
                //case 1
                qDebug()<<"case 1";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk =  k*ofzin;
                    Tidx offsetnk = (k-leftz)*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + j*sx;
                        Tidx offsetnj = offsetnk + (j-lefty)*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + (i-leftx)] = pIn.pImg[c*pgszin + offsetj + i];
                            }
                        }
                    }
                }

            }
            else
            {
                //case 2
                qDebug()<<"case 2";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk = (k-leftz)*ofzin;
                    Tidx offsetnk = k*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + j*sx;
                        Tidx offsetnj = offsetnk + (j-lefty)*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + (i-leftx)] = pIn.pImg[c*pgszin + offsetj + i];
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if(tz<=sz)
            {
                //case 3
                qDebug()<<"case 3";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk = k*ofzin;
                    Tidx offsetnk = (k-leftz)*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + (j-lefty)*sx;
                        Tidx offsetnj = offsetnk + j*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + (i-leftx)] = pIn.pImg[c*pgszin + offsetj + i];
                            }
                        }
                    }
                }
            }
            else
            {
                //case 4
                qDebug()<<"case 4";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk = (k-leftz)*ofzin;
                    Tidx offsetnk = k*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + (j-lefty)*sx;
                        Tidx offsetnj = offsetnk + j*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + (i-leftx)] = pIn.pImg[c*pgszin + offsetj + i];
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if(ty<=sy)
        {
            if(tz<=sz)
            {
                //case 5
                qDebug()<<"case 5";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk = k*ofzin;
                    Tidx offsetnk = (k-leftz)*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + j*sx;
                        Tidx offsetnj = offsetnk + (j-lefty)*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + i] = pIn.pImg[c*pgszin + offsetj + (i-leftx)];
                            }
                        }
                    }
                }
            }
            else
            {
                //case 6
                qDebug()<<"case 6";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk = (k-leftz)*ofzin;
                    Tidx offsetnk = k*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + j*sx;
                        Tidx offsetnj = offsetnk + (j-lefty)*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + i] = pIn.pImg[c*pgszin + offsetj + (i-leftx)];
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if(tz<=sz)
            {
                //case 7
                qDebug()<<"case 7";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk = k*ofzin;
                    Tidx offsetnk = (k-leftz)*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + (j-lefty)*sx;
                        Tidx offsetnj = offsetnk + j*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + i] = pIn.pImg[c*pgszin + offsetj + (i-leftx)];
                            }
                        }
                    }
                }
            }
            else
            {
                //case 8
                qDebug()<<"case 8";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk = (k-leftz)*ofzin;
                    Tidx offsetnk = k*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + (j-lefty)*sx;
                        Tidx offsetnj = offsetnk + j*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + i] = pIn.pImg[c*pgszin + offsetj + (i-leftx)];
                            }
                        }
                    }
                }
            }
        }
    }

    return;
}

template <class Tdata, class Tidx, class Y_IMG_TYPE>
void resizeImage(Y_IMG_TYPE &pOut, Y_IMG_TYPE pIn, int inttype, bool b_skipsampling)
{
    // resize image with multiple color channels

    //
    Tidx sx, sy, sz, sc;
    sx = pIn.dims[0];
    sy = pIn.dims[1];
    sz = pIn.dims[2];
    sc = pIn.dims[3];

    Tidx tx, ty, tz, tc;
    tx = pOut.dims[0];
    ty = pOut.dims[1];
    tz = pOut.dims[2];
    tc = pOut.dims[3];

    if(sc!=tc)
    {
        cout<<"Color dimensions are not match!"<<endl;
        return;
    }

    // no need to resize
    if(sx==tx && sy==ty && sz==tz)
    {
        foreach(pIn.totalplxs, i)
            pOut.pImg[i] = pIn.pImg[i];

        return;
    }

    // downsampling
    if( (sx>tx || sy>ty || sz>tz) && !b_skipsampling)
    {
        REAL dx, dy, dz;
        REAL sampleratio;

        Tidx x,y,z,c, ssx,ssy,ssz, ofz, ofy, idx, volsz;

        REAL bx, ex, by, ey, bz, ez;
        REAL tbx, tex, tby, tey, tbz, tez;

        REAL sum;
        REAL thresh = 1000;

        //
        pIn.getMeanIntensityValue();

        //
        for(c=0; c<sc; c++)
        {
            // yz plane
            tbx = 0;
            tex = sx;
            bool found = false;

            for(x=0; x<sx; x++)
            {
                sum = 0;
                for(y=0; y<sy; y++)
                {
                    for(z=0; z<sz; z++)
                    {
                        if(pIn.val4d(c, z, y, x)>pIn.means[c])
                        {
                            sum++;
                        }
                    }
                }

                if(sum<thresh)
                {
                    if(!found)
                    {
                        tbx = x;
                    }

                    if(x<tex && x>tbx)
                    {
                        tex = x;
                        break;
                    }
                }
                else
                {
                    found = true;
                }
            }

            if(c==0)
            {
                bx = tbx;
                ex = tex;
            }
            else
            {
                if(tex-tbx > ex - bx)
                {
                    bx = tbx;
                    ex = tex;
                }
            }


            // xz plane
            tby = 0;
            tey = sy;
            found = false;

            for(y=0; y<sy; y++)
            {
                sum = 0;
                for(x=0; x<sx; x++)
                {
                    for(z=0; z<sz; z++)
                    {
                        if(pIn.val4d(c, z, y, x)>pIn.means[c])
                        {
                            sum++;
                        }
                    }
                }

                if(sum<thresh)
                {
                    if(!found)
                    {
                        tby = y;
                    }

                    if(y<tey && y>tby)
                    {
                        tey = y;
                        break;
                    }
                }
                else
                {
                    found = true;
                }
            }

            if(c==0)
            {
                by = tby;
                ey = tey;
            }
            else
            {
                if(tey-tby > ey - by)
                {
                    by = tby;
                    ey = tey;
                }
            }

            // xy plane
            tbz = 0;
            tez = sz;
            found = false;

            for(z=0; z<sz; z++)
            {
                sum = 0;
                for(y=0; y<sy; y++)
                {
                    for(x=0; x<sx; x++)
                    {
                        if(pIn.val4d(c, z, y, x)>pIn.means[c])
                        {
                            sum++;
                        }
                    }
                }

                if(sum<thresh)
                {
                    if(!found)
                    {
                        tbz = z;
                    }

                    if(z<tez && z>tbz)
                    {
                        tez = z;
                        break;
                    }
                }
                else
                {
                    found = true;
                }
            }

            if(c==0)
            {
                bz = tbz;
                ez = tez;
            }
            else
            {
                if(tez-tbz > ez - bz)
                {
                    bz = tbz;
                    ez = tez;
                }
            }

            //
            cout<<"test ... "<<c<<" "<<pIn.means[c]<<" "<<ex-bx<<" "<<ey-by<<" "<<ez-bz<<endl;
        }

        dx = REAL(tx)/REAL(ex - bx + 5); // extra blank yz planes
        dy = REAL(ty)/REAL(ey - by + 5);
        dz = REAL(tz)/REAL(ez - bz + 5);

        sampleratio = y_min<REAL>(dz, y_min<REAL>(dx, dy)); // isotropic

        cout<<"downsampling ratio: "<<sampleratio<<" "<<dx<<" "<<dy<<" "<<dz<<endl;

        //
        bool b_isint=true;
        if(pOut.dt==4) b_isint = false;

        ssx = (REAL)sx*sampleratio;
        ssy = (REAL)sy*sampleratio;
        ssz = (REAL)sz*sampleratio;

        volsz = ssx*ssy*ssz;

        Tdata *p = NULL;
        y_new<Tdata,Tidx>(p, ssx*ssy*ssz*sc);

        //
        for(z=0;z<ssz;z++)
        {
            ofz = z*ssx*ssy;
            for(y=0;y<ssy;y++)
            {
                ofy = ofz + y*ssx;
                for( x=0;x<ssx;x++)
                {
                    idx = ofy + x;

                    REAL proj_x, proj_y, proj_z;
                    proj_x=x/sampleratio;
                    proj_y=y/sampleratio;
                    proj_z=z/sampleratio;

                    if(y_abs<REAL>(proj_x)<EPS)	proj_x=0.0;
                    if(y_abs<REAL>(proj_y)<EPS)	proj_y=0.0;
                    if(y_abs<REAL>(proj_z)<EPS)	proj_z=0.0;
                    if(proj_x<0 || proj_x>sx-1 || proj_y<0 || proj_y>sy-1 || proj_z<0 || proj_z>sz-1)
                    {
                        p[idx] = 0;
                        continue;
                    }

                    Tidx x_s,x_e,y_s,y_e,z_s,z_e;
                    x_s=floor(proj_x); x_e=ceil(proj_x);
                    y_s=floor(proj_y); y_e=ceil(proj_y);
                    z_s=floor(proj_z); z_e=ceil(proj_z);

                    x_s=x_s<0?0:x_s;
                    y_s=y_s<0?0:y_s;
                    z_s=z_s<0?0:z_s;

                    x_e=x_e>=sx?sx-1:x_e;
                    y_e=y_e>=sy?sy-1:y_e;
                    z_e=z_e>=sz?sz-1:z_e;

                    for(c=0; c<sc; c++)
                    {
                        if(inttype==1) // nearest neighbor
                        {
                            Tidx xnn, ynn, znn;
                            REAL we, ws;

                            we = (x_e-proj_x);
                            ws = (proj_x-x_s);
                            xnn = (ws<=we) ? x_s : x_e;

                            we = (y_e-proj_y);
                            ws = (proj_y-y_s);
                            ynn = (ws<=we) ? y_s : y_e;

                            we = (z_e-proj_z);
                            ws = (proj_z-z_s);
                            znn = (ws<=we) ? z_s : z_e;

                            p[idx+c*volsz]=pIn.val4d(c, znn, ynn, xnn);
                        }
                        else if(inttype==0) // linear
                        {
                            REAL l,r,u,d,f,b;
                            l = 1.0 - (proj_x-(REAL)x_s); r = 1.0 -l;
                            u = 1.0 - (proj_y-(REAL)y_s); d = 1.0 -u;
                            f = 1.0 - (proj_z-(REAL)z_s); b = 1.0 -f;

                            REAL i1, i2, j1, j2, w1, w2;

                            i1 = (REAL)(pIn.val4d(c, z_s, y_s, x_s)) * f + (REAL)(pIn.val4d(c, z_e, y_s, x_s)) * b;
                            i2 = (REAL)(pIn.val4d(c, z_s, y_e, x_s)) * f + (REAL)(pIn.val4d(c, z_e, y_e, x_s)) * b;
                            j1 = (REAL)(pIn.val4d(c, z_s, y_s, x_e)) * f + (REAL)(pIn.val4d(c, z_e, y_s, x_e)) * b;
                            j2 = (REAL)(pIn.val4d(c, z_s, y_e, x_e)) * f + (REAL)(pIn.val4d(c, z_e, y_e, x_e)) * b;

                            w1 = i1 * u + i2 * d;
                            w2 = j1 * u + j2 * d;

                            REAL val = w1 * l + w2 * r;

                            if(b_isint)
                                p[idx+c*volsz]=val+0.5;
                            else
                                p[idx+c*volsz]=val;
                        }

                    }
                }
            }
        }

        Tidx dt = D4D; // D4D
        Tidx dims[4];

        dims[0] = ssx; dims[1] = ssy; dims[2] = ssz; dims[3] = sc;

        pIn.clean();
        pIn.setImage(p,dims,dt);

        sx = ssx; sy = ssy; sz = ssz;
    }

    //
    Tidx pgszin = pIn.of3;
    Tidx pgszout = pOut.of3;

    Tidx ofzin = pIn.of2;
    Tidx ofzout = pOut.of2;

    //
    /// recenter
    Tidx centerx, centery, centerz;
    Tidx ncenterx, ncentery, ncenterz;

    centerx = sx/2;
    centery = sy/2;
    centerz = sz/2;

    ncenterx = tx/2;
    ncentery = ty/2;
    ncenterz = tz/2;

    //shift
    Tidx leftx = fabs(ncenterx-centerx);
    Tidx rightx = fabs(sx + leftx);
    if(sx>tx)
    {
        rightx = fabs(tx + leftx);
        rightx = rightx>sx?sx:rightx;
    }
    else
    {
        rightx = rightx>tx?tx:rightx;
    }

    Tidx lefty = fabs(ncentery - centery);
    Tidx righty = fabs(sy + lefty);
    if(sy>ty)
    {
        righty = fabs(ty + lefty);
        righty = righty>sy?sy:righty;
    }
    else
    {
        righty = righty>ty?ty:righty;
    }

    Tidx leftz = fabs(ncenterz - centerz);
    Tidx rightz = fabs(sz + leftz);
    if(sz>tz)
    {
        rightz = fabs(tz + leftz);
        rightz = rightz>sz?sz:rightz;
    }
    else
    {
        rightz = rightz>tz?tz:rightz;
    }

    qDebug()<<"test ... "<<leftx<<rightx<<" ... "<<lefty<<righty<<" ... "<<leftz<<rightz;
    qDebug()<<"test ... "<<sx<<sy<<sz<<" ... "<<tx<<ty<<tz;

    //simple 8 cases
    if(tx<=sx)
    {
        if(ty<=sy)
        {
            if(tz<=sz)
            {
                //case 1
                qDebug()<<"case 1";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk =  k*ofzin;
                    Tidx offsetnk = (k-leftz)*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + j*sx;
                        Tidx offsetnj = offsetnk + (j-lefty)*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + (i-leftx)] = pIn.pImg[c*pgszin + offsetj + i];
                            }
                        }
                    }
                }

            }
            else
            {
                //case 2
                qDebug()<<"case 2";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk = (k-leftz)*ofzin;
                    Tidx offsetnk = k*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + j*sx;
                        Tidx offsetnj = offsetnk + (j-lefty)*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + (i-leftx)] = pIn.pImg[c*pgszin + offsetj + i];
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if(tz<=sz)
            {
                //case 3
                qDebug()<<"case 3";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk = k*ofzin;
                    Tidx offsetnk = (k-leftz)*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + (j-lefty)*sx;
                        Tidx offsetnj = offsetnk + j*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + (i-leftx)] = pIn.pImg[c*pgszin + offsetj + i];
                            }
                        }
                    }
                }
            }
            else
            {
                //case 4
                qDebug()<<"case 4";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk = (k-leftz)*ofzin;
                    Tidx offsetnk = k*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + (j-lefty)*sx;
                        Tidx offsetnj = offsetnk + j*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + (i-leftx)] = pIn.pImg[c*pgszin + offsetj + i];
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if(ty<=sy)
        {
            if(tz<=sz)
            {
                //case 5
                qDebug()<<"case 5";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk = k*ofzin;
                    Tidx offsetnk = (k-leftz)*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + j*sx;
                        Tidx offsetnj = offsetnk + (j-lefty)*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + i] = pIn.pImg[c*pgszin + offsetj + (i-leftx)];
                            }
                        }
                    }
                }
            }
            else
            {
                //case 6
                qDebug()<<"case 6";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk = (k-leftz)*ofzin;
                    Tidx offsetnk = k*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + j*sx;
                        Tidx offsetnj = offsetnk + (j-lefty)*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + i] = pIn.pImg[c*pgszin + offsetj + (i-leftx)];
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if(tz<=sz)
            {
                //case 7
                qDebug()<<"case 7";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk = k*ofzin;
                    Tidx offsetnk = (k-leftz)*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + (j-lefty)*sx;
                        Tidx offsetnj = offsetnk + j*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + i] = pIn.pImg[c*pgszin + offsetj + (i-leftx)];
                            }
                        }
                    }
                }
            }
            else
            {
                //case 8
                qDebug()<<"case 8";
                for(Tidx k=leftz; k<rightz; k++)
                {
                    Tidx offsetk = (k-leftz)*ofzin;
                    Tidx offsetnk = k*ofzout;
                    for(Tidx j=lefty; j<righty; j++)
                    {
                        Tidx offsetj = offsetk + (j-lefty)*sx;
                        Tidx offsetnj = offsetnk + j*tx;
                        for(Tidx i=leftx; i<rightx; i++)
                        {
                            for(Tidx c=0; c<tc; c++)
                            {
                                pOut.pImg[c*pgszout + offsetnj + i] = pIn.pImg[c*pgszin + offsetj + (i-leftx)];
                            }
                        }
                    }
                }
            }
        }
    }

    return;
}

// func obtain displacement field from given affine matrix
template <class Tidx, class Y_IMG_IN, class Y_IMG_OUT>
void mat2field(Y_IMG_OUT &pDF, Y_IMG_IN pMAT, RegistrationOption opt)
{
    //
    /// 3d function: matrix -> displacement field
    //

    // Y_MAT nxn -> Y_IMAGE [z][y][x][v]
    if(pDF.dk!=FIELD)
    {
        cout<<"Invalid field input"<<endl;
        return;
    }

    //
    if(opt.inttype==NearestNeighbor)
    {
    }
    else if(opt.inttype==BSpline)
    {
    }
    else if(opt.inttype==ThinPlateSpline)
    {
        // 3D tps warp
        Y_MAT<REAL, Tidx> WA(pMAT.pImg, pMAT.dims[0], pMAT.dims[1]); // Matrix WA3x(m+4)

        // source image's control points
        Tidx m = pMAT.dims[1]-4;
        Y_MAT<REAL, Tidx> M(pDF.pImg+3, pMAT.dims[0], m); // Matrix M3xm

        // L(m+4)x1 matrix
        Y_MAT<REAL, Tidx> L;
        L.init(pMAT.dims[1], 1, 0);

        // dims
        Tidx sz = pDF.dims[3];
        Tidx sy = pDF.dims[2];
        Tidx sx = pDF.dims[1];

        // pDF: displacement field (ZYXV)
        for(Tidx z=0; z<sz; z++)
        {
            for(Tidx y=0; y<sy; y++)
            {
                for(Tidx x=0; x<sx; x++)
                {
                    //
                    Tidx idx = pDF.idx4d(z,y,x,0);

                    //
                    for(Tidx i=0;i<m;i++)
                    {
                        double ss=0, t;
                        t=x-M.v[i][0];  ss += t*t;
                        t=y-M.v[i][1];  ss += t*t;
                        t=z-M.v[i][2];  ss += t*t;

                        L.v[0][i] = tpsRBF<REAL>(ss+EPS);
                    }

                    L.v[0][m] = 1.0;
                    L.v[0][m+1] = x;
                    L.v[0][m+2] = y;
                    L.v[0][m+3] = z;

                    //
                    REAL sum=0;
                    for(Tidx i=0;i<m+4;i++)
                        sum += L.v[0][i]*WA.v[i][0];
                    pDF.pImg[ idx   ] = sum; // x

                    sum = 0;
                    for(Tidx i=0;i<m+4;i++)
                        sum += L.v[0][i]*WA.v[i][1];
                    pDF.pImg[ idx+1 ] = sum; // y

                    sum = 0;
                    for(Tidx i=0;i<m+4;i++)
                        sum += L.v[0][i]*WA.v[i][2];
                    pDF.pImg[ idx+2 ] = sum; // z
                }
            }
        }

        //
        L.clean();

    }
    else if(opt.inttype==TriLinear)
    {
        // 3D rigid or affine transform
        Y_MAT<REAL, Tidx> M(pMAT.pImg, 4, 4);
        Y_MAT<REAL, Tidx> R;
        R.init(4, 4, 1);

        for(Tidx i=0; i<3; i++)
        {
            for(Tidx j=0; j<3; j++)
            {
                R.v[i][j] = M.v[i][j];
            }
        }

        REAL *index=NULL, *position=NULL;
        y_new<REAL,Tidx>(index,4);
        y_new<REAL,Tidx>(position,4);

        // dims
        Tidx sz = pDF.dims[3];
        Tidx sy = pDF.dims[2];
        Tidx sx = pDF.dims[1];

        // scale
        REAL scalex = 1.0, scaley = 1.0, scalez = 1.0;

        // rotation center
        REAL cz, cy, cx;
        if(y_abs<REAL>(M.v[3][2])>1 || y_abs<REAL>(M.v[3][1])>1 || y_abs<REAL>(M.v[3][0])>1)
        {
            cz = (REAL)(M.v[3][2]);
            cy = (REAL)(M.v[3][1]);
            cx = (REAL)(M.v[3][0]);

            scalez = REAL(int(y_abs<REAL>(cz) * 2.0 / sz + 0.5));
            scaley = REAL(int(y_abs<REAL>(cy) * 2.0 / sy + 0.5));
            scalex = REAL(int(y_abs<REAL>(cx) * 2.0 / sx + 0.5));

            cz /= scalez;
            cy /= scaley;
            cx /= scalex;

        }
        else
        {
            cz = (REAL)(sz-1) / 2.0;
            cy = (REAL)(sy-1) / 2.0;
            cx = (REAL)(sx-1) / 2.0;
        }

        // translation
        Tidx tx = M.v[0][3] / scalex;
        Tidx ty = M.v[1][3] / scaley;
        Tidx tz = M.v[2][3] / scalez;

        // displacement field
        index[3] = 1.0;
        position[3] = 1.0;
        for(Tidx z=0; z<sz; z++)
        {
            index[2] = (REAL)z - cz;
            for(Tidx y=0; y<sy; y++)
            {
                index[1] = (REAL)y - cy;
                for(Tidx x=0; x<sx; x++)
                {
                    index[0] = (REAL)x - cx;

                    R.vectorMul(index, position);

                    Tidx idx = pDF.idx4d(z,y,x,0);

                    pDF.pImg[ idx   ] = position[0]/position[3] + cx + tx; // x
                    pDF.pImg[ idx+1 ] = position[1]/position[3] + cy + ty; // y
                    pDF.pImg[ idx+2 ] = position[2]/position[3] + cz + tz; // z
                }
            }
        }

        // de-alloc
        y_del2<REAL, REAL>(index,position);
    }
    else
    {
        cout<<"Your interpolation type is not supported"<<endl;
    }

    //
    return;
}

// func obtain matrix from given fields
template <class Tidx, class Y_IMG_IN, class Y_IMG_OUT>
void field2mat(Y_IMG_OUT &pDF, Y_IMG_IN pMAT)
{
    // convert field to affine matrix
    if(pDF.dk!=FIELD)
    {
    }

    // Y_IMAGE [z][y][x][v] -> Y_MAT [npoints][v]
    // Y_MAT::normalize3DAffine

    return;
}

// func interpolation
template <class Tidx, class Y_IMG_S, class Y_IMG_T, class Y_IMG_D>
void interpolation(Y_IMG_T &pT, Y_IMG_S pS, Y_IMG_D pDF, RegistrationOption opt)
{
    // error checking
    if(pDF.dk!=FIELD)
    {
        cout<<"Invalid field input for interpolation"<<endl;
        return;
    }

    // dims
    Tidx dfx = pDF.dims[1];
    Tidx dfy = pDF.dims[2];
    Tidx dfz = pDF.dims[3];

    Tidx sx = pS.dims[0];
    Tidx sy = pS.dims[1];
    Tidx sz = pS.dims[2];

    //
    Tidx imgdim = (Tidx)pS.dimt;
    for(Tidx i=0; i<imgdim; i++)
    {
        if(pT.dims[i]!=pS.dims[i])
        {
            cout<<"Inconsistent dimensions of the inputs for interpolation"<<endl;
            return;
        }
    }

    // scale ratio of df to image: this permits the df be subsampled
    REAL srx, sry, srz;
    srx=(REAL)(dfx)/(REAL)(sx);
    sry=(REAL)(dfy)/(REAL)(sy);
    srz=(REAL)(dfz)/(REAL)(sz);

    bool b_isint = true;
    if(pT.dt==4) b_isint = false; // float type

    // assuming pS.dims = pT.dims
    for(Tidx z=0;z<sz;z++)
    {
        for(Tidx y=0;y<sy;y++)
        {
            for(Tidx x=0;x<sx;x++)
            {
                Tidx idx = pS.idx4d(0,z,y,x);

                // df coordinates
                Tidx xx = round( double(x)*srx );
                Tidx yy = round( double(y)*sry );
                Tidx zz = round( double(z)*srz );

                xx--;
                if(xx<0) xx=0;
                else if(xx>=dfx) xx=dfx-1;

                yy--;
                if(yy<0) yy=0;
                else if(yy>=dfy) yy=dfy-1;

                zz--;
                if(zz<0) zz=0;
                else if(zz>=dfz) zz=dfz-1;

                // warped coordinates
                REAL proj_x, proj_y, proj_z;

                proj_x=(REAL)(pDF.val4d(zz,yy,xx,0))/srx + x;
                proj_y=(REAL)(pDF.val4d(zz,yy,xx,1))/sry + y;
                proj_z=(REAL)(pDF.val4d(zz,yy,xx,2))/srz + z;

                if(y_abs<REAL>(proj_x)<EPS)	proj_x=0.0;
                if(y_abs<REAL>(proj_y)<EPS)	proj_y=0.0;
                if(y_abs<REAL>(proj_z)<EPS)	proj_z=0.0;
                if(proj_x<0 || proj_x>pS.dims[0]-1 || proj_y<0 || proj_y>pS.dims[1]-1 || proj_z<0 || proj_z>pS.dims[2]-1)
                {
                    pT.pImg[idx] = 0;
                    continue;
                }

                Tidx x_s,x_e,y_s,y_e,z_s,z_e;
                x_s=floor(proj_x);  x_e=ceil(proj_x);
                y_s=floor(proj_y);  y_e=ceil(proj_y);
                z_s=floor(proj_z);  z_e=ceil(proj_z);

                x_s=x_s<0?0:x_s;
                y_s=y_s<0?0:y_s;
                z_s=z_s<0?0:z_s;

                x_e=x_e>=sx?sx-1:x_e;
                y_e=y_e>=sy?sy-1:y_e;
                z_e=z_e>=sz?sz-1:z_e;

                // interpolation
                if(opt.inttype==NearestNeighbor)
                {
                    Tidx xnn, ynn, znn;
                    REAL we, ws;

                    we = (x_e-proj_x);
                    ws = (proj_x-x_s);
                    xnn = (ws<=we) ? x_s : x_e;

                    we = (y_e-proj_y);
                    ws = (proj_y-y_s);
                    ynn = (ws<=we) ? y_s : y_e;

                    we = (z_e-proj_z);
                    ws = (proj_z-z_s);
                    znn = (ws<=we) ? z_s : z_e;

                    pT.pImg[idx]=pS.val4d(0, znn, ynn, xnn);
                }
                else if(opt.inttype==TriLinear)
                {
                    REAL l,r,u,d,f,b;
                    l = 1.0 - (proj_x-(REAL)x_s); r = 1.0 -l;
                    u = 1.0 - (proj_y-(REAL)y_s); d = 1.0 -u;
                    f = 1.0 - (proj_z-(REAL)z_s); b = 1.0 -f;

                    REAL i1, i2, j1, j2, w1, w2;

                    i1 = (REAL)(pS.val4d(0, z_s, y_s, x_s)) * f + (REAL)(pS.val4d(0, z_e, y_s, x_s)) * b;
                    i2 = (REAL)(pS.val4d(0, z_s, y_e, x_s)) * f + (REAL)(pS.val4d(0, z_e, y_e, x_s)) * b;
                    j1 = (REAL)(pS.val4d(0, z_s, y_s, x_e)) * f + (REAL)(pS.val4d(0, z_e, y_s, x_e)) * b;
                    j2 = (REAL)(pS.val4d(0, z_s, y_e, x_e)) * f + (REAL)(pS.val4d(0, z_e, y_e, x_e)) * b;

                    w1 = i1 * u + i2 * d;
                    w2 = j1 * u + j2 * d;

                    REAL val = w1 * l + w2 * r;

                    if(b_isint)
                        pT.pImg[idx]=val+0.5;
                    else
                        pT.pImg[idx]=val;
                }
            } // x
        }// y
    }// z

    return;
}

// func warp
template <class Tidx, class Y_IMG_S, class Y_IMG_T, class Y_IMG_D>
void warp(Y_IMG_T &pT, Y_IMG_S pS, Y_IMG_D pDF, RegistrationOption opt)
{

    // resize the source to the template
    Y_IMG_S pSR;
    if(pS.dims[0]!=pT.dims[0] || pS.dims[1]!=pT.dims[1] || pS.dims[2]!=pT.dims[2])
    {
        pSR.initImage(pT.dims, 4);
        resize<Tidx, Y_IMG_S, Y_IMG_S>(pSR, pS, IMCENTER);

        //for(Tidx i=0; i<pT.totalplxs; i++) pSR.pImg[i] = pT.pImg[i];
        //resize<Tidx, Y_IMG_S, Y_IMG_S>(pSR, pS, MASSCENTER); // match centers of mass (optional)
    }
    else
    {
        pSR.setImage(pS.pImg, pS.dims, 4);
    }

    // image warping
    if(opt.regtype == TRANSLATION)
    {

    }
    else if(opt.regtype == RIGID || opt.regtype == AFFINE)
    {
        //
        /// transform
        //

        bool b_fastTransform = false; // large-scale image affine transform

        //if(opt.inttype==NearestNeighbor) b_fastTransform = true;

        if(b_fastTransform)
        {
            // displacement field
            Y_IMG_D df;

            Tidx *szdf=NULL;
            y_new<Tidx, Tidx>(szdf, 4);

            szdf[0] = 3;
            szdf[1] = pT.dims[0];
            szdf[2] = pT.dims[1];
            szdf[3] = pT.dims[2];

            df.initField(szdf, 4); //

            // in this case, pDF is an affine matrix
            mat2field<Tidx, Y_IMG_D, Y_IMG_D>(df, pDF, opt);

            //
            interpolation<Tidx, Y_IMG_S, Y_IMG_T, Y_IMG_D>(pT, pSR, df, opt);

            // de-alloc
            y_del<Tidx>(szdf);
            df.clean();
        }
        else
        {
            // memory-efficient affine transform
            Y_MAT<REAL, Tidx> M(pDF.pImg, 4, 4);

            // rotation
            Y_MAT<REAL, Tidx> R;
            R.init(4, 4, 1);

            for(Tidx i=0; i<3; i++)
            {
                for(Tidx j=0; j<3; j++)
                {
                    R.v[i][j] = M.v[i][j];
                }
            }

            REAL *index=NULL, *position=NULL;
            y_new<REAL,Tidx>(index,4);
            y_new<REAL,Tidx>(position,4);

            // dims
            Tidx sz = pT.dims[2];
            Tidx sy = pT.dims[1];
            Tidx sx = pT.dims[0];

            // scale
            REAL scalex = 1.0, scaley = 1.0, scalez = 1.0;

            // rotation center
            REAL cz, cy, cx;
            if(y_abs<REAL>(M.v[3][2])>1 || y_abs<REAL>(M.v[3][1])>1 || y_abs<REAL>(M.v[3][0])>1)
            {
                cz = (REAL)(M.v[3][2]);
                cy = (REAL)(M.v[3][1]);
                cx = (REAL)(M.v[3][0]);

                // assume the cx <= sx/2
                scalez = REAL( 1.0 / round((double) sz / (y_abs<double>(cz) * 2.0) ) );
                scaley = REAL( 1.0 / round((double) sy / (y_abs<double>(cy) * 2.0) ) );
                scalex = REAL( 1.0 / round((double) sx / (y_abs<double>(cx) * 2.0) ) );

                cz /= scalez;
                cy /= scaley;
                cx /= scalex;

            }
            else
            {
                cz = (REAL)(sz-1) / 2.0;
                cy = (REAL)(sy-1) / 2.0;
                cx = (REAL)(sx-1) / 2.0;
            }

            // translation
            Tidx tx = M.v[0][3] / scalex;
            Tidx ty = M.v[1][3] / scaley;
            Tidx tz = M.v[2][3] / scalez;

            // displacement field
            bool b_isint = true;
            if(pT.dt==4) b_isint = false; // float type

            index[3] = 1.0;
            position[3] = 1.0;
            for(Tidx z=0; z<sz; z++)
            {
                index[2] = (REAL)z - cz;
                for(Tidx y=0; y<sy; y++)
                {
                    index[1] = (REAL)y - cy;
                    for(Tidx x=0; x<sx; x++)
                    {
                        index[0] = (REAL)x - cx;

                        R.vectorMul(index, position);

                        //qDebug()<<"position ... "<<position[0]<<position[1]<<position[2]<<position[3];

                        Tidx idx = pT.idx4d(0,z,y,x);

                        //qDebug()<<"idx ..."<<x<<y<<z<<idx;

                        REAL proj_x, proj_y, proj_z;

                        proj_x = position[0]/position[3] + cx + tx; // x
                        proj_y = position[1]/position[3] + cy + ty; // y
                        proj_z = position[2]/position[3] + cz + tz; // z

                        //qDebug()<<"proj ... "<<proj_x<<sx<<proj_y<<sy<<proj_z<<sz;

                        if(proj_x<0 || proj_x>pSR.dims[0]-1 || proj_y<0 || proj_y>pSR.dims[1]-1 || proj_z<0 || proj_z>pSR.dims[2]-1)
                        {
                            pT.pImg[idx] = 0;
                            continue;
                        }

                        Tidx x_s,x_e,y_s,y_e,z_s,z_e;
                        x_s=floor(proj_x);  x_e=ceil(proj_x);
                        y_s=floor(proj_y);  y_e=ceil(proj_y);
                        z_s=floor(proj_z);  z_e=ceil(proj_z);

                        x_s=x_s<0?0:x_s;
                        y_s=y_s<0?0:y_s;
                        z_s=z_s<0?0:z_s;

                        x_e=x_e>=sx?sx-1:x_e;
                        y_e=y_e>=sy?sy-1:y_e;
                        z_e=z_e>=sz?sz-1:z_e;

                        // interpolation
                        if(opt.inttype==NearestNeighbor)
                        {
                            Tidx xnn, ynn, znn;
                            REAL we, ws;

                            we = (x_e-proj_x);
                            ws = (proj_x-x_s);
                            xnn = (ws<=we) ? x_s : x_e;

                            we = (y_e-proj_y);
                            ws = (proj_y-y_s);
                            ynn = (ws<=we) ? y_s : y_e;

                            we = (z_e-proj_z);
                            ws = (proj_z-z_s);
                            znn = (ws<=we) ? z_s : z_e;

                            pT.pImg[idx]=pS.val4d(0, znn, ynn, xnn);
                        }
                        else if(opt.inttype==TriLinear)
                        {
                            REAL l,r,u,d,f,b;
                            l = 1.0 - (proj_x-(REAL)x_s); r = 1.0 -l;
                            u = 1.0 - (proj_y-(REAL)y_s); d = 1.0 -u;
                            f = 1.0 - (proj_z-(REAL)z_s); b = 1.0 -f;

                            REAL i1, i2, j1, j2, w1, w2;

                            i1 = (REAL)(pSR.val4d(0, z_s, y_s, x_s)) * f + (REAL)(pSR.val4d(0, z_e, y_s, x_s)) * b;
                            i2 = (REAL)(pSR.val4d(0, z_s, y_e, x_s)) * f + (REAL)(pSR.val4d(0, z_e, y_e, x_s)) * b;
                            j1 = (REAL)(pSR.val4d(0, z_s, y_s, x_e)) * f + (REAL)(pSR.val4d(0, z_e, y_s, x_e)) * b;
                            j2 = (REAL)(pSR.val4d(0, z_s, y_e, x_e)) * f + (REAL)(pSR.val4d(0, z_e, y_e, x_e)) * b;

                            w1 = i1 * u + i2 * d;
                            w2 = j1 * u + j2 * d;

                            REAL val = w1 * l + w2 * r;

                            if(b_isint)
                                pT.pImg[idx]=val+0.5;
                            else
                                pT.pImg[idx]=val;
                        }

                    }
                }
            }

            // de-alloc
            y_del2<REAL, REAL>(index,position);

        }

    }
    else if(opt.regtype == NONRIGID)
    {
        // displacement field
        Y_IMG_D df;

        Tidx *szdf=NULL;
        y_new<Tidx, Tidx>(szdf, 4);

        szdf[0] = 3;
        szdf[1] = pT.dims[0];
        szdf[2] = pT.dims[1];
        szdf[3] = pT.dims[2];

        df.initField(szdf, 4); //

        // pT stores 3x(m+1) elements
        // first 3 stored (m, 0, 0)
        // followed by position of landmarks of source image (xi, yi, zi), i=1, 2, ..., m.
        Tidx m = pT.pImg[0]*3 + 3;
        foreach(m, i)
        {
            df.pImg[i] = pT.pImg[i];
        }

        // pDF is a matrix: WA
        mat2field<Tidx, Y_IMG_D, Y_IMG_D>(df, pDF, opt);

        //
        interpolation<Tidx, Y_IMG_S, Y_IMG_T, Y_IMG_D>(pT, pSR, df, opt);

        // de-alloc
        y_del<Tidx>(szdf);
        df.clean();
    }
    else
    {
        cout<<"Your registration is not supported"<<endl;
        return;
    }

    //
    return;
}


// func warp
template <class Tidx, class Y_IMG_S, class Y_IMG_T, class Y_IMG_D>
void warp2(Y_IMG_T &pT, Y_IMG_S pS, Y_IMG_D pDF, REAL scalex, REAL scaley, REAL scalez, RegistrationOption opt)
{

    // resize the source to the template
    Y_IMG_S pSR;
    if(pS.dims[0]!=pT.dims[0] || pS.dims[1]!=pT.dims[1] || pS.dims[2]!=pT.dims[2])
    {
        pSR.initImage(pT.dims, 4);
        resize<Tidx, Y_IMG_S, Y_IMG_S>(pSR, pS, PADPOST);
    }
    else
    {
        pSR.setImage(pS.pImg, pS.dims, 4);
    }

    // image warping
    if(opt.regtype == TRANSLATION)
    {

    }
    else if(opt.regtype == RIGID || opt.regtype == AFFINE)
    {
        //
        /// transform
        //

        // memory-efficient affine transform
        Y_MAT<REAL, Tidx> M(pDF.pImg, 4, 4);

        // rotation
        Y_MAT<REAL, Tidx> R;
        R.init(4, 4, 1);

        for(Tidx i=0; i<3; i++)
        {
            for(Tidx j=0; j<3; j++)
            {
                R.v[i][j] = M.v[i][j];
            }
        }

        REAL *index=NULL, *position=NULL;
        y_new<REAL,Tidx>(index,4);
        y_new<REAL,Tidx>(position,4);

        // dims
        Tidx sz = pT.dims[2];
        Tidx sy = pT.dims[1];
        Tidx sx = pT.dims[0];

        // rotation center
        REAL cz, cy, cx;
        if(y_abs<REAL>(M.v[3][2])>1 || y_abs<REAL>(M.v[3][1])>1 || y_abs<REAL>(M.v[3][0])>1)
        {
            cz = (REAL)(M.v[3][2]);
            cy = (REAL)(M.v[3][1]);
            cx = (REAL)(M.v[3][0]);

            cz *= scalez;
            cy *= scaley;
            cx *= scalex;

        }
        else
        {
            cz = (REAL)(sz-1) / 2.0;
            cy = (REAL)(sy-1) / 2.0;
            cx = (REAL)(sx-1) / 2.0;
        }

        // translation
        Tidx tx = M.v[0][3] * scalex;
        Tidx ty = M.v[1][3] * scaley;
        Tidx tz = M.v[2][3] * scalez;

        // displacement field
        bool b_isint = true;
        if(pT.dt==4) b_isint = false; // float type

        index[3] = 1.0;
        position[3] = 1.0;
        for(Tidx z=0; z<sz; z++)
        {
            index[2] = (REAL)z - cz;
            for(Tidx y=0; y<sy; y++)
            {
                index[1] = (REAL)y - cy;
                for(Tidx x=0; x<sx; x++)
                {
                    index[0] = (REAL)x - cx;

                    R.vectorMul(index, position);

                    //qDebug()<<"position ... "<<position[0]<<position[1]<<position[2]<<position[3];

                    Tidx idx = pT.idx4d(0,z,y,x);

                    //qDebug()<<"idx ..."<<x<<y<<z<<idx;

                    REAL proj_x, proj_y, proj_z;

                    proj_x = position[0]/position[3] + cx + tx; // x
                    proj_y = position[1]/position[3] + cy + ty; // y
                    proj_z = position[2]/position[3] + cz + tz; // z

                    //qDebug()<<"proj ... "<<proj_x<<sx<<proj_y<<sy<<proj_z<<sz;

                    if(proj_x<0 || proj_x>pSR.dims[0]-1 || proj_y<0 || proj_y>pSR.dims[1]-1 || proj_z<0 || proj_z>pSR.dims[2]-1)
                    {
                        pT.pImg[idx] = 0;
                        continue;
                    }

                    Tidx x_s,x_e,y_s,y_e,z_s,z_e;
                    x_s=floor(proj_x);  x_e=ceil(proj_x);
                    y_s=floor(proj_y);  y_e=ceil(proj_y);
                    z_s=floor(proj_z);  z_e=ceil(proj_z);

                    x_s=x_s<0?0:x_s;
                    y_s=y_s<0?0:y_s;
                    z_s=z_s<0?0:z_s;

                    x_e=x_e>=sx?sx-1:x_e;
                    y_e=y_e>=sy?sy-1:y_e;
                    z_e=z_e>=sz?sz-1:z_e;

                    // interpolation
                    if(opt.inttype==NearestNeighbor)
                    {
                        Tidx xnn, ynn, znn;
                        REAL we, ws;

                        we = (x_e-proj_x);
                        ws = (proj_x-x_s);
                        xnn = (ws<=we) ? x_s : x_e;

                        we = (y_e-proj_y);
                        ws = (proj_y-y_s);
                        ynn = (ws<=we) ? y_s : y_e;

                        we = (z_e-proj_z);
                        ws = (proj_z-z_s);
                        znn = (ws<=we) ? z_s : z_e;

                        pT.pImg[idx]=pS.val4d(0, znn, ynn, xnn);
                    }
                    else if(opt.inttype==TriLinear)
                    {
                        REAL l,r,u,d,f,b;
                        l = 1.0 - (proj_x-(REAL)x_s); r = 1.0 -l;
                        u = 1.0 - (proj_y-(REAL)y_s); d = 1.0 -u;
                        f = 1.0 - (proj_z-(REAL)z_s); b = 1.0 -f;

                        REAL i1, i2, j1, j2, w1, w2;

                        i1 = (REAL)(pSR.val4d(0, z_s, y_s, x_s)) * f + (REAL)(pSR.val4d(0, z_e, y_s, x_s)) * b;
                        i2 = (REAL)(pSR.val4d(0, z_s, y_e, x_s)) * f + (REAL)(pSR.val4d(0, z_e, y_e, x_s)) * b;
                        j1 = (REAL)(pSR.val4d(0, z_s, y_s, x_e)) * f + (REAL)(pSR.val4d(0, z_e, y_s, x_e)) * b;
                        j2 = (REAL)(pSR.val4d(0, z_s, y_e, x_e)) * f + (REAL)(pSR.val4d(0, z_e, y_e, x_e)) * b;

                        w1 = i1 * u + i2 * d;
                        w2 = j1 * u + j2 * d;

                        REAL val = w1 * l + w2 * r;

                        if(b_isint)
                            pT.pImg[idx]=val+0.5;
                        else
                            pT.pImg[idx]=val;
                    }

                }
            }
        }

        // de-alloc
        y_del2<REAL, REAL>(index,position);

    }
    else if(opt.regtype == NONRIGID)
    {
        // displacement field
        Y_IMG_D df;

        Tidx *szdf=NULL;
        y_new<Tidx, Tidx>(szdf, 4);

        szdf[0] = 3;
        szdf[1] = pT.dims[0];
        szdf[2] = pT.dims[1];
        szdf[3] = pT.dims[2];

        df.initField(szdf, 4); //

        // pT stores 3x(m+1) elements
        // first 3 stored (m, 0, 0)
        // followed by position of landmarks of source image (xi, yi, zi), i=1, 2, ..., m.
        Tidx m = pT.pImg[0]*3 + 3;
        foreach(m, i)
        {
            df.pImg[i] = pT.pImg[i];
        }

        // pDF is a matrix: WA
        mat2field<Tidx, Y_IMG_D, Y_IMG_D>(df, pDF, opt);

        //
        interpolation<Tidx, Y_IMG_S, Y_IMG_T, Y_IMG_D>(pT, pSR, df, opt);

        // de-alloc
        y_del<Tidx>(szdf);
        df.clean();
    }
    else
    {
        cout<<"Your registration is not supported"<<endl;
        return;
    }

    //
    return;
}


// func calculate tps weights from control points
template <class Tidx, class Y_MAT_T>
void calcTPS(Y_MAT_T &WA, Y_LANDMARK controlPointsSrc, Y_LANDMARK controlPointsDst, double lambda)
{
    //
    /// compute matrix WA3x(m+4)
    //

    // error checking
    Tidx m = controlPointsSrc.size();
    if ( m < 3 )
    {
        cout<<"at least 3 points to define a plane"<<endl;
        return;
    }
    if(controlPointsDst.size()!=m)
    {
        cout<<"The number of feature points is not match"<<endl;
        return;
    }
    if(lambda<0)
    {
        cout<<"invalid regularization parameter"<<endl;
        return;
    }

    // allocate the matrix and vector
    Y_MAT_T L; // M:Kmxm + lambda*Imxm, Pmx4:(1, xi, yi, zi) i=0,1,...,m-1, Pt4xm, 04x4
    L.init(m+4, m+4, 0);

    Y_MAT_T V; // V3x(m+4)
    V.init(3, m+4, 0);

    // M
    double a = 0.0; // diagonal: reqularization parameters (lambda * a^2)
    for ( Tidx i=0; i<m; i++ )
    {
        for ( Tidx j=i+1; j<m; j++ )
        {
            //
            Y_MARKER<double> pt_src = controlPointsSrc.at(i);
            Y_MARKER<double> pt_dst = controlPointsSrc.at(j);

            double ss=0, t;

            t = pt_src.x - pt_dst.x; ss += t*t;
            t = pt_src.y - pt_dst.y; ss += t*t;
            t = pt_src.z - pt_dst.z; ss += t*t;

            L.v[i][j] = L.v[j][i] = tpsRBF<double>(ss+EPS);

            a += ss;
        }
    }
    a /= (double)(m*m);

    // Fill the rest of L
    for ( Tidx i=0; i<m; ++i )
    {
        // diagonal
        L.v[i][i] = lambda*(a*a);

        // P
        L.v[i][m] = 1.0;
        L.v[i][m+1] = controlPointsSrc.at(i).x;
        L.v[i][m+2] = controlPointsSrc.at(i).y;
        L.v[i][m+3] = controlPointsSrc.at(i).z;

        // P transposed
        L.v[m][i] = 1.0;
        L.v[m+1][i] = controlPointsSrc.at(i).x;
        L.v[m+2][i] = controlPointsSrc.at(i).y;
        L.v[m+3][i] = controlPointsSrc.at(i).z;
    }

    // Fill the right hand vector V
    for ( Tidx i=0; i<m; ++i )
    {
        V.v[i][0] = controlPointsDst.at(i).x;
        V.v[i][1] = controlPointsDst.at(i).y;
        V.v[i][2] = controlPointsDst.at(i).z;
    }

    // Solve the linear system
    L.lu(L,V); //L.pseudoinverse(); V.prod(L);

    //
    WA.copy(V);

    // bending energy = wt * M * w

    //
    L.clean();
    V.clean();

    //
    return;
}

// function of reflectional symmetry detection
template <class Tidx, class Tdata, class Y_IMG_T>
void symmetryDet(Y_IMG_T &pT, Y_LANDMARK &pLMs)
{
    //
    Tidx sx = pT.dims[0];
    Tidx sy = pT.dims[1];
    Tidx sz = pT.dims[2];
    Tidx sc = pT.dims[3];

    // 1: normalize, gaussian filtering, gradient
    normalizing<Tdata, Tidx>(pT.pImg, pT.totalplxs, 0, 255);

    Tidx *w=NULL, winsz;
    winsz = y_max<Tidx>(sx,sy)/5;

    y_new<Tidx, Tidx>(w,3);
    w[0] = winsz; w[1] = winsz; w[2] = winsz;
    guassianFiltering<REAL, Tidx>(pT.pImg, pT.dims, w);
    y_del<Tidx>(w);

    gradientMagnitude<Tdata, Tidx>(pT.pImg, pT.dims, 3);

    // 2: thresholding
    //thresholding<Tdata, Tidx>(pT.pImg, pT.dims);

    // 3: midline detect
    // 3.1: init a line (N control points) along y-axis
    Tidx N = 20;
    Tidx step = sy/N/2;

    if(step<1) step = 1;

    Tidx init_x = sx/2;
    Tidx init_z = sz/2;

    Y_MARKER<double> p;

    p.x = init_x;
    p.z = init_z;

    Tidx miny = sy/2 - N/2 * step;

    foreach(N, i)
    {
        p.y = miny + i*step;
        pLMs.push_back(p);
    }

    // 3.2: find midline by updating control point position with voronoi diagram
    REAL *sumxi=NULL, *sumyi=NULL, *sumzi=NULL, *sumvi=NULL, disti;
    y_new<REAL, Tidx>(sumxi,N);
    y_new<REAL, Tidx>(sumyi,N);
    y_new<REAL, Tidx>(sumzi,N);
    y_new<REAL, Tidx>(sumvi,N);

    REAL errorDist = INF;
    REAL stopThresh = 1.0;
    REAL distPre = 0, distCur = 0;

    qDebug()<<"Init Points  ... ";
    foreach(N, i)
    {
        cout<<pLMs.at(i).x<<", "<<pLMs.at(i).y<<", "<<pLMs.at(i).z<<","<<endl;
    }

    // voronoi diagram
    // update center of mass
    Tidx iter = 0;
    while (errorDist > stopThresh)
    {
        // init
        foreach(N, ii)
        {
            sumxi[ii] = 0;
            sumyi[ii] = 0;
            sumzi[ii] = 0;
            sumvi[ii] = 0;
        }
        distCur = 0;

        // iterating
        for(Tidx k=0; k<sz; k++)
        {
            Tidx ofz = k*pT.of2;
            for(Tidx j=0; j<sy; j++)
            {
                Tidx ofy = j*pT.of1 + ofz;
                for(Tidx i=0; i<sx; i++)
                {
                    Tidx idx = ofy + i;
                    Tdata val = pT.pImg[idx];


                    REAL minv=INF;
                    Tidx buoy = 0;
                    foreach(N, ii)
                    {
                        disti = sqrt( (i-pLMs.at(ii).x)*(i-pLMs.at(ii).x) + (j-pLMs.at(ii).y)*(j-pLMs.at(ii).y) + (k-pLMs.at(ii).z)*(k-pLMs.at(ii).z) );

                        if(disti<minv)
                        {
                            minv = disti;
                            buoy = ii;
                        }
                    }

                    sumxi[buoy] += val*i;
                    sumyi[buoy] += val*j;
                    sumzi[buoy] += val*k;
                    sumvi[buoy] += val;

                } // x
            } // y
        } // z

        // update control points
        foreach(N, ii)
        {
            Y_MARKER<double> p;

            p.x = sumxi[ii] / sumvi[ii];
            p.y = sumyi[ii] / sumvi[ii];
            p.z = sumzi[ii] / sumvi[ii];

            distCur += y_dist<double>(p.x, p.y, p.z, pLMs.at(ii).x, pLMs.at(ii).y, pLMs.at(ii).z);

            pLMs.at(ii).x = p.x; pLMs.at(ii).y = p.y; pLMs.at(ii).z = p.z;
        }

        // error
        errorDist = y_abs<REAL>(distCur - distPre);

        qDebug()<<"itering ... "<<++iter<<errorDist<<distPre<<distCur;

        distPre = distCur;
    }
    y_del<REAL>(sumxi);
    y_del<REAL>(sumyi);
    y_del<REAL>(sumzi);
    y_del<REAL>(sumvi);

    qDebug()<<"Detected Points  ... ";
    foreach(N, i)
    {
        cout<<Tidx(pLMs.at(i).x)<<", "<<Tidx(pLMs.at(i).y)<<", "<<Tidx(pLMs.at(i).z)<<","<<endl;
    }

    // fitting a line to the new control points




    //
    return;
}

//-------------------------------------------------------------------------------
/// image registration class
//-------------------------------------------------------------------------------
template <class Tdata, class Tidx, class Y_IMG_S, class Y_IMG_T, class Y_IMG_D>
class Y_IREG
{
public:
    Y_IREG(){}
    ~Y_IREG(){}

public:

    //
    // global alignment (rigid/affine)
    //
    // Chumchob, N. & Chen, K. A robust affine image registration method.
    // International Journal of Numerical Analysis and Modeling 6, 311-334 (2009).
    //
    // Kroon, D.-J.; Slump, C.H. "MRI modalitiy transformation in demon registration,"
    // Biomedical Imaging: From Nano to Macro, 2009. ISBI '09. IEEE International Symposium on,
    // vol., no., pp.963-966, June 28 2009-July 1 2009
    //
    void globalregistration(Y_IMG_D &pOutFlip, Y_IMG_D &pOut, Y_IMG_S pS, Y_IMG_T pT);

    // local alignment (SyN)
    void localregistration(Y_IMG_D &pOut, Y_IMG_S pS, Y_IMG_T pT);

    // landmark-based alignment
    void lmregistration(Y_IMG_D &pOut, Y_IMG_S pS, Y_IMG_T pT, Y_LANDMARK lmS, Y_LANDMARK lmT);

public:
    RegistrationOption   opt;
    Pyramid<Tdata, Tidx> pyr;
};

// func landmark-based alignment
template <class Tdata, class Tidx, class Y_IMG_S, class Y_IMG_T, class Y_IMG_D>
void Y_IREG<Tdata, Tidx, Y_IMG_S, Y_IMG_T, Y_IMG_D> :: lmregistration(Y_IMG_D &pOut, Y_IMG_S pS, Y_IMG_T pT, Y_LANDMARK lmS, Y_LANDMARK lmT)
{
    // input: pS, lmS, pT, lmT
    // output: pOut

    //
    /// Point Cloud Registration
    //

    //
    /// TPS-Warp
    //

    Y_MAT<REAL, Tidx> WA(pOut.pImg, pOut.dims[0], pOut.dims[1]); // Matrix WA(m+4)x3

    calcTPS<Tidx, Y_MAT<REAL, Tidx> >(WA, lmT, lmS, 0.05); // T2S TPS Matrix


    qDebug()<<"affine matrix ...";
    for(V3DLONG i=0; i<WA.column; i++)
    {
        for(V3DLONG j=0; j<WA.row; j++)
        {
            cout<<" "<<WA.v[i][j];
        }
        cout<<endl;
    }
    cout<<endl;


    //
    return;
}

// func global registration
template <class Tdata, class Tidx, class Y_IMG_S, class Y_IMG_T, class Y_IMG_D>
void Y_IREG<Tdata, Tidx, Y_IMG_S, Y_IMG_T, Y_IMG_D> :: globalregistration(Y_IMG_D &pOutFlip, Y_IMG_D &pOut, Y_IMG_S pS, Y_IMG_T pT)
{
    // similarity: SSD
    // optimization: GradientDescent

    // Inputs: pS -- source image
    //         pT -- template image
    //
    // Output: pOut -- affine matrix

    //
    // demon registration (optional)
    //
    // J.P. Thirion, “Image matching as a diffusion process: an analogy with maxwell’s demons,”
    // Medical Image Analysis, pp.243–260, September 1998.
    //

    /// step 0-1: resize pS to pT
    Y_IMG_S pSR;
    if(pS.dims[0]!=pT.dims[0] || pS.dims[1]!=pT.dims[1] || pS.dims[2]!=pT.dims[2])
    {
        pSR.initImage(pT.dims, 4);

        resize<Tidx, Y_IMG_S, Y_IMG_S>(pSR, pS, IMCENTER);

        //for(Tidx i=0; i<pT.totalplxs; i++) pSR.pImg[i] = pT.pImg[i];
        //resize<Tidx, Y_IMG_S, Y_IMG_S>(pSR, pS, MASSCENTER); // match centers of mass (optional)
    }
    else
    {
        pSR.setImage(pS.pImg, pS.dims, 4);
    }

    /// step 0-2: init affine matrix
    Y_MAT<REAL, Tidx> A(pOut.pImg, pOut.dims[0], pOut.dims[1]); // 3D A4x4
    A.unit(); // an initial guess assuming an identity transformation

    Y_MAT<REAL, Tidx> F(pOutFlip.pImg, pOutFlip.dims[0], pOutFlip.dims[1]); // 3D A4x4
    F.unit();

    Y_MAT<REAL, Tidx> R; // Rotations computed from principal axis transformation
    R.init(A.row, A.column, 1.0);

    //
    /// registration
    //

    Tidx sx = pT.dims[0];
    Tidx sy = pT.dims[1];
    Tidx sz = pT.dims[2];

    Tidx totalplxs = sx*sy*sz;

    //
    Tidx sx_ori = sx;
    Tidx sy_ori = sy;
    Tidx sz_ori = sz;

    Tidx *dsdims = NULL;
    y_new<Tidx, Tidx>(dsdims, 4);

    //
    for(Tidx iscale=0; iscale<this->pyr.s.size(); iscale++)
    {
        // align at the scale: iscale
        Tidx iters = pyr.s.at(iscale).iters;

        // sampling
        sx = pyr.s.at(iscale).sampleratio[0] * sx_ori;
        sy = pyr.s.at(iscale).sampleratio[1] * sy_ori;
        sz = pyr.s.at(iscale).sampleratio[2] * sz_ori;

        totalplxs = sx*sy*sz;

        dsdims[0] = sx;
        dsdims[1] = sy;
        dsdims[2] = sz;
        dsdims[3] = 1;

        Y_IMG_D pSDds;
        pSDds.initImage(dsdims, 4);

        Y_IMG_D pTDds;
        pTDds.initImage(dsdims, 4);

        Y_IMG_D pSWarpDds;
        pSWarpDds.initImage(dsdims, 4);

        resize<Tidx, Y_IMG_S, Y_IMG_D>(pSDds, pSR, SAMPLE);
        resize<Tidx, Y_IMG_T, Y_IMG_D>(pTDds, pT, SAMPLE);

        // smooth
        //        Tidx winsz =  y_min<Tidx>(y_min<Tidx>(pSDds.dims[0], pSDds.dims[1]), pSDds.dims[2])/3; //32;
        //        Tidx *w=NULL;
        //        y_new<Tidx, Tidx>(w,3);
        //        w[0] = winsz; w[1] = winsz; w[2] = winsz;
        //        guassianFiltering<REAL, Tidx>(pSDds.pImg, pSDds.dims, w);
        //        guassianFiltering<REAL, Tidx>(pTDds.pImg, pTDds.dims, w);
        //        y_del<Tidx>(w);

        // normalize and convert datatype to REAL and scale intensity to [0,1]
        normalizing<REAL, Tidx>(pSDds.pImg, pSDds.totalplxs, 0, 1);
        normalizing<REAL, Tidx>(pTDds.pImg, pTDds.totalplxs, 0, 1);

        // segmentation
        thresholding<REAL, Tidx>(pSDds.pImg, pSDds.dims);
        thresholding<REAL, Tidx>(pTDds.pImg, pTDds.dims);

        // Principal Axis Analysis
        Y_MAT<REAL, Tidx> Vs;
        Vs.init(3, 3, 0);
        Y_MAT<REAL, Tidx> Ss;
        Ss.init(3, 3, 0);

        Y_MAT<REAL, Tidx> Vt;
        Vt.init(3, 3, 0);
        Y_MAT<REAL, Tidx> St;
        St.init(3, 3, 0);

        //
        principalaxistransform< Y_IMG_D, Y_MAT<REAL, Tidx>, Tidx >(pSDds, Ss, Vs);
        principalaxistransform< Y_IMG_D, Y_MAT<REAL, Tidx>, Tidx >(pTDds, St, Vt);

        Vs.sortEigenVectors(Ss);
        Vt.sortEigenVectors(St);

        Ss.sortEigenValues();
        St.sortEigenValues();

        foreach(3, j)
        {
            foreach(3, i)
                cout<<" "<<Vs.v[j][i];
            cout<<endl;
        }
        cout<<endl;

        foreach(3, j)
        {
            foreach(3, i)
                cout<<" "<<Ss.v[j][i];
            cout<<endl;
        }
        cout<<endl;

        foreach(3, j)
        {
            foreach(3, i)
                cout<<" "<<Vt.v[j][i];
            cout<<endl;
        }
        cout<<endl;

        foreach(3, j)
        {
            foreach(3, i)
                cout<<" "<<St.v[j][i];
            cout<<endl;
        }
        cout<<endl;

        // R = Vs*Vt
        Vt.prod(Vs);

        foreach(3, j)
        {
            foreach(3, i)
                R.v[j][i] = Vt.v[j][i];
        }

        // adjust angles in xy plane when rotate counter-clockwise
        REAL angleXY = atan2(Vs.v[2][1], Vs.v[2][2]);
        if(y_abs<REAL>(angleXY) > PI/2 )
        {
            cout <<" adjust angles ... "<<angleXY<<endl;

            // define -pi/2 rotation matrix
            Y_MAT<REAL, Tidx> rot90;
            rot90.init(4,4,0);

            rot90.v[0][1] = 1;
            rot90.v[1][0] = -1;
            rot90.v[2][2] = 1;
            rot90.v[3][3] = 1;

            rot90.prod(R);
            R.assign(rot90);

            rot90.clean();
        }

        A.assign(R);

        cout<<"rotated matrix ..."<<endl;
        for(Tidx j=0; j<A.column; j++)
        {
            for(Tidx i=0; i<A.row; i++)
            {
                cout<<" "<<A.v[j][i];
            }
            cout<<endl;
        }
        cout<<endl;


        warp<Tidx, Y_IMG_D, Y_IMG_D, Y_IMG_D>(pSWarpDds, pSDds, pOut, opt); // rotated

        foreach(pSWarpDds.totalplxs, i)
            pSDds.pImg[i] = pSWarpDds.pImg[i]; //


        // segmentation
        thresholding<REAL, Tidx>(pSDds.pImg, pSDds.dims);
        //thresholding<REAL, Tidx>(pTDds.pImg, pTDds.dims);

        // gradient magnitude
        //        gradientMagnitude<REAL, Tidx>(pSDds.pImg, pSDds.dims);
        //        gradientMagnitude<REAL, Tidx>(pTDds.pImg, pTDds.dims);

        // extract surfaces
        //        extractsurfaces<REAL, Tidx>(pSDds.pImg, pSDds.dims);
        //        extractsurfaces<REAL, Tidx>(pTDds.pImg, pTDds.dims);

        // scale alignment !!!
        Y_MAT<REAL, Tidx> S;
        S.init(4,4,1);

        double scaleratioS2T=0;
        //        scaleratioS2T = scaleEstimating<REAL, Tidx>(pSDds.pImg, pTDds.pImg, pTDds.totalplxs);
        //        foreach(3, i)
        //            S.v[i][i] = scaleratioS2T;

        //        foreach(3,i)
        //        {
        //            scaleratioS2T += sqrt( double(Ss.v[i][i]) / double(St.v[i][i]) );
        //        }
        //        scaleratioS2T /= 3.0;

        foreach(3,i)
        {
            double val = sqrt( double(Ss.v[i][i]) / double(St.v[i][i]) );
            if(scaleratioS2T < val) scaleratioS2T = val;
        }

        foreach(3, i)
            S.v[i][i] = scaleratioS2T;

        A.assign(S);
        warp<Tidx, Y_IMG_D, Y_IMG_D, Y_IMG_D>(pSWarpDds, pSDds, pOut, opt); // scaled

        cout<<"scaled matrix "<<endl;
        foreach(4, j)
        {
            foreach(4, i)
                cout<<" "<<A.v[j][i];
            cout<<endl;
        }
        cout<<endl;

        foreach(pSWarpDds.totalplxs, i)
            pSDds.pImg[i] = pSWarpDds.pImg[i]; //

        thresholding<REAL, Tidx>(pSDds.pImg, pSDds.dims);

        // flip correction
        flipcorrectSSD<REAL, Tidx, Y_IMG_D, Y_MAT<REAL, Tidx> >(pSDds, pTDds, F); // flipped

        // S*F*R
        //R.prod(F);
        R.prod(S);
        //F.clean();
        S.clean();

        //
        //        if(opt.smltype == SSD)
        //        {
        //            //
        //            double E=INF, deltaE=0;
        //            double gamma=INF;
        //            for(Tidx i=0; i<4; i++)
        //            {
        //                if(pTDds.dims[i]>1 && gamma>pTDds.dims[i])
        //                    gamma = 10*pTDds.dims[i]; // init
        //            }
        //            double stopCriteria = 0.1;

        //            // fields: meshgrids and displacement
        //            Y_MAT<REAL, Tidx> MGrid, MDF;
        //            MGrid.init(3, totalplxs, 0);
        //            MDF.init(3, totalplxs, 0);

        //            for(Tidx z=0;z<sz;z++)
        //            {
        //                Tidx ofz = z*sx*sy;
        //                for(Tidx y=0;y<sy;y++)
        //                {
        //                    Tidx ofy = ofz + y*sx;
        //                    for(Tidx x=0;x<sx;x++)
        //                    {
        //                        Tidx idx = ofy + x;

        //                        MGrid.v[idx][0] = (REAL)x;
        //                        MGrid.v[idx][1] = (REAL)y;
        //                        MGrid.v[idx][2] = (REAL)z;
        //                    }
        //                }
        //            }

        //            // 8 vertices of the 3D image in voxel
        //            Y_MAT<REAL, Tidx> MVertices, MVerticesTmp, MVerticesVar, MVerticesDT;
        //            MVertices.init(3, 8, 0);
        //            MVerticesTmp.init(3, 8, 0);
        //            MVerticesVar.init(3, 8, 0);
        //            MVerticesDT.init(3, 8, 0);

        //            for(Tidx z=0;z<2;z++)
        //            {
        //                for(Tidx y=0;y<2;y++)
        //                {
        //                    for(Tidx x=0;x<2;x++)
        //                    {
        //                        Tidx idx=4*z+2*y+x;

        //                        MVertices.v[idx][0] = (REAL)x*(REAL)(sx-1);
        //                        MVertices.v[idx][1] = (REAL)y*(REAL)(sy-1);
        //                        MVertices.v[idx][2] = (REAL)z*(REAL)(sz-1);
        //                    }
        //                }
        //            }
        //            MVerticesTmp.assign(MVertices);
        //            MVerticesVar.assign(MVertices);

        //            //
        //            // K. S. Arun, T. S. Huang, and S. D. Blostein, "Least-Squares Fitting of Two 3-D Point Sets",
        //            // IEEE Transactions on Pattern Analysis and Machine Intelligence, PAMI-9(5): 698 - 700, 1987.
        //            //
        //            for(Tidx iter=0; iter<iters; iter++)
        //            {
        //                // update affine matrix
        //                A.getRigid(MVertices, MVerticesVar);

        //                //**************test*****************
        ////                cout<<"got A matrix ..."<<endl;
        ////                for(Tidx j=0; j<A.column; j++)
        ////                {
        ////                    for(Tidx i=0; i<A.row; i++)
        ////                    {
        ////                        cout<<" "<<A.v[j][i];
        ////                    }
        ////                    cout<<endl;
        ////                }
        ////                cout<<endl;
        //                //**********************************

        //                // update warped
        //                pSWarpDds.zeros();
        //                warp<Tidx, Y_IMG_D, Y_IMG_D, Y_IMG_D>(pSWarpDds, pSDds, pOut, opt); // source

        //                // similarity measure: ssd
        //                double ssdST = ssd<Tdata, Tidx>(pSWarpDds.pImg, pTDds.pImg, pTDds.totalplxs);

        //                // updating
        //                deltaE = ssdST-E;

        //                cout<<"globalalign ... iter ("<<iter<<") ... similarity measure (ssd) = "<<ssdST<<" ... delta = "<<deltaE<<endl;

        //                if(deltaE != deltaE)
        //                {
        //                    // NaN value
        //                    cout<<"NaN value occurred"<<endl;
        //                    return;
        //                }
        //                else if(deltaE>=0)
        //                {
        //                    gamma *= 0.95; // annealing
        //                }
        //                else
        //                {
        //                    // gradient descent
        //                    E = ssdST;

        //                    //
        //                    MVerticesTmp.assign(MVerticesVar);

        //                    // smooth
        //                    Tidx winsz = 4;
        //                    Tidx *w=NULL;
        //                    y_new<Tidx, Tidx>(w,3);
        //                    w[0] = winsz; w[1] = winsz; w[2] = winsz;
        //                    guassianFiltering<REAL, Tidx>(pSWarpDds.pImg, pSWarpDds.dims, w);
        //                    y_del<Tidx>(w);

        //                    // gradient descent
        //                    Tidx gstep = 1;
        //                    for(Tidx z=0;z<sz;z++)
        //                    {
        //                        Tidx ofz = z*sx*sy;
        //                        for(Tidx y=0;y<sy;y++)
        //                        {
        //                            Tidx ofy = ofz + y*sx;
        //                            for(Tidx x=0;x<sx;x++)
        //                            {
        //                                Tidx idx = ofy + x;

        //                                //
        //                                Tidx xs, xe;
        //                                xs = x-gstep; if(xs<0) xs=0;
        //                                xe = x+gstep; if(xe>=sx) xe=sx-1;

        //                                Tidx ys, ye;
        //                                ys = y-gstep; if(ys<0) ys=0;
        //                                ye = y+gstep; if(ye>=sy) ye=sy-1;

        //                                Tidx zs, ze;
        //                                zs = z-gstep; if(zs<0) zs=0;
        //                                ze = z+gstep; if(ze>=sz) ze=sz-1;

        //                                //
        //                                REAL vxl = pSWarpDds.val4d(0,z,y,xs);
        //                                REAL vxr = pSWarpDds.val4d(0,z,y,xe);

        //                                REAL vyu = pSWarpDds.val4d(0,z,ys,x);
        //                                REAL vyd = pSWarpDds.val4d(0,z,ye,x);

        //                                REAL vzf = pSWarpDds.val4d(0,zs,y,x);
        //                                REAL vzb = pSWarpDds.val4d(0,ze,y,x);

        //                                REAL vcurw = pSWarpDds.val4d(0,z,y,x);
        //                                REAL vcurt = pTDds.val4d(0,z,y,x);

        //                                //
        //                                if(vxl<0 || vxr<0 || vyu<0 || vyd<0 || vzf<0 || vzb<0 || vcurw<0  || vcurt<0)
        //                                {
        //                                    MDF.v[idx][0] = 0;
        //                                    MDF.v[idx][1] = 0;
        //                                    MDF.v[idx][2] = 0;
        //                                }
        //                                else
        //                                {
        //                                    REAL Ix = vxr - vxl;
        //                                    REAL Iy = vyd - vyu;
        //                                    REAL Iz = vzb - vzf;

        //                                    REAL It = vcurw - vcurt;

        //                                    MDF.v[idx][0] = It*Ix + (REAL)x;
        //                                    MDF.v[idx][1] = It*Iy + (REAL)y;
        //                                    MDF.v[idx][2] = It*Iz + (REAL)z;
        //                                }
        //                            }
        //                        }
        //                    }

        //                    //
        //                    Y_MAT<REAL, Tidx> B;
        //                    B.init(4, 4, 1);

        //                    B.getRigid(MGrid, MDF);

        //                    //
        //                    REAL *index=NULL, *position=NULL;
        //                    y_new<REAL,Tidx>(index,4);
        //                    y_new<REAL,Tidx>(position,4);

        //                    index[3] = 1.0;
        //                    position[3] = 1.0;
        //                    for(Tidx z=0; z<2; z++)
        //                    {
        //                        index[2] = ((REAL)(z) - 0.5)*(REAL)(sz-1);
        //                        for(Tidx y=0; y<2; y++)
        //                        {
        //                            index[1] = ((REAL)y - 0.5)*(REAL)(sy-1);
        //                            for(Tidx x=0; x<2; x++)
        //                            {
        //                                index[0] = ((REAL)x - 0.5)*(REAL)(sx-1);
        //                                Tidx idx=4*z+2*y+x;

        //                                B.vectorMul(index, position);

        //                                MVerticesDT.v[idx][0] = position[0]/position[3] - index[0];
        //                                MVerticesDT.v[idx][1] = position[1]/position[3] - index[1];
        //                                MVerticesDT.v[idx][2] = position[2]/position[3] - index[2];
        //                            }
        //                        }
        //                    }

        //                    // de-alloc
        //                    y_del2<REAL, REAL>(index,position);
        //                }

        //                // updates vertices
        //                for(Tidx j=0; j<MVerticesVar.column; j++)
        //                {
        //                    for(Tidx i=0; i<MVerticesVar.row; i++)
        //                    {
        //                        MVerticesVar.v[j][i] = MVerticesTmp.v[j][i] - gamma*MVerticesDT.v[j][i];
        //                    }
        //                }

        //                //
        //                if(y_abs(gamma)<stopCriteria || y_abs(E)<stopCriteria) break;
        //            }// end iterations
        //        } // end ssd registration
        //        else
        //        {
        //            // other similarity measure
        //        }

        //
        pSDds.clean();
        pTDds.clean();
        pSWarpDds.clean();

        //        qDebug()<<"affine matrix A ...";
        //        for(V3DLONG i=0; i<A.column; i++)
        //        {
        //            for(V3DLONG j=0; j<A.row; j++)
        //            {
        //                cout<<" "<<A.v[i][j];
        //            }
        //            cout<<endl;
        //        }
        //        cout<<endl;

    } // end multiscale analysis


    qDebug()<<"affine matrix F ...";
    for(V3DLONG i=0; i<F.column; i++)
    {
        for(V3DLONG j=0; j<F.row; j++)
        {
            cout<<" "<<F.v[i][j];
        }
        cout<<endl;
    }
    cout<<endl;

    qDebug()<<"principal axis transform matrix ...";
    for(V3DLONG i=0; i<R.column; i++)
    {
        for(V3DLONG j=0; j<R.row; j++)
        {
            cout<<" "<<R.v[i][j];
        }
        cout<<endl;
    }
    cout<<endl;

    //A.prod(R); // rigid matrix
    A.assign(R);

    qDebug()<<"affine matrix output ...";
    for(V3DLONG i=0; i<A.column; i++)
    {
        for(V3DLONG j=0; j<A.row; j++)
        {
            cout<<" "<<A.v[i][j];
        }
        cout<<endl;
    }
    cout<<endl;

    //
    y_del<Tidx>(dsdims);

    return;
}

// func local registration
template <class Tdata, class Tidx, class Y_IMG_S, class Y_IMG_T, class Y_IMG_D>
void Y_IREG<Tdata, Tidx, Y_IMG_S, Y_IMG_T, Y_IMG_D> :: localregistration(Y_IMG_D &pOut, Y_IMG_S pS, Y_IMG_T pT)
{

}


//-------------------------------------------------------------------------------
/// tile configuration copy from y_imglib.h
//-------------------------------------------------------------------------------

#define TC_COMMENT1 (" thumbnail file ")
#define TC_COMMENT2 (" tiles ")
#define TC_COMMENT3 (" dimensions (XYZC) ")
#define TC_COMMENT4 (" origin (XYZ) ")
#define TC_COMMENT5 (" resolution (XYZ) ")
#define TC_COMMENT6 (" image coordinates look up table ")
#define TC_COMMENT7 (" MST LUT ")
bool errorRead(ifstream *tc)
{

    cout<<"Your stitching configuration (.tc) file is illegal."<<endl;
    tc->close();
    return false;
}

#endif //__Y_IMGREG_H__
