//=============================================================================
// Copyright © 2009 Point Grey Research, Inc. All Rights Reserved.
// 
// This software is the confidential and proprietary information of Point
// Grey Research, Inc. ("Confidential Information").  You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with Point Grey Research, Inc. (PGR).
// 
// PGR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. PGR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================

//=============================================================================
// $Id: AVIRecorder.h,v 1.8 2009/05/12 00:09:27 soowei Exp $
//=============================================================================

#ifndef PGR_FC2_AVIRECORDER_H
#define PGR_FC2_AVIRECORDER_H

#include "FlyCapture2Platform.h"
#include "FlyCapture2Defs.h"

namespace FlyCapture2
{
    class Error;
    class Image;

    /** 
     * The AVIRecorder class provides the functionality for the user to record
     * images to an AVI file.
     */
    class FLYCAPTURE2_API AVIRecorder
    {
    public: 

        /**
         * Default constructor.
         */
        AVIRecorder();

        /**
         * Default destructor.
         */
        virtual ~AVIRecorder();
        
        /**
         * Open an AVI file in preparation for writing Images to disk.
         * The size of AVI files is limited to 2GB. The filenames are
         * automatically generated using the filename specified.
         *
         * @param pFileName The filename of the AVI file.
         * @param pOption Options to apply to the AVI file.
         *
         * @see AVIClose()
         *
         * @return An Error indicating the success or failure of the function.
         */ 
        virtual Error AVIOpen( 
            const char* pFileName, 
            AVIOption*  pOption );
 
        /**
         * Append an image to the AVI file.
         *
         * @param pImage The image to append.
         *
         * @return An Error indicating the success or failure of the function.
         */ 
        virtual Error AVIAppend( Image* pImage);
           
        /**
         * Close the AVI file.
         *
         * @see AVIOpen()
         *
         * @return An Error indicating the success or failure of the function.
         */ 
        virtual Error AVIClose( );

    private:

        AVIRecorder( const AVIRecorder& );
        AVIRecorder& operator=( const AVIRecorder& );

        struct AVIRecorderData; // Forward declaration

        AVIRecorderData* m_pAVIRecorderData;
    };
}

#endif //PGR_FC2_AVIRECORDER_H
