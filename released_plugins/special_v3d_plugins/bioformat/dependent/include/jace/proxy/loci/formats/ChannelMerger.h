#ifndef JACE_PROXY_LOCI_FORMATS_CHANNELMERGER_H
#define JACE_PROXY_LOCI_FORMATS_CHANNELMERGER_H

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
#ifndef JACE_PROXY_LOCI_FORMATS_READERWRAPPER_H
#include "jace/proxy/loci/formats/ReaderWrapper.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_TYPES_JBYTE_H
#include "jace/proxy/types/JByte.h"
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

BEGIN_NAMESPACE_4( jace, proxy, java, io )
class IOException;
END_NAMESPACE_4( jace, proxy, java, io )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class FormatException;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )

/**
 * The Jace C++ proxy class for loci.formats.ChannelMerger.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class ChannelMerger : public ::jace::proxy::loci::formats::ReaderWrapper
{
public: 

/**
 * makeChannelMerger
 *
 */
static ::jace::proxy::loci::formats::ChannelMerger makeChannelMerger( ::jace::proxy::loci::formats::IFormatReader p0 );

/**
 * ChannelMerger
 *
 */
ChannelMerger();

/**
 * ChannelMerger
 *
 */
ChannelMerger( ::jace::proxy::loci::formats::IFormatReader p0 );

/**
 * canMerge
 *
 */
::jace::proxy::types::JBoolean canMerge();

/**
 * getOriginalIndex
 *
 */
::jace::proxy::types::JInt getOriginalIndex( ::jace::proxy::types::JInt p0 );

/**
 * getImageCount
 *
 */
::jace::proxy::types::JInt getImageCount();

/**
 * getDimensionOrder
 *
 */
::jace::proxy::java::lang::String getDimensionOrder();

/**
 * isRGB
 *
 */
::jace::proxy::types::JBoolean isRGB();

/**
 * isIndexed
 *
 */
::jace::proxy::types::JBoolean isIndexed();

/**
 * openBytes
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > openBytes( ::jace::proxy::types::JInt p0 );

/**
 * openBytes
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > openBytes( ::jace::proxy::types::JInt p0, ::jace::JArray< ::jace::proxy::types::JByte > p1 );

/**
 * openBytes
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > openBytes( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4 );

/**
 * openBytes
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > openBytes( ::jace::proxy::types::JInt p0, ::jace::JArray< ::jace::proxy::types::JByte > p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5 );

/**
 * getIndex
 *
 */
::jace::proxy::types::JInt getIndex( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * getZCTCoords
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getZCTCoords( ::jace::proxy::types::JInt p0 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
ChannelMerger( jvalue value );
ChannelMerger( jobject object );
ChannelMerger( const ChannelMerger& object );
ChannelMerger( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::ChannelMerger>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::ChannelMerger>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::ChannelMerger>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::ChannelMerger>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::ChannelMerger( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::ChannelMerger>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::ChannelMerger>& proxy ) : 
    ::jace::proxy::loci::formats::ChannelMerger( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::ChannelMerger>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::ChannelMerger>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::ChannelMerger>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::ChannelMerger>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::ChannelMerger>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::ChannelMerger( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::ChannelMerger>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::ChannelMerger( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::ChannelMerger>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::ChannelMerger>& object ) : 
    ::jace::proxy::loci::formats::ChannelMerger( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_CHANNELMERGER_H

