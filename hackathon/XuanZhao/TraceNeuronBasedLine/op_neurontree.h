#ifndef OP_NEURONTREE_H
#define OP_NEURONTREE_H

#include "v3d_interface.h"
#include <vector>

using namespace std;

/**
 * @brief setNeuronTreeHash 设置NeuronTree的hash，n与索引一一对应
 * @param nt 需要进行操作的NeuronTree
 */
void setNeuronTreeHash(NeuronTree& nt);

/**
 * @brief shiftAndScaleNeuronTree 对神经元进行偏移和缩放操作，这里默认先进行缩放再进行偏移
 * @param nt 需要进行操作的原神经元
 * @param d_xyz 偏移的xyz的具体数值
 * @param scale 缩放的数值
 * @return 进行偏移和缩放操作之后的神经元
 */
NeuronTree shiftAndScaleNeuronTree(const NeuronTree& nt, const XYZ& d_xyz, const float scale);

/**
 * @brief shiftAndScaleNeuronTree 将神经元的坐标转化成切图图像的相对坐标
 * @param nt 需要进行操作的原神经元
 * @param center 切图图像的中心坐标（在整个大脑的坐标系下的坐标）
 * @param box 切图图像的大小
 * @param scale 切图图像相对于原始大脑的分辨率
 * @return 转化了坐标之后的神经元
 */
NeuronTree shiftAndScaleNeuronTree(const NeuronTree& nt, const XYZ& center, const XYZ& box,  const float scale);

/**
 * @brief cropNeuronTree 将神经元在一个box中的点crop出来
 * @param nt 需要进行操作的原神经元
 * @param min_xyz box的较小坐标
 * @param max_xyz box的较大坐标
 * @return crop之后的神经元
 */
NeuronTree cropNeuronTree(const NeuronTree& nt, const XYZ& min_xyz, const XYZ& max_xyz);

/**
 * @brief refineNeuronTree 将神经元整体移到信号中心，消除因为分辨率不同而造成的误差
 * @param nt 需要进行操作的原神经元
 * @param pdata 图像的一维数据
 * @param sz 图像的维度大小，x，y，z，channel
 * @return 移到信号中心的神经元
 */
NeuronTree refineNeuronTree(const NeuronTree& nt, unsigned char* pdata, V3DLONG* sz);

/**
 * @brief splitNeuronTree 将离散的swc分成独立的single tree
 * @param nt 需要进行操作的原神经元
 * @return 一组独立的神经元（single tree）
 */
vector<NeuronTree> splitNeuronTree(const NeuronTree& nt);


#endif // OP_NEURONTREE_H
