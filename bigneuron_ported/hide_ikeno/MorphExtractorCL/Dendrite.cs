using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;

namespace MorphExtractorCL
{
    // IOとアルゴリズムの中間を取り持つだけの役割にして、
    // もう少し薄くしたい (アルゴリズム部を委譲したい)
    public class Dendrite
    {
        public DendriteNode root;
        public List<List<DendriteNode>> nodelist;       //ノードリストの一時保存用

        private double resox, resoz, volunit;

        // ================================================
        // SetNodeTypeの中からしか変更しないことを徹底すること！！
        public int edge { get; private set; }
        public int branch { get; private set; }
        // ================================================

        // ==========================================
        // SetIdの中からしか変更しないことを徹底すること！！
        private int idsum;
        // ==========================================

        // ===============================================
        // SetDistanceの中からしか変更しないことを徹底すること！！
        public double TotalLength { get; private set; }
        public double TotalElectricalLength { get; private set; }

        public double MaxRealDistance { get; private set; }
        public double MaxElectricalDistance { get; private set; }

        public double XMax { get; private set; }
        public double YMax { get; private set; }
        public double ZMax { get; private set; }
        public double XMin { get; private set; }
        public double YMin { get; private set; }
        public double ZMin { get; private set; }
        // ===============================================

        public int MaxDegree { get; private set; }

        public Dendrite(int maxdist, double rx, double rz)
        {
            //インスタンス生成
            nodelist = new List<List<DendriteNode>>();

            //解像度
            resox = rx;
            resoz = rz;

            //単位体積
            volunit = resox * resox * resoz;

            //距離値ごとのノードリストを作成
            for (int z = 0; z <= maxdist; z++) nodelist.Add(new List<DendriteNode>());
        }

        //ノードの追加 distance:距離値 , cluster:同一距離値を持つ点座標クラスタ
        public void AddNode(int distance, IEnumerable<Point3Di> cluster)
        {
            nodelist[distance].Add(new DendriteNode(distance, cluster));
        }

        //ノード間連結性チェッカ
        private static bool NodeConnectionCheck(DendriteNode a, DendriteNode b)
        {
            foreach (Point3Di pta in a.voxels)
            {
                foreach (Point3Di ptb in b.voxels)
                {
                    int dx = ptb.X - pta.X;
                    int dy = ptb.Y - pta.Y;
                    int dz = ptb.Z - pta.Z;
                    if (Math.Abs(dx) <= 1 && Math.Abs(dy) <= 1 && Math.Abs(dz) <= 1)
                        return true;
                }
            }

            return false;
        }

        //ノード連結関係を調べて木構造を生成
        public void CreateNodeTree()
        {
            //ルートは距離値1のノード (SSDT開始点)
            root = nodelist[0][0];

            for (int z = 0; z < nodelist.Count - 1; z++)
            {
                List<DendriteNode> nodes1 = nodelist[z];
                List<DendriteNode> nodes2 = nodelist[z + 1];

                foreach (DendriteNode parent in nodes1)
                {
                    foreach (DendriteNode child in nodes2)
                    {
                        // 未連結であるか？
                        if (child.isConnected == false)
                        {
                            // clusterの連結性チェック
                            bool ischild = NodeConnectionCheck(parent, child);

                            if (ischild)
                            {
                                // 相互連結
                                parent.AddConnectedNode(child);
                                child.AddConnectedNode(parent);

                                // 既に連結済みである
                                child.isConnected = true;
                            }
                        }
                    }
                }
            }

            nodelist.Clear();

            SetGravityPointAndRadius();

            SetNodeType(); // GetEdgeNodesする前に呼ばないとバグる
            var edgenodes = GetEdgeNodes();
            // たぶん意味は無いと思うが、rootを変更しても問題になることは無いだろう的な
            ChangeRootNode(edgenodes.First());
        }

        public void Create()
        {
            ChangeRootNode(root);
        }

        //ルートノードの変更
        public void ChangeRootNode(DendriteNode node)
        {
            root = node;

            SetId();
            SetNodeType();
            SetDistance();
            SetParentNodeAsFirstElement();

            this.MaxDegree = SetDegree(root, null) + 1;
        }

        private void SetParentNodeAsFirstElement() {
            foreach (var node in root.ConnectedNodes)
            {
                SetParentNodeAsFirstElementRec(node, root);
            }
        }

