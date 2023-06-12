import requests

def jc_proxies():



    s = requests.session()

    proxies={

        "http": "127.0.0.1:8887",

    }

    r = s.get('http://127.0.0.1:51895',proxies=proxies,timeout=10)

    print(r.content)



jc_proxies()