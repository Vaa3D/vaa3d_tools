using System;
using System.Text;
using System.Diagnostics;
using System.Reflection;

using CommandLine;
using CommandLine.Text;

namespace MorphExtractorCL
{
    class IPParams
    {
        [Option("i", "input", Required = true, HelpText = "Input image directory")]
        public string ipath = "";

        [Option("o", "output", Required = false, HelpText = "Output filename\t\t\tDefault \"out.swc\"")]
        public string ofname = "out.swc";

        [Option("b", "binarize", Required = false, HelpText = "Binarization threshold [1-255]\tDefault 128")]
        public byte binarizeThreshold = 128;

        [Option("v", "volume", Required = false, HelpText = "Volume threshold (voxel)\t\tDefault 30 (voxel)")]
        public int volumeThreshold = 30;

        [Option("d", "distance", Required = false, HelpText = "Distance threshold (um)\t\tDefault 5 (um)")]
        public double distanceThreshold = 5.0;

        [Option("x", "resox", Required = false, HelpText = "Pixel resolution (um/pixel)\tDefault 1.0 (um/pixel)")]
        public double resolutionXY = 1.0;

        [Option("z", "resoz", Required = false, HelpText = "Interslice resorution (um/slice)\tDefault 1.0 (um/slice)")]
        public double resolutionZ = 1.0;

        [Option("s", "smooth", Required = false, HelpText = "Smoothing level\t\t\tDefault 0")]
        public int smoothingLevel = 0;

        [Option("a", "shave", Required = false, HelpText = "Beard removal level\t\tDefault 0")]
        public int shavingLevel = 0;

        [Option("t", "interporate", Required = false, HelpText = "Apply the interpolation\tDefault false")]
        public bool interporation = false;

        [Option(null, "output-sub", Required = false, HelpText = "Output sub dendrites\t\tDefault false")]
        public bool outputSub = false;

        [Option(null, "combine-sub", Required = false, HelpText = "Apply the combination of sub dendrites (require: --output-sub)\t\t\tDefault false")]
        public bool combinateSubDendrites = false;

        [Option(null, "output-vtk", Required = false, HelpText = "Output VTK\t\t\t\tDefault false")]
        public bool outputVTK = false;

        [Option("p", "suppress", Required = false, HelpText = "Suppress standard output")]
        public bool suppress = false;

        [Option("w", "white", Required = false, HelpText = "If the background color of the input image is white, add this option.")]
        public bool back = false;     //背景黒: false 背景白: true

        [Option("r", "revise", Required = false, HelpText = "Reconstruct swc using swc file revised by hand.")]
        public string revisedSwc = null;

        [HelpOption(HelpText = "Help")]
        public string GetUsage()
        {
            var versionInfo = FileVersionInfo.GetVersionInfo(Assembly.GetExecutingAssembly().Location);
            // AssemblyCopyrightAttribute asmcpy = (AssemblyCopyrightAttribute)Attribute.GetCustomAttribute(Assembly.GetExecutingAssembly(), typeof(AssemblyCopyrightAttribute));

            //ヘッダーの設定
            HeadingInfo head = new HeadingInfo(versionInfo.ProductName, "Version " + versionInfo.ProductVersion);
            HelpText help = new HelpText(head);
            help.Copyright = new CopyrightInfo("T. Minemoto", 2010);
            help.AddPreOptionsLine("University of Hyogo, Research Group of Fundamental Computer Engineering");
            help.AddPreOptionsLine("Usage: MorphExtractroCL.exe -i D:\\input -o out -b 123");

            //全オプションを表示(1行間隔)
            help.AdditionalNewLineAfterOption = false;
            help.AddOptions(this);

            return help.ToString();
        }

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendLine("input path            : " + ipath);
            sb.AppendLine("output file name      : " + ofname);
            sb.AppendLine("background color      : " + (back ? "White" : "Black"));
            sb.AppendLine("pixel resolution      : " + resolutionXY);
            sb.AppendLine("interslice resolution : " + resolutionZ);
            sb.AppendLine("interpolation         : " + interporation.ToString());
            sb.AppendLine("bin threshold         : " + binarizeThreshold.ToString());
            sb.AppendLine("voxel threshold       : " + volumeThreshold.ToString());
            sb.AppendLine("distance threshold    : " + distanceThreshold.ToString());
            sb.AppendLine("shaving level         : " + shavingLevel.ToString());
            sb.AppendLine("smoothing level       : " + smoothingLevel.ToString());
            sb.AppendLine("output sub dendrites  : " + outputSub.ToString());
            sb.AppendLine("combine sub dendrites : " + combinateSubDendrites.ToString());
            sb.AppendLine("output vtk file       : " + outputVTK.ToString());
            sb.AppendLine("revised swc file      : " + revisedSwc ?? "none");

