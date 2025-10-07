package com.zhuchao.android.fbase;

import static com.zhuchao.android.fbase.FileUtils.EmptyString;
import static com.zhuchao.android.fbase.FileUtils.NotEmptyString;

import android.annotation.SuppressLint;
import android.app.ActivityManager;
import android.app.Application;
import android.app.PendingIntent;
import android.app.usage.UsageStats;
import android.app.usage.UsageStatsManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.IntentSender;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageInstaller;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.provider.Settings;
import android.text.TextUtils;

import androidx.core.content.FileProvider;

import com.zhuchao.android.fbase.eventinterface.AppChangedListener;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

public class TAppUtils {
    private static final String TAG = "AppUtils";
    public static final String UNINSTALL_ACTION = "UNINSTALL";
    public static final String INSTALL_ACTION = "INSTALL";
    public static final String SCANING_ACTION = "SCANING";
    public static final String SCANING_COMPLETE_ACTION = "SCANINGCOMPLETE";
    private final Context mContext;
    private AppChangedListener mAppChangedCallback = null;
    //private ExecutorService mExecutorService;
    private final PackageManager mPackageManager;
    private List<AppInfo> AllAppInfo = null;
    //private List<AppInfor> UserAppInfors = new ArrayList<AppInfor>();
    //private List<String> Filter = new ArrayList<String>();


    public TAppUtils(Context context) {
        mContext = context;
        mAppChangedCallback = null;
        mPackageManager = mContext.getPackageManager();
        AllAppInfo = new ArrayList<AppInfo>();
        //mExecutorService = Executors.newSingleThreadExecutor();
        updateApplicationInfo();
    }

    public List<AppInfo> getAllAppInfo() {
        return AllAppInfo;
    }

    @SuppressLint("QueryPermissionsNeeded")
    private synchronized void getAllAppsInfor() {
        AllAppInfo.clear();
        //获得所有的安装包
        List<PackageInfo> installedPackages = null;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.TIRAMISU) {
            PackageManager.PackageInfoFlags packageInfoFlags = PackageManager.PackageInfoFlags.of(0);
            installedPackages = mPackageManager.getInstalledPackages(packageInfoFlags);
        } else {
            installedPackages = mPackageManager.getInstalledPackages(0);
        }

