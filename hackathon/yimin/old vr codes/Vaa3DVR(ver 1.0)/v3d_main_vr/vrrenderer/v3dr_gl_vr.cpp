//========= Copyright Valve Corporation ============//

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include "./v3dr_gl_vr.h"
#if defined( OSX )
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include <OpenGL/glu.h>
// Apple's version of glut.h #undef's APIENTRY, redefine it
#define APIENTRY
#else
#include <GL/glu.h>
#endif
#include <stdio.h>
#include <string>
#include <cstdlib>

#include <openvr.h>

#include "Matrices.h"//todo-yimin: this header is removable

#include "shader_m.h"
#include "Sphere.h"
#include "Cylinder.h" 

#if defined(POSIX)
#include "unistd.h"
#endif

#ifndef _WIN32
#define APIENTRY
#endif

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif


NeuronTree loadedNT,sketchNT;
long int vertexcount =0,swccount = 0;
#define dist_thres 1
#define default_radius 0.01



void ThreadSleep( unsigned long nMilliseconds )
{
#if defined(_WIN32)
	::Sleep( nMilliseconds );
#elif defined(POSIX)
	usleep( nMilliseconds * 1000 );
#endif
}

class CGLRenderModel
{
public:
	CGLRenderModel( const std::string & sRenderModelName );
	~CGLRenderModel();

	bool BInit( const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture );
	void Cleanup();
	void Draw();
	const std::string & GetName() const { return m_sModelName; }

private:
	GLuint m_glVertBuffer;
	GLuint m_glIndexBuffer;
	GLuint m_glVertArray;
	GLuint m_glTexture;
	GLsizei m_unVertexCount;
	std::string m_sModelName;
};

static bool g_bPrintf = true;

//-----------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
class CMainApplication
{
public:
	CMainApplication(int argc = 0, char *argv[] = 0);
	virtual ~CMainApplication();

	bool BInit();
	bool BInitGL();
	bool BInitCompositor();

	void SetupRenderModels();

	void Shutdown();

	void RunMainLoop();
	bool HandleInput();
	void ProcessVREvent( const vr::VREvent_t & event );
	void RenderFrame();

	void SetupMorphologyLine(int drawMode);
	void SetupMorphologyLine(NeuronTree neuron_Tree,GLuint& LineModeVAO, GLuint& LineModeVBO, GLuint& LineModeIndex,unsigned int& Vertcount,int drawMode);
	void SetupMorphologySurface(NeuronTree neurontree,vector<Sphere*>& spheres,vector<Cylinder*>& cylinders,vector<glm::vec3>& spheresPos);

	void RenderControllerAxes();

	bool SetupStereoRenderTargets();
	void SetupCompanionWindow();
	void SetupCameras();
	void SetupCamerasForMorphology();

	void SetupBoundingBox();//2017-06-11 currently not used.
	void NormalizeNeuronTree(NeuronTree& nt);
	void RenderStereoTargets();
	void RenderCompanionWindow();
	void RenderScene( vr::Hmd_Eye nEye );

	Matrix4 GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye );
	Matrix4 GetHMDMatrixPoseEye( vr::Hmd_Eye nEye );
	Matrix4 GetCurrentViewProjectionMatrix( vr::Hmd_Eye nEye );
	void UpdateHMDMatrixPose();

	Matrix4 ConvertSteamVRMatrixToMatrix4( const vr::HmdMatrix34_t &matPose );

	GLuint CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader );
	bool CreateAllShaders();

	void SetupRenderModelForTrackedDevice( vr::TrackedDeviceIndex_t unTrackedDeviceIndex );
	CGLRenderModel *FindOrLoadRenderModel( const char *pchRenderModelName );

private: 
	bool m_bDebugOpenGL;
	bool m_bVerbose;
	bool m_bPerf;
	bool m_bVblank;
	bool m_bGlFinishHack;

	vr::IVRSystem *m_pHMD;
	vr::IVRRenderModels *m_pRenderModels;
	std::string m_strDriver;
	std::string m_strDisplay;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ]; //note: contain everything: validity, matrix, ...
	Matrix4 m_rmat4DevicePose[ vr::k_unMaxTrackedDeviceCount ]; //note: store device transform matrices, copied from m_rTrackedDevicePose
	bool m_rbShowTrackedDevice[ vr::k_unMaxTrackedDeviceCount ];

private: // SDL bookkeeping
	SDL_Window *m_pCompanionWindow;
	uint32_t m_nCompanionWindowWidth;
	uint32_t m_nCompanionWindowHeight;

	SDL_GLContext m_pContext;

private: // OpenGL bookkeeping
	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	bool m_bShowMorphologyLine;
	bool m_bShowMorphologySurface;
	

	std::string m_strPoseClasses;                            // what classes we saw poses for this frame
	char m_rDevClassChar[ vr::k_unMaxTrackedDeviceCount ];   // for each device, a character representing its class


	
	float m_fNearClip;

	float m_fFarClip;

	//unsigned int m_uiVertcount;

	//VAO/VBO for surface and line of loaded neuron
	vector<Sphere*> loaded_spheres;
	vector<Cylinder*> loaded_cylinders;
	vector<glm::vec3> loaded_spheresPos;

	GLuint m_unMorphologyLineModeVAO;
	GLuint m_glMorphologyLineModeVertBuffer;
	GLuint m_glMorphologyLineModeIndexBuffer;
	unsigned int m_uiMorphologyLineModeVertcount;

	//VAO/VBO for surface and line of loaded neuron
	vector<Sphere*> sketch_spheres;
	vector<Cylinder*> sketch_cylinders;
	vector<glm::vec3> sketch_spheresPos;

	GLuint m_unSketchMorphologyLineModeVAO;
	GLuint m_glSketchMorphologyLineModeVertBuffer;
	GLuint m_glSketchMorphologyLineModeIndexBuffer;
	unsigned int m_uiSketchMorphologyLineModeVertcount;

	GLuint m_unCompanionWindowVAO; //two 2D boxes
	GLuint m_glCompanionWindowIDVertBuffer;
	GLuint m_glCompanionWindowIDIndexBuffer;
	unsigned int m_uiCompanionWindowIndexSize;

	GLuint m_glControllerVertBuffer;
	GLuint m_unControllerVAO;//note: axes for controller
	unsigned int m_uiControllerVertcount;

	Matrix4 m_mat4HMDPose;//note: m_rmat4DevicePose[hmd].invert()
	Matrix4 m_mat4eyePosLeft;
	Matrix4 m_mat4eyePosRight;

	Matrix4 m_mat4ProjectionCenter;
	Matrix4 m_mat4ProjectionLeft;
	Matrix4 m_mat4ProjectionRight;

	//for morphology rendering
	glm::mat4 m_HMDTrans;
	glm::mat4 m_EyeTransLeft;//head to eye
	glm::mat4 m_EyeTransRight; 
	glm::vec3 m_EyePosLeft;
	glm::vec3 m_EyePosRight;
	glm::mat4 m_ProjTransLeft;
	glm::mat4 m_ProjTransRight;

	//BoundingBox swcBB;
	glm::mat4 m_GlobalTransformation;//move neuron to center of the world

	struct VertexDataScene//question: why define this? only used for sizeof()
	{
		Vector3 position;
		Vector2 texCoord;
	};

	struct VertexDataWindow//question: companion window just uses the projected data points from HMD?
	{
		Vector2 position;
		Vector2 texCoord;

		VertexDataWindow( const Vector2 & pos, const Vector2 tex ) :  position(pos), texCoord(tex) {	}
	};

	Shader* morphologyShader;
	GLuint m_unCompanionWindowProgramID;
	GLuint m_unControllerTransformProgramID;
	GLuint m_unRenderModelProgramID;

	GLint m_nControllerMatrixLocation;
	GLint m_nRenderModelMatrixLocation;

	struct FramebufferDesc
	{
		GLuint m_nDepthBufferId;
		GLuint m_nRenderTextureId;
		GLuint m_nRenderFramebufferId;
		GLuint m_nResolveTextureId;
		GLuint m_nResolveFramebufferId;
	};
	FramebufferDesc leftEyeDesc;
	FramebufferDesc rightEyeDesc;

	bool CreateFrameBuffer( int nWidth, int nHeight, FramebufferDesc &framebufferDesc );
	
	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;

	std::vector< CGLRenderModel * > m_vecRenderModels; //note: a duplicated access to below. used in shutdown destroy, check existence routines;
	CGLRenderModel *m_rTrackedDeviceToRenderModel[ vr::k_unMaxTrackedDeviceCount ]; //note: maintain all the render models for VR devices; used in drawing
};

