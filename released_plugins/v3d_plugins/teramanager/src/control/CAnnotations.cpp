#include <algorithm>
#include <vector>
#include <list>
#include "CAnnotations.h"
#include "locale.h"

using namespace teramanager;
using namespace std;

CAnnotations* CAnnotations::uniqueInstance = NULL;
list<int> annotation::availableIDs = list<int>();
list<int> annotation::recyclableIDs = list<int>();

bool isMarker (annotation* ano) { return ano->type == 0;}

void CAnnotations::uninstance()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CAnnotations::uninstance()\n");
    #endif

    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
        annotation::availableIDs.clear();
        annotation::recyclableIDs.clear();
    }
}

CAnnotations::~CAnnotations()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CAnnotations::~CAnnotations()\n");
    #endif

    if(octree)
        delete octree;

    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CAnnotations destroyed\n");
    #endif
}

//recursive support method of 'clear' method
void CAnnotations::Octree::_rec_clear(const Poctant& p_octant) throw(MyException)
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
            delete ano;
        }
    }
}

//recursive support method of 'insert' method
void CAnnotations::Octree::_rec_insert(const Poctant& p_octant, annotation& neuron) throw(MyException)
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
        uint32 V_dim_halved = ROUND((float)p_octant->V_dim/2);
        uint32 H_dim_halved = ROUND((float)p_octant->H_dim/2);
        uint32 D_dim_halved = ROUND((float)p_octant->D_dim/2);

        //child1: [V_start,			V_start+V_dim/2),[H_start,			H_start+H_dim/2),[D_start,			D_start+D_dim/2)
        if	   (neuron.y >= p_octant->V_start	&& neuron.y < p_octant->V_start+V_dim_halved		&&
                   neuron.x >= p_octant->H_start	&& neuron.x < p_octant->H_start+H_dim_halved		&&
                   neuron.z >= p_octant->D_start    && neuron.z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child1)
                p_octant->child1 = new octant(p_octant->V_start,				V_dim_halved,
                                                                              p_octant->H_start,				H_dim_halved,
                                                                              p_octant->D_start,				D_dim_halved);
            //printf("inserting in child1\n");
            _rec_insert(p_octant->child1, neuron);
        }

        //child2: [V_start,			V_start+V_dim/2),[H_start,			H_start+H_dim/2),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron.y >= p_octant->V_start				&& neuron.y < p_octant->V_start+V_dim_halved		&&
                        neuron.x >= p_octant->H_start				&& neuron.x < p_octant->H_start+H_dim_halved		&&
                        neuron.z >= p_octant->D_start+D_dim_halved	&& neuron.z < p_octant->D_start+p_octant->D_dim)
        {
                if(!p_octant->child2)
                        p_octant->child2 = new octant(p_octant->V_start,				V_dim_halved,
                                                                                  p_octant->H_start,				H_dim_halved,
                                                                                  p_octant->D_start+D_dim_halved,	D_dim_halved);
                //printf("inserting in child2\n");
                _rec_insert(p_octant->child2, neuron);
        }


        //child3: [V_start,			V_start+V_dim/2),[H_start+H_dim/2,	H_start+H_dim  ),[D_start,			D_start+D_dim/2)
        else if(neuron.y >= p_octant->V_start				&& neuron.y < p_octant->V_start+V_dim_halved		&&
                        neuron.x >= p_octant->H_start+H_dim_halved	&& neuron.x < p_octant->H_start+p_octant->H_dim		&&
                        neuron.z >= p_octant->D_start				&& neuron.z < p_octant->D_start+D_dim_halved)
        {
                if(!p_octant->child3)
                        p_octant->child3 = new octant(p_octant->V_start,				V_dim_halved,
                                                                                  p_octant->H_start+H_dim_halved,	H_dim_halved,
                                                                                  p_octant->D_start,				D_dim_halved);
                //printf("inserting in child3\n");
                _rec_insert(p_octant->child3, neuron);
        }

        //child4: [V_start,			V_start+V_dim/2),[H_start+H_dim/2,	H_start+H_dim  ),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron.y >= p_octant->V_start				&& neuron.y < p_octant->V_start+V_dim_halved		&&
                        neuron.x >= p_octant->H_start+H_dim_halved	&& neuron.x < p_octant->H_start+p_octant->H_dim		&&
                        neuron.z >= p_octant->D_start+D_dim_halved	&& neuron.z < p_octant->D_start+p_octant->D_dim)
        {
                if(!p_octant->child4)
                        p_octant->child4 = new octant(p_octant->V_start,				V_dim_halved,
                                                                                  p_octant->H_start+H_dim_halved,	H_dim_halved,
                                                                                  p_octant->D_start+D_dim_halved,	D_dim_halved);
                //printf("inserting in child4\n");
                _rec_insert(p_octant->child4, neuron);
        }


        //child5: [V_start+V_dim/2, V_start+V_dim  ),[H_start,			H_start+H_dim/2),[D_start,			D_start+D_dim/2)
        else if(neuron.y >= p_octant->V_start+V_dim_halved	&& neuron.y < p_octant->V_start+p_octant->V_dim		&&
                        neuron.x >= p_octant->H_start				&& neuron.x < p_octant->H_start+H_dim_halved		&&
                        neuron.z >= p_octant->D_start				&& neuron.z < p_octant->D_start+D_dim_halved)
        {
                if(!p_octant->child5)
                        p_octant->child5 = new octant(p_octant->V_start+V_dim_halved,	V_dim_halved,
                                                                                  p_octant->H_start,				H_dim_halved,
                                                                                  p_octant->D_start,				D_dim_halved);
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
                                                                                  p_octant->D_start+D_dim_halved,	D_dim_halved);
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
                                                                                  p_octant->D_start,				D_dim_halved);
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
                                                                                  p_octant->D_start+D_dim_halved,	D_dim_halved);
                //printf("inserting in child8\n");
                _rec_insert(p_octant->child8, neuron);
        }
        else
        {
            char msg[1000];
            sprintf(msg,"in CAnnotations::Octree::insert(...): Cannot find the proper region wherein to insert given neuron [%.0f,%.0f,%.0f]", neuron.y, neuron.x, neuron.z);
            throw MyException(msg);
        }
    }
    else
    {
//        printf("\nIn octant V[%d-%d),H[%d-%d),D[%d-%d), neuron V[%.1f],H[%.1f],D[%.1f] INSERTED\n",
//                p_octant->V_start, p_octant->V_start+p_octant->V_dim,
//                p_octant->H_start, p_octant->H_start+p_octant->H_dim,
//                p_octant->D_start, p_octant->D_start+p_octant->D_dim,
//                neuron.y, neuron.x, neuron.z);
//        if(p_octant->neuron != NULL)
//        {
//            char msg[STATIC_STRING_SIZE];
//            sprintf(msg,"in CAnnotations::Octree::insert(...): duplicate annotation in octant X[%d-%d),Y[%d-%d),Z[%d-%d), ann1={%.2f, %.2f, %.2f}[ID = %d, type=%d], ann2={%.2f, %.2f, %.2f}[ID = %d, type=%d]",
//                        p_octant->H_start, p_octant->H_start+p_octant->H_dim,
//                        p_octant->V_start, p_octant->V_start+p_octant->V_dim,
//                        p_octant->D_start, p_octant->D_start+p_octant->D_dim, neuron.x, neuron.y, neuron.z, neuron.ID, neuron.type,  p_octant->neuron->x, p_octant->neuron->y, p_octant->neuron->z, p_octant->neuron->ID, p_octant->neuron->type);
//            throw MyException(msg);
//        }
        p_octant->n_annotations++;
        neuron.container = static_cast<void*>(p_octant);
        p_octant->annotations.push_back(&neuron);
    }
}

