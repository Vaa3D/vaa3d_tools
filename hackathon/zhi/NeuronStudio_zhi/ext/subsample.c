#include "subsample.h"


/* macros for range calculation */
#define LARGER(A,B) (( (A)>(B) )?(A):(B))
#define SMALLER(A,B) (( (A)<(B) )?(A):(B))


SUBSAMPLER* InitSubsampler
	(
	SUBSAMPLER  *ss,
	nsint        xdim,
	nsint        ydim,
	nsint        zdim,
	nsdouble     xscale,
	nsdouble     yscale,
	nsdouble     zscale
	)
	{
	 ss->xdim = xdim;
	 ss->ydim = ydim;
	 ss->zdim = zdim;
	 ss->xscale = xscale;
	 ss->yscale = yscale;
	 ss->zscale = zscale;

	 /* compute new stack width */
	 ss->newxdim = (nsint) ns_ceil(xdim*xscale);
	 if(ss->newxdim==0)  ss->newxdim = 1;

	 /* compute new stack height */
	 ss->newydim = (nsint) ns_ceil(ydim*yscale);
	 if(ss->newydim==0)  ss->newydim = 1;

	 /* compute new stack depth */
	 ss->newzdim = (nsint) ns_ceil(zdim*zscale);
	 if(ss->newzdim==0)  ss->newzdim = 1;

	 /* recompute scales using ceiling values */
	 ss->xscale = (nsdouble) ss->newxdim / ss->xdim;
	 ss->yscale = (nsdouble) ss->newydim / ss->ydim;
	 ss->zscale = (nsdouble) ss->newzdim / ss->zdim;

	 /* internal variables */
	 ss->zpoint     = 0.0;
	 ss->zwindow    = 1.0/ss->zscale;
	 ss->lastin     = -1;
	 ss->lastout    = -1;
	 ss->count      = 0;
	 ss->firstimage = NULL;
	 ss->lastimage  = NULL;

	 return ss;
	}


void SubsamplerSizes
	(
	nsint      xdim,
	nsint      ydim,
	nsint      zdim,
	nsdouble   xscale,
	nsdouble   yscale,
	nsdouble   zscale,
	nsint     *new_x_dim,
	nsint     *new_y_dim,
	nsint     *new_z_dim
	)
	{
	SUBSAMPLER ss;

	InitSubsampler( &ss, xdim, ydim, zdim, xscale, yscale, zscale );

	*new_x_dim = ss.newxdim;
	*new_y_dim = ss.newydim;
	*new_z_dim = ss.newzdim;
	}


/*
SUBSAMPLER* CreateSubsampler( nsint xdim, nsint ydim, nsint zdim, nsdouble xscale, nsdouble yscale, nsdouble zscale )
	{
	SUBSAMPLER *ss;

	// protect agains spurious values
	if(xscale>1.0 || xscale <0.0) return NULL;
	if(yscale>1.0 || yscale <0.0) return NULL;
	if(zscale>1.0 || zscale <0.0) return NULL;

	// allocate a subsampler object
	ss = (SUBSAMPLER*) ns_malloc(sizeof(SUBSAMPLER));
	if(ss==NULL) return NULL;

	// if allocation was succesfull then set the members
	return InitSubsampler( ss, xdim, ydim, zdim, xscale, yscale, zscale );
	}
*/


nsint SubsamplerIsComplete( SUBSAMPLER *subsampler )
	{
	return (subsampler->lastout == (subsampler->newzdim-1));
	}


nsint SubsamplerNextIndex(SUBSAMPLER* subsampler)
	{
	nsdouble  endpoint;
	nsint     lastindex;

	/* if no images return 0 index */
	if(subsampler->lastin==-1) return 0;

	/* if last image has been inserted return -1 */
	if(subsampler->lastin==(subsampler->zdim-1)) return -1;

	/* compute last index needed for current iteration */
	endpoint = subsampler->zpoint+subsampler->zwindow;
	lastindex = (nsint) endpoint;
	if(endpoint==ns_floor(endpoint)) lastindex--;

	/* if current iteration is ready for subsampling return -1 */
	if(lastindex==subsampler->lastin) return -1;

	/* otherwise return lastin+1 */
	return (subsampler->lastin+1);
	}


