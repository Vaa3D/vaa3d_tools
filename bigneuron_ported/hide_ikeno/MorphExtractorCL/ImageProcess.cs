using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;

using Sigen.IO.Vtk;
using System.Diagnostics;

namespace MorphExtractorCL
{
    class ImageProcess
    {
        ImageData imageData;

        // label -> [Point3DExd] := そのラベルが付けられたノードのリストを返す
        Dictionary<UInt32, List<Point3DExd>> clusters_e;

        Dendrite main_dendrite;
        List<Dendrite> sub_dendrites;
        List<int> sub_dendrites_volume;

        // main_dendriteにマージされていないsub_dendritesのインデックス
        HashSet<int> NotMergedSubDendritesIdx;

        IPParams param;

        public ImageProcess(IPParams param)
        {
            imageData = new ImageData();
            this.param = param;
        }

        /// <summary>
        /// 連続画像を読込み、ImageData#voxelに閾値によって二値化した3次元bool配列を書き込む
        /// </summary>
        /// <param name="param">パラメータ</param>
        public void LoadImageFromFile()
        {
            imageData.LoadImageFromFile(param);
        }

        /// <summary>
        /// 孤立点（三次元画像の26近傍がすべて背景色である点）の除去
        /// </summary>
        public void RemoveIsolatedPoint()
        {
            imageData.ClearFrame();
            imageData.RemoveIsolatedPoint();
        }

        /// <summary>
        /// 画素を座標データに変換→連結成分クラスタリング
        /// </summary>
        public void LabelingAndClustering()
        {
            var points = new Dictionary<string, Point3DExd>();

            //ボクセルオブジェクトの連想記憶リストを生成
            // 縁は無視する
            for (int z = 1; z < imageData.ZSize - 1; z++)
            {
                for (int y = 1; y < imageData.YSize - 1; y++)
                {
                    for (int x = 1; x < imageData.XSize - 1; x++)
                    {
                        if (imageData.voxel[z, y, x])
                        {
                            string key = GenKey(x, y, z);
                            points.Add(key, new Point3DExd(x, y, z));
                        }
                    }
                }
            }

            //連結オブジェクトチェック
            foreach (var p in points.Values)
            {
                //周囲26近傍に存在するオブジェクトを連結オブジェクトとして追加
                for (int k = -1; k <= 1; k++)
                {
                    for (int j = -1; j <= 1; j++)
                    {
                        for (int i = -1; i <= 1; i++)
                        {
                            if (k == 0 && j == 0 && i == 0) continue;

                            if (imageData.voxel[p.Z + k, p.Y + j, p.X + i])
                            {
                                string nkey = GenKey(p.X + i, p.Y + j, p.Z + k);
                                p.AddConnectedObj(points[nkey]);
                            }
                        }
                    }
                }
            }

            foreach (var p in points.Values)
            {
                p.ResetFlag();
            }

            //連結成分のラベリング
            uint label = 0;

            foreach (var p in points.Values)
            {
                if (p.flag == false)
                {
                    p.Label = label++;
                    LabelConnectingObj(p);
                }
            }

            //ラベル値でクラスタリング
            clusters_e = new Dictionary<uint, List<Point3DExd>>();
            for (uint i = 0; i < label; i++)
            {
                clusters_e.Add(i, new List<Point3DExd>());
            }

            foreach (var p in points.Values)
            {
                clusters_e[p.Label].Add(p);
            }

            //体積降順にソート
            List<List<Point3DExd>> sorted = new List<List<Point3DExd>>();

            //Dictionary要素をListに
            foreach (var item in clusters_e.Values)
            {
                sorted.Add(item);
            }

            //降順ソート
            sorted.Sort((a, b) => (b.Count - a.Count));

            //ラベルを書き換えてDictionaryに改めて追加
            clusters_e.Clear();

            for (uint i = 0; i < sorted.Count; i++)
            {
                foreach (Point3DExd p in sorted[(int)i])
                {
                    p.Label = i;
                }

                clusters_e.Add(i, sorted[(int)i]);
            }
        }

