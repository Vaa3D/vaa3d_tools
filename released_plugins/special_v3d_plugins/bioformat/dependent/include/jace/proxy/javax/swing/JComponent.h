#ifndef JACE_PROXY_JAVAX_SWING_JCOMPONENT_H
#define JACE_PROXY_JAVAX_SWING_JCOMPONENT_H

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
#ifndef JACE_PROXY_JAVA_AWT_CONTAINER_H
#include "jace/proxy/java/awt/Container.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_JAVA_IO_SERIALIZABLE_H
#include "jace/proxy/java/io/Serializable.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_PROXY_JAVA_UTIL_EVENTLISTENER_H
#include "jace/proxy/java/util/EventListener.h"
#endif

#ifndef JACE_PROXY_JAVA_LANG_STRING_H
#include "jace/proxy/java/lang/String.h"
#endif

#ifndef JACE_TYPES_JINT_H
#include "jace/proxy/types/JInt.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_5( jace, proxy, java, awt, event )
class ActionListener;
END_NAMESPACE_5( jace, proxy, java, awt, event )

BEGIN_NAMESPACE_4( jace, proxy, java, awt )
class Component;
END_NAMESPACE_4( jace, proxy, java, awt )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JFloat;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class String;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JBoolean;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Object;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JChar;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JLong;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, javax, swing )

/**
 * The Jace C++ proxy class for javax.swing.JComponent.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class JComponent : public ::jace::proxy::java::awt::Container, public virtual ::jace::proxy::java::io::Serializable
{
public: 

/**
 * setInheritsPopupMenu
 *
 */
void setInheritsPopupMenu( ::jace::proxy::types::JBoolean p0 );

/**
 * getInheritsPopupMenu
 *
 */
::jace::proxy::types::JBoolean getInheritsPopupMenu();

/**
 * JComponent
 *
 */
JComponent();

/**
 * updateUI
 *
 */
void updateUI();

/**
 * getUIClassID
 *
 */
::jace::proxy::java::lang::String getUIClassID();

/**
 * isPaintingTile
 *
 */
::jace::proxy::types::JBoolean isPaintingTile();

/**
 * isManagingFocus
 *
 */
::jace::proxy::types::JBoolean isManagingFocus();

/**
 * setNextFocusableComponent
 *
 */
void setNextFocusableComponent( ::jace::proxy::java::awt::Component p0 );

/**
 * getNextFocusableComponent
 *
 */
::jace::proxy::java::awt::Component getNextFocusableComponent();

/**
 * setRequestFocusEnabled
 *
 */
void setRequestFocusEnabled( ::jace::proxy::types::JBoolean p0 );

/**
 * isRequestFocusEnabled
 *
 */
::jace::proxy::types::JBoolean isRequestFocusEnabled();

/**
 * requestFocus
 *
 */
void requestFocus();

/**
 * requestFocus
 *
 */
::jace::proxy::types::JBoolean requestFocus( ::jace::proxy::types::JBoolean p0 );

/**
 * requestFocusInWindow
 *
 */
::jace::proxy::types::JBoolean requestFocusInWindow();

/**
 * grabFocus
 *
 */
void grabFocus();

/**
 * setVerifyInputWhenFocusTarget
 *
 */
void setVerifyInputWhenFocusTarget( ::jace::proxy::types::JBoolean p0 );

/**
 * getVerifyInputWhenFocusTarget
 *
 */
::jace::proxy::types::JBoolean getVerifyInputWhenFocusTarget();

/**
 * contains
 *
 */
