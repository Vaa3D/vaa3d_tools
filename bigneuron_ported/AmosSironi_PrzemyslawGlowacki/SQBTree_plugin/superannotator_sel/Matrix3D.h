#ifndef MATRIX3D_H
#define MATRIX3D_H

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

// these are used to detect connected components
#include <itkBinaryThresholdImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkBinaryImageToLabelMapFilter.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include "ShapeStatistics.h"
#include <vector>

#ifdef QT_VERSION_STR
    #include <QImage>
#endif

template<typename T>
class Matrix3D
{
public:
    typedef itk::Image<T, 3> ItkImageType;
private:
    typedef unsigned int     DefaultLabelType;
    
    Matrix3D( const Matrix3D<T>& rhs );
	Matrix3D<T>& operator=( const Matrix3D<T>& rhs );
	
public:
    typedef T DataType;
    Matrix3D() { mData = 0; mWidth = mHeight = mDepth = 0; updateCache(); mKeepOnDestr = false; }
    
    // empty, just garbage data
    Matrix3D( unsigned int w, unsigned int h, unsigned int d ) {
        mData = 0; mKeepOnDestr = false;
        realloc(w,h,d);
    }

    // returns true if coords are of a valid pixel in the image
    inline bool pixIsInImage( unsigned x, unsigned y, unsigned z )
    {
        if (x < 0)  return false;
        if (y < 0)  return false;
        if (z < 0)  return false;

        if ( x >= mWidth )  return false;
        if ( y >= mHeight )  return false;
        if ( z >= mDepth )  return false;

        return true;
    }

    // creates a binary image based on thresholding btw [thrMin, thrMax]
    // and creates a connected component map
    template<typename T2>
    void createLabelMap( T thrMin, T thrMax, Matrix3D<T2> *labelImg = 0, bool bFullyConnected = true, T2 *labelCount = 0,
                         std::vector<ShapeStatistics<itk::ShapeLabelObject<T2, 3> > > *shapeDescr = 0,
                         bool bComputeFeretAndPerimeter = false)
    {
        typedef itk::Image<T2, 3> LabelImageType;
        typedef itk::BinaryThresholdImageFilter < ItkImageType, ItkImageType> BinaryThresholdImageFilterType;
        typedef itk::ConnectedComponentImageFilter< ItkImageType, LabelImageType >	 ConnectedCompFilterType;

        typename BinaryThresholdImageFilterType::Pointer thresholdFilter
                = BinaryThresholdImageFilterType::New();

        thresholdFilter->SetInput( asItkImage() );
        thresholdFilter->SetLowerThreshold(thrMin);
        thresholdFilter->SetUpperThreshold(thrMax);
        thresholdFilter->SetInsideValue(255);
        thresholdFilter->SetOutsideValue(0);

        typename ItkImageType::Pointer thrImage = thresholdFilter->GetOutput();

        // label connected components
        typename ConnectedCompFilterType::Pointer CCFilter = ConnectedCompFilterType::New();
        CCFilter->SetInput( thrImage );
        CCFilter->SetFullyConnected( bFullyConnected );

        CCFilter->Update();

        // get output
        typename LabelImageType::Pointer resImg = CCFilter->GetOutput();


        // now copy----
        if (labelImg)
        {
            // allocate labelImg
            labelImg->reallocSizeLike( *this );

            const T2 * srcPtr = Matrix3D<T2>::getItkImageDataPtr( resImg );
            for (unsigned int i=0; i < labelImg->numElem(); i++)
                labelImg->data()[i] = srcPtr[i];
        }

        if (labelCount != 0)
            *labelCount = CCFilter->GetObjectCount();

        if ( shapeDescr )
        {
            // then compute descriptions for each object
            typedef typename itk::LabelImageToShapeLabelMapFilter< LabelImageType >			 LabelToShapeMapFilter;

            // now get shape characteristics
            typename LabelToShapeMapFilter::Pointer shapeLabelMapFilter = LabelToShapeMapFilter::New();

            //shapeLabelMapFilter->SetBackgroundValue(0);
            shapeLabelMapFilter->SetInput( resImg );
            //shapeLabelMapFilter->SetInput2( reader->GetOutput() );

            // this is 'expensive' but may help, otherwise most features outputted will be blank
            shapeLabelMapFilter->SetComputeFeretDiameter(bComputeFeretAndPerimeter);
            shapeLabelMapFilter->SetComputePerimeter(bComputeFeretAndPerimeter);

            shapeLabelMapFilter->Update();


            shapeDescr->clear();

            // Loop over all of the blobs
            for(unsigned int i = 1; i <= shapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++)
            {
                    typename LabelToShapeMapFilter::OutputImageType::LabelObjectType::Pointer labelObject = shapeLabelMapFilter->GetOutput()->GetLabelObject(i);

                    // Output the bounding box (an example of one possible property) of the ith region
                    //std::cout << "Object " << i << " has bounding box " << labelObject->GetBoundingBox() << std::endl;

                    //writeShapeInfoTxt( stdout, i, labelObject );
                    shapeDescr->push_back( ShapeStatistics<itk::ShapeLabelObject<T2, 3> >( labelObject, i ) );
            }
        }
    }

