#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif
#include <stdio.h>
#include <list>
#include <typeinfo>
#include "TimeSeries.h"
#include "ProgressBar.h"

TimeSeries::TimeSeries(const char *rootDir, std::string frames_format /* = "" */) throw (iim::IOException) : iim::VirtualVolume(rootDir)
{
    /**/iim::debug(iim::LEV2, iim::strprintf("rootDir = %s, frames_format = \"%s\"", root_dir, frames_format.c_str()).c_str(), __iim__current__function__);

    // check condition #1: valid folder
    if(!iim::isDirectory(root_dir))
        throw iim::IOException(iim::strprintf("in TimeSeries::TimeSeries(): \"%s\" not a valid folder", rootDir).c_str());

    // assume hierarchical format for the individual frames if 'format' is not provided
    if(frames_format.empty() || VirtualVolume::isHierarchical(frames_format))
    {
        // search for valid folders that match the format [CHANNEL_PREFIX][i]
        DIR *cur_dir=0;
        dirent *entry=0;
        std::list<std::string> entries;
        if (!(cur_dir=opendir(root_dir)))
            throw iim::IOException(iim::strprintf("in TimeSeries::TimeSeries(): cannot open folder \"%s\"", root_dir).c_str());
        std::string pattern = iim::TIME_FRAME_PREFIX + "%d";
        while ((entry=readdir(cur_dir)))
        {
            int number = 0;
            if(sscanf(entry->d_name, pattern.c_str(), &number) == 1)
                entries.push_front(entry->d_name);
        }
        closedir(cur_dir);
        entries.sort();

        // check condition #2: at least one time frame has been found
        if(entries.empty())
            throw iim::IOException(iim::strprintf("in TimeSeries::TimeSeries(): no hierarchical time frames found within \"%s\"", rootDir).c_str());

        // import each folder as a separate VirtualVolume
        // WARNING: all metadata files (if needed by that format) are assumed to be present. Otherwise, that format will be skipped.
        for(std::list<std::string>::iterator it = entries.begin(); it != entries.end(); it++)
        {
            std::string path = root_dir;
            path += "/";
            path += *it;
            VirtualVolume* volume = 0;
            try{volume = VirtualVolume::instance(path.c_str());}
            catch(iim::IOException &ex){iim::warning(iim::strprintf("Cannot import tiled time frame at \"%s\": %s", path.c_str(), ex.what()).c_str(),__iim__current__function__);}
            catch(...){iim::warning(iim::strprintf("Cannot import tiled time frame at \"%s\"", path.c_str()).c_str(),__iim__current__function__);}
            if(!volume)
                throw iim::IOException(iim::strprintf("in TimeSeries::TimeSeries(): cannot import frame \"%s\" in folder \"%s\": invalid or unsupported format", it->c_str(), root_dir).c_str());
            frames.push_back(volume);
        }
    }
    else
    {
        // search for valid file formats
        DIR *cur_dir=0;
        dirent *entry=0;
        std::list<std::string> entries;
        if (!(cur_dir=opendir(root_dir)))
            throw iim::IOException(iim::strprintf("in TimeSeries::TimeSeries(): cannot open folder \"%s\"", root_dir).c_str());
        while ((entry=readdir(cur_dir)))
        {
            std::string tmp = entry->d_name;
            if(tmp.compare(".") != 0 && tmp.compare("..") != 0 && tmp.find(".") != std::string::npos)
                entries.push_front(entry->d_name);
        }
        closedir(cur_dir);
        entries.sort();

        // try to import each file separately
        for(std::list<std::string>::iterator it = entries.begin(); it != entries.end(); it++)
        {
            std::string path = root_dir;
            path += "/";
            path += *it;
            VirtualVolume* volume = 0;
            try{volume = VirtualVolume::instance(path.c_str(), frames_format);}
            catch(iim::IOException &ex){iim::warning(iim::strprintf("Cannot import filed time frame at \"%s\": %s", path.c_str(), ex.what()).c_str(),__iim__current__function__);}
            catch(...){iim::warning(iim::strprintf("Cannot import filed time frame at \"%s\"", path.c_str()).c_str(),__iim__current__function__);}
            if(volume)
                frames.push_back(volume);
        }

        // check condition #2: at least one time frame has been found
        if(frames.empty())
            throw iim::IOException(iim::strprintf("in TimeSeries::TimeSeries(): no filed time frames found within \"%s\"", rootDir).c_str());
    }

    // check condition #2: all frames must have the same attributes
    for(int k=0; k<frames.size()-1; k++)
    {
        if( strcmp(typeid(*frames[k]).name() , typeid(*frames[k+1]).name()) != 0)
            throw iim::IOException(iim::strprintf("in TimeSeries::TimeSeries(): frames have different formats in folder \"%s\"", root_dir).c_str());

        if( frames[k]->getDIM_H() != frames[k+1]->getDIM_H())
            throw iim::IOException(iim::strprintf("in TimeSeries::TimeSeries(): frames have different X size in folder \"%s\"", root_dir).c_str());

        if( frames[k]->getDIM_V() != frames[k+1]->getDIM_V())
            throw iim::IOException(iim::strprintf("in TimeSeries::TimeSeries(): frames have different Y size in folder \"%s\"", root_dir).c_str());

        if( frames[k]->getDIM_D() != frames[k+1]->getDIM_D())
            throw iim::IOException(iim::strprintf("in TimeSeries::TimeSeries(): frames have different Z size in folder \"%s\"", root_dir).c_str());

        if( frames[k]->getDIM_C() != frames[k+1]->getDIM_C())
            throw iim::IOException(iim::strprintf("in TimeSeries::TimeSeries(): frames have different channels in folder \"%s\"", root_dir).c_str());

        if( frames[k]->getBYTESxCHAN() != frames[k+1]->getBYTESxCHAN())
            throw iim::IOException(iim::strprintf("in TimeSeries::TimeSeries(): frames have different bytes per channel in folder \"%s\"", root_dir).c_str());
    }

    // assigning to the attributes of time series the attributes of the first frame
    VXL_V = frames[0]->getVXL_V();
    VXL_H = frames[0]->getVXL_H();
    VXL_D = frames[0]->getVXL_D();
    ORG_V = frames[0]->getORG_V();
    ORG_H = frames[0]->getORG_H();
    ORG_D = frames[0]->getORG_D();
    DIM_V = frames[0]->getDIM_V();
    DIM_H = frames[0]->getDIM_H();
    DIM_D = frames[0]->getDIM_D();

    initChannels();

    t0 = 0;
    t1 = (int)(frames.size()-1);
    DIM_T = (int)(frames.size());
}

