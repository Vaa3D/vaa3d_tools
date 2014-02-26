

#define BKBF_ATTEN  0.5f


void ClearWithGradient(nsfloat* rgb)
{
    nsint lighting,smooth,matrixmode;
    nsfloat currcolor[4];


    // save and setup states
    glGetIntegerv(GL_LIGHTING,&lighting);
    glGetIntegerv(GL_SHADE_MODEL,&smooth);
    glGetFloatv(GL_CURRENT_COLOR,currcolor);
    glDisable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);


    // do transformations to ensure surface covers entire FOV
    glGetIntegerv(GL_MATRIX_MODE,&matrixmode);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // clear depth buffer (color buffer will be overwritten)
    glClear(GL_DEPTH_BUFFER_BIT);

    // draw two triangles
    glBegin(GL_TRIANGLE_STRIP);
    glColor4f(rgb[0]*BKBF_ATTEN,rgb[1]*BKBF_ATTEN,rgb[2]*BKBF_ATTEN,1.0f);
    glVertex3f(-1.0f,1.0f,-1.0f);
    glColor4f(rgb[0]*BKBF_ATTEN*BKBF_ATTEN,rgb[1]*BKBF_ATTEN*BKBF_ATTEN,rgb[2]*BKBF_ATTEN*BKBF_ATTEN,1.0f);
    glVertex3f(-1.0f,-1.0f,-1.0f);
    glColor4f(rgb[0],rgb[1],rgb[2],1.0f);
    glVertex3f(1.0f,1.0f,-1.0f);
    glColor4f(rgb[0]*BKBF_ATTEN,rgb[1]*BKBF_ATTEN,rgb[2]*BKBF_ATTEN,1.0f);
    glVertex3f(1.0f,-1.0f,-1.0f);
    glEnd();


    // clear the depth buffer
    glClear(GL_DEPTH_BUFFER_BIT);

    // restore original matrix;
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(matrixmode);


    // restore changed states
    if(lighting) glEnable(GL_LIGHTING);
    glShadeModel(smooth);
    glColor4f(currcolor[0],currcolor[1],currcolor[2],currcolor[3]);
	}