    // generates a cropped region in the destination matrix
    inline void cropRegion( unsigned int sX, unsigned int sY, unsigned int sZ,
                            unsigned int w, unsigned int h, unsigned int d,
                            Matrix3D<T> *dest) const
    {
        dest->realloc( w, h, d );

        unsigned int endZ = sZ + d;
        unsigned int endY = sY + h;
        unsigned int stXY = sX;
        unsigned int edXY = stXY + w;

        unsigned int szCount = 0;
        for(; sZ < endZ; sZ++ )
        {
            const T * srcZ = sliceData(sZ);
            unsigned int yCount = 0;
            for (unsigned int y=sY; y < endY; y++)
            {
                std::copy( srcZ + stXY + y*mWidth, srcZ + y*mWidth + edXY, dest->sliceData(szCount) + w*yCount );
                yCount++;
            }

            szCount++;
        }
    }

    // returns an idx list of the pixels with the given value
    void findPixelWithvalue( T val, std::vector<unsigned int> &idx )
    {
        idx.clear();
        for (unsigned int i=0; i < numElem(); i++)
            if ( data()[i] == val )
                idx.push_back( i );
    }

    // empty, just garbage data
    inline void realloc( unsigned int w, unsigned int h, unsigned int d ) {
        freeData();

        mWidth = w;
        mHeight = h;
        mDepth = d;

        allocateEmpty();
        mKeepOnDestr = false;
        updateCache();
    }

    void copyFrom( typename ItkImageType::Pointer img )
    {
        typename ItkImageType::IndexType index;
        index[0] = index[1] = index[2] = 0;

        typename ItkImageType::SizeType imSize = img->GetLargestPossibleRegion().GetSize();
        mWidth = imSize[0];
        mHeight = imSize[1];
        mDepth = imSize[2];

        allocateEmpty();
        mKeepOnDestr = false;
        updateCache();

        memcpy( mData, &img->GetPixel( index ), sizeof(T) * numElem() );
    }

    void copyFrom( const Matrix3D<T> &other )
    {
        freeData();

        mWidth = other.width();
        mHeight = other.height();
        mDepth = other.depth();

        allocateEmpty();
        mKeepOnDestr = false;

        updateCache();

        memcpy( mData, other.data(), sizeof(T) * numElem() );
    }

    // sets every element to have value 'val'
    inline void fill( T val ) {
        for (unsigned int i=0; i < mNumElem; i++)
            mData[i] = val;
    }

    // calls realloc, copying the size from matrix m
    template<typename K>
    inline void reallocSizeLike( const Matrix3D<K> &m ) {
        realloc( m.width(), m.height(), m.depth() );
    }

    // existing pointer, WILL NOT DELETE DATA ON EXIT!
    Matrix3D( T *data, unsigned int w, unsigned int h, unsigned int d ) {
        mWidth = w;
        mHeight = h;
        mDepth = d;
        mData = data;

        mKeepOnDestr = true;
        updateCache();
    }

    inline void updateCache() { mNumElem = mWidth*mHeight*mDepth; mSz = mWidth*mHeight; }

