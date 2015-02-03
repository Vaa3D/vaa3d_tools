#include <algorithm>
#include <vector>
#include <list>
#include "locale.h"
#include <math.h>
#include <set>
#include <iostream>
#include <algorithm>
#include <fstream>
#include "CAnnotations.h"
#include "CSettings.h"
#include "COperation.h"
#include "../presentation/PLog.h"

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

    #ifdef terafly_enable_debug_annotations
    itm::debug(itm::LEV_MAX, strprintf("%lld(%.0f, %.0f, %.0f) born", ID, x, y, z).c_str(), 0, true);
    #endif
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

    #ifdef terafly_enable_debug_annotations
    itm::debug(itm::LEV_MAX, strprintf("%lld(%.0f, %.0f, %.0f) DESTROYED (smart_delete = %s)", ID, x, y, z, smart_delete ? "true" : "false").c_str(), 0, true);
    #endif
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
    #ifdef terafly_enable_debug_annotations
    itm::debug(itm::LEV_MAX, strprintf("Add node %lld(%.0f, %.0f, %.0f) to list", p.n, p.x, p.y, p.z).c_str(), 0, true);
    #endif
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

        #ifdef terafly_enable_debug_annotations
        itm::debug(itm::LEV_MAX, strprintf("Added neuron %lld(%lld) {%.0f, %.0f, %.0f} to annotations list of octant X[%d,%d] Y[%d,%d] Z[%d,%d]",
                                           neuron.ID, neuron.parent ? neuron.parent->ID : -1, neuron.x, neuron.y, neuron.z,
                                           p_octant->H_start, p_octant->H_start+p_octant->H_dim,
                                           p_octant->V_start, p_octant->V_start+p_octant->V_dim,
                                           p_octant->D_start, p_octant->D_start+p_octant->D_dim).c_str(), 0, true);
        #endif
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

        #ifdef terafly_enable_debug_annotations
        itm::debug(itm::LEV_MAX, strprintf("REMOVED neuron %lld(%lld) {%.0f, %.0f, %.0f} from annotations list of octant X[%d,%d] Y[%d,%d] Z[%d,%d]",
                                           neuron->ID, neuron->parent ? neuron->parent->ID : -1, neuron->x, neuron->y, neuron->z,
                                           p_octant->H_start, p_octant->H_start+p_octant->H_dim,
                                           p_octant->V_start, p_octant->V_start+p_octant->V_dim,
                                           p_octant->D_start, p_octant->D_start+p_octant->D_dim).c_str(), 0, true);
        #endif
    }
}

//recursive support method of 'deep_count' method
itm::uint32 CAnnotations::Octree::_rec_deep_count(const Poctant& p_octant) throw(RuntimeException)
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

