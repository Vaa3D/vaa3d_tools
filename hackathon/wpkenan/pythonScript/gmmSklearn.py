from sklearn import mixture
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import sys

def main():
    # rawStr = "C:\\Users\\admin\\Desktop\\wp_data\\x_3136_y_1239_z_2218_gray.txt"
    rawStr = "C:\\Users\\admin\\Desktop\\wp_data\\041_x_9225_y_9875_z_2493_gray.txt"
    # rawStr = "C:\\Users\\admin\\Desktop\\wp_data\\066_x_9183_y_8016_z_2321_gray.txt"
    if len(sys.argv)==2:
        rawStr=sys.argv[1];
    elif len(sys.argv)>2:
        return False;

    strFile=rawStr.split('.')[0];
    print(strFile)

    filePath=strFile+".txt";
    file=open(filePath);
    content=file.readlines();
    file.close();
    for i in range(len(content)):
        content[i]=float(content[i].strip('\n'));

    # print(content)
    print(type(content))
    print(type(content[0]))

    content=np.array(content);
    content.shape=len(content),1
    gmm=mixture.GaussianMixture(n_components=3).fit(content)
    print("均值:")
    print(gmm.means_)
    # print("方差: ")
    # print(gmm.covariances_)

    # 获取预测值
    index=[]
    dict={};
    for i in range(len(gmm.means_)):
        index.append([i,float(gmm.means_[i])]);
    index.sort(key=lambda x:x[1]);

    for i in range(len(gmm.means_)):
        dict[index[i][0]]=i;

    for i in range(len(index)):
        print("%d %f"%(i,index[i][1]));
    y = gmm.predict(content)
    for i in range(len(y)):
        y[i]=dict[y[i]];

    outPath=strFile+"_gmm.txt";
    outFile=open(outPath,"w");
    for i in y:
        outFile.write(str(i)+"\n");
    outFile.close()

    pdy=pd.value_counts(y);
    print(pdy.sort_index())

if __name__=="__main__":
    main();


