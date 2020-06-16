#ifndef RANDOMFOREST_H
#define RANDOMFOREST_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVector>
#include <QMap>
#include <QTime>
#include <QStringList>

#include <algorithm>
#include <math.h>
#include <fstream>
#include <iostream>

#define DOUBLE_MIN 2.225e-308
#define INT_MIN 0x80000000

int getRandomNum();

class DecisionTree;

class RandomForest
{
public:
    int C,M,Ms;
    int numTrees;
    RandomForest(){
        this->trees = QVector<DecisionTree>();
    }
    RandomForest(int numTrees, QVector<QVector<float> > trainData);
    void updataOOBEstimate(QVector<float> *record, int Class);
    void start();
    int labelVote(QVector<int> treePredict);
    void calcImportance();
    int evaluate(QVector<float> record);
    int findMaxIndex(QVector<int> arr);
    QVector<int> sortTreesByAccuracy(QVector<QVector<float> > data);
    QVector<int> arraySort(QVector<double> arr);
    bool mergeRandomForest(RandomForest *oldRF, RandomForest *newRF, QVector<QVector<float> > data);
    QVector<DecisionTree>& getTrees(){
        return this->trees;
    }
    bool saveRandomForest(QString dir);
    bool readRandomForest(QString dir);

private:
    long starTime;

    double updata;
    double progress;
//    int* importance;
    QMap<QVector<float>*,QVector<int>*> estimateOOB;
    QVector<QVector<int>> prediction;
    double error;
    QVector<QVector<float>> trainData;
    QVector<DecisionTree> trees;
};

class TreeNode
{
public:
    bool isLeaf;
    TreeNode* left;
    TreeNode* right;
    int splitAttributeM;
    int Class;
    QVector<QVector<float> > data;
    float splitValue;
    int generation;
    QVector<int> attrArr;

    TreeNode() {
        this->left = 0;
        this->right = 0;
        this->splitAttributeM = -99;
        this->splitValue = -99;
        this->generation = 1;
        this->Class = -1;
        this->isLeaf = false;
        this->attrArr = QVector<int>();
        this->data = QVector<QVector<float> >();
    }

    ~TreeNode();

    TreeNode* clone();
};

class attributeCompare
{

public:

    static int m;

    attributeCompare() {}

    static void setM(int m){
        attributeCompare::m = m;
    }

    static bool compare(QVector<float> q1, QVector<float> q2){
        if(q1.at(m) < q2.at(m))
            return false;
        else {
            return true;
        }
    }
};

class DecisionTree
{
private:
    static const int IndexSkip = 2;
    static const int MinSizeToCheckEach = 10;
    int N;
    int testN;
    int correct;
    int* importance;
    TreeNode* root;
    RandomForest* forest;

    QVector<QVector<float> > randomlyPermuteAttribute(QVector<QVector<float>>& val, int m);
    QVector<QVector<float>> cloneData(QVector<QVector<float>> data);
    TreeNode* creatTree(QVector<QVector<float>> train, int nTree);
    void recursiveSplit(TreeNode *parent, int nTreeNum);
    int getMajorityClass(QVector<QVector<float>> data);
    double checkPosition(int m, int n, int nSub, double & lowestE, TreeNode* parent, int nTree);
    int checkIfLeaf(QVector<QVector<float>> data);
    QVector<QVector<float>> getUpper(QVector<QVector<float>> data, int nSplit);
    QVector<QVector<float>> getLower(QVector<QVector<float>> data, int nSplit);
    void sortAtAttribute(QVector<QVector<float> > &data, int m);
    QVector<double> getClassProbs(QVector<QVector<float>> data);
    double calcEntropy(QVector<double> ps);
    QVector<int> getVarsToInclude();
    void bootStrapSample(QVector<QVector<float>> data, QVector<QVector<float>>& train, QVector<QVector<float>>& val, int num);
    void flushData(TreeNode* node);

public:
    QVector<int> prediction;
    DecisionTree() {
        root = 0;
        importance = 0;
        forest = 0;
    }

    bool saveDesionTree(QString file,int i);
    bool readDesionTree(QString file);
    DecisionTree(QVector<QVector<float>> data, RandomForest* forest, int num);
    void calcTreeVariableImportanceAndError(QVector<QVector<float>> val, int nv);
    int calcTreeErrorRate(QVector<QVector<float>> val, int nu);
    QVector<int> calculateClasses(QVector<QVector<float>> val, int nu);
    int evaluate(QVector<float> record);

    int getClass(QVector<float> record);


};

#endif // RANDOMFOREST_H
