/**
  \file minimalOpenGL/main.cpp
  \author Morgan McGuire, http://casual-effects.com
  Distributed with the G3D Innovation Engine http://casual-effects.com/g3d

  Features demonstrated:
   * Window, OpenGL, and extension initialization
   * Triangle mesh rendering (GL Vertex Array Buffer)
   * Texture map loading (GL Texture Object)
   * Shader loading (GL Program and Shader Objects)
   * Fast shader argument binding (GL Uniform Buffer Objects)
   * Offscreen rendering / render-to-texture (GL Framebuffer Object)
   * Ray tracing
   * Procedural texture
   * Tiny vector math library
   * Mouse and keyboard handling

  This is a minimal example of an OpenGL 4 program using only
  GLFW and GLEW libraries to simplify initialization. It does
  not depend on G3D or any other external libraries at all. 
  You could use SDL or another thin library instead of those two.
  If you want to use VR, this also requires the OpenVR library.
  (All dependencies are included with G3D)
  
  This is useful as a testbed when isolating driver bugs and 
  seeking a minimal context. 

  It is also helpful if you're new to computer graphics and wish to
  see the underlying hardware API without the high-level features that
  G3D provides.

  I chose OpenGL 4.1 because it is the newest OpenGL available on OS
  X, and thus the newest OpenGL that can be used across the major PC
  operating systems of Windows, Linux, OS X, and Steam.

  If you're interested in other minimal graphics code for convenience,
  also look at the stb libraries for single-header, dependency-free support
  for image loading, parsing, fonts, noise, etc.:
     https://github.com/nothings/stb

  And a SDL-based minimal OpenGL program at:
     https://gist.github.com/manpat/112f3f31c983ccddf044
  
  Reference Frames:
      Object: The object being rendered (the Shape in this example) relative to its own origin
      World:  Global reference frame
      Body:   Controlled by keyboard and mouse
      Head:   Controlled by tracking (or fixed relative to the body for non-VR)
      Camera: Fixed relative to the head. The camera is the eye.
 */

// Uncomment to add VR support
#define _VR

// To switch the box to a teapot, uncomment the following two lines
//#include "teapot.h"
//#define Shape Teapot

////////////////////////////////////////////////////////////////////////////////

#include "matrix.h"
#include "minimalOpenGL.h"
#include "vrminimal.h"

#ifdef _VR
#   include "minimalOpenVR.h"
#endif

GLFWwindow* window = nullptr;

#ifdef _VR
    vr::IVRSystem* hmd = nullptr;
#endif

#ifndef Shape
#   define Shape Cube
#endif


