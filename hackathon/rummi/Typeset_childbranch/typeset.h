#ifndef TYPESET_H
#define TYPESET_H



#include "basic_surf_objs.h"
#include <vector>
#include <math.h>
using namespace std;

#define DISTP(a,b) sqrt(((a)->x-(b)->x)*((a)->x-(b)->x)+((a)->y-(b)->y)*((a)->y-(b)->y)+((a)->z-(b)->z)*((a)->z-(b)->z))

struct Point;
struct Point
{
    double n,x,y,z,r,p;
    //V3DLONG type;
    double type;
    //Point* p;
    V3DLONG childNum;
    vector<int> child_ids;
};
struct MPoint;
struct MPoint
{
    double x,y,z;
};

typedef vector<Point*> Segment;
typedef vector<Point*> Tree;
typedef vector<MPoint*> Markerpts;

void typeset_children_marker(Tree tree, double settype) //sets children as -333,then replaces -333 w/ settype
{
    for (V3DLONG i=0;i<tree.size(); i++) //go through tree list
    {

        if (tree.at(i)->p != -1) //as long as parent id is not -1
        {
            int parent_loc = tree.at(i)->p - 1; //parent location = parent id - 1

            if (tree.at(parent_loc)->type == -333) //checking type of parent by subtracting 1 from pid to get location
            {
                tree.at(i)->type = -333; //sets current node as -333
            }
        }
    }

    for (V3DLONG i=0;i<tree.size(); i++) //go through tree list
    {

        if (tree.at(i)->type == -333) //as long as parent id is not -1
        {
            tree.at(i)->type = settype; //sets type to settype as previously defined
        }
//        else if (tree.at(i)->type == -1)
//        {
//            tree.at(i)->type = 1; //soma = 1?
//        }
//        else
//        {
//            tree.at(i)->type = 20; //20 would indicate if type is unset. can add section that asks if user wants this.
//        }
    }

    v3d_msg("typeset_children_marker done!");

}

void typeset_children_branch(Tree tree, QWidget *parent) //sets branch as type set by user individually
{
    for (V3DLONG i=0;i<tree.size(); i++) //go through tree list
    {
        if (tree.at(i)->p != -1) //if node is not soma
        {
            int parent_loc = tree.at(i)->p - 1; //parent location = parent id - 1

            if (tree.at(parent_loc)->type == -333)
            {
                double current_settype;
                if (tree.at(parent_loc)->p == 1)
                {
                current_settype = QInputDialog::getDouble(parent, "Please set branch type starting at node coordinate: \n"
                                                  +QString("%1, %2,%3").arg(tree.at(i)->x).arg(tree.at(i)->y).arg(tree.at(i)->z),"Type:",0,0,4,1);
                }
                tree.at(parent_loc)->type = current_settype; //set's parent type to settype
                tree.at(i)->type = -333; //makes it easier to find
            }
        }
    }
}

NeuronTree typeset_marker(NeuronTree input, QList<ImageMarker> input1, double settype)
{
    NeuronTree result;
    V3DLONG siz = input.listNeuron.size();
    V3DLONG siz1 = input1.size();

    Tree tree;
    Markerpts coords;

    for (V3DLONG i=0;i<siz;i++) //gets swc file input
    {
        NeuronSWC s = input.listNeuron[i];
        Point* pt = new Point;
        pt->n = s.n;
        pt->x = s.x;
        pt->y = s.y;
        pt->z = s.z;
        pt->r = s.r;
        pt ->type = s.type;
        pt->p = s.parent;
        pt->childNum = 0;
        tree.push_back(pt);
    }

    double xnow,ynow,znow,dx,dy,dz,distance,min;
    double window=20;
    int min_place,check;
    vector<int> min_place_list;
    vector<int> skipped_marker_list;

    for (V3DLONG j=0;j<siz1;j++) //sets each marker point node location as -333
    {
        xnow = input1[j].x;
        ynow = input1[j].y;
        znow = input1[j].z;
        min = window;
        check = 0;

        for (V3DLONG i=0;i<siz;i++) //calculates distance from current marker to each node point
        {
            dx = tree.at(i)->x - xnow;
            dy = tree.at(i)->y - ynow;
            dz = tree.at(i)->z - znow;
            distance = sqrt(pow(dx,2) + pow(dy,2) + pow(dz,2));
            //marker_node_distance.push_back(distance);

            if (distance < min)
            {
                min = distance;
                min_place = i;
                min_place_list.push_back(min_place); //place on list, NOT node id
                check = 1;
            }
        } //if (check == 1){v3d_msg("found min value!");}

        if (check == 1)
        {
            tree.at(min_place)->type = -333;
            //v3d_msg("marker "+QString("%3").arg(j+1)+" set swc node "+QString("%4").arg(min_place+1)+" to -333");
        }
        if (min == window)
        {
            //v3d_msg("marker "+QString("%5").arg(j+1)+" does not fall within range of swc node, and was skipped");
            skipped_marker_list.push_back(j); //place on list, NOT marker number
        }
    }

    typeset_children_marker(tree,settype);

    v3d_msg("typeset_marker done.");

    for (V3DLONG i=0;i<tree.size();i++) // NEED THIS FOR LOOP EVENTUALLY
        {
            NeuronSWC s_new;
            Point* p3 = tree[i];
            s_new.n = i+1;
            s_new.pn = p3->p;
            //if (p3->p==NULL) s_new.pn = -1;
            //else
            //    s_new.pn = index_map[p3->p]+1;
            //if (p3->p==p) printf("There is loop in the tree!\n");
            s_new.x = p3->x;
            s_new.y = p3->y;
            s_new.z = p3->z;
            s_new.r = p3->r;
            s_new.type = p3->type;

            result.listNeuron.push_back(s_new);
        }

    return result;
}


NeuronTree typeset_branch(NeuronTree input, QWidget *parent)
{
    NeuronTree result;
    V3DLONG siz = input.listNeuron.size();

    Tree tree;

    for (V3DLONG i=0;i<siz;i++) //gets swc file input
    {
        NeuronSWC s = input.listNeuron[i];
        Point* pt = new Point;
        pt->n = s.n;
        pt->x = s.x;
        pt->y = s.y;
        pt->z = s.z;
        pt->r = s.r;
        pt ->type = s.type;
        pt->p = s.parent;
        pt->childNum = 0;
        tree.push_back(pt);
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        if (tree.at(i)->p == 1) //id's parent = soma
        {
            tree.at(i)->type = -333;
        }
    }
    typeset_children_branch(tree, parent);

    v3d_msg("typeset_branch done.");

}


#endif // TYPESET_H
