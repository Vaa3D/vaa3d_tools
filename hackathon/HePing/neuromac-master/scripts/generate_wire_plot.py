import timeit
import sys,time
import random
import sqlite3
import copy
import numpy
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from configparser import SafeConfigParser # For use with Python 2.7

timer = time.time

# set the views for the 3D plot
a_start = 90.0
e_start = 10.0

def plot_as_wires(cfg_file,db_name,syn_db) :    
    # cfg_file = sys.argv[1]
    # db_name = sys.argv[2]
    parser = SafeConfigParser()
    parser.read(cfg_file)
    colors= ['r','g','b','c','m','k']
    markers = [".","o","s","^","p","*"]
    c_mapping = {}
    m_mapping = {}

    conn = sqlite3.connect(db_name)
    cursor = conn.cursor()
    cursor.execute("select distinct name from swc_data order by name")
    rets = cursor.fetchall()
    names =[]
    c=0
    for entity in rets :
        print("entity: %s with color %s" % (str(entity[0]), colors[c % len(colors)]))
        names.append(entity[0])
        c_mapping[str(entity[0])] = colors[c%len(colors)]
        c = c + 1
    times = []
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("Z")
    xmin,ymin,zmin = 0.0, 0.0, 0.0 
    dim_xyz = eval(parser.get("substrate","dim_xyz"))
    xmax = dim_xyz[0]
    ymax = dim_xyz[1]
    zmax = dim_xyz[2]
    ax.set_xlim([xmin,xmax])
    ax.set_ylim([ymin,ymax])
    ax.set_zlim([zmin,zmax])
    ax.view_init(elev=e_start,azim=a_start)
    for name in names :
        cursor.execute("select * from swc_data where name=? order by id",(str(name),) )
        rets = cursor.fetchall()
        i = 0
        points = []

        u = np.linspace(0, 2 * np.pi, 20)
        v = np.linspace(0, np.pi, 20)
        soma = rets[0]
        soma_x = soma[3]
        soma_y = soma[4]
        soma_z = soma[5]
        radius = soma[9]
        x = soma_x + (radius * np.outer(np.cos(u), np.sin(v)))
        y = soma_y+(radius * np.outer(np.sin(u), np.sin(v)))
        z = soma_z+ (radius * np.outer(np.ones(np.size(u)), np.cos(v)))
        ax.plot_surface(x, y, z,  rstride=4, cstride=4, color=c_mapping[name])

        xlist = []
        ylist = []
        zlist = []        
        for entity in rets :
            from_point = np.array([entity[3],entity[4],entity[5]])
            to_point = np.array([entity[6],entity[7],entity[8]])

            xlist.extend([from_point[0],to_point[0]]) #xpairs[-1])
            xlist.append(np.nan)#None)
            ylist.extend([from_point[1],to_point[1]])#ypairs[-1])
            ylist.append(np.nan)#None)
            zlist.extend([from_point[2],to_point[2]])#zpairs[-1])
            zlist.append(np.nan)#None)

        t0 = timer()
        plt.plot(xlist,ylist,zlist,color=c_mapping[name],alpha=0.5)
        t1 = timer()
        times.append(t1-t0)
    
    if len(syn_db) > 0 :
        syn_conn = sqlite3.connect(syn_db)
        syn_cursor = syn_conn.cursor()
        syn_cursor.execute("select *  from synapses order by pre_syn_entity")
        rets = syn_cursor.fetchall()
        xs = []
        ys = []
        zs = []
        count = 0
        for ret in rets:
            xs.append(ret[2])
            ys.append(ret[3])
            zs.append(ret[4])
            count = count + 1
        ax.scatter(xs,ys,zs,c="b",marker="o")
        print("no_synapses (count)=", count)
    t0 = timer()
    if db_name.startswith(".."):
        out_name = db_name.split("/")[-1].split(".")[0]+"_wire.pdf"
    else: 
        out_name = db_name.split(".")[0]+"_wire.pdf"
    plt.savefig(out_name)
    t1 = timer()
    print("writing the figure took: %fs" % (t1 - t0))
    times.append(t1-t0)
    return np.sum(times)

if __name__=="__main__" :
    cfg_file = sys.argv[1]
    db_name = sys.argv[2]
    syn_db = ""
    try:
        syn_db = sys.argv[3]
    except Exception:
        pass
    tt = plot_as_wires(cfg_file,db_name,syn_db)
    print('total time for wires: ', tt)
