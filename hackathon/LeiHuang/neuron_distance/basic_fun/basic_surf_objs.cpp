/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




//some basic surf object IO functions that will be used by other programs
//by Hanchuan Peng
//090523
//090706: add swc reading and writing fucntions
//090716: add color save for apo file
//100119: merge with the io_ano_file.cpp

#include "basic_surf_objs.h"
#include "v3d_message.h"

#include <QString>

QList <CellAPO> readAPO_file(const QString& filename)
{
    QList <CellAPO> mylist;

	QFile qf(filename);
	if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
#ifndef DISABLE_V3D_MSG
		v3d_msg(QString("open file [%1] failed!").arg(filename));
#endif
		return mylist;
	}

//	PROGRESS_DIALOG("Loading Point cloud", widget);
//	PROGRESS_PERCENT(1); // 0 or 100 not be displayed. 081102

    int count = 0;
    mylist.clear();

    qDebug("-------------------------------------------------------");
    char _buf[10000]; //expand the size from 2000 to 10000. by PHC, 20121212.
    while (! qf.atEnd())
    {
        //char _buf[20000]; //move out of the loop, 20121212, PHC
        char *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++);
        if (buf[0]=='#' ||buf[0]=='\0')	continue;

        count++;
        CellAPO S;
        //memset(&S, 0, sizeof(S));

        QStringList qsl = QString(buf).split(",");
        if (qsl.size()==0)   continue;

        for (int i=0; i<qsl.size(); i++)
        {
        	qsl[i].truncate(200); //change from 99 to 200, 20121212, by PHC
        	if (i==0) S.n = qsl[i].toInt();
			else if (i==1) S.orderinfo = qsl[i];
        	else if (i==2) S.name = qsl[i]; //strcpy(S.name, qsl[i].toStdString().c_str()); //by PHC, 090219
        	else if (i==3) S.comment = qsl[i]; //by PHC, added on 090220
        	else if (i==4) S.z = qsl[i].toFloat();
        	else if (i==5) S.x = qsl[i].toFloat();
        	else if (i==6) S.y = qsl[i].toFloat();
			else if (i==7) S.pixmax = qsl[i].toFloat();
        	else if (i==8) S.intensity = qsl[i].toFloat();
			else if (i==9) S.sdev = qsl[i].toFloat();
        	else if (i==10) S.volsize = qsl[i].toFloat();
			else if (i==11) S.mass = qsl[i].toFloat();
        	else if (i==15) S.color.r = qsl[i].toUInt();
        	else if (i==16) S.color.g = qsl[i].toUInt();
        	else if (i==17) S.color.b = qsl[i].toUInt();
        }
        //qDebug("%s  ///  %d %s (%g %g %g) %g %g (%d %d %d)", buf, S.n, qPrintable(S.name), S.x, S.y, S.z, S.intensity, S.volsize, S.color.r,S.color.g,S.color.b);
		if (qsl.size()-1<8)
			S.intensity = rand()%255;
		if (qsl.size()-1<10)
			S.volsize = 1;
		if (qsl.size()-1<15)
			S.color = random_rgba8(255);
		S.on = true;
        //if (! listCell.contains(S)) // 081024
        {
        	mylist.append(S);
		}
    }
    qDebug("---------------------read %d lines, %d remained lines", count, mylist.size());

	return mylist;
}

