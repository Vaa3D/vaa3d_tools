/* 
 * Copyright 2009 Rensselaer Polytechnic Institute
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <strsafe.h>
#define MKDIR(x) _mkdir(x)
std::vector<std::string> *listFilesDir(const char* dir) {
    WIN32_FIND_DATAA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char szDir[MAX_PATH];
    int arg_length;
	std::vector<std::string> *files_dir = new std::vector<std::string>();
    
    arg_length= (int)strlen(dir);

    if (arg_length > (MAX_PATH - 3))
        return files_dir;

    StringCchCopyA(szDir, MAX_PATH, dir);
    StringCchCatA(szDir, MAX_PATH, "\\*");

    hFind = FindFirstFileA(szDir, &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
        return files_dir;
        
    do {
         files_dir->push_back(ffd.cFileName);
    } while (FindNextFileA(hFind, &ffd) != 0);
    
    FindClose(hFind);
    return files_dir;
}
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#define MKDIR(x) mkdir(x, 0755);
std::vector<std::string> *listFilesDir(const char* dir) {
    DIR *dp;
    struct dirent *ep;
    dp = opendir(dir);
    std::vector<std::string> *files_dir = new std::vector<std::string>();
    
    if (dp != NULL) {
        while( (ep = readdir(dp)) ) {
            files_dir->push_back(ep->d_name);
        }
        closedir(dp);
        return files_dir;
    }
    return files_dir;
}
#endif

//#include "ftkNuclearSegmentation.h"

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


#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

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


struct cubeCoord
{
	unsigned short sx, sy, sz;
	unsigned short ex, ey, ez;
};

//Check that file exists
bool fileExists(std::string &filename)
{
	ifstream file(filename.c_str());
	if(!file)
	{
		return 0;
	}
	return 1;
}

//Returns a pointer to an empty image of inputImageType (unsigned char pixel, 3-D)
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

//Function to return pointer to a read-in image
template <typename T>
typename T::Pointer readImage(const std::string &filename)
{
	std::cout << "Reading " << filename << " ..." << std::endl;
	typedef typename itk::ImageFileReader<T> ReaderType;
	typename ReaderType::Pointer reader = ReaderType::New();

	ReaderType::GlobalWarningDisplayOff();
	reader->SetFileName(filename);
	try
	{
		reader->Update();
	}
	catch(itk::ExceptionObject &err)
	{
		std::cerr << "Exception object caught!" << std::endl;
		std::cerr << err << std::endl;
	}
	std::cout << "Done." << std::endl;
	return reader->GetOutput();
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

//Function to process the poly data out of the label image
vtkSmartPointer<vtkPolyData> getVTKPolyDataPrecise(labelImageType::Pointer label)
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
		
		appendfilter->AddInput(pol);
		printf("Completed %d/%d\r",counter,max1);
	}

	appendfilter->Update();

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
	printf("Done\n");

	//Return processed poly data
	vtkSmartPointer<vtkPolyData> out = smoothfinal->GetOutput();
	return out;
}

//Function to generate poly data for each of the colored image files
void generatePolyData(const std::string &filename, std::string &ply, std::vector<vtkSmartPointer<vtkPolyData> > &data)
{
	//Read in image passed from argument
	labelImageType::Pointer itkImg = readImage<labelImageType>(filename);
	//Process the image to extract the polydata
	vtkSmartPointer<vtkPolyData> polyDat = getVTKPolyDataPrecise(itkImg);

	//Set polydata to a mapper
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInput(polyDat);
	mapper->Update();

	//Add poly data to the vector of data to be rendered later
	data.push_back(polyDat);

	//Declare a polydata writer, set processed polydata as input
	vtkSmartPointer<vtkPolyDataWriter> plyWriter = vtkSmartPointer<vtkPolyDataWriter>::New();
	plyWriter->SetInput(polyDat);

	//Get output string of the writer, and write that into a file
	ofstream plyoutput(ply.c_str());
	plyWriter->WriteToOutputStringOn();
	plyWriter->Write();
	plyoutput << plyWriter->GetOutputString();
	plyoutput.close();
}

//void renderPolyData(int n, float scale[], std::vector<vtkSmartPointer<vtkPolyData> > &data, std::vector<std::string> plys)
void renderPolyData(int &n, float scale[], std::vector<std::string> &plys)
{
	//Declare a set table of ten colors
	double colors[10][3];
	colors[0][0] = 1; colors[0][1] = 0; colors[0][2] = 0;
	colors[1][0] = 0; colors[1][1] = 1; colors[1][2] = 0;
	colors[2][0] = 0; colors[2][1] = 0; colors[2][2] = 1;
	colors[3][0] = 1; colors[3][1] = 1; colors[3][2] = 0;
	colors[4][0] = 1; colors[4][1] = 0; colors[4][2] = 1;
	colors[5][0] = 0; colors[5][1] = 1; colors[5][2] = 1;
	colors[6][0] = .5; colors[6][1] = 0; colors[7][1]= 0;
	colors[7][0] = 0; colors[7][1] = .5; colors[7][2] = 0;
	colors[8][0] = 0; colors[8][1] = 0; colors[8][2] = .5;
	colors[9][0] = .5; colors[9][1] = .5; colors[9][2] = 0;

	vtkRenderer *renderer = vtkRenderer::New();

	double val[6];
	//Loop through the number of images
	for(int i = 0; i < n; i++)
	{
		//Color = input image mod 10 (the number of colors)
		int clr = i % 10;

		//Poly data reader to read in cached poly data files for each image (faster than recalculation each execution)
		vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
		reader->SetFileName(plys[i].c_str());

		//Declare mapper, and set input to polydata reader
		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInput(reader->GetOutput());
		mapper->ImmediateModeRenderingOff();
		mapper->Update();

		//Declare actor, set mapper, and set actor properties (color and scale)
		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		actor->GetProperty()->SetColor(colors[clr][0], colors[clr][1], colors[clr][2]);
		actor->SetScale(scale[0], scale[1], scale[2]);

		//Get and print actor boundaries
		actor->GetBounds(val);
		std::cout << "The bounds are: ";
		for(int m = 0; m < 6; m++)
		{
			std::cout << val[m] << " ";
		}
		std::cout << std::endl;

		//Add actor to renderer
		renderer->AddActor(actor);
	}

	//Set up render window
	renderer->SetBackground(0,0,0);
	vtkRenderWindow *renwin = vtkRenderWindow::New();
	renwin->AddRenderer(renderer);
	renwin->SetSize(500,500);
	
	//Print camera specs
	renderer->ResetCamera();
	renderer->GetActiveCamera()->Zoom(1.3);
	std::cout << "Distace to focal point: " << renderer->GetActiveCamera()->GetDistance() << std::endl;
	double *pos = renderer->GetActiveCamera()->GetPosition();
	std::cout << "Camera position: " << pos[0] << " " << pos[1] << " " << pos[2] << std::endl;

	//Set up interactor
	vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
	iren->SetRenderWindow(renwin);
	vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();
	iren->SetInteractorStyle(style);

	//Render
	renwin->Render();
	iren->Start();
}

//Function to return file extension
std::string getFileExt(std::string &file)
{
	std::string fileExt;
	for(unsigned int i = (unsigned int)file.size() - 1; i >= 0; i--)
	{
		if(file[i] == '.')
		{
			fileExt = file.substr(i+1, (file.size() - 1 - i));
			return fileExt;
		}
	}
	return NULL;
}

//Function to return file name before the file extension
std::string getOrigFileName(std::string &file)
{
	std::string fileName;
	for(unsigned int i = 0; i < file.size(); i++)
	{
		if(file[i] == '.')
		{
			fileName = file.substr(0, i);
			return fileName;
		}
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	//Check for minimum number of required arguments
	if(argc < 2)
	{
		std::cerr << "Usage: Executable Input-File Optional-Command" << std::endl;
		return -1;
	}
	//If command argument is given, set it
	std::string argCommand;
	if(argc == 3)
	{
		argCommand = argv[2];
	}

	int counter = 0;
	std::vector<vtkSmartPointer<vtkPolyData> > polyVect;
	float scale[3];
	std::vector<std::string> fileNames;
	std::vector<std::string> plyFiles;

	std::string fileName;
	std::string colorFile;
	std::string outputFile;
	std::string fileExt;
	std::string plyFile;

	//Open parameters file
	ifstream segParam(argv[1]);
	if(!segParam)
	{
		std::cerr << "Error opening segmentation parameter file" << std::endl;
		return -1;
	}
 
	//Read in image scale
	segParam >> scale[0] >> scale[1] >> scale[2];

	//Read in label image file name
	segParam >> fileName;

	segParam.close();

	//Check the image file extension
	if(fileName != " " && fileName != "")
	{	
		//Get file extension and output file base (file name without extension)
		fileExt = getFileExt(fileName);
		outputFile = getOrigFileName(fileName);

		//If tif, process
		if(fileExt == "tif" || fileExt == "tiff" || fileExt == "TIF" || fileExt == "TIFF")
		{ 
			//If running from cache
			if(argCommand == "-c")
			{
				std::cout << "Running from cache..." << std::endl;
				std::vector<std::string> *dirFiles = listFilesDir("cache");
				if(dirFiles->size() <= 2)
				{
					std::cout << "Cache doesn't exist/nothing in cache" << std::endl;
					return -1;
				}
				std::string cacheFileExt;

				//Iterate through vector of strings containing file names in the cache directory
				for(std::vector<std::string>::iterator it = dirFiles->begin(); it != dirFiles->end(); ++it)
				{
					//If the proper file name base is found
					if((*it).find(outputFile) == 0)
					{
						//Get cache file extension
						cacheFileExt = getFileExt((*it));
						//If the file is a poly data file
						if (cacheFileExt == "ply")
						{
							//Add it to the poly data file list
							std::string output = "cache/";
							counter++;
							output.append((*it));
							plyFiles.push_back(output);
						}
					}
				}
				//Render poly data
				if(counter == 0)
				{
					std::cout << "No cached files for this image" << std::endl;
					return -1;
				}
				renderPolyData(counter, scale, plyFiles);
			}
			else
			{				
				//Make a cache directory if one isn't already there	
				MKDIR("cache");
	
				//Append ouput string to original file name for colored images			
				outputFile.append("_out.tif");
				
				//Create relative address string for the colored images to be cached
				std::string output = "cache/";
				output.append(outputFile);
				std::cout << "here" << std::endl;
				//Run graph coloring on the original image, to divide it into colored images
				ftk::NuclearSegmentation *segmentation = new ftk::NuclearSegmentation();
				std::vector<std::string> colorFiles = segmentation->RunGraphColoring(fileName, output);

				//Set counter variable to number of outputted color images
				counter = (int)colorFiles.size();
				//Cycle through the colored images
				for(int i = 0; i < counter; i++)
				{
					//Create string for .ply file name
					colorFile = colorFiles[i];
					plyFile = colorFiles[i].append(".ply");
					//Add this .ply file to the vector of poly data files to be rendered later
					plyFiles.push_back(plyFile);
					//If this .ply file doesn't already exist, generate the poly data and write the 
					//.ply file from the colored image files
					generatePolyData(colorFile, plyFiles[i], polyVect);
				}
				delete segmentation;
				renderPolyData(counter, scale, plyFiles);
				//renderPolyData(counter, scale, polyVect, plyFiles);
			}
		}
		//If file not of the proper type, print error
		else
		{
			std::cerr << "Invalid file type for segmentation output" << std::endl;
			return -1;
		}
	}
	//If parameter file can't be opened, print error
	else
	{
		std::cerr << "Invalid filename in parameters, load valid parameter file" << std::endl;
		return -1;
	}
	return 0;
}
