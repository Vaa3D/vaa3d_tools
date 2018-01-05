// y_NiftiImageIO.cpp
// created by Yang Yu, Dec 20, 2011
//

#include "y_NiftiImageIO.h"
#include "y_img.h"

//
//class NiftiImageIO
//

NiftiImageIO::NiftiImageIO():
    m_NiftiImage(0),
    m_RescaleSlope(1.0),
    m_RescaleIntercept(0.0),
    m_ComponentType(UNKNOWNCOMPONENTTYPE),
    m_LegacyAnalyze75Mode(true),
    dimx(1),
    dimy(1),
    dimz(1),
    dimc(1),
    dimt(1),
    dx(1.0),
    dy(1.0),
    dz(1.0),
    dc(1.0),
    dt(1.0)
{
    // supported file suffix
    // .nia .nii .nii.gz
    // .hdr .img .img.gz

    this->m_FileName = NULL;
}

NiftiImageIO::~NiftiImageIO()
{
    nifti_image_free(this->m_NiftiImage);
}

// file names
char* NiftiImageIO::getFileName()
{
    return this->m_FileName;
}

void NiftiImageIO::setFileName(char* FileName)
{
    if(!FileName)
    {
        cout<<"Invalid file name"<<endl;
        return;
    }

    y_new<char, int>(this->m_FileName, strlen(FileName) + 1);
    strcpy(this->m_FileName,FileName);

    return;
}

// dimensions
long NiftiImageIO::getDimx()
{
    return this->dimx;
}

long NiftiImageIO::getDimy()
{
    return this->dimy;
}

long NiftiImageIO::getDimz()
{
    return this->dimz;
}

long NiftiImageIO::getDimc()
{
    return this->dimc;
}

long NiftiImageIO::getDimt()
{
    return this->dimt;
}

void NiftiImageIO::setDimx(long x)
{
    if(x<=0)
    {
        cout << "invalid x dimension"<<endl;
    }
    this->dimx = x;
}

void NiftiImageIO::setDimy(long y)
{
    if(y<=0)
    {
        cout << "invalid y dimension"<<endl;
    }
    this->dimy = y;
}

void NiftiImageIO::setDimz(long z)
{
    if(z<=0)
    {
        cout << "invalid z dimension"<<endl;
    }
    this->dimz = z;
}

void NiftiImageIO::setDimc(long c)
{
    if(c<=0)
    {
        cout << "invalid c dimension"<<endl;
    }
    this->dimc = c;
}

void NiftiImageIO::setDimt(long t)
{
    if(t<=0)
    {
        cout << "invalid t dimension"<<endl;
    }
    this->dimt = t;
}

int NiftiImageIO::getDataType()
{
    return m_ComponentType;
}

void NiftiImageIO::setDataType(int datatype)
{
    this->m_ComponentType = (IOComponentType)datatype;
}

//
double NiftiImageIO::getDx()
{
    return dx;
}

double NiftiImageIO::getDy()
{
    return dy;
}

double NiftiImageIO::getDz()
{
    return dz;
}

//
bool NiftiImageIO::mustRescale()
{
    return abs(this->m_RescaleSlope) > numeric_limits< double >::epsilon()
            && ( abs(this->m_RescaleSlope - 1.0) > numeric_limits< double >::epsilon()
                 || abs(this->m_RescaleIntercept) > numeric_limits< double >::epsilon() );
}

void NiftiImageIO::setPixelType(IOPixelType pt)
{
    this->m_PixelType = pt;
}

// reading
bool NiftiImageIO::canReadFile(char *FileNameToRead)
{
    setFileName(FileNameToRead);

    // is_nifti_file returns
    //       > 0 for a nifti file
    //      == 0 for an analyze file,
    //       < 0 for an error,

    const char * filename = reinterpret_cast<const char *>(FileNameToRead);
    const int image_FTYPE = is_nifti_file(filename);

    if ( image_FTYPE > 0 )
    {
        return true;
    }
    else if (image_FTYPE == 0)
    {
        this->m_LegacyAnalyze75Mode=true;
        return true;
    }

    /* image_FTYPE < 0 */
    return false;
}

