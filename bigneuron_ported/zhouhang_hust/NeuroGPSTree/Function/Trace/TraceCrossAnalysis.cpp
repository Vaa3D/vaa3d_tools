#include "tracefilter.h"
#include "../../ngtypes/volume.h"
#include "../contourutil.h"
#include "../../ngtypes/volume.h"

//void TraceFilter::DeleteErrorDendConnect(const std::vector<VectorVec5d> &rawLine, const VectorMat2i &rawConnect, const Volume<unsigned short> &origImg, VectorMat2i &resultConnect, VectorVec4d &resultIndex)
//{
//    int nxx = rawLine.size();
//    resultConnect = rawConnect;
//    int tmpCurConnectIndex = 0;
//    int ddst = 0;
//    VectorVec5d headConnectLine;
//    VectorVec5d tailConnectLine;
//    for(int i = 0; i < nxx; ++i){
//        const Mat2i& curConnect = rawConnect[i];
//        Mat2i tmpCurConnect;
//        if(curConnect(0,0) > 0 && curConnect(0,1) > 0 || curConnect(0,0) < 0 && curConnect(0,1) > 0 || curConnect(0,0) > 0
//                && curConnect(0,1) < 0){
//            tmpCurConnect = curConnect;
//            ++ddst;
//            const VectorVec5d& curLine = rawLine[i];
//            Vec2d connectStat = 0.7 * Vec2d(std::abs(double(curConnect(0,0) ) ), std::abs(double(curConnect(0,1))) );
//            Vec2d tmpConnectStat; tmpConnectStat.setZero();
//            MatXd RR_0(2,4), RR_1(2,4);
//            VectorVec5d tmpCurLine;
//            for(VectorVec5d::size_type j = 3; j < curLine.size() -5; ++j){
//                tmpCurLine.push_back(curLine[j]);
//            }
//            CalcRayBurstRadWet(tmpCurLine, origImg, RR_0, RR_1);
//            double pd = 0.0;
//            JudgeHeadTailForDendCurve(RR_0, RR_1, connectStat, pd);
//            double pd1 = 0, pd2 = 0;
//            if(curConnect(0,0) > 0){
//                //VectorVec5d headConnectLine;
//                for(VectorVec5d::size_type j = 3; j < curLine.size() -5; ++j){
//                    headConnectLine.push_back(rawLine[curConnect(0,0)][j]);
//                }
//                CalcRayBurstRadWet(headConnectLine, origImg, RR_0, RR_1);
//                const Mat2i adjecentConnect = rawConnect[curConnect(0,0)];
//                connectStat = 0.5 * Vec2d(std::abs(double(adjecentConnect(0,0) ) ), std::abs(double(adjecentConnect(0,1))) );
//                JudgeHeadTailForDendCurve(RR_0, RR_1, connectStat, pd1);
//            }

//            if(curConnect(0,1) > 0){
//                //VectorVec5d tailConnectLine;
//                for(VectorVec5d::size_type j = 3; j < curLine.size() -5; ++j){
//                    tailConnectLine.push_back(rawLine[curConnect(0,1)][j]);
//                }
//                CalcRayBurstRadWet(tailConnectLine, origImg, RR_0, RR_1);
//                const Mat2i curConnect2 = rawConnect[curConnect(0,1)];
//                connectStat = 0.5 * Vec2d(std::abs(double(curConnect2(0,0) ) ), std::abs(double(curConnect2(0,1))) );
//                JudgeHeadTailForDendCurve(RR_0, RR_1, connectStat, pd2);
//            }

