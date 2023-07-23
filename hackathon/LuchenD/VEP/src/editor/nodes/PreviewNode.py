#coding:utf-8

import sys
from tools.vep_tools import PrintHelper

sys.path.append('..')
from vep_node_port import NodeInput, NodeOutput
from vep_node import Node
from vep_dtypes import VGDtypes

from vep_config import NodeConfig
from vep_env import register_node

import pandas as pd

NodeConfig.node_title_back_color['Preview'] = '#ef2A2A'

class PreviewTable(Node):

    package_name = 'Preview'
    node_title = 'Preview Table'
    node_description = '将Table的数据进行预览'
    is_preview_node = True 
    preview_port_index = 0

    input_pins = [
        NodeInput(pin_type='data',pin_class=VGDtypes.Table,pin_name='Table')
    ]

    output_pins = [
    ]

    def run_node(self):
        self.input(0)
    