//recursive support method of 'deep_count' method
uint32 CAnnotations::Octree::_rec_deep_count(const Poctant& p_octant) throw(MyException)
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
uint32 CAnnotations::Octree::_rec_height(const Poctant& p_octant) throw(MyException)
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
void CAnnotations::Octree::_rec_search(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int, std::list<annotation*>& neurons)  throw(MyException)
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
            uint32 V_dim_halved = ROUND((float)p_octant->V_dim/2);
            uint32 V_halved		= p_octant->V_start+V_dim_halved;
            uint32 H_dim_halved = ROUND((float)p_octant->H_dim/2);
            uint32 H_halved		= p_octant->H_start+H_dim_halved;
            uint32 D_dim_halved = ROUND((float)p_octant->D_dim/2);
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
CAnnotations::Octree::Poctant CAnnotations::Octree::_rec_find(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int) throw(MyException)
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
            uint32 V_dim_halved = ROUND((float)p_octant->V_dim/2);
            uint32 V_halved	= p_octant->V_start+V_dim_halved;
            uint32 H_dim_halved = ROUND((float)p_octant->H_dim/2);
            uint32 H_halved	= p_octant->H_start+H_dim_halved;
            uint32 D_dim_halved = ROUND((float)p_octant->D_dim/2);
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
uint32 CAnnotations::Octree::_rec_count(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int) throw(MyException)
{
    if(p_octant)
    {
        if(contains(V_int, H_int, D_int, p_octant->V_start, p_octant->V_dim, p_octant->H_start, p_octant->H_dim, p_octant->D_start, p_octant->D_dim))
           return p_octant->n_annotations;
        else
        {
            uint32 neuron_count = 0;
            uint32 V_dim_halved = ROUND((float)p_octant->V_dim/2);
            uint32 V_halved		= p_octant->V_start+V_dim_halved;
            uint32 H_dim_halved = ROUND((float)p_octant->H_dim/2);
            uint32 H_halved		= p_octant->H_start+H_dim_halved;
            uint32 D_dim_halved = ROUND((float)p_octant->D_dim/2);
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
                                                           uint32& V2_start, uint32& V2_dim, uint32& H2_start, uint32& H2_dim, uint32& D2_start, uint32& D2_dim) throw(MyException)
{
    return 	( V1_int.start  < (V2_start + V2_dim)	&&
                  V1_int.end    >  V2_start		&&
                  H1_int.start  < (H2_start + H2_dim)	&&
                  H1_int.end    >  H2_start		&&
                  D1_int.start  < (D2_start + D2_dim)	&&
                  D1_int.end    >  D2_start	 );
}

//returns true if first volume contains second volume
bool inline CAnnotations::Octree::contains  (const interval_t& V1_int,		 const interval_t& H1_int,		   const interval_t& D1_int,
                                             uint32& V2_start, uint32& V2_dim, uint32& H2_start, uint32& H2_dim, uint32& D2_start, uint32& D2_dim) throw(MyException)
{
    return (  V1_int.start  <=  V2_start                &&
              V1_int.end    >=  (V2_start+V2_dim)	&&
              H1_int.start  <=  H2_start		&&
              H1_int.end    >=  (H2_start+H2_dim)	&&
              D1_int.start  <=  D2_start		&&
              D1_int.end    >=  (D2_start+D2_dim));
}

CAnnotations::Octree::Octree(uint32 _DIM_V, uint32 _DIM_H, uint32 _DIM_D)
{
    DIM_V = _DIM_V;
    DIM_H = _DIM_H;
    DIM_D = _DIM_D;
    root = new octant(0,DIM_V,0,DIM_H,0,DIM_D);
}

CAnnotations::Octree::~Octree(void)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CAnnotations::~Octree()\n");
    #endif

    clear();
}

//clears octree content and deallocates used memory
void CAnnotations::Octree::clear() throw(MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CAnnotations::clear()\n");
    #endif

    _rec_clear(root);
    root = 0;
}

//insert given neuron in the octree
void CAnnotations::Octree::insert(annotation& neuron)  throw(MyException)
{
    _rec_insert(root,neuron);
}

//search for the annotations at the given coordinate. If found, returns the address of the annotations list
std::list<annotation*>* CAnnotations::Octree::find(float x, float y, float z) throw(MyException)
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
uint32 CAnnotations::Octree::deep_count() throw(MyException)
{
    return _rec_deep_count(root);
}

//returns the octree height
uint32 CAnnotations::Octree::height() throw(MyException)
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
void CAnnotations::Octree::find(interval_t V_int, interval_t H_int, interval_t D_int, std::list<annotation*>& neurons) throw(MyException)
{
    _rec_search(root, V_int, H_int, D_int, neurons);
}

//returns the number of neurons (=leafs) in the given volume without exploring the entire data structure
uint32 CAnnotations::Octree::count(interval_t V_int, interval_t H_int, interval_t D_int) throw(MyException)
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
void CAnnotations::addLandmarks(LandmarkList* markers) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CAnnotations::addLandmarks(markers[size = %d])\n",
           markers->size());
    #endif

    for(int i=0; i<markers->size(); i++)
    {
       annotation* node = new annotation();
       node->type = 0;
       node->subtype = (*markers)[i].category;
       node->next = node->prev = 0;
       node->r = (*markers)[i].radius;
       node->x = (*markers)[i].x;
       node->y = (*markers)[i].y;
       node->z = (*markers)[i].z;
       node->name = (*markers)[i].name;
       node->comment = (*markers)[i].comments;
       node->color = (*markers)[i].color;
//       printf("--------------------- teramanager plugin [thread ?] >> inserting marker %d=(%.1f,%.1f,%.1f)\n", node->ID, node->x, node->y, node->z);
       octree->insert(*node);
    }
}

