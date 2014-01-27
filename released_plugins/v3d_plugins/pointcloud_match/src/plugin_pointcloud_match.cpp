
//by Lei Qu
//2009-11-10

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "plugin_pointcloud_match.h"
#include "../../basic_c_fun/basic_surf_objs.h"
#include "q_pointcloud_match_dialogs.h"
#include "q_pointcloud_match.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(plugin_pointcloud_match, PointCloudMatchPlugin);

void PointMatchFromImageMarker(V3DPluginCallback &callback, QWidget *parent);
void PointMatchFromMarkerFile(V3DPluginCallback &callback, QWidget *parent);
void PointMatchFromSWCFile(V3DPluginCallback &callback, QWidget *parent);
void PointMatchFromAPOFile(V3DPluginCallback &callback, QWidget *parent);
void BatchMatchFromANOFile(V3DPluginCallback &callback, QWidget *parent);

bool ReadAndFormat4PointCloudMatch_marker(QString qs_filename_input,vector<Coord3D_PCM> &vec_pointcloud4match);
bool ReadAndFormat4PointCloudMatch_swc(QString qs_filename_input,vector<Coord3D_PCM> &vec_pointcloud4match);
bool ReadAndFormat4PointCloudMatch_apo(QString qs_filename_input,vector<Coord3D_PCM> &vec_pointcloud4match);

const QString title = "PointCloudMatchPlugin demo";

//************************************************************************************************************************************
QStringList PointCloudMatchPlugin::menulist() const
{
    return QStringList()
	<< tr("match markers from images...")
	<< tr("match from marker files...")
    << tr("match from swc files...")
	<< tr("match from apo files...")
	<< tr("batch match from ano file...");
}

void PointCloudMatchPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if(menu_name==tr("match markers from images..."))
	{
		PointMatchFromImageMarker(callback, parent);
	}
	else if(menu_name==tr("match from marker files..."))
	{
		PointMatchFromMarkerFile(callback, parent);
	}
	else if(menu_name==tr("match from swc files..."))
	{
		PointMatchFromSWCFile(callback, parent);
	}
	else if(menu_name==tr("match from apo files..."))
	{
		PointMatchFromAPOFile(callback, parent);
	}
	else if(menu_name==tr("batch match from ano file..."))
	{
		BatchMatchFromANOFile(callback, parent);
	}
}


