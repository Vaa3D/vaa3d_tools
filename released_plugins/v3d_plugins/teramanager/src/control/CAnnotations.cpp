#include <algorithm>
#include <vector>
#include <list>
#include "locale.h"
#include <math.h>
#include <set>
#include <iostream>
#include <algorithm>
#include "CAnnotations.h"
#include "CSettings.h"

using namespace teramanager;
using namespace std;

CAnnotations* CAnnotations::uniqueInstance = 0;
long long annotation::last_ID = -1;
itm::uint64 annotation::instantiated = 0;
itm::uint64 annotation::destroyed = 0;

bool isMarker (annotation* ano) { return ano->type == 0;}

annotation::annotation() throw (itm::RuntimeException){
    type = subtype  = itm::undefined_int32;
    r = x = y = z = itm::undefined_real32;
    parent = 0;
    vaa3d_n = -1;
    name = comment = "";
    color.r = color.g = color.b = color.a = 0;
    container = 0;
    smart_delete = true;

    // assign first usable ID
    if(last_ID == std::numeric_limits<long long>::max())
        throw itm::RuntimeException("Reached the maximum number of annotation instances. Please signal this issue to the developer");
    ID = ++last_ID;

    instantiated++;

    itm::debug(itm::LEV_MAX, strprintf("%lld(%.0f, %.0f, %.0f) born", ID, x, y, z).c_str(), 0, true);
}

annotation::~annotation()
{
    //"smart" deletion
    if(smart_delete)
    {
        // if this is a tree-like structure, destroy children first
        if(type == 1)
            for(std::set<annotation*>::iterator it = children.begin(); it != children.end(); it++)
                delete *it;

        // remove annotation from the Octree
        static_cast<CAnnotations::Octree::octant*>(container)->container->remove(this);
    }

    destroyed++;

    itm::debug(itm::LEV_MAX, strprintf("%lld(%.0f, %.0f, %.0f) DESTROYED (smart_delete = %s)", ID, x, y, z, smart_delete ? "true" : "false").c_str(), 0, true);
}

void annotation::ricInsertIntoTree(annotation* node, QList<NeuronSWC> &tree)
{
    // create NeuronSWC node
    NeuronSWC p;
    p.type = node->subtype;
    p.n = node->ID;
    p.x = node->x;
    p.y = node->y;
    p.z = node->z;
    p.r = node->r;
    p.pn = node->parent ? node->parent->ID : -1;

    // add node to list
    itm::debug(itm::LEV_MAX, strprintf("Add node %lld(%.0f, %.0f, %.0f) to list", p.n, p.x, p.y, p.z).c_str(), 0, true);
    tree.push_back(p);

    // recur on children nodes
    for(std::set<annotation*>::const_iterator it = node->children.begin(); it != node->children.end(); it++)
        ricInsertIntoTree((*it), tree);
}

void annotation::insertIntoTree(QList<NeuronSWC> &tree)
{
    ricInsertIntoTree(this, tree);
}

void CAnnotations::uninstance()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = 0;
    }
}

CAnnotations::~CAnnotations()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    if(octree)
        delete octree;
    octree = 0;

    /**/itm::debug(itm::LEV1, "object successfully destroyed", __itm__current__function__);
}

//recursive support method of 'clear' method
void CAnnotations::Octree::_rec_clear(const Poctant& p_octant) throw(RuntimeException)
{
    if(p_octant)
    {
        _rec_clear(p_octant->child1);
        _rec_clear(p_octant->child2);
        _rec_clear(p_octant->child3);
        _rec_clear(p_octant->child4);
        _rec_clear(p_octant->child5);
        _rec_clear(p_octant->child6);
        _rec_clear(p_octant->child7);
        _rec_clear(p_octant->child8);

        while(!p_octant->annotations.empty())
        {
            annotation *ano = p_octant->annotations.back();
            p_octant->annotations.pop_back();
            ano->smart_delete = false;  // turn "smart" delete off before calling the decontructor
            delete ano;
        }

        delete p_octant;
    }
}