bool writeAPO_file(const QString& filename, const QList <CellAPO> & listCell)
{
	QString curFile = filename;
	if (curFile.trimmed().isEmpty()) //then open a file dialog to choose file
	{
		curFile = QFileDialog::getSaveFileName(0,
											   "Select a APO (text, csv format) file to save the point cloud... ",
											   ".apo",
											   QObject::tr("Vaa3D/VANO point cloud (*.apo);;(*.*)"
											   ));
#ifndef DISABLE_V3D_MSG
		v3d_msg(QString("save file: %1").arg(curFile), false);
#endif

		if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
			return false;
	}

    FILE * fp = fopen(curFile.toLatin1(), "wt");
	if (!fp)
	{
#ifndef DISABLE_V3D_MSG
		v3d_msg("Fail to save point cloud to file.");
#endif
		return false;
	}

	fprintf(fp, "##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b\n");
	CellAPO * p_pt=0;
	for (int i=0;i<listCell.size(); i++)
	{
		//then save
		p_pt = (CellAPO *)(&(listCell.at(i)));
//		fprintf(fp, "%ld, %s, %s,%s, %ld,%ld,%ld, %5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,,%d,%d,%d\n",  
        fprintf(fp, "%ld, %s, %s,%s, %5.3f,%5.3f,%5.3f, %5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,,%d,%d,%d\n", //change from V3DLONG type to float, 20121212, by PHC
				p_pt->n, //i+1,
				qPrintable(p_pt->orderinfo),
				qPrintable(p_pt->name),
				qPrintable(p_pt->comment),

                //change from V3DLONG type to float, 20121212, by PHC
//				V3DLONG(p_pt->z+0.5),
//				V3DLONG(p_pt->x+0.5),
//				V3DLONG(p_pt->y+0.5),
				p_pt->z,
				p_pt->x,
				p_pt->y,
                
				p_pt->pixmax,
				p_pt->intensity,
				p_pt->sdev,
				p_pt->volsize,
				p_pt->mass,
				//the three strings here are reserved for other annotations
				//now with three color fields
				p_pt->color.r,
				p_pt->color.g,
				p_pt->color.b
		);
	}

	fclose(fp);
#ifndef DISABLE_V3D_MSG
	v3d_msg(QString("done with saving file: %1").arg(curFile), false);
#endif

	return true;
}

QList <ImageMarker> readMarker_file(const QString & filename)
{
	QList <ImageMarker> tmp_list;

	QFile qf(filename);
	if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
#ifndef DISABLE_V3D_MSG
		v3d_msg(QString("open file [%1] failed!").arg(filename));
#endif
		return tmp_list;
	}

	V3DLONG k=0;
    while (! qf.atEnd())
    {
		char curline[2000];
        qf.readLine(curline, sizeof(curline));
		k++;
		{
			if (curline[0]=='#' || curline[0]=='x' || curline[0]=='X' || curline[0]=='\0') continue;

			QStringList qsl = QString(curline).trimmed().split(",");
			int qsl_count=qsl.size();
			if (qsl_count<3)   continue;

			ImageMarker S;

			S.x = qsl[0].toFloat();
			S.y = qsl[1].toFloat();
			S.z = qsl[2].toFloat();
			S.radius = (qsl_count>=4) ? qsl[3].toInt() : 0;
			S.shape = (qsl_count>=5) ? qsl[4].toInt() : 1;
			S.name = (qsl_count>=6) ? qPrintable(qsl[5].trimmed()) : "";
			S.comment = (qsl_count>=7) ? qPrintable(qsl[6].trimmed()) : "";

			S.color = random_rgba8(255);
			if (qsl_count>=8) S.color.r = qsl[7].toUInt();
			if (qsl_count>=9) S.color.g = qsl[8].toUInt();
			if (qsl_count>=10) S.color.b = qsl[9].toUInt();

			S.type = (S.x==-1 || S.y==-1 || S.z==-1) ? 0 : 2;

			S.on = true; //listLoc[i].on;        //true;
			S.selected = false;

			tmp_list.append(S);
		}
	}

	return tmp_list;
}

