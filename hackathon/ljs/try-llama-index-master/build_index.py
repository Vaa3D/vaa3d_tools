import logging
import sys
from llama_index import GPTSimpleVectorIndex, Document, SimpleDirectoryReader
import os


import openai


os.environ['OPENAI_API_KEY'] = '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'
logging.basicConfig(stream=sys.stdout, level=logging.INFO)
logging.getLogger().addHandler(logging.StreamHandler(stream=sys.stdout))

# 读取data文件夹下的文档
documents = SimpleDirectoryReader('data').load_data()

from llama_index import LLMPredictor, GPTSimpleVectorIndex, PromptHelper, ServiceContext
from langchain import OpenAI

# define LLM
llm_predictor = LLMPredictor(llm=OpenAI(temperature=0, model_name="text-davinci-003"))

# define prompt helper
# set maximum input size
max_input_size = 4096
# set number of output tokens
num_output = 512
# set maximum chunk overlap
max_chunk_overlap = 20
prompt_helper = PromptHelper(max_input_size, num_output, max_chunk_overlap)

service_context = ServiceContext.from_defaults(llm_predictor=llm_predictor, prompt_helper=prompt_helper)

index = GPTSimpleVectorIndex.from_documents(
    documents, service_context=service_context
)

# index = GPTSimpleVectorIndex.from_documents(documents)

index.save_to_disk('index.json')

