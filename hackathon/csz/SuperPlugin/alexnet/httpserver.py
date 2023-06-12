from http.server import BaseHTTPRequestHandler, HTTPServer
import json
from predict import svmpredict,alexnetpredict
from v3dplugin import getResult
import numpy as np
import SimpleITK as sitk

class V3DHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers.get('Content-Length', 0))
        post_data = self.rfile.read(content_length)
        # 将接收到的JSON数据解析为Python对象
        data = json.loads(post_data.decode('utf-8'))
        # 在这里处理POST请求的数据，例如获取JSON数据中的字段
        imgpath=data['name']

        imgquality=data['quality']
        svmres=-1
        alexres=-1

        if len(imgquality)>0:
            svmres=svmpredict(imgquality)
        if len(imgpath)>0:
            alexres=alexnetpredict(imgpath)
        svmres=int(svmres)
        alexres=int(alexres)
        # print(svmres,alexres)
        # if svmres<3:
        #     svmres=1
        # else:
        #     svmres=2
        # if alexres<3:
        #     alexres=1
        # else:
        #     alexres=2
        print(imgpath,svmres,alexres)
        response_data = {
            'imgpath':imgpath,
            'qres': str(1),
            'sres': str(1)
        }
        response_json = json.dumps(response_data)
        # 设置响应头，Content-type为application/json
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        # 返回JSON响应
        self.wfile.write(response_json.encode('utf-8'))


def main():
    ts=HTTPServer(('127.0.0.1',6869),V3DHandler)
    ts.serve_forever()

if __name__=="__main__":
    main()