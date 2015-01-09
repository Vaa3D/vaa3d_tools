#ifndef COPERATION_H
#define COPERATION_H

#include "CPlugin.h"

namespace teramanager
{
    enum COMPONENT{ALL_COMPS, GPU, CPU, IO};        // system-component types

    class Operation
    {
         public:

            int milliseconds;
            COMPONENT comp;
            std::string message;
            int groupID;

            virtual std::string name() = 0;

            Operation(std::string m, COMPONENT c, int ms, int gid) : message(m), comp(c), milliseconds(ms), groupID(gid){}
            std::string compName()
            {
                if(comp == ALL_COMPS)
                    return "ACT";
                else if(comp == GPU)
                    return "GPU";
                else if(comp == CPU)
                    return "CPU";
                else if(comp == IO)
                    return "I/O";
                else
                    return "UNKNOWN";
            }
    };

    class ImportOperation : public Operation
    {
        public:

            static int gid;
            virtual std::string name(){return "OP_VOL_IMPORT";}
            static int newGroup(){return gid++;}
            ImportOperation(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
    };

    class NewViewerOperation : public Operation
    {
        public:

            static int gid;
            virtual std::string name(){return "OP_NEW_VIEWER";}
            static int newGroup(){return gid++;}
            NewViewerOperation(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
    };

    class RestoreViewerOperation : public Operation
    {
        public:

            static int gid;
            virtual std::string name(){return "OP_GET_VIEWER";}
            static int newGroup(){return gid++;}
            RestoreViewerOperation(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
    };

    class ZoominRoiOperation : public Operation
    {
        public:

            static int gid;
            virtual std::string name(){return "OP_ZOOMIN_ROI";}
            static int newGroup(){return gid++;}
            ZoominRoiOperation(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
    };

    class AnnotationOperation : public Operation
    {
        public:

            static int gid;
            virtual std::string name(){return "OP_ANNOTATION";}
            static int newGroup(){return gid++;}
            AnnotationOperation(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
    };
}

#endif // COPERATION_H
