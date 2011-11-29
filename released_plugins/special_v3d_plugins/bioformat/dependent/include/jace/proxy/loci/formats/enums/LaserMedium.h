#ifndef JACE_PROXY_LOCI_FORMATS_ENUMS_LASERMEDIUM_H
#define JACE_PROXY_LOCI_FORMATS_ENUMS_LASERMEDIUM_H

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
 * The Jace C++ proxy class for loci.formats.enums.LaserMedium.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class LaserMedium : public ::jace::proxy::java::lang::Enum, public virtual ::jace::proxy::loci::formats::enums::Enumeration
{
public: 

/**
 * values
 *
 */
static ::jace::JArray< ::jace::proxy::loci::formats::enums::LaserMedium > values();

/**
 * valueOf
 *
 */
static ::jace::proxy::loci::formats::enums::LaserMedium valueOf( ::jace::proxy::java::lang::String p0 );

/**
 * fromString
 *
 */
static ::jace::proxy::loci::formats::enums::LaserMedium fromString( ::jace::proxy::java::lang::String p0 );

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
 * public static final enum CU
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > CU();

/**
 * public static final enum AG
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > AG();

/**
 * public static final enum ARFL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > ARFL();

/**
 * public static final enum ARCL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > ARCL();

/**
 * public static final enum KRFL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > KRFL();

/**
 * public static final enum KRCL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > KRCL();

/**
 * public static final enum XEFL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > XEFL();

/**
 * public static final enum XECL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > XECL();

/**
 * public static final enum XEBR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > XEBR();

/**
 * public static final enum N
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > N();

/**
 * public static final enum AR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > AR();

/**
 * public static final enum KR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > KR();

/**
 * public static final enum XE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > XE();

/**
 * public static final enum HENE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > HENE();

/**
 * public static final enum HECD
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > HECD();

/**
 * public static final enum CO
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > CO();

/**
 * public static final enum CO2
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > CO2();

/**
 * public static final enum H2O
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > H2O();

/**
 * public static final enum HFL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > HFL();

/**
 * public static final enum NDGLASS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > NDGLASS();

/**
 * public static final enum NDYAG
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > NDYAG();

/**
 * public static final enum ERGLASS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > ERGLASS();

/**
 * public static final enum ERYAG
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > ERYAG();

/**
 * public static final enum HOYLF
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > HOYLF();

/**
 * public static final enum HOYAG
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > HOYAG();

/**
 * public static final enum RUBY
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > RUBY();

/**
 * public static final enum TISAPPHIRE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > TISAPPHIRE();

/**
 * public static final enum ALEXANDRITE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > ALEXANDRITE();

/**
 * public static final enum RHODAMINE6G
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > RHODAMINE6G();

/**
 * public static final enum COUMARINC30
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > COUMARINC30();

/**
 * public static final enum GAAS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > GAAS();

/**
 * public static final enum GAALAS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > GAALAS();

/**
 * public static final enum EMINUS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > EMINUS();

/**
 * public static final enum OTHER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium > OTHER();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
LaserMedium( jvalue value );
LaserMedium( jobject object );
LaserMedium( const LaserMedium& object );
LaserMedium( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, enums )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::enums::LaserMedium>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::enums::LaserMedium>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::enums::LaserMedium>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::enums::LaserMedium>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::enums::LaserMedium( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::enums::LaserMedium>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::enums::LaserMedium>& proxy ) : 
    ::jace::proxy::loci::formats::enums::LaserMedium( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::enums::LaserMedium( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::enums::LaserMedium( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::enums::LaserMedium>& object ) : 
    ::jace::proxy::loci::formats::enums::LaserMedium( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_ENUMS_LASERMEDIUM_H