//            if( std::abs(0.0 - pd) < 0.000001 && curConnect(0,0) > 0 && curConnect(0,1) > 0){
//                int nxxs = curLine.size();
//                Vec3d mmt1; mmt1.setZero();
//                for(int inxx = 0; inxx < std::min(nxxs, 8); ++inxx){
//                    mmt1 += (Vec3d(curLine[inxx](0), curLine[inxx](1), curLine[inxx](2)) -
//                                   Vec3d(curLine[1](0), curLine[1](1), curLine[1](2))).normalized();
//                }
//                mmt1.normalize();
//                Vec3d mmt2; mmt2.setZero();
//                for(int inxx = nxxs - 3; inxx < std::max(nxxs - 9, 0); --inxx){
//                    mmt2 += (Vec3d(curLine[inxx](0), curLine[inxx](1), curLine[inxx](2)) -
//                                   Vec3d(curLine[nxxs - 2](0), curLine[1](1), curLine[1](2))).normalized();
//                }
//                mmt2.normalize();
//                Vec3d headVec, tailVec;
//                CalcConnectKnotLocalDirection(headConnectLine, curLine[0], pd1, headVec);
//                CalcConnectKnotLocalDirection(tailConnectLine, curLine[0], pd2, tailVec);

//                double tmpvalue = (mmt2.transpose() * tailVec);
//                tmpvalue -= (mmt1.transpose() * headVec);
//                if( (0.0 <= (mmt2.transpose() * tailVec) && (mmt1.transpose() * headVec) <= 0) &&
//                        std::abs(tmpvalue) > 0.1){
//                    tmpCurConnect << 0, curConnect(0,1), 0,0;
//                    ++tmpCurConnectIndex;
//                    resultIndex.push_back(Vec4d(double(i), curLine[0](0), curLine[0](1), curLine[0](2)));
//                }

//                if( (0.0 >= (mmt2.transpose() * tailVec) && (mmt1.transpose() * headVec) >= 0) &&
//                        std::abs(tmpvalue) > 0.1){
//                    tmpCurConnect << curConnect(0,0),0, 0,0;
//                    ++tmpCurConnectIndex;
//                    int back = curLine.size() - 1;
//                    resultIndex.push_back(Vec4d(double(i), curLine[back](0), curLine[back](1), curLine[back](2)));
//                }
//            }

//            if( std::abs(pd - 1.0) < 0.00001 && curConnect(0,1) > 0){
//                double mm;
//                KnotAnalysis(curLine, rawLine[curConnect(0,1)], 1, 0.85, pd2, mm);
//                if(std::abs(mm - 1.0) < 0.00001){
//                    tmpCurConnect << curConnect(0,0), 0, 0, 0;
//                    ++tmpCurConnectIndex;
//                    int back = curLine.size() - 1;
//                    resultIndex.push_back(Vec4d(double(i), curLine[back](0), curLine[back](1), curLine[back](2)));
//                }
//            }

//            if( std::abs(pd + 1.0) < 0.00001 && curConnect(0,0) > 0){//pd == -1
//                double mm;
//                KnotAnalysis(curLine, rawLine[curConnect(0,0)], 0, 0.85, pd1, mm);
//                if(std::abs(mm - 1.0) < 0.00001){
//                    tmpCurConnect << 0, curConnect(0,1), 0, 0;
//                    ++tmpCurConnectIndex;
//                    resultIndex.push_back(Vec4d(double(i), curLine[0](0), curLine[0](1), curLine[0](2)));
//                }
//            }

//            resultConnect[i] = tmpCurConnect;
//        }

//        if(curConnect(0,0) < 0 && curConnect(0,1) < 0){
//            ++ddst;
//            const VectorVec5d curLine = rawLine[i];
//            VectorVec2d RR;
//            for(VectorVec5d::size_type j = 0; j < curLine.size(); ++j){
//                RR.push_back(Vec2d(curLine[j](3), curLine[j](4)));
//            }
//            int n_rr = RR.size();
//            double Lt1 = 0.0, Lt2 = 0.0;
//            for(int j = 0; j < std::min(20, n_rr); ++j){
//                Lt1 += RR[j](0);
//            }
//            Lt1 /= std::min(20, n_rr);
//            for(int j = std::max(n_rr - 21, 0); j < n_rr; ++j){
//                Lt2 += RR[j](0);
//            }
//            Lt2 /= n_rr - std::max(n_rr - 21, 0);

