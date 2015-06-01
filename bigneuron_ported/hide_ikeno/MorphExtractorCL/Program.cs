using System;
using CommandLine;

namespace MorphExtractorCL
{
    class Program
    {
        [STAThread] // おまじない
        static void Main(string[] args)
        {
            // コマンドライン引数の情報を入れる構造体
            var param = new IPParams();

            //コマンドライン引数の解析
            var setting = new CommandLineParserSettings(Console.Error);
            var paser = new CommandLineParser(setting);

            //パースに失敗した場合停止
            if (!paser.ParseArguments(args, param))
            {
                Environment.Exit(1);
            }

            var executor = new ImageProcessRunner(param);
            executor.Start();
        }
    }
}
