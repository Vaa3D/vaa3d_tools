#ifndef CANNOTATIONS_H
#define CANNOTATIONS_H

#include "CPlugin.h"

//annotation structure
struct teramanager::annotation
{
    int ID;                     //unique identifier
    int type;			//-1 = undefined, 0 = LocationSimple, 1 = NeuronSWC
    int subtype;                //see Vaa3D LocationSimple and NeuronSWC types
    float x, y, z;		//point coordinates
    float r;			//radius
    std::string name;           //name
    std::string comment;        //comment
    RGBA8 color;                //color
    annotation* prev;           //previous annotation handle (used in case of linked structures)
    annotation* next;           //next annotation handle (used in case of linked structures)
    void* container;            //address of the container object

    annotation() throw (MyException){
        type = subtype  = teramanager::undefined_int32;
        r = x = y = z = teramanager::undefined_real32;
        prev = next = 0;
        name = comment = "";
        color.r = color.g = color.b = color.a = 0;
        container = 0;
        if(!recyclableIDs.empty())
        {
            ID = recyclableIDs.front();
            recyclableIDs.pop_front();
        }
        else if(!availableIDs.empty())
        {
            ID = availableIDs.front();
            availableIDs.pop_front();
        }
        else
        {
            char errMsg[STATIC_STRING_SIZE];
            sprintf(errMsg, "in annotation(): no more IDs available. Possible reasons are too many annotations inseted (maximum is %d) or a memory leak."
                    "\n\nPlease signal this bug to the developers.", MAX_ANNOTATIONS_NUMBER);
            throw MyException(errMsg);
        }
    }
    ~annotation(){
        recyclableIDs.push_back(ID);
    }

    static std::list<int> availableIDs;     //list of available IDs, i.e. IDs that can be assigned and that were never assigned before
    static std::list<int> recyclableIDs;    //list of recyclable IDs, i.e. IDs that can be assigned after their owner has been destroyed
};

class teramanager::CAnnotations
{

    private:

        //octree structure
        class Octree
        {
            private:

                struct octant
                {
                    //VHD intervals that delimit the octant
                    uint32 V_start, V_dim;
                    uint32 H_start, H_dim;
                    uint32 D_start, D_dim;

                    //number of neurons in the octant
                    uint32 n_annotations;

                    //annotations
                    std::list<teramanager::annotation*> annotations;

                    //pointers to children octants
                    octant *child1;	//[V_start,         V_start+V_dim/2),[H_start,		H_start+H_dim/2),[D_start,		D_start+D_dim/2)
                    octant *child2;	//[V_start,         V_start+V_dim/2),[H_start,		H_start+H_dim/2),[D_start+D_dim/2,	D_start+D_dim  )
                    octant *child3;	//[V_start,         V_start+V_dim/2),[H_start+H_dim/2,	H_start+H_dim  ),[D_start,		D_start+D_dim/2)
                    octant *child4;	//[V_start,         V_start+V_dim/2),[H_start+H_dim/2,	H_start+H_dim  ),[D_start+D_dim/2,	D_start+D_dim  )
                    octant *child5;	//[V_start+V_dim/2, V_start+V_dim  ),[H_start,		H_start+H_dim/2),[D_start,		D_start+D_dim/2)
                    octant *child6;	//[V_start+V_dim/2, V_start+V_dim  ),[H_start,		H_start+H_dim/2),[D_start+D_dim/2,      D_start+D_dim  )
                    octant *child7;	//[V_start+V_dim/2, V_start+V_dim  ),[H_start+H_dim/2,	H_start+H_dim  ),[D_start,		D_start+D_dim/2)
                    octant *child8;	//[V_start+V_dim/2, V_start+V_dim  ),[H_start+H_dim/2,	H_start+H_dim  ),[D_start+D_dim/2,	D_start+D_dim  )

                    octant(uint32 _V_start, uint32 _V_dim, uint32 _H_start, uint32 _H_dim, uint32 _D_start, uint32 _D_dim)
                    {
                            child1  = child2 = child3 = child4 = child5 = child6 = child7 = child8 = NULL;
                            V_start = _V_start;
                            V_dim   = _V_dim;
                            H_start = _H_start;
                            H_dim   = _H_dim;
                            D_start = _D_start;
                            D_dim   = _D_dim;
                            n_annotations = 0;
                    }
                };
                typedef octant* Poctant;

                uint32 DIM_V, DIM_H, DIM_D;		//volume dimensions (in voxels) along VHD axes
                octant *root;				//pointer to root octant
                Octree(void){}				//default constructor is not available

                /*** SUPPORT methods ***/

                //recursive support methods
                void     _rec_clear(const Poctant& p_octant) throw(MyException);
                void     _rec_insert(const Poctant& p_octant, annotation& neuron) throw(MyException);
                uint32   _rec_deep_count(const Poctant& p_octant) throw(MyException);
                uint32   _rec_height(const Poctant& p_octant) throw(MyException);
                void     _rec_print(const Poctant& p_octant);
                void     _rec_search(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int, std::list<annotation*>& neurons) throw(MyException);
                Poctant  _rec_find(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int) throw(MyException);
                uint32   _rec_count(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int) throw(MyException);

