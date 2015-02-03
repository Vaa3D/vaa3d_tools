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



    class ConverterLoadBlockOperation : public Operation
    {
        public:

            static int gid;
            virtual std::string name(){return "OP_CONV_LOAD_BLOCK";}
            static int newGroup(){return gid++;}
            ConverterLoadBlockOperation(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
    };

    class ConverterWriteBlockOperation : public Operation
    {
        public:

            static int gid;
            virtual std::string name(){return "OP_CONV_WRITE_BLOCK";}
            static int newGroup(){return gid++;}
            ConverterWriteBlockOperation(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
    };

    class TiffLoadMetadata : public Operation
    {
        public:

            static int gid;
            virtual std::string name(){return "OP_TIFF_LOAD_MDATA";}
            static int newGroup(){return gid++;}
            TiffLoadMetadata(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
    };

    class TiffLoadData : public Operation
    {
        public:

            static int gid;
            virtual std::string name(){return "OP_TIFF_LOAD_DATA";}
            static int newGroup(){return gid++;}
            TiffLoadData(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
    };

    class TiffInitData : public Operation
    {
        public:

            static int gid;
            virtual std::string name(){return "OP_TIFF_INIT_DATA";}
            static int newGroup(){return gid++;}
            TiffInitData(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
    };

    class TiffAppendData : public Operation
    {
        public:

            static int gid;
            virtual std::string name(){return "OP_TIFF_APPEND_DATA";}
            static int newGroup(){return gid++;}
            TiffAppendData(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
    };
}

//#define TERAFLY_TIME_START(classname)   \
//QElapsedTimer timer##classname;         \
//timer##classname.start();               \
//teramanager::classname::newGroup();

//#define TERAFLY_TIME_RESTART(classname) \
//timer##classname.restart();

//#define TERAFLY_TIME_STOP(classname, component, message)    \
//teramanager::PLog::getInstance()->emitSendAppend(new teramanager::classname(message, component, timer##classname.elapsed()));

#define TERAFLY_TIME_START(classname)
#define TERAFLY_TIME_RESTART(classname)
#define TERAFLY_TIME_STOP(classname, component, message)

#endif // COPERATION_H