//************************************************************************************************************************************
void PointMatchFromImageMarker(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandleList h_wndlist=callback.getImageWindowList();
	if(h_wndlist.size()<=1)
	{
		QMessageBox::information(0,title,QObject::tr("Need at least 2 images."));
		return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//generate image choosing dialog to assign images which contain markers needed to be matched
	QStringList items;
	for(int i=0;i<h_wndlist.size();i++) items<<callback.getImageName(h_wndlist[i]);

	QDialog d(parent);
	QComboBox* combo1		=new QComboBox();	combo1->addItems(items);
	QComboBox* combo2		=new QComboBox();	combo2->addItems(items);
	QPushButton* ok    		=new QPushButton("OK");
	QPushButton* cancel		=new QPushButton("Cancel");
	QFormLayout *formLayout	=new QFormLayout;
	formLayout->addRow(QObject::tr("image 1 : "), combo1);
	formLayout->addRow(QObject::tr("image 2: "), combo2);
	formLayout->addRow(ok, cancel);
	d.setLayout(formLayout);
	d.setWindowTitle(QString("choose images in which markers needed to be matched"));

	d.connect(ok,    SIGNAL(clicked()),&d,SLOT(accept()));
	d.connect(cancel,SIGNAL(clicked()),&d,SLOT(reject()));
	if(d.exec()!=QDialog::Accepted) return;

	int i1=combo1->currentIndex();
	int i2=combo2->currentIndex();

	//obtain landmark list attched to the assign image
	LandmarkList list_landmark_img1=callback.getLandmark(h_wndlist[i1]);
	LandmarkList list_landmark_img2=callback.getLandmark(h_wndlist[i2]);
	printf(">>find %d markers in image 1\n",list_landmark_img1.size());
	printf(">>find %d markers in image 2\n",list_landmark_img2.size());
	if(list_landmark_img1.size()<5)
	{
		QMessageBox::information(0,title,QObject::tr("Need at least 5 markers in image 1."));
		return;
	}
	else if(list_landmark_img2.size()<5)
	{
		QMessageBox::information(0,title,QObject::tr("Need at least 5 markers in image 2."));
		return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//open para setting dialog
	int n_dismethod;																	//initial basic
	int arr_voterweight[100]={0},n_bin,K;												//initial adv
	int n_refinemethod;																	//refine basic
	int n_sampling,n_affineweight;														//refine adv - affine
	bool arr_constrainter[100]={0};int n_iternum,n_neighbor,n_topcandidatenum;			//refine adv - manifold

	ParaSettingDialog parasettingDlg(parent);
	if(parasettingDlg.exec()==QDialog::Accepted)
	{
		//initial basic
		if(parasettingDlg.m_pRadioButton_euclidian->isChecked())
			n_dismethod=0;
		else if(parasettingDlg.m_pRadioButton_geodesic->isChecked())
			n_dismethod=1;
		//initial adv
		arr_voterweight[0]=parasettingDlg.m_pLineEdit_NN_weight->text().toInt();
		arr_voterweight[1]=parasettingDlg.m_pLineEdit_linear_weight->text().toInt();
		arr_voterweight[2]=parasettingDlg.m_pLineEdit_histogram_weight->text().toInt();
		n_bin=parasettingDlg.m_pLineEdit_bin->text().toInt();
		K=parasettingDlg.m_pLineEdit_K->text().toInt();
		//refine basic
		if(!parasettingDlg.m_pGroupBox_refinematch->isChecked())
			n_refinemethod=0;
		else if(parasettingDlg.m_pRadioButton_affine->isChecked())
			n_refinemethod=1;
		else if(parasettingDlg.m_pRadioButton_manifold->isChecked())
			n_refinemethod=2;
		//refine adv - affine
		n_sampling=parasettingDlg.m_pLineEdit_nransacsampling->text().toInt();
		n_affineweight=parasettingDlg.m_pLineEdit_affineinvp_weight->text().toInt();
		//refine adv - manifold
		if(parasettingDlg.m_pCheckBox_dir->isChecked())
			arr_constrainter[0]=1;
		else
			arr_constrainter[0]=0;
		if(parasettingDlg.m_pCheckBox_dis->isChecked())
			arr_constrainter[1]=1;
		else
			arr_constrainter[1]=0;
		if(parasettingDlg.m_pCheckBox_topcandidate->isChecked())
			arr_constrainter[2]=1;
		else
			arr_constrainter[2]=0;
		n_iternum=parasettingDlg.m_pLineEdit_maxiternum->text().toInt();
		n_neighbor=parasettingDlg.m_pLineEdit_neighbornum4dirdisavg->text().toInt();
		n_topcandidatenum=parasettingDlg.m_pLineEdit_topcandidatenum->text().toInt();
	}
	else
		return;

	//------------------------------------------------------------------------------------------------------------------------------------
	//reorganize the markers to the format needed by point_cloud_registration.app
	vector<Coord3D_PCM> vec_1,vec_2;
	for(long i=0;i<list_landmark_img1.size();i++)
	{
		vec_1.push_back(Coord3D_PCM(list_landmark_img1[i].x,list_landmark_img1[i].y,list_landmark_img1[i].z));
	}
	for(long i=0;i<list_landmark_img2.size();i++)
	{
		vec_2.push_back(Coord3D_PCM(list_landmark_img2[i].x,list_landmark_img2[i].y,list_landmark_img2[i].z));
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//run point clouds matching
	vector<int> vec_1to2index;
	vector<Coord3D_PCM> vec_2_invp;
	if(!q_pointcloud_match(vec_1,vec_2,
			n_dismethod,												//initial basic
			arr_voterweight,n_bin,K,									//initial adv
			n_refinemethod,												//refine basic
			n_sampling,n_affineweight,									//refine adv - affine
			arr_constrainter,n_iternum,n_neighbor,n_topcandidatenum,	//refine adv - manifold
			vec_1to2index,												//output
			vec_2_invp))												//output - refine adv - affine
	{
		fprintf(stderr,"ERROR: q_pointcloud_match() return false! \n");
		return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//rereorganize matched marker pairs back to LandmarkList (for save back to images)
	//rereorganize matched marker pairs back to QList<ImageMarker> (for save to marker files)
	LandmarkList list_landmark_tar_new,list_landmark_sub_new;
	QList<ImageMarker> ql_marker_1_new,ql_marker_2_new;
	long n_validpair=0;
	for(long i=0;i<vec_1to2index.size();i++)
	{
		if(vec_1to2index[i]!=-1)
		{
			n_validpair++;

			list_landmark_tar_new.push_back(list_landmark_img1[i]);
			list_landmark_sub_new.push_back(list_landmark_img2[vec_1to2index[i]]);

			ImageMarker temp;
			temp.x=vec_1[i].x;
			temp.y=vec_1[i].y;
			temp.z=vec_1[i].z;
			ql_marker_1_new.push_back(temp);
			temp.x=vec_2[vec_1to2index[i]].x;
			temp.y=vec_2[vec_1to2index[i]].y;
			temp.z=vec_2[vec_1to2index[i]].z;
			ql_marker_2_new.push_back(temp);
		}
	}
	printf(">>point cloud registration complete sucessfully, %d valid pairs finded.\n",n_validpair);

	//------------------------------------------------------------------------------------------------------------------------------------
	//view matching result and choose save options
	QGroupBox *matchResultGroup=new QGroupBox(parent);
	matchResultGroup->setTitle(QObject::tr("Match result"));
	QString qs_result=QString(QObject::tr(">>%1 valid matched pairs found:<br><br>")).arg(int(n_validpair));
	for(long i=0;i<vec_1to2index.size();i++)
	{
		QString temp=QString(QObject::tr("(%1).\timg1(%2) --> img2(%3)<br>")).arg(i).arg(i).arg(int(vec_1to2index[i]));
		qs_result.append(temp);
	}
	QTextEdit *matchResultText=new QTextEdit(qs_result,matchResultGroup);
	matchResultText->setReadOnly(true);
	matchResultText->setFontPointSize(12);

	QGroupBox *saveOptionGroup=new QGroupBox(parent);
	saveOptionGroup->setTitle(QObject::tr("Save option"));
	QCheckBox *save2imagesCheckBox=new QCheckBox(saveOptionGroup);
	QCheckBox *save2markerfileCheckBox=new QCheckBox(saveOptionGroup);
	QCheckBox *save2aposwcfileCheckBox=new QCheckBox(saveOptionGroup);
	QCheckBox *saveinvpB2A2aposwcfileCheckBox=new QCheckBox(saveOptionGroup);
	save2imagesCheckBox->setText(QObject::tr("save matching results back to images"));
	save2markerfileCheckBox->setText(QObject::tr("save matching results to marker files"));
	save2aposwcfileCheckBox->setText(QObject::tr("save colorized and lined matched pairs to swc and apo files"));
	saveinvpB2A2aposwcfileCheckBox->setText(QObject::tr("save (2->1) inverse projected point set to apo and swc file"));

	QVBoxLayout *matchResultLayout=new QVBoxLayout(matchResultGroup);
	matchResultLayout->addWidget(matchResultText);

	QVBoxLayout *saveOptionLayout=new QVBoxLayout(saveOptionGroup);
	saveOptionLayout->addWidget(save2imagesCheckBox);
	saveOptionLayout->addWidget(save2markerfileCheckBox);
	saveOptionLayout->addWidget(save2aposwcfileCheckBox);
	saveOptionLayout->addWidget(saveinvpB2A2aposwcfileCheckBox);

	QHBoxLayout *savecancelLayout=new QHBoxLayout;
	QPushButton* save=new QPushButton("Save");
//	QPushButton* cancel=new QPushButton("Cancel");
	savecancelLayout->addWidget(save);
	savecancelLayout->addWidget(cancel);

	QVBoxLayout *mainLayout=new QVBoxLayout;
	mainLayout->addWidget(matchResultGroup);
	mainLayout->addWidget(saveOptionGroup);
	mainLayout->addWidget(new QLabel(QObject::tr("Note: suffix will be added automatically\n(*_img1.marker, *_img2.marker *.swc, *.apo)")));
	mainLayout->addLayout(savecancelLayout);

	QDialog dialog_showsave(parent);
	dialog_showsave.setWindowTitle(QObject::tr("Match Result & Save Option"));
	dialog_showsave.setLayout(mainLayout);
	dialog_showsave.connect(save,  SIGNAL(clicked()),&dialog_showsave,SLOT(accept()));
	dialog_showsave.connect(cancel,SIGNAL(clicked()),&dialog_showsave,SLOT(reject()));

	if(n_refinemethod!=1)//do not use affine refine
		saveinvpB2A2aposwcfileCheckBox->setEnabled(false);

	QStringList qsl_outputinfo;
	qsl_outputinfo.push_back("matching results were saved to:");
	if(dialog_showsave.exec()==QDialog::Accepted)
	{
		QString filename_nosuffix,filename_marker_img1,filename_marker_img2,filename_swc_lined,filename_apo_colored,filename_swc_invp,filename_apo_invp;
		filename_nosuffix=QFileDialog::getSaveFileName(parent,QString(QObject::tr("Only input filename (without suffix)")));
		filename_marker_img1=filename_nosuffix+QString("_1.marker");
		filename_marker_img2=filename_nosuffix+QString("_2.marker");
		filename_swc_lined=filename_nosuffix+QString(".swc");
		filename_apo_colored=filename_nosuffix+QString(".apo");
		filename_swc_invp=filename_nosuffix+QString("_invp.swc");
		filename_apo_invp=filename_nosuffix+QString("_invp.apo");

		if(save2imagesCheckBox->isChecked())
		{
			callback.setLandmark(h_wndlist[i1],list_landmark_tar_new);
			callback.setLandmark(h_wndlist[i2],list_landmark_sub_new);
			qsl_outputinfo.push_back("save matched markers back to images!");
		}
		if(save2markerfileCheckBox->isChecked())
		{
			writeMarker_file(filename_marker_img1,ql_marker_1_new);
			writeMarker_file(filename_marker_img2,ql_marker_2_new);
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_marker_img1));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_marker_img2));
		}
		if(save2aposwcfileCheckBox->isChecked())
		{
			q_export_matches2swc(vec_1,vec_2,vec_1to2index,qPrintable(filename_swc_lined));
			q_export_matches2apo(vec_1,vec_2,vec_2_invp,qPrintable(filename_apo_colored),12);
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_swc_lined));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_apo_colored));
		}
		if(saveinvpB2A2aposwcfileCheckBox->isEnabled() && saveinvpB2A2aposwcfileCheckBox->isChecked())
		{
			q_export_matches2apo(vec_1,vec_2,vec_2_invp,qPrintable(filename_apo_invp),30);
			q_export_matches2swc(vec_1,vec_2_invp,vec_1to2index,qPrintable(filename_swc_invp));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_apo_invp));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_swc_invp));
		}
	}
	else
		return;

	//display output info dialog
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setWindowTitle(QObject::tr("Output information"));
	msgBox.setText(qsl_outputinfo.join("\n"));
	msgBox.exec();
}

