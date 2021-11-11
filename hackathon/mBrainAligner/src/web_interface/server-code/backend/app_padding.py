# 内置模块
import json
import logging
import os
import re
import subprocess
import sys
import zipfile
from io import BytesIO
from shutil import copyfile

from flask import Flask, request, send_file
from flask_cors import CORS

sys.path.append("../")

# 第三方模块

# 自定义模块
from terminate_exe import proc_exist
from raw_text import raw  # 不转义路径处理
from send_mail_module import SendMail
from subprocessLocalCmd import subprocessLocal

app = Flask(__name__)
CORS(app)

# 启动日志
logging.basicConfig(level=logging.DEBUG, format="%(message)s")
logger = logging.getLogger()
logger.addHandler(logging.FileHandler("flask.log", "a"))
print = logger.debug


class Common():
    """
    公共参数
    """

    photo_save_path = "/home/ubuntu/mBrainAligner/results/user_upload"  # 上传储存路径
    config_save_file = "/home/ubuntu/mBrainAligner/examples/config/mouse_brain_initial_config.txt" # local registration 参数 写入文件路径，用于写入
    output_config_file_name = "mouse_brain_initial_config.txt" #  local registration 参数文件名， 用于复制文件到下载文件夹中的文件名
    # result_path = "../../results"  # 下载result
    # result_save_file = "result.zip"  # 打包下载文件名

    # sample
    sample_data_path = "/home/ubuntu/mBrainAligner/examples/registered_results/{}/"  # 下载sample的路径
    # sample_save_file_name = "{}_samples.zip" # 下载sample
    # sample_opt_cuurent = ""  # 选择的是哪一个sample option
    sample_options = ["fMOST", "LSFM", "VISoR", "MRI"] # 默认 samples 的类别， 与前端显示保持一致

    # global_reg_command = []

    # exe_cmd = {}

    reg_status = { # 判断 global 和 local 命令执行完毕的状态， 默认均为false
        "global": "false",
        "local": "false"
    }
    # 用户邮箱
    user_email = ""

    # 执行 command 输出日志写入的文件名
    cmd_log_file = "{}_cmd.log"

    # command sh 写入的文件名
    cmd_bat_file = "{}_cmd.sh"

    # 运行的程序
    exe_list = ["global_registration", "local_registration"]

    # 发送邮件的次数
    sent_mail_time = 0

    # 超时时间, 单位：s
    timeout = 60 * 60 * 3  # 3h


def get_abs_path(path):
    """
    获取 path 的绝对路径
    :param path:
    :return:
    """
    abs_path = os.path.abspath(path)
    print("get_abs_path: {} \n".format(abs_path))
    return abs_path


@app.route("/api/register", methods=["POST"])
def register():
    """
    注册邮箱
    :return:
    """
    print("register")
    try:
        # 清空
        Common.user_email = ""
        data = request.get_data()
        user_email = json.loads(data).get("userEmail")
        Common.user_email = user_email
        print(" Common.user_email:{}".format(Common.user_email))

        return json.dumps({"register_status": "true"})
    except Exception as e:
        return json.dumps({"register_status": "false"})


# download_status
@app.route("/api/download_status", methods=["GET"])
def download_status():
    """
    监测 global / local 是否执行完毕， 是否可以下载的状态
    :return:
    """
    print("download_status\n")
    if (Common.reg_status.get("global") == "true" and Common.reg_status.get(
            "local") == "no local") or (
            Common.reg_status.get("global") == "true" and Common.reg_status.get(
        "local") == "true"):
        ready_status = "true"
    else:
        ready_status = "false"
    res = {"ready_status": ready_status, "sent_mail_time": Common.sent_mail_time}
    print("download_status -- res: {}".format(json.dumps(res, ensure_ascii=False)))
    return json.dumps(res, ensure_ascii=False)


@app.route("/api/download", methods=["GET"])
def download():
    """将多个文件打包并下载"""
    # 文件列表
    print("download")
    zip_file_path = request.args.get("zip_file_path") # 下载路径
    folder_name = request.args.get("folder_name") # 文件夹名

    memory_file = BytesIO()

    print(">>zip file path: {}".format(zip_file_path))
    with zipfile.ZipFile(memory_file, "w", zipfile.ZIP_DEFLATED) as zf:
        for parent, dirnames, filenames in os.walk(zip_file_path):
            for file in filenames:
                path_file = os.path.join(parent, file)
                print(">>path_file : {}".format(path_file))
                write_path = "/".join(path_file.split("/")[6:])
                print(">>write_path: {}".format(write_path))
                with open(path_file, "rb") as ff:
                    # zf.writestr(zip中 文件的储存路径， 文件二进制文本)
                    zf.writestr(write_path, ff.read())
                    print("正在下载 文件名 {}".format(path_file))
    print("下载完毕， 路径 {} ".format(zip_file_path))
    memory_file.seek(0)
    return send_file(memory_file, attachment_filename="{}.zip".format(folder_name), as_attachment=True)


