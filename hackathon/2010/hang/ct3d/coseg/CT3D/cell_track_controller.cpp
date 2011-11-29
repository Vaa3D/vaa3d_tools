#include "../component_tree.h"
#include "cell_track.h"
#include "cell_track_controller.h"

#include <iostream>
#include <cstring>
#include <map>
#include <cassert>
using namespace std;

#ifndef INT_MAX
#define INT_MAX       2147483647
#endif

CellTrackController::CellTrackController()
{
	celltrack = NULL;
	current_time = 0;
}

CellTrackController::~CellTrackController()
{
}

bool CellTrackController::createCellTrack(vector<string> tree_files)
{
	this->clear();
	celltrack = new CellTrack();	
	return celltrack->createFromTrees(tree_files);
}

bool CellTrackController::createCellTrack(vector<string> image_files, int _min, int _max, int _single)
{
	if(_min > _max || _min < 0 || _single < 0 ) 
	{
		return false;
	}
	else
	{
		this->clear();
		ComponentTree *tree = new ComponentTree();
		vector<string> tree_files;
		for(int i = 0; i < (int)image_files.size(); i++)
		{
			tree->clear();
			tree->create((char*) image_files[i].c_str(), _min, _max, _single);
			//===============================================
			string tree_file = image_files[i];
			tree_file = tree_file.substr(0, tree_file.rfind("."));
			tree_file.append(".bin.tree");
			//===============================================
			tree->save((const char*)tree_file.c_str());
			tree_files.push_back(tree_file);
		}
		celltrack = new CellTrack();
		bool rt = celltrack->createFromTrees(tree_files);
		if(rt)
		{
			this->initTracksState();
		}
		/*
		vector<char*>::iterator it = tree_files.begin();
		while(it != tree_files.end())
		{
			delete (*it);
			it++;
		}
		*/

		return rt;
	}
}

bool CellTrackController::loadCellTrack(vector<string> image_results, vector<string> tree_files)
{
	this->clear();
	celltrack = new CellTrack();
	bool rt = celltrack->createFromImages(image_results);
	if(rt)
	{
		this->initTracksState();
		if(! tree_files.empty() && tree_files.size() == image_results.size())
		{
			if(!celltrack->correspondToTrees(tree_files))
			{
				cerr<<"loadCellTrack error : unable to match trees with cells of frames"<<endl;
			}
		}
	}
	return rt;
}

bool CellTrackController::loadCellTrack(string track_file, vector<string> tree_files)
{
	this->clear();
	celltrack = new CellTrack();
	return celltrack->load((char*)track_file.c_str());
}

void CellTrackController::setFirst()
{
	current_time = 0;
	cell_centers.clear();
}

void CellTrackController::setLast()
{
	current_time = celltrack->frameNum() - 1;
	cell_centers.clear();
}

void CellTrackController::setNext()
{
	if(current_time == (int)celltrack->frameNum() - 1)
	{
	}
	else
	{
		current_time++;
	}
	cell_centers.clear();
}

void CellTrackController::setPrev()
{
	if(current_time == 0)
	{
	}
	else
	{
		current_time--;
	}
	cell_centers.clear();
}
/************************************************************
 * Extract 3d texture of current frame
 * Including the marked area
 * **********************************************************/
