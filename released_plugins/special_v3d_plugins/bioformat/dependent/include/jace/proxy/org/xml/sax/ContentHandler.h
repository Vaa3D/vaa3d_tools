#ifndef JACE_PROXY_ORG_XML_SAX_CONTENTHANDLER_H
#define JACE_PROXY_ORG_XML_SAX_CONTENTHANDLER_H

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
#ifndef JACE_TYPES_JCHAR_H
#include "jace/proxy/types/JChar.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class String;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, org, xml, sax )

/**
 * The Jace C++ proxy class for org.xml.sax.ContentHandler.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class ContentHandler : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * startDocument
 *
 */
void startDocument();

/**
 * endDocument
 *
 */
void endDocument();

/**
 * startPrefixMapping
 *
 */
void startPrefixMapping( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * endPrefixMapping
 *
 */
void endPrefixMapping( ::jace::proxy::java::lang::String p0 );

/**
 * endElement
 *
 */
void endElement( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1, ::jace::proxy::java::lang::String p2 );

/**
 * characters
 *
 */
void characters( ::jace::JArray< ::jace::proxy::types::JChar > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * ignorableWhitespace
 *
 */
void ignorableWhitespace( ::jace::JArray< ::jace::proxy::types::JChar > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * processingInstruction
 *
 */
void processingInstruction( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * skippedEntity
 *
 */
void skippedEntity( ::jace::proxy::java::lang::String p0 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
/**
 * Special no arg constructor for interface virtual inheritance
 *
 */
ContentHandler();
ContentHandler( jvalue value );
ContentHandler( jobject object );
ContentHandler( const ContentHandler& object );
ContentHandler( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, org, xml, sax )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::org::xml::sax::ContentHandler>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::org::xml::sax::ContentHandler>::ElementProxy( const jace::ElementProxy< ::jace::proxy::org::xml::sax::ContentHandler>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::org::xml::sax::ContentHandler>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::org::xml::sax::ContentHandler( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::org::xml::sax::ContentHandler>::ElementProxy( const jace::ElementProxy< ::jace::proxy::org::xml::sax::ContentHandler>& proxy ) : 
    ::jace::proxy::org::xml::sax::ContentHandler( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::org::xml::sax::ContentHandler>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::org::xml::sax::ContentHandler>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::org::xml::sax::ContentHandler>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::org::xml::sax::ContentHandler>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::org::xml::sax::ContentHandler>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::org::xml::sax::ContentHandler( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::org::xml::sax::ContentHandler>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::org::xml::sax::ContentHandler( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::org::xml::sax::ContentHandler>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::org::xml::sax::ContentHandler>& object ) : 
    ::jace::proxy::org::xml::sax::ContentHandler( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_ORG_XML_SAX_CONTENTHANDLER_H

