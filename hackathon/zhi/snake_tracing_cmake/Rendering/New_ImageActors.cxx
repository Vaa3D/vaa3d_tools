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
#include "New_ImageActors.h"

ImageRenderActors::ImageRenderActors()
{
	this->LoadedImages.clear();
	//define the points of %90 color
	this->r = 150;
	this->g = 45;
	this->b = 10.0;
	this->brightness = 255;
	//opacity values
	this->opacity1 = 10;
	this->opacity1Value = .1;
	this->opacity2 = 255;
	this->opacity2Value = 1;
	this->RaycastSampleDist = 1;
	this->opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	this->syncOpacityTransfetFunction();
	this->colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	this->syncColorTransfetFunction();
}
int ImageRenderActors::loadImage(std::string ImageSource, std::string tag)
{
	if (ImageSource.empty())
	{
		return -1;
	}
	imageFileHandle *newImage= new imageFileHandle;
	newImage->filename = ImageSource;
	newImage->tag = tag;
	newImage->renderStatus = false;
	newImage->ren2d = false;
	newImage->sliceActor = 0;
	//newImage->colorTransferFunction = 0;
	newImage->ContourActor = 0;
	newImage->ContourFilter = 0;
	newImage->ContourMapper = 0;
	newImage->ImageData = 0;
	//newImage->opacityTransferFunction = 0;
	newImage->volume = 0;
	newImage->volumeMapper = 0;
	#ifdef USE_GPUREN
	{
		newImage->volumeMapperGPU = 0;
	}
	#endif
	newImage->volumeProperty = 0;
	newImage->reader = ReaderType::New();
	newImage->reader->SetFileName( ImageSource );
	newImage->x = 0;
	newImage->y = 0;
	newImage->z = 0;
	//Test opening and reading the input file
	try
	{
		newImage->reader->Update();
	}
	catch( itk::ExceptionObject & exp )
	{
		std::cerr << "Exception thrown while reading the input file " << std::endl;
		std::cerr << exp << std::endl;
		//return EXIT_FAILURE;
	}
	newImage->Rescale = IntensityRescaleType::New();
	newImage->Rescale->SetInput( newImage->reader->GetOutput() );
	newImage->connector= ConnectorType::New();
	newImage->connector->SetInput( newImage->Rescale->GetOutput() );
	newImage->projectionConnector = ConnectorType::New();
	newImage->projectionConnector->SetInput( newImage->reader->GetOutput() );

	newImage->ImageData = newImage->connector->GetOutput();
	this->LoadedImages.clear();
	this->LoadedImages.push_back(newImage);
	return (int) (this->LoadedImages.size() -1);
}
int ImageRenderActors::loadImage(std::string ImageSource, std::string tag, double x, double y, double z)
{
	if (ImageSource.empty())
	{
		return -1;
	}
	imageFileHandle *newImage= new imageFileHandle;
	newImage->filename = ImageSource;
	newImage->tag = tag;
	newImage->renderStatus = false;
	newImage->ren2d = false;
	newImage->sliceActor = 0;
	//newImage->colorTransferFunction = 0;
	newImage->ContourActor = 0;
	newImage->ContourFilter = 0;
	newImage->ContourMapper = 0;
	newImage->ImageData = 0;
	//newImage->opacityTransferFunction = 0;
	newImage->volume = 0;
	newImage->volumeMapper = 0;
	#ifdef USE_GPUREN
	{
		newImage->volumeMapperGPU = 0;
	}
#endif
	newImage->volumeProperty = 0;
	newImage->reader = ReaderType::New();
	newImage->reader->SetFileName( ImageSource );
	newImage->x = x;
	newImage->y = y;
	newImage->z = z;
	//Test opening and reading the input file
	try
	{
		newImage->reader->Update();
	}
	catch( itk::ExceptionObject & exp )
	{
		std::cerr << "Exception thrown while reading the input file " << std::endl;
		std::cerr << exp << std::endl;
		//return EXIT_FAILURE;
	}
	newImage->Rescale = IntensityRescaleType::New();
	newImage->Rescale->SetInput( newImage->reader->GetOutput() );
	newImage->connector= ConnectorType::New();
	newImage->connector->SetInput( newImage->Rescale->GetOutput() );
	newImage->ImageData = newImage->connector->GetOutput();
	newImage->projectionConnector = ConnectorType::New();
	newImage->projectionConnector->SetInput( newImage->reader->GetOutput() );

	this->LoadedImages.push_back(newImage);
	return (int) (this->LoadedImages.size() -1);
}
void ImageRenderActors::ShiftImage(int i, double x, double y, double z)
{
	this->LoadedImages[i]->x = x;
	this->LoadedImages[i]->y = y;
	this->LoadedImages[i]->z = z;
}
void ImageRenderActors::ShiftImage(int i, std::vector<double> shift)
{
	this->LoadedImages[i]->x = shift[0];
	this->LoadedImages[i]->y = shift[1];
	this->LoadedImages[i]->z = shift[2];
}
std::vector<double> ImageRenderActors::GetShiftImage(int i)
{
	std::vector<double> shift;
	shift.push_back(this->LoadedImages[i]->x);
	shift.push_back(this->LoadedImages[i]->y);
	shift.push_back(this->LoadedImages[i]->z);
	return shift;
}
vtkSmartPointer<vtkActor> ImageRenderActors::ContourActor(int i)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	this->LoadedImages[i]->ContourFilter = vtkSmartPointer<vtkContourFilter>::New();
	this->LoadedImages[i]->ContourFilter->SetInput(this->LoadedImages[i]->ImageData);
	this->LoadedImages[i]->ContourFilter->SetValue(0,10);
	this->LoadedImages[i]->ContourFilter->Update();
	this->LoadedImages[i]->ContourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	this->LoadedImages[i]->ContourMapper->SetInput(this->LoadedImages[i]->ContourFilter->GetOutput());
	this->LoadedImages[i]->ContourActor = vtkSmartPointer<vtkActor>::New();
	this->LoadedImages[i]->ContourActor->SetMapper(this->LoadedImages[i]->ContourMapper);
	this->LoadedImages[i]->ContourActor->GetProperty()->SetOpacity(.5);
	this->LoadedImages[i]->ContourActor->GetProperty()->SetColor(0.5,0.5,0.5);
	this->LoadedImages[i]->ContourActor->SetPosition(this->LoadedImages[i]->x, 
		this->LoadedImages[i]->y,this->LoadedImages[i]->z);
	this->LoadedImages[i]->ContourActor->SetPickable(0);
	return this->LoadedImages[i]->ContourActor;
}
vtkSmartPointer<vtkActor> ImageRenderActors::GetContourActor(int i)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	return this->LoadedImages[i]->ContourActor;
}
vtkSmartPointer<vtkVolume> ImageRenderActors::RayCastVolume(int i)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	/*this->LoadedImages[i]->opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	this->LoadedImages[i]->opacityTransferFunction->AddPoint(2,0.0);
	this->LoadedImages[i]->opacityTransferFunction->AddPoint(50,0.1);
	this->LoadedImages[i]->opacityTransferFunction->AddPoint(100,0.5);
	this->LoadedImages[i]->colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	this->LoadedImages[i]->colorTransferFunction->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
	this->LoadedImages[i]->colorTransferFunction->AddRGBPoint(40.0,0,0,.9);
	this->LoadedImages[i]->colorTransferFunction->AddRGBPoint(90.0,0,.9,0);
	this->LoadedImages[i]->colorTransferFunction->AddRGBPoint(150.0,.9,0,0);*/
	this->LoadedImages[i]->volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	this->LoadedImages[i]->volumeProperty->SetColor(this->colorTransferFunction);
	this->LoadedImages[i]->volumeProperty->SetScalarOpacity(this->opacityTransferFunction);
	this->LoadedImages[i]->volumeProperty->SetInterpolationTypeToLinear();
	this->LoadedImages[i]->volume = vtkSmartPointer<vtkVolume>::New();
