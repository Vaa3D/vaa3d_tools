/* mostVesselTracer.cpp

   Copyright 2010 Jingpeng Wu

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
   
 * the plugin for most vessel tracing
 * by jpwu@CBMP, 20100419
 * tojingpeng@gmail.com
 */


#include "mostVesselTracer.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(mostVesselTracer, mostVesselTracerPlugin);

//plugin funcs
const QString title = "Trace vessels of MOST volume!";
QStringList mostVesselTracerPlugin::menulist() const
{
    return QStringList()
            << tr("Start tracing")
            << tr("About");
}

void mostVesselTracerPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & v3d, QWidget * parent)
{
    if (menu_name == tr("Set Seeds"))
    {
        setSeeds(v3d, parent);
    }
    else if (menu_name == tr("Start tracing"))
    {
        set_dialog(v3d, parent);

       // startVesselTracing( v3d , parent );
    }
    else
    {
        QString msg = QString("MOST Vessel Tracing Plugin version %1\n initially developed by Jingpeng Wu (jpwu@CBMP) and refined slightly by Hanchuan Peng")
                      .arg(getPluginVersion(), 1, 'f', 1);
        QMessageBox::information(parent, "Version info", msg);
    }
}


void setSeeds(V3DPluginCallback2 &v3d, QWidget *parent )
{
    v3dhandle curwin = v3d.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    //ensure the 3d viewer window is open; if not, then open it
    v3d.open3DWindow(curwin);
}

void set_dialog(V3DPluginCallback2 &v3d, QWidget *parent)
{
    int x_begin;
    int x_end;
    int x_distance;
    int y_begin;
    int y_end;
    int y_distance;
    int z_begin;
    int z_end;
    int z_distance;
    int pruning_flag;
    int c;
    QString swcfile;
    int sslip;
    x_flag = 0;
    y_flag = 0;
    z_flag = 0;
    pruning_flag = 0;


    v3dhandle curwin = v3d.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* img = v3d.getImage(curwin);
    if (!img || !img->valid())
        return;

    AdaTDialog dialog;

    dialog.endx->setMaximum(img->getXDim()); dialog.endx->setMinimum(1); dialog.endx->setValue(img->getXDim());
    dialog.endy->setMaximum(img->getYDim()); dialog.endy->setMinimum(1); dialog.endy->setValue(img->getYDim());
    dialog.endz->setMaximum(img->getZDim()); dialog.endz->setMinimum(1); dialog.endz->setValue(img->getZDim());

    dialog.ds->setText(QString(img->getFileName()) + "_most.swc");

    dialog.channel->setMaximum(img->getCDim()); dialog.channel->setMinimum(1);dialog.channel->setValue(1);

    if (dialog.exec()!=QDialog::Accepted)
    return;
    else
    {
        InitThreshold = dialog.threshould_value->value();
        res_x_all = dialog.resx->value();
        res_y_all = dialog.resy->value();
        res_z_all = dialog.resz->value();
        seed_size_all = dialog.size->value();
        sslip = dialog.slipsize->value();
        swcfile = dialog.ds->text();
        c = dialog.channel->value();

        QFile file(swcfile);
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            v3d_msg("Cannot create the swc file to write, please try it again! ");
            return;
        }

        if(dialog.x_select->isChecked())
        {
            x_flag =1;
            x_begin = dialog.beginx->value();
            x_end = dialog.endx->value();
            x_distance = dialog.distancex->value();
        }
        if(dialog.y_select->isChecked())
        {
            y_flag =1;
            y_begin = dialog.beginy->value();
            y_end = dialog.endy->value();
            y_distance = dialog.distancey->value();
        }
        if(dialog.z_select->isChecked())
        {
            z_flag =1;
            z_begin = dialog.beginz->value();
            z_end = dialog.endz->value();
            z_distance = dialog.distancez->value();
        }
      //  v3d.open3DWindow(curwin);
        if(dialog.pruning->isChecked())
        {
            pruning_flag =1;
        }
        startVesselTracing(v3d,x_flag,y_flag,z_flag,x_begin,x_end,x_distance,y_begin,y_end,y_distance,z_begin,z_end,z_distance,swcfile,sslip,pruning_flag,c);


    }

    return;

}

