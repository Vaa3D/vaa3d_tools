package com.example.myapplication_test;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;


import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;

import static java.lang.Math.pow;
import static java.lang.Math.sqrt;

public class MainActivity extends AppCompatActivity {
    private MyGLSurfaceView myGLSurfaceView;
    private MyRenderer myrenderer;
    private static final String DEBUG_TAG = "Gestures";
    private static Context context;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //接受从fileactivity传递过来的文件路径
        Intent intent = getIntent();
        String filepath = intent.getStringExtra(FileActivity.EXTRA_MESSAGE);

        myrenderer = new MyRenderer();
        myrenderer.SetPath(filepath);

        myGLSurfaceView = new MyGLSurfaceView(this);
        setContentView(myGLSurfaceView);
    }


    //renderer 的生存周期和activity保持一致
    @Override
    protected void onPause() {
        super.onPause();
        myGLSurfaceView.onPause();

    }

    @Override
    protected void onResume() {
        super.onResume();
        myGLSurfaceView.onResume();
    }



    //opengl中的显示区域
    class MyGLSurfaceView extends GLSurfaceView {
        private float X, Y;
        private double dis_start;
        private boolean isZooming;


        public MyGLSurfaceView(Context context) {
            super(context);

            //设置一下opengl版本；
            setEGLContextClientVersion(2);

            setRenderer(myrenderer);
            setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        }
        public boolean onTouchEvent(MotionEvent motionEvent) {
            //ACTION_DOWN不return true，就无触发后面的各个事件
            if (motionEvent != null) {
                final float normalizedX =toOpenGLCoord(this,motionEvent.getX(),true);
                final float normalizedY =toOpenGLCoord(this,motionEvent.getY(),false);
//
//                final float normalizedX =motionEvent.getX();
//                final float normalizedY =motionEvent.getY();

                switch (motionEvent.getActionMasked()){
                    case MotionEvent.ACTION_DOWN:
                        X=normalizedX;
                        Y=normalizedY;
                        break;
                    case MotionEvent.ACTION_POINTER_DOWN:
                        isZooming=true;
                        float x1=toOpenGLCoord(this,motionEvent.getX(1),true);
                        float y1=toOpenGLCoord(this,motionEvent.getY(1),false);

//                        float x1=motionEvent.getX(1);
//                        float y1=motionEvent.getY(1);
                        dis_start=computeDis(normalizedX,x1,normalizedY,y1);

                        break;
                    case MotionEvent.ACTION_MOVE:
                        if(isZooming){
                            float x2=toOpenGLCoord(this,motionEvent.getX(1),true);
                            float y2=toOpenGLCoord(this,motionEvent.getY(1),false);

//                            float x2=motionEvent.getX(1);
//                            float y2=motionEvent.getY(1);
                            double dis=computeDis(normalizedX,x2,normalizedY,y2);
                            double scale=dis/dis_start;
                            myrenderer.zoom((float) scale);
                            dis_start=dis;
                        }else {
                            myrenderer.rotate(X - normalizedX, normalizedY - Y, (float)(computeDis(X, normalizedX, normalizedY, Y)));
                            X = normalizedX;
                            Y = normalizedY;
                        }
                        break;
//                    case MotionEvent.ACTION_POINTER_UP:
//                        isZooming=false;
//                        X = normalizedX;
//                        Y = normalizedY;
//                        break;
                    case MotionEvent.ACTION_UP:
                        break;
                    default:break;
                }
                return true;
            }
            return false;
        }
        private float toOpenGLCoord(View view,float value,boolean isWidth){
            if(isWidth){
                return (value / (float) view.getWidth()) * 2 - 1;
            }else {
                return -((value / (float) view.getHeight()) * 2 - 1);
            }
        }


        private double computeDis(float x1,float x2,float y1,float y2){
            return sqrt(pow((x2-x1),2)+pow((y2-y1),2));
        }
    }
}
