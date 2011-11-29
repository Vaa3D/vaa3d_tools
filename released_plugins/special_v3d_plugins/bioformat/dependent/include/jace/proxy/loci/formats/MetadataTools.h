#ifndef JACE_PROXY_LOCI_FORMATS_METADATATOOLS_H
#define JACE_PROXY_LOCI_FORMATS_METADATATOOLS_H

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

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class IFormatReader;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_4( jace, proxy, java, util )
class Hashtable;
END_NAMESPACE_4( jace, proxy, java, util )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, meta )
class MetadataRetrieve;
END_NAMESPACE_5( jace, proxy, loci, formats, meta )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, meta )
class IMetadata;
END_NAMESPACE_5( jace, proxy, loci, formats, meta )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class FormatException;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, meta )
class MetadataStore;
END_NAMESPACE_5( jace, proxy, loci, formats, meta )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )

/**
 * The Jace C++ proxy class for loci.formats.MetadataTools.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class MetadataTools : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * getLatestVersion
 *
 */
static ::jace::proxy::java::lang::String getLatestVersion();

/**
 * createOMEXMLMetadata
 *
 */
static ::jace::proxy::loci::formats::meta::IMetadata createOMEXMLMetadata();

/**
 * createOMEXMLMetadata
 *
 */
static ::jace::proxy::loci::formats::meta::IMetadata createOMEXMLMetadata( ::jace::proxy::java::lang::String p0 );

/**
 * createOMEXMLMetadata
 *
 */
static ::jace::proxy::loci::formats::meta::IMetadata createOMEXMLMetadata( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * createOMEXMLRoot
 *
 */
static ::jace::proxy::java::lang::Object createOMEXMLRoot( ::jace::proxy::java::lang::String p0 );

/**
 * isOMEXMLMetadata
 *
 */
static ::jace::proxy::types::JBoolean isOMEXMLMetadata( ::jace::proxy::java::lang::Object p0 );

/**
 * isOMEXMLRoot
 *
 */
static ::jace::proxy::types::JBoolean isOMEXMLRoot( ::jace::proxy::java::lang::Object p0 );

/**
 * getOMEXMLVersion
 *
 */
static ::jace::proxy::java::lang::String getOMEXMLVersion( ::jace::proxy::java::lang::Object p0 );

/**
 * getOMEMetadata
 *
 */
static ::jace::proxy::loci::formats::meta::IMetadata getOMEMetadata( ::jace::proxy::loci::formats::meta::MetadataRetrieve p0 );

/**
 * getOriginalMetadata
 *
 */
static ::jace::proxy::java::util::Hashtable getOriginalMetadata( ::jace::proxy::loci::formats::meta::IMetadata p0 );

/**
 * getOMEXML
 *
 */
static ::jace::proxy::java::lang::String getOMEXML( ::jace::proxy::loci::formats::meta::MetadataRetrieve p0 );

/**
 * validateOMEXML
 *
 */
static ::jace::proxy::types::JBoolean validateOMEXML( ::jace::proxy::java::lang::String p0 );

/**
 * validateOMEXML
 *
 */
static ::jace::proxy::types::JBoolean validateOMEXML( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JBoolean p1 );

/**
 * populatePixels
 *
 */
static void populatePixels( ::jace::proxy::loci::formats::meta::MetadataStore p0, ::jace::proxy::loci::formats::IFormatReader p1 );

/**
 * populatePixels
 *
 */
static void populatePixels( ::jace::proxy::loci::formats::meta::MetadataStore p0, ::jace::proxy::loci::formats::IFormatReader p1, ::jace::proxy::types::JBoolean p2 );

/**
 * populatePixels
 *
 */
static void populatePixels( ::jace::proxy::loci::formats::meta::MetadataStore p0, ::jace::proxy::loci::formats::IFormatReader p1, ::jace::proxy::types::JBoolean p2, ::jace::proxy::types::JBoolean p3 );

/**
 * createLSID
 *
 */
static ::jace::proxy::java::lang::String createLSID( ::jace::proxy::java::lang::String p0, ::jace::JArray< ::jace::proxy::types::JInt > p1 );

/**
 * verifyMinimumPopulated
 *
 */
static void verifyMinimumPopulated( ::jace::proxy::loci::formats::meta::MetadataRetrieve p0 );

/**
 * verifyMinimumPopulated
 *
 */
static void verifyMinimumPopulated( ::jace::proxy::loci::formats::meta::MetadataRetrieve p0, ::jace::proxy::types::JInt p1 );

/**
 * setDefaultCreationDate
 *
 */
static void setDefaultCreationDate( ::jace::proxy::loci::formats::meta::MetadataStore p0, ::jace::proxy::java::lang::String p1, ::jace::proxy::types::JInt p2 );

/**
 * populateOriginalMetadata
 *
 */
static void populateOriginalMetadata( ::jace::proxy::java::lang::Object p0, ::jace::proxy::java::lang::String p1, ::jace::proxy::java::lang::String p2 );

/**
 * getOriginalMetadata
 *
 */
static ::jace::proxy::java::lang::String getOriginalMetadata( ::jace::proxy::java::lang::Object p0, ::jace::proxy::java::lang::String p1 );

/**
 * convertMetadata
 *
 */
static void convertMetadata( ::jace::proxy::java::lang::String p0, ::jace::proxy::loci::formats::meta::MetadataStore p1 );

/**
 * convertMetadata
 *
 */
static void convertMetadata( ::jace::proxy::loci::formats::meta::MetadataRetrieve p0, ::jace::proxy::loci::formats::meta::MetadataStore p1 );

/**
 * asStore
 *
 */
static ::jace::proxy::loci::formats::meta::MetadataStore asStore( ::jace::proxy::loci::formats::meta::MetadataRetrieve p0 );

/**
 * asRetrieve
 *
 */
static ::jace::proxy::loci::formats::meta::MetadataRetrieve asRetrieve( ::jace::proxy::loci::formats::meta::MetadataStore p0 );

/**
 * keys
 *
 */
static ::jace::JArray< ::jace::proxy::java::lang::String > keys( ::jace::proxy::java::util::Hashtable p0 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
MetadataTools( jvalue value );
MetadataTools( jobject object );
MetadataTools( const MetadataTools& object );
MetadataTools( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::MetadataTools>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::MetadataTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::MetadataTools>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::MetadataTools>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::MetadataTools( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::MetadataTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::MetadataTools>& proxy ) : 
    ::jace::proxy::loci::formats::MetadataTools( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::MetadataTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::MetadataTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::MetadataTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::MetadataTools>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::MetadataTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::MetadataTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::MetadataTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::MetadataTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::MetadataTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::MetadataTools>& object ) : 
    ::jace::proxy::loci::formats::MetadataTools( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_METADATATOOLS_H

