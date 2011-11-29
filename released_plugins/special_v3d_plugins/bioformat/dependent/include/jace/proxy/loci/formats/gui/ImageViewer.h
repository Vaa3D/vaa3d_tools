#ifndef JACE_PROXY_LOCI_FORMATS_GUI_IMAGEVIEWER_H
#define JACE_PROXY_LOCI_FORMATS_GUI_IMAGEVIEWER_H

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
#ifndef JACE_PROXY_JAVAX_SWING_JFRAME_H
#include "jace/proxy/javax/swing/JFrame.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_JAVA_AWT_EVENT_ACTIONLISTENER_H
#include "jace/proxy/java/awt/event/ActionListener.h"
#endif

#ifndef JACE_PROXY_JAVAX_SWING_EVENT_CHANGELISTENER_H
#include "jace/proxy/javax/swing/event/ChangeListener.h"
#endif

#ifndef JACE_PROXY_JAVA_AWT_EVENT_KEYLISTENER_H
#include "jace/proxy/java/awt/event/KeyListener.h"
#endif

#ifndef JACE_PROXY_JAVA_AWT_EVENT_MOUSEMOTIONLISTENER_H
#include "jace/proxy/java/awt/event/MouseMotionListener.h"
#endif

#ifndef JACE_PROXY_JAVA_LANG_RUNNABLE_H
#include "jace/proxy/java/lang/Runnable.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_PROXY_JAVA_LANG_STRING_H
#include "jace/proxy/java/lang/String.h"
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

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, gui )

/**
 * The Jace C++ proxy class for loci.formats.gui.ImageViewer.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class ImageViewer : public ::jace::proxy::javax::swing::JFrame, public virtual ::jace::proxy::java::awt::event::ActionListener, public virtual ::jace::proxy::javax::swing::event::ChangeListener, public virtual ::jace::proxy::java::awt::event::KeyListener, public virtual ::jace::proxy::java::awt::event::MouseMotionListener, public virtual ::jace::proxy::java::lang::Runnable
{
public: 

/**
 * ImageViewer
 *
 */
ImageViewer();

/**
 * open
 *
 */
void open( ::jace::proxy::java::lang::String p0 );

/**
 * save
 *
 */
void save( ::jace::proxy::java::lang::String p0 );

/**
 * getImageIndex
 *
 */
::jace::proxy::types::JInt getImageIndex();

/**
 * getZ
 *
 */
::jace::proxy::types::JInt getZ();

/**
 * getT
 *
 */
::jace::proxy::types::JInt getT();

/**
 * getC
 *
 */
::jace::proxy::types::JInt getC();

/**
 * setVisible
 *
 */
void setVisible( ::jace::proxy::types::JBoolean p0 );

/**
 * run
 *
 */
void run();

/**
 * main
 *
 */
static void main( ::jace::JArray< ::jace::proxy::java::lang::String > p0 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
ImageViewer( jvalue value );
ImageViewer( jobject object );
ImageViewer( const ImageViewer& object );
ImageViewer( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, gui )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::gui::ImageViewer>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::gui::ImageViewer>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::gui::ImageViewer>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::gui::ImageViewer>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::gui::ImageViewer( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::gui::ImageViewer>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::gui::ImageViewer>& proxy ) : 
    ::jace::proxy::loci::formats::gui::ImageViewer( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::ImageViewer>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::ImageViewer>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::ImageViewer>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::gui::ImageViewer>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::ImageViewer>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::gui::ImageViewer( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::ImageViewer>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::gui::ImageViewer( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::ImageViewer>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::gui::ImageViewer>& object ) : 
    ::jace::proxy::loci::formats::gui::ImageViewer( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_GUI_IMAGEVIEWER_H

