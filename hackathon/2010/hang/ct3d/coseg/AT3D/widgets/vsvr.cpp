/**
 * @file    vsvr.h
 * @author  Thomas Lewiner <thomas.lewiner@polytechnique.org>
 * @author  Math Dept, PUC-Rio
 * @version 0.1
 * @date    30/05/2006
 *
 * @brief   Very Simple Volume Rendering
 */
//________________________________________________


#if !defined(WIN32) || defined(__CYGWIN__)
#pragma implementation
#endif // WIN32

#include <GL/glew.h> // openGL extension
#include <float.h>   // definition of FLT_EPSILON
#include <stdio.h>   // definition of printf
#include "vsvr.h"

#ifdef _DEBUG
#include <stdio.h>
#define PRINT_GL_DEBUG  { printf( "openGL watch at line %d: %s\n", __LINE__, ::gluErrorString( ::glGetError() ) ) ; }
#else  // _DEBUG
#define PRINT_GL_DEBUG  {}
#endif // _DEBUG


//_____________________________________________________________________________
// loads the texture and renders
bool VSVR::gl_render( int nslices /*= tex_ni()*/ )
//-----------------------------------------------------------------------------
{
  if( !_tex ) return false ;

  if( _rescale ) tex_rescale() ;

  if( !tf_glload() || !tex_glload() )
  {
    printf( "could not load texture!\n" ) ;
    return false ;
  }

  return gl_redisplay( nslices ) ;
}
//_____________________________________________________________________________




//_____________________________________________________________________________
// redisplay with the current setting (possibly a different viewpoint)
bool VSVR::gl_redisplay( int nslices /*= tex_ni()*/ ) const
//-----------------------------------------------------------------------------
{
  if( !_tex) return false ;

  // sets the openGL attributes and clipping planes
  gl_set () ;
  gl_clip() ;

  //--------------------------------------------------//
  // gets the direction of the observer
  double  gl_model[16] ; // = { 1.0f,0.0f,0.0f,0.0f, 0.0f,0.0f,-1.0f,0.0f, 0.0f,-1.0f,0.0f,0.0f, 0.0f,0.0f,0.0f,1.0f } ;
  double  gl_proj [16] ; // = { 1.0f,0.0f,0.0f,0.0f, 0.0f,1.0f,0.0f,0.0f, 0.0f,0.0f,1.0f,0.0f, 0.0f,0.0f,0.0f,1.0f } ;
  int     gl_view [ 4] ;
  ::glGetDoublev (GL_MODELVIEW_MATRIX , gl_model);
  ::glGetDoublev (GL_PROJECTION_MATRIX, gl_proj );
  ::glGetIntegerv(GL_VIEWPORT         , gl_view );



  //--------------------------------------------------//
  // gets the bounding box of the grid in the screen coordinates
  double xmin=FLT_MAX, xmax=-FLT_MAX, ymin=FLT_MAX, ymax=-FLT_MAX, zmin=FLT_MAX, zmax=-FLT_MAX;
  for( int i = 0; i < 8; ++i )
  {
    float bbx = (i&1) ? (float)tex_ni() : 0.0f ;
    float bby = (i&2) ? (float)tex_nj() : 0.0f ;
    float bbz = (i&4) ? (float)tex_nk() : 0.0f ;

    double x,y,z ;
    gluProject( bbx,bby,bbz, gl_model, gl_proj, gl_view, &x, &y, &z ) ;

    if( x < xmin ) xmin = x;
    if( x > xmax ) xmax = x;
    if( y < ymin ) ymin = y;
    if( y > ymax ) ymax = y;
    if( z < zmin ) zmin = z;
    if( z > zmax ) zmax = z;
  }


  //--------------------------------------------------//
  // world to tex coordinates
  double fx = 1.0 / tex_ni() ;
  double fy = 1.0 / tex_nj() ;
  double fz = 1.0 / tex_nk() ;

  //--------------------------------------------------//
  // draw each slice of the texture in the viewer coordinates
  float dz = (float)( (zmax-zmin) / nslices ) ;
  float z  = (float)zmax - dz/2.0f ;

  ::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  ::glBegin( GL_QUADS ) ;
  {
    for( int n = nslices-1 ; n >= 0 ; --n, z -= dz )
    {
      GLdouble point[3] ;
      ::gluUnProject( xmin,ymin,z, gl_model, gl_proj, gl_view, point + 0, point + 1, point + 2 ) ;
      ::glTexCoord3d( fx*point[0], fy*point[1], fz*point[2] );
      ::glVertex3dv( point ) ;

      ::gluUnProject( xmax,ymin,z, gl_model, gl_proj, gl_view, point + 0, point + 1, point + 2 ) ;
      ::glTexCoord3d( fx*point[0], fy*point[1], fz*point[2] );
      ::glVertex3dv( point ) ;

      ::gluUnProject( xmax,ymax,z, gl_model, gl_proj, gl_view, point + 0, point + 1, point + 2 ) ;
      ::glTexCoord3d( fx*point[0], fy*point[1], fz*point[2] );
      ::glVertex3dv( point ) ;

      ::gluUnProject( xmin,ymax,z, gl_model, gl_proj, gl_view, point + 0, point + 1, point + 2 ) ;
      ::glTexCoord3d( fx*point[0], fy*point[1], fz*point[2] );
      ::glVertex3dv( point ) ;
    }
  }
  ::glEnd() ; // GL_QUADS


  // unsets the openGL attributes and clipping planes
  gl_unclip() ;
  gl_unset () ;

  return true ;
}
//_____________________________________________________________________________




