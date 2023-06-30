# 1. 简介

python-node-editor

Ryven

Pyflows

这些工具大都与数据分析有关，相关性分析，可视化matplotlib，分类。scikit-learn...



# 结构

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

# 2. 项目设置

> Target: 实现每个节点的可视化

PySide6：qt6.4对python的GUI图形库

## 功能

- [x] 绘制网格，背景颜色设置
- [x] 滚动鼠标滚轮为放大缩小，而不是上下滚动
- [x] 使用鼠标中键完成画布拖动
- [ ] 创建圆角矩形

鼠标中键画布拖动时，在middleButtonPressed中创建虚拟事件完成按下中键即可拖动的操作



## 3. 开发日记

### 2023.2.22

完成VEP界面开发，样式表使用qt-material(https://github.com/UN-GCPDS/qt-material) dark-lightgreen.xml

#### todo

1. node节点设计，已有Branch意义不明，需要调研插件编写过程，抽象逻辑node
2. 中间层设计：python如何生成cpp dll文件？函数传递？output？
3. 架构：MVVM viewModel在哪？与2结合



### 2023.2.23

> Qt6的插件机制

Qt6是一个流行的跨平台C++应用程序开发框架，具有可扩展性和可定制性。Qt6提供了一个灵活的插件机制，使得开发人员可以编写可插拔的模块，并将它们集成到应用程序中。这些插件可以动态加载并在运行时被发现和使用。Qt6插件机制的核心是QPluginLoader类，它允许您加载共享库并实例化其中定义的类。

Qt6插件可以分为三种类型：

1. 插件库：这是最常见的类型，它是一个共享库（DLL或SO文件），包含一个或多个Qt插件。这种类型的插件可以在运行时动态加载。
2. 插件描述文件：这是一种XML文件，描述一个或多个Qt插件，包括名称、版本号、依赖项和其他元数据。这种类型的插件在编译时被处理，可以用于生成插件库。
3. 嵌入式插件：这是一种特殊类型的插件，它是在应用程序内部定义的，通常用于定制特定的功能。这种类型的插件可以与其他插件一起使用，并可以通过标准Qt插件接口进行访问。

在Qt6中，插件通过Qt插件接口进行定义和实现，该接口包括一个抽象基类QAbstractPlugin和两个派生类：QGenericPlugin和QFactoryInterface。其中，QGenericPlugin是一个通用的插件接口，它可以用于实现任何类型的插件，而QFactoryInterface则是一个工厂接口，它用于实现具有特定功能的工厂类。

要创建一个Qt6插件，需要完成以下步骤：

1. 定义一个实现QAbstractPlugin接口的类。
2. 在类中实现插件的元数据和功能代码。
3. 使用Qt插件宏将类声明为一个插件。
4. 在编译期间创建一个插件描述文件，并将其添加到项目中。
5. 将插件库编译为一个共享库，并将其添加到应用程序中。
6. 在应用程序中动态加载插件，并使用它们提供的功能。

总之，Qt6插件机制提供了一种方便的方式来扩展和定制应用程序，使得应用程序可以动态加载和使用插件，从而更好地满足用户的需求。

> Python代码生成dll文件的方式

1. 使用Cython和MVS。Cython是一个能够将Python代码转化为C或C++代码的工具，从而实现Python代码的编译和打包。使用Cython可以将Python代码转化为C代码，然后使用Microsoft Visual Studio进行编译和链接，最终生成DLL文件。但是Cython需要使用cythonize命令将python代码转为c代码，然后通过MVS等C语言IDE构建dll项目
2. 使用ctypes库，python内置库，用于在python中调用dll中的函数和变量

```python
import ctypes
lib = ctypes.cdll.LoadLibrary('.dll')
result = lib.func_name(arg1, arg2,...)
```













