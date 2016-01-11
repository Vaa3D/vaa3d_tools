/*
Programmer: Edward Hottendorf 
For use in the Big Neuron Project

Reconstruction3D is a blend of two programs, AnalyzeSkeleton(http://fiji.sc/AnalyzeSkeleton) and 
itkBinaryThinningImageFilter3D(http://www.insight-journal.org/browse/publication/181), 
and combines them together to thin and analyze a binarized image. Afterwards original code
was added to for certain functionalities.

More information on the thinning filter can be
found on http://fiji.sc/wiki/index.php/Skeletonize3D which references the itk version and also has an imagej version

The current output is in the form of an swc file
*/




#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

#include "itkConnectedThresholdImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkBinaryThinningImageFilter3D.h"

#include "Point.h"
#include "pixPoint.h"
#include "Vertex.h"
#include "Edge.h"
#include "Preprocessing.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <queue>

using namespace std;

const int UNDEFINED = -1;
const int SLAB = 0;
const int JUNCTION = 1;
const int END_POINT = 2;


//test
struct graph
{
	vector<Vertex*> vertices; //the actual vertices and edges are declared dynamically
	vector<Edge*> edges;
};
//
vector<pixPoint*> visitSkeleton(itk::Image<signed short, (unsigned)3> *image, pixPoint* root, bool ***, graph * g);
void trueAnalyze(itk::Image<signed short, (unsigned)3> *image, char* outfilename, itk::Image<signed short, (unsigned)3> *, int);
signed short getPixel(itk::Image<signed short, (unsigned)3> *image, int x, int y, int z);
void outputSWC(vector<pixPoint*> & Points, char* outfilename, itk::Image<signed short, (unsigned)3> *, long);
void spurCleanup(graph * g, vector<pixPoint*> & Points, char* outfilename, itk::Image<signed short, (unsigned)3> *);  
signed short getPixel(itk::Image<signed short, (unsigned)3> *image, int x, int y, int z);
void getNeighbors(itk::Image<signed short, (unsigned)3> *image, vector<pixPoint*> & neigh, pixPoint* p);
int numberOfNeighbors(itk::Image<signed short, (unsigned)3> *image, int xCoord, int yCoord, int zCoord);
bool *** resetVisited(itk::Image<signed short, (unsigned)3> *image);
Vertex* createVertex(itk::Image<signed short, (unsigned)3> *, pixPoint*);
int findInVertex(vector<Vertex*>&, pixPoint*);
int findRadius(pixPoint* crntPnt, itk::Image<signed short, (unsigned)3> * image);

void startPreprocess(itk::Image<signed short, (unsigned)3> *, itk::Image<signed short, (unsigned)3> *, int);
void createITKImage(unsigned char *datald, itk::Image<signed short, (unsigned)3> * I, long x, long y, long z, int threshold);

int main(int argc, char* argv[])
{
	// Verify the number of parameters in the command line
	if (argc <= 2)
	{
		std::cerr << "Usage: " << std::endl;
		std::cerr << argv[0] << " inputImageFile outputImageFile" << std::endl;
		return EXIT_FAILURE;
	}
	char* infilename = argv[1];
	char* outfilename = argv[2];

	int thresh;
	int treeNum = 10;
	
	cout << "What threshold do you want?";
	cin >> thresh;
	const   unsigned int Dimension = 3;
	typedef signed short PixelType;   // must be signed for CT since Hounsfield units can be < 0

	//typedef unsigned char PixelType;
	typedef itk::Image< PixelType, Dimension > ImageType;

	// Read image
	typedef itk::ImageFileReader< ImageType > ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(infilename);
	try
	{
	reader->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
	std::cout << ex << std::endl;
	return EXIT_FAILURE;
	}
	cout << infilename << " sucessfully read." << endl;


	
	// Define the thinning filter
	typedef itk::BinaryThinningImageFilter3D< ImageType, ImageType > ThinningFilterType;
	ThinningFilterType::Pointer thinningFilter = ThinningFilterType::New();

	//preprocess takes the input image and then returns the preprocessed image

	ImageType::Pointer image = ImageType::New(); //create the itk image here and pass it in as a pointer parameter because of scope limitations
	startPreprocess(reader->GetOutput(), image, thresh);
	thinningFilter->SetInput(image);



	//testing 
	/*
	typedef itk::Image< PixelType, Dimension > ImageType;
	typedef itk::ImageFileWriter< ImageType > WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetInput(image);
	writer->SetFileName(outfilename);

	try
	{
		writer->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
		std::cout << ex << std::endl;
		
	}
	cout << outfilename << " sucessfully written." << endl;

	cout << "Program terminated normally." << endl;

	*/
	//testing 



	thinningFilter->Update();
	
	//add threshold and tree arguments
	trueAnalyze(thinningFilter->GetOutput(), outfilename, reader->GetOutput(), treeNum);


	return 0;
}


