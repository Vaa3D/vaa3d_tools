#!/usr/bin/env python

#================================================================
#   Copyright (C) 2021 Yufeng Liu (Braintell, Southeast University). All rights reserved.
#   
#   Filename     : path_util.py
#   Author       : Yufeng Liu
#   Date         : 2021-04-01
#   Description  : 
#
#================================================================

import os

def get_file_prefix(filepath):
    filename = os.path.split(filepath)[-1]
    prefix = os.path.splitext(filename)[0]
    return prefix

def get_file_extension(filepath):
    filename = os.path.split(filepath)[-1]
    _, ext = os.path.splitext(filename)
    return ext

