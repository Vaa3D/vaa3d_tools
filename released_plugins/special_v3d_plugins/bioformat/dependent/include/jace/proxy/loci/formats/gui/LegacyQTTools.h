#ifndef JACE_PROXY_LOCI_FORMATS_GUI_LEGACYQTTOOLS_H
#define JACE_PROXY_LOCI_FORMATS_GUI_LEGACYQTTOOLS_H

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

BEGIN_NAMESPACE_4( jace, proxy, loci, common )
class ReflectedUniverse;
END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE_4( jace, proxy, loci, common )
class ReflectException;
END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class MissingLibraryException;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class FormatException;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, gui )

/**
 * The Jace C++ proxy class for loci.formats.gui.LegacyQTTools.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class LegacyQTTools : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * LegacyQTTools
 *
 */
LegacyQTTools();

/**
 * canDoQT
 *
 */
::jace::proxy::types::JBoolean canDoQT();

/**
 * isMac64BitJVM
 *
 */
::jace::proxy::types::JBoolean isMac64BitJVM();

/**
 * isQTExpired
 *
 */
::jace::proxy::types::JBoolean isQTExpired();

/**
 * getQTVersion
 *
 */
::jace::proxy::java::lang::String getQTVersion();

/**
 * getUniverse
 *
 */
::jace::proxy::loci::common::ReflectedUniverse getUniverse();

/**
 * checkQTLibrary
 *
 */
void checkQTLibrary();

/**
 * public static final NO_QT_MSG
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > NO_QT_MSG();

/**
 * public static final MAC_64BIT_JVM_MSG
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > MAC_64BIT_JVM_MSG();

/**
 * public static final EXPIRED_QT_MSG
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > EXPIRED_QT_MSG();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
LegacyQTTools( jvalue value );
LegacyQTTools( jobject object );
LegacyQTTools( const LegacyQTTools& object );
LegacyQTTools( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, gui )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::gui::LegacyQTTools>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::gui::LegacyQTTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::gui::LegacyQTTools>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::gui::LegacyQTTools>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::gui::LegacyQTTools( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::gui::LegacyQTTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::gui::LegacyQTTools>& proxy ) : 
    ::jace::proxy::loci::formats::gui::LegacyQTTools( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::LegacyQTTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::LegacyQTTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::LegacyQTTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::gui::LegacyQTTools>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::LegacyQTTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::gui::LegacyQTTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::LegacyQTTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::gui::LegacyQTTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::LegacyQTTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::gui::LegacyQTTools>& object ) : 
    ::jace::proxy::loci::formats::gui::LegacyQTTools( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_GUI_LEGACYQTTOOLS_H