//            if(Lt1 < Lt2){
//                tmpCurConnect << 0, curConnect(0,1), 0,0;
//                ++tmpCurConnectIndex;
//                resultIndex.push_back(Vec4d(double(i), curLine[0](0), curLine[0](1), curLine[0](2)));
//            }
//            else{
//                tmpCurConnect << curConnect(1,1),0, 0,0;
//                ++tmpCurConnectIndex;
//                resultIndex.push_back(Vec4d(double(i), curLine[0](0), curLine[0](1), curLine[0](2)));
//            }
//            resultConnect[i] = tmpCurConnect;
//        }
//    }
//}

//void TraceFilter::GetBoundaryBack(const std::vector<double> &outer_shell, const double threv, std::vector<double> &boundary_back)
//{
//    size_t nxx = outer_shell.size();
//    boundary_back.clear();
//    std::vector<double> adjust_shell;
//    double tmp;
//    double third = 1.0/3.0;
//    for (size_t i = 0; i < nxx; ++i){
//        tmp = outer_shell[i] < 400.0 ? outer_shell[i] : 400.0;
//        adjust_shell.push_back(tmp);
//    }

//    for (size_t i = 0; i < 100; ++i){
//        for (size_t j = 1; j < nxx - 1; ++j){
//            adjust_shell[j] = third * (adjust_shell[j - 1] + adjust_shell[j] + adjust_shell[j + 1]);
//        }

//        adjust_shell[nxx - 1] = 0.5 * (adjust_shell[nxx - 1] + adjust_shell[nxx - 2]);
//        adjust_shell[0] = 0.5 * (adjust_shell[0] + adjust_shell[1]);
//    }

//    boundary_back = outer_shell;

//    for (size_t i = 0; i < nxx; ++i){
//        if (std::abs(outer_shell[i]-adjust_shell[i]) > threv * std::sqrt(adjust_shell[i])){
//            boundary_back[i] = adjust_shell[i];
//        }
//    }
//}

//void TraceFilter::GetRayLimit(const Volume<double> &sphere_ray_wet, const double constriction_threv, std::vector<std::vector<double> > &ray_limit)
//{
//    int nx = sphere_ray_wet.x();
//    int ny = sphere_ray_wet.y();
//    int nz = sphere_ray_wet.z();
//    std::vector<double> sphere_ray;
//    std::vector<double> tmpUzz;
//    sphere_ray.clear();
//    int arra(0);

//    for (int i = 0; i < ny; ++i){
//        tmpUzz.clear();
//        for (int j = 0; j < nz; ++j){
//            for (int ij = 0; ij < nx; ++ij){
//                sphere_ray.push_back(sphere_ray_wet(ij, i, j));
//            }

//            CalculateOneRayLimit(sphere_ray, constriction_threv, arra);
//            tmpUzz.push_back(arra);
//            sphere_ray.clear();
//        }
//        ray_limit.push_back(tmpUzz);
//    }
//}

//void TraceFilter::GetGradientVectorFlowPPForTrace(const Volume<double> &sphere_ray_wet, Volume<double> &smooth_ray)
//{
//    smooth_ray.SetSize(sphere_ray_wet.x(), sphere_ray_wet.y(), sphere_ray_wet.z());

//    std::vector<double> S_diffttk;

//    for (int i = 0; i < sphere_ray_wet.y(); ++i){//角度1
//        for (int j = 0; j < sphere_ray_wet.z(); ++j){//角度2
//            std::vector<double> tmpSphere;
//            S_diffttk.clear();
//            for (int ij = 0; ij < sphere_ray_wet.x(); ++ij){//长度
//                tmpSphere.push_back((double)sphere_ray_wet(ij,i,j));
//            }