//ImageOperation: convert Vaa3D to itk image
void createITKImage(unsigned char *datald, itk::Image<signed short, (unsigned)3> * I, long x, long y, long z, int threshold)
{
	typedef itk::Image<signed short, 3> ImageType; //signed short is required for the thinning algorithm
	int SN = x;
	int SM = y;
	int SZ = z;


	ImageType::SizeType size;
	size[0] = SN;
	size[1] = SM;
	size[2] = SZ;

	ImageType::IndexType idx;
	idx.Fill(0);
	ImageType::RegionType region;
	region.SetSize(size);
	region.SetIndex(idx);

	I->SetRegions(region);
	I->Allocate();
	I->FillBuffer(0);

	signed short PixelValue;
	for (int iz = 0; iz < SZ; iz++)
	{
		int offsetk = iz*SM*SN;
		for (int iy = 0; iy < SM; iy++)
		{
			int offsetj = iy*SN;
			for (int ix = 0; ix < SN; ix++)
			{
				if ((signed short)threshold < (signed short)datald[offsetk + offsetj + ix])
				{
					PixelValue = 244;
				}
				else
				{
					PixelValue = 0;
				}
				itk::Index<3> indexX;
				indexX[0] = ix;
				indexX[1] = iy;
				indexX[2] = iz;
				I->SetPixel(indexX, PixelValue);
			}
		}
	}


}

void startPreprocess(itk::Image<signed short, (unsigned)3> * image, itk::Image<signed short, (unsigned)3> * output, int threshold){

	typedef itk::Image< signed short, (unsigned)3> ImageType;

	//This sets the region to the entire image
	ImageType::RegionType region = image->GetLargestPossibleRegion();

	ImageType::SizeType size = region.GetSize();

	long x = size[0];
	long y = size[1];
	long z = size[2];

	unsigned char *pData = NULL;
	try
	{
		pData = (unsigned char *)(new unsigned char[x*y*z]);
	}
	catch (...)
	{
		cout << "something went wrong";
		if (pData) { delete[]pData; pData = NULL; }
		return;
	}


	long mCount = y * x;
	
	for (long i = 0; i<z; i++)
	{
		for (long j = 0; j<y; j++)
		{
			for (long k = 0; k<x; k++)
			{
				long curpos = i * mCount + j*x + k;
				pData[curpos] = (unsigned char) getPixel(image, k, j, i);
				
			}
		}
	}
	
	pData = Preprocess(pData, x, y, z, 1, threshold);
	createITKImage(pData, output, x, y, z, threshold);

}














