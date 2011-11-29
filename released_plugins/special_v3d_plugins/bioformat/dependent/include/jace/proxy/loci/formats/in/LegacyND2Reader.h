#ifndef JACE_PROXY_LOCI_FORMATS_IN_LEGACYND2READER_H
#define JACE_PROXY_LOCI_FORMATS_IN_LEGACYND2READER_H

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
#ifndef JACE_PROXY_LOCI_FORMATS_FORMATREADER_H
#include "jace/proxy/loci/formats/FormatReader.h"
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

BEGIN_NAMESPACE_3( jace, proxy, types )
class JDouble;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class FormatException;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, in )

/**
 * The Jace C++ proxy class for loci.formats.in.LegacyND2Reader.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class LegacyND2Reader : public ::jace::proxy::loci::formats::FormatReader
{
public: 

/**
 * LegacyND2Reader
 *
 */
LegacyND2Reader();

/**
 * isThisType
 *
 */
::jace::proxy::types::JBoolean isThisType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JBoolean p1 );

/**
 * openBytes
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > openBytes( ::jace::proxy::types::JInt p0, ::jace::JArray< ::jace::proxy::types::JByte > p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5 );

/**
 * openFile
 *
 */
void openFile( ::jace::proxy::java::lang::String p0 );

/**
 * getNumSeries
 *
 */
::jace::proxy::types::JInt getNumSeries();

/**
 * getWidth
 *
 */
::jace::proxy::types::JInt getWidth( ::jace::proxy::types::JInt p0 );

/**
 * getHeight
 *
 */
::jace::proxy::types::JInt getHeight( ::jace::proxy::types::JInt p0 );

/**
 * getZSlices
 *
 */
::jace::proxy::types::JInt getZSlices( ::jace::proxy::types::JInt p0 );

/**
 * getTFrames
 *
 */
::jace::proxy::types::JInt getTFrames( ::jace::proxy::types::JInt p0 );

/**
 * getChannels
 *
 */
::jace::proxy::types::JInt getChannels( ::jace::proxy::types::JInt p0 );

/**
 * getBytesPerPixel
 *
 */
::jace::proxy::types::JInt getBytesPerPixel( ::jace::proxy::types::JInt p0 );

/**
 * getImage
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > getImage( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4 );

/**
 * getDX
 *
 */
::jace::proxy::types::JDouble getDX( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * getDY
 *
 */
::jace::proxy::types::JDouble getDY( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * getDZ
 *
 */
::jace::proxy::types::JDouble getDZ( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * getDT
 *
 */
::jace::proxy::types::JDouble getDT( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * getWavelength
 *
 */
::jace::proxy::types::JDouble getWavelength( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * getChannelName
 *
 */
::jace::proxy::java::lang::String getChannelName( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * getMagnification
 *
 */
::jace::proxy::types::JDouble getMagnification( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * getNA
 *
 */
::jace::proxy::types::JDouble getNA( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * getObjectiveName
 *
 */
::jace::proxy::java::lang::String getObjectiveName( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * getModality
 *
 */
::jace::proxy::types::JInt getModality( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
LegacyND2Reader( jvalue value );
LegacyND2Reader( jobject object );
LegacyND2Reader( const LegacyND2Reader& object );
LegacyND2Reader( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, in )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::in::LegacyND2Reader>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::in::LegacyND2Reader>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::in::LegacyND2Reader>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::in::LegacyND2Reader>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::in::LegacyND2Reader( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::in::LegacyND2Reader>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::in::LegacyND2Reader>& proxy ) : 
    ::jace::proxy::loci::formats::in::LegacyND2Reader( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::LegacyND2Reader>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::LegacyND2Reader>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::LegacyND2Reader>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::in::LegacyND2Reader>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::LegacyND2Reader>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::in::LegacyND2Reader( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::LegacyND2Reader>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::in::LegacyND2Reader( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::LegacyND2Reader>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::in::LegacyND2Reader>& object ) : 
    ::jace::proxy::loci::formats::in::LegacyND2Reader( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_IN_LEGACYND2READER_H

