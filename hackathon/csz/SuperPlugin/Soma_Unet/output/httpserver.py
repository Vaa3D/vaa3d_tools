from http.server import BaseHTTPRequestHandler, HTTPServer
import json

class V3DHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers.get('Content-Length', 0))
        post_data = self.rfile.read(content_length)
        # 将接收到的JSON数据解析为Python对象
        data = json.loads(post_data.decode('utf-8'))
        # 在这里处理POST请求的数据，例如获取JSON数据中的字段
        name = data.get('path')
        type = data.get('type')
        # 构造一个JSON格式的响应数据
        print(name,type)
        response_data = {
            'CompletedPath': 'ok',
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