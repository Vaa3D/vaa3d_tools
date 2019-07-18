#ifndef CNN_H
#define CNN_H

#include<time.h>

class CNN{
public:
    CNN();
    ~CNN();

    void init();
    void readFile();
    void ff(int index);
    void bp(int index);
    void update(int index);
    double evaluate();
    void train();
    void test();
    void checkGrad();
private:
    tm timer;
    static const int train_times = 10000000;
    double a;
    double r;
    static const int layer_num = 7;
    static const int batch_size = 1;
    static const int train_size = 20000;
    static const int test_size = 40000;
    //训练图像的label
    int label[train_size];

    //输入层（测试数据）
    double testI0[test_size][32][32];

    //输入层(训练数据)
    double I0[train_size][32][32];

    //连接
    bool connection[6][12];

    //C1层
    double C1[batch_size][6][28][28];//卷积后的图像
    double C1_conv[6][5][5];//卷积核
    double C1_dconv[6][5][5];//卷积核导数
    double C1_b[6];//bias
    double C1_db[6];//bias
    double C1_d[batch_size][6][28][28];//残差


    //S2层
    double S2[batch_size][6][14][14];//pool后的图像
    double S2_d[batch_size][6][14][14];//残差
    //double S2_w[6];
    //double S2_b[6];
    //double S2_dw[6];
    //double S2_db[6];
    //double S2_mean[batch_size][6][14][14];
    //保存ff时的平均采样值


    //C3层
    double C3[batch_size][16][10][10];
    double C3_conv[6][16][10][10];
    double C3_dconv[6][16][10][10];
    double C3_b[16];
    double C3_db[16];
    double C3_d[batch_size][16][10][10];


    //S4层
    double S4[batch_size][16][5][5];
    double S4_d[batch_size][16][5][5];
    //double S4_w[16];
    //double S4_b[16];
    //double S4_dw[16];
    //double S4_db[16];
    //double S4_mean[batch_size][16][5][5];


    //C5层
    double C5[batch_size][120];
    double C5_conv[16][120][5][5];
    double C5_dconv[16][120][5][5];
    double C5_b[120];
    double C5_db[120];
    double C5_d[batch_size][120];


    //F6层
    //double F6[batch_size][84];
    //double F6_p[120][84];
    //double F6_dp[120][84];
    //double F6_b[84];
    //double F6_db[84];
    //double F6_d[batch_size][84];
    double F6[batch_size][10];
    double F6_p[120][10];
    double F6_dp[120][10];
    double F6_b[10];
    double F6_db[10];
    double F6_d[batch_size][10];


    //F7层
    //double F7[batch_size][10];
    //double F7_p[84][10];
    //double F7_dp[84][10];
    //导数
    //double F7_b[10];
    //double F7_db[10];
    //double F7_d[batch_size][10];
private:
    double sigmoi(double x);
    double derivate_sigmoi(double z);
    double random(); //产生0~1之间的随机数	double getCost_checkgGrad();
};

#endif // CNN_H