void PointMatchFromMarkerFile(V3DPluginCallback &callback, QWidget *parent)
{
	//------------------------------------------------------------------------------------------------------------------------------------
	//open two files that contain the point cloud
	QString qs_filename_1,qs_filename_2;

	Choose2FileDialog choose2FileDlg(parent);
	if(choose2FileDlg.exec()==QDialog::Accepted)
	{
		qs_filename_1=choose2FileDlg.m_getFilename_1();
		qs_filename_2=choose2FileDlg.m_getFilename_2();
	}
	else
		return;

	//------------------------------------------------------------------------------------------------------------------------------------
	//open para setting dialog
	int n_dismethod;																	//initial basic
	int arr_voterweight[100]={0},n_bin,K;												//initial adv
	int n_refinemethod;																	//refine basic
	int n_sampling,n_affineweight;														//refine adv - affine
	bool arr_constrainter[100]={0};int n_iternum,n_neighbor,n_topcandidatenum;			//refine adv - manifold

	ParaSettingDialog parasettingDlg(parent);
	if(parasettingDlg.exec()==QDialog::Accepted)
	{
		//initial basic
		if(parasettingDlg.m_pRadioButton_euclidian->isChecked())
			n_dismethod=0;
		else if(parasettingDlg.m_pRadioButton_geodesic->isChecked())
			n_dismethod=1;
		//initial adv
		arr_voterweight[0]=parasettingDlg.m_pLineEdit_NN_weight->text().toInt();
		arr_voterweight[1]=parasettingDlg.m_pLineEdit_linear_weight->text().toInt();
		arr_voterweight[2]=parasettingDlg.m_pLineEdit_histogram_weight->text().toInt();
		n_bin=parasettingDlg.m_pLineEdit_bin->text().toInt();
		K=parasettingDlg.m_pLineEdit_K->text().toInt();
		//refine basic
		if(!parasettingDlg.m_pGroupBox_refinematch->isChecked())
			n_refinemethod=0;
		else if(parasettingDlg.m_pRadioButton_affine->isChecked())
			n_refinemethod=1;
		else if(parasettingDlg.m_pRadioButton_manifold->isChecked())
			n_refinemethod=2;
		//refine adv - affine
		n_sampling=parasettingDlg.m_pLineEdit_nransacsampling->text().toInt();
		n_affineweight=parasettingDlg.m_pLineEdit_affineinvp_weight->text().toInt();
		//refine adv - manifold
		if(parasettingDlg.m_pCheckBox_dir->isChecked())
			arr_constrainter[0]=1;
		else
			arr_constrainter[0]=0;
		if(parasettingDlg.m_pCheckBox_dis->isChecked())
			arr_constrainter[1]=1;
		else
			arr_constrainter[1]=0;
		if(parasettingDlg.m_pCheckBox_topcandidate->isChecked())
			arr_constrainter[2]=1;
		else
			arr_constrainter[2]=0;
		n_iternum=parasettingDlg.m_pLineEdit_maxiternum->text().toInt();
		n_neighbor=parasettingDlg.m_pLineEdit_neighbornum4dirdisavg->text().toInt();
		n_topcandidatenum=parasettingDlg.m_pLineEdit_topcandidatenum->text().toInt();
	}
	else
		return;

	//------------------------------------------------------------------------------------------------------------------------------------
	//read and format point clouds for matching
	//read marker files
	QList<ImageMarker> ql_marker_1,ql_marker_2;
	if(qs_filename_1.endsWith(".marker") && qs_filename_2.endsWith(".marker"))
    {
		ql_marker_1=readMarker_file(qs_filename_1);
		ql_marker_2=readMarker_file(qs_filename_2);
    	printf("\t>>read %d markers from [%s]\n",ql_marker_1.size(),qPrintable(qs_filename_1));
    	printf("\t>>read %d markers from [%s]\n",ql_marker_2.size(),qPrintable(qs_filename_2));
    }
    else
    {
    	printf("ERROR: at least one marker file is invalid.\n");
    	QMessageBox::information(parent,title,QObject::tr("Read marker files error!"));
    	return;
	}
	//reorganize the markers to the format needed by point_cloud_registration.app
	vector<Coord3D_PCM> vec_1,vec_2;
	for(long i=0;i<ql_marker_1.size();i++)
	{
		vec_1.push_back(Coord3D_PCM(ql_marker_1[i].x,ql_marker_1[i].y,ql_marker_1[i].z));
	}
	for(long i=0;i<ql_marker_2.size();i++)
	{
		vec_2.push_back(Coord3D_PCM(ql_marker_2[i].x,ql_marker_2[i].y,ql_marker_2[i].z));
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//run point clouds matching
	vector<int> vec_1to2index;
	vector<Coord3D_PCM> vec_2_invp;
	if(!q_pointcloud_match(vec_1,vec_2,
			n_dismethod,												//initial basic
			arr_voterweight,n_bin,K,									//initial adv
			n_refinemethod,												//refine basic
			n_sampling,n_affineweight,									//refine adv - affine
			arr_constrainter,n_iternum,n_neighbor,n_topcandidatenum,	//refine adv - manifold
			vec_1to2index,												//output
			vec_2_invp))												//output - refine adv - affine
	{
		fprintf(stderr,"ERROR: q_pointcloud_match() return false! \n");
		return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//rereorganize matched marker pairs back to LandmarkList
	QList<ImageMarker> ql_marker_1_new,ql_marker_2_new;
	long n_validpair=0;
	for(long i=0;i<vec_1to2index.size();i++)
	{
		if(vec_1to2index[i]!=-1)
		{
			n_validpair++;
			ql_marker_1_new.push_back(ql_marker_1[i]);
			ql_marker_2_new.push_back(ql_marker_2[vec_1to2index[i]]);
		}
	}
	printf(">>point cloud registration complete sucessfully, %d valid pairs finded.\n",n_validpair);

	//------------------------------------------------------------------------------------------------------------------------------------
	//view matching result and choose save options
	QGroupBox *matchResultGroup=new QGroupBox(parent);
	matchResultGroup->setTitle(QObject::tr("Match result"));
	QString qs_result=QString(QObject::tr(">>%1 valid matched pairs found:<br><br>")).arg(int(n_validpair));
	for(long i=0;i<vec_1to2index.size();i++)
	{
		QString temp=QString(QObject::tr("(%1).\tfile1(%2) --> file2(%3)<br>")).arg(i).arg(i).arg(int(vec_1to2index[i]));
		qs_result.append(temp);
	}
	QTextEdit *matchResultText=new QTextEdit(qs_result,matchResultGroup);
	matchResultText->setReadOnly(true);
	matchResultText->setFontPointSize(12);

	QGroupBox *saveOptionGroup=new QGroupBox(parent);
	saveOptionGroup->setTitle(QObject::tr("Save option"));
	QCheckBox *save2markerfileCheckBox=new QCheckBox(saveOptionGroup);
	QCheckBox *save2aposwcfileCheckBox=new QCheckBox(saveOptionGroup);
	QCheckBox *saveinvpB2A2aposwcfileCheckBox=new QCheckBox(saveOptionGroup);
	save2markerfileCheckBox->setText(QObject::tr("save matching results back to marker files"));
	save2aposwcfileCheckBox->setText(QObject::tr("save colorized and lined matched pairs to swc and apo files"));
	saveinvpB2A2aposwcfileCheckBox->setText(QObject::tr("save (2->1) inverse projected point set to apo and swc file"));

	QVBoxLayout *matchResultLayout=new QVBoxLayout(matchResultGroup);
	matchResultLayout->addWidget(matchResultText);

	QVBoxLayout *saveOptionLayout=new QVBoxLayout(saveOptionGroup);
	saveOptionLayout->addWidget(save2markerfileCheckBox);
	saveOptionLayout->addWidget(save2aposwcfileCheckBox);
	saveOptionLayout->addWidget(saveinvpB2A2aposwcfileCheckBox);

	QHBoxLayout *savecancelLayout=new QHBoxLayout;
	QPushButton* save=new QPushButton("Save");
	QPushButton* cancel=new QPushButton("Cancel");
	savecancelLayout->addWidget(save);
	savecancelLayout->addWidget(cancel);

	QVBoxLayout *mainLayout=new QVBoxLayout;
	mainLayout->addWidget(matchResultGroup);
	mainLayout->addWidget(saveOptionGroup);
	mainLayout->addWidget(new QLabel(QObject::tr("Note: suffix will be added automatically\n(*_img1.marker, *_img2.marker *.swc, *.apo)")));
	mainLayout->addLayout(savecancelLayout);

	QDialog dialog_showsave(parent);
	dialog_showsave.setWindowTitle(QObject::tr("Match Result & Save Option"));
	dialog_showsave.setLayout(mainLayout);
	dialog_showsave.connect(save,  SIGNAL(clicked()),&dialog_showsave,SLOT(accept()));
	dialog_showsave.connect(cancel,SIGNAL(clicked()),&dialog_showsave,SLOT(reject()));

	if(n_refinemethod!=1)//do not use affine refine
		saveinvpB2A2aposwcfileCheckBox->setEnabled(false);

	QStringList qsl_outputinfo;
	qsl_outputinfo.push_back("matching results were saved to:");
	if(dialog_showsave.exec()==QDialog::Accepted)
	{
		QString filename_nosuffix,filename_marker_img1,filename_marker_img2,filename_swc_lined,filename_apo_colored,filename_swc_invp,filename_apo_invp;
		filename_nosuffix=QFileDialog::getSaveFileName(parent,QString(QObject::tr("Only input filename (without suffix)")));
		filename_marker_img1=filename_nosuffix+QString("_1.marker");
		filename_marker_img2=filename_nosuffix+QString("_2.marker");
		filename_swc_lined=filename_nosuffix+QString(".swc");
		filename_apo_colored=filename_nosuffix+QString(".apo");
		filename_swc_invp=filename_nosuffix+QString("_invp.swc");
		filename_apo_invp=filename_nosuffix+QString("_invp.apo");

		if(save2markerfileCheckBox->isChecked())
		{
			writeMarker_file(filename_marker_img1,ql_marker_1_new);
			writeMarker_file(filename_marker_img2,ql_marker_2_new);
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_marker_img1));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_marker_img1));
		}
		if(save2aposwcfileCheckBox->isChecked())
		{
			q_export_matches2swc(vec_1,vec_2,vec_1to2index,qPrintable(filename_swc_lined));
			q_export_matches2apo(vec_1,vec_2,vec_2_invp,qPrintable(filename_apo_colored),12);
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_swc_lined));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_apo_colored));
		}
		if(saveinvpB2A2aposwcfileCheckBox->isEnabled() && saveinvpB2A2aposwcfileCheckBox->isChecked())
		{
			q_export_matches2apo(vec_1,vec_2,vec_2_invp,qPrintable(filename_apo_invp),30);
			q_export_matches2swc(vec_1,vec_2_invp,vec_1to2index,qPrintable(filename_swc_invp));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_apo_invp));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_swc_invp));
		}
	}
	else
		return;

	//display output info dialog
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setWindowTitle(QObject::tr("Output information"));
	msgBox.setText(qsl_outputinfo.join("\n"));
	msgBox.exec();
}