        private string GenKey(int x, int y, int z)
        {
            return "x" + x.ToString("X") + "y" + y.ToString("X") + "z" + z.ToString("X");
        }

        private void LabelConnectingObj(Point3DExd initialNode)
        {
            var stack = new Stack<Point3DExd>();
            stack.Push(initialNode);

            while (stack.Count > 0)
            {
                var currentNode = stack.Pop();
                foreach (var next in currentNode.cobj)
                {
                    if (next.flag == false)
                    {
                        next.Label = currentNode.Label; // initialNode.Labelでも可
                        stack.Push(next);
                    }
                }
            }
        }

        /// <summary>
        /// 主連結成分(体積最大の連結成分)の構造抽出
        /// </summary>
        public void ExtractMainDendrite()
        {
            // SSDT

            //処理対象のクラスタ (体積が最大のもの)
            List<Point3DExd> scluster = clusters_e[0];

            //距離変換
            // Point3DExd initial = scluster[0];  //距離変換開始点　初めは適当に選択

            // BFSを2回行うことで、最も距離の離れた2点間の距離を得る
            uint maxdistance = SSDTLoopEco(scluster);

            // 木構造生成
            main_dendrite = new Dendrite((int)maxdistance + 1, param.resolutionXY, param.resolutionZ);

            foreach (var tmp in scluster)
            {
                if (!tmp.flag)
                {
                    //同一距離値クラスタ
                    var cld = new List<Point3Di>();

                    //距離値でクラスタリング
                    RecursiveDistanceClusteringEco(tmp, cld);

                    main_dendrite.AddNode((int)tmp.Distance, cld);
                }
            }

            //木構造生成
            main_dendrite.CreateNodeTree();
        }

        void SSDT(Point3DExd node)
        {
            foreach (var next in node.cobj)
            {
                if (next.Distance == node.Distance && !next.flag)
                {
                    // flagをtrueにしているだけ？
                    next.Distance = node.Distance;
                    SSDT(next);
                }
            }
        }

        uint SSDTLoopEco(IList<Point3DExd> scluster, Point3Dd rootPoint = null)
        {
            if (rootPoint == null)
            {
                // use double sweep algorithm.
                Point3DExd p = scluster[0];
                uint maxDistance = 0; // とりあえず何かセットしておかないとunassigned variableと言われてしまうので

                for (int i = 0; i < 2; i++)
                {
                    foreach (var q in scluster)
                    {
                        q.ResetFlag();
                    }

                    var que = new Queue<Point3DExd>();

                    p.SetFlag();
                    p.Distance = 0;
                    que.Enqueue(p);

                    maxDistance = 0;

                    while (que.Count > 0)
                    {
                        var q = que.Dequeue();
                        foreach (var next in q.cobj)
                        {
                            if (next.flag == false)
                            {
                                next.SetFlag();
                                next.Distance = q.Distance + 1;
                                que.Enqueue(next);

                                maxDistance = Math.Max(maxDistance, next.Distance);
                                // set final node to p
                                p = next;
                            }
                        }
                    }
                }


                foreach (var q in scluster)
                {
                    q.ResetFlag();
                }

                return maxDistance;
            }
            else
            {
                // let p is root node.
                // p is node that argmin~p (distance(p, rootPoint))
                Point3DExd p = scluster[0];
                foreach (var q in scluster)
                {
                    q.ResetFlag();
                    double dp = Math.Pow(p.X - rootPoint.X, 2) + Math.Pow(p.Y - rootPoint.Y, 2) + Math.Pow(p.Z - rootPoint.Z, 2);
                    double dq = Math.Pow(q.X - rootPoint.X, 2) + Math.Pow(q.Y - rootPoint.Y, 2) + Math.Pow(q.Z - rootPoint.Z, 2);
                    if (dq < dp)
                    {
                        p = q;
                    }
                }

                var que = new Queue<Point3DExd>();
                p.Distance = 0;
                que.Enqueue(p);

                uint maxDistance = 0;

                while (que.Count > 0)
                {
                    var q = que.Dequeue();
                    foreach (var next in q.cobj)
                    {
                        if (next.flag == false)
                        {
                            next.Distance = q.Distance + 1;
                            maxDistance = Math.Max(maxDistance, next.Distance);
                            que.Enqueue(next);
                        }
                    }
                }

                foreach (var q in scluster)
                {
                    q.ResetFlag();
                }

                return maxDistance;
            }
        }

