#include "synapseresamplerdialog.h"
#include "ui_synapseresamplerdialog.h"

#include "itkImage.h"
#include "itkVectorImage.h"
#include "itkImageFileReader.h"

#include "itkExtractImageFilter.h"
#include "itkFixedCenterOfRotationAffineTransform.h"
#include "itkBSplineInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkWindowedSincInterpolateImageFunction.h"
#include "itkResampleImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkMirrorPadImageFilter.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>

typedef itk::Image<unsigned char, 3>  	ImageType;
typedef itk::ImageFileReader<ImageType> ReaderType;
typedef itk::ImageFileWriter<ImageType> WriterType;

typedef itk::VectorImage<float, 3>      OrientImageType;

/* Generate a (non-unique) coordinate rotation matrix, so that z-axis in the
   coordinates corresponds to direction (x, y, z) in the original coordinates.
   This one uses rotation about y-axis and then z-axis.
 */
static itk::Matrix<double,3,3> genRotMtx_001_to_xyz(double x, double y, double z)
{
        double r = sqrt(x * x + y * y + z * z);
        if(r == 0) {
                std::string err("[genRotMtx_001_to_xyz] Error: x, y, z are all zeros!\n");
                fprintf(stderr, "%s\n", err.c_str());
                throw err;
        }
        x = x / r;
        y = y / r;
        z = z / r;

        double cos_ay = z;
        double sin_ay = sqrt(1 - z * z);
        double cos_az = x / sin_ay;
        double sin_az = y / sin_ay;

        itk::Matrix<double,3,3> Ry, Rz, R;

        Ry.Fill(0);
        Ry(0, 0) = cos_ay;
        Ry(0, 2) = sin_ay;
        Ry(1, 1) = 1;
        Ry(2, 0) = -sin_ay;
        Ry(2, 2) = cos_ay;

        Rz.Fill(0);
        Rz(0, 0) = cos_az;
        Rz(0, 1) = -sin_az;
        Rz(1, 0) = sin_az;
        Rz(1, 1) = cos_az;
        Rz(2, 2) = 1;

        R = Rz * Ry;
        return R;
}


/* Same as above, but give x, y, z as array [x, y, z].
 */
template <class T>
itk::Matrix<double,3,3> genRotMtx_001_to_xyz(const T *v)
{
        return genRotMtx_001_to_xyz(v[0], v[1], v[2]);
}

// some interpolator typedefs
//typedef itk::BSplineInterpolateImageFunction            BSplineInterpolator;
//typedef itk::NearestNeighborInterpolateImageFunction    NNInterpolator;

/* Extract a subwindow of size specified by radii 'rad' centered at location
   'loc' and orientation 'orient'.  The output will be rotated so that the
   z-axis of the extracted window point to 'orient' direction in the original
   image (volume).
   - Assume the volume (i.e. "image") is sufficiently padded.
 */