bool NiftiImageIO::load(void *&buffer)
{
    const char * filename = reinterpret_cast<const char *>(this->getFileName());
    this->m_NiftiImage = nifti_image_read(filename, 1);

    static std::string prev;
    if ( prev != this->getFileName() )
    {
        prev = this->getFileName();
    }
    if ( this->m_NiftiImage == 0 )
    {
        cout << this->getFileName() << " is not recognized as a NIFTI file!" <<endl;
        return false;
    }

    //Check the intent code, it is a vector image, or matrix image, then this is not true.
    int dims = 0;
    if ( this->m_NiftiImage->intent_code == NIFTI_INTENT_VECTOR || this->m_NiftiImage->intent_code == NIFTI_INTENT_SYMMATRIX )
    {
        if ( this->m_NiftiImage->dim[4] > 1 )
        {
            dims = 4;
        }
        else if ( this->m_NiftiImage->dim[3] > 1 )
        {
            dims = 3;
        }
        else if ( this->m_NiftiImage->dim[2] > 1 )
        {
            dims = 2;
        }
        else
        {
            dims = 1;
        }
    }
    else if ( this->m_NiftiImage->intent_code == NIFTI_INTENT_GENMATRIX )
    {
        //TODO:  NEED TO DEAL WITH CASE WHERE NIFTI_INTENT_MATRIX
        cout<< this->getFileName() << " has an intent code of NIFTI_INTENT_GENMATRIX which is not yet implemented in ITK" <<endl;
        return false;
    }
    else
    { //Simple Scalar Image
        //
        //    this->SetNumberOfDimensions(this->m_NiftiImage->dim[0]);
        // HACK ALERT KW
        // Apparently some straight-from-the-scanner files report as 4D
        // with T = 1; this causes ImageFileReader to erroneously ignore the
        // reported
        // direction cosines.
        unsigned realdim;
        for ( realdim = this->m_NiftiImage->dim[0];
              this->m_NiftiImage->dim[realdim] == 1 && realdim > 3;
              realdim-- )
        {}
        dims = realdim;
        this->dimc = 1;
    }

    if ( this->m_NiftiImage->intent_code == NIFTI_INTENT_VECTOR || this->m_NiftiImage->intent_code == NIFTI_INTENT_SYMMATRIX )
    {
        dims = this->m_NiftiImage->dim[5];
    }
    else if ( this->m_NiftiImage->intent_code == NIFTI_INTENT_GENMATRIX )
    {
        //TODO:  NEED TO DEAL WITH CASE WHERE NIFTI_INTENT_MATRIX
        cout << this->getFileName() << " has an intent code of NIFTI_INTENT_GENMATRIX which is not yet implemented in ITK" <<endl;
        return false;
    }
    //TODO:  Dealing with NIFTI_INTENT_VECTOR or NIFTI_INTENT_GENMATRIX with data
    // type of NIFTI_TYPE_COMPLEX64 NIFTI_TYPE_COMPLEX128 NIFTI_TYPE_RGB24 not
    // supported.

    switch ( this->m_NiftiImage->datatype )
    {
    case NIFTI_TYPE_INT8:
        this->m_ComponentType = CHAR;
        this->m_PixelType = SCALAR;
        break;
    case NIFTI_TYPE_UINT8:
        this->m_ComponentType = UCHAR;
        this->m_PixelType = SCALAR;
        break;
    case NIFTI_TYPE_INT16:
        this->m_ComponentType = SHORT;
        this->m_PixelType = SCALAR;
        break;
    case NIFTI_TYPE_UINT16:
        this->m_ComponentType = USHORT;
        this->m_PixelType = SCALAR;
        break;
    case NIFTI_TYPE_INT32:
        this->m_ComponentType = INT;
        this->m_PixelType = SCALAR;
        break;
    case NIFTI_TYPE_UINT32:
        this->m_ComponentType = UINT;
        this->m_PixelType = SCALAR;
        break;
    case NIFTI_TYPE_FLOAT32:
        this->m_ComponentType = FLOAT;
        this->m_PixelType = SCALAR;
        break;
    case NIFTI_TYPE_FLOAT64:
        this->m_ComponentType = DOUBLE;
        this->m_PixelType = SCALAR;
        break;
    case NIFTI_TYPE_COMPLEX64:
        this->m_ComponentType = FLOAT;
        this->m_PixelType = COMPLEX;
        this->dimc = 2;
        break;
    case NIFTI_TYPE_COMPLEX128:
        this->m_ComponentType = DOUBLE;
        this->m_PixelType = COMPLEX;
        this->dimc = 2;
        break;
    case NIFTI_TYPE_RGB24:
        this->m_ComponentType = UCHAR;
        this->m_PixelType = RGB;
        this->dimc = 3;
        break;
    case NIFTI_TYPE_RGBA32:
        this->m_ComponentType = UCHAR;
        this->m_PixelType = RGBA;
        this->dimc = 4;
        break;
    default:
        break;
    }

    // there are a wide variety of intents we ignore
    // but a few wee need to care about
    switch ( this->m_NiftiImage->intent_code )
    {
    case NIFTI_INTENT_NONE:
        break;
    case NIFTI_INTENT_CORREL:
        break;
    case NIFTI_INTENT_TTEST:
        break;
    case NIFTI_INTENT_FTEST:
        break;
    case NIFTI_INTENT_ZSCORE:
        break;
    case NIFTI_INTENT_CHISQ:
        break;
    case NIFTI_INTENT_BETA:
        break;
    case NIFTI_INTENT_BINOM:
        break;
    case NIFTI_INTENT_GAMMA:
        break;
    case NIFTI_INTENT_POISSON:
        break;
    case NIFTI_INTENT_NORMAL:
        break;
    case NIFTI_INTENT_FTEST_NONC:
        break;
    case NIFTI_INTENT_CHISQ_NONC:
        break;
    case NIFTI_INTENT_LOGISTIC:
        break;
    case NIFTI_INTENT_LAPLACE:
        break;
    case NIFTI_INTENT_UNIFORM:
        break;
    case NIFTI_INTENT_TTEST_NONC:
        break;
    case NIFTI_INTENT_WEIBULL:
        break;
    case NIFTI_INTENT_CHI:
        break;
    case NIFTI_INTENT_INVGAUSS:
        break;
    case NIFTI_INTENT_EXTVAL:
        break;
    case NIFTI_INTENT_PVAL:
        break;
    case NIFTI_INTENT_LOGPVAL:
        break;
    case NIFTI_INTENT_LOG10PVAL:
        break;
    case NIFTI_INTENT_ESTIMATE:
        break;
    case NIFTI_INTENT_LABEL:
        break;
    case NIFTI_INTENT_NEURONAME:
        break;
    case NIFTI_INTENT_GENMATRIX:
        break;
    case NIFTI_INTENT_SYMMATRIX:
        this->setPixelType(SYMMETRICSECONDRANKTENSOR);
        break;
    case NIFTI_INTENT_DISPVECT:
        break;
    case NIFTI_INTENT_VECTOR:
        this->setPixelType(VECTOR);
        break;
    case NIFTI_INTENT_POINTSET:
        break;
    case NIFTI_INTENT_TRIANGLE:
        break;
    case NIFTI_INTENT_QUATERNION:
        break;
    case NIFTI_INTENT_DIMLESS:
        break;
    case NIFTI_INTENT_TIME_SERIES:
        break;
    case NIFTI_INTENT_NODE_INDEX:
        break;
    case NIFTI_INTENT_RGB_VECTOR:
        break;
    case NIFTI_INTENT_RGBA_VECTOR:
        break;
    case NIFTI_INTENT_SHAPE:
        break;
    }

    // set slope/intercept
    if ( this->m_NiftiImage->qform_code == 0 && this->m_NiftiImage->sform_code == 0 )
    {
        this->m_RescaleSlope = 1;
        this->m_RescaleIntercept = 0;
    }
    else
    {
        if ( ( this->m_RescaleSlope = this->m_NiftiImage->scl_slope ) == 0 )
        {
            this->m_RescaleSlope = 1;
        }
        this->m_RescaleIntercept = this->m_NiftiImage->scl_inter;
    }

    //
    // if rescale is necessary, promote type reported
    // to ImageFileReader to float
    if ( this->mustRescale() )
    {
        if ( this->m_ComponentType == CHAR
             || this->m_ComponentType == UCHAR
             || this->m_ComponentType == SHORT
             || this->m_ComponentType == USHORT
             || this->m_ComponentType == INT
             || this->m_ComponentType == UINT
             || this->m_ComponentType == LONG
             || this->m_ComponentType == ULONG )
        {
            this->m_ComponentType = FLOAT;
        }
    }
    //
    // set up the dimension stuff
    double spacingscale = 1.0; //default to mm
    switch ( XYZT_TO_SPACE(this->m_NiftiImage->xyz_units) )
    {
    case NIFTI_UNITS_METER:
        spacingscale = 1e3;
        break;
    case NIFTI_UNITS_MM:
        spacingscale = 1e0;
        break;
    case NIFTI_UNITS_MICRON:
        spacingscale = 1e-3;
        break;
    }
    double timingscale = 1.0; //Default to seconds
    switch ( XYZT_TO_TIME(this->m_NiftiImage->xyz_units) )
    {
    case NIFTI_UNITS_SEC:
        timingscale = 1.0;
        break;
    case NIFTI_UNITS_MSEC:
        timingscale = 1e-3;
        break;
    case NIFTI_UNITS_USEC:
        timingscale = 1e-6;
        break;
    }

    switch ( dims )
    {
    case 7:
        //this->SetDimensions(6, this->m_NiftiImage->nw);
        //NOTE: Scaling is not defined in this dimension
        //this->SetSpacing(6, this->m_NiftiImage->dw);
    case 6:
        //this->SetDimensions(5, this->m_NiftiImage->nv);
        //NOTE: Scaling is not defined in this dimension
        //this->SetSpacing(5, this->m_NiftiImage->dv);
    case 5:
        this->setDimc(this->m_NiftiImage->nu);
        //NOTE: Scaling is not defined in this dimension
        this->dc = this->m_NiftiImage->du;
    case 4:
        this->setDimt(this->m_NiftiImage->nt);
        this->dt = this->m_NiftiImage->dt * timingscale;
    case 3:
        this->setDimz(this->m_NiftiImage->nz);
        this->dz = this->m_NiftiImage->dz * spacingscale;
    case 2:
        this->setDimy(this->m_NiftiImage->ny);
        this->dy = this->m_NiftiImage->dy * spacingscale;
    case 1:
        this->setDimx(this->m_NiftiImage->nx);
        this->dx = this->m_NiftiImage->dx * spacingscale;
        break;
    default:
        cout << this->getFileName() << " has " << dims << " dimensions, and is not supported or invalid!" <<endl;
        return false;
    }

    // copy image to buffer
    dimt = this->m_NiftiImage->dim[5]; // Vector dimt = 3

    unsigned long pixelSize = this->m_NiftiImage->nbyper;
    unsigned long imSize = dimx*dimy*dimz*dimc*dimt;

    switch ( this->m_ComponentType )
    {
    case CHAR:
        y_new< char, unsigned long >((char*&)buffer, imSize);
        break;
    case UCHAR:
        y_new< unsigned char, unsigned long >((unsigned char*&)buffer, imSize);
        break;
    case SHORT:
        y_new< short, unsigned long >((short*&)buffer, imSize);
        break;
    case USHORT:
        y_new< unsigned short, unsigned long >((unsigned short*&)buffer, imSize);
        break;
    case INT:
        y_new< int, unsigned long >((int*&)buffer, imSize);
        break;
    case UINT:
        y_new< unsigned int, unsigned long >((unsigned int*&)buffer, imSize);
        break;
    case LONG:
        y_new< long, unsigned long >((long*&)buffer, imSize);
        break;
    case ULONG:
        y_new< unsigned long, unsigned long >((unsigned long*&)buffer, imSize);
        break;
    case FLOAT:
        y_new< float, unsigned long >((float*&)buffer, imSize);
        break;
    case DOUBLE:
        y_new< double, unsigned long >((double*&)buffer, imSize);
        break;
    case UNKNOWNCOMPONENTTYPE:
        cout << "Bad OnDiskComponentType UNKNOWNCOMPONENTTYPE" <<endl;
        return false;
    }

    cout<<"dimensions ... "<<this->dimx<<" "<<this->dimy<<" "<<this->dimz<<" "<<this->dimc<<" "<<this->dimt <<" pixel size ... "<<pixelSize<<endl;

    const char *       niftibuf = reinterpret_cast<const char *>(this->m_NiftiImage->data);
    char *             p = (char *)buffer;

    const unsigned long rowdist = this->m_NiftiImage->dim[1];
    const unsigned long slicedist = rowdist * this->m_NiftiImage->dim[2];
    const unsigned long volumedist = slicedist * this->m_NiftiImage->dim[3];
    const unsigned long seriesdist = volumedist * this->m_NiftiImage->dim[4];

    for ( unsigned long t = 0; t < this->m_NiftiImage->dim[5]; t++ )
    {
        for ( unsigned long z = 0; z < this->m_NiftiImage->dim[3]; z++ )
        {
            for ( unsigned long y = 0; y < this->m_NiftiImage->dim[2]; y++ )
            {
                for ( unsigned long x = 0; x < this->m_NiftiImage->dim[1]; x++ )
                {
                    for ( unsigned long c = 0; c < dimc; c++ )
                    {
                        const unsigned long nifti_index = ( c * seriesdist + volumedist * t + slicedist * z + rowdist * y + x ) * pixelSize;
                        const unsigned long index =( ( t * volumedist * dimc  + volumedist * c + slicedist * z + rowdist * y + x )  ) * pixelSize;

                        memcpy(p + index, niftibuf + nifti_index, pixelSize);
                    }
                }
            }
        }
    }

    buffer = (void *)p;

    // de-alloc
    nifti_image_free(this->m_NiftiImage);
    this->m_NiftiImage = 0;

    //
    return true;
}

