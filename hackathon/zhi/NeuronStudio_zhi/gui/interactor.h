#ifndef INTERACTOR_H
#define INTERACTOR_H

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsmath.h>
#include <ns-gl.h>


#define INTERACTOR_MODE_ROTATE  1
#define INTERACTOR_MODE_MOVE    2
#define INTERACTOR_MODE_ZOOM    3
#define INTERACTOR_MODE_ROLL    4


typedef struct _Interactor
   {
   // new features
   nsfloat  imatrix[16];
   nsint    iwinxdim;
   nsint    iwinydim;
	nsint    ioldmode;

   // retained feautures
   nsint    imode;
   nsfloat  idtouch;
   nsfloat  iatouch;
   nsfloat  ixpos;
   nsfloat  iypos;
   nsfloat  izpos;
   nsint    iprevx;
   nsint    iprevy;
   nsint    interacting;

   // deprecated
   nsfloat  iorientation;
   nsfloat  itilt;

   }
   Interactor;


NS_IMPEXP void InitInteractor( Interactor *interactor );

NS_IMPEXP void SetInteractorTranslationalTouch( Interactor *interactor, nsfloat touch );
NS_IMPEXP void SetInteractorRotationalTouch( Interactor *interactor, nsfloat touch );

NS_IMPEXP nsint GetInteractorMode( Interactor *interactor );
NS_IMPEXP void SetInteractorMode( Interactor *interactor, nsint mode );

NS_IMPEXP void SetInteractorWindowSize( Interactor *interactor, nsint xdim, nsint ydim );

NS_IMPEXP void StartInteractor( Interactor *interactor, HWND hWnd, nsint x, nsint y );
NS_IMPEXP nsint UpdateInteractor( Interactor *interactor, nsint x, nsint y );
NS_IMPEXP void EndInteractor( Interactor *interactor );

NS_IMPEXP void InteractorViewTransformation( Interactor *interactor );

#endif/* INTERACTOR_H */
