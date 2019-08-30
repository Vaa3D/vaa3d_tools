import sys
import random
import sqlite3
import copy
import numpy as np
import scipy

from configparser import SafeConfigParser # For use with Python 2.7

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
plt.rcParams['animation.ffmpeg_path'] = 'C:/FFmpeg/bin/ffmpeg'
import matplotlib.animation as manimation

from mpl_toolkits.mplot3d import Axes3D

# set the views for the 3D plot
a_start = 90.0
e_start = 10.0

if __name__ == "__main__" :
    cfg_file = sys.argv[1]
    db_name = sys.argv[2]
    parser = SafeConfigParser()
    parser.read(cfg_file)
    print("db_name: %s " % (db_name))

    FFMpegWriter = manimation.writers['ffmpeg']
    metadata = dict(title='Movie Test', artist='Matplotlib',
            comment='Movie support!')
    writer = FFMpegWriter(fps=5, metadata=metadata)
        
    fig = plt.figure(1) # the main figure woth skeleton plot
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
    ax.view_init(elev=e_start, azim=a_start)
    #ax.set_aspect('equal')
    if db_name.startswith(".."):
        movie_name = db_name.split("/")[-1].split(".")[0]+".mp4"
    else: 
        movie_name = db_name.split(".")[0]+".mp4"
    print("movie_name: ", movie_name)

    colors= ['r','g','b','c','m','k']
    markers = ["."]#,"x","o"]#,"o","s","^","p","*"]
    c_mapping = {}
    m_mapping = {}
    
    conn = sqlite3.connect(db_name)
    cursor = conn.cursor()
    cursor.execute("select distinct name from swc_data order by name")
    rets = cursor.fetchall()
    c = 0
    for entity in rets :
        print("entity: %s with color %s" % (str(entity[0]), colors[c % len(colors)]))
        c_mapping[str(entity[0])] = colors[c%len(colors)]
        c = c + 1

    cursor.execute("select distinct proc from swc_data order by proc")
    rets = cursor.fetchall()
    c = 0
    for entity in rets :
        print("proc: %s with marker %s" % (str(entity[0]), markers[c % len(markers)]))
        m_mapping[str(entity[0])] = markers[c%len(markers)]
        c = c + 1        
    
    cursor.execute("select * from swc_data order by id")
    rets = cursor.fetchall()
    colors= ['r','g','b','c','m','k']
    counter = 0
    with writer.saving(fig,movie_name,100) :
        for row in rets:
            counter = counter +1
            if counter % 20 == 0:
                print('counter >', counter, '<')
            swc_id,name,swc_type,x1,y1,z1,x2,y2,z2,radius,proc = row
            plt.plot([x1,x2],[y1,y2],zs=[z1,z2],color=c_mapping[str(name)])#,marker=m_mapping[str(proc)])
            writer.grab_frame()
        # for ii in np.linspace(a_start,a_start+360,20):
        #     ax.view_init(elev=e_start, azim=ii)
        #     writer.grab_frame()