//now the param includes the original image
void trueAnalyze(itk::Image<signed short, (unsigned)3> *image, char* outfilename, itk::Image<signed short, (unsigned)3> * origImage, int treeNumber){

	typedef itk::Image< signed short, (unsigned)3> ImageType;
	typedef signed short PixelType;

	vector <pixPoint*> endPoints;

	vector <Point*> points;

	vector<vector<pixPoint*> > skeletonPoints;
	//num of trees can be found by calling skeletonPoints.size()
	//num of pixels in a tree can be found by calling skeletonPoints[someNum].size();

	//This sets the region to the entire image
	ImageType::RegionType region = image->GetLargestPossibleRegion();

	ImageType::SizeType size = region.GetSize();

	int width = size[0];
	int height = size[1];
	int depth = size[2];


	signed short crntPoint;

	//Loop through each pixel to find every foreground pixel
	for (int x = 0; x < width; x++)
		for (int y = 0; y < height; y++)
			for (int z = 0; z < depth; z++)
			{

				crntPoint = getPixel(image, x, y, z);

					if (crntPoint != 0)
						points.push_back(new Point(x, y, z)); //the foreground pixels

			}

	//Loop through the foreground pixels to find the end points
	for (int i = 0; i < points.size(); i++)
	{
		if (numberOfNeighbors(image, points[i]->x, points[i]->y, points[i]->z) == 1){
			endPoints.push_back(new pixPoint(points[i]->x, points[i]->y, points[i]->z, END_POINT));

		}
	}

	bool *** visited = resetVisited(image);

	
	vector<graph*> skeletonGraphs;   //test

	//for every end point that is not already visit traverse the skeleton
	for (int i = 0; i < endPoints.size(); i++){

		if (visited[endPoints[i]->x][endPoints[i]->y][endPoints[i]->z] != true){
			graph * skelGraph = new graph(); //test
			skeletonPoints.push_back(visitSkeleton(image, endPoints[i], visited, skelGraph));
			skeletonGraphs.push_back(skelGraph); //test
		}
	}


	

	//bubble sort ascending order so the last tree is the largest
	int* indexNum = new int[skeletonPoints.size()];
	for (int i = 0; i < skeletonPoints.size(); i++)
	{
		indexNum[i] = i;
	}


	int i, j;
	int temp;
	bool swapped = true;

	for (i = (skeletonPoints.size() - 1); i >= 0 && swapped; i--)
	{
		swapped = false;

		for (j = 1; j <= i; j++)
		{
			if (skeletonPoints[indexNum[j - 1]].size() > skeletonPoints[indexNum[j]].size())
			{
				temp = indexNum[j - 1];
				indexNum[j - 1] = indexNum[j];
				indexNum[j] = temp;
				swapped = true;
			}
		}
	}


	//output certain number of largest trees to a swc file, will be a parameter
	if (treeNumber >  skeletonPoints.size())
		treeNumber = skeletonPoints.size();

	long startVisitNum = 0;
	for (int i = 1; i <= treeNumber; i++)
	{
		//commented out by ZJ since radius was turned off
		//spurCleanup(skeletonGraphs[indexNum[skeletonGraphs.size() - i]], skeletonPoints[indexNum[skeletonPoints.size() - i]], outfilename, origImage); //also have the graph class as input
		outputSWC(skeletonPoints[indexNum[skeletonPoints.size() - i]], outfilename, origImage,startVisitNum); //currently outputing the largest tree
		startVisitNum += skeletonPoints[indexNum[skeletonPoints.size() - i]].size();
	}


	points.clear();
	endPoints.clear();

	delete  visited;
}//end trueAnalyze