//            SmoothGradientCurvesForTrace(tmpSphere, S_diffttk);

//            for (size_t ij = 0; ij < S_diffttk.size(); ++ij){
//                smooth_ray(ij,i,j) = S_diffttk[ij];
//            }
//        }
//    }
//}

//void TraceFilter::SmoothGradientCurvesForTrace(const std::vector<double> &init_one_ray_wet, std::vector<double> &smooth_one_ray_wet)
//{
//    smooth_one_ray_wet.clear();
//    std::vector<double> one_ray_wet(init_one_ray_wet);
//    std::vector<double>::size_type n_ttk = one_ray_wet.size();
//    //ttk = min(ttk,600);
//    for (size_t i = 0; i < n_ttk; ++i){
//        one_ray_wet[i] = one_ray_wet[i] < 600.0 ? one_ray_wet[i] : 600.0;
//    }

//    std::vector<double> tmp_diff;//原始梯度
//    //diff_vector<double>(one_ray_wet, tmp_diff);//interp1
//    tmp_diff.clear();
//    for (std::vector<double>::size_type i = 0; i < one_ray_wet.size() - 1; ++i){
//        tmp_diff.push_back(one_ray_wet[i + 1] - one_ray_wet[i]);
//    }
//    std::vector<double> diff_ray;

//    //NG_Util::Interpl_2_Mean(tmp_diff, diff_ray);//插值
//    diff_ray.clear();
//    for (std::vector<double>::size_type i = 0; i < tmp_diff.size() - 1; ++i){
//        diff_ray.push_back(tmp_diff[i]);
//        diff_ray.push_back((tmp_diff[i+1] + tmp_diff[i]) / 2.0);
//    }
//    diff_ray.push_back(tmp_diff.back());


//    int n_diff_ttk = diff_ray.size();

//    std::vector<double> tmpone;
//    tmpone.resize(one_ray_wet.size() - 1);
//    std::copy(one_ray_wet.begin(), one_ray_wet.end() - 1, tmpone.begin());
//    std::vector<double> interpl_ray;
//    //NG_Util::Interpl_2_Mean(tmpone, interpl_ray);
//    interpl_ray.clear();
//    for (std::vector<double>::size_type i = 0; i < tmpone.size() - 1; ++i){
//        interpl_ray.push_back(tmpone[i]);
//        interpl_ray.push_back((tmpone[i+1] + tmpone[i]) / 2.0);
//    }
//    interpl_ray.push_back(tmpone.back());

//    interpl_ray.push_back(one_ray_wet.back());

//    //vector<double> yy(diff_ray);
//    std::vector<double> buffer_ray;// = yy;
//    //yy1 = max(-yy, 0);
//    double tmpYY;
//    for (std::vector<double>::size_type i = 0; i < diff_ray.size(); ++i){
//        tmpYY = -diff_ray[i] > 0.0 ? -diff_ray[i] : 0.0;
//        tmpYY *= -100.0 / interpl_ray[1 + i];
//        buffer_ray.push_back( tmpYY );
//    }

//    /*平滑梯度*/
//    for (int j = 1; j < 101; ++j){//只是循环100次
//        for (int i =1; i < n_diff_ttk - 1; ++i){
//            buffer_ray[i] = ((std::abs)(diff_ray[i]) * diff_ray[i]
//            + 2.0 * (diff_ray[i-1]+diff_ray[i+1])) / ((std::abs)(diff_ray[i]) + 4.0);
//            diff_ray = buffer_ray;
//        }
//    }

//    std::vector<int> tts;
//    //generate_n(back_inserter(tts), (int)(diff_ray.size() / 2) + 1,GenArray<int>(0, 2));
//    for(int i = 0; i < (int)(diff_ray.size() / 2) + 1; ++i){
//        tts.push_back(i * 2);//warning this bug
//    }

