//=============================================================================
// Copyright © 2001-2006 Point Grey Research, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of Point
// Grey Research, Inc. ("Confidential Information").  You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with Point Grey Research Inc.
//
// PGR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. PGR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================

//=============================================================================
//
// PGRFlyCapture.h
//
//   Defines the API to the PGR FlyCapture library.
//
//  We welcome your bug reports, suggestions, and comments:
//  www.ptgrey.com/support/contact
//
//=============================================================================

//=============================================================================
// $Id: PGRFlyCapture.h,v 1.1 2009/04/30 17:29:30 soowei Exp $
//=============================================================================
#ifndef __PGRFLYCAPTURE_H__
#define __PGRFLYCAPTURE_H__

#ifdef PGRFLYCAPTURE_EXPORTS
#define PGRFLYCAPTURE_API __declspec( dllexport )
#else
#define PGRFLYCAPTURE_API __declspec( dllimport )
#endif

#define PGRFLYCAPTURE_CALL_CONVEN __cdecl


#ifdef __cplusplus
extern "C"
{
#endif


//
// Description:
//  The version of the library.
//
#define PGRFLYCAPTURE_VERSION 108107


//
// Description:
//   Context pointer for the PGRFlyCapture library.
//
typedef void* FlyCaptureContext;


//
// Description:
//  The error codes returned by the functions in this library.
//
typedef enum FlyCaptureError
{
   // Function completed successfully.
   FLYCAPTURE_OK,
   // General failure.
   FLYCAPTURE_FAILED,
   // Invalid argument passed.
   FLYCAPTURE_INVALID_ARGUMENT,
   // Invalid context passed.
   FLYCAPTURE_INVALID_CONTEXT,
   // Function not implemented.
   FLYCAPTURE_NOT_IMPLEMENTED,
   // Device already initialized.
   FLYCAPTURE_ALREADY_INITIALIZED,
   // Grabbing has already been started.
   FLYCAPTURE_ALREADY_STARTED,
   // Callback is not registered
   FLYCAPTURE_CALLBACK_NOT_REGISTERED,
   // Callback is already registered
   FLYCAPTURE_CALLBACK_ALREADY_REGISTERED,
   // Problem controlling camera.
   FLYCAPTURE_CAMERACONTROL_PROBLEM,
   // Failed to open file.
   FLYCAPTURE_COULD_NOT_OPEN_FILE,
   // Failed to open a device handle.
   FLYCAPTURE_COULD_NOT_OPEN_DEVICE_HANDLE,
   // Memory allocation error
   FLYCAPTURE_MEMORY_ALLOC_ERROR,
   // flycaptureGrabImage() not called.
   FLYCAPTURE_NO_IMAGE,
   // Device not initialized.
   FLYCAPTURE_NOT_INITIALIZED,
   // flycaptureStart() not called.
   FLYCAPTURE_NOT_STARTED,
   // Request would exceed maximum bandwidth.
   FLYCAPTURE_MAX_BANDWIDTH_EXCEEDED,
   // Attached camera is not a PGR camera.
   FLYCAPTURE_NON_PGR_CAMERA,
   // Invalid video mode or framerate passed or retrieved.
   FLYCAPTURE_INVALID_MODE,
   // Unknown error.
   FLYCAPTURE_ERROR_UNKNOWN,
   // Invalid custom size.
   FLYCAPTURE_INVALID_CUSTOM_SIZE,
   // Operation timed out.
   FLYCAPTURE_TIMEOUT,
   // Too many image buffers are locked by the user.
   FLYCAPTURE_TOO_MANY_LOCKED_BUFFERS,
   // There is a version mismatch between one of the interacting modules:
   // pgrflycapture.dll, pgrflycapturegui.dll, and the camera driver.
   FLYCAPTURE_VERSION_MISMATCH,
   // The camera responded that it is currently busy.
   FLYCAPTURE_DEVICE_BUSY,
   // Function has been deprecated.  Please see documentation.
   FLYCAPTURE_DEPRECATED,
   // Supplied User Buffer is too small.
   FLYCAPTURE_BUFFER_SIZE_TOO_SMALL,


} FlyCaptureError;


//
// Description:
//  An enumeration of the different camera properties that can be set via the
//  API.
//
// Remarks:
//  A lot of these properties are included only for completeness and future
//  expandability, and will have no effect on a PGR camera.
//
typedef enum FlyCaptureProperty
{
   // The brightness property of the camera.
   FLYCAPTURE_BRIGHTNESS,
   // The auto exposure property of the camera.
   FLYCAPTURE_AUTO_EXPOSURE,
   // The sharpness property of the camera.
   FLYCAPTURE_SHARPNESS,
   // The hardware white balance property of the camera.
   FLYCAPTURE_WHITE_BALANCE,
   // The hue property of the camera.
   FLYCAPTURE_HUE,
   // The saturation property of the camera.
   FLYCAPTURE_SATURATION,
   // The gamma property of the camera.
   FLYCAPTURE_GAMMA,
   // The iris property of the camera.
   FLYCAPTURE_IRIS,
   // The focus property of the camera.
   FLYCAPTURE_FOCUS,
   // The zoom property of the camera.
   FLYCAPTURE_ZOOM,
   // The pan property of the camera.
   FLYCAPTURE_PAN,
   // The tilt property of the camera.
   FLYCAPTURE_TILT,
   // The shutter property of the camera.
   FLYCAPTURE_SHUTTER,
   // The gain  property of the camera.
   FLYCAPTURE_GAIN,
   // The trigger delay property of the camera.
   FLYCAPTURE_TRIGGER_DELAY,
   // The frame rate property of the camera.
   FLYCAPTURE_FRAME_RATE,

   //
   // Software white balance property. Use this to manipulate the
   // values for software whitebalance.  This is only applicable to cameras
   // that do not do onboard color processing.  On these cameras, hardware
   // white balance is disabled.
   //
   FLYCAPTURE_SOFTWARE_WHITEBALANCE,
   // The temperature property of the camera
   FLYCAPTURE_TEMPERATURE,

} FlyCaptureProperty;

//
// Description:
//  The type used to store the serial number uniquely identifying a FlyCapture
//  camera.
//
typedef unsigned long FlyCaptureCameraSerialNumber;

//
// Description:
//  An enumeration of the different type of bus events.
//
typedef enum FlyCaptureBusEvent
{
   // A message returned from the bus callback mechanism indicating a bus reset.
   FLYCAPTURE_MESSAGE_BUS_RESET = 0x02,
   // A message returned from the bus callback mechanism indicating a device has
   // arrived on the bus.
   FLYCAPTURE_MESSAGE_DEVICE_ARRIVAL,
   // A message returned from the bus callback mechanism indicating a device has
   // been removed from the bus.
   FLYCAPTURE_MESSAGE_DEVICE_REMOVAL,

} FlyCaptureBusEvent;


//
// Function prototype for the bus callback mechanism.  pParam contains the
// parameter passed in when registering the callback.  iMessage is one of the
// above FLYCAPTURE_MESSAGE_* #defines and ulParam is a message-defined
// parameter.
//
// See also: flycaptureModifyCallback()
//
typedef void __cdecl
FlyCaptureCallback( void* pParam, int iMessage, unsigned long ulParam );


//
// A value indicating an infinite wait.  This macro is used primarily used with
// the flycaptureSetGrabTimeoutEx() in order to indicate the software should
// wait indefinitely for the camera to produce an image.
//
#define FLYCAPTURE_INFINITE   0xFFFFFFFF


//
// Description:
//   Enum describing different framerates.
//
typedef enum FlyCaptureFrameRate
{
   // 1.875 fps. (Frames per second)
   FLYCAPTURE_FRAMERATE_1_875,
   // 3.75 fps.
   FLYCAPTURE_FRAMERATE_3_75,
   // 7.5 fps.
   FLYCAPTURE_FRAMERATE_7_5,
   // 15 fps.
   FLYCAPTURE_FRAMERATE_15,
   // 30 fps.
   FLYCAPTURE_FRAMERATE_30,
   // Deprecated.  Please use Custom image.
   FLYCAPTURE_FRAMERATE_UNUSED,
   // 60 fps.
   FLYCAPTURE_FRAMERATE_60,
   // 120 fps.
   FLYCAPTURE_FRAMERATE_120,
   // 240 fps.
   FLYCAPTURE_FRAMERATE_240,
   // Number of possible camera frame rates.
   FLYCAPTURE_NUM_FRAMERATES,
   // Custom frame rate.  Used with custom image size functionality.
   FLYCAPTURE_FRAMERATE_CUSTOM,
   // Hook for "any usable frame rate."
   FLYCAPTURE_FRAMERATE_ANY,

} FlyCaptureFrameRate;


//
// Description:
//   Enum describing different video modes.
//
// Remarks:
//   The explicit numbering is to provide downward compatibility for this enum.
//
typedef enum FlyCaptureVideoMode
{
   // 160x120 YUV444.
   FLYCAPTURE_VIDEOMODE_160x120YUV444     = 0,
   // 320x240 YUV422.
   FLYCAPTURE_VIDEOMODE_320x240YUV422     = 1,
   // 640x480 YUV411.
   FLYCAPTURE_VIDEOMODE_640x480YUV411     = 2,
   // 640x480 YUV422.
   FLYCAPTURE_VIDEOMODE_640x480YUV422     = 3,
   // 640x480 24-bit RGB.
   FLYCAPTURE_VIDEOMODE_640x480RGB        = 4,
   // 640x480 8-bit greyscale or bayer tiled color image.
   FLYCAPTURE_VIDEOMODE_640x480Y8         = 5,
   // 640x480 16-bit greyscale or bayer tiled color image.
   FLYCAPTURE_VIDEOMODE_640x480Y16        = 6,
   // 800x600 YUV422.
   FLYCAPTURE_VIDEOMODE_800x600YUV422     = 17,
   // 800x600 RGB.
   FLYCAPTURE_VIDEOMODE_800x600RGB        = 18,
   // 800x600 8-bit greyscale or bayer tiled color image.
   FLYCAPTURE_VIDEOMODE_800x600Y8         = 7,
   // 800x600 16-bit greyscale or bayer tiled color image.
   FLYCAPTURE_VIDEOMODE_800x600Y16        = 19,
   // 1024x768 YUV422.
   FLYCAPTURE_VIDEOMODE_1024x768YUV422    = 20,
   // 1024x768 RGB.
   FLYCAPTURE_VIDEOMODE_1024x768RGB       = 21,
   // 1024x768 8-bit greyscale or bayer tiled color image.
   FLYCAPTURE_VIDEOMODE_1024x768Y8        = 8,
   // 1024x768 16-bit greyscale or bayer tiled color image.
   FLYCAPTURE_VIDEOMODE_1024x768Y16       = 9,
   // 1280x960 YUV422.
   FLYCAPTURE_VIDEOMODE_1280x960YUV422    = 22,
   // 1280x960 RGB.
   FLYCAPTURE_VIDEOMODE_1280x960RGB       = 23,
   // 1280x960 8-bit greyscale or bayer titled color image.
   FLYCAPTURE_VIDEOMODE_1280x960Y8        = 10,
   // 1280x960 16-bit greyscale or bayer titled color image.
   FLYCAPTURE_VIDEOMODE_1280x960Y16       = 24,
   // 1600x1200 YUV422.
   FLYCAPTURE_VIDEOMODE_1600x1200YUV422   = 50,
   // 1600x1200 RGB.
   FLYCAPTURE_VIDEOMODE_1600x1200RGB      = 51,
   // 1600x1200 8-bit greyscale or bayer titled color image.
   FLYCAPTURE_VIDEOMODE_1600x1200Y8       = 11,
   // 1600x1200 16-bit greyscale or bayer titled color image.
   FLYCAPTURE_VIDEOMODE_1600x1200Y16      = 52,

   // Custom video mode.  Used with custom image size functionality.
   FLYCAPTURE_VIDEOMODE_CUSTOM            = 15,
   // Hook for "any usable video mode."
   FLYCAPTURE_VIDEOMODE_ANY               = 16,

   // Number of possible video modes.
   FLYCAPTURE_NUM_VIDEOMODES              = 23,

} FlyCaptureVideoMode;


//
// Description:
//  An enumeration used to describe the different camera models that can be
//  accessed through this SDK.
//
typedef enum FlyCaptureCameraModel
{
   FLYCAPTURE_FIREFLY,
   FLYCAPTURE_DRAGONFLY,
   FLYCAPTURE_AIM,
   FLYCAPTURE_SCORPION,
   FLYCAPTURE_TYPHOON,
   FLYCAPTURE_FLEA,
   FLYCAPTURE_DRAGONFLY_EXPRESS,
   FLYCAPTURE_FLEA2,
   FLYCAPTURE_FIREFLY_MV,
   FLYCAPTURE_DRAGONFLY2,
   FLYCAPTURE_BUMBLEBEE,
   FLYCAPTURE_BUMBLEBEE2,
   FLYCAPTURE_BUMBLEBEEXB3,
   FLYCAPTURE_GRASSHOPPER,
   FLYCAPTURE_CHAMELEON,
   FLYCAPTURE_UNKNOWN = -1,

   // Unused member to force this enum to compile to 32 bits.
   FCCM_FORCE_QUADLET   = 0x7FFFFFFF,
} FlyCaptureCameraModel;


//
// Description:
//  An enumeration used to describe the different camera color configurations.
//
typedef enum FlyCaptureCameraType
{
   // black and white system.
   FLYCAPTURE_BLACK_AND_WHITE,
   // color system.
   FLYCAPTURE_COLOR

} FlyCaptureCameraType;

//
// Description:
//  An enumeration used to describe the bus speed
//
typedef enum FlyCaptureBusSpeed
{
   // 100Mbits/sec.
   FLYCAPTURE_S100,
   // 200Mbits/sec.
   FLYCAPTURE_S200,
   // 400Mbits/sec.
   FLYCAPTURE_S400,
   // 480Mbits/sec. USB
   FLYCAPTURE_S480,
   // 800Mbits/sec.
   FLYCAPTURE_S800,
   // 1600Mbits/sec.
   FLYCAPTURE_S1600,
   // 3200Mbits/sec.
   FLYCAPTURE_S3200,
   // The fastest speed available.
   FLYCAPTURE_S_FASTEST,
   // Any speed that is available.
   FLYCAPTURE_ANY,
   FLYCAPTURE_SPEED_UNKNOWN = -1,
   // Unused member to force this enum to compile to 32 bits.
   FLYCAPTURE_SPEED_FORCE_QUADLET   = 0x7FFFFFFF,

} FlyCaptureBusSpeed;

//
// Description:
//  This structure stores a variety of different pieces of information
//  associated with a particular camera.  It is used with the
//  flycaptureBusEnumerateCamerasEx() method.  This structure has replaced
//  FlyCaptureInfo.
//
typedef struct FlyCaptureInfoEx
{
   // Camera serial number.
   FlyCaptureCameraSerialNumber	 SerialNumber;
   // Type of imager (color or b&w).
   FlyCaptureCameraType		 CameraType;
   // Camera model.
   FlyCaptureCameraModel         CameraModel;
   // Camera model string.  Null terminated.
   char  pszModelName[ 512 ];
   // Vendor name string.  Null terminated.
   char  pszVendorName[ 512 ];
   // Sensor info string.  Null terminated.
   char  pszSensorInfo[ 512 ];
   // 1394 DCAM compliance level.  DCAM version is this value / 100. eg, 1.31.
   int   iDCAMVer;
   // Low-level 1394 node number for this device.
   int   iNodeNum;
   // Low-level 1394 bus number for this device.
   int   iBusNum;
   // Camera max bus speed
   FlyCaptureBusSpeed		 CameraMaxBusSpeed;
   // Flag indicating that the camera is already initialized
   int	 iInitialized;

   // Reserved for future data.
   unsigned long  ulReserved[ 115 ];

} FlyCaptureInfoEx;

//
// Description:
//  This structure stores some extra driver info not stored on FlyCaptureInfoEx
//
typedef struct FlyCaptureDriverInfo
{
   // Null-terminated driver name for attached camera.
   char pszDriverName[ 512 ];
   //  Null-terminated driver Driver version
   char pszVersion [ 512 ];

} FlyCaptureDriverInfo;

//
// Description:
//   An enumeration used to describe the different color processing
//   methods.
//
// Remarks:
//   This is only relevant for cameras that do not do onboard color
//   processing, such as the Dragonfly.  The FLYCAPTURE_RIGOROUS
//   method is very slow and will not keep up with high frame rates.
//
typedef enum FlyCaptureColorMethod
{
   // Disable color processing.
   FLYCAPTURE_DISABLE,
   // Edge sensing de-mosaicing.  This is the most accurate method
   // that can still keep up with the camera's frame rate.
   FLYCAPTURE_EDGE_SENSING,
   // Nearest neighbor de-mosaicing.  This algorithm is significantly
   // faster than edge sensing, at the cost of accuracy.
   // Please note The Nearest Neighbor method has been remapped internally to 
   // Nearest Neighbor Fast due to observed artifacts with the original method.
   FLYCAPTURE_NEAREST_NEIGHBOR,
   // Faster, less accurate nearest neighbor de-mosaicing.
   FLYCAPTURE_NEAREST_NEIGHBOR_FAST,
   // Rigorous de-mosaicing.  This provides the best quality color
   // reproduction.  This method is so processor intensive that it
   // might not keep up with the camera's frame rate.  Best used for
   // offline processing where accurate color reproduction is required.
   FLYCAPTURE_RIGOROUS,   
   // High quality linear interpolation. This algorithm provides similar
   // results to Rigorous, but is up to 30 times faster.
   FLYCAPTURE_HQLINEAR,
} FlyCaptureColorMethod;


//
// Description:
//   An enumeration used to indicate the Bayer tile format of the stippled
//   images passed into a destippling function.
//
// Remarks:
//   This is only relevant for cameras that do not do onboard color
//   processing, such as the Dragonfly.  The four letters of the enum
//   value correspond to the "top left" 2x2 section of the stippled image.
//   For example, the first line of a BGGR image image will be
//   BGBGBG..., and the second line will be GRGRGR....
//
typedef enum FlyCaptureStippledFormat
{
   // Indicates a BGGR image.
   FLYCAPTURE_STIPPLEDFORMAT_BGGR,
   // Indicates a GBRG image.
   FLYCAPTURE_STIPPLEDFORMAT_GBRG,
   // Indicates a GRBG image.
   FLYCAPTURE_STIPPLEDFORMAT_GRBG,
   // Indicates a RGGB image.
   FLYCAPTURE_STIPPLEDFORMAT_RGGB,
   // Indicates the default stipple format for the Dragonfly or Firefly.
   FLYCAPTURE_STIPPLEDFORMAT_DEFAULT

} FlyCaptureStippledFormat;


//
// Description:
//   An enumeration used to indicate the pixel format of an image.  This
//   enumeration is used as a member of FlyCaptureImage and as a parameter
//   to FlyCaptureStartCustomImage().
//
typedef enum FlyCapturePixelFormat
{
   // 8 bit of mono.
   FLYCAPTURE_MONO8     = 0x00000001,
   // YUV 4:1:1.
   FLYCAPTURE_411YUV8   = 0x00000002,
   // YUV 4:2:2.
   FLYCAPTURE_422YUV8   = 0x00000004,
   // YUV 4:4:4.
   FLYCAPTURE_444YUV8   = 0x00000008,
   // R, G and B are the same and equal 8 bits.
   FLYCAPTURE_RGB8      = 0x00000010,
   // 16 bit mono.
   FLYCAPTURE_MONO16    = 0x00000020,
   // RR, G and B are the same and equal 16 bits.
   FLYCAPTURE_RGB16     = 0x00000040,
   // 16 bit signed mono .
   FLYCAPTURE_S_MONO16  = 0x00000080,
   // RR, G and B are the same and equal 16 bits signed
   FLYCAPTURE_S_RGB16   = 0x00000100,
   // 8 bit raw data output from sensor.
   FLYCAPTURE_RAW8      = 0x00000200,
   // 16 bit raw data output from  sensor.
   FLYCAPTURE_RAW16     = 0x00000400,
   // 24 bit BGR
   FLYCAPTURE_BGR       = 0x10000001,
   // 32 bit BGRU
   FLYCAPTURE_BGRU      = 0x10000002,
   // Unused member to force this enum to compile to 32 bits.
   FCPF_FORCE_QUADLET   = 0x7FFFFFFF,

} FlyCapturePixelFormat;


//
// Description:
//   Enumerates the image file formats that flycaptureSaveImage() can write to.
//
typedef enum FlyCaptureImageFileFormat
{
   // Single channel (8 or 16 bit) greyscale portable grey map.
   FLYCAPTURE_FILEFORMAT_PGM,
   // 3 channel RGB portable pixel map.
   FLYCAPTURE_FILEFORMAT_PPM,
   // 3 or 4 channel RGB windows bitmap.
   FLYCAPTURE_FILEFORMAT_BMP,
   // JPEG format.
   FLYCAPTURE_FILEFORMAT_JPG,
   // Portable Network Graphics format.  Not implemented.
   FLYCAPTURE_FILEFORMAT_PNG,
   // Raw data output.
   FLYCAPTURE_FILEFORMAT_RAW,

} FlyCaptureImageFileFormat;


//
// Description:
//  This structure defines the format by which time is represented in the
//  PGRFlycapture SDK.  The ulSeconds and ulMicroSeconds values represent the
//  absolute system time when the image was captured.  The ulCycleSeconds
//  and ulCycleCount are higher-precision values that have either been
//  propagated up from the 1394 bus or extracted from the image itself.  The
//  data will be extracted from the image if image timestamping is enabled and
//  directly (and less accurately) from the 1394 bus otherwise.
//
//  The ulCycleSeconds value will wrap around after 128 seconds.  The ulCycleCount
//  represents the 1/8000 second component. Use these two values when synchronizing
//  grabs between two computers sharing a common 1394 bus that may not have
//  precisely synchronized system timers.
//
typedef struct FlyCaptureTimestamp
{
   // The number of seconds since the epoch.
   unsigned long ulSeconds;
   // The microseconds component.
   unsigned long ulMicroSeconds;
   // The cycle time seconds.  0-127.
   unsigned long ulCycleSeconds;
   // The cycle time count.  0-7999. (1/8000ths of a second.)
   unsigned long ulCycleCount;
   // The cycle offset.  0-3071 (1/3072ths of a cycle count.)
   unsigned long ulCycleOffset;

} FlyCaptureTimestamp;


//
// Description:
//  This structure is used to pass image information into and out of the
//  API.
//
// Remarks:
//  The size of the image buffer is iRowInc * iRows, and depends on the
//  pixel format.
//
typedef struct FlyCaptureImage
{
   // Rows, in pixels, of the image.
   int iRows;
   // Columns, in pixels, of the image.
   int iCols;
   // Row increment.  The number of bytes per row.
   int iRowInc;
   // Video mode that this image was captured with.  This member is only
   // populated when the image is returned from a grab call.
   FlyCaptureVideoMode videoMode;
   // Timestamp of this image.
   FlyCaptureTimestamp timeStamp;
   // Pointer to the actual image data.
   unsigned char* pData;
   //
   // If the returned image is Y8, Y16, RAW8 or RAW16, this flag indicates
   // whether it is a greyscale or stippled (bayer tiled) image.  In all
   // other modes, this flag has no meaning.
   //
   bool bStippled;
   // The pixel format of this image.
   FlyCapturePixelFormat pixelFormat;

   // This field is always 1 for single lens cameras.  This field is 
   // used to indicate the number of images contained in the structure 
   // when dealing with multi-imager systems such as the Bumblebee2 
   // or XB3’
   int iNumImages;

   // Reserved for future use.
   unsigned long  ulReserved[ 5 ];

} FlyCaptureImage;


//=============================================================================
// 1394 Bus Functions
//=============================================================================
// Group = 1394 Bus Functions

//-----------------------------------------------------------------------------
//
// Name: flycaptureBusCameraCount()
//
// Description:
//   This function returns the number of 1394 cameras attached to the machine.
//
// Arguments:
//   puiCount - The number of cameras on the bus.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureBusCameraCount(
			 unsigned int* puiCount );