//recursive support method of 'toNeuronTree' method
void CAnnotations::Octree::_rec_to_neuron_tree(const Poctant& p_octant, QList<NeuronSWC> &segments) throw(itm::RuntimeException)
{
    if(p_octant && p_octant->V_dim > 1 && p_octant->H_dim > 1 && p_octant->D_dim > 1)
//    if(p_octant)
    {
        // 12 octant edges (24 corners) to be created...
        NeuronSWC corners[24];
        for(int i=0; i<24; i++)
        {
            corners[i].type = 2;
            corners[i].n = (i == 0 ? (segments.empty()? 0 : segments.back().n) : corners[i-1].n) + 1;
        }


        // assign coordinates to the 24 corners
        // --- top-left-front corner --------------------------
        corners[0].y = corners[8].y = corners[16].y = p_octant->V_start;
        corners[0].x = corners[8].x = corners[16].x = p_octant->H_start;
        corners[0].z = corners[8].z = corners[16].z = p_octant->D_start;
        // --- top-right-front corner -------------------------
        corners[1].y = corners[9].y = corners[17].y = p_octant->V_start;
        corners[1].x = corners[9].x = corners[17].x = p_octant->H_start + p_octant->H_dim -1;
        corners[1].z = corners[9].z = corners[17].z = p_octant->D_start;
        // --- top-right-back corner --------------------------
        corners[2].y = corners[10].y = corners[18].y = p_octant->V_start;
        corners[2].x = corners[10].x = corners[18].x = p_octant->H_start + p_octant->H_dim - 1;
        corners[2].z = corners[10].z = corners[18].z = p_octant->D_start + p_octant->D_dim - 1;
        // --- top-left-back corner ---------------------------
        corners[3].y = corners[11].y = corners[19].y = p_octant->V_start;
        corners[3].x = corners[11].x = corners[19].x = p_octant->H_start;
        corners[3].z = corners[11].z = corners[19].z = p_octant->D_start + p_octant->D_dim - 1;
        // --- bottom-left-front corner -----------------------
        corners[4].y = corners[12].y = corners[20].y = p_octant->V_start + p_octant->V_dim - 1;
        corners[4].x = corners[12].x = corners[20].x = p_octant->H_start;
        corners[4].z = corners[12].z = corners[20].z = p_octant->D_start;
        // --- bottom-right-front corner ----------------------
        corners[5].y = corners[13].y = corners[21].y = p_octant->V_start + p_octant->V_dim - 1;
        corners[5].x = corners[13].x = corners[21].x = p_octant->H_start + p_octant->H_dim - 1;
        corners[5].z = corners[13].z = corners[21].z =  p_octant->D_start;
        // --- bottom-right-back corner -----------------------
        corners[6].y = corners[14].y = corners[22].y = p_octant->V_start + p_octant->V_dim - 1;
        corners[6].x = corners[14].x = corners[22].x = p_octant->H_start + p_octant->H_dim - 1;
        corners[6].z = corners[14].z = corners[22].z = p_octant->D_start + p_octant->D_dim - 1;
        // --- bottom-left-back corner ------------------------
        corners[7].y = corners[15].y = corners[23].y = p_octant->V_start + p_octant->V_dim - 1;
        corners[7].x = corners[15].x = corners[23].x = p_octant->H_start;
        corners[7].z = corners[15].z = corners[23].z = p_octant->D_start + p_octant->D_dim - 1;

        // create links
        corners[0].pn = -1;
        corners[1].pn = corners[0].n;
        corners[2].pn = -1;
        corners[3].pn = corners[2].n;
        corners[4].pn = -1;
        corners[5].pn = corners[4].n;
        corners[6].pn = -1;
        corners[7].pn = corners[6].n;
        corners[8].pn = -1;
        corners[11].pn = corners[8].n;
        corners[9].pn = -1;
        corners[10].pn = corners[9].n;
        corners[12].pn = -1;
        corners[15].pn = corners[12].n;
        corners[13].pn = -1;
        corners[14].pn = corners[13].n;
        corners[16].pn = -1;
        corners[20].pn = corners[16].n;
        corners[17].pn = -1;
        corners[21].pn = corners[17].n;
        corners[18].pn = -1;
        corners[22].pn = corners[18].n;
        corners[19].pn = -1;
        corners[23].pn = corners[19].n;

        // add links to NeuronTree
        for(int i=0; i<24; i++)
        {
            // apply trims
            corners[i].x = itm::saturate_trim<float>(corners[i].x, DIM_H - 1);
            corners[i].y = itm::saturate_trim<float>(corners[i].y, DIM_V - 1);
            corners[i].z = itm::saturate_trim<float>(corners[i].z, DIM_D - 1);

            segments.push_back(corners[i]);
        }

        // process children
        _rec_to_neuron_tree(p_octant->child1, segments);
        _rec_to_neuron_tree(p_octant->child2, segments);
        _rec_to_neuron_tree(p_octant->child3, segments);
        _rec_to_neuron_tree(p_octant->child4, segments);
        _rec_to_neuron_tree(p_octant->child5, segments);
        _rec_to_neuron_tree(p_octant->child6, segments);
        _rec_to_neuron_tree(p_octant->child7, segments);
        _rec_to_neuron_tree(p_octant->child8, segments);
    }
}

//recursive support method of 'height' method
itm::uint32 CAnnotations::Octree::_rec_height(const Poctant& p_octant) throw(RuntimeException)
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

