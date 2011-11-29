#ifndef JACE_PROXY_LOCI_COMMON_DATATOOLS_H
#define JACE_PROXY_LOCI_COMMON_DATATOOLS_H

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
#ifndef JACE_TYPES_JBYTE_H
#include "jace/proxy/types/JByte.h"
#endif

#ifndef JACE_TYPES_JDOUBLE_H
#include "jace/proxy/types/JDouble.h"
#endif

#ifndef JACE_TYPES_JINT_H
#include "jace/proxy/types/JInt.h"
#endif

#ifndef JACE_TYPES_JSHORT_H
#include "jace/proxy/types/JShort.h"
#endif

#ifndef JACE_TYPES_JLONG_H
#include "jace/proxy/types/JLong.h"
#endif

#ifndef JACE_TYPES_JFLOAT_H
#include "jace/proxy/types/JFloat.h"
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
class DataInput;
END_NAMESPACE_4( jace, proxy, java, io )

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
class JShort;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

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

BEGIN_NAMESPACE_4( jace, proxy, java, io )
class DataOutput;
END_NAMESPACE_4( jace, proxy, java, io )

BEGIN_NAMESPACE_4( jace, proxy, loci, common )

/**
 * The Jace C++ proxy class for loci.common.DataTools.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class DataTools : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * readFile
 *
 */
static ::jace::proxy::java::lang::String readFile( ::jace::proxy::java::lang::String p0 );

/**
 * readSignedByte
 *
 */
static ::jace::proxy::types::JByte readSignedByte( ::jace::proxy::java::io::DataInput p0 );

/**
 * readUnsignedByte
 *
 */
static ::jace::proxy::types::JShort readUnsignedByte( ::jace::proxy::java::io::DataInput p0 );

/**
 * read2SignedBytes
 *
 */
static ::jace::proxy::types::JShort read2SignedBytes( ::jace::proxy::java::io::DataInput p0, ::jace::proxy::types::JBoolean p1 );

/**
 * read2UnsignedBytes
 *
 */
static ::jace::proxy::types::JInt read2UnsignedBytes( ::jace::proxy::java::io::DataInput p0, ::jace::proxy::types::JBoolean p1 );

/**
 * read4SignedBytes
 *
 */
static ::jace::proxy::types::JInt read4SignedBytes( ::jace::proxy::java::io::DataInput p0, ::jace::proxy::types::JBoolean p1 );

/**
 * read4UnsignedBytes
 *
 */
static ::jace::proxy::types::JLong read4UnsignedBytes( ::jace::proxy::java::io::DataInput p0, ::jace::proxy::types::JBoolean p1 );

/**
 * read8SignedBytes
 *
 */
static ::jace::proxy::types::JLong read8SignedBytes( ::jace::proxy::java::io::DataInput p0, ::jace::proxy::types::JBoolean p1 );

/**
 * readFloat
 *
 */
static ::jace::proxy::types::JFloat readFloat( ::jace::proxy::java::io::DataInput p0, ::jace::proxy::types::JBoolean p1 );

/**
 * readDouble
 *
 */
static ::jace::proxy::types::JDouble readDouble( ::jace::proxy::java::io::DataInput p0, ::jace::proxy::types::JBoolean p1 );

/**
 * writeString
 *
 */
static void writeString( ::jace::proxy::java::io::DataOutput p0, ::jace::proxy::java::lang::String p1 );

/**
 * writeDouble
 *
 */
static void writeDouble( ::jace::proxy::java::io::DataOutput p0, ::jace::proxy::types::JDouble p1, ::jace::proxy::types::JBoolean p2 );

/**
 * writeLong
 *
 */
static void writeLong( ::jace::proxy::java::io::DataOutput p0, ::jace::proxy::types::JLong p1, ::jace::proxy::types::JBoolean p2 );

/**
 * writeFloat
 *
 */
static void writeFloat( ::jace::proxy::java::io::DataOutput p0, ::jace::proxy::types::JFloat p1, ::jace::proxy::types::JBoolean p2 );

/**
 * writeInt
 *
 */