#ifdef USE_GPUREN
	{
		this->LoadedImages[i]->volumeMapperGPU = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
		this->LoadedImages[i]->volumeMapperGPU->SetInput(this->LoadedImages[i]->ImageData);
		this->LoadedImages[i]->volumeMapperGPU->SetSampleDistance((float)this->RaycastSampleDist);
		this->LoadedImages[i]->volumeMapperGPU->SetBlendModeToComposite();
		this->LoadedImages[i]->volume->SetMapper(this->LoadedImages[i]->volumeMapperGPU);
	}
#else
	{
		this->LoadedImages[i]->volumeMapper = vtkSmartPointer<vtkOpenGLVolumeTextureMapper3D>::New();
		this->LoadedImages[i]->volumeMapper->SetSampleDistance((float)this->RaycastSampleDist);
		this->LoadedImages[i]->volumeMapper->SetInput(this->LoadedImages[i]->ImageData);
		this->LoadedImages[i]->volume->SetMapper(this->LoadedImages[i]->volumeMapper);
	}
#endif
	this->LoadedImages[i]->volume->SetProperty(this->LoadedImages[i]->volumeProperty);
	this->LoadedImages[i]->volume->SetPosition(this->LoadedImages[i]->x, 
		this->LoadedImages[i]->y,this->LoadedImages[i]->z);
	this->LoadedImages[i]->volume->SetPickable(0);
	return this->LoadedImages[i]->volume;
}
vtkSmartPointer<vtkVolume> ImageRenderActors::GetRayCastVolume(int i)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	return this->LoadedImages[i]->volume;
}
bool ImageRenderActors::getRenderStatus(int i)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	return this->LoadedImages[i]->renderStatus;
}
bool ImageRenderActors::is2D(int i)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	return this->LoadedImages[i]->ren2d;
}
void ImageRenderActors::setIs2D(int i, bool Set2D)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	this->LoadedImages[i]->ren2d = Set2D;
}
void ImageRenderActors::setRenderStatus(int i, bool setStatus)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	this->LoadedImages[i]->renderStatus = setStatus;
}
std::vector<std::string> ImageRenderActors::GetImageList()
{
	this->ImageList.clear();
	for (unsigned int i = 0; i< this->LoadedImages.size(); i++)
	{
		this->ImageList.push_back( this->LoadedImages[i]->filename);
	}
	return this->ImageList;
}
bool ImageRenderActors::isRayCast(int i)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	if (this->LoadedImages[i]->tag.compare("Image")==0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
double ImageRenderActors::pointData(int i, int x, int y, int z)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	return this->LoadedImages[i]->ImageData->GetScalarComponentAsDouble(x,y,z,0);
}
std::vector<double> ImageRenderActors::GetImageSize(int i)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	std::vector<double> imgSize;
	imgSize.push_back((double) this->LoadedImages[i]->volume->GetMaxXBound());
	imgSize.push_back((double) this->LoadedImages[i]->volume->GetMaxYBound());
	imgSize.push_back((double) this->LoadedImages[i]->volume->GetMaxZBound());
	return imgSize;
}
vtkSmartPointer<vtkImageData> ImageRenderActors::GetImageData(int i)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	return this->LoadedImages[i]->ImageData;
}
std::vector<double> ImageRenderActors::getColorValues()
{
	std::vector<double> rgb;
	rgb.push_back(this->r);
	rgb.push_back(this->g);
	rgb.push_back(this->b);
	return rgb;
}
void ImageRenderActors::setColorValues(double r, double g, double b)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->syncColorTransfetFunction();
}
void ImageRenderActors::setColorValues(int i, double value)
{
	if (i==1)
	{
		this->r = value;
	}
	else if(i ==2)
	{
		this->g = value;
	}
	else
	{
		this->b = value;
	}
	this->syncColorTransfetFunction();
}
void ImageRenderActors::setBrightness(int value)
{
	this->brightness = (double)value;
	this->syncColorTransfetFunction();
}
int ImageRenderActors::getBrightness()
{
	return (int) this->brightness;
}
void ImageRenderActors::setOpacity(int value)
{
	this->opacity1 = (double ) value;
	this->syncOpacityTransfetFunction();
}
int ImageRenderActors::getOpacity()
{
	return (int) this->opacity1;
}
void ImageRenderActors::setOpacityValue(double opacity)
{
	this->opacity1Value= opacity;
	this->syncOpacityTransfetFunction();
}
double ImageRenderActors::getOpacityValue()
{
	return this->opacity1Value;
}
void ImageRenderActors::syncColorTransfetFunction()
{
	this->colorTransferFunction->RemoveAllPoints();
	this->colorTransferFunction->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
	this->colorTransferFunction->AddRGBPoint((this->b*this->brightness)/100, 1, 1, 1);//blue
	this->colorTransferFunction->AddRGBPoint((this->g*this->brightness)/100, 0, .01, 0);//green
	this->colorTransferFunction->AddRGBPoint((this->r*this->brightness)/100, .01, 0, 0);//red
	for (unsigned int i = 0; i< this->LoadedImages.size(); i++)
	{
		this->LoadedImages[i]->volume->Update();
	}
}
void ImageRenderActors::syncOpacityTransfetFunction()
{
	this->opacityTransferFunction->RemoveAllPoints();
	this->opacityTransferFunction->AddPoint(2,0.0);
	this->opacityTransferFunction->AddPoint(this->opacity1,this->opacity1Value);
	//this->opacityTransferFunction->AddPoint(this->opacity2,this->opacity2Value);
	for (unsigned int i = 0; i< this->LoadedImages.size(); i++)
	{
		this->LoadedImages[i]->volume->Update();
	}
}

