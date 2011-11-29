//
//=======================================================================
// Copyright 2011 Institute PICB.
// Authors: Hang Xiao
// Data : March 20, 2011
//=======================================================================
//


#include "cell_track.h"
#include "palette.h"
#include "../component_tree.h"
#include "../myalgorithms.h"
#include "bipartite.h"

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <cassert>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "lp_lib.h"
#include "my_surf_objs.h"

#ifndef INT_MAX
#define INT_MAX       2147483647
#endif

#ifndef EPSILON
#define EPSILON 0.00001
#endif

using namespace std;
typedef ComponentTree::Node TNode;

vector<char*> vec_chars(vector<string> & strings)
{
	vector<char*> chars_vec;
	vector<string>::iterator it = strings.begin();
	while(it != strings.end())
	{
		string str = *it;
		chars_vec.push_back((char*)str.c_str());
		it++;
	}
	return chars_vec;
}

/***************************************************************************
 * CellTrack construction function
 **************************************************************************/
CellTrack::CellTrack()
{
	m_method = 0;
}

CellTrack::~CellTrack()
{
	this->releaseFrames();
	this->releaseTracks();
}

void CellTrack::setMethod(int method)
{
	m_method = method;
}

bool CellTrack::save(char* track_file)
{
	assert(!m_frames.empty());
	string str_file(track_file);

	bool saveType = 1;
	ofstream ofs;
	if(str_file.find(".bin") != string::npos)
	{
		saveType = 0;
		ofs.open(str_file.c_str(), ios_base::out|ios_base::binary);
	}
	else if(str_file.find(".txt") != string::npos)
	{
		saveType = 1;
		ofs.open(str_file.c_str());
	}
	if(save(ofs, saveType))
	{
		ofs.close();
		return true;
	}
	else
	{
		ofs.close();
		return false;
	}
}

bool CellTrack::save(ofstream & ofs, bool saveType)
{
	if(saveType == 0) // binary
	{
	}
	else
	{
	}
	if(ofs.good()) return true;
	else return false;
}

bool CellTrack::load(char* track_file)
{
	assert(m_frames.empty());
	ifstream ifs;
	bool saveType = true;
	string str_file(track_file);
	if(str_file.find(".bin") != string::npos)
	{
		cout<<"load binary file: "<<track_file<<endl;
		saveType = false;
		ifs.open(track_file, ios_base::in|ios_base::binary);
	}
	else if(str_file.find(".txt") != string::npos)
	{
		cout<<"load txt file: "<<track_file<<endl;
		saveType = true;
		ifs.open(track_file);
	}
	else
	{
		cerr<<"Load Tree File: Cann't analysis file type."<<endl;
		cerr<<"Make sure .bin.tree file as binary and .txt.tree file as txt file"<<endl;
		return false;
	}
	bool rt = load(ifs, saveType);
	ifs.close();
	return rt;
}

bool CellTrack::load(ifstream& ifs, bool saveType)
{
	if(saveType == 0) // binary
	{
	}
	else
	{
	}
	if(ifs.good()) return true;
	else return false;
}

bool CellTrack::reload(char* track_file)
{
	this->releaseAllCells();
	this->releaseFrames();
	this->releaseTracks();
	return load(track_file);
}

bool CellTrack::createFromImages(vector<string> img_files)
{
	return createFromImages(vec_chars(img_files));
}
bool CellTrack::createFromImages(vector<char*> img_files)
{
	assert(m_frames.empty());
	assert(!img_files.empty());
	if(! createFramesFromImages(img_files, m_frames))
	{
		cerr<<"unable to create frames from images "<<endl;
		return false;
	}
	if(! createTracksFromFrames(m_frames, m_tracks))
	{
		cerr<<"unable to create tracks from frames"<<endl;
		return false;
	}
	//setTracksColor(); // the color of Tracks is alread set
	//this->m_img_files = img_files;
	return true;
}

bool CellTrack::correspondToTrees(vector<string> tree_files)
{
	return correspondToTrees(vec_chars(tree_files));
}
bool CellTrack::correspondToTrees(vector<char*> tree_files)
{
	assert(tree_files.size() == frameNum());
	for(int i = 0; i < (int)frameNum(); i++)
	{
		ComponentTree* tree = new ComponentTree(tree_files[i]);
		if(!m_frames[i]->correspondToTree(tree))
		{
			cerr<<"correspondToTree error: Frame "<<i<<"+1 unable to match tree"<<endl;
			return false;
		}
	}
	return true;
}

bool CellTrack::createFromTrees(vector<string> tree_files)
{
	return createFromTrees(vec_chars(tree_files));
}
bool CellTrack::createFromTrees(vector<char*> tree_files)
{
	assert(m_frames.empty());
	if(!createFramesFromTrees(tree_files, m_frames))
	{
		cerr<<"unable to create frames from trees "<<endl;
		return false;
	}
	if(!createTracksFromFrames(m_frames, m_tracks))
	{
		cerr<<"unable to create tracks from frames"<<endl;
		return false;
	}
	setTracksColor();
	//m_tree_files = tree_files;
	return true;
}

void CellTrack::setTracksColor()
{
	assert(this->m_tracks.size() > 0);
	Palette palette(m_tracks.size());
	vector<Track*>::iterator it = this->m_tracks.begin();
	int i = 0;
	while(it != m_tracks.end())
	{
		unsigned int r = palette(i).r;
		unsigned int g = palette(i).g;
		unsigned int b = palette(i).b;
		int color = r + g * 256 + b * 256 * 256; 
		(*it)->setColor(color);
		i++;
		it++;
	}
}

void CellTrack::exportSWCS()
{
	vector<Track*>::iterator it = this->m_tracks.begin();
	int i = 0;
	while(it != m_tracks.end())
	{
		ostringstream oss;
		oss<<"track"<<i<<".swc";
		string swcfile = oss.str();
		Track* track = *it;
		vector<Cell*> cells = track->getCells();
		if(cells.size()<3){it++;i++; continue;}
		cout<<cells.size()<<endl;
		vector<Cell*>::iterator itr = cells.begin();

		vector<MyMarker*> allmarkers;

		MyMarker* p = 0;
		while(itr != cells.end())
		{
			Cell* cell = *itr;
			float x,y,z;
			cell->getCenter(x,y,z);
			MyMarker* newmarker = new MyMarker();
			newmarker->x = x;
			newmarker->y = y;
			newmarker->z = z;
			newmarker->parent = p;
			allmarkers.push_back(newmarker);
			p = newmarker;
			itr++;
		}
		saveSWC_file(swcfile, allmarkers);
		it++;
		i++;
	}
}

void CellTrack::exportImages(char* prefix, char* dir) const
{
	vector<Frame*>::const_iterator it = m_frames.begin();
	int id = 0;
	//int color_num = this->trackNum();
	//Palette palette(color_num);
	//setTrackColor();
	while(it != m_frames.end())
	{
		id++;
		ostringstream oss(ostringstream::out);
		if(dir != NULL) oss<< dir <<"/";
		if(prefix != NULL) oss << prefix;
		oss << id;
		oss << ".tiff";
		(*it)->exportImage((char*) oss.str().c_str()/*, palette*/);
		it++;
	}
}
/************************************************************
 * chooseLocally: choose cells marked in current frame and 
 * previous frames as well as all the cells appeared only in
 * later frames
 *
 * When talking about choose , we mean chooseLocally
 *
 * Note: only frame is new created
 ************************************************************/
/*CellTrack* CellTrack::chooseLocally(vector<CellTrack::Track*>  tracks, int frame_id)
{
	assert(frame_id >= 0 && frame_id < frameNum());
	if(frame_id < frameNum() - 1) 
	{
		for(int i = frame_id + 1 ; i < frameNum(); i++)
		{
			vector<Cell*> cells = this->getFrame(i)->getCells();
			vector<Cell*>::iterator it = cells.begin();
			while(it != cells.end())
			{
				Cell* cell = *it;
				if(cell->getPrevCell() == NULL)
				{
					tracks.push_back(cell->getTrack());
				}
				it++;
			}
		}
	}
	return chooseGlobally(tracks);
}
*/

