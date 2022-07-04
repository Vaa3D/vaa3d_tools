import json
json_env = 'F:/ZhaoHuImages/AI_denoise/Zhaohu_StitchCode/Z-shift_manual_g_o_position_z.json'

with open(json_env, 'r')as fp:
    json_data_z = json.load(fp)
json_env = 'F:/ZhaoHuImages/AI_denoise/Zhaohu_StitchCode/Y-shift_manual_g_o_position_y.json'

with open(json_env, 'r')as fp:
    json_data_y = json.load(fp)

json_env = 'F:/ZhaoHuImages/AI_denoise/Zhaohu_StitchCode/X-shift_manual_g_o_position_x.json'

with open(json_env, 'r')as fp:
    json_data_x = json.load(fp)

tiles=[]
for i in json_data_z.keys():
    tiles.append(i)
tiles.sort()
d=[]
for i in tiles:
    t_d={}
    t_d['Tile'] = i
    t_d['z'] = json_data_z[i]
    t_d['y'] = json_data_y[i]#注意命名不统一
    t_d['x'] = json_data_x[i]#注意命名不统一
    t_d['x_length'] = 512
    t_d['y_length'] = 512
    d.append(t_d)

dd={}
dd['z_y_x_p'] = d
with open("F:/ZhaoHuImages/AI_denoise/Zhaohu_StitchCode/z_y_x_p.json", 'w') as f:
    json.dump(dd, f, indent=4, ensure_ascii=False)