    inline T *data() { return mData; }
    inline const T *data() const { return mData; }

    inline unsigned int coordToIdx( unsigned int x, unsigned int y, unsigned int z ) const
    {
        return x + y*mWidth + z*mSz;
    }

    // converts idx to (x,y,z)
    inline void idxToCoord( unsigned int idx, unsigned int &x, unsigned int &y, unsigned int &z ) const
    {
        unsigned int modA = idx % mSz;

        x = modA % mWidth;
        y = (( idx - x ) % mSz) / mWidth;
        z = (idx - x - y*mWidth) / mSz;
    }

    inline T& operator () (unsigned int x, unsigned int y, unsigned int z) {
        return mData[coordToIdx(x,y,z)];
    }

    inline const T& operator () (unsigned int x, unsigned int y, unsigned int z) const {
        return mData[coordToIdx(x,y,z)];
    }

    inline bool isEmpty() const { return mData == 0; }
    inline unsigned int numElem() const { return mNumElem; }

    template<typename T2>
    inline bool isSizeLike( const Matrix3D<T2> &m ) const {
        return (m.width()==width()) && (m.height() == height()) && (m.depth() == depth());
    }

    inline void freeData() {
        //qDebug("Free");
        if ((mData != 0) && (mKeepOnDestr == false)) {
            delete[] mData;

            mData = 0;
            mHeight = mWidth = mDepth = 0;
        } else {
            //if(mKeepOnDestr)
              //  qDebug("Matrix memory not freed.");
        }
    }


    inline unsigned int    width() const { return mWidth; }
    inline unsigned int    height() const { return mHeight; }
    inline unsigned int    depth() const { return mDepth; }

    ~Matrix3D()
    {
        freeData();
    }

    static bool getFileDimensions( const std::string &fName, unsigned int &w, unsigned int &h, unsigned int &d )
    {
        try
        {
            typename itk::ImageFileReader<ItkImageType>::Pointer reader = itk::ImageFileReader<ItkImageType>::New();
            reader->SetFileName( fName );
            //reader->Update();

            typename ItkImageType::Pointer img = reader->GetOutput();

            typename ItkImageType::SizeType imSize = img->GetLargestPossibleRegion().GetSize();

            w = imSize[0];
            h = imSize[1];
            d = imSize[2];
        }
        catch(std::exception &e)
        {
            return false;
        }

        return true;
    }

    void loadItkImage( ItkImageType* img )
	{
		freeData();
		
		typename ItkImageType::IndexType index;
		index[0] = index[1] = index[2] = 0;

		mData = &img->GetPixel( index );

		//img->Register();    //so it won't delete the data ;)
		img->GetPixelContainer()->SetContainerManageMemory(false);

		typename ItkImageType::SizeType imSize = img->GetLargestPossibleRegion().GetSize();
		mWidth = imSize[0];
		mHeight = imSize[1];
		mDepth = imSize[2];

		updateCache();
		mKeepOnDestr = false;
	}
    
    bool load( const std::string &fName )  // load from file
    {
        try
        {
            freeData(); // free before

            typename itk::ImageFileReader<ItkImageType>::Pointer reader = itk::ImageFileReader<ItkImageType>::New();
            reader->SetFileName( fName );
            reader->Update();

            typename ItkImageType::Pointer img = reader->GetOutput();

            typename ItkImageType::IndexType index;
            index[0] = index[1] = index[2] = 0;

            mData = &img->GetPixel( index );

            //img->Register();    //so it won't delete the data ;)
            img->GetPixelContainer()->SetContainerManageMemory(false);

            typename ItkImageType::SizeType imSize = img->GetLargestPossibleRegion().GetSize();
            mWidth = imSize[0];
            mHeight = imSize[1];
            mDepth = imSize[2];

            updateCache();
            mKeepOnDestr = false;
        }
        catch(std::exception &e)
        {
            return false;
        }

        return true;
    }

    // get raw pointer from itkImage
    static const T* getItkImageDataPtr( typename ItkImageType::Pointer ptr  )
    {
        typename ItkImageType::IndexType index;
        index[0] = index[1] = index[2] = 0;

        return &ptr->GetPixel(index);
    }

