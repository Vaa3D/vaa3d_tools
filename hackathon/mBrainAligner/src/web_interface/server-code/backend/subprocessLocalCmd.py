# 用途：用线程控制执行 command 时的超时问题
import os
import subprocess
from threading import Timer

class subprocessLocal():
    """
    local cmd 执行时间过长，使用线程对其进程超时设置
    """

    def kill_command(self, p):
        """终止命令的函数"""
        p.kill()

    def write(self, path, content):
        with open(path, "a", encoding="utf-8") as f:
            # content = content.decode("utf-8")
            f.write(content)
            print("local log 写入完毕\n")

    def execute(self, command, timeout):
        """
        执行 command
        :param command:
        :param timeout:
        :return:
        """
        print("subprocessLocal -- before Popen \n")
        # 执行shell命令
        p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        print("subprocessLocal -- after Popen \n")
        # 设置定时器去终止这个命令
        timer = Timer(timeout, self.kill_command, [p])
        return_code, stdout_content = "",""
        try:
            timer.start()
            stdout, stderr = p.communicate() # stdout: 执行cmd输出日志
            return_code = p.returncode
            print("subprocessLocal try -- return code: {}\n".format(return_code))
            stdout_content = stdout.decode("utf-8")
            print("subprocessLocal try -- stdout_content end \n")

        except Exception as ex:
            print("exception: ", ex)
            return None
        finally:
            print("finally")
            timer.cancel()
            print("subprocessLocal finally-- return code: {}\n".format(return_code))
            print("subprocessLocal finally-- stdout_content end \n")
            return return_code, stdout_content

if __name__ == "__main__":
    cmd1 = "/home/ubuntu/mBrainAligner/dist/linux_bin/global_registration -f /home/ubuntu/mBrainAligner/examples/target/CCF_25_u8_xpad.v3draw -c /home/ubuntu/mBrainAligner/examples/target/CCF_mask.v3draw -m /home/ubuntu/mBrainAligner/examples/subject/LSFM_raw.v3draw -p r+f -o /home/ubuntu/mBrainAligner/results/user_sample_result/LSFM202108251702/ -d 70"

    cmd2 = "/home/ubuntu/mBrainAligner/dist/linux_bin/local_registration -p /home/ubuntu/mBrainAligner/examples/config/mouse_brain_initial_config.txt -s /home/ubuntu/mBrainAligner/results/user_sample_result/LSFM202108251702/global.v3draw  -l /home/ubuntu/mBrainAligner/examples/target/target_landmarks/low_landmarks.marker -g /home/ubuntu/mBrainAligner/examples/target/ -o /home/ubuntu/mBrainAligner/results/user_sample_result/LSFM202108251702/"

    cmd3 = "ls -la"
    s = subprocessLocal()
    res = s.execute(command=cmd3, timeout= 60* 60)
    print("res: ", res)

