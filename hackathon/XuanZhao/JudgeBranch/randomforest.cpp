#include "randomforest.h"

int getRandomNum(){
    QTime time;
    time  = QTime::currentTime();
    qsrand(time.msec() + time.second()*1000);
    int n = qrand();
    return n;
}

TreeNode* TreeNode::clone(){
    TreeNode* treeCopy = new TreeNode();
    treeCopy->data = QVector<QVector<float> >();
    treeCopy->isLeaf = this->isLeaf;
    if(this->left){
        treeCopy->left = this->left->clone();
    }
    if(this->right){
        treeCopy->right = this->right->clone();
    }
    treeCopy->splitAttributeM = this->splitAttributeM;
    treeCopy->Class = this->Class;
    treeCopy->splitValue = this->splitValue;
    return treeCopy;
}

TreeNode::~TreeNode(){
    for(int i=0; i<this->data.size(); i++){
        this->data[i].clear();
    }
    this->data.clear();
//    qDeleteAll(this->data);
    this->attrArr.clear();
    if(this->left)
        delete left;
    if(this->right)
        delete right;
}

int attributeCompare::m = 0;

DecisionTree::DecisionTree(QVector<QVector<float> > data, RandomForest *forest, int num){
    this->forest = forest;
    this->N = data.size();
    this->importance = new int[this->forest->M];
    for(int i=0; i<this->forest->M; i++){
        this->importance[i] = 0;
    }
    this->prediction = QVector<int>();
    QVector<QVector<float>> train = QVector<QVector<float>>();
    QVector<QVector<float>> val = QVector<QVector<float>>();

    this->bootStrapSample(data,train,val,num);
    this->testN = val.size();
    this->correct = 0;

    this->root = this->creatTree(train,num);
    this->calcTreeVariableImportanceAndError(val,num);
    this->flushData(this->root);
}

int DecisionTree::evaluate(QVector<float> record){
    TreeNode* evalNode = root;
    while(true){
        if(evalNode->isLeaf)
            return evalNode->Class;
        if(record.at(evalNode->splitAttributeM) <= evalNode->splitValue)
            evalNode = evalNode->left;
        else
            evalNode = evalNode->right;
    }
}

QVector<QVector<float> > DecisionTree::randomlyPermuteAttribute(QVector<QVector<float> > &val, int m){
    int num = val.size() * 2;
    qDebug()<<"in randomlyPermuteAttribute";
    for(int i=0; i<num; i++){
        int a = getRandomNum()%val.size();
        int b = getRandomNum()%val.size();
        QVector<float>& arrA = val[a];
        QVector<float>& arrB = val[b];
        float temp = arrA[m];
        arrA[m] = arrB[m];
        arrB[m] = temp;
    }
    qDebug()<<"end randomlyPermuteAttribute";
    return val;
}

QVector<QVector<float> > DecisionTree::cloneData(QVector<QVector<float> > data){
    qDebug()<<"start cloneData";
    QVector<QVector<float> > clone = QVector<QVector<float> >();
    if(data.isEmpty())
        return clone;
    for(int i=0; i<data.size(); i++){
        QVector<float>& arr = data[i];
        QVector<float> arrClone = QVector<float>();
        for(int j=0; j<arr.size(); j++){
            arrClone.append(arr[j]);
        }
        clone.append(arrClone);
    }
    qDebug()<<"end cloneData";
    return clone;
}

int DecisionTree::getMajorityClass(QVector<QVector<float> > data){
    int* counts = new int[this->forest->C];
    for(QVector<float> record:data){
        int Class = (int) record.back();
        counts[Class-1]++;
    }
    int index = -99;
    int max = INT_MIN;
    for(int i=0; i<this->forest->C; i++){
        if(counts[i] > max){
            index = i + 1;
        }
    }
    delete counts;
    return index;
}