//Starting from the root point (decided by the parameter and assumed to be an endpoint), traverse all of the connecting skeleton points
//in a breadth first search. This ensures that slab pixels are contiguous in storage which allows for easier classifications of the edges and vertices.
vector<pixPoint*> visitSkeleton(itk::Image<signed short, (unsigned)3> *image, pixPoint* root, bool *** visited, graph * g){

	pixPoint* crntPoint = new pixPoint(root->x, root->y, root->z, root->pixType);   //dynamically allocate to deal with scope issues

	int num = 1;    //the visiting num. this starts at 1 for logical reasons so when refering to the vector of skeletonpoints it must be num - 1
	int count = 0;  //used in the slab portion of the code in order to determine if the next neighbor is an already visited junction
	int vnum = 0;   //the index of the vertex in the vector vertices

	visited[crntPoint->x][crntPoint->y][crntPoint->z] = true;
	crntPoint->visitNum = num;
	crntPoint->parent = -1;   //-1 parent means it does not exist. so this is the root

	queue<pixPoint*> pathQ;
	pathQ.push(crntPoint);    //start the root as the beginning of a path

	vector<pixPoint*> crntSkelPoints;         //the list of points that will be returned 
	crntSkelPoints.push_back(crntPoint);  

	vector<pixPoint*> neighbor;

	Edge* e;    //a temporary edge. this updated until it is complete and ready to be pushed

	//set up the edge and vertices with the root and prepare for traversal
	g->vertices.push_back(createVertex(image, crntPoint)); //vertex
	e = new Edge();
	e->v2 = NULL;
	e->v1 = g->vertices.back();
	g->vertices.back()->edges.push_back(e);  //test

	getNeighbors(image, neighbor, crntPoint);     //neighbor passed in as reference
	//all of the neighbors will only have x,y,z data at this point
	neighbor[0]->parent = crntPoint->visitNum;
	crntPoint = neighbor[0];

	neighbor.clear();   

	//start the modified breadth first traversal. Where the first slab in each edge is put into the queue and the queue is popped 
	//whenever the edge paths end (i.e. contact with a vertex or an end point)
	while (!pathQ.empty()) {

		//because multiple points can add their neighbors onto the queue we only consider the crnt points that are not visited
		if (visited[crntPoint->x][crntPoint->y][crntPoint->z] == true) {
			pathQ.pop();
			if (!pathQ.empty()){
				crntPoint = pathQ.front();
			}
			continue;
		}

		num++;
		crntPoint->visitNum = num;
		crntSkelPoints.push_back(crntPoint);
		visited[crntPoint->x][crntPoint->y][crntPoint->z] = true;
		getNeighbors(image, neighbor, crntPoint); //neighbor passed in by reference
		//all of the neighbors will only have x, y, z at this point

		if (neighbor.size() == 2){
			crntPoint->pixType = SLAB;

			count = 0;

			if (e->v2 != NULL){ //check for v2 != NULL because v2 will always be NULL until a new path is ready to be created. checking for parent junction lead to occassional abend because the Root and the junction could be adjacent and would only give a v1 to the edge before starting a new path (since endpoint to junction is not going to be a v1->v2 path)

				vnum = findInVertex(g->vertices, crntSkelPoints[crntPoint->parent - 1]);
				
				e = new Edge();
				e->v2 = NULL;
				e->v1 = g->vertices[vnum];		
				g->vertices[vnum]->edges.push_back(e);   //test
			}
			
			e->slab.push_back(crntPoint); //Add one pixel to the most current edge
			

			for (int i = 0; i < neighbor.size(); i++)
			{
				if (visited[neighbor[i]->x][neighbor[i]->y][neighbor[i]->z] == false)
				{
					neighbor[i]->parent = crntPoint->visitNum;

					count++;
					crntPoint = neighbor[i];

				}
			}

			if (count == 0){ //if the next neighbor is already visited, the path is done and next neighbor is junction

				//compare a neighbor against crntPoint parent in order to see which one is not the parent. The non-parent should be an already visited junction
				if (neighbor[0]->x == crntSkelPoints[crntPoint->parent - 1]->x
					&& neighbor[0]->y == crntSkelPoints[crntPoint->parent - 1]->y
					&& neighbor[0]->z == crntSkelPoints[crntPoint->parent - 1]->z){
					vnum = findInVertex(g->vertices, neighbor[1]);
					e->v2 = g->vertices[vnum];
					g->edges.push_back(e);

					g->vertices[vnum]->edges.push_back(e);   //test
				}
				else{
					vnum = findInVertex(g->vertices, neighbor[0]);
					e->v2 = g->vertices[vnum];
					g->edges.push_back(e);

					g->vertices[vnum]->edges.push_back(e);   //test
				}

				pathQ.pop();
				if (!pathQ.empty()){
					crntPoint = pathQ.front();
				}
			}

		}
		else if (neighbor.size() > 2){
			crntPoint->pixType = JUNCTION;

			//check the point against the list of vertices you have
			//create new vertex if this point is not on any vertex
			//otherwise add the already existing vertex to the current edge 
			//if the parent is a slab 
			vnum = findInVertex(g->vertices, crntPoint);
			if (vnum == -1){
				g->vertices.push_back(createVertex(image, crntPoint)); //vertex

				e->v2 = g->vertices.back(); 
				g->edges.push_back(e);

				g->vertices.back()->edges.push_back(e);   //test
			}
			else if (crntSkelPoints[crntPoint->parent - 1]->pixType == SLAB){
				e->v2 = g->vertices[vnum];
				g->edges.push_back(e);

				g->vertices[vnum]->edges.push_back(e);  //test
			}

			 
			for (int i = 0; i < neighbor.size(); i++)
			{
				if (visited[neighbor[i]->x][neighbor[i]->y][neighbor[i]->z] == false)
				{
					neighbor[i]->parent = crntPoint->visitNum;

					pathQ.push(neighbor[i]);

				}
			}

			pathQ.pop();				//pop here and at junctions because they are the end destinations of an edge
			if (!pathQ.empty()){
				crntPoint = pathQ.front();
			}

		}
		else{

			crntPoint->pixType = END_POINT;

			//If the parent is a SLAB, it is not possible for the end point to already be on a vertex so we make a new vertex
			if (crntSkelPoints[crntPoint->parent - 1]->pixType == SLAB){
				g->vertices.push_back(createVertex(image, crntPoint)); //vertex
				e->v2 = g->vertices.back(); //if we created a new vertex it means no one else has had contact yet and the only thing it is connected to so far is the parent (definitely a slab)
				g->edges.push_back(e);

				g->vertices.back()->edges.push_back(e);   //test
			}

			pathQ.pop();					//pop here and at junctions because they are the end destinations of an edge
			if (!pathQ.empty()){
				crntPoint = pathQ.front();
			}

		}

		neighbor.clear();
	} 

	return crntSkelPoints;

}//end visitSkeleton





