#include "CAnnotations.h"

using namespace teramanager;

CAnnotations* CAnnotations::uniqueInstance = NULL;
int CAnnotations::annotation::total = 0;

void CAnnotations::uninstance()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread unknown] >> CAnnotations uninstance() called\n");
    #endif

    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

CAnnotations::~CAnnotations()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread unknown] >> CAnnotations destroyed\n");
    #endif
    if(octree)
        delete octree;
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

        if(p_octant->neuron)
            delete p_octant->neuron;
        delete p_octant;
    }
}

//recursive support method of 'insert' method
void CAnnotations::Octree::_rec_insert(const Poctant& p_octant, CAnnotations::annotation& neuron) throw(MyException)
{
    //if the octant is greater than a point, the insert is recursively postponed until a 1x1x1 leaf octant is reached
    if(p_octant->V_dim > 1 || p_octant->H_dim > 1 || p_octant->D_dim > 1)
    {
        /*printf("\nIn octant V[%d-%d],H[%d-%d],D[%d-%d], neuron V[%d],H[%d],D[%d]\n",
                p_octant->V_start, p_octant->V_start+p_octant->V_dim,
                p_octant->H_start, p_octant->H_start+p_octant->H_dim,
                p_octant->D_start, p_octant->D_start+p_octant->D_dim,
                neuron.y, neuron.x, neuron.z);*/
        p_octant->n_neurons++;
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
        if(p_octant->neuron != NULL)
        {
            char msg[1000];
            sprintf(msg,"in CAnnotations::Octree::insert(...): duplicate neuron inserted at [%.0f,%.0f,%.0f]", neuron.y, neuron.x, neuron.z);
            throw MyException(msg);
        }
        p_octant->neuron = &neuron;
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
        printf("V[%d-%d],H[%d-%d],D[%d-%d]\n",p_octant->V_start, p_octant->V_start+p_octant->V_dim,
                                                                                  p_octant->H_start, p_octant->H_start+p_octant->H_dim,
                                                                                  p_octant->D_start, p_octant->D_start+p_octant->D_dim);
        if(p_octant->neuron)
            printf("|===> %.2f %.2f %.2f\n", p_octant->neuron->y, p_octant->neuron->x, p_octant->neuron->z);
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
void CAnnotations::Octree::_rec_find(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int, std::list<CAnnotations::annotation*>& neurons)  throw(MyException)
{
    if(p_octant)
    {
        if(p_octant->V_dim == 1 && p_octant->H_dim == 1 && p_octant->D_dim == 1 && p_octant->neuron)
                neurons.push_back(p_octant->neuron);
        else
        {
            uint32 V_dim_halved = ROUND((float)p_octant->V_dim/2);
            uint32 V_halved		= p_octant->V_start+V_dim_halved;
            uint32 H_dim_halved = ROUND((float)p_octant->H_dim/2);
            uint32 H_halved		= p_octant->H_start+H_dim_halved;
            uint32 D_dim_halved = ROUND((float)p_octant->D_dim/2);
            uint32 D_halved		= p_octant->D_start+D_dim_halved;

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start, H_dim_halved, p_octant->D_start,	D_dim_halved))
                _rec_find(p_octant->child1, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start, H_dim_halved, D_halved,			D_dim_halved))
                _rec_find(p_octant->child2, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,		   H_dim_halved, p_octant->D_start,	D_dim_halved))
                _rec_find(p_octant->child3, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,		   H_dim_halved, D_halved,			D_dim_halved))
                _rec_find(p_octant->child4, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	p_octant->H_start, H_dim_halved, p_octant->D_start,	D_dim_halved))
                _rec_find(p_octant->child5, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	p_octant->H_start, H_dim_halved, D_halved,			D_dim_halved))
                 _rec_find(p_octant->child6, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	H_halved,		   H_dim_halved, p_octant->D_start,	D_dim_halved))
                 _rec_find(p_octant->child7, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	H_halved,		   H_dim_halved, D_halved,			D_dim_halved))
                 _rec_find(p_octant->child8, V_int, H_int, D_int, neurons);
        }
    }
}