//-----------------------------------------------------------------------------
// Purpose: Outputs a set of optional arguments to debugging output, using
//          the printf format setting specified in fmt*.
//-----------------------------------------------------------------------------
void dprintf( const char *fmt, ... )
{
	va_list args;
	char buffer[ 2048 ];

	va_start( args, fmt );
	vsprintf_s( buffer, fmt, args );
	va_end( args );

	if ( g_bPrintf )
		printf( "%s", buffer );

	OutputDebugStringA( buffer );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMainApplication::CMainApplication( int argc, char *argv[] )
	: m_pCompanionWindow(NULL)
	, m_pContext(NULL)
	, m_nCompanionWindowWidth( 640 )
	, m_nCompanionWindowHeight( 320 )
	, morphologyShader ( NULL )
	, m_unCompanionWindowProgramID( 0 )
	, m_unControllerTransformProgramID( 0 )
	, m_unRenderModelProgramID( 0 )
	, m_pHMD( NULL )
	, m_pRenderModels( NULL )
	, m_bDebugOpenGL( false )
	, m_bVerbose( false )
	, m_bPerf( false )
	, m_bVblank( false )
	, m_bGlFinishHack( true )
	, m_glControllerVertBuffer( 0 )
	, m_unControllerVAO( 0 )
	, m_unMorphologyLineModeVAO( 0 )
	, m_uiMorphologyLineModeVertcount(0)
	, m_unSketchMorphologyLineModeVAO( 0 )
	, m_uiSketchMorphologyLineModeVertcount(0)
	, m_nControllerMatrixLocation( -1 )
	, m_nRenderModelMatrixLocation( -1 )
	, m_iTrackedControllerCount( 0 )
	, m_iTrackedControllerCount_Last( -1 )
	, m_iValidPoseCount( 0 )
	, m_iValidPoseCount_Last( -1 )
	, m_strPoseClasses("")
	, m_bShowMorphologyLine(true)
	, m_bShowMorphologySurface(false)

{

	for( int i = 1; i < argc; i++ )
	{
		if( !stricmp( argv[i], "-gldebug" ) )
		{
			m_bDebugOpenGL = true;
		}
		else if( !stricmp( argv[i], "-verbose" ) )
		{
			m_bVerbose = true;
		}
		else if( !stricmp( argv[i], "-novblank" ) )
		{
			m_bVblank = false;
		}
		else if( !stricmp( argv[i], "-noglfinishhack" ) )
		{
			m_bGlFinishHack = false;
		}
		else if( !stricmp( argv[i], "-noprintf" ) )
		{
			g_bPrintf = false;
		}
	}
	// other initialization tasks are done in BInit
	memset(m_rDevClassChar, 0, sizeof(m_rDevClassChar));
};


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMainApplication::~CMainApplication()
{
	// work is done in Shutdown
	dprintf( "Shutdown" );
}


//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
std::string GetTrackedDeviceString( vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL )
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, NULL, 0, peError );
	if( unRequiredBufferLen == 0 )
		return "";

	char *pchBuffer = new char[ unRequiredBufferLen ];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, pchBuffer, unRequiredBufferLen, peError );
	std::string sResult = pchBuffer;
	delete [] pchBuffer;
	return sResult;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::BInit()//wwbmark ≥ı ºªØ
{
	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0 )
	{
		printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	// Loading the SteamVR Runtime
	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Scene );

	if ( eError != vr::VRInitError_None )
	{
		m_pHMD = NULL;
		char buf[1024];
		sprintf_s( buf, sizeof( buf ), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL );
		return false;
	}


	m_pRenderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface( vr::IVRRenderModels_Version, &eError );
	//question: difference between IVRRenderModels and the CGLRenderModel?
	//is IVRRenderModels necessary for VR? comment it and observe the change
	if( !m_pRenderModels )
	{
		m_pHMD = NULL;
		vr::VR_Shutdown();

		char buf[1024];
		sprintf_s( buf, sizeof( buf ), "Unable to get render model interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL );
		return false;
	}

	int nWindowPosX = 700;
	int nWindowPosY = 100;
	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );
	if( m_bDebugOpenGL )
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );

	m_pCompanionWindow = SDL_CreateWindow( "Vaa3D VR", nWindowPosX, nWindowPosY, m_nCompanionWindowWidth, m_nCompanionWindowHeight, unWindowFlags );
	if (m_pCompanionWindow == NULL)
	{
		printf( "%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
		return false;
	}

	m_pContext = SDL_GL_CreateContext(m_pCompanionWindow);
	if (m_pContext == NULL)
	{
		printf( "%s - OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
		return false;
	}

	glewExperimental = GL_TRUE;
	GLenum nGlewError = glewInit();
	if (nGlewError != GLEW_OK)
	{
		printf( "%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString( nGlewError ) );
		return false;
	}
	glGetError(); // to clear the error caused deep in GLEW

	if ( SDL_GL_SetSwapInterval( m_bVblank ? 1 : 0 ) < 0 )
	{
		printf( "%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
		return false;
	}


	m_strDriver = "No Driver";
	m_strDisplay = "No Display";

	m_strDriver = GetTrackedDeviceString( m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String );
	m_strDisplay = GetTrackedDeviceString( m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String );

	std::string strWindowTitle = "Vaa3D VR - " + m_strDriver + " " + m_strDisplay;
	SDL_SetWindowTitle( m_pCompanionWindow, strWindowTitle.c_str() );
	
	m_fNearClip = 0.1f;

 	m_fFarClip = 30.0f;

 	//m_uiVertcount = 0;
	




// 		m_MillisecondsTimer.start(1, this);
// 		m_SecondsTimer.start(1000, this);
	
	if (!BInitGL())
	{
		printf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
		return false;
	}

	if (!BInitCompositor())
	{
		printf("%s - Failed to initialize VR Compositor!\n", __FUNCTION__);
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Outputs the string in message to debugging output.
//          All other parameters are ignored.
//          Does not return any meaningful value or reference.
//-----------------------------------------------------------------------------
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	dprintf( "GL Error: %s\n", message );
}


//-----------------------------------------------------------------------------
// Purpose: Initialize OpenGL. Returns true if OpenGL has been successfully
//          initialized, false if shaders could not be created.
//          If failure occurred in a module other than shaders, the function
//          may return true or throw an error. 
//-----------------------------------------------------------------------------
bool CMainApplication::BInitGL()
{
	if( m_bDebugOpenGL )
	{
		glDebugMessageCallback( (GLDEBUGPROC)DebugCallback, nullptr);
		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
	
	if( !CreateAllShaders() )
		return false;

	
	//printf("before normalize\n");
	//for(int i = 0;i<loadedNT.listNeuron.size();i++)
	//{
	//	NeuronSWC S=loadedNT.listNeuron.at(i);
	//	printf("%f, %f, %f, %f\n", S.x,S.y,S.z,S.r);
	//}
	NormalizeNeuronTree(loadedNT);
	//printf("after normalize\n");
	//for(int i = 0;i<loadedNT.listNeuron.size();i++)
	//{
	//	NeuronSWC S=loadedNT.listNeuron.at(i);
	//	printf("%f, %f, %f, %f\n", S.x,S.y,S.z,S.r);
	//}

	SetupMorphologyLine(0);
	//SetupMorphologyLine(loadedNT,m_unMorphologyLineModeVAO,m_glMorphologyLineModeVertBuffer,m_glMorphologyLineModeIndexBuffer,m_uiMorphologyLineModeVertcount,0);
	SetupMorphologySurface(loadedNT,loaded_spheres,loaded_cylinders,loaded_spheresPos);


	SetupCameras();
	SetupCamerasForMorphology();
	SetupBoundingBox();
	SetupStereoRenderTargets();

	SetupCompanionWindow();

	SetupRenderModels();


	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Initialize Compositor. Returns true if the compositor was
//          successfully initialized, false otherwise.
//-----------------------------------------------------------------------------
bool CMainApplication::BInitCompositor()//note: VRCompositor is responsible for getting pose and submitting framebuffers
{
	vr::EVRInitError peError = vr::VRInitError_None;

	if ( !vr::VRCompositor() )
	{
		printf( "Compositor initialization failed. See log file for details\n" );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::Shutdown()
{
	if( m_pHMD )
	{
		vr::VR_Shutdown();
		m_pHMD = NULL;
	}

	for( std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++ )
	{
		delete (*i);
	}
	m_vecRenderModels.clear();
	
	if( m_pContext )
	{
		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE );
		glDebugMessageCallback(nullptr, nullptr);

		if (morphologyShader != NULL)
		{
			delete morphologyShader;
		}
		if ( m_unControllerTransformProgramID )
		{
			glDeleteProgram( m_unControllerTransformProgramID );
		}
		if ( m_unRenderModelProgramID )
		{
			glDeleteProgram( m_unRenderModelProgramID );
		}
		if ( m_unCompanionWindowProgramID )
		{
			glDeleteProgram( m_unCompanionWindowProgramID );
		}

		glDeleteRenderbuffers( 1, &leftEyeDesc.m_nDepthBufferId );
		glDeleteTextures( 1, &leftEyeDesc.m_nRenderTextureId );
		glDeleteFramebuffers( 1, &leftEyeDesc.m_nRenderFramebufferId );
		glDeleteTextures( 1, &leftEyeDesc.m_nResolveTextureId );
		glDeleteFramebuffers( 1, &leftEyeDesc.m_nResolveFramebufferId );

		glDeleteRenderbuffers( 1, &rightEyeDesc.m_nDepthBufferId );
		glDeleteTextures( 1, &rightEyeDesc.m_nRenderTextureId );
		glDeleteFramebuffers( 1, &rightEyeDesc.m_nRenderFramebufferId );
		glDeleteTextures( 1, &rightEyeDesc.m_nResolveTextureId );
		glDeleteFramebuffers( 1, &rightEyeDesc.m_nResolveFramebufferId );



		for (int i=0;i<loaded_spheres.size();i++) delete loaded_spheres[i];
		for (int i=0;i<loaded_cylinders.size();i++) delete loaded_cylinders[i];

		for (int i=0;i<sketch_spheres.size();i++) delete sketch_spheres[i];
		for (int i=0;i<sketch_cylinders.size();i++) delete sketch_cylinders[i];

		if( m_unMorphologyLineModeVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unMorphologyLineModeVAO );
			glDeleteBuffers(1, &m_glMorphologyLineModeVertBuffer);
			glDeleteBuffers(1, &m_glMorphologyLineModeIndexBuffer);
		}
		
		if( m_unSketchMorphologyLineModeVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unSketchMorphologyLineModeVAO );
			glDeleteBuffers(1, &m_glSketchMorphologyLineModeVertBuffer);
			glDeleteBuffers(1, &m_glSketchMorphologyLineModeIndexBuffer);
		}


		if( m_unCompanionWindowVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unCompanionWindowVAO );
			glDeleteBuffers(1, &m_glCompanionWindowIDVertBuffer);
			glDeleteBuffers(1, &m_glCompanionWindowIDIndexBuffer);
		}
		if( m_unControllerVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unControllerVAO );
			glDeleteBuffers(1, &m_glControllerVertBuffer);
		}
	}

	if( m_pCompanionWindow )
	{
		SDL_DestroyWindow(m_pCompanionWindow);
		m_pCompanionWindow = NULL;
	}

	SDL_Quit();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::HandleInput()
{
	SDL_Event sdlEvent;
	bool bRet = false;

	while ( SDL_PollEvent( &sdlEvent ) != 0 )
	{
		if ( sdlEvent.type == SDL_QUIT )
		{
			bRet = true;
		}
		else if ( sdlEvent.type == SDL_KEYDOWN )
		{
			if ( sdlEvent.key.keysym.sym == SDLK_ESCAPE 
			     || sdlEvent.key.keysym.sym == SDLK_q )
			{
				bRet = true;
			}
			if( sdlEvent.key.keysym.sym == SDLK_c )
			{
				m_bShowMorphologyLine = !m_bShowMorphologyLine;
				m_bShowMorphologySurface = !m_bShowMorphologySurface;
			}
		}
	}

	// Process SteamVR events
	vr::VREvent_t event;
	while( m_pHMD->PollNextEvent( &event, sizeof( event ) ) )
	{
		ProcessVREvent( event );
	}

	// Process SteamVR controller state
	for( vr::TrackedDeviceIndex_t unDevice = 1; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++ )
	{
			if ( !m_pHMD->IsTrackedDeviceConnected( unDevice ) )
				continue;

			if( m_pHMD->GetTrackedDeviceClass( unDevice ) != vr::TrackedDeviceClass_Controller )
				continue;

			m_iTrackedControllerCount += 1;

			if( !m_rTrackedDevicePose[ unDevice ].bPoseIsValid )
				continue;
		vr::VRControllerState_t state;
		
		
		
		
		
		if( m_pHMD->GetControllerState( unDevice, &state, sizeof(state) ) )
		{
			if(state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger)){
				const Matrix4 & mat = m_rmat4DevicePose[unDevice];// mat means current controller pos
				Vector4 m_v4DevicePose = mat * Vector4( 0, 0, 0, 1 );
						//this part is for building a neuron tree to further save as SWC file
							//for every 10 frames we store a point as a neuronswc point
							//long int vertexcount =0,swccount = 0;
							//use vertexcount to control point counts, but it should not define here,swccount, too
							if (vertexcount%10 ==0)
							{
								swccount++;
								//should be defined somewhere else
								NeuronSWC SL0;
								SL0.x = m_v4DevicePose.x ;
								SL0.y = m_v4DevicePose.y ;
								SL0.z = m_v4DevicePose.z ;
								SL0.r = default_radius; //set default radius 1
								SL0.type = 274;//set default type 274
								SL0.n = swccount;
								if(swccount==1){
										SL0.pn = -1;
										//qDebug("Successfully run here.SL0.pn=%d\n",SL0.pn);
										}
								else if(vertexcount == 0){
									SL0.pn = -1;
									//qDebug("Successfully run here.SL1.pn=%d\n",SL0.pn);
									/*for(int i=0;i<sketchNT.listNeuron.size();i++)
									{
										NeuronSWC SL1;
										SL1=sketchNT.listNeuron.at(i);
										//cal dist between SL0 & SL1
										//dist = sqrt(x,y,z);
										float distSL0_1 = 10.1f;
										if(distSL0_1<dist_thres)
										{
											SL0.pn = SL1.n;
											break;
										}
										
									}*/
								}
								else{
									SL0.pn = sketchNT.listNeuron.at(SL0.n-1-1).n;

								}
								sketchNT.listNeuron.append(SL0);
								sketchNT.hashNeuron.insert(SL0.n, sketchNT.listNeuron.size()-1);
							}	
						vertexcount++;
			}
			//m_rbShowTrackedDevice[ unDevice ] = state.ulButtonPressed == 0;
		}
	}//*/

	return bRet;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RunMainLoop()
{
	bool bQuit = false;

	SDL_StartTextInput();
	SDL_ShowCursor( SDL_DISABLE );
	sketchNT.listNeuron.clear();
	sketchNT.hashNeuron.clear();
	while ( !bQuit )
	{
		bQuit = HandleInput();

		RenderFrame();
	}

	SDL_StopTextInput();
}


//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------
void CMainApplication::ProcessVREvent( const vr::VREvent_t & event )
{
	/*if((event.eventtype==vr::vrevent_buttonpress)&&(event.data.controller.button==vr::k_ebutton_steamvr_trigger))
	{
		sdl_showsimplemessagebox( sdl_messagebox_error, "trigger pressed! ", "trigger pressed!", null );
	}//*/
	//if(event.data.controller.button==vr::k_ebutton_steamvr_trigger)
	if((event.eventType==vr::VREvent_ButtonPress)&&(event.data.controller.button==vr::k_EButton_Grip))
	{
				m_bShowMorphologyLine = !m_bShowMorphologyLine;
				m_bShowMorphologySurface = !m_bShowMorphologySurface;		

	}
	if((event.eventType==vr::VREvent_ButtonUnpress)&&(event.data.controller.button==vr::k_EButton_SteamVR_Trigger))
	{
		vertexcount=0;
		//qDebug("Successfully run here.vertexcount=%d\n",vertexcount);
	}
	if((event.eventType==vr::VREvent_ButtonPress)&&(event.data.controller.button==vr::k_EButton_ApplicationMenu))
	{
		if(sketchNT.listNeuron.size()<1)
			return;
		QString filename = "swctofile.swc";
		writeSWC_file(filename, sketchNT);
		qDebug("Successfully writeSWC_file");
	}
	//////////////
	/*switch( event.eventType )
	{
	case vr::VREvent_TrackedDeviceActivated:
		{
			SetupRenderModelForTrackedDevice( event.trackedDeviceIndex );
			dprintf( "Device %u attached. Setting up render model.\n", event.trackedDeviceIndex );
		}
		break;
	case vr::VREvent_TrackedDeviceDeactivated:
		{
			dprintf( "Device %u detached.\n", event.trackedDeviceIndex );
		}
		break;
	case vr::VREvent_TrackedDeviceUpdated:
		{
			dprintf( "Device %u updated.\n", event.trackedDeviceIndex );
		}
		break;
	case vr::VREvent_ButtonPress:
		{
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "pressButton Called! ", "SUCCESS!", NULL );
		}
		break;
	case vr::VREvent_ButtonUnpress:
		{
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "uppressButton Called! ", "SUCCESS!", NULL );
		}
		break;
	}
	switch (event.data.controller.button)
	{
	case vr::k_EButton_ApplicationMenu:
		{
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "MenuButton Called! ", "SUCCESS!", NULL );
		}
		break;
	case vr::k_EButton_Grip:
		{
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "GripButton Called! ", "SUCCESS!", NULL );
		}
		break;
	case vr::k_EButton_SteamVR_Trigger:
		{
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Trigger Called! ", "SUCCESS!", NULL );
		}
		break;
	case vr::k_EButton_DPad_Right:
		{
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Touchpad Called! ", "SUCCESS!", NULL );
		}
		break;
	case vr::k_EButton_DPad_Left:
		{
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "DPad_Left Called! ", "SUCCESS!", NULL );
		}
		break;
	case vr::k_EButton_A:
		{
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "k_EButton_A Called! ", "SUCCESS!", NULL );
		}
		break;
	default :
		break;
	}//*/
	//////////////////
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderFrame()
{
	// for now as fast as possible
	if ( m_pHMD )
	{
		RenderControllerAxes();
		SetupMorphologyLine(1);
		SetupMorphologySurface(sketchNT,sketch_spheres,sketch_cylinders,sketch_spheresPos);
		//SetupMorphologyLine(sketchNT,m_unSketchMorphologyLineModeVAO,m_glSketchMorphologyLineModeVertBuffer,m_glSketchMorphologyLineModeIndexBuffer,m_uiSketchMorphologyLineModeVertcount,1);

		RenderStereoTargets();
		RenderCompanionWindow();

		vr::Texture_t leftEyeTexture = {(void*)(uintptr_t)leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture );
		vr::Texture_t rightEyeTexture = {(void*)(uintptr_t)rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture );
	}

	if ( m_bVblank && m_bGlFinishHack )
	{
		//$ HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
		// happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
		// appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
		// 1/29/2014 mikesart
		glFinish();
	}

	// SwapWindow
	{
		SDL_GL_SwapWindow( m_pCompanionWindow );
	}

	// Clear
	{
		// We want to make sure the glFinish waits for the entire present to complete, not just the submission
		// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
		glClearColor( 0, 0, 0, 1 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	// Flush and wait for swap.
	if ( m_bVblank )
	{
		glFlush();
		glFinish();
	}

	// Spew out the controller and pose count whenever they change.
	if ( m_iTrackedControllerCount != m_iTrackedControllerCount_Last || m_iValidPoseCount != m_iValidPoseCount_Last )
	{
		m_iValidPoseCount_Last = m_iValidPoseCount;//question: what are these? why keep count and count_last? just to observe change?
		m_iTrackedControllerCount_Last = m_iTrackedControllerCount;//note: maybe used for drawing controllers
		
		dprintf( "PoseCount:%d(%s) Controllers:%d\n", m_iValidPoseCount, m_strPoseClasses.c_str(), m_iTrackedControllerCount );
	}

	UpdateHMDMatrixPose();
}


//-----------------------------------------------------------------------------
// Purpose: Compiles a GL shader program and returns the handle. Returns 0 if
//			the shader couldn't be compiled for some reason.
//-----------------------------------------------------------------------------
GLuint CMainApplication::CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader )
{
	GLuint unProgramID = glCreateProgram();

	GLuint nSceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource( nSceneVertexShader, 1, &pchVertexShader, NULL);
	glCompileShader( nSceneVertexShader );

	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv( nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if ( vShaderCompiled != GL_TRUE)
	{
		dprintf("%s - Unable to compile vertex shader %d!\n", pchShaderName, nSceneVertexShader);
		glDeleteProgram( unProgramID );
		glDeleteShader( nSceneVertexShader );
		return 0;
	}
	glAttachShader( unProgramID, nSceneVertexShader);
	glDeleteShader( nSceneVertexShader ); // the program hangs onto this once it's attached

	GLuint  nSceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource( nSceneFragmentShader, 1, &pchFragmentShader, NULL);
	glCompileShader( nSceneFragmentShader );

	GLint fShaderCompiled = GL_FALSE;
	glGetShaderiv( nSceneFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
	if (fShaderCompiled != GL_TRUE)
	{
		dprintf("%s - Unable to compile fragment shader %d!\n", pchShaderName, nSceneFragmentShader );
		glDeleteProgram( unProgramID );
		glDeleteShader( nSceneFragmentShader );
		return 0;	
	}

	glAttachShader( unProgramID, nSceneFragmentShader );
	glDeleteShader( nSceneFragmentShader ); // the program hangs onto this once it's attached

	glLinkProgram( unProgramID );

	GLint programSuccess = GL_TRUE;
	glGetProgramiv( unProgramID, GL_LINK_STATUS, &programSuccess);
	if ( programSuccess != GL_TRUE )
	{
		dprintf("%s - Error linking program %d!\n", pchShaderName, unProgramID);
		glDeleteProgram( unProgramID );
		return 0;
	}

	glUseProgram( unProgramID );
	glUseProgram( 0 );

	return unProgramID;
}


//-----------------------------------------------------------------------------
// Purpose: Creates all the shaders used by HelloVR SDL
//-----------------------------------------------------------------------------
bool CMainApplication::CreateAllShaders()//todo: change shader code
{
	morphologyShader = new Shader("basic.vert", "basic.frag");

	m_unControllerTransformProgramID = CompileGLShader(
		"Controller",//note: for axes

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3ColorIn;\n"
		"out vec4 v4Color;\n"
		"void main()\n"
		"{\n"
		"	v4Color.xyz = v3ColorIn; v4Color.a = 1.0;\n"
		"	gl_Position = matrix * position;\n"
		"}\n",

		// fragment shader
		"#version 410\n"
		"in vec4 v4Color;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = v4Color;\n"
		"}\n"
		);
	m_nControllerMatrixLocation = glGetUniformLocation( m_unControllerTransformProgramID, "matrix" );
	if( m_nControllerMatrixLocation == -1 )
	{
		dprintf( "Unable to find matrix uniform in controller shader\n" );
		return false;
	}

	m_unRenderModelProgramID = CompileGLShader( 
		"render model",//note: for 3D controller model

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3NormalIn;\n"
		"layout(location = 2) in vec2 v2TexCoordsIn;\n"
		"out vec2 v2TexCoord;\n"
		"void main()\n"
		"{\n"
		"	v2TexCoord = v2TexCoordsIn;\n"
		"	gl_Position = matrix * vec4(position.xyz, 1);\n"
		"}\n",

		//fragment shader
		"#version 410 core\n"
		"uniform sampler2D diffuse;\n"
		"in vec2 v2TexCoord;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = texture( diffuse, v2TexCoord);\n"
		"}\n"

		);
	m_nRenderModelMatrixLocation = glGetUniformLocation( m_unRenderModelProgramID, "matrix" );
	if( m_nRenderModelMatrixLocation == -1 )
	{
		dprintf( "Unable to find matrix uniform in render model shader\n" );
		return false;
	}

	m_unCompanionWindowProgramID = CompileGLShader(
		"CompanionWindow",

		// vertex shader
		"#version 410 core\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 v2UVIn;\n"
		"noperspective out vec2 v2UV;\n"
		"void main()\n"
		"{\n"
		"	v2UV = v2UVIn;\n"
		"	gl_Position = position;\n"
		"}\n",

		// fragment shader
		"#version 410 core\n"
		"uniform sampler2D mytexture;\n"
		"noperspective in vec2 v2UV;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"		outputColor = texture(mytexture, v2UV);\n"
		"}\n"
		);

	return m_unControllerTransformProgramID != 0
		&& m_unRenderModelProgramID != 0
		&& m_unCompanionWindowProgramID != 0;
}


void CMainApplication::SetupMorphologySurface(NeuronTree neurontree,vector<Sphere*>& spheres,vector<Cylinder*>& cylinders,vector<glm::vec3>& spheresPos)
{
	NeuronSWC S0,S1;
	for(int i = spheres.size();i<neurontree.listNeuron.size();i++)
	{
		S0=neurontree.listNeuron.at(i);

		//draw sphere
		spheres.push_back(new Sphere((float)(S0.r)));
		spheresPos.push_back(glm::vec3(S0.x,S0.y,S0.z));
		if(S0.pn!=-1){
			S1 = neurontree.listNeuron.at(S0.pn-1);
			//draw cylinder
			float dist = glm::sqrt((S1.x-S0.x)*(S1.x-S0.x)+(S1.y-S0.y)*(S1.y-S0.y)+(S1.z-S0.z)*(S1.z-S0.z));
			cylinders.push_back(new Cylinder(S0.r,S1.r,dist));
		}
		//else {//todo-yimin: temp, to be deleted
		//	cylinders.push_back(new Cylinder(S0.r,S0.r,0));//generate a degenerated cylinder
		//}
	}
}

//-----------------------------------------------------------------------------
// Purpose: prepare data for line mode
//-----------------------------------------------------------------------------
void CMainApplication::SetupMorphologyLine( int drawMode)//pass 3 parameters: &neuronTree, VAO, VertVBO, IdxVBO
{
	if(drawMode==0){
		SetupMorphologyLine(loadedNT,m_unMorphologyLineModeVAO,m_glMorphologyLineModeVertBuffer,m_glMorphologyLineModeIndexBuffer,m_uiMorphologyLineModeVertcount,drawMode);
	}
	else{
		SetupMorphologyLine(sketchNT,m_unSketchMorphologyLineModeVAO,m_glSketchMorphologyLineModeVertBuffer,m_glSketchMorphologyLineModeIndexBuffer,m_uiSketchMorphologyLineModeVertcount,drawMode);
	}

}//*/

//-----------------------------------------------------------------------------
void CMainApplication::SetupMorphologyLine(NeuronTree neuron_Tree,GLuint& LineModeVAO, GLuint& LineModeVBO, GLuint& LineModeIndex,unsigned int& Vertcount,int drawMode)
{
	vector <glm::vec3> vertices;
	vector<GLuint> indices;

	NeuronSWC S0,S1;
	if(neuron_Tree.listNeuron.size()<1)
		return;
	for(int i =0;i<neuron_Tree.listNeuron.size();i++)
	{
		S0=neuron_Tree.listNeuron.at(i);
		vertices.push_back(glm::vec3(S0.x,S0.y,S0.z));
		glm::vec3 vcolor_load(0,1,0);//green for loaded neuron tree
		glm::vec3 vcolor_draw(1,0,0);//red for drawing neuron tree
		vertices.push_back((drawMode==0)?vcolor_load:vcolor_draw);
		if (S0.pn != -1)
		{
			S1 = neuron_Tree.listNeuron.at(S0.pn-1);
			indices.push_back(S0.n-1);
			indices.push_back(S1.n-1);
		}
		else
		{
			indices.push_back(S0.n-1);
			indices.push_back(S0.n-1);
		}
	}

	Vertcount = vertices.size();

	if(LineModeVAO ==0)
	{
		//setup vao and vbo stuff
		glGenVertexArrays(1, &LineModeVAO);
		glGenBuffers(1, &LineModeVBO);
		glGenBuffers(1, &LineModeIndex);

		//now allocate buffers
		glBindVertexArray(LineModeVAO);

		glBindBuffer(GL_ARRAY_BUFFER, LineModeVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LineModeIndex);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3), (GLvoid*)0);
		uintptr_t offset =  sizeof( glm::vec3 );
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3), (const void *)offset);

		glBindVertexArray(0);
	}
		glBindBuffer(GL_ARRAY_BUFFER, LineModeVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LineModeIndex);
	if( vertices.size() > 0 )
	{
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0],(drawMode==0)?GL_STATIC_DRAW:GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0],(drawMode==0)?GL_STATIC_DRAW: GL_DYNAMIC_DRAW);

	}		
}

//-----------------------------------------------------------------------------
// Purpose: Draw all of the controllers as X/Y/Z lines
//-----------------------------------------------------------------------------
void CMainApplication::RenderControllerAxes() //note: note render, actually setup VAO and VBO for axes
{
	// don't draw controllers if somebody else has input focus
	if( m_pHMD->IsInputFocusCapturedByAnotherProcess() )
		return;

	std::vector<float> vertdataarray;

	m_uiControllerVertcount = 0;
	m_iTrackedControllerCount = 0;

	for ( vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice )
	{
		if ( !m_pHMD->IsTrackedDeviceConnected( unTrackedDevice ) )
			continue;

		if( m_pHMD->GetTrackedDeviceClass( unTrackedDevice ) != vr::TrackedDeviceClass_Controller )
			continue;

		m_iTrackedControllerCount += 1;

		if( !m_rTrackedDevicePose[ unTrackedDevice ].bPoseIsValid )
			continue;

		const Matrix4 & mat = m_rmat4DevicePose[unTrackedDevice];

		Vector4 center = mat * Vector4( 0, 0, 0, 1 );

		for ( int i = 0; i < 3; ++i )//note: prepare data for axes and ray
		{
			Vector3 color( 0, 0, 0 );
			Vector4 point( 0, 0, 0, 1 );
			point[i] += 0.05f;  // offset in X, Y, Z
			color[i] = 1.0;  // R, G, B
			point = mat * point;
			vertdataarray.push_back( center.x );
			vertdataarray.push_back( center.y );
			vertdataarray.push_back( center.z );

			vertdataarray.push_back( color.x );
			vertdataarray.push_back( color.y );
			vertdataarray.push_back( color.z );
		
			vertdataarray.push_back( point.x );
			vertdataarray.push_back( point.y );
			vertdataarray.push_back( point.z );
		
			vertdataarray.push_back( color.x );
			vertdataarray.push_back( color.y );
			vertdataarray.push_back( color.z );
		
			m_uiControllerVertcount += 2;
		}

		Vector4 start = mat * Vector4( 0, 0, -0.02f, 1 );
		Vector4 end = mat * Vector4( 0, 0, -39.f, 1 );
		Vector3 color( .92f, .92f, .71f );

		vertdataarray.push_back( start.x );vertdataarray.push_back( start.y );vertdataarray.push_back( start.z );//note: ray?
		vertdataarray.push_back( color.x );vertdataarray.push_back( color.y );vertdataarray.push_back( color.z );

		vertdataarray.push_back( end.x );vertdataarray.push_back( end.y );vertdataarray.push_back( end.z );
		vertdataarray.push_back( color.x );vertdataarray.push_back( color.y );vertdataarray.push_back( color.z );
		m_uiControllerVertcount += 2;
	}

	// Setup the VAO the first time through.
	if ( m_unControllerVAO == 0 )
	{
		glGenVertexArrays( 1, &m_unControllerVAO );
		glBindVertexArray( m_unControllerVAO );

		glGenBuffers( 1, &m_glControllerVertBuffer );
		glBindBuffer( GL_ARRAY_BUFFER, m_glControllerVertBuffer );

		GLuint stride = 2 * 3 * sizeof( float );
		uintptr_t offset = 0;

		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof( Vector3 );
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray( 0 );
	}

	glBindBuffer( GL_ARRAY_BUFFER, m_glControllerVertBuffer );

	// set vertex data if we have some
	if( vertdataarray.size() > 0 )
	{
		//$ TODO: Use glBufferSubData for this...
		glBufferData( GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW );
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::SetupCameras()
{
	m_mat4ProjectionLeft = GetHMDMatrixProjectionEye( vr::Eye_Left );
	m_mat4ProjectionRight = GetHMDMatrixProjectionEye( vr::Eye_Right );
	m_mat4eyePosLeft = GetHMDMatrixPoseEye( vr::Eye_Left );
	m_mat4eyePosRight = GetHMDMatrixPoseEye( vr::Eye_Right );
}

void CMainApplication::SetupCamerasForMorphology()
{
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			m_EyeTransLeft[i][j]   = *(m_mat4eyePosLeft.get() + i * 4 + j);
			m_EyeTransRight[i][j]  = *(m_mat4eyePosRight.get() + i * 4 + j);
			m_ProjTransLeft[i][j]  = *(m_mat4ProjectionLeft.get() + i * 4 + j);
			m_ProjTransRight[i][j] = *(m_mat4ProjectionRight.get() + i * 4 + j);
		}
	}

	for (size_t i = 0; i < 3; i++)
	{
		m_EyePosLeft[i] = *(m_mat4eyePosLeft.get() + i * 4 + 3);
		m_EyePosRight[i] = *(m_mat4eyePosRight.get() + i * 4 + 3);
	}

}

void CMainApplication::SetupBoundingBox()
{
	BoundingBox swcBB = NULL_BoundingBox;
	for(int i = 0;i<loadedNT.listNeuron.size();i++)
	{
		NeuronSWC S=loadedNT.listNeuron.at(i);
		float d = S.r * 2;
		swcBB.expand(BoundingBox(XYZ(S) - d, XYZ(S) + d));
	}

	float DX = swcBB.Dx();
	float DY = swcBB.Dy();
	float DZ = swcBB.Dz();
	float maxD = swcBB.Dmax();

	float s[3];
	s[0] = 1 / maxD * 2;
	s[1] = 1 / maxD * 2;
	s[2] = 1 / maxD * 2;
	float t[3]; //swc center
	t[0] = -(swcBB.x0 + DX / 2);
	t[1] = -(swcBB.y0 + DY / 2);
	t[2] = -(swcBB.z0 + DZ / 2);

	m_GlobalTransformation = glm::translate(glm::mat4(), glm::vec3(t[0],t[1],t[2]));
	m_GlobalTransformation = glm::scale(m_GlobalTransformation, glm::vec3(s[0], s[1], s[2]));
	m_GlobalTransformation = glm::translate(m_GlobalTransformation, glm::vec3(-t[0]-0.6, -t[1]-1.0, -t[2]-0.4));
}

void CMainApplication::NormalizeNeuronTree(NeuronTree& nt)
{
	float r_max = -1;
	BoundingBox swcBB = NULL_BoundingBox;
	for(int i = 0;i<loadedNT.listNeuron.size();i++)
	{
		NeuronSWC S=loadedNT.listNeuron.at(i);
		float d = S.r * 2;
		swcBB.expand(BoundingBox(XYZ(S) - d, XYZ(S) + d));
		if (S.r > r_max)  r_max = S.r;
	}

	float DX = swcBB.Dx();
	float DY = swcBB.Dy();
	float DZ = swcBB.Dz();
	float maxD = swcBB.Dmax();

	//printf("boundingbox: (%f,%f,%f),(%f,%f,%f). size: (%f,%f,%f)\n", swcBB.x0,swcBB.y0,swcBB.z0,swcBB.x1,swcBB.y1,swcBB.z1,DX,DY,DZ );

	float scale = 1 / maxD * 2;
	float r_scale = 1 / r_max * 0.2;
	float trans_x = 0.6 - (swcBB.x0 + swcBB.x1)/2;
	float trans_y = 1.0 - (swcBB.y0 + swcBB.y1)/2;
	float trans_z = 0.4 - (swcBB.z0 + swcBB.z1)/2;

	//printf("transform: scale = %f, translate = (%f,%f,%f)\n", scale,trans_x,trans_y,trans_z );

	for(int i = 0;i<loadedNT.listNeuron.size();i++)
	{
		loadedNT.listNeuron[i].x = loadedNT.listNeuron[i].x * scale + trans_x * scale; 
		loadedNT.listNeuron[i].y = loadedNT.listNeuron[i].y * scale + trans_y * scale + 0.8;
		loadedNT.listNeuron[i].z = loadedNT.listNeuron[i].z * scale + trans_z * scale;
		loadedNT.listNeuron[i].r *= scale;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Creates a frame buffer. Returns true if the buffer was set up.
//          Returns false if the setup failed.
//-----------------------------------------------------------------------------
bool CMainApplication::CreateFrameBuffer( int nWidth, int nHeight, FramebufferDesc &framebufferDesc )//note: maybe no need to change
{
	glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId );
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight );
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,	framebufferDesc.m_nDepthBufferId );

	glGenTextures(1, &framebufferDesc.m_nRenderTextureId );
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId );
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId );
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	glGenTextures(1, &framebufferDesc.m_nResolveTextureId );
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::SetupStereoRenderTargets()
{
	if ( !m_pHMD )
		return false;

	m_pHMD->GetRecommendedRenderTargetSize( &m_nRenderWidth, &m_nRenderHeight );

	CreateFrameBuffer( m_nRenderWidth, m_nRenderHeight, leftEyeDesc );
	CreateFrameBuffer( m_nRenderWidth, m_nRenderHeight, rightEyeDesc );
	
	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::SetupCompanionWindow()//question: what's the content here?
{
	if ( !m_pHMD )
		return;

	std::vector<VertexDataWindow> vVerts;

	// left eye verts
	vVerts.push_back( VertexDataWindow( Vector2(-1, -1), Vector2(0, 1)) );//question: what's the use for this part? two 2D boxes
	vVerts.push_back( VertexDataWindow( Vector2(0, -1), Vector2(1, 1)) );
	vVerts.push_back( VertexDataWindow( Vector2(-1, 1), Vector2(0, 0)) );
	vVerts.push_back( VertexDataWindow( Vector2(0, 1), Vector2(1, 0)) );

	// right eye verts
	vVerts.push_back( VertexDataWindow( Vector2(0, -1), Vector2(0, 1)) );
	vVerts.push_back( VertexDataWindow( Vector2(1, -1), Vector2(1, 1)) );
	vVerts.push_back( VertexDataWindow( Vector2(0, 1), Vector2(0, 0)) );
	vVerts.push_back( VertexDataWindow( Vector2(1, 1), Vector2(1, 0)) );

	GLushort vIndices[] = { 0, 1, 3,   0, 3, 2,   4, 5, 7,   4, 7, 6};
	m_uiCompanionWindowIndexSize = _countof(vIndices);

	glGenVertexArrays( 1, &m_unCompanionWindowVAO );
	glBindVertexArray( m_unCompanionWindowVAO );

	glGenBuffers( 1, &m_glCompanionWindowIDVertBuffer );
	glBindBuffer( GL_ARRAY_BUFFER, m_glCompanionWindowIDVertBuffer );
	glBufferData( GL_ARRAY_BUFFER, vVerts.size()*sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW );

	glGenBuffers( 1, &m_glCompanionWindowIDIndexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_glCompanionWindowIDIndexBuffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_uiCompanionWindowIndexSize*sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof( VertexDataWindow, position ) );

	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof( VertexDataWindow, texCoord ) );

	glBindVertexArray( 0 );

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderStereoTargets()
{
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glEnable( GL_MULTISAMPLE );
	// Left Eye
	glBindFramebuffer( GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId );
 	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight );
 	RenderScene( vr::Eye_Left );
 	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	
	glDisable( GL_MULTISAMPLE );
	 	
 	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.m_nResolveFramebufferId );

    glBlitFramebuffer( 0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight, 
		GL_COLOR_BUFFER_BIT,
 		GL_LINEAR );

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 );	

	glEnable( GL_MULTISAMPLE );

	// Right Eye
	glBindFramebuffer( GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId );
 	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight );
 	RenderScene( vr::Eye_Right );
 	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
 	
	glDisable( GL_MULTISAMPLE );

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId );
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.m_nResolveFramebufferId );
	
    glBlitFramebuffer( 0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight, 
		GL_COLOR_BUFFER_BIT,
 		GL_LINEAR  );

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 );
}


