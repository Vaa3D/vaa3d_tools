#!/usr/bin/python
# -*- coding: UTF-8 -*-
'''
1. go into your own project folder by terminal
2. input example: python createProBaseFile.py solutionName
3. you will see there has been a new folder named solutionName
4. go into solutionName folder and you will see 3 files:solutionName.pro, solutionName_plugin.h, solutionName_plugin.cpp

Sun Oct 14 11:08:29 2018 by wp
'''
import sys
import time
import os




def main():
    """
     通过sys模块来识别参数demo, http://blog.csdn.net/ouyang_peng/
    """
    print('参数个数为:', len(sys.argv), '个参数。')
    print('参数列表:', str(sys.argv))
    print('脚本名为：', sys.argv[0])
    for i in range(0, len(sys.argv)):
        print('参数 %s 为：%s' % (i, sys.argv[i]))
    if len(sys.argv) !=2:
        print("You can only input 1 solutionName!");
        return False;
    # if(len)
    createPro(sys.argv[1]);
    createH(sys.argv[1]);
    createCpp(sys.argv[1]);



def createPro(solutionName):
    str="\n"\
        "TEMPLATE = lib\n"\
        "CONFIG += qt plugin warn_off\n"\
        "#CONFIG	+= x86_64\n"\
        "VAA3DPATH = ../../../../v3d_external\n"\
        "V3DMAINPATH = $$VAA3DPATH/v3d_main\n"\
        "INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun\n"\
        "\n"\
        "HEADERS += %s_plugin.h\n"\
        "SOURCES += %s_plugin.cpp\n"\
        "SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp\n"\
        "\n"\
        "TARGET = $$qtLibraryTarget(%s)\n"\
        "DESTDIR = $$V3DMAINPATH/../bin/plugins/wpkenanPlugin/%s\n"\
        "\n"\
        %(solutionName,solutionName,solutionName,solutionName);

    filePath = os.getcwd() + "\\%s" % (solutionName);
    if not os.path.exists(filePath):
        os.makedirs(filePath);
    filePath=filePath+"\\%s.pro"%(solutionName);

    if not os.path.exists(filePath):
        file=open(filePath,"w");
        file.write(str);



def createH(solutionName):
    str="/*\n" \
        "%s_plugin.h\n" \
        "%s\n" \
        "*/\n" \
        "\n" \
        "#ifndef __%s_PLUGIN_H__\n"\
        "#define __%s_PLUGIN_H__\n"\
        "\n"\
        "#include <QtGui>\n"\
        "#include <v3d_interface.h>\n"\
        "\n"\
        "class %sPlugin: public QObject, public V3DPluginInterface2_1\n" \
        "{\n" \
        "   Q_OBJECT\n" \
        "   Q_INTERFACES(V3DPluginInterface2_1);\n"\
        "\n"\
        "public:\n"\
        "   float getPluginVersion() const{return 1.1f;}\n\n"\
        "   QStringList menulist() const;\n" \
        "   void domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent);\n\n"\
        "   QStringList funclist() const;\n"\
        "   bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent);\n"\
        "};\n"\
        "\n"\
        "#endif\n"\
        %(solutionName,time.asctime( time.localtime(time.time()) ),solutionName.upper(),solutionName.upper(),solutionName);

    filePath = os.getcwd() + "\\%s" % (solutionName);
    if not os.path.exists(filePath):
        os.makedirs(filePath);
    filePath = filePath + "\\%s_plugin.h" % (solutionName);

    if not os.path.exists(filePath):
        file = open(filePath, "w");
        file.write(str);

def createCpp(solutionName):
    str="/*\n" \
        "%s_plugin.cpp\n" \
        "%s by wp\n" \
        "*/\n" \
        "\n" \
        "#include \"v3d_message.h\"\n" \
        "#include <vector>\n" \
        "#include \"%s_plugin.h\"\n" \
        "#include<iostream>\n" \
        "#include <QString>\n" \
        "#include <QtCore/QFile>\n" \
        "using namespace std;\n" \
        "\n" \
        "Q_EXPORT_PLUGIN2(%s,%sPlugin);\n" \
        "void function(V3DPluginCallback2 &callback,QWidget *parent);\n" \
        "bool function(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent);\n" \
        "const QString title = QObject::tr(\"%s\");\n" \
        "\n" \
        "QStringList %sPlugin::menulist() const{\n" \
        "   return QStringList()\n" \
        "       << tr(\"function\")\n" \
        "       << tr(\"about\");\n" \
        "}\n" \
        "\n" \
        "QStringList %sPlugin::funclist() const{\n" \
        "   return QStringList()\n" \
        "       << tr(\"function\")\n" \
        "       << tr(\"help\");\n" \
        "}\n" \
        "\n" \
        "void %sPlugin::domenu(const QString &menu_name,V3DPluginCallback2 &callback,QWidget *parent){\n" \
        "   if(menu_name==tr(\"function\")){\n" \
        "       function(callback,parent);\n" \
        "   }\n" \
        "   else{\n" \
        "       v3d_msg(tr(\"developing...%s by wp\"));\n" \
        "   }\n" \
        "}\n" \
        "\n" \
        "bool %sPlugin::dofunc(const QString &func_name,const V3DPluginArgList &input,V3DPluginArgList &output,V3DPluginCallback2 &callback,QWidget *parent){\n" \
        "   if(func_name==tr(\"function\")){\n" \
        "       return function(callback,input,output,parent);\n" \
        "   }\n" \
        "   else if(func_name==tr(\"help\")){\n" \
        "       return true;\n" \
        "   }\n" \
        "   return false;\n" \
        "}\n" \
        "\n" \
        "void function(V3DPluginCallback2 &callback,QWidget *parent){\n\n}\n" \
        "\n" \
        "bool function(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent){\nreturn true;\n}\n" \
        "\n" \
        %(solutionName,time.asctime(time.localtime(time.time())),solutionName,solutionName,solutionName,solutionName,solutionName,solutionName,solutionName,time.asctime(time.localtime(time.time())),solutionName)

    filePath = os.getcwd() + "\\%s" % (solutionName);
    if not os.path.exists(filePath):
        os.makedirs(filePath);
    filePath = filePath + "\\%s_plugin.cpp" % (solutionName);

    if not os.path.exists(filePath):
        file = open(filePath, "w");
        file.write(str);





if __name__ == "__main__":
    main()