bool *** resetVisited(itk::Image<signed short, (unsigned)3> *image){
	
	typedef itk::Image< signed short, (unsigned)3> ImageType;
	typedef signed short PixelType;

	//This sets the region to the entire image
	ImageType::RegionType region = image->GetLargestPossibleRegion();

	ImageType::SizeType size = region.GetSize();

	int width = size[0];
	int height = size[1];
	int depth = size[2];



	// Reset visited variable
	bool *** visited = new bool**[width];
	for (int i = 0; i < width; i++){
		visited[i] = new bool*[height];
		for (int u = 0; u < height; u++){
			visited[i][u] = new bool[depth];
		}
	}

	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
			for (int k = 0; k < depth; k++)
				visited[i][j][k] = false;

	return visited;
}//end resetVisited



//find all of the neighbors and keep a count rather than updating the vector
int numberOfNeighbors(itk::Image<signed short, (unsigned)3> *image, int xCoord, int yCoord, int zCoord){

	int n = 0;

	for (int x = -1; x <= 1; x++)
		for (int y = -1; y <= 1; y++)
			for (int z = -1; z <= 1; z++)
			{

				if (x == 0 && y == 0 && z == 0)
					continue;

				if (getPixel(image, xCoord + x, yCoord + y, zCoord + z) != 0)
					n++;

			}
	return n;
}//end numberOfNeighbors



