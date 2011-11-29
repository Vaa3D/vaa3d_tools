#ifndef JACE_PROXY_LOCI_FORMATS_ENUMS_CONTRASTMETHOD_H
#define JACE_PROXY_LOCI_FORMATS_ENUMS_CONTRASTMETHOD_H

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
#ifndef JACE_PROXY_JAVA_LANG_ENUM_H
#include "jace/proxy/java/lang/Enum.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_LOCI_FORMATS_ENUMS_ENUMERATION_H
#include "jace/proxy/loci/formats/enums/Enumeration.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class String;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, enums )
class EnumerationException;
END_NAMESPACE_5( jace, proxy, loci, formats, enums )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, enums )

/**
 * The Jace C++ proxy class for loci.formats.enums.ContrastMethod.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class ContrastMethod : public ::jace::proxy::java::lang::Enum, public virtual ::jace::proxy::loci::formats::enums::Enumeration
{
public: 

/**
 * values
 *
 */
static ::jace::JArray< ::jace::proxy::loci::formats::enums::ContrastMethod > values();

/**
 * valueOf
 *
 */
static ::jace::proxy::loci::formats::enums::ContrastMethod valueOf( ::jace::proxy::java::lang::String p0 );

/**
 * fromString
 *
 */
static ::jace::proxy::loci::formats::enums::ContrastMethod fromString( ::jace::proxy::java::lang::String p0 );

/**
 * getValue
 *
 */
::jace::proxy::java::lang::String getValue();

/**
 * toString
 *
 */
::jace::proxy::java::lang::String toString();

/**
 * public static final enum BRIGHTFIELD
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod > BRIGHTFIELD();

/**
 * public static final enum PHASE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod > PHASE();

/**
 * public static final enum DIC
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod > DIC();

/**
 * public static final enum HOFFMANMODULATION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod > HOFFMANMODULATION();

/**
 * public static final enum OBLIQUEILLUMINATION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod > OBLIQUEILLUMINATION();

/**
 * public static final enum POLARIZEDLIGHT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod > POLARIZEDLIGHT();

/**
 * public static final enum DARKFIELD
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod > DARKFIELD();

/**
 * public static final enum FLUORESCENCE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod > FLUORESCENCE();

/**
 * public static final enum OTHER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod > OTHER();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
ContrastMethod( jvalue value );
ContrastMethod( jobject object );
ContrastMethod( const ContrastMethod& object );
ContrastMethod( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, enums )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::enums::ContrastMethod>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::enums::ContrastMethod>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::enums::ContrastMethod>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::enums::ContrastMethod>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::enums::ContrastMethod( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::enums::ContrastMethod>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::enums::ContrastMethod>& proxy ) : 
    ::jace::proxy::loci::formats::enums::ContrastMethod( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::enums::ContrastMethod( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::enums::ContrastMethod( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::ContrastMethod>& object ) : 
    ::jace::proxy::loci::formats::enums::ContrastMethod( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_ENUMS_CONTRASTMETHOD_H

