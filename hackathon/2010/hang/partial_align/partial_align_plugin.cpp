/**
*  100811 RZC: change to handle any type image using  Image4DProxy's value_at/put_at
**/
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <vector>

#include <QtGui>

#include "partial_align_plugin.h"
#include "partial_align_gui.h"

#include "myfeature.h"
#include "edge_detection.h"
using namespace std;

int calc_feature(V3DPluginCallback2 &callback, QWidget *parent, MyFeatureType type);
int compute_gaussian(V3DPluginCallback2 &callback, QWidget *parent);
int compute_threshold(V3DPluginCallback2 &callback, QWidget *parent);
int compute_edge_map(V3DPluginCallback2 &callback, QWidget *parent);
int compute_edge_grid(V3DPluginCallback2 &callback, QWidget *parent);
int compare_feature(V3DPluginCallback2 &callback, QWidget *parent);


Q_EXPORT_PLUGIN2(partial_align, PartialAlignPlugin);

const QString title = "Partial Alignment Plugin";

QStringList PartialAlignPlugin::menulist() const
{
	return QStringList() 
		<< tr("calc average feature")
		<< tr("calc standard variance feature")
		<< tr("calc invariant moments feature")
		<< tr("calc SIFT feature")
		<< tr("compute gaussian filtering")
		<< tr("compute threshold")
		<< tr("compute edge map")
		<< tr("compute edge grid")
		<< tr("partial alignment")
		<< tr("about");
}

void PartialAlignPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("calc average feature"))
	{
		calc_feature(callback,parent,AVERAGE_FEATURE);
	}
	else if(menu_name == tr("calc standard variance feature"))
	{
		calc_feature(callback,parent,STD_VAR_FEATURE);
	}
	else if(menu_name == tr("calc invariant moments feature"))
	{
		calc_feature(callback,parent,INVARIANT_MOMENT_FEATURE);
	}
	else if(menu_name == tr("calc SIFT feature"))
	{
		calc_feature(callback,parent,SIFT_FEATURE);
	}
	else if(menu_name == tr("compute gaussian filtering"))
	{
		compute_gaussian(callback, parent);
	}
	else if(menu_name == tr("compute threshold"))
	{
		compute_threshold(callback, parent);
	}
	else if(menu_name == tr("compute edge map"))
	{
		compute_edge_map(callback,parent);
	}
	else if(menu_name == tr("compute edge grid"))
	{
		compute_edge_grid(callback,parent);
	}
	else if(menu_name == tr("partial alignment"))
	{
		compare_feature(callback,parent);
	}
	else
	{
		v3d_msg("Partial Alignment Plugin Demo version 1.0"
				"\ndeveloped by Hang Xiao 2011. (Janelia Farm Research Campus, HHMI)");
	}

}

int calc_feature(V3DPluginCallback2 &callback, QWidget *parent, MyFeatureType type)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}

	PartialAlignDialog dialog(callback, parent);
	dialog.combo_subject->setDisabled(true);
	dialog.channel_sub->setDisabled(true);

	if (dialog.exec()!=QDialog::Accepted) return -1;

	dialog.update();
	int i1 = dialog.combo_subject->currentIndex();
	int i2 = dialog.combo_target->currentIndex();

	V3DLONG sub_c = dialog.sub_c - 1;
	V3DLONG tar_c = dialog.tar_c - 1;

	Image4DSimple *image = callback.getImage(win_list[i2]);
	
	if(image->getCDim() < dialog.tar_c) {QMessageBox::information(0, title, QObject::tr("The channel isn't existed.")); return -1;}

	V3DLONG sz[3];
	sz[0] = image->getXDim();
	sz[1] = image->getYDim();
	sz[2] = image->getZDim();

	unsigned char *data1d = image->getRawDataAtChannel(tar_c);

	LandmarkList landmarks = callback.getLandmark(win_list[i2]);
	if(landmarks.empty()) {v3d_msg("No mark loaded"); return -1;}

	MyFeature<double> myfeature;
	myfeature.setFeatures(landmarks, data1d, sz, type);
	myfeature.printFeatures();

}