void CAnnotations::removeLandmarks(std::list<LocationSimple> &markers) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CAnnotations::removeLandmarks(markers[size = %d])\n",
           markers.size());
    #endif

    //if the octree is instantiated
    if(octree)
    {
        //retrieving the list of the annotations to be deleted
        std::list<annotation*> tbdList;
        for(std::list<LocationSimple>::iterator i = markers.begin(); i != markers.end(); i++)
        {
            std::list<annotation*>* anoListP = octree->find(i->x, i->y, i->z);
            if(anoListP)
            {
                for(std::list<annotation*>::iterator it = anoListP->begin(); it != anoListP->end(); it++)
                {
                    //printf("--------------------- teramanager plugin >> checking annotation %d=(%.1f,%.1f,%.1f)\n", (*it)->ID, (*it)->x, (*it)->y, (*it)->z);
                    if((*it)->type == 0)
                        tbdList.push_back(*it);
                }
            }
            else
                printf("--------------------- teramanager plugin >> marker (%.1f, %.1f, %.1f) not found in the Octree!!!\n", i->x, i->y, i->z);

        }
        tbdList.sort();
        tbdList.unique();

        //removing annotations from the Octree
        for(std::list<annotation*>::iterator it = tbdList.begin(); it != tbdList.end(); it++)
            static_cast<Octree::octant*>((*it)->container)->annotations.remove((*it));

        //deallocating annotations
        while(!tbdList.empty())
        {
            annotation* tbd = tbdList.back();
            tbdList.pop_back();
//            printf("--------------------- teramanager plugin [thread ?] >> removing marker %d=(%.1f,%.1f,%.1f)\n",
//                   tbd->ID, tbd->x, tbd->y, tbd->z);
            delete tbd;
        }
    }
}


