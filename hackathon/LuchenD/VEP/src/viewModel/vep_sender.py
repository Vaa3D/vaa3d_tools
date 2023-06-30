#encoding: utf-8

import json
import sys
from vep_node_port import NodePort

from PySide6.QtCore import QAbstractItemModel, QObject
from PySide6.QtWidgets import Q


class VepSender:

    def __init__(self):
        super(VepSender, self).__init__()

        self._port_list: list[NodePort] = []


    def setPort(self, port, port_type):
        self._port_list.append(port)


class JsonModel(QAbstractItemModel):

    def __init__(self, parent: QObject = None):
        super(JsonModel, self).__init__(parent)

        self._headers = ("key", "value")

    def clear(self):
        """ Clear data from the model"""
        self.load({})

    def load(self, document: dict):
        """ Load model from a nested dictionary returned by json.loads()

        Arguments:
            document (dict): Json-compatible dictionary
        """

        assert isinstance(
            document, (dict, list, tuple)
        ), "`document` must be of dict, list or tuple," f"not {type(document)}"

        self.beginResetModel()

        self.endResetModel()
        return True
