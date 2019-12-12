/* findCrossBranch_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-11-5 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "findCrossBranch_plugin.h"
#include "iostream"
#include "findCrossBranch.h"
using namespace std;
Q_EXPORT_PLUGIN2(findCrossBranch, TestPlugin);
 
bool get_sub_terafly(V3DPluginCallback2 &callback,
	QString inimg_file,
	QString outimg_file,
	V3DLONG x0, V3DLONG x1, V3DLONG y0, V3DLONG y1, V3DLONG z0, V3DLONG z1);
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-11-5"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		NeuronTree nt1 = readSWC_file(infiles[0]);
		cout << infiles[0] << endl;
		cout << inparas[0] << endl;

		SwcTree st;
		st.filePath = infiles[0];
		st.nt = nt1;
		st.searchRadius = atoi(inparas[0]);
		cout << "st.searchRadius: " << st.searchRadius << endl;

		int crossType1, crossType2;
		crossType1 = 0;
		crossType2 = 0;
		st.initialize();
		int crossPointsNum = st.crossBranchDetect(crossType1, crossType2);


		NeuronTree ntNoneCrossBranch;
		ntNoneCrossBranch.listNeuron.clear();
		ntNoneCrossBranch.hashNeuron.clear();

		st.crossBranchDeal(ntNoneCrossBranch);
		QString eswcfile = (outfiles.size() >= 1) ? outfiles[0] : "";
		QString markerfile = (outfiles.size() >= 2) ? outfiles[1] : "";
		
		
		markerfile = markerfile + "{" + QString::number(crossType1)+"_" + QString::number(crossType2) + "_" + QString::number(crossPointsNum)+"}" + ".marker";
		//writeESWC_file(eswcfile, st.nt);
		writeESWC_file(eswcfile + "delete.eswc", st.nt);
		writeESWC_file(eswcfile, ntNoneCrossBranch);
		writeMarker_file(markerfile, st.listMarker);

		
		

	}
	else if (func_name == tr("cutSwc2Blocks"))
	{
		float Rx, Ry, Rz, scale;
		V3DLONG swcNum;
		Rx = atoi(inparas[0]);
		Ry = atoi(inparas[1]);
		Rz = atoi(inparas[2]);
		scale = atoi(inparas[3]);
		swcNum = atoi(inparas[4]);
		cout << Rx << " " << Ry << " " << Rz << " " << scale << endl;



		QString inimg_file = infiles[0];
		
		//st.filePath = infiles[0];
		//st.nt = nt1;
		//st.initialize();
		//st.nt_copy.deepCopy(st.nt);
		NeuronTree resultSwc;
		float xC, yC, zC;
		for (int i = 1; i <= swcNum; i++){
			cout << "wp_debug: " << __LINE__ << ": " << i << endl;
			//NeuronTree nt1 = readSWC_file(infiles[i]);
			SwcTree st;
			NeuronTree nt1 = readSWC_file(infiles[i]);
			st.nt = nt1;
			if (i == 1){
				xC = st.nt.listNeuron[0].x;
				yC = st.nt.listNeuron[0].y;
				zC = st.nt.listNeuron[0].z;
				st.cutSwcAroundSoma(xC, yC, zC, Rx, Ry, Rz, scale, resultSwc);
				st.sort_swc(resultSwc);
			}
			else{
				NeuronTree tmp;
				st.cutSwcAroundSoma(xC, yC, zC, Rx, Ry, Rz, scale, tmp);
				st.sort_swc(tmp);
				st.add_swc(resultSwc, tmp);
			}
			
		}

		QString outswcfile = (outfiles.size() >= 1) ? outfiles[0] : "";
		QString outimagefile = (outfiles.size() >= 1) ? outfiles[1] : "";

		//QString outswcfile = (outfiles.size() >= 1) ? outfiles[0] : "";
		
		
		get_sub_terafly(callback, inimg_file, outimagefile, 
			V3DLONG(resultSwc.listNeuron[0].x - Rx), 
			V3DLONG(resultSwc.listNeuron[0].x + Rx),
			V3DLONG(resultSwc.listNeuron[0].y - Ry),
			V3DLONG(resultSwc.listNeuron[0].y + Ry),
			V3DLONG(resultSwc.listNeuron[0].z - Rz),
			V3DLONG(resultSwc.listNeuron[0].z + Rz));

		cout << resultSwc.listNeuron[0].x << endl;

		//float xC, yC, zC;
		xC = resultSwc.listNeuron[0].x;
		yC = resultSwc.listNeuron[0].y;
		zC = resultSwc.listNeuron[0].z;

		//Æ«ÖÃswc
		for (int i = 0; i < resultSwc.listNeuron.size(); i++){
			//cout << "be: " << resultSwc.listNeuron[i].x << endl;
			resultSwc.listNeuron[i].x = resultSwc.listNeuron[i].x - xC + Rx;
			resultSwc.listNeuron[i].y = resultSwc.listNeuron[i].y - yC + Ry;
			resultSwc.listNeuron[i].z = resultSwc.listNeuron[i].z - zC + Rz;
			//cout << resultSwc.listNeuron[i].x << endl;
		}
		writeSWC_file(outswcfile, resultSwc);
		
		//v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

bool get_sub_terafly(V3DPluginCallback2 &callback, 
	QString inimg_file, 
	QString outimg_file, 
	V3DLONG x0, V3DLONG x1, V3DLONG y0, V3DLONG y1, V3DLONG z0, V3DLONG z1)
{
	
	V3DLONG pagesz;
	pagesz = (x1 - x0 +1)*(y1 - y0 +1)*(z1 - z0 +1);
	V3DLONG im_cropped_sz[4];
	im_cropped_sz[0] = x1 - x0 + 1;
	im_cropped_sz[1] = y1 - y0 + 1;
	im_cropped_sz[2] = z1 - z0 + 1;
	im_cropped_sz[3] = 1;

	unsigned char * im_cropped = 0;


	try { im_cropped = new unsigned char[pagesz]; }
	catch (...)  { cout << "cannot allocate memory for image_mip." << endl; return false; }

	im_cropped = callback.getSubVolumeTeraFly(inimg_file.toStdString(), x0, x1 + 1,
		y0, y1 + 1, z0, z1 + 1);

	simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(), (unsigned char *)im_cropped, im_cropped_sz, 1);

}