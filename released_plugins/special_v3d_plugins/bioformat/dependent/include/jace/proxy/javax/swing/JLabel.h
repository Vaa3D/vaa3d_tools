#ifndef JACE_PROXY_JAVAX_SWING_JLABEL_H
#define JACE_PROXY_JAVAX_SWING_JLABEL_H

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
#ifndef JACE_PROXY_JAVAX_SWING_JCOMPONENT_H
#include "jace/proxy/javax/swing/JComponent.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_JAVAX_SWING_SWINGCONSTANTS_H
#include "jace/proxy/javax/swing/SwingConstants.h"
#endif

#ifndef JACE_PROXY_JAVAX_ACCESSIBILITY_ACCESSIBLE_H
#include "jace/proxy/javax/accessibility/Accessible.h"
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

BEGIN_NAMESPACE_4( jace, proxy, java, awt )
class Component;
END_NAMESPACE_4( jace, proxy, java, awt )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JChar;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, javax, swing )

/**
 * The Jace C++ proxy class for javax.swing.JLabel.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class JLabel : public ::jace::proxy::javax::swing::JComponent, public virtual ::jace::proxy::javax::swing::SwingConstants, public virtual ::jace::proxy::javax::accessibility::Accessible
{
public: 

/**
 * JLabel
 *
 */
JLabel( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * JLabel
 *
 */
JLabel( ::jace::proxy::java::lang::String p0 );

/**
 * JLabel
 *
 */
JLabel();

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
 * getText
 *
 */
::jace::proxy::java::lang::String getText();

/**
 * setText
 *
 */
void setText( ::jace::proxy::java::lang::String p0 );

/**
 * setDisplayedMnemonic
 *
 */
void setDisplayedMnemonic( ::jace::proxy::types::JInt p0 );

/**
 * setDisplayedMnemonic
 *
 */
void setDisplayedMnemonic( ::jace::proxy::types::JChar p0 );

/**
 * getDisplayedMnemonic
 *
 */
::jace::proxy::types::JInt getDisplayedMnemonic();

/**
 * setDisplayedMnemonicIndex
 *
 */
void setDisplayedMnemonicIndex( ::jace::proxy::types::JInt p0 );

/**
 * getDisplayedMnemonicIndex
 *
 */
::jace::proxy::types::JInt getDisplayedMnemonicIndex();

/**
 * getIconTextGap
 *
 */
::jace::proxy::types::JInt getIconTextGap();

/**
 * setIconTextGap
 *
 */
void setIconTextGap( ::jace::proxy::types::JInt p0 );

/**
 * getVerticalAlignment
 *
 */
::jace::proxy::types::JInt getVerticalAlignment();

/**
 * setVerticalAlignment
 *
 */
void setVerticalAlignment( ::jace::proxy::types::JInt p0 );

/**
 * getHorizontalAlignment
 *
 */
::jace::proxy::types::JInt getHorizontalAlignment();

/**
 * setHorizontalAlignment
 *
 */
void setHorizontalAlignment( ::jace::proxy::types::JInt p0 );

/**
 * getVerticalTextPosition
 *
 */
::jace::proxy::types::JInt getVerticalTextPosition();

/**
 * setVerticalTextPosition
 *
 */
void setVerticalTextPosition( ::jace::proxy::types::JInt p0 );

/**
 * getHorizontalTextPosition
 *
 */
::jace::proxy::types::JInt getHorizontalTextPosition();

/**
 * setHorizontalTextPosition
 *
 */
void setHorizontalTextPosition( ::jace::proxy::types::JInt p0 );

/**
 * getLabelFor
 *
 */
::jace::proxy::java::awt::Component getLabelFor();

/**
 * setLabelFor
 *
 */
void setLabelFor( ::jace::proxy::java::awt::Component p0 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
JLabel( jvalue value );
JLabel( jobject object );
JLabel( const JLabel& object );
JLabel( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, javax, swing )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::javax::swing::JLabel>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::javax::swing::JLabel>::ElementProxy( const jace::ElementProxy< ::jace::proxy::javax::swing::JLabel>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::javax::swing::JLabel>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::javax::swing::JLabel( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::javax::swing::JLabel>::ElementProxy( const jace::ElementProxy< ::jace::proxy::javax::swing::JLabel>& proxy ) : 
    ::jace::proxy::javax::swing::JLabel( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::javax::swing::JLabel>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::javax::swing::JLabel>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::javax::swing::JLabel>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::javax::swing::JLabel>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::JLabel>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::javax::swing::JLabel( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::JLabel>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::javax::swing::JLabel( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::JLabel>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::javax::swing::JLabel>& object ) : 
    ::jace::proxy::javax::swing::JLabel( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVAX_SWING_JLABEL_H