//recursive support method of 'remove' method
bool CAnnotations::Octree::_rec_remove(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int)  throw(MyException)
{
    if(p_octant)
    {
        if(p_octant->V_dim == 1 && p_octant->H_dim == 1 && p_octant->D_dim == 1 && p_octant->neuron)
        {
            delete p_octant->neuron;
            p_octant->neuron = 0;
            return true;
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
                _rec_remove(p_octant->child1, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start, H_dim_halved, D_halved,			D_dim_halved))
                _rec_remove(p_octant->child2, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,		   H_dim_halved, p_octant->D_start,	D_dim_halved))
                _rec_remove(p_octant->child3, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,		   H_dim_halved, D_halved,			D_dim_halved))
                _rec_remove(p_octant->child4, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	p_octant->H_start, H_dim_halved, p_octant->D_start,	D_dim_halved))
                _rec_remove(p_octant->child5, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	p_octant->H_start, H_dim_halved, D_halved,			D_dim_halved))
                 _rec_remove(p_octant->child6, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	H_halved,		   H_dim_halved, p_octant->D_start,	D_dim_halved))
                 _rec_remove(p_octant->child7, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	H_halved,		   H_dim_halved, D_halved,			D_dim_halved))
                 _rec_remove(p_octant->child8, V_int, H_int, D_int);
        }
    }
    else
        return false;
}

//recursive support method of 'count' method
uint32 CAnnotations::Octree::_rec_count(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int) throw(MyException)
{
    if(p_octant)
    {
        if(contains(V_int, H_int, D_int, p_octant->V_start, p_octant->V_dim, p_octant->H_start, p_octant->H_dim, p_octant->D_start, p_octant->D_dim))
           return p_octant->n_neurons;
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
     clear();
}

//clears octree content and deallocates used memory
void CAnnotations::Octree::clear() throw(MyException)
{
    _rec_clear(root);
    root = new octant(0,DIM_V,0,DIM_H,0,DIM_D);
}

//insert given neuron in the octree
void CAnnotations::Octree::insert(CAnnotations::annotation& neuron)  throw(MyException)
{
    _rec_insert(root,neuron);
}

//removes the given neuron in the octree
bool CAnnotations::Octree::remove(annotation& neuron) throw(MyException)
{
    interval_t V_range(static_cast<int>(floor(neuron.y)), static_cast<int>(ceil(neuron.y)));
    interval_t H_range(static_cast<int>(floor(neuron.x)), static_cast<int>(ceil(neuron.x)));
    interval_t D_range(static_cast<int>(floor(neuron.z)), static_cast<int>(ceil(neuron.z)));
    return _rec_remove(root, V_range, H_range, D_range);
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
void CAnnotations::Octree::find(interval_t V_int, interval_t H_int, interval_t D_int, std::list<CAnnotations::annotation*>& neurons) throw(MyException)
{
    _rec_find(root, V_int, H_int, D_int, neurons);
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
    printf("--------------------- teramanager plugin [thread unknown] >> CAnnotations::addLandmarks(markers(size = %d))\n",
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
       octree->insert(*node);
    }
}

void CAnnotations::removeLandmarks(LandmarkList* markers) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread unknown] >> CAnnotations::removeLandmarks(markers(size = %d))\n",
           markers->size());
    #endif

    for(int i=0; i<markers->size(); i++)
    {
        annotation node;
        node.x = (*markers)[i].x;
        node.y = (*markers)[i].y;
        node.z = (*markers)[i].z;
        octree->remove(node);
    }
}