bool writeMarker_file(const QString & filename, const QList <ImageMarker> & listMarker)
{
	QString curFile = filename;
	if (curFile.trimmed().isEmpty()) //then open a file dialog to choose file
	{
		curFile = QFileDialog::getSaveFileName(0,
											   "Select a MARKER (text, csv format) file to save the landmarks... ",
											   ".marker",
											   QObject::tr("V3D landmark file (*.marker);;(*.*)"
														   ));
#ifndef DISABLE_V3D_MSG
		v3d_msg(QString("save file: %1").arg(curFile), false);
#endif

		if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
			return false;
	}

    FILE * fp = fopen(curFile.toLatin1(), "wt");
	if (!fp)
	{
#ifndef DISABLE_V3D_MSG
		v3d_msg(QString("Could not open the file to save the landmarks."));
#endif
		return false;
	}

	fprintf(fp, "##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n");
	ImageMarker * p_pt=0;
	for (int i=0;i<listMarker.size(); i++)
	{
		//then save
		p_pt = (ImageMarker *)(&(listMarker.at(i)));
		fprintf(fp, "%5.3f, %5.3f, %5.3f, %d, %d, %s, %s, %d,%d,%d\n",
				// 090617 RZC: marker file is 1-based
				p_pt->x,
				p_pt->y,
				p_pt->z,
				int(p_pt->radius), p_pt->shape,
				qPrintable(p_pt->name), qPrintable(p_pt->comment),
				p_pt->color.r,p_pt->color.g,p_pt->color.b);
	}

	fclose(fp);
//#ifndef DISABLE_V3D_MSG
	v3d_msg(QString("done with saving file: %1").arg(curFile), false);
//#endif
	return true;
}

NeuronTree readSWC_file(const QString& filename)
{
	NeuronTree nt;
    nt.file = QFileInfo(filename).absoluteFilePath();
	QFile qf(filename);
	if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
#ifndef DISABLE_V3D_MSG
		v3d_msg(QString("open file [%1] failed!").arg(filename));
#endif
		return nt;
	}

	int count = 0;
    QList <NeuronSWC> listNeuron;
	QHash <int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();
	QString name = "";
	QString comment = "";

    qDebug("-------------------------------------------------------");
    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space

        //  add #name, #comment
        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
        	if (buf[1]=='n'&&buf[2]=='a'&&buf[3]=='m'&&buf[4]=='e'&&buf[5]==' ')
        		name = buf+6;
        	if (buf[1]=='c'&&buf[2]=='o'&&buf[3]=='m'&&buf[4]=='m'&&buf[5]=='e'&&buf[6]=='n'&&buf[7]=='t'&&buf[8]==' ')
        		comment = buf+9;

        	continue;
       	}

        count++;
        NeuronSWC S;

        QStringList qsl = QString(buf).trimmed().split(" ",QString::SkipEmptyParts);
        if (qsl.size()==0)   continue;

        for (int i=0; i<qsl.size(); i++)
        {
        	qsl[i].truncate(99);
        	if (i==0) S.n = qsl[i].toInt();
        	else if (i==1) S.type = qsl[i].toInt();
        	else if (i==2) S.x = qsl[i].toFloat();
        	else if (i==3) S.y = qsl[i].toFloat();
        	else if (i==4) S.z = qsl[i].toFloat();
			else if (i==5) S.r = qsl[i].toFloat();
        	else if (i==6) S.pn = qsl[i].toInt();
            //the ESWC extension, by PHC, 20120217
        	else if (i==7) S.seg_id = qsl[i].toInt();
        	else if (i==8) S.level = qsl[i].toInt();
            else if (i==9) S.creatmode = qsl[i].toInt();
            else if (i==10) S.timestamp = qsl[i].toInt();
             else if (i==11) S.tfresindex = qsl[i].toInt();
	//change ESWC format to adapt to flexible feature number, by WYN, 20150602
        	else 
		S.fea_val.append(qsl[i].toFloat());
       }

        //if (! listNeuron.contains(S)) // 081024
        {
        	listNeuron.append(S);
        	hashNeuron.insert(S.n, listNeuron.size()-1);
        }
    }
    qDebug("---------------------read %d lines, %d remained lines", count, listNeuron.size());

    if (listNeuron.size()<1)
    	return nt;


	//now update other NeuronTree members

    nt.n = 1; //only one neuron if read from a file
    nt.listNeuron = listNeuron;
    nt.hashNeuron = hashNeuron;
    nt.color = XYZW(0,0,0,0); /// alpha==0 means using default neuron color, 081115
    nt.on = true;
    nt.name = name.remove('\n'); if (nt.name.isEmpty()) nt.name = QFileInfo(filename).baseName();
    nt.comment = comment.remove('\n');

	return nt;
}

