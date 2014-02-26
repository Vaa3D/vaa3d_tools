#ifndef __OGLWIN_H__
#define __OGLWIN_H__

#include <windows.h>
#include <process.h>
#ifndef _MT
#define _MT
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "app_lib.h"
#include <math/nsray.h>
#include <math/nsdragrect.h>
#include "interactor.h"


typedef struct _NsRoiControls
	{
	NsVector3f  C1, C2;
	nsfloat     d2a, d2b, d1a, d1b, d0a, d0b, w, h, l, sz;
	nsint       dim[3];
	nssize      max;
	NsAABBox3d  B[6];
	NsLine3d    L[6];
	nsushort    selected;
	}
	NsRoiControls;


#define NS_ROI_CONTROL_NEAR_BIT    0x001
#define NS_ROI_CONTROL_FAR_BIT     0x002
#define NS_ROI_CONTROL_TOP_BIT     0x004
#define NS_ROI_CONTROL_BOTTOM_BIT  0x008
#define NS_ROI_CONTROL_LEFT_BIT    0x010
#define NS_ROI_CONTROL_RIGHT_BIT   0x020

#define NS_ROI_CONTROL_ALL_BITS  0xffff

#define NS_ROI_CONTROL_NEAR_IDX    0
#define NS_ROI_CONTROL_FAR_IDX     1
#define NS_ROI_CONTROL_TOP_IDX     2
#define NS_ROI_CONTROL_BOTTOM_IDX  3
#define NS_ROI_CONTROL_LEFT_IDX    4
#define NS_ROI_CONTROL_RIGHT_IDX   5


#define NS_ROI_CONTROL_SET_COLOR( selected, bit )\
	if( ( nsboolean )( (selected) & (bit) ) )glColor3f( 1.0f, 1.0f, 1.0f );\
	else glColor3f( 0.75f, 0.75f, 0.75f )

#define NS_ROI_CONTROL_COMPLEXITY  16

#define NS_ROI_CONTROL_BASE_RADIUS_SCALAR  0.75f
#define NS_ROI_CONTROL_BASE_HEIGHT_SCALAR  0.5f

#define NS_ROI_CONTROL_TIP_RADIUS_SCALAR  1.25f
#define NS_ROI_CONTROL_TIP_HEIGHT_SCALAR  2.75f


/* Pass NS_ROI_CONTROL_ALL_BITS for 'which_to_init' to
	initialize all the various controls. */
NS_IMPEXP void ns_roi_controls_init
	(
	NsRoiControls      *rc,
	nsushort            which_to_init,
	const NsCubei      *roi,
	const NsImage      *volume,
	const NsVoxelInfo  *voxel_info,
	nsushort            selected
	);


extern nssize ____num_selected;

struct _OpenGLWindow;
typedef struct _OpenGLWindow  OpenGLWindow;

typedef void ( *OpenGLWindowCallback )( OpenGLWindow *const, void* );


typedef struct _OpenGLLight
   {
   GLenum   index;
   GLfloat  position[4];
   GLfloat  diffuse[4];
   GLfloat  ambient[4];
   GLfloat  specular[4];
   }
   OpenGLLight;


#define InitOpenGLLight(light,idx,px,py,pz,pw,dr,dg,db,da,ar,ag,ab,aa,sr,sg,sb,sa)\
   (light)->index=GL_LIGHT0+(idx);\
   (light)->position[0]=(px);\
   (light)->position[1]=(py);\
   (light)->position[2]=(pz);\
   (light)->position[3]=(pw);\
   (light)->diffuse[0]=(dr);\
   (light)->diffuse[1]=(dg);\
   (light)->diffuse[2]=(db);\
   (light)->diffuse[3]=(da);\
   (light)->ambient[0]=(ar);\
   (light)->ambient[1]=(ag);\
   (light)->ambient[2]=(ab);\
   (light)->ambient[3]=(aa);\
   (light)->specular[0]=(sr);\
   (light)->specular[1]=(sg);\
   (light)->specular[2]=(sb);\
   (light)->specular[3]=(sa)


void ApplyOpenGLLight( const OpenGLLight *const light );