void CAnnotations::addCurves(NeuronTree* curves) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread unknown] >> CAnnotations::addCurves(curves->listNeuron.size() = %d))\n",
           curves->listNeuron.size());
    #endif

    //checking that contour points are stored contiguously
    for(int i=0; i<curves->listNeuron.size(); i++)
    {
        //printf("%d\t{%.1f %.1f %.1f}, pn = %d, hash = %d\n", i, curves->listNeuron[i].x, curves->listNeuron[i].y, curves->listNeuron[i].z,
               //curves->listNeuron[i].pn, curves->hashNeuron[i]);
        if(curves->listNeuron[i].pn != -1 && curves->listNeuron[i].pn != i)
            throw MyException("in CAnnotations::addCurves(...): non consecutive contour points found");
    }

    //creating and storing annotations in the octree
    annotation* prev = 0;
    for(int i=0; i<curves->listNeuron.size(); i++)
    {
        annotation* node = new annotation();
        node->type = 1;
        node->subtype = curves->listNeuron[i].type;

        if(curves->listNeuron[i].pn == -1)
        {
            if(prev)
                prev->next = 0;
            node->prev = 0;
        }
        else
        {
            prev->next = node;
            node->prev = prev;
        }

        node->r = curves->listNeuron[i].r;
        node->x = curves->listNeuron[i].x;
        node->y = curves->listNeuron[i].y;
        node->z = curves->listNeuron[i].z;
        octree->insert(*node);
        prev = node;
    }
    if(prev)
        prev->next = 0;
}

/*********************************************************************************
* Retrieves the annotation(s) in the given volume space
**********************************************************************************/
void CAnnotations::findLandmarks(interval_t X_range, interval_t Y_range, interval_t Z_range, LandmarkList& markers) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread unknown] >> CAnnotations::findLandmarks(X[%d,%d), Y[%d,%d), Z[%d,%d))",
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
            markers.push_back(marker);
        }
    }
    printf("...%d markers loaded\n", markers.size());
}

void CAnnotations::findCurves(interval_t X_range, interval_t Y_range, interval_t Z_range, NeuronTree& curves) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread unknown] >> CAnnotations::findCurves(X[%d,%d), Y[%d,%d), Z[%d,%d))\n",
           X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end);
    #endif

//    std::list<annotation*> nodes;
//    std::list<annotation*> curves;
//    octree->find(Y_range, X_range, Z_range, nodes);
//    for(std::list<annotation*>::iterator i = nodes.begin(); i != nodes.end(); i++)
//    {
//        if((*i)->type == 1 && (*i)->prev == 0 && (*i)->next != 0) //selecting curve points
//        {
//            LocationSimple marker;
//            marker.x = (*i)->x;
//            marker.y = (*i)->y;
//            marker.z = (*i)->z;
//            marker.radius = (*i)->r;
//            marker.category = (*i)->subtype;
//            markers.push_back(marker);
//        }
//    }
    //printf("...%d curve points loaded\n", markers.size());
}