//    for (std::vector<int>::size_type i = 0; i < tts.size(); ++i){
//        smooth_one_ray_wet.push_back(diff_ray[tts[i]]);
//    }

//    for (size_t i = 0; i < smooth_one_ray_wet.size(); ++i){
//        smooth_one_ray_wet[i] = smooth_one_ray_wet[i] > 0.0? 0.0 : (-smooth_one_ray_wet[i]);
//    }
//}

//void TraceFilter::CalculateOneRayLimit(const std::vector<double> &ray, const double constriction_threv, int &one_ray_limit)
//{
//    size_t nx = ray.size();
//    one_ray_limit = 0;

//    for (size_t i = 0; i < nx; ++i){
//        if (ray[i] < constriction_threv){
//            break;
//        }
//        else{
//            ++one_ray_limit;
//        }
//    }
//}

//void TraceFilter::CalcRayBurstRadWet(const VectorVec5d &curLine, const Volume<unsigned short> &origImg,
//                                 MatXd &radiusBeg, MatXd &radiusEnd)
//{
//    int n_curLine = curLine.size();
//    int conditionPos1 = 0;
//    int nx = origImg.x();
//    int ny = origImg.y();
//    int nz = origImg.z();
//    for(int i = 0; i < n_curLine; ++i){
//        Vec3d curLinePt(curLine[i](0), curLine[i](1), curLine[i](2));
//        if(curLinePt.minCoeff() > 1.0 && curLinePt(0) < nx -3 && curLinePt(1) < ny -3
//           && curLinePt(2) < nx -2){
//            conditionPos1 = i;
//            break;
//        }
//    }
//    int conditionPos2 = 0;
//    for(int i = 0; i < n_curLine; ++i){
//        Vec3d curLinePt(curLine[n_curLine - i - 1](0), curLine[n_curLine - i - 1](1),
//                curLine[n_curLine - i - 1](2));
//        if(curLinePt.minCoeff() > 1.0 && curLinePt(0) < nx -3 && curLinePt(1) < ny -3
//           && curLinePt(2) < nx -2){
//            conditionPos2 = n_curLine - i - 1;
//            break;
//        }
//    }
//    if(conditionPos2 - conditionPos1 > 8){
//        VectorVec3d tmpCurLine;
//        for(int i = conditionPos1; i <= conditionPos2 ; ++i){
//            tmpCurLine.push_back(Vec3d(curLine[i](0), curLine[i](1), curLine[i](2) ));
//        }
//        int n_curLine = tmpCurLine.size();
//        Vec3d mainDirection1, mainDirection2, mainDirection3;
//        VectorVec3d tmpLine;
//        tmpLine.resize(std::min(8, n_curLine));
//        std::copy(tmpCurLine.begin(), tmpCurLine.begin() + tmpLine.size(), tmpLine.begin());
//        Principald(tmpLine, mainDirection1, mainDirection2, mainDirection3);
//        radiusBeg.setZero();
//        for(int i = 0; i < 4; ++ i){
//            double tmpRadius;
//            RayburstRadiusFunc(tmpCurLine[i], origImg, mainDirection1, tmpRadius);
//            radiusBeg(0, i) = radiusBeg(1,i) = tmpRadius;
//        }

//        radiusEnd.setZero();
//        tmpLine.clear();
//        for(int i = n_curLine - 1; i >= std::min(0, n_curLine - 9); --i){
//            tmpLine.push_back(tmpCurLine[i]);
//        }
//        principald(tmpLine, mainDirection1, mainDirection2, mainDirection3);
//        for(int i = 0; i < 4; ++ i){
//            double tmpRadius;
//            RayburstRadiusFunc(tmpCurLine[n_curLine - i - 1], origImg, mainDirection1, tmpRadius);
//            radiusEnd(0, i) = radiusBeg(1,i) = tmpRadius;
//        }
//    }
//    else{
//        radiusBeg.setZero();
//        radiusEnd.setZero();
//    }
//}

