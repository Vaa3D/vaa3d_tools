
#include <ns-gl.h>


///////////////////////////////////
// prototypes
///////////////////////////////////
void SquareBegin( void );
void SquareRender( GLfloat coord[3], GLfloat rgba[4] );; //cal this for each square to render
void SquareEnd( void );



///////////////////////////////////
// definitions
///////////////////////////////////

GLfloat square_size = 0.5f;



void SquareSetSize(nsfloat s)
{
    square_size = s/2.0f;
}



/* This array specifies the vertex data */
GLfloat square_array[12] = {
                            0.0f,0.0f,0.0f,
                            0.0f,0.0f,0.0f,
                            0.0f,0.0f,0.0f,
                            0.0f,0.0f,0.0f
                        };
GLfloat square_up[3];
GLfloat square_right[3];
GLfloat square_negoff[3];
GLfloat square_posoff[3];

void SquareBegin( void )
{
    GLfloat modelview[16];
    GLfloat right[3];
    GLfloat up[3];

    // get the current modelview matrix
    glGetFloatv(GL_MODELVIEW_MATRIX , modelview);

    right[0] = modelview[0];
    right[1] = modelview[4];
    right[2] = modelview[8];
    up[0] = modelview[1];
    up[1] = modelview[5];
    up[2] = modelview[9];

    square_negoff[0] = (right[0]-up[0])*square_size;
    square_negoff[1] = (right[1]-up[1])*square_size;
    square_negoff[2] = (right[2]-up[2])*square_size;

    square_posoff[0] = (right[0]+up[0])*square_size;
    square_posoff[1] = (right[1]+up[1])*square_size;
    square_posoff[2] = (right[2]+up[2])*square_size;


    // set environment for rendering of square
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, 0, square_array );

}


void SquareRender( GLfloat coord[3], GLfloat rgba[4] )
{
    // update vertext data
    square_array[0] = coord[0]+square_posoff[0];
    square_array[1] = coord[1]+square_posoff[1];
    square_array[2] = coord[2]+square_posoff[2];

    square_array[3] = coord[0]-square_negoff[0];
    square_array[4] = coord[1]-square_negoff[1];
    square_array[5] = coord[2]-square_negoff[2];

    square_array[6] = coord[0]+square_negoff[0];
    square_array[7] = coord[1]+square_negoff[1];
    square_array[8] = coord[2]+square_negoff[2];

    square_array[9] = coord[0]-square_posoff[0];
    square_array[10] = coord[1]-square_posoff[1];
    square_array[11] = coord[2]-square_posoff[2];


    glColor4fv( rgba );
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    return;
}

void SquareEnd( void )
{
	glDisableClientState( GL_VERTEX_ARRAY );
}
