import json
json_env = 'F:/ZhaoHuImages/AI_denoise/Zhaohu_StitchCode/X-shift_manual_g_o_position.json'
with open(json_env, 'r')as fp:
    json_data = json.load(fp)
d = {}
my_min = 0
for i in json_data.keys():
    if json_data[i] < my_min:
        my_min = json_data[i]
for i in json_data.keys():
    json_data[i] = json_data[i] - my_min
for i in json_data.keys():
    d[i] = round(json_data[i])
with open('F:/ZhaoHuImages/AI_denoise/Zhaohu_StitchCode/X-shift_manual_g_o_position_x.json','w') as f:
    json.dump(d, f, indent=4, ensure_ascii=False)