/************************************************************
 * chooseGlobally: choose cells marked in current frame and 
 * previous frames, the cells in later frames will be deleted
 *
 * Note : only frame is new created
 ************************************************************/
CellTrack* CellTrack::choose(vector<CellTrack::Track*> tracks)
{
	int frame_num = frameNum();
	CellTrack* ct = new CellTrack();
	ct->m_tracks = tracks;
	ct->m_frames.resize(frame_num);
	set<Track*> set_tracks(tracks.begin(), tracks.end());
	assert(set_tracks.size() == tracks.size());

	for(int i = 0; i < frame_num; i++)
	{
		Frame* frame_this = this->getFrame(i);
		Frame* frame_new = new Frame();
		frame_new->m_width = frame_this->width();
		frame_new->m_height = frame_this->height();
		frame_new->m_depth = frame_this->depth();
		frame_new->m_tree_file = frame_this->m_tree_file;
		frame_new->m_tree = frame_this->m_tree;

		vector<Cell*> cells = frame_this->getCells();
		vector<Cell*>::iterator it = cells.begin();
		while(it != cells.end())
		{
			if(set_tracks.find((*it)->getTrack()) != set_tracks.end())
			{
				frame_new->addCell(*it);
			}
			it++;
		}
		ct->m_frames[i] = frame_new;
	}
	return ct;
}

/***************************************************************************
 * Remove marked tracks
 * *************************************************************************/
CellTrack* CellTrack::remove(vector<CellTrack::Track*> tracks)
{
	set<Track*> set_tracks(tracks.begin(), tracks.end());
	vector<Track*> rev_tracks;
	for(int i = 0; i < (int)this->trackNum(); i++)
	{
		Track* track = this->getTrack(i);
		if(set_tracks.find(track) == set_tracks.end()) rev_tracks.push_back(track);
	}
	return choose(rev_tracks);
}
/************************************************************
 * get Frames with id >= frame_start_id, < frame_end_id
 *
 * we will use the same pointer of frame and track
 * **********************************************************/
CellTrack* CellTrack::clip(int frame_start_id, int frame_end_id)
{
	assert(frame_start_id >= 0 && 
			frame_start_id <= frame_end_id &&
			frame_end_id <= (int)frameNum());

	CellTrack* ct = new CellTrack();
	ct->m_frames.resize(frame_end_id - frame_start_id);
	vector<Track*> tracks;
	for(int i = frame_start_id; i < frame_end_id; i++)
	{
		Frame* frame = this->getFrame(i);
		ct->m_frames[i - frame_start_id] = frame;
		vector<Cell*> cells = frame->getCells();
		vector<Cell*>::iterator it = cells.begin();
		while(it != cells.end())
		{
			Cell* cell = *it;
			if(cell->getPrevCell() == NULL)
			{
				tracks.push_back(cell->getTrack());
			}
			it++;
		}
	}
	ct->m_tracks = tracks;
	return ct;
}

CellTrack::Frame* CellTrack::getFrame(int time) const
{
	if(m_frames.empty()) return NULL;
	if(time >= (int)this->frameNum()) return NULL;
	return m_frames[time];
}

CellTrack::Track* CellTrack::getTrack(int index) const
{
	assert(! m_tracks.empty());
	assert(index < (int)this->trackNum());
	return m_tracks[index];
}

unsigned int CellTrack::frameNum() const
{
	return m_frames.size();
}

unsigned int CellTrack::trackNum() const
{
	return m_tracks.size();
}

void CellTrack::releaseFrames()
{
	if(!m_frames.empty())
	{
		vector<Frame*>::iterator it = m_frames.begin();
		while(it != m_frames.end()) 
		{
			delete *it;
			it++;
		}
		m_frames.clear();
	}
}
/************************************************************************
 * Becareful here, each cell have a pointer to track which will be free,
 * So make sure releaseTracks and releaseAllCells are done simulataneously 
 * **********************************************************************/
void CellTrack::releaseTracks()
{
	if(!m_tracks.empty())
	{
		vector<Track*>::iterator it = m_tracks.begin();
		while(it != m_tracks.end())
		{
			/*
			Cell* p = (*it)->getStartCell();
			p->setTrack(NULL);
			while(p->getNextCell() != NULL) 
			{
				p = p->getNextCell();
				p->setTrack(NULL);
			}*/
			delete *it;
			it++;
		}
		m_tracks.clear();
	}
}
/**************************************************************************
 * After releaseTracks, we may need to releaseAllCells
 * ************************************************************************/
void CellTrack::releaseAllCells()
{
	if(!m_tracks.empty())
	{
		vector<Track*>::iterator it = m_tracks.begin();
		while(it != m_tracks.end())
		{
			Cell* p = (*it)->getStartCell();
			while(p->getNextCell() != NULL) 
			{
				Cell* next_cell = p->getNextCell();
				delete p;
				p = next_cell;
			}
			delete p;
			(*it)->setStartCell(NULL);
			it++;
		}
	}
	/*========== old implementation ============
	int frame_num = frameNum();
	for(int i = 0; i < frame_num; i++)
	{
		vector<Cell*> cells = this->getFrame(i)->getCells();
		vector<Cell*>::iterator it = cells.begin();
		while(it != cells.end())
		{
			delete (*it);
			it++;
		}
	}
	===========================================*/
}

int max_item(vector<float> items)
{
	if(items.empty()) return -1;
	int numItem = items.size();
	float max_value = 0.0;
	int  max_label = 0;
	for(int i = 0; i < numItem; i++)
	{
		if(i == 0)
		{
			max_value = items[0];
			max_label = 0;
		}
		else
		{
			if(items[i] > max_value)
			{
				max_value = items[i];
				max_label = i;
			}
		}
	}
	return max_label;
}

