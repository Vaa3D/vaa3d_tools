#ifndef JACE_PROXY_JAVAX_SWING_SWINGCONSTANTS_H
#define JACE_PROXY_JAVAX_SWING_SWINGCONSTANTS_H

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
 * The Jace C++ proxy class for javax.swing.SwingConstants.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class SwingConstants : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * public static final CENTER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CENTER();

/**
 * public static final TOP
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TOP();

/**
 * public static final LEFT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > LEFT();

/**
 * public static final BOTTOM
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > BOTTOM();

/**
 * public static final RIGHT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > RIGHT();

/**
 * public static final NORTH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > NORTH();

/**
 * public static final NORTH_EAST
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > NORTH_EAST();

/**
 * public static final EAST
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > EAST();

/**
 * public static final SOUTH_EAST
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SOUTH_EAST();

/**
 * public static final SOUTH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SOUTH();

/**
 * public static final SOUTH_WEST
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SOUTH_WEST();

/**
 * public static final WEST
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > WEST();

/**
 * public static final NORTH_WEST
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > NORTH_WEST();

/**
 * public static final HORIZONTAL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > HORIZONTAL();

/**
 * public static final VERTICAL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > VERTICAL();

/**
 * public static final LEADING
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > LEADING();

/**
 * public static final TRAILING
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TRAILING();

/**
 * public static final NEXT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > NEXT();

/**
 * public static final PREVIOUS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > PREVIOUS();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
/**
 * Special no arg constructor for interface virtual inheritance
 *
 */
SwingConstants();
SwingConstants( jvalue value );
SwingConstants( jobject object );
SwingConstants( const SwingConstants& object );
SwingConstants( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, javax, swing )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::javax::swing::SwingConstants>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::javax::swing::SwingConstants>::ElementProxy( const jace::ElementProxy< ::jace::proxy::javax::swing::SwingConstants>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::javax::swing::SwingConstants>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::javax::swing::SwingConstants( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::javax::swing::SwingConstants>::ElementProxy( const jace::ElementProxy< ::jace::proxy::javax::swing::SwingConstants>& proxy ) : 
    ::jace::proxy::javax::swing::SwingConstants( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::javax::swing::SwingConstants>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::javax::swing::SwingConstants>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::javax::swing::SwingConstants>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::javax::swing::SwingConstants>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::SwingConstants>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::javax::swing::SwingConstants( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::SwingConstants>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::javax::swing::SwingConstants( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::SwingConstants>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::javax::swing::SwingConstants>& object ) : 
    ::jace::proxy::javax::swing::SwingConstants( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVAX_SWING_SWINGCONSTANTS_H

