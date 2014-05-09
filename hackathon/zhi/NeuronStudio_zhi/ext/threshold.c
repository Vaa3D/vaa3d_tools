#include "threshold.h"


nsfloat compute_confidence( const nsfloat *value, nssize width, nssize height, nssize length, nsfloat threshold )
	{
	nssize  x, y, z;
	nssize  stride;
	nssize  count;
	nssize  wrap;
	nssize  object;
	nssize  background;


	stride     = width * height;
	count      = 0;
	object     = 0;
	background = 0;

	for( z = 0; z < length; ++z )
		for( y = 0; y < height; ++y )
			for( x = 0; x < width; ++x )
				{
				if( *value >= threshold )
					{
					++object;

					if( x < width - 1 && *( value + 1 ) < threshold )
						++count;

					if( y < height - 1 && *( value + width ) < threshold )
						++count;

					if( z < length - 1 && *( value + stride ) < threshold )
						++count;
					}
				else
					{
					++background;

					if( x < width - 1 && *( value + 1 ) >= threshold )
						++count;

					if( y < height - 1 && *( value + width ) >= threshold )
						++count;

					if( z < length - 1 && *( value + stride ) >= threshold )
						++count;
					}

				++value;
				}

	wrap = 2 * width * height + 2 * width * length + 2 * height * length;

	return ( ( nsfloat )wrap / ( nsfloat )count ) /** ( ( nsfloat )object / ( nsfloat )background )*/;
	}



#define _TOLERANCE  0.001


/* The 'dynamic_range' parameter is used for confidence computation. */
void ComputeThreshold
	(
	nsfloat          *values,
	nsint             count,
	ThresholdStruct  *s,
	nsdouble          dynamic_range,
	nsboolean         flatten
	)
	{
    nsint i,hicount,locount;
    nsdouble threshold,contrast,hisum,losum, prevt;
    nsdouble dif,average,density,confidence;


	confidence = density = contrast = 0.0;

    // compute average of the data to use as first estimate
    hisum = 0.0f;
    for(i=0;i<count;i++)
    {
        hisum += values[i];
    }
    average = hisum/count;

    // compute threshold using isodata method
    threshold = average; // first guess is sample mean
    dif=_TOLERANCE+1.0;  // forces at least one iteration
    while(dif>_TOLERANCE)
    {
        // save previous value to compute difference
        prevt = threshold;

        hisum = losum = 0.0;
        hicount = locount = 0;
    
        for(i=0;i<count;i++)
        {
            if(values[i]<prevt)
            {
                losum += values[i];
                locount++;
            }
            else
            {
                hisum += values[i];
                hicount++;
            }
        }

        // update threshold
        if(locount==0) // in case all values are the same
        {
            threshold = values[0];
            contrast = 0.0;
            density = 1.0;
            confidence = 0.0;
        }
        else    // otherwise set at center of hi-average and lo-average
        {
            threshold = (hisum/hicount+losum/locount)/2.0;
            contrast = (hisum/hicount-losum/locount)/2.0;
            density = hicount/(nsdouble)count;
            confidence = contrast/density/(dynamic_range/2.0)/*128.0*/;
        }

        // compute difference of previous and current
        dif = ns_fabs(prevt-threshold);

    }

    // set members
    s->threshold = (nsfloat) threshold;
    s->average = (nsfloat) average;
    s->contrast = (nsfloat) contrast;
    s->density = (nsfloat) density;
    s->confidence = (nsfloat) confidence;


/* IMPORTANT: Comment out from here to end of function to
	avoid flattening the data. Improves speed. */

	if( flatten )
		{

		 // do one pass of the data to flatten sample
		 for(i=0;i<count;i++)
		 {
			  if(values[i]>threshold+contrast)
					values[i] = (nsfloat) (threshold+contrast);
			  else if(values[i]<threshold-contrast)
					values[i] = (nsfloat) (threshold-contrast);
        
		 }


		 // recompute threshold using isodata method
		 // threshold starts at previous value
		 dif=_TOLERANCE+1.0;  // forces at least one iteration
		 while(dif>_TOLERANCE)
		 {
			  // save previous value to compute difference
			  prevt = threshold;

			  hisum = losum = 0.0;
			  hicount = locount = 0;
    
			  for(i=0;i<count;i++)
			  {
					if(values[i]<prevt)
					{
						 losum += values[i];
						 locount++;
					}
					else
					{
						 hisum += values[i];
						 hicount++;
					}
			  }

			  // update threshold
			  if(locount==0) // in case all values are the same
			  {
					threshold = values[0];
					contrast = 0.0;
					density = 1.0;
					confidence = 0.0;
			  }
			  else    // otherwise set at center of hi-average and lo-average
			  {
					threshold = (hisum/hicount+losum/locount)/2.0;
					contrast = (hisum/hicount-losum/locount)/2.0;
					density = hicount/(nsdouble)count;
					confidence = contrast/density/(dynamic_range/2.0)/*128.0*/;
			  }

			  // compute difference of previous and current
			  dif = ns_fabs(prevt-threshold);

		 }

		 // pick lowest threshold
		 if(threshold<s->threshold)
		 {
			  s->threshold = (nsfloat) threshold;
			  s->average = (nsfloat) average;
			  s->contrast = (nsfloat) contrast;
			  s->density = (nsfloat) density;
			  s->confidence = (nsfloat) confidence;
		 }


	}
    

    return;
}