        void RecursiveDistanceClusteringEco(Point3DExd obj, IList<Point3Di> cld)
        {
            //クラスタに点追加
            cld.Add(new Point3Di(obj.X, obj.Y, obj.Z));

            //クラスタリング済み にフラグを変える
            obj.SetFlag();

            //26近傍のボクセルに再帰
            foreach (var item in obj.cobj)
            {
                if (item.Distance == obj.Distance && !item.flag)
                {
                    RecursiveDistanceClusteringEco(item, cld);
                }
            }
        }


        /// <summary>
        /// 副連結成分(体積最大の連結成分以外のやつ)の構造抽出
        /// </summary>
        public void ExtractSubDendrites()
        {
            sub_dendrites = new List<Dendrite>();
            sub_dendrites_volume = new List<int>();

            for (uint i = 1; i < clusters_e.Count; i++)
            {
                IList<Point3DExd> scluster = clusters_e[i]; //処理対象クラスタ
                int volume = scluster.Count;

                //体積がしきい値を下回ったら戻る (ヒント: clusters_eは体積降順でソートされている)
                if (volume < param.volumeThreshold) return;

                uint subdistance = SSDTLoopEco(scluster);

                Dendrite subDendrite = new Dendrite((int)subdistance + 1, param.resolutionXY, param.resolutionZ);

                foreach (var tmp in scluster)
                {
                    if (!tmp.flag)
                    {
                        var cld = new List<Point3Di>();//距離値クラスタリング用の一時リスト

                        //距離値でクラスタリング
                        RecursiveDistanceClusteringEco(tmp, cld);

                        subDendrite.AddNode((int)tmp.Distance, cld);
                    }
                }

                subDendrite.CreateNodeTree();

                //subリストに追加
                sub_dendrites.Add(subDendrite);
                sub_dendrites_volume.Add(volume);
            }
        }

        // 変数名がイマイチに見えるのは、
        // nodeやclusterなどの名称が統一されていないからではなかろうか
        struct NodeIndexer
        {
            public double Norm { get; private set; }
            public int ClusterIdx { get; private set; }
            public int NodeIdx;

            public NodeIndexer(double norm, int clusterIdx, int nodeIdx) : this()
            {
                this.Norm = norm;
                this.ClusterIdx = clusterIdx;
                this.NodeIdx = nodeIdx;
            }
        }

        class NodeRelation : IComparable<NodeRelation>
        {
            public double Norm { get; private set; }
            public int MasterNodeIdx { get; private set; }
            public int SlaveClusterIdx { get; private set; }
            public int SlaveNodeIdx { get; private set; }

            public NodeRelation(double norm, int mainNodeIdx, int subClusterIdx, int subNodeIdx)
            {
                Norm = norm;
                MasterNodeIdx = mainNodeIdx;
                SlaveClusterIdx = subClusterIdx;
                SlaveNodeIdx = subNodeIdx;
            }

            public int CompareTo(NodeRelation other)
            {
                if (Norm != other.Norm) return Norm < other.Norm ? -1 : 1;
                if (MasterNodeIdx != other.MasterNodeIdx) return MasterNodeIdx > other.MasterNodeIdx ? -1 : 1;
                if (SlaveClusterIdx != other.SlaveClusterIdx) return SlaveClusterIdx > other.SlaveClusterIdx ? -1 : 1;
                if (SlaveNodeIdx != other.SlaveNodeIdx) return SlaveNodeIdx > other.SlaveNodeIdx ? -1 : 1;
                return 0;
            }
        }