void DestroySubsamplerImage(SSIMAGE* image)
{
    if(image==NULL) return;

    //ns_free(image->data);
    ns_free(image);

    return;
}


void SubsamplerAddDestImage( SUBSAMPLER *subsampler, void *data )
	{
    /* set mebers of structure */
   subsampler->destimage.xdim = subsampler->newxdim;
   subsampler->destimage.ydim = subsampler->newydim;
   subsampler->destimage.data = data;
   subsampler->destimage.next = NULL;
	}


/* return 0 on success, -1 on error */
nsint SubsamplerAddSrcImage(SUBSAMPLER* subsampler, const void *data)
{
    SSIMAGE* im;

    /* if current iteration is full do not add image */
    if(SubsamplerNextIndex(subsampler)==-1) return -1;

    /* allocate an image structure */
    im = (SSIMAGE*) ns_malloc(sizeof(SSIMAGE));
    if(im==NULL) return -1;

    /* set mebers of structure */
    im->xdim = subsampler->xdim;
    im->ydim = subsampler->ydim;
    im->data = ( void* )data;
    im->next = NULL;

    /* place on subsampler list */
    if(subsampler->firstimage==NULL)
    {
        subsampler->firstimage = im;
        subsampler->lastimage = im;
    }
    else
    {
        subsampler->lastimage->next = im;
        subsampler->lastimage = im;
    }
    subsampler->lastin++;
    subsampler->count++;

    return 0;
}


void CleanSubsampler(SUBSAMPLER* subsampler)
{
    SSIMAGE* im;

    /* destroy all the images */
    while(subsampler->firstimage!=NULL)
    {
        im = subsampler->firstimage;
        subsampler->firstimage = subsampler->firstimage->next;
        DestroySubsamplerImage(im);
    }
    //ns_free(subsampler);

    return;
}


/*
nsint GetImageWidth(SSIMAGE* image)
{
    return image->xdim;
}


nsint GetImageHeight(SSIMAGE* image)
{
    return image->ydim;
}
*/


/* this routine can be used to shrink a two dimensional image */
nsuchar* ShrinkImage1ub( SUBSAMPLER *ss, nsuchar* data,nsint xdim, nsint ydim,
                           nsint newxdim, nsint newydim)
{
    nsuchar* newdata;
    nsint x,y;
    nsint xlo,xhi,ylo,yhi,row,col;
    nsdouble xwindow,ywindow;
    nsdouble value,start,end;
    nsdouble xoverlap,yoverlap;

    /* allocate buffer for new image */
    //newdata = (nsuchar*) ns_malloc(sizeof(nschar)*newxdim*newydim);
    //if(newdata==NULL) return NULL;
	newdata = ss->destimage.data;

    /* compute the size of the window */
    xwindow = (nsdouble) xdim/newxdim;
    ywindow = (nsdouble) ydim/newydim;
    
    /* for every pixel in the subsampled image */
    for(y=0;y<newydim;y++)
    {
        /* compute vertical pixel range in original image */
        ylo = (nsint) ns_floor(y*ywindow);
        yhi = (nsint) ns_ceil((y+1)*ywindow);

        for(x=0;x<newxdim;x++)
        {
            /* compute horizontal pixel range in original image */
            xlo = (nsint) ns_floor(x*xwindow);
            xhi = (nsint) ns_ceil((x+1)*xwindow);

            /* add pixel contributions */
            value = 0.0;
            for(row=ylo;row<yhi;row++)
            {
                /* compute what fraction of pixel overlaps destination */
                start = LARGER((nsdouble)row,y*ywindow);
                end = SMALLER((nsdouble)row+1.0,(y+1)*ywindow);
                yoverlap = LARGER(end-start,0.0);

                for(col=xlo;col<xhi;col++)
                {
                    /* compute what fraction of pixel overlaps destination */
                    start = LARGER((nsdouble)col,x*xwindow);
                    end = SMALLER((nsdouble)col+1,(x+1)*xwindow);
                    xoverlap = LARGER(end-start,0.0);

                    /* add contribution */
                    value= value+*(data+row*xdim+col)*xoverlap*yoverlap;
                }
            }

            /* normalize value */
            value = value /(xwindow*ywindow);
            if(value>255.00) value=255.0;

            /* assign value to bucket */
            *(newdata+y*newxdim+x) = (nsuchar) value;
        }
    }

    return newdata;
}