static void writeInt( ::jace::proxy::java::io::DataOutput p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * writeShort
 *
 */
static void writeShort( ::jace::proxy::java::io::DataOutput p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * bytesToShort
 *
 */
static ::jace::proxy::types::JShort bytesToShort( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3 );

/**
 * bytesToShort
 *
 */
static ::jace::proxy::types::JShort bytesToShort( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * bytesToShort
 *
 */
static ::jace::proxy::types::JShort bytesToShort( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * bytesToShort
 *
 */
static ::jace::proxy::types::JShort bytesToShort( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3 );

/**
 * bytesToShort
 *
 */
static ::jace::proxy::types::JShort bytesToShort( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * bytesToShort
 *
 */
static ::jace::proxy::types::JShort bytesToShort( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * bytesToInt
 *
 */
static ::jace::proxy::types::JInt bytesToInt( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3 );

/**
 * bytesToInt
 *
 */
static ::jace::proxy::types::JInt bytesToInt( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * bytesToInt
 *
 */
static ::jace::proxy::types::JInt bytesToInt( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * bytesToInt
 *
 */
static ::jace::proxy::types::JInt bytesToInt( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3 );

/**
 * bytesToInt
 *
 */
static ::jace::proxy::types::JInt bytesToInt( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * bytesToInt
 *
 */
static ::jace::proxy::types::JInt bytesToInt( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * bytesToFloat
 *
 */
static ::jace::proxy::types::JFloat bytesToFloat( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3 );

/**
 * bytesToFloat
 *
 */
static ::jace::proxy::types::JFloat bytesToFloat( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * bytesToFloat
 *
 */
static ::jace::proxy::types::JFloat bytesToFloat( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * bytesToFloat
 *
 */
static ::jace::proxy::types::JFloat bytesToFloat( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3 );

/**
 * bytesToFloat
 *
 */
static ::jace::proxy::types::JFloat bytesToFloat( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * bytesToFloat
 *
 */
static ::jace::proxy::types::JFloat bytesToFloat( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * bytesToLong
 *
 */
static ::jace::proxy::types::JLong bytesToLong( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3 );

/**
 * bytesToLong
 *
 */
static ::jace::proxy::types::JLong bytesToLong( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * bytesToLong
 *
 */
static ::jace::proxy::types::JLong bytesToLong( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * bytesToLong
 *
 */
static ::jace::proxy::types::JLong bytesToLong( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3 );

/**
 * bytesToLong
 *
 */
static ::jace::proxy::types::JLong bytesToLong( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * bytesToLong
 *
 */
static ::jace::proxy::types::JLong bytesToLong( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * bytesToDouble
 *
 */
static ::jace::proxy::types::JDouble bytesToDouble( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3 );

/**
 * bytesToDouble
 *
 */
static ::jace::proxy::types::JDouble bytesToDouble( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * bytesToDouble
 *
 */
static ::jace::proxy::types::JDouble bytesToDouble( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * bytesToDouble
 *
 */
static ::jace::proxy::types::JDouble bytesToDouble( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3 );

/**
 * bytesToDouble
 *
 */
static ::jace::proxy::types::JDouble bytesToDouble( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * bytesToDouble
 *
 */
static ::jace::proxy::types::JDouble bytesToDouble( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * shortToBytes
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > shortToBytes( ::jace::proxy::types::JShort p0, ::jace::proxy::types::JBoolean p1 );

/**
 * intToBytes
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > intToBytes( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JBoolean p1 );

/**
 * floatToBytes
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > floatToBytes( ::jace::proxy::types::JFloat p0, ::jace::proxy::types::JBoolean p1 );

/**
 * longToBytes
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > longToBytes( ::jace::proxy::types::JLong p0, ::jace::proxy::types::JBoolean p1 );

/**
 * doubleToBytes
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > doubleToBytes( ::jace::proxy::types::JDouble p0, ::jace::proxy::types::JBoolean p1 );

/**
 * shortsToBytes
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > shortsToBytes( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * intsToBytes
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > intsToBytes( ::jace::JArray< ::jace::proxy::types::JInt > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * floatsToBytes
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > floatsToBytes( ::jace::JArray< ::jace::proxy::types::JFloat > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * longsToBytes
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > longsToBytes( ::jace::JArray< ::jace::proxy::types::JLong > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * doublesToBytes
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > doublesToBytes( ::jace::JArray< ::jace::proxy::types::JLong > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * unpackShort
 *
 */
static void unpackShort( ::jace::proxy::types::JShort p0, ::jace::JArray< ::jace::proxy::types::JByte > p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3 );

/**
 * unpackBytes
 *
 */
static void unpackBytes( ::jace::proxy::types::JLong p0, ::jace::JArray< ::jace::proxy::types::JByte > p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JBoolean p4 );

/**
 * makeDataArray
 *
 */
static ::jace::proxy::java::lang::Object makeDataArray( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2, ::jace::proxy::types::JBoolean p3 );

/**
 * makeDataArray
 *
 */
static ::jace::proxy::java::lang::Object makeDataArray( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2, ::jace::proxy::types::JBoolean p3, ::jace::proxy::types::JBoolean p4 );

/**
 * swap
 *
 */
static ::jace::proxy::types::JShort swap( ::jace::proxy::types::JShort p0 );

/**
 * swap
 *
 */
static ::jace::proxy::types::JChar swap( ::jace::proxy::types::JChar p0 );

/**
 * swap
 *
 */
static ::jace::proxy::types::JInt swap( ::jace::proxy::types::JInt p0 );

/**
 * swap
 *
 */
static ::jace::proxy::types::JLong swap( ::jace::proxy::types::JLong p0 );

/**
 * swap
 *
 */
static ::jace::proxy::types::JFloat swap( ::jace::proxy::types::JFloat p0 );

/**
 * swap
 *
 */
static ::jace::proxy::types::JDouble swap( ::jace::proxy::types::JDouble p0 );

/**
 * stripString
 *
 */
static ::jace::proxy::java::lang::String stripString( ::jace::proxy::java::lang::String p0 );

/**
 * samePrefix
 *
 */
static ::jace::proxy::types::JBoolean samePrefix( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * sanitize
 *
 */
static ::jace::proxy::java::lang::String sanitize( ::jace::proxy::java::lang::String p0 );

/**
 * normalizeFloats
 *
 */
static ::jace::JArray< ::jace::proxy::types::JFloat > normalizeFloats( ::jace::JArray< ::jace::proxy::types::JFloat > p0 );

/**
 * normalizeDoubles
 *
 */
static ::jace::JArray< ::jace::proxy::types::JDouble > normalizeDoubles( ::jace::JArray< ::jace::proxy::types::JDouble > p0 );

/**
 * containsValue
 *
 */
static ::jace::proxy::types::JBoolean containsValue( ::jace::JArray< ::jace::proxy::types::JInt > p0, ::jace::proxy::types::JInt p1 );

/**
 * indexOf
 *
 */
static ::jace::proxy::types::JInt indexOf( ::jace::JArray< ::jace::proxy::types::JInt > p0, ::jace::proxy::types::JInt p1 );

/**
 * indexOf
 *
 */
static ::jace::proxy::types::JInt indexOf( ::jace::JArray< ::jace::proxy::java::lang::Object > p0, ::jace::proxy::java::lang::Object p1 );

/**
 * makeSigned
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > makeSigned( ::jace::JArray< ::jace::proxy::types::JByte > p0 );

/**
 * makeSigned
 *
 */
static ::jace::JArray< ::jace::proxy::types::JShort > makeSigned( ::jace::JArray< ::jace::proxy::types::JShort > p0 );

/**
 * makeSigned
 *
 */
static ::jace::JArray< ::jace::proxy::types::JInt > makeSigned( ::jace::JArray< ::jace::proxy::types::JInt > p0 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
DataTools( jvalue value );
DataTools( jobject object );
DataTools( const DataTools& object );
DataTools( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::common::DataTools>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::common::DataTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::common::DataTools>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::common::DataTools>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::common::DataTools( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::common::DataTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::common::DataTools>& proxy ) : 
    ::jace::proxy::loci::common::DataTools( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::common::DataTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::common::DataTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::common::DataTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::common::DataTools>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::common::DataTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::common::DataTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::common::DataTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::common::DataTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::common::DataTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::common::DataTools>& object ) : 
    ::jace::proxy::loci::common::DataTools( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_COMMON_DATATOOLS_H