int compute_gaussian(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}

	PartialAlignDialog dialog(callback, parent);
	dialog.combo_subject->setDisabled(true);
	dialog.channel_sub->setDisabled(true);

	if (dialog.exec()!=QDialog::Accepted) return -1;

	dialog.update();
	int i1 = dialog.combo_subject->currentIndex();
	int i2 = dialog.combo_target->currentIndex();

	V3DLONG sub_c = dialog.sub_c - 1;
	V3DLONG tar_c = dialog.tar_c - 1;
	
	bool ok;
	double sigma;
	int r;
	sigma =  QInputDialog::getDouble(parent, QObject::tr("Gaussian Filter"),
			QObject::tr("sigma :"),
			1.0, 0.01, 100.0, 1, &ok);
	if(ok)
	{
		r = QInputDialog::getInt(parent, QObject::tr("Gaussian Filter"), QObject::tr("radius :"), 3, 3, 7, 1, &ok);
	}

	Image4DSimple *image = callback.getImage(win_list[i2]);

	if(image->getCDim() < dialog.tar_c) {QMessageBox::information(0, title, QObject::tr("The channel isn't existed.")); return -1;}

	V3DLONG sz[3];
	sz[0] = image->getXDim();
	sz[1] = image->getYDim();
	sz[2] = image->getZDim();

	unsigned char *inimg1d = image->getRawDataAtChannel(tar_c);

	unsigned char * outimg1d = 0;

	computeGaussian(outimg1d, inimg1d, sz, sigma, r);

	Image4DSimple* p4DImage = new Image4DSimple();

	p4DImage->setData((unsigned char*)outimg1d, sz[0], sz[1], sz[2], 1, image->getDatatype());

	v3dhandle newwin;
	if(QMessageBox::Yes == QMessageBox::question(0, "", QString("Do you want to use the existing windows?"), QMessageBox::Yes, QMessageBox::No))
		newwin = callback.currentImageWindow();
	else
		newwin = callback.newImageWindow();

	callback.setImage(newwin, p4DImage);
	callback.setImageName(newwin, QObject::tr("Gaussian Image sigma = %1, radius = %2").arg(sigma).arg(r));
	callback.updateImageWindow(newwin);

	return true;
}

int compute_threshold(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}

	PartialAlignDialog dialog(callback, parent);
	dialog.combo_subject->setDisabled(true);
	dialog.channel_sub->setDisabled(true);

	if (dialog.exec()!=QDialog::Accepted) return -1;

	dialog.update();
	int i1 = dialog.combo_subject->currentIndex();
	int i2 = dialog.combo_target->currentIndex();

	V3DLONG sub_c = dialog.sub_c - 1;
	V3DLONG tar_c = dialog.tar_c - 1;
	
	bool ok;
	double theta;
	theta =  QInputDialog::getDouble(parent, QObject::tr("Threshold"),
			QObject::tr("value :"),
			20.0, 0.0, 256.0, 1, &ok);

	Image4DSimple *image = callback.getImage(win_list[i2]);

	if(image->getCDim() < dialog.tar_c) {QMessageBox::information(0, title, QObject::tr("The channel isn't existed.")); return -1;}

	V3DLONG sz[3];
	sz[0] = image->getXDim();
	sz[1] = image->getYDim();
	sz[2] = image->getZDim();

	unsigned char *inimg1d = image->getRawDataAtChannel(tar_c);

	unsigned char * outimg1d = 0;

	computeThreshold(outimg1d, inimg1d, sz, theta);

	Image4DSimple* p4DImage = new Image4DSimple();

	p4DImage->setData((unsigned char*)outimg1d, sz[0], sz[1], sz[2], 1, image->getDatatype());

	v3dhandle newwin;
	if(QMessageBox::Yes == QMessageBox::question(0, "", QString("Do you want to use the existing windows?"), QMessageBox::Yes, QMessageBox::No))
		newwin = callback.currentImageWindow();
	else
		newwin = callback.newImageWindow();

	callback.setImage(newwin, p4DImage);
	callback.setImageName(newwin, QObject::tr("Threshold = %1").arg(theta));
	callback.updateImageWindow(newwin);

	return true;
}

