/* saveZSlices_plugin.cpp
 * A program to save selected Z slices
 * 2013-08-05 : by Hanchuan Peng
 */
 
#include "v3d_message.h"
#include <QInputDialog>
#include "saveZSlices_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(saveZSlices, saveZSlices);
 

bool parseFormatString(QString t, V3DLONG & startnum, V3DLONG & increment, V3DLONG & endnum, V3DLONG sz2);
bool save_z_slices(V3DPluginCallback2 & callback, Image4DSimple * subject,
             V3DLONG startnum, V3DLONG increment, V3DLONG endnum, QString filenameprefix);
bool save_z_slices(V3DPluginCallback2 & callback, QString subjectFileName,
             V3DLONG startnum, V3DLONG increment, V3DLONG endnum, QString filenameprefix);

QString gen_file_name(QString prefixstr, V3DLONG k, V3DLONG maxn, QString extstr);

QStringList saveZSlices::menulist() const
{
	return QStringList() 
        <<tr("save subset Z slices of the current opened image to a series of files")
        <<tr("save Z slices of an image file to a series of files")
        <<tr("about");
}

QStringList saveZSlices::funclist() const
{
	return QStringList()
        <<tr("save_z_slices_to_file_series")
		<<tr("help");
}


void saveZSlices::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("save subset Z slices of the current opened image to a series of files"))
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

        QString m_inputfile = callback.getImageName(curwin), m_file;
        if (m_inputfile.startsWith("http"))
            m_file = QDir::currentPath() + "/tmp.raw";
        else
            m_file = m_inputfile;

        QFileInfo m_info( m_file );

        QString dir = QFileDialog::getExistingDirectory(0, tr("Open Directory"),
                                                     m_info.absolutePath(),
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);


        //copy data
        V3DLONG sz2_new = ceil(double(endnum-startnum+1)/increment);

        if (!save_z_slices(callback, subject, startnum, increment, endnum, dir+"/z_"))
            return;

        v3d_msg("Done!");

	}
    else if (menu_name == tr("save Z slices of an image file to a series of files"))
    {
        QString m_file = QFileDialog::getOpenFileName(0, tr("Open File"),
                                                        "",
                                                        tr("Images (*.tif *.tiff *.lsm)"));

        V3DLONG startnum, increment, endnum;

        bool ok;
        QString text = QInputDialog::getText(0, tr("Enter the range and increment"),
                                             tr("Range (startslice:increment:endslicenum)    (startslice is 1-based, endslicenum can be 'e', increment can be 1,2,...)    (example: 1:2:e)"),
                                             QLineEdit::Normal,
                                             "1:3:e", &ok);
        if (ok && !text.isEmpty())
        {
            if (!parseFormatString(text, startnum, increment, endnum, -1))
            {
                v3d_msg("The format of the string is not valid. Do nothing.");
                return;
            }
        }
        else
            return;

        QFileInfo m_info( m_file );

        QString dir = QFileDialog::getExistingDirectory(0, tr("Open Directory"),
                                                     m_info.absolutePath(),
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);


        //copy data
        V3DLONG sz2_new = ceil(double(endnum-startnum+1)/increment);

        if (!save_z_slices(callback, m_file, startnum, increment, endnum, dir+"/z_"))
            return;

        v3d_msg("Done!");
    }
	else
	{
        v3d_msg(tr("A program to save selected Z slices and export a series of files (NOT an image stack). "
            "Developed by Hanchuan Peng, 2013"));
	}
}

bool saveZSlices::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("save_z_slices_to_file_series"))
	{
        if (infiles.size()<1 || inparas.size()<1 || output.size()<1 )
        {
            v3d_msg("No input image or no parameters or no output image is specified.");
            return false;
        }

        Image4DSimple *inimg = callback.loadImage(infiles.at(0));
        if (!inimg || !inimg->valid())
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

        if (!save_z_slices(callback, inimg, startnum, increment, endnum, outfiles.at(0)))
            return false;

        if (inimg) {delete inimg; inimg=0;}
	}
	else if (func_name == tr("help"))
	{
        v3d_msg("This plugin saves a subset of slices and export to file series", 0);
        v3d_msg("Usage: v3d -x dll_name -f save_z_slices_to_file_series -i input_image_file -p parameters_string -o output_image_file_prefix", 0);
        v3d_msg("parameters_string: Range (startslice:increment:endslicenum)    (startslice is 1-based, endslicenum can be 'e', increment can be 1,2,...)    (example: 1:2:e)", 0);
    }
	else return false;
	
	return true;
}

