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


#ifndef _VSVR_H_
#define _VSVR_H_

#if !defined(WIN32) || defined(__CYGWIN__)
#pragma interface
#endif // WIN32


#include <stdlib.h> // definition of NULL


//_____________________________________________________________________________
/** Very Simple Volume Rendering */
/** \class VSVR
  * \brief the Very Simple Volume Rendering containing a texture and transfer function.
  */
class VSVR
//-----------------------------------------------------------------------------
{
// Constructors
public :
  /**
   * Main and default constructor
   * \brief constructor
   * \param tex_ni width  of the 3D texture (must be a power of 2)
   * \param tex_nj depth  of the 3D texture (must be a power of 2)
   * \param tex_nk height of the 3D texture (must be a power of 2)
   * \param tf_size size of the transfer function
   */
  VSVR ( const int tex_ni = -1, const int tex_nj = -1, const int tex_nk = -1, const int tf_size = -1 ) :
    _tex_extern (false), _tex_ni(tex_ni), _tex_nj(tex_nj), _tex_nk(tex_nk), _tex((float*)NULL),
    _tf_extern  (false), _tf_size(tf_size), _tf((float*)NULL), _rescale(true), _tex_glid(0)  {}

  /** Destructor */
  ~VSVR() { tex_glunload() ;  tf_glunload() ;  tex_free() ;  tf_free() ; }


//-----------------------------------------------------------------------------
// Rendering
public :
  /**
   * loads the texture and renders
   * \param nslices number of slices cut into the texture
   * \param opacity factor to apply to the transfer function
   */
  bool gl_render    ( int nslices = 256 ) ;

  /**
   * redisplay with the current setting (possibly a different viewpoint)
   * \param nslices number of slices cut into the texture
   */
  bool gl_redisplay ( int nslices = 256 ) const ;

  /** accesses the texture name */
  int  tex_glid     () const { return _tex_glid ; }

protected :
  /** rescale the texels to match the transfer function size */
  void tex_rescale  () ;
  /** loads the 3D texture */
  bool tex_glload   () ;
  /**
   * loads the transfer function
   * \param opacity factor to apply to the transfer function
   */
  bool tf_glload    () const ;

  /** unloads the 3D texture */
  void tex_glunload () ;
  /** unloads the transfer function */
  void tf_glunload  () const ;

  /** sets the openGL attributes */
  void gl_set       () const ;
  /** unsets the openGL attributes */
  void gl_unset     () const ;

  /** sets the clipping planes (uses the 6 first) */
  void gl_clip      () const ;
  /** unsets the clipping planes */
  void gl_unclip    () const ;

//-----------------------------------------------------------------------------
// 3D texture accessors
public :
  /**  accesses the width  of the 3D texture */
  inline const int tex_ni() const { return _tex_ni ; }
  /**  accesses the depth  of the 3D texture */
  inline const int tex_nj() const { return _tex_nj ; }
  /**  accesses the height of the 3D texture */
  inline const int tex_nk() const { return _tex_nk ; }
  /**
   * changes the size of the 3D texture
   * \param tex_ni width  of the 3D texture (must be a power of 2)
   * \param tex_nj depth  of the 3D texture (must be a power of 2)
   * \param tex_nk height of the 3D texture (must be a power of 2)
   */
  inline void tex_set_resolution( const int tex_ni, const int tex_nj, const int tex_nk ) { _tex_ni = tex_ni ;  _tex_nj = tex_nj ;  _tex_nk = tex_nk ; }

  /**
   * selects to use a 3D texture allocated from another class
   * \param tex is the pointer to the external 3D texture, allocated as a tex_ni*tex_nj*tex_nk vector running in i first. Its values will be rescaled.
   */
  inline void tex_set_extern ( float *tex )  { tex_free() ;  _tex_extern = tex != NULL ;  _tex = tex ; }
  /**
   * selects to allocate the 3D texture
   */
  inline void tex_set_intern () { if( _tex_extern ) _tex = NULL ;  _tex_extern = false ; }


  /** allocates the 3D texture */
  inline void tex_alloc  () { tex_free() ;  int tex_size = _tex_ni*_tex_nj*_tex_nk ;  if( tex_size > 0 ) _tex = new float[tex_size] ;  _rescale = true ; }
  /** frees the 3D texture */
  inline void tex_free   () { if( !_tex_extern ) delete [] _tex ; _tex = NULL ; }


  /**
   * accesses a specific voxel of the 3D texture
   * \param i abscisse of the voxel
   * \param j ordinate of the voxel
   * \param k height   of the voxel
   */
  inline const float tex_get ( const int i, const int j, const int k ) const { return _tex[ i + j*_tex_ni + k*_tex_nj*_tex_ni ] ; }
  /**
   * sets the value of a specific voxel of the 3D texture
   * \param val new value for the voxel
   * \param i abscisse of the voxel
   * \param j ordinate of the voxel
   * \param k height   of the voxel
   */
  inline void        tex_set ( const int i, const int j, const int k, const float val ) { _tex[ i + j*_tex_ni + k*_tex_nj*_tex_ni ] = val ; }


//-----------------------------------------------------------------------------
//  Transfer function (color map) accessors
public :
  /**  accesses the size of the transfer function */
  inline const int tf_size() const { return _tf_size ; }
  /**
   * changes the size of the transfer function
   * \param tf_size size of the transfer function
   */
  inline void tf_set_size ( const int tf_size ) { if( _tf_size != tf_size ) _rescale = true ;  _tf_size = tf_size ; }

  /**
   * selects to use a transfer function  allocated from another class
   * \param tf is the pointer to the external data, allocated as a size_x*size_y*size_z vector running in x first
   */
  inline void tf_set_extern ( float *tf )  { tf_free() ;  _tf_extern = tf != NULL ;  _tf = tf ; }
  /**
   * selects to allocate the transfer function
   */
  inline void tf_set_intern () { if( _tf_extern ) _tf = NULL ;  _tf_extern = false ; }


  /** allocates the transfer function */
  inline void tf_alloc  () { tf_free() ;  if( _tf_size > 0 ) _tf = new float[4*_tf_size] ; }
  /** frees the transfer function */
  inline void tf_free   () { if( !_tf_extern ) delete [] _tf ; _tf = NULL ; }


  /**
   * accesses a specific element of the transfer function
   * \param i element index
   * \param r returned red   component of the color map
   * \param g returned green component of the color map
   * \param b returned blue  component of the color map
   * \param a returned transparency    of the color map
   */
  inline void tf_get ( const int i, float &r, float &g, float &b, float &a ) const { float *ptr = _tf + i ;  r = *ptr ;  ptr += tf_size() ;  g = *ptr ;  ptr += tf_size() ;  b = *ptr ;  ptr += tf_size() ;   a = *ptr ; }
  /**
   * sets a specific element of the transfer function
   * \param i element index
   * \param r red   component of the color map
   * \param g green component of the color map
   * \param b blue  component of the color map
   * \param a transparency    of the color map
   */
  inline void tf_set ( const int i, const float r, const float g, const float b, const float a ) { float *ptr = _tf + i ;  *ptr = r ;  ptr += tf_size() ;  *ptr = g ;  ptr += tf_size() ;  *ptr = b ;  ptr += tf_size() ;  *ptr = a ; }


//-----------------------------------------------------------------------------
// Elements
protected :
  bool      _tex_extern ;  /**< selects wether to allocate the 3D texture or to use one allocated from another class */
  int       _tex_ni     ;  /**< width  of the 3D texture (must be a power of 2) */
  int       _tex_nj     ;  /**< depth  of the 3D texture (must be a power of 2) */
  int       _tex_nk     ;  /**< height of the 3D texture (must be a power of 2) */
  float    *_tex        ;  /**< the 3D texture : grid of float values */

  bool      _tf_extern  ;  /**< selects wether to allocate the transfer function or to use one allocated from another class */
  int       _tf_size    ;  /**< size of the transfer function */
  float    *_tf         ;  /**< the transfer function : colormap with 4 floats (rgba) per color*/


private :
  bool         _rescale    ;  /**< needs to rescale */
  unsigned int _tex_glid   ;  /**< openGL texture name */
};
//_____________________________________________________________________________


#endif // _VSVR_H_