int compute_edge_map(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}

	PartialAlignDialog dialog(callback, parent);
	dialog.combo_subject->setDisabled(true);
	dialog.channel_sub->setDisabled(true);

	if (dialog.exec()!=QDialog::Accepted) return -1;

	dialog.update();
	int i1 = dialog.combo_subject->currentIndex();
	int i2 = dialog.combo_target->currentIndex();

	V3DLONG sub_c = dialog.sub_c - 1;
	V3DLONG tar_c = dialog.tar_c - 1;
	
	Image4DSimple *image = callback.getImage(win_list[i2]);

	if(image->getCDim() < dialog.tar_c) {QMessageBox::information(0, title, QObject::tr("The channel isn't existed.")); return -1;}

	V3DLONG sz[3];
	sz[0] = image->getXDim();
	sz[1] = image->getYDim();
	sz[2] = image->getZDim();

	unsigned char *inimg1d = image->getRawDataAtChannel(tar_c);

	unsigned char * outimg1d = 0;

	//computeGaussian(outimg1d, inimg1d, sz, sigma, r);
	//inimg1d = outimg1d;
	computeGradience(outimg1d, inimg1d, sz, 0.0f);

	Image4DSimple* p4DImage = new Image4DSimple();

	p4DImage->setData((unsigned char*)outimg1d, sz[0], sz[1], sz[2], 1, image->getDatatype());

	v3dhandle newwin;
	if(QMessageBox::Yes == QMessageBox::question(0, "", QString("Do you want to use the existing windows?"), QMessageBox::Yes, QMessageBox::No))
		newwin = callback.currentImageWindow();
	else
		newwin = callback.newImageWindow();

	callback.setImage(newwin, p4DImage);
	callback.setImageName(newwin, QObject::tr("Image Gradient"));
	callback.updateImageWindow(newwin);

	return true;

}

int compute_edge_grid(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}

	PartialAlignDialog dialog(callback, parent);
	dialog.combo_subject->setDisabled(true);
	dialog.channel_sub->setDisabled(true);

	if (dialog.exec()!=QDialog::Accepted) return -1;

	dialog.update();
	int i1 = dialog.combo_subject->currentIndex();
	int i2 = dialog.combo_target->currentIndex();

	V3DLONG sub_c = dialog.sub_c - 1;
	V3DLONG tar_c = dialog.tar_c - 1;
	
	Image4DSimple *image = callback.getImage(win_list[i2]);

	if(image->getCDim() < dialog.tar_c) {QMessageBox::information(0, title, QObject::tr("The channel isn't existed.")); return -1;}

	V3DLONG sz[3];
	sz[0] = image->getXDim();
	sz[1] = image->getYDim();
	sz[2] = image->getZDim();

	V3DLONG gsz[3];
	gsz[0] = 10;
	gsz[1] = 10;
	gsz[2] = 10;

	unsigned char *inimg1d = image->getRawDataAtChannel(tar_c);

	V3DLONG * grids[3];
	V3DLONG gridsnum;
	if(computeEdgeGrid(grids,gridsnum,gsz,inimg1d, sz, 1.0, 3, 20) == -1)
	{
		v3d_msg("Compute Edge Grid error!");
		return -1;
	}

	LandmarkList newmarks;
	for(V3DLONG i = 0; i < gridsnum; i++)
		newmarks.push_back(LocationSimple(grids[0][i], grids[1][i], grids[2][i]));

	v3dhandle curwin = callback.currentImageWindow();
	callback.setLandmark(curwin, newmarks);
	callback.updateImageWindow(curwin);

	return true;

}

