#ifndef JACE_PROXY_JAVA_AWT_IMAGE_DATABUFFER_H
#define JACE_PROXY_JAVA_AWT_IMAGE_DATABUFFER_H

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
BEGIN_NAMESPACE_3( jace, proxy, types )
class JDouble;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JFloat;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, java, awt, image )

/**
 * The Jace C++ proxy class for java.awt.image.DataBuffer.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class DataBuffer : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * getDataTypeSize
 *
 */
static ::jace::proxy::types::JInt getDataTypeSize( ::jace::proxy::types::JInt p0 );

/**
 * getDataType
 *
 */
::jace::proxy::types::JInt getDataType();

/**
 * getSize
 *
 */
::jace::proxy::types::JInt getSize();

/**
 * getOffset
 *
 */
::jace::proxy::types::JInt getOffset();

/**
 * getOffsets
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getOffsets();

/**
 * getNumBanks
 *
 */
::jace::proxy::types::JInt getNumBanks();

/**
 * getElem
 *
 */
::jace::proxy::types::JInt getElem( ::jace::proxy::types::JInt p0 );

/**
 * getElem
 *
 */
::jace::proxy::types::JInt getElem( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * setElem
 *
 */
void setElem( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * setElem
 *
 */
void setElem( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * getElemFloat
 *
 */
::jace::proxy::types::JFloat getElemFloat( ::jace::proxy::types::JInt p0 );

/**
 * getElemFloat
 *
 */
::jace::proxy::types::JFloat getElemFloat( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * setElemFloat
 *
 */
void setElemFloat( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JFloat p1 );

/**
 * setElemFloat
 *
 */
void setElemFloat( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JFloat p2 );

/**
 * getElemDouble
 *
 */
::jace::proxy::types::JDouble getElemDouble( ::jace::proxy::types::JInt p0 );

/**
 * getElemDouble
 *
 */
::jace::proxy::types::JDouble getElemDouble( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * setElemDouble
 *
 */
void setElemDouble( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JDouble p1 );

/**
 * setElemDouble
 *
 */
void setElemDouble( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JDouble p2 );

/**
 * public static final TYPE_BYTE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_BYTE();

/**
 * public static final TYPE_USHORT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_USHORT();

/**
 * public static final TYPE_SHORT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_SHORT();

/**
 * public static final TYPE_INT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_INT();

/**
 * public static final TYPE_FLOAT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_FLOAT();

/**
 * public static final TYPE_DOUBLE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_DOUBLE();

/**
 * public static final TYPE_UNDEFINED
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_UNDEFINED();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
DataBuffer( jvalue value );
DataBuffer( jobject object );
DataBuffer( const DataBuffer& object );
DataBuffer( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, java, awt, image )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::awt::image::DataBuffer>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::awt::image::DataBuffer>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::awt::image::DataBuffer>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::awt::image::DataBuffer>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::awt::image::DataBuffer( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::awt::image::DataBuffer>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::awt::image::DataBuffer>& proxy ) : 
    ::jace::proxy::java::awt::image::DataBuffer( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::awt::image::DataBuffer>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::awt::image::DataBuffer>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::awt::image::DataBuffer>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::awt::image::DataBuffer>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::awt::image::DataBuffer>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::awt::image::DataBuffer( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::awt::image::DataBuffer>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::awt::image::DataBuffer( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::awt::image::DataBuffer>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::awt::image::DataBuffer>& object ) : 
    ::jace::proxy::java::awt::image::DataBuffer( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_AWT_IMAGE_DATABUFFER_H

