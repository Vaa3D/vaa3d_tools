/* snake_tracing_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-04-01 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "snake_tracing_plugin.h"

#include "TracingCore/OpenSnakeTracer.h"
#include "TracingCore/ImageOperation.h"
#include "TracingCore/SnakeOperation.h"
//#include "ParametersGroup.h"

//#include "PointOperation.h"
//#include "TracingCore/ImageOperation.h"

void autotrace(V3DPluginCallback2 &callback, QWidget *parent);

using namespace std;
Q_EXPORT_PLUGIN2(snake_tracing, snake_tracing);
 
QStringList snake_tracing::menulist() const
{
	return QStringList() 
        <<tr("trace")
		<<tr("about");
}

QStringList snake_tracing::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void snake_tracing::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("trace"))
	{
        autotrace(callback,parent);
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by Zhi Zhou, 2014-04-01"));
	}
}

bool snake_tracing::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

void autotrace(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    //Mc_Stack *stack = NVInterface::makeStack(p4DImage);

    unsigned char* data1d = p4DImage->getRawData();
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    int in_sz[3];
    in_sz[0] = N;
    in_sz[1] = M;
    in_sz[2] = P;


    bool ok1;
    int c,p = 0;

    if(sc==1)
    {
        c=1;
        ok1=true;
    }
    else
    {
        c = QInputDialog::getInteger(parent, "Channel",
                                         "Enter channel NO:",
                                         1, 1, sc, 1, &ok1);
    }

    if(!ok1)
        return;

    if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Merge Close Nodes?"), QMessageBox::Yes, QMessageBox::No))    p = 1;
    //read image
    ImageOperation *IM;
    OpenSnakeTracer *Tracer;

    IM = new ImageOperation;
    Tracer = new OpenSnakeTracer;

   // IM->ImRead(callback.getImageName(curwin).toStdString().c_str());
    IM->Imcreate(data1d,in_sz);

    //preprocessing
    std::cout<<"Compute Gradient Vector Flow..."<<std::endl;
    IM->computeGVF(1000,5,0);
    std::cout<<"Compute Vesselness (CPU)..."<<std::endl;
    IM->ComputeGVFVesselness();
    std::cout<<"Detect Seed Points..."<<std::endl;
    IM->SeedDetection(IM->v_threshold,0,0);
    std::cout<<"Adjust Seed Points..."<<std::endl;
    IM->SeedAdjustment(10);
    std::cout<<"Preprocessing Finished..."<<std::endl;

    IM->ImComputeInitBackgroundModel(IM->v_threshold);
    IM->ImComputeInitForegroundModel();

    //tracing
    std::cout<<"--------------Tracing--------------"<<std::endl;
    IM->ImRefresh_LabelImage();
    Tracer->SetImage(IM);
    Tracer->Init();
   // Tracer->Open_Curve_Snake_Tracing();

    v3d_msg("done!");
}