//-----------------------------------------------------------------------------
//
// Name: flycaptureBusEnumerateCamerasEx()
//
// Description:
//   This function enumerates all of the cameras found on the machine.
//   It fills an array of FlyCaptureInfoEx structures with all of the
//   pertinent information from the attached cameras. The index of a given
//   FlyCaptureInfoEx structure in the array parInfo is the device number.
//
// Arguments:
//   arInfo  - An array of FlyCaptureInfoEx structures, at least as
//             large as the number of cameras on the bus.
//   puiSize - The size of the array passed in.  The number of cameras
//             detected is passed back in this argument also.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// See Also:
//   flycaptureBusCameraCount()
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureBusEnumerateCamerasEx(
                                FlyCaptureInfoEx*  arInfo,
                                unsigned int*      puiSize );


//-----------------------------------------------------------------------------
//
// Name: flycaptureModifyCallback()
//
// Description:
//   This function registers or deregisters a bus callback function.
//   When the state of the bus changes, the registered callback
//   function will be called with a FLYCAPTURE_MESSAGE_X parameter indicating
//   the type of event.  Please see the FlyCap example for more information on
//   how to use callback functionality.
//
// Arguments:
//   context     - The FlyCapture context to access.
//   pfnCallback - A pointer to an externally defined callback function.
//   pParam      - A user-specified parameter to be passed back to the callback
//                 function.  Can be NULL.
//   bAdd        - True if the callback is to be added to the list of callbacks,
//                 false if the callback is to be removed.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureModifyCallback(
                         FlyCaptureContext   context,
                         FlyCaptureCallback* pfnCallback,
                         void*               pParam,
                         bool                bAdd );