double DecisionTree::calcEntropy(QVector<double> ps){
    double e = 0;
    for(double p:ps){
        if(p!=0)
            e += p*log(p)/log(2);
    }
    return -e;
}

QVector<int> DecisionTree::getVarsToInclude(){
    bool* whichVarsToInclude = new bool[this->forest->M];
    for(int i=0; i<this->forest->M; i++)
        whichVarsToInclude[i] = false;

    while (true) {
        int a = getRandomNum()%this->forest->M;
        whichVarsToInclude[a] = true;
        int N = 0;
        for(int i=0; i<this->forest->M; i++){
            if(whichVarsToInclude[i])
                N++;
        }
        if(N == this->forest->Ms)
            break;
    }

    QVector<int> shortRecord = QVector<int>();
    for(int i=0; i<this->forest->M; i++){
        if(whichVarsToInclude[i])
            shortRecord.append(i);
    }
    return shortRecord;
}

void DecisionTree::bootStrapSample(QVector<QVector<float> > data, QVector<QVector<float> > &train, QVector<QVector<float> > &val, int num){
    QVector<int> indexs = QVector<int>();
    for(int n=0; n<N; n++){
        indexs.append(getRandomNum()%N);
    }
    QVector<bool> isIn = QVector<bool>(N,false);
    for(int i=0; i<indexs.size(); i++){
        int index = indexs[i];
        QVector<float> record = QVector<float>();
        for(int j=0; j<data.at(index).size(); j++){
            record.append(data[index][j]);
        }
        train.append(record);
        isIn[index] = true;
    }

    for(int i=0; i<N; i++){
        if(!isIn.at(i)){
            QVector<float> record = QVector<float>();
            for(int j=0; j<data.at(i).size(); j++){
                record.append(data[i][j]);
            }
            val.append(record);
        }
    }
}

double DecisionTree::checkPosition(int m, int n, int nSub, double &lowestE, TreeNode* parent, int nTree){
    if(n<1)
        return 0;
    if(n>nSub)
        return 0;
    QVector<QVector<float> > lower = this->getLower(parent->data,n);
    QVector<QVector<float> > upper = this->getUpper(parent->data,n);

    if(lower.isEmpty())
        qDebug()<<"lower list is null";
    if(upper.isEmpty())
        qDebug()<<"upper list is null";
    QVector<double> p = this->getClassProbs(parent->data);
    QVector<double> pl = this->getClassProbs(lower);
    QVector<double> pu = this->getClassProbs(upper);
    double ep = this->calcEntropy(p);
    double el = this->calcEntropy(pl);
    double eu  = this->calcEntropy(pu);

    double e = ep - el*lower.size()/(double)nSub - eu*upper.size()/(double)nSub;

    if(e>lowestE){
        lowestE = e;
        parent->splitAttributeM = m;
        parent->splitValue = parent->data.at(n).at(m);
        for(int j=0; j<lower.size(); j++){
            parent->left->data.append(lower[j]);
        }
        for(int j=0; j<upper.size(); j++){
            parent->right->data.append(upper[j]);
        }
    }

    return e;
}

int DecisionTree::getClass(QVector<float> record){
    return (int) record.back();
}

int DecisionTree::checkIfLeaf(QVector<QVector<float> > data){
    bool isLeaf = true;
    if(data.isEmpty()){
        return -1;
    }
    int ClassA = this->getClass(data[0]);
    for(int i=1; i<data.size(); i++){
        QVector<float>& recordB = data[i];
        if(ClassA != this->getClass(recordB)){
            isLeaf = false;
            break;
        }
    }

    if(isLeaf)
        return this->getClass(data.at(0));
    else
        return -1;

}

QVector<QVector<float> > DecisionTree::getUpper(QVector<QVector<float> > data, int nSplit){
    QVector<QVector<float> > upper = QVector<QVector<float> >();
    for(int i=nSplit; i<data.size(); i++){
        QVector<float> record = QVector<float>();
        for(int j=0; j<data[i].size(); j++){
            record.append(data[i][j]);
        }
        upper.append(record);
    }
    return upper;
}

