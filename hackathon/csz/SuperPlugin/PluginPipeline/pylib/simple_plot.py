#!/usr/bin/env python

#================================================================
#   Copyright (C) 2021 Yufeng Liu (Braintell, Southeast University). All rights reserved.
#   
#   Filename     : simple_plot.py
#   Author       : Yufeng Liu
#   Date         : 2021-04-08
#   Description  : 
#
#================================================================

import numpy as np
import matplotlib.pyplot as plt


__colors__ = ['b', 'g', 'r', 'c', 'm', 'y', 'k']

def plot_lines(datas, legends, fmts=None, figname='fig.png', linewidth=1, grid=True, xlabel='X-axis', ylabel='Y-axis'):
    if fmts is None:
        fmts = []
        for i in range(len(datas)):
            fmts.append(__colors__[i % len(__colors__)] + 'o-')
        

    for d,l,f in zip(datas, legends, fmts):
        x = np.arange(len(d))
        plt.plot(x, d, f, label=l, linewidth=linewidth, markersize=2)
    
    plt.grid(grid, linestyle='--')
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.legend()
    plt.savefig(figname, dpi=300)

    
if __name__ == '__main__':
    import sys
    import os
    import numpy as np
    import path_util
    
    files = [   '/home/lyf/Research/auto_trace/neuronet/exps/exp005/loss_ce_train.txt', 
                '/home/lyf/Research/auto_trace/neuronet/exps/exp005/loss_dice_train.txt',
                #'/home/lyf/Research/auto_trace/neuronet/exps/exp005/loss_ce_val.txt', 
                #'/home/lyf/Research/auto_trace/neuronet/exps/exp005/loss_dice_val.txt',
    ]
    
    datas, legends = [], []
    for i, f in enumerate(files):
        data = np.genfromtxt(f)
        if os.path.split(f)[-1].find('train') >= 0:
            data = np.convolve(data, np.ones(10)/10, mode='valid')
        leg = path_util.get_file_prefix(f)
        datas.append(data)
        legends.append(leg)

    plot_lines(datas, legends, xlabel='steps', ylabel='loss_ce/loss_dice', linewidth=1)






