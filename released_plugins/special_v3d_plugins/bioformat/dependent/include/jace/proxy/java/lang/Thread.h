#ifndef JACE_PROXY_JAVA_LANG_THREAD_H
#define JACE_PROXY_JAVA_LANG_THREAD_H

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
#ifndef JACE_PROXY_JAVA_LANG_RUNNABLE_H
#include "jace/proxy/java/lang/Runnable.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_TYPES_JINT_H
#include "jace/proxy/types/JInt.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class String;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_4( jace, proxy, java, util )
class Map;
END_NAMESPACE_4( jace, proxy, java, util )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JBoolean;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Throwable;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JLong;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )

/**
 * The Jace C++ proxy class for java.lang.Thread.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Thread : public virtual ::jace::proxy::java::lang::Object, public virtual ::jace::proxy::java::lang::Runnable
{
public: 

/**
 * currentThread
 *
 */
static ::jace::proxy::java::lang::Thread currentThread();

/**
 * yield
 *
 */
static void yield();

/**
 * sleep
 *
 */
static void sleep( ::jace::proxy::types::JLong p0 );

/**
 * sleep
 *
 */
static void sleep( ::jace::proxy::types::JLong p0, ::jace::proxy::types::JInt p1 );

/**
 * Thread
 *
 */
Thread();

/**
 * Thread
 *
 */
Thread( ::jace::proxy::java::lang::Runnable p0 );

/**
 * Thread
 *
 */
Thread( ::jace::proxy::java::lang::String p0 );

/**
 * Thread
 *
 */
Thread( ::jace::proxy::java::lang::Runnable p0, ::jace::proxy::java::lang::String p1 );

/**
 * start
 *
 */
void start();

/**
 * run
 *
 */
void run();

/**
 * stop
 *
 */
void stop();

/**
 * stop
 *
 */
void stop( ::jace::proxy::java::lang::Throwable p0 );

/**
 * interrupt
 *
 */
void interrupt();

/**
 * interrupted
 *
 */
static ::jace::proxy::types::JBoolean interrupted();

/**
 * isInterrupted
 *
 */
::jace::proxy::types::JBoolean isInterrupted();

/**
 * destroy
 *
 */
void destroy();

/**
 * isAlive
 *
 */
::jace::proxy::types::JBoolean isAlive();

/**
 * suspend
 *
 */
void suspend();

/**
 * resume
 *
 */
void resume();

/**
 * setPriority
 *
 */
void setPriority( ::jace::proxy::types::JInt p0 );

/**
 * getPriority
 *
 */
::jace::proxy::types::JInt getPriority();

/**
 * setName
 *
 */
void setName( ::jace::proxy::java::lang::String p0 );

/**
 * getName
 *
 */
::jace::proxy::java::lang::String getName();

/**
 * activeCount
 *
 */
static ::jace::proxy::types::JInt activeCount();

/**
 * enumerate
 *
 */
static ::jace::proxy::types::JInt enumerate( ::jace::JArray< ::jace::proxy::java::lang::Thread > p0 );

/**
 * countStackFrames
 *
 */
::jace::proxy::types::JInt countStackFrames();

/**
 * join
 *
 */
void join( ::jace::proxy::types::JLong p0 );

/**
 * join
 *
 */
void join( ::jace::proxy::types::JLong p0, ::jace::proxy::types::JInt p1 );

/**
 * join
 *
 */
void join();

/**
 * dumpStack
 *
 */
static void dumpStack();

/**
 * setDaemon
 *
 */
void setDaemon( ::jace::proxy::types::JBoolean p0 );

/**
 * isDaemon
 *
 */
::jace::proxy::types::JBoolean isDaemon();

/**
 * checkAccess
 *
 */
void checkAccess();

/**
 * toString
 *
 */
::jace::proxy::java::lang::String toString();

/**
 * holdsLock
 *
 */
static ::jace::proxy::types::JBoolean holdsLock( ::jace::proxy::java::lang::Object p0 );

/**
 * getAllStackTraces
 *
 */
static ::jace::proxy::java::util::Map getAllStackTraces();

/**
 * getId
 *
 */
::jace::proxy::types::JLong getId();

/**
 * public static final MIN_PRIORITY
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MIN_PRIORITY();

/**
 * public static final NORM_PRIORITY
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > NORM_PRIORITY();

/**
 * public static final MAX_PRIORITY
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MAX_PRIORITY();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
Thread( jvalue value );
Thread( jobject object );
Thread( const Thread& object );
Thread( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::lang::Thread>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::lang::Thread>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Thread>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::lang::Thread>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::lang::Thread( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::lang::Thread>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Thread>& proxy ) : 
    ::jace::proxy::java::lang::Thread( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::lang::Thread>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Thread>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Thread>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Thread>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Thread>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::lang::Thread( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Thread>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::lang::Thread( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Thread>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Thread>& object ) : 
    ::jace::proxy::java::lang::Thread( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_LANG_THREAD_H