QVector<QVector<float> > DecisionTree::getLower(QVector<QVector<float> > data, int nSplit){
    QVector<QVector<float> > lower = QVector<QVector<float> >();
    for(int i=0; i<nSplit; i++){
        QVector<float> record = QVector<float>();
        for(int j=0; j<data[i].size(); j++){
            record.append(data[i][j]);
        }
        lower.append(record);
    }
    return lower;
}

void DecisionTree::sortAtAttribute(QVector<QVector<float> >& data, int m){
    attributeCompare::setM(m);
    std::sort(data.begin(),data.end(),attributeCompare::compare);
}

QVector<double> DecisionTree::getClassProbs(QVector<QVector<float> > data){
    double N = data.size();

    int* counts = new int[this->forest->C];
    for(QVector<float> record:data){
        counts[this->getClass(record)-1]++;
    }

    QVector<double> ps = QVector<double>();
    for(int j=0; j<this->forest->C; j++){
        ps.append(counts[j]/N);
    }
    delete counts;
    return ps;
}

void DecisionTree::recursiveSplit(TreeNode* parent, int nTreeNum){
    if(!parent->isLeaf){
        int Class = this->checkIfLeaf(parent->data);
        qDebug()<<"class: "<<Class;
        if(Class != -1){
            parent->isLeaf = true;
            parent->Class = Class;
            return;
        }

        int nSub = parent->data.size();

        parent->left = new TreeNode();
        parent->left->generation = parent->generation + 1;
        parent->right = new TreeNode();
        parent->right->generation = parent->generation + 1;
        double lowestE = DOUBLE_MIN;
        if(parent->attrArr.size() == 0){
            parent->Class = this->getMajorityClass(parent->data);
            return;
        }

        int sameClass = 0;
        for(int m:parent->attrArr){
            this->sortAtAttribute(parent->data,m);
            QVector<int> indexsToCheck = QVector<int>();
            for(int n=1; n<nSub; n++){
                int classA = this->getClass(parent->data.at(n-1));
                int classB = this->getClass(parent->data.at(n));
                if(classA != classB)
                    indexsToCheck.append(n);
            }
            if(indexsToCheck.size() == 0)
                sameClass++;
        }
        if(sameClass == parent->attrArr.size()){
            parent->isLeaf = true;
            parent->Class = this->getMajorityClass(parent->data);
            return;
        }
        for(int m:parent->attrArr){
            this->sortAtAttribute(parent->data,m);
            QVector<int> indexsToCheck = QVector<int>();
            for(int n=1; n<nSub; n++){
                int classA = this->getClass(parent->data.at(n-1));
                int classB = this->getClass(parent->data.at(n));
                if(classA != classB)
                    indexsToCheck.append(n);
            }
            if(indexsToCheck.size() > MinSizeToCheckEach){
                for(int i=0; i<indexsToCheck.size(); i+=IndexSkip){
                    this->checkPosition(m,indexsToCheck.at(i),nSub,lowestE,parent,nTreeNum);
                    if(lowestE == 0)
                        break;
                }
            }else {
                for(int index:indexsToCheck){
                    this->checkPosition(m,index,nSub,lowestE,parent,nTreeNum);
                    if(lowestE == 0)
                        break;
                }
            }
            if(lowestE == 0)
                break;
        }

        QVector<int>::iterator it = parent->attrArr.begin();
        while (it != parent->attrArr.end()) {
            int attr = *it;
            if(attr == parent->splitAttributeM){
                parent->attrArr.erase(it);//.remove(it);
            }
            it++;
        }
        for(int i=0; i<parent->attrArr.size(); i++){
            parent->left->attrArr.append(parent->attrArr[i]);
            parent->right->attrArr.append(parent->attrArr[i]);
        }


        if(parent->left->data.size() == 1){
            parent->left->isLeaf = true;
            parent->left->Class = this->getClass(parent->left->data.at(0));
        }else if (parent->left->attrArr.size() == 0) {
            parent->left->isLeaf = true;
            parent->left->Class = this->getMajorityClass(parent->left->data);
        }else {
            Class = this->checkIfLeaf(parent->left->data);
            if(Class == -1){
                parent->left->isLeaf = false;
                parent->left->Class = Class;
            }else {
                parent->left->isLeaf = true;
                parent->left->Class = Class;
            }
        }

        if(parent->right->data.size() == 1){
            parent->right->isLeaf = true;
            parent->right->Class = this->getClass(parent->right->data.at(0));
        }else if (parent->right->attrArr.size() == 0) {
            parent->right->isLeaf = true;
            parent->right->Class = this->getMajorityClass(parent->right->data);
        }else {
            Class = this->checkIfLeaf(parent->right->data);
            if(Class == -1){
                parent->right->isLeaf = false;
                parent->right->Class = Class;
            }else {
                parent->right->isLeaf = true;
                parent->right->Class = Class;
            }
        }

        if(!parent->left->isLeaf){
            this->recursiveSplit(parent->left,nTreeNum);
        }

        if(!parent->right->isLeaf){
            this->recursiveSplit(parent->right,nTreeNum);
        }
    }
}

