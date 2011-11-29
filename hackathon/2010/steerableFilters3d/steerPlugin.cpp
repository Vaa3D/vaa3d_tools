/////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU General Public License         //
// version 2 as published by the Free Software Foundation.             //
//                                                                     //
// This program is distributed in the hope that it will be useful, but //
// WITHOUT ANY WARRANTY; without even the implied warranty of          //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// General Public License for more details.                            //
//                                                                     //
// Written and (C) by German Gonzale and Aurelien Lucchi               //
// Contact <german.gonzale@epfl.ch> or <aurelien.lucchi@gmail.com>     //
// for comments & bug reports                                          //
/////////////////////////////////////////////////////////////////////////

#include <QtGui>

#include <algorithm>
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <values.h>

#include "steerPlugin.h"
#include "SteerableFilter3D.h"
#include "steerdialog.h"

using namespace std;

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(steerPlugin, steerPlugin)

int doFiltering(V3DPluginCallback &callback, QWidget *parent);


const QString title = "SteerableFilter3D";
QStringList steerPlugin::menulist() const
{
    return QStringList() << tr("SteerableFilter3D")
						 << tr("about this plugin");
}

void steerPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
   if (menu_name == tr("SteerableFilter3D"))
    {
      doFiltering(callback, parent);
    }
	else if (menu_name == tr("about this plugin"))
	{
      QMessageBox::information(parent, "Version info", "SteerableFilter3D (28-July-2010) developed by German Gonzale and Aurelien Lucchi (Computer Vision laboratory at EPFL)");
	}
	
}
void cubeFloat2Uchar(float* inputData, uchar*& outputData,
                     int nx, int ny, int nz)
{
  float minValue = FLT_MAX;
  float maxValue = -1;
  int cubeIdx = 0;
  for(int z=0; z < nz; z++)
    for(int y=0; y < ny; y++)
      for(int x=0; x < nx; x++)
        {
          if(maxValue < inputData[cubeIdx])
              maxValue = inputData[cubeIdx];
          if(minValue > inputData[cubeIdx])
              minValue = inputData[cubeIdx];

          cubeIdx++;
        }

  //printf("[cubeFloat2Uchar] min %f, max %f\n", minValue, maxValue);

  // allocate memory
  outputData = new uchar[nx*ny*nz];

  // copy to output cube
  float scale = 255.0f/(maxValue-minValue);
  cubeIdx = 0;
  for(int z=0; z < nz; z++)
    for(int y=0; y < ny; y++)
      for(int x=0; x < nx; x++)
        {
          outputData[cubeIdx] = (inputData[cubeIdx]-minValue)*scale;
          cubeIdx++;
        }
}

int steer3d(V3DPluginCallback &callback, SteerDialog* sd, uchar* data1d,V3DLONG cubeWidth,V3DLONG cubeHeight,V3DLONG cubeDepth,V3DLONG sc)
{
   //cube vector sigma_x sigma_y sigma_z theta phi alpha
   double sigmaX = sd->getSigmaX();
   double sigmaY = sd->getSigmaY();
   double sigmaZ = sd->getSigmaZ();
   double theta = sd->getTheta()*M_PI/180.0;
   double phi = sd->getPhi()*M_PI/180.0;
   double alpha = sd->getAlpha()*M_PI/180.0;

   //printf("[steerPlugin] cubeSize = %d %d %d\n",cubeDepth,cubeHeight,cubeWidth);

   vector<double> dcoeffs;
   sd->getCoeffs(dcoeffs);
   SteerableFilter3D* stf = new SteerableFilter3D(data1d,cubeDepth,cubeHeight,cubeWidth,
                                                  dcoeffs,sigmaX,sigmaY,sigmaZ);

   //printf("[steerPlugin] %d cube derivatives computed\n", (int)stf->cubeDerivs.size());

#ifdef STEER_DEBUG
   // display first three derivatives
   for(int i = 0; i < 3; i++)
   {
      Cube< float>* deriv = stf->cubeDerivs[i];

      uchar* outputData;
      cubeFloat2Uchar(deriv->voxels_origin, outputData,cubeWidth,cubeHeight,cubeDepth);

      // show in v3d
      stringstream sout;
      sout << "deriv" << i;
      Image4DSimple p4DImageOutput;
      p4DImageOutput.setData(outputData,cubeWidth,cubeHeight,cubeDepth,sc,V3D_UINT8);

      v3dhandle newwin = callback.newImageWindow();
      callback.setImage(newwin, &p4DImageOutput);
      callback.setImageName(newwin, sout.str().c_str());
      callback.updateImageWindow(newwin);
   }
#endif

   stf->filter(theta,phi,alpha);

   uchar* outputData;
   cubeFloat2Uchar(stf->result->voxels_origin, outputData,cubeWidth,cubeHeight,cubeDepth);

   // show in v3d
   //const char* filename = "result";

   stringstream sFilename;
   sFilename << "result_";
   for(vector<double>::iterator itCoeffs = dcoeffs.begin();
      itCoeffs != dcoeffs.end(); itCoeffs++)
   {
      sFilename << *itCoeffs;
   }

   Image4DSimple p4DImageOutput;
   p4DImageOutput.setData(outputData,cubeWidth,cubeHeight,cubeDepth,sc,V3D_UINT8);

   v3dhandle newwin = callback.newImageWindow();
   callback.setImage(newwin, &p4DImageOutput);
   callback.setImageName(newwin, sFilename.str().c_str());
   callback.updateImageWindow(newwin);

   delete stf;
}

int doFiltering(V3DPluginCallback &callback, QWidget *parent)
{
   SteerDialog* sd = new SteerDialog;
   int retValue = sd->exec();

   if (retValue==CONVOLVE_DIRAC)
   {
      //int nx = MAX(64,sd->getSigmaX()*32);
      //int ny = MAX(64,sd->getSigmaY()*32);
      //int nz = MAX(64,sd->getSigmaZ()*32);
      int nx = sd->getSigmaX()*32 + 1;
      int ny = sd->getSigmaY()*32 + 1;
      int nz = sd->getSigmaZ()*32 + 1;
      Cube<uchar>* diracCube = createDiracCube(nx,ny,nz);
      steer3d(callback,sd,diracCube->voxels_origin,nx,ny,nz,1);
      delete diracCube;
   }
   else if (retValue==QDialog::Accepted)
   {
      Image4DSimple *p4DImage = callback.getImage(callback.currentImageWindow());
      if (! p4DImage) return -1;

      unsigned char* data1d = p4DImage->getRawData();
      V3DLONG cubeWidth = p4DImage->getXDim();
      V3DLONG cubeHeight = p4DImage->getYDim();
      V3DLONG cubeDepth = p4DImage->getZDim();
      V3DLONG sc = p4DImage->getCDim();

      steer3d(callback,sd,data1d,cubeWidth,cubeHeight,cubeDepth,sc);
   }
   else
   {
      delete sd;
      return -1;
   }

   delete sd;
   return 0;
}