@app.route("/api/upload", methods=["POST"])
def upload():
    """
    # 上传图片
    :return:
    """
    print("upload")
    if request.method == "POST":
        photo_obj = request.files.get("file")  # 对应input标签中name="photo"属性
        print("photo_obj:{} ".format(photo_obj))

        # 获取文件名
        photo_name = photo_obj.filename
        print("photo_name: {}".format(photo_name))
        # 储存文件
        photo_obj.save(os.path.join(Common.photo_save_path, photo_name))
    return "ok"


@app.route("/api/sample_data", methods=["GET"])
def sample():
    """
    下载默认 sample 触发
    :return:
    """
    print("sample")
    sample_opt_current = request.args.get("sample_opt_current")
    print("sample_opt_current {} ".format(sample_opt_current))
    if sample_opt_current:
        # 赋值全局变量
        Common.sample_opt_current = sample_opt_current
        print("Common.sample_opt_current:{}".format(Common.sample_opt_current))

        # sample_data_path 赋值 sample data 路径
        sample_data_path = Common.sample_data_path
        sample_data_path = sample_data_path.format(sample_opt_current)
        # 获取相对路径的绝对路径，用于使用在下载时要用到的路径
        sample_data_path = get_abs_path(sample_data_path)

        print("sample_data_path:{} \n".format(sample_data_path))

        # 发送邮件
        res = SendMail().send_text_process(path=sample_data_path, folder_name=sample_opt_current,
                                           receiver=Common.user_email)

        print("sample , zip res: [{}]".format(res))
        return res
    else:
        return json.dumps({"msg": "download sample error"})


@app.route("/api/write_config", methods=["GET"])
def write_config():
    """
    监测前端 local registration 参数有变动时触发，将 local registration 参数写入文件
    :return:
    """
    print("write_config")
    text = request.args.get("config_text")
    print("write config text: {}".format(text))

    with open(Common.config_save_file, "w", encoding="utf-8") as f:
        f.write(text)
        f.flush()
    print("args, write config:{}".format(text))
    return "write_config ok"