//recursive support method of 'insert' method
void CAnnotations::Octree::_rec_insert(const Poctant& p_octant, annotation& neuron) throw(RuntimeException)
{
    //if the octant is greater than a point, the insert is recursively postponed until a 1x1x1 leaf octant is reached
    if(p_octant->V_dim > 1 || p_octant->H_dim > 1 || p_octant->D_dim > 1)
    {
//        printf("\nIn octant V[%d-%d),H[%d-%d),D[%d-%d), neuron V[%.1f],H[%.1f],D[%.1f]\n",
//                p_octant->V_start, p_octant->V_start+p_octant->V_dim,
//                p_octant->H_start, p_octant->H_start+p_octant->H_dim,
//                p_octant->D_start, p_octant->D_start+p_octant->D_dim,
//                neuron.y, neuron.x, neuron.z);
        p_octant->n_annotations++;
        uint32 V_dim_halved = static_cast<int>(round((float)p_octant->V_dim/2));
        uint32 H_dim_halved = static_cast<int>(round((float)p_octant->H_dim/2));
        uint32 D_dim_halved = static_cast<int>(round((float)p_octant->D_dim/2));

        //child1: [V_start,			V_start+V_dim/2),[H_start,			H_start+H_dim/2),[D_start,			D_start+D_dim/2)
        if	   (neuron.y >= p_octant->V_start               && neuron.y < p_octant->V_start+V_dim_halved		&&
                neuron.x >= p_octant->H_start               && neuron.x < p_octant->H_start+H_dim_halved		&&
                neuron.z >= p_octant->D_start               && neuron.z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child1)
                p_octant->child1 = new octant(p_octant->V_start,                V_dim_halved,
                                                                                p_octant->H_start,				H_dim_halved,
                                                                                p_octant->D_start,				D_dim_halved, p_octant->container);
            //printf("inserting in child1\n");
            _rec_insert(p_octant->child1, neuron);
        }

        //child2: [V_start,			V_start+V_dim/2),[H_start,			H_start+H_dim/2),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron.y >= p_octant->V_start				&& neuron.y < p_octant->V_start+V_dim_halved		&&
                neuron.x >= p_octant->H_start				&& neuron.x < p_octant->H_start+H_dim_halved		&&
                neuron.z >= p_octant->D_start+D_dim_halved	&& neuron.z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child2)
                p_octant->child2 = new octant(p_octant->V_start,              V_dim_halved,
                                                                              p_octant->H_start,				H_dim_halved,
                                                                              p_octant->D_start+D_dim_halved,	D_dim_halved, p_octant->container);
            //printf("inserting in child2\n");
            _rec_insert(p_octant->child2, neuron);
        }


        //child3: [V_start,			V_start+V_dim/2),[H_start+H_dim/2,	H_start+H_dim  ),[D_start,			D_start+D_dim/2)
        else if(neuron.y >= p_octant->V_start				&& neuron.y < p_octant->V_start+V_dim_halved		&&
                neuron.x >= p_octant->H_start+H_dim_halved	&& neuron.x < p_octant->H_start+p_octant->H_dim		&&
                neuron.z >= p_octant->D_start				&& neuron.z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child3)
                    p_octant->child3 = new octant(p_octant->V_start,          V_dim_halved,
                                                                              p_octant->H_start+H_dim_halved,	H_dim_halved,
                                                                              p_octant->D_start,				D_dim_halved, p_octant->container);
            //printf("inserting in child3\n");
            _rec_insert(p_octant->child3, neuron);
        }

        //child4: [V_start,			V_start+V_dim/2),[H_start+H_dim/2,	H_start+H_dim  ),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron.y >= p_octant->V_start				&& neuron.y < p_octant->V_start+V_dim_halved		&&
                neuron.x >= p_octant->H_start+H_dim_halved	&& neuron.x < p_octant->H_start+p_octant->H_dim		&&
                neuron.z >= p_octant->D_start+D_dim_halved	&& neuron.z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child4)
                p_octant->child4 = new octant(p_octant->V_start,              V_dim_halved,
                                                                              p_octant->H_start+H_dim_halved,	H_dim_halved,
                                                                              p_octant->D_start+D_dim_halved,	D_dim_halved, p_octant->container);
            //printf("inserting in child4\n");
            _rec_insert(p_octant->child4, neuron);
        }


        //child5: [V_start+V_dim/2, V_start+V_dim  ),[H_start,			H_start+H_dim/2),[D_start,			D_start+D_dim/2)
        else if(neuron.y >= p_octant->V_start+V_dim_halved	&& neuron.y < p_octant->V_start+p_octant->V_dim		&&
                neuron.x >= p_octant->H_start				&& neuron.x < p_octant->H_start+H_dim_halved		&&
                neuron.z >= p_octant->D_start				&& neuron.z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child5)
                p_octant->child5 = new octant(p_octant->V_start+V_dim_halved, V_dim_halved,
                                                                              p_octant->H_start,				H_dim_halved,
                                                                              p_octant->D_start,				D_dim_halved, p_octant->container);
            //printf("inserting in child5\n");
            _rec_insert(p_octant->child5, neuron);
        }

        //child6: [V_start+V_dim/2, V_start+V_dim  ),[H_start,			H_start+H_dim/2),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron.y >= p_octant->V_start+V_dim_halved	&& neuron.y < p_octant->V_start+p_octant->V_dim		&&
                neuron.x >= p_octant->H_start				&& neuron.x < p_octant->H_start+H_dim_halved		&&
                neuron.z >= p_octant->D_start+D_dim_halved	&& neuron.z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child6)
                p_octant->child6 = new octant(p_octant->V_start+V_dim_halved,	V_dim_halved,
                                                                              p_octant->H_start,				H_dim_halved,
                                                                              p_octant->D_start+D_dim_halved,	D_dim_halved, p_octant->container);
            //printf("inserting in child6\n");
            _rec_insert(p_octant->child6, neuron);
        }


        //child7: [V_start+V_dim/2, V_start+V_dim  ),[H_start+H_dim/2,	H_start+H_dim  ),[D_start,			D_start+D_dim/2)
        else if(neuron.y >= p_octant->V_start+V_dim_halved	&& neuron.y < p_octant->V_start+p_octant->V_dim		&&
                neuron.x >= p_octant->H_start+H_dim_halved	&& neuron.x < p_octant->H_start+p_octant->H_dim		&&
                neuron.z >= p_octant->D_start				&& neuron.z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child7)
                p_octant->child7 = new octant(p_octant->V_start+V_dim_halved,	V_dim_halved,
                                                                                p_octant->H_start+H_dim_halved,	H_dim_halved,
                                                                                p_octant->D_start,				D_dim_halved, p_octant->container);
            //printf("inserting in child7\n");
            _rec_insert(p_octant->child7, neuron);
        }


        //child8: [V_start+V_dim/2, V_start+V_dim  ),[H_start+H_dim/2,	H_start+H_dim  ),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron.y >= p_octant->V_start+V_dim_halved			&& neuron.y < p_octant->V_start+p_octant->V_dim   &&
                neuron.x >= p_octant->H_start+H_dim_halved			&& neuron.x < p_octant->H_start+p_octant->H_dim   &&
                neuron.z >= p_octant->D_start+D_dim_halved			&& neuron.z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child8)
                p_octant->child8 = new octant(p_octant->V_start+V_dim_halved,	V_dim_halved,
                                                                                p_octant->H_start+H_dim_halved,	H_dim_halved,
                                                                                p_octant->D_start+D_dim_halved,	D_dim_halved, p_octant->container);
            //printf("inserting in child8\n");
            _rec_insert(p_octant->child8, neuron);
        }
        else
            throw RuntimeException(strprintf("in CAnnotations::Octree::insert(...): Out of bounds neuron [%.0f,%.0f,%.0f] (vaa3d n = %d).\n\n"
                                             "To activate out of bounds neuron visualization, please go to \"Options\"->\"3D annotation\"->\"Virtual space size\" and select the option \"Unlimited\".",
                                             neuron.x, neuron.y, neuron.z, neuron.vaa3d_n));
    }
    else
    {
//        printf("\nIn octant V[%d-%d),H[%d-%d),D[%d-%d), neuron V[%.1f],H[%.1f],D[%.1f] INSERTED\n",
//                p_octant->V_start, p_octant->V_start+p_octant->V_dim,
//                p_octant->H_start, p_octant->H_start+p_octant->H_dim,
//                p_octant->D_start, p_octant->D_start+p_octant->D_dim,
//                neuron.y, neuron.x, neuron.z);

        p_octant->n_annotations++;
        neuron.container = static_cast<void*>(p_octant);
        p_octant->annotations.push_back(&neuron);

        itm::debug(itm::LEV_MAX, strprintf("Added neuron %lld(%lld) {%.0f, %.0f, %.0f} to annotations list of octant X[%d,%d] Y[%d,%d] Z[%d,%d]",
                                           neuron.ID, neuron.parent ? neuron.parent->ID : -1, neuron.x, neuron.y, neuron.z,
                                           p_octant->H_start, p_octant->H_start+p_octant->H_dim,
                                           p_octant->V_start, p_octant->V_start+p_octant->V_dim,
                                           p_octant->D_start, p_octant->D_start+p_octant->D_dim).c_str(), 0, true);
    }
}

