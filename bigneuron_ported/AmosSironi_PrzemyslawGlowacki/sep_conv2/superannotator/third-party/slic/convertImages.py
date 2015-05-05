import commands
import os
import re

exec_cmd = True

#src_dir = 'raw_mitochondria/boundary_originals/'
#dst_dir = 'raw_mitochondria/boundary_originals_jpg/'
#src_dir = 'raw_mitochondria/boundary_annotation/'
#dst_dir = 'raw_mitochondria/boundary_annotation_jpg/'
#src_dir = 'vol1_sub/'
#dst_dir = 'vol1_sub2/'
src_dir = ''
dst_dir = ''
src_extension = '.jpg'
dst_extension = '.png'
options = ' '

# Create directory if it does not exist. 
if not os.access(dst_dir, os.F_OK):
    print 'Creating model ' + dst_dir
    if exec_cmd:
        os.mkdir(dst_dir)

ls_files = commands.getoutput('ls ' + src_dir + '*' + src_extension)
files = ls_files.split()

#print ls_files
#print files

for i in range(0,len(files)):
    #print files[i]
    filename = re.sub('\w+\/','',files[i])
    #print filename
    filenameGroup = re.search('(\w+).(\w+)',filename)
    filename = filenameGroup.group(1) + dst_extension
    #print filename

    cmd = 'convert ' + files[i] + options + dst_dir + filename
    print cmd
    if exec_cmd:
        os.system(cmd)
