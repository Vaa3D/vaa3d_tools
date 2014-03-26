import json;
import os;
import numpy as np;

def CreateDataBundle(config):
#     dataPath = config["dataPath"]; #'/Users/zhaot/Work/neutube/neurolabi/data';
    sessionPath = config["sessionPath"]; #dataPath + '/flyem/FIB/skeletonization/session11';
    if config.has_key("swcDir"):
        swcPath = config['swcDir']; #sessionPath + '/' + config['swcDir'];
    else:
        swcPath = sessionPath + '/swc/adjusted';
    
    swcFileList = list();
    if config.has_key("minBodySize") | config.has_key("maxBodySize"):
        data = np.loadtxt(sessionPath + '/bodies/bodysize.txt', delimiter=',');
        bodyList = data[:, 0];
        bodySize = data[:, 1];
        lowerThreshold = -1;
        upperThreshold = -1;
        if config.has_key("minBodySize"):
            lowerThreshold = config["minBodySize"];
        if config.has_key("maxBodySize"):
            upperThreshold = config["maxBodySize"];

        print lowerThreshold;
        print upperThreshold;
        for i in range(0, len(bodyList)):
#             print bodySize[i];
#             print (bodySize[i] >= lowerThreshold) | (lowerThreshold < 0);
#             print (bodySize[i] <= upperThreshold) | (upperThreshold < 0);
#             print ((bodySize[i] >= lowerThreshold) | (lowerThreshold < 0)) & ((bodySize[i] <= upperThreshold) | (upperThreshold < 0));
            if ((bodySize[i] >= lowerThreshold) | (lowerThreshold < 0)) & ((bodySize[i] <= upperThreshold) | (upperThreshold < 0)):
                print str(int(bodyList[i]));
                swcFileList.append(str(int(bodyList[i])) + '.swc');
    else:
        swcFileList = os.listdir(swcPath);
        
    print len(swcFileList), " neurons";   

    dataBundle = {"neuron": list()};

    neuronClass = {};

    #Add predicted
    if (config["addingClass"]):
#         predictPath = sessionPath + "/predict.txt";
        predictPath = config["predict"];
        if predictPath.endswith(".json"):
            f = open(predictPath);
            classBundle = json.load(f);
            f.close();
            for neuron in classBundle["neuron"]:
                for bodyId in neuron["id"]:
                    neuronClass[bodyId] = neuron["class"];
        else:
            f = open(predictPath);
            lines = f.readlines();
            f.close();
            for line in lines:
                line.strip();
                line = line.replace("_", " ");
                line = line.replace("(", " ");
                line = line.replace(")", " ");
                line = line.replace(":", " ");
                tokens = line.split();
                #print(tokens)
                if len(tokens) > 1:
                    print(int(tokens[1]), tokens[3]);
                    neuronClass[int(tokens[1])] = tokens[3];
                    if tokens[3] == "unknown":
                        neuronClass[int(tokens[1])] = tokens[3] + " " + tokens[4];

    for f in swcFileList:
        if f.endswith('.swc'):
#             print f;
            bodyId = f.split('.')[0];
            neuron = {"id": int(bodyId), "name": "FIB_" + bodyId, 
                      "model": swcPath + "/" + f};
            if (config["addingClass"]):
                neuron["class"] = neuronClass[int(bodyId)];
            if config.has_key("objDir"):
                neuron["volume"] = (config["objDir"] + "/" + os.path.basename(f)).replace('.swc', '.obj');
            dataBundle["neuron"].append(neuron);
            
    otherFields = ["image_resolution", "synapse_scale", "synapse", "source_offset", "source_dimension"];
    for field in otherFields:
        if config.has_key(field):
            dataBundle[field] = config[field];

    # dataBundlePath = sessionPath + "/data_bundle_with_class.json";
    dataBundlePath = config["output"]; #sessionPath + "/data_bundle.json";
    outFile = open(dataBundlePath, "w");
    json.dump(dataBundle, outFile, indent = 2);
    outFile.close();
     
    print dataBundlePath + " saved";
    
if __name__ == '__main__':
    dataPath = '/Users/zhaot/Work/neutube/neurolabi/data';
    config = { "sessionPath": dataPath + '/flyem/FIB/skeletonization/session11', "addingClass": False};
    config["output"] = config["sessionPath"] + "/data_bundle.json";
    CreateDataBundle(config);