        /// <summary>
        /// SubDendriteを距離閾値に従ってMainDendriteに接続
        /// </summary>
        public void ConnectSubDendritesToMainDendrite()
        {
            // nodelist[0]		: main
            // nodelist[1:]  	: sub
            List<List<DendriteNode>> nodelist = new List<List<DendriteNode>>();

            //メインクラスタの分岐，端点ノードを取得
            List<DendriteNode> pnode = main_dendrite.GetEdgeNodeAndBranchNodes();

            nodelist.Add(pnode);

            NotMergedSubDendritesIdx = new HashSet<int>();

            for (int i = 0; i < sub_dendrites.Count; i++)
            {
                //サブクラスタの分岐，端点ノードを取得
                List<DendriteNode> node = sub_dendrites[i].GetEdgeNodeAndBranchNodes();

                nodelist.Add(node);
                NotMergedSubDendritesIdx.Add(i);
            }

            var Q = new SkewHeap<NodeRelation>();
            for (int i = 0; i < nodelist[0].Count; i++)
            {
                for (int j = 1; j < nodelist.Count; j++)
                {
                    double normMin = double.MaxValue;
                    int argMin = -1;
                    for (int k = 0; k < nodelist[j].Count; k++)
                    {
                        double d = nodelist[0][i].EuclideanDistanceTo(nodelist[j][k]);
                        if (d <= normMin)
                        {
                            normMin = d;
                            argMin = k;
                        }
                    }

                    if (argMin != -1 && normMin <= param.distanceThreshold)
                    {
                        Q.Push(new NodeRelation(normMin, i, j, argMin));
                    }
                }
            }

            while (true)
            {
                NodeRelation rel = null;
                while (Q.Empty() == false)
                {
                    rel = Q.Pop();
                    if (nodelist[rel.SlaveClusterIdx].Count > 0)
                    {
                        break;
                    }
                }

                if (Q.Empty()) break;

                DendriteNode parent = nodelist[0][rel.MasterNodeIdx];
                DendriteNode cand = nodelist[rel.SlaveClusterIdx][rel.SlaveNodeIdx];

                //連結ノードにお互いを追加
                cand.AddConnectedNode(parent);
                parent.AddConnectedNode(cand);

                int mainNodeSizePrev = nodelist[0].Count;

                //連結したサブノードリストはメインノードリストに吸収
                nodelist[0].AddRange(nodelist[rel.SlaveClusterIdx]);

                //サブを消去
                nodelist[rel.SlaveClusterIdx].Clear();

                // マージされた
                NotMergedSubDendritesIdx.Remove(rel.SlaveClusterIdx - 1);

                // 吸収されたサブノードから、吸収されていないサブノードまでの距離を算出
                for (int i = mainNodeSizePrev; i < nodelist[0].Count; i++)
                {
                    for (int j = 1; j < nodelist.Count; j++)
                    {
                        double normMin = double.MaxValue;
                        int argMin = -1;
                        for (int k = 0; k < nodelist[j].Count; k++)
                        {
                            double d = nodelist[0][i].EuclideanDistanceTo(nodelist[j][k]);
                            if (d <= normMin)
                            {
                                normMin = d;
                                argMin = k;
                            }
                        }

                        if (argMin != -1 && normMin <= param.distanceThreshold)
                        {
                            Q.Push(new NodeRelation(normMin, i, j, argMin));
                        }
                    }
                }
            }

            main_dendrite.ChangeRootNode(main_dendrite.root);
            pnode = main_dendrite.GetEdgeNodes();
            // どうしてFirstでなくてLastなのかよく分からない
            main_dendrite.ChangeRootNode(pnode.Last());
        }

