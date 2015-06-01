using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;

namespace Sigen.IO.Swc
{
    public class SwcParser
    {
        private static string[] delimiters = new string[] { " " };
        private static string[] commentTokens = new string[] { "#" };

        // 読み込んだファイルに含まれるコメント行 (コメントトークンも含まれる)
        private IList<string> comments;

        // コメント行以外の行をデリミタで区切ったもの
        private IList<string[]> contents;

        public SwcParser(IEnumerable<string> fileContent)
        {
            comments = new List<string>();
            contents = new List<string[]>();

            foreach (var line in fileContent)
            {
                // コメントトークンで始まっていたら
                if (commentTokens.Any(token => line.StartsWith(token)))
                {
                    // その行をcommentsに追加 (コメントトークンも含まれる)
                    comments.Add(line);
                }
                else
                {
                    string[] record = line.Split(delimiters, StringSplitOptions.RemoveEmptyEntries);
                    if (record.Length > 0)
                    {
                        System.Diagnostics.Debug.Assert(record.Length == 7, "# of columns is 7");
                        contents.Add(record);
                    }
                }
            }
        }

        public IEnumerable<string> Comments
        {
            get { return comments; }
        }

        public IEnumerable<string[]> Contents
        {
            get { return contents; }
        }

        public IEnumerable<SwcNode> GetSwcNode()
        {
            foreach (var record in contents)
            {
                int id, type, parent;
                double x, y, z, r;

                try
                {
                    id = int.Parse(record[0]);
                    type = int.Parse(record[1]);
                    x = double.Parse(record[2]);
                    y = double.Parse(record[3]);
                    z = double.Parse(record[4]);
                    r = double.Parse(record[5]);
                    parent = int.Parse(record[6]);
                }
                catch (Exception)
                {
                    throw new Exception("Error occurred (reading swc)");
                }

                yield return new SwcNode(id, type, x, y, z, r, parent);
            }
        }
    }

    public class SwcNode
    {
        public int Id { get; private set; }

        public int NodeType { get; private set; }

        public double X { get; private set; }

        public double Y { get; private set; }

        public double Z { get; private set; }

        public double Radius { get; private set; }

        public int ParentNodeId { get; private set; }

        public SwcNode(int id, int type, double x, double y, double z, double radius, int parentNodeId)
        {
            this.Id = id;
            this.NodeType = type;
            this.X = x;
            this.Y = y;
            this.Z = z;
            this.Radius = radius;
            this.ParentNodeId = parentNodeId;
        }

        public void SetNodeTypeAsSoma()
        {
            this.NodeType = 1;
        }

        public void SetNodeTypeAsDendrite()
        {
            this.NodeType = 3;
        }

        public IEnumerable<SwcNode> CreateSomaPartners()
        {
            var somaNode = this;
            System.Diagnostics.Debug.Assert(somaNode.Id == 1);
            double rs = somaNode.Radius;
            SwcNode[] partners = new SwcNode[2];
            partners[0] = new SwcNode(2, 1, somaNode.X, somaNode.Y - rs, somaNode.Z, rs, 1);
            partners[1] = new SwcNode(3, 1, somaNode.X, somaNode.Y + rs, somaNode.Z, rs, 1);
            return partners;
        }

        public void IncreaseParentNodeIdIfParentIsNotRoot()
        {
            if (ParentNodeId != -1 && ParentNodeId != 1)
            {
                ParentNodeId += 2;
            }
        }

        public void IncreaseId()
        {
            Id += 2;
        }
    }
}
