import json

from sympy import *

json_env = 'F:/Zhaohu_StitchCode/fixed_overlap-shift.json'

with open(json_env, 'r')as fp:
    json_data = json.load(fp)
eqs = json_data["result_y_x_s"]
my_symbols = set()
for i in eqs:
    my_symbols.add(i[0])
    my_symbols.add(i[1])
my_symbols_str = ''
for i in my_symbols:
    my_symbols_str = my_symbols_str + i + ' '
my_symbols_str = my_symbols_str[0:-1]

my_symbols = symbols(my_symbols_str)

lin_eqs = []

for i in my_symbols:
    tt = 0.0
    for j in eqs:
        if str(i) == j[0]:
            tt = tt + i - Symbol(j[1]) + j[2]
        if str(i) == j[1]:
            tt = tt + i - Symbol(j[0]) - j[2]
    lin_eqs.append(Eq(tt, 0))
lin_eqs.append(Eq(my_symbols[0], 0))
print(lin_eqs)
r = linsolve(lin_eqs, my_symbols)
d = {}
for i in range(len(my_symbols)):
    for j in r:
        d[str(my_symbols[i])] = round(float(j[i]), 2)
with open('F:/Zhaohu_StitchCode/Y-shift_manual_g_o_position.json','w') as f:
    json.dump(d,f,indent=4,ensure_ascii=False)
print(r)