ImageActorPointerType ImageRenderActors::CreateSliceActor(int i)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	vtkImageData * newimage = this->LoadedImages[i]->ImageData;
	this->LoadedImages[i]->sliceActor = ImageActorPointerType::New();
	this->LoadedImages[i]->sliceActor->SetInput(newimage);
	this->LoadedImages[i]->sliceActor->SetZSlice(0);
	this->LoadedImages[i]->sliceActor->SetPosition(this->LoadedImages[i]->x, 
		this->LoadedImages[i]->y,this->LoadedImages[i]->z);
	this->LoadedImages[i]->sliceActor->SetPickable(0);

	return this->LoadedImages[i]->sliceActor;

}
ImageActorPointerType ImageRenderActors::GetSliceActor(int i)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	return this->LoadedImages[i]->sliceActor;
}
std::vector<int> ImageRenderActors::MinCurrentMaxSlices(int i)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	std::vector<int> slices;
	slices.push_back(this->LoadedImages[i]->sliceActor->GetSliceNumberMin());
	slices.push_back(this->LoadedImages[i]->sliceActor->GetZSlice());
	slices.push_back(this->LoadedImages[i]->sliceActor->GetSliceNumberMax());
	return slices;
}
void ImageRenderActors::SetSliceNumber(int i, int num)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	if (!this->LoadedImages[i]->ren2d)
	{
		return;
	}
	std::vector<int> slices;
	slices = this->MinCurrentMaxSlices(i);
	std::cout << num << " min " << slices[0] << " current " << slices[1] << " max " << slices[2] << std::endl;
	if (slices[2] < num)
	{
		this->LoadedImages[i]->sliceActor->SetZSlice(slices[2]);
	}
	else if (slices[0] > num)
	{
		this->LoadedImages[i]->sliceActor->SetZSlice(slices[0]);
	}
	else
	{
		this->LoadedImages[i]->sliceActor->SetZSlice(num);
	}
}
vtkSmartPointer<vtkImageActor> ImageRenderActors::createProjection(int i, int method)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	this->LoadedImages[i]->ProjectionActor = vtkSmartPointer<vtkImageActor>::New();
	if (method ==2)
	{
		this->LoadedImages[i]->MinProjection = MinProjectionType::New();
		this->LoadedImages[i]->MinProjection->SetInput(this->LoadedImages[i]->Rescale->GetOutput());
		this->LoadedImages[i]->projectionConnector->SetInput(this->LoadedImages[i]->MinProjection->GetOutput());
	}
	else if (method == 1)
	{
		this->LoadedImages[i]->MeanProjection = MeanProjectionType::New();
		this->LoadedImages[i]->MeanProjection->SetInput(this->LoadedImages[i]->Rescale->GetOutput());
		this->LoadedImages[i]->projectionConnector->SetInput(this->LoadedImages[i]->MeanProjection->GetOutput());
	}
	else
	{
		this->LoadedImages[i]->MaxProjection = MaxProjectionType::New();
		this->LoadedImages[i]->MaxProjection->SetInput(this->LoadedImages[i]->Rescale->GetOutput());
		this->LoadedImages[i]->projectionConnector->SetInput(this->LoadedImages[i]->MaxProjection->GetOutput());
	}
	this->LoadedImages[i]->ProjectionActor->SetInput(this->LoadedImages[i]->projectionConnector->GetOutput());
	this->LoadedImages[i]->ProjectionActor->SetPosition(this->LoadedImages[i]->x, 
		this->LoadedImages[i]->y,this->LoadedImages[i]->z);
	this->LoadedImages[i]->ProjectionActor->SetPickable(0);

	return this->LoadedImages[i]->ProjectionActor;
}
vtkSmartPointer<vtkImageActor> ImageRenderActors::GetProjectionImage(int i)
{
	if (i == -1)
	{
		i = int (this->LoadedImages.size() - 1);
	}
	return this->LoadedImages[i]->ProjectionActor;
}


