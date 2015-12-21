//fileio for the LSM configuration parameters
// by Hanchuan Peng
// 20110516


#include "configfileio_lsm.h"
#include "smartscope_util.h"

const QString ssConfigFolder = "C:\\smartscope_conf\\";


bool Parameters_LSM::importKeywordString2ParameterValues(QString ss, QString vv)
{
	QString tss = ss.trimmed().toUpper();
	QString tvv = vv.trimmed();

	if (tss.isEmpty() || tvv.isEmpty())
	{
		v3d_msg("Either the keyword string or the value string is empty. Do nothing.\n");
		return false;
	}

	if (tss=="DEVICE_X"){this->DEVICE_X = tvv.toInt();}
	else if (tss=="DEVICE_Y"){this->DEVICE_Y = tvv.toInt(); }
	else if (tss=="DEVICE_Z"){this->DEVICE_Z = tvv.toInt(); }
	else if (tss=="AO_X"){this->AO_X = tvv.toInt(); }
	else if (tss=="AO_Y"){this->AO_Y = tvv.toInt(); }
	else if (tss=="AO_Z"){this->AO_Z = tvv.toInt(); }
	else if (tss=="AI_COUNTER_RED"){this->AI_COUNTER_RED = tvv.toInt(); }
	else if (tss=="AI_COUNTER_GREEN"){this->AI_COUNTER_GREEN = tvv.toInt(); }
	else if (tss=="DEVICE_RED_AI"){this->DEVICE_RED_AI = tvv.toInt(); }
	else if (tss=="DEVICE_RED_CTR"){this->DEVICE_RED_CTR = tvv.toInt(); }
	else if (tss=="PORT_RED_AI"){this->PORT_RED_AI = tvv.toInt(); }
	else if (tss=="PORT_RED_CTR"){this->PORT_RED_CTR = tvv.toInt(); }
	else if (tss=="DEVICE_GREEN_AI"){this->DEVICE_GREEN_AI = tvv.toInt(); }
	else if (tss=="DEVICE_GREEN_CTR"){this->DEVICE_GREEN_CTR = tvv.toInt(); }
	else if (tss=="PORT_GREEN_AI"){this->PORT_GREEN_AI = tvv.toInt(); }
	else if (tss=="PORT_GREEN_CTR"){this->PORT_GREEN_CTR = tvv.toInt(); }
	else if (tss=="DEVICE_TIMING"){this->DEVICE_TIMING = tvv.toInt(); }
	else if (tss=="PORT_TIMING"){this->PORT_TIMING = tvv.toInt(); }
	else if (tss=="DEVICE_CLOCK"){this->DEVICE_CLOCK = tvv.toInt(); }
	else if (tss=="PORT_CLOCK"){this->PORT_CLOCK = tvv.toInt(); }
	else if (tss=="FOVX_MIN"){this->FOVX_MIN = tvv.toDouble(); }
	else if (tss=="FOVX_MAX"){this->FOVX_MAX = tvv.toDouble(); }
	else if (tss=="FOVX_STEP"){this->FOVX_STEP = tvv.toDouble(); }
	else if (tss=="FOVY_MIN"){this->FOVY_MIN = tvv.toDouble(); }
	else if (tss=="FOVY_MAX"){this->FOVY_MAX = tvv.toDouble(); }
	else if (tss=="FOVY_STEP"){this->FOVY_STEP = tvv.toDouble(); }
	else if (tss=="FOVZ_MIN"){this->FOVZ_MIN = tvv.toDouble(); }
	else if (tss=="FOVZ_MAX"){this->FOVZ_MAX = tvv.toDouble(); }
	else if (tss=="FOVZ_STEP"){this->FOVZ_STEP = tvv.toDouble(); }
	else if (tss=="RATIO_X"){this->RATIO_X = tvv.toDouble(); }
	else if (tss=="RATIO_Y"){this->RATIO_Y = tvv.toDouble(); }
	else if (tss=="RATIO_Z"){this->RATIO_Z = tvv.toDouble(); }
	else if (tss=="TIMEOUT"){this->TIMEOUT = tvv.toDouble(); }
	else if (tss=="SCANNING_RATE"){this->SCANNING_RATE = tvv.toDouble(); }
	else if (tss=="SX"){this->SX = tvv.toLong(); }
	else if (tss=="SY"){this->SY = tvv.toLong(); }
	else if (tss=="SZ"){this->SZ = tvv.toLong(); }
	//else if (tss=="CHANNEL"){this->CHANNEL = tvv.toInt(); }
	else if (tss=="CHANNEL")
	{
		if(tvv.compare("g488")==0) // green
			this->CHANNEL = 0;
		else if(tvv.compare("r561")==0)// red
			this->CHANNEL = 1;
		else if(tvv.compare("g488,r561")==0)// both
			this->CHANNEL = 2;
	}
	else if (tss=="MARKER_SORTED"){this->MARKER_SORTED = tvv.toInt(); }
	else if (tss=="STARTX"){this->STARTX = tvv.toLong(); }
	else if (tss=="STARTY"){this->STARTY = tvv.toLong(); }
	else if (tss=="ENDX"){this->ENDX = tvv.toLong(); }
	else if (tss=="ENDY"){this->ENDY = tvv.toLong(); }

	else if (tss=="BF_TO_LSM_A"){this->BF_TO_LSM_A = tvv.toDouble(); }
	else if (tss=="BF_TO_LSM_D"){this->BF_TO_LSM_D = tvv.toDouble(); }
	else if (tss=="BF_TO_LSM_E"){this->BF_TO_LSM_E = tvv.toDouble(); }
	else if (tss=="BF_TO_LSM_F"){this->BF_TO_LSM_F = tvv.toDouble(); }

	else if (tss=="LSM_TO_BF_A"){this->LSM_TO_BF_A = tvv.toDouble(); }
	else if (tss=="LSM_TO_BF_D"){this->LSM_TO_BF_D = tvv.toDouble(); }
	else if (tss=="LSM_TO_BF_E"){this->LSM_TO_BF_E = tvv.toDouble(); }
	else if (tss=="LSM_TO_BF_F"){this->LSM_TO_BF_F = tvv.toDouble(); }

	else if (tss=="SCAN_METHOD"){this->SCAN_METHOD = tvv.toInt(); }
	else if (tss=="USE_SIMIMG"){this->USE_SIMIMG = tvv.toInt(); }
	else if (tss=="SCAN_Z_REVERSE"){this->SCAN_Z_REVERSE = tvv.toInt(); }

	return true;
}

bool Parameters_LSM::load_ParameterFile(QString openFileNameLabel)
{
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
		return false; //must not be a valid file

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
			if (importKeywordString2ParameterValues(itemList.at(0).trimmed().toUpper(), itemList.at(1).trimmed())==true)
			{
				cnt++;
				continue;
			}
		}
	}

	return isValid(); 
}	

bool Parameters_LSM::save_ParameterFile(QString openFileNameLabel)
{
	FILE * fp=0;
	fp = fopen((char *)qPrintable(openFileNameLabel), "wt");
	if (!fp)
	{
		v3d_msg(QString("Fail to open file %1 to write.").arg(openFileNameLabel));
		return false;
	}

	//now save
	fprintf(fp, "DEVICE_X=%d\n", this->DEVICE_X);
	fprintf(fp, "DEVICE_Y=%d\n", this->DEVICE_Y);
	fprintf(fp, "DEVICE_Z=%d\n", this->DEVICE_Z);
	fprintf(fp, "AO_X=%d\n", this->AO_X);
	fprintf(fp, "AO_Y=%d\n", this->AO_Y);
	fprintf(fp, "AO_Z=%d\n", this->AO_Z);
	fprintf(fp, "AI_COUNTER_RED=%d\n", this->AI_COUNTER_RED);
	fprintf(fp, "AI_COUNTER_GREEN=%d\n", this->AI_COUNTER_GREEN);
	fprintf(fp, "DEVICE_RED_AI=%d\n", this->DEVICE_RED_AI);
	fprintf(fp, "DEVICE_RED_CTR=%d\n", this->DEVICE_RED_CTR);
	fprintf(fp, "PORT_RED_AI=%d\n", this->PORT_RED_AI);
	fprintf(fp, "PORT_RED_CTR=%d\n", this->PORT_RED_CTR);
	fprintf(fp, "DEVICE_GREEN_AI=%d\n", this->DEVICE_GREEN_AI);
	fprintf(fp, "DEVICE_GREEN_CTR=%d\n", this->DEVICE_GREEN_CTR);
	fprintf(fp, "PORT_GREEN_AI=%d\n", this->PORT_GREEN_AI);
	fprintf(fp, "PORT_GREEN_CTR=%d\n", this->PORT_GREEN_CTR);
	fprintf(fp, "DEVICE_TIMING=%d\n", this->DEVICE_TIMING);
	fprintf(fp, "PORT_TIMING=%d\n", this->PORT_TIMING);
	fprintf(fp, "DEVICE_CLOCK=%d\n", this->DEVICE_CLOCK);
	fprintf(fp, "PORT_CLOCK=%d\n", this->PORT_CLOCK);
	fprintf(fp, "FOVX_MIN=%10.2f\n", this->FOVX_MIN);
	fprintf(fp, "FOVX_MAX=%10.2f\n", this->FOVX_MAX);
	fprintf(fp, "FOVX_STEP=%10.2f\n", this->FOVX_STEP);
	fprintf(fp, "FOVY_MIN=%10.2f\n", this->FOVY_MIN);
	fprintf(fp, "FOVY_MAX=%10.2f\n", this->FOVY_MAX);
	fprintf(fp, "FOVY_STEP=%10.2f\n", this->FOVY_STEP);
	fprintf(fp, "FOVZ_MIN=%10.2f\n", this->FOVZ_MIN);
	fprintf(fp, "FOVZ_MAX=%10.2f\n", this->FOVZ_MAX);
	fprintf(fp, "FOVZ_STEP=%10.2f\n", this->FOVZ_STEP);
	fprintf(fp, "RATIO_X=%10.2f\n", this->RATIO_X);
	fprintf(fp, "RATIO_Y=%10.2f\n", this->RATIO_Y);
	fprintf(fp, "RATIO_Z=%10.2f\n", this->RATIO_Z);
	fprintf(fp, "TIMEOUT=%10.2f\n", this->TIMEOUT);
	fprintf(fp, "SCANNING_RATE=%10.2f\n", this->SCANNING_RATE);	
	fprintf(fp, "SX=%ld\n", this->SX);	
	fprintf(fp, "SY=%ld\n", this->SY);	
	fprintf(fp, "SZ=%ld\n", this->SZ);	
	//fprintf(fp, "CHANNEL=%d\n", this->CHANNEL);
	
	if(this->CHANNEL == 0) // green
		fprintf(fp, "CHANNEL=g488\n");
	else if(this->CHANNEL == 1) // red
		fprintf(fp, "CHANNEL=r561\n");
	else if(this->CHANNEL == 2)// both
		fprintf(fp, "CHANNEL=g488,r561\n");

	fprintf(fp, "MARKER_SORTED=%d\n", this->MARKER_SORTED);	
	fprintf(fp, "STARTX=%ld\n", this->STARTX);	
	fprintf(fp, "STARTY=%ld\n", this->STARTY);	
	fprintf(fp, "ENDX=%ld\n", this->ENDX);	
	fprintf(fp, "ENDY=%ld\n", this->ENDY);	

	fprintf(fp, "BF_TO_LSM_A=%.6f\n", this->BF_TO_LSM_A);	
	fprintf(fp, "BF_TO_LSM_D=%.6f\n", this->BF_TO_LSM_D);	
	fprintf(fp, "BF_TO_LSM_E=%.6f\n", this->BF_TO_LSM_E);	
	fprintf(fp, "BF_TO_LSM_F=%.6f\n", this->BF_TO_LSM_F);	

	fprintf(fp, "LSM_TO_BF_A=%.6f\n", this->LSM_TO_BF_A);	
	fprintf(fp, "LSM_TO_BF_D=%.6f\n", this->LSM_TO_BF_D);	
	fprintf(fp, "LSM_TO_BF_E=%.6f\n", this->LSM_TO_BF_E);	
	fprintf(fp, "LSM_TO_BF_F=%.6f\n", this->LSM_TO_BF_F);	

	fprintf(fp, "SCAN_METHOD=%d\n", this->SCAN_METHOD);
	fprintf(fp, "USE_SIMIMG=%d\n", this->USE_SIMIMG);

	fprintf(fp, "SCAN_Z_REVERSE=%d\n", this->SCAN_Z_REVERSE);

	//finish up
	if (fp) fclose(fp);
	return true;
}	