//recursive support method of 'remove' method
void CAnnotations::Octree::_rec_remove(const Poctant& p_octant, annotation *neuron) throw(RuntimeException)
{
    //if the octant is greater than a point, the remove operation is recursively postponed until a 1x1x1 leaf octant is reached
    if(p_octant->V_dim > 1 || p_octant->H_dim > 1 || p_octant->D_dim > 1)
    {
        p_octant->n_annotations--;
        uint32 V_dim_halved = static_cast<int>(round((float)p_octant->V_dim/2));
        uint32 H_dim_halved = static_cast<int>(round((float)p_octant->H_dim/2));
        uint32 D_dim_halved = static_cast<int>(round((float)p_octant->D_dim/2));

        //child1: [V_start,			V_start+V_dim/2),[H_start,			H_start+H_dim/2),[D_start,			D_start+D_dim/2)
        if	   (neuron->y >= p_octant->V_start	&& neuron->y < p_octant->V_start+V_dim_halved		&&
                neuron->x >= p_octant->H_start	&& neuron->x < p_octant->H_start+H_dim_halved		&&
                neuron->z >= p_octant->D_start    && neuron->z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child1)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child1, neuron);
        }

        //child2: [V_start,			V_start+V_dim/2),[H_start,			H_start+H_dim/2),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron->y >= p_octant->V_start				&& neuron->y < p_octant->V_start+V_dim_halved		&&
                neuron->x >= p_octant->H_start				&& neuron->x < p_octant->H_start+H_dim_halved		&&
                neuron->z >= p_octant->D_start+D_dim_halved	&& neuron->z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child2)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child2, neuron);
        }


        //child3: [V_start,			V_start+V_dim/2),[H_start+H_dim/2,	H_start+H_dim  ),[D_start,			D_start+D_dim/2)
        else if(neuron->y >= p_octant->V_start				&& neuron->y < p_octant->V_start+V_dim_halved		&&
                neuron->x >= p_octant->H_start+H_dim_halved	&& neuron->x < p_octant->H_start+p_octant->H_dim		&&
                neuron->z >= p_octant->D_start				&& neuron->z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child3)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child3, neuron);
        }

        //child4: [V_start,			V_start+V_dim/2),[H_start+H_dim/2,	H_start+H_dim  ),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron->y >= p_octant->V_start				&& neuron->y < p_octant->V_start+V_dim_halved		&&
                neuron->x >= p_octant->H_start+H_dim_halved	&& neuron->x < p_octant->H_start+p_octant->H_dim		&&
                neuron->z >= p_octant->D_start+D_dim_halved	&& neuron->z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child4)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child4, neuron);
        }


        //child5: [V_start+V_dim/2, V_start+V_dim  ),[H_start,			H_start+H_dim/2),[D_start,			D_start+D_dim/2)
        else if(neuron->y >= p_octant->V_start+V_dim_halved	&& neuron->y < p_octant->V_start+p_octant->V_dim		&&
                neuron->x >= p_octant->H_start				&& neuron->x < p_octant->H_start+H_dim_halved		&&
                neuron->z >= p_octant->D_start				&& neuron->z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child5)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child5, neuron);
        }

        //child6: [V_start+V_dim/2, V_start+V_dim  ),[H_start,			H_start+H_dim/2),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron->y >= p_octant->V_start+V_dim_halved	&& neuron->y < p_octant->V_start+p_octant->V_dim		&&
                neuron->x >= p_octant->H_start				&& neuron->x < p_octant->H_start+H_dim_halved		&&
                neuron->z >= p_octant->D_start+D_dim_halved	&& neuron->z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child6)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child6, neuron);
        }


        //child7: [V_start+V_dim/2, V_start+V_dim  ),[H_start+H_dim/2,	H_start+H_dim  ),[D_start,			D_start+D_dim/2)
        else if(neuron->y >= p_octant->V_start+V_dim_halved	&& neuron->y < p_octant->V_start+p_octant->V_dim		&&
                neuron->x >= p_octant->H_start+H_dim_halved	&& neuron->x < p_octant->H_start+p_octant->H_dim		&&
                neuron->z >= p_octant->D_start				&& neuron->z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child7)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child7, neuron);
        }


        //child8: [V_start+V_dim/2, V_start+V_dim  ),[H_start+H_dim/2,	H_start+H_dim  ),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron->y >= p_octant->V_start+V_dim_halved			&& neuron->y < p_octant->V_start+p_octant->V_dim   &&
                neuron->x >= p_octant->H_start+H_dim_halved			&& neuron->x < p_octant->H_start+p_octant->H_dim   &&
                neuron->z >= p_octant->D_start+D_dim_halved			&& neuron->z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child8)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child8, neuron);
        }
        else
            throw RuntimeException(strprintf("in CAnnotations::Octree::remove(...): Cannot find the proper region wherein to remove the given neuron [%.0f,%.0f,%.0f]", neuron->y, neuron->x, neuron->z));
    }
    else
    {
        p_octant->n_annotations--;

        if(std::find(p_octant->annotations.begin(), p_octant->annotations.end(), neuron) == p_octant->annotations.end())
            throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

        p_octant->annotations.remove(neuron);

        itm::debug(itm::LEV_MAX, strprintf("REMOVED neuron %lld(%lld) {%.0f, %.0f, %.0f} from annotations list of octant X[%d,%d] Y[%d,%d] Z[%d,%d]",
                                           neuron->ID, neuron->parent ? neuron->parent->ID : -1, neuron->x, neuron->y, neuron->z,
                                           p_octant->H_start, p_octant->H_start+p_octant->H_dim,
                                           p_octant->V_start, p_octant->V_start+p_octant->V_dim,
                                           p_octant->D_start, p_octant->D_start+p_octant->D_dim).c_str(), 0, true);
    }
}

//recursive support method of 'deep_count' method
uint32 CAnnotations::Octree::_rec_deep_count(const Poctant& p_octant) throw(RuntimeException)
{
    if(p_octant)
        if(p_octant->V_dim == 1 && p_octant->H_dim == 1 && p_octant->D_dim == 1)
             return 1;
        else
            return  _rec_deep_count(p_octant->child1)+_rec_deep_count(p_octant->child2)+_rec_deep_count(p_octant->child3)+
                    _rec_deep_count(p_octant->child4)+_rec_deep_count(p_octant->child5)+_rec_deep_count(p_octant->child6)+
                    _rec_deep_count(p_octant->child7)+_rec_deep_count(p_octant->child8);
    else return 0;
}

