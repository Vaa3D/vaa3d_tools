

## 前置准备

#### 项目介绍

```
本项目可以自行扩展，目的在于构建智能Vaa3D-x数据平台，帮助用户解答有关于Vaa3D-x的相关问题，可以在data中制作相应的语料数据，结合GPT，即可实现智能的数据平台。
```

### 安装依赖

```shell
pip3 install -r requirments.txt
```

### 获取openai的API Key

可以参考openai的官方文档：https://platform.openai.com/account/api-keys

## 运行

#### 在代码中填入API key，或者设置环境变量

```shell
export OPENAI_API_KEY=XXYYZZ
```

#### 建立索引

```shell
python3 build_index.py
```

#### 查询索引

```shell
python3 query_index.py
```