bool CellTrack::createFramesFromTrees(ComponentTree* tree1, ComponentTree* tree2,vector<CellTrack::Frame*> &frames)
{
	if(m_method == 0) // ILP programming
	{
		assert(frames.empty());

		if(tree1->width() != tree2->width() || tree1->height() != tree2->height() || tree1->depth() != tree2->depth())
		{
			cerr<<"The two trees with different size. Unalbe to align."<<endl;
			return false;
		}

		// 1. get weights
		vector<float> weights;
		tree1->setWeightMatrix(tree2, weights);
		int numVars1 = (int)tree1->nodeNum();
		int numVars2 = (int)tree2->nodeNum();
		assert((int)weights.size() == numVars1 * numVars2);
		// Forbid the assignment of root node
		weights[numVars1*numVars2 - 1] = 0.0;
		/*
		   for(int i = 0; i < numVars1; i++)
		   {
		   for(int j = 0; j < numVars2; j++)
		   {
		   cout<<weights[i*numVars2 + j]<<" ";
		   }
		   cout<<endl;
		   }
		   */
		vector<vector<int> > paths1 = tree1->getPaths();
		vector<vector<int> > paths2 = tree2->getPaths();

		// 2. linear model
		lprec *lp;
		int Ncol, *colno=NULL, k;       
		REAL * row = NULL;
		int i=0,j=0;
		Ncol = numVars1 * numVars2; 
		lp = make_lp(0,Ncol);           
		if(lp == NULL) return false;    

		colno = (int *) malloc(Ncol * sizeof(*colno));
		row = (REAL *) malloc(Ncol * sizeof(*row));
		if((colno == NULL) || (row == NULL)) return false;
		for(i=0;i< Ncol;i++) row[i]=1.0; // assign all the content of row as 1

		set_add_rowmode(lp,TRUE);
		set_binary(lp,Ncol,TRUE);

		// add paths1 constraints
		// one path one constraint

		ComponentTree::Paths::iterator it=paths1.begin();
		while(it != paths1.end())
		{
			k=0;
			ComponentTree::Path::iterator itr = (*it).begin();
			while(itr != (*it).end())
			{
				i = (*itr);
				for(j=0;j<numVars2;j++)colno[k++] = i*numVars2+j+1;
				itr++;
			}
			if(!add_constraintex(lp, k, row, colno, LE, 1))
				return false;
			it++;
		}
		// add paths2 constraints
		it=paths2.begin();
		while (it != paths2.end())
		{
			ComponentTree::Path::iterator itr = (*it).begin();
			k=0;
			while(itr != (*it).end())
			{
				j = (*itr);
				for(i=0;i<numVars1;i++)colno[k++] = i*numVars2+j+1;
				itr++;
			}
			if(!add_constraintex(lp, k, row, colno, LE, 1))
				return false;
			it++;
		}
		set_add_rowmode(lp,FALSE);	

		// add the object
		k=0;
		for(i=0;i< numVars1; i++)
			for(j=0; j< numVars2; j++)
			{
				colno[k] = i*numVars2+j+1; //todo: why i*numVar2 + j + 1
				row[k++] = weights[i * numVars2 + j];
			}
		if(!set_obj_fnex(lp, k, row, colno))return false;
		set_maxim(lp);
		set_verbose(lp,IMPORTANT);

		// 3. solve the linear problem
		if(::solve(lp) != OPTIMAL)
		{
			cout<<"Not optimized results"<<endl;
			return false;
		}
		// 4. save results to row
		get_variables(lp,row);
		Frame* frame1 = new Frame();
		Frame* frame2 = new Frame();
		frame1->setTree(tree1);
		frame2->setTree(tree2);
		
		float sum_weights = 0.0;
		for(i = 0; i < numVars1; i++)
		{
			for(int j = 0; j < numVars2; j++)
			{
				if(fabs(row[i * numVars2 + j] - 1.0) < 0.1 /*&& weights[i*numVars2 + j] > 0.01*/)
				{
					cout<<"("<<i<<","<<j<<")"<<weights[i*numVars2 + j]<<" ";
					sum_weights += weights[i*numVars2 + j];
					Cell* cell1 = new Cell;
					Cell* cell2 = new Cell;
					cell1->setTree(tree1);
					cell2->setTree(tree2);
					cell1->setCurNodeLabel(i);
					cell2->setCurNodeLabel(j);
					cell1->setNextCell(cell2);
					cell2->setPrevCell(cell1); 
					frame1->addCell(cell1);
					frame2->addCell(cell2);
				}
			}
		}
		cout<<endl;
		cout<<"sum of weight : "<<sum_weights<<endl;

		frames.push_back(frame1);
		frames.push_back(frame2);

		if(colno != NULL) free(colno);
		if(row != NULL) free(row);
		if(lp != NULL) delete_lp(lp);

		return true;
	}
	else if(m_method == 1)  // three point condition
	{
		assert(frames.empty());
		if(tree1->width() != tree2->width() || tree1->height() != tree2->height() || tree1->depth() != tree2->depth())
		{
			cerr<<"The two trees with different size. Unalbe to align."<<endl;
			return false;
		}
		// 1. get weights
		vector<float> weights;
		tree1->setWeightMatrix(tree2, weights);
		//assert((int)weights.size() == numVars1 * numVars2);

		vector<ComponentTree::Node*> nodes1 = tree1->root()->getPostOrderNodes();
		vector<ComponentTree::Node*> nodes2 = tree2->root()->getPostOrderNodes();

		int numVars1 = (int)tree1->nodeNum();
		int numVars2 = (int)tree2->nodeNum();
		// output weights
//		cout<<"=========================weights[i,j]========================="<<endl;
//		for(int j = 0; j < numVars2; j++) cout<<"\t  "<<j;
//		cout<<endl;
//		for(int i = 0; i < numVars1; i++)
//		{
//			cout<<i<<":\t";
//			for(int j = 0; j < numVars2; j++)
//			{
//				float weight = weights[i * numVars2 + j];
//				weight = (int)(weight * 100)/100.0;
//				cout<<weight<<"\t";
//			}
//			cout<<endl;
//		}
		vector<float> wvt(numVars1*numVars2, 0.0);  // weight of specific vertex -> subtree
		vector<vector<int> > rev_wvt(numVars1*numVars2, vector<int>());
		vector<float> wtv(numVars1*numVars2, 0.0); // weight of subtree -> specific vertex
		vector<vector<int> > rev_wtv(numVars1*numVars2, vector<int>());
		//vector<float> wff(numVars1*numVars2, 0.0); // weight of forest to forest
		//vector<vector<int> > rev_wff(numVars1*numVars2, vector<int>());
		vector<float> wtt(numVars1*numVars2, 0.0); // weight of tree to tree
		vector<vector<int> > rev_wtt(numVars1*numVars2, vector<int>());

		//calc wvt, wvt[v][u] = max{wvv[v][u] , wvt[v][c1], wvt[v][c2], ...}
		for(int i = 0; i < numVars1; i++)
		{
			ComponentTree::Node* node1 = nodes1[i];
			int label1 = node1->getLabel();
			assert(i == label1);
			for(int j = 0; j < numVars2; j++)
			{
				ComponentTree::Node* node2 = nodes2[j];
				int label2 = node2->getLabel();
				assert(j == label2);
				vector<ComponentTree::Node*> & childs = node2->getChilds(); 
				float max_weight = weights[label1 * numVars2 + label2];
				int max_label = label2;
				vector<ComponentTree::Node*>::iterator it = childs.begin();
				while(it != childs.end())
				{
					int child_label = (*it)->getLabel();
					float child_weight = wvt[label1 * numVars2 + child_label];
					if(child_weight > max_weight)
					{
						max_weight = child_weight;
						max_label = child_label;
					}
					it++;
				}
				wvt[label1*numVars2 + label2] = max_weight;
				if(max_weight > weights[label1 * numVars2 + label2])
				{
					rev_wvt[label1*numVars2 + label2] = rev_wvt[label1*numVars2 + max_label];
					//cout<<label1<<"->"<<rev_wvt[label1*numVars2 + max_label][1]<<"\t";
				}
				else 
				{
					vector<int> match;
					match.push_back(label1);
					match.push_back(label2);
					rev_wvt[label1*numVars2 + label2] = match;
					//cout<<label1<<"->"<<label2<<"\t";
				}
			}
			//cout<<endl;
		}
		// output wvt
//		cout<<"====================wvt[i,j]========================="<<endl;
//		//for(int j = 0; j < numVars2; j++) cout<<"\t "<<j;
//		for(int i = 0; i < numVars1; i++)
//		{
//			//cout<<i<<"\t";
//			for(int j = 0; j < numVars2; j++)
//			{
//				float weight = wvt[i * numVars2 + j];
//				weight = (int)(weight * 100)/100.0;
//				cout<<rev_wvt[i*numVars2 + j][0]<<"->"<<rev_wvt[i*numVars2 + j][1]<<"("<<weight<<")"<<"\t";
//			}
//			cout<<endl;
//		}

		//calc wtv, wtv[v][u] = max{wvv[v][u] , wtv[c1][u], wtv[c2][u], ...}
		for(int j = 0; j < numVars2; j++)
		{
			ComponentTree::Node* node2 = nodes2[j];
			int label2 = node2->getLabel();
			for(int i = 0; i < numVars1; i++)
			{
				ComponentTree::Node* node1 = nodes1[i];
				int label1 = node1->getLabel();
				vector<ComponentTree::Node*> & childs = node1->getChilds(); 
				float max_weight = weights[label1 * numVars2 + label2];
				int max_label = label1;
				vector<ComponentTree::Node*>::iterator it = childs.begin();
				while(it != childs.end())
				{
					int child_label = (*it)->getLabel();
					float child_weight = wtv[child_label * numVars2 + label2];
					if(child_weight > max_weight)
					{
						max_weight = child_weight;
						max_label = child_label;
					}
					it++;
				}
				wtv[label1*numVars2 + label2] = max_weight;
				if(max_weight > weights[label1 * numVars2 + label2]) rev_wtv[label1*numVars2 + label2] = rev_wtv[max_label*numVars2 + label2];
				else 
				{
					vector<int> match;
					match.push_back(label1);
					match.push_back(label2);
					rev_wtv[label1*numVars2 + label2] = match;
				}
			}
		}
		// output wtv
//		cout<<"====================wtv[i,j]========================="<<endl;
//		//for(int j = 0; j < numVars2; j++) cout<<"\t "<<j;
//		for(int i = 0; i < numVars1; i++)
//		{
//			//cout<<i<<"\t";
//			for(int j = 0; j < numVars2; j++)
//			{
//				float weight = wtv[i * numVars2 + j];
//				weight = (int)(weight * 100)/100.0;
//				cout<<rev_wtv[i*numVars2 + j][0]<<"->"<<rev_wtv[i*numVars2 + j][1]<<"("<<weight<<")"<<"\t";
//			}
//			cout<<endl;
//		}
		// calc wtt
		// wtt[v][u] = max{wvt[v][u], wtv[v][u], bipartite maching between children of v and children of u}
		for(int i = 0; i < numVars1; i++)
		{
			ComponentTree::Node* node1 = nodes1[i];
			int label1 = node1->getLabel();
			vector<ComponentTree::Node*>& childs1 = node1->getChilds();
			int numChilds1 = childs1.size();
			for(int j = 0; j < numVars2; j++)
			{
				ComponentTree::Node* node2 = nodes2[j];
				int label2 = node2->getLabel();
				vector<ComponentTree::Node*>& childs2 = node2->getChilds();
				int numChilds2 = childs2.size(); 
//				if(numChilds1 == 0 || numChilds2 == 0) 
//				{
//					wff[label1 * numVars2 + label2] = 0.0;
//					rev_wff[label1 * numVars2 + label2].clear();
//				}
				if(numChilds1 == 0)  // leaf node
				{
					wtt[label1 * numVars2 + label2] = wvt[label1 * numVars2 + label2];
					rev_wtt[label1 * numVars2 + label2] = rev_wvt[label1 * numVars2 + label2];
				}
				else if(numChilds2 == 0)
				{
					wtt[label1 * numVars2 + label2] = wtv[label1 * numVars2 + label2];
					rev_wtt[label1 * numVars2 + label2] = rev_wtv[label1 * numVars2 + label2];
				}
				else
				{
					vector<float> three_weights(3, 0.0);
					vector<vector<int> > three_matches(3, vector<int>());
					three_weights[0] = wvt[label1*numVars2 + label2];
					three_matches[0] = rev_wvt[label1*numVars2 + label2];

					three_weights[1] = wtv[label1*numVars2 + label2];
					three_matches[1] = rev_wtv[label1*numVars2 + label2];
					vector<float> children_weights(numChilds1 * numChilds2, 0.0);
					for(int ii = 0; ii < numChilds1; ii++)
					{
						ComponentTree::Node* child1 = childs1[ii];
						for(int jj = 0; jj < numChilds2;jj++)
						{
							ComponentTree::Node* child2 = childs2[jj];
							children_weights[ii * numChilds2 + jj] = wtt[child1->getLabel() * numVars2 + child2->getLabel()];
						}
					}
					vector<int> ids1, ids2;
					vector<int> match;
					three_weights[2] = bipartite_matching(children_weights, numChilds1, numChilds2, ids1, ids2);
					assert(ids1.size() == ids2.size());
					for(int k = 0; k < (int)ids1.size(); k++)
					{
						int id1 = childs1[ids1[k]]->getLabel();
						int id2 = childs2[ids2[k]]->getLabel();
						vector<int>& child_match = rev_wtt[id1 * numVars2 + id2];
						match.insert(match.end(), child_match.begin(), child_match.end());
					}
					three_matches[2] = match;
					//wff[label1*numVars2 + label2] = three_weights[2];
					//rev_wff[label1*numVars2 + label2] = three_matches[2];
					int which_item = max_item(three_weights);
					wtt[label1*numVars2 + label2] = three_weights[which_item];
					rev_wtt[label1*numVars2 + label2] = three_matches[which_item];
				}
			}
		}
		// output wff
//		cout<<"====================wff[i,j]========================="<<endl;
//		//for(int j = 0; j < numVars2; j++) cout<<"\t "<<j;
//		for(int i = 0; i < numVars1; i++)
//		{
//			//cout<<i<<"\t";
//			for(int j = 0; j < numVars2; j++)
//			{
//				float weight = wff[i * numVars2 + j];
//				weight = (int)(weight * 100)/100.0;
//				int num = rev_wff[i*numVars2 + j].size()/2;
//				if(num >= 1)
//				{
//					cout<<"("<<i<<","<<j<<")";
//					for(int k = 0; k < num; k++)
//					{
//						cout<<"["<<rev_wff[i*numVars2 + j][2*k]<<"->"<<rev_wff[i*numVars2 + j][2*k+1]<<"]";
//					}
//					cout<<"("<<weight<<")"<<"\t";
//				}
//			}
//			cout<<endl;
//		}
//		// output wtt
//		cout<<"====================wtt[i,j]========================="<<endl;
//		//for(int j = 0; j < numVars2; j++) cout<<"\t "<<j;
//		for(int i = 0; i < numVars1; i++)
//		{
//			//cout<<i<<"\t";
//			for(int j = 0; j < numVars2; j++)
//			{
//				float weight = wtt[i * numVars2 + j];
//				weight = (int)(weight * 100)/100.0;
//				int num = rev_wtt[i*numVars2 + j].size()/2;
//				if(num > 1)
//				{
//				for(int k = 0; k < num; k++)
//				{
//					cout<<"["<<rev_wtt[i*numVars2 + j][2*k]<<"->"<<rev_wtt[i*numVars2 + j][2*k+1]<<"]";
//				}
//				cout<<"("<<weight<<")"<<"\t";
//				}
//				else
//				{
//					cout<<rev_wtt[i*numVars2 + j][0]<<"->"<<rev_wtt[i*numVars2 + j][1]<<"("<<weight<<")"<<"\t";
//				}
//			}
//			cout<<endl;
//		}
		Frame* frame1 = new Frame();
		Frame* frame2 = new Frame();
		frame1->setTree(tree1);
		frame2->setTree(tree2);
	
		vector<int>& match = rev_wtt.back();
		//cout<<"match num : "<<match.size()<<endl;
		assert(match.size() % 2 == 0);
		int match_num = match.size() / 2;
		for(int k = 0; k < match_num; k++)
		{
			int i = match[2*k];
			int j = match[2*k + 1];
			cout<<"("<<i<<","<<j<<")"<<weights[i * numVars2 + j]<<" ";
			Cell* cell1 = new Cell;
			Cell* cell2 = new Cell;
			cell1->setTree(tree1);
			cell2->setTree(tree2);
			cell1->setCurNodeLabel(i);
			cell2->setCurNodeLabel(j);
			cell1->setNextCell(cell2);
			cell2->setPrevCell(cell1); 
			frame1->addCell(cell1);
			frame2->addCell(cell2);
		}
		cout<<endl;
		cout<<"sum of weight : "<<wtt.back()<<endl;	
		frames.push_back(frame1);
		frames.push_back(frame2);

		
		return true;
	}
}

