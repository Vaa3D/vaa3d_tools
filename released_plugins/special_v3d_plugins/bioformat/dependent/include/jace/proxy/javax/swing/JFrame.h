#ifndef JACE_PROXY_JAVAX_SWING_JFRAME_H
#define JACE_PROXY_JAVAX_SWING_JFRAME_H

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
#ifndef JACE_PROXY_JAVA_AWT_FRAME_H
#include "jace/proxy/java/awt/Frame.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_JAVAX_SWING_WINDOWCONSTANTS_H
#include "jace/proxy/javax/swing/WindowConstants.h"
#endif

#ifndef JACE_PROXY_JAVAX_ACCESSIBILITY_ACCESSIBLE_H
#include "jace/proxy/javax/accessibility/Accessible.h"
#endif

#ifndef JACE_PROXY_JAVAX_SWING_ROOTPANECONTAINER_H
#include "jace/proxy/javax/swing/RootPaneContainer.h"
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

BEGIN_NAMESPACE_4( jace, proxy, java, awt )
class Component;
END_NAMESPACE_4( jace, proxy, java, awt )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, awt )
class Container;
END_NAMESPACE_4( jace, proxy, java, awt )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, javax, swing )

/**
 * The Jace C++ proxy class for javax.swing.JFrame.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class JFrame : public ::jace::proxy::java::awt::Frame, public virtual ::jace::proxy::javax::swing::WindowConstants, public virtual ::jace::proxy::javax::accessibility::Accessible, public virtual ::jace::proxy::javax::swing::RootPaneContainer
{
public: 

/**
 * JFrame
 *
 */
JFrame();

/**
 * JFrame
 *
 */
JFrame( ::jace::proxy::java::lang::String p0 );

/**
 * setDefaultCloseOperation
 *
 */
void setDefaultCloseOperation( ::jace::proxy::types::JInt p0 );

/**
 * getDefaultCloseOperation
 *
 */
::jace::proxy::types::JInt getDefaultCloseOperation();

/**
 * remove
 *
 */
void remove( ::jace::proxy::java::awt::Component p0 );

/**
 * getContentPane
 *
 */
::jace::proxy::java::awt::Container getContentPane();

/**
 * setContentPane
 *
 */
void setContentPane( ::jace::proxy::java::awt::Container p0 );

/**
 * getGlassPane
 *
 */
::jace::proxy::java::awt::Component getGlassPane();

/**
 * setGlassPane
 *
 */
void setGlassPane( ::jace::proxy::java::awt::Component p0 );

/**
 * setDefaultLookAndFeelDecorated
 *
 */
static void setDefaultLookAndFeelDecorated( ::jace::proxy::types::JBoolean p0 );

/**
 * isDefaultLookAndFeelDecorated
 *
 */
static ::jace::proxy::types::JBoolean isDefaultLookAndFeelDecorated();

/**
 * public static final EXIT_ON_CLOSE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > EXIT_ON_CLOSE();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
JFrame( jvalue value );
JFrame( jobject object );
JFrame( const JFrame& object );
JFrame( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, javax, swing )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::javax::swing::JFrame>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::javax::swing::JFrame>::ElementProxy( const jace::ElementProxy< ::jace::proxy::javax::swing::JFrame>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::javax::swing::JFrame>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::javax::swing::JFrame( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::javax::swing::JFrame>::ElementProxy( const jace::ElementProxy< ::jace::proxy::javax::swing::JFrame>& proxy ) : 
    ::jace::proxy::javax::swing::JFrame( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::javax::swing::JFrame>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::javax::swing::JFrame>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::javax::swing::JFrame>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::javax::swing::JFrame>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::JFrame>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::javax::swing::JFrame( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::JFrame>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::javax::swing::JFrame( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::JFrame>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::javax::swing::JFrame>& object ) : 
    ::jace::proxy::javax::swing::JFrame( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVAX_SWING_JFRAME_H

