# import zmq
# context = zmq.Context(1)
# # set up the proxy, for all-to-all capabilities
# proxy_sub = context.socket(zmq.XSUB)
# #proxy_sub.setsockopt(zmq.SUBSCRIBE, "")
# proxy_sub.bind("tcp://*:%s"%5559)
# proxy_pub = context.socket(zmq.XPUB)
# proxy_pub.bind("tcp://*:%s"%5560)
# zmq.device(zmq.PROXY,proxy_sub,proxy_pub)

import zmq

def main():
    try:
        context = zmq.Context(1)
        # Socket facing clients
        frontend = context.socket(zmq.SUB)
        frontend.bind("tcp://*:5559")
        frontend.setsockopt(zmq.SUBSCRIBE, "")
        # Socket facing services
        backend = context.socket(zmq.PUB)
        backend.bind("tcp://*:5560")
        zmq.device(zmq.FORWARDER, frontend, backend)
    except Exception as e:
        print(e)
        print("bringing down zmq device")
    finally:
        pass
        frontend.close()
        backend.close()
        context.term()

if __name__ == "__main__":
    main()
