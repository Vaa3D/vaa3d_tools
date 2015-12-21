//=============================================================================
// Copyright © 2008 Point Grey Research, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of Point
// Grey Research, Inc. ("Confidential Information").  You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with PGR.
//
// PGR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. PGR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================
//=============================================================================
// $Id: FlyCapture2Defs_C.h,v 1.66 2010/07/20 18:40:25 arturp Exp $
//=============================================================================

#ifndef PGR_FC2_FLYCAPTURE2DEFS_C_H
#define PGR_FC2_FLYCAPTURE2DEFS_C_H

#include <stdlib.h>

//=============================================================================
// C definitions header file for FlyCapture2. 
//
// This file defines the C enumerations, typedefs and structures for FlyCapture2
//
// Please see FlyCapture2Defs.h or the API documentation for full details
// of the various enumerations and structures.
//=============================================================================

#ifdef __cplusplus
extern "C"
{
#endif

//=============================================================================
// Typedefs
//=============================================================================  

typedef int BOOL;

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifndef FULL_32BIT_VALUE
#define FULL_32BIT_VALUE 0x7FFFFFFF
#endif 

#define MAX_STRING_LENGTH   512

/**
 * A context to the FlyCapture2 C library. It must be created before
 * performing any calls to the library.
 */ 
typedef void* fc2Context;

/**
 * A context to the FlyCapture2 C GUI library. It must be created before
 * performing any calls to the library.
 */ 
typedef void* fc2GuiContext;

/**
 * An internal pointer used in the fc2Image structure.
 */ 
typedef void* fc2ImageImpl;

/**
 * A context referring to the AVI recorder object.
 */ 
typedef void* fc2AVIContext;

/**
 * A context referring to the ImageStatistics object.
 */ 
typedef void* fc2ImageStatisticsContext;

/**
 * A GUID to the camera.  It is used to uniquely identify a camera.
 */ 
typedef struct _fc2PGRGuid
{
    unsigned int value[4];

} fc2PGRGuid;

//=============================================================================
// Enumerations
//=============================================================================

typedef enum _fc2Error
{
    FC2_ERROR_UNDEFINED = -1, /**< Undefined */
    FC2_ERROR_OK, /**< Function returned with no errors. */
    FC2_ERROR_FAILED, /**< General failure. */
    FC2_ERROR_NOT_IMPLEMENTED, /**< Function has not been implemented. */
    FC2_ERROR_FAILED_BUS_MASTER_CONNECTION, /**< Could not connect to Bus Master. */
    FC2_ERROR_NOT_CONNECTED, /**< Camera has not been connected. */
    FC2_ERROR_INIT_FAILED, /**< Initialization failed. */ 
    FC2_ERROR_NOT_INTITIALIZED, /**< Camera has not been initialized. */
    FC2_ERROR_INVALID_PARAMETER, /**< Invalid parameter passed to function. */
    FC2_ERROR_INVALID_SETTINGS, /**< Setting set to camera is invalid. */         
    FC2_ERROR_INVALID_BUS_MANAGER, /**< Invalid Bus Manager object. */
    FC2_ERROR_MEMORY_ALLOCATION_FAILED, /**< Could not allocate memory. */ 
    FC2_ERROR_LOW_LEVEL_FAILURE, /**< Low level error. */
    FC2_ERROR_NOT_FOUND, /**< Device not found. */
    FC2_ERROR_FAILED_GUID, /**< GUID failure. */
    FC2_ERROR_INVALID_PACKET_SIZE, /**< Packet size set to camera is invalid. */
    FC2_ERROR_INVALID_MODE, /**< Invalid mode has been passed to function. */
    FC2_ERROR_NOT_IN_FORMAT7, /**< Error due to not being in Format7. */
    FC2_ERROR_NOT_SUPPORTED, /**< This feature is unsupported. */
    FC2_ERROR_TIMEOUT, /**< Timeout error. */
    FC2_ERROR_BUS_MASTER_FAILED, /**< Bus Master Failure. */
    FC2_ERROR_INVALID_GENERATION, /**< Generation Count Mismatch. */
    FC2_ERROR_LUT_FAILED, /**< Look Up Table failure. */
    FC2_ERROR_IIDC_FAILED, /**< IIDC failure. */
    FC2_ERROR_STROBE_FAILED, /**< Strobe failure. */
    FC2_ERROR_TRIGGER_FAILED, /**< Trigger failure. */
    FC2_ERROR_PROPERTY_FAILED, /**< Property failure. */
    FC2_ERROR_PROPERTY_NOT_PRESENT, /**< Property is not present. */
    FC2_ERROR_REGISTER_FAILED, /**< Register access failed. */
    FC2_ERROR_READ_REGISTER_FAILED, /**< Register read failed. */
    FC2_ERROR_WRITE_REGISTER_FAILED, /**< Register write failed. */
    FC2_ERROR_ISOCH_FAILED, /**< Isochronous failure. */
    FC2_ERROR_ISOCH_ALREADY_STARTED, /**< Isochronous transfer has already been started. */
    FC2_ERROR_ISOCH_NOT_STARTED, /**< Isochronous transfer has not been started. */
    FC2_ERROR_ISOCH_START_FAILED, /**< Isochronous start failed. */
    FC2_ERROR_ISOCH_RETRIEVE_BUFFER_FAILED, /**< Isochronous retrieve buffer failed. */
    FC2_ERROR_ISOCH_STOP_FAILED, /**< Isochronous stop failed. */
    FC2_ERROR_ISOCH_SYNC_FAILED, /**< Isochronous image synchronization failed. */
    FC2_ERROR_ISOCH_BANDWIDTH_EXCEEDED, /**< Isochronous bandwidth exceeded. */
    FC2_ERROR_IMAGE_CONVERSION_FAILED, /**< Image conversion failed. */
    FC2_ERROR_IMAGE_LIBRARY_FAILURE, /**< Image library failure. */
    FC2_ERROR_BUFFER_TOO_SMALL, /**< Buffer is too small. */
    FC2_ERROR_IMAGE_CONSISTENCY_ERROR, /**< There is an image consistency error. */
    FC2_ERROR_FORCE_32BITS = FULL_32BIT_VALUE

} fc2Error;  

typedef enum _fc2BusCallbackType
{
    FC2_BUS_RESET,
    FC2_ARRIVAL,
    FC2_REMOVAL,
    FC2_CALLBACK_TYPE_FORCE_32BITS = FULL_32BIT_VALUE

} fc2BusCallbackType;

typedef enum _fc2GrabMode
{
    FC2_DROP_FRAMES,
    FC2_BUFFER_FRAMES,
    FC2_UNSPECIFIED_GRAB_MODE,
    FC2_GRAB_MODE_FORCE_32BITS = FULL_32BIT_VALUE

} fc2GrabMode;

typedef enum _fc2GrabTimeout
{
    FC2_TIMEOUT_NONE = 0,
    FC2_TIMEOUT_INFINITE = -1,
    FC2_TIMEOUT_UNSPECIFIED = -2,
    FC2_GRAB_TIMEOUT_FORCE_32BITS = FULL_32BIT_VALUE

} fc2GrabTimeout;

typedef enum _fc2BandwidthAllocation
{
    FC2_BANDWIDTH_ALLOCATION_OFF = 0,
    FC2_BANDWIDTH_ALLOCATION_ON = 1,
    FC2_BANDWIDTH_ALLOCATION_UNSUPPORTED = 2,
    FC2_BANDWIDTH_ALLOCATION_UNSPECIFIED = 3,
    FC2_BANDWIDTH_ALLOCATION_FORCE_32BITS = FULL_32BIT_VALUE

}fc2BandwidthAllocation;

typedef enum _fc2InterfaceType
{        
    FC2_INTERFACE_IEEE1394,
    FC2_INTERFACE_USB_2,
	FC2_INTERFACE_GIGE,
    FC2_INTERFACE_UNKNOWN,
    FC2_INTERFACE_TYPE_FORCE_32BITS = FULL_32BIT_VALUE

} fc2InterfaceType;

typedef enum _fc2PropertyType
{
    FC2_BRIGHTNESS,
    FC2_AUTO_EXPOSURE,
    FC2_SHARPNESS,
    FC2_WHITE_BALANCE,
    FC2_HUE,
    FC2_SATURATION,
    FC2_GAMMA,
    FC2_IRIS,
    FC2_FOCUS,
    FC2_ZOOM,
    FC2_PAN,
    FC2_TILT,
    FC2_SHUTTER,
    FC2_GAIN,
    FC2_TRIGGER_MODE,
    FC2_TRIGGER_DELAY,
    FC2_FRAME_RATE,
    FC2_TEMPERATURE,
    FC2_UNSPECIFIED_PROPERTY_TYPE,
    FC2_PROPERTY_TYPE_FORCE_32BITS = FULL_32BIT_VALUE

} fc2PropertyType;

typedef enum _fc2FrameRate
{
    FC2_FRAMERATE_1_875, /**< 1.875 fps. */       
    FC2_FRAMERATE_3_75, /**< 3.75 fps. */   
    FC2_FRAMERATE_7_5, /**< 7.5 fps. */    
    FC2_FRAMERATE_15, /**< 15 fps. */   
    FC2_FRAMERATE_30, /**< 30 fps. */     
    FC2_FRAMERATE_60, /**< 60 fps. */     
    FC2_FRAMERATE_120, /**< 120 fps. */
    FC2_FRAMERATE_240, /**< 240 fps. */
    FC2_FRAMERATE_FORMAT7, /**< Custom frame rate for Format7 functionality. */
    FC2_NUM_FRAMERATES, /**< Number of possible camera frame rates. */
    FC2_FRAMERATE_FORCE_32BITS = FULL_32BIT_VALUE

} fc2FrameRate;

typedef enum _fc2VideoMode
{
    FC2_VIDEOMODE_160x120YUV444, /**< 160x120 YUV444. */
    FC2_VIDEOMODE_320x240YUV422, /**< 320x240 YUV422. */
    FC2_VIDEOMODE_640x480YUV411, /**< 640x480 YUV411. */
    FC2_VIDEOMODE_640x480YUV422, /**< 640x480 YUV422. */
    FC2_VIDEOMODE_640x480RGB, /**< 640x480 24-bit RGB. */
    FC2_VIDEOMODE_640x480Y8, /**< 640x480 8-bit. */
    FC2_VIDEOMODE_640x480Y16, /**< 640x480 16-bit. */
    FC2_VIDEOMODE_800x600YUV422, /**< 800x600 YUV422. */
    FC2_VIDEOMODE_800x600RGB, /**< 800x600 RGB. */
    FC2_VIDEOMODE_800x600Y8, /**< 800x600 8-bit. */
    FC2_VIDEOMODE_800x600Y16, /**< 800x600 16-bit. */
    FC2_VIDEOMODE_1024x768YUV422, /**< 1024x768 YUV422. */
    FC2_VIDEOMODE_1024x768RGB, /**< 1024x768 RGB. */
    FC2_VIDEOMODE_1024x768Y8, /**< 1024x768 8-bit. */
    FC2_VIDEOMODE_1024x768Y16, /**< 1024x768 16-bit. */
    FC2_VIDEOMODE_1280x960YUV422, /**< 1280x960 YUV422. */
    FC2_VIDEOMODE_1280x960RGB, /**< 1280x960 RGB. */
    FC2_VIDEOMODE_1280x960Y8, /**< 1280x960 8-bit. */
    FC2_VIDEOMODE_1280x960Y16, /**< 1280x960 16-bit. */
    FC2_VIDEOMODE_1600x1200YUV422, /**< 1600x1200 YUV422. */
    FC2_VIDEOMODE_1600x1200RGB, /**< 1600x1200 RGB. */
    FC2_VIDEOMODE_1600x1200Y8, /**< 1600x1200 8-bit. */
    FC2_VIDEOMODE_1600x1200Y16, /**< 1600x1200 16-bit. */
    FC2_VIDEOMODE_FORMAT7, /**< Custom video mode for Format7 functionality. */
    FC2_NUM_VIDEOMODES, /**< Number of possible video modes. */
    FC2_VIDEOMODE_FORCE_32BITS = FULL_32BIT_VALUE

} fc2VideoMode;

typedef enum _fc2Mode
{
    FC2_MODE_0 = 0,
    FC2_MODE_1,
    FC2_MODE_2,
    FC2_MODE_3,
    FC2_MODE_4,
    FC2_MODE_5,
    FC2_MODE_6,
    FC2_MODE_7,
    FC2_MODE_8,
    FC2_MODE_9,
    FC2_MODE_10,
    FC2_MODE_11,
    FC2_MODE_12,
    FC2_MODE_13,
    FC2_MODE_14,
    FC2_MODE_15,
    FC2_MODE_16,
    FC2_MODE_17,
    FC2_MODE_18,
    FC2_MODE_19,
    FC2_MODE_20,
    FC2_MODE_21,
    FC2_MODE_22,
    FC2_MODE_23,
    FC2_MODE_24,
    FC2_MODE_25,
    FC2_MODE_26,
    FC2_MODE_27,
    FC2_MODE_28,
    FC2_MODE_29,
    FC2_MODE_30,
    FC2_MODE_31,
    FC2_NUM_MODES, /**< Number of modes */
    FC2_MODE_FORCE_32BITS = FULL_32BIT_VALUE

} fc2Mode;  

typedef enum _fc2PixelFormat
{
    FC2_PIXEL_FORMAT_MONO8     = 0x80000000, /**< 8 bits of mono information. */
    FC2_PIXEL_FORMAT_411YUV8   = 0x40000000, /**< YUV 4:1:1. */
    FC2_PIXEL_FORMAT_422YUV8   = 0x20000000, /**< YUV 4:2:2. */
    FC2_PIXEL_FORMAT_444YUV8   = 0x10000000, /**< YUV 4:4:4. */
    FC2_PIXEL_FORMAT_RGB8      = 0x08000000, /**< R = G = B = 8 bits. */
    FC2_PIXEL_FORMAT_MONO16    = 0x04000000, /**< 16 bits of mono information. */
    FC2_PIXEL_FORMAT_RGB16     = 0x02000000, /**< R = G = B = 16 bits. */
    FC2_PIXEL_FORMAT_S_MONO16  = 0x01000000, /**< 16 bits of signed mono information. */
    FC2_PIXEL_FORMAT_S_RGB16   = 0x00800000, /**< R = G = B = 16 bits signed. */
    FC2_PIXEL_FORMAT_RAW8      = 0x00400000, /**< 8 bit raw data output of sensor. */
    FC2_PIXEL_FORMAT_RAW16     = 0x00200000, /**< 16 bit raw data output of sensor. */
    FC2_PIXEL_FORMAT_MONO12    = 0x00100000, /**< 12 bits of mono information. */
    FC2_PIXEL_FORMAT_RAW12     = 0x00080000, /**< 12 bit raw data output of sensor. */
    FC2_PIXEL_FORMAT_BGR       = 0x80000008, /**< 24 bit BGR. */
    FC2_PIXEL_FORMAT_BGRU      = 0x40000008, /**< 32 bit BGRU. */
    FC2_PIXEL_FORMAT_RGB       = FC2_PIXEL_FORMAT_RGB8, /**< 24 bit RGB. */
    FC2_PIXEL_FORMAT_RGBU      = 0x40000002, /**< 32 bit RGBU. */
    FC2_NUM_PIXEL_FORMATS	      =  15, /**< Number of pixel formats. */
    FC2_UNSPECIFIED_PIXEL_FORMAT = 0 /**< Unspecified pixel format. */

} fc2PixelFormat;

typedef enum _fc2BusSpeed
{
    FC2_BUSSPEED_S100, /**< 100Mbits/sec. */
    FC2_BUSSPEED_S200, /**< 200Mbits/sec. */
    FC2_BUSSPEED_S400, /**< 400Mbits/sec. */
    FC2_BUSSPEED_S480, /**< 480Mbits/sec. Only for USB cameras. */
    FC2_BUSSPEED_S800, /**< 800Mbits/sec. */
    FC2_BUSSPEED_S1600, /**< 1600Mbits/sec. */
    FC2_BUSSPEED_S3200, /**< 3200Mbits/sec. */
    FC2_BUSSPEED_10BASE_T, /**< 10Base-T. Only for GigE cameras. */
    FC2_BUSSPEED_100BASE_T, /**< 100Base-T.  Only for GigE cameras.*/
    FC2_BUSSPEED_1000BASE_T, /**< 1000Base-T (Gigabit Ethernet).  Only for GigE cameras. */
    FC2_BUSSPEED_10000BASE_T, /**< 10000Base-T.  Only for GigE cameras. */
    FC2_BUSSPEED_S_FASTEST, /**< The fastest speed available. */
    FC2_BUSSPEED_ANY, /**< Any speed that is available. */
    FC2_BUSSPEED_SPEED_UNKNOWN = -1, /**< Unknown bus speed. */
    FC2_BUSSPEED_FORCE_32BITS = FULL_32BIT_VALUE

} fc2BusSpeed;

typedef enum _fc2ColorProcessingAlgorithm
{
    FC2_DEFAULT,
    FC2_NO_COLOR_PROCESSING,
    FC2_NEAREST_NEIGHBOR_FAST,
    FC2_EDGE_SENSING,
    FC2_HQ_LINEAR,
    FC2_RIGOROUS,
    FC2_COLOR_PROCESSING_ALGORITHM_FORCE_32BITS = FULL_32BIT_VALUE

} fc2ColorProcessingAlgorithm;

typedef enum _fc2BayerTileFormat
{
    FC2_BT_NONE, /**< No bayer tile format. */
    FC2_BT_RGGB, /**< Red-Green-Green-Blue. */
    FC2_BT_GRBG, /**< Green-Red-Blue-Green. */
    FC2_BT_GBRG, /**< Green-Blue-Red-Green. */
    FC2_BT_BGGR, /**< Blue-Green-Green-Red. */
    FC2_BT_FORCE_32BITS = FULL_32BIT_VALUE

} fc2BayerTileFormat;

typedef enum _fc2ImageFileFormat
{
    FC2_FROM_FILE_EXT = -1, /**< Determine file format from file extension. */
    FC2_PGM, /**< Portable gray map. */
    FC2_PPM, /**< Portable pixmap. */
    FC2_BMP, /**< Bitmap. */
    FC2_JPEG, /**< JPEG. */
    FC2_JPEG2000, /**< JPEG 2000. */
    FC2_TIFF, /**< Tagged image file format. */
    FC2_PNG, /**< Portable network graphics. */
    FC2_RAW, /**< Raw data. */
    FC2_IMAGE_FILE_FORMAT_FORCE_32BITS = FULL_32BIT_VALUE

} fc2ImageFileFormat;

typedef enum _fc2GigEPropertyType
{
    FC2_HEARTBEAT,
    FC2_HEARTBEAT_TIMEOUT

} fc2GigEPropertyType;

typedef enum _fc2StatisticsChannel
{
    FC2_STATISTICS_GREY,
    FC2_STATISTICS_RED,
    FC2_STATISTICS_GREEN,
    FC2_STATISTICS_BLUE,
    FC2_STATISTICS_HUE,
    FC2_STATISTICS_SATURATION,
    FC2_STATISTICS_LIGHTNESS,
    FC2_STATISTICS_FORCE_32BITS = FULL_32BIT_VALUE
} fc2StatisticsChannel;


typedef enum _fc2OSType
{
	FC2_WINDOWS_X86,
	FC2_WINDOWS_X64,
	FC2_LINUX_X86,
	FC2_LINUX_X64,
	FC2_MAC,
	FC2_UNKNOWN_OS,
    FC2_OSTYPE_FORCE_32BITS = FULL_32BIT_VALUE
} fc2OSType;

typedef enum _fc2ByteOrder
{
	FC2_BYTE_ORDER_LITTLE_ENDIAN,
	FC2_BYTE_ORDER_BIG_ENDIAN,
    FC2_BYTE_ORDER_FORCE_32BITS = FULL_32BIT_VALUE
} fc2ByteOrder;

//=============================================================================
// Structures
//=============================================================================

//
// Description:
//	 An image. It is comparable to the Image class in the C++ library.
//   The fields in this structure should be considered read only.
//
typedef struct _fc2Image
{
    unsigned int rows;
    unsigned int cols;
    unsigned int stride;
    unsigned char* pData;
    unsigned int dataSize;
    fc2PixelFormat format;
    fc2BayerTileFormat bayerFormat;

    fc2ImageImpl imageImpl;

} fc2Image;

typedef struct _fc2SystemInfo
{
	fc2OSType osType;
	char osDescription[ MAX_STRING_LENGTH];
	fc2ByteOrder byteOrder;
	size_t	sysMemSize;
	char cpuDescription[ MAX_STRING_LENGTH];
	size_t	numCpuCores;
	char driverList[ MAX_STRING_LENGTH];
	char libraryList[ MAX_STRING_LENGTH];
	char gpuDescription[ MAX_STRING_LENGTH];
	size_t screenWidth;
	size_t screenHeight;
    unsigned int reserved[16];

} fc2SystemInfo;

typedef struct _fc2Version
{
    unsigned int major;
    unsigned int minor;
    unsigned int type;
    unsigned int build;
} fc2Version;

typedef struct _fc2Config
{
    unsigned int numBuffers;
    unsigned int numImageNotifications;
    int grabTimeout;
    fc2GrabMode grabMode; 
    fc2BusSpeed isochBusSpeed;
    fc2BusSpeed asyncBusSpeed;
    fc2BandwidthAllocation bandwidthAllocation;
    unsigned int reserved[16];

} fc2Config;

typedef struct _fc2PropertyInfo
{
    fc2PropertyType type;
    BOOL present;
    BOOL autoSupported;
    BOOL manualSupported;
    BOOL onOffSupported;
    BOOL onePushSupported;
    BOOL absValSupported;
    BOOL readOutSupported;
    unsigned int min;
    unsigned int max;
    float absMin;
    float absMax;
    char pUnits[MAX_STRING_LENGTH];
    char pUnitAbbr[MAX_STRING_LENGTH];
    unsigned int reserved[8];

} fc2PropertyInfo, fc2TriggerDelayInfo;   

typedef struct _Property
{
    fc2PropertyType   type;
    BOOL present;
    BOOL absControl;
    BOOL onePush;
    BOOL onOff;
    BOOL autoManualMode;
    unsigned int valueA;   
    unsigned int valueB;   
    float absValue;
    unsigned int reserved[8];

    // For convenience, trigger delay is the same structure
    // used in a separate function along with trigger mode.

} fc2Property, fc2TriggerDelay;

typedef struct _fc2TriggerModeInfo
{
    BOOL present;
    BOOL readOutSupported;
    BOOL onOffSupported;
    BOOL polaritySupported;
    BOOL valueReadable;
    unsigned int sourceMask;
    BOOL softwareTriggerSupported;
    unsigned int modeMask;
    unsigned int reserved[8];

} fc2TriggerModeInfo;

typedef struct _fc2TriggerMode
{      
    BOOL onOff;
    unsigned int polarity;
    unsigned int source;
    unsigned int mode;
    unsigned int parameter;      
    unsigned int reserved[8];

} fc2TriggerMode;

typedef struct _fc2StrobeInfo
{
    unsigned int source;
    BOOL present;
    BOOL readOutSupported;
    BOOL onOffSupported;
    BOOL polaritySupported;
    float minValue;
    float maxValue;
    unsigned int reserved[8];

} fc2StrobeInfo;

typedef struct _fc2StrobeControl
{      
    unsigned int source;
    BOOL onOff;
    unsigned int polarity;
    float delay;
    float duration;
    unsigned int reserved[8];

} fc2StrobeControl;

typedef struct _fc2Format7ImageSettings
{
    fc2Mode mode;
    unsigned int offsetX;
    unsigned int offsetY;
    unsigned int width;
    unsigned int height;
    fc2PixelFormat pixelFormat;
    unsigned int reserved[8];

} fc2Format7ImageSettings;

typedef struct _fc2Format7Info
{
    fc2Mode         mode;

    unsigned int maxWidth;
    unsigned int maxHeight;
    unsigned int offsetHStepSize;
    unsigned int offsetVStepSize;
    unsigned int imageHStepSize;
    unsigned int imageVStepSize;
    unsigned int pixelFormatBitField;
    unsigned int packetSize;
    unsigned int minPacketSize;
    unsigned int maxPacketSize;
    float percentage;
    unsigned int reserved[16];

} fc2Format7Info;

typedef struct _fc2Format7PacketInfo
{
    unsigned int recommendedBytesPerPacket;
    unsigned int maxBytesPerPacket;
    unsigned int unitBytesPerPacket;
    unsigned int reserved[8];

} fc2Format7PacketInfo;

typedef struct _fc2IPAddress
{
    unsigned char octets[4];
} fc2IPAddress;

typedef struct _fc2MACAddress
{
    unsigned char octets[6];
} fc2MACAddress;

typedef struct _fc2GigEProperty
{
    fc2GigEPropertyType propType;        
    BOOL isReadable;
    BOOL isWritable;
    unsigned int min;
    unsigned int max;
    unsigned int value;

    unsigned int reserved[8];
} fc2GigEProperty;

typedef struct _fc2GigEStreamChannel
{
    unsigned int networkInterfaceIndex;
    unsigned int hostPost;
    BOOL doNotFragment;
    unsigned int packetSize;
    unsigned int interPacketDelay;      
    fc2IPAddress destinationIpAddress;
    unsigned int sourcePort;

    unsigned int reserved[8];
} fc2GigEStreamChannel;

typedef struct _fc2GigEConfig
{
    unsigned int numChannels;
    fc2GigEStreamChannel channels[512];

    unsigned int reserved[8];
} fc2GigEConfig;

typedef struct _fc2GigEImageSettingsInfo
{
    unsigned int maxWidth;
    unsigned int maxHeight;
    unsigned int offsetHStepSize;
    unsigned int offsetVStepSize;
    unsigned int imageHStepSize;
    unsigned int imageVStepSize;
    unsigned int pixelFormatBitField;

    unsigned int reserved[16];
} fc2GigEImageSettingsInfo;

typedef struct _fc2GigEImageSettings
{
    unsigned int offsetX;
    unsigned int offsetY;
    unsigned int width;
    unsigned int height;
    fc2PixelFormat pixelFormat;

    unsigned int reserved[8];
} fc2GigEImageSettings;

typedef struct _fc2TimeStamp
{
    long long seconds;
    unsigned int microSeconds;
    unsigned int cycleSeconds;
    unsigned int cycleCount;
    unsigned int cycleOffset;
    unsigned int reserved[8];

} fc2TimeStamp;

typedef struct _fc2ConfigROM
{
    unsigned int nodeVendorId;
    unsigned int chipIdHi;
    unsigned int chipIdLo;
    unsigned int unitSpecId;
    unsigned int unitSWVer;
    unsigned int unitSubSWVer;
    unsigned int vendorUniqueInfo_0;
    unsigned int vendorUniqueInfo_1;
    unsigned int vendorUniqueInfo_2;
    unsigned int vendorUniqueInfo_3;
    char pszKeyword[ MAX_STRING_LENGTH ];
    unsigned int reserved[16];

} fc2ConfigROM;

typedef struct _fc2CameraInfo
{      
    unsigned int serialNumber;
    fc2InterfaceType interfaceType;
    BOOL isColorCamera;
    char modelName[ MAX_STRING_LENGTH];
    char vendorName[ MAX_STRING_LENGTH];
    char sensorInfo[ MAX_STRING_LENGTH];
    char sensorResolution[ MAX_STRING_LENGTH];
    char driverName[ MAX_STRING_LENGTH];
    char firmwareVersion[ MAX_STRING_LENGTH];
    char firmwareBuildTime[ MAX_STRING_LENGTH];
    fc2BusSpeed maximumBusSpeed;
    fc2BayerTileFormat bayerTileFormat;

    // IIDC specific information
    unsigned int iidcVer;
    fc2ConfigROM configROM;

    // GigE specific information
    unsigned int gigEMajorVersion;
    unsigned int gigEMinorVersion;
    char userDefinedName[ MAX_STRING_LENGTH];
    char xmlURL1[ MAX_STRING_LENGTH];
    char xmlURL2[ MAX_STRING_LENGTH];
    fc2MACAddress macAddress;
    fc2IPAddress ipAddress;
    fc2IPAddress subnetMask;
    fc2IPAddress defaultGateway;

    unsigned int reserved[16];

} fc2CameraInfo;

typedef struct _fc2EmbeddedImageInfoProperty
{
    BOOL available;
    BOOL onOff;

} fc2EmbeddedImageInfoProperty;

typedef struct _fc2EmbeddedImageInfo
{
    fc2EmbeddedImageInfoProperty timestamp;
    fc2EmbeddedImageInfoProperty gain;
    fc2EmbeddedImageInfoProperty shutter;
    fc2EmbeddedImageInfoProperty brightness;
    fc2EmbeddedImageInfoProperty exposure;
    fc2EmbeddedImageInfoProperty whiteBalance;
    fc2EmbeddedImageInfoProperty frameCounter;
    fc2EmbeddedImageInfoProperty strobePattern;
    fc2EmbeddedImageInfoProperty GPIOPinState;
    fc2EmbeddedImageInfoProperty ROIPosition;

} fc2EmbeddedImageInfo;

typedef struct _fc2ImageMetadata
{
    unsigned int embeddedTimeStamp;
    unsigned int embeddedGain;
    unsigned int embeddedShutter;
    unsigned int embeddedBrightness;
    unsigned int embeddedExposure;
    unsigned int embeddedWhiteBalance;
    unsigned int embeddedFrameCounter;
    unsigned int embeddedStrobePattern;
    unsigned int embeddedGPIOPinState;
    unsigned int embeddedROIPosition;        
    unsigned int reserved[31];

} fc2ImageMetadata;

typedef struct _fc2LUTData
{
    BOOL supported;
    BOOL enabled;
    unsigned int numBanks;
    unsigned int numChannels;
    unsigned int inputBitDepth;
    unsigned int outputBitDepth;
    unsigned int numEntries;
    unsigned int reserved[8];

} fc2LUTData;

typedef struct _fc2PNGOption
{
    BOOL interlaced; 
    unsigned int compressionLevel;
    unsigned int reserved[16];

} fc2PNGOption;

typedef struct _fc2PPMOption
{
    BOOL binaryFile;
    unsigned int reserved[16];

} fc2PPMOption ;

typedef struct _fc2PGMOption
{
    BOOL binaryFile;
    unsigned int reserved[16];

} fc2PGMOption;

typedef enum _fc2TIFFCompressionMethod
{
    FC2_TIFF_NONE = 1,
    FC2_TIFF_PACKBITS,
    FC2_TIFF_DEFLATE,
    FC2_TIFF_ADOBE_DEFLATE,
    FC2_TIFF_CCITTFAX3,
    FC2_TIFF_CCITTFAX4,
    FC2_TIFF_LZW,
    FC2_TIFF_JPEG,
} fc2TIFFCompressionMethod;

typedef struct _fc2TIFFOption
{
    fc2TIFFCompressionMethod compression; 
    unsigned int reserved[16];

} fc2TIFFOption;

typedef struct _fc2JPEGOption
{
    BOOL progressive; 
    unsigned int quality;
    unsigned int reserved[16];

} fc2JPEGOption;

typedef struct _fc2JPG2Option
{
    unsigned int quality;
    unsigned int reserved[16];
} fc2JPG2Option;

typedef struct _fc2AVIOption
{
   float frameRate;
   unsigned int reserved[256];

} fc2AVIOption;

//=============================================================================
// Callbacks
//=============================================================================

typedef void* fc2CallbackHandle;
typedef void (*fc2BusEventCallback)( void* pParameter, unsigned int serialNumber );
typedef void (*fc2ImageEventCallback)( fc2Image* image, void* pCallbackData );
typedef void (*fc2AsyncCommandCallback)( fc2Error retError, void* pUserData );

#ifdef __cplusplus
};
#endif

#endif // PGR_FC2_FLYCAPTURE2DEFS_C_H

