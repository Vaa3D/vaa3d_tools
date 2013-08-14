/****************************************************************************
**
 pointcloud_atlas_io.cpp
 by Hanchuan Peng
 2009_May-18
**
****************************************************************************/

#include <stdio.h>

#include "pointcloud_atlas_io.h"
#include "../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h"
#include "../../../../v3d_external/v3d_main/basic_c_fun/v3d_message.h"

bool savePointCloudAtlasInfoListToFile(const char *filename, const apoAtlasLinkerInfoAll & apoinfo)
{
	FILE *fp = fopen(filename, "wt");
	if (!fp)
	{
		printf("Unable to open the specified file [%s] to write. Do nothing.", filename);
		return false;
	}
	
	int i,j;

	fprintf(fp, "CLASS_REFERENCE=%s\n", qPrintable(apoinfo.referenceMarkerName));	
//	fprintf(fp, "REGISTRATION_TARGET=%s\n\n", qPrintable(apoinfo.regTargetFileName)); 
	
	for (i=0;i<apoinfo.items.size();i++)
	{
		fprintf(fp,"\n## %d class record\n",i+1);
		fprintf(fp, "CLASS=%s\n", qPrintable(apoinfo.items[i].className));
		fprintf(fp, "CLASS_REGISTERED_FILE::%s=%s\n", qPrintable(apoinfo.items[i].className), qPrintable(apoinfo.items[i].registeredFile));
//		fprintf(fp, "CLASS_CUTOFF_RATIO::%s=%f\n",  qPrintable(apoinfo.items[i].className), apoinfo.items[i].threratio); // FL, 20100909
		fprintf(fp, "CLASS_SIG_FOLDER::%s=%s\n", qPrintable(apoinfo.items[i].className), qPrintable(apoinfo.items[i].sigFolder));
		for (j=0;j<apoinfo.items[i].sigFolderFileList.size();j++)
			fprintf(fp, "CLASS_SIG_FILE::%s=%s\n", qPrintable(apoinfo.items[i].className), qPrintable(apoinfo.items[i].sigFolderFileList[j]));
		fprintf(fp, "CLASS_REF_FOLDER::%s=%s\n", qPrintable(apoinfo.items[i].className), qPrintable(apoinfo.items[i].refFolder));
		for (j=0;j<apoinfo.items[i].refFolderFileList.size();j++)
			fprintf(fp, "CLASS_REF_FILE::%s=%s\n", qPrintable(apoinfo.items[i].className), qPrintable(apoinfo.items[i].refFolderFileList[j]));
		fprintf(fp,"\n");
	}
	fclose(fp);
	
	return true;
}

bool loadPointCloudAtlasInfoListFromFile(const char *filename, apoAtlasLinkerInfoAll & apoinfo)
{
	QString openFileNameLabel = filename;
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
	
	QString curClassNameStr; //this is used for verifying the format of the point cloud atlas file format is correct
	int curClassIndex; //this is used for accessing the current CLASS
	
	int cnt=0; 
	for (int i=0;i<tmpList.size(); i++)
	{
		//printf("(%s)\n", tmpList.at(i).toAscii().data());
		QStringList itemList;
		
		itemList = tmpList.at(i).split("=");
		if (itemList.size()==2)
		{
			QStringList keywordlist = itemList.at(0).trimmed().split("::");
			if (keywordlist.size()==1) //then should treat as class name
			{
				if (itemList.at(0).trimmed().toUpper()=="CLASS" && itemList.at(1).trimmed().toUpper().isEmpty()==false)
				{
					QString valuestr = itemList.at(1).trimmed().toUpper();
					bool b_exist=false;
					for (int j=0;j<apoinfo.items.size();j++)
					{
						if (apoinfo.items[j].className==valuestr)
						{
							curClassIndex = j; //update to the current index
							b_exist = true;
						}	
					}
					if (!b_exist) //if not exist then add a record
					{
						apoAtlasLinkerInfo anew;
						anew.className = valuestr;
						apoinfo.items.append(anew);
						curClassIndex = apoinfo.items.size()-1; //update to be the last index
						cnt++;
					}
					curClassNameStr = itemList.at(1).trimmed().toUpper(); //always convert to Upper case for class name.  of course this is not empty 
				}
//				else if (itemList.at(0).trimmed().toUpper()=="REGISTRATION_TARGET" && itemList.at(1).trimmed().toUpper().isEmpty()==false)
//				{	
//					 apoinfo.regTargetFileName = itemList.at(1).trimmed();
//				}
				else if (itemList.at(0).trimmed().toUpper()=="CLASS_REFERENCE" && itemList.at(1).trimmed().toUpper().isEmpty()==false)
				{
					apoinfo.referenceMarkerName = itemList.at(1).trimmed();
				}
			}
			else if (keywordlist.size()==2) //then should treat as class property list
			{
				if ((curClassNameStr==keywordlist.at(1).trimmed().toUpper()) && (curClassIndex>=0 && curClassIndex<apoinfo.items.size())) //this must match to verify the format the point cloud atlas file format
				{
					QString keystr = keywordlist.at(0).trimmed().toUpper();
					QString valuestr = itemList.at(1).trimmed();
					if (keystr=="CLASS_SIG_FOLDER")
						apoinfo.items[curClassIndex].sigFolder = valuestr;
					else if (keystr=="CLASS_REF_FOLDER")
						apoinfo.items[curClassIndex].refFolder = valuestr;
					else if (keystr=="CLASS_SIG_FILE")
						apoinfo.items[curClassIndex].sigFolderFileList.append(valuestr);
					else if (keystr=="CLASS_REF_FILE")
						apoinfo.items[curClassIndex].refFolderFileList.append(valuestr);
					else if (keystr=="CLASS_REGISTERED_FILE")
						apoinfo.items[curClassIndex].registeredFile = valuestr;
//					else if (keystr=="CLASS_CUTOFF_RATIO")
//						apoinfo.items[curClassIndex].threratio = valuestr; // FL, 20100909
					//otherwise skip and do nothing
				}
			}
			//otherwise skip this line
		}
	}
	
	//return (cnt>0 && !apoinfo.regTargetFileName.isEmpty()) ? true : false;
	return (cnt>0) ? true : false; //allow return an empty string so Fuhui can verify if automatically determine the reference target for registration
	
	return true;
}


