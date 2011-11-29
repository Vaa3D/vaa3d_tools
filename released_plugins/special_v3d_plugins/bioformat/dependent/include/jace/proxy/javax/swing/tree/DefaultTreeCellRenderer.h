#ifndef JACE_PROXY_JAVAX_SWING_TREE_DEFAULTTREECELLRENDERER_H
#define JACE_PROXY_JAVAX_SWING_TREE_DEFAULTTREECELLRENDERER_H

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
#ifndef JACE_PROXY_JAVAX_SWING_JLABEL_H
#include "jace/proxy/javax/swing/JLabel.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_JAVAX_SWING_TREE_TREECELLRENDERER_H
#include "jace/proxy/javax/swing/tree/TreeCellRenderer.h"
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

BEGIN_NAMESPACE_4( jace, proxy, java, awt )
class Component;
END_NAMESPACE_4( jace, proxy, java, awt )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JShort;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
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

BEGIN_NAMESPACE_3( jace, proxy, types )
class JFloat;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, javax, swing, tree )

/**
 * The Jace C++ proxy class for javax.swing.tree.DefaultTreeCellRenderer.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class DefaultTreeCellRenderer : public ::jace::proxy::javax::swing::JLabel, public virtual ::jace::proxy::javax::swing::tree::TreeCellRenderer
{
public: 

/**
 * DefaultTreeCellRenderer
 *
 */
DefaultTreeCellRenderer();

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
 * revalidate
 *
 */
void revalidate();

/**
 * repaint
 *
 */
void repaint( ::jace::proxy::types::JLong p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4 );

/**
 * repaint
 *
 */
void repaint();

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
void firePropertyChange( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

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
 * firePropertyChange
 *
 */
void firePropertyChange( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JBoolean p1, ::jace::proxy::types::JBoolean p2 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
DefaultTreeCellRenderer( jvalue value );
DefaultTreeCellRenderer( jobject object );
DefaultTreeCellRenderer( const DefaultTreeCellRenderer& object );
DefaultTreeCellRenderer( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, javax, swing, tree )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer>::ElementProxy( const jace::ElementProxy< ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer>::ElementProxy( const jace::ElementProxy< ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer>& proxy ) : 
    ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer>& object ) : 
    ::jace::proxy::javax::swing::tree::DefaultTreeCellRenderer( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVAX_SWING_TREE_DEFAULTTREECELLRENDERER_H