int compare_feature(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}

	PartialAlignDialog dialog(callback, parent);

	if (dialog.exec()!=QDialog::Accepted) return -1;

	dialog.update();
	int i1 = dialog.combo_subject->currentIndex();
	int i2 = dialog.combo_target->currentIndex();

	V3DLONG sub_c = dialog.sub_c - 1;
	V3DLONG tar_c = dialog.tar_c - 1;
	
	Image4DSimple *sub_img = callback.getImage(win_list[i1]);
	Image4DSimple *tar_img = callback.getImage(win_list[i2]);

	if(sub_img->getCDim() < dialog.sub_c) {QMessageBox::information(0, title, QObject::tr("The channel of subject isn't existed.")); return -1;}
	if(tar_img->getCDim() < dialog.tar_c) {QMessageBox::information(0, title, QObject::tr("The channel of target isn't existed.")); return -1;}

	if(sub_img->getXDim() != tar_img->getXDim() || sub_img->getYDim() != tar_img->getYDim() || sub_img->getZDim() != tar_img->getZDim())
	{ QMessageBox::information(0, title, QObject::tr("The size of the two images are different!")); return -1;}

	LandmarkList sub_marks; // = callback.getLandmark(win_list[i1]);
	LandmarkList tar_marks = callback.getLandmark(win_list[i2]);
	LandmarkList result_marks;
	if(tar_marks.empty()) {v3d_msg("No marks in target image"); return -1;}

	V3DLONG sz[3];
	sz[0] = sub_img->getXDim();
	sz[1] = sub_img->getYDim();
	sz[2] = sub_img->getZDim();

	V3DLONG gsz[3];
	gsz[0] = 10;
	gsz[1] = 10;
	gsz[2] = 10;

	unsigned char *sub_inimg1d = sub_img->getRawDataAtChannel(sub_c);
	unsigned char *tar_inimg1d = sub_img->getRawDataAtChannel(tar_c);

	V3DLONG * grids[3];
	V3DLONG gridsnum;
	if(computeEdgeGrid(grids, gridsnum, gsz, sub_inimg1d, sz, 1.0, 3, 20) == -1)
	{
		v3d_msg("Error when compute edge grids!");
		return -1;
	}

	for(V3DLONG i = 0; i < gridsnum; i++)
		sub_marks.push_back(LocationSimple(grids[0][i], grids[1][i], grids[2][i]));

	MyFeature<double> sub_feat, tar_feat;

	sub_feat.setFeatures(sub_marks, sub_inimg1d, sz , INVARIANT_MOMENT_FEATURE);
	tar_feat.setFeatures(tar_marks, tar_inimg1d, sz , INVARIANT_MOMENT_FEATURE);

	V3DLONG * match_list = 0;
	V3DLONG match_sz = tar_feat.size();
	cout<<"sub_markers : "<<sub_marks.size()<<" match_sz = "<<match_sz<<endl;
	if(get_matched_feature_list(match_list, match_sz, sub_feat, tar_feat) == -1)
	{
		v3d_msg("error when match features");
		return -1;
	}
	cout<<"match list : "<<endl;
	for(V3DLONG i = 0; i < match_sz; i++)
	{
		cout<<match_list[i]<<" ";
		result_marks.push_back(sub_marks[match_list[i]]);
	}
	cout<<endl;

	callback.setLandmark(win_list[i1], result_marks);
	callback.updateImageWindow(win_list[i1]);

	if(match_list) {delete[] match_list; match_list = 0;}
	if(grids[0]) { delete[] grids[0]; grids[0] = 0;}
	if(grids[1]) { delete[] grids[1]; grids[1] = 0;}
	if(grids[2]) { delete[] grids[2]; grids[2] = 0;}
	return true;


}