bool CellTrack::createFramesFromTrees(vector<char*> tree_files,vector<CellTrack::Frame*> &frames)
{
	assert(frames.empty());
	assert(tree_files.size() >= 2);
	if(tree_files.size() == 2)
	{
		ComponentTree* tree1 = new ComponentTree(tree_files[0]);
		ComponentTree* tree2 = new ComponentTree(tree_files[1]);
		bool rt = createFramesFromTrees(tree1, tree2, frames);
		assert(frames.size() == 2);
		Frame* frame1 = frames[0];
		Frame* frame2 = frames[1];
		//frame1->releaseTree(tree_files[0]);
		//frame2->releaseTree(tree_files[1]);
		return rt;
	}

	frames.resize(tree_files.size());
	int numFrames = frames.size();

	Frame* prev_frame = NULL;
	ComponentTree* first_tree = NULL;
	ComponentTree* second_tree = NULL;
	first_tree = new ComponentTree(tree_files[0]);
	int i = 0;
	for(; i < numFrames-1; i++)
	{
		pair<Frame*, Frame*> frame_pair;
		second_tree = new ComponentTree(tree_files[i+1]);
		vector<Frame*> two_frames;
		if(!createFramesFromTrees(first_tree, second_tree, two_frames))
		{
			cerr<<"Unable to produce frames from two trees"<<endl;
			return false;
		}

		first_tree = second_tree;

		frames[i] = two_frames[0];
		frames[i]->mergePrevFrame(prev_frame);
		//frames[i]->releaseTree(tree_files[i]);          // if space is not enough
		prev_frame = two_frames[1];
	}
	frames[i] = prev_frame;
	//frames[i]->releaseTree(tree_files[i]);
	return true;
}