bool parseFormatString(QString t, V3DLONG & startnum, V3DLONG & increment, V3DLONG & endnum, V3DLONG sz2)
{
    if (sz2<=0)
        sz2 = (V3DLONG)1024*(V3DLONG)1024; //set as one million, by PHC 20131102

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

bool save_z_slices(V3DPluginCallback2 & callback, QString subjectFileName,
             V3DLONG startnum, V3DLONG increment, V3DLONG endnum, QString filenameprefix)
{
    if (subjectFileName.isEmpty())
        return false;

    V3DLONG sz0, sz1, sz2, sz3;
    V3DLONG sz2_new = ceil(double(endnum-startnum+1)/increment);

    QString curfile = filenameprefix;
    V3DLONG k=0, c, pagesz;
    for (V3DLONG i=startnum, k=0; i<=endnum; i+=increment, k++)
    {
        printf("Current slice no = %ld\n", i);

        Image4DSimple *subject = callback.loadImage((char *)qPrintable(subjectFileName), i);
        if (!subject || !subject->valid())
        {
            v3d_msg("Fail to load the specified input image, or get to the end slice of the valid data.", 0);
            break;
        }

        V3DLONG sz0 = subject->getXDim();
        V3DLONG sz1 = subject->getYDim();
        V3DLONG sz2 = subject->getZDim();
        V3DLONG sz3 = subject->getCDim();

        curfile = gen_file_name(filenameprefix, k, sz2_new, ".v3draw");
        callback.saveImage(subject, (char *)qPrintable(curfile));

        if (subject)
        {
            delete subject;
            subject=0;
        }
    }

    return true;
}

bool save_z_slices(V3DPluginCallback2 & callback, Image4DSimple * subject,
             V3DLONG startnum, V3DLONG increment, V3DLONG endnum, QString filenameprefix)
{
    if (!subject || !subject->valid())
        return false;

    V3DLONG sz0 = subject->getXDim();
    V3DLONG sz1 = subject->getYDim();
    V3DLONG sz2 = subject->getZDim();
    V3DLONG sz3 = subject->getCDim();

    //copy data
    V3DLONG sz2_new = ceil(double(endnum-startnum+1)/increment);

    Image4DSimple outImage;

    outImage.createBlankImage(sz0, sz1, 1, sz3, subject->getDatatype());
    if (!outImage.valid())
        return false;

    QString curfile = filenameprefix;
    V3DLONG k=0, c, pagesz;
    for (V3DLONG i=startnum, k=0; i<=endnum; i+=increment, k++)
    {
        switch (subject->getDatatype())
        {
        case V3D_UINT8:
            pagesz = sz0*sz1*subject->getUnitBytes();
            for (c=0;c<sz3;c++)
            {
                //printf("c=%d i=%d k=%d\n", c, i, k);
                unsigned char *dst = outImage.getRawDataAtChannel(c);
                unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz;
                memcpy(dst, src, pagesz);
            }
            curfile = gen_file_name(filenameprefix, k, sz2_new, ".v3draw");
            callback.saveImage(&outImage, (char *)qPrintable(curfile));

            break;

        case V3D_UINT16:
            pagesz = sz0*sz1*subject->getUnitBytes();
            for (c=0;c<sz3;c++)
            {
                unsigned char *dst = outImage.getRawDataAtChannel(c);
                unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz;
                memcpy(dst, src, pagesz);
            }
            curfile = gen_file_name(filenameprefix, k, sz2_new, ".v3draw");
            callback.saveImage(&outImage, (char *)qPrintable(curfile));

            break;

        case V3D_FLOAT32:
            pagesz = sz0*sz1*subject->getUnitBytes();
            for (c=0;c<sz3;c++)
            {
                unsigned char *dst = outImage.getRawDataAtChannel(c);
                unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz;
                memcpy(dst, src, pagesz);
            }
            curfile = gen_file_name(filenameprefix, k, sz2_new, ".v3draw");
            callback.saveImage(&outImage, (char *)qPrintable(curfile));
            break;

        default: v3d_msg("You should never see this. The data is not returned meaningfully. Check your data and code.");
            return false;
            break;
        }
    }

    return true;
}

QString gen_file_name(QString prefixstr, V3DLONG k, V3DLONG maxn, QString extstr)
{
    QString ks=""; ks.setNum(k);
    if (k<10)
        ks.prepend("0000");
    else if (k<100)
        ks.prepend("000");
    else if (k<1000)
        ks.prepend("00");
    else if (k<10000)
        ks.prepend("0");

    return (prefixstr + "_" + ks + extstr);
}