TreeNode* DecisionTree::creatTree(QVector<QVector<float> > train, int nTree){
    TreeNode* root = new TreeNode();
    root->data = this->cloneData(train);
    QVector<int> vars = this->getVarsToInclude();
    for(int i=0; i<vars.size(); i++){
        root->attrArr.append(vars[i]);
    }
    vars.clear();
    this->recursiveSplit(root,nTree);
    return root;
}

int DecisionTree::calcTreeErrorRate(QVector<QVector<float> > val, int nu){
    int correct = 0;
    for(QVector<float> record:val){
        int preLable = this->evaluate(record);
        this->forest->updataOOBEstimate(&record,preLable);
        int actualLable = (int) record.back();
        if(preLable == actualLable)
            correct++;
    }

    double err = 1 - correct/((double) val.size());
    qDebug()<<"Number of correct = "<<correct<<", out of: "<<val.size();
    qDebug()<<"Test-Data error rate of tree "<<nu<<" is "<<(err*100)<<" %";
    return correct;
}

QVector<int> DecisionTree::calculateClasses(QVector<QVector<float> > val, int nu){
    QVector<int> preList = QVector<int>();
    for(QVector<float> record:val){
        int preLable = this->evaluate(record);
        preList.append(preLable);
    }
    for(int i=0; i<preList.size(); i++){
        this->prediction.append(preList[i]);
    }
//    this->prediction.insert(this->prediction.begin(),preList.begin(),preList.end());
    return preList;
}

void DecisionTree::calcTreeVariableImportanceAndError(QVector<QVector<float> > val, int nv){
    this->correct = this->calcTreeErrorRate(val,nv);
    qDebug()<<"start calculateClasses";
    this->calculateClasses(val,nv);

    qDebug()<<"end calculateClasses";


    for(int m=0; m<this->forest->M; m++){
        qDebug()<<"m: "<<m<<endl;
        QVector<QVector<float>> testData = this->cloneData(val);
        this->randomlyPermuteAttribute(testData,m);
        int correctAfterPermute = 0;
        for(int i=0; i<testData.size(); i++){
            QVector<float>& arr = testData[i];
            int preLabel = this->evaluate(arr);
            if(preLabel == this->getClass(arr))
                correctAfterPermute++;
        }
        importance[m] += (this->correct - correctAfterPermute);
    }
    qDebug()<<"The importance of tree "<<nv<<" as follows:";
    for(int m=0; m<this->forest->M; m++){
        qDebug()<<"Attr "<<m<<" : "<<importance[m];
    }
}

