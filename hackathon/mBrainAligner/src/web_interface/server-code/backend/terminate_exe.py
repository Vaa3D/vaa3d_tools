#coding: utf-8
# 用途：终止进程
import psutil
import os
def proc_exist(process_name):
    pl = psutil.pids()
    # print("pl:",pl)
    for pid in pl:
        # print("pid:", psutil.Process(pid).name())
        if psutil.Process(pid).name() == process_name:
            print("runnung pid : {}".format(pid))
            return pid


if __name__ == "__main__":
    process_name = "global_registration"
    res = proc_exist(process_name)
    print("res: ", res)
    if isinstance(res,int):
        print('chrome.exe is running')
        os.system("kill {}".format(res))
        print("关了 {}".format(process_name))
    else:
        print('no such process...')