//void TraceFilter::CalcRayBurstOnePtRadWet(const Vec3d &curLinePt, const Volume<unsigned short> &origImg, const Vec3d &mainDirection1, Vec2d &aa1)
//{
////    const Vec3d &L_data = curLinePt;
////    double slice = 0.3;//(double)(minLen) / 82.0;
////    const int blocksum = 26;//41;
////    const int Theta = 20, Phi = 10;
////    int subx = origImg.x();
////    int suby = origImg.y();
////    int subz = origImg.z();
////    std::vector<double> r0;
////    for(int i = 0; i < blocksum; ++i){
////        r0.push_back(slice * i);
////    }
////    //generate_n(back_inserter(r0), blocksum, GenArray<double>(0.0, slice));//41个

////    /*这里是求轮廓区域*/
////    Volume<double> Sphere_XX; Sphere_XX.SetSize(r0.size(), Theta, Phi);
////    const double a = 360.0 / (double)Theta;
////    const double b = 180.0 / (double)Phi;
////    const double PI_180 = 3.141592654 / 180.0;

////    //double prev_Density(0.0);
////    double dd(0.0), ww(0.0);
////    double x,y,z;

////    for (std::vector<double>::size_type k = 0; k < r0.size(); ++k)//0
////    {
////        for (int i = 1; i <= Theta; ++i)
////        {
////            for (int j = 1; j <= Phi; ++j)
////            {
////                x = r0[k] * std::sin(b * (double)j * PI_180) * std::cos(a * (double)i * PI_180) + L_data(0);
////                y = r0[k] * std::sin(b * (double)j * PI_180) * std::sin(a * (double)i * PI_180) + L_data(1);
////                z = r0[k] * std::cos(b * (double)j * PI_180) + L_data(2);
////                dd = ww = 0.0;

////                ContourUtil::CalculateSphereOneNode(origImg, subx, suby, subz, 1.0, x, y, z, dd, ww);

////                Sphere_XX(k, i-1, j-1) = (double)(dd / (ww + 0.0001));
////            }
////        }
////    }//for

////    //列存储
////    int lenR0_1 = int(r0.size()) - 1;
////    std::vector<double> outer_shell;
////    for (int j = 0; j < Phi; ++j){
////        for (int i = 0; i < Theta; ++i){
////            outer_shell.push_back(Sphere_XX(lenR0_1, i, j));
////        }
////    }

////    std::vector<double> boundary_back;
////    GetBoundaryBack(outer_shell, 4, boundary_back);

////    //three_vs = mean(Lssx1)+3.5*std(Lssx1)
////    double constriction_threv(0.0);
////    double constriction_threv_mean =
////        std::accumulate(boundary_back.begin(), boundary_back.end(), 0.0) / boundary_back.size();
////    //求标准差---------------------------------------
////    double constriction_threv_mean_sqrt_sum(0);
////    size_t num_boundary_back = boundary_back.size();
////    for (size_t i = 0; i< num_boundary_back; ++i)
////    {
////        constriction_threv_mean_sqrt_sum +=
////            (boundary_back[i] - constriction_threv_mean) * (boundary_back[i] - constriction_threv_mean);
////    }
////    constriction_threv = 4.5 * std::sqrt(constriction_threv_mean_sqrt_sum / (boundary_back.size() - 1));//warning!4.5
////    constriction_threv += constriction_threv_mean;
////    for (int i = 0; i < Phi; ++i){
////        for (int j = 0; j < Theta; ++j)
////            Sphere_XX(lenR0_1, j, i) = boundary_back[i * Theta + j];
////    }


////    std::vector<std::vector<double> > ray_limit;
////    GetRayLimit(Sphere_XX, constriction_threv, ray_limit);//three_vs, Uzz
////    Volume<double> smooth_ray;//Uz
////    //    gradientVectorFlowPP_Trace(Sphere_XX, Uz);
////    GradientVectorFlowPP_Trace(Sphere_XX, smooth_ray);