//_____________________________________________________________________________
//_____________________________________________________________________________




//_____________________________________________________________________________
// rescale the texels to match the transfer function size
void VSVR::tex_rescale()
//-----------------------------------------------------------------------------
{
  // gets the maximal values
  float tex_min = FLT_MAX,  tex_max = -FLT_MAX ;
  int n = tex_ni()*tex_nj()*tex_nk() ;
  float *ptr = _tex ;
  for( int i = 0 ; i < n ; ++i, ++ptr )
  {
    float tex = *ptr ;
    if( tex_min > tex ) tex_min = tex ;
    if( tex_max < tex ) tex_max = tex ;
  }

  // rescale the values
  float tex_fact = (float)tf_size() / (tex_max - tex_min) ;
  if( tex_fact < FLT_EPSILON || tex_fact > 1e5 ) return ;
  ptr = _tex ;
  for( int i = 0 ; i < n ; ++i, ++ptr )
  {
    *ptr = (*ptr-tex_min) * tex_fact ;
  }

  _rescale = false ;
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// loads the 3D texture
bool VSVR::tex_glload()
//-----------------------------------------------------------------------------
{
  tex_glunload() ;

  // enable 3D texture
  ::glEnable(GL_TEXTURE_3D);

  // init the 3D texture
  ::glGenTextures(1, &_tex_glid);
  ::glBindTexture(GL_TEXTURE_3D, _tex_glid );

  // texture environment setup ( GL_CLAMP_TO_EDGE avoids invalid mapping at the texture border )
  ::glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  ::glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  ::glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  ::glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  ::glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  ::glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  // load the texture image
  ::glTexImage3D(GL_TEXTURE_3D,  // target
    0,                                  // level
    GL_RGBA,                            // internal format
    (int) tex_ni(),                     // width
    (int) tex_nj(),                     // height
    (int) tex_nk(),                     // depth
    0,                                  // border
    GL_RGBA,                     // format
    GL_FLOAT,                           // type
    _tex            );                  // buffer

  ::glPixelTransferi(GL_MAP_COLOR, GL_FALSE);

  return ::glGetError() == GL_NO_ERROR ;
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// loads the transfer function
bool VSVR::tf_glload() const
//-----------------------------------------------------------------------------
{
	return true;
  ::glPixelTransferi(GL_MAP_COLOR  , GL_TRUE);

  float *ptr = _tf ;
  ::glPixelMapfv(GL_PIXEL_MAP_I_TO_R, tf_size(), ptr);  ptr += tf_size() ;
  ::glPixelMapfv(GL_PIXEL_MAP_I_TO_G, tf_size(), ptr);  ptr += tf_size() ;
  ::glPixelMapfv(GL_PIXEL_MAP_I_TO_B, tf_size(), ptr);  ptr += tf_size() ;
  ::glPixelMapfv(GL_PIXEL_MAP_I_TO_A, tf_size(), ptr);

  return ::glGetError() == GL_NO_ERROR ;
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// unloads the 3D texture
void VSVR::tex_glunload()
//-----------------------------------------------------------------------------
{
  ::glDeleteTextures( 1, &_tex_glid ) ;
  _tex_glid = 0 ;
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// unloads the transfer function
void VSVR::tf_glunload() const
//-----------------------------------------------------------------------------
{
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// sets the openGL attributes
void VSVR::gl_set() const
//-----------------------------------------------------------------------------
{
  // push the relevant parts of the OpenGL state
  ::glPushAttrib(GL_COLOR_BUFFER_BIT   |
                 GL_DEPTH_BUFFER_BIT   |
                 GL_ENABLE_BIT         |
                 GL_LIGHTING_BIT       |
                 GL_POLYGON_BIT        |
                 GL_TEXTURE_BIT);

  // openGL setup
  ::glEnable (GL_TEXTURE_3D);
  ::glDisable(GL_LIGHTING);
  ::glDisable(GL_CULL_FACE);
  ::glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


  // enable alpha blending
  ::glEnable   (GL_BLEND);
  ::glDepthMask(GL_FALSE);
#ifdef GL_EXT_blend_minmax
  ::glBlendEquationEXT(GL_FUNC_ADD_EXT);
#endif // GL_EXT_blend_minmax
  ::glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// unsets the openGL attributes
void VSVR::gl_unset() const
//-----------------------------------------------------------------------------
{
  ::glDisable (GL_TEXTURE_3D);
  ::glPopAttrib() ;
}
//_____________________________________________________________________________




//_____________________________________________________________________________
// sets the clipping planes (uses the 6 first)
void VSVR::gl_clip() const
//-----------------------------------------------------------------------------
{
  // clip the 6 faces of the cube
  GLdouble plane[4] ;
  plane[0] = +1. ;  plane[1] =  0. ;  plane[2] =  0. ;  plane[3] = FLT_EPSILON ;
  ::glEnable( GL_CLIP_PLANE0 ) ;
  ::glClipPlane( GL_CLIP_PLANE0, plane ) ;
  plane[0] = -1. ;  plane[1] =  0. ;  plane[2] =  0. ;  plane[3] = tex_ni() - FLT_EPSILON ;
  ::glEnable( GL_CLIP_PLANE1 ) ;
  ::glClipPlane( GL_CLIP_PLANE1, plane ) ;

  plane[0] =  0. ;  plane[1] = +1. ;  plane[2] =  0. ;  plane[3] = FLT_EPSILON ;
  ::glEnable( GL_CLIP_PLANE2 ) ;
  ::glClipPlane( GL_CLIP_PLANE2, plane ) ;

  plane[0] =  0. ;  plane[1] = -1. ;  plane[2] =  0. ;  plane[3] = tex_nj() + FLT_EPSILON ;
  ::glEnable( GL_CLIP_PLANE3 ) ;
  ::glClipPlane( GL_CLIP_PLANE3, plane ) ;

  plane[0] =  0. ;  plane[1] =  0. ;  plane[2] = +1. ;  plane[3] = FLT_EPSILON ;
  ::glEnable( GL_CLIP_PLANE4 ) ;
  ::glClipPlane( GL_CLIP_PLANE4, plane ) ;

  plane[0] =  0. ;  plane[1] =  0. ;  plane[2] = -1. ;  plane[3] = tex_nk() + FLT_EPSILON ;
  ::glEnable( GL_CLIP_PLANE5 ) ;
  ::glClipPlane( GL_CLIP_PLANE5, plane ) ;
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// unsets the clipping planes
void VSVR::gl_unclip() const
//-----------------------------------------------------------------------------
{
  // disable cube clip plane
  ::glDisable( GL_CLIP_PLANE0 ) ;
  ::glDisable( GL_CLIP_PLANE1 ) ;
  ::glDisable( GL_CLIP_PLANE2 ) ;
  ::glDisable( GL_CLIP_PLANE3 ) ;
  ::glDisable( GL_CLIP_PLANE4 ) ;
  ::glDisable( GL_CLIP_PLANE5 ) ;
}
//_____________________________________________________________________________
