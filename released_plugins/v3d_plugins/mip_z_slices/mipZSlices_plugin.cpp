/* MIPZSlices_plugin.cpp
 * A program to calculate MIP of selected Z slices
 * 2013-08-05 : by Hanchuan Peng
 */
 
#include "v3d_message.h"
#include <QInputDialog>
#include "mipZSlices_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(mipZSlices, MIPZSlices);
 

bool parseFormatString(QString t, V3DLONG & startnum, V3DLONG & increment, V3DLONG & endnum, V3DLONG sz2);
bool mip_z_slices(Image4DSimple * subject, Image4DSimple & outImage,
             V3DLONG startnum, V3DLONG increment, V3DLONG endnum);

QStringList MIPZSlices::menulist() const
{
	return QStringList() 
		<<tr("calculate the MIP of a subset of Z slices")
		<<tr("about");
}

QStringList MIPZSlices::funclist() const
{
	return QStringList()
		<<tr("mip_zslices")
		<<tr("help");
}

void MIPZSlices::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("calculate the MIP of a subset of Z slices"))
	{
        v3dhandle curwin = callback.currentImageWindow();
        Image4DSimple* subject = callback.getImage(curwin);
        if (!curwin || !subject)
        {
            v3d_msg("You don't have any image open in the main window.");
            return;
        }

        V3DLONG sz0 = subject->getXDim();
        V3DLONG sz1 = subject->getYDim();
        V3DLONG sz2 = subject->getZDim();
        V3DLONG sz3 = subject->getCDim();

        //

        V3DLONG startnum, increment, endnum;

        bool ok;
        QString text = QInputDialog::getText(0, tr("Enter the range and increment"),
                                             tr("Range (startslice:increment:endslicenum)    (startslice is 1-based, endslicenum can be 'e', increment can be 1,2,...)    (examples: 1:2:e for every other Z-slice, 1:1:e for all Z-slices)"),
                                             QLineEdit::Normal,
                                             "1:1:e", &ok);
        if (ok && !text.isEmpty())
        {
            if (!parseFormatString(text, startnum, increment, endnum, sz2))
            {
                v3d_msg("The format of the string is not valid. Do nothing.");
                return;
            }
        }
        else
            return;

        QString m_InputFileName = callback.getImageName(curwin);

        //copy data
        V3DLONG sz2_new = ceil(double(endnum-startnum+1)/increment);

        Image4DSimple p4DImage;
        if (!mip_z_slices(subject, p4DImage, startnum, increment, endnum))
            return;

        //==

        v3dhandle newwin;
        if(QMessageBox::Yes == QMessageBox::question (0, "", 
        	QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
            newwin = callback.currentImageWindow();
        else
            newwin = callback.newImageWindow();

        callback.setImage(newwin, &p4DImage);
        callback.setImageName(newwin, m_InputFileName + "_mip_" + text + ".v3draw");
        callback.updateImageWindow(newwin);
	}
	else
	{
        v3d_msg(tr("A program to calculate MIP of all or a selected subset of Z slices and export to an image stack. "
			"Developed by Hanchuan Peng, 2013-08-05"));
	}
}

bool MIPZSlices::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("mip_zslices"))
	{
        if (infiles.size()<1 || inparas.size()<1 || output.size()<1 )
        {
            v3d_msg("No input image or no parameters or no output image is specified.");
            return false;
        }

        Image4DSimple *inimg = callback.loadImage(infiles.at(0));
        Image4DSimple outimg;

        if(!inimg || !inimg->valid())
        {
            v3d_msg("Fail to load the specified input image.", 0);
            return false;
        }

        V3DLONG startnum, increment, endnum;
        QString paratext = inparas.at(0);
        if (!paratext.isEmpty())
        {
            if (!parseFormatString(paratext, startnum, increment, endnum, inimg->getZDim()))
            {
                v3d_msg("The format of the string is not valid. Do nothing.");
                return false;
            }
        }
        else
            return false;

        if (!mip_z_slices(inimg, outimg, startnum, increment, endnum))
            return false;

        if (!callback.saveImage(&outimg,outfiles.at(0)))
            return false;

        if (inimg) {delete inimg; inimg=0;}
	}
	else if (func_name == tr("help"))
	{
        v3d_msg("This plugin calculates the maximum intensity projection of all or a subset of Z-slices and export to a stack (and save to a file)", 0);
        v3d_msg("Usage: v3d -x dll_name -f mip_zslices -i input_image_file -p parameters_string -o output_image_file", 0);
        v3d_msg("parameters_string: Range (startslice:increment:endslicenum)    (startslice is 1-based, endslicenum can be 'e', increment can be 1,2,...)    (examples: 1:2:e for every other z-slice and 1:1:e for all z-slices)", 0);
    }
	else return false;
	
	return true;
}