bool CellTrack::createFramesFromImages(vector<char*> img_files, vector<CellTrack::Frame*> &frames)
{
	assert(!img_files.empty());
	assert(frames.empty());
	int frame_num = img_files.size();
	frames.resize(frame_num);
	map<unsigned int, Cell*> map_cell;
	for(int i = 0;i < frame_num; i++)
	{
		frames[i] = new Frame;
		map_cell = frames[i]->createFromImage(img_files[i], map_cell);
	}
	return true;
}


bool CellTrack::createTracksFromFrames(CellTrack::Frames& frames, vector<CellTrack::Track*> &tracks, int start_time)
{
	assert(!frames.empty());
	assert(tracks.empty());
	vector<Frame*>::iterator it = frames.begin();
	int t = start_time;
	int track_id = 0;
	while(it != frames.end())
	{
		vector<Cell*> cells = (*it)->getCells();
		vector<Cell*>::iterator itr = cells.begin();
		while(itr != cells.end())
		{
			if((*itr)->getPrevCell() == NULL)
			{
				Track* track = new Track;
				track->m_start_time = t;
				track->m_entry_cell = (*itr);
				track->m_color = (*itr)->m_color;
				track->m_track_id = track_id++;
				(*itr)->m_track = track;
				tracks.push_back(track);
			}
			else
			{
				(*itr)->m_track = (*itr)->getPrevCell()->getTrack();
			}
			itr++;
		}
		it++;
		t++;
	}
	return true;
}
/**************************************************************************
 * static functions
 *************************************************************************/
CellTrack::Cell::Cell()
{
	m_fir_node_label = -1;
	m_sec_node_label = -1;
	m_cur_node_label = -1;
	m_mod_node_label = -1;

	m_prev_cell = NULL;
	m_next_cell = NULL;

	m_track = NULL;
	m_tree = NULL;

	m_color = 0;
}

CellTrack::Track* CellTrack::Cell::getTrack() const
{
	if(m_track == NULL)
	{
		cerr<<"Cell::getTrack null track"<<endl;
	}
	return m_track;
}

void CellTrack::Cell::setTrack(CellTrack::Track* track)
{
	//assert(track != NULL);
	m_track = track;
}

ComponentTree* CellTrack::Cell::getTree() const
{
	return m_tree;
}
void CellTrack::Cell::setTree(ComponentTree* tree)
{
	m_tree = tree;
}

int CellTrack::Cell::getOverlap(CellTrack::Cell* cell2)
{
	vector<int>& vertices1 = this->getVertices();
	vector<int>& vertices2 = cell2->getVertices();
	int count = 0;
	if(vertices1.size() > vertices2.size())
	{
		set<int> vertices_set(vertices1.begin(), vertices1.end());
		vector<int>& vertices = vertices2;
		for(int i = 0; i < (int)vertices.size(); i++)
		{
			if(vertices_set.find(vertices[i]) != vertices_set.end())
			{
				count++;
			}
		}
	}
	else
	{
		set<int> vertices_set(vertices2.begin(), vertices2.end());
		vector<int>& vertices = vertices1;
		for(int i = 0; i < vertices.size(); i++)
		{
			if(vertices_set.find(vertices[i]) != vertices_set.end())
			{
				count++;
			}
		}
	}
	return count;
}

/**
 * normally c = 3
 * **/
#ifdef __v3d__
void CellTrack::Cell::draw(unsigned char* image, int sz0, int sz1, int sz2)
{
	unsigned char* imageR = image;
	unsigned char* imageG = image + sz0 * sz1 * sz2;
	unsigned char* imageB = image + 2 * sz0 * sz1 * sz2;

	vector<int> & vertices = getVertices();
	if(vertices.empty()) return;
	vector<int>::iterator it = vertices.begin();
	int color = this->getColor();
	unsigned char r = color % 256;
	unsigned char g = (color / 256)%256;
	unsigned char b = (color / 256 / 256)%256;
	while(it != vertices.end())
	{
		int index = (*it);
		imageR[index] = r;
		imageG[index] = g;
		imageB[index] = b;
		it++;
	}
}

void CellTrack::Cell::drawMarker(unsigned char* image, int sz0, int sz1, int sz2)
{
	unsigned char* imageR = image;
	unsigned char* imageG = image + sz0 * sz1 * sz2;
	unsigned char* imageB = image + 2 * sz0 * sz1 * sz2;

	vector<int> &vertices = getCenterArea();
	assert(!vertices.empty());
	if(vertices.empty()) return;
	vector<int>::iterator it = vertices.begin();
	int color = this->getColor();
	unsigned char r = 255 - color % 256;
	unsigned char g = 255 - (color / 256)%256;
	unsigned char b = 255 - (color / 256 / 256)%256;
	while(it != vertices.end())
	{
		int index = (*it) ;
		imageR[index] = r;
		imageG[index] = g;
		imageB[index] = b;
		it++;
	}
}
#else
void CellTrack::Cell::draw(unsigned char* image)
{
	vector<int> & vertices = getVertices();
	if(vertices.empty()) return;
	vector<int>::iterator it = vertices.begin();
	int color = this->getColor();
	unsigned char r = color % 256;
	unsigned char g = (color / 256)%256;
	unsigned char b = (color / 256 / 256)%256;
	while(it != vertices.end())
	{
		int index = (*it) * 3;
		image[index] = r;
		image[index + 1] = g;
		image[index + 2] = b;
		it++;
	}
}

void CellTrack::Cell::drawMarker(unsigned char* image)
{
	vector<int> &vertices = getCenterArea();
	assert(!vertices.empty());
	if(vertices.empty()) return;
	vector<int>::iterator it = vertices.begin();
	int color = this->getColor();
	unsigned char r = 255 - color % 256;
	unsigned char g = 255 - (color / 256)%256;
	unsigned char b = 255 - (color / 256 / 256)%256;
	while(it != vertices.end())
	{
		int index = (*it) * 3;
		image[index] = r;
		image[index + 1] = g;
		image[index + 2] = b;
		it++;
	}
}
#endif

