package com.example.myapplication_test;

import android.opengl.GLES20;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;



public class MyPattern{

    private final int scale = 128;

    private final int mProgram;

    private FloatBuffer vertexBuffer;
    private FloatBuffer textureBuffer;

    private ShortBuffer drawListBuffer;
    private FloatBuffer colorBuffer;

    private FloatBuffer amb_light_buffer;
    private FloatBuffer diff_light_buffer;
    private FloatBuffer spec_light_buffer;
    private FloatBuffer pos_light_buffer;
    private FloatBuffer spot_dir_buffer;

    private int positionHandle;
    private int textureHandle;
    private int colorHandle;
    private int vPMatrixHandle;




//    private float[] vertexPoints = getPosition();
//    private byte[][] vertexCoord = getIntensity();


    private float[] getPosition(){
        float[] positions = new float[scale * scale * scale * 3];
        for (int x = 0; x < scale; ++x){
            for (int y = 0; y < scale; ++y){
                for (int z = 0; z < scale; z++){
                    positions[(scale * scale * z + scale * y + x) * 3] = (float)((float)(x-64) / 128.0);
                    positions[(scale * scale * z + scale * y + x) * 3 + 1] = (float)((float)(y-64) / 128.0);
                    positions[(scale * scale * z + scale * y + x) * 3 + 2] = (float)((float)(z-64) / 128.0);
                }
            }
        }
        return positions;
    }

    private float[] vertexPoints = {0};

    private final float[] vertexCoord={
            0.0f,0.0f,     //左上角
            0.0f,1.0f,     //左下角
            1.0f,0.0f,     //右上角
            1.0f,1.0f,     //右下角
    };

    private final String vertexShaderCode =
            // This matrix member variable provides a hook to manipulate
            // the coordinates of the objects that use this vertex shader
            "uniform mat4 uMVPMatrix;" +
             "attribute vec2 vCoordinate;" +
             "attribute vec4 vPosition;" +
             "varying vec2 aCoordinate;" +
                    "void main() {" +
                    "  gl_Position = uMVPMatrix * vPosition;" +
                    "  aCoordinate = vCoordinate;" +
                    "}";


    private final String fragmentShaderCode =
            "precision mediump float;" +
//                    "varying vec4 ourColor;" +
                    "uniform sampler2D vTexture;" +
                    "varying vec2 aCoordinate;" +
                    "void main() {" +
//                    "   gl_FragColor = ourColor;" +
                    "   gl_FragColor=texture2D(vTexture,aCoordinate);" +
                    "}";




    public MyPattern() {


        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER,
                vertexShaderCode);
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER,
                fragmentShaderCode);

        // create empty OpenGL ES Program
        mProgram = GLES20.glCreateProgram();

        // add the vertex shader to program
        GLES20.glAttachShader(mProgram, vertexShader);

        // add the fragment shader to program
        GLES20.glAttachShader(mProgram, fragmentShader);

        // creates OpenGL ES program executables
        GLES20.glLinkProgram(mProgram);

    }





    public void draw(float[] mvpMatrix, int texId) {
        // Add program to OpenGL ES environment
        GLES20.glUseProgram(mProgram);

        //设置z轴坐标
        setVertex(texId);

        //设置一下buffer
        BufferSet();

        //get handle to vertex shader's vPosition member
        positionHandle = GLES20.glGetAttribLocation(mProgram, "vPosition");
        textureHandle = GLES20.glGetAttribLocation(mProgram, "vCoordinate");

        // get handle to vertex shader's uMVPMatrix member
        vPMatrixHandle = GLES20.glGetUniformLocation(mProgram,"uMVPMatrix");



        // Pass the projection and view transformation to the shader
        GLES20.glUniformMatrix4fv(vPMatrixHandle, 1, false, mvpMatrix, 0);

        //准备坐标数据
        GLES20.glVertexAttribPointer(positionHandle, 3, GLES20.GL_FLOAT, false, 0, vertexBuffer);

        //启用顶点的句柄
        GLES20.glEnableVertexAttribArray(positionHandle);

        //准备纹理的坐标
        GLES20.glVertexAttribPointer(textureHandle, 2, GLES20.GL_FLOAT, false, 0, textureBuffer);

        //启用纹理的句柄
        GLES20.glEnableVertexAttribArray(textureHandle);



        GLES20.glActiveTexture(GLES20.GL_TEXTURE0); //设置使用的纹理编号
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D,texId); //绑定指定的纹理id



        //绘制三个点
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, vertexPoints.length/2);

        //禁止顶点数组的句柄
        GLES20.glDisableVertexAttribArray(positionHandle);
        GLES20.glDisableVertexAttribArray(textureHandle);


        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D,0); //解除绑定指定的纹理id

    }


    //加载着色器
    private static int loadShader(int type, String shaderCode){

        // create a vertex shader type (GLES20.GL_VERTEX_SHADER)
        // or a fragment shader type (GLES20.GL_FRAGMENT_SHADER)
        int shader = GLES20.glCreateShader(type);

        // add the source code to the shader and compile it
        GLES20.glShaderSource(shader, shaderCode);
        GLES20.glCompileShader(shader);

        return shader;
    }



    private void BufferSet(){
        //分配内存空间,每个浮点型占4字节空间
        vertexBuffer = ByteBuffer.allocateDirect(vertexPoints.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        //传入指定的坐标数据
        vertexBuffer.put(vertexPoints);
        vertexBuffer.position(0);


        //分配内存空间,每个浮点型占4字节空间
        textureBuffer = ByteBuffer.allocateDirect(vertexCoord.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        //传入指定的坐标数据
        textureBuffer.put(vertexCoord);
        textureBuffer.position(0);
    }


    private void setVertex(int texID){
        vertexPoints = new float[]{
                -1.0f, 1.0f, (float) (texID) * 2.0f / 224.0f - 1.0f,
                -1.0f, -1.0f, (float) (texID) * 2.0f / 224.0f - 1.0f,
                1.0f, 1.0f, (float) (texID) * 2.0f / 224.0f - 1.0f,
                1.0f, -1.0f, (float) (texID) * 2.0f / 224.0f - 1.0f
        };
    }


//    private void setVertex(int texID){
//        vertexPoints = new float[]{
//                -1.0f, 1.0f, (float) (texID) * 2.0f / 128.0f - 1.0f,
//                -1.0f, -1.0f, (float) (texID) * 2.0f / 128.0f - 1.0f,
//                1.0f, 1.0f, (float) (texID) * 2.0f / 128.0f - 1.0f,
//                1.0f, -1.0f, (float) (texID) * 2.0f / 128.0f - 1.0f
//        };
//    }


}