#ifdef __v3d__
unsigned char* CellTrackController::getTexData()
{
	int w = getWidth();
	int h = getHeight();
	int d = getDepth();
	unsigned char* image = new  unsigned char[3*w*h*d];
	for(int i = 0; i < 3*w*h*d; i++) image[i] = 0;
	vector<CellTrack::Cell*> visable_cells = celltrack->getFrame(current_time)->getCells();
	vector<CellTrack::Cell*>::iterator it = visable_cells.begin();
	ComponentTree* tree = celltrack->getFrame(current_time)->getTree();
	while(it != visable_cells.end())
	{
		(*it)->draw(image, w, h, d);
		it++;
	}
	//cout<<"cell num = "<<visable_cells.size()<<endl;
	vector<CellTrack::Cell*> marked_cells = getMarkedCells();
	it = marked_cells.begin();
	while(it != marked_cells.end())
	{
		(*it)->drawMarker(image, w, h, d);
		it++;
	}
	return image;
}
#else
unsigned char* CellTrackController::getTexData()
{
	int w = getWidth();
	int h = getHeight();
	int d = getDepth();
	unsigned char* image = new  unsigned char[3*w*h*d];
	for(int i = 0; i < 3*w*h*d; i++) image[i] = 0;
	vector<CellTrack::Cell*> visable_cells = celltrack->getFrame(current_time)->getCells();
	vector<CellTrack::Cell*>::iterator it = visable_cells.begin();
	ComponentTree* tree = celltrack->getFrame(current_time)->getTree();
	while(it != visable_cells.end())
	{
		(*it)->draw(image);
		it++;
	}
	//cout<<"cell num = "<<visable_cells.size()<<endl;
	vector<CellTrack::Cell*> marked_cells = getMarkedCells();
	it = marked_cells.begin();
	while(it != marked_cells.end())
	{
		(*it)->drawMarker(image);
		it++;
	}
	return image;
}
#endif

int CellTrackController::currentTime()
{
	return current_time;
}

int CellTrackController::getWidth()
{
	return celltrack->getFrame(current_time)->width();
}

int CellTrackController::getHeight()
{
	return celltrack->getFrame(current_time)->height();
}

int CellTrackController::getDepth()
{
	return celltrack->getFrame(current_time)->depth();
}

CellTrack::Cell* CellTrackController::getClickedCell(float x, float y, float z)
{
	if(cell_centers.empty())
	{
		setCellCenters();
	}
	int w = this->getWidth();
	int h = this->getHeight();
	int d = this->getDepth();
	int min_dist = INT_MAX;
	CellTrack::Cell* obj_cell = NULL;
	vector<CellTrack::Cell*> cells = celltrack->getFrame(current_time)->getCells();
	vector<CellTrack::Cell*>::iterator it = cells.begin();
	while(it != cells.end())
	{
		vector<float>& center = cell_centers[*it];
		float center_x = center[0];
		float center_y = center[1];
		float center_z = center[2];
		float dist = (x - center_x)*(x - center_x) +
			(y - center_y)*(y - center_y) + 
			(z - center_z)*(z - center_z);
		if (dist < min_dist)
		{
			min_dist = dist;
			obj_cell = *it;
		}
		it++;
	}
	return obj_cell;
}
// todo : doxygen comment
vector<CellTrack::Cell*> CellTrackController::getMarkedCells()
{
	vector<CellTrack::Cell*> current_cells = celltrack->getFrame(current_time)->getCells();
	vector<CellTrack::Cell*> marked_cells;
	vector<CellTrack::Cell*>::iterator it = current_cells.begin();
	while(it != current_cells.end())
	{
		CellTrack::Cell* cell = *it;
		if(tracks_state[cell->getTrack()])
		{
			marked_cells.push_back(cell);
		}
		it++;
	}
	return marked_cells;
}

vector<CellTrack::Track*> CellTrackController::getMarkedTracks()
{
	vector<CellTrack::Track*> tracks;
	map<CellTrack::Track*,  bool>::iterator it = tracks_state.begin();
	while(it != tracks_state.end())
	{
		if((*it).second) tracks.push_back((*it).first);
		it++;
	}
	return tracks;
}

void CellTrackController::markTracks(vector<CellTrack::Track*> tracks)
{
	vector<CellTrack::Track*>::iterator it = tracks.begin();
	while(it != tracks.end())
	{
		tracks_state[*it] = true;
		it++;
	}
}

void CellTrackController::unMarkTracks(vector<CellTrack::Track*> tracks)
{
	vector<CellTrack::Track*>::iterator it = tracks.begin();
	while(it != tracks.end())
	{
		tracks_state[*it] = false;
		it++;
	}
}

void CellTrackController::markCell(CellTrack::Cell* cell)
{
	tracks_state[cell->getTrack()] = true;
}

void CellTrackController::unMarkCell(CellTrack::Cell* cell)
{
	tracks_state[cell->getTrack()] = false;
}