        /// <summary>
        /// ひげの除去
        /// </summary>
        public void Shaving()
        {
            main_dendrite.Shaving(param.shavingLevel);
        }

        /// <summary>
        /// 平滑化
        /// </summary>
        public void Smoothing()
        {
            main_dendrite.Smoothing(param.smoothingLevel);
        }

        /// <summary>
        /// 樹状突起構造をswcファイルに出力
        /// </summary>
        public void OutputDendrite()
        {
            DendriteIO io = new DendriteIO();
            IList<DendriteNode> edge = main_dendrite.GetEdgeNodes();
            main_dendrite.ChangeRootNode(edge.First());

            SetSwcHeader(main_dendrite);

            io.SaveToSWC(param.ofname, main_dendrite);

            if (param.outputVTK)
            {
                string vtkFilename = Path.ChangeExtension(param.ofname, "vtk");
                VtkWriter.SaveToVTKLegacy(vtkFilename, main_dendrite);
            }

            // sub_dendritesについても出力する（sub_dendritesのスムージング、ひげの除去には未対応)
            if (param.outputSub)
            {
                OutputSubDendrite();
            }
        }

        private void SetSwcHeader(Dendrite d)
        {
            d.SwcHeader.SetVersionDate();
            d.SwcHeader.SetSource(param.ipath);
            d.SwcHeader.SetScale(param.resolutionXY, param.resolutionXY, param.resolutionZ);
            d.SwcHeader.SetRootSetting(false);

            d.SwcHeader.SetInterpolation(param.interporation);

            if (param.interporation)
            {
                d.SwcHeader.SetInterpolation(true);
                d.SwcHeader.SetDistanceThreshold(param.distanceThreshold);
                d.SwcHeader.SetVolumeThreshold(param.volumeThreshold);
            }

            if (param.shavingLevel > 0)
            {
                d.SwcHeader.SetClipping(param.shavingLevel);
            }

            if (param.smoothingLevel > 0)
            {
                d.SwcHeader.SetSmoothing(param.smoothingLevel);
            }
        }

        struct Tuple2 : IEquatable<Tuple2>
        {
            int first, second;

            public bool Equals(Tuple2 other)
            {
                return this.first == other.first && this.second == other.second;
            }

            public Tuple2(int fst, int snd)
            {
                this.first = fst;
                this.second = snd;
            }
        }

