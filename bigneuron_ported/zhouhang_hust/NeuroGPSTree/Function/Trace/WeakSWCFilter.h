#ifndef WEAKSWCFILTER
#define WEAKSWCFILTER
#include "../../ngtypes/basetypes.h"
#include "../ineuronprocessobject.h"
#include "../../ngtypes/volume.h"
class Tree;
class WeakSWCFilter;
#ifdef _WIN32
typedef std::tr1::shared_ptr<WeakSWCFilter> NGWeakSWCFilter;
#else
typedef std::shared_ptr<WeakSWCFilter> NGWeakSWCFilter;
#endif
typedef unsigned char NGCHAR;
template<typename T> class Volume;
typedef Volume<NGCHAR> CVolume;
typedef Volume<unsigned short> SVolume;
typedef Volume<int> IVolume;
typedef Volume<double> DVolume;

class WeakSWCFilter :
	public INeuronProcessObject
{
public:
	static NGWeakSWCFilter New(){return NGWeakSWCFilter(new WeakSWCFilter());}
	WeakSWCFilter();
	~WeakSWCFilter();

	bool Update();
	ConstDataPointer GetOutput();
	DataPointer ReleaseData();

	void SetInputBack(ConstDataPointer p){m_Back = p;}
	void SetInputBin(ConstDataPointer p){m_Bin = p;}
	void SetInputSWC(ConstDataPointer p){m_SWC = p;}

protected:
	void CubesSetLabel(const std::vector<std::vector<VectorVec5d> >& swcCell, const std::vector<size_t>& SWCId, VectorVec3i& filledPtSet);

	void CubesLabel(const std::vector<VectorVec5d>& swcCurve, VectorVec3i& filledPtSet);

	void CubeLabel(const VectorVec3d &curDendrite, VectorVec3i& filledPtSet);

	void CalcParmOfCurveNodeList(const SVolume &origImg, const SVolume &backImg, 
		const VectorVec3d &curNode, std::vector<double> &radius, std::vector<double> &rav);

	void CalcParmOfCurveNode(const SVolume &origImg, const SVolume &backImg, 
		const Vec3d &curNode, double &radius, double &wet);

	void RegiongrowFusedTraces(const VectorVec3i& filledPtSet, double thre);

	void RegionInflationModifyV2(const VectorVec3i &curPoints, IVolume &growLabelMatrix, double threv, VectorVec3i &growPoints);

	void TreeStructureFusedDetection(const std::vector<std::vector<VectorVec5d> >& swcCell, const std::vector<std::vector<size_t> >& connectStruct,
        const std::vector<size_t>& candidateSWCId, std::vector<size_t>& connectionLabel, std::vector<VectorVec5d>& addCurve);

	void TransferVectorVec5d2VectorVec3d(const VectorVec5d& orig, VectorVec3d& dst);

	int TreeStructureFusedDetectionSub(const VectorVec3i& curCurve);

	void RegionFlationBinary(const VectorVec3i& curPoints, VectorVec3i& growPoints);

    void MakeNearestNode2SwcFromColldeCurves( const std::vector<std::vector<VectorVec5d> >& swcCell, 
        const std::vector<size_t>& mainSwcIndList, 
        int detectionNum, int remainCollideSwcInd, int remainCollideCurveInd, 
        VectorVec5d& addSwc );

private:
	//m_Input origimage
	ConstDataPointer m_Bin;
	ConstDataPointer m_Back;
	ConstDataPointer m_SWC;
	IVolume labelBlock;
	CVolume indexImg;

#ifdef _WIN32
	std::tr1::shared_ptr<const Volume<unsigned short> > origImgPointer;
	std::tr1::shared_ptr<const Volume<unsigned short> > backImgPointer;
	std::tr1::shared_ptr<const Volume<unsigned char> > binImgPointer;
#else
	std::shared_ptr<const SVolume > origImgPointer;
	std::shared_ptr<const SVolume > backImgPointer;
	std::shared_ptr<const CVolume > binImgPointer;
#endif
};

#endif
