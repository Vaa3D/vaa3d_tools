#ifndef JACE_PROXY_JAVA_AWT_WINDOW_H
#define JACE_PROXY_JAVA_AWT_WINDOW_H

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
#ifndef JACE_PROXY_JAVA_AWT_CONTAINER_H
#include "jace/proxy/java/awt/Container.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_JAVAX_ACCESSIBILITY_ACCESSIBLE_H
#include "jace/proxy/javax/accessibility/Accessible.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_PROXY_JAVA_UTIL_EVENTLISTENER_H
#include "jace/proxy/java/util/EventListener.h"
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

BEGIN_NAMESPACE_4( jace, proxy, java, beans )
class PropertyChangeListener;
END_NAMESPACE_4( jace, proxy, java, beans )

BEGIN_NAMESPACE_4( jace, proxy, java, awt )
class Component;
END_NAMESPACE_4( jace, proxy, java, awt )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, awt )
class Frame;
END_NAMESPACE_4( jace, proxy, java, awt )

BEGIN_NAMESPACE_4( jace, proxy, java, awt )

/**
 * The Jace C++ proxy class for java.awt.Window.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Window : public ::jace::proxy::java::awt::Container, public virtual ::jace::proxy::javax::accessibility::Accessible
{
public: 

/**
 * Window
 *
 */
Window( ::jace::proxy::java::awt::Frame p0 );

/**
 * Window
 *
 */
Window( ::jace::proxy::java::awt::Window p0, CopyConstructorSpecifier );

/**
 * addNotify
 *
 */
void addNotify();

/**
 * pack
 *
 */
void pack();

/**
 * show
 *
 */
void show();

/**
 * hide
 *
 */
void hide();

/**
 * dispose
 *
 */
void dispose();

/**
 * toFront
 *
 */
void toFront();

/**
 * toBack
 *
 */
void toBack();

/**
 * getWarningString
 *
 */
::jace::proxy::java::lang::String getWarningString();

/**
 * getOwner
 *
 */
::jace::proxy::java::awt::Window getOwner();

/**
 * getOwnedWindows
 *
 */
::jace::JArray< ::jace::proxy::java::awt::Window > getOwnedWindows();

/**
 * setAlwaysOnTop
 *
 */
void setAlwaysOnTop( ::jace::proxy::types::JBoolean p0 );

/**
 * isAlwaysOnTop
 *
 */
::jace::proxy::types::JBoolean isAlwaysOnTop();

/**
 * getFocusOwner
 *
 */
::jace::proxy::java::awt::Component getFocusOwner();

/**
 * getMostRecentFocusOwner
 *
 */
::jace::proxy::java::awt::Component getMostRecentFocusOwner();

/**
 * isActive
 *
 */
::jace::proxy::types::JBoolean isActive();

/**
 * isFocused
 *
 */
::jace::proxy::types::JBoolean isFocused();

/**
 * setFocusCycleRoot
 *
 */
void setFocusCycleRoot( ::jace::proxy::types::JBoolean p0 );

/**
 * isFocusCycleRoot
 *
 */
::jace::proxy::types::JBoolean isFocusCycleRoot();

/**
 * getFocusCycleRootAncestor
 *
 */
::jace::proxy::java::awt::Container getFocusCycleRootAncestor();

/**
 * isFocusableWindow
 *
 */
::jace::proxy::types::JBoolean isFocusableWindow();

/**
 * getFocusableWindowState
 *
 */
::jace::proxy::types::JBoolean getFocusableWindowState();

/**
 * setFocusableWindowState
 *
 */
void setFocusableWindowState( ::jace::proxy::types::JBoolean p0 );

/**
 * addPropertyChangeListener
 *
 */
void addPropertyChangeListener( ::jace::proxy::java::beans::PropertyChangeListener p0 );

/**
 * addPropertyChangeListener
 *
 */
void addPropertyChangeListener( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::beans::PropertyChangeListener p1 );

/**
 * isShowing
 *
 */
::jace::proxy::types::JBoolean isShowing();

/**
 * applyResourceBundle
 *
 */
void applyResourceBundle( ::jace::proxy::java::lang::String p0 );

/**
 * setLocationRelativeTo
 *
 */
void setLocationRelativeTo( ::jace::proxy::java::awt::Component p0 );

/**
 * createBufferStrategy
 *
 */
void createBufferStrategy( ::jace::proxy::types::JInt p0 );

/**
 * setLocationByPlatform
 *
 */
void setLocationByPlatform( ::jace::proxy::types::JBoolean p0 );

/**
 * isLocationByPlatform
 *
 */
::jace::proxy::types::JBoolean isLocationByPlatform();

/**
 * setBounds
 *
 */
void setBounds( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
Window( jvalue value );
Window( jobject object );
Window( const Window& object );
Window( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, awt )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::awt::Window>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::awt::Window>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::awt::Window>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::awt::Window>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::awt::Window( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::awt::Window>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::awt::Window>& proxy ) : 
    ::jace::proxy::java::awt::Window( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::awt::Window>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::awt::Window>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::awt::Window>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::awt::Window>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::awt::Window>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::awt::Window( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::awt::Window>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::awt::Window( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::awt::Window>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::awt::Window>& object ) : 
    ::jace::proxy::java::awt::Window( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_AWT_WINDOW_H