        public void ConnectSubDendritesToOtherSubDendrites()
        {
            var nodelist = new Dictionary<int, List<DendriteNode>>();

            foreach (int index in NotMergedSubDendritesIdx)
            {
                List<DendriteNode> node = sub_dendrites[index].GetEdgeNodeAndBranchNodes();
                nodelist.Add(index, node);
            }

            DendriteNode masterNode = null;
            DendriteNode slaveNode = null;

            var cache = new Dictionary<Tuple2, NodeIndexer>();

            while (true)
            {
                // masterとslaveの距離をあらかじめ計算しておいて配列に保存し、ソートしておくことで
                // 高速に計算できるが、需要がないようなので後回しにする。
                // 配列に保存するのは距離がdistanceThreashold以下のものだけにしないとメモリが足りないと思います。
                // Masterがどこかのノードに吸収されるかもしれないので、
                // UnionFind木で管理しておきましょう

                int masterIndex = 0;
                int slaveIndex = 0;
                double min = Double.MaxValue;

                foreach (int master in NotMergedSubDendritesIdx)
                {
                    for (int master2 = 0; master2 < nodelist[master].Count; master2++)
                    {
                        var dictKey = new Tuple2(master, master2);
                        if (cache.ContainsKey(dictKey) == false || nodelist[cache[dictKey].ClusterIdx].Count == 0)
                        {
                            foreach (int slave in NotMergedSubDendritesIdx)
                            {
                                if (master == slave) continue;
                                for (int slave2 = 0; slave2 < nodelist[slave].Count; slave2++)
                                {
                                    double tmpdist = nodelist[master][master2].EuclideanDistanceTo(nodelist[slave][slave2]);

                                    // キャッシュがなかった場合は新しく追加
                                    if (cache.ContainsKey(dictKey) == false)
                                    {
                                        cache.Add(dictKey, new NodeIndexer(tmpdist, slave, slave2));
                                    }
                                    // キャッシュしていたノードが他のノードに併合されたパターン
                                    else if (nodelist[cache[dictKey].ClusterIdx].Count == 0)
                                    {
                                        cache[dictKey] = new NodeIndexer(tmpdist, slave, slave2);
                                    }
                                    // 再探索時に、キャッシュしていたノードよりも近いノードを見つけた場合には、キャッシュしている値を変更する
                                    else if (cache[dictKey].Norm >= tmpdist)
                                    {
                                        cache[dictKey] = new NodeIndexer(tmpdist, slave, slave2);
                                    }

                                    if (min >= tmpdist)
                                    {
                                        min = tmpdist;
                                        masterNode = nodelist[slave][slave2];
                                        slaveNode = nodelist[master][master2];
                                        masterIndex = master;
                                        slaveIndex = slave;
                                    }
                                }
                            }
                        }
                        // キャッシュが存在した場合はそれを利用
                        else
                        {
                            NodeIndexer nodeIndex = cache[dictKey];
                            double tmpdist = nodeIndex.Norm;
                            int slave = nodeIndex.ClusterIdx;
                            int slave2 = nodeIndex.NodeIdx;
                            if (min >= tmpdist)
                            {
                                min = tmpdist;
                                masterNode = nodelist[slave][slave2];
                                slaveNode = nodelist[master][master2];
                                masterIndex = master;
                                slaveIndex = slave;
                            }
                        }

                    }
                }

                if (min <= param.distanceThreshold)
                {
                    //連結ノードにお互いを追加
                    masterNode.AddConnectedNode(slaveNode);
                    slaveNode.AddConnectedNode(masterNode);

                    //連結したサブノードリストはメインノードリストに吸収
                    nodelist[masterIndex].AddRange(nodelist[slaveIndex]);

                    //サブを消去
                    nodelist[slaveIndex].Clear();

                    // マージされた
                    NotMergedSubDendritesIdx.Remove(slaveIndex);
                    sub_dendrites_volume[masterIndex] += sub_dendrites_volume[slaveIndex];
                    sub_dendrites_volume[slaveIndex] = 0;
                }
                else break;
            }

            foreach (int index in NotMergedSubDendritesIdx)
            {
                if (nodelist[index].Count > 0)
                {
                    Dendrite node = sub_dendrites[index];
                    var edge = node.GetEdgeNodes();
                    node.ChangeRootNode(edge.Last());
                }
            }

        }

        private void OutputSubDendrite()
        {
            DendriteIO io = new DendriteIO();

            string rawFilename = Path.GetFileNameWithoutExtension(param.ofname);
            string dirname = Path.Combine(Path.GetDirectoryName(param.ofname), rawFilename + "-sub");
            if (Directory.Exists(dirname))
            {
                Directory.Delete(dirname, true);
            }

            Directory.CreateDirectory(dirname);

            // 出力するファイル名の末尾の桁数
            // ファイル名は0オリジンにするので .Count - 1 にしている
            int digit = (NotMergedSubDendritesIdx.Count - 1).ToString().Length;
            // 出力するファイル名の末尾のID
            int sub_dendrite_id = 0;

            // 独立して存在している（他のノードに併合されていない）側枝ノードを、体積降順で並べ替える
            var orderByVolume = NotMergedSubDendritesIdx.OrderByDescending(x => sub_dendrites_volume[x]);

            foreach (int sub_index in orderByVolume)
            {
                Dendrite sdendrite = sub_dendrites[sub_index];
                IList<DendriteNode> edge = sdendrite.GetEdgeNodes();
                sdendrite.ChangeRootNode(edge[0]);

                string filename = CreateFileNameForSubDendriteIO(dirname, digit, sub_dendrite_id);
                string path = Path.Combine(dirname, filename);
                io.SaveToSWC(path, sdendrite);
                sub_dendrite_id++;

                if (param.outputVTK)
                {
                    string vtkFilename = Path.ChangeExtension(filename, "vtk");
                    string vtkPath = Path.Combine(dirname, vtkFilename);
                    VtkWriter.SaveToVTKLegacy(vtkPath, sdendrite);
                }
            }
        }

