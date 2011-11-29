#ifndef JACE_PROXY_LOCI_FORMATS_META_FILTERMETADATA_H
#define JACE_PROXY_LOCI_FORMATS_META_FILTERMETADATA_H

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
#ifndef JACE_PROXY_LOCI_FORMATS_META_METADATASTORE_H
#include "jace/proxy/loci/formats/meta/MetadataStore.h"
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

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Integer;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JBoolean;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Double;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Boolean;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, meta )

/**
 * The Jace C++ proxy class for loci.formats.meta.FilterMetadata.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class FilterMetadata : public virtual ::jace::proxy::java::lang::Object, public virtual ::jace::proxy::loci::formats::meta::MetadataStore
{
public: 

/**
 * FilterMetadata
 *
 */
FilterMetadata( ::jace::proxy::loci::formats::meta::MetadataStore p0, ::jace::proxy::types::JBoolean p1 );

/**
 * createRoot
 *
 */
void createRoot();

/**
 * getRoot
 *
 */
::jace::proxy::java::lang::Object getRoot();

/**
 * setRoot
 *
 */
void setRoot( ::jace::proxy::java::lang::Object p0 );

/**
 * setUUID
 *
 */
void setUUID( ::jace::proxy::java::lang::String p0 );

/**
 * setArcType
 *
 */
void setArcType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setChannelComponentColorDomain
 *
 */