//recursive support method of 'height' method
uint32 CAnnotations::Octree::_rec_height(const Poctant& p_octant) throw(RuntimeException)
{
    if(p_octant)
    {
        uint32 height_1 = _rec_height(p_octant->child1);
        uint32 height_2 = _rec_height(p_octant->child2);
        uint32 height_3 = _rec_height(p_octant->child3);
        uint32 height_4 = _rec_height(p_octant->child4);
        uint32 height_5 = _rec_height(p_octant->child5);
        uint32 height_6 = _rec_height(p_octant->child6);
        uint32 height_7 = _rec_height(p_octant->child7);
        uint32 height_8 = _rec_height(p_octant->child8);
        return 1+ ( MAX(MAX(MAX(height_1,height_2),MAX(height_3,height_4)),MAX(MAX(height_5,height_6),MAX(height_7,height_8))) );
    }
    else return 0;
}

//recursive support method of 'height' method
void CAnnotations::Octree::_rec_print(const Poctant& p_octant)
{
    if(p_octant)
    {
        printf("V[%d-%d),H[%d-%d),D[%d-%d)\n",p_octant->V_start, p_octant->V_start+p_octant->V_dim,
                                                                                  p_octant->H_start, p_octant->H_start+p_octant->H_dim,
                                                                                  p_octant->D_start, p_octant->D_start+p_octant->D_dim);
        for(std::list<teramanager::annotation*>::iterator i = p_octant->annotations.begin(); i!= p_octant->annotations.end(); i++)
            printf("|===> %.2f %.2f %.2f\n", (*i)->y, (*i)->x, (*i)->z);
        _rec_print(p_octant->child1);
        _rec_print(p_octant->child2);
        _rec_print(p_octant->child3);
        _rec_print(p_octant->child4);
        _rec_print(p_octant->child5);
        _rec_print(p_octant->child6);
        _rec_print(p_octant->child7);
        _rec_print(p_octant->child8);
    }
}

//recursive support method of 'find' method
void CAnnotations::Octree::_rec_search(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int, std::list<annotation*>& neurons)  throw(RuntimeException)
{
    if(p_octant)
    {
        if(p_octant->V_dim == 1 && p_octant->H_dim == 1 && p_octant->D_dim == 1)
        {
            for(std::list<teramanager::annotation*>::iterator i = p_octant->annotations.begin(); i!= p_octant->annotations.end(); i++)
                neurons.push_back((*i));
        }
        else
        {
            uint32 V_dim_halved = static_cast<int>(round((float)p_octant->V_dim/2));
            uint32 V_halved		= p_octant->V_start+V_dim_halved;
            uint32 H_dim_halved = static_cast<int>(round((float)p_octant->H_dim/2));
            uint32 H_halved		= p_octant->H_start+H_dim_halved;
            uint32 D_dim_halved = static_cast<int>(round((float)p_octant->D_dim/2));
            uint32 D_halved		= p_octant->D_start+D_dim_halved;

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start, H_dim_halved, p_octant->D_start,	D_dim_halved))
                _rec_search(p_octant->child1, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start, H_dim_halved, D_halved,			D_dim_halved))
                _rec_search(p_octant->child2, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,		   H_dim_halved, p_octant->D_start,	D_dim_halved))
                _rec_search(p_octant->child3, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,		   H_dim_halved, D_halved,			D_dim_halved))
                _rec_search(p_octant->child4, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	p_octant->H_start, H_dim_halved, p_octant->D_start,	D_dim_halved))
                _rec_search(p_octant->child5, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	p_octant->H_start, H_dim_halved, D_halved,			D_dim_halved))
                _rec_search(p_octant->child6, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	H_halved,		   H_dim_halved, p_octant->D_start,	D_dim_halved))
                _rec_search(p_octant->child7, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	H_halved,		   H_dim_halved, D_halved,			D_dim_halved))
                _rec_search(p_octant->child8, V_int, H_int, D_int, neurons);
        }
    }
}

//recursive support method of 'rec_find' method
CAnnotations::Octree::Poctant CAnnotations::Octree::_rec_find(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int) throw(RuntimeException)
{
    //printf("\n_rec_find(p_octant=%d, V_int=[%d-%d), H_int=[%d-%d), D_int=[%d-%d))\n", p_octant, V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end);

    if(p_octant)
    {
//        printf("_rec_find(): in octant V[%d-%d),H[%d-%d),D[%d-%d)\n",
//                p_octant->V_start, p_octant->V_start+p_octant->V_dim,
//                p_octant->H_start, p_octant->H_start+p_octant->H_dim,
//                p_octant->D_start, p_octant->D_start+p_octant->D_dim);
        if(p_octant->V_dim == 1 && p_octant->H_dim == 1 && p_octant->D_dim == 1)
            return p_octant;
        else
        {
            //printf("_rec_find(): smisting...\n");
            uint32 V_dim_halved = static_cast<int>(round((float)p_octant->V_dim/2));
            uint32 V_halved	= p_octant->V_start+V_dim_halved;
            uint32 H_dim_halved = static_cast<int>(round((float)p_octant->H_dim/2));
            uint32 H_halved	= p_octant->H_start+H_dim_halved;
            uint32 D_dim_halved = static_cast<int>(round((float)p_octant->D_dim/2));
            uint32 D_halved	= p_octant->D_start+D_dim_halved;

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, p_octant->V_start, p_octant->V_start+V_dim_halved,   p_octant->H_start,  p_octant->H_start+H_dim_halved, p_octant->D_start,	p_octant->D_start+D_dim_halved);
            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start,  H_dim_halved, p_octant->D_start,	D_dim_halved))
                return _rec_find(p_octant->child1, V_int, H_int, D_int);
            //printf("no.\n");

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, p_octant->V_start, p_octant->V_start+V_dim_halved,   p_octant->H_start,  p_octant->H_start+H_dim_halved, D_halved,		D_halved+D_dim_halved);
            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start,  H_dim_halved, D_halved,		D_dim_halved))
                return _rec_find(p_octant->child2, V_int, H_int, D_int);
            //printf("no.\n");

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, p_octant->V_start, p_octant->V_start+V_dim_halved,   H_halved,           H_halved+H_dim_halved,          p_octant->D_start,	p_octant->D_start+D_dim_halved);
            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,           H_dim_halved, p_octant->D_start,	D_dim_halved))
                return _rec_find(p_octant->child3, V_int, H_int, D_int);
            //printf("no.\n");

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, p_octant->V_start, p_octant->V_start+V_dim_halved,   H_halved,           H_halved+H_dim_halved,          D_halved,		D_halved+D_dim_halved);
            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,           H_dim_halved, D_halved,		D_dim_halved))
                return _rec_find(p_octant->child4, V_int, H_int, D_int);
            //printf("no.\n");

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, V_halved,          V_halved+V_dim_halved,            p_octant->H_start,  p_octant->H_start+H_dim_halved, p_octant->D_start,	p_octant->D_start+D_dim_halved);
            if(intersects(V_int, H_int, D_int, V_halved,	  V_dim_halved,	p_octant->H_start,  H_dim_halved, p_octant->D_start,	D_dim_halved))
                return _rec_find(p_octant->child5, V_int, H_int, D_int);
            //printf("no.\n");

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, V_halved,          V_halved+V_dim_halved,            p_octant->H_start,  p_octant->H_start+H_dim_halved, D_halved,		D_halved+D_dim_halved);
            if(intersects(V_int, H_int, D_int, V_halved,          V_dim_halved,	p_octant->H_start,  H_dim_halved, D_halved,		D_dim_halved))
                return _rec_find(p_octant->child6, V_int, H_int, D_int);
            //printf("no.\n");

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, V_halved,          V_halved+V_dim_halved,            H_halved,           H_halved+H_dim_halved,          p_octant->D_start,	p_octant->D_start+D_dim_halved);
            if(intersects(V_int, H_int, D_int, V_halved,	  V_dim_halved,	H_halved,           H_dim_halved, p_octant->D_start,	D_dim_halved))
                return _rec_find(p_octant->child7, V_int, H_int, D_int);
            //printf("no.\n");

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, V_halved,          V_halved+V_dim_halved,            H_halved,           H_halved+H_dim_halved,          D_halved,		D_halved+D_dim_halved);
            if(intersects(V_int, H_int, D_int, V_halved,	  V_dim_halved,	H_halved,           H_dim_halved, D_halved,		D_dim_halved))
                return _rec_find(p_octant->child8, V_int, H_int, D_int);
            //printf("no.\n");

            return 0;
        }
    }
    else
        return 0;
}

