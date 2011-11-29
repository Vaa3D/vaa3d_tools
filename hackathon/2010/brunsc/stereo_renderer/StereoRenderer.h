#ifndef V3D_STEREO_RENDERER_H_
#define V3D_STEREO_RENDERER_H_

class DrawingRenderer {
public:
    void draw() = 0;
    virtual ~DrawingRenderer() {}
};

// Base class for stereo renderers
class StereoRenderer
{
public:
    enum EyeTransformType {
        EYE_TRANSFORM_SKEW; // skew transform for better clipping consistency
        EYE_TRANSFORM_ROTATE; // rotation transform for simple geometric concept
    };
    
    StereoRenderer() : eyeTransformType(EYE_TRANSFORM_ROTATE) {}
    virtual ~StereoRenderer() {}
    
    virtual void draw_stereo() = 0;
    
    virtual void setEyeTransformType(EyeTransformType e) {
        eyeTransformType = e;
    }
    virtual EyeTransformType getEyeTransformType() const {
        return eyeTransformType;
    }

protected:
    // boolean values to help renderer decide what to attempt to draw
    virtual bool drawsLeftEyeView() {return false;}
    virtual bool drawsRightEyeView() {return false;}
    virtual bool drawsCenterEyeView() {return true;}
    
    // Transform camera from default view to particular eye view
    virtual void setLeftEyeView() = 0;
    virtual void setRightEyeView() = 0;
    virtual void setCenterEyeView() {}
    
    // Restore default view from particular eye view
    virtual void popLeftEyeView() = 0;
    virtual void popRightEyeView() = 0;
    virtual void popCenterEyeView() {}
    
    EyeTransformType eyeTransformType;
};

// OpenGLStereoRenderer is abstract, but implements outer OpenGL
// based stereo rendering block
class OpenGLStereoRenderer : public StereoRenderer
{
    OpenGLStereoRenderer(DrawingRenderer& monoRenderer)
        : monoRenderer(monoRenderer) {}

    void draw() 
    {
        if ( drawsLeftEyeView() ) {
            setLeftEyeView();
            monoRenderer.draw();
            popLeftEyeView();
        }
        
        if ( drawsLeftEyeView() ) {
            setRightEyeView();
            monoRenderer.draw();
            popRightEyeView();
        }
        
        if ( drawsCenterEyeView() ) {
            setCenterEyeView();
            monoRenderer.draw();
            popCenterEyeView();
        }
    }
    
protected:
    GLdouble scratchMatrix[16];
    
    virtual void setLeftEyeView() {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        switch (eyeTransformType) 
        {
        case EYE_TRANSFORM_ROTATE:
            glGetDoublev(GL_MODELVIEW_MATRIX, scratchMatrix); // remember current modelview
            glLoadIdentity();
            glRotated(2.0, 0, 1, 0); // rotate about Y
            glMultMatrixd(scratchMatrix); // end result is premultiply by Rotation
            break;
        // TODO - shear-matrix mode
        default:
            throw std::runtime_error("Oops! Eye transform type not yet implemented");
            break;
        }
    }
    virtual void setRightEyeView() {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        switch (eyeTransformType) 
        {
        case EYE_TRANSFORM_ROTATE:
            glGetDoublev(GL_MODELVIEW_MATRIX, scratchMatrix); // remember current modelview
            glLoadIdentity();
            glRotated(-2.0, 0, 1, 0); // rotate about Y
            glMultMatrixd(scratchMatrix); // end result is premultiply by Rotation
            break;
        // TODO - shear-matrix mode
        default:
            throw std::runtime_error("Oops! Eye transform type not yet implemented");
            break;
        }
    }
    virtual void setCenterEyeView() {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
    }
    
    void popLeftEyeView() {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    void popRightEyeView() {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    void popCenterEyeView() {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    
    DrawingRenderer& monoRenderer;
};

class AnaglyphStereoRenderer : public OpenGLStereoRenderer {
public:
    class ColorMask { union {
        struct {GLboolean r,g,b;}
        GLboolean c[3];
    }};
    
    AnaglyphStereoRenderer(DrawingRenderer& monoRenderer)
        : OpenGLStereoRenderer(monoRenderer, 
                ColorMask leftColorMask,
                ColorMask rightColorMask) {}
    
    virtual void draw() 
    {
        glClear(GL_COLOR_BUFFER_BIT);
        OpenGLStereoRenderer::draw();
    }
    
protected:
    ColorMask leftColorMask;
    ColorMask rightColorMask;
    
    virtual void setLeftEyeView() {
        glColorMask(leftColorMask[0], 
                leftColorMask[1], 
                leftColorMask[2], 
                GL_TRUE);
        OpenGLStereoRenderer::setLeftEyeView();
    }
    
    virtual void popLeftEyeView() {
        OpenGLStereoRenderer::popLeftEyeView();
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
    
    // TODO right eye view
};

#endif /* V3D_STEREO_RENDERER_H_ */
