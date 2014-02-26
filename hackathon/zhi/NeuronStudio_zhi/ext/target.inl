/*************************************************************

    target.h :  smooths a vector using a gaussian filter and
                returns the maximum after smoothing.

**************************************************************/


// returns zero-based index or -1 on memory error
nsint GetTargetIndex(nsfloat* values, nsint count, nsint passes)
{
    nsint i,pos,maxstart,maxend;
    nsdouble maxval;
    nsdouble *buff1,*buff2;
    nsdouble *prev,*curr,*hold;
    nsdouble gaussian[5] = {0.039,0.235,0.452,0.235,0.039};


	maxend = 0;


    // allocate two vectors of doubles to hold temp data
    buff1 = (nsdouble*) ns_malloc(sizeof(nsdouble)*(count+4));
    buff2 = (nsdouble*) ns_malloc(sizeof(nsdouble)*(count+4));
    if(buff1==NULL||buff2==NULL)
    {
        ns_free(buff1);
        ns_free(buff2);
        return -1;
    }

    //set curr and prev to correct positions
    curr = buff1+2;
    prev = buff2+2;

    // copy data from input vector into curr array
    for(i=0;i<count;i++) curr[i] = values[i];


    //do smoothing passes
    for(i=0;i<passes;i++)
    {
        // swap buffers
        hold = prev;
        prev = curr;
        curr = hold;

        // clear current array
        ns_memset(curr-2,0,sizeof(nsdouble)*(count+4));

        // assume gussian has five values
        for(pos=0;pos<count;pos++)
        {
            curr[pos-2] += prev[pos]*gaussian[0];
            curr[pos-1] += prev[pos]*gaussian[1];
            curr[pos+0] += prev[pos]*gaussian[2];
            curr[pos+1] += prev[pos]*gaussian[3];
            curr[pos+2] += prev[pos]*gaussian[4];
        }
    }


    // serach for highest peak
    maxstart = 0;
    maxval = curr[0];
    for(i=0;i<count;i++)
    {
        if(curr[i]>maxval)
        {
            maxstart = i;
            maxval = curr[i];
        }
        if(curr[i]==maxval) maxend = i;
    }


    // ns_free memory
    ns_free(buff1);
    ns_free(buff2);

    return (maxstart+maxend)/2;
}





nsint* GetTargetIndices(nsfloat* values, nsint count, nsint passes,
                      nsfloat cutoff, nsint* found)
{
    nsint i,pos,maxi,saved;
    nsdouble maxval,cutoffval;

    nsdouble *buff1,*buff2;
    nsdouble *prev,*curr,*hold;
    nsdouble gaussian[5] = {0.039,0.235,0.452,0.235,0.039};
    nsint* peaks;


    // allocate two vectors of doubles to hold temp data
    buff1 = (nsdouble*) ns_malloc(sizeof(nsdouble)*(count+4));
    buff2 = (nsdouble*) ns_malloc(sizeof(nsdouble)*(count+4));
    if(buff1==NULL||buff2==NULL)
    {
        ns_free(buff1);
        ns_free(buff2);
        return NULL;
    }

    //set curr and prev to correct positions
    curr = buff1+2;
    prev = buff2+2;

    // copy data from input vector into curr array
    for(i=0;i<count;i++) curr[i] = values[i];


    //do smoothing passes
    for(i=0;i<passes;i++)
    {
        // swap buffers
        hold = prev;
        prev = curr;
        curr = hold;

        // clear current array
        ns_memset(curr-2,0,sizeof(nsdouble)*(count+4));

        // assume gussian has five values
        for(pos=0;pos<count;pos++)
        {
            curr[pos-2] += prev[pos]*gaussian[0];
            curr[pos-1] += prev[pos]*gaussian[1];
            curr[pos+0] += prev[pos]*gaussian[2];
            curr[pos+1] += prev[pos]*gaussian[3];
            curr[pos+2] += prev[pos]*gaussian[4];
        }
    }

    // serach for highest peak
    maxi = 0;
    maxval = curr[maxi];
    for(i=0;i<count;i++)
    {
        if(curr[i]>maxval)
        {
            maxi = i;
            maxval = curr[maxi];
        }
    }

    //zero out anything that is not a local peak
    *found = 0;
    cutoffval = cutoff * values[maxi]/*maxval*/;
    for(i=0;i<count;i++)
    {
        if(/*curr[i]*/values[i]<cutoffval||curr[i]<curr[i-1]||curr[i]<curr[i+1]|| values[i] == 0.0 )
        {
            prev[i] = 0.0;
        }
        else
        {
            prev[i] = curr[i];
            (*found)++;
        }
    }

    // allocate an array of integers to return values
    if((*found)==0) peaks = (nsint*) ns_malloc(sizeof(nsint));
    else peaks = ns_malloc(sizeof(nsint)*(*found));
    if(peaks==NULL) return NULL;

    // save all the peaks and return
    saved = 0;
    for(i=0;i<count;i++)
    {
        if(prev[i]!=0.0)
        {
            peaks[saved] = i;
            saved++;
        }
    }

    // ns_free memory
    ns_free(buff1);
    ns_free(buff2);

    return peaks;
}