template <class TInputImagePointer, class TOutputImage, class InterpolatorType>
typename TOutputImage::Pointer
extractZRotatedSubvolume(const TInputImagePointer inputImage,
                                                 double loc_x, double loc_y, double loc_z,
                                                 double orient_x, double orient_y, double orient_z,
                                                 unsigned long rad_x, unsigned long rad_y, unsigned long rad_z, bool inverseProjection = false)
{
        typedef typename TInputImagePointer::ObjectType		InputImageType;
        typedef TOutputImage								OutputImageType;
        //typedef itk::Image<float,3>							FloatImageType;
        typedef typename InputImageType::IndexType			IndexType;
        typedef typename InputImageType::SizeType			SizeType;
        typedef typename InputImageType::RegionType			RegionType;
        typedef itk::ExtractImageFilter<InputImageType,InputImageType>			ExtractorType;
        typedef itk::ResampleImageFilter<InputImageType,OutputImageType>		ResamplerType;
        typedef itk::FixedCenterOfRotationAffineTransform<double,3>				TransformType;
        //typedef InterpolatorTemplate<InputImageType>			InterpolatorType;
        //typedef itk::WindowedSincInterpolateImageFunction<InputImageType,4>		InterpolatorType;

        assert(InputImageType::ImageDimension == 3);  // 3D volumes only

        itk::Point<double,3> loc;
        loc[0] = loc_x; loc[1] = loc_y; loc[2] = loc_z;

        // transform
        TransformType::Pointer transform = TransformType::New();
        itk::Point<double,3> center;
        //itk::Vector<double,3> shift;
        for(int i=0; i<3; i++) {
                center[i] = loc[i]; // - origin1[i];
        }
        transform->SetCenterOfRotationComponent(center);
        itk::Matrix<double,3,3> rotMtx = genRotMtx_001_to_xyz(orient_x, orient_y, orient_z);
        transform->SetMatrixComponent(rotMtx);
        //std::cout << "rotMtx:\n" << rotMtx << std::endl; //debug

        // interpolator
        typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
        //interpolator->SetSplineOrder(3);

        // rotation by resampling
        typename ResamplerType::Pointer resampler = ResamplerType::New();
        resampler->SetDefaultPixelValue( 0 );

        if (inverseProjection) {
            resampler->SetTransform( (const TransformType *) transform->GetInverseTransform().GetPointer() );
        }
        else
            resampler->SetTransform(transform);

        //resampler->SetInterpolator(interpolator);  // default is LinearInterpolateImageFunction

        resampler->SetInput(inputImage);
        resampler->SetInterpolator(interpolator);

        resampler->SetOutputStartIndex( resampler->GetInput()->GetLargestPossibleRegion().GetIndex() );
        resampler->SetSize( resampler->GetInput()->GetLargestPossibleRegion().GetSize() );
        resampler->SetOutputOrigin(  resampler->GetInput()->GetOrigin() );
        resampler->SetOutputSpacing( resampler->GetInput()->GetSpacing() );
        resampler->SetOutputDirection( resampler->GetInput()->GetDirection() );

        //debug
        //std::cout << "resampler->GetInput()->GetBufferedRegion().GetIndex():\n" << resampler->GetInput()->GetBufferedRegion().GetIndex() << std::endl;
        //std::cout << "resampler->GetInput()->GetBufferedRegion().GetSize():\n" << resampler->GetInput()->GetBufferedRegion().GetSize() << std::endl;
        //std::cout << "resampler->GetOutputStartIndex():\n" << resampler->GetOutputStartIndex() << std::endl;
        //std::cout << "resampler->GetSize():\n" << resampler->GetSize() << std::endl;
        //resampler->Update();
        //std::cout << "resampler->GetOutput()->GetBufferedRegion():\n" << resampler->GetOutput()->GetBufferedRegion() << std::endl;
        //std::cout << "resampler->GetInput()->GetBufferedRegion():\n" << resampler->GetInput()->GetBufferedRegion() << std::endl;
        //std::cout << "resampler->GetOutput()->GetBufferedRegion():\n" << resampler->GetOutput()->GetBufferedRegion() << std::endl;
        //std::cout << "resampler->GetOutput()->GetLargestPossibleRegion():\n" << resampler->GetOutput()->GetLargestPossibleRegion() << std::endl;
        //std::cout << "resampler->GetOutput()->GetRequestedRegion():\n" << resampler->GetOutput()->GetRequestedRegion() << std::endl;

        // finally extract the wanted subvolume
#if 0
        typename ExtractorType::Pointer extractor2 = ExtractorType::New();
        unsigned long dstWindowRad[3] = {rad_x, rad_y, rad_z};
        IndexType origin2;
        SizeType size2;
        for(int i=0; i<3; i++) {
                origin2[i] = (long)(center[i]+0.5) - (long)dstWindowRad[i];
                size2[i] = 2 * dstWindowRad[i] + 1;
        }
        RegionType region2(origin2, size2);
        extractor2->SetExtractionRegion(region2);
        extractor2->SetInput(resampler->GetOutput());

        //debug
        //std::cout << "region2:\n" << region2 << std::endl;

        extractor2->Update();

        //debug
        //std::cout << "extractor2->GetOutput()->GetBufferedRegion():\n" << extractor2->GetOutput()->GetBufferedRegion() << std::endl;

        typename TOutputImage::Pointer outputImage = extractor2->GetOutput();
#else
        resampler->UpdateLargestPossibleRegion();
        typename TOutputImage::Pointer outputImage = resampler->GetOutput();
#endif
        return outputImage;
}


