#include <QObject>
#include <cassert>
#include "cell_track_ex.h"

CellTrackEX::CellTrackEX() : QObject(), CellTrack()
{
}

CellTrackEX::~CellTrackEX()
{
}

bool  CellTrackEX::createFramesFromTrees(ComponentTree* tree1, ComponentTree* tree2,vector<CellTrack::Frame*> &frames )
{
	return CellTrack::createFramesFromTrees(tree1, tree2, frames);
}
bool CellTrackEX::createFramesFromTrees(vector<char*> tree_files,vector<CellTrack::Frame*> &frames)
{
	assert(frames.empty());
	assert(tree_files.size() >= 2);
	if(tree_files.size() == 2)
	{
		ComponentTree* tree1 = new ComponentTree(tree_files[0]);
		ComponentTree* tree2 = new ComponentTree(tree_files[1]);
		bool rt = createFramesFromTrees(tree1, tree2, frames);
		emit setProgressValue(2);
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
		emit setProgressValue(numFrames + i);
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
bool CellTrackEX::createFramesFromImages(vector<char*> img_files,vector<CellTrack::Frame*> &frames )
{
	return CellTrack::createFramesFromImages(img_files, frames);
}