bool DecisionTree::saveDesionTree(QString file, int i){

    QString path = file + "/DecisionTree_" + QString::number(i) + ".dt";

    std::ofstream outfile;
    outfile.open(path.toStdString().c_str());
    if(!outfile)
        return false;

    outfile<<"##n,left,right,isLeaf,splitAttributeM,Class,splitValue,generation,attrArr\n";

    QVector<TreeNode*> queue = QVector<TreeNode*>();
    queue.append(this->root);

    QVector<TreeNode*> treeNodes = QVector<TreeNode*>();
    treeNodes.append(this->root);

    while (!queue.isEmpty()) {
        TreeNode* temp = queue.front();
        queue.remove(0);
        if(temp->left){
            treeNodes.append(temp->left);
            queue.append(temp->left);
        }
        if(temp->right){
            treeNodes.append(temp->right);
            queue.append(temp->right);
        }
    }

    QMap<TreeNode*,int> treeNodeIndexMap = QMap<TreeNode*,int>();

    for(int k=0; k<treeNodes.size(); k++){
        treeNodeIndexMap.insert(treeNodes.at(k),k);
    }

    queue.append(this->root);

    while (!queue.isEmpty()) {
        TreeNode* temp = queue.front();
        queue.remove(0);

        int n = treeNodeIndexMap.value(temp);
        int left = -1, right = -1;
        if(treeNodeIndexMap.contains(temp->left)){
            left = treeNodeIndexMap.value(temp->left);
        }
        if(treeNodeIndexMap.contains(temp->right)){
            right = treeNodeIndexMap.value(temp->right);
        }
        int leaf = temp->isLeaf?1:0;

        outfile<<n<<" "<<left<<" "<<right<<" "<<leaf<<" "<<temp->splitAttributeM<<" "
              <<temp->Class<<" "<<temp->splitValue<<" "<<temp->generation;
        if(!temp->attrArr.isEmpty()){
            for(int k=0; k<temp->attrArr.size(); k++){
                outfile<<" "<<temp->attrArr.at(k);
            }
        }
        outfile<<"\n";

        if(temp->left){
            queue.append(temp->left);
        }
        if(temp->right){
            queue.append(temp->right);
        }
    }

    outfile.close();

    qDebug()<<"done with saving file "<<path;
    return true;
}

bool DecisionTree::readDesionTree(QString file){
    QVector<std::string> arrayList = QVector<std::string>();
    qDebug()<<"-------------in read DecisionTree------------";
    std::ifstream infile;
    infile.open(file.toStdString().c_str());

    if(infile){
        std::string str;
        while (std::getline(infile,str)) {
            arrayList.append(str);
        }
    }else {
        return false;
    }

    infile.close();

//    qDebug()<<"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

    QVector<TreeNode*> treeNodes = QVector<TreeNode*>();
    QVector<QStringList> lists = QVector<QStringList>();

    for(int i=0; i<arrayList.size(); i++){

        qDebug()<<"i: "<<i;

        QString current = QString::fromStdString(arrayList[i]);
//        current.fromStdString(arrayList.at(i));
        QStringList s = current.split(QString(" "));
//        qDebug()<<"s[0].left(0) : "<<s[0].left(0);
        if(s[0].left(1) == "#")
            continue;
        lists.append(s);
        TreeNode * temp = new TreeNode();
        temp->isLeaf = s[3].toInt() == 1?true:false;
        temp->splitAttributeM = s[4].toInt();
        temp->Class = s[5].toInt();
        temp->splitValue = s[6].toFloat();
        temp->generation = s[7].toInt();

        if(s.size()>8){
            for(int j=8; j<s.size(); j++){
                temp->attrArr.append(s[j].toInt());
            }
        }
        treeNodes.append(temp);
    }
    if(treeNodes.isEmpty()){
        qDebug()<<"the file is null";
        return false;
    }

//    qDebug()<<"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    this->root = treeNodes.at(0);
    for(int i=0; i<treeNodes.size(); i++){
        TreeNode* temp = treeNodes.at(i);
        QStringList s = lists[i];
        int left = s[1].toInt();
        int right = s[2].toInt();

        if(left>0){
            temp->left = treeNodes.at(left);
        }

        if(right>0){
            temp->right = treeNodes.at(right);
        }
    }

//    qDebug()<<"ccccccccccccccccccccccccccccccccccccc";

    return true;
}

