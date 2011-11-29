#ifndef JACE_PROXY_JAVA_AWT_COMPONENT_H
#define JACE_PROXY_JAVA_AWT_COMPONENT_H

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
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_JAVA_AWT_IMAGE_IMAGEOBSERVER_H
#include "jace/proxy/java/awt/image/ImageObserver.h"
#endif

#ifndef JACE_PROXY_JAVA_AWT_MENUCONTAINER_H
#include "jace/proxy/java/awt/MenuContainer.h"
#endif

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

#ifndef JACE_PROXY_JAVA_AWT_EVENT_MOUSEMOTIONLISTENER_H
#include "jace/proxy/java/awt/event/MouseMotionListener.h"
#endif

#ifndef JACE_PROXY_JAVA_BEANS_PROPERTYCHANGELISTENER_H
#include "jace/proxy/java/beans/PropertyChangeListener.h"
#endif

#ifndef JACE_PROXY_JAVA_AWT_EVENT_KEYLISTENER_H
#include "jace/proxy/java/awt/event/KeyListener.h"
#endif

#ifndef JACE_TYPES_JFLOAT_H
#include "jace/proxy/types/JFloat.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_5( jace, proxy, java, awt, event )
class MouseMotionListener;
END_NAMESPACE_5( jace, proxy, java, awt, event )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JShort;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, java, awt, image )
class ColorModel;
END_NAMESPACE_5( jace, proxy, java, awt, image )

BEGIN_NAMESPACE_4( jace, proxy, java, awt )
class Container;
END_NAMESPACE_4( jace, proxy, java, awt )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JChar;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, beans )
class PropertyChangeListener;
END_NAMESPACE_4( jace, proxy, java, beans )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JByte;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JDouble;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JFloat;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, java, awt, event )
class KeyListener;
END_NAMESPACE_5( jace, proxy, java, awt, event )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class String;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JBoolean;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JLong;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, awt )

/**
 * The Jace C++ proxy class for java.awt.Component.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Component : public virtual ::jace::proxy::java::lang::Object, public virtual ::jace::proxy::java::awt::image::ImageObserver, public virtual ::jace::proxy::java::awt::MenuContainer, public virtual ::jace::proxy::java::io::Serializable
{
public: 

/**
 * getName
 *
 */
::jace::proxy::java::lang::String getName();

/**
 * setName
 *
 */
void setName( ::jace::proxy::java::lang::String p0 );

/**
 * getParent
 *
 */
::jace::proxy::java::awt::Container getParent();

/**
 * getTreeLock
 *
 */
::jace::proxy::java::lang::Object getTreeLock();

/**
 * isValid
 *
 */
::jace::proxy::types::JBoolean isValid();

/**
 * isDisplayable
 *
 */
::jace::proxy::types::JBoolean isDisplayable();

/**
 * isVisible
 *
 */
::jace::proxy::types::JBoolean isVisible();

/**
 * isShowing
 *
 */
::jace::proxy::types::JBoolean isShowing();

/**
 * isEnabled
 *
 */
::jace::proxy::types::JBoolean isEnabled();

/**
 * setEnabled
 *
 */
void setEnabled( ::jace::proxy::types::JBoolean p0 );

/**
 * enable
 *
 */
void enable();

/**
 * enable
 *
 */
void enable( ::jace::proxy::types::JBoolean p0 );

/**
 * disable
 *
 */
void disable();

/**
 * isDoubleBuffered
 *
 */
::jace::proxy::types::JBoolean isDoubleBuffered();

/**
 * enableInputMethods
 *
 */
void enableInputMethods( ::jace::proxy::types::JBoolean p0 );

/**
 * setVisible
 *
 */
void setVisible( ::jace::proxy::types::JBoolean p0 );

/**
 * show
 *
 */
void show();

/**
 * show
 *
 */
void show( ::jace::proxy::types::JBoolean p0 );

/**
 * hide
 *
 */
void hide();

/**
 * isForegroundSet
 *
 */
::jace::proxy::types::JBoolean isForegroundSet();

/**
 * isBackgroundSet
 *
 */