//recursive support method of 'count' method
uint32 CAnnotations::Octree::_rec_count(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int) throw(RuntimeException)
{
    if(p_octant)
    {
        if(contains(V_int, H_int, D_int, p_octant->V_start, p_octant->V_dim, p_octant->H_start, p_octant->H_dim, p_octant->D_start, p_octant->D_dim))
           return p_octant->n_annotations;
        else
        {
            uint32 neuron_count = 0;
            uint32 V_dim_halved = static_cast<int>(round((float)p_octant->V_dim/2));
            uint32 V_halved		= p_octant->V_start+V_dim_halved;
            uint32 H_dim_halved = static_cast<int>(round((float)p_octant->H_dim/2));
            uint32 H_halved		= p_octant->H_start+H_dim_halved;
            uint32 D_dim_halved = static_cast<int>(round((float)p_octant->D_dim/2));
            uint32 D_halved		= p_octant->D_start+D_dim_halved;

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start, H_dim_halved, p_octant->D_start,	D_dim_halved))
                 neuron_count+= _rec_count(p_octant->child1, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start, H_dim_halved, D_halved,			D_dim_halved))
                neuron_count+= _rec_count(p_octant->child2, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,		   H_dim_halved, p_octant->D_start,	D_dim_halved))
                 neuron_count+= _rec_count(p_octant->child3, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,		   H_dim_halved, D_halved,			D_dim_halved))
                neuron_count+= _rec_count(p_octant->child4, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	p_octant->H_start, H_dim_halved, p_octant->D_start,	D_dim_halved))
                neuron_count+= _rec_count(p_octant->child5, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	p_octant->H_start, H_dim_halved, D_halved,			D_dim_halved))
                 neuron_count+= _rec_count(p_octant->child6, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	H_halved,		   H_dim_halved, p_octant->D_start,	D_dim_halved))
                 neuron_count+= _rec_count(p_octant->child7, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	H_halved,		   H_dim_halved, D_halved,			D_dim_halved))
                neuron_count+= _rec_count(p_octant->child8, V_int, H_int, D_int);

            return neuron_count;
        }
    }
    else return 0;
}

//returns true if two given volumes intersect each other
bool inline CAnnotations::Octree::intersects(const interval_t& V1_int,		 const interval_t& H1_int,		   const interval_t& D1_int,
                                                           uint32& V2_start, uint32& V2_dim, uint32& H2_start, uint32& H2_dim, uint32& D2_start, uint32& D2_dim) throw(RuntimeException)
{
    return 	( V1_int.start  < (V2_start + V2_dim)	&&
              V1_int.end    >  V2_start             &&
              H1_int.start  < (H2_start + H2_dim)	&&
              H1_int.end    >  H2_start             &&
              D1_int.start  < (D2_start + D2_dim)	&&
              D1_int.end    >  D2_start	 );
}

//returns true if first volume contains second volume
bool inline CAnnotations::Octree::contains  (const interval_t& V1_int,		 const interval_t& H1_int,		   const interval_t& D1_int,
                                             uint32& V2_start, uint32& V2_dim, uint32& H2_start, uint32& H2_dim, uint32& D2_start, uint32& D2_dim) throw(RuntimeException)
{
    return (  V1_int.start  <=  V2_start            &&
              V1_int.end    >=  (V2_start+V2_dim)	&&
              H1_int.start  <=  H2_start            &&
              H1_int.end    >=  (H2_start+H2_dim)	&&
              D1_int.start  <=  D2_start            &&
              D1_int.end    >=  (D2_start+D2_dim));
}

CAnnotations::Octree::Octree(uint32 _DIM_V, uint32 _DIM_H, uint32 _DIM_D)
{
    /**/itm::debug(itm::LEV1, strprintf("dimV = %d, dimH = %d, dimD = %d", _DIM_V, _DIM_H, _DIM_D).c_str(), __itm__current__function__);

    if(CSettings::instance()->getAnnotationSpaceUnlimited())
    {
        DIM_V = std::numeric_limits<uint32>::max();
        DIM_H = std::numeric_limits<uint32>::max();
        DIM_D = std::numeric_limits<uint32>::max();

        /**/itm::debug(itm::LEV1, strprintf("unbounded annotation space activated", _DIM_V, _DIM_H, _DIM_D).c_str(), __itm__current__function__);
    }
    else
    {
        DIM_V = _DIM_V;
        DIM_H = _DIM_H;
        DIM_D = _DIM_D;
    }
    root = new octant(0,DIM_V,0,DIM_H,0,DIM_D, this);
}

CAnnotations::Octree::~Octree(void)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    clear();

    /**/itm::debug(itm::LEV1, "object successfully DESTROYED", __itm__current__function__);
}

//clears octree content and deallocates used memory
void CAnnotations::Octree::clear() throw(RuntimeException)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    _rec_clear(root);
    root = 0;
}

//insert given neuron in the octree
void CAnnotations::Octree::insert(annotation& neuron)  throw(RuntimeException)
{
    _rec_insert(root,neuron);
}

//remove given neuron from the octree (returns 1 if succeeds)
bool CAnnotations::Octree::remove(annotation *neuron) throw(itm::RuntimeException)
{
    std::list<annotation*>* matching_nodes = this->find(neuron->x, neuron->y, neuron->z);
    if(std::find(matching_nodes->begin(), matching_nodes->end(), neuron) == matching_nodes->end())
        return false;
    else
    {
        _rec_remove(root, neuron);
        return true;
    }
}