@app.route("/api/run_cmd", methods=["GET"])
def run_cmd():
    """
    点击 "run command 时执行"
    :return:
    """
    print("run cmd start !!!")
    # 1. 检测是否存在运行的exe程序，如果有，先kill
    for exe_item in Common.exe_list:
        res = proc_exist(exe_item)  # 返回正在运行的 global / local 的进程pid
        if isinstance(res, int):
            print('run cmd  --{} is running'.format(exe_item))
            os.system("kill {}".format(res))
            print("run cmd  -- {} has closed!".format(exe_item))
        else:
            print('run cmd  -- no such process of [{}]'.format(exe_item))
    # 初始化 sent_mail_time
    Common.sent_mail_time = 0

    # 2. 每执行一次， 初始化 global 和 local 状态：两者均为 true 时， 则表示两个命令运行成功并且结束，此状态下代表可以发送邮件
    Common.reg_status = {
        "global": "false",
        "local": "false"
    }

    # 3. 获取前端 post 的 json 数据
    args = request.args

    # 4. 获取输出文件夹的路径： output path
    out_path = args.get("out_path")
    if out_path:
        out_path = out_path.strip("-o").strip()
        print("run cmd  -- out_path: {} \n".format(out_path))
        if not os.path.exists(out_path):
            os.makedirs(out_path)

    # 下载文件时使用 out_path  的绝对路径
    abs_out_path = get_abs_path(out_path)

    # 5. 获取当前文件夹名
    folder_name = args.get("currentFolderName")
    print("folder_name: {}".format(folder_name))

    # 6. 获取global cmd 参数
    globalRegistrationCmdParams = args.get("globalRegistrationCmdParams")
    global_cmd = raw(globalRegistrationCmdParams)
    print("run cmd -- global_cmd: {}\n".format(global_cmd))

    # 7. 获取local cmd 参数
    localRegCmdParams = args.get("localRegistrationCmdParams")
    local_cmd = raw(localRegCmdParams) if localRegCmdParams else ""
    print("run cmd -- local cmd: {} \n".format(local_cmd))

    # 8. 执行 command 日志文件的绝对路径
    absolute_log_path = os.path.abspath(os.path.join(
        out_path, Common.cmd_log_file.format(folder_name)))

    # 9. 将命令写入文件的绝对路径
    absolute_cmd_bat_path = os.path.abspath(os.path.join(
        out_path, Common.cmd_bat_file.format(folder_name)))

    # 9.1 输出config.txt文件
    if local_cmd:
        src = Common.config_save_file
        to = os.path.join(out_path, Common.output_config_file_name)
        copyfile(src, to)

    # 10 将执行的 command 写入文件
    with open(absolute_cmd_bat_path, "w", encoding="utf-8")as f:
        if not local_cmd:
            # global bat
            bat_txt = global_cmd
        else:
            bat_txt = global_cmd + "\n\n" + local_cmd + "\n\n"
        # print("run cmd -- bat_txt: {} \n".format())
        f.write(bat_txt)
        f.close()

    # 11. 执行命令
    try:
        print("run start !!! \n")

        # 11.1 先执行 global
        print("run cmd -- 执行 global cmd start \n")
        global_obj, global_out = subprocess.getstatusoutput(
            "{} > {}".format(global_cmd, absolute_log_path))

        # 判断执行 global cmd 后返回码：值为1时，代表执行完毕并且成功，此时将 global 的状态改为 true；否则 global 状态为 false
        if global_obj == 0:
            print("value of global_obj: {}\n".format(global_obj))
            print("global cmd is killed by other progress, setting global status = false \n")
            Common.reg_status["global"] = "false"
        else:
            print("value of global_obj: {}\n".format(global_obj))
            print("global cmd status is normal, setting global status = true \n")
            Common.reg_status["global"] = "true"
        # Common.reg_status["global"] = "true"
        print("run cmd -- 执行 global cmd end \n")
        ############################################## 执行 global cmd 结束

        # 11.2 执行 local cmd
        if local_cmd:
            print("run cmd -- 执行 local cmd start \n")
            # 调用可设置超时时间的方法 local_process.execute 执行 local cmd
            local_process_res = local_process.execute(command=local_cmd, timeout=Common.timeout)
            if local_process_res:
                # 返回 状态码，执行日志
                return_code, stdout_content = local_process_res  # 进程的返回值，返回1，则执行成功

                # 将 local cmd 执行产生的 log 写入文件
                print("local process log 加入文件 \n")
                local_process.write(path=absolute_log_path, content=stdout_content)
                # 判断 local cmd 是否执行成功：状态码为 1 代表执行成功并且结束，此时设置 local 状态为 true， 否者，为false
                if return_code == 1:
                    print("local cmd status is normal, setting local status = true \n")
                    Common.reg_status["local"] = "true"
                else:
                    print("local cmd is killed by other progress, setting local status = false \n")
                    Common.reg_status["local"] = "false"

            else:
                print("local_process_res -- None, setting  local status = false \n")
                Common.reg_status["local"] = "false"
            print("run cmd -- 执行 local cmd end \n ")
            ############################################## 执行 local cmd 结束
        else:
            # 此情况下，没有 local cmd
            Common.reg_status["local"] = "no local"
            print("run cmd -- 执行 no local \n ")
        print("run cmd end, updating end !!!\n")

    except Exception as os_system_excetion:
        print("run cmd -- eception: {}".format(os_system_excetion))
        Common.reg_status["local"] = "false"
        # raise

    print("run cmd -- final Comm.reg_status: {} \n".format(Common.reg_status))

    # 12. 如果跑完程序 发送邮件
    if (Common.reg_status.get("global") == "true" and Common.reg_status.get(
            "local") == "no local" and Common.sent_mail_time == 0) or (
            Common.reg_status.get("global") == "true" and Common.reg_status.get(
        "local") == "true" and Common.sent_mail_time == 0):
        print("run cmd -- sending mails ! \n")
        Common.sent_mail_time = 1
        # 调用 SendMail().send_text_process() 发送邮件
        res = SendMail().send_text_process(
            path=abs_out_path, folder_name=folder_name, receiver=Common.user_email)
        print("rum cmd, send mail res: {} \n".format(res))
        mail_status = "true"
    else:
        print("run cmd -- not sending email because of status !!!\n")
        mail_status = "false"

    print("run end!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n")
    di = {}
    di["mail_status"] = mail_status
    return json.dumps(di, ensure_ascii=False)


############################# 日志功能 start
# @app.route("/api/log", methods=["GET"])
# def log():
#     """
#     日志功能
#     :return:
#     """
#     print("log")
#     out_path = request.args.get("path")
#     folder_name = request.args.get("folder_name")
#     # print("run cmd out path: {}".format(out_path))
#     if out_path:
#         out_path = out_path.strip("-o").strip()
#         out_path = os.path.join("./", out_path)
#         print("log, out_path: {}, global_status: {}, local_status: {} \n".format(out_path,
#                                                                                  Common.reg_status.get(
#                                                                                      "global"),
#                                                                                  Common.reg_status.get("local")))
#         if not os.path.exists(out_path):
#             os.makedirs(out_path)
#
#     with open(os.path.join(out_path, Common.cmd_log_file.format(folder_name)), "r", encoding="utf-8",
#               errors='ignore') as f:
#         file_txt = f.read()
#         f.close()
#
#     #  如果跑完程序 log 结束请求
#     if (Common.reg_status.get("global") == "true" and Common.reg_status.get("local") == "no local") or (
#             Common.reg_status.get("global") == "true" and Common.reg_status.get("local") == "true"):
#         log_status = "false"
#     else:
#         log_status = "true"
#     di = {}
#     di["log_text"] = file_txt
#     di["log_status"] = log_status
#     print("log, di.get('log_status'): {}".format(di.get("log_status")))
#     return json.dumps(di, ensure_ascii=False)
############################# 日志功能 end


if __name__ == "__main__":
    # 初始化
    local_process = subprocessLocal()
    app.run(host="0.0.0.0", port=3000, debug=True)