// writing
bool NiftiImageIO::canWriteFile(char *FileNameToWrite)
{
    const char * filename = reinterpret_cast<const char *>(FileNameToWrite);
    const int ValidFileNameFound = nifti_is_complete_filename(filename) > 0;

    this->setFileName(FileNameToWrite);

    return ValidFileNameFound;
}

void NiftiImageIO::write(const void *buffer, long sx, long sy, long sz, long sc, long st, int datatype)
{
    if ( this->m_NiftiImage == 0 )
    {
        this->m_NiftiImage = nifti_simple_init_nim();
    }

    const char * filename = reinterpret_cast<const char *>(this->getFileName());
    const char *tempextension = nifti_find_file_extension( filename );
    if ( tempextension == NULL )
    {
        cout << "Bad Nifti file name. No extension found for file: " << this->getFileName() <<endl;
    }
    const std::string ExtensionName(tempextension);

    const char *tempbasename = nifti_makebasename( filename );

    const std::string::size_type ext = ExtensionName.rfind(".gz");
    const bool IsCompressed = ( ext == std::string::npos ) ? false : true;

    bool b_nii = false; // hdr/img
    if ( ExtensionName == ".nii" || ExtensionName == ".nii.gz" )
    {
        this->m_NiftiImage->nifti_type = NIFTI_FTYPE_NIFTI1_1;
        b_nii = true;
    }
    else if ( ExtensionName == ".nia" )
    {
        this->m_NiftiImage->nifti_type = NIFTI_FTYPE_ASCII;
    }
    else if ( ExtensionName == ".hdr" || ExtensionName == ".img" || ExtensionName == ".hdr.gz" || ExtensionName == ".img.gz" )
    {
        this->m_NiftiImage->nifti_type = NIFTI_FTYPE_NIFTI1_2;
        //this->m_NiftiImage->nifti_type = NIFTI_FTYPE_ANALYZE; //NOTE: OREINTATION IS NOT WELL DEFINED IN THIS FORMAT.
    }
    else
    {
        cout << "Bad Nifti file name: " << this->getFileName() << endl;
        return;
    }
    this->m_NiftiImage->fname = nifti_makehdrname(tempbasename, this->m_NiftiImage->nifti_type, false, IsCompressed);
    this->m_NiftiImage->iname = nifti_makeimgname(tempbasename, this->m_NiftiImage->nifti_type, false, IsCompressed);

    //
    nifti_1_header  hdr;
    nifti1_extender pad={0,0,0,0};

    bzero((void *)&hdr, sizeof(hdr));
    hdr.sizeof_hdr = MIN_HEADER_SIZE;
    hdr.dim[0] = 5;
    this->m_NiftiImage->nx = hdr.dim[1] = sx;
    this->m_NiftiImage->ny = hdr.dim[2] = sy;
    this->m_NiftiImage->nz = hdr.dim[3] = sz;
    this->m_NiftiImage->nt = hdr.dim[4] = st;
    this->m_NiftiImage->du = hdr.dim[5] = sc;

    //this->m_NiftiImage->nvox = hdr.dim[dims]

    if(sc>1)
        this->m_NiftiImage->nvox = sc;
    else if (sz>1)
        this->m_NiftiImage->nvox = sz;
    else if (sy>1)
        this->m_NiftiImage->nvox = sy;
    else
        this->m_NiftiImage->nvox = sx;


    if(datatype == 1)
    {
        hdr.datatype = NIFTI_TYPE_UINT8;
        hdr.bitpix = 8;
    }
    else if(datatype == 2)
    {
        hdr.datatype = NIFTI_TYPE_UINT16;
        hdr.bitpix = 16;
    }
    else
    {
        hdr.datatype = NIFTI_TYPE_FLOAT32;
        hdr.bitpix = 32;
    }
    hdr.pixdim[1] = 1.0;
    hdr.pixdim[2] = 1.0;
    hdr.pixdim[3] = 1.0;
    hdr.pixdim[4] = 1.0;
    if (b_nii)
        hdr.vox_offset = (float) NII_HEADER_SIZE;
    else
        hdr.vox_offset = (float)0;
    hdr.scl_slope = 1.0f;
    hdr.xyzt_units = static_cast< int >( NIFTI_UNITS_MM | NIFTI_UNITS_SEC );
    if (b_nii)
        strncpy(hdr.magic, "n+1\0", 4);
    else
        strncpy(hdr.magic, "ni1\0", 4);

    // data
    size_t nbyper = (size_t)(hdr.bitpix/8);
    unsigned long numVoxels = sx*sy*sz*sc*st;
    unsigned long buffer_size = numVoxels*(unsigned long)nbyper;

    char *nifti_buf = NULL;
    try
    {
        nifti_buf = new char [buffer_size];
    }
    catch(...)
    {
        cout << "Fail to allocate memory" <<endl;
        return;
    }

    const char *const buf = reinterpret_cast<const char *>(buffer);

    // nifti_layout[vec][t][z][y][x]
    const unsigned long rowdist = (long)hdr.dim[1];
    const unsigned long slicedist = rowdist * (long)hdr.dim[2];
    const unsigned long volumedist = slicedist * (long)hdr.dim[3];
    const unsigned long seriesdist = volumedist * (long)hdr.dim[4];

    for ( unsigned long t = 0; t < hdr.dim[4]; t++ )
    {
        for ( unsigned long z = 0; z < hdr.dim[3]; z++ )
        {
            for ( unsigned long y = 0; y < hdr.dim[2]; y++ )
            {
                for ( unsigned long x = 0; x < hdr.dim[1]; x++ )
                {
                    for ( unsigned long c = 0; c < sc; c++ )
                    {
                        const unsigned long nifti_index = ( c * seriesdist + volumedist * t + slicedist * z + rowdist * y + x ) * nbyper;
                        const unsigned long index =( ( t * volumedist * sc  + volumedist * c + slicedist * z + rowdist * y + x )  ) * nbyper;

                        memcpy(nifti_buf + nifti_index, buf + index, nbyper);
                    }
                }
            }
        }
    }

    //
    FILE *fp = fopen(this->m_NiftiImage->fname,"w");
    if (fp == NULL)
    {
        fprintf(stderr, "\nError opening header file %s for write\n",this->m_NiftiImage->fname);
        exit(1);
    }
    long ret = fwrite(&hdr, MIN_HEADER_SIZE, 1, fp);
    if (ret != 1)
    {
        fprintf(stderr, "\nError writing header file %s\n",this->m_NiftiImage->fname);
        return;
    }
    if (b_nii == 1)
    {
        ret = fwrite(&pad, 4, 1, fp);
        if (ret != 1)
        {
            fprintf(stderr, "\nError writing header file extension pad %s\n",this->m_NiftiImage->fname);
            return;
        }

        ret = fwrite(nifti_buf, (size_t)(hdr.bitpix/8), numVoxels, fp);

	//cout<<"test ... "<<" \nret : "<<ret<<" \nnumVoxels : "<<numVoxels<<" \nsx: "<<sx<<" \nsy: "<<sy<<" \nsz: "<<sz<<" \nsc: "<<sc<<" \nst: "<<st;

        if (ret != numVoxels)
        {
            fprintf(stderr, "\nError writing data to %s\n",this->m_NiftiImage->fname);
            return;
        }

        fclose(fp);
    }
    else
    {
        fclose(fp);     // close .hdr file

        fp = fopen(this->m_NiftiImage->iname,"w");
        if (fp == NULL)
        {
            fprintf(stderr, "\nError opening data file %s for write\n",this->m_NiftiImage->iname);
            return;
        }
        ret = fwrite(nifti_buf, (size_t)(hdr.bitpix/8), numVoxels, fp);
        if (ret != numVoxels)
        {
            fprintf(stderr, "\nError writing data to %s\n",this->m_NiftiImage->iname);
            return;
        }

        fclose(fp);
    }

    // de-alloc
    if(nifti_buf) {delete[] nifti_buf; nifti_buf=0;}

    //
    return;
}

