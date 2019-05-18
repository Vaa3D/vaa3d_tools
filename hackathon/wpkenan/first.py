import tensorflow as tf
import matplotlib.pyplot as plt
import cv2
import numpy
import pandas as pd

print("Start")
filePath="C:\\Users\\admin\\Desktop\\wp_data\\041_x_9225_y_9875_z_2493_gray.txt"
file=None;
if(filePath!=""):
    file=open(filePath);
content=file.readlines();
file.close();

for i in range(len(content)):
    content[i]=int(content[i].strip('\n'));
print(type(content))

#统计频率并做图
result=pd.value_counts(content);
print(result.sort_index())
outPath=filePath.strip('.txt')+"_frequency.txt";
outFile=open(outPath,'w');
outFile.write(str(result.sort_index()))
outFile.close();

x=result.index;
y=result.values;
plt.scatter(x,y);

plt.show()
