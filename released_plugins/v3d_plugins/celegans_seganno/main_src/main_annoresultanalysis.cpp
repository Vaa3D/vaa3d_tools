//main_annoresultanalysis.cpp
// by Lei Qu
//2010-12-21

#include <QtGui>
#include <stdio.h>
#include <unistd.h>
extern char *optarg;
extern int optind, opterr;

#include "basic_surf_objs.h"

void printHelp();
bool read_aliaslist_file(const QString &qs_filename,QList<QStringList> &qv_aliaslist);

int main(int argc, char *argv[])
{
//	QString qs_filename_anno		="/Users/qul/work/0.data/celegans_anno/result/standford_326/3061_musclecell_anno.apo";
//	QString qs_filename_manual		="/Users/qul/work/0.data/celegans_anno/atlas_related/data/cnd1threeL1_1213061.apo";
//	QString qs_filename_aliaslist	="/Users/qul/work/0.data/celegans_anno/atlas_related/aliaslist.txt";
//	QString qs_filename_output		="/Users/qul/work/0.data/celegans_anno/result/standford_326/resultanalysis.txt";
//	double d_disthreshold			=10;

	if(argc<=1)
	{
		printHelp();
		return 0;
	}

	QString qs_filename_anno		=NULL;
	QString qs_filename_manual		=NULL;
	QString qs_filename_aliaslist	=NULL;
	QString qs_filename_output		=NULL;
	double d_disthreshold			=10;
	//------------------------------------------------------------------------------------------------------------------------------------
	int c;
	static char optstring[]="ht:a:m:l:d:o:";
	opterr=0;
	while((c=getopt(argc,argv,optstring))!=-1)
    {
		switch (c)
        {
			case 'h':
				printHelp ();
				return 0;
				break;
			case 'a':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -a.\n");
					return 1;
				}
				qs_filename_anno.append(optarg);
				break;
			case 'm':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -m.\n");
					return 1;
				}
				qs_filename_manual.append(optarg);
				break;
			case 'l':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -l.\n");
					return 1;
				}
				qs_filename_aliaslist.append(optarg);
				break;
			case 'd':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -d.\n");
					return 1;
				}
				d_disthreshold=atof(optarg);
				break;
			case 'o':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				qs_filename_output.append(optarg);
				break;
			case '?':
				fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
				return 1;
				break;
		}
    }

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>atlas guided celegans cell annotation:\n");
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>input parameters:\n");
	printf(">>  input anno file:                %s\n",qPrintable(qs_filename_anno));
	printf(">>  input manual file:              %s\n",qPrintable(qs_filename_manual));
	printf(">>  input aliaslist file:           %s\n",qPrintable(qs_filename_aliaslist));
	printf(">>  d_disthreshold:                 %f\n",d_disthreshold);
	printf(">>-------------------------\n");
	printf(">>output parameters:\n");
	printf(">>  output anno info file:          %s\n",qPrintable(qs_filename_output));
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("1. Read alias list (txt). \n");
	QList<QStringList> ql_aliaslist;
	if(!read_aliaslist_file(qs_filename_aliaslist,ql_aliaslist))
	{
		printf("ERROR: read_aliaslist_file() return false!");
		return false;
	}
	printf("\t>>read %d alias from [%s]\n",ql_aliaslist.size(),qPrintable(qs_filename_aliaslist));

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("2. Read atlas based annotation result (apo). \n");
	QList<CellAPO> ql_anno;
	ql_anno=readAPO_file(qs_filename_anno);
	printf("\t>>read %d points from [%s]\n",ql_anno.size(),qPrintable(qs_filename_anno));
	if(ql_anno.size()<=0)
	{
		printf("ERROR: Given atlas file is empty or invalid!");
		return false;
	}

	//regular atlas based annotation name
	for(long i=0;i<ql_anno.size();i++)
	{
		QString qs_cellname=ql_anno[i].name;
		qs_cellname=qs_cellname.trimmed();
		qs_cellname=qs_cellname.replace(' ','_');
		qs_cellname=qs_cellname.toUpper();
		ql_anno[i].name=qs_cellname;
	}

	//find anno2alias index
	QVector<long> qv_ind_anno2alias(ql_anno.size(),-1);
	for(long i=0;i<ql_anno.size();i++)
	{
		QString qs_cellname=ql_anno[i].name;

		//find current cellname whether exist in the alias list
		bool b_find=0;
		for(long m=0;m<ql_aliaslist.size();m++)
		{
			for(long n=0;n<ql_aliaslist[m].size();n++)
			{
				if(qs_cellname==ql_aliaslist[m][n])
				{
					b_find=1;
					qv_ind_anno2alias[i]=m;
					break;
				}
			}
			if(b_find) break;
		}
		if(!b_find)
		{
			printf("ERROR: Annotated name[%s] can no found in the alias list, check program!\n",qPrintable(qs_cellname));
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("3. Read valid manual annotation result (apo). \n");
	//read all manual annotation result
	QList<CellAPO> ql_manual;
	ql_manual=readAPO_file(qs_filename_manual);
	printf("\t>>read %d points from [%s]\n",ql_manual.size(),qPrintable(qs_filename_manual));
	if(ql_manual.size()<=0)
	{
		printf("ERROR: Given manual annotation file is empty or invalid!");
		return false;
	}

	//regularize manual annotation name
	for(long i=0;i<ql_manual.size();i++)
	{
		QString qs_cellname=ql_manual[i].name;
		qs_cellname=qs_cellname.trimmed();
		qs_cellname=qs_cellname.replace(' ','_');
		qs_cellname=qs_cellname.toUpper();
		ql_manual[i].name=qs_cellname;
	}

	//find valid manual annotation and 2alias index by comparing with the aliaslist
	QList<CellAPO> ql_manual_valid;
	QVector<long> qv_ind_validman2alias;
	long n_valid=0;
	for(long i=0;i<ql_manual.size();i++)
	{
		QString qs_cellname=ql_manual[i].name;

		//find current cellname whether exist in the alias list
		bool b_find=0;
		for(long m=0;m<ql_aliaslist.size();m++)
		{
			for(long n=0;n<ql_aliaslist[m].size();n++)
			{
				if(qs_cellname==ql_aliaslist[m][n])
				{
					b_find=1;
					n_valid++;
					ql_manual_valid.push_back(ql_manual[i]);
					qv_ind_validman2alias.push_back(m);
					break;
				}
			}
			if(b_find) break;
		}
	}
	printf("\t>>%ld valid manual annotation founded \n",n_valid);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("4. find matching index of valid manual2anno by comparising their 2alias index. \n");
	QVector< QVector<long> > qv2d_ind_match;	//[0]:ind in aliaslist, [1]: ind in manual, [2]: ind in anno
	QVector<long> qv_ind_match(3,-1);
	QVector<long> qv_ind_anno2man(ql_anno.size(),-1);
	QVector<long> qv_ind_man2anno(ql_manual_valid.size(),-1);
	for(long i=0;i<ql_manual_valid.size();i++)
	{
		bool b_findmatch=0;
		long l_ind_man2anno;
		for(long j=0;j<ql_anno.size();j++)
			if(qv_ind_validman2alias[i]==qv_ind_anno2alias[j])
			{
				b_findmatch=1;
				l_ind_man2anno=j;
				qv_ind_match[0]=qv_ind_validman2alias[i];
				qv_ind_match[1]=i;
				qv_ind_match[2]=j;
				qv2d_ind_match.push_back(qv_ind_match);
				break;
			}
		if(b_findmatch)
		{
			qv_ind_man2anno[i]=l_ind_man2anno;
			qv_ind_anno2man[l_ind_man2anno]=i;
		}
	}
	printf("\t>>[%d] matched pairs founded\n",qv2d_ind_match.size());

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("5. compute the distance of matched cells. \n");
	QVector<double> qv_matchdis(qv2d_ind_match.size(),0);
	for(long i=0;i<qv_matchdis.size();i++)
	{
		double dx=ql_manual_valid[qv2d_ind_match[i][1]].x-ql_anno[qv2d_ind_match[i][2]].x;
		double dy=ql_manual_valid[qv2d_ind_match[i][1]].y-ql_anno[qv2d_ind_match[i][2]].y;
		double dz=ql_manual_valid[qv2d_ind_match[i][1]].z-ql_anno[qv2d_ind_match[i][2]].z;
		qv_matchdis[i]=sqrt(dx*dx+dy*dy+dz*dz);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("6. find wrongly annotation cells by compare with the manual annotation result. \n");
	QVector<long> qv_ind_wronganno;
	for(long i=0;i<qv_matchdis.size();i++)
		if(qv_matchdis[i]>d_disthreshold)
			qv_ind_wronganno.push_back(i);

	printf("\tfind the nearest manual annotated cells for incorrect annotated cells: \n");
	printf("\t>>(incorrect annotated cell index) --> (nearest manual annotated cell index) \n");
	QVector<long> qv_ind_wrongannonearestmanual(qv_ind_wronganno.size(),-1);
	for(long i=0;i<qv_ind_wronganno.size();i++)
	{
		//estimate what current wrongly annotated cell should be annotated as
		//find the nearest manual annotated cell to current wrongly annotated cell
		double mindis_manual=1e+10;
		long ind_mindis_manual=-1;
		for(long j=0;j<qv2d_ind_match.size();j++)
		{
			double dx=ql_manual_valid[qv2d_ind_match[j][1]].x-ql_anno[qv2d_ind_match[qv_ind_wronganno[i]][2]].x;
			double dy=ql_manual_valid[qv2d_ind_match[j][1]].y-ql_anno[qv2d_ind_match[qv_ind_wronganno[i]][2]].y;
			double dz=ql_manual_valid[qv2d_ind_match[j][1]].z-ql_anno[qv2d_ind_match[qv_ind_wronganno[i]][2]].z;
			double dis=sqrt(dx*dx+dy*dy+dz*dz);
			if(dis<mindis_manual)
			{
				mindis_manual=dis;
				ind_mindis_manual=qv2d_ind_match[j][1];
			}
		}
		//find the nearest annotated cell for the manual cell which nearest to current wrongly annotated cell
		double mindis_anno=1e+10;
		long ind_mindis_anno=-1;
		for(long j=0;j<ql_anno.size();j++)
		{
			double dx=ql_manual_valid[ind_mindis_manual].x-ql_anno[j].x;
			double dy=ql_manual_valid[ind_mindis_manual].y-ql_anno[j].y;
			double dz=ql_manual_valid[ind_mindis_manual].z-ql_anno[j].z;
			double dis=sqrt(dx*dx+dy*dy+dz*dz);
			if(dis<mindis_anno)
			{
				mindis_anno=dis;
				ind_mindis_anno=j;
			}
		}
		//if current cell is the nearest anno cell for certain manual cell, and vise versa && distance < d_disthreshold
		//we take this manual cell as the new prediction, otherwise we take current cell is predicted to nothing
//		if(qv2d_ind_match[qv_ind_wronganno[i]][2]==ind_mindis_anno && mindis_manual<d_disthreshold)
		if(mindis_manual<d_disthreshold)
		{
			qv_ind_wrongannonearestmanual[i]=ind_mindis_manual;
			printf("\t>>%s, dis=%.2f --> %s \n",qPrintable(ql_anno[qv2d_ind_match[qv_ind_wronganno[i]][2]].name),qv_matchdis[qv_ind_wronganno[i]],
					qPrintable(ql_anno[qv_ind_man2anno[qv_ind_wrongannonearestmanual[i]]].name));
		}
		else
		{
			qv_ind_wrongannonearestmanual[i]=-1;
			printf("\t>>%s, dis=%.2f --> nan \n",qPrintable(ql_anno[qv2d_ind_match[qv_ind_wronganno[i]][2]].name),qv_matchdis[qv_ind_wronganno[i]]);
		}
	}

	//compute total rightanno ratio
	double d_rightanno_ratio_total=(double)(qv2d_ind_match.size()-qv_ind_wronganno.size())/qv2d_ind_match.size();
	printf("\trightanno_ratio = [%d/%d] = %f\n",qv2d_ind_match.size()-qv_ind_wronganno.size(),
			qv2d_ind_match.size(),d_rightanno_ratio_total);

	//find matched muscle cells num
	long n_musclecells=0;
	for(long i=0;i<qv2d_ind_match.size();i++)
		if(ql_anno[qv2d_ind_match[i][2]].name.contains("BWM",Qt::CaseInsensitive))
			n_musclecells++;
	//find wrongly annotated muscle cell num and index
	QVector<long> qv_ind_wronganno_muscle;
	for(long i=0;i<qv_ind_wronganno.size();i++)
		if(ql_anno[qv2d_ind_match[qv_ind_wronganno[i]][2]].name.contains("BWM",Qt::CaseInsensitive))
			qv_ind_wronganno_muscle.push_back(qv_ind_wronganno[i]);
	//compute muscle cell rightanno ratio
	double d_rightanno_ratio_muscle=(double)(n_musclecells-qv_ind_wronganno_muscle.size())/n_musclecells;
	printf("\trightanno_ratio_muslce = [%ld/%ld] = %f\n",n_musclecells-qv_ind_wronganno_muscle.size(),
			n_musclecells,d_rightanno_ratio_muscle);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("8. save analysis info to file. \n");
	if(!qs_filename_output.isEmpty())
	{
		FILE *p_file=fopen(qs_filename_output.toAscii(),"w");
		if(!p_file)
		{
			printf("ERROR: open file error!\n");
			return false;
		}

		//save total correct annotaton rate
		fprintf(p_file,"rightanno_ratio_total=[%d/%d]=%f\n",qv2d_ind_match.size()-qv_ind_wronganno.size(),
				qv2d_ind_match.size(),d_rightanno_ratio_total);
		for(long i=0;i<qv_ind_wronganno.size();i++)
		{
			if(qv_ind_wrongannonearestmanual[i]<-0.5)
				fprintf(p_file,"%s->nan,",qPrintable(ql_anno[qv2d_ind_match[qv_ind_wronganno[i]][2]].name));
			else
				fprintf(p_file,"%s->%s,",qPrintable(ql_anno[qv2d_ind_match[qv_ind_wronganno[i]][2]].name),
						qPrintable(ql_anno[qv_ind_man2anno[qv_ind_wrongannonearestmanual[i]]].name));
		}
		fprintf(p_file,"\n");

		//save muscle cell correct annotation rate
		fprintf(p_file,"rightanno_ratio_muscle=[%ld/%ld]=%f\n",n_musclecells-qv_ind_wronganno_muscle.size(),
				n_musclecells,d_rightanno_ratio_muscle);
		for(long i=0;i<qv_ind_wronganno_muscle.size();i++)
			fprintf(p_file,"%s,",qPrintable(ql_anno[qv2d_ind_match[qv_ind_wronganno_muscle[i]][2]].name));
		fprintf(p_file,"\n");
		fprintf(p_file,"\n");

		fclose(p_file);
		printf("\t>>save to:[%s] complete.\n",qPrintable(qs_filename_output));
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf(">>Program exit successful!\n");
	return 0;
}


void printHelp()
{
	//pirnt help messages
	printf("\nUsage: main_annoresultanalysis\n");
	printf("Input paras:\n");
	printf("\t  -a   input atlas based annotation result file (apo).\n");
	printf("\t  -m   input manual annotation result file (apo).\n");
	printf("\t  -l   input aliaslist file (txt).\n");
	printf("\t [-d]  input max matched cell distance (default 10).\n");
	printf("Output paras:\n");
	printf("\t [-o]  output analysis info (txt).\n");
	printf("\n");
	printf("\t [-h]	print this message.\n");
	return;
}

bool read_aliaslist_file(const QString &qs_filename,QList<QStringList> &qv_aliaslist)
{
	//check paras
	if(qs_filename.isEmpty())
	{
		printf("ERROR: Invalid input file name.\n");
		return false;
	}
	if(!qv_aliaslist.isEmpty())
	{
		printf("WARNING: qv2d_aliaslist is not empty, original data will be deleted.\n");
		qv_aliaslist.clear();
	}

	QFile qf_file(qs_filename);
	if(!qf_file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		printf("ERROR: open file error!\n");
		return false;
	}

    while(!qf_file.atEnd())
    {
		char curline[2000];
        qf_file.readLine(curline,sizeof(curline));

        QString qs_curline(curline);
        qs_curline=qs_curline.trimmed();
        if(qs_curline.size()==0) continue;
        qs_curline.chop(1);	//remove the "," in the end of each line

        QStringList qsl_alias=qs_curline.split(",",QString::SkipEmptyParts);
        if(qsl_alias.size()==0) continue;

        qv_aliaslist.push_back(qsl_alias);
    }

	return true;
}