void CellTrack::Cell::setCenterArea()
{
	m_center_area.clear();
	int width = m_tree->width();
	int height = m_tree->height();
	int depth = m_tree->depth();
	float mean_x, mean_y, mean_z;
	getCenter(mean_x, mean_y, mean_z);
	int cell_width = 0; 
	int cell_height = 0;
	int cell_depth = 0;

	vector<int> vertices = getVertices();
	int min_x = INT_MAX;
	int min_y = INT_MAX;
	int min_z = INT_MAX;
	int max_x = 0;
	int max_y = 0;
	int max_z = 0;

	vector<int>::iterator it = vertices.begin();
	while(it != vertices.end())
	{
		int w = (*it) % width;
		int h = (*it) / width % height;
		int d = (*it) / width / height % depth;
		min_x = w < min_x ? w : min_x;
		min_y = h < min_y ? h : min_y;
		min_z = d < min_z ? d : min_z;
		max_x = w > max_x ? w : max_x;
		max_y = h > max_y ? h : max_y;
		max_z = d > max_z ? d : max_z;
		it++;
	}
	cell_width = max_x - min_x;
	cell_height = max_y - min_y;
	cell_depth = max_z - min_z;

	float aa = (cell_width/2.0)*(cell_width/2.0)/4.0;
	float bb = (cell_height/2.0)*(cell_height/2.0)/4.0;
	float cc = (cell_depth/2.0)*(cell_depth/2.0)/4.0;
	it = vertices.begin();
	while(it != vertices.end())
	{
		int x = (*it) % width;
		int y = (*it) / width % height;
		int z = (*it) / width / height % depth;
		if((x - mean_x)*(x - mean_x)/aa + (y - mean_y)*(y - mean_y)/bb + (z - mean_z)*(z - mean_z)/cc <= 1.0)
		{
			m_center_area.push_back((int)(*it));
		}
		it++;
	}
}

vector<int>& CellTrack::Cell::getCenterArea() 
{
	int width = m_tree->width();
	int height = m_tree->height();
	int depth = m_tree->depth();
	if(!m_center_area.empty()) return m_center_area;
	float mean_x, mean_y, mean_z;
	getCenter(mean_x, mean_y, mean_z);
	int cell_width = 0; 
	int cell_height = 0;
	int cell_depth = 0;

	vector<int> vertices = getVertices();
	int min_x = INT_MAX;
	int min_y = INT_MAX;
	int min_z = INT_MAX;
	int max_x = 0;
	int max_y = 0;
	int max_z = 0;

	vector<int>::iterator it = vertices.begin();
	while(it != vertices.end())
	{
		int w = (*it) % width;
		int h = (*it) / width % height;
		int d = (*it) / width / height % depth;
		min_x = w < min_x ? w : min_x;
		min_y = h < min_y ? h : min_y;
		min_z = d < min_z ? d : min_z;
		max_x = w > max_x ? w : max_x;
		max_y = h > max_y ? h : max_y;
		max_z = d > max_z ? d : max_z;
		it++;
	}
	cell_width = max_x - min_x;
	cell_height = max_y - min_y;
	cell_depth = max_z - min_z;

	float aa = (cell_width/2.0)*(cell_width/2.0)/4.0;
	float bb = (cell_height/2.0)*(cell_height/2.0)/4.0;
	float cc = (cell_depth/2.0)*(cell_depth/2.0)/4.0;
	it = vertices.begin();
	while(it != vertices.end())
	{
		int x = (*it) % width;
		int y = (*it) / width % height;
		int z = (*it) / width / height % depth;
		if((x - mean_x)*(x - mean_x)/aa + (y - mean_y)*(y - mean_y)/bb + (z - mean_z)*(z - mean_z)/cc <= 1.0)
		{
			m_center_area.push_back((int)(*it));
		}
		it++;
	}

	return m_center_area;
}

int CellTrack::Cell::getFirNodeLabel() const
{
	return m_fir_node_label;
}

void CellTrack::Cell::setFirNodeLabel(int label)
{
	m_fir_node_label = label;
}

int CellTrack::Cell::getSecNodeLabel() const
{
	return m_sec_node_label;
}

void CellTrack::Cell::setSecNodeLabel(int label)
{
	m_sec_node_label = label;
}

int CellTrack::Cell::getCurNodeLabel() const
{
	return m_cur_node_label;
}

void CellTrack::Cell::setCurNodeLabel(int label)
{
	m_cur_node_label = label;
}

int CellTrack::Cell::getModNodeLabel() const
{
	return m_mod_node_label;
}

void CellTrack::Cell::setModNodeLabel(int label)
{
	m_mod_node_label = label;
}

int CellTrack::Cell::getNodeLabel() const
{
	if(m_mod_node_label != -1) return m_mod_node_label;
	else if(m_cur_node_label != -1) return m_cur_node_label;
	else if(m_sec_node_label != -1 || m_fir_node_label != -1)
	{
		if(m_fir_node_label != -1) return m_fir_node_label;
		if(m_sec_node_label != -1) return m_sec_node_label;
	}
	return -1;
}

TNode* CellTrack::Cell::getNode() const
{
	int label = getNodeLabel();
	if(label == -1)
	{
		cerr<<"Cell::getNode error, no effective node label"<<endl;
		return NULL;
	}
	else
	{
		return m_tree->getNode(label);
	}
}

CellTrack::Cell* CellTrack::Cell::getPrevCell() const
{
	return m_prev_cell;
}

void CellTrack::Cell::setPrevCell(CellTrack::Cell* prev_cell)
{
	m_prev_cell = prev_cell;
}

CellTrack::Cell* CellTrack::Cell::getNextCell() const
{
	return m_next_cell;
}

void CellTrack::Cell::setNextCell(CellTrack::Cell* next_cell)
{
	m_next_cell = next_cell;
}

vector<int>& CellTrack::Cell::getVertices()
{
	if(!m_vertices.empty()) return m_vertices;
	else if(m_tree != NULL)
	{
		m_vertices = this->getNode()->getBetaPoints();
		return m_vertices;
	}
	else
	{
		//cerr<<"Cell::getVertices: Unable to get vertices"<<endl;
		return m_vertices;
	}
}

void CellTrack::Cell::setVertices()
{
	if(m_tree == NULL) return;
	else 
	{
		m_vertices.clear();
		m_vertices = getNode()->getBetaPoints();
	}
}

int CellTrack::Cell::getSize()
{
	if(m_vertices.empty())
	{
		//	return -1;
		return getVertices().size();
	}
	return m_vertices.size();
}
/**************************************************************
 * Volume will be calculated in a kind of formula
 * ************************************************************/
int CellTrack::Cell::getVolume()
{
	return getSize();
}

void CellTrack::Cell::getCenter(float & mean_w, float & mean_h, float &mean_d)
{
	int w = m_tree->width();
	int h = m_tree->height();
	int d = m_tree->depth();
	vector<int>& vertices = getVertices();
	vector<int>::iterator it = vertices.begin();
	mean_w = 0.0;
	mean_h = 0.0;
	mean_d = 0.0;
	while(it != vertices.end())
	{
		mean_w += (*it) % w;
		mean_h += (*it/w) % h;
		mean_d += (*it/w/h) % d;
		it++;
	}
	mean_w = mean_w / vertices.size();
	mean_h = mean_h / vertices.size();
	mean_d = mean_d / vertices.size();
}
// assert the color is same to track color
unsigned int CellTrack::Cell::getColor() const
{
	if(m_color == 0)
		return getTrack()->getColor();
	else
		return m_color;
}

void CellTrack::Cell::setColor(unsigned int color) 
{
	m_color = color;
}
/***************************************************************************
 * CellTrack::Frame
 * *************************************************************************/
CellTrack::Frame::Frame()
{
	m_tree = NULL;
	m_tree_file = "";
	m_width = -1;
	m_height = -1;
	m_depth = -1;
}

