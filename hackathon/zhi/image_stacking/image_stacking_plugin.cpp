/* image_stacking_plugin.cpp
 * Stack two images
 * 2014-01-08 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "image_stacking_plugin.h"
#include "basic_surf_objs.h"
using namespace std;
Q_EXPORT_PLUGIN2(image_stacking, image_stacking);
void stack_pair(V3DPluginCallback2 &callback, QWidget *parent);
NeuronTree markers2swc(LandmarkList listLandmarks);
controlPanel* controlPanel::panel = 0;
 
QStringList image_stacking::menulist() const
{
	return QStringList() 
        <<tr("istack")
		<<tr("about");
}

QStringList image_stacking::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void image_stacking::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("istack"))
	{
        stack_pair(callback,parent);
	}
	else
	{
		v3d_msg(tr("Stack two images. "
			"Developed by Zhi Zhou, 2014-01-08"));
	}
}

bool image_stacking::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

void stack_pair(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandleList win_list = callback.getImageWindowList();
    if (win_list.size()<2)
    {
        v3d_msg("You need at least two opened images to stack!");
        return;
    }

    if (controlPanel::panel)
    {
        controlPanel::panel->show();
        return;
    }
    else
    {
        controlPanel* p = new controlPanel(callback, parent);
        if (p)
        {
            p->show();
            p->raise();
            p->move(100,100);
            p->activateWindow();
        }
    }
}

controlPanel::controlPanel(V3DPluginCallback2 &_v3d, QWidget *parent) :
    QDialog(parent), m_v3d(_v3d)
{
    win_list = m_v3d.getImageWindowList();
    QStringList items;
    for (int i=0; i<win_list.size(); i++)
        items << m_v3d.getImageName(win_list[i]);

    combo_subject = new QComboBox(); combo_subject->addItems(items);
    combo_target = new QComboBox(); combo_target->addItems(items);

    label_subject = new QLabel(QObject::tr("Subject image: "));
    label_target = new QLabel(QObject::tr("Target image: "));

    QPushButton* stack     = new QPushButton("stack");

    gridLayout = new QGridLayout();
    gridLayout->addWidget(label_subject, 1,0,1,2);
    gridLayout->addWidget(combo_subject,1,2,1,2);
    gridLayout->addWidget(label_target, 2,0,1,2);
    gridLayout->addWidget(combo_target,2,2,1,2);
    gridLayout->addWidget(stack, 3,3);

    setLayout(gridLayout);
    setWindowTitle(QString("Stack two images"));

    connect(stack,     SIGNAL(clicked()), this, SLOT(_slot_stack()));
}

controlPanel::~controlPanel()
{
     if(panel){delete panel; panel=0;}

}

void controlPanel::_slot_stack()
{
    int sub_index = combo_subject->currentIndex();
    int tar_index = combo_target->currentIndex();

    if (sub_index == tar_index)
    {
        v3d_msg("You have selected the same image. You need to specify and stack two different images. Try again!");
        return;
    }

    v3dhandle sub_win = win_list[sub_index];
    v3dhandle tar_win = win_list[tar_index];

    LandmarkList sub_listLandmarks = m_v3d.getLandmark(sub_win);
    LandmarkList tar_listLandmarks = m_v3d.getLandmark(tar_win);

    if(sub_listLandmarks.count() < 5 || tar_listLandmarks.count() < 5)
    {
        v3d_msg("Please select at least 5 markers for both subject and target images!");
        return;
    }

    NeuronTree sub_swc = markers2swc(sub_listLandmarks);
    NeuronTree tar_swc = markers2swc(tar_listLandmarks);

    writeSWC_file("sub_image.swc",sub_swc);
    writeSWC_file("tar_image.swc",tar_swc);

    system("/local1/work/yinan/pointcloud_match/main_pointcloud_match -t tar_image.swc -s sub_image.swc -o swc_inv.swc");
    QString sub_name = m_v3d.getImageName(win_list[sub_index]);
    QString cmd_affine = QString("/local1/work/vaa3d_tools/hackathon/affine_transform/main_affinetransform -t swc_inv.swc -s sub_image.swc -S %1 -o sub_tar.v3draw").arg(sub_name.toStdString().c_str());
    system(qPrintable(cmd_affine));

    Image4DSimple* p4DImage_tar = m_v3d.getImage(tar_win);

    if (!p4DImage_tar)
    {
        v3d_msg("The target image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    unsigned char* tar_image = p4DImage_tar->getRawData();
    V3DLONG tar_pagesz = p4DImage_tar->getTotalUnitNumberPerChannel();

    V3DLONG tar_N = p4DImage_tar->getXDim();
    V3DLONG tar_M = p4DImage_tar->getYDim();
    V3DLONG tar_P = p4DImage_tar->getZDim();
    V3DLONG tar_sc = p4DImage_tar->getCDim();
    ImagePixelType tar_pixeltype = p4DImage_tar->getDatatype();

    unsigned char * sub_image = 0;
    int sub_datatype;
    V3DLONG sub_size[4];
    simple_loadimage_wrapper(m_v3d, "sub_tar.v3draw", sub_image, sub_size, sub_datatype);

    V3DLONG stack_N, stack_M, stack_P;
    if(tar_N > sub_size[0]) stack_N = tar_N; else stack_N = sub_size[0];
    if(tar_M > sub_size[1]) stack_M = tar_M; else stack_M = sub_size[1];
    stack_P = tar_P + sub_size[2];

    V3DLONG stack_pagesz = stack_N * stack_M * stack_P;
    unsigned char* stacked_image = 0;
    try {stacked_image = new unsigned char [stack_pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for stacked image."); return;}
    memset(stacked_image, 0, sizeof(unsigned char)*stack_pagesz);

    for(V3DLONG iz = 0; iz < sub_size[2]; iz++)
    {
        V3DLONG offsetk = iz*sub_size[1]*sub_size[0];
        for(V3DLONG iy = 0; iy <  sub_size[1]; iy++)
        {
            V3DLONG offsetj = iy*sub_size[0];
            for(V3DLONG ix = 0; ix < sub_size[0]; ix++)
            {
                   stacked_image[offsetk + offsetj + ix] = sub_image[offsetk + offsetj + ix];
            }

        }
    }

    V3DLONG i = 0;
    for(V3DLONG iz = sub_size[2]; iz < stack_P; iz++)
    {
        V3DLONG offsetk = iz*stack_N*stack_M;
        for(V3DLONG iy = 0; iy <  tar_M; iy++)
        {
            V3DLONG offsetj = iy*stack_N;
            for(V3DLONG ix = 0; ix < tar_N; ix++)
            {
                   stacked_image[offsetk + offsetj + ix] = tar_image[i];
                   i++;
            }

        }
    }

    V3DLONG in_sz[4];
    in_sz[0] = stack_N;
    in_sz[1] = stack_M;
    in_sz[2] = stack_P;
    in_sz[3] = 1;
    simple_saveimage_wrapper(m_v3d,"stacked_image.v3draw", (unsigned char *)stacked_image, in_sz, 1);
    if(stacked_image) {delete stacked_image; stacked_image = 0;}
    if(sub_image){delete sub_image; sub_image = 0;}
    v3d_msg("done!!");
    return;

}

NeuronTree markers2swc(LandmarkList listLandmarks)
{
    //NeutronTree structure
    NeuronTree marker_swc;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    //set node
    for(int i = 0; i < listLandmarks.count(); i++)
    {
        NeuronSWC S;
        S.n 	= i;
        S.type 	= 2;
        S.x 	= listLandmarks[i].x-1;
        S.y 	= listLandmarks[i].y-1;
        S.z 	= listLandmarks[i].z-1;
        S.r 	= 1;
        S.pn 	= -1;
        listNeuron.append(S);
        hashNeuron.insert(S.n, listNeuron.size()-1);
    }

    marker_swc.n = -1;
    marker_swc.on = true;
    marker_swc.listNeuron = listNeuron;
    marker_swc.hashNeuron = hashNeuron;

    return marker_swc;
}
