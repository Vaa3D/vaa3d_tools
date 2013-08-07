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
        V3DLONG sz2_new = ceil(double(endnum-startnum+1))/increment;
        Image4DSimple p4DImage;
        p4DImage.createBlankImage(sz0, sz1, sz2_new, sz3, subject->getDatatype());

		V3DLONG k=0, c, pagesz;
        for (V3DLONG i=startnum, k=0; i<=endnum, k<sz2_new; i+=increment, k++)
        {
            switch (subject->getDatatype())
            {
            case V3D_UINT8:
				pagesz = sz0*sz1*subject->getUnitBytes();
                for (c=0;c<sz3;c++)
                {
                printf("c=%d i=%d k=%d\n", c, i, k);
            		unsigned char *dst = p4DImage.getRawDataAtChannel(c) + k*pagesz;
            		unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz;
	            	memcpy(dst, src, pagesz);
                }
                break;
            case V3D_UINT16:
				pagesz = sz0*sz1*subject->getUnitBytes();
                for (c=0;c<sz3;c++)
                {
            		unsigned char *dst = p4DImage.getRawDataAtChannel(c) + k*pagesz;
            		unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz;
            		memcpy(dst, src, pagesz);
                }
            	break;
            case V3D_FLOAT32:
				pagesz = sz0*sz1*subject->getUnitBytes();
                for (c=0;c<sz3;c++)
                {
            		unsigned char *dst = p4DImage.getRawDataAtChannel(c) + k*pagesz;
            		unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz;
            		memcpy(dst, src, pagesz);
                }
                break;
            default: v3d_msg("You should never see this. The data is not returned meaningfully. Check your data and code.");
                break;
            }
        }

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
		v3d_msg(tr("A program to extract selected Z slices. "
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
		v3d_msg("to implement");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("to implement");
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
