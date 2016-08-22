__author__ = 'coriannaj'

###

import os
import psycopg2

def get_mip(series_num):

    try:
        conn = psycopg2.connect(host="limsdb2", database="lims2", user="atlasreader", password="atlasro", port="5432")
    except psycopg2.Error as e:
        print "Unable to connect to LIMS"
        print e
        return

    cur = conn.cursor()

    series_query = '%' + series_num

    cur.execute("SELECT id, storage_directory FROM specimens splice"
                " WHERE name LIKE %s",[series_query])

    record =  cur.fetchone()
    img_name = "min_xy_" + str(record[0]) + ".jp2"
    img_path = os.path.join(record[1],img_name)

    print img_path

    cur.close()
    conn.close()

    return img_path

get_mip('241547.03.02.01')