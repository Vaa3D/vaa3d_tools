# SNAP使用教程
 SNAP是一个包含整个剪枝流程的Vaa3D插件，它可以针对一个single-neuron tree进行剪枝，也可以指定多个soma将它进行拆分。

# 目录
1.[安装](#安装)

2.[通过菜单调用](#通过菜单调用)

3.[通过命令行调用](#通过命令行调用)

4.[License](#License)

# <a name="安装"></a>安装
开发者：

 1. Please follow the build instructions here to build released plugins along with the Vaa3d main program:
         https://github.com/Vaa3D/Vaa3D_Wiki/wiki/BuildVaa3d.wiki

 2. Build this Vaa3D plugin following this wiki page: https://github.com/Vaa3D/Vaa3D_Wiki/wiki/CompilePlugins.wiki 

用户：

1.下载Vaa3d软件

2.下载SNAP插件，将该文件夹复制到vaa3d主程序的plugin目录里面，再次打开Vaa3d时就能看到SNAP插件

# <a name="通过菜单调用"></a>通过菜单调用

1.首先我们打开vaa3d的主菜单，点击plugin，找到SNAP插件并点击，如下图所示
![SNAP](null)

2.pruning功能是针对一个single tree进行剪枝，首先打开一张图，并将.swc/.eswc文件拖到图里，然后点击pruning会出现如下对话框：
![SNAP Pruning](null)
以下是对所有参数的解释：
参数                
| 参数    |   含义  |  默认值   |
| --- | --- | --- |
|  length threshold   | 长度阈值，用于第一步的剪枝   |  11   |
|  linearity threshold   | 线性阈值，用于第一步的剪枝    | 1.4    |
|  angle threshold   |  角度阈值，表示分叉点的夹角，用于第三步的剪枝   |  160   |
|  lamda   | 第二步中将角度拟合到泊松分布的参数    |  2   |
|  max length   | 正常tip点与soma点的路径长度的最大值    |   850  |
|  soma area   |  第一步中在soma附近区域的剪枝，该数值表示soma多少倍半径区域的范围   |  5  |
|   noisyPruning  | 是否进行噪声剪枝    |  否   |
|   multiSomaPruning  |  是否为多个soma的情况，若是，请指将其他soma的位置用marker标出   |   否  |
| structurePruning    |  是否进行结构剪枝  |  否   |
| inflectionPruning  | 是否对不正常的拐点进行剪枝  | 否  | 
其中noisyPruning，inflectionPruning需要图像参与，multiSomaPruning需要指定其他soma的位置，一般选择前三项Pruning，inflectionPruning速度较慢。

3.NeuronSplit是针对一个single tree进行拆分，将每个soma的位置用marker标记出来，然后点击NeuronSplit，等待一段时间，拆分出来的神经元将以不同颜色展示出来。

# <a name="通过命令行调用"></a>通过命令行调用

下面是一个命令行的通用例子

```
vaa3dPath /x dllpath /f pruning /i swcPath markerNIndexPath imgPath /p lengththreshold linearitythreshold anglethreshold lamda maxlength somaarea noisyPruning multiSomaPruning structurePruning inflectionPruning
```


其中/p参数与通过菜单调用的参数一致，/i中，swcpath表示swc或者eswc文件的路径，markerNIndexPath为一个txt文件，它记录各个soma在swc文件中的n值，或者是一个.marker文件的路径，imgPath表示图像的路径，具体可参考测试数据的格式。

命令行附有一个函数负责将.marker文件转换为各个soma在swc文件中的n值的txt文件，调用方式如下：

```
vaa3dPath /x dllpath /f convertMarkers2SomaNList /i swcPath markerPath /o markerNIndexPath
```
 







