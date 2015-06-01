using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Sigen.IO.Swc;

namespace MorphExtractorCL
{
    /// <summary>
    /// ノードの種類
    /// </summary>
    public enum DendriteNodeType
    {
        //EDGE:端点，BRANCH:分岐点，CONNECT:左以外の連結点
        EDGE = 2,
        BRANCH = 1,
        CONNECT = 0,
        // 使用されていなかったので削除
        // UNDEFINED = 3,
        // SOMA = 4,
    }

    /// <summary>
    /// 樹状突起ノードクラス (同じ距離のノード郡 (山崎氏の論文(2006)ではclusterと呼ばれているもの))
    /// </summary>
    public class DendriteNode
    {
        // ボクセル座標リスト
        public List<Point3Di> voxels { get; private set; }

        // 連結ノードリスト
        public List<DendriteNode> ConnectedNodes { get; private set; }

        // ノードタイプ
        public DendriteNodeType NodeType { get; set; }

        // SSDT距離値
        public int Distance { get; private set; }

        // 分類番号
        public int Id { get; set; }

        // 重心点座標
        public double gx, gy, gz;

        // 連結判定子
        public bool isConnected;

        public double RealDistance { get; set; }

        public double ElectricalDistance { get; set; }

        public double Radius { get; set; }

        // 次数 (根っこ用 Pregitzer et al. 2002)
        public int Degree { get; set; }

        public double volume;

        // 計算用一時変数
        public double tmpx, tmpy, tmpz, tmpr;

        public DendriteNode(int distance, IEnumerable<Point3Di> cluster)
        {
            voxels = new List<Point3Di>();
            voxels.AddRange(cluster);

            ConnectedNodes = new List<DendriteNode>();

            Distance = distance;
            NodeType = DendriteNodeType.CONNECT;
            isConnected = false;
        }

        // DendriteIO.ParseSWC() から利用されている
        public DendriteNode(int id, double x, double y, double z, double radius, int parentId)
        {
            voxels = new List<Point3Di>();
            ConnectedNodes = new List<DendriteNode>();

            isConnected = false;

            Id = id;
            gx = x;
            gy = y;
            gz = z;
            Radius = radius;

            // 親ノードidとして利用
            this.Distance = parentId;
        }

        // typeは無視
        public DendriteNode(SwcNode swcNode) : this(swcNode.Id, swcNode.X, swcNode.Y, swcNode.Z, swcNode.Radius, swcNode.ParentNodeId)
        {

        }

        // どこからも参照されていない
        /*
        private void AddRange(IEnumerable<Point3Di> cluster)
        {
            voxels.AddRange(cluster);
        }
        */
        public void AddConnectedNode(DendriteNode node)
        {
            ConnectedNodes.Add(node);
        }

        public void SetGravityPointByClusters(double resox, double resoz)
        {
            if (voxels.Count == 0)
            {
                // voxels.Count == 0の時点でそもそも何かおかしい気がする
                gx = 0.0;
                gy = 0.0;
                gz = 0.0;
            }
            else
            {
                gx = voxels.Average(v => v.X) * resox;
                gy = voxels.Average(v => v.Y) * resox;
                gz = voxels.Average(v => v.Z) * resoz;
            }
        }

        public void CalcRadius(double unit)
        {
            // unitはボクセルあたりの体積
            // この計算は正しくないような気がする…
            //
            // S = pi * r^2
            // S := voxels.Count * unit
            // pi := Math.PI
            // r = this.Radius
            this.Radius = Math.Sqrt(voxels.Count() * unit / Math.PI);
        }

        // どこからも参照されていない
        /* 
        void CalcRadius2(double resox, double resoz)
        {
            Point3Dd g = new Point3Dd(gx, gy, gz);
            double max = voxels.Max(point =>
                g.EuclideanDistanceTo(new Point3Dd(point.X * resox, point.Y * resox, point.Z * resoz))
            );

            Radius = max;
        }
        */

        public int GetConnectedNodeNum()
        {
            return ConnectedNodes.Count;
        }

        // どこからも参照されていない
        /*
        int GetConnectedNodeNum(DendriteNode parent)
        {
            int n = ConnectedNodes.Count(node => node != parent);

            return n;
        }
        */

        public double EuclideanDistanceTo(DendriteNode other)
        {
            return this.EuclideanDistanceTo(other.CenterPoint());
        }

        public double EuclideanDistanceTo(Point3Dd other) {
            return this.CenterPoint().EuclideanDistanceTo(other);
        }

        public Point3Dd CenterPoint() {
            return new Point3Dd(this.gx, this.gy, this.gz);
        }
    }
}