//search for the annotations at the given coordinate. If found, returns the address of the annotations list
std::list<annotation*>* CAnnotations::Octree::find(float x, float y, float z) throw(RuntimeException)
{
    interval_t V_range(static_cast<int>(floor(y)), static_cast<int>(ceil(y)));
    interval_t H_range(static_cast<int>(floor(x)), static_cast<int>(ceil(x)));
    interval_t D_range(static_cast<int>(floor(z)), static_cast<int>(ceil(z)));
    if(V_range.end == V_range.start)
        V_range.end++;
    if(H_range.end == H_range.start)
        H_range.end++;
    if(D_range.end == D_range.start)
        D_range.end++;
    Poctant oct = _rec_find(root, V_range, H_range, D_range);
    if(oct)
        return &(oct->annotations);
    else
        return 0;
}

//returns the number of neurons (=leafs) in the octree by exploring the entire data structure
uint32 CAnnotations::Octree::deep_count() throw(RuntimeException)
{
    return _rec_deep_count(root);
}

//returns the octree height
uint32 CAnnotations::Octree::height() throw(RuntimeException)
{
    return _rec_height(root);
}

//print the octree content
void CAnnotations::Octree::print()
{
    printf("\n\nOCTREE start printing...\n\n");
    _rec_print(root);
    printf("\n\nOCTREE end printing...\n\n");
}

//search for neurons in the given 3D volume and puts found neurons into 'neurons'
void CAnnotations::Octree::find(interval_t V_int, interval_t H_int, interval_t D_int, std::list<annotation*>& neurons) throw(RuntimeException)
{
    /**/itm::debug(itm::LEV2, strprintf("interval = [%d,%d](V) x [%d,%d](H) x [%d,%d](D)", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end).c_str(), __itm__current__function__);
    _rec_search(root, V_int, H_int, D_int, neurons);
    /**/itm::debug(itm::LEV2, strprintf("found %d neurons", neurons.size()).c_str(), __itm__current__function__);
}

//returns the number of neurons (=leafs) in the given volume without exploring the entire data structure
uint32 CAnnotations::Octree::count(interval_t V_int, interval_t H_int, interval_t D_int) throw(RuntimeException)
{
    //adjusting default parameters
    V_int.start = V_int.start == -1 ? 0		: V_int.start;
    V_int.end   = V_int.end   == -1 ? DIM_V : V_int.end;
    H_int.start = H_int.start == -1 ? 0		: H_int.start;
    H_int.end   = H_int.end   == -1 ? DIM_H : H_int.end;
    D_int.start = D_int.start == -1 ? 0		: D_int.start;
    D_int.end   = D_int.end   == -1 ? DIM_D : D_int.end;

    return _rec_count(root, V_int, H_int, D_int);
}


/*********************************************************************************
* Adds the given annotation(s)
**********************************************************************************/
void CAnnotations::addLandmarks(itm::interval_t X_range, itm::interval_t Y_range, itm::interval_t Z_range, LandmarkList &markers) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("X[%d,%d), Y[%d,%d), Z[%d,%d), markers.size = %d",
                                        X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end, markers.size()).c_str(), __itm__current__function__);

    /**/itm::debug(itm::LEV3, strprintf("%d markers before clearLandmarks", count()).c_str(), __itm__current__function__);
    clearLandmarks(X_range, Y_range, Z_range);
    /**/itm::debug(itm::LEV3, strprintf("%d markers after clearLandmarks", count()).c_str(), __itm__current__function__);

    for(int i=0; i<markers.size(); i++)
    {
       annotation* node = new annotation();
       node->type = 0;
       node->subtype = markers[i].category;
       node->parent = 0;
       node->r = markers[i].radius;
       node->x = markers[i].x;
       node->y = markers[i].y;
       node->z = markers[i].z;
       node->name = markers[i].name;
       node->comment = markers[i].comments;
       node->color = markers[i].color;
       octree->insert(*node);
    }

    /**/itm::debug(itm::LEV3, strprintf("%d markers after insertions", count()).c_str(), __itm__current__function__);
}

void CAnnotations::clearCurves(itm::interval_t X_range, itm::interval_t Y_range, itm::interval_t Z_range) throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("X[%d,%d), Y[%d,%d), Z[%d,%d)", X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end).c_str(), __itm__current__function__);

    std::list<annotation*> nodes;
    std::set <annotation*> roots;
    octree->find(Y_range, X_range, Z_range, nodes);

    // retrieve root nodes from the nodes founds so far
    for(std::list<annotation*>::const_iterator it = nodes.begin(); it != nodes.end(); it++)
    {
        // is a neuron node (type = 1)
        if((*it)->type == 1)
        {
            annotation *p = *it;
            while(p->parent != 0)
                p = p->parent;
            roots.insert(p);
        }
    }

    // clear all segments starting from the retrieved root nodes
    for(std::set<annotation*>::const_iterator it = roots.begin(); it != roots.end(); it++)
        delete *it;
}

void CAnnotations::clearLandmarks(itm::interval_t X_range, itm::interval_t Y_range, itm::interval_t Z_range) throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("X[%d,%d), Y[%d,%d), Z[%d,%d)", X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end).c_str(), __itm__current__function__);

    std::list<annotation*> nodes;
    octree->find(Y_range, X_range, Z_range, nodes);

    /**/itm::debug(itm::LEV3, strprintf("found %d nodes", nodes.size()).c_str(), __itm__current__function__);
    for(std::list<annotation*>::const_iterator it = nodes.begin(); it != nodes.end(); it++)
        if((*it)->type == 0)
            delete *it;
}

void CAnnotations::addCurves(itm::interval_t X_range, itm::interval_t Y_range, itm::interval_t Z_range, NeuronTree& nt) throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("X[%d,%d), Y[%d,%d), Z[%d,%d)", X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end).c_str(), __itm__current__function__);

    // first clear curves in the given range
    itm::uint64 deletions = annotation::destroyed;
    clearCurves(X_range, Y_range, Z_range);
    deletions = annotation::destroyed - deletions;
    /**/itm::debug(itm::LEV3, strprintf("nt.size() = %d, deleted = %llu", nt.listNeuron.size(), deletions).c_str(), __itm__current__function__);

    // remove duplicates
