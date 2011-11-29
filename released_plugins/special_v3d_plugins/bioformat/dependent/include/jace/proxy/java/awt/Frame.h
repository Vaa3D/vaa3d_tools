#ifndef JACE_PROXY_JAVA_AWT_FRAME_H
#define JACE_PROXY_JAVA_AWT_FRAME_H

#ifndef JACE_OS_DEP_H
#include "jace/os_dep.h"
#endif

#ifndef JACE_NAMESPACE_H
#include "jace/namespace.h"
#endif

#ifndef JACE_JOBJECT_H
#include "jace/proxy/JObject.h"
#endif

#ifndef JACE_JARRAY_H
#include "jace/JArray.h"
#endif

#ifndef JACE_JFIELD_PROXY_H
#include "jace/JFieldProxy.h"
#endif

#ifndef JACE_JCLASSIMPL_H
#include "jace/JClassImpl.h"
#endif

/**
 * The super class for this class.
 *
 */
#ifndef JACE_PROXY_JAVA_AWT_WINDOW_H
#include "jace/proxy/java/awt/Window.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_JAVA_AWT_MENUCONTAINER_H
#include "jace/proxy/java/awt/MenuContainer.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_TYPES_JINT_H
#include "jace/proxy/types/JInt.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class String;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JBoolean;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, awt )

/**
 * The Jace C++ proxy class for java.awt.Frame.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Frame : public ::jace::proxy::java::awt::Window, public virtual ::jace::proxy::java::awt::MenuContainer
{
public: 

/**
 * Frame
 *
 */
Frame();

/**
 * Frame
 *
 */
Frame( ::jace::proxy::java::lang::String p0 );

/**
 * addNotify
 *
 */
void addNotify();

/**
 * getTitle
 *
 */
::jace::proxy::java::lang::String getTitle();

/**
 * setTitle
 *
 */
void setTitle( ::jace::proxy::java::lang::String p0 );

/**
 * isResizable
 *
 */
::jace::proxy::types::JBoolean isResizable();

/**
 * setResizable
 *
 */
void setResizable( ::jace::proxy::types::JBoolean p0 );

/**
 * setState
 *
 */
void setState( ::jace::proxy::types::JInt p0 );

/**
 * setExtendedState
 *
 */
void setExtendedState( ::jace::proxy::types::JInt p0 );

/**
 * getState
 *
 */
::jace::proxy::types::JInt getState();

/**
 * getExtendedState
 *
 */
::jace::proxy::types::JInt getExtendedState();

/**
 * setUndecorated
 *
 */
void setUndecorated( ::jace::proxy::types::JBoolean p0 );

/**
 * isUndecorated
 *
 */
::jace::proxy::types::JBoolean isUndecorated();

/**
 * removeNotify
 *
 */
void removeNotify();

/**
 * setCursor
 *
 */
void setCursor( ::jace::proxy::types::JInt p0 );

/**
 * getCursorType
 *
 */
::jace::proxy::types::JInt getCursorType();

/**
 * getFrames
 *
 */
static ::jace::JArray< ::jace::proxy::java::awt::Frame > getFrames();

/**
 * public static final DEFAULT_CURSOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > DEFAULT_CURSOR();

/**
 * public static final CROSSHAIR_CURSOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CROSSHAIR_CURSOR();

/**
 * public static final TEXT_CURSOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TEXT_CURSOR();

/**
 * public static final WAIT_CURSOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > WAIT_CURSOR();

/**
 * public static final SW_RESIZE_CURSOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SW_RESIZE_CURSOR();

/**
 * public static final SE_RESIZE_CURSOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SE_RESIZE_CURSOR();

/**
 * public static final NW_RESIZE_CURSOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > NW_RESIZE_CURSOR();

/**
 * public static final NE_RESIZE_CURSOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > NE_RESIZE_CURSOR();

/**
 * public static final N_RESIZE_CURSOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > N_RESIZE_CURSOR();

/**
 * public static final S_RESIZE_CURSOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > S_RESIZE_CURSOR();

/**
 * public static final W_RESIZE_CURSOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > W_RESIZE_CURSOR();

/**
 * public static final E_RESIZE_CURSOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > E_RESIZE_CURSOR();

/**
 * public static final HAND_CURSOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > HAND_CURSOR();

/**
 * public static final MOVE_CURSOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MOVE_CURSOR();

/**
 * public static final NORMAL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JACE_NORMAL();

/**
 * public static final ICONIFIED
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > ICONIFIED();

/**
 * public static final MAXIMIZED_HORIZ
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MAXIMIZED_HORIZ();

/**
 * public static final MAXIMIZED_VERT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MAXIMIZED_VERT();

/**
 * public static final MAXIMIZED_BOTH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MAXIMIZED_BOTH();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
Frame( jvalue value );
Frame( jobject object );
Frame( const Frame& object );
Frame( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, awt )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::awt::Frame>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::awt::Frame>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::awt::Frame>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::awt::Frame>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::awt::Frame( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::awt::Frame>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::awt::Frame>& proxy ) : 
    ::jace::proxy::java::awt::Frame( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::awt::Frame>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::awt::Frame>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::awt::Frame>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::awt::Frame>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::awt::Frame>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::awt::Frame( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::awt::Frame>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::awt::Frame( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::awt::Frame>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::awt::Frame>& object ) : 
    ::jace::proxy::java::awt::Frame( object.getJavaJniValue() ), Object( NO_OP )
  {
    fieldID = object.fieldID; 

    if ( object.parent )
    {
      JNIEnv* env = ::jace::helper::attach();
      parent = ::jace::helper::newGlobalRef( env, object.parent );
    }
    else
      parent = 0;

    if ( object.parentClass )
    {
      JNIEnv* env = ::jace::helper::attach();
      parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, object.parentClass ) );
    }
    else
      parentClass = 0;
  }
#endif

END_NAMESPACE( jace )

#endif // #ifndef JACE_PROXY_JAVA_AWT_FRAME_H