typedef struct _OpenGLMaterial
   {
   GLfloat  diffuse[4];
   GLfloat  ambient[4];
   GLfloat  specular[4];
   GLfloat  shininess[1];
   }
   OpenGLMaterial;


#define InitOpenGLMaterial(mat,dr,dg,db,da,ar,ag,ab,aa,sr,sg,sb,sa,sh)\
   (mat)->diffuse[0]=(dr);\
   (mat)->diffuse[1]=(dg);\
   (mat)->diffuse[2]=(db);\
   (mat)->diffuse[3]=(da);\
   (mat)->ambient[0]=(ar);\
   (mat)->ambient[1]=(ag);\
   (mat)->ambient[2]=(ab);\
   (mat)->ambient[3]=(aa);\
   (mat)->specular[0]=(sr);\
   (mat)->specular[1]=(sg);\
   (mat)->specular[2]=(sb);\
   (mat)->specular[3]=(sa);\
   (mat)->shininess[0]=(sh)


void ApplyOpenGLMaterial( const OpenGLMaterial *const material );



typedef struct _OpenGLTexture /* Assumed to be 2D. */
   {
   GLuint   ID;
   GLint    level;
   GLsizei  width;
   GLsizei  height;
   GLenum   format;
   GLenum   type;
   GLint    channels;
   void    *pixels;
   }
   OpenGLTexture;


void ConstructOpenGLTexture( OpenGLTexture *const tex );

int CreateOpenGLTexture
   (
   OpenGLTexture *const  tex,
   const GLint           level,
   const GLsizei         width,
   const GLsizei         height,
   const GLint           channels,
   const GLenum          format,
   const GLenum          type,
   void                 *pixels
   );

void ApplyOpenGLTexture( const OpenGLTexture *const tex );

void DestructOpenGLTexture( OpenGLTexture *const tex );



struct _OpenGLWindow
   {
   HINSTANCE    hInstance;
   HWND         hOwner;
   HWND         hWnd;
   HDC          hDC;
   HGLRC        hGLRC;

   int          enabled;
   int          visible;

   int          x;            /* -1 means center it */
   int          y;            /* -1 means center it */
   int          width;        /* 256 by default */
   int          height;       /* 256 by default */

   char        *title;

   char        *menuName;     /* Resource name or NULL */
   char        *iconName;     /* Resource name or NULL */
   char        *cursorName;   /* Resource name or NULL */

   int          menuID;

   int          colorBits;    /* 32 by default */
   int          depthBits;    /* 16 by default */

   float        clearRed;     /* 0 by default */
   float        clearGreen;   /* 0 by default */
   float        clearBlue;    /* 0 by default */
   float        clearAlpha;   /* 1 by default */
   float        clearDepth;   /* 1 by default */

   float        fieldOfView;  /* 45 degrees by default */

   float        zNear;        /* 1 by default */
   float        zFar;         /* 1024 by default */

   int          isOrtho;      /* 0 by default */
   float        xOrthoLeft;   /* -1 by default */
   float        xOrthoRight;  /* 1 by default */
   float        yOrthoTop;    /* 1 by default */
   float        yOrthoBottom; /* -1 by default */
   float        zOrthoNear;   /* -1 by default */
   float        zOrthoFar;    /* 1 by default */

   float        xEye;         /* 0 by default */
   float        yEye;         /* 0 by default */
   float        zEye;         /* 0 by default */

   int          oldMouseX;
   int          oldMouseY;
   int          mouseX;
   int          mouseY;
   unsigned     mouseKeyFlags;

	const char  *text;

//   int          mouseMode;    /* None by default. See below. */

   Interactor   interactor;

   void        *userData;     /* NULL by default */

   /* All callbacks NULL by default. */

   OpenGLWindowCallback   menuSelection;
   OpenGLWindowCallback   viewTransform;
   OpenGLWindowCallback   renderFrame;
   //OpenGLWindowCallback   mouseMove;
   //OpenGLWindowCallback   mouseLButtonDown;
   //OpenGLWindowCallback   mouseLButtonUp;
   //OpenGLWindowCallback   mouseLButtonDblClk;
   //OpenGLWindowCallback   mouseRButtonDown;
   //OpenGLWindowCallback   mouseRButtonUp;
	OpenGLWindowCallback   preNcDestroy;
   OpenGLWindowCallback   postNcDestroy;
	OpenGLWindowCallback   postCreate;
	OpenGLWindowCallback   on_timer;

   OpenGLWindowCallback   keyMap[256];

   /* Display lists */

   GLuint  dlCylinder;
   GLuint  dlSphere;
	GLuint  dlHalfSphere;

   int  isCreating;
   int  isCreated;

	int is_lbutton_down;
	int is_mouse_dragging;
	int is_sizing_or_moving;
	int is_wheeling;

	GLuint textures[3];

	GLuint text_base;

	/* Internal stuff. */
	NsVector3i   seed;
	nsboolean    found;
	NsRay3d      ray;
	NsLine3d     line;
	NsDragRect   drag_rect;
	nsboolean    isDragging;
	nsboolean    hasCapture;
	HCURSOR      roi_cursors[ _ROI_NUM_CURSORS ];
	nsint        roi_curr_cursor;
	nsboolean    roi_did_change;
	NsVector2i   old_client;
	NsVector2i   new_client;
	NsVector3d   old_voxel;
	NsVector3d   new_voxel;
	nsint        closest_idx;
	nsuint       coord;
	nsint       *face;
	nsdouble     track;
	nsdouble     voxel_face_min;
	nsdouble     voxel_face_max;
	nsint        image_face_min;
	nsint        image_face_max;
   };