void DecisionTree::flushData(TreeNode* node){
    qDebug()<<"in flushData";
    if(node){
        qDebug()<<"data size: "<<node->data.size();
        for(int i=0; i<node->data.size(); i++){
            node->data[i].clear();
        }
        node->data.clear();
    }


    if(node && node->left){
        this->flushData(node->left);
    }

    if(node && node->right){
        this->flushData(node->right);
    }

}












RandomForest::RandomForest(int numTrees, QVector<QVector<float> > trainData){
    std::cout<<"in initial RandomForest"<<endl;
    this->numTrees = numTrees;
    this->trainData = trainData;
    this->trees = QVector<DecisionTree>();

    this->estimateOOB = QMap<QVector<float>*,QVector<int>* >();
    this->prediction = QVector<QVector<int>>();
}

void RandomForest::start(){
    for(int t=0; t<numTrees; t++){
        qDebug()<<"t: "<<t<<endl;
        DecisionTree tree = DecisionTree(this->trainData,this,t+1);
        this->trees.append(tree);
    }
}

int RandomForest::labelVote(QVector<int> treePredict){
    int max = 0;
    int maxclass = -1;
    for(int i=0; i<treePredict.size(); i++){
        int count = 0;
        for(int j=0; j<treePredict.size(); j++){
            if(treePredict.at(j) == treePredict.at(i))
                count++;
            if(count>max){
                maxclass = treePredict.at(i);
                max = count;
            }
        }
    }
    return maxclass;
}

void RandomForest::updataOOBEstimate(QVector<float>* record, int Class){
    if(!estimateOOB.contains(record)){
        QVector<int>* map = new QVector<int>(this->C,0);
        estimateOOB.insert(record,map);
    }else {
//        QVector<int>& map = estimateOOB.//value(record);
//        map[Class-1]++;
        (*estimateOOB[record])[Class-1]++;
    }
}

int RandomForest::evaluate(QVector<float> record){
//    qDebug()<<"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$";
    QVector<int> counts = QVector<int>();
    for(int i=0; i<this->C; i++){
        counts.append(0);
    }
//    qDebug()<<"------------in evaluate------------";
    for(int t=0; t<numTrees; t++){
//        qDebug()<<"t: "<<t;
        int Class = trees[t].evaluate(record);
//        qDebug()<<"class: "<<Class;
        counts[Class-1]++;
    }
//    qDebug()<<"--------------findMaxIndex---------------";
    int i =  this->findMaxIndex(counts);
//    qDebug()<<"-----------end findMaxIndex---------------";
    counts.clear();
    return i;
}

int RandomForest::findMaxIndex(QVector<int> arr){
    int index = 0;
    int max = INT_MIN;
    for(int i=0; i<arr.size(); i++){
        if(arr.at(i)>max){
            max = arr.at(i);
            index = i;
        }
    }
    return index;
}

QVector<int> RandomForest::sortTreesByAccuracy(QVector<QVector<float> > data){
    QVector<double> accuracies = QVector<double>(this->numTrees,0);
    for(int i=0; i<this->numTrees; i++){
        int correctNum = 0;
        for(int j=0; j<data.size(); j++){
            int label = (int) data.at(j).back();
            int preLable = trees[i].evaluate(data.at(j));
            if(label == preLable){
                correctNum++;
            }
        }
        accuracies[i] = correctNum/(double)data.size();
    }
    return this->arraySort(accuracies);
}