nsint SubsampleLum1ub(SUBSAMPLER* subsampler)
{
    nsdouble *weights,fpixel;
    nsint i,x,y,ipixel,index;
    nsuchar *data/*,*data2*/;
    SSIMAGE *image,*hold;
    nsint zlo/*,zhi*/;
    nsdouble zoverlap,start,end;

    /* if more images are needed return NULL */
    if(SubsamplerNextIndex(subsampler)!=-1) return -1;

    /*****************
    * Subsample in Z
    ******************/
    /* allocate weights arrays */
    weights = (nsdouble*) ns_malloc(sizeof(nsdouble)*subsampler->count);
    if(weights==NULL)
    {
        return -1;
    }

    /* compute the weights for each image */
    index = subsampler->lastin-subsampler->count+1; /* starting index */
    for(i=0,image=subsampler->firstimage;image!=NULL;image=image->next,i++)
    {
        /* compute what fraction of image overlaps destination */
        start = LARGER((nsdouble)index,(subsampler->lastout+1)*subsampler->zwindow);
        end = SMALLER((index+1.0),(subsampler->lastout+2)*subsampler->zwindow);
        zoverlap = LARGER(end-start,0.0);

        /* normalize */
        weights[i] = zoverlap/subsampler->zwindow;

        //PRINTF1("\n%f",weights[i]);
        
        index++;
    }

    /* allocate image buffer xdim*ydim */
    data = (nsuchar*) ns_malloc(subsampler->xdim*subsampler->ydim);
    if(data==NULL)
    {
        ns_free(weights);
        return -1;
    }

    /* for each pixel */
    for(y=0;y<subsampler->ydim;y++)
    {
        for(x=0;x<subsampler->xdim;x++)
        {
            /* reset value */
            fpixel = 0.0;

            /* add contributions of every image to that pixel */
            i = 0;
            for(image=subsampler->firstimage;image!=NULL;image=image->next)
            {
                /* add weighted pixel contribution */
                fpixel += (weights[i] * *(((nsuchar*)image->data)+y*subsampler->xdim+x));
                i++;
            }
            ipixel = (nsint) fpixel;
            if(ipixel>255) ipixel = 255;

            /* set the pixel at location x,y to computed pixel */
            *(data+y*subsampler->xdim+x) = (nsuchar) ipixel;
        }
    }

    /* free weights array */
    ns_free(weights);

    /********************
    * Subsample in X & Y
    *********************/
    /*data2 =*/ ShrinkImage1ub(subsampler, data,subsampler->xdim,subsampler->ydim,
                       subsampler->newxdim,subsampler->newydim);
    ns_free(data);
    
	//if(data2==NULL) return -1;

    /* allocate an image structure */
    //image = (SSIMAGE*) ns_malloc(sizeof(SSIMAGE));
    //if(image==NULL)
    //{
      //  ns_free(data2);
        //return NULL;
    //}

    /* set members */
    //image->xdim = subsampler->newxdim;
    //image->ydim = subsampler->newydim;
    //image->data = data2;
    //image->next = NULL;

    /* update members */
    subsampler->lastout++;
    subsampler->zpoint += subsampler->zwindow;

    /* remove unneeded images */
    index = subsampler->lastin-subsampler->count+1; /* starting index */
    zlo = (nsint) ns_floor((subsampler->lastout+1)*(subsampler->zwindow));
    while(subsampler->firstimage!=NULL)
    {
        if(index>=zlo) break;

        //PRINTF1("\nRemoving %d... ",index);

        /* remove image */
        hold = subsampler->firstimage;
        subsampler->firstimage=hold->next;
        if(subsampler->firstimage==NULL) subsampler->lastimage=NULL;
        DestroySubsamplerImage(hold);
        index++;
        subsampler->count--;

        //PRINTF1("after count: %d\n",subsampler->count);

    }

    //return image;
	return 0;
}




