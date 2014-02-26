#include "splat.h"
#include <ns-gl.h>


nsint splat_init = 0;
GLuint splat_tex_id;
nsfloat splat_size = 0.50f;

/* This array is used to specify texture and vertex data with GL_T2F_V3F */
nsfloat splat_array[20] = {            //  2    3    4
                            1.0f,1.0f,  0.0f,0.0f,0.0f,
                            0.0f,1.0f,  0.0f,0.0f,0.0f,
                            1.0f,0.0f,  0.0f,0.0f,0.0f,
                            0.0f,0.0f,  0.0f,0.0f,0.0f
                        };


#define SPLAT_RES   16
#define SPLAT_PI    3.141593f

nsfloat splat_up[3];
nsfloat splat_right[3];
nsfloat splat_negoff[3];
nsfloat splat_posoff[3];




void SplatRender( nsfloat coord[3], nsfloat rgba[4] )
{
    // update vertext data
    splat_array[2] = coord[0]+splat_posoff[0];
    splat_array[3] = coord[1]+splat_posoff[1];
    splat_array[4] = coord[2]+splat_posoff[2];

    splat_array[7] = coord[0]-splat_negoff[0];
    splat_array[8] = coord[1]-splat_negoff[1];
    splat_array[9] = coord[2]-splat_negoff[2];

    splat_array[12] = coord[0]+splat_negoff[0];
    splat_array[13] = coord[1]+splat_negoff[1];
    splat_array[14] = coord[2]+splat_negoff[2];

    splat_array[17] = coord[0]-splat_posoff[0];
    splat_array[18] = coord[1]-splat_posoff[1];
    splat_array[19] = coord[2]-splat_posoff[2];




    // draw polygons
    //glColor4f(value,value,value,value);
	//glColor4f(splat_color[0]*value,splat_color[1]*value,splat_color[2]*value,value);
	//glColor4f(splat_color[0],splat_color[1],splat_color[2],value);

	glColor4f( rgba[0], rgba[1], rgba[2], rgba[3] );
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);



    /*
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(1.0f,1.0f);
    glVertex3f(coord[0]+splat_posoff[0],
               coord[1]+splat_posoff[1],
               coord[2]+splat_posoff[2]);

    glTexCoord2f(0.0f,1.0f);
    glVertex3f(coord[0]-splat_negoff[0],
               coord[1]-splat_negoff[1],
               coord[2]-splat_negoff[2]);

    glTexCoord2f(1.0f,0.0f);
    glVertex3f(coord[0]+splat_negoff[0],
               coord[1]+splat_negoff[1],
               coord[2]+splat_negoff[2]);

    glTexCoord2f(0.0f,0.0f);
    glVertex3f(coord[0]-splat_posoff[0],
               coord[1]-splat_posoff[1],
               coord[2]-splat_posoff[2]);

    glEnd();

    */


    return;
}


nsint InitializeSplat(void)
{
    nsint     datasize;
    nsuchar *data,*p;
    nsint i,j;
    nsfloat radius,xdist,ydist,dist;
    nsuchar value;



    /* allocate memory for data */
    datasize = SPLAT_RES*SPLAT_RES;
    data = ns_malloc(4*datasize);

	if( NULL == data )
		return 0;

    /* generate data */
    radius = (nsfloat)(SPLAT_RES/2);
    p= data;
    for(j=0;j<SPLAT_RES;j++)
    {
        ydist = j-radius;

        for(i=0;i<SPLAT_RES;i++)
        {
            xdist = i-radius;

            dist = (nsfloat)ns_sqrt((nsdouble)(xdist*xdist+ydist*ydist));
            if(dist>radius) value = 0;
            else value = (nsuchar) (128.0f+127.0f*(nsfloat)ns_cos(SPLAT_PI*dist/radius));


            p[0] = 255;
            p[1] = 255;
            p[2] = 255;

            //if(dist>0.4*SPLAT_RES) value = 255;

            p[3] = value;

            p+=4;
        }
    }

    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    /* get texture name and save to global */
    glGenTextures(1,&splat_tex_id);

    /* set texture parameters */
    glBindTexture(GL_TEXTURE_2D,splat_tex_id);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_MODULATE);

    /* bind texture data */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SPLAT_RES, SPLAT_RES,
                    0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, SPLAT_RES, SPLAT_RES,
    //                GL_RGBA, GL_UNSIGNED_BYTE, data);




    /* free allocated buffer */
    ns_free(data);

    return 0;
}


void SplatSetSize(nsfloat s)
{
    splat_size = s/2.0f;
}


void SplatBegin( void )
{
    nsfloat modelview[16];
    nsfloat right[3];
    nsfloat up[3];


	glDepthMask( GL_FALSE );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );

    if(splat_init==0)
    {
        InitializeSplat();
        splat_init=0;
    }

    // get the current modelview matrix
    glGetFloatv(GL_MODELVIEW_MATRIX , modelview);

    right[0] = modelview[0];
    right[1] = modelview[4];
    right[2] = modelview[8];
    up[0] = modelview[1];
    up[1] = modelview[5];
    up[2] = modelview[9];

    splat_negoff[0] = (right[0]-up[0])*splat_size;
    splat_negoff[1] = (right[1]-up[1])*splat_size;
    splat_negoff[2] = (right[2]-up[2])*splat_size;

    splat_posoff[0] = (right[0]+up[0])*splat_size;
    splat_posoff[1] = (right[1]+up[1])*splat_size;
    splat_posoff[2] = (right[2]+up[2])*splat_size;


    // set environment for splat rendering
    glBindTexture(GL_TEXTURE_2D,splat_tex_id);
    glInterleavedArrays(GL_T2F_V3F,0,splat_array);
}


void SplatEnd( void )
	{
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );
	}
