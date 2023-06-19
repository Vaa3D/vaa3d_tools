#encoing: utf-8

import json
import socket

class VepServer:

    def __init__(self):
        super(VepServer, self).__init__()

    """
    message、host和port。其中，message是要发送的消息，host是目标主机的IP地址，port是目标主机的端口号。函数内部使用socket模块创建了一个TCP连接，将消息编码后发送给目标主机
    """
    def send_message(self, message, host, port):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((host, port))
            s.sendall(json.dumps(message).encode())