QVector<int> RandomForest::arraySort(QVector<double> arr){
    double temp;
    int index;
    int k = arr.size();
    QVector<int> indexs = QVector<int>(k,0);
    for(int i=0;i<k;i++){
        indexs[i] = i;
    }

    for(int i=0; i<k; i++){
        for(int j=0; j<k-i-1; j++){
            if(arr.at(j)<arr.at(j+1)){
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;

                index = indexs[j];
                indexs[j] = indexs[j+1];
                indexs[j+1] = index;
            }
        }
    }

    return indexs;
}

bool RandomForest::mergeRandomForest(RandomForest* oldRF, RandomForest* newRF, QVector<QVector<float> > data){
    if(oldRF->C != newRF->C || oldRF->M != newRF->M || oldRF->Ms != newRF->Ms){
        return false;
    }else {
        this->C = oldRF->C;
        this->M = oldRF->M;
        this->Ms = oldRF->Ms;
    }

    QVector<int> oldIndex = oldRF->sortTreesByAccuracy(data);
    QVector<int> newIndex = newRF->sortTreesByAccuracy(data);

    QVector<DecisionTree> oldTrees = oldRF->getTrees();
    QVector<DecisionTree> newTrees = newRF->getTrees();

    this->trees = QVector<DecisionTree>();
    int oc = oldTrees.size();
    int nc = newTrees.size();

    for(int i=0; i<oc*0.8; i++){
        this->trees.append(oldTrees.at(oldIndex.at(i)));
    }

    for(int i=0; i<nc*0.2; i++){
        this->trees.append(newTrees.at(newIndex.at(i)));
    }

    this->numTrees = (int) (oc*0.8) + (int) (nc*0.2);

    return true;
}

//QVector<DecisionTree> &RandomForest::getTrees(){
//    return (this->trees);
//}

bool RandomForest::saveRandomForest(QString dir){
    QString path = dir + "/randomForest.rf";
    std::ofstream outfile;
    outfile.open(path.toStdString().c_str());

    if(!outfile)
        return false;

    outfile<<"##C M Ms numTrees\n";
    outfile<<this->C<<" "<<this->M<<" "<<this->Ms<<" "<<this->numTrees<<"\n";
    outfile.close();

    for(int i=0; i<this->trees.size(); i++){
        DecisionTree& tree = trees[i];
        tree.saveDesionTree(dir,i);
    }

    return true;
}

bool RandomForest::readRandomForest(QString dir){
    QString rfPath = dir + "/randomForest.rf";
    QVector<std::string> arrayList = QVector<std::string>();
    qDebug()<<"-------------in read RandomForest------------";
    std::ifstream infile;
    infile.open(rfPath.toStdString().c_str());

    if(infile){
        std::string str;
        while (std::getline(infile,str)) {
            arrayList.append(str);
        }
    }else {
        return false;
    }

    infile.close();

    qDebug()<<"start to read decisiontrees";

    if(arrayList.size()>1){
        QString current = QString::fromStdString(arrayList[1]);
        qDebug()<<arrayList[1].c_str();
//        current.fromStdString(arrayList[1]);
        qDebug()<<current;
        QStringList s = current.split(" ");
        if(s.size()!=4)
            return false;
        this->C = s.at(0).toInt();
        this->M = s.at(1).toInt();
        this->Ms = s.at(2).toInt();
        this->numTrees = s.at(3).toInt();
    }else {
        return false;
    }

    this->trees.clear();

    for(int i=0; i<this->numTrees; i++){
        QString dtPath = dir + "/DecisionTree_" + QString::number(i) + ".dt";
        DecisionTree d = DecisionTree();
        qDebug()<<"i: "<<i;
        d.readDesionTree(dtPath);
        this->trees.append(d);
    }

    return true;
}