void CellTrack::Frame::exportImage(char* img_file/*, Palette& palette*/)
{
	int width = this->width();
	int height = this->height();
	int depth = this->depth();
	cout<<"output image "<<img_file<<" with ";
	cout<<this->cellNum()<<" cells"<<endl;
	//this->setVertices();
	//srand(time(NULL));
	int size = width * height * depth;
	unsigned char* img = new unsigned char[size*3];
	for(int i = 0; i < 3*size; i++) img[i] = 0;
	vector<Cell*>::iterator it = m_cells.begin();
	//ComponentTree* tree = this->getTree();
	while(it != m_cells.end())
	{
		//(*it)->setTree(tree); // not needed maybe
		vector<int>& vertices = (*it)->getVertices();
		if(vertices.empty()) 
		{
			it++;
			continue;
		}
		//assert(!vertices.empty());
		//int color_id = (*it)->getTrack()->getColorId();
		//unsigned char r = palette(color_id).r; //rand() % 256;
		//unsigned char g = palette(color_id).g; //rand() % 256;
		//unsigned char b = palette(color_id).b; //rand() % 256;
		unsigned int color = (*it)->getTrack()->getColor();
		unsigned char r = color % 256;
		unsigned char g = (color/256)%256;
		unsigned char b = (color/256/256)%256;
		vector<int>::iterator itr = vertices.begin();
		while(itr != vertices.end())
		{
			img[(*itr)*3] = r;
			img[(*itr)*3 + 1] = g;
			img[(*itr)*3 + 2] = b;
			itr++;
		}
		it++;
	}
	writetiff(img_file, img, 3, width, height, depth);
	delete img;
}

void CellTrack::Frame::addCell(CellTrack::Cell* cell)
{
	m_cells.push_back(cell);
}

map<unsigned int, CellTrack::Cell*> CellTrack::Frame::createFromImage(char* img_file, map<unsigned int, CellTrack::Cell*> &prev_map_cell)
{
	int width = 0;
	int height = 0;
	int depth = 0;
	int channels = 0;
	unsigned char* img = readtiff(img_file, &width, &height, &depth, &channels);
	assert(width >= 1);
	assert(height >= 1);
	assert(depth >= 1);
	assert(channels == 3);
	m_width = width;
	m_height = height;
	m_depth = depth;
	int img_size = width * height * depth;
	
	map<unsigned int, Cell*> map_cell;
	for(int i = 0; i < img_size; i++)
	{
		unsigned int color = img[3*i] + img[3*i + 1] * 256 + img[3*i + 2] * 256 * 256 ;
		if(color == 0) continue;
		Cell* cell = NULL;
		if(map_cell.find(color) != map_cell.end())
		{
			cell = map_cell[color];
			cell->m_vertices.push_back(i);
		}
		else
		{
			cell = new Cell;
			cell->m_vertices.push_back(i);
			cell->m_color = color;

			map_cell[color] = cell;
			this->m_cells.push_back(cell);

			if(prev_map_cell.find(color) != prev_map_cell.end())
			{
				Cell* prev_cell = prev_map_cell[color];
				cell->m_prev_cell = prev_cell;
				prev_cell->m_next_cell = cell;
			}
		}
	}
	delete img;
	return map_cell;
}

void CellTrack::Frame::bipartiteMatching(CellTrack::Frame* frame2, vector<int>& ids1, vector<int>& ids2)
{
	if(frame2 == NULL) return;
	Frame* frame1 = this;
	int size = this->width() * this->height() * this->depth();
	vector<int> matrix1(size, -1);
	vector<int> matrix2(size, -1);
	vector<Cell*>& cells1 = frame1->m_cells;
;
	vector<Cell*>& cells2 = frame2->m_cells;
	vector<Cell*>::iterator it = cells1.begin();
	int id = 0;
	while(it != cells1.end())
	{
		vector<int>& vertices = (*it)->getVertices();
        vector<int>::iterator itr = vertices.begin();
        while(itr != vertices.end())
        {
            matrix1[*itr] = id;
            itr++;
        }
		id++;
		it++;
	}
	it = cells2.begin();
	id = 0;
	while(it != cells2.end())
	{
		vector<int> vertices = (*it)->getVertices();
        vector<int>::iterator itr = vertices.begin();
        while(itr != vertices.end())
        {
            matrix2[*itr] = id;
            itr++;
        }
		id++;
		it++;
	}
	int cells1Num = cells1.size();
	int cells2Num = cells2.size();
	vector<float> weights(cells1Num*cells2Num, 0.0);
	for(int i = 0; i < size; i++)
	{
		int id1 = matrix1[i];
		int id2 = matrix2[i];
		// if overlab then set true
		if(id1 != -1 && id2 != -1) weights[id1*cells2Num + id2] += 1.0;
	}

	bipartite_matching(weights, cells1Num, cells2Num, ids1, ids2);
}

void CellTrack::Frame::mergePrevFrame(CellTrack::Frame* prev_frame)
{
	if(prev_frame == NULL) return;   // very important
	vector<int> ids1;
	vector<int> ids2;
	//prev_frame->bipartiteMatching(this, ids1, ids2);
	this->bipartiteMatching(prev_frame, ids2, ids1);
	map<int,int> rev_ids1;//
	for(int i = 0; i < ids1.size(); i++)
	{
		int id = ids1[i];
		rev_ids1[id] = i;
	}
	vector<Cell*>& cells1 = prev_frame->m_cells;
	vector<Cell*>& cells2 = this->m_cells;
	int cells1Num = prev_frame->cellNum();
	for(int id1 = 0; id1 < cells1Num; id1++)
	{
		if(rev_ids1.find(id1) != rev_ids1.end())
		{
			int id2 = ids2[rev_ids1[id1]];
			cells2[id2]->setFirNodeLabel(cells1[id1]->getCurNodeLabel());
			cells2[id2]->setSecNodeLabel(cells1[id1]->getCurNodeLabel());
				int cur_label = cells1[id1]->m_cur_node_label > cells2[id2]->m_cur_node_label ? cells1[id1]->m_cur_node_label:  cells2[id2]->m_cur_node_label;
				Cell* prev_cell = cells1[id1]->getPrevCell();
				cells2[id2]->setPrevCell(prev_cell);
				prev_cell->setNextCell(cells2[id2]);
				delete cells1[id1];
		}
		else
		{
			m_cells.push_back(cells1[id1]);
		}
	}
	delete prev_frame;
}