/* Values for 'mode' */

//enum
  // {
   //OGLWIN_MOUSE_MODE_NONE,        /* i.e. no mode */
   //OGLWIN_MOUSE_MODE_INTERACTOR   /* Rotation, translation, zooming */
   //};


/* Values for 'mouseKeyFlags' */

#define OGLWIN_MKF_CONTROL   MK_CONTROL
#define OGLWIN_MKF_LBUTTON   MK_LBUTTON
#define OGLWIN_MKF_RBUTTON   MK_RBUTTON
#define OGLWIN_MKF_SHIFT     MK_SHIFT


enum
   {
	OGLWIN_KEY_0 = '0',
	OGLWIN_KEY_1 = '1',
	OGLWIN_KEY_2 = '2',
	OGLWIN_KEY_3 = '3',
	OGLWIN_KEY_4 = '4',
	OGLWIN_KEY_5 = '5',
	OGLWIN_KEY_6 = '6',
	OGLWIN_KEY_7 = '7',
	OGLWIN_KEY_8 = '8',
	OGLWIN_KEY_9 = '9',
	OGLWIN_KEY_A = 'A',
	OGLWIN_KEY_B = 'B',
	OGLWIN_KEY_C = 'C',
	OGLWIN_KEY_D = 'D',
	OGLWIN_KEY_E = 'E',
	OGLWIN_KEY_F = 'F',
	OGLWIN_KEY_G = 'G',
	OGLWIN_KEY_H = 'H',
	OGLWIN_KEY_I = 'I',
	OGLWIN_KEY_J = 'J',
	OGLWIN_KEY_K = 'K',
	OGLWIN_KEY_L = 'L',
	OGLWIN_KEY_M = 'M',
	OGLWIN_KEY_N = 'N',
	OGLWIN_KEY_O = 'O',
	OGLWIN_KEY_P = 'P',
	OGLWIN_KEY_Q = 'Q',
	OGLWIN_KEY_R = 'R',
	OGLWIN_KEY_S = 'S',
	OGLWIN_KEY_T = 'T',
	OGLWIN_KEY_U = 'U',
	OGLWIN_KEY_V = 'V',
	OGLWIN_KEY_W = 'W',
	OGLWIN_KEY_X = 'X',
	OGLWIN_KEY_Y = 'Y',
	OGLWIN_KEY_Z = 'Z',

	OGLWIN_KEY_UP    = VK_UP,
	OGLWIN_KEY_DOWN  = VK_DOWN,
	OGLWIN_KEY_LEFT  = VK_LEFT,
	OGLWIN_KEY_RIGHT = VK_RIGHT,

	OGLWIN_KEY_F1  = VK_F1,
	OGLWIN_KEY_F2  = VK_F2,
	OGLWIN_KEY_F3  = VK_F3,
	OGLWIN_KEY_F4  = VK_F4,
	OGLWIN_KEY_F5  = VK_F5,
	OGLWIN_KEY_F6  = VK_F6,
	OGLWIN_KEY_F7  = VK_F7,
	OGLWIN_KEY_F8  = VK_F8,
	OGLWIN_KEY_F9  = VK_F9,
	OGLWIN_KEY_F10 = VK_F10,
	OGLWIN_KEY_F11 = VK_F11,
	OGLWIN_KEY_F12 = VK_F12
   };


