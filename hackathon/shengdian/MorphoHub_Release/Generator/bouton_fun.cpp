#include "bouton_fun.h"
void printHelp()
{
    /*vaa3d -x <libname> -f Bouton_Generation -i <input-para-list> -o <out-para-list> -p <fun_id> <node-specified> <out_type>
     * <fun_id>:
     * case 0: get bouton from terafly formatted data. <input_image_terafly> and <input_swc> are needed.
     * case 1: get bouton from 3D image block.
     * case 2: get the intensity of all the nodes (SWC or ESWC) from terafly formatted data.
     * case 3:
     * <node-specified>:
     * case 0: all nodes in swc will be used.
     * case 1: axonal nodes (type=2)
     * <out_type>:
     * case 0: output bouton data to a point cloud (apo)
     * case 1: output bouton data to SWC/ESWC file (radius feature = 2)
     * case 2: output bouton data to ESWC file (level feature = 2,keep non-bouton node)
     * case 3: output bouton data to ESWC file (remove non-bouton node)
    */

}