    // creates an ItkImage pointer to represent the data in this volume
    typename ItkImageType::Pointer asItkImage() const
    {
        typename ItkImageType::Pointer itkImg = ItkImageType::New();

        typename ItkImageType::PixelContainer::Pointer pixContainer = ItkImageType::PixelContainer::New();
        pixContainer->SetImportPointer( mData, mNumElem );

        itkImg->SetPixelContainer( pixContainer );

        typename ItkImageType::IndexType imStart;
        imStart[0] = imStart[1] = imStart[2] = 0;

        typename ItkImageType::SizeType imSize;
        imSize[0] = mWidth;
        imSize[1] = mHeight;
        imSize[2] = mDepth;

        typename ItkImageType::RegionType imRegion;
        imRegion.SetSize( imSize );
        imRegion.SetIndex( imStart );

        itkImg->SetRegions(imRegion);

        return itkImg;
    }


public:
    bool save( const std::string &fName ) const
    {
        typedef itk::Image<T, 3> ItkImageType;

        try
        {
            typename ItkImageType::Pointer itkImg = asItkImage();

#if 0
            typename ItkImageType::PixelContainer::Pointer pixContainer = ItkImageType::PixelContainer::New();
            pixContainer->SetImportPointer( mData, mNumElem );

            itkImg->SetPixelContainer( pixContainer );

            typename ItkImageType::SizeType imSize;
            imSize[0] = mWidth;
            imSize[1] = mHeight;
            imSize[2] = mDepth;

            itkImg->SetRegions(imSize);
#endif

            typename itk::ImageFileWriter<ItkImageType>::Pointer writer = itk::ImageFileWriter<ItkImageType>::New();
            writer->SetFileName(fName);
            writer->SetInput(itkImg);

            writer->Update();
        }
        catch( std::exception &e )
        {
            return false;
        }

        return true;
    }

    // this is for a given Z-slice
    inline const T *sliceData(unsigned int z) const
    {
        if (isEmpty())
            #ifdef QT_VERSION_STR
                qFatal("Tried to get slice from empty ImageVolume");
            #else
                return (T *)0;
            #endif

        if (z >= mDepth)
            z = mDepth-1;

        return mData + z*mWidth*mHeight;
    }

    inline T *sliceData(unsigned int z)
    {
        if (isEmpty())
            #ifdef QT_VERSION_STR
                qFatal("Tried to get slice from empty ImageVolume");
            #else
                return (T *)0;
            #endif

        if (z >= mDepth)
            z = mDepth-1;

        return mData + z*mWidth*mHeight;
    }

#ifdef QT_VERSION_STR
    // sliceCoord 0..2
    inline void QImageSlice( unsigned int z, QImage &qimg ) const
    {
        qimg = QImage( width(), height(), QImage::Format_RGB32 );
        unsigned int sz = width()*height();

        unsigned int *dataPtr = (unsigned int *) qimg.constBits(); //trick!

        const unsigned char *p = sliceData(z);

        for (unsigned int i=0; i < sz; i++) {
            unsigned int D = p[i];
            dataPtr[i] = D | (D<<8) | (D<<16) | (0xFF<<24);
        }
    }
#endif

    // if all elemenst are equal
    bool operator ==(const Matrix3D<T>& b) const
    {
        for (unsigned int i=0; i < mNumElem; i++)
        {
            if ( b.data()[i] != data()[i] ){
                //qDebug(" != at %d", (int)i );
                return false;
            }
        }

        return true;
    }

    void set( int x, int y, int z, typename ItkImageType::PixelType value )
    {
        //Assuming coords valid
        mData[x + y*mWidth + z*mWidth*mHeight] = value;
    }

private:
    T *mData;
    unsigned int mWidth, mHeight, mDepth;
    unsigned int mSz; // this is a cached version of mWidth*mHeight
    unsigned int mNumElem;
    bool    mKeepOnDestr;  // if data should not be deleted upon object destruction

    void allocateEmpty() {
        mSz = mWidth*mHeight;
        mNumElem = mSz * mDepth;

        mData = new T[mNumElem];
    }
};


#endif // MATRIX3D_H