            return sb.ToString();
        }
    }

    class ImageProcessRunner
    {
        IPParams param;
        ImageProcess ip;

        Stopwatch sw = new Stopwatch();

        public ImageProcessRunner(IPParams param)
        {
            this.param = param;
            ip = new ImageProcess(param);
        }

        public void Start()
        {
            Console.CursorVisible = false;

            sw.Start();
            this.Work();
            sw.Stop();

            Console.CursorVisible = true;
        }

        public void Work()
        {
            try
            {
                ShowMessage(param.ToString());

                //画像の読込み
                ShowMessageWithoutNewline("loading image from file... ");
                ip.LoadImageFromFile();
                ShowTimeConsumption();

                //孤立点の除去
                ShowMessageWithoutNewline("removing isolated points... ");
                ip.RemoveIsolatedPoint();
                ShowTimeConsumption();

                //ラベリング
                ShowMessageWithoutNewline("labeling and clustering voxels... ");
                ip.LabelingAndClustering();
                ShowTimeConsumption();

                if (param.revisedSwc != null) {
                    UseRevisedSwc();
                }
                else {
                    //主枝の抽出
                    ShowMessageWithoutNewline("extracting main dendrite... ");
                    ip.ExtractMainDendrite();
                    ShowTimeConsumption();

                    //補間するか否か
                    if (param.interporation)
                    {
                        //側枝の抽出
                        ShowMessageWithoutNewline("extracting sub dendrites... ");
                        ip.ExtractSubDendrites();
                        ShowTimeConsumption();

                        //側枝の接続
                        ShowMessageWithoutNewline("connecting sub dendrites to main dendrite... ");
                        ip.ConnectSubDendritesToMainDendrite();
                        ShowTimeConsumption();

                        // 側枝と側枝を接続するか？
                        if (param.outputSub && param.combinateSubDendrites)
                        {
                            ShowMessageWithoutNewline("connecting sub dendrites to other sub dendrites... ");
                            ip.ConnectSubDendritesToOtherSubDendrites();
                            ShowTimeConsumption();
                        }
                    }
                }

                //ひげ除去
                if (param.shavingLevel > 0)
                {
                    ShowMessageWithoutNewline("shaving dendrite... ");
                    ip.Shaving();
                    ShowTimeConsumption();
                }

                //平滑化
                if (param.smoothingLevel > 0)
                {
                    ShowMessageWithoutNewline("smoothing dendrite... ");
                    ip.Smoothing();
                    ShowTimeConsumption();
                }

                //出力
                ShowMessageWithoutNewline("Output dendrite... ");
                ip.OutputDendrite();
                ShowTimeConsumption();

                ShowMessageWithoutNewline("Finish!! ");
                ShowTimeConsumption();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                Environment.Exit(1);
            }
        }

        void UseRevisedSwc()
        {
            Debug.Assert(param.revisedSwc != null);

            //SWCの読み込み
            ShowMessageWithoutNewline("load from swc... ");
            var io = new DendriteIO();
            var swc = io.LoadFromSWC(param.revisedSwc);
            ShowTimeConsumption();

            //主枝の抽出
            ShowMessageWithoutNewline("extracting main dendrite... ");
            ip.ExtractMainDendrite();
            ShowTimeConsumption();

            //側枝の抽出
            ShowMessageWithoutNewline("extracting sub dendrites... ");
            ip.ExtractSubDendrites();
            ShowTimeConsumption();

            // swcファイルのノードとmain_dendrite, sub_dendriteを対応付ける
            ShowMessageWithoutNewline("relate swc node to images... ");
            ip.RelateSwcNode(swc);
            ShowTimeConsumption();

            //補間するか否か
            if (param.interporation)
            {
                //側枝の接続
                ShowMessageWithoutNewline("connecting sub dendrites to main dendrite... ");
                ip.ConnectSubDendritesToMainDendrite();
                ShowTimeConsumption();

                // 側枝と側枝を接続するか？
                if (param.outputSub && param.combinateSubDendrites)
                {
                    ShowMessageWithoutNewline("connecting sub dendrites to other sub dendrites... ");
                    ip.ConnectSubDendritesToOtherSubDendrites();
                    ShowTimeConsumption();
                }
            }
        }

        // suppressモードとnon-suppressモードの両方に対応するために、Consoleのラッパーを作りました
        void ShowMessage(string s)
        {
            if (param.suppress == false)
            {
                Console.WriteLine(s);
            }
        }

        void ShowMessageWithoutNewline(string s)
        {
            if (param.suppress == false)
            {
                Console.Write(s);
            }
        }

        void ShowTimeConsumption()
        {
            if (param.suppress == false)
            {
                Console.WriteLine("{0}:{1}:{2}", sw.Elapsed.Hours.ToString("00"), sw.Elapsed.Minutes.ToString("00"), sw.Elapsed.Seconds.ToString("00"));
            }
        }
    }
}
