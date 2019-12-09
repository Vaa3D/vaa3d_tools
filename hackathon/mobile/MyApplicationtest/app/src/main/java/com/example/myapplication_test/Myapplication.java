package com.example.myapplication_test;

import android.app.Application;
import android.content.Context;


//用于在app全局获取context

public class Myapplication extends Application {
    private static Context context;
    @Override
    public void onCreate() {
        super.onCreate();
        context = getApplicationContext();
    }
    public static Context getContext() {
        return context;
    }
}