bool loadPointCloudAtlasConfigFile(const char *filename, atlasConfig & cfginfo)
{
	QString openFileNameLabel = filename;
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
    
    cfginfo.cellTypeFileName.clear();
//    cfginfo.saveRegDataTag = false;
    
	for (int i=0;i<tmpList.size(); i++)
    {
		QStringList itemList;
		
		itemList = tmpList.at(i).split("=");
        
		if (itemList.size()==2) // only parse those lines that only have one "=", ignore blank lines
		{
            if ((itemList.at(0).trimmed().toUpper() == "CELL_NAME_FILE_PREFIX")&& (itemList.at(1).trimmed().toUpper().isEmpty()==false))
            {
                cfginfo.cellNameFilePrefix = itemList.at(1).trimmed();
             }
            else if ((itemList.at(0).trimmed().toUpper() == "CELL_NAME_FILE_SUFFIX")&& (itemList.at(1).trimmed().toUpper().isEmpty()==false))
            {
                cfginfo.cellNameFileSurfix = itemList.at(1).trimmed();
            }
//            else if ((itemList.at(0).trimmed().toUpper() == "REF_MARKER_CONTROL_POINTS_CELL_NAME_FILE")&& 
//                     (itemList.at(1).trimmed().toUpper().isEmpty()==false))
//            {
//                cfginfo.refMarkerCptCellNameFile = itemList.at(1).trimmed();
//            }
            else if ((itemList.at(0).trimmed().toUpper() == "CELL_TYPE_FILE")&& 
                     (itemList.at(1).trimmed().toUpper().isEmpty()==false))
            {
                cfginfo.cellTypeFileName.append(itemList.at(1).trimmed());
            }            
//            else if ((itemList.at(0).trimmed().toUpper() == "SAVE_REGISTRATION_DATA")&& 
//                     (itemList.at(1).trimmed().toUpper()== "TRUE"))
//            {
//                cfginfo.saveRegDataTag = true ;
//            }            
            else if ((itemList.at(0).trimmed().toUpper() == "CELL_STATISTICS_SUFFIX")&& 
                     (itemList.at(1).trimmed().toUpper().isEmpty()==false))
            {
                cfginfo.cellStatFileSurfix = itemList.at(1).trimmed();
            }
//            else if ((itemList.at(0).trimmed().toUpper() == "COEXPRESSION_FILE_SURFIX")&& 
//                     (itemList.at(1).trimmed().toUpper().isEmpty()==false))
//            {
//                cfginfo.coexpressFileSurfix = itemList.at(1).trimmed();
//            }
            else if ((itemList.at(0).trimmed().toUpper() == "MARKER_MAP_NAME")&& 
                     (itemList.at(1).trimmed().toUpper().isEmpty()==false))
            {
                cfginfo.markerMapFileName = itemList.at(1).trimmed();
            }
            else if ((itemList.at(0).trimmed().toUpper() == "INPUT_FOLDER")&& 
                     (itemList.at(1).trimmed().toUpper().isEmpty()==false))
            {
                cfginfo.inputFolder = itemList.at(1).trimmed();
            }
            else if ((itemList.at(0).trimmed().toUpper() == "OUTPUT_FOLDER")&& 
                     (itemList.at(1).trimmed().toUpper().isEmpty()==false))
            {
                cfginfo.outputFolder = itemList.at(1).trimmed();
            }
            
               
        }
        
    }
    
   
	return true;
}

