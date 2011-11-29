#ifndef JACE_PROXY_LOCI_FORMATS_IN_SDTINFO_H
#define JACE_PROXY_LOCI_FORMATS_IN_SDTINFO_H

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
#ifndef JACE_PROXY_JAVA_LANG_STRING_H
#include "jace/proxy/java/lang/String.h"
#endif

#ifndef JACE_TYPES_JBOOLEAN_H
#include "jace/proxy/types/JBoolean.h"
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
BEGIN_NAMESPACE_4( jace, proxy, java, io )
class IOException;
END_NAMESPACE_4( jace, proxy, java, io )

BEGIN_NAMESPACE_4( jace, proxy, loci, common )
class RandomAccessInputStream;
END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE_4( jace, proxy, java, util )
class Hashtable;
END_NAMESPACE_4( jace, proxy, java, util )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, in )

/**
 * The Jace C++ proxy class for loci.formats.in.SDTInfo.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class SDTInfo : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * SDTInfo
 *
 */
SDTInfo( ::jace::proxy::loci::common::RandomAccessInputStream p0, ::jace::proxy::java::util::Hashtable p1 );

/**
 * public static final BH_HEADER_CHKSUM
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JShort > JACE_BH_HEADER_CHKSUM();

/**
 * public static final BH_HEADER_NOT_VALID
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JShort > JACE_BH_HEADER_NOT_VALID();

/**
 * public static final BH_HEADER_VALID
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JShort > JACE_BH_HEADER_VALID();

/**
 * public static final SETUP_IDENTIFIER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > SETUP_IDENTIFIER();

/**
 * public static final DATA_IDENTIFIER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > DATA_IDENTIFIER();

/**
 * public static final FLOW_DATA_IDENTIFIER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > FLOW_DATA_IDENTIFIER();

/**
 * public static final DLL_DATA_IDENTIFIER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > DLL_DATA_IDENTIFIER();

/**
 * public static final FCS_DATA_IDENTIFIER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > FCS_DATA_IDENTIFIER();

/**
 * public static final X_STRING
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > X_STRING();

/**
 * public static final Y_STRING
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > Y_STRING();

/**
 * public static final T_STRING
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > T_STRING();

/**
 * public static final C_STRING
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > C_STRING();

/**
 * public width
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > width();

/**
 * public height
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > height();

/**
 * public timeBins
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > timeBins();

/**
 * public channels
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > channels();

/**
 * public revision
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > revision();

/**
 * public infoOffs
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > infoOffs();

/**
 * public infoLength
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > infoLength();

/**
 * public setupOffs
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > setupOffs();

/**
 * public setupLength
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > setupLength();

/**
 * public dataBlockOffs
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > dataBlockOffs();

/**
 * public noOfDataBlocks
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > noOfDataBlocks();

/**
 * public dataBlockLength
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > dataBlockLength();

/**
 * public measDescBlockOffs
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > measDescBlockOffs();

/**
 * public noOfMeasDescBlocks
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > noOfMeasDescBlocks();

/**
 * public measDescBlockLength
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > measDescBlockLength();

/**
 * public headerValid
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > headerValid();

/**
 * public reserved1
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JLong > reserved1();

/**
 * public reserved2
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > reserved2();

/**
 * public chksum
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > chksum();

/**
 * public info
 *
 */
::jace::JFieldProxy< ::jace::proxy::java::lang::String > info();

/**
 * public setup
 *
 */
::jace::JFieldProxy< ::jace::proxy::java::lang::String > setup();

/**
 * public hasMeasureInfo
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JBoolean > hasMeasureInfo();

/**
 * public time
 *
 */
::jace::JFieldProxy< ::jace::proxy::java::lang::String > time();

/**
 * public date
 *
 */
::jace::JFieldProxy< ::jace::proxy::java::lang::String > date();

/**
 * public modSerNo
 *
 */
::jace::JFieldProxy< ::jace::proxy::java::lang::String > modSerNo();

/**
 * public measMode
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > measMode();

/**
 * public cfdLL
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > cfdLL();

/**
 * public cfdLH
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > cfdLH();

/**
 * public cfdZC
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > cfdZC();

/**
 * public cfdHF
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > cfdHF();

/**
 * public synZC
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > synZC();

/**
 * public synFD
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > synFD();

/**
 * public synHF
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > synHF();

/**
 * public tacR
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > tacR();

/**
 * public tacG
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > tacG();

/**
 * public tacOF
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > tacOF();

/**
 * public tacLL
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > tacLL();

/**
 * public tacLH
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > tacLH();

/**
 * public adcRE
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > adcRE();

/**
 * public ealDE
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > ealDE();

/**
 * public ncx
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > ncx();

/**
 * public ncy
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > ncy();

/**
 * public page
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > page();

/**
 * public colT
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > colT();

/**
 * public repT
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > repT();

/**
 * public stopt
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > stopt();

/**
 * public overfl
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > overfl();

/**
 * public useMotor
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > useMotor();

/**
 * public steps
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > steps();

/**
 * public offset
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > offset();

/**
 * public dither
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > dither();

/**
 * public incr
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > incr();

/**
 * public memBank
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > memBank();

/**
 * public modType
 *
 */
