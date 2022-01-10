/*=========================================================================
Copyright 2009 Rensselaer Polytechnic Institute
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 
=========================================================================*/
#ifndef NEW_IMAGEACTORS_H_
#define NEW_IMAGEACTORS_H_

#include <QVTKWidget.h>

#include "vtkCallbackCommand.h"
#include "vtkCellArray.h"
#include "vtkCellPicker.h"
#include "vtkCommand.h"
#include "vtkCubeSource.h"
#include "vtkFloatArray.h"
#include "vtkGlyph3D.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkInteractorStyleRubberBandZoom.h"
#include "vtkInteractorStyleImage.h"
#include <vtkImagePlaneWidget.h>
#include "vtkPiecewiseFunction.h"
#include "vtkPlaybackRepresentation.h"
#include "vtkPlaybackWidget.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkCamera.h"
#include "vtkSliderRepresentation2D.h"
#include "vtkSliderWidget.h"
#include "vtkSphereSource.h"
#include "vtkPointWidget.h"

#include "vtkBoundingBox.h"
#include "vtkAxesActor.h"
#include "vtkTransform.h"

#include "vtkActor.h"
#include "vtkContourFilter.h"
#include "vtkColorTransferFunction.h"
#include "vtkImageData.h"
#include "vtkImageToStructuredPoints.h"
#include "vtkImageActor.h"
#include "vtkLODActor.h"
#include "vtkOpenGLVolumeTextureMapper3D.h"
#include "itkMaximumProjectionImageFilter.h"
#include "itkMinimumProjectionImageFilter.h"
#include "itkMeanProjectionImageFilter.h"
#ifdef USE_GPUREN
#include <vtkGPUVolumeRayCastMapper.h>
#endif
#include "vtkPiecewiseFunction.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkSmartPointer.h"
#include "vtkVolume.h"
#include "vtkVolumeProperty.h"

#include "itkImageFileReader.h"
#include "itkImageToVTKImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include <stdio.h>
#include <string>
#include <vector>
//#include <set>
//#include <QtGui> 

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkVTKImageExport.h>
#include <itkVTKImageImport.h>
#include <vtkImageImport.h>
#include <itkImage.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>

#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>
#include <vtkLODActor.h>
#include <vtkSmartPointer.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkImageData.h>
#include <vtkImageReader.h>
#include <vtkImageWriter.h>
#include <vtkStructuredPointsWriter.h>
#include <vtkContourFilter.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkMarchingCubes.h>
#include <vtkCamera.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkDecimatePro.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkAppendPolyData.h>
#include <vtkRegularPolygonSource.h>

#include <vtkLine.h>
#include <vtkTubeFilter.h>
#include <vtkLineSource.h>
#include <vtkCylinderSource.h>


#include <vtkSmartPointer.h>
#include <vtkRendererCollection.h>
#include <vtkWorldPointPicker.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkObjectFactory.h>

#include <vtkDoubleArray.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkCommand.h>
#include <vtkObject.h>

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))
struct cubeCoord
{
	unsigned short sx, sy, sz;
	unsigned short ex, ey, ez;
};

typedef unsigned char inputPixelType; 
typedef unsigned char outputPixelType;

typedef itk::Image<inputPixelType,3> inputImageType;
typedef itk::Image<outputPixelType,3> outputImageType;
typedef itk::Image<short int,3> labelImageType;
typedef itk::ImageRegionConstIterator<inputImageType> constIteratorType;
typedef itk::ImageRegionIterator<inputImageType> iteratorType;
typedef itk::ImageRegionConstIterator<labelImageType> constLabelIteratorType;
typedef itk::ImageRegionIterator<labelImageType> labelIteratorType;
typedef itk::VTKImageExport<inputImageType> ExportFilterType;

const unsigned int Dimension = 3;
typedef unsigned char  ImageActorPixelType;
typedef itk::Image< ImageActorPixelType, Dimension >   RenderImageType;
typedef itk::ImageFileReader< RenderImageType >    ReaderType;
typedef itk::ImageToVTKImageFilter<RenderImageType> ConnectorType;
typedef itk::MaximumProjectionImageFilter < RenderImageType, RenderImageType> MaxProjectionType;
typedef itk::MinimumProjectionImageFilter < RenderImageType, RenderImageType> MinProjectionType;
typedef itk::MeanProjectionImageFilter < RenderImageType, RenderImageType> MeanProjectionType;
typedef itk::RescaleIntensityImageFilter< RenderImageType, RenderImageType> IntensityRescaleType;
typedef vtkSmartPointer<vtkImageActor> ImageActorPointerType;

template <typename ITK_Exporter, typename VTK_Importer>
void connectPipelines(ITK_Exporter exporter, VTK_Importer *importer);