        private string CreateFileNameForSubDendriteIO(string dirname, int digit, int subdendriteId)
        {
            string withoutExt = Path.GetFileNameWithoutExtension(param.ofname);
            string suffix = "sub" + subdendriteId.ToString().PadLeft(digit, '0');
            return Path.ChangeExtension(withoutExt + "-" + suffix, "swc");
        }

        public void RelateSwcNode(Dendrite swc)
        {
            // addNodeToList(main_dendrite.root, null); の直前で-1を設定したかったけど
            // unassigned local variableで怒られた(´・ω・｀)
            int currentSubDendriteIndex = -1;

            var nodeListImg = new List<DendriteNode>();
            var node2subindex = new Dictionary<DendriteNode, int>();

            Action<DendriteNode, DendriteNode> addNodeToList = null;
            addNodeToList = (node, parent) =>
            {
                nodeListImg.Add(node);
                node2subindex.Add(node, currentSubDendriteIndex);

                foreach (var next in node.ConnectedNodes) {
                    if (next == parent) continue;
                    addNodeToList(next, node);
                }
            };

            addNodeToList(main_dendrite.root, null);
            for (int i = 0; i < sub_dendrites.Count; i++)
            {
                currentSubDendriteIndex = i;
                addNodeToList(sub_dendrites[i].root, null);
            }

            foreach (var node in nodeListImg)
            {
                node.isConnected = false;
            }

            var img2swc = new Dictionary<DendriteNode, DendriteNode>();
            var que = new Queue<DendriteNode>();
            var removeCandidate = new HashSet<int>();

            Action<DendriteNode, DendriteNode> mapping = null;
            mapping = (node, parent) =>
            {
                Debug.Assert(nodeListImg.Count > 0);
                var pair = nodeListImg.First();
                foreach (var item in nodeListImg)
                {
                    if (node.EuclideanDistanceTo(item) < node.EuclideanDistanceTo(pair)) {
                        pair = item;
                    }
                }
                pair.isConnected = true;
                img2swc[pair] = node;
                que.Enqueue(pair);

                Debug.Assert(node2subindex.ContainsKey(pair));
                if (node2subindex[pair] != -1) {
                    removeCandidate.Add(node2subindex[pair]);
                }

                foreach (var next in node.ConnectedNodes) {
                    if (next == parent) {
                        continue;
                    }
                    mapping(next, node);
                }
            };
            mapping(swc.root, null);

            while (que.Count > 0)
            {
                var node = que.Dequeue();
                foreach (var next in node.ConnectedNodes)
                {
                    if (next.isConnected)
                    {
                        continue;
                    }
                    next.isConnected = true;
                    var newNode = new DendriteNode(-1, next.gx, next.gy, next.gz, next.Radius, -1);
                    Debug.Assert(img2swc.ContainsKey(node));
                    img2swc[node].AddConnectedNode(newNode);
                    newNode.AddConnectedNode(img2swc[node]);
                    que.Enqueue(next);
                    img2swc[next] = newNode;
                }
            }

            swc.ChangeRootNode(swc.root);
            main_dendrite = swc;

            int offset = 0;
            foreach (var item in removeCandidate.ToList().OrderBy(x => x))
            {
                sub_dendrites.RemoveAt(item + offset);
                sub_dendrites_volume.RemoveAt(item + offset);
                offset--;
            }
        }
    }
}
