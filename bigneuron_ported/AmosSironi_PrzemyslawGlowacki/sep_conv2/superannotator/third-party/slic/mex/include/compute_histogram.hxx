#ifndef COMPUTE_HISTOGRAM_HXX
#define COMPUTE_HISTOGRAM_HXX

template<typename T>
struct HistogramOpts
{
    T begin, end;
    unsigned int nbins;
};

typedef std::vector<unsigned int>  HistogramType;

// if appendHistogram == true => it keeps on building on the previous histogram
// if computeMean == true => returns mean of pixels, otherwise zero
template<class Iterator, typename T>
inline double computeHistogram( const Iterator &begin, const Iterator &end, HistogramType &hist, const HistogramOpts<T> &opts,
                       bool appendHistogram = false, bool computeMean = false )
{
	double accum = 0.0;
	unsigned int nVals = 0;
	
    if (appendHistogram)
    {
        if (hist.size() != opts.nbins)
            fatalMsg("Histogram size incorrect.");
    } else
    {
        hist.clear();
        hist.resize( opts.nbins, (T) 0 );
    }

    const int nbins = opts.nbins;

    float binStep = ((opts.end - opts.begin)*1.0f)/nbins;
    float binStepInv = 1.0 / binStep;

    for( Iterator it = begin; it < end; it++ )
    {
        float val = *it;

        int bin = (int) (val * binStepInv);
        if (bin < 0) bin = 0;
        if (bin >= nbins) bin = nbins-1;

        hist[bin]++;
        
        if (computeMean) {
			accum += val;
			nVals++;
		}
    }
    
    if (computeMean)
		return accum / nVals;
	else
		return 0.0;
}

#endif // COMPUTE_HISTOGRAM_HXX
