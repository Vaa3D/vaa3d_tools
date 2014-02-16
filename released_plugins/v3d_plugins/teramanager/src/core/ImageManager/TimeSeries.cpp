#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif
#include <stdio.h>
#include <list>
#include <typeinfo>
#include "TimeSeries.h"

using namespace std;
using namespace iim;

TimeSeries::TimeSeries(const char *rootDir) throw (IOException) : VirtualVolume(rootDir)
{
    /**/iim::debug(iim::LEV2, strprintf("rootDir = %s", root_dir).c_str(), __iim__current__function__);

    // check precondition #1: valid folder
    if(!isDirectory(root_dir))
        throw IOException(strprintf("in TimeSeries::TimeSeries(): \"%s\" not a valid folder", rootDir).c_str());

    // search for valid folders that match the format [CHANNEL_PREFIX][i]
    DIR *cur_dir=0;
    dirent *entry=0;
    list<string> entries;
    if (!(cur_dir=opendir(root_dir)))
        throw IOException(strprintf("in TimeSeries::TimeSeries(): cannot open folder \"%s\"", root_dir).c_str());
    string pattern = TIME_FRAME_PREFIX + "\%d";
    while ((entry=readdir(cur_dir)))
    {
        int number = 0;
        if(sscanf(entry->d_name, pattern.c_str(), &number) == 1)
            entries.push_front(entry->d_name);
    }
    closedir(cur_dir);
    entries.sort();

    // import each folder as a separate VirtualVolume
    // WARNING: all metadata files (if needed by that format) are assumed to be present. Otherwise, that format will be skipped.
    for(list<string>::iterator it = entries.begin(); it != entries.end(); it++)
    {
        string path = root_dir;
        path += "/";
        path += *it;
        VirtualVolume* volume = VirtualVolume::instance(path.c_str());
        if(!volume)
            throw IOException(strprintf("in TimeSeries::TimeSeries(): cannot import frame \"%s\" in folder \"%s\": invalid or unsupported format", it->c_str(), root_dir).c_str());
        frames.push_back(volume);
    }

    // check condition #2: all frames must have the same attributes
    for(int k=0; k<frames.size()-1; k++)
    {
        if( strcmp(typeid(*frames[k]).name() , typeid(*frames[k+1]).name()) != 0)
            throw IOException(strprintf("in TimeSeries::TimeSeries(): frames have different formats in folder \"%s\"", root_dir).c_str());

        if( frames[k]->getMVoxels() != frames[k+1]->getMVoxels())
            throw IOException(strprintf("in TimeSeries::TimeSeries(): frames have different sizes in folder \"%s\"", root_dir).c_str());

        if( frames[k]->getCHANS() != frames[k+1]->getCHANS())
            throw IOException(strprintf("in TimeSeries::TimeSeries(): frames have different channels in folder \"%s\"", root_dir).c_str());

        if( frames[k]->getBYTESxCHAN() != frames[k+1]->getBYTESxCHAN())
            throw IOException(strprintf("in TimeSeries::TimeSeries(): frames have different bytes per channel in folder \"%s\"", root_dir).c_str());
    }

    t0 = 0;
    t1 = frames.size()-1;
}

TimeSeries::~TimeSeries(void) throw (IOException)
{
    /**/iim::debug(iim::LEV2, 0, __iim__current__function__);
}

// pure virtual method inherithed from abstract class <VirtualVolume>
void TimeSeries::initChannels ( ) throw (IOException)
{
    /**/iim::debug(iim::LEV2, 0, __iim__current__function__);

    // check precondition #1: valid time series
    if(frames.empty())
        throw IOException("in TimeSeries::initChannels(): empty time series");

    // check precondition #2: all frames have the same number of channels
    for(int k=0; k<frames.size()-1; k++)
        if(frames[k]->getCHANS() != frames[k+1]->getCHANS())
            throw IOException("in TimeSeries::initChannels(): frames have different number of channels");

    // check precondition #3: all frames have the same bytes per channel
    for(int k=0; k<frames.size()-1; k++)
        if(frames[k]->getBYTESxCHAN() != frames[k+1]->getBYTESxCHAN())
            throw IOException("in TimeSeries::initChannels(): frames have different bytes per channel");

    CHANS = frames[0]->getCHANS();
    BYTESxCHAN = static_cast<int>(frames[0]->getBYTESxCHAN());
}

// returns only the active frames, i.e. those in the range [t0, t1]
vector<VirtualVolume*> TimeSeries::getActiveFrames() throw (IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

    // check precondition #1: valid time series
    if(frames.empty())
        throw IOException("in TimeSeries::getActiveFrames(): empty time series");

    vector<VirtualVolume*> activeFrames;
    for(int k=0; k<frames.size(); k++)
        if(k >= t0 && k <= t1)
            activeFrames.push_back(frames[k]);
    return activeFrames;
}

VirtualVolume* TimeSeries::getFrameAt(int t) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("t = %d", t).c_str(), __iim__current__function__);

    // check precondition #1: valid time series
    if(frames.empty())
        throw IOException("in TimeSeries::getFrameAt(): empty time series");

    // check precondition #2: valid frame selected
    if(t < 0 || t >= frames.size())
        throw IOException("in TimeSeries::getFrameAt(): invalid frame selected");

    return frames[t];
}

// pure virtual methods inherithed from abstract class <VirtualVolume>
real32 * TimeSeries::loadSubvolume_to_real32(int V0,int V1, int H0, int H1, int D0, int D1)  throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d", V0, V1, H0, H1, D0, D1).c_str(), __iim__current__function__);


    throw IOException("Not yet implemented");
}

uint8 * TimeSeries::loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels /*=0*/, int ret_type /*=iim::DEF_IMG_DEPTH*/) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d, *channels=%d, ret_type=%d", V0, V1, H0, H1, D0, D1, *channels, ret_type).c_str(), __iim__current__function__);

    throw IOException("Not yet implemented");
}
