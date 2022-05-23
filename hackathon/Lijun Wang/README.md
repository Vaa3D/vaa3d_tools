V1.0版本当前只支持windows端，使用V1.0版本插件请按照以下步骤配置环境：
1. 按照https://github.com/Vaa3D/Vaa3D_Wiki/wiki/Build-Vaa3D-on-Windows-(QMake) 网页所示方法配置好vaa3d环境（若已有vaa3d环境则忽略此条）。
2. 安装配置python3，并在Pro文件和cpp文件中修改相应地址。
3. 下载最新版本的imagej（Fiji），将bin文件夹中的ConvertTo8bit.ijm放到../imagej/Fiji.app下。然后将这个imagej文件夹拷贝到../v3d_external/bin目录下。
4. 将bin目录下的split_swc.py文件拷贝到../v3d_external/bin目录下。
5. 编译构建本插件，注意选择Release选项进行构件。
6. 打开vaa3d就可以使用名为xjy的插件了，插件目前有两个菜单项，refine选项的流程为“用户选择数据存放地址、需要拆分的swc的地址以及拆分后swc的存放地址-程序拆分swc-对swc进行refine-拼接swc得到refine结果”，refine without stitching的流程为“用户选择数据存放地址、每块数据对应的swc的地址-对swc进行refine-生成每块数据对应的refine后的swc”。