//=============================================================================
// Construction/Destruction Functions
//=============================================================================
// Group = Construction/Destruction Functions

//-----------------------------------------------------------------------------
//
// Name: flycaptureCreateContext()
//
// Description:
//   This function creates a FlyCaptureContext and allocates all of the memory
//   that it requires.  The purpose of the FlyCaptureContext is to act as a
//   handle to one of the cameras attached to the system. This call must be
//   made before any other calls involving the context will work.
//
// Arguments:
//   pContext - A pointer to the FlyCaptureContext to be created.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureCreateContext(
			FlyCaptureContext* pContext );


//-----------------------------------------------------------------------------
//
// Name: flycaptureDestroyContext()
//
// Description:
//   Destroys the given FlyCaptureContext.  In order to prevent memory leaks
//   from occurring, this function must be called when the user is finished
//   with the FlyCaptureContext.
//
// Arguments:
//   context - The FlyCaptureContext to be destroyed.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureDestroyContext(
			 FlyCaptureContext context );


//-----------------------------------------------------------------------------
//
// Name: flycaptureInitialize()
//
// Description:
//   This function initializes one of the cameras on the bus and associates it
//   with the provided FlyCaptureContext. This call must be made after a
//   flycaptureCreateContext() command and prior to a flycaptureStart() command
//   in order for images to be grabbed.  Users can also use the
//   flycaptureInitializeFromSerialNumber() command to initialize a context
//   with a specific serial number.
//
// Arguments:
//   context  - The FlyCaptureContext to be associated with the camera being
//              initialized.
//   ulDevice - The device index of the FlyCapture camera to be initialized
//              (as indicated by flycaptureBusEnumerateCamerasEx()).
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//  If there is only one device on the bus, its index is generally 0.
//
// See Also:
//   flycaptureInitializeFromSerialNumber(), flycaptureCreateContext(),
//   flycaptureStart(), flycaptureBusEnumerateCamerasEx()
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureInitialize(
		     FlyCaptureContext context,
		     unsigned long     ulDevice );

//-----------------------------------------------------------------------------
//
// Name: flycaptureInitializeFromSerialNumber()
//
// Description:
//   Similar to the flycaptureInitialize() command, this function initializes
//   one of the cameras on the bus and associates it with the given
//   FlyCaptureContext.  This function differs from its counterpart in that it
//   takes a serial number rather than a bus index.
//
// Arguments:
//   context      - The FlyCaptureContext to be associated with the camera
//                  being initialized.
//   serialNumber - The serial number of the FlyCapture camera system to be
//                  initialized.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// See Also:
//   flycaptureInitialize(), flycaptureCreateContext(), flycaptureStart()
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureInitializeFromSerialNumber(
				     FlyCaptureContext            context,
				     FlyCaptureCameraSerialNumber serialNumber );


//-----------------------------------------------------------------------------
//
// Name: flycaptureGetCameraInfo()
//
// Description:
//   Retrieves information about the camera.
//
// Arguments:
//   context - The FlyCaptureContext associated with the camera.
//   pInfo   - Receives the camera information.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetCameraInfo(
                        FlyCaptureContext context,
                        FlyCaptureInfoEx* pInfo );