void CellTrack::Frame::mergePrevFrame_OLD(CellTrack::Frame* prev_frame)
{
	if(prev_frame == NULL) return;
	assert(prev_frame->getTree() == this->getTree());
	ComponentTree* tree = getTree();
	int size = this->width() * this->height() * this->depth();
	vector<int> matrix1(size, -1);
	vector<int> matrix2(size, -1);
	vector<Cell*> cells1 = prev_frame->m_cells;
;
	vector<Cell*> cells2 = this->m_cells;
	vector<Cell*>::iterator it = cells1.begin();
	int id = 0;
	while(it != cells1.end())
	{
		//(*it)->setTree(tree); // not needed maybe
		vector<int>& vertices = (*it)->getVertices();
        vector<int>::iterator itr = vertices.begin();
        while(itr != vertices.end())
        {
            matrix1[*itr] = id;
            itr++;
        }
		id++;
		it++;
	}
	it = cells2.begin();
	id = 0;
	while(it != cells2.end())
	{
		//(*it)->setTree(tree); // not needed maybe
		vector<int> vertices = (*it)->getVertices();
        vector<int>::iterator itr = vertices.begin();
        while(itr != vertices.end())
        {
            matrix2[*itr] = id;
            itr++;
        }
		id++;
		it++;
	}
	int cells1Num = cells1.size();
	int cells2Num = cells2.size();
	vector<bool> weight(cells1Num*cells2Num, 0);
	for(int i = 0; i < size; i++)
	{
		int id1 = matrix1[i];
		int id2 = matrix2[i];
		// if overlab then set true
		if(id1 != -1 && id2 != -1) weight[id1*cells2Num + id2] = true;
	}
	for(int i = 0; i < cells1Num; i++)
	{
		// find which j is correspond to i
		int sum = 0;
		int the_j = -1;
		for(int j = 0; j < cells2Num; j++) 
		{
			sum += weight[i*cells2Num + j];
			//if(weight[i*cells2Num + j] && the_j == -1) the_j = j;
			if(weight[i*cells2Num + j]) 
			{
				if(j > the_j) the_j = j; // get the maximum the_j
			}
		}
		//assert(sum <= 1);
		if(sum > 1) cout<<"sum : "<< sum<<endl;
		if(sum)
		{
			assert(tree != NULL);
			int fir_node = cells2[the_j]->getFirNodeLabel();
			if(true)//cells1[i]->getFirNodeLabel() > fir_node) // consider multiple i to j
			{
				if(fir_node > -1)
				{
					//Cell* prev_cell = cells2[the_j]->getPrevCell();
					//prev_cell->setNextCell(NULL);
					//cells2[the_j]->setPrevCell(NULL);
				}
				cells2[the_j]->setFirNodeLabel(cells1[i]->getCurNodeLabel());
				cells2[the_j]->setSecNodeLabel(cells2[the_j]->getCurNodeLabel());	
				int cur_label = cells1[i]->m_cur_node_label > cells2[the_j]->m_cur_node_label ? cells1[i]->m_cur_node_label:  cells2[the_j]->m_cur_node_label;
				cells2[the_j]->setCurNodeLabel(cur_label);
				Cell* prev_cell = cells1[i]->getPrevCell();
				cells2[the_j]->setPrevCell(prev_cell);
				prev_cell->setNextCell(cells2[the_j]);
				delete cells1[i];
			}
		}
		else
		{
			m_cells.push_back(cells1[i]);
		}
	}
	delete prev_frame;
}

/***********************************************************************
 * CellTrack::Frame::getReverseAlphaMappint(ComponentTree* tree)
 * us m_cur_node_label to fill matrix
 ***********************************************************************/
/*vector<int> CellTrack::Frame::getReverseAlphaMapping()
{	
	ComponentTree* tree = this->getTree();
	int size = tree->pixelNum();
	vector<int> matrix(size, -1);
	vector<Cell*>::iterator it = m_cells.begin();
	while(it != m_cells.end())
	{
		//int label = (*it)->getCurNode(tree)->getLabel();
		int label = (*it)->m_cur_node_label;
		vector<int> vertices;
		if((*it)->m_vertices.empty()) vertices =  (*it)->getNode(tree)->getBetaPoints();
		else vertices = (*it)->getVertices();
		vector<int>::iterator itr = vertices.begin();
		while(itr != vertices.end())
		{
			matrix[*itr] = label;
			itr++;
		}
		it++;
	}
	return matrix;
}
*/
ComponentTree* CellTrack::Frame::getTree()
{
	if(m_tree != NULL) return m_tree;
	else if(!m_tree_file.empty())
	{
		m_tree = new ComponentTree((char*) m_tree_file.c_str());
		vector<Cell*> cells = this->getCells();
		vector<Cell*>::iterator it = cells.begin();
		while(it != cells.end())
		{
			(*it)->setTree(m_tree);
			it++;
		}
		return m_tree;
	}
	else 
	{
		//cerr<<"Frame::getTree() unable to get component tree "<<m_tree_file.c_str()<<endl;
		return NULL;
	}
}

void CellTrack::Frame::setTree(ComponentTree* tree)
{
	assert(tree != NULL);
	m_tree = tree;
	m_width = tree->width();
	m_height = tree->height();
	m_depth = tree->depth();
}

bool CellTrack::Frame::correspondToTree(ComponentTree* tree)
{
	if(tree->width() != m_width || 
			tree->height() != m_height ||
			tree->depth() != m_depth)
	{
		return false;
	}
	assert(!m_cells.empty());
	vector<Cell*>::iterator it = m_cells.begin();
	while(it != m_cells.end())
	{
		Cell* cell = *it;
		TNode* node = tree->getNode(cell->getVertices());
		if(node != NULL) cell->setCurNodeLabel(node->getLabel());
		else return false;
		it++;
	}
	assert(this->m_tree != NULL);
	this->m_tree = tree;
	return true;
}

void CellTrack::Frame::releaseTree(char* tree_file)
{
	m_tree_file = string(tree_file);
	cout<<"release tree "<< tree_file<<endl;
	if(m_tree != NULL) 
	{
		m_tree->clear();
		m_tree = NULL;
	}
	vector<Cell*> cells = this->getCells();
	vector<Cell*>::iterator it = cells.begin();
	while(it != cells.end())
	{
		(*it)->setTree(NULL);
		it++;
	}
}

void CellTrack::Frame::releaseVertices()
{
	// if load from tree files
	if(m_tree != NULL || !m_tree_file.empty())
	{
		vector<Cell*>::iterator it = m_cells.begin();
		while(it != m_cells.end())
		{
			(*it)->m_vertices.clear();
			it++;
		}
	}
}
/*
void CellTrack::Frame::setVertices()
{

	ComponentTree* tree = this->getTree();
	if(tree == NULL)
	{
		//cerr<<"Unable to setVertices, make sure all cells' vertices are alread set"<<endl;
		return;
	}
	vector<Cell*>::iterator it = m_cells.begin();
	while(it != m_cells.end())
	{
		if((*it)->m_vertices.empty())
		{
			(*it)->m_vertices = (*it)->getNode(tree)->getBetaPoints();
		}
		it++;
	}
}
*/
int CellTrack::Frame::cellNum() const
{
	return m_cells.size();
}

vector<CellTrack::Cell*> CellTrack::Frame::getCells() const
{
	return m_cells;
}

int CellTrack::Frame::width() const
{
	assert(m_width > 0);
	return m_width;
}

int CellTrack::Frame::height() const
{
	assert(m_height > 0);
	return m_height;
}

int CellTrack::Frame::depth() const
{
	assert(m_height > 0);
	return m_depth;
}

CellTrack::Track::Track()
{
	m_start_time = -1;
	m_entry_cell = NULL;
	//m_color_id = -1;
	m_color = 0;
	m_track_id = -1;
}

CellTrack::Cell* CellTrack::Track::getStartCell() const
{
	return m_entry_cell;
}

void CellTrack::Track::setStartCell(CellTrack::Cell* cell)
{
	m_entry_cell = cell;
}

//int CellTrack::Track::getColorId() const
//{
//	assert(m_color_id != -1);
//	return m_color_id;
//}

unsigned int CellTrack::Track::getColor() const
{
	return m_color;
}

void CellTrack::Track::setColor(unsigned int color)
{
	m_color = color;
}

void CellTrack::Track::addNext(Cell* cell)
{
	//m_cells.push_back(cell);
}

vector<CellTrack::Cell*> CellTrack::Track::getCells() const
{
	vector<Cell*> cells;
	Cell* p = m_entry_cell;
	while(p != NULL)
	{
		cells.push_back(p);
		p = p->getNextCell();
	}
	return cells;
}

int CellTrack::Track::cellNum() const
{
	int cell_num = 1;

	Cell* p = m_entry_cell;
	while(p->getNextCell() != NULL) 
	{
		cell_num++;
		p = p->getNextCell();
	}
	return cell_num;
}
int CellTrack::Track::trackId() const
{
	return m_track_id;
}
int CellTrack::Track::startTime() const
{
	return m_start_time;
}
