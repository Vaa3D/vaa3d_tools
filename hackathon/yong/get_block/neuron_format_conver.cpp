/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




//neuron_format_converter.cpp
//by Hanchuan Peng
//2010-06-01
// an easy way to convert several different neuron formats used in V3D


#include "neuron_format_converter.h"
#include "basic_surf_objs.h"
#include <v_neuronswc.h>

NeuronTree V_NeuronSWC__2__NeuronTree(V_NeuronSWC & seg) // convert V_NeuronSWC to Vaa3D's external neuron structure NeuronTree
{
    NeuronTree SS;

    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    {
        int count = 0;
        for (int k=0;k<seg.row.size();k++)
        {
            count++;
            NeuronSWC S;

            S.n 	= seg.row.at(k).data[0];
            S.type 	= seg.row.at(k).data[1];
            S.x 	= seg.row.at(k).data[2];
            S.y 	= seg.row.at(k).data[3];
            S.z 	= seg.row.at(k).data[4];
            S.r 	= seg.row.at(k).data[5];
            S.pn 	= seg.row.at(k).data[6];

            //for hit & editing
            S.seg_id       = seg.row.at(k).seg_id;
            S.nodeinseg_id = seg.row.at(k).nodeinseg_id;

            //qDebug("%s  ///  %d %d (%g %g %g) %g %d", buf, S.n, S.type, S.x, S.y, S.z, S.r, S.pn);

            //if (! listNeuron.contains(S)) // 081024
            {
                listNeuron.append(S);
                hashNeuron.insert(S.n, listNeuron.size()-1);
            }
        }
        printf("---------------------read %d lines, %d remained lines", count, listNeuron.size());

        SS.n = -1;
        SS.color = XYZW(seg.color_uc[0],seg.color_uc[1],seg.color_uc[2],seg.color_uc[3]);
        SS.on = true;
        SS.listNeuron = listNeuron;
        SS.hashNeuron = hashNeuron;

        SS.name = seg.name.c_str();
        SS.file = seg.file.c_str();
    }

    return SS;
}

NeuronTree V_NeuronSWC_list__2__NeuronTree(V_NeuronSWC_list & tracedNeuron) //convert to Vaa3D's external neuron structure
{
    //first conversion

    V_NeuronSWC seg = merge_V_NeuronSWC_list(tracedNeuron);
    seg.name = tracedNeuron.name;
    seg.file = tracedNeuron.file;

    //second conversion
    return V_NeuronSWC__2__NeuronTree(seg);
}

V_NeuronSWC_list NeuronTree__2__V_NeuronSWC_list(NeuronTree * nt)           //convert to V3D's internal neuron structure
{
    if (!nt)
        return V_NeuronSWC_list();

    V_NeuronSWC cur_seg;	cur_seg.clear();
    QList<NeuronSWC> & qlist = nt->listNeuron;

    for (V3DLONG i=0;i<qlist.size();i++)
    {
        V_NeuronSWC_unit v;
        v.n		= qlist[i].n;
        v.type	= qlist[i].type;
        v.x 	= qlist[i].x;
        v.y 	= qlist[i].y;
        v.z 	= qlist[i].z;
        v.r 	= qlist[i].r;
        v.parent = qlist[i].pn;

        cur_seg.append(v);
        //qDebug("%d ", cur_seg.nnodes());
    }
    cur_seg.name = qPrintable(QString("%1").arg(1));
    cur_seg.b_linegraph=false; //do not forget to do this

    V_NeuronSWC_list editableNeuron;
    editableNeuron.seg = cur_seg.decompose(); //////////////
        qDebug("	editableNeuron.seg.size = %d", editableNeuron.seg.size());

    editableNeuron.name = qPrintable(nt->name);
    editableNeuron.file = qPrintable(nt->file);
    editableNeuron.b_traced = false;
    editableNeuron.color_uc[0] = nt->color.r;
    editableNeuron.color_uc[1] = nt->color.g;
    editableNeuron.color_uc[2] = nt->color.b;
    editableNeuron.color_uc[3] = nt->color.a;

    return editableNeuron;
}



V_NeuronSWC_list NeuronTree__2__V_NeuronSWC_list(NeuronTree & nt)           //convert to V3D's internal neuron structure. overload for convenience
{
    return NeuronTree__2__V_NeuronSWC_list( &nt );
}