void NiftiImageIO::writeSampled(const void *buffer, long sx, long sy, long sz, long sc, long st, int datatype, float srx, float sry, float srz)
{
    if ( this->m_NiftiImage == 0 )
    {
        this->m_NiftiImage = nifti_simple_init_nim();
    }

    const char * filename = reinterpret_cast<const char *>(this->getFileName());
    const char *tempextension = nifti_find_file_extension( filename );
    if ( tempextension == NULL )
    {
        cout << "Bad Nifti file name. No extension found for file: " << this->getFileName() <<endl;
    }
    const std::string ExtensionName(tempextension);

    const char *tempbasename = nifti_makebasename( filename );

    const std::string::size_type ext = ExtensionName.rfind(".gz");
    const bool IsCompressed = ( ext == std::string::npos ) ? false : true;

    bool b_nii = false; // hdr/img
    if ( ExtensionName == ".nii" || ExtensionName == ".nii.gz" )
    {
        this->m_NiftiImage->nifti_type = NIFTI_FTYPE_NIFTI1_1;
        b_nii = true;
    }
    else if ( ExtensionName == ".nia" )
    {
        this->m_NiftiImage->nifti_type = NIFTI_FTYPE_ASCII;
    }
    else if ( ExtensionName == ".hdr" || ExtensionName == ".img" || ExtensionName == ".hdr.gz" || ExtensionName == ".img.gz" )
    {
        this->m_NiftiImage->nifti_type = NIFTI_FTYPE_NIFTI1_2;
        //this->m_NiftiImage->nifti_type = NIFTI_FTYPE_ANALYZE; //NOTE: OREINTATION IS NOT WELL DEFINED IN THIS FORMAT.
    }
    else
    {
        cout << "Bad Nifti file name: " << this->getFileName() << endl;
        return;
    }
    this->m_NiftiImage->fname = nifti_makehdrname(tempbasename, this->m_NiftiImage->nifti_type, false, IsCompressed);
    this->m_NiftiImage->iname = nifti_makeimgname(tempbasename, this->m_NiftiImage->nifti_type, false, IsCompressed);

    //
    nifti_1_header  hdr;
    nifti1_extender pad={0,0,0,0};

    bzero((void *)&hdr, sizeof(hdr));
    hdr.sizeof_hdr = MIN_HEADER_SIZE;
    hdr.dim[0] = 5;
    this->m_NiftiImage->nx = hdr.dim[1] = sx;
    this->m_NiftiImage->ny = hdr.dim[2] = sy;
    this->m_NiftiImage->nz = hdr.dim[3] = sz;
    this->m_NiftiImage->nt = hdr.dim[4] = st;
    this->m_NiftiImage->nu = hdr.dim[5] = sc;

    //this->m_NiftiImage->nvox = hdr.dim[dims]

    if(sc>1)
        this->m_NiftiImage->nvox = sc;
    else if (sz>1)
        this->m_NiftiImage->nvox = sz;
    else if (sy>1)
        this->m_NiftiImage->nvox = sy;
    else
        this->m_NiftiImage->nvox = sx;

    if(datatype == 1)
    {
        hdr.datatype = NIFTI_TYPE_UINT8;
        hdr.bitpix = 8;
    }
    else if(datatype == 2)
    {
        hdr.datatype = NIFTI_TYPE_UINT16;
        hdr.bitpix = 16;
    }
    else // datatype == 4
    {
        hdr.datatype = NIFTI_TYPE_FLOAT32;
        hdr.bitpix = 32;
    }
    hdr.pixdim[1] = 1.0;
    hdr.pixdim[2] = 1.0;
    hdr.pixdim[3] = 1.0;
    hdr.pixdim[4] = 1.0;
    if (b_nii)
        hdr.vox_offset = (float) NII_HEADER_SIZE;
    else
        hdr.vox_offset = (float)0;
    hdr.scl_slope = 1.0f/srx;
    hdr.xyzt_units = static_cast< int >( NIFTI_UNITS_MM | NIFTI_UNITS_SEC );
    if (b_nii)
        strncpy(hdr.magic, "n+1\0", 4);
    else
        strncpy(hdr.magic, "ni1\0", 4);

    // data
    size_t nbyper = (size_t)(hdr.bitpix/8);
    unsigned long numVoxels = sx*sy*sz*sc*st;
    unsigned long buffer_size = numVoxels*(unsigned long)nbyper;

    char *nifti_buf = NULL;
    try
    {
        nifti_buf = new char [buffer_size];
    }
    catch(...)
    {
        cout << "Fail to allocate memory" <<endl;
        return;
    }

    const char *const buf = reinterpret_cast<const char *>(buffer);

    // nifti_layout[vec][t][z][y][x]
    const unsigned long rowdist = (long)hdr.dim[1];
    const unsigned long slicedist = rowdist * (long)hdr.dim[2];
    const unsigned long volumedist = slicedist * (long)hdr.dim[3];
    const unsigned long seriesdist = volumedist * (long)hdr.dim[4];

    for ( unsigned long t = 0; t < hdr.dim[4]; t++ )
    {
        for ( unsigned long z = 0; z < hdr.dim[3]; z++ )
        {
            for ( unsigned long y = 0; y < hdr.dim[2]; y++ )
            {
                for ( unsigned long x = 0; x < hdr.dim[1]; x++ )
                {
                    for ( unsigned long c = 0; c < sc; c++ )
                    {
                        const unsigned long nifti_index = ( c * seriesdist + volumedist * t + slicedist * z + rowdist * y + x ) * nbyper;
                        const unsigned long index =( ( t * volumedist * sc  + volumedist * c + slicedist * z + rowdist * y + x )  ) * nbyper;

                        memcpy(nifti_buf + nifti_index, buf + index, nbyper);
                    }
                }
            }
        }
    }

    //
    FILE *fp = fopen(this->m_NiftiImage->fname,"w");
    if (fp == NULL)
    {
        fprintf(stderr, "\nError opening header file %s for write\n",this->m_NiftiImage->fname);
        exit(1);
    }
    long ret = fwrite(&hdr, MIN_HEADER_SIZE, 1, fp);
    if (ret != 1)
    {
        fprintf(stderr, "\nError writing header file %s\n",this->m_NiftiImage->fname);
        return;
    }
    if (b_nii == 1)
    {
        ret = fwrite(&pad, 4, 1, fp);
        if (ret != 1)
        {
            fprintf(stderr, "\nError writing header file extension pad %s\n",this->m_NiftiImage->fname);
            return;
        }

        ret = fwrite(nifti_buf, (size_t)(hdr.bitpix/8), numVoxels, fp);

    //cout<<"test ... "<<" \nret : "<<ret<<" \nnumVoxels : "<<numVoxels<<" \nsx: "<<sx<<" \nsy: "<<sy<<" \nsz: "<<sz<<" \nsc: "<<sc<<" \nst: "<<st;

        if (ret != numVoxels)
        {
            fprintf(stderr, "\nError writing data to %s\n",this->m_NiftiImage->fname);
            return;
        }

        fclose(fp);
    }
    else
    {
        fclose(fp);     // close .hdr file

        fp = fopen(this->m_NiftiImage->iname,"w");
        if (fp == NULL)
        {
            fprintf(stderr, "\nError opening data file %s for write\n",this->m_NiftiImage->iname);
            return;
        }
        ret = fwrite(nifti_buf, (size_t)(hdr.bitpix/8), numVoxels, fp);
        if (ret != numVoxels)
        {
            fprintf(stderr, "\nError writing data to %s\n",this->m_NiftiImage->iname);
            return;
        }

        fclose(fp);
    }

    // de-alloc
    if(nifti_buf) {delete[] nifti_buf; nifti_buf=0;}

    //
    return;
}