//recursive support method of 'prune' method
void CAnnotations::Octree::_rec_prune(const Poctant& p_octant) throw(itm::RuntimeException)
{
    if(p_octant)
    {
        // prune goes here
        if(p_octant->V_dim == 1 && p_octant->H_dim == 1 && p_octant->D_dim == 1)
        {
            std::list<annotation*>::iterator i = p_octant->annotations.begin();
            while (i != p_octant->annotations.end() && p_octant->annotations.size() > 1)
            {
                annotation &ai  = **i;
                std::list<annotation*>::iterator j = i;
                while (j != p_octant->annotations.end() && p_octant->annotations.size() > 1)
                {
                    annotation &aj = **j;
                    if(*i != *j                        &&  // two different nodes
                       ai.type    == 1 && aj.type == 1 &&  // both have neuron type
                       ai         ==      aj           &&  // both have same coordinates
                       ai.parent  &&      aj.parent    &&  // both have a valid parent
                     *(ai.parent) ==    *(aj.parent))      // both have the same parent's coordinates (otherwise this is a crossroad and MUST not be pruned)
                    {
                        // aj is considered as duplicate --> ai takes its place in 4 actions:
                        printf("found duplicate neuron at (%d, %d, %d)\n", p_octant->H_start, p_octant->V_start, p_octant->D_start);

                        // 1) children of aj are assigned new parent ai
                        for(std::set<annotation*>::iterator ajc = aj.children.begin(); ajc != aj.children.end(); ajc++)
                            (*ajc)->parent = &ai;

                        // 2) children of aj become children of ai
                        ai.children.insert(aj.children.begin(), aj.children.end());

                        // 3) remove aj from it's parent children list
                        aj.parent->children.erase(&aj);

                        // 4) remove aj from the octree
                        p_octant->annotations.erase(j);

                        // 5) deallocate memory for aj
                        aj.smart_delete = false;
                        delete &aj;
                    }
                    else
                        ++j;
                }
                if(i != p_octant->annotations.end())
                    ++i;
            }
        }
        else
        {
            _rec_prune(p_octant->child1);
            _rec_prune(p_octant->child2);
            _rec_prune(p_octant->child3);
            _rec_prune(p_octant->child4);
            _rec_prune(p_octant->child5);
            _rec_prune(p_octant->child6);
            _rec_prune(p_octant->child7);
            _rec_prune(p_octant->child8);
        }
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
itm::uint32 CAnnotations::Octree::_rec_count(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int) throw(RuntimeException)
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
                                                           itm::uint32& V2_start, itm::uint32& V2_dim, itm::uint32& H2_start, itm::uint32& H2_dim, itm::uint32& D2_start, itm::uint32& D2_dim) throw(RuntimeException)
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
                                             itm::uint32& V2_start, itm::uint32& V2_dim, itm::uint32& H2_start, itm::uint32& H2_dim, itm::uint32& D2_start, itm::uint32& D2_dim) throw(RuntimeException)
{
    return (  V1_int.start  <=  V2_start            &&
              V1_int.end    >=  (V2_start+V2_dim)	&&
              H1_int.start  <=  H2_start            &&
              H1_int.end    >=  (H2_start+H2_dim)	&&
              D1_int.start  <=  D2_start            &&
              D1_int.end    >=  (D2_start+D2_dim));
}

CAnnotations::Octree::Octree(itm::uint32 _DIM_V, itm::uint32 _DIM_H, itm::uint32 _DIM_D)
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

//prunes the octree by removing all nodes duplicates while maintaining the same branched structure
void CAnnotations::Octree::prune() throw(itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    _rec_prune(root);
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
itm::uint32 CAnnotations::Octree::deep_count() throw(RuntimeException)
{
    return _rec_deep_count(root);
}

//returns the octree height
itm::uint32 CAnnotations::Octree::height() throw(RuntimeException)
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
itm::uint32 CAnnotations::Octree::count(interval_t V_int, interval_t H_int, interval_t D_int) throw(RuntimeException)
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

    QElapsedTimer timer;
    timer.start();
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
    PLog::instance()->appendOperation(new AnnotationOperation("store annotations: add landmarks", itm::CPU, timer.elapsed()));

    /**/itm::debug(itm::LEV3, strprintf("%d markers after insertions", count()).c_str(), __itm__current__function__);
}