        //遍历每个安装包，获取对应的信息
        for (PackageInfo packageInfo : installedPackages) {
            final AppInfo appInfo = new AppInfo();
            appInfo.setApplicationInfo(packageInfo.applicationInfo);
            appInfo.setVersionCode(packageInfo.versionCode);
            appInfo.setVersionCodeName(packageInfo.versionName);
            //得到icon
            Drawable drawable = packageInfo.applicationInfo.loadIcon(mPackageManager);
            appInfo.setIcon(drawable);
            //得到程序的名字
            String apkName = packageInfo.applicationInfo.loadLabel(mPackageManager).toString();
            appInfo.setName(apkName);
            //得到程序的包名
            String packageName = packageInfo.packageName;
            appInfo.setPackageName(packageName);
            //得到程序的资源文件夹
            String sourceDir = packageInfo.applicationInfo.sourceDir;
            appInfo.setFilePath(sourceDir);
            File file = new File(sourceDir);
            //得到apk的大小
            long size = file.length();
            appInfo.setSize(size);
            //获取到安装应用程序的标记
            int flags = packageInfo.applicationInfo.flags;
            //表示系统app
            //表示用户app
            appInfo.setUserApp((flags & ApplicationInfo.FLAG_SYSTEM) == 0);
            //表示在sd卡
            //表示内存
            appInfo.setRom((flags & ApplicationInfo.FLAG_EXTERNAL_STORAGE) == 0);
            if (!AllAppInfo.contains(appInfo)) {
                AllAppInfo.add(appInfo);
                callUserCallback(packageName);
            }
        }//for
        callUserCallback(null);
    }

    public void printAllApps() {
        for (AppInfo Info : AllAppInfo) {
            if (Info != null) MMLog.log(TAG, Info.totoString());
        }
    }

    @SuppressLint("UnspecifiedRegisterReceiverFlag")
    public void registerApplicationsReceiver() {
        try {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(Intent.ACTION_PACKAGE_ADDED);
            intentFilter.addAction(Intent.ACTION_PACKAGE_REMOVED);
            intentFilter.addAction(Intent.ACTION_PACKAGE_REPLACED);
            intentFilter.addAction(SCANING_COMPLETE_ACTION);
            intentFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
            intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);

            intentFilter.addDataScheme("package");
            mContext.registerReceiver(AppChangedReceiver, intentFilter);
        } catch (Exception e) {
            //e.printStackTrace();
            MMLog.log(TAG, e.toString());
        }
    }

    public void unRegAppReceiver() {
        try {
            if (mContext != null) mContext.unregisterReceiver(AppChangedReceiver);
        } catch (Exception e) {
            //e.printStackTrace();
        }
    }

    private final BroadcastReceiver AppChangedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            //MMLog.log(TAG, "BroadcastReceiver:" + intent.getAction());
            switch (action) {
                case Intent.ACTION_PACKAGE_ADDED:
                case Intent.ACTION_PACKAGE_REMOVED:
                case Intent.ACTION_PACKAGE_REPLACED: {
                    updateApplicationInfo();
                }
            }
        }
    };

    private void updateApplicationInfo() {
        ThreadUtils.runThread(new Runnable() {
            @Override
            public void run() {
                getAllAppsInfor();
            }
        });
    }

    private void callUserCallback(String packageName) {
        if ((mAppChangedCallback != null)) {
            ThreadUtils.runOnMainUiThread(new Runnable() {
                @Override
                public void run() {
                    mAppChangedCallback.onApplicationChanged(SCANING_COMPLETE_ACTION, packageName);
                }
            });
        }
    }

    public void setApplicationChangedListener(AppChangedListener appChangedCallback) {
        mAppChangedCallback = appChangedCallback;
    }

    public void free() {
        unRegAppReceiver();
        AllAppInfo.clear();
    }

    public AppInfo getAppInfoByName(String Name) {
        for (AppInfo Info : AllAppInfo) {
            if (Info.getName().equals(Name)) return Info;
        }
        return null;
    }

    public String getAppVersionName(String packageName) {
        if (EmptyString(packageName)) return "";
        try {
            PackageManager packageManager = mContext.getPackageManager();
            PackageInfo packageInfo = packageManager.getPackageInfo(packageName, 0);
            return packageInfo == null ? "" : packageInfo.versionName;
        } catch (PackageManager.NameNotFoundException e) {
            return "";
        }
    }

    public static String getAppVersionName(Context context, String packageName) {
        if (EmptyString(packageName)) return "";
        try {
            PackageManager packageManager = context.getPackageManager();
            PackageInfo packageInfo = packageManager.getPackageInfo(packageName, 0);
            return packageInfo == null ? "" : packageInfo.versionName;
        } catch (PackageManager.NameNotFoundException e) {
            return "";
        }
    }

    public int getAppVersionCode(String packageName) {
        if (EmptyString(packageName)) return 0;
        try {
            PackageManager packageManager = mContext.getPackageManager();
            PackageInfo packageInfo = packageManager.getPackageInfo(packageName, 0);
            return (packageInfo == null) ? 0 : packageInfo.versionCode;
        } catch (PackageManager.NameNotFoundException e) {
            return 0;
        }
    }

    public static int getAppVersionCode(Context context, String packageName) {
        if (EmptyString(packageName)) return 0;
        try {
            PackageManager packageManager = context.getPackageManager();
            PackageInfo packageInfo = packageManager.getPackageInfo(packageName, 0);
            return (packageInfo == null) ? 0 : packageInfo.versionCode;
        } catch (PackageManager.NameNotFoundException e) {
            return 0;
        }
    }

    public static String getAppName(Context context, String packageName) {
        if (EmptyString(packageName)) return "";
        try {
            PackageManager packageManager = context.getPackageManager();
            PackageInfo packageInfo = packageManager.getPackageInfo(packageName, 0);
            return packageInfo == null ? "" : packageInfo.applicationInfo.loadLabel(packageManager).toString();
        } catch (PackageManager.NameNotFoundException e) {
            return "";
        }
    }

    public String getAppName(String packageName) {
        if (EmptyString(packageName)) return "";
        try {
            PackageManager packageManager = mContext.getPackageManager();
            PackageInfo packageInfo = packageManager.getPackageInfo(packageName, 0);
            return packageInfo == null ? "" : packageInfo.applicationInfo.loadLabel(packageManager).toString();
        } catch (PackageManager.NameNotFoundException e) {
            return "";
        }
    }

    public String getPackageName(String apkPath) {
        PackageManager packageManager = mContext.getPackageManager();
        PackageInfo packageInfo = packageManager.getPackageArchiveInfo(apkPath, PackageManager.GET_ACTIVITIES);
        if (packageInfo != null) return packageInfo.packageName;
        else return null;
    }

    public static String getPackageName(Context context, String apkPath) {
        PackageManager packageManager = context.getPackageManager();
        PackageInfo packageInfo = packageManager.getPackageArchiveInfo(apkPath, PackageManager.GET_ACTIVITIES);
        if (packageInfo != null) return packageInfo.packageName;
        else return null;
    }

    public AppInfo getAppInfo(String packageName) {
        if (packageName == null) return null;
        for (AppInfo Info : AllAppInfo) {
            //if (Info == null) continue;
            if (packageName.equals(Info.getPackageName())) return Info;
        }
        return null;
    }

    public static PackageInfo getPackageInfo(Context context, String apkPath) {
        PackageManager packageManager = context.getPackageManager();
        return packageManager.getPackageArchiveInfo(apkPath, PackageManager.GET_ACTIVITIES);
    }

    public boolean existApp(String packageName) {
        return getAppInfo(packageName) != null;
    }

    public boolean isAppInstalled(String packageName) {
        if (EmptyString(packageName)) return false;
        try {
            ApplicationInfo info = mContext.getPackageManager().getApplicationInfo(packageName, 0);
            return info.enabled;
        } catch (PackageManager.NameNotFoundException e) {
            //e.printStackTrace();
            return false;
        }
    }

    public static boolean isAppInstalled(Context context, String packageName) {
        if (EmptyString(packageName)) return false;
        try {
            ApplicationInfo info = context.getPackageManager().getApplicationInfo(packageName, 0);
            return info.enabled;
        } catch (PackageManager.NameNotFoundException e) {
            //e.printStackTrace();
            return false;
        }
    }

    public boolean startApp(String packageName) {
        if (EmptyString(packageName)) {
            return false;
        }
        Intent intent = mContext.getPackageManager().getLaunchIntentForPackage(packageName);
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mContext.startActivity(intent);
            MMLog.log(TAG, "startApp ... " + packageName);
            return true;
        } else {
            MMLog.log(TAG, "app not found " + packageName);
            return false;
        }
    }

    public static void startApp(Context context, String packageName) {
        if (EmptyString(packageName)) {
            return;
        }
        Intent intent = context.getPackageManager().getLaunchIntentForPackage(packageName);
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent);
            MMLog.log(TAG, "startApp ... " + packageName);
        } else {
            MMLog.log(TAG, "App Not Found " + packageName);
        }
    }

    public void install(String filePath) {
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        Uri contentUri = FileProvider.getUriForFile(mContext, mContext.getPackageName() + ".fileProvider", new File(filePath));
        intent.setDataAndType(contentUri, "application/vnd.android.package-archive");
        MMLog.log(TAG, "fileProvider path = " + mContext.getPackageName() + ".fileProvider");
        mContext.startActivity(intent);
    }

    public static void install(Context context, String filePath) {
        Intent intent = new Intent(Intent.ACTION_VIEW);
        //BuildConfig.LIBRARY_PACKAGE_NAME
        intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        Uri contentUri = FileProvider.getUriForFile(context, context.getPackageName() + ".fileProvider", new File(filePath));
        intent.setDataAndType(contentUri, "application/vnd.android.package-archive");
        MMLog.log(TAG, "fileProvider path = " + context.getPackageName() + ".fileProvider");
        context.startActivity(intent);
    }

    public void uninstall(String packageName) {
        Uri uri = Uri.parse("package:" + packageName);
        Intent intent = new Intent(Intent.ACTION_DELETE, uri);
        mContext.startActivity(intent);
    }

    public static void uninstall(Context context, String packageName) {
        Uri uri = Uri.parse("package:" + packageName);
        Intent intent = new Intent(Intent.ACTION_DELETE, uri);
        context.startActivity(intent);
    }

    @SuppressLint("RequestInstallPackagesPolicy")
    public synchronized static boolean installSilent(Context context, String apkFilePathName) {
        boolean ret = false;
        File file = new File(apkFilePathName);
        String apkName = apkFilePathName.substring(apkFilePathName.lastIndexOf(File.separator) + 1, apkFilePathName.lastIndexOf(".apk"));
        PackageManager packageManager = context.getPackageManager();
        PackageInstaller packageInstaller = packageManager.getPackageInstaller();
        PackageInstaller.SessionParams params = new PackageInstaller.SessionParams(PackageInstaller.SessionParams.MODE_FULL_INSTALL);
        PackageInstaller.Session session = null;
        OutputStream outputStream = null;
        FileInputStream inputStream = null;
        try {
            //创建Session //开启Session
            int sessionId = packageInstaller.createSession(params);
            session = packageInstaller.openSession(sessionId);
            //获取输出流，用于将apk写入session
            outputStream = session.openWrite(apkName, 0, -1);
            inputStream = new FileInputStream(file);
            byte[] buffer = new byte[4096];
            int n; //读取apk文件写入session
            while ((n = inputStream.read(buffer)) > 0) {
                outputStream.write(buffer, 0, n);
            }
            inputStream.close();
            inputStream = null;
            outputStream.flush();
            outputStream.close();
            outputStream = null;

            Intent intent = new Intent();
            intent.setAction("android.intent.action.SILENT_INSTALL_PACKAGE_COMPLETE");
            intent.putExtra("apkFilePathName", apkFilePathName);
            PendingIntent pendingIntent = PendingIntent.getBroadcast(context, 0, intent, PendingIntent.FLAG_IMMUTABLE);
            IntentSender intentSender = pendingIntent.getIntentSender();
            session.commit(intentSender);//提交启动安装
            //MMLog.log(TAG, "installed: "+apkPath);
            ret = true;
        } catch (Exception e) {
            MMLog.log(TAG, e.toString());
            if (session != null) {
                session.abandon();
            }
        } finally {
            if (outputStream != null) {
                try {
                    outputStream.close();
                } catch (IOException e) {
                    MMLog.log(TAG, e.toString());
                }
            }
            if (inputStream != null) {
                try {
                    inputStream.close();
                } catch (IOException e) {
                    MMLog.log(TAG, e.toString());
                }
            }
        }
        return ret;
    }

    public synchronized static void uninstallApk(String packageName) {
        try {
            MMLog.d(TAG, "going to uninstall app packageName=" + packageName);
            String[] args = {"pm", "uninstall", "-k", "--user", "0", packageName};
            ProcessBuilder processBuilder = new ProcessBuilder(args);
            Process process = processBuilder.start();
            BufferedReader successResult = null;
            BufferedReader errorResult = null;
            StringBuilder successMsg = new StringBuilder();
            StringBuilder errorMsg = new StringBuilder();
            try {
                successResult = new BufferedReader(new InputStreamReader(process.getInputStream()));
                errorResult = new BufferedReader(new InputStreamReader(process.getErrorStream()));
                String line;
                while ((line = successResult.readLine()) != null) {
                    successMsg.append(line);
                }
                while ((line = errorResult.readLine()) != null) {
                    errorMsg.append(line);
                }
                if (NotEmptyString(errorMsg.toString())) MMLog.i(TAG, "uninstall " + errorMsg.toString());
                if (NotEmptyString(successMsg.toString()) && successMsg.toString().contains("Success")) {
                    MMLog.i(TAG, "uninstall " + successMsg.toString());
                }
            } catch (Exception e) {
                MMLog.e(TAG, String.valueOf(e));
                ///e.printStackTrace();
            } finally {
                try {
                    if (successResult != null) {
                        successResult.close();
                    }
                } catch (IOException e) {
                    MMLog.e(TAG, e.toString());
                    //e.printStackTrace();
                }
                try {
                    if (errorResult != null) {
                        errorResult.close();
                    }
                } catch (IOException e) {
                    MMLog.e(TAG, e.toString());
                    //e.printStackTrace();
                }
                try {
                    if (process != null) {
                        process.destroy();
                    }
                } catch (Exception e) {
                    MMLog.e(TAG, e.toString());
                    //e.printStackTrace();
                }
            }
        } catch (Exception e) {
            MMLog.e(TAG, String.valueOf(e));
            ///e.printStackTrace();
        }
    }

    public synchronized static void killApplication(Context context, String packageName) {
        ActivityManager mActivityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        Method method = null;
        try {
            method = Class.forName("android.app.ActivityManager").getMethod("forceStopPackage", String.class);
            method.invoke(mActivityManager, packageName);
            MMLog.i(TAG, "Kill application: " + packageName);
        } catch (NoSuchMethodException | ClassNotFoundException | IllegalAccessException | InvocationTargetException e) {
            MMLog.log(TAG, e.toString());//e.printStackTrace();
        }
    }

    public static ObjectArray<String> getRunningProcess(Context context) {
        ActivityManager mActivityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> mList = mActivityManager.getRunningAppProcesses();
        ObjectArray<String> objectArray = new ObjectArray<String>();
        //ArrayList<Integer> arrayList = new ArrayList<>();
        for (ActivityManager.RunningAppProcessInfo runningAppProcessInfo : mList) {
            objectArray.add(runningAppProcessInfo.processName);
        }
        return objectArray;
    }

    public static boolean isProcessRunning(Context context, String PackageName) {
        ActivityManager activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> mList = activityManager.getRunningAppProcesses();
        for (ActivityManager.RunningAppProcessInfo runningAppProcessInfo : mList) {
            if (runningAppProcessInfo.processName.equals(PackageName)) return true;
        }
        return false;
    }

    public static boolean isServiceRunning(Context context, String serviceName) {
        ActivityManager activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        ArrayList<ActivityManager.RunningServiceInfo> mList = (ArrayList<ActivityManager.RunningServiceInfo>) activityManager.getRunningServices(30);

        for (ActivityManager.RunningServiceInfo runningServiceInfo : mList) {
            if (runningServiceInfo.service.getClassName().equals(serviceName)) return true;
        }
        return false;
    }

    public static boolean isForegroundApp(Context context) {
        return TextUtils.equals(getForegroundActivityName(context), context.getPackageName());
    }

    /**
     * 获取时间段内，
     */
    public static long getAppTotalForegroundTime(Context context) {
        long END_TIME = System.currentTimeMillis();
        UsageStats usageStats = getPackageUsageStats(context, END_TIME - 60000, END_TIME);
        return usageStats != null ? usageStats.getTotalTimeInForeground() : 0;
    }

    /**
     * 获取记录当前应用的UsageStats对象
     */
    public static UsageStats getPackageUsageStats(Context context, long startTime, long endTime) {
        List<UsageStats> UsageStatsList = getUsageStatsList(context, startTime, endTime);
        if (UsageStatsList == null || UsageStatsList.isEmpty()) return null;

        for (UsageStats usageStats : UsageStatsList) {
            if (TextUtils.equals(usageStats.getPackageName(), context.getPackageName())) {
                return usageStats;
            }
        }
        return null;
    }

    public static String getForegroundActivityName(Context context) {
        String topClassName = null;
        long END_TIME = System.currentTimeMillis();
        ActivityManager activityManager = (ActivityManager) context.getApplicationContext().getSystemService(Context.ACTIVITY_SERVICE);
        UsageStats initStat = getForegroundUsageStats(context, END_TIME - 60000 * 60, END_TIME);
        if (initStat != null) {
            topClassName = initStat.getPackageName();
            MMLog.log(TAG, "getForegroundActivityName topClassName=" + topClassName);
        }
        return topClassName;
    }

    /**
     * 获取记录前台应用的UsageStats对象
     * <uses-permission android:name="android.permission.PACKAGE_USAGE_STATS"/>
     */
    private static UsageStats getForegroundUsageStats(Context context, long startTime, long endTime) {
        UsageStats usageStatsResult = null;
        List<UsageStats> UsageStatsList = getUsageStatsList(context, startTime, endTime);
        if (UsageStatsList == null || UsageStatsList.isEmpty()) return null;
        for (UsageStats usageStats : UsageStatsList) {
            if (usageStatsResult == null || usageStatsResult.getLastTimeUsed() < usageStats.getLastTimeUsed()) {
                usageStatsResult = usageStats;//找出最最近使用过的APP
            }
        }
        return usageStatsResult;
    }

    /**
     * 通过UsageStatsManager获取List<UsageStats>集合
     */
    public static List<UsageStats> getUsageStatsList(Context context, long startTime, long endTime) {
        UsageStatsManager manager = (UsageStatsManager) context.getApplicationContext().getSystemService(Context.USAGE_STATS_SERVICE);
        //UsageStatsManager.INTERVAL_WEEKLY，UsageStatsManager的参数定义了5个，具体查阅源码
        List<UsageStats> UsageStatsList = manager.queryUsageStats(UsageStatsManager.INTERVAL_BEST, startTime, endTime);
        if (UsageStatsList == null || UsageStatsList.size() == 0) {// 没有权限，获取不到数据
            ///Intent intent = new Intent(Settings.ACTION_USAGE_ACCESS_SETTINGS);
            ///intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            ///context.getApplicationContext().startActivity(intent);
            ///Toast t = Toast.makeText(context,"need permmition to access settings .", Toast.LENGTH_LONG);
            ///t.show();
            ///MMLog.log(TAG, "getUsageStatsList fail,need Settings.ACTION_USAGE_ACCESS_SETTINGS permission");
            return null;
        }
        return UsageStatsList;
    }

    public static void startSystemSetting(Context context) {
        context.startActivity(new Intent(Settings.ACTION_SETTINGS).setFlags(Intent.FLAG_ACTIVITY_NEW_TASK));
    }

    public static void startWifiSetting(Context context) {
        context.startActivity(new Intent(Settings.ACTION_WIFI_SETTINGS).setFlags(Intent.FLAG_ACTIVITY_NEW_TASK));
    }

    @SuppressLint("PrivateApi")
    public static Application getApplicationByReflection() {
        try {
            return (Application) Class.forName("android.app.ActivityThread").getMethod("currentApplication").invoke(null, (Object[]) null);
        } catch (Exception e) {
            //e.printStackTrace();
            MMLog.d(TAG, "Exception:" + e);
        }
        return null;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////
    public static class AppInfo {
        private ApplicationInfo applicationInfo;
        private String packageName;
        private String name;
        private Drawable icon;
        private long size;
        private boolean isUserApp;//用户APP 还是系统APP
        private boolean isRom; //安装位置
        private String filePath;//文件位置
        private long versionCode = 0;
        private String versionCodeName = "0";

        public AppInfo() {
        }

        public AppInfo(String name, Drawable icon) {
            this.name = name;
            this.icon = icon;
        }

        public ApplicationInfo getApplicationInfo() {
            return applicationInfo;
        }

        public void setApplicationInfo(ApplicationInfo applicationInfo) {
            this.applicationInfo = applicationInfo;
        }

        public String getPackageName() {
            return packageName;
        }

        public void setPackageName(String packageName) {
            this.packageName = packageName;
        }

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        public Drawable getIcon() {
            return icon;
        }

        public void setIcon(Drawable icon) {
            this.icon = icon;
        }

        public long getSize() {
            return size;
        }

        public void setSize(long size) {
            this.size = size;
        }

        public boolean isUserApp() {
            return isUserApp;
        }

        public void setUserApp(boolean userApp) {
            this.isUserApp = userApp;
        }

        public boolean isRom() {
            return isRom;
        }

        public void setRom(boolean rom) {
            this.isRom = rom;
        }

        public String getFilePath() {
            return filePath;
        }

        public void setFilePath(String filePath) {
            this.filePath = filePath;
        }

        public long getVersionCode() {
            return versionCode;
        }

        public void setVersionCode(long versionCode) {
            this.versionCode = versionCode;
        }

        public String getVersionCodeName() {
            return versionCodeName;
        }

        public void setVersionCodeName(String versionCodeName) {
            this.versionCodeName = versionCodeName;
        }

        public String totoString() {
            return "Name=" + name + ",Version=" + versionCode + ",PackageName=" + packageName + ",Ico=" + icon + ",Size=" + size + ",User=" + isUserApp + ",Rom=" + isRom + ",Path=" + filePath;
        }
    }


}
