import socket

from socket import error

import threading

class ProxyServerTest():

    def __init__(self):
        self.ser=socket.socket(socket.AF_INET,socket.SOCK_STREAM)

    def run(self):
        try:
            self.ser.bind(('127.0.0.1', 8887))
            self.ser.listen(5)
        except error as e:
            print("The local service : "+str(e))
            return "The local service : "+str(e)
        while 1:
            try :
                client,addr=self.ser.accept()
                print('accept %s connect'%(addr,))
                data=client.recv(1024)
                if not data:
                    break
            except error as e:
                print("Local receiving client : "+str(e))
                return "Local receiving client : "+str(e)
            try:
                mbsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                mbsocket.connect(('10.147.19.177', 51895))
                mbsocket.send(data)
                print(data)
            except error as e:
                print("Sent to the proxy server : "+str(e))
                return "Sent to the proxy server : "+str(e)
            while 1:
                try:
                    data_1 = mbsocket.recv(1024)
                    if not data_1:
                        break
                    # print(data_1)
                    client.send(data_1)
                except error as e:
                    print("Back to the client : "+str(e))
                    return "Back to the client : "+str(e)
            client.close()
            mbsocket.close()

def main():

    try:
        pst = ProxyServerTest()
        t = threading.Thread(target=pst.run, name='LoopThread')
        print('Waiting for connection...')
        t.start()
        t.join()
    except Exception as e:
        print("main : "+str(e))
        return "main : "+str(e)
if __name__ == '__main__':
    main()