//-----------------------------------------------------------------------------
// Purpose: Renders a scene with respect to nEye.
//-----------------------------------------------------------------------------
void CMainApplication::RenderScene( vr::Hmd_Eye nEye )
{
	glm::vec3 surfcolor(0.95f,0.95f,0.95f);
	glm::vec3 wireframecolor(0.545f, 0.361f, 0.855f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	if (m_bShowMorphologySurface)
	{
		morphologyShader->use();
		
		morphologyShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		morphologyShader->setVec3("lightPos", 1.2f, 1.0f, 2.0f);
		glm::mat4 projection,view;
		if (nEye == vr::Eye_Left)
		{
			morphologyShader->setVec3("viewPos", m_EyePosLeft);
			projection = m_ProjTransLeft;
			view = m_EyeTransLeft * m_HMDTrans;// * m_GlobalTransformation;
		}
		else if (nEye == vr::Eye_Right)
		{
			morphologyShader->setVec3("viewPos", m_EyePosRight); 
			projection = m_ProjTransRight;
			view = m_EyeTransRight * m_HMDTrans;// * m_GlobalTransformation;
		}
		morphologyShader->setMat4("projection", projection);
		morphologyShader->setMat4("view", view);


		// world transformation
		int cy_count = 0;
		for(int i = 0;i<loaded_spheres.size();i++)
		{
			//draw sphere
			glm::mat4 model;
			Sphere* sphr = loaded_spheres[i];
			glm::vec3 sPos = loaded_spheresPos[i];
			model = glm::translate(glm::mat4(), sPos);
			//model = globalmatrix * model;
			morphologyShader->setMat4("model", model);
			morphologyShader->setVec3("objectColor", surfcolor);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			sphr->Render();
			morphologyShader->setVec3("objectColor", wireframecolor);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			sphr->Render();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			//draw cylinder
			int pn = loadedNT.listNeuron[i].pn;
			if (pn != -1)
			{
				//cylinder between node i and (pn-1)
				glm::vec3 v1(0.0f, -1.0f, 0.0f);
				glm::vec3 v2 = loaded_spheresPos[pn-1] - loaded_spheresPos[i];
				float dist = glm::length(v2); //dprintf("dist= %f\n", dist);
				//v1 = glm::normalize(v1);
				v2 = glm::normalize(v2);
				float cos_angle = glm::dot(v1, v2);
				glm::vec3 axis = glm::cross(v1, v2);//todo-yimin: optimizable: these things can be pre-calculated

				Cylinder* cy = loaded_cylinders[cy_count++];
				model = glm::translate(glm::mat4(),loaded_spheresPos[i]);
				model = glm::rotate(model, glm::acos(cos_angle), axis);
				model = glm::translate(model, glm::vec3(.0f, -dist/2, .0f));

				morphologyShader->setMat4("model", model);
				morphologyShader->setVec3("objectColor", surfcolor);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				cy->Render();
				morphologyShader->setVec3("objectColor", wireframecolor);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				cy->Render();
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

		cy_count = 0;
		for(int i = 0;i<sketch_spheres.size();i++)
		{
			glm::mat4 model;
			Sphere* sphr = sketch_spheres[i];
			glm::vec3 sPos = sketch_spheresPos[i];
			model = glm::translate(glm::mat4(), sPos);
			//model = globalmatrix * model;
			morphologyShader->setMat4("model", model);
			morphologyShader->setVec3("objectColor", surfcolor);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			sphr->Render();
			morphologyShader->setVec3("objectColor", wireframecolor);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			sphr->Render();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			//draw cylinder
			int pn = sketchNT.listNeuron[i].pn;
			if (pn != -1)
			{
				//cylinder between node i and (pn-1)
				glm::vec3 v1(0.0f, -1.0f, 0.0f);
				glm::vec3 v2 = sketch_spheresPos[pn-1] - sketch_spheresPos[i];
				float dist = glm::length(v2); //dprintf("dist= %f\n", dist);
				//v1 = glm::normalize(v1);
				v2 = glm::normalize(v2);
				float cos_angle = glm::dot(v1, v2);
				glm::vec3 axis = glm::cross(v1, v2);

				Cylinder* cy = sketch_cylinders[cy_count++];
				model = glm::translate(glm::mat4(),sketch_spheresPos[i]);
				model = glm::rotate(model, glm::acos(cos_angle), axis);
				model = glm::translate(model, glm::vec3(.0f, -dist/2, .0f));

				morphologyShader->setMat4("model", model);
				morphologyShader->setVec3("objectColor", surfcolor);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				cy->Render();
				morphologyShader->setVec3("objectColor", wireframecolor);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				cy->Render();
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}


	}

	if (m_bShowMorphologyLine)
	{
		glUseProgram(m_unControllerTransformProgramID);
		glUniformMatrix4fv(m_nControllerMatrixLocation, 1, GL_FALSE, GetCurrentViewProjectionMatrix(nEye).get());
		glBindVertexArray(m_unMorphologyLineModeVAO);
		glDrawElements(GL_LINES, m_uiMorphologyLineModeVertcount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glUseProgram(m_unControllerTransformProgramID);
		glUniformMatrix4fv(m_nControllerMatrixLocation, 1, GL_FALSE, GetCurrentViewProjectionMatrix(nEye).get());
		glBindVertexArray(m_unSketchMorphologyLineModeVAO);
		glDrawElements(GL_LINES, m_uiSketchMorphologyLineModeVertcount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	bool bIsInputCapturedByAnotherProcess = m_pHMD->IsInputFocusCapturedByAnotherProcess();

	if( !bIsInputCapturedByAnotherProcess )
	{
		// draw the controller axis lines
		glUseProgram( m_unControllerTransformProgramID );
		glUniformMatrix4fv( m_nControllerMatrixLocation, 1, GL_FALSE, GetCurrentViewProjectionMatrix( nEye ).get() );
		glBindVertexArray( m_unControllerVAO );
		glDrawArrays( GL_LINES, 0, m_uiControllerVertcount );
		glBindVertexArray( 0 );
	}

	// ----- Render Model rendering -----
	glUseProgram( m_unRenderModelProgramID );

	for( uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++ )
	{
		if( !m_rTrackedDeviceToRenderModel[ unTrackedDevice ] || !m_rbShowTrackedDevice[ unTrackedDevice ] )
			continue;

		const vr::TrackedDevicePose_t & pose = m_rTrackedDevicePose[ unTrackedDevice ];
		if( !pose.bPoseIsValid )
			continue;

		if( bIsInputCapturedByAnotherProcess && m_pHMD->GetTrackedDeviceClass( unTrackedDevice ) == vr::TrackedDeviceClass_Controller )
			continue;

		const Matrix4 & matDeviceToTracking = m_rmat4DevicePose[ unTrackedDevice ];
		Matrix4 matMVP = GetCurrentViewProjectionMatrix( nEye ) * matDeviceToTracking;
		glUniformMatrix4fv( m_nRenderModelMatrixLocation, 1, GL_FALSE, matMVP.get() );

		m_rTrackedDeviceToRenderModel[ unTrackedDevice ]->Draw();
	}

	glUseProgram( 0 );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderCompanionWindow()
{
	glDisable(GL_DEPTH_TEST);
	glViewport( 0, 0, m_nCompanionWindowWidth, m_nCompanionWindowHeight );

	glBindVertexArray( m_unCompanionWindowVAO );
	glUseProgram( m_unCompanionWindowProgramID );

	// render left eye (first half of index array )
	glBindTexture(GL_TEXTURE_2D, leftEyeDesc.m_nResolveTextureId );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glDrawElements( GL_TRIANGLES, m_uiCompanionWindowIndexSize/2, GL_UNSIGNED_SHORT, 0 );

	// render right eye (second half of index array )
	glBindTexture(GL_TEXTURE_2D, rightEyeDesc.m_nResolveTextureId  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glDrawElements( GL_TRIANGLES, m_uiCompanionWindowIndexSize/2, GL_UNSIGNED_SHORT, (const void *)(uintptr_t)(m_uiCompanionWindowIndexSize) );

	glBindVertexArray( 0 );
	glUseProgram( 0 );
}


//-----------------------------------------------------------------------------
// Purpose: Gets a Matrix Projection Eye with respect to nEye.
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye )
{
	if ( !m_pHMD )
		return Matrix4();

	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix( nEye, m_fNearClip, m_fFarClip );

	return Matrix4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1], 
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2], 
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}


//-----------------------------------------------------------------------------
// Purpose: Gets an HMDMatrixPoseEye with respect to nEye.
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetHMDMatrixPoseEye( vr::Hmd_Eye nEye )
{
	if ( !m_pHMD )
		return Matrix4();

	vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform( nEye );
	Matrix4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0, 
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);

	return matrixObj.invert();
}


//-----------------------------------------------------------------------------
// Purpose: Gets a Current View Projection Matrix with respect to nEye,
//          which may be an Eye_Left or an Eye_Right.
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetCurrentViewProjectionMatrix( vr::Hmd_Eye nEye )
{
	Matrix4 global(
		m_GlobalTransformation[0][0], m_GlobalTransformation[0][1], m_GlobalTransformation[0][2], m_GlobalTransformation[0][3],
		m_GlobalTransformation[1][0], m_GlobalTransformation[1][1], m_GlobalTransformation[1][2], m_GlobalTransformation[1][3],
		m_GlobalTransformation[2][0], m_GlobalTransformation[2][1], m_GlobalTransformation[2][2], m_GlobalTransformation[2][3],
		m_GlobalTransformation[3][0], m_GlobalTransformation[3][1], m_GlobalTransformation[3][2], m_GlobalTransformation[3][3]
	
	);
	
	Matrix4 matMVP;
	if( nEye == vr::Eye_Left )
	{
		matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;// * global;
	}
	else if( nEye == vr::Eye_Right )
	{
		matMVP = m_mat4ProjectionRight * m_mat4eyePosRight *  m_mat4HMDPose;// * global;
	}

	return matMVP;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::UpdateHMDMatrixPose()
{
	if ( !m_pHMD )
		return;

	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );

	m_iValidPoseCount = 0;
	m_strPoseClasses = "";
	for ( int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice )
	{
		if ( m_rTrackedDevicePose[nDevice].bPoseIsValid )
		{
			m_iValidPoseCount++;//note: count all the valid devices? but the value is never used.
			m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4( m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking );
			if (m_rDevClassChar[nDevice]==0)
			{
				switch (m_pHMD->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'G'; break;
				case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
				default:                                       m_rDevClassChar[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];
		}
	}

	if ( m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid )
	{
		m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
		m_mat4HMDPose.invert();
		for (size_t i = 0; i < 4; i++)
		{
			for (size_t j = 0; j < 4; j++)
			{
				m_HMDTrans[i][j] = *(m_mat4HMDPose.get() + i * 4 + j);
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Finds a render model we've already loaded or loads a new one
//-----------------------------------------------------------------------------
CGLRenderModel *CMainApplication::FindOrLoadRenderModel( const char *pchRenderModelName )
{
	CGLRenderModel *pRenderModel = NULL;
	for( std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++ )
	{
		if( !stricmp( (*i)->GetName().c_str(), pchRenderModelName ) )
		{
			pRenderModel = *i;
			break;
		}
	}

	// load the model if we didn't find one
	if( !pRenderModel )
	{
		vr::RenderModel_t *pModel;
		vr::EVRRenderModelError error;
		while ( 1 )
		{
			error = vr::VRRenderModels()->LoadRenderModel_Async( pchRenderModelName, &pModel );
			if ( error != vr::VRRenderModelError_Loading )
				break;

			ThreadSleep( 1 );
		}

		if ( error != vr::VRRenderModelError_None )
		{
			dprintf( "Unable to load render model %s - %s\n", pchRenderModelName, vr::VRRenderModels()->GetRenderModelErrorNameFromEnum( error ) );
			return NULL; // move on to the next tracked device
		}

		vr::RenderModel_TextureMap_t *pTexture;
		while ( 1 )
		{
			error = vr::VRRenderModels()->LoadTexture_Async( pModel->diffuseTextureId, &pTexture );
			if ( error != vr::VRRenderModelError_Loading )
				break;

			ThreadSleep( 1 );
		}

		if ( error != vr::VRRenderModelError_None )
		{
			dprintf( "Unable to load render texture id:%d for render model %s\n", pModel->diffuseTextureId, pchRenderModelName );
			vr::VRRenderModels()->FreeRenderModel( pModel );
			return NULL; // move on to the next tracked device
		}

		pRenderModel = new CGLRenderModel( pchRenderModelName );
		if ( !pRenderModel->BInit( *pModel, *pTexture ) )
		{
			dprintf( "Unable to create GL model from render model %s\n", pchRenderModelName );
			delete pRenderModel;
			pRenderModel = NULL;
		}
		else
		{
			m_vecRenderModels.push_back( pRenderModel );
		}
		vr::VRRenderModels()->FreeRenderModel( pModel );
		vr::VRRenderModels()->FreeTexture( pTexture );
	}
	return pRenderModel;
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL a Render Model for a single tracked device
//-----------------------------------------------------------------------------
void CMainApplication::SetupRenderModelForTrackedDevice( vr::TrackedDeviceIndex_t unTrackedDeviceIndex )
{
	if( unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount )
		return;

	// try to find a model we've already set up
	std::string sRenderModelName = GetTrackedDeviceString( m_pHMD, unTrackedDeviceIndex, vr::Prop_RenderModelName_String );
	CGLRenderModel *pRenderModel = FindOrLoadRenderModel( sRenderModelName.c_str() );
	if( !pRenderModel )
	{
		std::string sTrackingSystemName = GetTrackedDeviceString( m_pHMD, unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String );
		dprintf( "Unable to load render model for tracked device %d (%s.%s)", unTrackedDeviceIndex, sTrackingSystemName.c_str(), sRenderModelName.c_str() );
	}
	else
	{
		m_rTrackedDeviceToRenderModel[ unTrackedDeviceIndex ] = pRenderModel;
		m_rbShowTrackedDevice[ unTrackedDeviceIndex ] = true;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
void CMainApplication::SetupRenderModels()//question: purpose? for the 16 VR devices?
{
	memset( m_rTrackedDeviceToRenderModel, 0, sizeof( m_rTrackedDeviceToRenderModel ) );

	if( !m_pHMD )
		return;

	for( uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++ )//note: don't include the hmd
	{
		if( !m_pHMD->IsTrackedDeviceConnected( unTrackedDevice ) )
			continue;

		SetupRenderModelForTrackedDevice( unTrackedDevice );
	}

}


//-----------------------------------------------------------------------------
// Purpose: Converts a SteamVR matrix to our local matrix class
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::ConvertSteamVRMatrixToMatrix4( const vr::HmdMatrix34_t &matPose )//note: just called once.
{
	Matrix4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
		);
	return matrixObj;
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
CGLRenderModel::CGLRenderModel( const std::string & sRenderModelName )//todo£∫ set vao vbo to zero
	: m_sModelName( sRenderModelName )
{
	m_glIndexBuffer = 0;
	m_glVertArray = 0;
	m_glVertBuffer = 0;
	m_glTexture = 0;
}


CGLRenderModel::~CGLRenderModel()
{
	Cleanup();
}


//-----------------------------------------------------------------------------
// Purpose: Allocates and populates the GL resources for a render model
//-----------------------------------------------------------------------------
bool CGLRenderModel::BInit( const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture )
{
	// create and bind a VAO to hold state for this model
	glGenVertexArrays( 1, &m_glVertArray );
	glBindVertexArray( m_glVertArray );

	// Populate a vertex buffer
	glGenBuffers( 1, &m_glVertBuffer );
	glBindBuffer( GL_ARRAY_BUFFER, m_glVertBuffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof( vr::RenderModel_Vertex_t ) * vrModel.unVertexCount, vrModel.rVertexData, GL_STATIC_DRAW );

	// Identify the components in the vertex buffer
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( vr::RenderModel_Vertex_t ), (void *)offsetof( vr::RenderModel_Vertex_t, vPosition ) );
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( vr::RenderModel_Vertex_t ), (void *)offsetof( vr::RenderModel_Vertex_t, vNormal ) );
	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( vr::RenderModel_Vertex_t ), (void *)offsetof( vr::RenderModel_Vertex_t, rfTextureCoord ) );

	// Create and populate the index buffer
	glGenBuffers( 1, &m_glIndexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( uint16_t ) * vrModel.unTriangleCount * 3, vrModel.rIndexData, GL_STATIC_DRAW );

	glBindVertexArray( 0 );

	// create and populate the texture
	glGenTextures(1, &m_glTexture );
	glBindTexture( GL_TEXTURE_2D, m_glTexture );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, vrDiffuseTexture.unWidth, vrDiffuseTexture.unHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, vrDiffuseTexture.rubTextureMapData );

	// If this renders black ask McJohn what's wrong.
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	GLfloat fLargest;
	glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest );

	glBindTexture( GL_TEXTURE_2D, 0 );

	m_unVertexCount = vrModel.unTriangleCount * 3;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Frees the GL resources for a render model
//-----------------------------------------------------------------------------
void CGLRenderModel::Cleanup()//todo: delete vao, vbo
{
	if( m_glVertBuffer )
	{
		glDeleteBuffers(1, &m_glIndexBuffer);
		glDeleteVertexArrays( 1, &m_glVertArray );
		glDeleteBuffers(1, &m_glVertBuffer);
		m_glIndexBuffer = 0;
		m_glVertArray = 0;
		m_glVertBuffer = 0;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Draws the render model
//-----------------------------------------------------------------------------
void CGLRenderModel::Draw()
{
	glBindVertexArray( m_glVertArray );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_glTexture );

	glDrawElements( GL_TRIANGLES, m_unVertexCount, GL_UNSIGNED_SHORT, 0 );

	glBindVertexArray( 0 );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool doimageVRViewer(NeuronTree nt)
{
	CMainApplication *pMainApplication = new CMainApplication( 0, 0 );

	loadedNT.listNeuron.clear();
	loadedNT.hashNeuron.clear();
	loadedNT.listNeuron = nt.listNeuron;
	loadedNT.hashNeuron = nt.hashNeuron;

	if (!pMainApplication->BInit())
	{
		pMainApplication->Shutdown();
		return 1;
	}

	pMainApplication->RunMainLoop();

	pMainApplication->Shutdown();

	return 0;
}

bool doimageVRViewer()
{
	NeuronTree nt;
	nt.listNeuron.clear();
	nt.hashNeuron.clear();
	return doimageVRViewer(nt);
}