void PointMatchFromSWCFile(V3DPluginCallback &callback, QWidget *parent)
{
	//open two files that contain the point cloud
	QString qs_filename_1,qs_filename_2;

	Choose2FileDialog choose2FileDlg(parent);
	if(choose2FileDlg.exec()==QDialog::Accepted)
	{
		qs_filename_1=choose2FileDlg.m_getFilename_1();
		qs_filename_2=choose2FileDlg.m_getFilename_2();
	}
	else
		return;

	//------------------------------------------------------------------------------------------------------------------------------------
	//open para setting dialog
	int n_dismethod;																	//initial basic
	int arr_voterweight[100]={0},n_bin,K;												//initial adv
	int n_refinemethod;																	//refine basic
	int n_sampling,n_affineweight;														//refine adv - affine
	bool arr_constrainter[100]={0};int n_iternum,n_neighbor,n_topcandidatenum;			//refine adv - manifold

	ParaSettingDialog parasettingDlg(parent);
	if(parasettingDlg.exec()==QDialog::Accepted)
	{
		//initial basic
		if(parasettingDlg.m_pRadioButton_euclidian->isChecked())
			n_dismethod=0;
		else if(parasettingDlg.m_pRadioButton_geodesic->isChecked())
			n_dismethod=1;
		//initial adv
		arr_voterweight[0]=parasettingDlg.m_pLineEdit_NN_weight->text().toInt();
		arr_voterweight[1]=parasettingDlg.m_pLineEdit_linear_weight->text().toInt();
		arr_voterweight[2]=parasettingDlg.m_pLineEdit_histogram_weight->text().toInt();
		n_bin=parasettingDlg.m_pLineEdit_bin->text().toInt();
		K=parasettingDlg.m_pLineEdit_K->text().toInt();
		//refine basic
		if(!parasettingDlg.m_pGroupBox_refinematch->isChecked())
			n_refinemethod=0;
		else if(parasettingDlg.m_pRadioButton_affine->isChecked())
			n_refinemethod=1;
		else if(parasettingDlg.m_pRadioButton_manifold->isChecked())
			n_refinemethod=2;
		//refine adv - affine
		n_sampling=parasettingDlg.m_pLineEdit_nransacsampling->text().toInt();
		n_affineweight=parasettingDlg.m_pLineEdit_affineinvp_weight->text().toInt();
		//refine adv - manifold
		if(parasettingDlg.m_pCheckBox_dir->isChecked())
			arr_constrainter[0]=1;
		else
			arr_constrainter[0]=0;
		if(parasettingDlg.m_pCheckBox_dis->isChecked())
			arr_constrainter[1]=1;
		else
			arr_constrainter[1]=0;
		if(parasettingDlg.m_pCheckBox_topcandidate->isChecked())
			arr_constrainter[2]=1;
		else
			arr_constrainter[2]=0;
		n_iternum=parasettingDlg.m_pLineEdit_maxiternum->text().toInt();
		n_neighbor=parasettingDlg.m_pLineEdit_neighbornum4dirdisavg->text().toInt();
		n_topcandidatenum=parasettingDlg.m_pLineEdit_topcandidatenum->text().toInt();
	}
	else
		return;

	//------------------------------------------------------------------------------------------------------------------------------------
	//read swc files
	NeuronTree nt_tar,nt_sub;
	if(qs_filename_1.endsWith(".swc") && qs_filename_2.endsWith(".swc"))
    {
		nt_tar=readSWC_file(qs_filename_1);
		nt_sub=readSWC_file(qs_filename_2);
    	printf("\t>>read %d points from [%s]\n",nt_tar.listNeuron.size(),qPrintable(qs_filename_1));
    	printf("\t>>read %d points from [%s]\n",nt_sub.listNeuron.size(),qPrintable(qs_filename_2));
    }
    else
    {
    	printf("ERROR: at least one swc file is invalid.\n");
    	QMessageBox::information(parent,title,QObject::tr("Read swc files error!"));
    	return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//reorganize the markers to the format needed by point_cloud_registration.app
	vector<Coord3D_PCM> vec_1,vec_2;
	Coord3D_PCM temp;
	for(int i=0;i<nt_tar.listNeuron.size();i++)
	{
		temp.x=nt_tar.listNeuron.at(i).x;
		temp.y=nt_tar.listNeuron.at(i).y;
		temp.z=nt_tar.listNeuron.at(i).z;
		vec_1.push_back(temp);
	}
	for(int i=0;i<nt_sub.listNeuron.size();i++)
	{
//		temp.x=nt_sub.listNeuron.at(i).x;
//		temp.y=nt_sub.listNeuron.at(i).y;
//		temp.z=nt_sub.listNeuron.at(i).z;
		temp.x=nt_sub.listNeuron.at(i).x;
		temp.y=nt_sub.listNeuron.at(i).y;
		temp.z=nt_sub.listNeuron.at(i).z;
		vec_2.push_back(temp);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//run point clouds matching
	vector<int> vec_1to2index;
	vector<Coord3D_PCM> vec_2_invp;
	if(!q_pointcloud_match(vec_1,vec_2,
			n_dismethod,												//initial basic
			arr_voterweight,n_bin,K,									//initial adv
			n_refinemethod,												//refine basic
			n_sampling,n_affineweight,									//refine adv - affine
			arr_constrainter,n_iternum,n_neighbor,n_topcandidatenum,	//refine adv - manifold
			vec_1to2index,												//output
			vec_2_invp))												//output - refine adv - affine
	{
		fprintf(stderr,"ERROR: q_pointcloud_match() return false! \n");
		return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//rereorganize matched marker pairs back to QList<ImageMarker> for save marker file
	QList<ImageMarker> ql_marker_1_new,ql_marker_2_new;
	long n_validpair=0;
	for(long i=0;i<vec_1to2index.size();i++)
	{
		if(vec_1to2index[i]!=-1)
		{
			n_validpair++;

			ImageMarker temp;
			temp.x=vec_1[i].x;
			temp.y=vec_1[i].y;
			temp.z=vec_1[i].z;
			ql_marker_1_new.push_back(temp);
			temp.x=vec_2[vec_1to2index[i]].x;
			temp.y=vec_2[vec_1to2index[i]].y;
			temp.z=vec_2[vec_1to2index[i]].z;
			ql_marker_2_new.push_back(temp);
		}
	}
	printf(">>point cloud registration complete sucessfully, %d valid pairs finded.\n",n_validpair);


	//------------------------------------------------------------------------------------------------------------------------------------
	//view matching result and choose save options
	QGroupBox *matchResultGroup=new QGroupBox(parent);
	matchResultGroup->setTitle(QObject::tr("Match result"));
	QString qs_result=QString(QObject::tr(">>%1 valid matched pairs found:<br><br>")).arg(int(n_validpair));
	for(long i=0;i<vec_1to2index.size();i++)
	{
		QString temp=QString(QObject::tr("(%1).\tfile1(%2) --> file2(%3)<br>")).arg(i).arg(i).arg(int(vec_1to2index[i]));
		qs_result.append(temp);
	}
	QTextEdit *matchResultText=new QTextEdit(qs_result,matchResultGroup);
	matchResultText->setReadOnly(true);
	matchResultText->setFontPointSize(12);

	QGroupBox *saveOptionGroup=new QGroupBox(parent);
	saveOptionGroup->setTitle(QObject::tr("Save option"));
	QCheckBox *save2markerfileCheckBox=new QCheckBox(saveOptionGroup);
	QCheckBox *save2aposwcfileCheckBox=new QCheckBox(saveOptionGroup);
	QCheckBox *saveinvpB2A2aposwcfileCheckBox=new QCheckBox(saveOptionGroup);
	save2markerfileCheckBox->setText(QObject::tr("save matching results to marker files"));
	save2aposwcfileCheckBox->setText(QObject::tr("save colorized and lined matched pairs to swc and apo files"));
	saveinvpB2A2aposwcfileCheckBox->setText(QObject::tr("save (2->1) inverse projected point set to apo and swc file"));

	QVBoxLayout *matchResultLayout=new QVBoxLayout(matchResultGroup);
	matchResultLayout->addWidget(matchResultText);

	QVBoxLayout *saveOptionLayout=new QVBoxLayout(saveOptionGroup);
	saveOptionLayout->addWidget(save2markerfileCheckBox);
	saveOptionLayout->addWidget(save2aposwcfileCheckBox);
	saveOptionLayout->addWidget(saveinvpB2A2aposwcfileCheckBox);

	QHBoxLayout *savecancelLayout=new QHBoxLayout;
	QPushButton* save=new QPushButton("Save");
	QPushButton* cancel=new QPushButton("Cancel");
	savecancelLayout->addWidget(save);
	savecancelLayout->addWidget(cancel);

	QVBoxLayout *mainLayout=new QVBoxLayout;
	mainLayout->addWidget(matchResultGroup);
	mainLayout->addWidget(saveOptionGroup);
	mainLayout->addWidget(new QLabel(QObject::tr("Note: suffix will be added automatically\n(*_img1.marker, *_img2.marker *.swc, *.apo)")));
	mainLayout->addLayout(savecancelLayout);

	QDialog dialog_showsave(parent);
	dialog_showsave.setWindowTitle(QObject::tr("Match Result & Save Option"));
	dialog_showsave.setLayout(mainLayout);
	dialog_showsave.connect(save,  SIGNAL(clicked()),&dialog_showsave,SLOT(accept()));
	dialog_showsave.connect(cancel,SIGNAL(clicked()),&dialog_showsave,SLOT(reject()));

	if(n_refinemethod!=1)//do not use affine refine
		saveinvpB2A2aposwcfileCheckBox->setEnabled(false);

	QStringList qsl_outputinfo;
	qsl_outputinfo.push_back("matching results were saved to:");
	if(dialog_showsave.exec()==QDialog::Accepted)
	{
		QString filename_nosuffix,filename_marker_img1,filename_marker_img2,filename_swc_lined,filename_apo_colored,filename_swc_invp,filename_apo_invp;
		filename_nosuffix=QFileDialog::getSaveFileName(parent,QString(QObject::tr("Only input filename (without suffix)")));
		filename_marker_img1=filename_nosuffix+QString("_1.marker");
		filename_marker_img2=filename_nosuffix+QString("_2.marker");
		filename_swc_lined=filename_nosuffix+QString(".swc");
		filename_apo_colored=filename_nosuffix+QString(".apo");
		filename_swc_invp=filename_nosuffix+QString("_invp.swc");
		filename_apo_invp=filename_nosuffix+QString("_invp.apo");

		if(save2markerfileCheckBox->isChecked())
		{
			writeMarker_file(filename_marker_img1,ql_marker_1_new);
			writeMarker_file(filename_marker_img2,ql_marker_2_new);
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_marker_img1));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_marker_img2));
		}
		if(save2aposwcfileCheckBox->isChecked())
		{
			q_export_matches2swc(vec_1,vec_2,vec_1to2index,qPrintable(filename_swc_lined));
			q_export_matches2apo(vec_1,vec_2,vec_2_invp,qPrintable(filename_apo_colored),12);
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_swc_lined));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_apo_colored));
		}
		if(saveinvpB2A2aposwcfileCheckBox->isEnabled() && saveinvpB2A2aposwcfileCheckBox->isChecked())
		{
			q_export_matches2apo(vec_1,vec_2,vec_2_invp,qPrintable(filename_apo_invp),30);
			q_export_matches2swc(vec_1,vec_2_invp,vec_1to2index,qPrintable(filename_swc_invp));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_apo_invp));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_swc_invp));
		}
	}
	else
		return;

	//display output info dialog
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setWindowTitle(QObject::tr("Output information"));
	msgBox.setText(qsl_outputinfo.join("\n"));
	msgBox.exec();
}

