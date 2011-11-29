#ifndef JACE_PROXY_JAVA_AWT_CONTAINER_H
#define JACE_PROXY_JAVA_AWT_CONTAINER_H

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
#ifndef JACE_PROXY_JAVA_AWT_COMPONENT_H
#include "jace/proxy/java/awt/Component.h"
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

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Object;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JFloat;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, awt )

/**
 * The Jace C++ proxy class for java.awt.Container.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Container : public ::jace::proxy::java::awt::Component
{
public: 

/**
 * Container
 *
 */
Container();

/**
 * getComponentCount
 *
 */
::jace::proxy::types::JInt getComponentCount();

/**
 * countComponents
 *
 */
::jace::proxy::types::JInt countComponents();

/**
 * getComponent
 *
 */
::jace::proxy::java::awt::Component getComponent( ::jace::proxy::types::JInt p0 );

/**
 * getComponents
 *
 */
::jace::JArray< ::jace::proxy::java::awt::Component > getComponents();

/**
 * add
 *
 */
::jace::proxy::java::awt::Component add( ::jace::proxy::java::awt::Component p0 );

/**
 * add
 *
 */
::jace::proxy::java::awt::Component add( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::awt::Component p1 );

/**
 * add
 *
 */
::jace::proxy::java::awt::Component add( ::jace::proxy::java::awt::Component p0, ::jace::proxy::types::JInt p1 );

/**
 * setComponentZOrder
 *
 */
void setComponentZOrder( ::jace::proxy::java::awt::Component p0, ::jace::proxy::types::JInt p1 );

/**
 * getComponentZOrder
 *
 */
::jace::proxy::types::JInt getComponentZOrder( ::jace::proxy::java::awt::Component p0 );

/**
 * add
 *
 */
void add( ::jace::proxy::java::awt::Component p0, ::jace::proxy::java::lang::Object p1 );

/**
 * add
 *
 */
void add( ::jace::proxy::java::awt::Component p0, ::jace::proxy::java::lang::Object p1, ::jace::proxy::types::JInt p2 );

/**
 * remove
 *
 */
void remove( ::jace::proxy::types::JInt p0 );

/**
 * remove
 *
 */
void remove( ::jace::proxy::java::awt::Component p0 );

/**
 * removeAll
 *
 */
void removeAll();

/**
 * doLayout
 *
 */
void doLayout();

/**
 * layout
 *
 */
void layout();

/**
 * invalidate
 *
 */
void invalidate();

/**
 * validate
 *
 */
void validate();

/**
 * getAlignmentX
 *
 */
::jace::proxy::types::JFloat getAlignmentX();

/**
 * getAlignmentY
 *
 */
::jace::proxy::types::JFloat getAlignmentY();

/**
 * getComponentAt
 *
 */
::jace::proxy::java::awt::Component getComponentAt( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * locate
 *
 */
::jace::proxy::java::awt::Component locate( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * findComponentAt
 *
 */
::jace::proxy::java::awt::Component findComponentAt( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * addNotify
 *
 */
void addNotify();

/**
 * removeNotify
 *
 */
void removeNotify();

/**
 * isAncestorOf
 *
 */
::jace::proxy::types::JBoolean isAncestorOf( ::jace::proxy::java::awt::Component p0 );

/**
 * areFocusTraversalKeysSet
 *
 */
::jace::proxy::types::JBoolean areFocusTraversalKeysSet( ::jace::proxy::types::JInt p0 );

/**
 * isFocusCycleRoot
 *
 */
::jace::proxy::types::JBoolean isFocusCycleRoot( ::jace::proxy::java::awt::Container p0 );

/**
 * transferFocusBackward
 *
 */
void transferFocusBackward();

/**
 * isFocusTraversalPolicySet
 *
 */
::jace::proxy::types::JBoolean isFocusTraversalPolicySet();

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
 * setFocusTraversalPolicyProvider
 *
 */
void setFocusTraversalPolicyProvider( ::jace::proxy::types::JBoolean p0 );

/**
 * isFocusTraversalPolicyProvider
 *
 */
::jace::proxy::types::JBoolean isFocusTraversalPolicyProvider();

/**
 * transferFocusDownCycle
 *
 */
void transferFocusDownCycle();

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
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
Container( jvalue value );
Container( jobject object );
Container( const Container& object );
Container( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, awt )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::awt::Container>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::awt::Container>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::awt::Container>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::awt::Container>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::awt::Container( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::awt::Container>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::awt::Container>& proxy ) : 
    ::jace::proxy::java::awt::Container( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::awt::Container>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::awt::Container>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::awt::Container>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::awt::Container>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::awt::Container>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::awt::Container( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::awt::Container>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::awt::Container( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::awt::Container>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::awt::Container>& object ) : 
    ::jace::proxy::java::awt::Container( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_AWT_CONTAINER_H