bool writeSWC_file(const QString& filename, const NeuronTree& nt, const QStringList *infostring)
{
	QString curFile = filename;
	if (curFile.trimmed().isEmpty()) //then open a file dialog to choose file
	{
		curFile = QFileDialog::getSaveFileName(0,
											   "Select a SWC file to save the neuronal or relational data... ",
											   ".swc",
											   QObject::tr("Neuron structure file (*.swc);;(*.*)"
														   ));
#ifndef DISABLE_V3D_MSG
		v3d_msg(QString("save file: %1").arg(curFile), false);
#endif
        
		if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
			return false;
	}
    
    FILE * fp = fopen(curFile.toLatin1(), "wt");
	if (!fp)
	{
#ifndef DISABLE_V3D_MSG
		v3d_msg("Could not open the file to save the neuron.");
#endif
		return false;
	}
    
	fprintf(fp, "#name %s\n", qPrintable(nt.name.trimmed()));
	fprintf(fp, "#comment %s\n", qPrintable(nt.comment.trimmed()));
    
    if (infostring)
    {
        for (int j=0;j<infostring->size();j++)
            fprintf(fp, "#%s\n", qPrintable(infostring->at(j).trimmed()));
    }
    
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");
	NeuronSWC * p_pt=0;
	for (int i=0;i<nt.listNeuron.size(); i++)
	{
		p_pt = (NeuronSWC *)(&(nt.listNeuron.at(i)));
		fprintf(fp, "%ld %d %5.3f %5.3f %5.3f %5.3f %ld\n",
				p_pt->n, p_pt->type, p_pt->x, p_pt->y, p_pt->z, p_pt->r, p_pt->pn);
	}
    
	fclose(fp);
#ifndef DISABLE_V3D_MSG
	v3d_msg(QString("done with saving file: ")+filename, false);
#endif
	return true;
}

bool writeESWC_file(const QString& filename, const NeuronTree& nt)
{
	QString curFile = filename;
	if (curFile.trimmed().isEmpty()) //then open a file dialog to choose file
	{
		curFile = QFileDialog::getSaveFileName(0,
											   "Select a ESWC (enhanced SWC) file to save the neuronal or relational data... ",
											   ".eswc",
											   QObject::tr("Enhanced Neuron structure file (*.eswc);;(*.*)"
														   ));
#ifndef DISABLE_V3D_MSG
		v3d_msg(QString("save file: %1").arg(curFile), false);
#endif
        
		if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
			return false;
	}
    
    FILE * fp = fopen(curFile.toLatin1(), "wt");
	if (!fp)
	{
#ifndef DISABLE_V3D_MSG
		v3d_msg("Could not open the file to save the neuron.");
#endif
		return false;
	}
    
	fprintf(fp, "#name %s\n", qPrintable(nt.name.trimmed()));
	fprintf(fp, "#comment %s\n", qPrintable(nt.comment.trimmed()));
    
    fprintf(fp, "##n,type,x,y,z,radius,parent,seg_id,level,mode,timestamp,feature_value\n");
	NeuronSWC * p_pt=0;
	for (int i=0;i<nt.listNeuron.size(); i++)
	{
		p_pt = (NeuronSWC *)(&(nt.listNeuron.at(i)));
        fprintf(fp, "%ld %d %5.3f %5.3f %5.3f %5.3f %ld %ld %ld %d %.0f",
                p_pt->n, p_pt->type, p_pt->x, p_pt->y, p_pt->z, p_pt->r, p_pt->pn, p_pt->seg_id, p_pt->level, p_pt->creatmode, p_pt->timestamp);
		for (int j=0;j<p_pt->fea_val.size();j++)
            fprintf(fp, " %.5f", p_pt->fea_val.at(j));
		fprintf(fp, "\n");
	}
	fclose(fp);
#ifndef DISABLE_V3D_MSG
	v3d_msg(QString("done with saving file: ")+filename, false);
#endif
	return true;
}


