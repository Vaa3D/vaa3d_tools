#include "TimeSeries.h"

using namespace std;
using namespace iim;

TimeSeries::TimeSeries(const char *rootDir) throw (IOException) : VirtualVolume(rootDir)
{
}

TimeSeries::~TimeSeries(void) throw (IOException)
{

}

// pure virtual method inherithed from abstract class <VirtualVolume>
void TimeSeries::initChannels ( ) throw (IOException)
{
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
    throw IOException("Not yet implemented");
}

uint8 * TimeSeries::loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels /*=0*/, int ret_type /*=iim::DEF_IMG_DEPTH*/) throw (IOException)
{
    throw IOException("Not yet implemented");
}