void setChannelComponentColorDomain( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setChannelComponentIndex
 *
 */
void setChannelComponentIndex( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setChannelComponentPixels
 *
 */
void setChannelComponentPixels( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setCircleCx
 *
 */
void setCircleCx( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setCircleCy
 *
 */
void setCircleCy( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setCircleID
 *
 */
void setCircleID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setCircleR
 *
 */
void setCircleR( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setCircleTransform
 *
 */
void setCircleTransform( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setContactExperimenter
 *
 */
void setContactExperimenter( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setDatasetDescription
 *
 */
void setDatasetDescription( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setDatasetExperimenterRef
 *
 */
void setDatasetExperimenterRef( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setDatasetGroupRef
 *
 */
void setDatasetGroupRef( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setDatasetID
 *
 */
void setDatasetID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setDatasetLocked
 *
 */
void setDatasetLocked( ::jace::proxy::java::lang::Boolean p0, ::jace::proxy::types::JInt p1 );

/**
 * setDatasetName
 *
 */
void setDatasetName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setDatasetRefID
 *
 */
void setDatasetRefID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorAmplificationGain
 *
 */
void setDetectorAmplificationGain( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorGain
 *
 */
void setDetectorGain( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorID
 *
 */
void setDetectorID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorManufacturer
 *
 */
void setDetectorManufacturer( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorModel
 *
 */
void setDetectorModel( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorOffset
 *
 */
void setDetectorOffset( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorSerialNumber
 *
 */
void setDetectorSerialNumber( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorType
 *
 */
void setDetectorType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorVoltage
 *
 */
void setDetectorVoltage( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorZoom
 *
 */
void setDetectorZoom( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorSettingsBinning
 *
 */
void setDetectorSettingsBinning( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorSettingsDetector
 *
 */
void setDetectorSettingsDetector( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorSettingsGain
 *
 */
void setDetectorSettingsGain( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorSettingsOffset
 *
 */
void setDetectorSettingsOffset( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorSettingsReadOutRate
 *
 */
void setDetectorSettingsReadOutRate( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDetectorSettingsVoltage
 *
 */
void setDetectorSettingsVoltage( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDichroicID
 *
 */
void setDichroicID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDichroicLotNumber
 *
 */
void setDichroicLotNumber( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDichroicManufacturer
 *
 */
void setDichroicManufacturer( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDichroicModel
 *
 */
void setDichroicModel( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDimensionsPhysicalSizeX
 *
 */
void setDimensionsPhysicalSizeX( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDimensionsPhysicalSizeY
 *
 */
void setDimensionsPhysicalSizeY( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDimensionsPhysicalSizeZ
 *
 */
void setDimensionsPhysicalSizeZ( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDimensionsTimeIncrement
 *
 */
void setDimensionsTimeIncrement( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDimensionsWaveIncrement
 *
 */
void setDimensionsWaveIncrement( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDimensionsWaveStart
 *
 */
void setDimensionsWaveStart( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setDisplayOptionsDisplay
 *
 */
void setDisplayOptionsDisplay( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setDisplayOptionsID
 *
 */
void setDisplayOptionsID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setDisplayOptionsZoom
 *
 */
void setDisplayOptionsZoom( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1 );

/**
 * setEllipseCx
 *
 */
void setEllipseCx( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setEllipseCy
 *
 */
void setEllipseCy( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setEllipseID
 *
 */
void setEllipseID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setEllipseRx
 *
 */
void setEllipseRx( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setEllipseRy
 *
 */
void setEllipseRy( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setEllipseTransform
 *
 */
void setEllipseTransform( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setEmFilterLotNumber
 *
 */
void setEmFilterLotNumber( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setEmFilterManufacturer
 *
 */
void setEmFilterManufacturer( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setEmFilterModel
 *
 */
void setEmFilterModel( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setEmFilterType
 *
 */
void setEmFilterType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setExFilterLotNumber
 *
 */
void setExFilterLotNumber( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setExFilterManufacturer
 *
 */
void setExFilterManufacturer( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setExFilterModel
 *
 */
void setExFilterModel( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setExFilterType
 *
 */
void setExFilterType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setExperimentDescription
 *
 */
void setExperimentDescription( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setExperimentExperimenterRef
 *
 */
void setExperimentExperimenterRef( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setExperimentID
 *
 */
void setExperimentID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setExperimentType
 *
 */
void setExperimentType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setExperimenterEmail
 *
 */
void setExperimenterEmail( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setExperimenterFirstName
 *
 */
void setExperimenterFirstName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setExperimenterID
 *
 */
void setExperimenterID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setExperimenterInstitution
 *
 */
void setExperimenterInstitution( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setExperimenterLastName
 *
 */
void setExperimenterLastName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setExperimenterOMEName
 *
 */
void setExperimenterOMEName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setExperimenterMembershipGroup
 *
 */
void setExperimenterMembershipGroup( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setFilamentType
 *
 */
void setFilamentType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setFilterFilterWheel
 *
 */
void setFilterFilterWheel( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setFilterID
 *
 */
void setFilterID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setFilterLotNumber
 *
 */
void setFilterLotNumber( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setFilterManufacturer
 *
 */
void setFilterManufacturer( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setFilterModel
 *
 */
void setFilterModel( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setFilterType
 *
 */
void setFilterType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setFilterSetDichroic
 *
 */
void setFilterSetDichroic( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setFilterSetEmFilter
 *
 */
void setFilterSetEmFilter( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setFilterSetExFilter
 *
 */
void setFilterSetExFilter( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setFilterSetID
 *
 */
void setFilterSetID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setFilterSetLotNumber
 *
 */
void setFilterSetLotNumber( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setFilterSetManufacturer
 *
 */
void setFilterSetManufacturer( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setFilterSetModel
 *
 */
void setFilterSetModel( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setGroupID
 *
 */
void setGroupID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setGroupName
 *
 */
void setGroupName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setImageAcquiredPixels
 *
 */
void setImageAcquiredPixels( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setImageCreationDate
 *
 */
void setImageCreationDate( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setImageDefaultPixels
 *
 */
void setImageDefaultPixels( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setImageDescription
 *
 */
void setImageDescription( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setImageExperimentRef
 *
 */
void setImageExperimentRef( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setImageExperimenterRef
 *
 */
void setImageExperimenterRef( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setImageGroupRef
 *
 */
void setImageGroupRef( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setImageID
 *
 */
void setImageID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setImageInstrumentRef
 *
 */
void setImageInstrumentRef( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setImageName
 *
 */
void setImageName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setImagingEnvironmentAirPressure
 *
 */
void setImagingEnvironmentAirPressure( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1 );

/**
 * setImagingEnvironmentCO2Percent
 *
 */
void setImagingEnvironmentCO2Percent( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1 );

/**
 * setImagingEnvironmentHumidity
 *
 */
void setImagingEnvironmentHumidity( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1 );

/**
 * setImagingEnvironmentTemperature
 *
 */
void setImagingEnvironmentTemperature( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1 );

/**
 * setInstrumentID
 *
 */
void setInstrumentID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setLaserFrequencyMultiplication
 *
 */
void setLaserFrequencyMultiplication( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLaserLaserMedium
 *
 */
void setLaserLaserMedium( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLaserPockelCell
 *
 */
void setLaserPockelCell( ::jace::proxy::java::lang::Boolean p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLaserPulse
 *
 */
void setLaserPulse( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLaserRepetitionRate
 *
 */
void setLaserRepetitionRate( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLaserTuneable
 *
 */
void setLaserTuneable( ::jace::proxy::java::lang::Boolean p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLaserType
 *
 */
void setLaserType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLaserWavelength
 *
 */
void setLaserWavelength( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLightSourceID
 *
 */
void setLightSourceID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLightSourceManufacturer
 *
 */
void setLightSourceManufacturer( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLightSourceModel
 *
 */
void setLightSourceModel( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLightSourcePower
 *
 */
void setLightSourcePower( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLightSourceSerialNumber
 *
 */
void setLightSourceSerialNumber( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLightSourceRefAttenuation
 *
 */
void setLightSourceRefAttenuation( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setLightSourceRefLightSource
 *
 */
void setLightSourceRefLightSource( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setLightSourceRefWavelength
 *
 */
void setLightSourceRefWavelength( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setLightSourceSettingsAttenuation
 *
 */
void setLightSourceSettingsAttenuation( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLightSourceSettingsLightSource
 *
 */
void setLightSourceSettingsLightSource( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLightSourceSettingsWavelength
 *
 */
void setLightSourceSettingsWavelength( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLineID
 *
 */
void setLineID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setLineTransform
 *
 */
void setLineTransform( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setLineX1
 *
 */
void setLineX1( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setLineX2
 *
 */
void setLineX2( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setLineY1
 *
 */
void setLineY1( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setLineY2
 *
 */
void setLineY2( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setLogicalChannelContrastMethod
 *
 */
void setLogicalChannelContrastMethod( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelDetector
 *
 */
void setLogicalChannelDetector( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelEmWave
 *
 */
void setLogicalChannelEmWave( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelExWave
 *
 */
void setLogicalChannelExWave( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelFilterSet
 *
 */
void setLogicalChannelFilterSet( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelFluor
 *
 */
void setLogicalChannelFluor( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelID
 *
 */
void setLogicalChannelID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelIlluminationType
 *
 */
void setLogicalChannelIlluminationType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelLightSource
 *
 */
void setLogicalChannelLightSource( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelMode
 *
 */
void setLogicalChannelMode( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelName
 *
 */
void setLogicalChannelName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelNdFilter
 *
 */
void setLogicalChannelNdFilter( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelOTF
 *
 */
void setLogicalChannelOTF( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelPhotometricInterpretation
 *
 */
void setLogicalChannelPhotometricInterpretation( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelPinholeSize
 *
 */
void setLogicalChannelPinholeSize( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelPockelCellSetting
 *
 */
void setLogicalChannelPockelCellSetting( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelSamplesPerPixel
 *
 */
void setLogicalChannelSamplesPerPixel( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelSecondaryEmissionFilter
 *
 */
void setLogicalChannelSecondaryEmissionFilter( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setLogicalChannelSecondaryExcitationFilter
 *
 */
void setLogicalChannelSecondaryExcitationFilter( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setMaskHeight
 *
 */
void setMaskHeight( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setMaskID
 *
 */
void setMaskID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setMaskTransform
 *
 */
void setMaskTransform( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setMaskWidth
 *
 */
void setMaskWidth( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setMaskX
 *
 */
void setMaskX( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setMaskY
 *
 */
void setMaskY( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setMaskPixelsBigEndian
 *
 */
void setMaskPixelsBigEndian( ::jace::proxy::java::lang::Boolean p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setMaskPixelsBinData
 *
 */
void setMaskPixelsBinData( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setMaskPixelsExtendedPixelType
 *
 */
void setMaskPixelsExtendedPixelType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setMaskPixelsID
 *
 */
void setMaskPixelsID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setMaskPixelsSizeX
 *
 */
void setMaskPixelsSizeX( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setMaskPixelsSizeY
 *
 */
void setMaskPixelsSizeY( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setMicrobeamManipulationExperimenterRef
 *
 */
void setMicrobeamManipulationExperimenterRef( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setMicrobeamManipulationID
 *
 */
void setMicrobeamManipulationID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setMicrobeamManipulationType
 *
 */
void setMicrobeamManipulationType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setMicrobeamManipulationRefID
 *
 */
void setMicrobeamManipulationRefID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setMicroscopeID
 *
 */
void setMicroscopeID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setMicroscopeManufacturer
 *
 */
void setMicroscopeManufacturer( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setMicroscopeModel
 *
 */
void setMicroscopeModel( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setMicroscopeSerialNumber
 *
 */
void setMicroscopeSerialNumber( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setMicroscopeType
 *
 */
void setMicroscopeType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setOTFBinaryFile
 *
 */
void setOTFBinaryFile( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setOTFID
 *
 */
void setOTFID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setOTFObjective
 *
 */
void setOTFObjective( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setOTFOpticalAxisAveraged
 *
 */
void setOTFOpticalAxisAveraged( ::jace::proxy::java::lang::Boolean p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setOTFPixelType
 *
 */
void setOTFPixelType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setOTFSizeX
 *
 */
void setOTFSizeX( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setOTFSizeY
 *
 */
void setOTFSizeY( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setObjectiveCalibratedMagnification
 *
 */
void setObjectiveCalibratedMagnification( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setObjectiveCorrection
 *
 */
void setObjectiveCorrection( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setObjectiveID
 *
 */
void setObjectiveID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setObjectiveImmersion
 *
 */
void setObjectiveImmersion( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setObjectiveIris
 *
 */
void setObjectiveIris( ::jace::proxy::java::lang::Boolean p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setObjectiveLensNA
 *
 */
void setObjectiveLensNA( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setObjectiveManufacturer
 *
 */
void setObjectiveManufacturer( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setObjectiveModel
 *
 */
void setObjectiveModel( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setObjectiveNominalMagnification
 *
 */
void setObjectiveNominalMagnification( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setObjectiveSerialNumber
 *
 */
void setObjectiveSerialNumber( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setObjectiveWorkingDistance
 *
 */
void setObjectiveWorkingDistance( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setObjectiveSettingsCorrectionCollar
 *
 */
void setObjectiveSettingsCorrectionCollar( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1 );

/**
 * setObjectiveSettingsMedium
 *
 */
void setObjectiveSettingsMedium( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setObjectiveSettingsObjective
 *
 */
void setObjectiveSettingsObjective( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setObjectiveSettingsRefractiveIndex
 *
 */
void setObjectiveSettingsRefractiveIndex( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1 );

/**
 * setPathD
 *
 */
void setPathD( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPathID
 *
 */
void setPathID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPixelsBigEndian
 *
 */
void setPixelsBigEndian( ::jace::proxy::java::lang::Boolean p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setPixelsDimensionOrder
 *
 */
void setPixelsDimensionOrder( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setPixelsID
 *
 */
void setPixelsID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setPixelsPixelType
 *
 */
void setPixelsPixelType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setPixelsSizeC
 *
 */
void setPixelsSizeC( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setPixelsSizeT
 *
 */
void setPixelsSizeT( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setPixelsSizeX
 *
 */
void setPixelsSizeX( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setPixelsSizeY
 *
 */
void setPixelsSizeY( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setPixelsSizeZ
 *
 */
void setPixelsSizeZ( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setPlaneHashSHA1
 *
 */
void setPlaneHashSHA1( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPlaneID
 *
 */
void setPlaneID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPlaneTheC
 *
 */
void setPlaneTheC( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPlaneTheT
 *
 */
void setPlaneTheT( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPlaneTheZ
 *
 */
void setPlaneTheZ( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPlaneTimingDeltaT
 *
 */
void setPlaneTimingDeltaT( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPlaneTimingExposureTime
 *
 */
void setPlaneTimingExposureTime( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPlateColumnNamingConvention
 *
 */
void setPlateColumnNamingConvention( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setPlateDescription
 *
 */
void setPlateDescription( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setPlateExternalIdentifier
 *
 */
void setPlateExternalIdentifier( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setPlateID
 *
 */
void setPlateID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setPlateName
 *
 */
void setPlateName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setPlateRowNamingConvention
 *
 */
void setPlateRowNamingConvention( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setPlateStatus
 *
 */
void setPlateStatus( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setPlateWellOriginX
 *
 */
void setPlateWellOriginX( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1 );

/**
 * setPlateWellOriginY
 *
 */
void setPlateWellOriginY( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1 );

/**
 * setPlateRefID
 *
 */
void setPlateRefID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setPlateRefSample
 *
 */
void setPlateRefSample( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setPlateRefWell
 *
 */
void setPlateRefWell( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setPointCx
 *
 */
void setPointCx( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPointCy
 *
 */
void setPointCy( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPointID
 *
 */
void setPointID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPointR
 *
 */
void setPointR( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPointTransform
 *
 */
void setPointTransform( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPolygonID
 *
 */
void setPolygonID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPolygonPoints
 *
 */
void setPolygonPoints( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPolygonTransform
 *
 */
void setPolygonTransform( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPolylineID
 *
 */
void setPolylineID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPolylinePoints
 *
 */
void setPolylinePoints( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setPolylineTransform
 *
 */
void setPolylineTransform( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setProjectDescription
 *
 */
void setProjectDescription( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setProjectExperimenterRef
 *
 */
void setProjectExperimenterRef( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setProjectGroupRef
 *
 */
void setProjectGroupRef( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setProjectID
 *
 */
void setProjectID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setProjectName
 *
 */
void setProjectName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setProjectRefID
 *
 */
void setProjectRefID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setPumpLightSource
 *
 */
void setPumpLightSource( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setROIID
 *
 */
void setROIID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setROIT0
 *
 */
void setROIT0( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setROIT1
 *
 */
void setROIT1( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setROIX0
 *
 */
void setROIX0( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setROIX1
 *
 */
void setROIX1( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setROIY0
 *
 */
void setROIY0( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setROIY1
 *
 */
void setROIY1( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setROIZ0
 *
 */
void setROIZ0( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setROIZ1
 *
 */
void setROIZ1( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setROIRefID
 *
 */
void setROIRefID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setReagentDescription
 *
 */
void setReagentDescription( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setReagentID
 *
 */
void setReagentID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setReagentName
 *
 */
void setReagentName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setReagentReagentIdentifier
 *
 */
void setReagentReagentIdentifier( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setRectHeight
 *
 */
void setRectHeight( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setRectID
 *
 */
void setRectID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setRectTransform
 *
 */
void setRectTransform( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setRectWidth
 *
 */
void setRectWidth( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setRectX
 *
 */
void setRectX( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setRectY
 *
 */
void setRectY( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setRegionID
 *
 */
void setRegionID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setRegionName
 *
 */
void setRegionName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setRegionTag
 *
 */
void setRegionTag( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setRoiLinkDirection
 *
 */
void setRoiLinkDirection( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setRoiLinkName
 *
 */
void setRoiLinkName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setRoiLinkRef
 *
 */
void setRoiLinkRef( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setScreenDescription
 *
 */
void setScreenDescription( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setScreenExtern
 *
 */
void setScreenExtern( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setScreenID
 *
 */
void setScreenID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setScreenName
 *
 */
void setScreenName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setScreenProtocolDescription
 *
 */
void setScreenProtocolDescription( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setScreenProtocolIdentifier
 *
 */
void setScreenProtocolIdentifier( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setScreenReagentSetDescription
 *
 */
void setScreenReagentSetDescription( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setScreenReagentSetIdentifier
 *
 */
void setScreenReagentSetIdentifier( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setScreenType
 *
 */
void setScreenType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setScreenAcquisitionEndTime
 *
 */
void setScreenAcquisitionEndTime( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setScreenAcquisitionID
 *
 */
void setScreenAcquisitionID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setScreenAcquisitionStartTime
 *
 */
void setScreenAcquisitionStartTime( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setScreenRefID
 *
 */
void setScreenRefID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setShapeBaselineShift
 *
 */
void setShapeBaselineShift( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeDirection
 *
 */
void setShapeDirection( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeFillColor
 *
 */
void setShapeFillColor( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeFillOpacity
 *
 */
void setShapeFillOpacity( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeFillRule
 *
 */
void setShapeFillRule( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeFontFamily
 *
 */
void setShapeFontFamily( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeFontSize
 *
 */
void setShapeFontSize( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeFontStretch
 *
 */
void setShapeFontStretch( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeFontStyle
 *
 */
void setShapeFontStyle( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeFontVariant
 *
 */
void setShapeFontVariant( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeFontWeight
 *
 */
void setShapeFontWeight( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeG
 *
 */
void setShapeG( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeGlyphOrientationVertical
 *
 */
void setShapeGlyphOrientationVertical( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeID
 *
 */
void setShapeID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeLocked
 *
 */
void setShapeLocked( ::jace::proxy::java::lang::Boolean p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeStrokeAttribute
 *
 */
void setShapeStrokeAttribute( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeStrokeColor
 *
 */
void setShapeStrokeColor( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeStrokeDashArray
 *
 */
void setShapeStrokeDashArray( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeStrokeLineCap
 *
 */
void setShapeStrokeLineCap( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeStrokeLineJoin
 *
 */
void setShapeStrokeLineJoin( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeStrokeMiterLimit
 *
 */
void setShapeStrokeMiterLimit( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeStrokeOpacity
 *
 */
void setShapeStrokeOpacity( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeStrokeWidth
 *
 */
void setShapeStrokeWidth( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeText
 *
 */
void setShapeText( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeTextAnchor
 *
 */
void setShapeTextAnchor( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeTextDecoration
 *
 */
void setShapeTextDecoration( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeTextFill
 *
 */
void setShapeTextFill( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeTextStroke
 *
 */
void setShapeTextStroke( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeTheT
 *
 */
void setShapeTheT( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeTheZ
 *
 */
void setShapeTheZ( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeVectorEffect
 *
 */
void setShapeVectorEffect( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeVisibility
 *
 */
void setShapeVisibility( ::jace::proxy::java::lang::Boolean p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setShapeWritingMode
 *
 */
void setShapeWritingMode( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setStageLabelName
 *
 */
void setStageLabelName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setStageLabelX
 *
 */
void setStageLabelX( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1 );

/**
 * setStageLabelY
 *
 */
void setStageLabelY( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1 );

/**
 * setStageLabelZ
 *
 */
void setStageLabelZ( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1 );

/**
 * setStagePositionPositionX
 *
 */
void setStagePositionPositionX( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setStagePositionPositionY
 *
 */
void setStagePositionPositionY( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setStagePositionPositionZ
 *
 */
void setStagePositionPositionZ( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setThumbnailHref
 *
 */
void setThumbnailHref( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setThumbnailID
 *
 */
void setThumbnailID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setThumbnailMIMEtype
 *
 */
void setThumbnailMIMEtype( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * setTiffDataFileName
 *
 */
void setTiffDataFileName( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setTiffDataFirstC
 *
 */
void setTiffDataFirstC( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setTiffDataFirstT
 *
 */
void setTiffDataFirstT( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setTiffDataFirstZ
 *
 */
void setTiffDataFirstZ( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setTiffDataIFD
 *
 */
void setTiffDataIFD( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setTiffDataNumPlanes
 *
 */
void setTiffDataNumPlanes( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setTiffDataUUID
 *
 */
void setTiffDataUUID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setTransmittanceRangeCutIn
 *
 */
void setTransmittanceRangeCutIn( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setTransmittanceRangeCutInTolerance
 *
 */
void setTransmittanceRangeCutInTolerance( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setTransmittanceRangeCutOut
 *
 */
void setTransmittanceRangeCutOut( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setTransmittanceRangeCutOutTolerance
 *
 */
void setTransmittanceRangeCutOutTolerance( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setTransmittanceRangeTransmittance
 *
 */
void setTransmittanceRangeTransmittance( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setWellColumn
 *
 */
void setWellColumn( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setWellExternalDescription
 *
 */
void setWellExternalDescription( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setWellExternalIdentifier
 *
 */
void setWellExternalIdentifier( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setWellID
 *
 */
void setWellID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setWellReagent
 *
 */
void setWellReagent( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setWellRow
 *
 */
void setWellRow( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setWellType
 *
 */
void setWellType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setWellSampleID
 *
 */
void setWellSampleID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setWellSampleImageRef
 *
 */
void setWellSampleImageRef( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setWellSampleIndex
 *
 */
void setWellSampleIndex( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setWellSamplePosX
 *
 */
void setWellSamplePosX( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setWellSamplePosY
 *
 */
void setWellSamplePosY( ::jace::proxy::java::lang::Double p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setWellSampleTimepoint
 *
 */
void setWellSampleTimepoint( ::jace::proxy::java::lang::Integer p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * setWellSampleRefID
 *
 */
void setWellSampleRefID( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
FilterMetadata( jvalue value );
FilterMetadata( jobject object );
FilterMetadata( const FilterMetadata& object );
FilterMetadata( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, meta )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::meta::FilterMetadata>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::meta::FilterMetadata>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::meta::FilterMetadata>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::meta::FilterMetadata>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::meta::FilterMetadata( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::meta::FilterMetadata>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::meta::FilterMetadata>& proxy ) : 
    ::jace::proxy::loci::formats::meta::FilterMetadata( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::meta::FilterMetadata>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::meta::FilterMetadata>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::meta::FilterMetadata>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::meta::FilterMetadata>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::meta::FilterMetadata>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::meta::FilterMetadata( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::meta::FilterMetadata>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::meta::FilterMetadata( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::meta::FilterMetadata>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::meta::FilterMetadata>& object ) : 
    ::jace::proxy::loci::formats::meta::FilterMetadata( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_META_FILTERMETADATA_H