// Define interaction style
class MouseInteractorStyle : public vtkInteractorStyleTrackballCamera
{
  public:
    static MouseInteractorStyle* New();
    vtkTypeRevisionMacro(MouseInteractorStyle, vtkInteractorStyleTrackballCamera);
 
    virtual void OnLeftButtonDown() 
    {
 
      //std::cout << "Picking pixel: " << this->Interactor->GetEventPosition()[0] << " " << this->Interactor->GetEventPosition()[1] << std::endl;
      this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0], 
                         this->Interactor->GetEventPosition()[1], 
                         0,  // always zero.
                         this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
      double picked[3];
      this->Interactor->GetPicker()->GetPickPosition(picked);
      std::cout << "Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << std::endl;
      // Forward events
      vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }
 
};

std::vector<vtkSmartPointer<vtkPolyData>> getVTKPolyDataPrecise(labelImageType::Pointer label);
inputImageType::Pointer getEmptyImage(int s1, int s2, int s3);


struct imageFileHandle
{
	std::string tag;
	std::string filename;
	bool renderStatus;
	bool ren2d;
	vtkSmartPointer<vtkImageData> ImageData;
	ReaderType::Pointer reader;
	ConnectorType::Pointer connector;;
	ConnectorType::Pointer projectionConnector;
	MaxProjectionType::Pointer MaxProjection;
	MeanProjectionType::Pointer MeanProjection;
	MinProjectionType::Pointer MinProjection;
	IntensityRescaleType::Pointer Rescale;
	std::vector<double> ImageSize;
	double x,y,z;
//Contour Filter pointers
	vtkSmartPointer<vtkContourFilter> ContourFilter;
	vtkSmartPointer<vtkPolyDataMapper> ContourMapper;
	vtkSmartPointer<vtkActor> ContourActor;
//Raycast pointers
	vtkSmartPointer<vtkVolumeProperty> volumeProperty;
	vtkSmartPointer<vtkOpenGLVolumeTextureMapper3D> volumeMapper;
//image slicer
	ImageActorPointerType sliceActor;
	vtkSmartPointer<vtkImageActor> ProjectionActor;
#ifdef USE_GPUREN
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapperGPU;
#endif
	vtkSmartPointer<vtkVolume> volume;
};



class  ImageRenderActors
{
public:
	ImageRenderActors();
	int loadImage(std::string ImageSource, std::string tag);
	int loadImage(std::string ImageSource, std::string tag, double x, double y, double z);
//render actors
	vtkSmartPointer<vtkActor> ContourActor(int i);
	vtkSmartPointer<vtkActor> GetContourActor(int i);
	ImageActorPointerType CreateSliceActor(int i);
	ImageActorPointerType GetSliceActor(int i);
	vtkSmartPointer<vtkImageActor> createProjection(int i, int method);
	vtkSmartPointer<vtkImageActor> GetProjectionImage(int i);
	vtkSmartPointer<vtkVolume> RayCastVolume(int i);
	vtkSmartPointer<vtkVolume> GetRayCastVolume(int i);
	bool getRenderStatus(int i);
	void setRenderStatus(int i, bool setStatus);
//file information
	std::vector<std::string> GetImageList();
	std::string FileNameOf(int i){ return this->LoadedImages[i]->filename;};
	unsigned int NumberOfImages() {return (unsigned int)this->LoadedImages.size();};
	bool isRayCast(int i);
	bool is2D(int i);
	void setIs2D(int i, bool Set2D);
	void ShiftImage(int i, double x, double y, double z);
	void ShiftImage(int i, std::vector<double> shift);
	std::vector<double> GetShiftImage(int i);
	double pointData(int i, int x, int y, int z);
	std::vector<double> GetImageSize(int i);
	std::vector<int> MinCurrentMaxSlices(int i);
	void SetSliceNumber(int i, int num);
	vtkSmartPointer<vtkImageData> GetImageData(int i);
	std::vector<double> getColorValues();
	void setColorValues(double r, double g, double b);
	void setColorValues(int i, double value);
	void setBrightness(int value);
	int getBrightness();
	void setOpacity(int  value);//this is the threshold
	int getOpacity();
	void setOpacityMax(int  value);//this is the threshold
	int getOpacityMax();
	void setOpacityValue(double opacity);
	double getOpacityValue();
	void setOpacityValueMax(double opacity);
	double getOpacityValueMax();
private:
	bool useGPURendering;
	void syncColorTransfetFunction();
	void syncOpacityTransfetFunction();
	vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction;
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction;
	std::vector<imageFileHandle*> LoadedImages;
	std::vector<std::string> ImageList;
	double r,g,b, opacity1, opacity2, opacity1Value, opacity2Value, RaycastSampleDist;
	double brightness;
};
#endif



 

 