void CAnnotations::clearCurves(itm::interval_t X_range, itm::interval_t Y_range, itm::interval_t Z_range) throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("X[%d,%d), Y[%d,%d), Z[%d,%d)", X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end).c_str(), __itm__current__function__);

    QElapsedTimer timer;
    timer.start();
    std::list<annotation*> nodes;
    std::set <annotation*> roots;
    octree->find(Y_range, X_range, Z_range, nodes);
    PLog::instance()->appendOperation(new AnnotationOperation("clear curves: find curve nodes in the given range", itm::CPU, timer.elapsed()));

    // retrieve root nodes from the nodes founds so far
    timer.restart();
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
    PLog::instance()->appendOperation(new AnnotationOperation("clear curves: retrieve root nodes from the nodes founds so far", itm::CPU, timer.elapsed()));

    // clear all segments starting from the retrieved root nodes
    timer.restart();
    for(std::set<annotation*>::const_iterator it = roots.begin(); it != roots.end(); it++)
        delete *it;
    PLog::instance()->appendOperation(new AnnotationOperation("clear curves: clear all segments starting from the retrieved root nodes", itm::CPU, timer.elapsed()));
}

void CAnnotations::clearLandmarks(itm::interval_t X_range, itm::interval_t Y_range, itm::interval_t Z_range) throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("X[%d,%d), Y[%d,%d), Z[%d,%d)", X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end).c_str(), __itm__current__function__);

    QElapsedTimer timer;
    timer.start();
    std::list<annotation*> nodes;
    octree->find(Y_range, X_range, Z_range, nodes);
    PLog::instance()->appendOperation(new AnnotationOperation("clear landmarks: find landmarks in the given range", itm::CPU, timer.elapsed()));

    /**/itm::debug(itm::LEV3, strprintf("found %d nodes", nodes.size()).c_str(), __itm__current__function__);
    timer.restart();
    for(std::list<annotation*>::const_iterator it = nodes.begin(); it != nodes.end(); it++)
        if((*it)->type == 0)
            delete *it;
    PLog::instance()->appendOperation(new AnnotationOperation("clear landmarks: remove landmarks", itm::CPU, timer.elapsed()));
}

void CAnnotations::addCurves(itm::interval_t X_range, itm::interval_t Y_range, itm::interval_t Z_range, NeuronTree& nt) throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("X[%d,%d), Y[%d,%d), Z[%d,%d)", X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end).c_str(), __itm__current__function__);

    // first clear curves in the given range
    itm::uint64 deletions = annotation::destroyed;
    clearCurves(X_range, Y_range, Z_range);
    deletions = annotation::destroyed - deletions;
    /**/itm::debug(itm::LEV3, strprintf("nt.size() = %d, deleted = %llu", nt.listNeuron.size(), deletions).c_str(), __itm__current__function__);

    // then allocate and initialize curve nodes
    QElapsedTimer timer;
    timer.start();
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

        #ifdef terafly_enable_debug_annotations
        itm::debug(itm::LEV_MAX, strprintf("inserting curve point %lld(%.1f,%.1f,%.1f), n=(%d), pn(%d)\n", ann->ID, ann->x, ann->y, ann->z, nt.listNeuron[i].n, nt.listNeuron[i].pn).c_str(), 0, true);
        #endif

        octree->insert(*ann);
        annotationsMap[nt.listNeuron[i].n] = ann;
        swcMap[nt.listNeuron[i].n] = &(nt.listNeuron[i]);
    }

    PLog::instance()->appendOperation(new AnnotationOperation("store annotations: allocate and initialize curve nodes", itm::CPU, timer.elapsed()));

    // finally linking nodes
    timer.restart();
    for(std::map<int, annotation*>::iterator it = annotationsMap.begin(); it!= annotationsMap.end(); it++)
    {
        it->second->parent = swcMap[it->first]->pn == -1 ? 0 : annotationsMap[swcMap[it->first]->pn];
        if(it->second->parent)
        {
            #ifdef terafly_enable_debug_annotations
            itm::debug(itm::LEV_MAX, strprintf("Add %lld(%.0f, %.0f, %.0f) to %lld(%.0f, %.0f, %.0f)'s children list\n",
                                               it->second->ID, it->second->x, it->second->y, it->second->z, it->second->parent->ID,
                                               it->second->parent->x, it->second->parent->y, it->second->parent->z).c_str(), 0, true);
            #endif

            it->second->parent->children.insert(it->second);
        }
    }
    PLog::instance()->appendOperation(new AnnotationOperation("store annotations: link curve nodes", itm::CPU, timer.elapsed()));
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
    QElapsedTimer timer;
    timer.start();

    /**/itm::debug(itm::LEV3, "find all nodes in the given range", __itm__current__function__);
    octree->find(Y_range, X_range, Z_range, nodes);
    PLog::instance()->appendOperation(new AnnotationOperation("find landmarks: find all annotations in the given range", itm::CPU, timer.elapsed()));


    /**/itm::debug(itm::LEV3, "select markers only", __itm__current__function__);
    timer.restart();
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
    PLog::instance()->appendOperation(new AnnotationOperation("find landmarks: select landmarks only", itm::CPU, timer.elapsed()));
}

