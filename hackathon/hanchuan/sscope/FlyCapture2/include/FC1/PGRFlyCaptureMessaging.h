//=============================================================================
// Copyright © 2006 Point Grey Research, Inc. All Rights Reserved.
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
// PGRFlyCaptureMessaging.h
//
//   Defines the API to the PGR FlyCapture Messaging library.
//
//  We welcome your bug reports, suggestions, and comments: 
//  www.ptgrey.com/support/contact
//
//=============================================================================

//=============================================================================
// $Id: PGRFlyCaptureMessaging.h,v 1.1 2009/04/30 17:29:30 soowei Exp $
//=============================================================================
#ifndef __PGRFLYCAPTUREMESSAGING_H__
#define __PGRFLYCAPTUREMESSAGING_H__

//=============================================================================
// Project Includes
//=============================================================================
#include "pgrflycapture.h"

#ifdef __cplusplus
extern "C"
{
#endif

//
// This is used as the serial number when initializing or receiving 
// bus messages.
//
#define FLYCAPTURE_BUS_MESSAGE   999999999

//
// Description:
//  This structure is used in messages as either the timestamp of
//  an image or a bus event time.
//
typedef struct FlyCaptureSystemTime
{
   unsigned short usHour;
   unsigned short usMinute;
   unsigned short usSecond;
   unsigned short usMilliseconds;
} FlyCaptureSystemTime;

//
// Description:
//  Enumerates the message types that can be received.
//
typedef enum FlyCaptureMessageType
{
   // The bus was reset.
   FLYCAPTURE_BUS_RESET,
   // A device was connected.
   FLYCAPTURE_DEVICE_ARRIVAL,
   // A device was disconnected.
   FLYCAPTURE_DEVICE_REMOVAL,
   // A 1394b bus has experienced an error.
   FLYCAPTURE_BUS_ERROR,
   // An image has been grabbed.
   FLYCAPTURE_GRABBED_IMAGE,
   // A register has been read.
   FLYCAPTURE_REGISTER_READ,
   // A block of registers has been read.
   FLYCAPTURE_REGISTER_READ_BLOCK,
   // A register has been written to.
   FLYCAPTURE_REGISTER_WRITE,
   // A block of registers has been written to.
   FLYCAPTURE_REGISTER_WRITE_BLOCK,   
} FlyCaptureMessageType;
 

//
// Description:
//  This structure is used to receive messages for specific cameras or
//  bus events.  The information received is message dependent.
//   
typedef struct FlyCaptureMessage
{
   // The type of message being received.
   FlyCaptureMessageType     msgType;
   
   // The message specific details.
   union
   {
      struct
      {
         // The bus number of the device.
         int                    iBusNumber;
         // The timestamp of the bus event.
         FlyCaptureSystemTime   stTimeStamp;
      } Reset;

      struct
      {
         // The full name of the device including model.
         char                   szDevice[128];
         // The serial number of the device.
         unsigned long          ulSerialNumber;
         // The bus number of the device.
         int                    iBusNumber;
         // The node number of the device.
         int                    iNodeNumber;
         // The timestamp of the bus event.
         FlyCaptureSystemTime   stTimeStamp;
      } Arrival;

      struct
      {
         // The full name of the device including model.
         char                   szDevice[128];
         // The serial number of the device.
         unsigned long          ulSerialNumber;
         // The bus number of the device.
         int                    iBusNumber;
         // The node number of the device.
         int                    iNodeNumber;
         // The timestamp of the bus event.
         FlyCaptureSystemTime   stTimeStamp;
      } Removal;

      struct
      {
         // The full name of the device including model.
         char                   szDevice[128];
         // The serial number of the device.
         unsigned long          ulSerialNumber;
         // The bus number of the device.
         int                    iBusNumber;
         // The node number of the device.
         int                    iNodeNumber;
         // The timestamp of the bus error.
         FlyCaptureSystemTime   stTimeStamp;
         // The bus error code.
         unsigned long          ulErrorCode;
      } BusError;
      
      struct
      {
         // The full name of the device including model.
         char                   szDevice[128];
         // The serial number of the device.
         unsigned long          ulSerialNumber;
         // The bus number of the device.
         int                    iBusNumber;
         // The node number of the device.
         int                    iNodeNumber;
         // The sequence number of the grabbed image.
         unsigned long          ulSequence;
         // The size (in bytes) of the grabbed image.
         unsigned long          ulBytes;
         // The timestamp of the grabbed image.
         FlyCaptureSystemTime   stTimeStamp;
      } Image;
      
      struct
      {
         // The full name of the device including model.
         char                   szDevice[128];
         // The serial number of the device.
         unsigned long          ulSerialNumber;
         // The bus number of the device.
         int                    iBusNumber;
         // The node number of the device.
         int                    iNodeNumber;
         // The register being read/written to.
         unsigned long          ulRegister;
         // The value being read/written.
         unsigned long          ulValue;
         // The error received after a read/write.
         char                   szError[16];
      } Register;
      
      struct
      {
         // The full name of the device including model.
         char                   szDevice[128];
         // The serial number of the device.
         unsigned long          ulSerialNumber;
         // The bus number of the device.
         int                    iBusNumber;
         // The node number of the device.
         int                    iNodeNumber;
         // The register being read/written to.
         unsigned long          ulRegister;
         // The number of register quadlets read/written to.
         unsigned long          ulNumberOfQuadlets;
         // The error received after a read/write.
         char                   szError[16];
      } RegisterBlock;   
   } Msg;
   
   // Reserved for future use.
   unsigned long             ulReserved[64];
} FlyCaptureMessage;


//=============================================================================
// Messaging Functions
//=============================================================================
// Group = Messaging

//-----------------------------------------------------------------------------
// Name: flycaptureSetMessageLoggingStatus()
//
// Description:
//   This function turns message logging on and off.
//
// Arguments:
//   context  - The FlyCaptureContext associated with the camera.
//   bEnable  - TRUE turns message logging on, FALSE turns it off.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureSetMessageLoggingStatus( 
                                  FlyCaptureContext context,
                                  bool              bEnable );


