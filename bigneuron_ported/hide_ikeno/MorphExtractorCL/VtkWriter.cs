using System;
using System.IO;
using System.Text;

using MorphExtractorCL;

namespace Sigen.IO.Vtk
{
    public class VtkWriter
    {
        StringBuilder sbmain, tmp, tmpsb;
        int nlines, nlpoint, ndata;
        bool flag;

        // 外部クラスからのコンストラクタの呼び出しを禁止する
        private VtkWriter() {
        }

        public static void SaveToVTKLegacy(string fname, Dendrite dendrite) {
            var writer = new VtkWriter();
            writer.Write(fname, dendrite);
        }

        private void Write(string fname, Dendrite dendrite) {
            sbmain = new StringBuilder();
            tmp = new StringBuilder();
            tmpsb = new StringBuilder();

            // VTKフォーマットの仕様書はこちら http://www.vtk.org/VTK/img/file-formats.pdf
            // このあたりの情報良さそう http://www.cacr.caltech.edu/~slombey/asci/vtk/vtk_formats.simple.html
            sbmain.AppendLine("# vtk DataFile Version 2.0"); // おまじない
            sbmain.AppendLine("Colored Dendrite");           // タイトル
            sbmain.AppendLine("ASCII");                      // テキストで記述する (VTKはバイナリでも記述できる)
            sbmain.AppendLine("DATASET POLYDATA");           // lineで記述したい

            // 点データの記述
            sbmain.AppendLine("POINTS " + dendrite.GetDendriteNodeNum().ToString() + " float");
            OutputVTKPointsRec(dendrite.root, null);

            // 線データの記述
            nlines = 0;   // 線の本数
            nlpoint = 0;  // 1本の線を構成する点の個数
            ndata = 0;    // 書き込むデータの個数
            flag = false; // 書き込みの途中であるか？

            OutputVTKLinesRec(dendrite.root, null);

            sbmain.AppendLine("LINES " + nlines.ToString() + " " + ndata.ToString());
            sbmain.Append(tmpsb.ToString());

            sbmain.AppendLine("POINT_DATA " + dendrite.GetDendriteNodeNum().ToString());

            sbmain.AppendLine("SCALARS distance float 1");
            sbmain.AppendLine("LOOKUP_TABLE default");
            OutputVTKDistanceRec(dendrite.root, null);

            sbmain.AppendLine("SCALARS radius float 1");
            sbmain.AppendLine("LOOKUP_TABLE radius");
            OutputVTKRadiusRec(dendrite.root, null);

            sbmain.AppendLine("SCALARS functionaldistance float 1");
            sbmain.AppendLine("LOOKUP_TABLE functionaldistance");
            OutputVTKFDistanceRec(dendrite.root, null);

            sbmain.AppendLine("SCALARS degree int 1");
            sbmain.AppendLine("LOOKUP_TABLE degree");
            OutputVTKDegreeRec(dendrite.root, null);

            using (StreamWriter sw = new StreamWriter(Path.ChangeExtension(fname, ".vtk")))
            {
                sw.Write(sbmain.ToString());
            }
        }

        private void OutputVTKPointsRec(DendriteNode node, DendriteNode parent)
        {

            sbmain.AppendLine(node.gx.ToString("G7") + " " + node.gy.ToString("G7") + " " + node.gz.ToString("G7"));

            foreach (DendriteNode next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    OutputVTKPointsRec(next, node);
                }

            }
        }

        private void OutputVTKLinesRec(DendriteNode node, DendriteNode parent)
        {
            // !flag && node.NodeType != DendriteNodeType.CONNECT がなりたつのは node == rootnode のときだけ？

            // edgeまたはbranchまできたら、いったん書き込む
            if (flag && node.NodeType != DendriteNodeType.CONNECT)
            {
                nlpoint++;
                ndata++;
                tmp.Append(node.Id.ToString());

                ndata++;
                tmp.Insert(0, nlpoint.ToString() + " ");

                tmpsb.AppendLine(tmp.ToString());

                flag = false;
                nlpoint = 0;
                nlines++;
            }
            else
            {
                // node is not rootnode
                if (parent != null)
                {
                    ndata++;
                    nlpoint++;
                    tmp.Append(node.Id.ToString() + " ");
                }
            }

            foreach (DendriteNode next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    // edgeまたはbranchだったら、そこから開始する
                    if (!flag && node.NodeType != DendriteNodeType.CONNECT)
                    {
                        flag = true;
                        tmp.Remove(0, tmp.Length);
                        tmp.Append(node.Id.ToString() + " ");
                        nlpoint++;
                        ndata++;
                    }
                    OutputVTKLinesRec(next, node);
                }

            }
        }

        private void OutputVTKRadiusRec(DendriteNode node, DendriteNode parent)
        {
            sbmain.AppendLine(node.Radius.ToString("G7"));

            foreach (DendriteNode next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    OutputVTKRadiusRec(next, node);
                }
            }
        }

        private void OutputVTKDistanceRec(DendriteNode node, DendriteNode parent)
        {
            sbmain.AppendLine(node.RealDistance.ToString("G7"));
            foreach (DendriteNode next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    OutputVTKDistanceRec(next, node);
                }
            }
        }

        private void OutputVTKFDistanceRec(DendriteNode node, DendriteNode parent)
        {
            sbmain.AppendLine(node.ElectricalDistance.ToString("G7"));
            foreach (DendriteNode next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    OutputVTKFDistanceRec(next, node);
                }
            }
        }

        //void OutputVTKColorRec(DendriteNode node, DendriteNode parent)
        //{
        //    sb.AppendLine(node.NodeColor.R.ToString() + " " + node.NodeColor.G.ToString() + " " + node.NodeColor.B.ToString());
        //    //sb.AppendLine(node.NodeColor.ToArgb().ToString());
        //    foreach (DendriteNode next in node.ConnectedNodes)
        //    {
        //        if (next != parent)
        //        {
        //            OutputVTKColorRec(next, node);
        //        }
        //    }
        //}

        private void OutputVTKDegreeRec(DendriteNode node, DendriteNode parent)
        {
            sbmain.AppendLine(node.Degree.ToString());
            foreach (DendriteNode next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    OutputVTKDegreeRec(next, node);
                }
            }
        }
    }
}

