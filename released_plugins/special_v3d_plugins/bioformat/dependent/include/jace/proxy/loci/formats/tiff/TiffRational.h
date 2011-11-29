#ifndef JACE_PROXY_LOCI_FORMATS_TIFF_TIFFRATIONAL_H
#define JACE_PROXY_LOCI_FORMATS_TIFF_TIFFRATIONAL_H

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
#ifndef JACE_PROXY_JAVA_LANG_NUMBER_H
#include "jace/proxy/java/lang/Number.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_JAVA_LANG_COMPARABLE_H
#include "jace/proxy/java/lang/Comparable.h"
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

BEGIN_NAMESPACE_3( jace, proxy, types )
class JByte;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JDouble;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JShort;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JLong;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Object;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JFloat;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, tiff )

/**
 * The Jace C++ proxy class for loci.formats.tiff.TiffRational.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class TiffRational : public ::jace::proxy::java::lang::Number, public virtual ::jace::proxy::java::lang::Comparable
{
public: 

/**
 * TiffRational
 *
 */
TiffRational( ::jace::proxy::types::JLong p0, ::jace::proxy::types::JLong p1 );

/**
 * getNumerator
 *
 */
::jace::proxy::types::JLong getNumerator();

/**
 * getDenominator
 *
 */
::jace::proxy::types::JLong getDenominator();

/**
 * reduce
 *
 */
void reduce();

/**
 * byteValue
 *
 */
::jace::proxy::types::JByte byteValue();

/**
 * doubleValue
 *
 */
::jace::proxy::types::JDouble doubleValue();

/**
 * floatValue
 *
 */
::jace::proxy::types::JFloat floatValue();

/**
 * intValue
 *
 */
::jace::proxy::types::JInt intValue();

/**
 * longValue
 *
 */
::jace::proxy::types::JLong longValue();

/**
 * shortValue
 *
 */
::jace::proxy::types::JShort shortValue();

/**
 * equals
 *
 */
::jace::proxy::types::JBoolean equals( ::jace::proxy::java::lang::Object p0 );

/**
 * hashCode
 *
 */
::jace::proxy::types::JInt hashCode();

/**
 * toString
 *
 */
::jace::proxy::java::lang::String toString();

/**
 * compareTo
 *
 */
::jace::proxy::types::JInt compareTo( ::jace::proxy::java::lang::Object p0 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
TiffRational( jvalue value );
TiffRational( jobject object );
TiffRational( const TiffRational& object );
TiffRational( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, tiff )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::tiff::TiffRational>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::tiff::TiffRational>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::tiff::TiffRational>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::tiff::TiffRational>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::tiff::TiffRational( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::tiff::TiffRational>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::tiff::TiffRational>& proxy ) : 
    ::jace::proxy::loci::formats::tiff::TiffRational( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffRational>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffRational>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffRational>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffRational>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffRational>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::tiff::TiffRational( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffRational>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::tiff::TiffRational( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffRational>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffRational>& object ) : 
    ::jace::proxy::loci::formats::tiff::TiffRational( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_TIFF_TIFFRATIONAL_H