//-----------------------------------------------------------------------------
//
// Name: flycaptureGetCameraInfo()
//
// Description:
//   Retrieves information about the camera.
//
// Arguments:
//   context - The FlyCaptureContext associated with the camera.
//   pInfo   - Receives the camera information.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetDriverInfo(
                        FlyCaptureContext context,
                        FlyCaptureDriverInfo* pInfo );


//-----------------------------------------------------------------------------
// Name: flycaptureGetBusSpeed()
//
// Description:
//   This function gets the current asynchronous and isochronous bus speeds.
//   Asynchronous data transmission is primarily register reads and writes.
//   Isochronous data transmission is reserved for image transmission.
//
// Arguments:
//   context        - The FlyCaptureContext associated with the camera to be
//                    queried.
//   pAsyncBusSpeed - The current asynchronous bus speed.
//   pIsochBusSpeed - The current isochronous bus speed.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// See Also:
//   flycaptureSetBusSpeed()
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetBusSpeed(
		      FlyCaptureContext   context,
		      FlyCaptureBusSpeed* pAsyncBusSpeed,
		      FlyCaptureBusSpeed* pIsochBusSpeed );


//-----------------------------------------------------------------------------
// Name: flycaptureSetBusSpeed()
//
// Description:
//   This function sets the asynchronous and isochronous transmit and receive
//   bus speeds.
//
// Arguments:
//   context        - The FlyCaptureContext associated with the camera to be queried.
//   asyncBusSpeed  - The desired asynchronous data communication speed.
//   isochBusSpeed  - The desired isochronous data communication speed.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   If only one of asyncBusSpeed or isochBusSpeed is required, set the other
//   parameter to FLYCAPTURE_ANY.
//
// See Also:
//   flycaptureGetBusSpeed()
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetBusSpeed(
		      FlyCaptureContext  context,
		      FlyCaptureBusSpeed asyncBusSpeed,
		      FlyCaptureBusSpeed isochBusSpeed );


//=============================================================================
// General Functions
//=============================================================================
// Group = General Functions

//-----------------------------------------------------------------------------
//
// Name: flycaptureGetLibraryVersion()
//
// Description:
//  This function returns the version of the library defined at the top
//  of this header file (PGRFLYCAPTURE_VERSION), which is in the format
//  100*(major version)+(minor version).
//
// Returns:
//  An integer indicating the current version of the library.
//
PGRFLYCAPTURE_API int PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetLibraryVersion();


//-----------------------------------------------------------------------------
//
// Name: FlyCaptureErrorToString()
//
// Description:
//    This function returns a description of the provided FlyCaptureError.
//
// Arguments:
//   error - The FlyCapture error to be parsed.
//
// Returns:
//   A null-terminated character string that describes the FlyCapture error.
//
PGRFLYCAPTURE_API const char* PGRFLYCAPTURE_CALL_CONVEN
flycaptureErrorToString(
			FlyCaptureError error );

//-----------------------------------------------------------------------------
//
// Name: flycaptureRegisterToString()
//
// Description:
//    This function returns a description of the provided register number.
//
// Arguments:
//   ulRegister - The register to be translated.
//
// Returns:
//   A null-terminated character string that describes the register.
//
PGRFLYCAPTURE_API const char* PGRFLYCAPTURE_CALL_CONVEN
flycaptureRegisterToString(
                           unsigned long ulRegister  );


//=============================================================================
// Control Functions
//=============================================================================
// Group = Control Functions

//-----------------------------------------------------------------------------
//
// Name: flycaptureCheckVideoMode()
//
// Description:
//   This function allows the user to check if a given mode is supported by the
//   camera.
//
// Arguments:
//   context     - An initialized FlyCaptureContext.
//   videoMode   - The video mode to check.
//   frameRate   - The frame rate to check.
//   pbSupported - A pointer to a bool that will store whether or not the mode
//                 is supported.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureCheckVideoMode(
			 FlyCaptureContext   context,
			 FlyCaptureVideoMode videoMode,
			 FlyCaptureFrameRate frameRate,
			 bool*		     pbSupported );


//-----------------------------------------------------------------------------
//
// Name: flycaptureGetCurrentVideoMode()
//
// Description:
//   This function allows the user to request the camera's current video mode
//   and frame rate.
//
// Arguments:
//   context    - An initialized FlyCaptureContext.
//   pVideoMode - A pointer to a video mode to be filled in.
//   pFrameRate - A pointer to a frame rate to be filled in.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetCurrentVideoMode(
			      FlyCaptureContext    context,
			      FlyCaptureVideoMode* pVideoMode,
			      FlyCaptureFrameRate* pFrameRate );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureGetCurrentCustomImage()
//
// Description:
//   This function allows the user to request the current Format 7 settings
//   on the camera, provided that the camera is in Format 7.
//
// Arguments:
//   context            - The FlyCaptureContext to start grabbing.
//   puiMode            - The mode currently active (0-7).
//   puiImagePosLeft	- Maximum horizontal pixels.
//   puiImagePosTop     - Maximum vertical pixels.
//   puiWidth           - Indicates the horizontal "step size" of the custom
//                        image.
//   puiHeight          - Indicates the vertical "step size" of the custom
//                        image.
//   puiPacketSizeBytes	- Packet size in bytes.
//   pPixelFormat       - Current pixel format.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetCurrentCustomImage(
			   FlyCaptureContext	      context,
			   unsigned int*	      puiMode,
			   unsigned int*	      puiImagePosLeft,
			   unsigned int*	      puiImagePosTop,
			   unsigned int*	      puiWidth,
			   unsigned int*	      puiHeight,
			   unsigned int*	      puiPacketSizeBytes,
			   float*		      pfSpeed,
                           FlyCapturePixelFormat*     pPixelFormat );


//-----------------------------------------------------------------------------
//
// Name: flycaptureGetColorProcessingMethod()
//
// Description:
//   This function allows users to check the current color processing method.
//
// Arguments:
//   context - The FlyCapture context to access.
//   pMethod - A pointer to a FlyCaptureColorMethod that will store the current
//             color processing method.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// See Also:
//  flycaptureSetColorProcessingMethod()
//
//
// Remarks:
//  This function is only applicable when using the SDK and driver with cameras
//  that do not do on board color processing. See the definition of
//  FlyCaptureColorMethod for detailed descriptions of the available modes.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetColorProcessingMethod(
				   FlyCaptureContext        context,
				   FlyCaptureColorMethod*   pMethod );


//-----------------------------------------------------------------------------
//
// Name: flycaptureSetColorProcessingMethod()
//
// Description:
//   This function allows users to select the method used for color processing.
//
// Arguments:
//   context - The FlyCapture context to access.
//   method  - A variable of type FlyCaptureColorMethod indicating the color
//             processing method to be used.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// See Also:
//   flycaptureGetColorProcessingMethod()
//
// Remarks:
//  The Nearest Neighbor method has been remapped internally to Nearest
//  Neighbor Fast due to observed artifacts with the original method.
//  This function is only applicable when using the SDK and driver with cameras
//  that do not do on board color processing. See the definition of
//  FlyCaptureColorMethod for detailed descriptions of the available modes.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetColorProcessingMethod(
				   FlyCaptureContext	    context,
				   FlyCaptureColorMethod    method );


//-----------------------------------------------------------------------------
//
// Name: flycaptureGetColorTileFormat()
//
// Description:
//   This function allows users to check the current color tile destippling
//   format.
//
// Arguments:
//   context - The FlyCapture context to access.
//   pformat - A pointer to a FlyCaptureStippledFormat that will store the current
//             color tile format.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   The color tile format indicates the format of the stippled image the camera
//   returns.  This function is only applicable to cameras that do not do
//   onboard color processing.
//
// See Also:
//   flycaptureSetColorTileFormat()
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetColorTileFormat(
			      FlyCaptureContext		 context,
			      FlyCaptureStippledFormat*  pformat );


//-----------------------------------------------------------------------------
//
// Name: flycaptureSetColorTileFormat()
//
// Description:
//   This function sets the color tile destippling format.
//
// Arguments:
//   context - The FlyCapture context to access.
//   format  - The FlyCaptureStippledFormat to set.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//  The color tile format indicates the format of the stippled image the camera
//  returns.  This function is only applicable to cameras that do not do
//  onboard color processing.
//
// See Also:
//   flycaptureGetColorTileFormat()
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetColorTileFormat(
			     FlyCaptureContext        context,
			     FlyCaptureStippledFormat format );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureStart()
//
// Description:
//   This function starts the image grabbing process.  It should be called
//   after flycaptureCreateContext() and flycaptureInitialize().
//
// Arguments:
//   context   - The FlyCaptureContext to start grabbing.
//   videoMode - The video mode to start the camera in.
//   frameRate - The frame rate to start the camera at.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   It is during this and other related start calls where driver level image
//   buffer allocation occurs.
//
// See Also:
//   flycaptureCreateContext(), flycaptureInitialize(),
//   flycaptureInitializeFromSerialNumber(), flycaptureStartCustomImage(),
//   flycaptureStop()
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureStart(
		FlyCaptureContext   context,
		FlyCaptureVideoMode videoMode,
		FlyCaptureFrameRate frameRate );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureQueryCustomImage()
//
// Description:
//   This function queries the options available for the advanced Custom Image
//   or DCAM Format 7 functionality.
//
// Arguments:
//   context                 - The FlyCaptureContext to start grabbing.
//   uiMode                  - The mode to query (0-7).
//   pbAvailable             - Indicates the availability of this mode.
//   puiMaxImagePixelsWidth  - Maximum horizontal pixels.
//   puiMaxImagePixelsHeight - Maximum vertical pixels.
//   puiPixelUnitHorz        - Indicates the horizontal "step size" of the custom
//                             image.
//   puiPixelUnitVert        - Indicates the vertical "step size" of the custom
//                             image.
//   puiPixelFormats         - A bit field indicating the supported pixel formats
//                             of this mode.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// See Also:
//   flycaptureStartCustomImage()
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureQueryCustomImage(
			   FlyCaptureContext context,
			   unsigned int	     uiMode,
			   bool*             pbAvailable,
			   unsigned int*     puiMaxImagePixelsWidth,
			   unsigned int*     puiMaxImagePixelsHeight,
			   unsigned int*     puiPixelUnitHorz,
			   unsigned int*     puiPixelUnitVert,
                           unsigned int*     puiPixelFormats );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureQueryCustomImageEx()
