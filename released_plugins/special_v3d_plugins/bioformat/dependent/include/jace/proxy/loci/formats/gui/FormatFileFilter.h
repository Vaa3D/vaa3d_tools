#ifndef JACE_PROXY_LOCI_FORMATS_GUI_FORMATFILEFILTER_H
#define JACE_PROXY_LOCI_FORMATS_GUI_FORMATFILEFILTER_H

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
#ifndef JACE_PROXY_JAVAX_SWING_FILECHOOSER_FILEFILTER_H
#include "jace/proxy/javax/swing/filechooser/FileFilter.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_JAVA_IO_FILEFILTER_H
#include "jace/proxy/java/io/FileFilter.h"
#endif

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

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class IFormatReader;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Object;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, gui )

/**
 * The Jace C++ proxy class for loci.formats.gui.FormatFileFilter.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class FormatFileFilter : public ::jace::proxy::javax::swing::filechooser::FileFilter, public virtual ::jace::proxy::java::io::FileFilter, public virtual ::jace::proxy::java::lang::Comparable
{
public: 

/**
 * FormatFileFilter
 *
 */
FormatFileFilter( ::jace::proxy::loci::formats::IFormatReader p0 );

/**
 * FormatFileFilter
 *
 */
FormatFileFilter( ::jace::proxy::loci::formats::IFormatReader p0, ::jace::proxy::types::JBoolean p1 );

/**
 * getReader
 *
 */
::jace::proxy::loci::formats::IFormatReader getReader();

/**
 * getDescription
 *
 */
::jace::proxy::java::lang::String getDescription();

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
FormatFileFilter( jvalue value );
FormatFileFilter( jobject object );
FormatFileFilter( const FormatFileFilter& object );
FormatFileFilter( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, gui )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::gui::FormatFileFilter>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::gui::FormatFileFilter>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::gui::FormatFileFilter>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::gui::FormatFileFilter>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::gui::FormatFileFilter( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::gui::FormatFileFilter>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::gui::FormatFileFilter>& proxy ) : 
    ::jace::proxy::loci::formats::gui::FormatFileFilter( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::FormatFileFilter>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::FormatFileFilter>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::FormatFileFilter>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::gui::FormatFileFilter>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::FormatFileFilter>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::gui::FormatFileFilter( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::FormatFileFilter>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::gui::FormatFileFilter( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::FormatFileFilter>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::gui::FormatFileFilter>& object ) : 
    ::jace::proxy::loci::formats::gui::FormatFileFilter( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_GUI_FORMATFILEFILTER_H