bool importKeywordString2FileType(QString ss, QString vv, QString basedir, P_ObjectFileType & cc)
{
	QString tss = ss.trimmed().toUpper();
	QString tvv = vv.trimmed();
	
	if (tss.isEmpty() || tvv.isEmpty())
	{
		v3d_msg("Either the keyword string or the value string is empty. Do nothing.\n");
		return false;
	}
	QDir tmpdir;
	
	tvv = basedir+tvv;
    printf("Locating file under the basedir [%s]... \n", tvv.toLatin1().data());
	if (tmpdir.exists(tvv)==false)
	{
		tvv = vv.trimmed();
        printf("!!! Did not find the file under the basedir. Re-Locating file using the directly supplied name [%s]... \n", tvv.toLatin1().data());
		if (tmpdir.exists(tvv)==false)
		{
			v3d_msg(QString("The specified file is not found. Do nothing.\n[%1]\n").arg(tvv));	//modified by Lei Qu 2009/11/16: show non-exist file's name
			return false;
		}
	}
	if (tss=="GRAYIMG" || tss=="RAWIMG")
	{
		cc.raw_image_file_list.push_back(tvv);
		printf("\t--> push a raw image file [%s]\n", qPrintable(tvv));
	}
	else if (tss=="MASKIMG" || tss=="LABELFIELD")
	{
		cc.labelfield_image_file_list.push_back(tvv);
		printf("\t--> push a label field (mask) image file [%s]\n", qPrintable(tvv));
	}
	else if (tss=="ANOFILE")
	{
		cc.annotation_file_list.push_back(tvv);
		printf("\t--> push an annotation file (csv) [%s]\n", qPrintable(tvv));
	}
	else if (tss=="SWCFILE" || tss=="NEURON")
	{
		cc.swc_file_list.push_back(tvv);
		printf("\t--> push a swc file [%s]\n", qPrintable(tvv));
	}
	else if (tss=="APOFILE" || tss=="POINTCLOUDFILE")
	{
		cc.pointcloud_file_list.push_back(tvv);
		printf("\t--> push a point cloud file [%s]\n", qPrintable(tvv));
	}
    // Added by Peng Xie, 06-05-2019
    else if (tss=="MARKERFILE")
    {
        cc.marker_file_list.push_back(tvv);
        printf("\t--> push a marker file [%s]\n", qPrintable(tvv));
    }
	else if (tss=="SURFILE" || tss=="SURFACE")
	{
		cc.surface_file_list.push_back(tvv);
		printf("\t--> push a surface file [%s]\n", qPrintable(tvv));
	}
	else
	{
		printf("\t--> Unknown keyword [%s]. Do nothing.\n", qPrintable(tss));
		return false;
	}
	
	return true;
}

bool loadAnoFile(QString openFileNameLabel, P_ObjectFileType & cc)
{
	//load all other data
	
	QFile file(openFileNameLabel);
	QString baseName = openFileNameLabel.section('/', -1);
	QString baseDir = openFileNameLabel;
	baseDir.chop(baseName.size());
	
	file.open(QIODevice::ReadOnly);
	QString tmp = file.readAll();
	file.close();
	
	if (tmp.isEmpty())
		return false;
	
    QStringList tmpList = tmp.split(QString("\n"));
	if (tmpList.size()<=0)
	{
		return false; //must not be a valid file
	}
	
	int cnt=0;
	for (int i=0;i<tmpList.size(); i++)
	{
		//printf("(%s)\n", tmpList.at(i).toAscii().data());
		QStringList itemList;
		
		if (tmpList.at(i).isEmpty() || tmpList.at(i).at(0)=='#') //a comment line, do nothing
			continue;
		
		itemList = tmpList.at(i).split("=");
		if (itemList.size()==2)
		{
			if (importKeywordString2FileType(itemList.at(0).trimmed().toUpper(), itemList.at(1).trimmed(), baseDir, cc)==true)
			{
				cnt++;
				continue;
			}
		}
		
		itemList = tmpList.at(i).split("+=");
		if (itemList.size()==2)
		{
			if (importKeywordString2FileType(itemList.at(0).trimmed().toUpper(), itemList.at(1).trimmed(), baseDir, cc)==true)
			{
				cnt++;
				continue;
			}
		}
	}
	
	return (cnt>0) ? true : false;
}	