bool parseFormatString(QString t, V3DLONG & startnum, V3DLONG & increment, V3DLONG & endnum, V3DLONG sz2)
{
    if (sz2<=0)
        return false;

    QStringList list = t.split(":");
    if (list.size()<2)
        return false;

    bool ok;

    startnum = list.at(0).toLong(&ok)-1;
    if (!ok)
        startnum = 0;

    if (list.size()==2)
    {
        increment = 1;
        endnum = list.at(1).toLong(&ok);
        if (!ok)
            endnum = sz2-1;
    }
    else
    {
        increment = list.at(1).toLong(&ok);
        if (!ok) increment = 1;
        endnum = list.at(2).toLong(&ok);
        if (!ok) endnum = sz2-1;
    }

    if (increment<0) //this will not reverse the order of all z slices in a stack. This can be enhanced later.
        increment = -increment;
    if (endnum>=sz2)
        endnum = sz2-1;
    if (startnum<0)
        startnum = 0;
    if (startnum>endnum)
    {
        V3DLONG tmp=endnum; endnum=startnum; startnum=tmp;
    }

    qDebug() << " start=" << startnum << " increment=" << increment << " end=" << endnum;
    return true;
}


bool mip_z_slices(Image4DSimple * subject, Image4DSimple & outImage,
             V3DLONG startnum, V3DLONG increment, V3DLONG endnum)
{
    if (!subject || !subject->valid())
        return false;

    V3DLONG sz0 = subject->getXDim();
    V3DLONG sz1 = subject->getYDim();
    V3DLONG sz2 = subject->getZDim();
    V3DLONG sz3 = subject->getCDim();

    //
    V3DLONG sz2_new = 1;

    outImage.createBlankImage(sz0, sz1, sz2_new, sz3, subject->getDatatype());
    if (!outImage.valid())
        return false;

    V3DLONG c, pagesz;
    for (V3DLONG i=startnum; i<=endnum; i+=increment)
    {
        switch (subject->getDatatype())
        {
        case V3D_UINT8:
            pagesz = sz0*sz1;
            for (c=0;c<sz3;c++)
            {
                unsigned char *dst = outImage.getRawDataAtChannel(c);
   	            unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz;
                if (i==startnum)
                {
        	        memcpy(dst, src, pagesz);
                }
                else
                {
    	            for (V3DLONG j=0; j<pagesz; j++)
    	            	if (dst[j]<src[j]) dst[j] = src[j];
                }
            }
            break;
        case V3D_UINT16:
            pagesz = sz0*sz1;
            for (c=0;c<sz3;c++)
            {
                unsigned char *dst = outImage.getRawDataAtChannel(c);
   	            unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz*subject->getUnitBytes();
                if (i==startnum)
                {
        	        memcpy(dst, src, pagesz);
                }
                else
                {
                	unsigned short int *dst1 = (unsigned short int *)dst;
                	unsigned short int *src1 = (unsigned short int *)src;
    	            for (V3DLONG j=0; j<pagesz; j++)
    	            	if (dst1[j]<src1[j]) dst1[j] = src1[j];
                }
            }
            break;
        case V3D_FLOAT32:
            pagesz = sz0*sz1;
            for (c=0;c<sz3;c++)
            {
                unsigned char *dst = outImage.getRawDataAtChannel(c);
   	            unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz*subject->getUnitBytes();
                if (i==startnum)
                {
        	        memcpy(dst, src, pagesz);
                }
                else
                {
                	float *dst1 = (float *)dst;
                	float *src1 = (float *)src;
    	            for (V3DLONG j=0; j<pagesz; j++)
    	            	if (dst1[j]<src1[j]) dst1[j] = src1[j];
                }
            }
            break;
        default: v3d_msg("You should never see this. The data is not returned meaningfully. Check your data and code.");
            return false;
            break;
        }
    }

    return true;
}