TimeSeries::~TimeSeries(void) throw (iim::IOException)
{
    /**/iim::debug(iim::LEV2, 0, __iim__current__function__);
}

// pure virtual method inherithed from abstract class <VirtualVolume>
void TimeSeries::initChannels ( ) throw (iim::IOException)
{
    /**/iim::debug(iim::LEV2, 0, __iim__current__function__);

    // check precondition #1: valid time series
    if(frames.empty())
        throw iim::IOException("in TimeSeries::initChannels(): empty time series");

    // call initChannels for each frame
    for(int k=0; k<frames.size()-1; k++)
        frames[k]->initChannels();

    // check precondition #2: all frames have the same number of channels
    for(int k=0; k<frames.size()-1; k++)
        if(frames[k]->getDIM_C() != frames[k+1]->getDIM_C())
            throw iim::IOException("in TimeSeries::initChannels(): frames have different number of channels");

    // check precondition #3: all frames have the same bytes per channel
    for(int k=0; k<frames.size()-1; k++)
        if(frames[k]->getBYTESxCHAN() != frames[k+1]->getBYTESxCHAN())
            throw iim::IOException("in TimeSeries::initChannels(): frames have different bytes per channel");

    DIM_C = frames[0]->getDIM_C();
    BYTESxCHAN = static_cast<int>(frames[0]->getBYTESxCHAN());
}

// set active channels (@OVERRIDES VirtualVolume.h by Alessandro on 2014-02-23)
void TimeSeries::setActiveChannels ( iim::uint32 *_active, int _n_active )
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

    // check precondition #1: valid time series
    if(frames.empty())
        throw iim::IOException("in TimeSeries::setActiveChannels(): empty time series");

    for(int k=0; k<frames.size(); k++)
        frames[k]->setActiveChannels(_active, _n_active);
}

// returns only the active frames, i.e. those in the range [t0, t1]
std::vector<iim::VirtualVolume*> TimeSeries::getActiveFrames() throw (iim::IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

    // check precondition #1: valid time series
    if(frames.empty())
        throw iim::IOException("in TimeSeries::getActiveFrames(): empty time series");

    std::vector<VirtualVolume*> activeFrames;
    for(int k=0; k<frames.size(); k++)
        if(k >= t0 && k <= t1)
            activeFrames.push_back(frames[k]);
    return activeFrames;
}

iim::VirtualVolume* TimeSeries::getFrameAt(int t) throw (iim::IOException)
{
    /**/iim::debug(iim::LEV3, iim::strprintf("t = %d", t).c_str(), __iim__current__function__);

    // check precondition #1: valid time series
    if(frames.empty())
        throw iim::IOException("in TimeSeries::getFrameAt(): empty time series");

    // check precondition #2: valid frame selected
    if(t < 0 || t >= frames.size())
        throw iim::IOException("in TimeSeries::getFrameAt(): invalid frame selected");

    return frames[t];
}