bool savePointCloudAtlasConfigFile(const char *filename, const atlasConfig & cfginfo)
{
	FILE *fp = fopen(filename, "wt");
	if (!fp)
	{
		printf("Unable to open the specified file [%s] to write. Do nothing.", filename);
		return false;
	}
	
	int i,j;
    
	fprintf(fp, "INPUT_FOLDER=%s\n", qPrintable(cfginfo.inputFolder));	
	fprintf(fp, "OUTPUT_FOLDER=%s\n\n", qPrintable(cfginfo.outputFolder));
	fprintf(fp, "CELL_NAME_FILE_PREFIX=%s\n\n", qPrintable(cfginfo.cellNameFilePrefix)); 
	fprintf(fp, "CELL_NAME_FILE_SUFFIX=%s\n\n", qPrintable(cfginfo.cellNameFileSurfix)); 
//	fprintf(fp, "REF_MARKER_CONTROL_POINTS_CELL_NAME_FILE=%s\n\n", qPrintable(cfginfo.refMarkerCptCellNameFile)); 
    
    for (i=0; i<cfginfo.cellTypeFileName.size(); i++)
    {
        fprintf(fp, "CELL_TYPE_FILE=%s\n\n", qPrintable(cfginfo.cellTypeFileName.at(i))); 
    }
        
//	fprintf(fp, "SAVE_REGISTRATION_DATA=%s\n\n", cfginfo.saveRegDataTag?"true":"false"); 
    
	fprintf(fp, "CELL_STATISTICS_SUFFIX=%s\n\n", qPrintable(cfginfo.cellStatFileSurfix)); 
//	fprintf(fp, "COEXPRESSION_FILE_SURFIX=%s\n\n", qPrintable(cfginfo.coexpressFileSurfix)); 
	fprintf(fp, "MARKER_MAP_NAME=%s\n\n", qPrintable(cfginfo.markerMapFileName)); 
	
    fclose(fp);
    
	return true;
}


void printApo(apoAtlasLinkerInfoAll & apoinfo)
{
	printf("referenceMarkerName = %s\n", qPrintable(apoinfo.referenceMarkerName));
//	printf("regTargetFileName = %s\n", qPrintable(apoinfo.regTargetFileName));
	
	for (int i=0; i<apoinfo.items.size(); i++)
	{
		printf("classname = %s\n", qPrintable(apoinfo.items[i].className));
		printf("registered file = %s\n", qPrintable(apoinfo.items[i].registeredFile));
		
//		printf("cutoff ratio = %s\n", qPrintable(apoinfo.items[i].threratio)); //FL, 20100909

		printf("sigFolder = %s\n", qPrintable(apoinfo.items[i].sigFolder));		
		for (int j=0; j<apoinfo.items[i].sigFolderFileList.size(); j++)
			printf("sigFolderFileList =%s\n", qPrintable(apoinfo.items[i].sigFolderFileList[j]));

		printf("sigFolder = %s\n", qPrintable(apoinfo.items[i].refFolder));
		for (int j=0; j<apoinfo.items[i].refFolderFileList.size(); j++)
			printf("refFolderFileList =%s\n", qPrintable(apoinfo.items[i].refFolderFileList[j]));
		
		printf("\n");
	}
	 
}