void CAnnotations::removeCurves(std::list<NeuronSWC> &curves) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CAnnotations::removeCurves(curves->listNeuron[size = %d])\n", curves.size());
    #endif

    if(octree)
    {
        //retrieving source points of curves to be removed
        std::list<annotation*> sources;
        for(std::list<NeuronSWC>::iterator i = curves.begin(); i != curves.end(); i++)
        {
            if(i->pn == -1)
            {
                std::list<annotation*>* anoListP = octree->find(i->x, i->y, i->z);
                if(anoListP)
                {
                    //int sizeStored = sources.size();
                    for(std::list<annotation*>::iterator it = anoListP->begin(); it != anoListP->end(); it++)
                    {
                        if((*it)->type == 1 && (*it)->prev == 0)
                            sources.push_back(*it);
                    }
                    //if(sources.size()-sizeStored > 1)
                        //throw MyException("in CAnnotations::removeCurves(): found 2 coincident curves starting points. Curves sharing the same source point are not supported yet.");
                }
                else
                    printf("--------------------- teramanager plugin >> source curve point (%.1f, %.1f, %.1f) not found in the Octree!!!\n", i->x, i->y, i->z);
            }
        }
        sources.sort();
        sources.unique();

        //removing curve points
        std::list<annotation*> tbdList;
        for(std::list<annotation*>::iterator i = sources.begin(); i != sources.end(); i++)
        {
            annotation* anoP = *i;
//            printf("--------------------- teramanager plugin >> preparing to remove curve whose SOURCE is %d=(%.1f,%.1f,%.1f), address=%d\n",
//                   anoP->ID, anoP->x, anoP->y, anoP->z, anoP);
            while(anoP)
            {
                static_cast<Octree::octant*>(anoP->container)->annotations.remove(anoP);
                tbdList.push_back(anoP);
                anoP = anoP->next;
            }
        }
        while(!tbdList.empty())
        {
            annotation* tbd = tbdList.back();
            tbdList.pop_back();
//            printf("--------------------- teramanager plugin [thread ?] >> removing curve point %d=(%.1f,%.1f,%.1f)\n",
//                   tbd->ID, tbd->x, tbd->y, tbd->z);
            delete tbd;
        }
    }
}

