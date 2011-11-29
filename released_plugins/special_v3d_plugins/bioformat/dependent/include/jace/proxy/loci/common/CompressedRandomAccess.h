#ifndef JACE_PROXY_LOCI_COMMON_COMPRESSEDRANDOMACCESS_H
#define JACE_PROXY_LOCI_COMMON_COMPRESSEDRANDOMACCESS_H

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
#ifndef JACE_PROXY_LOCI_COMMON_IRANDOMACCESS_H
#include "jace/proxy/loci/common/IRandomAccess.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
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
 * The Jace C++ proxy class for loci.common.CompressedRandomAccess.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class CompressedRandomAccess : public virtual ::jace::proxy::java::lang::Object, public virtual ::jace::proxy::loci::common::IRandomAccess
{
public: 

/**
 * CompressedRandomAccess
 *
 */
CompressedRandomAccess();

/**
 * close
 *
 */
void close();

/**
 * getFilePointer
 *
 */
::jace::proxy::types::JLong getFilePointer();

/**
 * length
 *
 */
::jace::proxy::types::JLong length();

/**
 * read
 *
 */
::jace::proxy::types::JInt read();

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
 * seek
 *
 */
void seek( ::jace::proxy::types::JLong p0 );

/**
 * setLength
 *
 */
void setLength( ::jace::proxy::types::JLong p0 );

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
 * write
 *
 */
void write( ::jace::JArray< ::jace::proxy::types::JByte > p0 );

/**
 * write
 *
 */
void write( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * write
 *
 */
void write( ::jace::proxy::types::JInt p0 );

/**
 * writeBoolean
 *
 */
void writeBoolean( ::jace::proxy::types::JBoolean p0 );

/**
 * writeByte
 *
 */
void writeByte( ::jace::proxy::types::JInt p0 );

/**
 * writeBytes
 *
 */
void writeBytes( ::jace::proxy::java::lang::String p0 );

/**
 * writeChar
 *
 */
void writeChar( ::jace::proxy::types::JInt p0 );

/**
 * writeChars
 *
 */
void writeChars( ::jace::proxy::java::lang::String p0 );

/**
 * writeDouble
 *
 */
void writeDouble( ::jace::proxy::types::JDouble p0 );

/**
 * writeFloat
 *
 */
void writeFloat( ::jace::proxy::types::JFloat p0 );

/**
 * writeInt
 *
 */
void writeInt( ::jace::proxy::types::JInt p0 );

/**
 * writeLong
 *
 */
void writeLong( ::jace::proxy::types::JLong p0 );

/**
 * writeShort
 *
 */
void writeShort( ::jace::proxy::types::JInt p0 );

/**
 * writeUTF
 *
 */
void writeUTF( ::jace::proxy::java::lang::String p0 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
CompressedRandomAccess( jvalue value );
CompressedRandomAccess( jobject object );
CompressedRandomAccess( const CompressedRandomAccess& object );
CompressedRandomAccess( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::common::CompressedRandomAccess>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::common::CompressedRandomAccess>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::common::CompressedRandomAccess>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::common::CompressedRandomAccess>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::common::CompressedRandomAccess( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::common::CompressedRandomAccess>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::common::CompressedRandomAccess>& proxy ) : 
    ::jace::proxy::loci::common::CompressedRandomAccess( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::common::CompressedRandomAccess>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::common::CompressedRandomAccess>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::common::CompressedRandomAccess>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::common::CompressedRandomAccess>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::common::CompressedRandomAccess>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::common::CompressedRandomAccess( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::common::CompressedRandomAccess>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::common::CompressedRandomAccess( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::common::CompressedRandomAccess>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::common::CompressedRandomAccess>& object ) : 
    ::jace::proxy::loci::common::CompressedRandomAccess( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_COMMON_COMPRESSEDRANDOMACCESS_H

