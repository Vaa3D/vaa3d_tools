#coding:utf-8
import sys
sys.path.append('..')
from vep_node_port import NodeInput,NodeOutput
from vep_node import Node
from vep_dtypes import VGDtypes
from vep_env import register_node
from widgets.PortWidget import *

class FuntionNodeWithNoArgs(Node):

    package_name = 'Test'
    node_title = 'MyTestNode'
    node_description = 'Funtion for no arguments'

    """ void func_name(void) {
    }"""
    input_pins = [
        """execin:  execout?: void"""
    ]

    output_pins = []


# @register_node(name='Sort_swc',
#                package='Vaa3D Plugin Module',
#                output={'Result': VGDtypes.String})
# def SortSWC(data: VGDtypes.String,
#             )

class ImageRead(Node):

    node_title = 'Image Read'
    node_description = '从指定目录读取4DImage图像'

    input_pins = [
        NodeInput(pin_type='exec'),
        NodeInput(pin_name='ImagePath', pin_type='data', pin_class=VGDtypes.String)
    ]

    output_pins = [
        NodeOutput(pin_type='exec',pin_name='Process'),
        # NodeOutput(pin_name='line',pin_class=VGDtypes.String,pin_type='data'),
        NodeOutput(pin_name='File',pin_type='data', pin_class=VGDtypes.File)
    ]

class SizeCal(Node):

    node_title = 'Get Size'

    input_pins = [
        NodeInput(pin_type='exec'),
        NodeInput(pin_name='import', pin_type='data', pin_class=VGDtypes.String)
    ]

    output_pins = [
        # NodeInput(pin_type='exec'),
        NodeOutput(pin_name='len', pin_type='data', pin_class=VGDtypes.Integer)
    ]


class ImportNode(Node):

    node_title = 'Import'

    input_pins = [
        NodeInput(pin_type='exec'),
        NodeInput(pin_name='Import', pin_type='data', pin_class=VGDtypes.String)

    ]

    output_pins = [
        NodeOutput(pin_type='exec')
    ]


class IndexNode(Node):

    node_title = 'Index'

    input_pins = [
        NodeInput(pin_name='index', pin_type='data', pin_class=VGDtypes.File),
        NodeInput(pin_name='change', pin_type='data', pin_class=VGDtypes.String)
    ]

    output_pins = [
        NodeOutput(pin_type='data', pin_class=VGDtypes.Integer)
    ]

class VarWithAPI(Node):
    node_title = 'Var API Test'

    input_pins = [
        NodeInput(pin_type='exec'),
        NodeInput(pin_name='API', pin_type='data', pin_class=VGDtypes.String),
        NodeInput(pin_name='', pin_type='data', pin_widget=IntInputWdg),
        NodeInput(pin_name='', pin_type='data', pin_widget=FloatInputWdg),
        NodeInput(pin_name='', pin_type='data', pin_widget=StringInputWdg),
        NodeInput(pin_name='', pin_type='data', pin_widget=CheckboxWdg),
        NodeInput(pin_name='', pin_type='data', pin_widget=SelectComboBox)

    ]

    output_pins = [
        NodeOutput(pin_type='exec'),
        NodeOutput(pin_type='data', pin_class=VGDtypes.Integer)
    ]

class FileInput(Node):
    package_name = 'File Action'
    node_title = 'Input File'

    input_pins = [
        NodeInput(pin_type='exec'),
        NodeInput(pin_name='File Path', pin_widget=FileInputWdg, pin_type='data')
    ]
    output_pins = [
        NodeOutput(pin_type='exec'),
        NodeOutput(pin_name='file', pin_type='exec')
    ]

class FileStream(Node):
    package_name = 'File Action'
    node_title = "File Stream"

    input_pins = [
        NodeInput(pin_type='exec'),
        NodeInput(pin_name='operation', pin_type='data', pin_widget=SelectComboBox)
    ]

    output_pins = [
        # NodeOutput(pin_type='exec'),
        NodeOutput(pin_name='op', pin_type='exec')
        # NodeOutput(pin_name='operation', pin_type='data', pin_widget=SelectComboBox)
    ]

class ParseFile(Node):
    package_name = 'File Action'
    node_title = 'Parse File'
    input_pins = [
        NodeInput(pin_type='exec'),
        NodeInput(pin_name='File Op', pin_type='exec'),
        NodeInput(pin_name='module', pin_widget=SelectComboBox),
        NodeInput(pin_name='line data', pin_widget=SelectComboBox),
        NodeInput(pin_name='output', pin_widget=SelectComboBox),
    ]

    output_pins = [
        NodeOutput(pin_type='exec')
        # NodeOutput(pin_name='output', pin_widget=SelectComboBox)
    ]

class SortSWCNode(Node):
    package_name = 'Module Funtion'
    node_title = 'sort swc'
    input_pins = [
        NodeInput(pin_name='input',pin_type='exec'),
        NodeInput(pin_name='output path', pin_widget=SelectComboBox)

    ]

    output_pins = [
        NodeOutput(pin_name='output', pin_type='exec')
    ]


class ImageWindowNode(Node):
    package_name = 'Module Funtion'
    node_title = 'Vaa3D Image Window'
    input_pins = [
        NodeInput(pin_name='input',pin_type='exec'),
        NodeInput(pin_name='operation', pin_widget=SelectComboBox)

    ]

    output_pins = [
        NodeOutput(pin_name='output', pin_type='exec')
    ]

class ImageNode(Node):
    package_name = 'Module Funtion'
    node_title = 'Vaa3D Image Operation'
    input_pins = [
        NodeInput(pin_name='input',pin_type='exec'),

        NodeInput(pin_name='type', pin_type='var', pin_class=VGDtypes.String),
                 NodeInput(pin_name='operation', pin_widget=SelectComboBox)
    ]

    output_pins = [
        NodeOutput(pin_name='output', pin_type='exec')
    ]

class APP2Node(Node):
    package_name = 'Module Funtion'
    node_title = 'APP2'
    input_pins = [
        NodeInput( pin_type='exec'),
        NodeInput(pin_name='input', pin_type='exec'),
        NodeInput(pin_name='Output Path', pin_widget=FileInputWdg, pin_type='data')

    ]

    output_pins = [
        NodeOutput(pin_type='exec'),
        NodeOutput(pin_type='data', pin_name='output path')
    ]