void CAnnotations::addCurves(NeuronTree* curves) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CAnnotations::addCurves(curves->listNeuron[size() = %d])\n",
           curves->listNeuron.size());
    #endif

    std::map<int, annotation*> annotationsMap;
    std::map<int, NeuronSWC*> swcMap;
    for(QList <NeuronSWC>::iterator i = curves->listNeuron.begin(); i!= curves->listNeuron.end(); i++)
    {
        annotation* ann = new annotation();
        ann->type = 1;
        ann->name = curves->name.toStdString();
        ann->comment = curves->comment.toStdString();
        ann->color = curves->color;
        ann->subtype = i->type;
        ann->r = i->r;
        ann->x = i->x;
        ann->y = i->y;
        ann->z = i->z;
//        printf("--------------------- teramanager plugin [thread ?] >> inserting curve point %d(n=%d)=(%.1f,%.1f,%.1f)\n", ann->ID, i->n, ann->x, ann->y, ann->z);
        octree->insert(*ann);
        annotationsMap[i->n] = ann;
        swcMap[i->n] = &(*i);
    }
    for(std::map<int, annotation*>::iterator i = annotationsMap.begin(); i!= annotationsMap.end(); i++)
    {
        i->second->prev = swcMap[i->first]->pn == -1 ? 0 : annotationsMap[swcMap[i->first]->pn];
        if(i->second->prev)
            i->second->prev->next = i->second;
    }
//    printf("--------------------- teramanager plugin >> inserted %d curve points\n", annotationsMap.size());
}

/*********************************************************************************
* Retrieves the annotation(s) in the given volume space
**********************************************************************************/
void CAnnotations::findLandmarks(interval_t X_range, interval_t Y_range, interval_t Z_range, std::list<LocationSimple> &markers) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CAnnotations::findLandmarks(X[%d,%d), Y[%d,%d), Z[%d,%d))",
           X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end);
    #endif

    std::list<annotation*> nodes;
    octree->find(Y_range, X_range, Z_range, nodes);
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
    printf("...%d markers loaded\n", markers.size());
}

void CAnnotations::findCurves(interval_t X_range, interval_t Y_range, interval_t Z_range, std::list<NeuronSWC> &curves) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CAnnotations::findCurves(X[%d,%d), Y[%d,%d), Z[%d,%d))",
           X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end);
    #endif

    std::list<annotation*> nodes;
    octree->find(Y_range, X_range, Z_range, nodes);

    //finding curve sources (i.e. starting points)
    std::vector<annotation*> sources;
    for(std::list<annotation*>::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if((*i)->type == 1) //selecting curve points
        {
            annotation* source = (*i);

            while(source->prev != 0)
                source = source->prev;
            if(find(sources.begin(),sources.end(), source) == sources.end())
                sources.push_back(source);
        }
    }
    for(int i=0; i<sources.size(); i++)
    {
        annotation* annP = sources[i];
        while(annP)
        {
            NeuronSWC p;
            p.type = annP->subtype;
            p.n = annP->ID;
            p.x = annP->x;
            p.y = annP->y;
            p.z = annP->z;
            p.r = annP->r;
            p.pn = annP->prev ? annP->prev->ID : -1;
            curves.push_back(p);
            annP = annP->next;
        }
    }
    printf("...%d curve points loaded (nodes = %d, sources = %d)\n", curves.size(), nodes.size(), sources.size());
}

