#ocoding:utf-8
import sys
from tools.vep_tools import PrintHelper

sys.path.append('..')
from vep_node_port import NodeInput, NodeOutput
from vep_node import Node
from vep_dtypes import VGDtypes

from vep_env import register_node
from vep_config import NodeConfig

NodeConfig.node_title_back_color['StringAction'] = '#800080'


class StringAction(str):

    @register_node(name='StartsWith',input={'target':VGDtypes.String,'prefix':VGDtypes.String},output={'Is':bool},package='StringAction')
    def StartsWith(self:VGDtypes.String,prefix:VGDtypes.String):

        return self.startswith(prefix)

    @register_node(name='Strip',input={'target':VGDtypes.String},output={'str':VGDtypes.String},package='StringAction')
    def Strip(self:VGDtypes.String):
        return self.strip()
    
    @register_node(name='Split',input={'target':VGDtypes.String,'seperator':VGDtypes.String},output={'Splits':VGDtypes.Array},package='StringAction')
    def Split(self:VGDtypes.String,seperator:VGDtypes.String):
        return self.split(seperator)
    
        