/* this routine can be used to shrink a two dimensional image */
nsushort* ShrinkImage1us12(SUBSAMPLER *ss, nsushort* data,nsint xdim, nsint ydim,
                           nsint newxdim, nsint newydim)
{
    nsushort* newdata;
    nsint x,y;
    nsint xlo,xhi,ylo,yhi,row,col;
    nsdouble xwindow,ywindow;
    nsdouble value,start,end;
    nsdouble xoverlap,yoverlap;

    /* allocate buffer for new image */
    //newdata = (nsushort*) ns_malloc(sizeof(nsshort)*newxdim*newydim);
    //if(newdata==NULL) return NULL;
	newdata = ss->destimage.data;

    /* compute the size of the window */
    xwindow = (nsdouble) xdim/newxdim;
    ywindow = (nsdouble) ydim/newydim;
    
    /* for every pixel in the subsampled image */
    for(y=0;y<newydim;y++)
    {
        /* compute vertical pixel range in original image */
        ylo = (nsint) ns_floor(y*ywindow);
        yhi = (nsint) ns_ceil((y+1)*ywindow);

        for(x=0;x<newxdim;x++)
        {
            /* compute horizontal pixel range in original image */
            xlo = (nsint) ns_floor(x*xwindow);
            xhi = (nsint) ns_ceil((x+1)*xwindow);

            /* add pixel contributions */
            value = 0.0;
            for(row=ylo;row<yhi;row++)
            {
                /* compute what fraction of pixel overlaps destination */
                start = LARGER((nsdouble)row,y*ywindow);
                end = SMALLER((nsdouble)row+1.0,(y+1)*ywindow);
                yoverlap = LARGER(end-start,0.0);

                for(col=xlo;col<xhi;col++)
                {
                    /* compute what fraction of pixel overlaps destination */
                    start = LARGER((nsdouble)col,x*xwindow);
                    end = SMALLER((nsdouble)col+1,(x+1)*xwindow);
                    xoverlap = LARGER(end-start,0.0);

                    /* add contribution */
                    value= value+*(data+row*xdim+col)*xoverlap*yoverlap;
                }
            }

            /* normalize value */
            value = value /(xwindow*ywindow);
            if(value>4095.0) value=4095.0;

            /* assign value to bucket */
            *(newdata+y*newxdim+x) = (nsushort) value;
        }
    }

    return newdata;
}