/*********************************************************************************
* Save/load methods
**********************************************************************************/
void CAnnotations::save(const char* filepath) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CAnnotations::save()\n");
    #endif

    //retrieving annotations
    std::list<annotation*> annotations;
    if(octree)
        octree->find(interval_t(0, octree->DIM_V), interval_t(0, octree->DIM_H), interval_t(0, octree->DIM_D), annotations);

    //saving ano file
    QDir anoFile(filepath);
    FILE* f = fopen(filepath, "w");
    if(!f)
    {
        char errMsg[STATIC_STRING_SIZE];
        sprintf(errMsg, "in CAnnotations::save(): cannot save to path \"%s\"", filepath);
        throw MyException(errMsg);
    }
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
                fprintf(f, "%d %d %.3f %.3f %.3f %.3f %d\n", (*i)->ID, (*i)->subtype, (*i)->x, (*i)->y, (*i)->z, (*i)->r, (*i)->prev ? (*i)->prev->ID : -1);

    //file closing
    fclose(f);
}
void CAnnotations::load(const char* filepath) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CAnnotations::load()\n");
    #endif

    //precondition checks
    if(!octree)
        throw MyException("CAnnotations::load(): octree not yet initialized");

    //clearing annotations
    this->clear();

    //opening ANO file in read mode
    char errMsg[STATIC_STRING_SIZE];
    FILE* f = fopen(filepath, "r");
    if(!f)
    {
        sprintf(errMsg, "in CAnnotations::save(): cannot load file \"%s\"", filepath);
        throw MyException(errMsg);
    }

    //reading ANO file line by line
    char lineBuf[FILE_LINE_BUFFER_SIZE];
    while(fgets(lineBuf, FILE_LINE_BUFFER_SIZE, f))
    {
        char tokenizer[STATIC_STRING_SIZE];
        char filetype[STATIC_STRING_SIZE];
        char filename[STATIC_STRING_SIZE];
        if(sscanf(lineBuf, "%s", tokenizer) != 1)
        {
            sprintf(errMsg, "in CAnnotations::load(const char* filepath = \"%s\"): expected line \"%s\", found \"%s\"",
                    filepath, "<filetype>=<filename>", lineBuf);
            throw MyException(errMsg);
        }
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
                printf("--------------------- teramanager plugin >> inserting marker %d=(%.1f,%.1f,%.1f)\n", ann->ID, ann->x, ann->y, ann->z);
                octree->insert(*ann);
            }
            printf("--------------------- teramanager plugin >> inserted %d markers\n", cells.size());
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
                printf("--------------------- teramanager plugin >> inserting curve point %d(%d)=(%.1f,%.1f,%.1f)\n", ann->ID, i->n, ann->x, ann->y, ann->z);
                octree->insert(*ann);
                annotationsMap[i->n] = ann;
                swcMap[i->n] = &(*i);
            }
            for(std::map<int, annotation*>::iterator i = annotationsMap.begin(); i!= annotationsMap.end(); i++)
            {
                i->second->prev = swcMap[i->first]->pn == -1 ? 0 : annotationsMap[swcMap[i->first]->pn];
                if(i->second->prev)
                    i->second->prev->next = i->second;
            }
            printf("--------------------- teramanager plugin >> inserted %d curve points\n", annotationsMap.size());
        }
        else
        {
            sprintf(errMsg, "in CAnnotations::load(const char* filepath = \"%s\"): unable to recognize file type \"%s\"", filepath, filetype);
            throw MyException(errMsg);
        }

    }
    fclose(f);
}