//
// Description:
//   This function queries the options available for the advanced Custom Image
//   or DCAM Format 7functionality.  This function differs from
//   flycaptureStartCustomImage() in that it allows the user to retrieve the
//   offset unit size as well (which may be different than the image unit size).
//
// Arguments:
//   context - The FlyCaptureContext to start grabbing.
//   uiMode - The mode to query (0-7).
//   pbAvailable - Indicates the availability of this mode.
//   puiMaxImagePixelsWidth - Maximum horizonal pixels.
//   puiMaxImagePixelsHeight - Maximum vertical pixels.
//   puiPixelUnitHorz - Indicates the horizontal "step size" of the custom
//                      image.
//   puiPixelUnitVert - Indicates the vertical "step size" of the custom
//                      image.
//   puiOffsetUnitHorz - Indicates the horizontal "step size" of the offset
//                      in the custom image.
//   puiOffsetUnitVert - Indicates the vertical "step size" of the offset in
//                      the custom image.
//   puiPixelFormats  - A bit field indicating the supported pixel formats of
//                      this mode.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// See Also:
//   flycaptureStartCustomImage()
//
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureQueryCustomImageEx(
			   FlyCaptureContext context,
			   unsigned int	     uiMode,
			   bool*             pbAvailable,
			   unsigned int*     puiMaxImagePixelsWidth,
			   unsigned int*     puiMaxImagePixelsHeight,
			   unsigned int*     puiPixelUnitHorz,
			   unsigned int*     puiPixelUnitVert,
                           unsigned int*     puiOffsetUnitHorz,
                           unsigned int*     puiOffsetUnitVert,
                           unsigned int*     puiPixelFormats );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureStartCustomImage()
//
// Description:
//   This function starts the image grabbing process with "custom
//   image" (DCAM Format 7) functionality, which allows the user to select a
//   custom image size and/or region of interest.
//
// Arguments:
//   context         - The FlyCaptureContext to start grabbing.
//   uiMode          - The camera-specific mode.  (0-7).
//   uiImagePosLeft  - The left position of the (sub)image.
//   uiImagePosTop   - Top top position of the (sub)image.
//   uiWidth         - The width of the (sub)image.
//   uiHeight        - The height of the (sub)image.
//   fBandwidth      - A number between 1.0 and 100.0 which represents the
//                     percentage of the camera's maximum bandwidth to use for
//                     transmission.
//   format          - The pixel format to be used.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   It is during this and other related start calls where driver level image
//   buffer allocation occurs.
//
// See Also:
//   flycaptureStartCustomImagePacket(), flycaptureQueryCustomImage()
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureStartCustomImage(
			   FlyCaptureContext       context,
			   unsigned int	           uiMode,
			   unsigned int            uiImagePosLeft,
			   unsigned int            uiImagePosTop,
			   unsigned int	           uiWidth,
			   unsigned int	           uiHeight,
			   float                   fBandwidth,
                           FlyCapturePixelFormat   format );


//-----------------------------------------------------------------------------
//
// Name:
//  flycaptureStop()
//
// Description:
//   This function halts all image grabbing for the specified FlyCaptureContext.
//
// Arguments:
//   context - The FlyCaptureContext to stop.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   This function invalidates all buffers returned by flycaptureLockNext()
//   and flycaptureLockLatest().
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureStop(
	       FlyCaptureContext context );


//=============================================================================
// Image Related Functions
//=============================================================================
// Group = Image Related Functions

//-----------------------------------------------------------------------------
// Name: flycaptureSetGrabTimeoutEx()
//
// Description:
//   This function allows the user to set the timeout value for
//   flycaptureGrabImage*(), flycaptureLockLatest() and flycaptureLockNext().  
//   This is not normally necessary but can be useful in specific applications.  
//   For example, setting uiTimeout to be 0 will result in non-blocking 
//   grab call.
//
// Arguments:
//   context   - The FlyCaptureContext associated with the camera to be queried.
//   ulTimeout - The timeout value, in milliseconds.  A value of
//               FLYCAPTURE_INFINITE indicates an infinite wait.  A value of
//               zero indicates a nonblocking grab call.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//  The default grab timeout value is "infinite."  It is not normally necessary
//  to set this value.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetGrabTimeoutEx(
                           FlyCaptureContext context,
                           unsigned long     ulTimeout );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureGrabImage()
//
// Description:
//   This function grabs the newest image from the FlyCapture camera system and
//   passes the image buffer and information to the user.
//
// Arguments:
//   context       - The FlyCapture context to lock the image in.
//   ppImageBuffer - Pointer to the returned image buffer pointer.
//   piRows        - Pointer to the returned rows.
//   piCols        - Pointer to the returned columns.
//   piRowInc      - Pointer to the returned row increment (number of bytes per row.)
//   pVideoMode    - Pointer to the returned video mode.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//  This function will block until a new image is available.  You can
//  optionally set the timeout value for the wait using the
//  flycaptureSetGrabTimeoutEx() function (by default the wait time is
//  infinite.) Setting the timeout value should normally not be necessary.
//
// See Also:
//  flycaptureStart(), flycaptureGrabImage2(), flycaptureSetGrabTimeoutEx()
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGrabImage(
		    FlyCaptureContext	  context,
		    unsigned char**	  ppImageBuffer,
		    int*		  piRows,
		    int*		  piCols,
		    int*		  piRowInc,
		    FlyCaptureVideoMode*  pVideoMode );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureGrabImage2()
//
// Description:
//   This function is identical to flycaptureGrabImage() except that it returns
//   a FlyCaptureImage structure.
//
// Arguments:
//   context - The FlyCapture context to lock the image in.
//   pimage  - A pointer to a FlyCaptureImage structure that will contain the
//             image information.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//  See remarks for flycaptureGrabImage().
//
// See Also:
//  flycaptureStart(), flycaptureGrabImage(), flycaptureSetGrabTimeoutEx()
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGrabImage2(
		     FlyCaptureContext	  context,
		     FlyCaptureImage*	  pimage );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureSaveImage()
//
// Description:
//   Writes the specified image buffer to disk.
//
// Arguments:
//   context   - The FlyCapture context to access.
//   pImage    - The image to save.  This can be populated by the user, by only
//               filling out the pData, size, and pixel format information, or
//               can be the structure returned by flycaptureConvertImage().
//   pszPath   - The name of the file to write to.
//   format    - The file format to write.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSaveImage(
                    FlyCaptureContext           context,
                    const FlyCaptureImage*      pImage,
                    const char*                 pszPath,
                    FlyCaptureImageFileFormat   format);

//-----------------------------------------------------------------------------
//
// Name:  flycaptureSetJPEGCompressionQuality()
//
// Description:
//   Sets the JPEG compression quality to the specified value.
//
// Arguments:
//   context   - The FlyCapture context to access.
//   iQuality  - The JPEG compression quality to use when saving JPEG images
//               with flycaptureSaveImage()
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetJPEGCompressionQuality(
                    FlyCaptureContext           context,
                    int				iQuality);

//-----------------------------------------------------------------------------
//
// Name:  flycaptureConvertImage()
//
// Description:
//   Convert an arbitrary image format to another format.
//
// Arguments:
//   context      - The FlyCapture context to access.
//   pimageSrc    - The source image to convert
//   pimageDest   - The destination image to convert.  The pData member must be
//                  initialized to an output buffer of sufficient size, and
//                  the pixelFormat member indicates the desired output format.
//                  Only BGR and BGRU are currently supported.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   This function replaces flycaptureConvertToBGR24(),
//   flycaptureStippledToBGR24(), and flycaptureStippledToBGRU32().
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureConvertImage(
                       FlyCaptureContext        context,
                       const FlyCaptureImage*   pimageSrc,
                       FlyCaptureImage*         pimageDest );

//-----------------------------------------------------------------------------
//
// Name:  flycaptureInplaceRGB24toBGR24()
//
// Description:
//   Changes the input image buffer from 24-bit RGB to windows-displayable
//   24-bit BGR.
//
// Arguments:
//   pImageBuffer - Pointer to the image contents.
//   iImagePixels - Size of the image, in pixels.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureInplaceRGB24toBGR24(
                              unsigned char* pImageBuffer,
                              int            iImagePixels );


//-----------------------------------------------------------------------------
//
// Name: flycaptureInplaceWhiteBalance()
//
// Description:
//   This function performs an inplace software based white balance on the
//   provided image.
//
// Arguments:
//   context   - The FlyCapture context.
//   pData     - The BGR24 image data.
//   iRows     - Image rows.
//   iCols     - Image columns.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//  The image must be in BGR24 format.  I.e., the output from one of the above
//  functions.  This function has no effect on cameras that are detected to
//  have hardware whitebalance.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureInplaceWhiteBalance(
			      FlyCaptureContext context,
			      unsigned char*	pData,
			      int   		iRows,
			      int   		iCols );


//=============================================================================
// Camera Property Functions
//=============================================================================
// Group = Camera Property Functions