void CAnnotations::findCurves(interval_t X_range, interval_t Y_range, interval_t Z_range, QList<NeuronSWC> &curves) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("X_range = [%d,%d), Y_range = [%d,%d), Z_range = [%d,%d)",
                                        X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end).c_str(), __itm__current__function__);

    std::list<annotation*> nodes;
    QElapsedTimer timer;
    timer.start();

    /**/itm::debug(itm::LEV3, "find all nodes in the given range", __itm__current__function__);
    octree->find(Y_range, X_range, Z_range, nodes);
    PLog::instance()->appendOperation(new AnnotationOperation("find curves: find all annotations in the given range", itm::CPU, timer.elapsed()));

    // find roots
    timer.restart();
    /**/itm::debug(itm::LEV3, "find roots", __itm__current__function__);
    std::set<annotation*> roots;
    for(std::list<annotation*>::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if((*i)->type == 1) //selecting curve points
        {
            annotation* p = (*i);
            while(p->parent != 0)
                p = p->parent;
            roots.insert(p);
        }
    }
    PLog::instance()->appendOperation(new AnnotationOperation("find curves: find roots", itm::CPU, timer.elapsed()));

    /**/itm::debug(itm::LEV3, strprintf("%d roots found, now inserting all nodes", roots.size()).c_str(), __itm__current__function__);
    timer.restart();
    for(std::set<annotation*>::const_iterator it = roots.begin(); it != roots.end(); it++)
        (*it)->insertIntoTree(curves);
    PLog::instance()->appendOperation(new AnnotationOperation("find curves: insert all linked nodes starting from roots", itm::CPU, timer.elapsed()));
    /**/itm::debug(itm::LEV3, strprintf("%d nodes inserted", curves.size()).c_str(), __itm__current__function__);
}

