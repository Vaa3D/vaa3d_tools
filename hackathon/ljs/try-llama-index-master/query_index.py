import os
import logging
import sys
from llama_index import GPTSimpleVectorIndex
os.environ['OPENAI_API_KEY'] = ''#需要自行获取key api
# logging.basicConfig(stream=sys.stdout, level=logging.INFO)
# logging.getLogger().addHandler(logging.StreamHandler(stream=sys.stdout))

# 加载索引

new_index = GPTSimpleVectorIndex.load_from_disk('/Users/jazz/Desktop/try-llama-index-master/index.json')


if __name__ == '__main__':
    args = sys.argv[1:]
    s = str(args)
    response = new_index.query(s)
    print(response)