nsint SubsampleLum1us12(SUBSAMPLER* subsampler)
{
    nsdouble *weights,fpixel;
    nsint i,x,y,ipixel,index;
    nsushort *data/*,*data2*/;
    SSIMAGE *image,*hold;
    nsint zlo/*,zhi*/;
    nsdouble zoverlap,start,end;

    /* if more images are needed return NULL */
    if(SubsamplerNextIndex(subsampler)!=-1) return -1;

    /*****************
    * Subsample in Z
    ******************/
    /* allocate weights arrays */
    weights = (nsdouble*) ns_malloc(sizeof(nsdouble)*subsampler->count);
    if(weights==NULL)
    {
        return -1;
    }

    /* compute the weights for each image */
    index = subsampler->lastin-subsampler->count+1; /* starting index */
    for(i=0,image=subsampler->firstimage;image!=NULL;image=image->next,i++)
    {
        /* compute what fraction of image overlaps destination */
        start = LARGER((nsdouble)index,(subsampler->lastout+1)*subsampler->zwindow);
        end = SMALLER((index+1.0),(subsampler->lastout+2)*subsampler->zwindow);
        zoverlap = LARGER(end-start,0.0);

        /* normalize */
        weights[i] = zoverlap/subsampler->zwindow;

        //PRINTF1("\n%f",weights[i]);
        
        index++;
    }

    /* allocate image buffer xdim*ydim */
    data = (nsushort*) ns_malloc(sizeof(nsshort)*subsampler->xdim*subsampler->ydim);
    if(data==NULL)
    {
        ns_free(weights);
        return -1;
    }

    /* for each pixel */
    for(y=0;y<subsampler->ydim;y++)
    {
        for(x=0;x<subsampler->xdim;x++)
        {
            /* reset value */
            fpixel = 0.0;

            /* add contributions of every image to that pixel */
            i = 0;
            for(image=subsampler->firstimage;image!=NULL;image=image->next)
            {
                /* add weighted pixel contribution */
                fpixel += (weights[i] * *(((nsushort*)image->data)+y*subsampler->xdim+x));
                i++;
            }
            ipixel = (nsint) fpixel;
            if(ipixel>4095) ipixel = 4095;

            /* set the pixel at location x,y to computed pixel */
            *(data+y*subsampler->xdim+x) = (nsushort) ipixel;
        }
    }

    /* free weights array */
    ns_free(weights);

    /********************
    * Subsample in X & Y
    *********************/
    /*data2 =*/ ShrinkImage1us12(subsampler,data,subsampler->xdim,subsampler->ydim,
                       subsampler->newxdim,subsampler->newydim);
    ns_free(data);
    
	//if(data2==NULL) return -1;

    /* allocate an image structure */
    //image = (SSIMAGE*) ns_malloc(sizeof(SSIMAGE));
    //if(image==NULL)
    //{
      //  ns_free(data2);
        //return NULL;
    //}

    /* set members */
    //image->xdim = subsampler->newxdim;
    //image->ydim = subsampler->newydim;
    //image->data = data2;
    //image->next = NULL;

    /* update members */
    subsampler->lastout++;
    subsampler->zpoint += subsampler->zwindow;

    /* remove unneeded images */
    index = subsampler->lastin-subsampler->count+1; /* starting index */
    zlo = (nsint) ns_floor((subsampler->lastout+1)*(subsampler->zwindow));
    while(subsampler->firstimage!=NULL)
    {
        if(index>=zlo) break;

        //PRINTF1("\nRemoving %d... ",index);

        /* remove image */
        hold = subsampler->firstimage;
        subsampler->firstimage=hold->next;
        if(subsampler->firstimage==NULL) subsampler->lastimage=NULL;
        DestroySubsamplerImage(hold);
        index++;
        subsampler->count--;

        //PRINTF1("after count: %d\n",subsampler->count);

    }

    //return image;
	return 0;
}







/* this routine can be used to shrink a two dimensional image */
nsushort* ShrinkImage1us16(SUBSAMPLER *ss, nsushort* data,nsint xdim, nsint ydim,
                           nsint newxdim, nsint newydim)
{
    nsushort* newdata;
    nsint x,y;
    nsint xlo,xhi,ylo,yhi,row,col;
    nsdouble xwindow,ywindow;
    nsdouble value,start,end;
    nsdouble xoverlap,yoverlap;

    /* allocate buffer for new image */
    //newdata = (nsushort*) ns_malloc(sizeof(nsshort)*newxdim*newydim);
    //if(newdata==NULL) return NULL;
	newdata = ss->destimage.data;

    /* compute the size of the window */
    xwindow = (nsdouble) xdim/newxdim;
    ywindow = (nsdouble) ydim/newydim;
    
    /* for every pixel in the subsampled image */
    for(y=0;y<newydim;y++)
    {
        /* compute vertical pixel range in original image */
        ylo = (nsint) ns_floor(y*ywindow);
        yhi = (nsint) ns_ceil((y+1)*ywindow);

        for(x=0;x<newxdim;x++)
        {
            /* compute horizontal pixel range in original image */
            xlo = (nsint) ns_floor(x*xwindow);
            xhi = (nsint) ns_ceil((x+1)*xwindow);

            /* add pixel contributions */
            value = 0.0;
            for(row=ylo;row<yhi;row++)
            {
                /* compute what fraction of pixel overlaps destination */
                start = LARGER((nsdouble)row,y*ywindow);
                end = SMALLER((nsdouble)row+1.0,(y+1)*ywindow);
                yoverlap = LARGER(end-start,0.0);

                for(col=xlo;col<xhi;col++)
                {
                    /* compute what fraction of pixel overlaps destination */
                    start = LARGER((nsdouble)col,x*xwindow);
                    end = SMALLER((nsdouble)col+1,(x+1)*xwindow);
                    xoverlap = LARGER(end-start,0.0);

                    /* add contribution */
                    value= value+*(data+row*xdim+col)*xoverlap*yoverlap;
                }
            }

            /* normalize value */
            value = value /(xwindow*ywindow);
            if(value>65535.0) value=65535.0;

            /* assign value to bucket */
            *(newdata+y*newxdim+x) = (nsushort) value;
        }
    }

    return newdata;
}