//==================================================================================
// for Parameters_ROI_LSM
//==================================================================================
bool Parameters_ROI_LSM::importKeywordString2ParameterValues(QString ss, QString vv)
{
	QString tss = ss.trimmed().toUpper();
	QString tvv = vv.trimmed();

	if (tss.isEmpty() || tvv.isEmpty())
	{
		v3d_msg("Either the keyword string or the value string is empty. Do nothing.\n");
		return false;
	}

	if (tss=="DEVICE_X"){this->DEVICE_X = tvv.toInt();}
	else if (tss=="DEVICE_Y"){this->DEVICE_Y = tvv.toInt(); }
	else if (tss=="DEVICE_Z"){this->DEVICE_Z = tvv.toInt(); }
	else if (tss=="AO_X"){this->AO_X = tvv.toInt(); }
	else if (tss=="AO_Y"){this->AO_Y = tvv.toInt(); }
	else if (tss=="AO_Z"){this->AO_Z = tvv.toInt(); }
	else if (tss=="AI_COUNTER_RED"){this->AI_COUNTER_RED = tvv.toInt(); }
	else if (tss=="AI_COUNTER_GREEN"){this->AI_COUNTER_GREEN = tvv.toInt(); }
	else if (tss=="DEVICE_RED_AI"){this->DEVICE_RED_AI = tvv.toInt(); }
	else if (tss=="DEVICE_RED_CTR"){this->DEVICE_RED_CTR = tvv.toInt(); }
	else if (tss=="PORT_RED_AI"){this->PORT_RED_AI = tvv.toInt(); }
	else if (tss=="PORT_RED_CTR"){this->PORT_RED_CTR = tvv.toInt(); }
	else if (tss=="DEVICE_GREEN_AI"){this->DEVICE_GREEN_AI = tvv.toInt(); }
	else if (tss=="DEVICE_GREEN_CTR"){this->DEVICE_GREEN_CTR = tvv.toInt(); }
	else if (tss=="PORT_GREEN_AI"){this->PORT_GREEN_AI = tvv.toInt(); }
	else if (tss=="PORT_GREEN_CTR"){this->PORT_GREEN_CTR = tvv.toInt(); }
	else if (tss=="DEVICE_TIMING"){this->DEVICE_TIMING = tvv.toInt(); }
	else if (tss=="PORT_TIMING"){this->PORT_TIMING = tvv.toInt(); }
	else if (tss=="DEVICE_CLOCK"){this->DEVICE_CLOCK = tvv.toInt(); }
	else if (tss=="PORT_CLOCK"){this->PORT_CLOCK = tvv.toInt(); }
	else if (tss=="FOVX_MIN"){this->FOVX_MIN = tvv.toDouble(); }
	else if (tss=="FOVX_MAX"){this->FOVX_MAX = tvv.toDouble(); }
	else if (tss=="FOVX_STEP"){this->FOVX_STEP = tvv.toDouble(); }
	else if (tss=="FOVY_MIN"){this->FOVY_MIN = tvv.toDouble(); }
	else if (tss=="FOVY_MAX"){this->FOVY_MAX = tvv.toDouble(); }
	else if (tss=="FOVY_STEP"){this->FOVY_STEP = tvv.toDouble(); }
	else if (tss=="FOVZ_MIN"){this->FOVZ_MIN = tvv.toDouble(); }
	else if (tss=="FOVZ_MAX"){this->FOVZ_MAX = tvv.toDouble(); }
	else if (tss=="FOVZ_STEP"){this->FOVZ_STEP = tvv.toDouble(); }
	else if (tss=="RATIO_X"){this->RATIO_X = tvv.toDouble(); }
	else if (tss=="RATIO_Y"){this->RATIO_Y = tvv.toDouble(); }
	else if (tss=="RATIO_Z"){this->RATIO_Z = tvv.toDouble(); }
	else if (tss=="TIMEOUT"){this->TIMEOUT = tvv.toDouble(); }
	else if (tss=="SCANNING_RATE"){this->SCANNING_RATE = tvv.toDouble(); }
	else if (tss=="REZ_X"){this->REZ_X = tvv.toDouble(); } // Resolution X
	else if (tss=="REZ_Y"){this->REZ_Y = tvv.toDouble(); }
	else if (tss=="REZ_Z"){this->REZ_Z = tvv.toDouble(); }
	else if (tss=="ORIGIN_X"){this->ORIGIN_X = tvv.toDouble(); } // Origin x
	else if (tss=="ORIGIN_Y"){this->ORIGIN_Y = tvv.toDouble(); }
	else if (tss=="ORIGIN_Z"){this->ORIGIN_Z = tvv.toDouble(); }
	else if (tss=="SX"){this->SX = tvv.toLong(); }
	else if (tss=="SY"){this->SY = tvv.toLong(); }
	else if (tss=="SZ"){this->SZ = tvv.toLong(); }
	//else if (tss=="CHANNEL"){this->CHANNEL = tvv.toInt(); }
	else if (tss=="CHANNEL")
	{
		if(tvv.compare("g488")==0) // green
			this->CHANNEL = 0;
		else if(tvv.compare("r561")==0)// red
			this->CHANNEL = 1;
		else if(tvv.compare("g488,r561")==0)// both
			this->CHANNEL = 2;
	}
	else if (tss=="MARKER_SORTED"){this->MARKER_SORTED = tvv.toInt(); }
	else if (tss=="STARTX"){this->STARTX = tvv.toLong(); }
	else if (tss=="STARTY"){this->STARTY = tvv.toLong(); }
	else if (tss=="ENDX"){this->ENDX = tvv.toLong(); }
	else if (tss=="ENDY"){this->ENDY = tvv.toLong(); }

	else if (tss=="BF_TO_LSM_A"){this->BF_TO_LSM_A = tvv.toDouble(); }
	else if (tss=="BF_TO_LSM_D"){this->BF_TO_LSM_D = tvv.toDouble(); }
	else if (tss=="BF_TO_LSM_E"){this->BF_TO_LSM_E = tvv.toDouble(); }
	else if (tss=="BF_TO_LSM_F"){this->BF_TO_LSM_F = tvv.toDouble(); }

	else if (tss=="LSM_TO_BF_A"){this->LSM_TO_BF_A = tvv.toDouble(); }
	else if (tss=="LSM_TO_BF_D"){this->LSM_TO_BF_D = tvv.toDouble(); }
	else if (tss=="LSM_TO_BF_E"){this->LSM_TO_BF_E = tvv.toDouble(); }
	else if (tss=="LSM_TO_BF_F"){this->LSM_TO_BF_F = tvv.toDouble(); }

	else if (tss=="SCAN_METHOD"){this->SCAN_METHOD = tvv.toInt(); }
	else if (tss=="USE_SIMIMG"){this->USE_SIMIMG = tvv.toInt(); }

	else if (tss=="SCAN_Z_REVERSE"){this->SCAN_Z_REVERSE = tvv.toInt(); }

	return true;
}

bool Parameters_ROI_LSM::load_ParameterFile(QString openFileNameLabel)
{
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
		return false; //must not be a valid file

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
			if (importKeywordString2ParameterValues(itemList.at(0).trimmed().toUpper(), itemList.at(1).trimmed())==true)
			{
				cnt++;
				continue;
			}
		}
	}

	//return isValid(); //resolution and origin may not have value at the moment
	return true;
}	

bool Parameters_ROI_LSM::save_ParameterFile(QString openFileNameLabel)
{
	FILE * fp=0;
	fp = fopen((char *)qPrintable(openFileNameLabel), "wt");
	if (!fp)
	{
		v3d_msg(QString("Fail to open file %1 to write.").arg(openFileNameLabel));
		return false;
	}

	//now save
	fprintf(fp, "DEVICE_X=%d\n", this->DEVICE_X);
	fprintf(fp, "DEVICE_Y=%d\n", this->DEVICE_Y);
	fprintf(fp, "DEVICE_Z=%d\n", this->DEVICE_Z);
	fprintf(fp, "AO_X=%d\n", this->AO_X);
	fprintf(fp, "AO_Y=%d\n", this->AO_Y);
	fprintf(fp, "AO_Z=%d\n", this->AO_Z);
	fprintf(fp, "AI_COUNTER_RED=%d\n", this->AI_COUNTER_RED);
	fprintf(fp, "AI_COUNTER_GREEN=%d\n", this->AI_COUNTER_GREEN);
	fprintf(fp, "DEVICE_RED_AI=%d\n", this->DEVICE_RED_AI);
	fprintf(fp, "DEVICE_RED_CTR=%d\n", this->DEVICE_RED_CTR);
	fprintf(fp, "PORT_RED_AI=%d\n", this->PORT_RED_AI);
	fprintf(fp, "PORT_RED_CTR=%d\n", this->PORT_RED_CTR);
	fprintf(fp, "DEVICE_GREEN_AI=%d\n", this->DEVICE_GREEN_AI);
	fprintf(fp, "DEVICE_GREEN_CTR=%d\n", this->DEVICE_GREEN_CTR);
	fprintf(fp, "PORT_GREEN_AI=%d\n", this->PORT_GREEN_AI);
	fprintf(fp, "PORT_GREEN_CTR=%d\n", this->PORT_GREEN_CTR);
	fprintf(fp, "DEVICE_TIMING=%d\n", this->DEVICE_TIMING);
	fprintf(fp, "PORT_TIMING=%d\n", this->PORT_TIMING);
	fprintf(fp, "DEVICE_CLOCK=%d\n", this->DEVICE_CLOCK);
	fprintf(fp, "PORT_CLOCK=%d\n", this->PORT_CLOCK);
	fprintf(fp, "FOVX_MIN=%10.2f\n", this->FOVX_MIN);
	fprintf(fp, "FOVX_MAX=%10.2f\n", this->FOVX_MAX);
	fprintf(fp, "FOVX_STEP=%10.2f\n", this->FOVX_STEP);
	fprintf(fp, "FOVY_MIN=%10.2f\n", this->FOVY_MIN);
	fprintf(fp, "FOVY_MAX=%10.2f\n", this->FOVY_MAX);
	fprintf(fp, "FOVY_STEP=%10.2f\n", this->FOVY_STEP);
	fprintf(fp, "FOVZ_MIN=%10.2f\n", this->FOVZ_MIN);
	fprintf(fp, "FOVZ_MAX=%10.2f\n", this->FOVZ_MAX);
	fprintf(fp, "FOVZ_STEP=%10.2f\n", this->FOVZ_STEP);
	fprintf(fp, "RATIO_X=%10.2f\n", this->RATIO_X);
	fprintf(fp, "RATIO_Y=%10.2f\n", this->RATIO_Y);
	fprintf(fp, "RATIO_Z=%10.2f\n", this->RATIO_Z);
	fprintf(fp, "TIMEOUT=%10.2f\n", this->TIMEOUT);
	fprintf(fp, "SCANNING_RATE=%10.2f\n", this->SCANNING_RATE);	
	fprintf(fp, "REZ_X=%10.6f\n", this->REZ_X); //Resolution x
	fprintf(fp, "REZ_Y=%10.6f\n", this->REZ_Y);
	fprintf(fp, "REZ_Z=%10.6f\n", this->REZ_Z);
	fprintf(fp, "ORIGIN_X=%10.6f\n", this->ORIGIN_X); //Origin x
	fprintf(fp, "ORIGIN_Y=%10.6f\n", this->ORIGIN_Y);
	fprintf(fp, "ORIGIN_Z=%10.6f\n", this->ORIGIN_Z);
	fprintf(fp, "SX=%ld\n", this->SX);	
	fprintf(fp, "SY=%ld\n", this->SY);	
	fprintf(fp, "SZ=%ld\n", this->SZ);	
	//fprintf(fp, "CHANNEL=%d\n", this->CHANNEL);	

	if(this->CHANNEL == 0) // green
		fprintf(fp, "CHANNEL=g488\n");//fprintf(fp, "CHANNEL=%s\n", "g488");
	else if(this->CHANNEL == 1) // red
		fprintf(fp, "CHANNEL=r561\n"); // fprintf(fp, "CHANNEL=%s\n", "r561");
	else if(this->CHANNEL == 2)// both
		fprintf(fp, "CHANNEL=g488,r561\n"); //fprintf(fp, "CHANNEL=%s\n", "g488,r561");

	fprintf(fp, "MARKER_SORTED=%d\n", this->MARKER_SORTED);	
	fprintf(fp, "STARTX=%ld\n", this->STARTX);	
	fprintf(fp, "STARTY=%ld\n", this->STARTY);	
	fprintf(fp, "ENDX=%ld\n", this->ENDX);	
	fprintf(fp, "ENDY=%ld\n", this->ENDY);	

	fprintf(fp, "BF_TO_LSM_A=%.6f\n", this->BF_TO_LSM_A);	
	fprintf(fp, "BF_TO_LSM_D=%.6f\n", this->BF_TO_LSM_D);	
	fprintf(fp, "BF_TO_LSM_E=%.6f\n", this->BF_TO_LSM_E);	
	fprintf(fp, "BF_TO_LSM_F=%.6f\n", this->BF_TO_LSM_F);	

	fprintf(fp, "LSM_TO_BF_A=%.6f\n", this->LSM_TO_BF_A);	
	fprintf(fp, "LSM_TO_BF_D=%.6f\n", this->LSM_TO_BF_D);	
	fprintf(fp, "LSM_TO_BF_E=%.6f\n", this->LSM_TO_BF_E);	
	fprintf(fp, "LSM_TO_BF_F=%.6f\n", this->LSM_TO_BF_F);	

	fprintf(fp, "SCAN_METHOD=%d\n", this->SCAN_METHOD);
	fprintf(fp, "USE_SIMIMG=%d\n", this->USE_SIMIMG);

	fprintf(fp, "SCAN_Z_REVERSE=%d\n", this->SCAN_Z_REVERSE);

	//finish up
	if (fp) fclose(fp);
	return true;
}	
//==================================================================================


void LSM_Setup_Dialog::copyDataOut(Parameters_LSM *pl)
{
	if (!pl)
		return;

	pl->DEVICE_X = dev_x;
	pl->DEVICE_Y = dev_y;
	pl->DEVICE_Z = dev_z;

	pl->AO_X = ao_x;
	pl->AO_Y = ao_y;
	pl->AO_Z = ao_z;

	pl->AI_COUNTER_RED = aictr_red;
	pl->AI_COUNTER_GREEN = aictr_green;

	pl->DEVICE_RED_AI = dev_red_ai;
	pl->DEVICE_RED_CTR = dev_red_ctr;
	pl->PORT_RED_AI = red_n_ai;
	pl->PORT_RED_CTR = red_n_ctr;
	pl->DEVICE_GREEN_AI = dev_green_ai;
	pl->DEVICE_GREEN_CTR = dev_green_ctr;
	pl->PORT_GREEN_AI = green_n_ai;
	pl->PORT_GREEN_CTR = green_n_ctr;

	pl->DEVICE_TIMING = dev_timing;
	pl->PORT_TIMING = timing_n;
	pl->DEVICE_CLOCK = dev_clk;
	pl->PORT_CLOCK = clk_n;

	pl->FOVX_MIN = fov_min_x;
	pl->FOVX_MAX = fov_max_x;
	pl->FOVX_STEP = fov_step_x;

	pl->FOVY_MIN = fov_min_y;
	pl->FOVY_MAX = fov_max_y;
	pl->FOVY_STEP = fov_step_y;

	pl->FOVZ_MIN = fov_min_z;
	pl->FOVZ_MAX = fov_max_z;
	pl->FOVZ_STEP = fov_step_z;

	pl->RATIO_X = ratio_x;
	pl->RATIO_Y = ratio_y;
	pl->RATIO_Z = ratio_z;

	pl->TIMEOUT = time_out;
	pl->SCANNING_RATE = scanning_rate;

	pl->SX = dims_x;
	pl->SY = dims_y;
	pl->SZ = dims_z;

	pl->CHANNEL = ch;
	pl->MARKER_SORTED = marker_sorted;

	pl->SCAN_METHOD = scanMethod;
	pl->USE_SIMIMG = b_useSimImg;
	pl->SCAN_Z_REVERSE = b_scan_z_reverse;
}