//-----------------------------------------------------------------------------
//
// Name:  flycaptureGetCameraPropertyRange()
//
// Description:
//   Allows the user to examine the default, minimum, maximum, and auto
//   characteristics for the given property.
//
// Arguments:
//   context        - The FlyCapture context to extract the properties from.
//   cameraProperty - A FlyCaptureProperty indicating the property to
//                    examine.
//   pbPresent	    - A pointer to a bool that will contain whether or not
//                    camera property is present.
//   plMin          - A pointer to a long that will contain the minimum
//                    property value.
//   plMax          - A pointer to a long that will contain the maximum
//                    property value.
//   plDefault      - A pointer to a long that will contain the default
//                    property value.
//   pbAuto	    - A pointer to a bool that will contain whether or not
//                    the Auto setting is available for this property.
//   pbManual	    - A pointer to a bool that will contain whether or not
//                    this property may be manually adjusted.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   Pass NULL for any pointer argument to ignore that argument.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetCameraPropertyRange(
				 FlyCaptureContext  context,
				 FlyCaptureProperty cameraProperty,
				 bool*              pbPresent,
				 long*              plMin,
				 long*              plMax,
				 long*              plDefault,
				 bool*              pbAuto,
				 bool*		    pbManual );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureGetCameraProperty()
//
// Description:
//   Allows the user to query the current value of the given property.
//
// Arguments:
//   context        - The FlyCapture context to extract the properties from.
//   cameraProperty - A FlyCaptureProperty indicating the property to
//                    query.
//   plValueA       - A pointer to storage space for the "A", or first value
//                    associated with this property.
//   plValueB       - A pointer to storage space for the "B", or second value
//                    associated with this property.
//   pbAuto         - A pointer to a bool that will store the current Auto
//                    value of the property.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   Pass NULL for any pointer argument to ignore that argument.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetCameraProperty(
			    FlyCaptureContext   context,
			    FlyCaptureProperty  cameraProperty,
			    long*               plValueA,
			    long*               plValueB,
			    bool*               pbAuto );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureSetCameraProperty()
//
// Description:
//   Allows the user to set the given property.
//
// Arguments:
//   context        - The FlyCaptureContext to set the properties in.
//   cameraProperty - A FlyCaptureProperty indicating the property to set.
//   lValueA        - A long containing the "A", or first new value of the
//                    property.
//   lValueB        - A long containing the "B", or second new value of the
//                    property.
//   bAuto          - A boolean containing the new 'auto' state of the property.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   Calling this function with either of FLYCAPTURE_SOFTWARE_WHITEBALANCE
//   as the cameraProperty parameter and 'true' for the bAuto parameter will
//   invoke a single shot auto white balance method.  The assumption is that
//   flycaptureGrabImage() has been called previously with a white object
//   centered in the field of view.  This will only work if the camera is a
//   color camera and in RGB mode.  The Red and Blue whitebalance parameters
//   only affect cameras that do offboard color calculation such as the
//   Dragonfly.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetCameraProperty(
			    FlyCaptureContext  context,
			    FlyCaptureProperty cameraProperty,
			    long               lValueA,
			    long               lValueB,
			    bool               bAuto );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureSetCameraPropertyBroadcast()
//
// Description:
//   Allows the user to set the given property for all cameras on the bus.
//
// Arguments:
//   context        - The FlyCaptureContext to set the properties in.
//   cameraProperty - A FlyCaptureProperty indicating the property to set.
//   lValueA        - A long containing the "A", or first new value of the
//                    property.
//   lValueB        - A long containing the "B", or second new value of the
//                    property.
//   bAuto          - A boolean containing the new 'auto' state of the property.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   This function will set the given property for all the cameras on the
//   1394 bus.  If you are using multiple busses (ie, more than one 1394 card)
//   you must call this function for each bus, on a context representing a
//   camera on that bus.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetCameraPropertyBroadcast(
				     FlyCaptureContext  context,
				     FlyCaptureProperty cameraProperty,
				     long               lValueA,
				     long               lValueB,
				     bool               bAuto );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureGetCameraPropertyRangeEx()
//
// Description:
//   Replaces flycaptureGetCameraPropertyRange() and provides better access to
//   camera features.
//
// Arguments:
//   context        - The FlyCapture context to extract the properties from.
//   cameraProperty - A FlyCaptureProperty indicating the property to
//                    examine.
//   pbPresent      - Indicates the presence of this property on the camera.
//   pbOnePush      - Indicates the availability of the one push feature.
//   pbReadOut      - Indicates the ability to read out the value of this property.
//   pbOnOff        - Indicates the ability to turn this property on and off.
//   pbAuto         - Indicates the availability of auto mode for this property.
//   pbManual       - Indicates the ability to manually control this property.
//   piMin          - The minimum value of the property is returned in this
//                    argument.
//   piMax          - The maximum value of the property is returned in this
//                    argument.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   Pass NULL for any pointer argument to ignore that argument.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetCameraPropertyRangeEx(
                                   FlyCaptureContext    context,
                                   FlyCaptureProperty   cameraProperty,
                                   bool*                pbPresent,
                                   bool*                pbOnePush,
                                   bool*                pbReadOut,
                                   bool*                pbOnOff,
                                   bool*                pbAuto,
                                   bool*		pbManual,
                                   int*                 piMin,
                                   int*                 piMax );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureGetCameraPropertyEx()
//
// Description:
//   Replaces flycaptureGetCameraProperty() and provides better access to
//   camera features.
//
// Arguments:
//   context        - The FlyCapture context to extract the properties from.
//   cameraProperty - A FlyCaptureProperty indicating the property to
//                    query.
//   pbOnePush      - The value of the one push bit.
//   pbOnOff        - The value of the On/Off bit.
//   pbAuto         - The value of the Auto bit.
//   piValueA       - The current value of this property.
//   piValueB       - The current secondary value of this property. (only
//                    used for the two whitebalance values.)
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   Pass NULL for any pointer argument to ignore that argument.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetCameraPropertyEx(
                              FlyCaptureContext   context,
                              FlyCaptureProperty  cameraProperty,
                              bool*               pbOnePush,
                              bool*               pbOnOff,
                              bool*               pbAuto,
                              int*                piValueA,
                              int*                piValueB );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureSetCameraPropertyEx()
//
// Description:
//   Replaces flycaptureSetCameraPropertyEx() and provides better access to
//   camera features.
//
// Arguments:
//   context        - The FlyCaptureContext to set the properties in.
//   cameraProperty - A FlyCaptureProperty indicating the property to set.
//   bOnePush       - Set the one push bit.
//   bOnOff         - Set the on/off bit.
//   bAuto          - Set the auto bit.
//   iValueA        - The value to set.
//   iValueB        - The secondary value to set.  (only used for the two
//                    whitebalance values.)
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetCameraPropertyEx(
                              FlyCaptureContext    context,
                              FlyCaptureProperty   cameraProperty,
                              bool                 bOnePush,
                              bool                 bOnOff,
                              bool                 bAuto,
                              int                  iValueA,
                              int                  iValueB );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureSetCameraPropertyBroadcastEx()
//
// Description:
//   Replaces flycaptureSetCameraPropertyBroadcast() and provides better access
//   to camera features.
//
// Arguments:
//   context        - The FlyCaptureContext to set the properties in.
//   cameraProperty - A FlyCaptureProperty indicating the property to set.
//   bOnePush       - Set the one push bit.
//   bOnOff         - Set the on/off bit.
//   bAuto          - Set the auto bit.
//   iValueA        - The value to set.
//   iValueB        - The secondary value to set.  (only used for the two
//                    whitebalance values.)
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   This function will set the given property for all the cameras on the
//   1394 bus.  If you are using multiple busses (ie, more than one 1394 card)
//   you must call this function for each bus, on a context representing a
//   camera on that bus.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetCameraPropertyBroadcastEx(
                                       FlyCaptureContext    context,
                                       FlyCaptureProperty   cameraProperty,
                                       bool                 bOnePush,
                                       bool                 bOnOff,
                                       bool                 bAuto,
                                       int                  iValueA,
                                       int                  iValueB );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureGetCameraAbsPropertyRange()
//
// Description:
//   Allows the user to determine the presence and range of the absolute value
//   registers for the camera
//
// Arguments:
//   context - The Flycapture context to query.
//   cameraProperty - A FlyCaptureProperty indicating which property to query.
//   pbPresent - Whether or not this register has absolute value support.
//   pfMin - The minimum value that this register can handle.
//   pfMax - The maximum value that this register can handle.
//   ppszUnits - A string indicating the units of the register.
//   ppszUnitAbbr - An abbreviation of the units
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetCameraAbsPropertyRange(
				    FlyCaptureContext  context,
				    FlyCaptureProperty cameraProperty,
				    bool*              pbPresent,
				    float*             pfMin,
				    float*             pfMax,
				    const char**       ppszUnits,
				    const char**       ppszUnitAbbr );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureGetCameraAbsProperty()
//
// Description:
//   Allows the user to get the current absolute value for a given parameter
//   from the camera if it is supported.
//
// Arguments:
//   context - The FlyCapture context to query.
//   cameraProperty - A FlyCaptureProperty indicating which property to query.
//   pfValue - A pointer to a float that will contain the result.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the operation.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetCameraAbsProperty(
                               FlyCaptureContext   context,
                               FlyCaptureProperty  cameraProperty,
                               float*              pfValue );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureGetCameraAbsPropertyEx()
//
// Description:
//   Allows the user to get the current absolute value for a given parameter
//   from the camera if it is supported.  This function also allows the user
//   to query the states of the one push, on/off, and auto controls in the
//   property's standard register.
//
// Arguments:
//   context        - The FlyCapture context to query.
//   cameraProperty - A FlyCaptureProperty indicating which property to query.
//   pbOnePush      - A valid pointer to a bool that will store the one push state
//   pbOnOff        - A valid pointer to a bool that will store the on/off state.
//   pbAuto         - A valid pointer to a bool that will store the auto state
//   pfValue        - A pointer to a float that will contain the result.
//
//
// Remarks:
//   The data returned by this function is extracted by a series of two register
//   reads.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the operation.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetCameraAbsPropertyEx(
                               FlyCaptureContext   context,
                               FlyCaptureProperty  cameraProperty,
                               bool*               pbOnePush,
                               bool*               pbOnOff,
                               bool*               pbAuto,
                               float*              pfValue );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureSetCameraAbsProperty()
