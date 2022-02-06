# coding: utf-8
# 用途： 发送邮件模块
import os
import smtplib
from email.mime.application import MIMEApplication
from email.mime.multipart import MIMEMultipart
import smtplib
from email.mime.text import MIMEText
import json


class SendMail():
    def __init__(self):
        #  服务器
        self.host = "smtp.163.com"
        # 服务器端口
        self.port = 465

        # 授权码
        self.pwd = "RGKGUJOUOGXPYFRM"

        # 发件人
        self.sender = "jarvisgroup@163.com"

        # 服务器地址
        self.server_ip = "http://159.75.201.35"

    def send_text_process(self, path, folder_name, receiver):
            """
            发送文本
            :param path: 读取路径
            :param folder_name: 文件夹名
            :param receiver: 收件人--注册邮箱
            :return:
            """
            try:
                # 收件邮箱
                # self.receiver = "543323433@qq.com"
                # 邮件正文
                # local
                # send_text = '<div>Folder Name: {folder_name} </div>' \
                #             '<p>Please copy the following link and open it with your browser:  <span style="font-style:Italic;font-size:10px;">http://localhost:3000/api/download?zip_file_path={path}&folder_name={folder_name}</span></p>' \
                #             '<div>' \
                #             '<a ' \
                #             'href="http://localhost:3000/api/download?zip_file_path={path}&folder_name={folder_name}" ' \
                #             'style="font-size:16px;color:blue" ' \
                #             'download ' \
                #             'target="_self"' \
                #             '>click to download</a>' \
                #             '</div>'

                # server
                send_text = '<div>Folder Name: {folder_name} </div>' \
                            '<p>Please copy the following link and open it with your browser:  <span style="font-style:Italic;font-size:10px;">{server_ip}/api/download?zip_file_path={path}&folder_name={folder_name}</span></p>' \
                            '<div>' \
                            '<a ' \
                            'href="{server_ip}/api/download?zip_file_path={path}&folder_name={folder_name}". ' \
                            'style="font-size:16px;color:blue" ' \
                            'download ' \
                            'target="_self"' \
                            '>click to download</a>' \
                            '</div>'

                # 设置正文为html
                send_html = MIMEText(send_text.format(folder_name=folder_name, path=path, server_ip=self.server_ip), "html")
                # 设置邮件标题
                send_html["subject"] = "Ready to download [{}]".format(folder_name)

                # 设置发送人
                send_html["from"] = self.sender
                # 设置接收人
                send_html["to"] = receiver

                s = smtplib.SMTP_SSL(host=self.host, port=self.port)
                # 登录邮箱
                s.login(self.sender, self.pwd)
                # 发送邮件
                s.sendmail(self.sender, receiver, send_html.as_string())
                print("sent mail success !")
                return json.dumps({"send_status": "true"}, ensure_ascii=False)
            except Exception as e:
                return json.dumps({"send_status": "false"}, ensure_ascii=False)


    # def send_attach(self, path, folder_name, receiver):
    #     """
    #     发送附件
    #     :param path: 读取路径
    #     :param folder_name: 文件夹名
    #     :param receiver: 收件人--注册邮箱
    #     :return:
    #     """
    #     try:
    #         # 收件邮箱
    #         # self.receiver = "543323433@qq.com"
    #         # 邮件正文
    #         # local
    #         # send_text = '<div>Folder Name: {folder_name} </div>' \
    #         #             '<div>Download Address:  <span style="font-style:Italic;font-size:10px;">http://localhost:3000/api/download?zip_file_path={path}&folder_name={folder_name}</span></div>' \
    #         #             '<div>' \
    #         #             '<a ' \
    #         #             'href="http://localhost:3000/api/download?zip_file_path={path}&folder_name={folder_name}" ' \
    #         #             'style="font-size:16px;color:blue" ' \
    #         #             'download=download target="_self">click to download</a>' \
    #         #             '</div>'
    #
    #         # server
    #         send_text = '<div>Folder Name: {folder_name} </div>' \
    #                     '<div>Download Address:  <span style="font-style:Italic;font-size:10px;">http://159.75.201.35/api/download?zip_file_path={path}&folder_name={folder_name}</span></div>' \
    #                     '<div>' \
    #                     '<a ' \
    #                     'href="http://159.75.201.35/api/download?zip_file_path={path}&folder_name={folder_name}" ' \
    #                     'style="font-size:16px;color:blue" ' \
    #                     'download ' \
    #                     'target="_self"' \
    #                     '>click to download</a>' \
    #                     '</div>'
    #
    #         #构建附件
    #         msg = MIMEMultipart()
    #         file = "{}.zip".format(folder_name)
    #         open_path = "./Registration_sample_data\zip_file"
    #         attach = MIMEApplication(open(os.path.join(open_path, file), "rb").read())
    #         attach.add_header('Content-Disposition', 'attachment', filename=file)
    #         msg.attach(attach)
    #         # print("msg:", msg)
    #
    #
    #         # 设置正文为html
    #         body = MIMEText(send_text.format(folder_name=folder_name, path=path), "html")
    #         msg.attach(body)
    #         # 设置邮件标题
    #         msg["subject"] = "Ready to download [{}]".format(folder_name)
    #
    #         # 设置发送人
    #         msg["from"] = self.sender
    #         # 设置接收人
    #         msg["to"] = receiver
    #
    #         s = smtplib.SMTP_SSL(host=self.host, port=self.port)
    #         # 登录邮箱
    #         s.login(self.sender, self.pwd)
    #         # 发送邮件
    #         s.sendmail(self.sender, receiver, msg.as_string())
    #         print("sent mail to [{}] success !".format(receiver))
    #         return json.dumps({"send_status": "true"}, ensure_ascii=False)
    #     except Exception as e:
    #         return json.dumps({"send_status": "false"}, ensure_ascii=False)




if __name__ == "__main__":
    send_mail = SendMail()
    # path = "./Registration_sample_data/registered_sample_data/LSFM/"
    # path = './Registration_sample_data/registered_sample_data/fMOST/'
    # path = "./Registration_sample_data/user_sample_result/fMOST202106031523/"
    path = r"mBrainAligner_data\3rdparty_windows\user_result\18888.v3draw202106031531/"
    rev = "543323433@qq.com"
    folder_name = "aaaa"
    send_mail.send_mail_process(path, folder_name, rev)
