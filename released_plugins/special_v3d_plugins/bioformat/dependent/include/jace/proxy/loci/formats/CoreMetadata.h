#ifndef JACE_PROXY_LOCI_FORMATS_COREMETADATA_H
#define JACE_PROXY_LOCI_FORMATS_COREMETADATA_H

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
#ifndef JACE_PROXY_JAVA_LANG_STRING_H
#include "jace/proxy/java/lang/String.h"
#endif

#ifndef JACE_TYPES_JBOOLEAN_H
#include "jace/proxy/types/JBoolean.h"
#endif

#ifndef JACE_PROXY_JAVA_UTIL_HASHTABLE_H
#include "jace/proxy/java/util/Hashtable.h"
#endif

#ifndef JACE_TYPES_JINT_H
#include "jace/proxy/types/JInt.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )

/**
 * The Jace C++ proxy class for loci.formats.CoreMetadata.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class CoreMetadata : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * CoreMetadata
 *
 */
CoreMetadata();

/**
 * public sizeX
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > sizeX();

/**
 * public sizeY
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > sizeY();

/**
 * public sizeZ
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > sizeZ();

/**
 * public sizeC
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > sizeC();

/**
 * public sizeT
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > sizeT();

/**
 * public thumbSizeX
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > thumbSizeX();

/**
 * public thumbSizeY
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > thumbSizeY();

/**
 * public pixelType
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > pixelType();

/**
 * public imageCount
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > imageCount();

/**
 * public dimensionOrder
 *
 */
::jace::JFieldProxy< ::jace::proxy::java::lang::String > dimensionOrder();

/**
 * public orderCertain
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JBoolean > orderCertain();

/**
 * public rgb
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JBoolean > rgb();

/**
 * public littleEndian
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JBoolean > littleEndian();

/**
 * public interleaved
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JBoolean > interleaved();

/**
 * public indexed
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JBoolean > indexed();

/**
 * public falseColor
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JBoolean > falseColor();

/**
 * public metadataComplete
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JBoolean > metadataComplete();

/**
 * public seriesMetadata
 *
 */
::jace::JFieldProxy< ::jace::proxy::java::util::Hashtable > seriesMetadata();

/**
 * public thumbnail
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JBoolean > thumbnail();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
CoreMetadata( jvalue value );
CoreMetadata( jobject object );
CoreMetadata( const CoreMetadata& object );
CoreMetadata( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::CoreMetadata>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::CoreMetadata>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::CoreMetadata>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::CoreMetadata>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::CoreMetadata( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::CoreMetadata>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::CoreMetadata>& proxy ) : 
    ::jace::proxy::loci::formats::CoreMetadata( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::CoreMetadata>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::CoreMetadata>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::CoreMetadata>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::CoreMetadata>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::CoreMetadata>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::CoreMetadata( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::CoreMetadata>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::CoreMetadata( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::CoreMetadata>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::CoreMetadata>& object ) : 
    ::jace::proxy::loci::formats::CoreMetadata( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_COREMETADATA_H