/* The 'dynamic_range' parameter is used for confidence computation. */
void ComputeDualThreshold
	(
	nsfloat          *values,
	nsint             count,
	ThresholdStruct  *s,
	nsdouble          dynamic_range
	)
	{
    nsint i,hicount,locount,dualcount;
    nsdouble threshold,contrast,hisum,losum, prevt;
    nsdouble dif,average,density,confidence;


	confidence = density = contrast = 0.0;

    // compute average of the data to use as first estimate
    hisum = 0.0f;
    for(i=0;i<count;i++)
    {
        hisum += values[i];
    }
    average = hisum/count;

    // compute threshold using isodata method
    threshold = average; // first guess is sample mean
    dif=_TOLERANCE+1.0;  // forces at least one iteration
    while(dif>_TOLERANCE)
    {
        // save previous value to compute difference
        prevt = threshold;

        hisum = losum = 0.0;
        hicount = locount = 0;
    
        for(i=0;i<count;i++)
        {
            if(values[i]<prevt)
            {
                losum += values[i];
                locount++;
            }
            else
            {
                hisum += values[i];
                hicount++;
            }
        }

        // update threshold
        if(locount==0) // in case all values are the same
        {
            threshold = values[0];
            contrast = 0.0;
            density = 1.0;
            confidence = 0.0;
        }
        else    // otherwise set at center of hi-average and lo-average
        {
            threshold = (hisum/hicount+losum/locount)/2.0;
            contrast = (hisum/hicount-losum/locount)/2.0;
            density = hicount/(nsdouble)count;
            confidence = contrast/density/(dynamic_range/2.0)/*128.0*/;
        }

        // compute difference of previous and current
        dif = ns_fabs(prevt-threshold);

    }

    // set members
    s->threshold = (nsfloat) threshold;
    s->average = (nsfloat) average;
    s->contrast = (nsfloat) contrast;
    s->density = (nsfloat) density;
    s->confidence = (nsfloat) confidence;


/* Second threshold. */


// compute average of values below threshold
    hisum = 0.0f;
	 dualcount=0;
    for(i=0;i<count;i++)
    {
        if(values[i]<threshold)
		  {
				dualcount++;
				hisum += values[i];
		  }
    }
    average = hisum/dualcount;
	 threshold=average;

	 // recompute threshold using isodata method
	 // threshold starts at previous value
	 dif=_TOLERANCE+1.0;  // forces at least one iteration
	 while(dif>_TOLERANCE)
	 {
		  // save previous value to compute difference
		  prevt = threshold;

		  hisum = losum = 0.0;
		  hicount = locount = 0;
 
		  for(i=0;i<count;i++)
		  {
			if(values[i] < s->threshold)
			{
				if(values[i]<prevt)
				{
					 losum += values[i];
					 locount++;
				}
				else
				{
					 hisum += values[i];
					 hicount++;
				}
			}
		  }

		  // update threshold
		  if(locount==0||hicount==0) // in case all values are the same
		  {
				threshold = s->threshold;
		  }
		  else    // otherwise set at center of hi-average and lo-average
		  {
				threshold = (hisum/hicount+losum/locount)/2.0;
		  }

		  // compute difference of previous and current
		  dif = ns_fabs(prevt-threshold);

	 }

	 // save threshold
	 s->dual_threshold = (nsfloat) threshold;
 

    return;
}
