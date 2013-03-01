#ifndef TOOLS_H
#define TOOLS_H

#include "../../../vaa3d/v3d_main/neuron_editing/v_neuronswc.h"
#include <math.h>
#include <vector>
#include <string>
#include <stdio.h>

#include <QDebug>

V_NeuronSWC segmentPruning(V_NeuronSWC in_swc);


/*
This is a Optical-Character-Recognition program
Copyright (C) 2000-2007  Joerg Schulenburg

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 see README for EMAIL-address

 the following code was send by Ryan Dibble <dibbler@umich.edu>

  The algorithm is very simple but works good hopefully.

  Compare the grayscale histogram with a mass density diagram:
  I think the algorithm is a kind of
  divide a body into two parts in a way that the mass
  centers have the largest distance from each other,
  the function is weighted in a way that same masses have a advantage

  - otsu algorithm is failing on diskrete multi color images

  TODO:
    RGB: do the same with all colors (CMYG?) seperately

    test: hardest case = two colors
       bbg: test done, using a two color gray file. Output:
       # threshold: Value = 43 gmin=43 gmax=188

 my changes:
   - float -> double
   - debug option added (vvv & 1..2)
   - **image => *image,  &image[i][1] => &image[i*cols+1]
   - do only count pixels near contrast regions
     this makes otsu much better for shadowed fonts or multi colored text
     on white background

 (m) Joerg Schulenburg (see README for email address)

 ToDo:
   - measure contrast
   - detect low-contrast regions
/*======================================================================*/
/*   OTSU global thresholding routine                                   */
/*   modified by jpwu, 2011.5.21                                        */
/*======================================================================*/
int otsu (int * ihist);
int otsu2(int * ihist);

#endif // TOOLS_H
