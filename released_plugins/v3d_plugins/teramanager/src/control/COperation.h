#ifndef COPERATION_H
#define COPERATION_H

#include "CPlugin.h"

namespace teramanager
{
    enum COMPONENT{ALL_COMPS, GPU, CPU, IO};        // system-component types
    static std::string comp2str(COMPONENT c){
        if(c == ALL_COMPS)
            return "MIX";
        else if(c == GPU)
            return "GPU";
        else if(c == CPU)
            return "CPU";
        else if(c == IO)
            return "IO";
        else
            return "unknown";
    }
    static COMPONENT str2comp(std::string s){
        for(int k=0; k<4; k++)
            if(comp2str(COMPONENT(k)).compare(s)==0)
                return COMPONENT(k);
    }

    class Operation
    {
         public:

            int milliseconds;
            COMPONENT comp;
            std::string message;
            int groupID;

            virtual std::string name() = 0;

            Operation(std::string m, COMPONENT c, int ms, int gid) : message(m), comp(c), milliseconds(ms), groupID(gid){}
            Operation(){}
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
        private:
            ImportOperation(){}

        public:

            static int gid;
            virtual std::string name(){return "OP_VOL_IMPORT";}
            static int newGroup(){return gid++;}
            ImportOperation(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
            friend class PLog;
    };

    class NewViewerOperation : public Operation
    {
        private:
            NewViewerOperation(){}

        public:

            static int gid;
            virtual std::string name(){return "OP_NEW_VIEWER";}
            static int newGroup(){return gid++;}
            NewViewerOperation(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
            friend class PLog;
    };

    class RestoreViewerOperation : public Operation
    {
        private:
            RestoreViewerOperation(){}

        public:

            static int gid;
            virtual std::string name(){return "OP_GET_VIEWER";}
            static int newGroup(){return gid++;}
            RestoreViewerOperation(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
            friend class PLog;
    };

    class ZoominRoiOperation : public Operation
    {
        private:
            ZoominRoiOperation(){}

        public:

            static int gid;
            virtual std::string name(){return "OP_ZOOMIN_ROI";}
            static int newGroup(){return gid++;}
            ZoominRoiOperation(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
            friend class PLog;
    };

    class AnnotationOperation : public Operation
    {
        private:
            AnnotationOperation(){}

        public:

            static int gid;
            virtual std::string name(){return "OP_ANNOTATION";}
            static int newGroup(){return gid++;}
            AnnotationOperation(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
            friend class PLog;
    };



    class ConverterLoadBlockOperation : public Operation
    {
        private:
            ConverterLoadBlockOperation(){}

        public:

            static int gid;
            virtual std::string name(){return "OP_CONV_LOAD_BLOCK";}
            static int newGroup(){return gid++;}
            ConverterLoadBlockOperation(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
            friend class PLog;
    };

    class ConverterWriteBlockOperation : public Operation
    {
        private:
            ConverterWriteBlockOperation(){}

        public:

            static int gid;
            virtual std::string name(){return "OP_CONV_WRITE_BLOCK";}
            static int newGroup(){return gid++;}
            ConverterWriteBlockOperation(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
            friend class PLog;
    };

    class TiffLoadMetadata : public Operation
    {
        private:
            TiffLoadMetadata(){}

        public:

            static int gid;
            virtual std::string name(){return "OP_TIFF_LOAD_MDATA";}
            static int newGroup(){return gid++;}
            TiffLoadMetadata(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
            friend class PLog;
    };

    class TiffLoadData : public Operation
    {
        private:
            TiffLoadData(){}

        public:

            static int gid;
            virtual std::string name(){return "OP_TIFF_LOAD_DATA";}
            static int newGroup(){return gid++;}
            TiffLoadData(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
            friend class PLog;
    };

    class TiffInitData : public Operation
    {
        private:
            TiffInitData(){}

        public:

            static int gid;
            virtual std::string name(){return "OP_TIFF_INIT_DATA";}
            static int newGroup(){return gid++;}
            TiffInitData(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
            friend class PLog;
    };

    class TiffAppendData : public Operation
    {
        private:
            TiffAppendData(){}

        public:

            static int gid;
            virtual std::string name(){return "OP_TIFF_APPEND_DATA";}
            static int newGroup(){return gid++;}
            TiffAppendData(std::string m, COMPONENT c, int ms) : Operation(m, c, ms, gid){}
            friend class PLog;
    };
}

#define TERAFLY_TIME_START(classname)   \
QElapsedTimer timer##classname;         \
timer##classname.start();               \
teramanager::classname::newGroup();

#define TERAFLY_TIME_RESTART(classname) \
timer##classname.restart();

#define TERAFLY_TIME_STOP(classname, component, message)    \
if(teramanager::PLog::instance()->isIoCoreOperationsEnabled()) \
    teramanager::PLog::instance()->emitSendAppend(new teramanager::classname(message, component, timer##classname.elapsed()));

//#define TERAFLY_TIME_START(classname)
//#define TERAFLY_TIME_RESTART(classname)
//#define TERAFLY_TIME_STOP(classname, component, message)

#endif // COPERATION_H