//
// Description:
//   Allows the user to set the absolute value of the given parameter if the
//   mode is supported.
//
// Arguments:
//   context        - The FlyCapture context to query.
//   cameraProperty - A FlyCaptureProperty indicating which property to query.
//   fValue         - A float containing the new value of the parameter.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the operation.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetCameraAbsProperty(
			       FlyCaptureContext  context,
			       FlyCaptureProperty cameraProperty,
			       float              fValue );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureSetCameraAbsPropertyEx()
//
// Description:
//   Allows the user to set the absolute value of the given parameter if the
//   mode is supported.  This function also allows the user to specify the
//   one push, on/off, and auto settings of the same property.
//
// Arguments:
//   context        - The FlyCapture context to query.
//   cameraProperty - A FlyCaptureProperty indicating which property to query.
//   bOnePush       - A bool indicating if one push should be enabled.
//   bOnOff         - A bool indicating if the property should be on or off.
//   bAuto          - A bool indicating if the property should be automatically
//                    controlled by the camera.
//   fValue         - A float containing the new value of the parameter.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the operation.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetCameraAbsPropertyEx(
			       FlyCaptureContext  context,
			       FlyCaptureProperty cameraProperty,
                               bool               bOnePush,
                               bool               bOnOff,
                               bool               bAuto,
			       float              fValue );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureSetCameraAbsPropertyBroadcastEx()
//
// Description:
//   Allows the user to set the absolute value of the given parameter if the
//   mode is supported.  This function also allows the user to specify the
//   one push, on/off, and auto settings of the same property.
//
// Arguments:
//   context        - The FlyCapture context to query.
//   cameraProperty - A FlyCaptureProperty indicating which property to query.
//   bOnePush       - A bool indicating if one push should be enabled.
//   bOnOff         - A bool indicating if the property should be on or off.
//   bAuto          - A bool indicating if the property should be automatically
//                    controlled by the camera.
//   fValue         - A float containing the new value of the parameter.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the operation.
//
// Remarks:
//   This function will set the given property for all the cameras on the
//   1394 bus that are associated with the context passed.  If multiple busses
//   (i.e. more than one 1394 card) exist, a call to this function must be made
//   for each bus using a context representing a camera on that bus.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetCameraAbsPropertyBroadcastEx(
			       FlyCaptureContext  context,
			       FlyCaptureProperty cameraProperty,
                               bool               bOnePush,
                               bool               bOnOff,
                               bool               bAuto,
			       float              fValue );


//-----------------------------------------------------------------------------
//
// Name:  flycaptureSetCameraAbsPropertyBroadcast()
//
// Description:
//   Allows the user to set the absolute value of the given parameter to all
//   cameras on the current bus.
//
// Arguments:
//   context - The FlyCapture context to query.
//   cameraProperty - A FlyCaptureProperty indicating which property to query.
//   fValue - A float containing the new value of the parameter.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the operation.
//
// Remarks:
//   This function will set the given property for all the cameras on the
//   1394 bus that are associated with the context passed.  If multiple busses
//   (i.e. more than one 1394 card) exist, a call to this function must be made
//   for each bus using a context representing a camera on that bus.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetCameraAbsPropertyBroadcast(
                                        FlyCaptureContext  context,
                                        FlyCaptureProperty cameraProperty,
                                        float              fValue );


//-----------------------------------------------------------------------------
// Name:  flycaptureGetCameraRegister()
//
// Description:
//   This function allows the user to get any of camera's registers.
//
// Arguments:
//   context    - The FlyCaptureContext associated with the camera to be queried.
//   ulRegister - The 32 bit register location to query.
//   pulValue   - The 32 bit value currently stored in the register.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   The ulRegister value is actually an offset applied to a base address.
//   Typically this base addess is 0xFFFFF0F00000 but it is not constant.
//   Refer to the "Unit Dependent Directory" section of the DCAM spec for more
//   information.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetCameraRegister(
			    FlyCaptureContext context,
			    unsigned long     ulRegister,
			    unsigned long*    pulValue );


//-----------------------------------------------------------------------------
// Name:  flycaptureSetCameraRegister()
//
// Description:
//   This function allows the user to set any of the camera's registers.
//
// Arguments:
//   context    - The FlyCaptureContext associated with the camera to be queried.
//   ulRegister - The 32 bit register location to set.
//   ulValue    - The 32 bit value to store in the register.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   The ulRegister value is actually an offset applied to a base address.
//   Typically this base addess is 0xFFFFF0F00000 but it is not constant.
//   Refer to the "Unit Dependent Directory" section of the DCAM spec for more
//   information.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetCameraRegister(
			    FlyCaptureContext context,
			    unsigned long     ulRegister,
			    unsigned long     ulValue );


//-----------------------------------------------------------------------------
// Name:  flycaptureSetCameraRegisterBroadcast()
//
// Description:
//   This function allows the user to set any register for all cameras on
//   the bus.
//
// Arguments:
//   context    - The FlyCaptureContext associated with the camera to be queried.
//   ulRegister - The 32 bit register location to set.
//   ulValue    - The 32 bit value to store in the register.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   The ulRegister value is actually an offset applied to a base address.
//   Typically this base addess is 0xFFFFF0F00000 but it is not constant.
//   Refer to the "Unit Dependent Directory" section of the DCAM spec for more
//   information.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetCameraRegisterBroadcast(
				     FlyCaptureContext context,
				     unsigned long     ulRegister,
				     unsigned long     ulValue );


//-----------------------------------------------------------------------------
// Name:  flycaptureGetMemoryChannel()
//
// Description:
//   This function will query the camera to see what the currently set memory
//   channel is and/or what the maximum valid channel is.  At least one pointer
//   must be valid.
//
// Arguments:
//   context - The FlyCaptureContext associated with the camera
//   puiCurrentChannel - NULL or a valid pointer to an unsigned int to store
//                       the current channel in.
//   puiNumChannels - NULL or a valid pointer to an unsigned int to store the
//                    maximum valid memory channel. Zero indicates no user
//                    channels.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   Refer to your camera's technical reference for the registers affected by
//   the memory channels. Use flycaptureGetMemoryChannel() to check the current
//   and/or maximum number of channels available.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetMemoryChannel(
                              FlyCaptureContext context,
                              unsigned int*     puiCurrentChannel,
                              unsigned int*     puiNumChannels );


//-----------------------------------------------------------------------------
// Name:  flycaptureSaveToMemoryChannel()
//
// Description:
//   This function will save a group of the current camera registers to the
//   specified memory channel on the camera.
//
// Arguments:
//   context    - The FlyCaptureContext associated with the camera
//   ulChannel  - The channel to store the values in
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   Refer to your camera's technical reference for the registers affected by
//   the memory channels. Use flycaptureGetMemoryChannel() to check the current
//   and/or maximum number of channels available.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSaveToMemoryChannel(
                              FlyCaptureContext context,
                              unsigned long     ulChannel );


//-----------------------------------------------------------------------------
// Name:  flycaptureRestoreFromMemoryChannel()
//
// Description:
//   This function will restore a group of register settings from the specified
//   memory channel on the camera.  This will make the specified channel the
//   current channel.
//
// Arguments:
//   context    - The FlyCaptureContext associated with the camera
//   ulChannel  - The channel to change to
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   Refer to your camera's technical reference for the registers affected by
//   the memory channels. Use flycaptureGetMemoryChannel() to check the current
//   and/or maximum number of channels available.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureRestoreFromMemoryChannel(
                                   FlyCaptureContext context,
                                   unsigned long     ulChannel );


//-----------------------------------------------------------------------------
// Name: flycaptureGetCameraTrigger()
//
// Description:
//   Deprecated.  Please use flycaptureGetTrigger().
//
// Returns:
//   FLYCAPTURE_DEPRECATED.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetCameraTrigger(
			   FlyCaptureContext context,
			   unsigned int*     puiPresence,
			   unsigned int*     puiOnOff,
			   unsigned int*     puiPolarity,
			   unsigned int*     puiTriggerMode );


//-----------------------------------------------------------------------------
// Name: flycaptureSetCameraTrigger()
//
// Description:
//    Deprecated.  Please use flycaptureSetTrigger().
//
// Returns:
//   FLYCAPTURE_DEPRECATED.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetCameraTrigger(
			   FlyCaptureContext context,
			   unsigned int      uiOnOff,
			   unsigned int	     uiPolarity,
			   unsigned int      uiTriggerMode );


//-----------------------------------------------------------------------------
// Name: flycaptureSetCameraTriggerBroadcast()
//
// Description:
//   Deprecated.  Please use flycaptureSetTriggerBroadcast().
//
// Returns:
//   FLYCAPTURE_DEPRECATED.
//
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetCameraTriggerBroadcast(
				    FlyCaptureContext context,
				    unsigned char     ucOnOff,
				    unsigned char     ucPolarity,
				    unsigned char     ucTriggerMode );

//-----------------------------------------------------------------------------
// Name: flycaptureQueryTrigger()
//
// Description:
//   This function allows the user to query the trigger functionality of the
//   camera.
//
// Arguments:
//   context	     - The context associated with the camera to be queried.
//   pbPresent       - Whether or not the camera has trigger functionality.
//   pbReadOut       - Whether or not the user can read values in the trigger
//                     functionality.
//   pbOnOff         - Whether or not the functionality can be turned on or
//                     off.
//   pbPolarity      - Whether or not the polarity can be changed.
//   pbValueRead     - Whether or not the raw trigger input can be read.
//   puiSourceMask   - A bit field indicating which trigger sources are available.
//   pbSoftwareTrigger  - Whether or not software triggering is available.
//   puiModeMask     - A bit field indicating which trigger modes are available.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   Polarity and trigger sources are camera dependant.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureQueryTrigger(
                       FlyCaptureContext  context,
                       bool*              pbPresent,
                       bool*              pbReadOut,
                       bool*              pbOnOff,
                       bool*              pbPolarity,
                       bool*              pbValueRead,
                       unsigned int*      puiSourceMask,
                       bool*              pbSoftwareTrigger,
                       unsigned int*      puiModeMask );