void doimageVRViewer_v2(NeuronTree nt,int lineType) {
    std::cout << "Minimal OpenGL 4.1 Example by Morgan McGuire\n\nW, A, S, D, C, Z keys to translate\nMouse click and drag to rotate\nESC to quit\n\n";
    std::cout << std::fixed;

    uint32_t framebufferWidth = 1280, framebufferHeight = 720;
#   ifdef _VR
        const int numEyes = 2;
        hmd = initOpenVR(framebufferWidth, framebufferHeight);
        assert(hmd);
#   else
        const int numEyes = 1;
#   endif

    const int windowHeight = 720;
    const int windowWidth = (framebufferWidth * windowHeight) / framebufferHeight;

    window = initOpenGL(windowWidth, windowHeight, "minimalOpenGL");
        
    Vector3 bodyTranslation(0.0f, 1.6f, 5.0f);
    Vector3 bodyRotation;

    //////////////////////////////////////////////////////////////////////
    // Allocate the frame buffer. This code allocates one framebuffer per eye.
    // That requires more GPU memory, but is useful when performing temporal 
    // filtering or making render calls that can target both simultaneously.

    GLuint framebuffer[numEyes];
    glGenFramebuffers(numEyes, framebuffer);

    GLuint colorRenderTarget[numEyes], depthRenderTarget[numEyes];
    glGenTextures(numEyes, colorRenderTarget);
    glGenTextures(numEyes, depthRenderTarget);
    for (int eye = 0; eye < numEyes; ++eye) {
        glBindTexture(GL_TEXTURE_2D, colorRenderTarget[eye]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glBindTexture(GL_TEXTURE_2D, depthRenderTarget[eye]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, framebufferWidth, framebufferHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer[eye]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorRenderTarget[eye], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, depthRenderTarget[eye], 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/////////////////////////////////////////////////////////////////
	//draw swc lines
	//drawSWClines(nt,lineType);

		int SWC_num=nt.listNeuron.size();
		qDebug("SWC_num=%d\t",SWC_num);
		//const int SWC_num=45;
		//float **aposition = new float*[SWC_num];
		//float aposition[SWC_num][3];
		std::vector<float> aposition;
		int index_num = SWC_num * 2;
		//int aindex[index_num];
		std::vector<int> aindex;
		//int * aindex = new int [index_num];
		NeuronSWC S0;
		int count=0;
		qDebug("run here %d\t",count);count++;

		for(int i = 0,k=0;i<SWC_num;i++)
		{
			//aposition[i] = new float[3];
			/*aposition[i][0]=i*5.f;
			aposition[i][1]=i*5.f;
			aposition[i][2]=i*5.f;*/
			/*for(int j =0;j<3;j++)
			{
				aposition[i][j]=Shape::position[i][j]*10;
			}*/
			//if(!nt.listNeuron.size()>0)
			//	break;
			S0=nt.listNeuron.at(i);
			//vertdata.push_back( fl0 );
			aposition.push_back( S0.x/10);	
			aposition.push_back( S0.y/10);	
			aposition.push_back( S0.z/10);	
			/*qDebug("aposition[i][1]=%.2f\t",aposition[i][1]);
			qDebug("aposition[i][0]=%.2f\t",aposition[i][0]);
			qDebug("aposition[i][2]=%.2f\t",aposition[i][2]);//*/
			//qDebug("run here %d\t",count);count++;
			if(S0.pn==-1)
			{
				aindex.push_back(i);
				aindex.push_back(i);
				qDebug("run here %d\t",count);count++;
			}
			else
			{
				aindex.push_back(i);
				aindex.push_back(S0.pn-1);
			}
			k+=2;
		}//*/
		
		/////////////////////////////////////////////////////////////////
		// Load vertex array buffers
		qDebug("run here %d\t",count);count++;
		GLuint positionBuffer = GL_NONE;
		glGenBuffers(1, &positionBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(Shape::position), Shape::position, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * aposition.size(), &aposition[0], GL_STATIC_DRAW);

		qDebug("run here %d\t",count);count++;
		//float **atexCoord =new float*[SWC_num];
		/*float atexCoord[SWC_num][2];
		for(int i = 0;i<SWC_num;i++)
		{
			//atexCoord[i] = new float[2];
			for(int j= 0;j<2;j++)
			{
				//atexCoord[i][j]=0;
				atexCoord[i][j]=Shape::texCoord[i][j];
			}
		}
		qDebug("atexCoord[9][1]=%.2f\n",atexCoord[9][1]);//*/
		GLuint texCoordBuffer = GL_NONE;
		glGenBuffers(1, &texCoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(Shape::texCoord), Shape::texCoord, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, SWC_num * 2, 0, GL_STATIC_DRAW);

		//float **anormal =new float*[SWC_num];
		/*float anormal[SWC_num][3];
		for(int i = 0;i<SWC_num;i++)
		{
			//anormal[i] = new float[3];
			for(int j= 0;j<3;j++)
			{
				//anormal[i][j]=0;
				anormal[i][j]=Shape::normal[i][j];
			}
		}//*/
		GLuint normalBuffer = GL_NONE;
		glGenBuffers(1, &normalBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(Shape::normal), Shape::normal, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, SWC_num * 3, 0, GL_STATIC_DRAW);



		//float **atangent =new float*[SWC_num];
		
		/*float atangent[SWC_num][4];
		for(int i = 0;i<SWC_num;i++)
		{
			//atangent[i] = new float[4];
			for(int j= 0;j<4;j++)
			{
				//atangent[i][j]=0;
				atangent[i][j]=Shape::tangent[i][j];
			}
		}//*/
		//
		GLuint tangentBuffer = GL_NONE;
		glGenBuffers(1, &tangentBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(Shape::tangent), Shape::tangent, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER,SWC_num * 4 , 0, GL_STATIC_DRAW);

		const int numVertices = sizeof(float) * aposition.size()/ sizeof(aposition[0]);
		qDebug("numVertices=%d",numVertices);
		/*for(int i =0;i<numVertices;)
		{
			qDebug("aposition[0]= %.2f\t",aposition.at(i));
			i+=3;
		}//*/
		(void)numVertices;
		
		//const int index_num = SWC_num * 2 - 2;
		//float *aindex = new float[index_num];
		/*int aindex[index_num];
		for(int i=0;i<index_num;i++)
		{
			//aindex[i]=Shape::index[i];
			if(i==0)
			{
				aindex[i]=0;
			}
			else if(i%2!=0)
			{
				aindex[i]=aindex[i-1]+1;
			}
			else
			{
				aindex[i]=aindex[i-1];
			}
		}//*/
		//
		//qDebug("aindex[46]=%.2f\n",aindex[46]);
		GLuint indexBuffer = GL_NONE;
		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Shape::index), Shape::index, GL_STATIC_DRAW);
		//const int numIndices = sizeof(Shape::index) / sizeof(Shape::index[0]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * aindex.size(), &aindex[0], GL_STATIC_DRAW);
		
		
		const int numIndices = sizeof(int) * aindex.size()/ sizeof(aindex[0]);
		//const int numIndices =index_num;
		qDebug("numIndices=%d",numIndices);
		for(int i =0;i<numIndices;i++)
		{
			qDebug("aindex[0]= %d\t",aindex.at(i));
		}


    /////////////////////////////////////////////////////////////////////
    // Create the main shader
    const GLuint shader = createShaderProgram(loadTextFile("min.vrt"), loadTextFile("min.pix"));

    // Binding points for attributes and uniforms discovered from the shader
    const GLint positionAttribute   = glGetAttribLocation(shader,  "position");
    const GLint normalAttribute     = glGetAttribLocation(shader,  "normal");
    const GLint texCoordAttribute   = glGetAttribLocation(shader,  "texCoord");
    const GLint tangentAttribute    = glGetAttribLocation(shader,  "tangent");    
    const GLint colorTextureUniform = glGetUniformLocation(shader, "colorTexture");

    const GLuint uniformBlockIndex = glGetUniformBlockIndex(shader, "Uniform");
    const GLuint uniformBindingPoint = 1;
    glUniformBlockBinding(shader, uniformBlockIndex, uniformBindingPoint);

    GLuint uniformBlock;
    glGenBuffers(1, &uniformBlock);

    {
        // Allocate space for the uniform block buffer
        GLint uniformBlockSize;
        glGetActiveUniformBlockiv(shader, uniformBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &uniformBlockSize);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBlock);
        glBufferData(GL_UNIFORM_BUFFER, uniformBlockSize, nullptr, GL_DYNAMIC_DRAW);
    }

    const GLchar* uniformName[] = {
        "Uniform.objectToWorldNormalMatrix",
        "Uniform.objectToWorldMatrix",
        "Uniform.modelViewProjectionMatrix",
        "Uniform.light",
        "Uniform.cameraPosition"};

    const int numBlockUniforms = sizeof(uniformName) / sizeof(uniformName[0]);
#   ifdef _DEBUG
    {
        GLint debugNumUniforms = 0;
        glGetProgramiv(shader, GL_ACTIVE_UNIFORMS, &debugNumUniforms);
        for (GLint i = 0; i < debugNumUniforms; ++i) {
            GLchar name[1024];
            GLsizei size = 0;
            GLenum type = GL_NONE;
            glGetActiveUniform(shader, i, sizeof(name), nullptr, &size, &type, name);
            std::cout << "Uniform #" << i << ": " << name << "\n";
        }
        assert(debugNumUniforms >= numBlockUniforms);
    }
#   endif

    // Map uniform names to indices within the block
    GLuint uniformIndex[numBlockUniforms];
    glGetUniformIndices(shader, numBlockUniforms, uniformName, uniformIndex);
    assert(uniformIndex[0] < 10000);

    // Map indices to byte offsets
    GLint  uniformOffset[numBlockUniforms];
    glGetActiveUniformsiv(shader, numBlockUniforms, uniformIndex, GL_UNIFORM_OFFSET, uniformOffset);
    assert(uniformOffset[0] >= 0);

    // Load a texture map
    GLuint colorTexture = GL_NONE;
    {
        int textureWidth, textureHeight, channels;
        std::vector<unsigned char > data;
        loadBMP("color.bmp", textureWidth, textureHeight, channels, data);

        glGenTextures(1, &colorTexture);
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, textureWidth, textureHeight, 0, (channels == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    GLuint trilinearSampler = GL_NONE;
    {
        glGenSamplers(1, &trilinearSampler);
        glSamplerParameteri(trilinearSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(trilinearSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(trilinearSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(trilinearSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
	/*for(int i =0;i<24;i++)
		delete [] aposition[i];
	delete []aposition;
	/*delete []anormal;
	delete []atangent;
	delete []atexCoord;
	delete []aindex;//*/
	//////////////////////////
#   ifdef _VR
        vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
#   endif

    // Main loop:
    int timer = 0;
    while (! glfwWindowShouldClose(window)) {
        assert(glGetError() == GL_NONE);

        const float nearPlaneZ = -0.1f;
        const float farPlaneZ = -100.0f;
        const float verticalFieldOfView = 45.0f * PI / 180.0f;

        Matrix4x4 eyeToHead[numEyes], projectionMatrix[numEyes], headToBodyMatrix;
#       ifdef _VR
            getEyeTransformations(hmd, trackedDevicePose, nearPlaneZ, farPlaneZ, headToBodyMatrix.data, eyeToHead[0].data, eyeToHead[1].data, projectionMatrix[0].data, projectionMatrix[1].data);
#       else
            projectionMatrix[0] = Matrix4x4::perspective(float(framebufferWidth), float(framebufferHeight), nearPlaneZ, farPlaneZ, verticalFieldOfView);
#       endif

        // printf("float nearPlaneZ = %f, farPlaneZ = %f; int width = %d, height = %d;\n", nearPlaneZ, farPlaneZ, framebufferWidth, framebufferHeight);

        const Matrix4x4& bodyToWorldMatrix = 
            Matrix4x4::translate(bodyTranslation) *
            Matrix4x4::roll(bodyRotation.z) *
            Matrix4x4::yaw(bodyRotation.y) *
            Matrix4x4::pitch(bodyRotation.x);

        const Matrix4x4& headToWorldMatrix = bodyToWorldMatrix * headToBodyMatrix;

        for (int eye = 0; eye < numEyes; ++eye) {
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer[eye]);
            glViewport(0, 0, framebufferWidth, framebufferHeight);

            glClearColor(0.1f, 0.2f, 0.3f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            const Matrix4x4& objectToWorldMatrix       = Matrix4x4::translate(0.0f, 0.5f, 0.0f) * Matrix4x4::yaw(PI / 3.0f);
            const Matrix3x3& objectToWorldNormalMatrix = Matrix3x3(objectToWorldMatrix).transpose().inverse();
            const Matrix4x4& cameraToWorldMatrix       = headToWorldMatrix * eyeToHead[eye];

            const Vector3& light = Vector3(1.0f, 0.5f, 0.2f).normalize();

            // Draw the background
            drawSky(framebufferWidth, framebufferHeight, nearPlaneZ, farPlaneZ, cameraToWorldMatrix.data, projectionMatrix[eye].inverse().data, &light.x);

            ////////////////////////////////////////////////////////////////////////
            // Draw a mesh
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glEnable(GL_CULL_FACE);
            glDepthMask(GL_TRUE);
        
            glUseProgram(shader);

            // in position
            glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
            glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(positionAttribute);

            // in normal
            glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
            glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(normalAttribute);

            // in tangent
            if (tangentAttribute != -1) {
                // Only bind if used
                glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
                glVertexAttribPointer(tangentAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(tangentAttribute);
            }

            // in texCoord 
            glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
            glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(texCoordAttribute);

            // indexBuffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

            // uniform colorTexture (samplers cannot be placed in blocks)
            const GLint colorTextureUnit = 0;
            glActiveTexture(GL_TEXTURE0 + colorTextureUnit);
            glBindTexture(GL_TEXTURE_2D, colorTexture);
            glBindSampler(colorTextureUnit, trilinearSampler);
            glUniform1i(colorTextureUniform, colorTextureUnit);

            // Other uniforms in the interface block
            {
                glBindBufferBase(GL_UNIFORM_BUFFER, uniformBindingPoint, uniformBlock);

                GLubyte* ptr = (GLubyte*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
                // mat3 is passed to openGL as if it was mat4 due to padding rules.
                for (int row = 0; row < 3; ++row) {
                    memcpy(ptr + uniformOffset[0] + sizeof(float) * 4 * row, objectToWorldNormalMatrix.data + row * 3, sizeof(float) * 3);
                }

                memcpy(ptr + uniformOffset[1], objectToWorldMatrix.data, sizeof(objectToWorldMatrix));

                const Matrix4x4& modelViewProjectionMatrix = projectionMatrix[eye] * cameraToWorldMatrix.inverse() * objectToWorldMatrix;
                memcpy(ptr + uniformOffset[2], modelViewProjectionMatrix.data, sizeof(modelViewProjectionMatrix));
                memcpy(ptr + uniformOffset[3], &light.x, sizeof(light));
                const Vector4& cameraPosition = cameraToWorldMatrix.col(3);
                memcpy(ptr + uniformOffset[4], &cameraPosition.x, sizeof(Vector3));
                glUnmapBuffer(GL_UNIFORM_BUFFER);
            }

            glDrawElements(GL_LINES, numIndices, GL_UNSIGNED_INT, 0);
#           ifdef _VR
            {
                const vr::Texture_t tex = { reinterpret_cast<void*>(intptr_t(colorRenderTarget[eye])), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
                vr::VRCompositor()->Submit(vr::EVREye(eye), &tex);
            }
#           endif
        } // for each eye

        ////////////////////////////////////////////////////////////////////////
#       ifdef _VR
            // Tell the compositor to begin work immediately instead of waiting for the next WaitGetPoses() call
            vr::VRCompositor()->PostPresentHandoff();
#       endif

        // Mirror to the window
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBlitFramebuffer(0, 0, framebufferWidth, framebufferHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_NONE);

        // Display what has been drawn on the main window
        glfwSwapBuffers(window);

        // Check for events
        glfwPollEvents();

        // Handle events
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, 1);
        }

        // WASD keyboard movement
        const float cameraMoveSpeed = 0.01f;
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W)) { bodyTranslation += Vector3(headToWorldMatrix * Vector4(0, 0, -cameraMoveSpeed, 0)); }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S)) { bodyTranslation += Vector3(headToWorldMatrix * Vector4(0, 0, +cameraMoveSpeed, 0)); }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A)) { bodyTranslation += Vector3(headToWorldMatrix * Vector4(-cameraMoveSpeed, 0, 0, 0)); }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D)) { bodyTranslation += Vector3(headToWorldMatrix * Vector4(+cameraMoveSpeed, 0, 0, 0)); }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_C)) { bodyTranslation.y -= cameraMoveSpeed; }
        if ((GLFW_PRESS == glfwGetKey(window, GLFW_KEY_SPACE)) || (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_Z))) { bodyTranslation.y += cameraMoveSpeed; }

        // Keep the camera above the ground
        if (bodyTranslation.y < 0.01f) { bodyTranslation.y = 0.01f; }

        static bool inDrag = false;
        const float cameraTurnSpeed = 0.005f;
        if (GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
            static double startX, startY;
            double currentX, currentY;

            glfwGetCursorPos(window, &currentX, &currentY);
            if (inDrag) {
                bodyRotation.y -= float(currentX - startX) * cameraTurnSpeed;
                bodyRotation.x -= float(currentY - startY) * cameraTurnSpeed;
            }
            inDrag = true; startX = currentX; startY = currentY;
        } else {
            inDrag = false;
        }

        ++timer;
    }

