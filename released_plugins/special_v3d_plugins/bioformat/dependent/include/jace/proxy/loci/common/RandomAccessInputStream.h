#ifndef JACE_PROXY_LOCI_COMMON_RANDOMACCESSINPUTSTREAM_H
#define JACE_PROXY_LOCI_COMMON_RANDOMACCESSINPUTSTREAM_H

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
#ifndef JACE_PROXY_JAVA_IO_INPUTSTREAM_H
#include "jace/proxy/java/io/InputStream.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_JAVA_IO_DATAINPUT_H
#include "jace/proxy/java/io/DataInput.h"
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

BEGIN_NAMESPACE_4( jace, proxy, java, io )
class IOException;
END_NAMESPACE_4( jace, proxy, java, io )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JByte;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JDouble;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JChar;
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

BEGIN_NAMESPACE_3( jace, proxy, types )
class JFloat;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, common )

/**
 * The Jace C++ proxy class for loci.common.RandomAccessInputStream.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class RandomAccessInputStream : public ::jace::proxy::java::io::InputStream, public virtual ::jace::proxy::java::io::DataInput
{
public: 

/**
 * RandomAccessInputStream
 *
 */
RandomAccessInputStream( ::jace::proxy::java::lang::String p0 );

/**
 * RandomAccessInputStream
 *
 */
RandomAccessInputStream( ::jace::JArray< ::jace::proxy::types::JByte > p0 );

/**
 * setExtend
 *
 */
void setExtend( ::jace::proxy::types::JInt p0 );

/**
 * seek
 *
 */
void seek( ::jace::proxy::types::JLong p0 );

/**
 * read
 *
 */
::jace::proxy::types::JInt read();

/**
 * length
 *
 */
::jace::proxy::types::JLong length();

/**
 * getFilePointer
 *
 */
::jace::proxy::types::JLong getFilePointer();

/**
 * close
 *
 */
void close();

/**
 * order
 *
 */
void order( ::jace::proxy::types::JBoolean p0 );

/**
 * isLittleEndian
 *
 */
::jace::proxy::types::JBoolean isLittleEndian();

/**
 * readString
 *
 */
::jace::proxy::java::lang::String readString( ::jace::proxy::java::lang::String p0 );

/**
 * findString
 *
 */
::jace::proxy::java::lang::String findString( ::jace::JArray< ::jace::proxy::java::lang::String > p0 );

/**
 * findString
 *
 */
::jace::proxy::java::lang::String findString( ::jace::proxy::types::JInt p0, ::jace::JArray< ::jace::proxy::java::lang::String > p1 );

/**
 * readBoolean
 *
 */
::jace::proxy::types::JBoolean readBoolean();

/**
 * readByte
 *
 */
::jace::proxy::types::JByte readByte();

/**
 * readChar
 *
 */
::jace::proxy::types::JChar readChar();

/**
 * readDouble
 *
 */
::jace::proxy::types::JDouble readDouble();

/**
 * readFloat
 *
 */
::jace::proxy::types::JFloat readFloat();

/**
 * readInt
 *
 */
::jace::proxy::types::JInt readInt();

/**
 * readLine
 *
 */
::jace::proxy::java::lang::String readLine();

/**
 * readCString
 *
 */
::jace::proxy::java::lang::String readCString();

/**
 * readString
 *
 */
::jace::proxy::java::lang::String readString( ::jace::proxy::types::JInt p0 );

/**
 * readLong
 *
 */
::jace::proxy::types::JLong readLong();

/**
 * readShort
 *
 */
::jace::proxy::types::JShort readShort();

/**
 * readUnsignedByte
 *
 */
::jace::proxy::types::JInt readUnsignedByte();

/**
 * readUnsignedShort
 *
 */
::jace::proxy::types::JInt readUnsignedShort();

/**
 * readUTF
 *
 */
::jace::proxy::java::lang::String readUTF();

/**
 * skipBytes
 *
 */
::jace::proxy::types::JInt skipBytes( ::jace::proxy::types::JInt p0 );

/**
 * read
 *
 */
::jace::proxy::types::JInt read( ::jace::JArray< ::jace::proxy::types::JByte > p0 );

/**
 * read
 *
 */
::jace::proxy::types::JInt read( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * readFully
 *
 */
void readFully( ::jace::JArray< ::jace::proxy::types::JByte > p0 );

/**
 * readFully
 *
 */
void readFully( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * available
 *
 */
::jace::proxy::types::JInt available();

/**
 * mark
 *
 */
void mark( ::jace::proxy::types::JInt p0 );

/**
 * markSupported
 *
 */
::jace::proxy::types::JBoolean markSupported();

/**
 * reset
 *
 */
void reset();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
RandomAccessInputStream( jvalue value );
RandomAccessInputStream( jobject object );
RandomAccessInputStream( const RandomAccessInputStream& object );
RandomAccessInputStream( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::common::RandomAccessInputStream>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::common::RandomAccessInputStream>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::common::RandomAccessInputStream>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::common::RandomAccessInputStream>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::common::RandomAccessInputStream( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::common::RandomAccessInputStream>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::common::RandomAccessInputStream>& proxy ) : 
    ::jace::proxy::loci::common::RandomAccessInputStream( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::common::RandomAccessInputStream>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::common::RandomAccessInputStream>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::common::RandomAccessInputStream>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::common::RandomAccessInputStream>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::common::RandomAccessInputStream>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::common::RandomAccessInputStream( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::common::RandomAccessInputStream>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::common::RandomAccessInputStream( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::common::RandomAccessInputStream>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::common::RandomAccessInputStream>& object ) : 
    ::jace::proxy::loci::common::RandomAccessInputStream( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_COMMON_RANDOMACCESSINPUTSTREAM_H

