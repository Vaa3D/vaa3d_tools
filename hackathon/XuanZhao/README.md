## neuronQC
### 简要介绍
用于检测半自动重建神经元的一些硬性指标错误
### 调用方法
```
vaa3d_path /x dll_path /f neuronQC_batch /i swc_dir csv_path /p short_length_thres node_length_thres loop_three_bifircation
```
vaa3d_path: vaa3d应用路径<br>
dll_path: dll路径（也可改为插件名neuronQC）<br>
swc_dir: 存有swc的文件，swc可有多个，该插件函数为批量处理<br>
csv_path: csv文件路径，用于记录结果<br>
short_length_thres：短分支的长度阈值设置参数，默认为10<br>
node_length_thres：swc两个node之间的长度阈值设置参数，默认为15<br>
loop_three_bifircation：是否检测loop和三分叉，默认检测<br>

## imageProcess
### 简要介绍
一些可能会有用的一些辅助函数
### 各函数调用方法
#### enhanceImage
对图像做下面的简单处理：
```
f(i) = ((i/255)^2/3)*255
```
调用
```
vaa3d_path /x dll_path /f enhanceImage /i img_path
```
vaa3d_path: vaa3d应用路径<br>
dll_path: dll路径（也可改为插件名,可在.pro文件查看,类似为$$qtLibrary(name),其中name为插件名）<br>
img_path: 图像路径<br>
#### get_2d_image2
将图像和对应swc叠加再一起做一个mip<br>

调用
```
vaa3d_path /x dll_path /f get_2d_image2 /i swc_path1 swc_path2 ... /p img_path
```
vaa3d_path: vaa3d应用路径<br>
dll_path: dll路径<br>
swc_pathi: swc路径,可多个<br>
img_path：图像路径<br>
#### getSWCL0image
获取swc对应的图像，图像太大会获取不了<br>
调用
```
vaa3d_path /x dll_path /f getSWCL0image /i swc_path /p brain_path times /o out_dir 
```
vaa3d_path: vaa3d应用路径<br>
dll_path: dll路径<br>
swc_path: swc路径<br>
brain_path：terafly格式全脑路径<br>
times：哪一层分辨率（1，2，4，8...之类的数）<br>
out_dir: 保存文件夹路径<br>
#### convertTeraflyDataTov3draw
将terafly格式数据的某一层分辨率拼接成一个图<br>
调用
```
vaa3d_path /x dll_path /f convertTeraflyDataTov3draw /i brain_path /p resolution /o out_path 
```
vaa3d_path: vaa3d应用路径<br>
dll_path: dll路径<br>
brain_path：terafly格式全脑某一层分辨率的路径<br>
resolution：哪一层分辨率（1，2，4，8...之类的数,1为最高分辨率，2为次高...）<br>
out_path: 保存图像的路径<br>
#### bilateralfilter
对图像做一个双边滤波处理<br>
调用
```
vaa3d_path /x dll_path /f bilateralfilter /i img_path /p is_normal space_sigma_xy space_sigma_z color_sigma 
```
vaa3d_path: vaa3d应用路径<br>
dll_path: dll路径<br>
img_path：图像路径<br>
is_normal：是否先将图像转换到0-255，默认是<br>
space_sigma_xy: 核xy方向的大小,默认为2<br>
space_sigma_z: 核xy方向的大小，默认为1<br>
color_sigma: color sigma大小，默认为35<br>
#### changeContrast
对图像做一个线性变换<br>
调用
```
vaa3d_path /x dll_path /f changeContrast /i img_path /p pencert_down pencert_up
```
vaa3d_path: vaa3d应用路径<br>
dll_path: dll路径<br>
img_path：图像路径<br>
pencert_down：转换为0的起点，百分数<br>
pencert_up: 转换为255的起点，百分数<br>
#### v3draw2tif
将v3draw格式的图像批量转为tif格式<br>
调用
```
vaa3d_path /x dll_path /f v3draw2tif /i img_dir /o out_dir
```
vaa3d_path: vaa3d应用路径<br>
dll_path: dll路径<br>
img_dir：要转换图像的文件夹<br>
out_dir：输出文件夹<br>