/*********************************************************************************
* Save/load methods
**********************************************************************************/
void CAnnotations::save(const char* filepath) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("filepath = \"%s\"", filepath).c_str(), __itm__current__function__);

    //measure elapsed time
    QElapsedTimer timer;
    timer.start();

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


    PLog::instance()->appendOperation(new AnnotationOperation("save annotations: save .ano to disk", itm::IO, timer.elapsed()));
}
void CAnnotations::load(const char* filepath) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("filepath = \"%s\"", filepath).c_str(), __itm__current__function__);

    //precondition checks
    if(!octree)
        throw RuntimeException("CAnnotations::load(): octree not yet initialized");

    //measure elapsed time
    QElapsedTimer timer;
    timer.start();

    //clearing annotations
    this->clear();

    // open ANO file
    std::ifstream f(filepath);
    if(!f.is_open())
        throw RuntimeException(strprintf("in CAnnotations::save(): cannot load file \"%s\"", filepath));

    // read line by line
    for (std::string line; std::getline(f, line); )
    {
        std::vector < std::string > tokens;
        teramanager::split(line, "=", tokens);
        if(tokens.size() != 2)
            throw RuntimeException(strprintf("in CAnnotations::load(const char* filepath = \"%s\"): cannot parse line \"%s\"",filepath,line.c_str()));

        QDir dir(filepath);
        dir.cdUp();
        if(tokens[0].compare("APOFILE") == 0)
        {
            QList <CellAPO> cells = readAPO_file(dir.absolutePath().append("/").append(tokens[1].c_str()));
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
        else if(tokens[0].compare("SWCFILE") == 0)
        {
            NeuronTree nt = readSWC_file(dir.absolutePath().append("/").append(tokens[1].c_str()));

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
                    #ifdef terafly_enable_debug_annotations
                    itm::debug(itm::LEV_MAX, strprintf("Add %lld(%.0f, %.0f, %.0f) to %lld(%.0f, %.0f, %.0f)'s children list\n",
                                                       i->second->ID, i->second->x, i->second->y, i->second->z, i->second->parent->ID,
                                                       i->second->parent->x, i->second->parent->y, i->second->parent->z).c_str(), 0, true);
                    #endif

                    i->second->parent->children.insert(i->second);
                }
            }
        }
        else
            throw RuntimeException(strprintf("in CAnnotations::load(const char* filepath = \"%s\"): unable to recognize file type \"%s\"", filepath, tokens[0].c_str()));
    }
    f.close();

    PLog::instance()->appendOperation(new AnnotationOperation("load annotations: read .ano from disk", itm::IO, timer.elapsed()));
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

/*********************************************************************************
* Diff between two APO files
**********************************************************************************/
void CAnnotations::diffAPO( std::string apo1Path,               // first apo file path (assumed as truth)
                            std::string apo2Path,               // second apo file path
                            int x0 /*=0*/, int x1 /*=-1*/,      // VOI [x0, x1) in the global reference sys
                            int y0 /*=0*/, int y1 /*=-1*/,      // VOI [y0, y1) in the global reference sys
                            int z0 /*=0*/, int z1 /*=-1*/,      // VOI [z0, z1) in the global reference sys
                            std::string diffPath)               // path where the difference apo file (containing only FPs and FNs) has to be stored (optional)
throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("apo1Path = \"%s\", apo2Path = \"%s\", x0 = %d, x1 =%d, y0 = %d, y1 = %d, z0 = %d, z1 = %d",
                                        apo1Path.c_str(), apo2Path.c_str(), x0, x1, y0, y1, z0, z1).c_str(), __itm__current__function__);

    // parse default parameters
    x1 = x1 > 0 ? x1 : std::numeric_limits<int>::max();
    y1 = y1 > 0 ? y1 : std::numeric_limits<int>::max();
    z1 = z1 > 0 ? z1 : std::numeric_limits<int>::max();

    // read cells
    QList<CellAPO> cells1 = readAPO_file(apo1Path.c_str());
    QList<CellAPO> cells2 = readAPO_file(apo2Path.c_str());

    // insert cells into the same octree. In this way, it is much faster to compute TPs, FPs and FNs
    Octree* diff_octree = new Octree(std::numeric_limits<uint32>::max(), std::numeric_limits<uint32>::max(), std::numeric_limits<uint32>::max());
    std::vector<annotation*> ano1, ano2;
    itm::uint64 cell1count = 0;
    for(int i=0; i<cells1.size(); i++)
    {
        if( cells1[i].x >= x0 && cells1[i].x < x1 &&    //
            cells1[i].y >= y0 && cells1[i].y < y1 &&    // cell is within selected VOI
            cells1[i].z >= z0 && cells1[i].z < z1)      //
        {

            annotation* cell = new annotation;
            cell->type = 0;
            cell->subtype = 0;
            cell->x = cells1[i].x;
            cell->y = cells1[i].y;
            cell->z = cells1[i].z;
            ano1.push_back(cell);
            diff_octree->insert(*cell);
            cell1count++;
        }
    }
    itm::uint64 cell2count = 0;
    for(int i=0; i<cells2.size(); i++)
    {
        if( cells2[i].x >= x0 && cells2[i].x < x1 &&    //
            cells2[i].y >= y0 && cells2[i].y < y1 &&    // cell is within selected VOI
            cells2[i].z >= z0 && cells2[i].z < z1)      //
        {
            annotation* cell = new annotation;
            cell->type = 0;
            cell->subtype = 0;
            cell->x = cells2[i].x;
            cell->y = cells2[i].y;
            cell->z = cells2[i].z;
            ano2.push_back(cell);
            diff_octree->insert(*cell);
            cell2count++;
        }
    }

    // count false positives, true positives, and false negatives    
    QList<CellAPO> diff_cells;
    itm::uint64 FPs = 0;
    itm::uint64 TPs = 0;
    itm::uint64 FNs = 0;
    for(int i=0; i<ano2.size(); i++)
        if( static_cast<CAnnotations::Octree::octant*>(ano2[i]->container)->annotations.size() == 1)
        {
            CellAPO cell;
            cell.n = ano2[i]->ID;
            cell.name = ano2[i]->name.c_str();
            cell.comment = ano2[i]->comment.c_str();
            cell.x = ano2[i]->x;
            cell.y = ano2[i]->y;
            cell.z = ano2[i]->z;
            cell.volsize = ano2[i]->r*ano2[i]->r*4*teramanager::pi;
            cell.color.r = 255;
            cell.color.g = 0;
            cell.color.b = 0;
            diff_cells.push_back(cell);
            FPs++;
        }
        else
            TPs++;
    for(int i=0; i<ano1.size(); i++)
        if( static_cast<CAnnotations::Octree::octant*>(ano1[i]->container)->annotations.size() == 1)
        {
            CellAPO cell;
            cell.n = ano2[i]->ID;
            cell.name = ano2[i]->name.c_str();
            cell.comment = ano2[i]->comment.c_str();
            cell.x = ano2[i]->x;
            cell.y = ano2[i]->y;
            cell.z = ano2[i]->z;
            cell.volsize = ano2[i]->r*ano2[i]->r*4*teramanager::pi;
            cell.color.r = 0;
            cell.color.g = 0;
            cell.color.b = 255;
            diff_cells.push_back(cell);
            FNs++;
        }

    // release memory for octree
    delete diff_octree;

    // save diff apo (point cloud) file
//    diffPath = "C:/diff.apo";
    if(!diffPath.empty())
        writeAPO_file(diffPath.c_str(), diff_cells);

    // display result
    std::string message = itm::strprintf(   "VOI = X[%d,%d), Y[%d,%d), Z[%d,%d)\n"
                                            "#Cells (from truth): %d\n"
                                            "#Cells (from .apo) : %d\n"
                                            "#TPs: %lld\n"
                                            "#FPs: %lld\n"
                                            "#FNs: %lld\n"
                                            "TPR: %.3f\n"
                                            "FPR: %.3f\n",
                                            x0, x1, y0, y1, z0, z1,
                                            cell1count,
                                            cell2count,
                                            TPs,
                                            FPs,
                                            FNs,
                                            (TPs+0.1f)/(TPs+FNs),
                                            (FPs+0.1f)/(cell2count));
    QMessageBox::information(0, "Result", message.c_str());
}

/*********************************************************************************
* Converts the octree to a NeuronTree. This is actually a draw method.
**********************************************************************************/
NeuronTree CAnnotations::Octree::toNeuronTree() throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    // unlimited octrees are not supported
    if(DIM_V == std::numeric_limits<uint32>::max() || DIM_H == std::numeric_limits<uint32>::max() || DIM_D == std::numeric_limits<uint32>::max())
        throw itm::RuntimeException("Cannot draw unbounded octree. Please deactivate the \"Unlimited\" option for the annotation space size and re-open the image");

    NeuronTree nt;
    nt.editable = false;
    nt.linemode = true;
    _rec_to_neuron_tree(root, nt.listNeuron);

    /**/itm::debug(itm::LEV2, "Successfully generated the Neuron Tree", __itm__current__function__);

    return nt;
}