////    MatXd Ux(smooth_ray.x(), smooth_ray.y());
////    for(int i = 0; i < smooth_ray.x(); ++i){
////        for(int j = 0; j < smooth_ray.y(); ++j){
////            Ux(i,j) = smooth_ray(i,j,0);
////        }
////    }


////    std::vector<double> idexxS;//分段长度
//////    generate_n(back_inserter(idexxS), blocksum, GenArray<double>(1.0, 1.0));//注意这里要同步
////    for(std::vector<double>::size_type i = 1; i <= r0.size(); ++i){
////        idexxS.push_back(i);
////    }

////    double pp0(0);
//////    vector<double> TT(Uz.x());
//////    vector<double> jjt(Uz.x());
////    std::vector<double> TT(smooth_ray.x());
////    std::vector<double> jjt(smooth_ray.x());

////    double reduce_smooth[2];
////    reduce_smooth[0] = 0.9;
////    reduce_smooth[1] = 1.0 - reduce_smooth[0];

////    int repeat = 10;

////    for (int jj = 0; jj < repeat;++jj){
////        for (int i = 1; i <= Theta; ++i){
////            for (int j = 1; j <= Phi; ++j){
////                pp0 = Ux(i, j);
////                for (int ij = 0; ij < smooth_ray.x(); ++ij){
////                    TT[ij] = smooth_ray(ij, i - 1, j - 1);
////                    jjt[ij] = TT[ij] * std::exp( -0.05 *std::pow( idexxS[ij] - pp0, 2.0));
////                }

////                Ux(i,j) += reduce_smooth[0] * ( ( (std::inner_product(idexxS.begin(), idexxS.end(), jjt.begin(), 0.0)) /
////                                                  (std::accumulate(jjt.begin(), jjt.end(), 0.0) ) ) - Ux(i,j) )
////                        - reduce_smooth[1] * (4.0 * Ux(i,j) - Ux(i-1,j) - Ux(i+1,j) - Ux(i,j-1) - Ux(i,j+1));
////                Ux(i,j) = std::min(Ux(i, j ) , Uzz(i - 1, j - 1);

////            }
////            Ux = (Ux.array() > 0.0).select(Ux, 0.0);
////            Ux.row(0) = Ux.row(2);
////            Ux.row(Theta + 1) = Ux.row(Theta - 1);
////            Ux.col(0) = Ux.col(2);
////            Ux.col(Phi + 1) = Ux.col(Phi - 1);
////        }
////    }//for

////    double kk = 0;
////    double kk_Length = 0.0;
////    Vec3d diffDirection;//direcs00....wtf
////    for(int j = 1; j <= Phi; ++ j){
////        for(int i = 1; i < Theta; ++i){
////            diffDirection << std::sin(b * j * PI_180) * std::cos(a * i * PI_180),
////                    std::sin(b * j * PI_180) * std::sin(a * i * PI_180),
////                    std::cos(b * j * PI_180);
////            if(std::abs(diffDirection * mainDirection1) < 0.15){
////                kk_Length += 0.3 * std::max(Ux(i,j), 0.0);
////                ++kk;
////            }
////        }
////    }
////    aa1.setZero();
////    VectorVec3d curLinePts;
////    curLinePts.push_back(curLinePt);
////    std::vector<double> aa_data;
////    WeightValue(curLinePts, origImg, aa_data);
////    aa1 << std::max(kk_Length / std::max(1.0, kk), 0.1), aa_data[0];
//}