::jace::proxy::types::JBoolean contains( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * getAlignmentY
 *
 */
::jace::proxy::types::JFloat getAlignmentY();

/**
 * setAlignmentY
 *
 */
void setAlignmentY( ::jace::proxy::types::JFloat p0 );

/**
 * getAlignmentX
 *
 */
::jace::proxy::types::JFloat getAlignmentX();

/**
 * setAlignmentX
 *
 */
void setAlignmentX( ::jace::proxy::types::JFloat p0 );

/**
 * setDebugGraphicsOptions
 *
 */
void setDebugGraphicsOptions( ::jace::proxy::types::JInt p0 );

/**
 * getDebugGraphicsOptions
 *
 */
::jace::proxy::types::JInt getDebugGraphicsOptions();

/**
 * resetKeyboardActions
 *
 */
void resetKeyboardActions();

/**
 * requestDefaultFocus
 *
 */
::jace::proxy::types::JBoolean requestDefaultFocus();

/**
 * setVisible
 *
 */
void setVisible( ::jace::proxy::types::JBoolean p0 );

/**
 * setEnabled
 *
 */
void setEnabled( ::jace::proxy::types::JBoolean p0 );

/**
 * setToolTipText
 *
 */
void setToolTipText( ::jace::proxy::java::lang::String p0 );

/**
 * getToolTipText
 *
 */
::jace::proxy::java::lang::String getToolTipText();

/**
 * setAutoscrolls
 *
 */
void setAutoscrolls( ::jace::proxy::types::JBoolean p0 );

/**
 * getAutoscrolls
 *
 */
::jace::proxy::types::JBoolean getAutoscrolls();

/**
 * enable
 *
 */
void enable();

/**
 * disable
 *
 */
void disable();

/**
 * getClientProperty
 *
 */
::jace::proxy::java::lang::Object getClientProperty( ::jace::proxy::java::lang::Object p0 );

/**
 * putClientProperty
 *
 */
void putClientProperty( ::jace::proxy::java::lang::Object p0, ::jace::proxy::java::lang::Object p1 );

/**
 * isLightweightComponent
 *
 */
static ::jace::proxy::types::JBoolean isLightweightComponent( ::jace::proxy::java::awt::Component p0 );

/**
 * reshape
 *
 */
void reshape( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * getX
 *
 */
::jace::proxy::types::JInt getX();

/**
 * getY
 *
 */
::jace::proxy::types::JInt getY();

/**
 * getWidth
 *
 */
::jace::proxy::types::JInt getWidth();

/**
 * getHeight
 *
 */
::jace::proxy::types::JInt getHeight();

/**
 * isOpaque
 *
 */
::jace::proxy::types::JBoolean isOpaque();

/**
 * setOpaque
 *
 */
void setOpaque( ::jace::proxy::types::JBoolean p0 );

/**
 * firePropertyChange
 *
 */
void firePropertyChange( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JBoolean p1, ::jace::proxy::types::JBoolean p2 );

/**
 * firePropertyChange
 *
 */
void firePropertyChange( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * firePropertyChange
 *
 */
void firePropertyChange( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JChar p1, ::jace::proxy::types::JChar p2 );

/**
 * getTopLevelAncestor
 *
 */
::jace::proxy::java::awt::Container getTopLevelAncestor();

/**
 * addNotify
 *
 */
void addNotify();

/**
 * removeNotify
 *
 */
void removeNotify();

/**
 * repaint
 *
 */
void repaint( ::jace::proxy::types::JLong p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4 );

/**
 * revalidate
 *
 */
void revalidate();

/**
 * isValidateRoot
 *
 */
::jace::proxy::types::JBoolean isValidateRoot();

/**
 * isOptimizedDrawingEnabled
 *
 */
::jace::proxy::types::JBoolean isOptimizedDrawingEnabled();

/**
 * paintImmediately
 *
 */
void paintImmediately( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setDoubleBuffered
 *
 */
void setDoubleBuffered( ::jace::proxy::types::JBoolean p0 );

/**
 * isDoubleBuffered
 *
 */
::jace::proxy::types::JBoolean isDoubleBuffered();

/**
 * public static final WHEN_FOCUSED
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > WHEN_FOCUSED();

/**
 * public static final WHEN_ANCESTOR_OF_FOCUSED_COMPONENT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > WHEN_ANCESTOR_OF_FOCUSED_COMPONENT();

/**
 * public static final WHEN_IN_FOCUSED_WINDOW
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > WHEN_IN_FOCUSED_WINDOW();

/**
 * public static final UNDEFINED_CONDITION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > UNDEFINED_CONDITION();

/**
 * public static final TOOL_TIP_TEXT_KEY
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > TOOL_TIP_TEXT_KEY();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
JComponent( jvalue value );
JComponent( jobject object );
JComponent( const JComponent& object );
JComponent( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, javax, swing )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::javax::swing::JComponent>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::javax::swing::JComponent>::ElementProxy( const jace::ElementProxy< ::jace::proxy::javax::swing::JComponent>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::javax::swing::JComponent>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::javax::swing::JComponent( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::javax::swing::JComponent>::ElementProxy( const jace::ElementProxy< ::jace::proxy::javax::swing::JComponent>& proxy ) : 
    ::jace::proxy::javax::swing::JComponent( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::javax::swing::JComponent>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::javax::swing::JComponent>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::javax::swing::JComponent>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::javax::swing::JComponent>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::JComponent>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::javax::swing::JComponent( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::JComponent>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::javax::swing::JComponent( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::JComponent>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::javax::swing::JComponent>& object ) : 
    ::jace::proxy::javax::swing::JComponent( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVAX_SWING_JCOMPONENT_H

