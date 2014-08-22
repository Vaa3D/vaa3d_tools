#ifndef CANNOTATIONS_H
#define CANNOTATIONS_H

#include <set>

#include "CPlugin.h"
#include "math.h"

//annotation structure
struct teramanager::annotation
{
    long long ID;                   //unique identifier
    int type;                       //-1 = undefined, 0 = LocationSimple, 1 = NeuronSWC
    int subtype;                    //see Vaa3D LocationSimple and NeuronSWC types
    float x, y, z;                  //point coordinates
    float r;                        //radius
    std::string name;               //name
    std::string comment;            //comment
    RGBA8 color;                    //color
    annotation* parent;             //parent node pointer (used in case of linked structures)
    std::set<annotation*> children; //children nodes pointers (used in case of linked structures)
    void* container;                //address of the container object
    int vaa3d_n;                    //Vaa3D's swc/apo index
    bool smart_delete;              // = true by default, enables "smart" deletion (see decontructor code)

    // constructor and deconstructor
    annotation() throw (itm::RuntimeException);
    ~annotation();

    inline bool operator==(const annotation& r) const{
        return  x       == r.x &&
                y       == r.y &&
                z       == r.z &&
                type    == r.type &&
                subtype == r.subtype;
    }

    void ricInsertIntoTree(annotation* node, QList<NeuronSWC> &tree);
    void insertIntoTree(QList<NeuronSWC> &tree);

    static long long last_ID;       //last ID assigned
    static itm::uint64 instantiated;//statistic "instantiated": counts the number of instantiated objects
    static itm::uint64 destroyed;   //statistic "destroyed"   : counts the number of destroyed objects
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
                    itm::uint32 V_start, V_dim;
                    itm::uint32 H_start, H_dim;
                    itm::uint32 D_start, D_dim;

                    //number of neurons in the octant
                    itm::uint32 n_annotations;

                    //annotations stored in the octant (only in a leaf)
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

                    //octree container handle
                    Octree* container;

