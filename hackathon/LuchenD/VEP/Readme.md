# VEP

基于Vaa3D-x的可视化编程模块

## 1. 依赖与安装

Pyside6

```python
pip install pyside6
```

## 2. 项目文件组织

- src: Py交互界面源码
  - editor: 所有交互组件构造与功能
    - nodes: 用于存放自定义Node节点
    - qss: 存放样式表
    - structs: 存放处理py层级数据结构
    - widgets: 存在qt控件级别
    - tools: 内嵌项目工具
    - main.py：项目运行入口
    - vep_xxx: nodes, ports，edge，关注三个交互组件即可
  - model: json处理
  - viewModel: 建的玩

## 3. 结构简介

1. view: 试图
2. control：流的控制
3. model：对传统模型的封装

## View

editor 编辑器

期内有个载体，graph图，用于存放可视化编程的界面

内有node节点，用于承载特定的功能 

对于每一个Node来说，名为GraphNode，input,output, exec作具体功能执行处，NodePort, NodeTitle, Position, NodeEdge



## 技术选型

使用qt而非electron，两者均为桌面应用开发框架，qt为c++,electron多为node-js。

- qt拥有Qt for python
- 可封装python，对于算法使用更方便
- 美观程度，qt可使用QML or QSS进行样式调节，对应在html的CSS，可操作空间较大

## 4. 项目设置

> Target: 实现每个节点的可视化

PySide6：qt6.4对python的GUI图形库

## 功能

- [x] 绘制网格，背景颜色设置
- [x] 滚动鼠标滚轮为放大缩小，而不是上下滚动
- [x] 使用鼠标中键完成画布拖动
- [ ] 创建圆角矩形

鼠标中键画布拖动时，在middleButtonPressed中创建虚拟事件完成按下中键即可拖动的操作