#   ifdef _VR
        if (hmd != nullptr) {
            vr::VR_Shutdown();
        }
#   endif

    // Close the GL context and release all resources
    glfwTerminate();

}
void drawSWClines(NeuronTree nt,int lineType)
{
	//if(!nt.listNeuron.size()>0)
	//	return;
	const QList <NeuronSWC> & listNeuron = nt.listNeuron;
	const QHash <int, int> & hashNeuron = nt.hashNeuron;
	//RGBA8 rgba = nt.color;
	//bool on    = nt.on;
	//bool editable = nt.editable;
   // int cur_linemode = nt.linemode; //added by PHC 20130926
	//int cur_lineType = (cur_linemode==0 || cur_linemode==1) ? cur_linemode : lineType;
	//if (! on) return;

	/*NeuronSWC p_cur, p_pre;
	for (int i=0;i<nt.listNeuron.size(); i++)
	{
		p_cur = (NeuronSWC *)(&(nt.listNeuron.at(i)));
		int p_n = p_cur ->parent;
		if(p_n! = -1)
		{
			p_pre = (NeuronSWC *)(&(nt.listNeuron.at(p_n)));
			drawline(p_cur,p_pre);
		}
	}//*/
    NeuronSWC S0, S1;//s1 is current,s0 is parent 
	for (int i=0; i<listNeuron.size(); i++)
	{
		S1 = listNeuron.at(i);   // at(i) faster than [i]
		//if (S1.pn <1)	continue; 	// skip the first point
		bool valid = false;
		if (S1.pn == -1) // root end, 081105
		{
			S0 = S1;
			valid = true;
		}
		else if (S1.pn >=0) //change to >=0 from >0, PHC 091123
		{
//			for (int j=0; j<listNeuron.size(); j++)
//			{
//				S0 = listNeuron.at(j);
//				if (S0.n==S1.pn)
//				{
//					valid = true;
//					break;
//				}
//			}
			// or using hash for finding parent node
			int j = hashNeuron.value(S1.pn, -1);
			if (j>=0 && j <listNeuron.size())
			{
				S0 = listNeuron.at(j);
				valid = true;
			}
		}
		if (! valid)  continue;
		drawline(S0,S1);
	}
	//v3d_msg("succeed in processing neurontree data.\n");

}
void drawline(NeuronSWC p_pre,NeuronSWC p_cur)
{

}
/*#ifdef _WINDOWS
    int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw) {
        return main(0, nullptr);
    }
#endif*/