//void TraceFilter::JudgeHeadTailForDendCurve(const MatXd &RR0, const MatXd &RR1, const Vec2d &connectStat, double &parentStat)
//{
//    parentStat = 0;
//    double r0 = 0, r1 = 0, v0 = 0, v1 = 0;
//    for(int i = 0; i < 4; ++i){
//        r0 += RR0(0, i);
//        r1 += RR1(0, i);
//        v0 += RR0(1,i);
//        v1 += RR1(1,i);
//    }
//    r0 /= 4.0;
//    r1 /= 4.0;
//    v0 /= 4.0;
//    v1 /= 4.0;
//    double v = std::max(std::min(v0, v1), 1.0);
//    double F1 = r0 + 0.5 * v0 / v + connectStat(0);
//    double F2 = r1 + 0.5 * v1 / v + connectStat(1);
//    if(F1 > F2 + 0.5){
//        parentStat = 1;
//    }
//    if(F1 + 0.5 < F2){
//        parentStat = -1;
//    }
//}

//void TraceFilter::KnotAnalysis(const VectorVec5d &curLine, const VectorVec5d &adjecentLine, double k, double threv, double pd, double &mm)
//{

//}

//void TraceFilter::CalcConnectKnotLocalDirection(const VectorVec5d &adjecentLine, const Vec5d &curLineNode, double knotStat, Vec3d &resultDirection)
//{

//}

//void TraceFilter::RayBurstSampling(const Volume<double> &Sphere_XX, const double three_vs,
//                                   std::vector<std::vector<double> > &Uzz)
//{
//    int nx = Sphere_XX.x();
//    int ny = Sphere_XX.y();
//    int nz = Sphere_XX.z();
//    std::vector<double> LLs;
//    std::vector<double> tmpUzz;
//    LLs.clear();
//    int arra(0);

//    for (int i = 0; i < ny; ++i){
//        tmpUzz.clear();
//        for (int j = 0; j < nz; ++j){
//            for (int ij = 0; ij < nx; ++ij){
//                LLs.push_back(Sphere_XX(ij, i, j));
//            }
//            RayBurstSampling1(LLs, three_vs, arra);
//            tmpUzz.push_back(arra);
//        }
//        Uzz.push_back(tmpUzz);
//    }
//}


//void TraceFilter::RayBurstSampling1(const std::vector<double> &LLs, const double three_vs, int &arra)
//{
//    int nx = LLs.size();
//    arra = 0;

//    for (int i = 0; i < nx; ++i){
//        if (LLs[i] < three_vs){
//            break;
//        }
//        else{
//            ++arra;
//        }
//    }
//}

//void TraceFilter::Principald(const VectorVec3d &dataL, Vec3d &x1, Vec3d &x2, Vec3d &x3)
//{
//    int nx = int(dataL.size());
//    Vec3d nn;
//    nn.setZero();
//    double tmp(0.0);
//    Vec3d tmpVec;
//    for (int i = 0; i < nx - 1; ++i){
//        tmpVec = dataL[i + 1] - dataL[i];
//        tmp = tmpVec.norm();
//        nn += tmp * tmpVec;
//    }
//    x1 = nn.normalized();
//    CalcOrthoBasis(x1, x2, x3);
//}

//void TraceFilter::WeighRayValue(const VectorVec3d &dataL1, const Volume<unsigned char> &L_XX3, std::vector<double> &aa_data)
//{
//    //typedef mydouble mydist;
////    size_t nxss = dataL1.size();//
////    aa_data.clear();

////    size_t nx = L_XX3.x();
////    size_t ny = L_XX3.y();
////    size_t nz = L_XX3.z();

////    //坐标
////    typedef double spheredist;
////    spheredist x,y,z;

////    spheredist dd, ww;//, w1;

////    for (size_t i = 0; i < nxss; ++i)
////    {
////        x = dataL1[i](0);
////        y = dataL1[i](1);
////        z = dataL1[i](2);

////        dd = ww = 0.0;

////        ContourUtil::CalculateSphereOneNode(L_XX3, nx, ny, nz, 0.05, x, y, z, dd, ww);

////        aa_data.push_back(dd / (ww + 0.0001));
////    }
//}