////not called by atlasbuilder, FL 20091030 comment
//bool alignPointCloudAtlas(const apoAtlasLinkerInfoAll & apo_atlas_info) //this function is used to scan and align apo files for atlas building
//{
//	QStringList cellNameListAll;
//	int i;
//
//	QList<CellAPO> ref_common = readAPO_file(apo_atlas_info.regTargetFileName);
//	if (ref_common.size()<=0) 
//	{
//		v3d_msg("Unable to read valid info from the common reference mark point cloud file. Check your data");
//		return false;
//	}
//	
//	for (i=0;i<ref_common.size();i++)
//		if (isValidCellName(ref_common[i].name))
//			cellNameListAll << ref_common[i].name.trimmed();
//	QStringList cellNamesRef = cellNameListAll; //make a copy for the target ref cell name, and merge it with other ref later
//	
//	if (apo_atlas_info.items.size()<=0)
//	{
//		v3d_msg("There is 0 class to be aligned in point cloud atlas building. Do nothing.");
//		return false;
//	}	
//	
//	for (int n=0;n<apo_atlas_info.items.size();n++) //for n classes
//	{
//		const apoAtlasLinkerInfo & cur_class = apo_atlas_info.items[n];
//		if (cur_class.sigFolderFileList.size()<=0 || cur_class.refFolderFileList.size()<=0 || cur_class.sigFolderFileList.size()!=cur_class.refFolderFileList.size())
//		{
//			v3d_msg("The current class's signal and reference file lists have unmatched file numbers. Skip this class.");
//			continue;
//		}
//		
//		//read each pair of the signal and reference data, and align to the common reference 
//		
//		QHash<QString, int> hash_ref_cur, hash_sig_cur;
//		QList < QList<CellAPO> > cur_sig_celllist, cur_ref_celllist; //store the aligned cell point cloud information
//		QStringList cellNameListCurClass_sig, cellNameListCurClass_ref;
//		for (int k=0;k<cur_class.sigFolderFileList.size();k++)
//		{
//			hash_ref_cur.clear(); hash_sig_cur.clear();
//			
//			//first read files
//			QString my_sig_file = cur_class.sigFolderFileList[k].trimmed(); if (!cur_class.sigFolder.trimmed().isEmpty()) my_sig_file.prepend( cur_class.sigFolder.trimmed()+"/");
//			QString my_ref_file = cur_class.refFolderFileList[k].trimmed(); if (!cur_class.refFolder.trimmed().isEmpty()) my_ref_file.prepend( cur_class.refFolder.trimmed()+"/");
//			QList<CellAPO> my_sig = readAPO_file(my_sig_file);
//			QList<CellAPO> my_ref = readAPO_file(my_ref_file);
//			if (my_sig.size()<=0 || my_ref.size()<=0) 
//			{
//				v3d_msg("Unable to read valid info from the current class reference or signal point cloud file. Skip this file.");
//				continue;
//			}
//
//			//push to cell name list
//			for (i=0;i<my_ref.size();i++)
//				if (isValidCellName(my_ref[i].name))
//				{
//					QString t = my_ref[i].name.trimmed();
//					cellNameListAll << t;
//					cellNameListCurClass_ref << t;
//					hash_ref_cur[t] = i;
//				}	
//			for (i=0;i<my_sig.size();i++)
//				if (isValidCellName(my_sig[i].name))
//				{	
//					QString t = my_sig[i].name.trimmed();
//					cellNameListAll << t;
//					cellNameListCurClass_sig << t;
//					hash_sig_cur[t] = i;
//				}	
//		
//			//now do alignment
//			
//			
//			//if success, then add to the aligned PC list
//		}
//
//		if(!writeCellNameList2File(cur_class.registeredFile+"_cellname.txt", uniqueCellNameList(cellNameListCurClass_sig)))
//		{
//			continue;
//		}
//		
//		cellNamesRef << cellNameListCurClass_ref; //append the ref cell names
//		
//		//generate the average aligned signal and reference point cloud info for this class
//		
//		//save to the output file
//	}
//	
//	//write the overall cell name list
//	writeCellNameList2File(apo_atlas_info.regTargetFileName+"_refcellname.txt", uniqueCellNameList(cellNamesRef));
//	writeCellNameList2File(apo_atlas_info.regTargetFileName+"_allcellname.txt", uniqueCellNameList(cellNameListAll));
//
//	return true;
//}

bool isValidCellName(const QString & s) //remove all empty and VANO defined keywords
{
	QString t = s.trimmed();
	return (t.isEmpty() || 
			t.contains("NOUSE", Qt::CaseInsensitive) || 
			t.contains("Split Cell", Qt::CaseInsensitive) || 
			t.contains("New Cell", Qt::CaseInsensitive) ||
			t.contains("Merge", Qt::CaseInsensitive)
	) ? false : true;
}

QStringList uniqueCellNameList(const QStringList & longlist)
{
	QHash <QString, long> hash;
	for (long i=0;i<longlist.size();i++)
		hash[longlist[i].trimmed()] = i;
	return hash.uniqueKeys();
}

bool writeCellNameList2File(const QString & outfile, const QStringList & cellnamelist)
{
	QStringList processedlist = cellnamelist; processedlist.sort();
	FILE * fp=fopen((char *)qPrintable(outfile), "wt");
	if (!fp)
	{
		v3d_msg("Fail to open file to write cell name information. skip this class.");
		return false;
	}
	for (int i=0;i<processedlist.size();i++)
		fprintf(fp, "%s\n", qPrintable(processedlist[i]));
	fclose(fp);
	
	return true;
}
