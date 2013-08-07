/* extractZSlices_plugin.cpp
 * A program to extract selected Z slices
 * 2013-08-05 : by Hanchuan Peng
 */
 
#include "v3d_message.h"
#include <QInputDialog>
#include "extractZSlices_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(extractZSlices, extractZSlices);
 

bool parseFormatString(QString t, V3DLONG & startnum, V3DLONG & increment, V3DLONG & endnum, V3DLONG sz2);
bool extract_z_slices(Image4DSimple * subject, Image4DSimple & outImage,
             V3DLONG startnum, V3DLONG increment, V3DLONG endnum);

QStringList extractZSlices::menulist() const
{
	return QStringList() 
		<<tr("extract a subset of Z slices to a stack")
		<<tr("about");
}

QStringList extractZSlices::funclist() const
{
	return QStringList()
		<<tr("subzslices2stack")
		<<tr("help");
}

void extractZSlices::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("extract a subset of Z slices to a stack"))
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
                                             tr("Range (startslice:increment:endslicenum)    (startslice is 1-based, endslicenum can be 'e', increment can be 1,2,...)    (example: 1:2:e)"),
                                             QLineEdit::Normal,
                                             "1:3:e", &ok);
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
        if (!extract_z_slices(subject, p4DImage, startnum, increment, endnum))
            return;

        //==

        v3dhandle newwin;
        if(QMessageBox::Yes == QMessageBox::question (0, "", 
        	QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
            newwin = callback.currentImageWindow();
        else
            newwin = callback.newImageWindow();

        callback.setImage(newwin, &p4DImage);
        callback.setImageName(newwin, m_InputFileName + "_extracted_image_" + text + ".v3draw");
        callback.updateImageWindow(newwin);
	}
	else
	{
        v3d_msg(tr("A program to extract selected Z slices and export to an image stack. "
			"Developed by Hanchuan Peng, 2013-08-05"));
	}
}

bool extractZSlices::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("subzslices2stack"))
	{
        if (infiles.size()<1 || inparas.size()<1 || output.size()<1 )
        {
            v3d_msg("No input image or no parameters or no output image is specified.");
            return false;
        }

        Image4DSimple inimg, outimg;

        inimg.loadImage(infiles.at(0));
        if (!inimg.valid())
        {
            v3d_msg("Fail to load the specified input image.");
            return false;
        }

        V3DLONG startnum, increment, endnum;
        QString paratext = inparas.at(0);
        if (!paratext.isEmpty())
        {
            if (!parseFormatString(paratext, startnum, increment, endnum, inimg.getZDim()))
            {
                v3d_msg("The format of the string is not valid. Do nothing.");
                return false;
            }
        }
        else
            return false;

        if (!extract_z_slices(&inimg, outimg, startnum, increment, endnum))
            return false;

        if (!outimg.saveImage(outfiles.at(0)))
            return false;
	}
	else if (func_name == tr("help"))
	{
        v3d_msg("This plugin extracts a subset of slices and export to a stack (and save to a file)", 0);
        v3d_msg("Usage: v3d -x dll_name -f subzslices2stack -i input_image_file -p parameters_string -o output_image_file", 0);
        v3d_msg("parameters_string: Range (startslice:increment:endslicenum)    (startslice is 1-based, endslicenum can be 'e', increment can be 1,2,...)    (example: 1:2:e)", 0);
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


bool extract_z_slices(Image4DSimple * subject, Image4DSimple & outImage,
             V3DLONG startnum, V3DLONG increment, V3DLONG endnum)
{
    if (!subject || !subject->valid())
        return false;

    V3DLONG sz0 = subject->getXDim();
    V3DLONG sz1 = subject->getYDim();
    V3DLONG sz2 = subject->getZDim();
    V3DLONG sz3 = subject->getCDim();

    //copy data
    V3DLONG sz2_new = ceil(double(endnum-startnum+1)/increment);
    //qDebug() << "sz2_new=" << sz2_new << "end=" << endnum << "start=" << startnum << "incre=" << increment;

    outImage.createBlankImage(sz0, sz1, sz2_new, sz3, subject->getDatatype());
    if (!outImage.valid())
        return false;

    V3DLONG k=0, c, pagesz;
    for (V3DLONG i=startnum, k=0; i<=endnum, k<sz2_new; i+=increment, k++)
    {
        switch (subject->getDatatype())
        {
        case V3D_UINT8:
            pagesz = sz0*sz1*subject->getUnitBytes();
            for (c=0;c<sz3;c++)
            {
                //printf("c=%d i=%d k=%d\n", c, i, k);
                unsigned char *dst = outImage.getRawDataAtChannel(c) + k*pagesz;
                unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz;
                memcpy(dst, src, pagesz);
            }
            break;
        case V3D_UINT16:
            pagesz = sz0*sz1*subject->getUnitBytes();
            for (c=0;c<sz3;c++)
            {
                unsigned char *dst = outImage.getRawDataAtChannel(c) + k*pagesz;
                unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz;
                memcpy(dst, src, pagesz);
            }
            break;
        case V3D_FLOAT32:
            pagesz = sz0*sz1*subject->getUnitBytes();
            for (c=0;c<sz3;c++)
            {
                unsigned char *dst = outImage.getRawDataAtChannel(c) + k*pagesz;
                unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz;
                memcpy(dst, src, pagesz);
            }
            break;
        default: v3d_msg("You should never see this. The data is not returned meaningfully. Check your data and code.");
            return false;
            break;
        }
    }

    return true;
}