//    for(int i=0; i<nt.listNeuron.size(); i++)
//    {
//        ;
//    }

    std::map<int, annotation*> annotationsMap;
    std::map<int, NeuronSWC*> swcMap;
    for(int i=0; i<nt.listNeuron.size(); i++)
    {
        annotation* ann = new annotation();
        ann->type = 1;
        ann->name = nt.name.toStdString();
        ann->comment = nt.comment.toStdString();
        ann->color = nt.color;
        ann->subtype = nt.listNeuron[i].type;
        ann->r = nt.listNeuron[i].r;
        ann->x = nt.listNeuron[i].x;
        ann->y = nt.listNeuron[i].y;
        ann->z = nt.listNeuron[i].z;
        itm::debug(itm::LEV_MAX, strprintf("inserting curve point %lld(%.1f,%.1f,%.1f), n=(%d), pn(%d)\n", ann->ID, ann->x, ann->y, ann->z, nt.listNeuron[i].n, nt.listNeuron[i].pn).c_str(), 0, true);
        octree->insert(*ann);
        annotationsMap[nt.listNeuron[i].n] = ann;
        swcMap[nt.listNeuron[i].n] = &(nt.listNeuron[i]);
    }
    for(std::map<int, annotation*>::iterator it = annotationsMap.begin(); it!= annotationsMap.end(); it++)
    {
        it->second->parent = swcMap[it->first]->pn == -1 ? 0 : annotationsMap[swcMap[it->first]->pn];
        if(it->second->parent)
        {
            itm::debug(itm::LEV_MAX, strprintf("Add %lld(%.0f, %.0f, %.0f) to %lld(%.0f, %.0f, %.0f)'s children list\n",
                                               it->second->ID, it->second->x, it->second->y, it->second->z, it->second->parent->ID,
                                               it->second->parent->x, it->second->parent->y, it->second->parent->z).c_str(), 0, true);
            it->second->parent->children.insert(it->second);
        }
    }
//    printf("--------------------- teramanager plugin >> inserted %d curve points\n", annotationsMap.size());
}

/*********************************************************************************
* Retrieves the annotation(s) in the given volume space
**********************************************************************************/
void CAnnotations::findLandmarks(interval_t X_range, interval_t Y_range, interval_t Z_range, QList<LocationSimple> &markers) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("X_range = [%d,%d), Y_range = [%d,%d), Z_range = [%d,%d)",
                                        X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end).c_str(), __itm__current__function__);

    std::list<annotation*> nodes;

    /**/itm::debug(itm::LEV3, "find all nodes in the given range", __itm__current__function__);
    octree->find(Y_range, X_range, Z_range, nodes);


    /**/itm::debug(itm::LEV3, "select markers only", __itm__current__function__);
    for(std::list<annotation*>::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if((*i)->type == 0) //selecting markers
        {
            LocationSimple marker;
            marker.x = (*i)->x;
            marker.y = (*i)->y;
            marker.z = (*i)->z;
            marker.radius = (*i)->r;
            marker.category = (*i)->subtype;
            marker.color = (*i)->color;
            marker.name = (*i)->name;
            marker.comments = (*i)->comment;
            markers.push_back(marker);
        }
    }
}

void CAnnotations::findCurves(interval_t X_range, interval_t Y_range, interval_t Z_range, QList<NeuronSWC> &curves) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("X_range = [%d,%d), Y_range = [%d,%d), Z_range = [%d,%d)",
                                        X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end).c_str(), __itm__current__function__);

    std::list<annotation*> nodes;

    /**/itm::debug(itm::LEV3, "find all nodes in the given range", __itm__current__function__);
    octree->find(Y_range, X_range, Z_range, nodes);

    // find roots
    /**/itm::debug(itm::LEV3, "find roots", __itm__current__function__);
    std::set<annotation*> roots;
    for(std::list<annotation*>::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if((*i)->type == 1) //selecting curve points
        {
            annotation* p = (*i);
            while(p->parent != 0)
                p = p->parent;
            //if(find(roots.begin(),roots.end(), source) == roots.end())
            roots.insert(p);
        }
    }

    /**/itm::debug(itm::LEV3, strprintf("%d roots found, now inserting all nodes", roots.size()).c_str(), __itm__current__function__);
    for(std::set<annotation*>::const_iterator it = roots.begin(); it != roots.end(); it++)
        (*it)->insertIntoTree(curves);
    /**/itm::debug(itm::LEV3, strprintf("%d nodes inserted", curves.size()).c_str(), __itm__current__function__);
}