bool saveAnoFile(QString openFileNameLabel, const P_ObjectFileType & cc) // a convenient overloading function for case there is no comments
{
	QStringList qsl;
	return saveAnoFile(openFileNameLabel, cc, qsl);
}

bool saveAnoFile(QString openFileNameLabel, const P_ObjectFileType & cc, const QStringList & commentStrList)
{
	//get basic info of the .ano linker file
	QFileInfo fi(openFileNameLabel);
	QString baseName = fi.fileName();
	QString baseDir = fi.absolutePath(); //note not to call  absoluteFilePath() which include the file name as well
	
	//now open file to write
	
	FILE * fp=0;
	fp = fopen((char *)qPrintable(openFileNameLabel), "wt");
	if (!fp)
	{
		v3d_msg("Fail to open file to write.");
		return false;
	}
	
	//now save
	int i;
	QString outstr;
	
	for (i=0;i<commentStrList.size();i++)
		fprintf(fp, "# %s\n", qPrintable(commentStrList.at(i)));
	
	for (i=0;i<cc.raw_image_file_list.size();i++)
	{
		fi.setFile(cc.raw_image_file_list.at(i));
		//printf("[---%s %s %s %s]\n",qPrintable(cc.raw_image_file_list.at(i)), qPrintable(fi.fileName()), qPrintable(baseDir), qPrintable(fi.absolutePath()));
		outstr = (baseDir==fi.absolutePath()) ? fi.fileName() : cc.raw_image_file_list.at(i); 
		fprintf(fp, "GRAYIMG=%s\n", qPrintable(outstr));
	}
	
	for (i=0;i<cc.labelfield_image_file_list.size();i++)
	{
		fi.setFile(cc.labelfield_image_file_list.at(i));
		outstr = (baseDir==fi.absolutePath()) ? fi.fileName() : cc.labelfield_image_file_list.at(i); 
		fprintf(fp, "MASKIMG=%s\n", qPrintable(outstr));
	}
	
	for (i=0;i<cc.annotation_file_list.size();i++)
	{
		fi.setFile(cc.annotation_file_list.at(i));
		outstr = (baseDir==fi.absolutePath()) ? fi.fileName() : cc.annotation_file_list.at(i); 
		fprintf(fp, "ANOFILE=%s\n", qPrintable(outstr));
	}
	
	for (i=0;i<cc.swc_file_list.size();i++)
	{
		fi.setFile(cc.swc_file_list.at(i));
		outstr = (baseDir==fi.absolutePath()) ? fi.fileName() : cc.swc_file_list.at(i); 
		fprintf(fp, "SWCFILE=%s\n", qPrintable(outstr));
	}
	
	for (i=0;i<cc.pointcloud_file_list.size();i++)
	{
		fi.setFile(cc.pointcloud_file_list.at(i));
		outstr = (baseDir==fi.absolutePath()) ? fi.fileName() : cc.pointcloud_file_list.at(i); 
		fprintf(fp, "APOFILE=%s\n", qPrintable(outstr));
	}
	
	for (i=0;i<cc.surface_file_list.size();i++)
	{
		fi.setFile(cc.surface_file_list.at(i));
		outstr = (baseDir==fi.absolutePath()) ? fi.fileName() : cc.surface_file_list.at(i); 
		fprintf(fp, "SURFACE=%s\n", qPrintable(outstr));
	}
	
	//finish up
	
	if (fp) fclose(fp);
	return true;
}	