        private void SetParentNodeAsFirstElementRec(DendriteNode node, DendriteNode parent) {
            node.ConnectedNodes.Remove(parent);
            node.ConnectedNodes.Insert(0, parent);
            foreach (var next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    SetParentNodeAsFirstElementRec(next, node);
                }
            }
        }

        #region set id
        private void SetId()
        {
            idsum = 0;
            SetIdRec(root, null);
        }

        private void SetIdRec(DendriteNode node, DendriteNode parent)
        {
            //分類番号の設定
            node.Id = idsum;
            idsum++;

            foreach (var next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    SetIdRec(next, node);
                }
            }
        }
        #endregion

        #region set node type
        private void SetNodeType()
        {
            edge = 0;
            branch = 0;

            SetNodeTypeRec(root, null);
        }

        private void SetNodeTypeRec(DendriteNode node, DendriteNode parent)
        {
            //連結しているノードの数でノードのタイプを判定
            if (node.GetConnectedNodeNum() == 1) // 他の一つのクラスターにしか繋がっていないクラスターはEdgeクラスターである
            {
                node.NodeType = DendriteNodeType.EDGE;
                edge++;
            }
            else if (node.GetConnectedNodeNum() >= 3) // 他の3つのクラスターに繋がっているクラスターはBranchクラスターである
            {
                node.NodeType = DendriteNodeType.BRANCH;
                branch++;
            }
            else
            {
                node.NodeType = DendriteNodeType.CONNECT; // 他の2つのクラスターに繋がっているクラスターはEdgeクラスターでもBranchクラスターでもない
            }

            foreach (var child in node.ConnectedNodes)
            {
                if (child != parent)
                {
                    SetNodeTypeRec(child, node);
                }
            }
        }
        #endregion

        #region set gravity point
        // この関数は、MorphExtractorからしか呼んではならない (swc -> swcで使うとバグる。なぜならclusterを保存していないから)
        // すなわち、このメソッドがDendriteに書かれている時点で何かがおかしい
        private void SetGravityPointAndRadius()
        {
            SetGravityPointAndRadiusRec(root, null);
        }

        private void SetGravityPointAndRadiusRec(DendriteNode node, DendriteNode parent)
        {
            //重心計算
            node.SetGravityPointByClusters(resox, resoz);

            //突起半径計算
            node.CalcRadius(volunit);

            foreach (DendriteNode next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    SetGravityPointAndRadiusRec(next, node);
                }
            }
        }
        #endregion // set gravity point

        #region set distance
        private void SetDistance()
        {
            root.RealDistance = 0.0;
            root.ElectricalDistance = 0.0;
            TotalLength = 0.0;
            TotalElectricalLength = 0.0;
            MaxRealDistance = 0.0;
            MaxElectricalDistance = 0.0;

            XMax = root.gx;
            XMin = root.gx;
            YMax = root.gy;
            YMin = root.gy;
            ZMax = root.gz;
            XMin = root.gz;

            foreach (var next in root.ConnectedNodes)
            {
                // さすがに if (next != null) は不要だろう
                SetDistanceRec(next, root);
            }
        }

        //ノード情報から樹状突起の実際の長さと電気長などを計算
        private void SetDistanceRec(DendriteNode node, DendriteNode parent)
        {
            XMax = Math.Max(XMax, node.gx);
            XMin = Math.Min(XMin, node.gx);
            YMax = Math.Max(YMax, node.gy);
            YMin = Math.Min(YMin, node.gy);
            ZMax = Math.Max(ZMax, node.gz);
            ZMin = Math.Min(ZMin, node.gz);

            //親ノードと子ノード間のユークリッド距離
            double d = node.EuclideanDistanceTo(parent);

            node.RealDistance = parent.RealDistance + d;

            //二点のノードの平均半径
            double aveRadius = (node.Radius + parent.Radius) / 2;

            //電気的距離 R = ρ*L/(pi*r_ave^2) 定数部分は除去して R = L/r_ave^2
            node.ElectricalDistance = parent.ElectricalDistance + d / (aveRadius);

            MaxRealDistance = Math.Max(node.RealDistance, MaxRealDistance);
            MaxElectricalDistance = Math.Max(node.ElectricalDistance, MaxElectricalDistance);

            TotalLength += d;
            TotalElectricalLength += d / aveRadius;

            foreach (var next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    SetDistanceRec(next, node);
                }
            }
        }
        #endregion // set distance

        #region set 
        // 呼び出す前に、SetNodeType()しておくこと
        private int SetDegree(DendriteNode node, DendriteNode parent)
        {
            // node is not root
            if (node.NodeType == DendriteNodeType.EDGE && parent != null)
            {
                node.Degree = 1;
                return node.Degree;
            }

            IList<int> vals = new List<int>();
            foreach (DendriteNode next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    int deg = SetDegree(next, node);
                    vals.Add(deg);
                }
            }

            if (node.NodeType == DendriteNodeType.CONNECT || parent == null)
            {
                // サイズが0のswcを食わせると、ここで落ちることが判明したので
                // ad-hocな処理だけどとりあえず修復
                if (vals.Count > 0)
                {
                    node.Degree = vals[0];
                }
                else
                {
                    node.Degree = 1;
                }
                return node.Degree;
            }
            else // NodeType = BRANCH
            {
                // このdegreeの定義はよく分からない
                int max = vals.Max();
                if (vals.Sum() == max * vals.Count) node.Degree = max + 1;
                else node.Degree = max;
                return node.Degree;
            }
        }
        #endregion // set degree

        #region clipping (shaving)
        /// <summary>
        /// ひげの除去を行う
        /// 
        /// ひげとは、ブランチノードからshaveinglevelブロック以内に存在するエッジノードのことである
        /// </summary>
        public void Shaving(int shavingLevel)
        {
            SetNodeType();

            var branchnodes = GetBranchNodes();

            foreach (var branch in branchnodes)
            {
                // ↓この条件は本当に必要？？
                if (branch != root)
                {
                    //端点からwhiskerlevelのステップで分岐点に到達可能かのチェック
                    // foreachを使うと動かない (ShavingRecの中でConnectedNodesが削除されるから
                    //
                    // removeされたものが1つでもあったらi--しないといけないかも？？
                    for (int i = 0; i < branch.GetConnectedNodeNum(); i++)
                    {
                        var next = branch.ConnectedNodes[i];
                        ShavingNodeChecker(next, branch, shavingLevel);
                    }
                }
            }

            //ノード情報の更新
            ChangeRootNode(root);
        }

        private bool ShavingNodeChecker(DendriteNode node, DendriteNode parent, int level)
        {
            if (level > 0)
            {
                if (node.NodeType == DendriteNodeType.EDGE && node != root)
                {
                    // 端点が見つかればそこからノードを削除
                    // ConnectedNodes[0]はnodeのparent？？？ ← その通り
                    // 基本的にはそうだが、補間によって接続されると、そうとも限らないかもしれない
                    // ChangeRootNodeしたときにうまく動いているかもしれないけど
                    ShavingRec(node.ConnectedNodes[0], node);
                    return true;
                }
                else if (node.NodeType == DendriteNodeType.CONNECT)
                {
                    // 通過点の場合 探索続行
                    // foreachを使うと動かない (ShavingRecの中でConnectedNodesが削除されるから
                    // 
                    // removeされたものが1つでもあったらi--しないといけないかも？？
                    for (int i = 0; i < node.GetConnectedNodeNum(); i++)
                    {
                        var next = node.ConnectedNodes[i];
                        if (next != parent)
                        {
                            if (ShavingNodeChecker(next, node, level - 1))
                            {
                                i--;
                            }
                        }
                    }
                }
            }

            return false;
        }

        // whiskerlevelの長さをもつひげを除去
        private void ShavingRec(DendriteNode master, DendriteNode slave)
        {
            master.ConnectedNodes.Remove(slave);

            if (master.NodeType != DendriteNodeType.BRANCH)
            {
                // ConnectedNodes[0]はnodeのparent
                ShavingRec(master.ConnectedNodes[0], master);
            }
        }

        public void YetAnotherClippingMethod(int clippingLevel) {
            SetNodeType();

            // どうせChangeRootNodeしたときにDegreeを書き換えるので
            // Degreeを利用させてもらう (distanceはprivate setだった)

            Action<DendriteNode, DendriteNode> setDegreeZeroFunc = null;
            setDegreeZeroFunc = (node, parent) =>
            {
                node.Degree = 0;
                foreach (var next in node.ConnectedNodes) {
                    if (next != parent) {
                        setDegreeZeroFunc(next, node);
                    }
                }
            };
            setDegreeZeroFunc(root, null);

            root.Degree = int.MaxValue;

            foreach (var edgenode in GetEdgeNodes())
            {
                int d = 1;
                for (DendriteNode node = edgenode; node != root; node = node.ConnectedNodes[0])
                {
                    if (node.Degree < d)
                    {
                        node.Degree = d;
                        d++;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            foreach (var branchnode in GetBranchNodes())
            {
                bool isShortAll = true;
                for (int i = 1; i < branchnode.ConnectedNodes.Count; i++)
                {
                    if (branchnode.ConnectedNodes[i].Degree > clippingLevel)
                    {
                        isShortAll = false;
                    }
                }

                if (isShortAll)
                {
                    int maxDegree = 0;
                    for (int i = 1; i < branchnode.ConnectedNodes.Count; i++)
                    {
                        maxDegree = Math.Max(maxDegree, branchnode.ConnectedNodes[i].Degree);
                    }

                    bool isSkipped = false;
                    for (int i = 1; i < branchnode.ConnectedNodes.Count; i++)
                    {
                        if (branchnode.ConnectedNodes[i].Degree == maxDegree && isSkipped == false)
                        {
                            isSkipped = true;
                            continue;
                        }

                        if (branchnode.ConnectedNodes[i].Degree <= clippingLevel)
                        {
                            branchnode.ConnectedNodes.RemoveAt(i);
                            i--;
                        }
                    }
                }
                else
                {
                    for (int i = 1; i < branchnode.ConnectedNodes.Count; i++)
                    {
                        if (branchnode.ConnectedNodes[i].Degree <= clippingLevel)
                        {
                            branchnode.ConnectedNodes.RemoveAt(i);
                            i--;
                        }
                    }
                }
            }

            ChangeRootNode(root);
        }

        #endregion // clipping (shaving)

        #region smoothing

        public void Smoothing(int smoothingLevel)
        {
            for (int x = 0; x < smoothingLevel; x++)
            {
                SmoothGravityLine(root, null);
                SmoothRadius(root, null);
                RenewValues(root, null);
            }

            ChangeRootNode(root);
        }

        //重心座標の平滑化
        private static void SmoothGravityLine(DendriteNode node, DendriteNode parent)
        {
            //接続ノードの重心座標の平均をとる
            node.tmpx = node.gx + node.ConnectedNodes.Sum(cobj => cobj.gx);
            node.tmpy = node.gy + node.ConnectedNodes.Sum(cobj => cobj.gy);
            node.tmpz = node.gz + node.ConnectedNodes.Sum(cobj => cobj.gz);

            int count = node.GetConnectedNodeNum() + 1;

            node.tmpx /= count;
            node.tmpy /= count;
            node.tmpz /= count;

            /*
            else if(node.nodetype == DendriteNodeType.BRANCH)
            {
                for(int i=0;i<node.GetConnectedNodeNum();i++)
                {
                    if(node.cnodes[i] != parent)
                    {
                        double tx,ty,tz;

                        tx = parent.gx+node.gx+node.cnodes[i].gx;
                        ty = parent.gy+node.gy+node.cnodes[i].gy;
                        tz = parent.gz+node.gz+node.cnodes[i].gz;

                        node.gx = tx/3.0;
                        node.gy = ty/3.0;
                        node.gz = tz/3.0;
                    }
                }
            }
            */
            //node.CalcRadius0(resox,resoz);
            //node.CalcRadius(this.volunit);

            foreach (var next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    SmoothGravityLine(next, node);
                }
            }
        }

        //径の平滑化
        private static void SmoothRadius(DendriteNode node, DendriteNode parent)
        {
            node.tmpr = node.Radius + node.ConnectedNodes.Sum(cobj => cobj.Radius);

            int count = node.GetConnectedNodeNum() + 1;
            node.tmpr /= count;

            foreach (var next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    SmoothRadius(next, node);
                }
            }
        }

        //平滑後の値の更新
        private static void RenewValues(DendriteNode node, DendriteNode parent)
        {
            node.gx = node.tmpx;
            node.gy = node.tmpy;
            node.gz = node.tmpz;
            node.Radius = node.tmpr;

            foreach (var next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    RenewValues(next, node);
                }
            }
        }

        #endregion // smoothing

        #region pruning
        public void Pruning(int nodeId) {
            PruningRec(root, null, nodeId);
            ChangeRootNode(root);
        }

        private void PruningRec(DendriteNode node, DendriteNode parent, int nodeId) {
            if (node.Id == nodeId)
            {
                node.ConnectedNodes.Clear();
                node.AddConnectedNode(parent);
                return;
            }

            foreach (var next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    PruningRec(next, node, nodeId);
                }
            }
        }
        #endregion

        #region Node getter
        /// <summary>
        /// [Pure] rootから辿ることのできる全てのノードをListに詰め込んで返す
        /// </summary>
        public List<DendriteNode> GetDendriteNodes()
        {
            var list = new List<DendriteNode>();

            GetDendriteNodesRec(list, root, null);
            return list;
        }

        private void GetDendriteNodesRec(IList<DendriteNode> list, DendriteNode node, DendriteNode parent)
        {
            list.Add(node);
            foreach (var next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    GetDendriteNodesRec(list, next, node);
                }
            }
        }

        public List<DendriteNode> GetBranchNodes()
        {
            var list = new List<DendriteNode>();
            GetBranchNodesRec(list, root, null);
            return list;
        }

        private static void GetBranchNodesRec(IList<DendriteNode> list, DendriteNode node, DendriteNode parent)
        {
            if (node.NodeType == DendriteNodeType.BRANCH) list.Add(node);

            foreach (var next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    GetBranchNodesRec(list, next, node);
                }
            }
        }

        // IEnumerableに変えてみようとしたけどうまくいかなかった
        public List<DendriteNode> GetEdgeNodes()
        {
            var list = new List<DendriteNode>();
            GetEdgeNodesRec(list, root, null);
            return list;
        }

        private static void GetEdgeNodesRec(IList<DendriteNode> list, DendriteNode node, DendriteNode parent)
        {
            if (node.NodeType == DendriteNodeType.EDGE)
            {
                list.Add(node);
            }

            foreach (var next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    GetEdgeNodesRec(list, next, node);
                }
            }
        }

        public List<DendriteNode> GetEdgeNodeAndBranchNodes()
        {
            var list = new List<DendriteNode>();
            GetEdgeAndBranchNodesRec(list, root, null);
            return list;
        }

        private void GetEdgeAndBranchNodesRec(IList<DendriteNode> list, DendriteNode node, DendriteNode parent)
        {
            if (node.NodeType == DendriteNodeType.EDGE || node.NodeType == DendriteNodeType.BRANCH) list.Add(node);

            foreach (var next in node.ConnectedNodes)
            {
                if (next != parent)
                {
                    GetEdgeAndBranchNodesRec(list, next, node);
                }
            }
        }

        #endregion

        #region 統計量getter

        /// <summary>
        /// [Pure] dendriteを構成するDendriteNodeの数(クラスターの数とも言う)
        /// 
        /// Create()やSetId()などでDendriteNodeにIdが付けられていることが条件
        /// </summary>
        public int GetDendriteNodeNum() // GetConnectedNodeNumにちなんで名前をつけた
        {
            return idsum;
        }

        /// <summary>
        /// [Pure] edgeノードの個数を返す
        /// </summary>
        public int GetEdgeNodeNum()
        {
            return edge;
        }

        /// <summary>
        /// [Pure] このDendriteの簡易的な統計量を出力する
        /// </summary>
        public string GetInfo()
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendLine("Num of nodes:  " + idsum.ToString());
            sb.AppendLine("Num of end nodes:  " + edge.ToString());
            sb.AppendLine("Num of branching nodes:  " + branch.ToString());
            sb.AppendLine("Max real distance:  " + MaxRealDistance.ToString());
            sb.AppendLine("Max electrical distance:  " + MaxElectricalDistance.ToString());
            sb.AppendLine("Total real distance:  " + TotalLength.ToString());
            sb.AppendLine("Total electrical distance:  " + TotalElectricalLength.ToString());

            return sb.ToString();
        }

        #endregion // 統計量getter

        private SwcHeaderStore swcHeader = new SwcHeaderStore();

        //SwcHeaderはmutableなので、getterだけあってもあまり意味が無い
        public SwcHeaderStore SwcHeader
        {
            get { return swcHeader; }
        }
    }
}