//-----------------------------------------------------------------------------
// Name: flycaptureGetTrigger()
//
// Description:
//   This function allows the user to query the state of the camera's trigger
//   functionality.  This function replaces the deprecated
//   flycaptureGetCameraTrigger() function.
//
// Arguments:
//   context      - The context associated with the camera to be queried.
//   pbOnOff      - The On/Off state is returned in this parameter.
//   piPolarity   - The polarity value is returned in this parameter.
//   piSource     - The source value is returned in this parameter.
//   piRawValue   - The raw signal value is returned in this parameter.
//   piMode       - The trigger mode is returned in this parameter.
//   piParameter  - The parameter for the trigger function
//                  is returned in this parameter.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetTrigger(
                     FlyCaptureContext context,
                     bool*             pbOnOff,
                     int*              piPolarity,
                     int*              piSource,
                     int*              piRawValue,
                     int*              piMode,
                     int*              piParameter );


//-----------------------------------------------------------------------------
// Name: flycaptureSetTrigger()
//
// Description:
//   This function allows the user to set the state of the camera's
//   trigger functionality.  THis function replaces the deprecated
//   flycaptureSetCameraTrigger() function.
//
// Arguments:
//   context	  - The context associated with the camera to be queried.
//   bOnOff       - Turn the trigger on or off.
//   iPolarity    - The polarity of the trigger. 1 or 0.
//   iSource      - The new trigger source.  Corresponds to the source mask.
//   iMode        - The new trigger mode.  Corresponds to the mode mask.
//   iParameter   - The (optional) parameter to the trigger function, if required.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//  If you have set a grab timeout using flycaptureSetGrabTimeoutEx(), this
//  timeout will be used in asynchronous trigger mode as well:
//  flycaptureGrabImage*() will return with the image when you either trigger
//  the camera, or the timeout value expires.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetTrigger(
                     FlyCaptureContext context,
                     bool              bOnOff,
                     int               iPolarity,
                     int               iSource,
                     int               iMode,
                     int               iParameter );


//-----------------------------------------------------------------------------
// Name: flycaptureSetTriggerBroadcast()
//
// Description:
//   This function duplicates the functionality of flycaptureSetTrigger, except
//   it broadcasts changes to all cameras on the bus.
//
// Arguments:
//   context	  - The context associated with the camera to be queried.
//   bOnOff       - Turn the trigger on or off.
//   iPolarity    - The polarity of the trigger. 1 or 0.
//   iSource      - The new trigger source.  Corresponds to the source mask.
//   iMode        - The new trigger mode.  Corresponds to the mode mask.
//   iParameter   - The (optional) parameter to the trigger function, if required.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetTriggerBroadcast(
                              FlyCaptureContext context,
                              bool              bOnOff,
                              int               iPolarity,
                              int               iSource,
                              int               iMode,
                              int               iParameter );

//-----------------------------------------------------------------------------
// Name: flycaptureGetStrobe()
//
// Description:
//   This function allows the user to query the state of one of the camera's
//   strobe sources.  Only for use with cameras which support DCAM v1.31 compliant
//   strobes.
//
// Arguments:
//   context	  			- The context associated with the camera to be queried.
//   iSource      			- The strobe source to be queried.
//   pbOnOff      			- The current on/off status is returned in this paramaeter.
//   pbPolarityActiveLow   	- The current polarity of the strobe is returned. 1 or 0.
//   piDelay      			- The current delay is returned in this parameter.
//   piDuration   			- The current duration is returned in this parameter.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetStrobe(
                    FlyCaptureContext context,
                    int               iSource,
                    bool*             pbOnOff,
                    bool*             pbPolarityActiveLow,
                    int*              piDelay,
                    int*              piDuration );

//-----------------------------------------------------------------------------
// Name: flycaptureSetStrobe()
//
// Description:
//   This function allows the user to set the state of one of the camera's
//   strobe sources.  Only for use with cameras which support DCAM v1.31 compliant
//   strobes.
//
// Arguments:
//   context	  			- The context associated with the camera to be queried.
//   iSource      			- The strobe source to be set.
//   bOnOff       			- Describes whether to turn the strobe on or off.
//   bPolarityActiveLow    	- The polarity of the strobe. 1 or 0.
//   iDelay       			- The delay of the strobe.
//   iDuration    			- The duration of the strobe.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetStrobe(
                    FlyCaptureContext context,
                    int               iSource,
                    bool              bOnOff,
                    bool              bPolarityActiveLow,
                    int               iDelay,
                    int               iDuration );

//-----------------------------------------------------------------------------
// Name: flycaptureSetStrobeBroadcast()
//
// Description:
//   This function duplicates the functionality of flycaptureSetStrobe() but
//   broadcasts the settings to all cameras on the bus.  Only for use with
//   cameras which support DCAM v1.31 compliant strobes.
//
// Arguments:
//   context	  			- The context associated with the camera to be queried.
//   iSource      			- The strobe source to be set.
//   bOnOff       			- Describes whether to turn the strobe on or off.
//   bPolarityActiveLow    	- The polarity of the strobe. 1 or 0.
//   iDelay       			- The delay of the strobe.
//   iDuration    			- The duration of the strobe.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetStrobeBroadcast(
                    FlyCaptureContext context,
                    int               iSource,
                    bool              bOnOff,
                    bool              bPolarityActiveLow,
                    int               iDelay,
                    int               iDuration );

//-----------------------------------------------------------------------------
// Name: flycaptureQueryStrobe()
//
// Description:
//   This function queries the abilities and available settings for a particular
//   strobe source.  Only for use with cameras which support DCAM v1.31 compliant
//   strobes.
//
// Arguments:
//   context	  - The context associated with the camera to be queried.
//   iSource      - The strobe source to be queried.
//   pbAvailable  - NULL or a parameter which indicates if strobe is supported
//   pbReadOut    - Describes whether the source allows reading of the current value.
//   pbOnOff      - Describes whether the source can be turned on or off.
//   pbPolarity   - Describes whether the source's polarity can be changed.
//   piMinValue   - This parameter holds the minimum value of the delay and duration.
//   piMaxValue   - This parameter holds the maximum value of the delay and duration.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureQueryStrobe(
                      FlyCaptureContext context,
                      int               iSource,
                      bool*             pbAvailable,
                      bool*             pbReadOut,
                      bool*             pbOnOff,
                      bool*             pbPolarity,
                      int*              piMinValue,
                      int*              piMaxValue );

//-----------------------------------------------------------------------------
// Name: flycaptureQueryLookUpTable()
//
// Description:
//   This function queries the availability and state of the camera's look up
//   table.
//
// Arguments:
//   context        - The context associated with the camera to be queried.
//   pbAvailable    - NULL or a parameter which indicates if the LUT is supported
//   puiNumChannels - NULL or a parameter which indicates the number of
//                    available channels.  NOTE some cameras will return
//                    available, but zero channels.  Typically, these cameras
//                    will have a single channel and not support turning the
//                    LUT off.
//   pbOn           - NULL or a parameter which indicates whether the LUT is currently on
//   puiBitDepth    - NULL or a parameter which indicates the bit depth of the
//                    LUT (this will be the number of bits in the output values).
//   puiNumEntries  - NULL or a parameter which indicates the number of entries
//                    in the table (this will be the number of input values).
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   At least one parameter besides the context must be valid or an error will
//   be returned.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureQueryLookUpTable(
                           FlyCaptureContext context,
                           bool*             pbAvailable,
                           unsigned int*     puiNumChannels,
                           bool*             pbOn,
                           unsigned int*     puiBitDepth,
                           unsigned int*     puiNumEntries );


//-----------------------------------------------------------------------------
// Name: flycaptureEnableLookUpTable()
//
// Description:
//   This function turns the look up table on or off.
//
// Arguments:
//   context - The context associated with the camera.
//   bOn - true to enable, false to disable.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
// Remarks:
//   The look up table on some cameras can not be turned off.  These cameras
//   return FLYCAPTURE_NOT_IMPLEMENTED if bOn is false.  See the description of
//   flycaptureQueryLookUpTable() for help on identifying these cameras.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureEnableLookUpTable(
                            FlyCaptureContext context,
                            bool              bOn );


//-----------------------------------------------------------------------------
// Name: flycaptureSetLookUpTableChannel()
//
// Description:
//   This function will set the specified look up table on the camera.
//
// Arguments:
//   context - The context associated with the camera.
//   uiChannel - The channel to set
//   puiArray - a valid array of "numberOfEntries" unsigned ints with values
//              less than 2^( "bitDepth" )
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetLookUpTableChannel(
                                FlyCaptureContext   context,
                                unsigned int        uiChannel,
                                const unsigned int* puiArray );

//-----------------------------------------------------------------------------
// Name: flycaptureGetLookUpTableChannel()
//
// Description:
//   This function will retrieve the specified look up table on the camera.
//
// Arguments:
//   context   - The context associated with the camera.
//   uiChannel - The channel to retrieve
//   puiArray  - a valid array of "numberOfEntries" unsigned ints
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetLookUpTableChannel(
                                FlyCaptureContext context,
                                unsigned int      uiChannel,
                                unsigned int*     puiArray );


#ifdef __cplusplus
};
#endif

#endif // #ifndef __PGRFLYCAPTURE_H__
