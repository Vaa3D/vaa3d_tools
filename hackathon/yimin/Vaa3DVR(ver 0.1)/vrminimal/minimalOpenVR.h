#pragma once
/**
  \file minimalOpenGL/minimalOpenVR.h
  \author Morgan McGuire, http://casual-effects.com
  Distributed with the G3D Innovation Engine http://casual-effects.com/g3d

  Minimal headers for including Valve's OpenVR / SteamVR API, 
  which currently supports Vive and Oculus Virtual Reality 
  head mounted displays (HMDs). This does not depend on any 
  vector math library or specific OpenGL intialization library.

  This requires the bin, lib, and headers directories from the 
  OpenVR SDK (which are also distributed with G3D):
   
     https://github.com/ValveSoftware/openvr

  The runtime for OpenVR is distributed with Steam. Ensure that 
  you've run Steam and let it update to the latest SteamVR before
  running an OpenVR program.
*/

#include "openvr.h"
#include <string>

#ifdef _WINDOWS
#   pragma comment(lib, "openvr_api")
#endif

/** Called by initOpenVR */
std::string getHMDString(vr::IVRSystem* pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError = nullptr) {
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, nullptr, 0, peError);
	if (unRequiredBufferLen == 0) {
	    return "";
    }

	char* pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;

	return sResult;
}


/** Call immediately before initializing OpenGL 

    \param hmdWidth, hmdHeight recommended render target resolution
*/
vr::IVRSystem* initOpenVR(uint32_t& hmdWidth, uint32_t& hmdHeight) {
	vr::EVRInitError eError = vr::VRInitError_None;
	vr::IVRSystem* hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);

	if (eError != vr::VRInitError_None) {
        fprintf(stderr, "OpenVR Initialization Error: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
        return nullptr;
	}
    
	const std::string& driver = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	const std::string& model  = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ModelNumber_String);
	const std::string& serial = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);
    const float freq = hmd->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);

    //get the proper resolution of the hmd
    hmd->GetRecommendedRenderTargetSize(&hmdWidth, &hmdHeight);

    fprintf(stderr, "HMD: %s '%s' #%s (%d x %d @ %g Hz)\n", driver.c_str(), model.c_str(), serial.c_str(), hmdWidth, hmdHeight, freq);

    // Initialize the compositor
    vr::IVRCompositor* compositor = vr::VRCompositor();
	if (! compositor) {
		fprintf(stderr, "OpenVR Compositor initialization failed. See log file for details\n");
        vr::VR_Shutdown();
        assert("VR failed" && false);
	}

    return hmd;
}


/**
 */
void getEyeTransformations
   (vr::IVRSystem*  hmd,
    vr::TrackedDevicePose_t* trackedDevicePose,
    float           nearPlaneZ,
    float           farPlaneZ,
    float*          headToWorldRowMajor3x4,
    float*          ltEyeToHeadRowMajor3x4, 
    float*          rtEyeToHeadRowMajor3x4,
    float*          ltProjectionMatrixRowMajor4x4, 
    float*          rtProjectionMatrixRowMajor4x4) {

    assert(nearPlaneZ < 0.0f && farPlaneZ < nearPlaneZ);

    vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

#   if defined(_DEBUG) && 0
        fprintf(stderr, "Devices tracked this frame: \n");
        int poseCount = 0;
	    for (int d = 0; d < vr::k_unMaxTrackedDeviceCount; ++d)	{
		    if (trackedDevicePose[d].bPoseIsValid) {
			    ++poseCount;
			    switch (hmd->GetTrackedDeviceClass(d)) {
                case vr::TrackedDeviceClass_Controller:        fprintf(stderr, "   Controller: ["); break;
                case vr::TrackedDeviceClass_HMD:               fprintf(stderr, "   HMD: ["); break;
                case vr::TrackedDeviceClass_Invalid:           fprintf(stderr, "   <invalid>: ["); break;
                case vr::TrackedDeviceClass_Other:             fprintf(stderr, "   Other: ["); break;
                case vr::TrackedDeviceClass_TrackingReference: fprintf(stderr, "   Reference: ["); break;
                default:                                       fprintf(stderr, "   ???: ["); break;
			    }
                for (int r = 0; r < 3; ++r) { 
                    for (int c = 0; c < 4; ++c) {
                        fprintf(stderr, "%g, ", trackedDevicePose[d].mDeviceToAbsoluteTracking.m[r][c]);
                    }
                }
                fprintf(stderr, "]\n");
		    }
	    }
        fprintf(stderr, "\n");
#   endif

    assert(trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid);
    const vr::HmdMatrix34_t head = trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;

    const vr::HmdMatrix34_t& ltMatrix = hmd->GetEyeToHeadTransform(vr::Eye_Left);
    const vr::HmdMatrix34_t& rtMatrix = hmd->GetEyeToHeadTransform(vr::Eye_Right);

    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 4; ++c) {
            ltEyeToHeadRowMajor3x4[r * 4 + c] = ltMatrix.m[r][c];
            rtEyeToHeadRowMajor3x4[r * 4 + c] = rtMatrix.m[r][c];
            headToWorldRowMajor3x4[r * 4 + c] = head.m[r][c];
        }
    }

    const vr::HmdMatrix44_t& ltProj = hmd->GetProjectionMatrix(vr::Eye_Left,  -nearPlaneZ, -farPlaneZ);//, vr::API_OpenGL);
    const vr::HmdMatrix44_t& rtProj = hmd->GetProjectionMatrix(vr::Eye_Right, -nearPlaneZ, -farPlaneZ);//, vr::API_OpenGL);

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            ltProjectionMatrixRowMajor4x4[r * 4 + c] = ltProj.m[r][c];
            rtProjectionMatrixRowMajor4x4[r * 4 + c] = rtProj.m[r][c];
        }
    }
}


/** Call immediately before OpenGL swap buffers */
void submitToHMD(GLint ltEyeTexture, GLint rtEyeTexture, bool isGammaEncoded) {
    const vr::EColorSpace colorSpace = isGammaEncoded ? vr::ColorSpace_Gamma : vr::ColorSpace_Linear;

    const vr::Texture_t lt = { reinterpret_cast<void*>(intptr_t(ltEyeTexture)), vr::TextureType_OpenGL, colorSpace };
    vr::VRCompositor()->Submit(vr::Eye_Left, &lt);

    const vr::Texture_t rt = { reinterpret_cast<void*>(intptr_t(rtEyeTexture)), vr::TextureType_OpenGL, colorSpace };
    vr::VRCompositor()->Submit(vr::Eye_Right, &rt);

    // Tell the compositor to begin work immediately instead of waiting for the next WaitGetPoses() call
    vr::VRCompositor()->PostPresentHandoff();
}
