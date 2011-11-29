#ifndef JACE_PROXY_LOCI_FORMATS_IN_PERKINELMERREADER_H
#define JACE_PROXY_LOCI_FORMATS_IN_PERKINELMERREADER_H

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
#ifndef JACE_PROXY_JAVA_LANG_STRING_H
#include "jace/proxy/java/lang/String.h"
#endif

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
 * The Jace C++ proxy class for loci.formats.in.PerkinElmerReader.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class PerkinElmerReader : public ::jace::proxy::loci::formats::FormatReader
{
public: 

/**
 * PerkinElmerReader
 *
 */
PerkinElmerReader();

/**
 * isSingleFile
 *
 */
::jace::proxy::types::JBoolean isSingleFile( ::jace::proxy::java::lang::String p0 );

/**
 * isThisType
 *
 */
::jace::proxy::types::JBoolean isThisType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JBoolean p1 );

/**
 * fileGroupOption
 *
 */
::jace::proxy::types::JInt fileGroupOption( ::jace::proxy::java::lang::String p0 );

/**
 * openBytes
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > openBytes( ::jace::proxy::types::JInt p0, ::jace::JArray< ::jace::proxy::types::JByte > p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5 );

/**
 * getSeriesUsedFiles
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > getSeriesUsedFiles( ::jace::proxy::types::JBoolean p0 );

/**
 * close
 *
 */
void close( ::jace::proxy::types::JBoolean p0 );

/**
 * public static final HTML_REGEX
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > HTML_REGEX();

/**
 * public static final DATE_FORMAT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > DATE_FORMAT();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
PerkinElmerReader( jvalue value );
PerkinElmerReader( jobject object );
PerkinElmerReader( const PerkinElmerReader& object );
PerkinElmerReader( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, in )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::in::PerkinElmerReader>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::in::PerkinElmerReader>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::in::PerkinElmerReader>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::in::PerkinElmerReader>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::in::PerkinElmerReader( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::in::PerkinElmerReader>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::in::PerkinElmerReader>& proxy ) : 
    ::jace::proxy::loci::formats::in::PerkinElmerReader( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::PerkinElmerReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::PerkinElmerReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::PerkinElmerReader>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::in::PerkinElmerReader>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::PerkinElmerReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::in::PerkinElmerReader( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::PerkinElmerReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::in::PerkinElmerReader( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::PerkinElmerReader>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::in::PerkinElmerReader>& object ) : 
    ::jace::proxy::loci::formats::in::PerkinElmerReader( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_IN_PERKINELMERREADER_H