                //returns true if two given volumes intersect each other
                bool inline intersects(const interval_t& V1_int,const interval_t& H1_int,const interval_t& D1_int,
                                       uint32& V2_start, uint32& V2_dim, uint32& H2_start, uint32& H2_dim, uint32& D2_start, uint32& D2_dim)  throw(MyException);

                //returns true if first volume contains second volume
                bool inline contains  (const interval_t& V1_int, const interval_t& H1_int, const interval_t& D1_int,
                                       uint32& V2_start, uint32& V2_dim, uint32& H2_start, uint32& H2_dim, uint32& D2_start, uint32& D2_dim)  throw(MyException);

            public:

                //CONSTRUCTOR and DECONSTRUCTOR
                Octree(uint32 _DIM_V, uint32 _DIM_H, uint32 _DIM_D);
                ~Octree(void);

                //clears octree content and deallocates used memory
                void clear() throw(MyException);

                //insert given neuron in the octree
                void insert(annotation& neuron) throw(MyException);

                //search for neurons in the given 3D volume and puts found neurons into 'neurons'
                void find(interval_t V_int, interval_t H_int, interval_t D_int, std::list<annotation*>& neurons) throw(MyException);

                //search for the annotations at the given coordinate. If found, returns the address of the annotations list
                std::list<annotation*>* find(float x, float y, float z) throw(MyException);

                //returns the number of neurons (=leafs) in the given volume without exploring the entire data structure
                uint32 count(interval_t V_int = interval_t(-1,-1), interval_t H_int = interval_t(-1,-1), interval_t D_int = interval_t(-1,-1))  throw(MyException);

                //returns the number of neurons (=leafs) in the octree by exploring the entire data structure
                uint32 deep_count()  throw(MyException);

                //returns the octree height
                uint32 height()  throw(MyException);

                //print the octree content
                void print();

                friend class CAnnotations;
        };


        //members
        Octree* octree;         //octree associated to the 3D image space where annotations are stored
        int octreeDimX;
        int octreeDimY;
        int octreeDimZ;

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        CAnnotations() : octree(0), octreeDimX(undefined_int32), octreeDimY(undefined_int32), octreeDimZ(undefined_int32){}
        static CAnnotations* uniqueInstance;
        CAnnotations(uint32 volHeight, uint32 volWidth, uint32 volDepth) : octreeDimX(volWidth), octreeDimY(volHeight), octreeDimZ(volDepth)
        {
            #ifdef TMP_DEBUG
            printf("--------------------- teramanager plugin [thread ?] >> CAnnotations::CAnnotations(volHeight = %d, volWidth = %d, volDepth = %d)\n",
                   volHeight, volWidth, volDepth);
            #endif

            octree = new Octree(octreeDimY, octreeDimX, octreeDimZ);

            #ifdef TMP_DEBUG
            printf("--------------------- teramanager plugin [thread ?] >> CAnnotations created\n");
            #endif
        }

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CAnnotations* instance(uint32 volHeight, uint32 volWidth, uint32 volDepth)
        {
            if (uniqueInstance == 0)
            {
                for(int i=0; i<teramanager::MAX_ANNOTATIONS_NUMBER; i++)
                    annotation::availableIDs.push_back(i);
                uniqueInstance = new CAnnotations(volHeight, volWidth, volDepth);
            }
            return uniqueInstance;
        }
        static CAnnotations* getInstance() throw (MyException)
        {
            if(uniqueInstance)
                return uniqueInstance;
            else
                throw MyException("in CAnnotations::getInstance(): no object has been instantiated yet");
        }
        static void uninstance();
        ~CAnnotations();

        /*********************************************************************************
        * Adds/removes the given annotation(s)
        **********************************************************************************/
        void addLandmarks(LandmarkList* markers) throw (MyException);
        void removeLandmarks(std::list<LocationSimple> &markers) throw (MyException);
        void addCurves(NeuronTree* curves) throw (MyException);
        void removeCurves(std::list<NeuronSWC> &curves) throw (MyException);

        /*********************************************************************************
        * Retrieves the annotation(s) in the given volume space
        **********************************************************************************/
        void findLandmarks(interval_t X_range, interval_t Y_range, interval_t Z_range, std::list<LocationSimple> &markers) throw (MyException);
        void findCurves(interval_t X_range, interval_t Y_range, interval_t Z_range, std::list<NeuronSWC> &curves) throw (MyException);

        /*********************************************************************************
        * Save/load method
        **********************************************************************************/
        void save(const char* filepath) throw (MyException);
        void load(const char* filepath) throw (MyException);

        /*********************************************************************************
        * Removes all the annotations from the octree
        **********************************************************************************/
        void clear()  throw (MyException)
        {
            #ifdef TMP_DEBUG
            printf("--------------------- teramanager plugin [thread ?] >> CAnnotations::clear()\n");
            #endif

            delete octree;
            octree = new Octree(octreeDimY, octreeDimX, octreeDimZ);
        }


};

#endif // CANNOTATIONS_H
