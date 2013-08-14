// by Hanchuan Peng
//090518
// revised by Fuhui Long, 20090520

#include "converter_pcatlas_data.h"

void convert_FLAtlasBuilderInfo_to_apoAtlasLinkerInfo(const MarkerGeneInfo & m, apoAtlasLinkerInfoAll & apoinfo)
{
	int i,j;
	MarkerStacks markerstacks;
	
//	apoinfo.regTargetFileName = m.reference_stack_filename.c_str();
	apoinfo.referenceMarkerName = m.reference_markername.c_str(); //FL 20091028
	
	for (j=0; j<m.marker_stacks.size(); j++)
	{
		apoAtlasLinkerInfo a;
		markerstacks = m.marker_stacks.at(j);
	
		a.className = markerstacks.markername.c_str();
		a.registeredFile = markerstacks.registered_file.c_str();
		
		a.sigFolderFileList.clear();
		for (i=0;i<markerstacks.signal_ch_filename.size();i++)
			a.sigFolderFileList << markerstacks.signal_ch_filename.at(i).c_str();

		a.refFolderFileList.clear();
		for (i=0;i<markerstacks.reference_ch_filename.size();i++)
			a.sigFolderFileList << markerstacks.reference_ch_filename.at(i).c_str();
		
		a.sigFolder = "";
		a.refFolder = "";
		
		apoinfo.items.append(a);
	}
}

void convert_FLAtlasBuilderInfo_from_apoAtlasLinkerInfo(MarkerGeneInfo & m, const apoAtlasLinkerInfoAll & a)
{
	int i,j;
	
//	m.reference_stack_filename = (char *)qPrintable(a.regTargetFileName);
	m.reference_markername = (char *)qPrintable(a.referenceMarkerName);
	
	for (j=0; j<a.items.size(); j++)
	{
		MarkerStacks newitem;
		
		newitem.markername = (char *)qPrintable(a.items[j].className);
		newitem.registered_file = (char *)qPrintable(a.items[j].registeredFile);
		
		newitem.signal_ch_filename.clear(); 
		for (i=0;i<a.items[j].sigFolderFileList.size();i++)
			newitem.signal_ch_filename.push_back(qPrintable(a.items[j].sigFolder + '/' + a.items[j].sigFolderFileList.at(i)));
		
		newitem.reference_ch_filename.clear(); 
		for (i=0;i<a.items[j].refFolderFileList.size();i++)
			newitem.reference_ch_filename.push_back(qPrintable(a.items[j].refFolder + '/' + a.items[j].refFolderFileList.at(i)));
			
		m.marker_stacks.push_back(newitem);	
	}
}




