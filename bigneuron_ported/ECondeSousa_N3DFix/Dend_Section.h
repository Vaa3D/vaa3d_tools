/*
* N3DFix - automatic removal of swelling artifacts in neuronal 2D/3D reconstructions
* last update: Mar 2016
* VERSION 2.0
*
* Authors: Eduardo Conde-Sousa <econdesousa@gmail.com>
*          Paulo de Castro Aguiar <pauloaguiar@ineb.up.pt>
* Date:    Mar 2016
*
* N3DFix v2.0 is described in the following publication (!UNDER REVIEW!)
* Conde-Sousa E, Szucs P, Peng H, Aguiar P - Neuroinformatics, 2016
*
*    Disclaimer
*    ----------
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You can view a copy of the GNU General Public License at
*    <http://www.gnu.org/licenses/>.
*/



#ifndef DEND_SECTION_H
#define DEND_SECTION_H

#include <vector>

struct RawPoints {

    long pid, tid, ppid;
    float  x, y, z, r, arc;
    // RawPoints will have a field named r (RawPoints.r). Nevertheless, this only happens to follow the swc directives
    // After reading, despite being refered as r, it really means diam. [DANGEROUS...!]
    //  It is first multiplied by 2,
    //  then all the calculations are performed,
    //  and at the end, at writing time (to the output swc file) this field is again halved
};


#endif
