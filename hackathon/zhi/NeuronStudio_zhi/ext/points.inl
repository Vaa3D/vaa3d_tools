/***********************************************************
*   points.h :  Simulates points repelling on the surface
*               of a sphere.
*
*   Copyright (c) Alfredo Rodriguez, 2003
*
*   Last Rev Date 07/09/2003
*
************************************************************/


typedef struct tagPOINT3D
{
    nsfloat x;
    nsfloat y;
    nsfloat z;
} POINT3D;


#define DIMENSION 50.0f


void GetPointsCartesian( POINT3D *points, nsint count)
{
    nsint i;
    nsdouble x,y,z,mag;


    /* generate points */
    for(i=0;i<count;i++)
    {
        /* generate random xyz */
        x = (nsdouble)(ns_rand()-(NS_RANDI_MAX/2));
        y = (nsdouble)(ns_rand()-(NS_RANDI_MAX/2));
        z = (nsdouble)(ns_rand()-(NS_RANDI_MAX/2));

        /* compute magnitude */
        mag = x*x+y*y+z*z;
        mag = ns_sqrt(mag);

        /* normalize */
        x /= mag;
        y /= mag;
        z /= mag;

        /* multiply by desired radius and assign to structure */
        points[i].x = (nsfloat)x*DIMENSION;
        points[i].y = (nsfloat)y*DIMENSION;
        points[i].z = (nsfloat)z*DIMENSION;
        
    }
}


void GetPointsPolar( POINT3D *points, nsint count)
{
    nsint i;
    nsdouble phi,theta,p;


    /* generate points */
    for(i=0;i<count;i++)
    {
        /* generate random angles */
        phi = ns_rand()*6.2832/NS_RANDI_MAX;
        theta = ns_rand()*6.2832/NS_RANDI_MAX;

        /* compute xyz coordinates */
        p = ns_sin(phi);
        points[i].x = DIMENSION*(nsfloat)(p * ns_cos(theta));
        points[i].y = DIMENSION*(nsfloat)ns_cos(phi);
        points[i].z = DIMENSION*(nsfloat)(p * ns_sin(theta));
    }
}


void GetPointsCylindrical( POINT3D *points, nsint count)
{
    nsint i;
    nsdouble x,y,z,theta,p;


    /* generate points */
    for(i=0;i<count;i++)
    {
        /* generate random elevation */
        y = 2.0*ns_rand()/NS_RANDI_MAX-1.0;

        /* generate random longitude */
        theta = ns_rand()*6.2832/NS_RANDI_MAX;

        /* compute xz projection */
        p = ns_sqrt(1-y*y);

        /* compute x and z components */
        x = p * ns_cos(theta);
        z = p * ns_sin(theta);

        /* compute xyz coordinates */
        points[i].x = (nsfloat)(DIMENSION*x);
        points[i].y = (nsfloat)(DIMENSION*y);
        points[i].z = (nsfloat)(DIMENSION*z);
    }
}


nsint ComputeForces(POINT3D* points, POINT3D* forces, nsint count)
{
    nsint i,j;
    POINT3D *curr,*other,*force;
    nsdouble dx,dy,dz,mag,factor,fx,fy,fz;

    if(count<2) return 0;

    /* for every point */
    force = forces;
    curr = points;
    for(i=0;i<count;i++)
    {
        /* compute the distance vector to every other point */
        other = points;
        fx = fy = fz = 0.0;
        for(j=0;j<count;j++,other++)
        {
            if(other==curr) continue;

            /* compute the distance vector between the two points */
            dx = curr->x-other->x;
            dy = curr->y-other->y;
            dz = curr->z-other->z;

            /* compute magnitude */
            mag = ns_sqrt(dx*dx+dy*dy+dz*dz);

            /* add to displacement */
            factor = 1.0/(mag*mag*mag);
            fx += (dx*factor);
            fy += (dy*factor);
            fz += (dz*factor);

            /* also add the force of the opposite */
            dx = curr->x+other->x;
            dy = curr->y+other->y;
            dz = curr->z+other->z;
            mag = ns_sqrt(dx*dx+dy*dy+dz*dz);
            factor = 1.0/(mag*mag*mag);
            fx += (dx*factor);
            fy += (dy*factor);
            fz += (dz*factor);

        }

        force->x = (nsfloat) fx;
        force->y = (nsfloat) fy;
        force->z = (nsfloat) fz;

        curr++;
        force++;
    }

    return 0;
}


void ApplyForces(POINT3D* points, POINT3D* forces, nsint count)
{
    nsint i;
    nsdouble mag,maxmag,factor;
    POINT3D *force,*point;

    if(count<2) return;

    /* compute the biggest magnitude */
    force = forces;
    maxmag = 0.0;
    for(i=0;i<count;i++)
    {
        mag = ns_sqrt(force->x*force->x+force->y*force->y+force->z*force->z);
        if(mag>maxmag) maxmag=mag;

        force++;
    }

    /* displace points */
    force = forces;
    point = points;
    factor = 1.0/maxmag;
    for(i=0;i<count;i++)
    {
        /* displace in the direction of the force */
        point->x = (nsfloat)(point->x+factor*force->x);
        point->y = (nsfloat)(point->y+factor*force->y);
        point->z = (nsfloat)(point->z+factor*force->z);

        force++;
        point++;
    }

    force = forces;
    point = points;
    for(i=0;i<count;i++)
    {
        /* bring back into radius */
        mag = ns_sqrt(point->x*point->x+point->y*point->y+point->z*point->z);
        factor = 1/mag*DIMENSION;
        point->x = (nsfloat)(point->x*factor);
        point->y = (nsfloat)(point->y*factor);
        point->z = (nsfloat)(point->z*factor);
        
        force++;
        point++;
    }

    return;
}


void WritePoints(POINT3D* points, nsint count)
{
    nschar filename[300];
    nsint i;
    //FILE *fp;
    POINT3D *p;
    nsfloat x,y,z,mag;

    /* make the filename */
    //sprintf(filename,"%d.dat",count);

    /* open the file for writing */
    //fp = fopen(filename,"w");
    //if(fp==NULL) return -1;

    /* output points */
    p = points;
    for(i=0;i<count;i++)
    {
        /* normalize */
        mag = (nsfloat) ns_sqrt(p->x*p->x+p->y*p->y+p->z*p->z);
        x = p->x/mag;
        y = p->y/mag;
        z = p->z/mag;

        /* output to a line */
       // fprintf(fp,"% f, % f, % f,\n",x,y,z);

        /* adavance pointer */
        p++;
    }

    /* close the file */
    //fclose(fp);
}