/*********************************************************************************
* Save/load methods
**********************************************************************************/
void CAnnotations::save(const char* filepath) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("filepath = \"%s\"", filepath).c_str(), __itm__current__function__);

    //retrieving annotations
    std::list<annotation*> annotations;
    if(octree)
        octree->find(interval_t(0, octree->DIM_V), interval_t(0, octree->DIM_H), interval_t(0, octree->DIM_D), annotations);

    //saving ano file
    QDir anoFile(filepath);
    FILE* f = fopen(filepath, "w");
    if(!f)
        throw RuntimeException(strprintf("in CAnnotations::save(): cannot save to path \"%s\"", filepath));
    fprintf(f, "APOFILE=%s\n",anoFile.dirName().toStdString().append(".apo").c_str());
    fprintf(f, "SWCFILE=%s\n",anoFile.dirName().toStdString().append(".swc").c_str());
    fclose(f);

    //saving apo (point cloud) file
    QList<CellAPO> points;
    for(std::list<annotation*>::iterator i = annotations.begin(); i!= annotations.end(); i++)
        if((*i)->type == 0)     //selecting markers
        {
            CellAPO cell;
            cell.n = (*i)->ID;
            cell.name = (*i)->name.c_str();
            cell.comment = (*i)->comment.c_str();
            cell.x = (*i)->x;
            cell.y = (*i)->y;
            cell.z = (*i)->z;
            cell.volsize = (*i)->r*(*i)->r*4*teramanager::pi;
            cell.color = (*i)->color;
            points.push_back(cell);
        }
    writeAPO_file(QString(filepath).append(".apo"), points);

    //saving SWC file
    f = fopen(QString(filepath).append(".swc").toStdString().c_str(), "w");
    fprintf(f, "#name undefined\n");
    fprintf(f, "#comment terafly_annotations\n");
    fprintf(f, "#n type x y z radius parent\n");
        for(std::list<annotation*>::iterator i = annotations.begin(); i != annotations.end(); i++)
            if((*i)->type == 1) //selecting NeuronSWC
                fprintf(f, "%lld %d %.3f %.3f %.3f %.3f %lld\n", (*i)->ID, (*i)->subtype, (*i)->x, (*i)->y, (*i)->z, (*i)->r, (*i)->parent ? (*i)->parent->ID : -1);

    //file closing
    fclose(f);
}
void CAnnotations::load(const char* filepath) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("filepath = \"%s\"", filepath).c_str(), __itm__current__function__);

    //precondition checks
    if(!octree)
        throw RuntimeException("CAnnotations::load(): octree not yet initialized");

    //clearing annotations
    this->clear();

    //opening ANO file in read mode
    FILE* f = fopen(filepath, "r");
    if(!f)
        throw RuntimeException(strprintf("in CAnnotations::save(): cannot load file \"%s\"", filepath));

    //reading ANO file line by line
    char lineBuf[FILE_LINE_BUFFER_SIZE];
    while(fgets(lineBuf, FILE_LINE_BUFFER_SIZE, f))
    {
        char tokenizer[STATIC_STRING_SIZE];
        char filetype[STATIC_STRING_SIZE];
        char filename[STATIC_STRING_SIZE];
        if(sscanf(lineBuf, "%s", tokenizer) != 1)
            throw RuntimeException(strprintf("in CAnnotations::load(const char* filepath = \"%s\"): expected line \"%s\", found \"%s\"",
                                           filepath, "<filetype>=<filename>", lineBuf));
        char * pch;
        pch = strtok (tokenizer,"=");
        strcpy(filetype, pch);
        pch = strtok (0,"=");
        strcpy(filename, pch);

        QDir dir(filepath);
        dir.cdUp();
        if(strcmp(filetype, "APOFILE") == 0)
        {
            QList <CellAPO> cells = readAPO_file(dir.absolutePath().append("/").append(filename));
            for(QList <CellAPO>::iterator i = cells.begin(); i!= cells.end(); i++)
            {
                annotation* ann = new annotation();
                ann->type = 0;
                ann->name = i->name.toStdString();
                ann->comment = i->comment.toStdString();
                ann->color = i->color;
                ann->r = sqrt(i->volsize / (4*teramanager::pi));
                ann->x = i->x;
                ann->y = i->y;
                ann->z = i->z;
                octree->insert(*ann);
            }
            //printf("--------------------- teramanager plugin >> inserted %d markers\n", cells.size());
        }
        else if(strcmp(filetype, "SWCFILE") == 0)
        {
            NeuronTree nt = readSWC_file(dir.absolutePath().append("/").append(filename));

            std::map<int, annotation*> annotationsMap;
            std::map<int, NeuronSWC*> swcMap;
            for(QList <NeuronSWC>::iterator i = nt.listNeuron.begin(); i!= nt.listNeuron.end(); i++)
            {
                annotation* ann = new annotation();
                ann->type = 1;
                ann->name = nt.name.toStdString();
                ann->comment = nt.comment.toStdString();
                ann->color = nt.color;
                ann->subtype = i->type;
                ann->r = i->r;
                ann->x = i->x;
                ann->y = i->y;
                ann->z = i->z;
                ann->vaa3d_n = i->n;
                octree->insert(*ann);
                annotationsMap[i->n] = ann;
                swcMap[i->n] = &(*i);
            }
            for(std::map<int, annotation*>::iterator i = annotationsMap.begin(); i!= annotationsMap.end(); i++)
            {
                i->second->parent = swcMap[i->first]->pn == -1 ? 0 : annotationsMap[swcMap[i->first]->pn];
                if(i->second->parent)
                {
                    itm::debug(itm::LEV_MAX, strprintf("Add %lld(%.0f, %.0f, %.0f) to %lld(%.0f, %.0f, %.0f)'s children list\n",
                                                       i->second->ID, i->second->x, i->second->y, i->second->z, i->second->parent->ID,
                                                       i->second->parent->x, i->second->parent->y, i->second->parent->z).c_str(), 0, true);
                    i->second->parent->children.insert(i->second);
                }
            }
        }
        else
            throw RuntimeException(strprintf("in CAnnotations::load(const char* filepath = \"%s\"): unable to recognize file type \"%s\"", filepath, filetype));
    }
    fclose(f);
}

/*********************************************************************************
* Conversion from VTK to APO files
**********************************************************************************/
void CAnnotations::convertVtk2APO(std::string vtkPath, std::string apoPath) throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("vtkPath = \"%s\", apoPath = \"%s\"", vtkPath.c_str(), apoPath.c_str()).c_str(), __itm__current__function__);

    // open file and check header
    FILE* f = fopen(vtkPath.c_str(), "r");
    if(!f)
        throw itm::RuntimeException(itm::strprintf("in CAnnotations::convertVtk2APO(): cannot open file at \"%s\"", vtkPath.c_str()));
    char lineBuffer[1024];
    char stringBuffer[1024];
    int points_n;
    if(!fgets(lineBuffer, 1024, f))
        throw itm::RuntimeException(itm::strprintf("Cannot read 1st line in \"%s\"", vtkPath.c_str()));
    if(!fgets(lineBuffer, 1024, f))
        throw itm::RuntimeException(itm::strprintf("Cannot read 2nd line in \"%s\"", vtkPath.c_str()));
    if(!fgets(lineBuffer, 1024, f))
        throw itm::RuntimeException(itm::strprintf("Cannot read 3rd line in \"%s\"", vtkPath.c_str()));
    if(!fgets(lineBuffer, 1024, f))
        throw itm::RuntimeException(itm::strprintf("Cannot read 4th line in \"%s\"", vtkPath.c_str()));
    if(!fgets(lineBuffer, 1024, f))
        throw itm::RuntimeException(itm::strprintf("Cannot read 5th line in \"%s\"", vtkPath.c_str()));
    if(sscanf(lineBuffer, "%s %d", stringBuffer, &points_n) != 2)
        throw itm::RuntimeException(itm::strprintf("5th line (\"%s\") is not in the format <string> <number> <string> as expected, in \"%s\"", lineBuffer, vtkPath.c_str()));
    if(strcmp(stringBuffer, "POINTS") != 0)
        throw itm::RuntimeException(itm::strprintf("expected \"POINTS\" at 5th line (\"%s\"), in \"%s\"", lineBuffer, vtkPath.c_str()));

    // read cells
    QList<CellAPO> cells;
    for(int i=0; i<points_n; i++)
    {
        if(!fgets(lineBuffer, 1024, f))
            throw itm::RuntimeException(itm::strprintf("Cannot read %d-th line in \"%s\"", i, vtkPath.c_str()));
        float x=0,y=0,z=0;
        if(sscanf(lineBuffer, "%f %f %f", &x, &y, &z) != 3)
            throw itm::RuntimeException(itm::strprintf("%d-th line (\"%s\") is not in the format <float> <float> <float> as expected, in \"%s\"", i, lineBuffer, vtkPath.c_str()));


        CellAPO cell;
        cell.n = i;
        cell.name = "na";
        cell.comment = "na";
        cell.x = x;
        cell.y = y;
        cell.z = z;
        cell.volsize = 4*teramanager::pi;
        cell.color.r = cell.color.g = 0;
        cell.color.b = 255;
        cells.push_back(cell);
    }
    fclose(f);

    // save to APO file
    writeAPO_file(apoPath.c_str(), cells);
}