//void startVesselTracing ( V3DPluginCallback2 &v3d, QWidget *parent )
void startVesselTracing(V3DPluginCallback2 &v3d,int xflag,int yflag,int zflag,int xbegin, int xend,int xdis,int ybegin,int yend,int ydis,int zbegin,int zend,int zdis,QString swcfile,int slipsize,int pruning_flag,int c)
{
    v3dhandle curwin = v3d.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }
    //ensure the 3d viewer window is open; if not, then open it
   // v3d.open3DWindow(curwin);

    // get land mark list

    LandmarkList seedList = v3d.getLandmark(curwin);

    Image4DSimple* oldimg = v3d.getImage(curwin);
    unsigned char* data1d = oldimg->getRawDataAtChannel(c-1);

    ImagePixelType pixeltype = oldimg->getDatatype();
    V3DLONG pagesz = oldimg->getTotalUnitNumberPerChannel();
    unsigned char *output_image=0;
    switch (pixeltype)
    {
    case V3D_UINT8:
        try {output_image = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for output_image."); return;}

        for(V3DLONG i = 0; i<pagesz; i++)
            output_image[i] = data1d[i];

        break;
        default: v3d_msg("Invalid data type. Do nothing."); return;
    }

    MOSTImage img;
    // set data
    img.setData( (unsigned char*)output_image, oldimg->getXDim(),oldimg->getYDim(),oldimg->getZDim(),oldimg->getCDim(),oldimg->getDatatype());


    if ( seedList.isEmpty() )
            {
               QTime qtime_seed;
               qtime_seed.start();
               //img.auto_detect_seedz(seedList,img.getZDim()/2);
               if(xflag)
               {
                   for(int i =xbegin;i<=xend;i+=xdis)
                   img.auto_detect_seedx(seedList,i,InitThreshold,seed_size_all);
                  // img.auto_detect_seedx(seedList,xend);
               }
               if(yflag)
               {
                   for(int i =ybegin;i<=yend;i+=ydis)
                   img.auto_detect_seedy(seedList,i,InitThreshold,seed_size_all);
                  // img.auto_detect_seedy(seedList,yend);
               }
               if(zflag)
               {
                   for(int i =zbegin;i<=zend;i+=zdis)
                   img.auto_detect_seedz(seedList,i,InitThreshold,seed_size_all);
                   //img.auto_detect_seedz(seedList,zend);
               }

               qDebug("  cost time seed = %g sec", qtime_seed.elapsed()*0.001);
            }
    // clear visited, only excute once
    static long init_flag = 0;
   /* if ( init_flag <= 0 )
    {
        visited.fill( false, oldimg->getTotalUnitNumber());
        init_flag ++;
    }*/
    for(init_flag = 0;init_flag<oldimg->getTotalUnitNumber();init_flag++)
        {
            visited.push_back(false);
        }

    // converte the formate
    NeuronTree vt;
    QTime qtime;
     qtime.start();
     vt = img.trace_seed_list(seedList, visited,InitThreshold,res_x_all,res_y_all,res_z_all,swcfile,slipsize,pruning_flag);
     qDebug("  cost time totol= %g sec", qtime.elapsed()*0.001);

     v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swcfile),1);


//    NeuronTree vt_old = v3d.getSWC(curwin);

    // visualization
  //  v3d.setLandmark(curwin, seedList);
   // v3d.setSWC(curwin,vt);
  //  v3d.pushObjectIn3DWindow(curwin);
  //  v3d.updateImageWindow(curwin);
    //img.~MOSTImage();
}