                    octant(itm::uint32 _V_start, itm::uint32 _V_dim, itm::uint32 _H_start, itm::uint32 _H_dim, itm::uint32 _D_start, itm::uint32 _D_dim, Octree* _container)
                    {
                        child1  = child2 = child3 = child4 = child5 = child6 = child7 = child8 = 0;
                        V_start = _V_start;
                        V_dim   = _V_dim;
                        H_start = _H_start;
                        H_dim   = _H_dim;
                        D_start = _D_start;
                        D_dim   = _D_dim;
                        n_annotations = 0;
                        container = _container;
                    }
                };
                typedef octant* Poctant;

                itm::uint32 DIM_V, DIM_H, DIM_D;		//volume dimensions (in voxels) along VHD axes
                octant *root;				//pointer to root octant
                Octree(void){}				//default constructor is not available

                /*** SUPPORT methods ***/

                //recursive support methods
                void        _rec_clear(const Poctant& p_octant) throw(itm::RuntimeException);
                void        _rec_insert(const Poctant& p_octant, annotation& neuron) throw(itm::RuntimeException);
                void        _rec_remove(const Poctant& p_octant, annotation* neuron) throw(itm::RuntimeException);
                itm::uint32 _rec_deep_count(const Poctant& p_octant) throw(itm::RuntimeException);
                itm::uint32 _rec_height(const Poctant& p_octant) throw(itm::RuntimeException);
                void        _rec_print(const Poctant& p_octant);
                void        _rec_search(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int, std::list<annotation*>& neurons) throw(itm::RuntimeException);
                Poctant     _rec_find(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int) throw(itm::RuntimeException);
                itm::uint32 _rec_count(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int) throw(itm::RuntimeException);
                void        _rec_prune(const Poctant& p_octant) throw(itm::RuntimeException);

                //returns true if two given volumes intersect each other
                bool inline intersects(const interval_t& V1_int,const interval_t& H1_int,const interval_t& D1_int,
                                       itm::uint32& V2_start, itm::uint32& V2_dim, itm::uint32& H2_start, itm::uint32& H2_dim, itm::uint32& D2_start, itm::uint32& D2_dim)  throw(itm::RuntimeException);

                //returns true if first volume contains second volume
                bool inline contains  (const interval_t& V1_int, const interval_t& H1_int, const interval_t& D1_int,
                                       itm::uint32& V2_start, itm::uint32& V2_dim, itm::uint32& H2_start, itm::uint32& H2_dim, itm::uint32& D2_start, itm::uint32& D2_dim)  throw(itm::RuntimeException);

            public:

                //CONSTRUCTOR and DECONSTRUCTOR
                Octree(itm::uint32 _DIM_V, itm::uint32 _DIM_H, itm::uint32 _DIM_D);
                ~Octree(void);

                //clears octree content and deallocates used memory
                void clear() throw(itm::RuntimeException);

                //insert given neuron in the octree
                void insert(annotation& neuron) throw(itm::RuntimeException);

                //remove given neuron from the octree (returns 1 if succeeds)
                bool remove(annotation* neuron) throw(itm::RuntimeException);

                //search for neurons in the given 3D volume and puts found neurons into 'neurons'
                void find(interval_t V_int, interval_t H_int, interval_t D_int, std::list<annotation*>& neurons) throw(itm::RuntimeException);

                //search for the annotations at the given coordinate. If found, returns the address of the annotations list
                std::list<annotation*>* find(float x, float y, float z) throw(itm::RuntimeException);

                //returns the number of neurons (=leafs) in the given volume without exploring the entire data structure
                itm::uint32 count(interval_t V_int = interval_t(-1,-1), interval_t H_int = interval_t(-1,-1), interval_t D_int = interval_t(-1,-1))  throw(itm::RuntimeException);

                //returns the number of neurons (=leafs) in the octree by exploring the entire data structure
                itm::uint32 deep_count()  throw(itm::RuntimeException);

                //returns the octree height
                itm::uint32 height()  throw(itm::RuntimeException);

                //prunes the octree by removing all nodes duplicates while maintaining the same branched structure
                void prune() throw(itm::RuntimeException);

                //print the octree content
                void print();

                friend class CAnnotations;
                friend class annotation;

                static inline double round(double val){
                    return floor(val + 0.5);
                }
                static inline double round(float val){
                    return floor(val + 0.5f);
                }
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
        CAnnotations(itm::uint32 volHeight, itm::uint32 volWidth, itm::uint32 volDepth) : octreeDimX(volWidth), octreeDimY(volHeight), octreeDimZ(volDepth)
        {
            /**/itm::debug(itm::LEV1, strprintf("volHeight = %d, volWidth = %d, volDepth = %d", volHeight, volWidth, volDepth).c_str(), __itm__current__function__);

            octree = new Octree(octreeDimY, octreeDimX, octreeDimZ);

            /**/itm::debug(itm::LEV1, "object successfully constructed", __itm__current__function__);
        }

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CAnnotations* instance(itm::uint32 volHeight, itm::uint32 volWidth, itm::uint32 volDepth)
        {
            if (uniqueInstance == 0)
                uniqueInstance = new CAnnotations(volHeight, volWidth, volDepth);
            return uniqueInstance;
        }
        static CAnnotations* getInstance() throw (itm::RuntimeException)
        {
            if(uniqueInstance)
                return uniqueInstance;
            else
                throw itm::RuntimeException("in CAnnotations::getInstance(): no object has been instantiated yet");
        }
        static void uninstance();
        ~CAnnotations();

        /*********************************************************************************
        * Find/Add/Clear annotation(s)
        **********************************************************************************/
        void findLandmarks (itm::interval_t X_range, itm::interval_t Y_range, itm::interval_t Z_range, QList<LocationSimple> &markers) throw (itm::RuntimeException);
        void addLandmarks  (itm::interval_t X_range, itm::interval_t Y_range, itm::interval_t Z_range, LandmarkList& markers) throw (itm::RuntimeException);
        void clearLandmarks(itm::interval_t X_range, itm::interval_t Y_range, itm::interval_t Z_range) throw (itm::RuntimeException);

        void findCurves (itm::interval_t X_range, itm::interval_t Y_range, itm::interval_t Z_range, QList<NeuronSWC> &curves) throw (itm::RuntimeException);
        void addCurves  (itm::interval_t X_range, itm::interval_t Y_range, itm::interval_t Z_range, NeuronTree& nt) throw (itm::RuntimeException);
        void clearCurves(itm::interval_t X_range, itm::interval_t Y_range, itm::interval_t Z_range) throw (itm::RuntimeException);

        // @obsolete void removeCurves(std::list<NeuronSWC> &curves) throw (RuntimeException);
        // @obsolote void removeLandmarks(std::list<LocationSimple> &markers) throw (RuntimeException);

        /*********************************************************************************
        * Save/load method
        **********************************************************************************/
        void save(const char* filepath) throw (itm::RuntimeException);
        void load(const char* filepath) throw (itm::RuntimeException);

        /*********************************************************************************
        * Removes all the annotations from the octree
        **********************************************************************************/
        void clear()  throw (itm::RuntimeException)
        {
            /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

            delete octree;
            octree = new Octree(octreeDimY, octreeDimX, octreeDimZ);
        }

        /*********************************************************************************
        * Checks whether the octree is empty
        **********************************************************************************/
        bool isEmpty()
        {
            if(octree)
                return octree->deep_count() == 0;
            else
                return true;
        }

        /*********************************************************************************
        * Counts the number of stored annotations
        **********************************************************************************/
        itm::uint32 count()
        {
            if(octree)
                return octree->count();
            else
                return 0;
        }

        /*********************************************************************************
        * Prunes the octree by removing duplicate nodes w/o altering the branching structure
        **********************************************************************************/
        void prune() throw (itm::RuntimeException)
        {
            if(octree)
                octree->prune();
        }

        /*********************************************************************************
        * Conversion from VTK to APO files
        **********************************************************************************/
        static void convertVtk2APO(std::string vtkPath, std::string apoPath) throw (itm::RuntimeException);

        /*********************************************************************************
        * Diff between two APO files
        **********************************************************************************/
        static void diffAPO(std::string apo1Path,   // first apo file path
                            std::string apo2Path,   // second apo file path
                            int x0=0, int x1=-1,    // VOI [x0, x1) in the global reference sys
                            int y0=0, int y1=-1,    // VOI [y0, y1) in the global reference sys
                            int z0=0, int z1=-1,    // VOI [z0, z1) in the global reference sys
                            std::string diffPath="")// path where the difference apo file (containing only FPs and FNs) has to be stored (optional)
        throw (itm::RuntimeException);


        static inline bool isMarkerOutOfRendererBounds(const LocationSimple& marker){
            return marker.x < 0 || marker.y < 0 || marker.z < 0 ;
        }

        friend class annotation;
};

#endif // CANNOTATIONS_H