LSM_Setup_Dialog::LSM_Setup_Dialog(Parameters_LSM *pl, bool b_initnull)
{
	b_resetInProgress=false;

	if (!pl || b_initnull==true)
	{
		dev_x = 1;
		dev_y = 1; //NI PCI 6115
		dev_z = 3;// NI USB 6229 for z piezo

		ao_x = 0; 
		ao_y = 1;
		ao_z = 2;

		aictr_red = 1;
		aictr_green = 1;

		dev_red_ai = 1;
		dev_red_ctr = 3;
		red_n_ai = 1;
		red_n_ctr = 1; //original is 1

		dev_green_ai = 1;
		dev_green_ctr = 3;
		green_n_ai = 0;
		green_n_ctr = 0; //original is 0

		dev_timing = 1;
		timing_n = 0; //PFI
		dev_clk = 2;
		clk_n = 0;

		fov_min_x = -150.0;
		fov_max_x = 150.0;
		fov_step_x = 1.0;

		fov_min_y = -150.0;
		fov_max_y = 150.0;
		fov_step_y = 1.0;

		fov_min_z = 30.0;
		fov_max_z = 60.0;
		fov_step_z = 1.0;

		ratio_x = 59.0;
		ratio_y = 59.0;
		ratio_z = 10.0;

		time_out = 600.0;
		scanning_rate = 20000.0;

		ch = 2;

		dims_x = (fov_max_x - fov_min_x)/fov_step_x; //this is a bug, should add 1 here, by PHC, 120430
		dims_y = (fov_max_y - fov_min_y)/fov_step_y; //this is a bug, should add 1 here, by PHC, 120430
		dims_z = (fov_max_z - fov_min_z)/fov_step_z; //this is a bug, should add 1 here, by PHC, 120430

		scanMethod = 0;
		b_useSimImg = 0;
		b_scan_z_reverse = 0;

		// reserve for non do_LSM_single() functions
		
	}
	else //then just copy from *pl
	{
		dev_x = pl->DEVICE_X;
		dev_y = pl->DEVICE_Y;
		dev_z = pl->DEVICE_Z;

		ao_x = pl->AO_X;
		ao_y = pl->AO_Y;
		ao_z = pl->AO_Z;

		aictr_red = pl->AI_COUNTER_RED;
		aictr_green = pl->AI_COUNTER_GREEN;

		dev_red_ai = pl->DEVICE_RED_AI;
		dev_red_ctr = pl->DEVICE_RED_CTR;
		red_n_ai = pl->PORT_RED_AI;
		red_n_ctr = pl->PORT_RED_CTR;

		dev_green_ai = pl->DEVICE_GREEN_AI;
		dev_green_ctr = pl->DEVICE_GREEN_CTR;
		green_n_ai = pl->PORT_GREEN_AI;
		green_n_ctr = pl->PORT_GREEN_CTR;

		dev_timing = pl->DEVICE_TIMING;
		timing_n = pl->PORT_TIMING;
		dev_clk = pl->DEVICE_CLOCK;
		clk_n = pl->PORT_CLOCK;

		fov_min_x = pl->FOVX_MIN;
		fov_max_x = pl->FOVX_MAX;
		dims_x = pl->SX;
		if (dims_x<1) 
		{
			v3d_msg("The x dimension is illegal!");
			dims_x=1;
		}
		fov_step_x = pl->FOVX_STEP;//(fov_max_x-fov_min_x)/dims_x; 

		fov_min_y = pl->FOVY_MIN;
		fov_max_y = pl->FOVY_MAX;
		dims_y = pl->SY;
		if (dims_y<1) 
		{
			v3d_msg("The y dimension is illegal!");
			dims_y=1;
		}
		fov_step_y = pl->FOVY_STEP;//(fov_max_y-fov_min_y)/dims_y; 

		fov_min_z = pl->FOVZ_MIN;
		fov_max_z = pl->FOVZ_MAX;
		dims_z = pl->SZ;
		if (dims_z<1) 
		{
			v3d_msg("The z dimension is illegal!");
			dims_z=1;
		}
		fov_step_z = pl->FOVZ_STEP;//(fov_max_z-fov_min_z)/dims_z; 

		ratio_x = pl->RATIO_X;
		ratio_y = pl->RATIO_Y;
		ratio_z = pl->RATIO_Z;

		time_out = pl->TIMEOUT;
		scanning_rate = pl->SCANNING_RATE;

		ch = pl->CHANNEL;
		marker_sorted = pl->MARKER_SORTED;

		scanMethod = pl->SCAN_METHOD;
		b_useSimImg = pl->USE_SIMIMG;
		b_scan_z_reverse = pl->SCAN_Z_REVERSE;
	}

	//create a dialog
	QVBoxLayout *mainLayout = new QVBoxLayout;
	QGroupBox *groupLSM = new QGroupBox(tr("LSM Configuration"));
	mainLayout->addWidget(groupLSM);
	gridLayout = new QGridLayout();

	// set device
	//label_dev_conf = new QLabel(QObject::tr("LSM Configuration: "));
	//gridLayout->addWidget(label_dev_conf, 0,0,1,1);

	label_xdev = new QLabel(QObject::tr("X: Device "));
	label_ydev = new QLabel(QObject::tr("Y: Device "));
	label_zdev = new QLabel(QObject::tr("Z: Device "));

	qsb_dev_x = new QSpinBox();
	qsb_dev_y = new QSpinBox();
	qsb_dev_z = new QSpinBox();

	// device init
	qsb_dev_x->setMaximum(3); qsb_dev_x->setMinimum(1); qsb_dev_x->setValue(dev_x); qsb_dev_x->setSingleStep(1);
	qsb_dev_y->setMaximum(3); qsb_dev_y->setMinimum(1); qsb_dev_y->setValue(dev_y); qsb_dev_y->setSingleStep(1);
	qsb_dev_z->setMaximum(3); qsb_dev_z->setMinimum(1); qsb_dev_z->setValue(dev_z); qsb_dev_z->setSingleStep(1);

	gridLayout->addWidget(label_xdev, 2,0,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_dev_x, 2,1,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_ydev, 3,0,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_dev_y, 3,1,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_zdev, 4,0,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_dev_z, 4,1,1,1,Qt::AlignRight);

	label_xao = new QLabel(QObject::tr("ao "));
	label_yao = new QLabel(QObject::tr("ao "));
	label_zao = new QLabel(QObject::tr("ao "));

	qsb_ao_x = new QSpinBox();
	qsb_ao_y = new QSpinBox();
	qsb_ao_z = new QSpinBox();

	qsb_ao_x->setMaximum(3); qsb_ao_x->setMinimum(0); qsb_ao_x->setValue(ao_x); qsb_ao_x->setSingleStep(1);
	qsb_ao_y->setMaximum(3); qsb_ao_y->setMinimum(0); qsb_ao_y->setValue(ao_y); qsb_ao_y->setSingleStep(1);
	qsb_ao_z->setMaximum(3); qsb_ao_z->setMinimum(0); qsb_ao_z->setValue(ao_z); qsb_ao_z->setSingleStep(1);

	gridLayout->addWidget(label_xao, 2,2,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_ao_x, 2,3,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_yao, 3,2,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_ao_y, 3,3,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_zao, 4,2,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_ao_z, 4,3,1,1,Qt::AlignRight);

	// TOBE_DELETED
	//QStringList qstrlist_aictr;
	//qstrlist_aictr << "analog PMT"; qstrlist_aictr << "photon counting PMT";
	//combo_aictr =  new QComboBox(); combo_aictr->addItems(qstrlist_aictr); combo_aictr->setCurrentIndex(aictr);
	//label_aictr = new QLabel(QObject::tr("Detector type: "));

	//gridLayout->addWidget(label_aictr, 5,0,Qt::AlignRight); gridLayout->addWidget(combo_aictr, 5,1);

	// red ai_ctr
	QStringList qstrlist_aictr_red;
	qstrlist_aictr_red << "analog PMT"; qstrlist_aictr_red << "photon counting PMT";
	combo_aictr_red =  new QComboBox(); combo_aictr_red->addItems(qstrlist_aictr_red); combo_aictr_red->setCurrentIndex(aictr_red);
	label_aictr_red = new QLabel(QObject::tr("2nd Channel (red)  :Detector type:"));

	// green ai_ctr
	QStringList qstrlist_aictr_green;
	qstrlist_aictr_green << "analog PMT"; qstrlist_aictr_green << "photon counting PMT";
	combo_aictr_green =  new QComboBox(); combo_aictr_green->addItems(qstrlist_aictr_green); combo_aictr_green->setCurrentIndex(aictr_green);
	label_aictr_green = new QLabel(QObject::tr("1st Channel (green):Detector type:"));


	label_reddev = new QLabel(QObject::tr("Device "));
	label_greendev = new QLabel(QObject::tr("Device "));
	label_timingdev = new QLabel(QObject::tr("Timing: Device "));
	label_clkdev = new QLabel(QObject::tr("Clock: Device "));

	// TOBE_DELETED
	//qstr_aictr = (aictr==0) ? "ai " : "ctr ";

	qstr_aictr_red = (aictr_red==0) ? "ai " : "ctr ";
	qstr_aictr_green = (aictr_green==0) ? "ai " : "ctr ";

	label_red = new QLabel; // ai or ctr
	label_green = new QLabel;

	label_red->setText(qstr_aictr_red.toStdString().c_str()); // ai or ctr
	label_green->setText(qstr_aictr_green.toStdString().c_str());

	label_timing = new QLabel(QObject::tr("PFI "));
	label_clk = new QLabel(QObject::tr("ctr "));

	qsb_dev_red = new QSpinBox();
	qsb_dev_green = new QSpinBox();
	qsb_dev_timing = new QSpinBox();
	qsb_dev_clk = new QSpinBox();
	qsb_red = new QSpinBox();
	qsb_green = new QSpinBox();
	qsb_timing = new QSpinBox();
	qsb_clk = new QSpinBox();

	// red, green, timing, and clk init
	qsb_dev_red->setMaximum(100); qsb_dev_red->setMinimum(0); qsb_dev_red->setSingleStep(1);
	qsb_dev_green->setMaximum(100); qsb_dev_green->setMinimum(0); qsb_dev_green->setSingleStep(1);
	qsb_red->setMaximum(100); qsb_red->setMinimum(0); 
	
	if(aictr_red==0) {qsb_dev_red->setValue(dev_red_ai); qsb_red->setValue(red_n_ai); }
	else {qsb_dev_red->setValue(dev_red_ctr); qsb_red->setValue(red_n_ctr); }

	qsb_red->setSingleStep(1);
	qsb_green->setMaximum(100); qsb_green->setMinimum(0); 

	if(aictr_green==0) {qsb_dev_green->setValue(dev_green_ai); qsb_green->setValue(green_n_ai); }
	else {qsb_dev_green->setValue(dev_green_ctr); qsb_green->setValue(green_n_ctr); }

	qsb_green->setSingleStep(1);

	qsb_dev_timing->setMaximum(100); qsb_dev_timing->setMinimum(0); qsb_dev_timing->setValue(dev_timing); qsb_dev_timing->setSingleStep(1);
	qsb_dev_clk->setMaximum(100); qsb_dev_clk->setMinimum(0); qsb_dev_clk->setValue(dev_clk); qsb_dev_clk->setSingleStep(1);
	qsb_timing->setMaximum(100); qsb_timing->setMinimum(0); qsb_timing->setValue(timing_n); qsb_timing->setSingleStep(1);
	qsb_clk->setMaximum(100); qsb_clk->setMinimum(0); qsb_clk->setValue(clk_n); qsb_clk->setSingleStep(1);

	gridLayout->addWidget(label_aictr_green, 6,0,1,1,Qt::AlignRight); gridLayout->addWidget(combo_aictr_green, 6,1,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_greendev, 6,2,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_dev_green, 6,3,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_green, 6,4,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_green, 6,5,1,1,Qt::AlignRight);

	gridLayout->addWidget(label_aictr_red, 7,0,1,1,Qt::AlignRight); gridLayout->addWidget(combo_aictr_red, 7,1,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_reddev, 7,2,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_dev_red, 7,3,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_red, 7,4,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_red, 7,5,1,1,Qt::AlignRight);

	gridLayout->addWidget(label_timingdev, 8,0,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_dev_timing, 8,1,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_timing, 8,2,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_timing, 8,3,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_clkdev, 10,0,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_dev_clk, 10,1,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_clk, 10,2,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_clk, 10,3,1,1,Qt::AlignRight);

	groupLSM->setLayout(gridLayout);

	// groupbox for imaging paras
	QGroupBox *groupImg = new QGroupBox(tr("Image Configuration"));
	QGridLayout *gridLayoutImg = new QGridLayout();
	// add to parent grid
	mainLayout->addWidget(groupImg);

	// set imaging parameters
	check_stepsize_or_dims = new QCheckBox();
	check_stepsize_or_dims->setText(QObject::tr("Setting step-sizes (ON) or image dimensions (OFF)."));
	check_stepsize_or_dims->setChecked(true);

	check_scan_z_reverse  = new QCheckBox();
	check_scan_z_reverse->setText(QObject::tr("Z Scanning Reverse"));
	check_scan_z_reverse->setChecked(b_scan_z_reverse);

	reset     = new QPushButton("Reset FOV, Dim");

	// image display
	label_image_size = new QLabel(QObject::tr("Image size:"));
	//label_image_size->setFrameStyle(QFrame::Panel | QFrame::Raised); //Sunken

	// total time display
	label_total_time_num = new QLabel(QObject::tr("Scanning time:"));
	//label_total_time_num->setFrameStyle(QFrame::Panel | QFrame::Raised); //Sunken

	gridLayoutImg->addWidget(check_stepsize_or_dims, 13,0,1,1); gridLayoutImg->addWidget(check_scan_z_reverse, 13,1,1,1);
	gridLayoutImg->addWidget(reset, 13,2,1,1);
	gridLayoutImg->addWidget(label_image_size,  13,4,1,1,Qt::AlignRight); gridLayoutImg->addWidget(label_total_time_num, 13,5,1,1,Qt::AlignRight);

	//x,y,z scanning range

	label_fov_min_x = new QLabel(QObject::tr("FOV: x_min(um) "));
	label_fov_max_x = new QLabel(QObject::tr("FOV: x_max(um) "));
	label_fov_min_y = new QLabel(QObject::tr("FOV: y_min(um) "));
	label_fov_max_y = new QLabel(QObject::tr("FOV: y_max(um) "));
	label_fov_min_z = new QLabel(QObject::tr("FOV: z_min(um) "));
	label_fov_max_z = new QLabel(QObject::tr("FOV: z_max(um) "));

	label_fov_step_x = new QLabel(QObject::tr("FOV: x_step(um) "));
	label_fov_step_y = new QLabel(QObject::tr("FOV: y_step(um) "));
	label_fov_step_z = new QLabel(QObject::tr("FOV: z_step(um) "));

	qsb_fov_min_x = new QDoubleSpinBox(); 
	qsb_fov_max_x = new QDoubleSpinBox();
	qsb_fov_min_y = new QDoubleSpinBox(); 
	qsb_fov_max_y = new QDoubleSpinBox();
	qsb_fov_min_z = new QDoubleSpinBox(); 
	qsb_fov_max_z = new QDoubleSpinBox();

	qsb_fov_step_x = new QDoubleSpinBox();
	qsb_fov_step_y = new QDoubleSpinBox();
	qsb_fov_step_z = new QDoubleSpinBox();

	qsb_fov_min_x->setMaximum(1000); qsb_fov_min_x->setMinimum(-1000); qsb_fov_min_x->setValue(fov_min_x); qsb_fov_min_x->setSingleStep(0.01);
	qsb_fov_max_x->setMaximum(1000); qsb_fov_max_x->setMinimum(-1000); qsb_fov_max_x->setValue(fov_max_x); qsb_fov_max_x->setSingleStep(0.01);
	qsb_fov_min_y->setMaximum(1000); qsb_fov_min_y->setMinimum(-1000); qsb_fov_min_y->setValue(fov_min_y); qsb_fov_min_y->setSingleStep(0.01);
	qsb_fov_max_y->setMaximum(1000); qsb_fov_max_y->setMinimum(-1000); qsb_fov_max_y->setValue(fov_max_y); qsb_fov_max_y->setSingleStep(0.01);
	qsb_fov_min_z->setMaximum(1000); qsb_fov_min_z->setMinimum(0); qsb_fov_min_z->setValue(fov_min_z); qsb_fov_min_z->setSingleStep(0.01);
	qsb_fov_max_z->setMaximum(1000); qsb_fov_max_z->setMinimum(0); qsb_fov_max_z->setValue(fov_max_z); qsb_fov_max_z->setSingleStep(0.01);
	

	qsb_fov_step_x->setDecimals(2);
	qsb_fov_step_y->setDecimals(2);
	qsb_fov_step_z->setDecimals(2);

	qsb_fov_step_x->setMaximum(1000); qsb_fov_step_x->setMinimum(0.01); qsb_fov_step_x->setValue(fov_step_x); qsb_fov_step_x->setSingleStep(0.01); 
	qsb_fov_step_y->setMaximum(1000); qsb_fov_step_y->setMinimum(0.01); qsb_fov_step_y->setValue(fov_step_y); qsb_fov_step_y->setSingleStep(0.01); 
	qsb_fov_step_z->setMaximum(1000); qsb_fov_step_z->setMinimum(0.01); qsb_fov_step_z->setValue(fov_step_z); qsb_fov_step_z->setSingleStep(0.01); 

	gridLayoutImg->addWidget(label_fov_min_x, 14,0,1,1,Qt::AlignRight); gridLayoutImg->addWidget(qsb_fov_min_x, 14,1,1,1,Qt::AlignRight); 
	gridLayoutImg->addWidget(label_fov_max_x, 14,2,1,1,Qt::AlignRight); gridLayoutImg->addWidget(qsb_fov_max_x, 14,3,1,1,Qt::AlignRight);
	gridLayoutImg->addWidget(label_fov_step_x, 14,4,1,1,Qt::AlignRight); gridLayoutImg->addWidget(qsb_fov_step_x, 14,5,1,1,Qt::AlignRight);

	gridLayoutImg->addWidget(label_fov_min_y, 15,0,1,1,Qt::AlignRight); gridLayoutImg->addWidget(qsb_fov_min_y, 15,1,1,1,Qt::AlignRight); 
	gridLayoutImg->addWidget(label_fov_max_y, 15,2,1,1,Qt::AlignRight); gridLayoutImg->addWidget(qsb_fov_max_y, 15,3,1,1,Qt::AlignRight);
	gridLayoutImg->addWidget(label_fov_step_y, 15,4,1,1,Qt::AlignRight); gridLayoutImg->addWidget(qsb_fov_step_y, 15,5,1,1,Qt::AlignRight);

	gridLayoutImg->addWidget(label_fov_min_z, 16,0,1,1,Qt::AlignRight); gridLayoutImg->addWidget(qsb_fov_min_z, 16,1,1,1,Qt::AlignRight); 
	gridLayoutImg->addWidget(label_fov_max_z, 16,2,1,1,Qt::AlignRight); gridLayoutImg->addWidget(qsb_fov_max_z, 16,3,1,1,Qt::AlignRight);
	gridLayoutImg->addWidget(label_fov_step_z, 16,4,1,1,Qt::AlignRight); gridLayoutImg->addWidget(qsb_fov_step_z, 16,5,1,1,Qt::AlignRight);

	
	//always disable the step-size setting. Can only adjust them using the dimensions
	qsb_fov_step_x->setDisabled(true);
	qsb_fov_step_y->setDisabled(true);
	qsb_fov_step_z->setDisabled(true);

	//x,y,z dimensions
	label_dim_x = new QLabel(QObject::tr("Image Dimensions X: "));
	label_dim_y = new QLabel(QObject::tr(" Y: "));
	label_dim_z = new QLabel(QObject::tr(" Z: "));

	qsb_dim_x = new QSpinBox();
	qsb_dim_y = new QSpinBox();
	qsb_dim_z = new QSpinBox();

	qsb_dim_x->setMaximum(10000); qsb_dim_x->setMinimum(1); qsb_dim_x->setValue(dims_x); qsb_dim_x->setSingleStep(1);
	qsb_dim_y->setMaximum(10000); qsb_dim_y->setMinimum(1); qsb_dim_y->setValue(dims_y); qsb_dim_y->setSingleStep(1);
	qsb_dim_z->setMaximum(10000); qsb_dim_z->setMinimum(1); qsb_dim_z->setValue(dims_z); qsb_dim_z->setSingleStep(1);

	gridLayoutImg->addWidget(label_dim_x, 18,0,1,1,Qt::AlignRight); gridLayoutImg->addWidget(qsb_dim_x, 18,1,1,1,Qt::AlignRight);
	gridLayoutImg->addWidget(label_dim_y, 18,2,1,1,Qt::AlignRight); gridLayoutImg->addWidget(qsb_dim_y, 18,3,1,1,Qt::AlignRight);
	gridLayoutImg->addWidget(label_dim_z, 18,4,1,1,Qt::AlignRight); gridLayoutImg->addWidget(qsb_dim_z, 18,5,1,1,Qt::AlignRight);

	// groupBox
	groupImg->setLayout(gridLayoutImg);

	// groupbox for laser paras
	QGroupBox *groupLaser = new QGroupBox(tr("Run Configuration"));
	QGridLayout *gridLayoutLaser = new QGridLayout();
	// add to parent grid
	mainLayout->addWidget(groupLaser);

	// ratio for x,y,z scanning
	label_ratio_x = new QLabel(QObject::tr("Ratio(um/V) X: "));
	label_ratio_y = new QLabel(QObject::tr(" Y: "));
	label_ratio_z = new QLabel(QObject::tr(" Z: "));

	qsb_ratio_x = new QDoubleSpinBox();
	qsb_ratio_y = new QDoubleSpinBox();
	qsb_ratio_z = new QDoubleSpinBox();

	qsb_ratio_x->setMaximum(1000); qsb_ratio_x->setMinimum(0.01); qsb_ratio_x->setValue(ratio_x); qsb_ratio_x->setSingleStep(0.01);
	qsb_ratio_y->setMaximum(1000); qsb_ratio_y->setMinimum(0.01); qsb_ratio_y->setValue(ratio_y); qsb_ratio_y->setSingleStep(0.01);
	qsb_ratio_z->setMaximum(1000); qsb_ratio_z->setMinimum(0.01); qsb_ratio_z->setValue(ratio_z); qsb_ratio_z->setSingleStep(0.01);

	gridLayoutLaser->addWidget(label_ratio_x, 20,0,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(qsb_ratio_x, 20,1,1,1,Qt::AlignRight);
	gridLayoutLaser->addWidget(label_ratio_y, 20,2,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(qsb_ratio_y, 20,3,1,1,Qt::AlignRight);
	gridLayoutLaser->addWidget(label_ratio_z, 20,4,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(qsb_ratio_z, 20,5,1,1,Qt::AlignRight);

	// time out
	label_timeout = new QLabel(QObject::tr("Time Out (s): "));
	qsb_timeout = new QDoubleSpinBox();
	qsb_timeout->setMaximum(100000000); qsb_timeout->setMinimum(0.01); qsb_timeout->setValue(time_out); qsb_timeout->setSingleStep(0.01);

	// scan method
	label_scan_method = new QLabel(QObject::tr("Scanning Method: ")); // ZJL
	QStringList scanmd;
	scanmd << "single_acc"; scanmd << "zigzag_acc"; 
	combo_scan_method = new QComboBox(); combo_scan_method->addItems(scanmd); combo_scan_method->setCurrentIndex(scanMethod);
	
	// use sim img
	label_use_sim_img = new QLabel(QObject::tr("Simulation image: "));
	check_use_sim_img = new QCheckBox();
	check_use_sim_img->setText(QObject::tr("           Use"));
	check_use_sim_img->setLayoutDirection(Qt::RightToLeft);
	check_use_sim_img->setChecked(b_useSimImg);

	gridLayoutLaser->addWidget(label_timeout,     22,0,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(qsb_timeout,       22,1,1,1,Qt::AlignRight);
	gridLayoutLaser->addWidget(label_use_sim_img, 22,2,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(check_use_sim_img, 22,3,1,1,Qt::AlignRight);
	
	// scanning rate
	label_scanrate = new QLabel(QObject::tr("Scanning Rate (Hz): "));
	qsb_scanrate = new QDoubleSpinBox();
	qsb_scanrate->setMaximum(100000000); qsb_scanrate->setMinimum(0.01); qsb_scanrate->setValue(scanning_rate); qsb_scanrate->setSingleStep(0.01);
	gridLayoutLaser->addWidget(label_scanrate, 24,0,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(qsb_scanrate, 24,1,1,1,Qt::AlignRight);
	gridLayoutLaser->addWidget(label_scan_method, 24,2,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(combo_scan_method, 24,3,1,1,Qt::AlignRight);


	// red, green channel, or both color imaging
	QStringList chs;
	chs << "green"; chs << "red"; chs << "both";
	combo_channel =  new QComboBox(); combo_channel->addItems(chs); combo_channel->setCurrentIndex(ch);

	label_ch = new QLabel(QObject::tr("Choose laser (r561nm/g488nm): "));

	gridLayoutLaser->addWidget(label_ch, 26,0,Qt::AlignRight); gridLayoutLaser->addWidget(combo_channel, 26,1);

	// sorting markers or not
	QStringList sorts;
	sorts << "without sorting markers"; sorts << "with sorting markers";
	combo_sort =  new QComboBox(); combo_sort->addItems(sorts); combo_sort->setCurrentIndex(marker_sorted);

	label_sort = new QLabel(QObject::tr("Stimulating : "));

	paste     = new QPushButton("Reuse Config Info");

	gridLayoutLaser->addWidget(label_sort, 28,0,Qt::AlignRight); gridLayoutLaser->addWidget(combo_sort, 28,1); gridLayoutLaser->addWidget(paste, 28,5);

	//
	label_log_dir = new QLabel(QObject::tr("Choose a log file directory: "));
	edit_log_dir = new QLineEdit(dir);

	pb_browse = new QPushButton("Browse...");

	gridLayoutLaser->addWidget(label_log_dir,30,0,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(edit_log_dir, 30,1,1,4); gridLayoutLaser->addWidget(pb_browse, 30,5,1,1,Qt::AlignRight);

	// groupBox
	groupLaser->setLayout(gridLayoutLaser);

	// state update
	if(check_stepsize_or_dims->isChecked())
	{
		qsb_fov_step_x->setEnabled(true);
		qsb_fov_step_y->setEnabled(true);
		qsb_fov_step_z->setEnabled(true);

		qsb_dim_x->setDisabled(true);
		qsb_dim_y->setDisabled(true);
		qsb_dim_z->setDisabled(true);
	}
	else
	{
		qsb_dim_x->setEnabled(true);
		qsb_dim_y->setEnabled(true);
		qsb_dim_z->setEnabled(true);

		qsb_fov_step_x->setDisabled(true);
		qsb_fov_step_y->setDisabled(true);
		qsb_fov_step_z->setDisabled(true);
	}

	// button
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                      | QDialogButtonBox::Cancel);
	// add to parent grid
	mainLayout->addWidget(buttonBox);

	// update time label
	double total_time = (double)(dims_x*dims_y*dims_z/(double)scanning_rate);
	QString str_time="Scanning time: [" + QString("<font color='red'> %1 </font>").arg(total_time,0,'f',2)+"]s";
	label_total_time_num->setText(str_time);

	// update image size display
	int num_ch;
	ch = combo_channel->currentIndex();
	if(ch==0 || ch==1) num_ch=1;
	else num_ch=2;
	double imgSize = dims_x*dims_y*dims_z * num_ch * sizeof(float)/1024.0/1024.0;
	QString str_imgSize="Image size: [" + QString("<font color='red'> %1 </font>").arg(imgSize,0,'f',2)+"]MB";
	label_image_size->setText(str_imgSize);

	//gridlayout
	setLayout(mainLayout);
	setWindowTitle(QString("Microscope Imaging"));

	//slot interface
	connect(qsb_dev_x, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_dev_y, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_dev_z, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_ao_x, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_ao_y, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_ao_z, SIGNAL(valueChanged(int)), this, SLOT(update()));

	connect(qsb_dev_red, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_dev_green, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_dev_timing, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_dev_clk, SIGNAL(valueChanged(int)), this, SLOT(update()));

	connect(qsb_red, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_green, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_timing, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_clk, SIGNAL(valueChanged(int)), this, SLOT(update()));

	connect(check_stepsize_or_dims, SIGNAL(stateChanged(int)), this, SLOT(update()));
	connect(check_scan_z_reverse, SIGNAL(stateChanged(int)), this, SLOT(update()));

	connect(qsb_fov_min_x, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_fov_max_x, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_fov_min_y, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_fov_max_y, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_fov_min_z, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_fov_max_z, SIGNAL(valueChanged(double)), this, SLOT(update()));

	connect(qsb_fov_step_x, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_fov_step_y, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_fov_step_z, SIGNAL(valueChanged(double)), this, SLOT(update()));

	connect(qsb_ratio_x, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_ratio_y, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_ratio_z, SIGNAL(valueChanged(double)), this, SLOT(update()));

	connect(qsb_dim_x, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_dim_y, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_dim_z, SIGNAL(valueChanged(int)), this, SLOT(update()));

	connect(qsb_timeout, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_scanrate, SIGNAL(valueChanged(double)), this, SLOT(update()));

	// for scan method and use_sim_img
	connect(combo_scan_method, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
	connect(check_use_sim_img, SIGNAL(stateChanged(int)), this, SLOT(update()));
	//

	connect(pb_browse,     SIGNAL(clicked()), this, SLOT(get_dir()));
	connect(combo_channel, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));

	connect(combo_sort, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));

	connect(combo_aictr_red, SIGNAL(currentIndexChanged(int)), this, SLOT(update_dev_aictr()));
	connect(combo_aictr_green, SIGNAL(currentIndexChanged(int)), this, SLOT(update_dev_aictr()));
	
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	connect(reset, SIGNAL(clicked()), this, SLOT(resetFOVDIM()));
	connect(paste, SIGNAL(clicked()), this, SLOT(pasteConfigInfo()));
}

LSM_Setup_Dialog::~LSM_Setup_Dialog(){}

void LSM_Setup_Dialog::pasteConfigInfo()
{
	b_resetInProgress=true;

	// get paras from $ssConfigFolder/mi_configuration.txt
	QString lsm_FileName = ssConfigFolder + "mi_configuration_clipboard.txt";

	Parameters_LSM pl;
	pl.load_ParameterFile(lsm_FileName);


	dev_x = pl.DEVICE_X; dev_y = pl.DEVICE_Y; dev_z = pl.DEVICE_Z;
	ao_x = pl.AO_X;	ao_y = pl.AO_Y;	ao_z = pl.AO_Z;	aictr_red = pl.AI_COUNTER_RED; aictr_green = pl.AI_COUNTER_GREEN;
	dev_red_ai = pl.DEVICE_RED_AI; dev_red_ctr = pl.DEVICE_RED_CTR; red_n_ai = pl.PORT_RED_AI; red_n_ctr = pl.PORT_RED_CTR; 
	dev_green_ai = pl.DEVICE_GREEN_AI; dev_green_ctr = pl.DEVICE_GREEN_CTR; green_n_ai = pl.PORT_GREEN_AI; green_n_ctr = pl.PORT_GREEN_CTR;
	dev_timing = pl.DEVICE_TIMING; timing_n = pl.PORT_TIMING; dev_clk = pl.DEVICE_CLOCK; clk_n = pl.PORT_CLOCK;

	fov_min_x = pl.FOVX_MIN;	fov_max_x = pl.FOVX_MAX;	dims_x = pl.SX;	fov_step_x = pl.FOVX_STEP; 
	fov_min_y = pl.FOVY_MIN;	fov_max_y = pl.FOVY_MAX;	dims_y = pl.SY;	fov_step_y = pl.FOVY_STEP; 
	fov_min_z = pl.FOVZ_MIN;	fov_max_z = pl.FOVZ_MAX;	dims_z = pl.SZ;	fov_step_z = pl.FOVZ_STEP; 
	ratio_x = pl.RATIO_X; ratio_y = pl.RATIO_Y;	ratio_z = pl.RATIO_Z; time_out = pl.TIMEOUT;
	scanning_rate = pl.SCANNING_RATE; ch = pl.CHANNEL; marker_sorted = pl.MARKER_SORTED;
	scanMethod = pl.SCAN_METHOD; b_useSimImg = pl.USE_SIMIMG; //duty_cycle = pl.DUTY_CYCLE;
	b_scan_z_reverse=pl.SCAN_Z_REVERSE;

	qsb_dev_x->setValue(dev_x); qsb_dev_y->setValue(dev_y); qsb_dev_z->setValue(dev_z);
	qsb_ao_x->setValue(ao_x); qsb_ao_y->setValue(ao_y); qsb_ao_z->setValue(ao_z); combo_aictr_red->setCurrentIndex(aictr_red); combo_aictr_green->setCurrentIndex(aictr_green);
	  
	if(aictr_red==0) {qsb_dev_red->setValue(dev_red_ai); qsb_red->setValue(red_n_ai);}
	else {qsb_dev_red->setValue(dev_red_ctr); qsb_red->setValue(red_n_ctr);}

	if(aictr_green==0) {qsb_dev_green->setValue(dev_green_ai); qsb_green->setValue(green_n_ai);}
	else {qsb_dev_green->setValue(dev_green_ctr); qsb_green->setValue(green_n_ctr);}

	qsb_dev_timing->setValue(dev_timing);  qsb_timing->setValue(timing_n); qsb_dev_clk->setValue(dev_clk); qsb_clk->setValue(clk_n);

	qsb_fov_min_x->setValue(fov_min_x); 
	qsb_fov_max_x->setValue(fov_max_x);
	qsb_fov_step_x->setValue(fov_step_x);
	
	qsb_fov_min_y->setValue(fov_min_y); 
	qsb_fov_max_y->setValue(fov_max_y); 
	qsb_fov_step_y->setValue(fov_step_y);

	qsb_fov_min_z->setValue(fov_min_z); 
	qsb_fov_max_z->setValue(fov_max_z);
	qsb_fov_step_z->setValue(fov_step_z);

	qsb_dim_x->setValue(dims_x);
	qsb_dim_y->setValue(dims_y);
	qsb_dim_z->setValue(dims_z);

	qsb_ratio_x->setValue(ratio_x);
	qsb_ratio_y->setValue(ratio_y);
	qsb_ratio_z->setValue(ratio_z);

	qsb_scanrate->setValue(scanning_rate);
	combo_channel->setCurrentIndex(ch);
	combo_sort->setCurrentIndex(marker_sorted);
	combo_scan_method->setCurrentIndex(scanMethod);
	check_use_sim_img->setChecked(b_useSimImg);
	check_scan_z_reverse->setChecked(b_scan_z_reverse);

	// update time label
	double total_time = (double)(dims_x*dims_y*dims_z/(double)scanning_rate);
	QString str_time="Scanning time: [" + QString("<font color='red'> %1 </font>").arg(total_time,0,'f',2)+"]s";
	label_total_time_num->setText(str_time);

	// update image size display
	int num_ch;
	ch = combo_channel->currentIndex();
	if(ch==0 || ch==1) num_ch=1;
	else num_ch=2;
	double imgSize = dims_x*dims_y*dims_z * num_ch * sizeof(float)/1024.0/1024.0;
	QString str_imgSize="Image size: [" + QString("<font color='red'> %1 </font>").arg(imgSize,0,'f',2)+"]MB";
	label_image_size->setText(str_imgSize);

	b_resetInProgress=false;
}

void LSM_Setup_Dialog::resetFOVDIM()
{
	b_resetInProgress=true;

	fov_min_x = -150.0;
	fov_max_x = 150.0;
	fov_step_x = 1.0;

	fov_min_y = -150.0;
	fov_max_y = 150.0;
	fov_step_y = 1.0;

	fov_min_z = 30.0;
	fov_max_z = 60.0;
	fov_step_z = 1.0;

	dims_x = (fov_max_x - fov_min_x)/fov_step_x;
	dims_y = (fov_max_y - fov_min_y)/fov_step_y;
	dims_z = (fov_max_z - fov_min_z)/fov_step_z;

	// set values
	qsb_fov_min_x->setValue(fov_min_x); 
	qsb_fov_max_x->setValue(fov_max_x);
	qsb_fov_step_x->setValue(fov_step_x);
	
	qsb_fov_min_y->setValue(fov_min_y); 
	qsb_fov_max_y->setValue(fov_max_y); 
	qsb_fov_step_y->setValue(fov_step_y);

	qsb_fov_min_z->setValue(fov_min_z); 
	qsb_fov_max_z->setValue(fov_max_z);
	qsb_fov_step_z->setValue(fov_step_z);

	qsb_dim_x->setValue(dims_x);
	qsb_dim_y->setValue(dims_y);
	qsb_dim_z->setValue(dims_z);

	// update time label
	double total_time = (double)(dims_x*dims_y*dims_z/(double)scanning_rate);
	QString str_time="Scanning time: [" + QString("<font color='red'> %1 </font>").arg(total_time,0,'f',2)+"]s";
	label_total_time_num->setText(str_time);

	// update image size display
	int num_ch;
	ch = combo_channel->currentIndex();
	if(ch==0 || ch==1) num_ch=1;
	else num_ch=2;
	double imgSize = dims_x*dims_y*dims_z * num_ch * sizeof(float)/1024.0/1024.0;
	QString str_imgSize="Image size: [" + QString("<font color='red'> %1 </font>").arg(imgSize,0,'f',2)+"]MB";
	label_image_size->setText(str_imgSize);

	b_resetInProgress=false;	
}

void LSM_Setup_Dialog::update()
{
	if(b_resetInProgress==false)
	{
		dev_x = qsb_dev_x->text().toInt();
		dev_y = qsb_dev_y->text().toInt();
		dev_z = qsb_dev_z->text().toInt();

		ao_x = qsb_ao_x->text().toInt();
		ao_y = qsb_ao_y->text().toInt();
		ao_z = qsb_ao_z->text().toInt();
		
		if(aictr_red==0) {dev_red_ai = qsb_dev_red->text().toInt(); red_n_ai = qsb_red->text().toInt(); }//ai port
		else {dev_red_ctr = qsb_dev_red->text().toInt(); red_n_ctr = qsb_red->text().toInt(); }//ctr port
		
		if(aictr_green==0) {dev_green_ai = qsb_dev_green->text().toInt(); green_n_ai = qsb_green->text().toInt();}
		else {dev_green_ctr = qsb_dev_green->text().toInt(); green_n_ctr = qsb_green->text().toInt();}

		dev_timing = qsb_dev_timing->text().toInt();
		dev_clk = qsb_dev_clk->text().toInt();

		timing_n = qsb_timing->text().toInt();
		clk_n = qsb_clk->text().toInt();

		ratio_x = qsb_ratio_x->text().toDouble();
		ratio_y = qsb_ratio_y->text().toDouble();
		ratio_z = qsb_ratio_z->text().toDouble();

		time_out = qsb_timeout->text().toDouble();
		scanning_rate = qsb_scanrate->text().toDouble();

		ch = combo_channel->currentIndex();

		marker_sorted = combo_sort->currentIndex();

		scanMethod = combo_scan_method->currentIndex();

		if(check_use_sim_img->isChecked())
			b_useSimImg = 1;
		else
			b_useSimImg = 0;

		// update scan_z_reverse
		if(check_scan_z_reverse->isChecked())
			b_scan_z_reverse = 1;
		else
			b_scan_z_reverse = 0;

		if(check_stepsize_or_dims->isChecked())
		{
			qsb_fov_step_x->setEnabled(true);
			qsb_fov_step_y->setEnabled(true);
			qsb_fov_step_z->setEnabled(true);

			qsb_dim_x->setDisabled(true);
			qsb_dim_y->setDisabled(true);
			qsb_dim_z->setDisabled(true);

			// fov, step -> dims
			update_dims_x();
			update_dims_y();
			update_dims_z();
		}
		else
		{
			qsb_dim_x->setEnabled(true);
			qsb_dim_y->setEnabled(true);
			qsb_dim_z->setEnabled(true);

			qsb_fov_step_x->setDisabled(true);
			qsb_fov_step_y->setDisabled(true);
			qsb_fov_step_z->setDisabled(true);

			// fov, dims -> step
			update_step_x();
			update_step_y();
			update_step_z();
		}
	
		// update time label
		double total_time = (double)(dims_x*dims_y*dims_z/(double)scanning_rate);
		QString str_time="Scanning time: [" + QString("<font color='red'> %1 </font>").arg(total_time,0,'f',2)+"]s";
		label_total_time_num->setText(str_time);

		// update image size display
		int num_ch;
		ch = combo_channel->currentIndex();
		if(ch==0 || ch==1) num_ch=1;
		else num_ch=2;
		double imgSize = dims_x*dims_y*dims_z * num_ch * sizeof(float)/1024.0/1024.0;
			
		QString str_imgSize="Image size: [" + QString("<font color='red'> %1 </font>").arg(imgSize,0,'f',2)+"]MB";
		label_image_size->setText(str_imgSize);
	}// end if b_resetInProgress
}



void LSM_Setup_Dialog::update_dev_aictr()
{
	aictr_red = combo_aictr_red->currentIndex();
	qstr_aictr_red = (aictr_red==0) ? "ai " : "ctr ";

	aictr_green = combo_aictr_green->currentIndex();
	qstr_aictr_green = (aictr_green==0) ? "ai " : "ctr ";

	label_red->setText(qstr_aictr_red.toStdString().c_str()); // ai or ctr
	label_green->setText(qstr_aictr_green.toStdString().c_str());

	// also update default port numbers for ai and ctr
	if(aictr_red==0) {qsb_dev_red->setValue(dev_red_ai); qsb_red->setValue(red_n_ai); }
	else {qsb_dev_red->setValue(dev_red_ctr); qsb_red->setValue(red_n_ctr); }

	if(aictr_green==0) {qsb_dev_green->setValue(dev_green_ai); qsb_green->setValue(green_n_ai); }
	else {qsb_dev_green->setValue(dev_green_ctr); qsb_green->setValue(green_n_ctr);}
}

void LSM_Setup_Dialog::update_step_x()
{
	fov_min_x = qsb_fov_min_x->text().toDouble();
	fov_max_x = qsb_fov_max_x->text().toDouble();

	dims_x = qsb_dim_x->text().toInt();

	if(dims_x<0)
	{
		v3d_msg("The dims_x parameter is illegal in update_step_x().");
		return;
	}

	fov_step_x = (fov_max_x - fov_min_x)/double(dims_x);

	qsb_fov_step_x->setValue(fov_step_x);
}

void LSM_Setup_Dialog::update_step_y()
{
	fov_min_y = qsb_fov_min_y->text().toDouble();
	fov_max_y = qsb_fov_max_y->text().toDouble();

	dims_y = qsb_dim_y->text().toInt();

	if(dims_y<0)
	{
		v3d_msg("The dims_y parameter is illegal in update_step_y().");
		return;
	}

	fov_step_y = (fov_max_y - fov_min_y)/double(dims_y);

	qsb_fov_step_y->setValue(fov_step_y);
}

void LSM_Setup_Dialog::update_step_z()
{
	fov_min_z = qsb_fov_min_z->text().toDouble();
	fov_max_z = qsb_fov_max_z->text().toDouble();

	dims_z = qsb_dim_z->text().toInt();

	if(dims_z<0)
	{
		v3d_msg("The dims_z parameter is illegal in update_step_z().");
		return;
	}

	fov_step_z = (fov_max_z - fov_min_z)/double(dims_z);

	qsb_fov_step_z->setValue(fov_step_z);
}

void LSM_Setup_Dialog::update_dims_x()
{
	fov_min_x = qsb_fov_min_x->text().toDouble();
	fov_max_x = qsb_fov_max_x->text().toDouble();

	fov_step_x = qsb_fov_step_x->text().toDouble();

	double fov_length = fov_max_x - fov_min_x;
	dims_x = long( fov_length / fov_step_x);
	qsb_dim_x->setValue(dims_x);
}

void LSM_Setup_Dialog::update_dims_y()
{
	fov_min_y = qsb_fov_min_y->text().toDouble();
	fov_max_y = qsb_fov_max_y->text().toDouble();

	fov_step_y = qsb_fov_step_y->text().toDouble();

	double fov_length = fov_max_y - fov_min_y;
	dims_y = long( fov_length / fov_step_y);
	qsb_dim_y->setValue(dims_y);
}

void LSM_Setup_Dialog::update_dims_z()
{
	fov_min_z = qsb_fov_min_z->text().toDouble();
	fov_max_z = qsb_fov_max_z->text().toDouble();

	fov_step_z = qsb_fov_step_z->text().toDouble();

	double fov_length = fov_max_z - fov_min_z;
	dims_z = long(fov_length / fov_step_z);
	qsb_dim_z->setValue(dims_z);
}

void LSM_Setup_Dialog::get_dir()
{
	dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "c:\\ ", 
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	edit_log_dir->setText(dir);

	qDebug() << dir;
}


//======================================================================================
//           ROI Parameters Setup dialog
//======================================================================================

LSM_ROI_Setup_Dialog::LSM_ROI_Setup_Dialog(Parameters_LSM *pl, bool b_initnull)
{
	b_resetInProgress=false;

	if (!pl || b_initnull==true)
	{
		fov_min_x = -150.0;
		fov_max_x = 150.0;
		fov_step_x = 1.0;

		fov_min_y = -150.0;
		fov_max_y = 150.0;
		fov_step_y = 1.0;

		fov_min_z = 30.0;
		fov_max_z = 60.0;
		fov_step_z = 1.0;

		ratio_x = 59.0;
		ratio_y = 59.0;
		ratio_z = 10.0;

		time_out = 600.0;
		scanning_rate = 20000.0;

		dims_x = (fov_max_x - fov_min_x)/fov_step_x;
		dims_y = (fov_max_y - fov_min_y)/fov_step_y;
		dims_z = (fov_max_z - fov_min_z)/fov_step_z;

		scanMethod = 0;
		b_useSimImg = 0;
		b_scan_z_reverse = 0;
	}
	else //then just copy from *pl
	{
		fov_min_x = pl->FOVX_MIN;
		fov_max_x = pl->FOVX_MAX;
		dims_x = pl->SX;
		if (dims_x<1) 
		{
			v3d_msg("The x dimension is illegal!");
			dims_x=1;
		}
		fov_step_x = pl->FOVX_STEP; //(fov_max_x-fov_min_x)/dims_x;

		fov_min_y = pl->FOVY_MIN;
		fov_max_y = pl->FOVY_MAX;
		dims_y = pl->SY;
		if (dims_y<1) 
		{
			v3d_msg("The y dimension is illegal!");
			dims_y=1;
		}
		fov_step_y = pl->FOVY_STEP; //(fov_max_y-fov_min_y)/dims_y;

		fov_min_z = pl->FOVZ_MIN;
		fov_max_z = pl->FOVZ_MAX;
		dims_z = pl->SZ;
		if (dims_z<1) 
		{
			v3d_msg("The z dimension is illegal!");
			dims_z=1;
		}
		fov_step_z = pl->FOVZ_STEP; //(fov_max_z-fov_min_z)/dims_z;

		ratio_x = pl->RATIO_X;
		ratio_y = pl->RATIO_Y;
		ratio_z = pl->RATIO_Z;

		time_out = pl->TIMEOUT;
		scanning_rate = pl->SCANNING_RATE;

		ch = pl->CHANNEL;
		marker_sorted = pl->MARKER_SORTED;

		scanMethod = pl->SCAN_METHOD;
		b_useSimImg = pl->USE_SIMIMG;
		b_scan_z_reverse = pl->SCAN_Z_REVERSE;
	}

	//create a dialog
	QVBoxLayout *mainLayout = new QVBoxLayout;
	QGroupBox *groupImg = new QGroupBox(tr("Image Configuration"));
	mainLayout->addWidget(groupImg);

	gridLayout = new QGridLayout();

	check_stepsize_or_dims = new QCheckBox();
	check_stepsize_or_dims->setText(QObject::tr("Setting step-sizes (ON) or image dimensions (OFF)."));
	check_stepsize_or_dims->setChecked(true);

	check_scan_z_reverse  = new QCheckBox();
	check_scan_z_reverse->setText(QObject::tr("Z Scanning Reverse"));
	check_scan_z_reverse->setChecked(b_scan_z_reverse);

	// image display
	label_image_size = new QLabel(QObject::tr("Image size:"));
	//label_image_size->setFrameStyle(QFrame::Panel | QFrame::Raised); //Sunken

	// total time display
	label_total_time_num = new QLabel(QObject::tr("Scanning time:"));
	//label_total_time_num->setFrameStyle(QFrame::Panel | QFrame::Raised); //Sunken

	gridLayout->addWidget(check_stepsize_or_dims, 3,0,1,1); gridLayout->addWidget(check_scan_z_reverse, 3,1,1,1);
	gridLayout->addWidget(label_image_size,  3,4,1,1,Qt::AlignRight); gridLayout->addWidget(label_total_time_num, 3,5,1,1,Qt::AlignRight);

	//x,y,z scanning range
	label_fov_min_x = new QLabel(QObject::tr("FOV: x_min(um) "));
	label_fov_max_x = new QLabel(QObject::tr("FOV: x_max(um) "));
	label_fov_min_y = new QLabel(QObject::tr("FOV: y_min(um) "));
	label_fov_max_y = new QLabel(QObject::tr("FOV: y_max(um) "));
	label_fov_min_z = new QLabel(QObject::tr("FOV: z_min(um) "));
	label_fov_max_z = new QLabel(QObject::tr("FOV: z_max(um) "));

	label_fov_step_x = new QLabel(QObject::tr("FOV: x_step(um) "));
	label_fov_step_y = new QLabel(QObject::tr("FOV: y_step(um) "));
	label_fov_step_z = new QLabel(QObject::tr("FOV: z_step(um) "));

	qsb_fov_min_x = new QDoubleSpinBox(); 
	qsb_fov_max_x = new QDoubleSpinBox();
	qsb_fov_min_y = new QDoubleSpinBox(); 
	qsb_fov_max_y = new QDoubleSpinBox();
	qsb_fov_min_z = new QDoubleSpinBox(); 
	qsb_fov_max_z = new QDoubleSpinBox();

	qsb_fov_step_x = new QDoubleSpinBox();
	qsb_fov_step_y = new QDoubleSpinBox();
	qsb_fov_step_z = new QDoubleSpinBox();

	qsb_fov_min_x->setMaximum(1000); qsb_fov_min_x->setMinimum(-1000); qsb_fov_min_x->setValue(fov_min_x); qsb_fov_min_x->setSingleStep(0.01);
	qsb_fov_max_x->setMaximum(1000); qsb_fov_max_x->setMinimum(-1000); qsb_fov_max_x->setValue(fov_max_x); qsb_fov_max_x->setSingleStep(0.01);
	qsb_fov_min_y->setMaximum(1000); qsb_fov_min_y->setMinimum(-1000); qsb_fov_min_y->setValue(fov_min_y); qsb_fov_min_y->setSingleStep(0.01);
	qsb_fov_max_y->setMaximum(1000); qsb_fov_max_y->setMinimum(-1000); qsb_fov_max_y->setValue(fov_max_y); qsb_fov_max_y->setSingleStep(0.01);
	qsb_fov_min_z->setMaximum(1000); qsb_fov_min_z->setMinimum(0); qsb_fov_min_z->setValue(fov_min_z); qsb_fov_min_z->setSingleStep(0.01);
	qsb_fov_max_z->setMaximum(1000); qsb_fov_max_z->setMinimum(0); qsb_fov_max_z->setValue(fov_max_z); qsb_fov_max_z->setSingleStep(0.01);

	qsb_fov_step_x->setDecimals(2);
	qsb_fov_step_y->setDecimals(2);
	qsb_fov_step_z->setDecimals(2);

	qsb_fov_step_x->setMaximum(1000); qsb_fov_step_x->setMinimum(0.01); qsb_fov_step_x->setValue(fov_step_x); qsb_fov_step_x->setSingleStep(0.01);
	qsb_fov_step_y->setMaximum(1000); qsb_fov_step_y->setMinimum(0.01); qsb_fov_step_y->setValue(fov_step_y); qsb_fov_step_y->setSingleStep(0.01);
	qsb_fov_step_z->setMaximum(1000); qsb_fov_step_z->setMinimum(0.01); qsb_fov_step_z->setValue(fov_step_z); qsb_fov_step_z->setSingleStep(0.01);

	gridLayout->addWidget(label_fov_min_x, 4,0,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_fov_min_x, 4,1,1,1,Qt::AlignRight); 
	gridLayout->addWidget(label_fov_max_x, 4,2,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_fov_max_x, 4,3,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_fov_step_x,4,4,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_fov_step_x,4,5,1,1,Qt::AlignRight);

	gridLayout->addWidget(label_fov_min_y, 5,0,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_fov_min_y, 5,1,1,1,Qt::AlignRight); 
	gridLayout->addWidget(label_fov_max_y, 5,2,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_fov_max_y, 5,3,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_fov_step_y,5,4,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_fov_step_y,5,5,1,1,Qt::AlignRight);

	gridLayout->addWidget(label_fov_min_z, 6,0,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_fov_min_z, 6,1,1,1,Qt::AlignRight); 
	gridLayout->addWidget(label_fov_max_z, 6,2,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_fov_max_z, 6,3,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_fov_step_z,6,4,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_fov_step_z,6,5,1,1,Qt::AlignRight);

	//always disable the step-size setting. Can only adjust them using the dimensions
	qsb_fov_step_x->setDisabled(true);
	qsb_fov_step_y->setDisabled(true);
	qsb_fov_step_z->setDisabled(true);

	//x,y,z dimensions
	label_dim_x = new QLabel(QObject::tr("Image Dimensions X: "));
	label_dim_y = new QLabel(QObject::tr(" Y: "));
	label_dim_z = new QLabel(QObject::tr(" Z: "));

	qsb_dim_x = new QSpinBox();
	qsb_dim_y = new QSpinBox();
	qsb_dim_z = new QSpinBox();

	qsb_dim_x->setMaximum(10000); qsb_dim_x->setMinimum(1); qsb_dim_x->setValue(dims_x); qsb_dim_x->setSingleStep(1);
	qsb_dim_y->setMaximum(10000); qsb_dim_y->setMinimum(1); qsb_dim_y->setValue(dims_y); qsb_dim_y->setSingleStep(1);
	qsb_dim_z->setMaximum(10000); qsb_dim_z->setMinimum(1); qsb_dim_z->setValue(dims_z); qsb_dim_z->setSingleStep(1);

	gridLayout->addWidget(label_dim_x, 8,0,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_dim_x, 8,1,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_dim_y, 8,2,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_dim_y, 8,3,1,1,Qt::AlignRight);
	gridLayout->addWidget(label_dim_z, 8,4,1,1,Qt::AlignRight); gridLayout->addWidget(qsb_dim_z, 8,5,1,1,Qt::AlignRight);

	// groupBox
	groupImg->setLayout(gridLayout);

	// groupbox for laser paras
	QGroupBox *groupLaser = new QGroupBox(tr("Run Configuration"));
	QGridLayout *gridLayoutLaser = new QGridLayout();
	// add to parent grid
	mainLayout->addWidget(groupLaser);

	// ratio for x,y,z scanning
	label_ratio_x = new QLabel(QObject::tr("Ratio(um/V) X: "));
	label_ratio_y = new QLabel(QObject::tr(" Y: "));
	label_ratio_z = new QLabel(QObject::tr(" Z: "));

	qsb_ratio_x = new QDoubleSpinBox();
	qsb_ratio_y = new QDoubleSpinBox();
	qsb_ratio_z = new QDoubleSpinBox();

	qsb_ratio_x->setMaximum(1000); qsb_ratio_x->setMinimum(0.01); qsb_ratio_x->setValue(ratio_x); qsb_ratio_x->setSingleStep(0.01);
	qsb_ratio_y->setMaximum(1000); qsb_ratio_y->setMinimum(0.01); qsb_ratio_y->setValue(ratio_y); qsb_ratio_y->setSingleStep(0.01);
	qsb_ratio_z->setMaximum(1000); qsb_ratio_z->setMinimum(0.01); qsb_ratio_z->setValue(ratio_z); qsb_ratio_z->setSingleStep(0.01);

	gridLayoutLaser->addWidget(label_ratio_x, 10,0,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(qsb_ratio_x, 10,1,1,1,Qt::AlignRight);
	gridLayoutLaser->addWidget(label_ratio_y, 10,2,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(qsb_ratio_y, 10,3,1,1,Qt::AlignRight);
	gridLayoutLaser->addWidget(label_ratio_z, 10,4,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(qsb_ratio_z, 10,5,1,1,Qt::AlignRight);

	// time out
	label_timeout = new QLabel(QObject::tr("Time Out (s): "));
	qsb_timeout = new QDoubleSpinBox();
	qsb_timeout->setMaximum(100000000); qsb_timeout->setMinimum(0.01); qsb_timeout->setValue(time_out); qsb_timeout->setSingleStep(0.01);

	// scan method
	label_scan_method = new QLabel(QObject::tr("Scanning Method: ")); // ZJL
	QStringList scanmd;
	scanmd << "single_acc"; scanmd << "zigzag_acc"; 
	combo_scan_method = new QComboBox(); combo_scan_method->addItems(scanmd); combo_scan_method->setCurrentIndex(scanMethod);
	
	// use sim img
	label_use_sim_img = new QLabel(QObject::tr("Simulation image: "));
	check_use_sim_img = new QCheckBox();
	check_use_sim_img->setText(QObject::tr("           Use"));
	check_use_sim_img->setLayoutDirection(Qt::RightToLeft);
	check_use_sim_img->setChecked(b_useSimImg);

	gridLayoutLaser->addWidget(label_timeout,     12,0,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(qsb_timeout,       12,1,1,1,Qt::AlignRight);
	gridLayoutLaser->addWidget(label_use_sim_img, 12,2,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(check_use_sim_img, 12,3,1,1,Qt::AlignRight);
	
	// scanning rate
	label_scanrate = new QLabel(QObject::tr("Scanning Rate (Hz): "));
	qsb_scanrate = new QDoubleSpinBox();
	qsb_scanrate->setMaximum(100000000); qsb_scanrate->setMinimum(0.01); qsb_scanrate->setValue(scanning_rate); qsb_scanrate->setSingleStep(0.01);
	gridLayoutLaser->addWidget(label_scanrate, 14,0,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(qsb_scanrate, 14,1,1,1,Qt::AlignRight);
	gridLayoutLaser->addWidget(label_scan_method, 14,2,1,1,Qt::AlignRight); gridLayoutLaser->addWidget(combo_scan_method, 14,3,1,1,Qt::AlignRight);

	// red, green channel, or both color imaging
	QStringList chs;
	chs << "green"; chs << "red"; chs << "both";
	combo_channel =  new QComboBox(); combo_channel->addItems(chs); combo_channel->setCurrentIndex(ch);

	label_ch = new QLabel(QObject::tr("Choose laser (r561nm/g488nm): "));

	paste        = new QPushButton("Reuse Config Info");
	preset_paras = new QPushButton("Choose Preset Z range and X,Y,Z Resolution");
	createMenuOfPresetParas();

	gridLayoutLaser->addWidget(label_ch, 16,0,Qt::AlignRight); gridLayoutLaser->addWidget(combo_channel, 16,1); 
	gridLayoutLaser->addWidget(preset_paras, 16,3);            gridLayoutLaser->addWidget(paste, 16,5);

	// groupBox
	groupLaser->setLayout(gridLayoutLaser);

	// button
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                      | QDialogButtonBox::Cancel);
	// add to parent grid
	mainLayout->addWidget(buttonBox);


	// state update
	if(check_stepsize_or_dims->isChecked())
	{
		qsb_fov_step_x->setEnabled(true);
		qsb_fov_step_y->setEnabled(true);
		qsb_fov_step_z->setEnabled(true);

		qsb_dim_x->setDisabled(true);
		qsb_dim_y->setDisabled(true);
		qsb_dim_z->setDisabled(true);
	}
	else
	{
		qsb_dim_x->setEnabled(true);
		qsb_dim_y->setEnabled(true);
		qsb_dim_z->setEnabled(true);

		qsb_fov_step_x->setDisabled(true);
		qsb_fov_step_y->setDisabled(true);
		qsb_fov_step_z->setDisabled(true);
	}

	// update time label
	double total_time = (double)(dims_x*dims_y*dims_z/(double)scanning_rate);
	QString str_time="Scanning time: [" + QString("<font color='red'> %1 </font>").arg(total_time,0,'f',2)+"]s";
	label_total_time_num->setText(str_time);

	// update image size display
	int num_ch;
	ch = combo_channel->currentIndex();
	if(ch==0 || ch==1) num_ch=1;
	else num_ch=2;
	double imgSize = dims_x*dims_y*dims_z * num_ch * sizeof(float)/1024.0/1024.0;
	QString str_imgSize="Image size: [" + QString("<font color='red'> %1 </font>").arg(imgSize,0,'f',2)+"]MB";
	label_image_size->setText(str_imgSize);

	//gridlayout
	setLayout(mainLayout);
	setWindowTitle(QString("ROI Imaging Configuration"));

	//slot interface
	connect(check_stepsize_or_dims, SIGNAL(stateChanged(int)), this, SLOT(update()));

	connect(qsb_fov_min_x, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_fov_max_x, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_fov_min_y, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_fov_max_y, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_fov_min_z, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_fov_max_z, SIGNAL(valueChanged(double)), this, SLOT(update()));

	connect(qsb_fov_step_x, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_fov_step_y, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_fov_step_z, SIGNAL(valueChanged(double)), this, SLOT(update()));

	connect(qsb_ratio_x, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_ratio_y, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_ratio_z, SIGNAL(valueChanged(double)), this, SLOT(update()));

	connect(qsb_timeout, SIGNAL(valueChanged(double)), this, SLOT(update()));
	connect(qsb_scanrate, SIGNAL(valueChanged(double)), this, SLOT(update()));

	connect(qsb_dim_x, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_dim_y, SIGNAL(valueChanged(int)), this, SLOT(update()));
	connect(qsb_dim_z, SIGNAL(valueChanged(int)), this, SLOT(update()));

	// for scan method and use_sim_img
	connect(combo_scan_method, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
	connect(check_use_sim_img, SIGNAL(stateChanged(int)), this, SLOT(update()));
	//
	connect(combo_channel, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(paste, SIGNAL(clicked()), this, SLOT(pasteConfigInfo()));
	connect(preset_paras, SIGNAL(clicked()), this, SLOT(doMenuOfPrestParas()));
}


LSM_ROI_Setup_Dialog::~LSM_ROI_Setup_Dialog(){}

void LSM_ROI_Setup_Dialog::doMenuOfPrestParas()
{
	try
	{
		menu_preset_paras.exec(QCursor::pos());
	}
	catch (...)
	{
		v3d_msg("Fail to run the XFormWidget::doMenuOfTriviewZoom() function.\n", 0);
	}
}

void LSM_ROI_Setup_Dialog::createMenuOfPresetParas()
{
    QAction* Act;

	Act = new QAction(tr("One slice at Z:30-30.25, Resolution: 0.25/0.25/0.25"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(preset_paras_0()));
    menu_preset_paras.addAction(Act);

	Act = new QAction(tr("Z:10-50, Resolution: 0.25/0.25/0.25"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(preset_paras_1()));
    menu_preset_paras.addAction(Act);

    Act = new QAction(tr("Z:10-50, Resolution: 0.5/0.5/0.5"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(preset_paras_2()));
    menu_preset_paras.addAction(Act);

    Act = new QAction(tr("Z:10-50, Resolution: 0.25/0.25/0.5"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(preset_paras_3()));
    menu_preset_paras.addAction(Act);

    Act = new QAction(tr("Z:20-40, Resolution: 0.25/0.25/0.25"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(preset_paras_4()));
    menu_preset_paras.addAction(Act);

	Act = new QAction(tr("Z:20-40, Resolution: 0.5/0.5/0.5"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(preset_paras_5()));
    menu_preset_paras.addAction(Act);

	Act = new QAction(tr("Use the setting of the last scan"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(preset_paras_6()));
    menu_preset_paras.addAction(Act);
}

//Z:30-30.25, Resolution: 0.25/0.25/0.25
void LSM_ROI_Setup_Dialog::preset_paras_0()
{
	b_resetInProgress=true;
	
	qsb_fov_min_z->setValue(30.0); 
	qsb_fov_max_z->setValue(30.25);
	
	qsb_fov_step_x->setValue(0.25);
	qsb_fov_step_y->setValue(0.25);
	qsb_fov_step_z->setValue(0.25);

	fov_min_x = qsb_fov_min_x->text().toDouble();
	fov_max_x = qsb_fov_max_x->text().toDouble();
	fov_step_x = 0.25;
	double fov_length_x = fov_max_x - fov_min_x;
	dims_x = long( fov_length_x / fov_step_x);
	qsb_dim_x->setValue(dims_x);

	fov_min_y = qsb_fov_min_y->text().toDouble();
	fov_max_y = qsb_fov_max_y->text().toDouble();
	fov_step_y = 0.25;
	double fov_length_y = fov_max_y - fov_min_y;
	dims_y = long( fov_length_y / fov_step_y);
	qsb_dim_y->setValue(dims_y);

	fov_min_z = 30.0;
	fov_max_z = 30.25;
	fov_step_z = 0.25;
	double fov_length_z = fov_max_z - fov_min_z;
	dims_z = long( fov_length_z / fov_step_z);
	qsb_dim_z->setValue(dims_z);

	// update time label
	double total_time = (double)(dims_x*dims_y*dims_z/(double)scanning_rate);
	QString str_time="Scanning time: [" + QString("<font color='red'> %1 </font>").arg(total_time,0,'f',2)+"]s";
	label_total_time_num->setText(str_time);

	// update image size display
	scanning_rate = qsb_scanrate->text().toDouble();
	int num_ch;
	ch = combo_channel->currentIndex();
	if(ch==0 || ch==1) num_ch=1;
	else num_ch=2;
	double imgSize = dims_x*dims_y*dims_z * num_ch * sizeof(float)/1024.0/1024.0;
	QString str_imgSize="Image size: [" + QString("<font color='red'> %1 </font>").arg(imgSize,0,'f',2)+"]MB";
	label_image_size->setText(str_imgSize);

	b_resetInProgress=false;
}

//Z:10-50, Resolution: 0.25/0.25/0.25
void LSM_ROI_Setup_Dialog::preset_paras_1()
{
	b_resetInProgress=true;
	
	qsb_fov_min_z->setValue(10.0); 
	qsb_fov_max_z->setValue(50.0);
	
	qsb_fov_step_x->setValue(0.25);
	qsb_fov_step_y->setValue(0.25);
	qsb_fov_step_z->setValue(0.25);

	fov_min_x = qsb_fov_min_x->text().toDouble();
	fov_max_x = qsb_fov_max_x->text().toDouble();
	fov_step_x = 0.25;
	double fov_length_x = fov_max_x - fov_min_x;
	dims_x = long( fov_length_x / fov_step_x);
	qsb_dim_x->setValue(dims_x);

	fov_min_y = qsb_fov_min_y->text().toDouble();
	fov_max_y = qsb_fov_max_y->text().toDouble();
	fov_step_y = 0.25;
	double fov_length_y = fov_max_y - fov_min_y;
	dims_y = long( fov_length_y / fov_step_y);
	qsb_dim_y->setValue(dims_y);

	fov_min_z = 10.0;
	fov_max_z = 50.0;
	fov_step_z = 0.25;
	double fov_length_z = fov_max_z - fov_min_z;
	dims_z = long( fov_length_z / fov_step_z);
	qsb_dim_z->setValue(dims_z);

	// update time label
	double total_time = (double)(dims_x*dims_y*dims_z/(double)scanning_rate);
	QString str_time="Scanning time: [" + QString("<font color='red'> %1 </font>").arg(total_time,0,'f',2)+"]s";
	label_total_time_num->setText(str_time);

	// update image size display
	scanning_rate = qsb_scanrate->text().toDouble();
	int num_ch;
	ch = combo_channel->currentIndex();
	if(ch==0 || ch==1) num_ch=1;
	else num_ch=2;
	double imgSize = dims_x*dims_y*dims_z * num_ch * sizeof(float)/1024.0/1024.0;
	QString str_imgSize="Image size: [" + QString("<font color='red'> %1 </font>").arg(imgSize,0,'f',2)+"]MB";
	label_image_size->setText(str_imgSize);

	b_resetInProgress=false;
}

//Z:10-50, Resolution: 0.5/0.5/0.5
void LSM_ROI_Setup_Dialog::preset_paras_2()
{
	b_resetInProgress=true;
	
	qsb_fov_min_z->setValue(10.0); 
	qsb_fov_max_z->setValue(50.0);
	
	qsb_fov_step_x->setValue(0.5);
	qsb_fov_step_y->setValue(0.5);
	qsb_fov_step_z->setValue(0.5);

	fov_min_x = qsb_fov_min_x->text().toDouble();
	fov_max_x = qsb_fov_max_x->text().toDouble();
	fov_step_x = 0.5;
	double fov_length_x = fov_max_x - fov_min_x;
	dims_x = long( fov_length_x / fov_step_x);
	qsb_dim_x->setValue(dims_x);

	fov_min_y = qsb_fov_min_y->text().toDouble();
	fov_max_y = qsb_fov_max_y->text().toDouble();
	fov_step_y = 0.5;
	double fov_length_y = fov_max_y - fov_min_y;
	dims_y = long( fov_length_y / fov_step_y);
	qsb_dim_y->setValue(dims_y);

	fov_min_z = 10.0;
	fov_max_z = 50.0;
	fov_step_z = 0.5;
	double fov_length_z = fov_max_z - fov_min_z;
	dims_z = long( fov_length_z / fov_step_z);
	qsb_dim_z->setValue(dims_z);

	// update time label
	double total_time = (double)(dims_x*dims_y*dims_z/(double)scanning_rate);
	QString str_time="Scanning time: [" + QString("<font color='red'> %1 </font>").arg(total_time,0,'f',2)+"]s";
	label_total_time_num->setText(str_time);

	// update image size display
	scanning_rate = qsb_scanrate->text().toDouble();
	int num_ch;
	ch = combo_channel->currentIndex();
	if(ch==0 || ch==1) num_ch=1;
	else num_ch=2;
	double imgSize = dims_x*dims_y*dims_z * num_ch * sizeof(float)/1024.0/1024.0;
	QString str_imgSize="Image size: [" + QString("<font color='red'> %1 </font>").arg(imgSize,0,'f',2)+"]MB";
	label_image_size->setText(str_imgSize);

	b_resetInProgress=false;
}

//Z:10-50, Resolution: 0.25/0.25/0.5
void LSM_ROI_Setup_Dialog::preset_paras_3()
{
	b_resetInProgress=true;

	qsb_fov_min_z->setValue(10.0); 
	qsb_fov_max_z->setValue(50.0);
	
	qsb_fov_step_x->setValue(0.25);
	qsb_fov_step_y->setValue(0.25);
	qsb_fov_step_z->setValue(0.5);

	fov_min_x = qsb_fov_min_x->text().toDouble();
	fov_max_x = qsb_fov_max_x->text().toDouble();
	fov_step_x = 0.25;
	double fov_length_x = fov_max_x - fov_min_x;
	dims_x = long( fov_length_x / fov_step_x);
	qsb_dim_x->setValue(dims_x);

	fov_min_y = qsb_fov_min_y->text().toDouble();
	fov_max_y = qsb_fov_max_y->text().toDouble();
	fov_step_y = 0.25;
	double fov_length_y = fov_max_y - fov_min_y;
	dims_y = long( fov_length_y / fov_step_y);
	qsb_dim_y->setValue(dims_y);

	fov_min_z = 10.0;
	fov_max_z = 50.0;
	fov_step_z = 0.5;
	double fov_length_z = fov_max_z - fov_min_z;
	dims_z = long( fov_length_z / fov_step_z);
	qsb_dim_z->setValue(dims_z);

	// update time label
	double total_time = (double)(dims_x*dims_y*dims_z/(double)scanning_rate);
	QString str_time="Scanning time: [" + QString("<font color='red'> %1 </font>").arg(total_time,0,'f',2)+"]s";
	label_total_time_num->setText(str_time);

	// update image size display
	scanning_rate = qsb_scanrate->text().toDouble();
	int num_ch;
	ch = combo_channel->currentIndex();
	if(ch==0 || ch==1) num_ch=1;
	else num_ch=2;
	double imgSize = dims_x*dims_y*dims_z * num_ch * sizeof(float)/1024.0/1024.0;
	QString str_imgSize="Image size: [" + QString("<font color='red'> %1 </font>").arg(imgSize,0,'f',2)+"]MB";
	label_image_size->setText(str_imgSize);

	b_resetInProgress=false;
}

//Z:Z:20-40, Resolution: 0.25/0.25/0.25
void LSM_ROI_Setup_Dialog::preset_paras_4()
{
	b_resetInProgress=true;

	qsb_fov_min_z->setValue(20.0); 
	qsb_fov_max_z->setValue(40.0);
	
	qsb_fov_step_x->setValue(0.25);
	qsb_fov_step_y->setValue(0.25);
	qsb_fov_step_z->setValue(0.25);

	fov_min_x = qsb_fov_min_x->text().toDouble();
	fov_max_x = qsb_fov_max_x->text().toDouble();
	fov_step_x = 0.25;
	double fov_length_x = fov_max_x - fov_min_x;
	dims_x = long( fov_length_x / fov_step_x);
	qsb_dim_x->setValue(dims_x);

	fov_min_y = qsb_fov_min_y->text().toDouble();
	fov_max_y = qsb_fov_max_y->text().toDouble();
	fov_step_y = 0.25;
	double fov_length_y = fov_max_y - fov_min_y;
	dims_y = long( fov_length_y / fov_step_y);
	qsb_dim_y->setValue(dims_y);

	fov_min_z = 20.0;
	fov_max_z = 40.0;
	fov_step_z = 0.25;
	double fov_length_z = fov_max_z - fov_min_z;
	dims_z = long( fov_length_z / fov_step_z);
	qsb_dim_z->setValue(dims_z);

	// update time label
	double total_time = (double)(dims_x*dims_y*dims_z/(double)scanning_rate);
	QString str_time="Scanning time: [" + QString("<font color='red'> %1 </font>").arg(total_time,0,'f',2)+"]s";
	label_total_time_num->setText(str_time);

	// update image size display
	scanning_rate = qsb_scanrate->text().toDouble();
	int num_ch;
	ch = combo_channel->currentIndex();
	if(ch==0 || ch==1) num_ch=1;
	else num_ch=2;
	double imgSize = dims_x*dims_y*dims_z * num_ch * sizeof(float)/1024.0/1024.0;
	QString str_imgSize="Image size: [" + QString("<font color='red'> %1 </font>").arg(imgSize,0,'f',2)+"]MB";
	label_image_size->setText(str_imgSize);

	b_resetInProgress=false;
}

//Z:Z:20-40, Resolution: 0.5/0.5/0.5
void LSM_ROI_Setup_Dialog::preset_paras_5()
{
	b_resetInProgress=true;

	qsb_fov_min_z->setValue(20.0); 
	qsb_fov_max_z->setValue(40.0);
	
	qsb_fov_step_x->setValue(0.5);
	qsb_fov_step_y->setValue(0.5);
	qsb_fov_step_z->setValue(0.5);

	fov_min_x = qsb_fov_min_x->text().toDouble();
	fov_max_x = qsb_fov_max_x->text().toDouble();
	fov_step_x = 0.5;
	double fov_length_x = fov_max_x - fov_min_x;
	dims_x = long( fov_length_x / fov_step_x);
	qsb_dim_x->setValue(dims_x);

	fov_min_y = qsb_fov_min_y->text().toDouble();
	fov_max_y = qsb_fov_max_y->text().toDouble();
	fov_step_y = 0.5;
	double fov_length_y = fov_max_y - fov_min_y;
	dims_y = long( fov_length_y / fov_step_y);
	qsb_dim_y->setValue(dims_y);

	fov_min_z = 20.0;
	fov_max_z = 40.0;
	fov_step_z = 0.5;
	double fov_length_z = fov_max_z - fov_min_z;
	dims_z = long( fov_length_z / fov_step_z);
	qsb_dim_z->setValue(dims_z);

	// update time label
	double total_time = (double)(dims_x*dims_y*dims_z/(double)scanning_rate);
	QString str_time="Scanning time: [" + QString("<font color='red'> %1 </font>").arg(total_time,0,'f',2)+"]s";
	label_total_time_num->setText(str_time);

	// update image size display
	scanning_rate = qsb_scanrate->text().toDouble();
	int num_ch;
	ch = combo_channel->currentIndex();
	if(ch==0 || ch==1) num_ch=1;
	else num_ch=2;
	double imgSize = dims_x*dims_y*dims_z * num_ch * sizeof(float)/1024.0/1024.0;
	QString str_imgSize="Image size: [" + QString("<font color='red'> %1 </font>").arg(imgSize,0,'f',2)+"]MB";
	label_image_size->setText(str_imgSize);

	b_resetInProgress=false;
}


// use the setting of the last scan
void LSM_ROI_Setup_Dialog::preset_paras_6()
{
	b_resetInProgress=true;

	// open the last scan parameter file
	// get configure filename and load configuration info
	QString filename = get_last_img_name("LSM", ".raw");
	if(filename==QString(""))
		return;

	filename.chop(4); // chop ".raw"
	filename.append(".txt");

	Parameters_LSM pl;
	pl.load_ParameterFile(filename);
	qDebug()<<"logfile name:"<<filename;

	fov_min_x = pl.FOVX_MIN;	fov_max_x = pl.FOVX_MAX;	dims_x = pl.SX;	fov_step_x = pl.FOVX_STEP; 
	fov_min_y = pl.FOVY_MIN;	fov_max_y = pl.FOVY_MAX;	dims_y = pl.SY;	fov_step_y = pl.FOVY_STEP; 
	fov_min_z = pl.FOVZ_MIN;	fov_max_z = pl.FOVZ_MAX;	dims_z = pl.SZ;	fov_step_z = pl.FOVZ_STEP; 
	ratio_x = pl.RATIO_X; ratio_y = pl.RATIO_Y;	ratio_z = pl.RATIO_Z; time_out = pl.TIMEOUT;
	scanning_rate = pl.SCANNING_RATE; ch = pl.CHANNEL; marker_sorted = pl.MARKER_SORTED;
	scanMethod = pl.SCAN_METHOD; b_useSimImg = pl.USE_SIMIMG; 
	b_scan_z_reverse=pl.SCAN_Z_REVERSE;

	qsb_fov_min_x->setValue(fov_min_x); 
	qsb_fov_max_x->setValue(fov_max_x);
	qsb_fov_step_x->setValue(fov_step_x);
	
	qsb_fov_min_y->setValue(fov_min_y); 
	qsb_fov_max_y->setValue(fov_max_y); 
	qsb_fov_step_y->setValue(fov_step_y);

	qsb_fov_min_z->setValue(fov_min_z); 
	qsb_fov_max_z->setValue(fov_max_z);
	qsb_fov_step_z->setValue(fov_step_z);

	qsb_dim_x->setValue(dims_x);
	qsb_dim_y->setValue(dims_y);
	qsb_dim_z->setValue(dims_z);

	qsb_ratio_x->setValue(ratio_x);
	qsb_ratio_y->setValue(ratio_y);
	qsb_ratio_z->setValue(ratio_z);

	qsb_scanrate->setValue(scanning_rate);
	combo_channel->setCurrentIndex(ch);
	combo_scan_method->setCurrentIndex(scanMethod);
	check_use_sim_img->setChecked(b_useSimImg);
	check_scan_z_reverse->setChecked(b_scan_z_reverse);

	// update time label
	double total_time = (double)(dims_x*dims_y*dims_z/(double)scanning_rate);
	QString str_time="Scanning time: [" + QString("<font color='red'> %1 </font>").arg(total_time,0,'f',2)+"]s";
	label_total_time_num->setText(str_time);

	// update image size display
	int num_ch;
	ch = combo_channel->currentIndex();
	if(ch==0 || ch==1) num_ch=1;
	else num_ch=2;
	double imgSize = dims_x*dims_y*dims_z * num_ch * sizeof(float)/1024.0/1024.0;
	QString str_imgSize="Image size: [" + QString("<font color='red'> %1 </font>").arg(imgSize,0,'f',2)+"]MB";
	label_image_size->setText(str_imgSize);

	b_resetInProgress=false;
}


void LSM_ROI_Setup_Dialog::pasteConfigInfo()
{
	b_resetInProgress=true;

	// get paras from $ssConfigFolder/mi_configuration.txt
	QString lsm_FileName = ssConfigFolder + "mi_configuration_clipboard.txt";

	Parameters_LSM pl;
	pl.load_ParameterFile(lsm_FileName);

	fov_min_x = pl.FOVX_MIN;	fov_max_x = pl.FOVX_MAX;	dims_x = pl.SX;	fov_step_x = pl.FOVX_STEP; 
	fov_min_y = pl.FOVY_MIN;	fov_max_y = pl.FOVY_MAX;	dims_y = pl.SY;	fov_step_y = pl.FOVY_STEP; 
	fov_min_z = pl.FOVZ_MIN;	fov_max_z = pl.FOVZ_MAX;	dims_z = pl.SZ;	fov_step_z = pl.FOVZ_STEP; 
	ratio_x = pl.RATIO_X; ratio_y = pl.RATIO_Y;	ratio_z = pl.RATIO_Z; time_out = pl.TIMEOUT;
	scanning_rate = pl.SCANNING_RATE; ch = pl.CHANNEL; marker_sorted = pl.MARKER_SORTED;
	scanMethod = pl.SCAN_METHOD; b_useSimImg = pl.USE_SIMIMG; 
	b_scan_z_reverse=pl.SCAN_Z_REVERSE;

	qsb_fov_min_x->setValue(fov_min_x); 
	qsb_fov_max_x->setValue(fov_max_x);
	qsb_fov_step_x->setValue(fov_step_x);
	
	qsb_fov_min_y->setValue(fov_min_y); 
	qsb_fov_max_y->setValue(fov_max_y); 
	qsb_fov_step_y->setValue(fov_step_y);

	qsb_fov_min_z->setValue(fov_min_z); 
	qsb_fov_max_z->setValue(fov_max_z);
	qsb_fov_step_z->setValue(fov_step_z);

	qsb_dim_x->setValue(dims_x);
	qsb_dim_y->setValue(dims_y);
	qsb_dim_z->setValue(dims_z);

	qsb_ratio_x->setValue(ratio_x);
	qsb_ratio_y->setValue(ratio_y);
	qsb_ratio_z->setValue(ratio_z);

	qsb_scanrate->setValue(scanning_rate);
	combo_channel->setCurrentIndex(ch);
	combo_scan_method->setCurrentIndex(scanMethod);
	check_use_sim_img->setChecked(b_useSimImg);
	check_scan_z_reverse->setChecked(b_scan_z_reverse);

	// update time label
	double total_time = (double)(dims_x*dims_y*dims_z/(double)scanning_rate);
	QString str_time="Scanning time: [" + QString("<font color='red'> %1 </font>").arg(total_time,0,'f',2)+"]s";
	label_total_time_num->setText(str_time);

	// update image size display
	int num_ch;
	ch = combo_channel->currentIndex();
	if(ch==0 || ch==1) num_ch=1;
	else num_ch=2;
	double imgSize = dims_x*dims_y*dims_z * num_ch * sizeof(float)/1024.0/1024.0;
	QString str_imgSize="Image size: [" + QString("<font color='red'> %1 </font>").arg(imgSize,0,'f',2)+"]MB";
	label_image_size->setText(str_imgSize);

	b_resetInProgress=false;
}



void LSM_ROI_Setup_Dialog::update()
{
	if(b_resetInProgress==false)
	{
		ratio_x = qsb_ratio_x->text().toDouble();
		ratio_y = qsb_ratio_y->text().toDouble();
		ratio_z = qsb_ratio_z->text().toDouble();

		time_out = qsb_timeout->text().toDouble();
		scanning_rate = qsb_scanrate->text().toDouble();

		ch = combo_channel->currentIndex();

		scanMethod = combo_scan_method->currentIndex();

		if(check_use_sim_img->isChecked())
			b_useSimImg = 1;
		else
			b_useSimImg = 0;

		// update scan_z_reverse
		if(check_scan_z_reverse->isChecked())
			b_scan_z_reverse = 1;
		else
			b_scan_z_reverse = 0;

		if(check_stepsize_or_dims->isChecked())
		{
			qsb_fov_step_x->setEnabled(true);
			qsb_fov_step_y->setEnabled(true);
			qsb_fov_step_z->setEnabled(true);

			qsb_dim_x->setDisabled(true);
			qsb_dim_y->setDisabled(true);
			qsb_dim_z->setDisabled(true);

			// fov, step -> dims
			update_dims_x();
			update_dims_y();
			update_dims_z();

		}
		else
		{
			qsb_dim_x->setEnabled(true);
			qsb_dim_y->setEnabled(true);
			qsb_dim_z->setEnabled(true);

			qsb_fov_step_x->setDisabled(true);
			qsb_fov_step_y->setDisabled(true);
			qsb_fov_step_z->setDisabled(true);

			// fov, dims -> step
			update_step_x();
			update_step_y();
			update_step_z();
		}

		// update time label
		double total_time = (double)(dims_x*dims_y*dims_z/(double)scanning_rate);
		QString str_time="Scanning time: [" + QString("<font color='red'> %1 </font>").arg(total_time,0,'f',2)+"]s";
		label_total_time_num->setText(str_time);

		// update image size display
		int num_ch;
		ch = combo_channel->currentIndex();
		if(ch==0 || ch==1) num_ch=1;
		else num_ch=2;
		double imgSize = dims_x*dims_y*dims_z * num_ch * sizeof(float)/1024.0/1024.0;
		QString str_imgSize="Image size: [" + QString("<font color='red'> %1 </font>").arg(imgSize,0,'f',2)+"]MB";
		label_image_size->setText(str_imgSize);
	}
}


void LSM_ROI_Setup_Dialog::update_step_x()
{
	fov_min_x = qsb_fov_min_x->text().toDouble();
	fov_max_x = qsb_fov_max_x->text().toDouble();

	dims_x = qsb_dim_x->text().toInt();

	if(dims_x<0)
	{
		v3d_msg("The dims_x parameter is illegal in update_step_x().");
		return;
	}

	fov_step_x = (fov_max_x - fov_min_x)/double(dims_x);

	qsb_fov_step_x->setValue(fov_step_x);
}

void LSM_ROI_Setup_Dialog::update_step_y()
{
	fov_min_y = qsb_fov_min_y->text().toDouble();
	fov_max_y = qsb_fov_max_y->text().toDouble();

	dims_y = qsb_dim_y->text().toInt();

	if(dims_y<0)
	{
		v3d_msg("The dims_y parameter is illegal in update_step_y().");
		return;
	}

	fov_step_y = (fov_max_y - fov_min_y)/double(dims_y);

	qsb_fov_step_y->setValue(fov_step_y);
}

void LSM_ROI_Setup_Dialog::update_step_z()
{
	fov_min_z = qsb_fov_min_z->text().toDouble();
	fov_max_z = qsb_fov_max_z->text().toDouble();

	dims_z = qsb_dim_z->text().toInt();

	if(dims_z<0)
	{
		v3d_msg("The dims_z parameter is illegal in update_step_z().");
		return;
	}

	fov_step_z = (fov_max_z - fov_min_z)/double(dims_z);

	qsb_fov_step_z->setValue(fov_step_z);
}

void LSM_ROI_Setup_Dialog::update_dims_x()
{
	fov_min_x = qsb_fov_min_x->text().toDouble();
	fov_max_x = qsb_fov_max_x->text().toDouble();

	fov_step_x = qsb_fov_step_x->text().toDouble();

	double fov_length = fov_max_x - fov_min_x;
	dims_x = long( fov_length / fov_step_x);
	qsb_dim_x->setValue(dims_x);
}

void LSM_ROI_Setup_Dialog::update_dims_y()
{
	fov_min_y = qsb_fov_min_y->text().toDouble();
	fov_max_y = qsb_fov_max_y->text().toDouble();

	fov_step_y = qsb_fov_step_y->text().toDouble();

	double fov_length = fov_max_y - fov_min_y;
	dims_y = long( fov_length / fov_step_y);
	qsb_dim_y->setValue(dims_y);
}

void LSM_ROI_Setup_Dialog::update_dims_z()
{
	fov_min_z = qsb_fov_min_z->text().toDouble();
	fov_max_z = qsb_fov_max_z->text().toDouble();

	fov_step_z = qsb_fov_step_z->text().toDouble();

	double fov_length = fov_max_z - fov_min_z;
	dims_z = long(fov_length / fov_step_z);
	qsb_dim_z->setValue(dims_z);
}

void LSM_ROI_Setup_Dialog::copyDataOut(Parameters_LSM *pl)
{
	if (!pl)
		return;

	pl->FOVX_MIN = fov_min_x;
	pl->FOVX_MAX = fov_max_x;
	pl->FOVX_STEP = fov_step_x;

	pl->FOVY_MIN = fov_min_y;
	pl->FOVY_MAX = fov_max_y;
	pl->FOVY_STEP = fov_step_y;

	pl->FOVZ_MIN = fov_min_z;
	pl->FOVZ_MAX = fov_max_z;
	pl->FOVZ_STEP = fov_step_z;

	pl->RATIO_X = ratio_x;
	pl->RATIO_Y = ratio_y;
	pl->RATIO_Z = ratio_z;

	pl->TIMEOUT = time_out;
	pl->SCANNING_RATE = scanning_rate;

	pl->SX = dims_x;
	pl->SY = dims_y;
	pl->SZ = dims_z;

	pl->CHANNEL = ch;
	pl->MARKER_SORTED = marker_sorted;

	pl->SCAN_METHOD = scanMethod;
	pl->USE_SIMIMG = b_useSimImg;
	pl->SCAN_Z_REVERSE = b_scan_z_reverse;
}