// pure virtual methods inherithed from abstract class <VirtualVolume>
iim::real32 * TimeSeries::loadSubvolume_to_real32(int V0,int V1, int H0, int H1, int D0, int D1)  throw (iim::IOException)
{
    /**/iim::debug(iim::LEV3, iim::strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d", V0, V1, H0, H1, D0, D1).c_str(), __iim__current__function__);


    throw iim::IOException("Not yet implemented");
}

iim::uint8 * TimeSeries::loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels /*=0*/, int ret_type /*=iim::DEF_IMG_DEPTH*/) throw (iim::IOException)
{
    /**/iim::debug(iim::LEV3, iim::strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d, *channels=%d, ret_type=%d, t0 = %d, t1 = %d", V0, V1, H0, H1, D0, D1, channels ? *channels : -1, ret_type, t0, t1).c_str(), __iim__current__function__);

    // initializations
    V0 = V0 < 0 ? 0 : V0;
    H0 = H0 < 0 ? 0 : H0;
    D0 = D0 < 0 ? 0 : D0;
    V1 = (V1 < 0 || V1 > (int)DIM_V) ? DIM_V : V1; // iannello MODIFIED
    H1 = (H1 < 0 || H1 > (int)DIM_H) ? DIM_H : H1; // iannello MODIFIED
    D1 = (D1 < 0 || D1 > (int)DIM_D) ? DIM_D : D1; // iannello MODIFIED

    // check for valid 3D selection
    if(V1-V0 <=0 || H1-H0 <= 0 || D1-D0 <= 0)
        throw iim::IOException("in TimeSeries::loadSubvolume_to_UINT8: invalid subvolume intervals");

    // check for valid time frames selection
    if(frames.empty())
        throw iim::IOException("in TimeSeries::loadSubvolume_to_UINT8(): empty time series");
    if(t0 < 0 || t0 >= frames.size())
        throw iim::IOException("in TimeSeries::loadSubvolume_to_UINT8(): invalid time frames selection");
    if(t1 < 0 || t1 >= frames.size())
        throw iim::IOException("in TimeSeries::loadSubvolume_to_UINT8(): invalid time frames selection");
    if(t1 - t0 < 0)
        throw iim::IOException(iim::strprintf("in TimeSeries::loadSubvolume_to_UINT8(): invalid time frames selection: [%d, %d]", t0, t1).c_str());

    // check for valid channel selection
    for(int k=0; k<frames.size()-1; k++)
    {
        if(frames[k]->getNACtiveChannels() != frames[k+1]->getNACtiveChannels())
            throw iim::IOException("in TimeSeries::loadSubvolume_to_UINT8(): not all frames have the same active channels");
        iim::uint32* channels_k = frames[k]->getActiveChannels();
        iim::uint32* channels_k1 = frames[k+1]->getActiveChannels();
        if(!channels_k || !channels_k1)
            throw iim::IOException("in TimeSeries::loadSubvolume_to_UINT8(): frames have invalid channel selections");
        for(int c=0; c<frames[k]->getNACtiveChannels(); c++)
            if(channels_k[c] != channels_k1[c])
                throw iim::IOException("in TimeSeries::loadSubvolume_to_UINT8(): not all frames have the same active channels");
    }

    // initialize progress bar
    if(t1 - t0 > 0)
    {
        iim::imProgressBar::getInstance()->start("5D data loading from disk", false);
        iim::imProgressBar::getInstance()->update(0,"Initializing...");
        iim::imProgressBar::getInstance()->show(false);
    }

    // compute subvol dimension
    size_t subvol_frame_size = static_cast<size_t>(H1-H0) * (V1-V0) * (D1-D0) * (frames[0]->getNACtiveChannels());
    size_t subvol_size = subvol_frame_size * (t1-t0+1);
    iim::uint8* subvol_data = 0;

    /**/iim::debug(iim::LEV3, iim::strprintf("allocating memory for X(%d) x Y(%d) x Z(%d) x C(%d) x T(%d)", H1-H0, V1-V0, D1-D0, frames[0]->getNACtiveChannels(), t1-t0+1).c_str(), __iim__current__function__);

    // try to allocate memory
    try{          subvol_data = new iim::uint8[subvol_size];   }
    catch(...) {  throw iim::IOException("in TimeSeries::loadSubvolume_to_UINT8(): failed to allocate memory for image data"); }

    // load data
    for ( int t=0; t<=t1-t0; t++ )
    {
        if(t1 - t0 > 0)
        {
            iim::imProgressBar::getInstance()->update( (static_cast<float>(t) / (t1-t0))*100, iim::strprintf("Loading time frame %d/%d", t, t1-t0).c_str());
            iim::imProgressBar::getInstance()->setMessage(1, iim::strprintf("Loading time frame %d/%d", t, t1-t0).c_str());
            iim::imProgressBar::getInstance()->show(false);
        }

        iim::uint8* temp_data = frames[t+t0]->loadSubvolume_to_UINT8(V0, V1, H0, H1, D0, D1);
        memcpy(subvol_data + t*subvol_frame_size, temp_data, subvol_frame_size*sizeof(iim::uint8));
        delete[] temp_data;
    }

    // add gaussian noise
    if(iim::ADD_NOISE_TO_TIME_SERIES)
    {
        for(int t=0; t<=t1-t0; t++)
        {
            float w = static_cast<float>(t+t0)/(DIM_T-1);
            size_t t_stride = t*subvol_frame_size;
            for(size_t n=0; n<subvol_frame_size; n++)
                subvol_data[t_stride + n] = static_cast<iim::uint8>((1-w)*subvol_data[t_stride + n] + w*(rand()%256) +0.5f);
        }
    }

    return subvol_data;
}