/*********************************************************************************
* Save/load methods
**********************************************************************************/
void CAnnotations::save(const char* filepath) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread unknown] >> CAnnotations::save()\n");
    #endif

    //opening file in write mode
    FILE* f = fopen(filepath, "w");
    if(!f)
    {
        char errMsg[STATIC_STRING_SIZE];
        sprintf(errMsg, "in CAnnotations::save(): cannot save to path \"%s\"", filepath);
        throw MyException(errMsg);
    }

    //storing file header
    fprintf(f, "#name terafly_annotations\n");
    fprintf(f, "#voldims %d %d %d\n", octree->DIM_H, octree->DIM_V, octree->DIM_D);
    fprintf(f, "#n type subtype x y z radius parent\n");

    //storing individual annotations
    if(octree)
    {        
        std::list<annotation*> annotations;
        octree->find(interval_t(0, octree->DIM_V), interval_t(0, octree->DIM_H), interval_t(0, octree->DIM_D), annotations);

        //ordering by ascending ID
        annotations.sort(annotation::compareAnnotations);

        for(std::list<annotation*>::iterator i = annotations.begin(); i != annotations.end(); i++)
            fprintf(f, "%d %d %d %.3f %.3f %.3f %.3f %d\n", (*i)->ID, (*i)->type, (*i)->subtype, (*i)->x, (*i)->y, (*i)->z, (*i)->r, (*i)->prev ? (*i)->prev->ID : -1);
    }

    //file closing
    fclose(f);
}
void CAnnotations::load(const char* filepath) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread unknown] >> CAnnotations::load()\n");
    #endif

    //precondition checks
    if(!octree)
        throw MyException("CAnnotations::load(): octree not yet initialized");

    //clearing annotations data structure
    octree->clear();

    //opening file in read mode
    char errMsg[STATIC_STRING_SIZE];
    FILE* f = fopen(filepath, "r");
    if(!f)
    {
        sprintf(errMsg, "in CAnnotations::save(): cannot save to path \"%s\"", filepath);
        throw MyException(errMsg);
    }

    //reading file header and checking its correctness
    char lineBuf[FILE_LINE_BUFFER_SIZE];
    fgets(lineBuf, FILE_LINE_BUFFER_SIZE, f);
    if(strncmp(lineBuf, "#name terafly_annotations", strlen("#name terafly_annotations")) != 0 )
    {
        sprintf(errMsg, "in CAnnotations::load(const char* filepath = \"%s\"): expected line \"%s\", found \"%s\"",
                filepath, "#name terafly_annotations", lineBuf);
        throw MyException(errMsg);
    }
    fgets(lineBuf, FILE_LINE_BUFFER_SIZE, f);
    int DIM_V=-1, DIM_H=-1, DIM_D=-1;
    if(sscanf(lineBuf, "%*s %d %d %d", &DIM_V, &DIM_H, &DIM_D) != 3)
    {
        sprintf(errMsg, "in CAnnotations::load(const char* filepath = \"%s\"): expected line \"%s\", found \"%s\"",
                filepath, "#voldims <number> <number> <number>", lineBuf);
        throw MyException(errMsg);
    }
    if(DIM_V != octree->DIM_V || DIM_H != octree->DIM_H || DIM_D != octree->DIM_D)
    {
        sprintf(errMsg, "in CAnnotations::load(const char* filepath = \"%s\"): volume dimensions mismatch. Expected \"%d %d %d\", found \"%d %d %d\"",
                filepath, octree->DIM_V, octree->DIM_H, octree->DIM_D, DIM_V, DIM_H, DIM_D);
        throw MyException(errMsg);
    }    
    fgets(lineBuf, FILE_LINE_BUFFER_SIZE, f);

    //reading individual annotations
    std::map<int, annotation*> annotations;
    while(fgets(lineBuf, FILE_LINE_BUFFER_SIZE, f))
    {
        //reading i-th annotation fields
        int ID=-1, type=-1, subtype=-1, parent=-1;
        float x=-1.0f, y=-1.0f, z=-1.0f, radius=-1.0f;
        if(sscanf(lineBuf, "%d %d %d %f %f %f %f %d", &ID, &type, &subtype, &x, &y, &z, &radius, &parent) != 8)
        {
            sprintf(errMsg, "in CAnnotations::load(const char* filepath = \"%s\"): expected line \"%s\", found \"%s\"",
                    filepath, "<number> <number> <number> <number> <number> <number> <number> <number>", lineBuf);
            throw MyException(errMsg);
        }

        //checking if there are duplicates
        if(annotations.find(ID) != annotations.end())
        {
            sprintf(errMsg, "in CAnnotations::load(const char* filepath = \"%s\"): duplicate annotation elements found", lineBuf);
            throw MyException(errMsg);
        }

        //inserting annotation
        annotation* ann = new annotation(ID, type, subtype, x, y, z, radius, 0, 0);
        octree->insert(*ann);

        //linking annotation to its predecessor (this is possible IFF annotations are stored in ascending order on the ID column)
        if(parent != -1)
        {
            if(annotations.find(parent) == annotations.end())
            {
                sprintf(errMsg, "in CAnnotations::load(const char* filepath = \"%s\"): annotations are not in ascending order on the ID column", lineBuf);
                throw MyException(errMsg);
            }
            ann->prev = annotations[parent];
            ann->prev->next = ann;
        }
    }

    //file closing
    fclose(f);
}
