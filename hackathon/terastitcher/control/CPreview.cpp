//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
*
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

#include "CPreview.h"
#include "CImport.h"
#include "presentation/PTabImport.h"
#include "presentation/PMain.h"
#include "StackStitcher.h"
#include <cv.h>
#include <highgui.h>

using namespace terastitcher;

CPreview* CPreview::uniqueInstance = NULL;

void CPreview::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

CPreview::~CPreview()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CPreview destroyed\n", this->thread()->currentThreadId());
    #endif
}

//automatically called when current thread is started
void CPreview::run()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CPreview::run() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        //obtaining desired slice
        StackStitcher stitcher(volume);
        stitcher.mergeTiles("", -1, -1, NULL, false, -1, -1, -1, -1, slice_index, slice_index+1,
                            false, false, S_SHOW_STACK_MARGIN, true, false, "tif", 8);

        //allocation of image data
        printf("allocation of image data\n");
        Image4DSimple* img = new Image4DSimple();
        char path[VM_STATIC_STRINGS_SIZE];
        sprintf(path, "%s/test_middle_slice.tif", CImport::instance()->getVolume()->getSTACKS_DIR());

        //loading slice using OpenCV
        printf("allocation of image data\n");
        IplImage* slice_img = cvLoadImage(path, CV_LOAD_IMAGE_GRAYSCALE);
        if(!slice_img)
            throw MyException(QString("Unable to load slice \"").append(path).append("\" to be shown into Vaa3D").toStdString().c_str());
        int width = slice_img->width;
        int height = slice_img->height;

        uint8* img_data = new uint8[width*height];
        int slice_img_step = slice_img->widthStep/sizeof(uchar);
        for(int i=0; i<height; i++)
        {
            uint8* slice_img_data = ((uint8*)slice_img->imageData)+i*slice_img_step;
            for(int j=0; j<width; j++)
                img_data[i*width +j] = slice_img_data[j];
        }

        //releasing memory and setting image data
        printf("releasing memory and setting image data\n");
        cvReleaseImage(&slice_img);
        img->setData(img_data, width, height, 1, 1, V3D_UINT8);
        img->setFileName(path);

        //everything went OK
        emit sendOperationOutcome(0, img);
    }
    catch(MyException& exception)   {emit sendOperationOutcome(&exception);}
    catch(const char* error)        {emit sendOperationOutcome(new MyException(error));}
}