void PointMatchFromAPOFile(V3DPluginCallback &callback, QWidget *parent)
{
	//------------------------------------------------------------------------------------------------------------------------------------
	//open two files that contain the point cloud
	QString qs_filename_1,qs_filename_2;

	Choose2FileDialog choose2FileDlg(parent);
	if(choose2FileDlg.exec()==QDialog::Accepted)
	{
		qs_filename_1=choose2FileDlg.m_getFilename_1();
		qs_filename_2=choose2FileDlg.m_getFilename_2();
	}
	else
		return;

	//------------------------------------------------------------------------------------------------------------------------------------
	//open para setting dialog
	int n_dismethod;																	//initial basic
	int arr_voterweight[100]={0},n_bin,K;												//initial adv
	int n_refinemethod;																	//refine basic
	int n_sampling,n_affineweight;														//refine adv - affine
	bool arr_constrainter[100]={0};int n_iternum,n_neighbor,n_topcandidatenum;			//refine adv - manifold

	ParaSettingDialog parasettingDlg(parent);
	if(parasettingDlg.exec()==QDialog::Accepted)
	{
		//initial basic
		if(parasettingDlg.m_pRadioButton_euclidian->isChecked())
			n_dismethod=0;
		else if(parasettingDlg.m_pRadioButton_geodesic->isChecked())
			n_dismethod=1;
		//initial adv
		arr_voterweight[0]=parasettingDlg.m_pLineEdit_NN_weight->text().toInt();
		arr_voterweight[1]=parasettingDlg.m_pLineEdit_linear_weight->text().toInt();
		arr_voterweight[2]=parasettingDlg.m_pLineEdit_histogram_weight->text().toInt();
		n_bin=parasettingDlg.m_pLineEdit_bin->text().toInt();
		K=parasettingDlg.m_pLineEdit_K->text().toInt();
		//refine basic
		if(!parasettingDlg.m_pGroupBox_refinematch->isChecked())
			n_refinemethod=0;
		else if(parasettingDlg.m_pRadioButton_affine->isChecked())
			n_refinemethod=1;
		else if(parasettingDlg.m_pRadioButton_manifold->isChecked())
			n_refinemethod=2;
		//refine adv - affine
		n_sampling=parasettingDlg.m_pLineEdit_nransacsampling->text().toInt();
		n_affineweight=parasettingDlg.m_pLineEdit_affineinvp_weight->text().toInt();
		//refine adv - manifold
		if(parasettingDlg.m_pCheckBox_dir->isChecked())
			arr_constrainter[0]=1;
		else
			arr_constrainter[0]=0;
		if(parasettingDlg.m_pCheckBox_dis->isChecked())
			arr_constrainter[1]=1;
		else
			arr_constrainter[1]=0;
		if(parasettingDlg.m_pCheckBox_topcandidate->isChecked())
			arr_constrainter[2]=1;
		else
			arr_constrainter[2]=0;
		n_iternum=parasettingDlg.m_pLineEdit_maxiternum->text().toInt();
		n_neighbor=parasettingDlg.m_pLineEdit_neighbornum4dirdisavg->text().toInt();
		n_topcandidatenum=parasettingDlg.m_pLineEdit_topcandidatenum->text().toInt();
	}
	else
		return;

	//------------------------------------------------------------------------------------------------------------------------------------
	//read apo files
	QList<CellAPO> ql_cellapo_1,ql_cellapo_2;
	if(qs_filename_1.endsWith(".apo") && qs_filename_2.endsWith(".apo"))
    {
		ql_cellapo_1=readAPO_file(qs_filename_1);
		ql_cellapo_2=readAPO_file(qs_filename_2);
    	printf("\t>>read %d points from [%s]\n",ql_cellapo_1.size(),qPrintable(qs_filename_1));
    	printf("\t>>read %d points from [%s]\n",ql_cellapo_2.size(),qPrintable(qs_filename_2));
    }
    else
    {
    	printf("ERROR: at least one apo file is invalid.\n");
    	QMessageBox::information(parent,title,QObject::tr("Read apo files error!"));
    	return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//reorganize the markers to the format needed by point_cloud_registration.app
	vector<Coord3D_PCM> vec_1,vec_2;
	Coord3D_PCM temp;
	for(int i=0;i<ql_cellapo_1.size();i++)
	{
		temp.x=ql_cellapo_1[i].x;
		temp.y=ql_cellapo_1[i].y;
		temp.z=ql_cellapo_1[i].z;
		vec_1.push_back(temp);
	}
	for(int i=0;i<ql_cellapo_2.size();i++)
	{
		temp.x=ql_cellapo_2[i].x;
		temp.y=ql_cellapo_2[i].y;
		temp.z=ql_cellapo_2[i].z;
		vec_2.push_back(temp);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//run point clouds matching
	vector<int> vec_1to2index;
	vector<Coord3D_PCM> vec_2_invp;
	if(!q_pointcloud_match(vec_1,vec_2,
			n_dismethod,												//initial basic
			arr_voterweight,n_bin,K,									//initial adv
			n_refinemethod,												//refine basic
			n_sampling,n_affineweight,									//refine adv - affine
			arr_constrainter,n_iternum,n_neighbor,n_topcandidatenum,	//refine adv - manifold
			vec_1to2index,												//output
			vec_2_invp))												//output - refine adv - affine
	{
		fprintf(stderr,"ERROR: q_pointcloud_match() return false! \n");
		return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//rereorganize matched point pairs back to QList<CellAPO> (for save back to apo file)
	//rereorganize matched point pairs back to QList<ImageMarker> (for save to marker file)
	QList<CellAPO> ql_cellapo_1_new,ql_cellapo_2_new;
	QList<ImageMarker> ql_marker_1_new,ql_marker_2_new;
	long n_validpair=0;
	for(long i=0;i<vec_1to2index.size();i++)
	{
		if(vec_1to2index[i]!=-1)
		{
			n_validpair++;

			ql_cellapo_1_new.push_back(ql_cellapo_1[i]);
			ql_cellapo_2_new.push_back(ql_cellapo_2[vec_1to2index[i]]);

			ImageMarker temp;
			temp.x=vec_1[i].x;
			temp.y=vec_1[i].y;
			temp.z=vec_1[i].z;
			ql_marker_1_new.push_back(temp);
			temp.x=vec_2[vec_1to2index[i]].x;
			temp.y=vec_2[vec_1to2index[i]].y;
			temp.z=vec_2[vec_1to2index[i]].z;
			ql_marker_2_new.push_back(temp);
		}
	}
	printf(">>point cloud registration complete sucessfully, %d valid pairs finded.\n",n_validpair);

	//------------------------------------------------------------------------------------------------------------------------------------
	//view matching result and choose save options
	QGroupBox *matchResultGroup=new QGroupBox(parent);
	matchResultGroup->setTitle(QObject::tr("Match result"));
	QString qs_result=QString(QObject::tr(">>%1 valid matched pairs found:<br><br>")).arg(int(n_validpair));
	for(long i=0;i<vec_1to2index.size();i++)
	{
		QString temp=QString(QObject::tr("(%1).\tfile1(%2) --> file2(%3)<br>")).arg(i).arg(i).arg(int(vec_1to2index[i]));
		qs_result.append(temp);
	}
	QTextEdit *matchResultText=new QTextEdit(qs_result,matchResultGroup);
	matchResultText->setReadOnly(true);
	matchResultText->setFontPointSize(12);

	QGroupBox *saveOptionGroup=new QGroupBox(parent);
	saveOptionGroup->setTitle(QObject::tr("Save option"));
	QCheckBox *save2apofilesCheckBox=new QCheckBox(saveOptionGroup);
	QCheckBox *save2markerfilesCheckBox=new QCheckBox(saveOptionGroup);
	QCheckBox *save2aposwcfileCheckBox=new QCheckBox(saveOptionGroup);
	QCheckBox *saveinvpB2A2aposwcfileCheckBox=new QCheckBox(saveOptionGroup);
	save2apofilesCheckBox->setText(QObject::tr("save matching results back to apo files"));
	save2markerfilesCheckBox->setText(QObject::tr("save matching results to marker files"));
	save2aposwcfileCheckBox->setText(QObject::tr("save colorized and lined matched pairs to swc and apo files"));
	saveinvpB2A2aposwcfileCheckBox->setText(QObject::tr("save (2->1) inverse projected point set to apo and swc file"));

	QVBoxLayout *matchResultLayout=new QVBoxLayout(matchResultGroup);
	matchResultLayout->addWidget(matchResultText);

	QVBoxLayout *saveOptionLayout=new QVBoxLayout(saveOptionGroup);
	saveOptionLayout->addWidget(save2apofilesCheckBox);
	saveOptionLayout->addWidget(save2markerfilesCheckBox);
	saveOptionLayout->addWidget(save2aposwcfileCheckBox);
	saveOptionLayout->addWidget(saveinvpB2A2aposwcfileCheckBox);

	QHBoxLayout *savecancelLayout=new QHBoxLayout;
	QPushButton* save=new QPushButton("Save");
	QPushButton* cancel=new QPushButton("Cancel");
	savecancelLayout->addWidget(save);
	savecancelLayout->addWidget(cancel);

	QVBoxLayout *mainLayout=new QVBoxLayout;
	mainLayout->addWidget(matchResultGroup);
	mainLayout->addWidget(saveOptionGroup);
	mainLayout->addWidget(new QLabel(QObject::tr("Note: suffix will be added automatically\n(*_img1.marker, *_img2.marker *.swc, *.apo)")));
	mainLayout->addLayout(savecancelLayout);

	QDialog dialog_showsave(parent);
	dialog_showsave.setWindowTitle(QObject::tr("Match Result & Save Option"));
	dialog_showsave.setLayout(mainLayout);
	dialog_showsave.connect(save,  SIGNAL(clicked()),&dialog_showsave,SLOT(accept()));
	dialog_showsave.connect(cancel,SIGNAL(clicked()),&dialog_showsave,SLOT(reject()));

	if(n_refinemethod!=1)//do not use affine refine
		saveinvpB2A2aposwcfileCheckBox->setEnabled(false);

	QStringList qsl_outputinfo;
	qsl_outputinfo.push_back("matching results were saved to:");
	if(dialog_showsave.exec()==QDialog::Accepted)
	{
		QString filename_nosuffix,filename_apo_file1,filename_apo_file2,filename_marker_img1,filename_marker_img2,filename_swc_lined,filename_apo_colored,filename_swc_invp,filename_apo_invp;
		filename_nosuffix=QFileDialog::getSaveFileName(parent,QString(QObject::tr("Only input filename (without suffix)")));
		filename_apo_file1=filename_nosuffix+QString("_file1.apo");
		filename_apo_file2=filename_nosuffix+QString("_file2.apo");
		filename_marker_img1=filename_nosuffix+QString("_1.marker");
		filename_marker_img2=filename_nosuffix+QString("_2.marker");
		filename_swc_lined=filename_nosuffix+QString(".swc");
		filename_apo_colored=filename_nosuffix+QString(".apo");
		filename_swc_invp=filename_nosuffix+QString("_invp.swc");
		filename_apo_invp=filename_nosuffix+QString("_invp.apo");

		if(save2apofilesCheckBox->isChecked())
		{
			writeAPO_file(filename_apo_file1,ql_cellapo_1_new);
			writeAPO_file(filename_apo_file2,ql_cellapo_2_new);
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_apo_file1));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_apo_file2));
		}
		if(save2markerfilesCheckBox->isChecked())
		{
			writeMarker_file(filename_marker_img1,ql_marker_1_new);
			writeMarker_file(filename_marker_img2,ql_marker_2_new);
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_marker_img1));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_marker_img2));
		}
		if(save2aposwcfileCheckBox->isChecked())
		{
			q_export_matches2swc(vec_1,vec_2,vec_1to2index,qPrintable(filename_swc_lined));
			q_export_matches2apo(vec_1,vec_2,vec_2_invp,qPrintable(filename_apo_colored),12);
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_swc_lined));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_apo_colored));
		}
		if(saveinvpB2A2aposwcfileCheckBox->isEnabled() && saveinvpB2A2aposwcfileCheckBox->isChecked())
		{
			q_export_matches2apo(vec_1,vec_2,vec_2_invp,qPrintable(filename_apo_invp),30);
			q_export_matches2swc(vec_1,vec_2_invp,vec_1to2index,qPrintable(filename_swc_invp));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_apo_invp));
			qsl_outputinfo.push_back(QString("[%1]").arg(filename_swc_invp));
		}
	}
	else
		return;

	//display output info dialog
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setText(qsl_outputinfo.join("\n"));
	msgBox.exec();
}

