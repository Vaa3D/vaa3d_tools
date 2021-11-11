# 用途：替换 windows 系统中使用路径是的 "\", 使其不被转义
# linux版本下不需要
import os
trans_map = {
   '\a': r'\a',
   '\b': r'\b',
   '\f': r'\f',
   '\n': r'\n',
   '\r': r'\r',
   '\t': r'\t',
   '\v': r'\v',
   '\'': r'\'',
   '\"': r'\"',
   '\0': r'\0',
   '\1': r'\1',
   '\2': r'\2',
   '\3': r'\3',
   '\4': r'\4',
   '\5': r'\5',
   '\6': r'\6',
   '\7': r'\7',
   '\8': r'\8',
   '\9': r'\9'
}


def raw(text):
   """Returns a raw string representation of text"""
   new_str = ''
   for char in text:
      try:
         new_str += trans_map[char]
      except KeyError:
         new_str += char

   return new_str
#
# text = "mBrainAligner_data\3rdparty\global_registration.exe -f mBrainAligner_data/data/average_template_25_u8_xpad.v3draw -c mBrainAligner_data/data/recentered_image_pad_scr.v3draw -m mBrainAligner_data/Registration_data/18458.v3draw  -p r+s -o mBrainAligner_data/result/ -th 30"
# r = raw(text)
# print(r)
# os.system(r)