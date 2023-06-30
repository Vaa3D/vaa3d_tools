#coding:utf-8
import sys

sys.path.append('..')
from vep_node_port import NodeInput, NodeOutput
from vep_node import Node
from vep_dtypes import VGDtypes


class TableReader:

    package_name = 'Data Processing'

    node_title = 'Read CSV'
    node_description = '读取CSV'

    input_pins = [
        NodeInput(pin_type='exec',pin_name=''),
        NodeInput(pin_name='CSV Path', pin_type='data', pin_class=VGDtypes.String)
    ]

    output_pins = [
        NodeOutput(pin_name='',pin_type='exec'),
        NodeOutput(pin_name='str', pin_type='data', pin_class=VGDtypes.String)
    ]

    def run_node(self):

        self.output(0, 'True' if self.input(0) else "False")
