#coding:utf-8
import sys

sys.path.append('..')
from vep_node_port import NodeInput, NodeOutput
from vep_node import Node
from vep_dtypes import VGDtypes


class BeginNode(Node):
    package_name = 'Action Default'
    node_title = 'Begin To Run'
    node_description = '本graph必须包含的一个节点，graph的开始'

    input_pins = [
    ]

    output_pins = [
        NodeOutput(pin_name='Begin',pin_type='exec')
    ]

    
    def run_node(self):
        self.exec_output(0)

from widgets.PortWidget import StringArrayWdg, ReadFileWdg, WriteFileWdg
class BeginNodeWithInclude(Node):
    package_name = 'Action Default'
    node_title = 'Begin To Run With Include'
    node_description = '本graph必须包含的一个节点，graph的开始,同时包含Include列表'

    input_pins = [
        NodeInput(pin_name='Include', pin_type='data', pin_class=VGDtypes.File, pin_widget=ReadFileWdg)
    ]

    output_pins = [
        NodeOutput(pin_name='Begin', pin_type='exec')
    ]

    def run_node(self):
        self.exec_output(0)


class PrintNode(Node):
    package_name = 'Action Default'
    node_title = 'Print To Console'
    node_description = '本graph必须包含的一个节点，graph的开始'

    input_pins = [
        NodeInput(pin_name='', pin_type='exec'),
        NodeInput(pin_name='str', pin_class=VGDtypes.String)
    ]

    output_pins = [NodeOutput(pin_name='',pin_type='exec'),NodeOutput(pin_name='str',pin_class = VGDtypes.String)]

    def run_node(self):

        input_v = self.input(1)
        print(input_v)
        self.output(1,input_v)
        self.exec_output(0)