void InitOpenGLWindow( OpenGLWindow *const ogl, HINSTANCE hInstance, HWND hOwner );


void open_gl_begin_timer( OpenGLWindow *ogl, nssize milliseconds );
void open_gl_end_timer( OpenGLWindow *ogl );

void open_gl_begin_timer_by_message( OpenGLWindow *ogl );
void open_gl_end_timer_by_message( OpenGLWindow *ogl );


/* Call InitOpenGLWindow() before creating the window! Returns 0 on failure. */

int CreateOpenGLWindow( OpenGLWindow *const ogl );

void DestroyOpenGLWindow( OpenGLWindow *const ogl );


void ShowOpenGLWindow( OpenGLWindow *const ogl, int visible );

void MaximizeOpenGLWindow( OpenGLWindow *const ogl );

void MinimizeOpenGLWindow( OpenGLWindow *const ogl );


void EnableOpenGLWindow( OpenGLWindow *const ogl, int enabled );


void RepaintOpenGLWindow( OpenGLWindow *const ogl );


void OpenGLWindowDrawText( OpenGLWindow *ogl, const char *s, float x, float y, float z );


void OpenGLWindowDrawCylinder( OpenGLWindow *const ogl, float radius1, float radius2, float height, int complexity );
void OpenGLWindowDrawCylinderWithCaps( OpenGLWindow *ogl, float r1, float r2, float h, int compl, int top, int bottom );


void OpenGLWindowDrawCone( OpenGLWindow *const ogl, float radius, float height, int complexity );
void OpenGLWindowDrawConeWithBase( OpenGLWindow *const ogl, float radius, float height, int complexity );


void OpenGLWindowDrawSphere( OpenGLWindow *const ogl, float radius, int complexity );

/* Circle sits in the XZ plane. */

void OpenGLWindowDrawCircle( OpenGLWindow *const ogl, float radius, int complexity );


/* NOTE: The complexity is only used in the first call and cannot be changed
   unless the destroy function is called.  */

/* Cylinder of radius 1, height 1, with the center of its base at 0,0,0. */

void OpenGLWindowCylinder( OpenGLWindow *const ogl, int complexity );

void DestroyOpenGLWindowCylinder( OpenGLWindow *const ogl );


/* Sphere of radius 1 centered around the origin. */

void OpenGLWindowSphere( OpenGLWindow *const ogl, int complexity );

void DestroyOpenGLWindowSphere( OpenGLWindow *const ogl );


/* Half-Sphere of radius 1 centered around the origin. */

void OpenGLWindowHalfSphere( OpenGLWindow *const ogl, int complexity );

void DestroyOpenGLWindowHalfSphere( OpenGLWindow *const ogl );


void OpenGLWindowPerspectiveProjection
   (
   OpenGLWindow *const  ogl,
   float                fieldOfView,
   float                aspectRatio,
   float                zNear,
   float                zFar
   );


//void SetOpenGLWindowMenuText( OpenGLWindow *const ogl, int menuID, const char *text );

void SetOpenGLWindowMenuChecked( OpenGLWindow *const ogl, int menuID, int checked );

/*
void SetOpenGLWindowClearColor
   ( 
   OpenGLWindow *const  ogl,
   float                red,
   float                green,
   float                blue,
   float                alpha
   ); 
*/


void _ogl_window_cancel_alt_drag( void );


#endif/* __OGLWIN_H__ */
