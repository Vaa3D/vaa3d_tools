from http.server import BaseHTTPRequestHandler, HTTPServer
import json
from predict import getstart
import SimpleITK as sitk
import numpy as np
class V3DHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers.get('Content-Length', 0))
        post_data = self.rfile.read(content_length)
        # 将接收到的JSON数据解析为Python对象
        data = json.loads(post_data.decode('utf-8'))
        # 在这里处理POST请求的数据，例如获取JSON数据中的字段
        name = data.get('name')

        data_array = np.frombuffer(data['imgdata'].encode('latin1'), dtype=np.uint8)
        print(data_array.shape)
        xdim = data['xdim']
        ydim = data['ydim']
        zdim = data['zdim']
        data_array = np.reshape(data_array, (zdim, ydim, xdim))
        data_array = np.flip(data_array, axis=1)
        print(data_array.shape)
        data_array = np.array(data_array, dtype='uint8')
        # 构造一个JSON格式的响应数据
        print(name)
        # img=sitk.ReadImage(name,sitk.sitkUInt8)
        # img=sitk.GetArrayFromImage(img)
        cent=getstart(data_array)
        print(cent)
        pack=str(int(cent[0]))+","+str(int(cent[1]))+","+str(int(cent[2]))
        response_data = {
            'startpoint': pack
        }
        response_json = json.dumps(response_data)
        # 设置响应头，Content-type为application/json
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        # 返回JSON响应
        self.wfile.write(response_json.encode('utf-8'))


def main():
    ts=HTTPServer(('127.0.0.1',6870),V3DHandler)
    ts.serve_forever()

if __name__=="__main__":
    main()
    # img=sitk.ReadImage(r"D:\A_test\test1\ewa\140921c16.tif.v3dpbd.tiff",sitk.sitkUInt8)
    # img=sitk.GetArrayFromImage(img)
    # cent=getstart(img)