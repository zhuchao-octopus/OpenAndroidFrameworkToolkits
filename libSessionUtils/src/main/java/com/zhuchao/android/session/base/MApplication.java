package com.zhuchao.android.session.base;

import android.app.Application;
import android.content.Context;
import android.os.Build;
import android.view.WindowManager;

import com.zhuchao.android.fbase.MMLog;
import com.zhuchao.android.fbase.TAppProcessUtils;
import com.zhuchao.android.session.Cabinet;
import com.zhuchao.android.session.GlobalBroadcastReceiver;

public class MApplication extends Application {
    private static final String TAG = "MApplication";
    protected static Context appContext = null;//需要使用的上下文对象
    public static WindowManager.LayoutParams LayoutParams = new WindowManager.LayoutParams();

    public static Context getAppContext() {
        return appContext;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        appContext = this.getApplicationContext();
        String appName = TAppProcessUtils.getCurrentProcessNameAndId(this);
        String sdkVersion = String.valueOf(Build.VERSION.SDK_INT);
        /////////////////////////////////////////////////////////////////////////////////
        //初始化各模块组件
        MMLog.d(TAG, "START.. application for " + appName + " sdk.version=" + sdkVersion);
        GlobalBroadcastReceiver.registerGlobalBroadcastReceiver(this);
        if (appName != null && appName.contains("com.zhuchao.android.car")) {
            MMLog.d(TAG, "Initial few modules for " + TAppProcessUtils.getCurrentProcessNameAndId(this) + " ");
            Cabinet.InitialBaseModules(this);
        } else {
            MMLog.d(TAG, "Initial all modules for " + TAppProcessUtils.getCurrentProcessNameAndId(this) + " ");
            Cabinet.InitialAllModules(this);
        }
        //////////////////////////////////////////////////////////////////////////////////
    }

    @Override
    public void onLowMemory() {
        super.onLowMemory();
    }

    @Override
    public void onTrimMemory(int level) {
        super.onTrimMemory(level);
    }

    @Override
    public void registerActivityLifecycleCallbacks(ActivityLifecycleCallbacks callback) {
        super.registerActivityLifecycleCallbacks(callback);
    }

    @Override
    public void onTerminate() {
        GlobalBroadcastReceiver.unregisterGlobalBroadcastReceiver(this);
        Cabinet.FreeModules(this);
        MMLog.d("MApplication", "onTerminate! " + TAppProcessUtils.getCurrentProcessNameAndId(this));
        super.onTerminate();
    }

}