nsint SubsampleLum1us16(SUBSAMPLER* subsampler)
{
    nsdouble *weights,fpixel;
    nsint i,x,y,ipixel,index;
    nsushort *data/*,*data2*/;
    SSIMAGE *image,*hold;
    nsint zlo/*,zhi*/;
    nsdouble zoverlap,start,end;

    /* if more images are needed return NULL */
    if(SubsamplerNextIndex(subsampler)!=-1) return -1;

    /*****************
    * Subsample in Z
    ******************/
    /* allocate weights arrays */
    weights = (nsdouble*) ns_malloc(sizeof(nsdouble)*subsampler->count);
    if(weights==NULL)
    {
        return -1;
    }

    /* compute the weights for each image */
    index = subsampler->lastin-subsampler->count+1; /* starting index */
    for(i=0,image=subsampler->firstimage;image!=NULL;image=image->next,i++)
    {
        /* compute what fraction of image overlaps destination */
        start = LARGER((nsdouble)index,(subsampler->lastout+1)*subsampler->zwindow);
        end = SMALLER((index+1.0),(subsampler->lastout+2)*subsampler->zwindow);
        zoverlap = LARGER(end-start,0.0);

        /* normalize */
        weights[i] = zoverlap/subsampler->zwindow;

        //PRINTF1("\n%f",weights[i]);
        
        index++;
    }

    /* allocate image buffer xdim*ydim */
    data = (nsushort*) ns_malloc(sizeof(nsshort)*subsampler->xdim*subsampler->ydim);
    if(data==NULL)
    {
        ns_free(weights);
        return -1;
    }

    /* for each pixel */
    for(y=0;y<subsampler->ydim;y++)
    {
        for(x=0;x<subsampler->xdim;x++)
        {
            /* reset value */
            fpixel = 0.0;

            /* add contributions of every image to that pixel */
            i = 0;
            for(image=subsampler->firstimage;image!=NULL;image=image->next)
            {
                /* add weighted pixel contribution */
                fpixel += (weights[i] * *(((nsushort*)image->data)+y*subsampler->xdim+x));
                i++;
            }
            ipixel = (nsint) fpixel;
            if(ipixel>65535) ipixel = 65535;

            /* set the pixel at location x,y to computed pixel */
            *(data+y*subsampler->xdim+x) = (nsushort) ipixel;
        }
    }

    /* free weights array */
    ns_free(weights);

    /********************
    * Subsample in X & Y
    *********************/
    /*data2 =*/ ShrinkImage1us16(subsampler,data,subsampler->xdim,subsampler->ydim,
                       subsampler->newxdim,subsampler->newydim);
    ns_free(data);

 //   if(data2==NULL) return -1;

    /* allocate an image structure */
    //image = (SSIMAGE*) ns_malloc(sizeof(SSIMAGE));
    //if(image==NULL)
    //{
      //  ns_free(data2);
        //return NULL;
    //}

    /* set members */
    //image->xdim = subsampler->newxdim;
    //image->ydim = subsampler->newydim;
    //image->data = data2;
    //image->next = NULL;

    /* update members */
    subsampler->lastout++;
    subsampler->zpoint += subsampler->zwindow;

    /* remove unneeded images */
    index = subsampler->lastin-subsampler->count+1; /* starting index */
    zlo = (nsint) ns_floor((subsampler->lastout+1)*(subsampler->zwindow));
    while(subsampler->firstimage!=NULL)
    {
        if(index>=zlo) break;

        //PRINTF1("\nRemoving %d... ",index);

        /* remove image */
        hold = subsampler->firstimage;
        subsampler->firstimage=hold->next;
        if(subsampler->firstimage==NULL) subsampler->lastimage=NULL;
        DestroySubsamplerImage(hold);
        index++;
        subsampler->count--;

        //PRINTF1("after count: %d\n",subsampler->count);

    }

    //return image;
	return 0;
}