//-----------------------------------------------------------------------------
// Name: flycaptureGetMessageLoggingStatus()
//
// Description:
//   This function returns the status of message logging.
//
// Arguments:
//   context   - The FlyCaptureContext associated with the camera.
//   pbEnabled - TRUE if message logging is on, FALSE if it is off.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureGetMessageLoggingStatus(
                                  FlyCaptureContext context, 
                                  bool* pbEnabled );


//-----------------------------------------------------------------------------
// Name: flycaptureInitializeMessaging()
//
// Description:
//   This function initializes messaging for a specific camera or all buses.
//
// Arguments:
//   context         - The FlyCaptureContext associated with the camera.
//   ulSerialNumber  - The serial number of the camera of which to
//                     initialize messaging for.  Use FLYCAPTURE_BUS_MESSAGE as 
//                     the serial number to initialize bus event messages.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureInitializeMessaging( 
                              FlyCaptureContext context,
                              ULONG             ulSerialNumber );


//-----------------------------------------------------------------------------
// Name: flycaptureCloseMessaging()
//
// Description:
//   This function closes messaging for a specific camera or all buses.
//
// Arguments:
//   context         - The FlyCaptureContext associated with the camera.
//   ulSerialNumber  - The serial number of the camera of which to
//                     close messaging for. Use FLYCAPTURE_BUS_MESSAGE as the
//                     serial number to close bus event messaging.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureCloseMessaging( 
                         FlyCaptureContext context,
                         ULONG             ulSerialNumber );


//-----------------------------------------------------------------------------
// Name: flycaptureReceiveMessage()
//
// Description:
//   This function is used to receive messages from cameras or bus events.
//
// Arguments:
//   context         - The FlyCaptureContext associated with the camera.
//   ulSerialNumber  - The serial number of the camera of which to
//                     close messaging for.  Use FLYCAPTURE_BUS_MESSAGE as the
//                     serial number to receive bus event messages.
//   pMessage        - A pointer to the message which will be filled
//                     when a message is received.
//   polRead         - A pointer to an overlapped I/O structure.  The event
//                     handle of this structure is set when a message
//                     is received.
//
// Returns:
//   A FlyCaptureError indicating the success or failure of the function.
//
PGRFLYCAPTURE_API FlyCaptureError PGRFLYCAPTURE_CALL_CONVEN
flycaptureReceiveMessage( 
                         FlyCaptureContext  context,
                         ULONG              ulSerialNumber,
                         FlyCaptureMessage* pMessage,
                         OVERLAPPED*        polRead );


//-----------------------------------------------------------------------------
//
// Name: flycaptureBusErrorToString() 
//
// FlyCaptureMessage.BusError.ulErrorCode
// Description:
//   This function provides the user with a mechanism for decoding the error
//   code member returned as part of a FlycaptureMessage FLYCAPTURE_BUS_ERROR.
//   It returns a string containing a description of the provided error.
//
// Arguments:
//   ulErrorCode - The error code to be translated.
//
// Returns: 
//   A string containing a human readable interpretation of the error code.
//  
PGRFLYCAPTURE_API const char* PGRFLYCAPTURE_CALL_CONVEN
flycaptureBusErrorToString( 
                           unsigned long ulErrorCode  );



#ifdef __cplusplus
};
#endif

#endif // #ifndef __PGRFLYCAPTUREMESSAGING_H__