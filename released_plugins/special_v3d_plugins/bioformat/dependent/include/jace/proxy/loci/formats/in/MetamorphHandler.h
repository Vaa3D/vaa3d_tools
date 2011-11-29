#ifndef JACE_PROXY_LOCI_FORMATS_IN_METAMORPHHANDLER_H
#define JACE_PROXY_LOCI_FORMATS_IN_METAMORPHHANDLER_H

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
#ifndef JACE_PROXY_ORG_XML_SAX_HELPERS_DEFAULTHANDLER_H
#include "jace/proxy/org/xml/sax/helpers/DefaultHandler.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class String;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_4( jace, proxy, java, util )
class Hashtable;
END_NAMESPACE_4( jace, proxy, java, util )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JDouble;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, in )

/**
 * The Jace C++ proxy class for loci.formats.in.MetamorphHandler.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class MetamorphHandler : public ::jace::proxy::org::xml::sax::helpers::DefaultHandler
{
public: 

/**
 * MetamorphHandler
 *
 */
MetamorphHandler( ::jace::proxy::java::util::Hashtable p0 );

/**
 * getDate
 *
 */
::jace::proxy::java::lang::String getDate();

/**
 * getImageName
 *
 */
::jace::proxy::java::lang::String getImageName();

/**
 * getPixelSizeX
 *
 */
::jace::proxy::types::JDouble getPixelSizeX();

/**
 * getPixelSizeY
 *
 */
::jace::proxy::types::JDouble getPixelSizeY();

/**
 * getTemperature
 *
 */
::jace::proxy::types::JDouble getTemperature();

/**
 * getBinning
 *
 */
::jace::proxy::java::lang::String getBinning();

/**
 * getReadOutRate
 *
 */
::jace::proxy::types::JDouble getReadOutRate();

/**
 * getZoom
 *
 */
::jace::proxy::types::JDouble getZoom();

/**
 * getStagePositionX
 *
 */
::jace::proxy::types::JDouble getStagePositionX();

/**
 * getStagePositionY
 *
 */
::jace::proxy::types::JDouble getStagePositionY();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
MetamorphHandler( jvalue value );
MetamorphHandler( jobject object );
MetamorphHandler( const MetamorphHandler& object );
MetamorphHandler( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, in )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::in::MetamorphHandler>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::in::MetamorphHandler>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::in::MetamorphHandler>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::in::MetamorphHandler>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::in::MetamorphHandler( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::in::MetamorphHandler>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::in::MetamorphHandler>& proxy ) : 
    ::jace::proxy::loci::formats::in::MetamorphHandler( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::MetamorphHandler>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::MetamorphHandler>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::MetamorphHandler>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::in::MetamorphHandler>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::MetamorphHandler>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::in::MetamorphHandler( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::MetamorphHandler>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::in::MetamorphHandler( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::MetamorphHandler>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::in::MetamorphHandler>& object ) : 
    ::jace::proxy::loci::formats::in::MetamorphHandler( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_IN_METAMORPHHANDLER_H