## dynamicApp2
### 简要介绍
基于app2的一些改动尝试
### 各函数调用方法
#### dynamicApp2
在一个图像块内进行追踪，对每次追踪的长度加上一个限制，并对它进行反向追踪进行验证，然后再继续追踪，直到追踪到tip点<br>
调用
```
vaa3d_path /x dll_path /f dynamicApp2 /i img_path /p ... length_thres ...
```
vaa3d_path: vaa3d应用路径<br>
dll_path: dll路径<br>
img_path: 图像路径<br>
length_thres: 长度限制参数<br>
其他参数与app2一致<br>
#### ultratracerAxonTerafly
对terafly数据格式进行轴突追踪<br>
调用
```
vaa3d_path /x dll_path /f ultratracerAxonTerafly /i swc_path brain_path tmp_image_dir
```
vaa3d_path: vaa3d应用路径<br>
dll_path: dll路径<br>
swc_path: 初始swc路径，初始swc为一条轴突的信号，该函数会在该信号的基础上继续往下追踪<br>
brain_path: terafly全脑路径<br>
tmp_image_dir：中间结果，不输入则不存<br>

## consensus
### 简要介绍
consensus的一些尝试
### 各函数调用方法
#### consensus
在一个图像块内改变条件用各种方式进行追踪，将追踪的图像进行融合，然后在融合的图像进行追踪，最后得到追踪结果
```
vaa3d_path /x dll_path /f consensus /i img_path marker_path /p kmeans_th
```
vaa3d_path: vaa3d应用路径<br>
dll_path: dll路径<br>
img_path: 图像路径<br>
marker_path: soma marker路径<br>
kmeans_th: 是否用kmeans的方法来得到一个阈值，默认不用<br>

## app2WithPreinfo
### 简要介绍
给app2一些预信息的一些尝试
### 各函数调用方法
#### app2WithPreinfo
画一条前景，画一条背景给app2提供一些阈值上的信息
```
vaa3d_path /x dll_path /f app2WithPreinfo /i input_dir brain_path /o out_dir /p ratio th resolution_times image_flag lower upper is_multi app2_length contrast_th contrast_ratio
```
vaa3d_path: vaa3d应用路径<br>
dll_path: dll路径<br>
input_dir: 输入的目录，该目录存放一个swc文件和一个apo文件，swc文件画有一条背景和一条前景，apo为soma位置<br>
brain_path: terafly数据格式全脑路径<br>
out_dir: 输出目录<br>
ratio: ratio用于阈值计算，th = fmean*ratio + bmean*（1-ratio），fmean为前景，bmean为背景<br>
th: 如果ratio为0则以bstd/（bstd+fstd）自动计算ratio，若ratio为-4则以bmean/（bmean+fmean）自动计算ratio，ratio若为-1则用app2本身计算阈值的方式，ratio若为-2则自己设置th，ratio为-3，阈值则为bmean+th<br>
resolution_times：在哪个分辨率的图上跑，1为最高，2为次高，4为...<br>
image_flag: 对图像做预处理的不同方法<br>
lower：当image_flag为2或3时，用于三段映射变换，当为3时，第一段(0-bmean)和第二段（bmean-fmean）都做线性变换，当为2时，只有第二段做线性变换<br>
upper：lower对应bmean，upper对应fmean<br>
is_multi：是否用multi app2<br>
app2_length：app2剪枝的长度阈值<br>
contrast_th：对比度阈值<br>
contrast_ratio：对比度ratio<br>

## Retrace
详见Retace文件夹里的README

## SNAP
详见SNAP文件夹里的README

## TypeLength
### 简要介绍
统计swc各type的长度以及比例信息
### 调用方法
菜单调用

## judgeBranch
### 简要介绍
统计一些分支的特征使用随机森林来训练和判断swc各分支的正误
### 调用方法
菜单调用