//get all of the pixels in the neighborhood and add them to a vector that is passed in by reference
//loop caution: for obvious reasons this does not clear out the neighbor vector when finished. 
void getNeighbors(itk::Image<signed short, (unsigned)3> *image, vector<pixPoint*> & neigh, pixPoint* p){



	for (int x = -1; x <= 1; x++)
		for (int y = -1; y <= 1; y++)
			for (int z = -1; z <= 1; z++)
			{

				if (x == 0 && y == 0 && z == 0)
					continue;

				if (getPixel(image, p->x + x, p->y + y, p->z + z) != 0)
					neigh.push_back(new pixPoint(p->x + x, p->y + y, p->z + z, UNDEFINED));

			}
}//end getNeighbors



//Get pixel from the specified image. It does check the bounds of the image. If an x,y,z coord is check that does not exist then 0 is returned
signed short getPixel(itk::Image<signed short, (unsigned)3> *image, int x, int y, int z){

	typedef itk::Image< signed short, (unsigned)3> ImageType;
	typedef signed short PixelType;

	//This sets the region to the entire image
	ImageType::RegionType region = image->GetLargestPossibleRegion();

	ImageType::SizeType size = region.GetSize();

	int width = size[0];
	int height = size[1];
	int depth = size[2];

	ImageType::IndexType pixelIndex;
	pixelIndex[0] = x;
	pixelIndex[1] = y;
	pixelIndex[2] = z;

	if (x >= 0 && x < width && y >= 0 && y < height && z >= 0 && z < depth)
		return image->GetPixel(pixelIndex);
	else{
		return 0;
	}
}//end getpixel





void outputSWC(vector<pixPoint*> & Points, char* outfilename, itk::Image<signed short, (unsigned)3> * origImage, long startVisitNum){


	ofstream outFile;
	
	int radius;

	if (startVisitNum == 0) //first tree
	{
		outFile.open(outfilename, ios::trunc);
		outFile << "#name reconstruction3d \n#comment \n##n,type,x,y,z,radius,parent\n";
	}
	else //subsequent trees
		outFile.open(outfilename, ios::app);

	for (int i = 0; i < Points.size(); i++)
	{
		if (Points[i] == NULL)
		{
			continue;
		}
		//commented by zj on 1/9 since not needed by BIGNEURON
		//radius = findRadius(Points[i], origImage);
		radius = 1;
		
		if (Points[i]->parent == -1)  //root of a separate tree
			outFile << Points[i]->visitNum + startVisitNum << ' ' << 7 << ' ' << Points[i]->x << ' ' << Points[i]->y << ' ' << Points[i]->z << ' ' << radius << ' ' << -1 << endl;

		else
			outFile << Points[i]->visitNum + startVisitNum << ' ' << 7 << ' ' << Points[i]->x << ' ' << Points[i]->y << ' ' << Points[i]->z << ' ' << radius << ' ' << Points[i]->parent + startVisitNum << endl;
	}

	outFile.close();
	cout << outfilename << " Successfully written to swc file.";


}//end outputSWC

//Create a vertex at the crntPnt parameter and include all of the junctions and endpoints that are connected to it through junctions.
//In other words, its neighbors that are junctions and/or endpoints and the neighbors of those junctions that are junctions and/or endpoints and so on until there are none left
Vertex* createVertex(itk::Image<signed short, (unsigned)3> *image, pixPoint* crntPnt){

	Vertex* v1 = new Vertex();

	v1->points.push_back(crntPnt);

	queue<pixPoint*> neighborQue;

	vector<pixPoint*> tempNeigh;
	vector <pixPoint*> neighbors;
	neighborQue.push(crntPnt); //crnt is passed in

	//gather the neighbor of neighbors with a search that finds all connected points that have more than 2 neighbors or only one neighbor
	while (!neighborQue.empty()){
		crntPnt = neighborQue.front();

		getNeighbors(image, neighbors, crntPnt);
		for (int i = 0; i < neighbors.size(); i++){

			//populate tempNeigh with pixPoints and then check the number of neighbors. neighbors > 3 = vertex point (add to vertex). neighbors == 1 end point (add to vertex). neighbors == 2 slab (do not add to vertex)
			getNeighbors(image, tempNeigh, neighbors[i]);
			if (tempNeigh.size() > 2){ 
				//if number of neighbors is greater than 2 add all points to the vertex that are not already contained. 
				//Add these points to the queue to check their neighbors and repeat the process
				if (!v1->contains(neighbors[i])){
					v1->points.push_back(neighbors[i]);
					neighborQue.push(neighbors[i]);
				}
			}
			else if (tempNeigh.size() == 1){
				//if number of neighbors is 1 then add the point to the vertex and continue to the next point on the queue
				if (!v1->contains(neighbors[i])){
					v1->points.push_back(neighbors[i]);
				}
			}

			tempNeigh.clear(); 
		}
		neighbors.clear();
		neighborQue.pop();	
	}

	return v1;
}


