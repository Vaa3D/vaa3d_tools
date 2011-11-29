#ifndef JACE_PROXY_LOCI_COMMON_XMLTOOLS_H
#define JACE_PROXY_LOCI_COMMON_XMLTOOLS_H

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
#ifndef JACE_TYPES_JBYTE_H
#include "jace/proxy/types/JByte.h"
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

BEGIN_NAMESPACE_4( jace, proxy, java, io )
class IOException;
END_NAMESPACE_4( jace, proxy, java, io )

BEGIN_NAMESPACE_6( jace, proxy, org, xml, sax, helpers )
class DefaultHandler;
END_NAMESPACE_6( jace, proxy, org, xml, sax, helpers )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, io )
class InputStream;
END_NAMESPACE_4( jace, proxy, java, io )

BEGIN_NAMESPACE_4( jace, proxy, loci, common )
class RandomAccessInputStream;
END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE_4( jace, proxy, java, util )
class Hashtable;
END_NAMESPACE_4( jace, proxy, java, util )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, common )

/**
 * The Jace C++ proxy class for loci.common.XMLTools.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class XMLTools : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * sanitizeXML
 *
 */
static ::jace::proxy::java::lang::String sanitizeXML( ::jace::proxy::java::lang::String p0 );

/**
 * indentXML
 *
 */
static ::jace::proxy::java::lang::String indentXML( ::jace::proxy::java::lang::String p0 );

/**
 * indentXML
 *
 */
static ::jace::proxy::java::lang::String indentXML( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * indentXML
 *
 */
static ::jace::proxy::java::lang::String indentXML( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JBoolean p1 );

/**
 * indentXML
 *
 */
static ::jace::proxy::java::lang::String indentXML( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * parseXML
 *
 */
static ::jace::proxy::java::util::Hashtable parseXML( ::jace::proxy::java::lang::String p0 );

/**
 * parseXML
 *
 */
static void parseXML( ::jace::proxy::java::lang::String p0, ::jace::proxy::org::xml::sax::helpers::DefaultHandler p1 );

/**
 * parseXML
 *
 */
static void parseXML( ::jace::proxy::loci::common::RandomAccessInputStream p0, ::jace::proxy::org::xml::sax::helpers::DefaultHandler p1 );

/**
 * parseXML
 *
 */
static void parseXML( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::org::xml::sax::helpers::DefaultHandler p1 );

/**
 * validateXML
 *
 */
static ::jace::proxy::types::JBoolean validateXML( ::jace::proxy::java::lang::String p0 );

/**
 * validateXML
 *
 */
static ::jace::proxy::types::JBoolean validateXML( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
XMLTools( jvalue value );
XMLTools( jobject object );
XMLTools( const XMLTools& object );
XMLTools( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::common::XMLTools>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::common::XMLTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::common::XMLTools>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::common::XMLTools>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::common::XMLTools( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::common::XMLTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::common::XMLTools>& proxy ) : 
    ::jace::proxy::loci::common::XMLTools( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::common::XMLTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::common::XMLTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::common::XMLTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::common::XMLTools>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::common::XMLTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::common::XMLTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::common::XMLTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::common::XMLTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::common::XMLTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::common::XMLTools>& object ) : 
    ::jace::proxy::loci::common::XMLTools( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_COMMON_XMLTOOLS_H

