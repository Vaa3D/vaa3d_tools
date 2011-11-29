#ifndef JACE_PROXY_LOCI_FORMATS_IN_IMPROVISIONTIFFREADER_H
#define JACE_PROXY_LOCI_FORMATS_IN_IMPROVISIONTIFFREADER_H

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
#ifndef JACE_PROXY_LOCI_FORMATS_IN_BASETIFFREADER_H
#include "jace/proxy/loci/formats/in/BaseTiffReader.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_PROXY_JAVA_LANG_STRING_H
#include "jace/proxy/java/lang/String.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_3( jace, proxy, types )
class JBoolean;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, io )
class IOException;
END_NAMESPACE_4( jace, proxy, java, io )

BEGIN_NAMESPACE_4( jace, proxy, loci, common )
class RandomAccessInputStream;
END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, in )

/**
 * The Jace C++ proxy class for loci.formats.in.ImprovisionTiffReader.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class ImprovisionTiffReader : public ::jace::proxy::loci::formats::in::BaseTiffReader
{
public: 

/**
 * ImprovisionTiffReader
 *
 */
ImprovisionTiffReader();

/**
 * isThisType
 *
 */
::jace::proxy::types::JBoolean isThisType( ::jace::proxy::loci::common::RandomAccessInputStream p0 );

/**
 * close
 *
 */
void close( ::jace::proxy::types::JBoolean p0 );

/**
 * public static final IMPROVISION_MAGIC_STRING
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > IMPROVISION_MAGIC_STRING();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
ImprovisionTiffReader( jvalue value );
ImprovisionTiffReader( jobject object );
ImprovisionTiffReader( const ImprovisionTiffReader& object );
ImprovisionTiffReader( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, in )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::in::ImprovisionTiffReader>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::in::ImprovisionTiffReader>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::in::ImprovisionTiffReader>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::in::ImprovisionTiffReader>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::in::ImprovisionTiffReader( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::in::ImprovisionTiffReader>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::in::ImprovisionTiffReader>& proxy ) : 
    ::jace::proxy::loci::formats::in::ImprovisionTiffReader( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::ImprovisionTiffReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::ImprovisionTiffReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::ImprovisionTiffReader>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::in::ImprovisionTiffReader>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::ImprovisionTiffReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::in::ImprovisionTiffReader( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::ImprovisionTiffReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::in::ImprovisionTiffReader( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::ImprovisionTiffReader>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::in::ImprovisionTiffReader>& object ) : 
    ::jace::proxy::loci::formats::in::ImprovisionTiffReader( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_IN_IMPROVISIONTIFFREADER_H