int findInVertex(vector<Vertex*>& vertices, pixPoint* pnt){
	//loop through the vertex
	//if pnt is found return its index location
	//otherwise return -1
	for (int i = 0; i < vertices.size(); i++){
		if (vertices[i]->contains(pnt)){
			return i;
		}
	}
	return -1;
}


//Find the radius of a point
int findRadius(pixPoint* crntPnt, itk::Image<signed short, (unsigned)3> * image){
	
	int radius = 0,
		val = 1,
		x,
		y;

	int count = 0;

	do {  

		radius++;

		//check every pixel at distance "radius" from the point
		//if the val of the pixel is 0 then increment a count
		for (x = -radius; x <= radius; x++){
			y = sqrt(-4 * ((x * x) - (radius * radius))) / 2;
			val = getPixel(image, crntPnt->x + x, crntPnt->y + y, crntPnt->z);
			if (val == 0){
				count++;   
			}

			val = getPixel(image, crntPnt->x + x, crntPnt->y - y, crntPnt->z);
			if (val == 0){
				count++;  
			}

		}
	} while ((double)count / (6.28 * radius) < 0.16); //stop loop when the count surpasses 16% of the current perimeter

	return radius;
	

}

//compares every branch that has an endpoint with the radius of the junction to eliminate spurs
void spurCleanup(graph * g, vector<pixPoint*> & Points, char* outfilename, itk::Image<signed short, (unsigned)3> * origImage){ 
	
	int sum;

	sum = 0;

	//Because vertices may have multiple points we find the sum
	for (int u = 0; u < g->vertices[0]->edges[0]->v2->points.size(); u++){
		sum += findRadius(g->vertices[0]->edges[0]->v2->points[u], origImage);
	}

	//the sum is then divided to become an average
	//we compare the length of the branch we are looking to delete with this average
	if ((double)g->vertices[0]->edges[0]->slab.size() < 1.30*((double)sum / g->vertices[0]->edges[0]->v2->points.size())){
		for (int cnt = 0; cnt < g->vertices[0]->edges[0]->slab.size(); cnt++){
			//Point is then marked null. Output swc checks for null and skips over any points that are null during it's output loop
			Points[g->vertices[0]->edges[0]->slab[cnt]->visitNum - 1] = NULL;
		}
	}

	for (int i = 1; i < g->vertices.size(); i++){
		if (g->vertices[i]->edges.size() == 1){
			sum = 0;
			//sum for the average
			for (int u = 0; u < g->vertices[i]->edges[0]->v1->points.size(); u++){
				sum += findRadius(g->vertices[i]->edges[0]->v1->points[u], origImage);
			}

			//branch length compared to average and then removed by marking null if appropriate
			if ((double)g->vertices[i]->edges[0]->slab.size() < 1.30*((double)sum / g->vertices[i]->edges[0]->v1->points.size())){
				for (int cnt = 0; cnt < g->vertices[i]->edges[0]->slab.size(); cnt++){
					Points[g->vertices[i]->edges[0]->slab[cnt]->visitNum - 1] = NULL;
				}
			} 
			
		}
	}

}