void BatchMatchFromANOFile(V3DPluginCallback &callback, QWidget *parent)
{
	//------------------------------------------------------------------------------------------------------------------------------------
	//choose target file (marker,swc,apo), subject ano file and output dir
	QString qs_filename_tar,qs_filename_ano,qs_pathname_output;
	ChooseFilesDialog_ano d(parent);
	if(d.exec()==QDialog::Accepted)
	{
		qs_filename_tar=d.m_getFilename_1();
		qs_filename_ano=d.m_getFilename_2();
		qs_pathname_output=d.m_getOutputDir();
	}
	else
		return;

	//check the validation of input file and directory
	QDir qdir;
	if(!qdir.exists(qs_filename_tar) || !qdir.exists(qs_filename_ano) || !qdir.exists(qs_pathname_output))
	{
		printf("ERROR: at least one input filename or filepath do not exist!\n");
		QMessageBox::information(parent,title,QObject::tr("At least one input filename or filepath do not exist!"));
		return;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//open para setting dialog
	int n_dismethod;																	//initial basic
	int arr_voterweight[100]={0},n_bin,K;												//initial adv
	int n_refinemethod;																	//refine basic
	int n_sampling,n_affineweight;														//refine adv - affine
	bool arr_constrainter[100]={0};int n_iternum,n_neighbor,n_topcandidatenum;			//refine adv - manifold

	ParaSettingDialog parasettingDlg(parent);
	if(parasettingDlg.exec()==QDialog::Accepted)
	{
		//initial basic
		if(parasettingDlg.m_pRadioButton_euclidian->isChecked())
			n_dismethod=0;
		else if(parasettingDlg.m_pRadioButton_geodesic->isChecked())
			n_dismethod=1;
		//initial adv
		arr_voterweight[0]=parasettingDlg.m_pLineEdit_NN_weight->text().toInt();
		arr_voterweight[1]=parasettingDlg.m_pLineEdit_linear_weight->text().toInt();
		arr_voterweight[2]=parasettingDlg.m_pLineEdit_histogram_weight->text().toInt();
		n_bin=parasettingDlg.m_pLineEdit_bin->text().toInt();
		K=parasettingDlg.m_pLineEdit_K->text().toInt();
		//refine basic
		if(!parasettingDlg.m_pGroupBox_refinematch->isChecked())
			n_refinemethod=0;
		else if(parasettingDlg.m_pRadioButton_affine->isChecked())
			n_refinemethod=1;
		else if(parasettingDlg.m_pRadioButton_manifold->isChecked())
			n_refinemethod=2;
		//refine adv - affine
		n_sampling=parasettingDlg.m_pLineEdit_nransacsampling->text().toInt();
		n_affineweight=parasettingDlg.m_pLineEdit_affineinvp_weight->text().toInt();
		//refine adv - manifold
		if(parasettingDlg.m_pCheckBox_dir->isChecked())
			arr_constrainter[0]=1;
		else
			arr_constrainter[0]=0;
		if(parasettingDlg.m_pCheckBox_dis->isChecked())
			arr_constrainter[1]=1;
		else
			arr_constrainter[1]=0;
		if(parasettingDlg.m_pCheckBox_topcandidate->isChecked())
			arr_constrainter[2]=1;
		else
			arr_constrainter[2]=0;
		n_iternum=parasettingDlg.m_pLineEdit_maxiternum->text().toInt();
		n_neighbor=parasettingDlg.m_pLineEdit_neighbornum4dirdisavg->text().toInt();
		n_topcandidatenum=parasettingDlg.m_pLineEdit_topcandidatenum->text().toInt();
	}
	else
		return;

	//------------------------------------------------------------------------------------------------------------------------------------
	//read target point cloud file and reorganize to the format needed by point_cloud_registration.app
	vector<Coord3D_PCM> vec_1;
	QString qs_suffix_tar=qs_filename_tar.trimmed().toUpper().section('.',-1);

	bool flag=true;
	if(qs_suffix_tar=="MARKER")
	{
		flag=ReadAndFormat4PointCloudMatch_marker(qs_filename_tar,vec_1);
	}
	else if(qs_suffix_tar=="SWC")
	{
		flag=ReadAndFormat4PointCloudMatch_swc(qs_filename_tar,vec_1);
	}
	else if(qs_suffix_tar=="APO")
	{
		flag=ReadAndFormat4PointCloudMatch_apo(qs_filename_tar,vec_1);
	}
	else
	{
    	printf("ERROR: input target file is invalid.\n");
    	QMessageBox::information(parent,title,QObject::tr("Input target point cloud file is invalid!"));
    	return;
	}
	if(!flag)
	{
    	printf("ERROR: read target point cloud error.\n");
    	QMessageBox::information(parent,title,QObject::tr("Read target point cloud error!"));
    	return;
	}

	//read subject apo file
	P_ObjectFileType cc;
	if(!loadAnoFile(qs_filename_ano,cc))
	{
    	printf("ERROR: loadAnoFile return false.\n");
    	QMessageBox::information(parent,title,QObject::tr("Read ano files error!"));
    	return;
	}

	//read subject point cloud --> reorganize --> point cloud match -->output
	QProgressDialog progress("Point clouds batch matching...","Cancel",0,cc.swc_file_list.size(),parent);
	progress.setWindowModality(Qt::WindowModal);
	progress.show();

	vector<Coord3D_PCM> vec_2;
	QStringList qsl_error;

	for(long i=0;i<cc.swc_file_list.size();i++)
	{
		progress.setValue(i);
		progress.setLabelText(QString("Point clouds batch matching [ %1 / %2 ]").arg(i+1).arg(cc.swc_file_list.size()));

		//read and reorganize subject point cloud
		if(!ReadAndFormat4PointCloudMatch_swc(cc.swc_file_list[i],vec_2))
		{
			printf("ERROR: error read subject point cloud from [%s]\n",qPrintable(cc.swc_file_list[i]));
			qsl_error.push_back("Read subject file Error: "+cc.swc_file_list[i]);
			continue;
		}

		//run point clouds matching
		vector<int> vec_1to2index;
		vector<Coord3D_PCM> vec_2_invp;
		if(!q_pointcloud_match(vec_1,vec_2,
				n_dismethod,												//initial basic
				arr_voterweight,n_bin,K,									//initial adv
				n_refinemethod,												//refine basic
				n_sampling,n_affineweight,									//refine adv - affine
				arr_constrainter,n_iternum,n_neighbor,n_topcandidatenum,	//refine adv - manifold
				vec_1to2index,												//output
				vec_2_invp))												//output - refine adv - affine
		{
			fprintf(stderr,"ERROR: q_pointcloud_match() return false! \n");
			return;
		}

		//write matching results to given directory
		QString qs_filename_linedpair_swc,qs_filename_coloredpair_apo,qs_filename_invp_swc,qs_filename_invp_apo;
		QString qs_basename=QFileInfo(cc.swc_file_list[i]).baseName();
		qs_filename_coloredpair_apo=qs_pathname_output+'/'+qs_basename+"_mpair.apo";
		qs_filename_linedpair_swc=qs_pathname_output+'/'+qs_basename+"_mpair.swc";
		qs_filename_invp_apo=qs_pathname_output+'/'+qs_basename+"_invp.apo";
		qs_filename_invp_swc=qs_pathname_output+'/'+qs_basename+"_invp.swc";
		if(d.m_pCheckBox_save2aposwcfile->isChecked())
		{
			q_export_matches2swc(vec_1,vec_2,vec_1to2index,qPrintable(qs_filename_linedpair_swc));
			q_export_matches2apo(vec_1,vec_2,vec_2_invp,qPrintable(qs_filename_coloredpair_apo),12);
		}
		if(d.m_pCheckBox_saveinvpB2A2aposwcfile->isChecked())
		{
			q_export_matches2apo(vec_1,vec_2,vec_2_invp,qPrintable(qs_filename_invp_apo),30);
			q_export_matches2swc(vec_1,vec_2_invp,vec_1to2index,qPrintable(qs_filename_invp_swc));
		}


		if(progress.wasCanceled())
			break;
	}
	progress.setValue(cc.swc_file_list.size());

	return;
}


//************************************************************************************************************************************
bool ReadAndFormat4PointCloudMatch_marker(QString qs_filename_input,vector<Coord3D_PCM> &vec_pointcloud4match)
{
	//read marker file
	QList<ImageMarker> ql_marker_input;
	ql_marker_input=readMarker_file(qs_filename_input);
	printf("\t>>read %d markers from [%s]\n",ql_marker_input.size(),qPrintable(qs_filename_input));
	if(ql_marker_input.size()<=0)
	{
		printf("ERROR: input marker file is empty.\n");
		return false;
	}

	//reorganize the input markers to the format needed by point_cloud_registration.app
	vec_pointcloud4match.clear();
	for(long i=0;i<ql_marker_input.size();i++)
	{
		vec_pointcloud4match.push_back(Coord3D_PCM(ql_marker_input[i].x,ql_marker_input[i].y,ql_marker_input[i].z));
	}

	return true;
}
bool ReadAndFormat4PointCloudMatch_swc(QString qs_filename_input,vector<Coord3D_PCM> &vec_pointcloud4match)
{
	//read swc file
	NeuronTree nt_input;
	nt_input=readSWC_file(qs_filename_input);
	printf("\t>>read %d points from [%s]\n",nt_input.listNeuron.size(),qPrintable(qs_filename_input));
	if(nt_input.listNeuron.size()<=0)
    {
    	printf("ERROR: input swc file is empty.\n");
    	return false;
	}

	//reorganize the input markers to the format needed by point_cloud_registration.app
	Coord3D_PCM temp;
	vec_pointcloud4match.clear();
	for(int i=0;i<nt_input.listNeuron.size();i++)
	{
		temp.x=nt_input.listNeuron.at(i).x;
		temp.y=nt_input.listNeuron.at(i).y;
		temp.z=nt_input.listNeuron.at(i).z;
		vec_pointcloud4match.push_back(temp);
	}

	return true;
}
bool ReadAndFormat4PointCloudMatch_apo(QString qs_filename_input,vector<Coord3D_PCM> &vec_pointcloud4match)
{
	//read apo file
	QList<CellAPO> ql_cellapo_input;
	ql_cellapo_input=readAPO_file(qs_filename_input);
	printf("\t>>read %d points from [%s]\n",ql_cellapo_input.size(),qPrintable(qs_filename_input));
    if(ql_cellapo_input.size()<=0)
    {
    	printf("ERROR: input apo file is empty.\n");
    	return false;
	}

	//reorganize the markers to the format needed by point_cloud_registration.app
	Coord3D_PCM temp;
	vec_pointcloud4match.clear();
	for(int i=0;i<ql_cellapo_input.size();i++)
	{
		temp.x=ql_cellapo_input[i].x;
		temp.y=ql_cellapo_input[i].y;
		temp.z=ql_cellapo_input[i].z;
		vec_pointcloud4match.push_back(temp);
	}

	return true;
}
