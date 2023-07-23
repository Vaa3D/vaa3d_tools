import json
import requests

class JsonHandler:
    def __init__(self, url):
        self.url = url

    def send_json(self, data):
        headers = {'Content-type': 'application/json'}
        response = requests.post(self.url, data=json.dumps(data), headers=headers)
        return response.json()

    def parse_json(self, json_string):
        return json.loads(json_string)