std::vector<vtkSmartPointer<vtkPolyData>> getVTKPolyDataPrecise(labelImageType::Pointer label)
{
	labelIteratorType liter = labelIteratorType(label,label->GetLargestPossibleRegion());
	liter.GoToBegin();

	//find the maximum number of cells
	unsigned short max1 = 0;
	for(liter.GoToBegin();!liter.IsAtEnd();++liter)
	{
		max1 = MAX(max1,liter.Get());
	}

	//find all the cubes in which cells lie
	cubeCoord* carray = new cubeCoord[max1+1];
	for(int counter=0; counter<=max1; counter++)
	{
		carray[counter].sx=6000;carray[counter].sy=6000;carray[counter].sz=6000;
		carray[counter].ex=0;carray[counter].ey=0;carray[counter].ez=0;
	}

	//Declare label image iterators
	typedef itk::ImageRegionConstIteratorWithIndex<labelImageType> ConstLabelIteratorWithIndex;
	ConstLabelIteratorWithIndex cliter = ConstLabelIteratorWithIndex(label,label->GetLargestPossibleRegion());
	inputImageType::IndexType index;

	//Iterate through the label image
	for(cliter.GoToBegin();!cliter.IsAtEnd();++cliter)
	{
		int cur = cliter.Get();
		if(cur!=0)
		{
			index = cliter.GetIndex();
			carray[cur].sx= MIN(index[0],carray[cur].sx);
			carray[cur].sy= MIN(index[1],carray[cur].sy);
			carray[cur].sz= MIN(index[2],carray[cur].sz);
			carray[cur].ex= MAX(index[0],carray[cur].ex);
			carray[cur].ey= MAX(index[1],carray[cur].ey);
			carray[cur].ez= MAX(index[2],carray[cur].ez);
		}
	}

	//find the largest image size we need
	unsigned short wx=0,wy=0,wz=0;
	for(int counter=1; counter<=max1; counter++)
	{
		wx = MAX(carray[counter].ex-carray[counter].sx+1,wx);
		wy = MAX(carray[counter].ey-carray[counter].sy+1,wy);
		wz = MAX(carray[counter].ez-carray[counter].sz+1,wz);
	}
	// accommodate padding
	wx = wx+2;wy = wy +2; wz = wz+2;
	printf("wx wy wz %u %u %u\n",wx,wy,wz);

	//Declare itk image exporter and vtk image importer, then connect their pipelines
    vtkSmartPointer<vtkImageImport> vtkimporter = vtkSmartPointer<vtkImageImport>::New();
	ExportFilterType::Pointer itkexporter = ExportFilterType::New();
	connectPipelines(itkexporter,(vtkImageImport *)vtkimporter);

	//Using Marching cubes as a contour filter
	vtkSmartPointer<vtkMarchingCubes> contourf = vtkSmartPointer<vtkMarchingCubes>::New();
	contourf->SetInput(vtkimporter->GetOutput());
	contourf->SetValue(0,127);
	contourf->ComputeNormalsOff();
	contourf->ComputeScalarsOff();
	contourf->ComputeGradientsOff();

	//Using vtkSmoothPolyDataFilter as an initial smoothing filter
	vtkSmartPointer<vtkSmoothPolyDataFilter> smoothf = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
	smoothf->SetInput(contourf->GetOutput());
	smoothf->SetRelaxationFactor(0.3);
	smoothf->SetNumberOfIterations(20);

	//Set up polydata transform
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->PostMultiply();
	transform->Identity();
	vtkSmartPointer<vtkTransformPolyDataFilter> tf = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	tf->SetTransform(transform);
	tf->SetInput(smoothf->GetOutput());

	vtkSmartPointer<vtkAppendPolyData> appendfilter = vtkSmartPointer<vtkAppendPolyData>::New();


	std::vector<vtkSmartPointer<vtkPolyData>> ALL_POLY;

	//Generate empty image, and iterate through the cells, filling the image
	inputImageType::Pointer t = getEmptyImage(wx,wy,wz);
	for(int counter=1; counter<=max1; counter++)
	{
		inputImageType::SizeType size;
		inputImageType::RegionType region;
		index.Fill(1);

		region.SetIndex(index);
		region.SetSize(size);

		labelImageType::SizeType lsize;
		labelImageType::IndexType lindex;
		labelImageType::RegionType lregion;

		itkexporter->SetInput(t);

		t->FillBuffer(0);
		lsize[0] = carray[counter].ex-carray[counter].sx+1;
		lsize[1] = carray[counter].ey-carray[counter].sy+1;
		lsize[2] = carray[counter].ez-carray[counter].sz+1;

		lindex[0] = carray[counter].sx;
		lindex[1] = carray[counter].sy;
		lindex[2] = carray[counter].sz;

		lregion.SetIndex(lindex);
		lregion.SetSize(lsize);
		labelIteratorType localiter = labelIteratorType(label,lregion);

		size = lsize;
		region.SetSize(size);
		iteratorType iter = iteratorType(t,region);
		for(localiter.GoToBegin(),iter.GoToBegin();!localiter.IsAtEnd();++localiter,++iter)
		{
			if(localiter.Get()==counter)
			{
				iter.Set(255);
			}
		}
		t->Modified();
		vtkimporter->Modified();

		transform->Identity();	
		transform->Translate(carray[counter].sx-1,carray[counter].sy-1,carray[counter].sz-1);
		tf->SetTransform(transform);
		tf->Update();
		vtkSmartPointer<vtkPolyData> pol=vtkSmartPointer<vtkPolyData>::New();
		pol->DeepCopy(tf->GetOutput());

		//appendfilter->AddInput(pol);
		ALL_POLY.push_back(pol);
		printf("Completed %d/%d\r",counter,max1);
	}

	/*appendfilter->Update();
   
	//Decimate filter (reduces the number of triangles in the mesh) for the poly data
	vtkSmartPointer<vtkDecimatePro> decimate = vtkSmartPointer<vtkDecimatePro>::New();
	decimate->SetInput(appendfilter->GetOutput());
	decimate->SetTargetReduction(0.75);
	printf("Decimating the contours...");
	decimate->Update();
	printf("Done\n");

	//Smooth poly data filter for a final smoothing
	printf("Smoothing the contours after decimation...");
	vtkSmartPointer<vtkSmoothPolyDataFilter> smoothfinal = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
	smoothfinal->SetRelaxationFactor(0.2);
	smoothfinal->SetInput(decimate->GetOutput());
	smoothfinal->SetNumberOfIterations(0);
	smoothfinal->Update();
	printf("Done\n");*/

	//Return processed poly data
	//vtkSmartPointer<vtkPolyData> out = smoothfinal->GetOutput();
	return ALL_POLY;
}

//Function to connect the pipelines between the itk image exporter to the vtk image importer
template <typename ITK_Exporter, typename VTK_Importer>
void connectPipelines(ITK_Exporter exporter, VTK_Importer *importer)
{
	importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());
	importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
	importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
	importer->SetSpacingCallback(exporter->GetSpacingCallback());
	importer->SetOriginCallback(exporter->GetOriginCallback());
	importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());
	importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());
	importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
	importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
	importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
	importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
	importer->SetCallbackUserData(exporter->GetCallbackUserData());
}

inputImageType::Pointer getEmptyImage(int s1, int s2, int s3)
{
	inputImageType::Pointer p = inputImageType::New();
	inputImageType::SizeType size;
	inputImageType::IndexType index;
	inputImageType::RegionType region;
	size[0] = s1;
	size[1] = s2;
	size[2] = s3;
	index.Fill(0);
	region.SetSize(size);
	region.SetIndex(index);
	p->SetRegions(region);
	p->Allocate();
	return p;
}