::jace::JFieldProxy< ::jace::proxy::java::lang::String > modType();

/**
 * public synTH
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > synTH();

/**
 * public deadTimeComp
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > deadTimeComp();

/**
 * public polarityL
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > polarityL();

/**
 * public polarityF
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > polarityF();

/**
 * public polarityP
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > polarityP();

/**
 * public linediv
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > linediv();

/**
 * public accumulate
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > accumulate();

/**
 * public flbckY
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > flbckY();

/**
 * public flbckX
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > flbckX();

/**
 * public bordU
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > bordU();

/**
 * public bordL
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > bordL();

/**
 * public pixTime
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > pixTime();

/**
 * public pixClk
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > pixClk();

/**
 * public trigger
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > trigger();

/**
 * public scanX
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > scanX();

/**
 * public scanY
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > scanY();

/**
 * public scanRX
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > scanRX();

/**
 * public scanRY
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > scanRY();

/**
 * public fifoTyp
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > fifoTyp();

/**
 * public epxDiv
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > epxDiv();

/**
 * public modTypeCode
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > modTypeCode();

/**
 * public modFpgaVer
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > modFpgaVer();

/**
 * public overflowCorrFactor
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > overflowCorrFactor();

/**
 * public adcZoom
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > adcZoom();

/**
 * public cycles
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > cycles();

/**
 * public hasMeasStopInfo
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JBoolean > hasMeasStopInfo();

/**
 * public status
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > status();

/**
 * public flags
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > flags();

/**
 * public stopTime
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > stopTime();

/**
 * public curStep
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > curStep();

/**
 * public curCycle
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > curCycle();

/**
 * public curPage
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > curPage();

/**
 * public minSyncRate
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > minSyncRate();

/**
 * public minCfdRate
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > minCfdRate();

/**
 * public minTacRate
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > minTacRate();

/**
 * public minAdcRate
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > minAdcRate();

/**
 * public maxSyncRate
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > maxSyncRate();

/**
 * public maxCfdRate
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > maxCfdRate();

/**
 * public maxTacRate
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > maxTacRate();

/**
 * public maxAdcRate
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > maxAdcRate();

/**
 * public mReserved1
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > mReserved1();

/**
 * public mReserved2
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > mReserved2();

/**
 * public hasMeasFCSInfo
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JBoolean > hasMeasFCSInfo();

/**
 * public chan
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > chan();

/**
 * public fcsDecayCalc
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > fcsDecayCalc();

/**
 * public mtResol
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JLong > mtResol();

/**
 * public cortime
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > cortime();

/**
 * public calcPhotons
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JLong > calcPhotons();

/**
 * public fcsPoints
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > fcsPoints();

/**
 * public endTime
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > endTime();

/**
 * public overruns
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > overruns();

/**
 * public fcsType
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > fcsType();

/**
 * public crossChan
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > crossChan();

/**
 * public mod
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > mod();

/**
 * public crossMod
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > crossMod();

/**
 * public crossMtResol
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JLong > crossMtResol();

/**
 * public hasExtendedMeasureInfo
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JBoolean > hasExtendedMeasureInfo();

/**
 * public imageX
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > imageX();

/**
 * public imageY
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > imageY();

/**
 * public imageRX
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > imageRX();

/**
 * public imageRY
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > imageRY();

/**
 * public xyGain
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > xyGain();

/**
 * public masterClock
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > masterClock();

/**
 * public adcDE
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > adcDE();

/**
 * public detType
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > detType();

/**
 * public xAxis
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > xAxis();

/**
 * public hasMeasHISTInfo
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JBoolean > hasMeasHISTInfo();

/**
 * public fidaTime
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > fidaTime();

/**
 * public fildaTime
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > fildaTime();

/**
 * public fidaPoints
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > fidaPoints();

/**
 * public fildaPoints
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > fildaPoints();

/**
 * public mcsTime
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JFloat > mcsTime();

/**
 * public mcsPoints
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > mcsPoints();

/**
 * public blockNo
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > blockNo();

/**
 * public dataOffs
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > dataOffs();

/**
 * public nextBlockOffs
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > nextBlockOffs();

/**
 * public blockType
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JInt > blockType();

/**
 * public measDescBlockNo
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JShort > measDescBlockNo();

/**
 * public lblockNo
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JLong > lblockNo();

/**
 * public blockLength
 *
 */
::jace::JFieldProxy< ::jace::proxy::types::JLong > blockLength();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
SDTInfo( jvalue value );
SDTInfo( jobject object );
SDTInfo( const SDTInfo& object );
SDTInfo( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, in )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::in::SDTInfo>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::in::SDTInfo>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::in::SDTInfo>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::in::SDTInfo>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::in::SDTInfo( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::in::SDTInfo>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::in::SDTInfo>& proxy ) : 
    ::jace::proxy::loci::formats::in::SDTInfo( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::SDTInfo>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::SDTInfo>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::SDTInfo>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::in::SDTInfo>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::SDTInfo>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::in::SDTInfo( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::SDTInfo>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::in::SDTInfo( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::SDTInfo>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::in::SDTInfo>& object ) : 
    ::jace::proxy::loci::formats::in::SDTInfo( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_IN_SDTINFO_H

