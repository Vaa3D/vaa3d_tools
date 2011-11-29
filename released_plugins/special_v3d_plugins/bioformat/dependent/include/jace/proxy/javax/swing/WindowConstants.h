#ifndef JACE_PROXY_JAVAX_SWING_WINDOWCONSTANTS_H
#define JACE_PROXY_JAVAX_SWING_WINDOWCONSTANTS_H

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
#ifndef JACE_PROXY_JAVA_LANG_OBJECT_H
#include "jace/proxy/java/lang/Object.h"
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
BEGIN_NAMESPACE_4( jace, proxy, javax, swing )

/**
 * The Jace C++ proxy class for javax.swing.WindowConstants.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class WindowConstants : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * public static final DO_NOTHING_ON_CLOSE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > DO_NOTHING_ON_CLOSE();

/**
 * public static final HIDE_ON_CLOSE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > HIDE_ON_CLOSE();

/**
 * public static final DISPOSE_ON_CLOSE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > DISPOSE_ON_CLOSE();

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
/**
 * Special no arg constructor for interface virtual inheritance
 *
 */
WindowConstants();
WindowConstants( jvalue value );
WindowConstants( jobject object );
WindowConstants( const WindowConstants& object );
WindowConstants( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, javax, swing )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::javax::swing::WindowConstants>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::javax::swing::WindowConstants>::ElementProxy( const jace::ElementProxy< ::jace::proxy::javax::swing::WindowConstants>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::javax::swing::WindowConstants>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::javax::swing::WindowConstants( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::javax::swing::WindowConstants>::ElementProxy( const jace::ElementProxy< ::jace::proxy::javax::swing::WindowConstants>& proxy ) : 
    ::jace::proxy::javax::swing::WindowConstants( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::javax::swing::WindowConstants>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::javax::swing::WindowConstants>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::javax::swing::WindowConstants>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::javax::swing::WindowConstants>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::WindowConstants>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::javax::swing::WindowConstants( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::WindowConstants>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::javax::swing::WindowConstants( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::WindowConstants>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::javax::swing::WindowConstants>& object ) : 
    ::jace::proxy::javax::swing::WindowConstants( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVAX_SWING_WINDOWCONSTANTS_H

