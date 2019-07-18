import timeit
import sys,time
import random
import sqlite3
import copy
import numpy
import numpy as np
import matplotlib.pyplot as plt
# from mpl_toolkits.mplot3d import Axes3D
from configparser import SafeConfigParser # For use with Python 2.7s

def collect_swc_data(db_name,cfg_file) :
    parser = SafeConfigParser()
    parser.read(cfg_file)
    
    conn = sqlite3.connect(db_name)
    cursor = conn.cursor()
    cursor.execute("select distinct name from swc_data order by name")
    rets = cursor.fetchall()  # fetches all (or all remaining) rows of a query result set and returns a list of tuples
    names =[]
    c=0
    for entity in rets :
        names.append(entity[0])

    prefix=""
    if db_name.startswith(".."):
        prefix = db_name.split("/")
        print(".. prefix: ", prefix, "\nNot sure this works correctly...")
    else:
        prefix = "/".join(db_name.split(".")[0].split("/")[:-1])
        print("prefix: ", prefix)

    all_points = {}
    soma_radius = None
    for name in names :
        all_points[name] = {}
        cursor.execute("select * from swc_data where name=? order by id",(str(name),) )
        rets = cursor.fetchall()
        points = {}
        is_soma = True

        print('processing: ', name)
        for entity in rets :
            contents = {}
            index = entity[0]
            if is_soma:
                points['soma'] = (entity[3],entity[4],entity[5])
                soma_radius = parser.getint(entity[1].split("__")[0], "soma_radius")
                print("found soma_radius: ", soma_radius)
                is_soma = False
            contents['name'] = entity[1]
            contents['swc_type'] = entity[2]
            contents['f'] = (entity[3],entity[4],entity[5])
            contents['t'] = (entity[6],entity[7],entity[8])
            contents['r'] = entity[9]
            points[entity[0]] =contents
        write_swc(name,points,prefix,soma_radius)
        all_points[name] = points

def write_swc(name,points,prefix,soma_radius) :
    if len(prefix) > 0:
        print("writing to file: ", prefix + "/" + name + ".swc")
        writer = open(prefix+"/"+name+".swc","w")
    else:
        print("writing to file: ", name + ".swc")
        writer = open(name+".swc","w")
        
    # insert the soma + first segments
    sp = points['soma']
    print("construcing som with soma_radius: ", soma_radius)
    soma_str = "1 1 %s %s %s %s -1\n" % (sp[0],sp[1],sp[2],soma_radius)
    soma_str += "2 1 %s %s %s %s 1\n" % (sp[0],sp[1]-soma_radius,sp[2],soma_radius)
    soma_str += "3 1 %s %s %s %s 1\n" % (sp[0],sp[1]+soma_radius,sp[2],soma_radius)
    writer.write(soma_str)
    writer.flush()
    
    index_mapping = {}
    new_index = 4
    index_for_later = []
    for index in sorted(points.keys()) :
        # prepare line for SWC
        if index == 'soma':
            continue
        # prepare line for SWC
        to_p = points[index]['t']
        f_p = points[index]['f']
        swc_type = points[index]['swc_type']
        if points[index]['f'] == points['soma']:
            # stems rooted at the soma
            to_write = "%i %i %s %s %s %s %i\n" % (new_index,swc_type, to_p[0],to_p[1], to_p[2],points[index]['r'],1)
            writer.write(to_write)
            writer.flush()
            index_mapping[index] = new_index
            new_index = new_index +1
        else:
            try:
                parent_index = _from_point(points[index]['f'],points)
                #print "parent_index: ", parent_index
                if parent_index in index_mapping:
                    converted_index = index_mapping[parent_index]
                    to_write = "%i %i %s %s %s %s %i\n" % \
                      (new_index, swc_type,to_p[0],to_p[1], to_p[2],points[index]['r'],converted_index)
                    index_mapping[index] = new_index
                    writer.write(to_write)
                    writer.flush()
                    index_mapping[index] = new_index
                    new_index = new_index +1                     
                else:
                    print("Could not find %s in the converted list..." % (parent_index))
                    print("Error occured in convertion file to SWC (index:%s, parent=%s)" % (index, parent_index))
                    print("converte_index: ", converted_index)
                    index_for_later.append(index)
                    #time.sleep(2)           
            except KeyError:
                print("Error occured in convertion file to SWC (index:%s, parent=%s)" % (index, parent_index))

    print("Done, now take care of the leftover indices...[for indices not inserted in order]")

    for index in sorted(index_for_later):
        to_p = points[index]['t']
        f_p = points[index]['f']
        swc_type = points[index]['swc_type']
        if points[index]['f'] == points['soma']:
            # stems rooted at the soma
            to_write = "%i %i %s %s %s %s %i\n" % (new_index,swc_type, to_p[0],to_p[1], to_p[2],points[index]['r'],1)
            writer.write(to_write)
            writer.flush()
            index_mapping[index] = new_index
            new_index = new_index +1
        else:
            try:
                parent_index = _from_point(points[index]['f'],points)
                #print "parent_index: ", parent_index
                converted_index = index_mapping[parent_index]
                to_write = "%i %i %s %s %s %s %i\n" % \
                  (new_index, swc_type,to_p[0],to_p[1], to_p[2],points[index]['r'],converted_index)
                index_mapping[index] = new_index
                writer.write(to_write)
                writer.flush()
                index_mapping[index] = new_index
                new_index = new_index +1                     
            except KeyError:
                print("LATER:: Error occured in convertion file to SWC (index:%s, parent=%s)" % (index, parent_index))


def _from_point(p,points) :
    for index in points:
        if index == 'soma' :
            continue
        if points[index]['t'] == p :
            return index
    print("not found: ", p, ' [should not happen]')


if __name__=="__main__" :
    cfg_file = sys.argv[1]
    db_name = sys.argv[2]
    collect_swc_data(db_name,cfg_file)