::jace::proxy::types::JBoolean isBackgroundSet();

/**
 * isFontSet
 *
 */
::jace::proxy::types::JBoolean isFontSet();

/**
 * getColorModel
 *
 */
::jace::proxy::java::awt::image::ColorModel getColorModel();

/**
 * setLocation
 *
 */
void setLocation( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * move
 *
 */
void move( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * setSize
 *
 */
void setSize( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * resize
 *
 */
void resize( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * setBounds
 *
 */
void setBounds( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

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
 * isLightweight
 *
 */
::jace::proxy::types::JBoolean isLightweight();

/**
 * isPreferredSizeSet
 *
 */
::jace::proxy::types::JBoolean isPreferredSizeSet();

/**
 * isMinimumSizeSet
 *
 */
::jace::proxy::types::JBoolean isMinimumSizeSet();

/**
 * isMaximumSizeSet
 *
 */
::jace::proxy::types::JBoolean isMaximumSizeSet();

/**
 * getAlignmentX
 *
 */
::jace::proxy::types::JFloat getAlignmentX();

/**
 * getAlignmentY
 *
 */
::jace::proxy::types::JFloat getAlignmentY();

/**
 * doLayout
 *
 */
void doLayout();

/**
 * layout
 *
 */
void layout();

/**
 * validate
 *
 */
void validate();

/**
 * invalidate
 *
 */
void invalidate();

/**
 * isCursorSet
 *
 */
::jace::proxy::types::JBoolean isCursorSet();

/**
 * repaint
 *
 */
void repaint();

/**
 * repaint
 *
 */
void repaint( ::jace::proxy::types::JLong p0 );

/**
 * repaint
 *
 */
void repaint( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * repaint
 *
 */
void repaint( ::jace::proxy::types::JLong p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4 );

/**
 * setIgnoreRepaint
 *
 */
void setIgnoreRepaint( ::jace::proxy::types::JBoolean p0 );

/**
 * getIgnoreRepaint
 *
 */
::jace::proxy::types::JBoolean getIgnoreRepaint();

/**
 * contains
 *
 */
::jace::proxy::types::JBoolean contains( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * inside
 *
 */
::jace::proxy::types::JBoolean inside( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * getComponentAt
 *
 */
::jace::proxy::java::awt::Component getComponentAt( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * locate
 *
 */
::jace::proxy::java::awt::Component locate( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * addKeyListener
 *
 */
void addKeyListener( ::jace::proxy::java::awt::event::KeyListener p0 );

/**
 * removeKeyListener
 *
 */
void removeKeyListener( ::jace::proxy::java::awt::event::KeyListener p0 );

/**
 * getKeyListeners
 *
 */
::jace::JArray< ::jace::proxy::java::awt::event::KeyListener > getKeyListeners();

/**
 * addMouseMotionListener
 *
 */
void addMouseMotionListener( ::jace::proxy::java::awt::event::MouseMotionListener p0 );

/**
 * removeMouseMotionListener
 *
 */
void removeMouseMotionListener( ::jace::proxy::java::awt::event::MouseMotionListener p0 );

/**
 * getMouseMotionListeners
 *
 */
::jace::JArray< ::jace::proxy::java::awt::event::MouseMotionListener > getMouseMotionListeners();

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
 * isFocusTraversable
 *
 */
::jace::proxy::types::JBoolean isFocusTraversable();

/**
 * isFocusable
 *
 */
::jace::proxy::types::JBoolean isFocusable();

/**
 * setFocusable
 *
 */
void setFocusable( ::jace::proxy::types::JBoolean p0 );

/**
 * areFocusTraversalKeysSet
 *
 */
::jace::proxy::types::JBoolean areFocusTraversalKeysSet( ::jace::proxy::types::JInt p0 );

/**
 * setFocusTraversalKeysEnabled
 *
 */
void setFocusTraversalKeysEnabled( ::jace::proxy::types::JBoolean p0 );

/**
 * getFocusTraversalKeysEnabled
 *
 */
::jace::proxy::types::JBoolean getFocusTraversalKeysEnabled();

/**
 * requestFocus
 *
 */
void requestFocus();

/**
 * requestFocusInWindow
 *
 */
::jace::proxy::types::JBoolean requestFocusInWindow();

/**
 * transferFocus
 *
 */
void transferFocus();

/**
 * getFocusCycleRootAncestor
 *
 */
::jace::proxy::java::awt::Container getFocusCycleRootAncestor();

/**
 * isFocusCycleRoot
 *
 */
::jace::proxy::types::JBoolean isFocusCycleRoot( ::jace::proxy::java::awt::Container p0 );

/**
 * nextFocus
 *
 */
void nextFocus();

/**
 * transferFocusBackward
 *
 */
void transferFocusBackward();

/**
 * transferFocusUpCycle
 *
 */
void transferFocusUpCycle();

/**
 * hasFocus
 *
 */
::jace::proxy::types::JBoolean hasFocus();

/**
 * isFocusOwner
 *
 */
::jace::proxy::types::JBoolean isFocusOwner();

/**
 * toString
 *
 */
::jace::proxy::java::lang::String toString();

/**
 * list
 *
 */
void list();

/**
 * addPropertyChangeListener
 *
 */
void addPropertyChangeListener( ::jace::proxy::java::beans::PropertyChangeListener p0 );

/**
 * removePropertyChangeListener
 *
 */
void removePropertyChangeListener( ::jace::proxy::java::beans::PropertyChangeListener p0 );

/**
 * getPropertyChangeListeners
 *
 */
::jace::JArray< ::jace::proxy::java::beans::PropertyChangeListener > getPropertyChangeListeners();

/**
 * addPropertyChangeListener
 *
 */
void addPropertyChangeListener( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::beans::PropertyChangeListener p1 );

/**
 * removePropertyChangeListener
 *
 */
void removePropertyChangeListener( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::beans::PropertyChangeListener p1 );

/**
 * getPropertyChangeListeners
 *
 */
::jace::JArray< ::jace::proxy::java::beans::PropertyChangeListener > getPropertyChangeListeners( ::jace::proxy::java::lang::String p0 );

/**
 * firePropertyChange
 *
 */
void firePropertyChange( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JByte p1, ::jace::proxy::types::JByte p2 );

/**
 * firePropertyChange
 *
 */
void firePropertyChange( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JChar p1, ::jace::proxy::types::JChar p2 );

/**
 * firePropertyChange
 *
 */
void firePropertyChange( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JShort p1, ::jace::proxy::types::JShort p2 );

/**
 * firePropertyChange
 *
 */
void firePropertyChange( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JLong p1, ::jace::proxy::types::JLong p2 );

/**
 * firePropertyChange
 *
 */
void firePropertyChange( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JFloat p1, ::jace::proxy::types::JFloat p2 );

/**
 * firePropertyChange
 *
 */
void firePropertyChange( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JDouble p1, ::jace::proxy::types::JDouble p2 );

/**
 * public static final TOP_ALIGNMENT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JFloat > TOP_ALIGNMENT();

/**
 * public static final CENTER_ALIGNMENT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JFloat > CENTER_ALIGNMENT();

/**
 * public static final BOTTOM_ALIGNMENT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JFloat > BOTTOM_ALIGNMENT();

/**
 * public static final LEFT_ALIGNMENT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JFloat > LEFT_ALIGNMENT();

/**
 * public static final RIGHT_ALIGNMENT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JFloat > RIGHT_ALIGNMENT();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
Component( jvalue value );
Component( jobject object );
Component( const Component& object );
Component( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, awt )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::awt::Component>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::awt::Component>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::awt::Component>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::awt::Component>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::awt::Component( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::awt::Component>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::awt::Component>& proxy ) : 
    ::jace::proxy::java::awt::Component( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::awt::Component>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::awt::Component>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::awt::Component>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::awt::Component>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::awt::Component>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::awt::Component( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::awt::Component>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::awt::Component( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::awt::Component>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::awt::Component>& object ) : 
    ::jace::proxy::java::awt::Component( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_AWT_COMPONENT_H