SynapseResamplerDialog::SynapseResamplerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SynapseResamplerDialog)
{
    ui->setupUi(this);

    ui->butBack->setEnabled(false);
    ui->butFwd->setEnabled(false);
}

void SynapseResamplerDialog::setAuxData( const Matrix3D<PixelType> *rawImg, Matrix3D<LabelType> *labelImg )
{
    mRawImg = rawImg;
    mLabelImg = labelImg;
}

void SynapseResamplerDialog::mainWindowClicked(unsigned int px, unsigned int py, unsigned int pz)
{
    static OrientImageType::Pointer  orientImg = 0;

    if (orientImg.IsNull())
    {
        QMessageBox::information( this, "Orientation image", "No orientation image was opened previously. Please open one in order to continue." );

        QString fileName = QFileDialog::getOpenFileName( 0, "Load orientation image", ".", "*.nrrd" );

        if (fileName.isEmpty())
            return;

        std::string stdFName = fileName.toLocal8Bit().constData();

        itk::ImageFileReader< OrientImageType >::Pointer reader = itk::ImageFileReader< OrientImageType >::New();
        reader->SetFileName(stdFName);
        reader->Update();

        orientImg = reader->GetOutput();
    }

    // get orientation at the given point
    OrientImageType::IndexType  index;
    index[0] = px; index[1] = py; index[2] = pz;

    float vx = orientImg->GetPixel(index)[0];
    float vy = orientImg->GetPixel(index)[1];
    float vz = orientImg->GetPixel(index)[2];

    qDebug("Orient estimate: %.2f %.2f %.2f", vx, vy, vz);


    // save image
    {
        // image is interpolated with bspline interpolation
        ImageType::Pointer rotImg = extractZRotatedSubvolume<ImageType::Pointer, ImageType, itk::BSplineInterpolateImageFunction<ImageType> >(
                                        mRawImg->asItkImage(), px, py, pz, vx, vy, vz, 0, 0, 0, false );

        WriterType::Pointer writer = WriterType::New();

        writer->SetInput(rotImg);
        writer->SetFileName("/tmp/rotated.tif");
        writer->Update();
    }

    // save annot
    {
        // interpolated with NN
        ImageType::Pointer rotImg = extractZRotatedSubvolume<ImageType::Pointer, ImageType, itk::LinearInterpolateImageFunction<ImageType> >(
                                        mLabelImg->asItkImage(), px, py, pz, vx, vy, vz, 0, 0, 0, false );

        WriterType::Pointer writer = WriterType::New();

        writer->SetInput(rotImg);
        writer->SetFileName("/tmp/rotated-label.tif");
        writer->Update();
    }

    // run!
    QStringList args;
    args << QString("/tmp/rotated.tif");    // file itself
    args << QString("/tmp/rotated-label.tif");
    args << QString("%1").arg(pz);          // z-value
    args << QString("yes");

    // this will wait until the process finishes
    QProcess::execute( QFileInfo( qApp->argv()[0] ).absoluteFilePath(), args );

    // so now load the annotation and perform the inverse transformation on the labels image
    {
        Matrix3D<LabelType> newLabels;
        newLabels.load("/tmp/rotated-label.tif");

        ImageType::Pointer rotImg = extractZRotatedSubvolume<ImageType::Pointer, ImageType, itk::LinearInterpolateImageFunction<ImageType> >(
                                        newLabels.asItkImage(), px, py, pz, vx, vy, vz, 0, 0, 0, true );

        // copy to labels
        mLabelImg->copyFrom( rotImg );
    }
}

SynapseResamplerDialog::~SynapseResamplerDialog()
{
    delete ui;
}