void CellTrackController::markCellsReversely()
{
	vector<CellTrack::Cell*> cells = celltrack->getFrame(current_time)->getCells();
	vector<CellTrack::Cell*>::iterator it = cells.begin();
	while(it != cells.end())
	{
		CellTrack::Cell* cell = *it;
		if(tracks_state[cell->getTrack()])
		{
			tracks_state[cell->getTrack()] = false;
		}
		else tracks_state[cell->getTrack()] = true;
		it++;
	}
}

void CellTrackController::initTracksState(vector<CellTrack::Track*> marked_tracks)
{
	assert(celltrack != NULL);
	tracks_state.clear();
	int track_num = celltrack->trackNum();
	for(int i = 0; i < track_num; i++)
	{
		CellTrack::Track* track = celltrack->getTrack(i);
		tracks_state[track] = false;
	}

	if(!marked_tracks.empty())
	{
		vector<CellTrack::Track*>::iterator it = marked_tracks.begin();
		while(it != marked_tracks.end())
		{
			assert(tracks_state.find(*it) != tracks_state.end());
			tracks_state[*it] = true;
			it++;
		}
	}
}

/************************************************************
 * tracks start from later frame are called unvisited tracks
 * **********************************************************/
void CellTrackController::choose(bool keep_unvisited_tracks)
{
	CellTrack* old_celltrack = celltrack;
	vector<CellTrack::Track*> tracks = this->getMarkedTracks();
	if(keep_unvisited_tracks)
	{
		vector<CellTrack::Track*> all_tracks = tracks;
		int frame_num = celltrack->frameNum();
		for(int i = current_time  + 1 ; i < frame_num; i++)
		{
			vector<CellTrack::Cell*> cells = celltrack->getFrame(i)->getCells();
			vector<CellTrack::Cell*>::iterator it = cells.begin();
			while(it != cells.end())
			{
				CellTrack::Cell* cell = *it;
				CellTrack::Track* track = cell->getTrack();
				//if appeare in later frame and not manually track
				if(cell->getPrevCell() == NULL && !tracks_state[track])
				{
					all_tracks.push_back(cell->getTrack());
				}
				it++;
			}
		}
		celltrack = celltrack->choose(all_tracks);
	}
	else
	{
		celltrack = celltrack->choose(tracks);
	}
	//this->initTracksState(/*tracks*/);
	this->markTracks(tracks);
	pushState(old_celltrack);
	cell_centers.clear();
}

void CellTrackController::remove()
{
	CellTrack* old_celltrack = celltrack;
	vector<CellTrack::Track*> tracks = this->getMarkedTracks();
	celltrack = celltrack->remove(tracks);
	this->unMarkTracks(tracks);
	pushState(old_celltrack);
	cell_centers.clear();
}

void CellTrackController::pushState(CellTrack* old)
{
	history.push_back(old);
}

CellTrack* CellTrackController::popState()
{
	if(history.empty())
	{
		return NULL;
	}
	else
	{
		CellTrack* rt = history.back();
		history.pop_back();
		return rt;
	}
}

void CellTrackController::undo()
{
	if(!history.empty())
	{
		//vector<CellTrack::Track*> tracks = this->getMarkedTracks();
		celltrack->releaseFrames();
		//delete celltrack;
		celltrack = this->popState();
	}
}

void CellTrackController::clear()
{
	if(celltrack!=NULL)
	{
		celltrack->releaseAllCells();
		delete celltrack;
		celltrack=NULL;
	}
	history.clear();
	tracks_state.clear();
	cell_centers.clear();
}

void CellTrackController::setCellCenters()
{
	assert(cell_centers.empty());
	int w = this->getWidth();
	int h = this->getHeight();
	int d = this->getDepth();
	vector<CellTrack::Cell*> cells = celltrack->getFrame(current_time)->getCells();
	vector<CellTrack::Cell*>::iterator it = cells.begin();
	while(it != cells.end())
	{
		float mean_x, mean_y, mean_z;
		(*it)->getCenter(mean_x, mean_y, mean_z);
		vector<float> center;
		center.push_back(mean_x);
		center.push_back(mean_y);
		center.push_back(mean_z);
		cell_centers[*it] = center;
		it++;
	}
}
