package com.zhuchao.android.session;

import static android.content.Context.BIND_AUTO_CREATE;
import static com.zhuchao.android.fbase.MessageEvent.MESSAGE_EVENT_AIDL_CANBOX_CLASS_NAME;
import static com.zhuchao.android.fbase.MessageEvent.MESSAGE_EVENT_AIDL_MUSIC_CLASS_NAME;
import static com.zhuchao.android.fbase.MessageEvent.MESSAGE_EVENT_AIDL_PACKAGE_NAME;
import static com.zhuchao.android.fbase.MessageEvent.MESSAGE_EVENT_OCTOPUS_ACTION_CANBOX_SERVICE;
import static com.zhuchao.android.fbase.MessageEvent.MESSAGE_EVENT_OCTOPUS_ACTION_CAR_CLIENT;
import static com.zhuchao.android.fbase.MessageEvent.MESSAGE_EVENT_OCTOPUS_ACTION_CAR_SERVICE;
import static com.zhuchao.android.fbase.MessageEvent.MESSAGE_EVENT_OCTOPUS_CAR_CLIENT;
import static com.zhuchao.android.fbase.MessageEvent.MESSAGE_EVENT_OCTOPUS_CAR_SERVICE;

import android.annotation.SuppressLint;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.media.MediaMetadata;
import android.media.browse.MediaBrowser;
import android.media.session.MediaController;
import android.media.session.MediaSession;
import android.media.session.PlaybackState;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.zhuchao.android.car.aidl.IMyAidlInterface;
import com.zhuchao.android.car.aidl.IMyAidlInterfaceListener;
import com.zhuchao.android.car.aidl.PEventCourier;
import com.zhuchao.android.car.aidl.PMovie;
import com.zhuchao.android.fbase.DataID;
import com.zhuchao.android.fbase.EventCourier;
import com.zhuchao.android.fbase.FileUtils;
import com.zhuchao.android.fbase.MMLog;
import com.zhuchao.android.fbase.MessageEvent;
import com.zhuchao.android.fbase.ObjectList;
import com.zhuchao.android.fbase.TAppUtils;
import com.zhuchao.android.fbase.TCourierEventBus;
import com.zhuchao.android.fbase.TTS;
import com.zhuchao.android.fbase.eventinterface.TCourierEventBusInterface;
import com.zhuchao.android.net.TNetUtils;
import com.zhuchao.android.persist.TPersistent;
import com.zhuchao.android.serialport.TUartFile;
import com.zhuchao.android.session.base.MApplication;

import org.jetbrains.annotations.NotNull;

import java.util.List;

public class Cabinet {
    private static final String TAG = "CABINET";
    @SuppressLint("StaticFieldLeak")
    private static TPlayManager tPlayManager = null;
    @SuppressLint("StaticFieldLeak")
    private static TNetUtils tNetUtils = null;
    @SuppressLint("StaticFieldLeak")
    private static TAppUtils tAppUtils = null;
    @SuppressLint("StaticFieldLeak")
    private static TDeviceManager tDeviceManager = null;
    @SuppressLint("StaticFieldLeak")
    private static TPersistent tPersistent = null;
    private static TCourierEventBusInterface tCourierEventBus = null;
    private static IMyAidlInterface tIMyCarAidlInterface = null;
    public static ObjectList properties = new ObjectList();
    private static MediaBrowser mMediaBrowser;
    @SuppressLint("StaticFieldLeak")
    private static TTS tTts = null;

    public static Context getMyApplicationContext() {
        return MApplication.getAppContext();
    }

    public static synchronized void initialEventBus() {
        if (tCourierEventBus == null) {
            tCourierEventBus = new TCourierEventBus();
        }
    }

    public static synchronized void initialPlayManager(Context context) {
        tPlayManager = TPlayManager.getInstance(context);
        tPlayManager.setPlayOrder(DataID.PLAY_MANAGER_PLAY_ORDER2);//循环顺序播放
        tPlayManager.setAutoPlaySource(DataID.SESSION_SOURCE_FAVORITELIST);//自动播放源列表
    }

    public static synchronized TPlayManager getPlayManager() {
        tPlayManager = TPlayManager.getInstance(MApplication.getAppContext());
        ///tPlayManager.setPlayOrder(DataID.PLAY_MANAGER_PLAY_ORDER2);//循环顺序播放
        ///tPlayManager.setAutoPlaySource(DataID.SESSION_SOURCE_FAVORITELIST);//自动播放源列表
        return tPlayManager;
    }

    public static synchronized TPersistent getPersistent(Context context) {
        if (tPersistent == null) return new TPersistent(context, context.getPackageName());
        else return tPersistent;
    }

    public static synchronized TDeviceManager getDeviceManager() {
        if (tDeviceManager == null) tDeviceManager = new TDeviceManager();
        return tDeviceManager;
    }

    public static synchronized TUartFile getUartDevice(String devicePath, int baudRate) {
        return getDeviceManager().getDevice(devicePath, baudRate);
    }

    public static synchronized TTS getTTS(Context context) {
        if (tTts == null) tTts = new TTS(context);
        return tTts;
    }

    public static synchronized TNetUtils getNet(Context context) {
        if (tNetUtils == null) tNetUtils = new TNetUtils(context);
        return tNetUtils;
    }

    public static synchronized TNetUtils getNetUtils(Context context) {
        if (tNetUtils == null) tNetUtils = new TNetUtils(context);
        return tNetUtils;
    }

    public static synchronized TCourierEventBusInterface getEventBus() {
        if (tCourierEventBus == null) tCourierEventBus = new TCourierEventBus();
        return tCourierEventBus;
    }

    public static synchronized void InitialBaseModules(@NotNull Context context) {
        //MMLog.d(TAG, "Initial few modules for " + TAppProcessUtils.getCurrentProcessNameAndId(context) + " ");
        try {
            initialEventBus();
            initialPlayManager(context);
        } catch (Exception e) {
            MMLog.e(TAG, e.toString());
        }
    }

    public static synchronized void InitialAllModules(@NotNull Context context) {
        //MMLog.d(TAG, "Initial all modules for " + TAppProcessUtils.getCurrentProcessNameAndId(context) + " ");
        try {
            initialEventBus();
            initialPlayManager(context);
            ///initialMyCarAidlInterface(context);
        } catch (Exception e) {
            MMLog.e(TAG, e.toString());
        }
    }

    public static synchronized void FreeModules(@NotNull Context context) {
        try {
            if (tCourierEventBus != null) tCourierEventBus.free();
            if (tPlayManager != null) tPlayManager.free();
            if (tNetUtils != null) tNetUtils.free();
            if (tDeviceManager != null) tDeviceManager.closeAllUartDevice();

            if (tPersistent != null) tPersistent.commit();//持久化数据
            if (tAppUtils != null) tAppUtils.free();
            if (tIMyCarAidlInterface != null) disconnectedMyAidlService(context);
            if (tTts != null) tTts.shutdown();
        } catch (Exception e) {
            //e.printStackTrace();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //aidl
    public static void initialMyCarAidlInterface(Context context) {
        ///Intent intent = new Intent(mContext, MMCarService.class);
        //MMLog.d(TAG,"initial MMCarService proxy");
        Intent intent = new Intent(MESSAGE_EVENT_OCTOPUS_ACTION_CANBOX_SERVICE);
        //intent.setPackage(mContext.getPackageName());
        intent.setComponent(new ComponentName(MESSAGE_EVENT_AIDL_PACKAGE_NAME, MESSAGE_EVENT_AIDL_CANBOX_CLASS_NAME));
        //Intent newIntent = new Intent(createExplicitFromImplicitIntent(mContext,intent));
        boolean isBound = context.bindService(intent, mCanboxServiceConnection, BIND_AUTO_CREATE);
        if (!isBound) MMLog.d(TAG, "Connect to canbox service proxy failed!");
    }

    private static final ServiceConnection mCanboxServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            tIMyCarAidlInterface = IMyAidlInterface.Stub.asInterface(service);
            try {
                tIMyCarAidlInterface.registerListener(mIMyCarAidlInterfaceListener);
            } catch (RemoteException e) {
                //throw new RuntimeException(e);
            }
            MMLog.d(TAG, "Connect to canbox service proxy successfully!");
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            tIMyCarAidlInterface = null;
            MMLog.d(TAG, "Canbox service disconnected!");
        }
    };
    private static final IMyAidlInterfaceListener mIMyCarAidlInterfaceListener = new IMyAidlInterfaceListener.Stub() {

        @Override
        public void onMessageAidlInterface(PEventCourier msg) throws RemoteException {
            MMLog.d(TAG, TAG + "." + msg.toStr());
        }

        @Override
        public void onMessageMusic(int MsgId, int status, long timeChanged, long length, PMovie pMovie) throws RemoteException {

        }
    };

    public static void disconnectedMyAidlService(Context context) {
        if (tIMyCarAidlInterface != null && tIMyCarAidlInterface.asBinder().isBinderAlive()) {
            try {
                context.unbindService(mCanboxServiceConnection);
                tIMyCarAidlInterface.unregisterListener(mIMyCarAidlInterfaceListener);
                tIMyCarAidlInterface = null;
            } catch (RemoteException e) {
                //throw new RuntimeException(e);
            }
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    public static IMyAidlInterface getMyCarAidlInterface() {
        return tIMyCarAidlInterface;
    }

    public static <T> T getMyCarAidlInterface(Class<T> classOfT) {
        if (tIMyCarAidlInterface != null) return classOfT.cast(tIMyCarAidlInterface);
        else return null;
    }

    public static void IAidlSendMessage(PEventCourier pEventCourier) {
        if (tIMyCarAidlInterface != null) {
            try {
                tIMyCarAidlInterface.sendMessage(pEventCourier);
            } catch (RemoteException e) {
                //throw new RuntimeException(e);
                MMLog.d(TAG, "ICarAidlSend " + e);
            }
        } else {
            MMLog.d(TAG, "ICarAidlSend is null!");
        }
    }

    public static void IAidlSendMessage(EventCourier eventCourier) {
        String action = MessageEvent.MESSAGE_EVENT_OCTOPUS_ACTION_HELLO;
        Intent intent = new Intent();
        switch (eventCourier.getId()) {
            case MESSAGE_EVENT_OCTOPUS_CAR_SERVICE:
                action = MESSAGE_EVENT_OCTOPUS_ACTION_CAR_SERVICE;
                intent.setAction(action);
                intent.setPackage(MESSAGE_EVENT_AIDL_PACKAGE_NAME);
                MApplication.getAppContext().sendBroadcast(intent);
                break;
            case MESSAGE_EVENT_OCTOPUS_CAR_CLIENT:
                action = MESSAGE_EVENT_OCTOPUS_ACTION_CAR_CLIENT;
                intent.setAction(action);
                ///intent.setPackage(MApplication.getAppContext().getPackageName());
                ///intent.setComponent(new ComponentName("com.zhuchao.android.car", "com.zhuchao.android.session.GlobalBroadcastReceiver"));
                MApplication.getAppContext().sendBroadcast(intent);
                break;
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //aidl music

    private static void connectRemoteService() {
        ComponentName componentName = new ComponentName(MESSAGE_EVENT_AIDL_PACKAGE_NAME, MESSAGE_EVENT_AIDL_MUSIC_CLASS_NAME);
        // 2.创建MediaBrowser
        mMediaBrowser = new MediaBrowser(getMyApplicationContext(), componentName, mMediaBrowserConnectionCallbacks, null);
        // 3.建立连接
        mMediaBrowser.connect();
    }

    private static final MediaBrowser.ConnectionCallback mMediaBrowserConnectionCallbacks = new MediaBrowser.ConnectionCallback() {
        @Override
        public void onConnected() {
            MMLog.d(TAG, "MediaBrowser.onConnected");
            if (mMediaBrowser.isConnected()) {
                String mediaId = mMediaBrowser.getRoot();
                mMediaBrowser.unsubscribe(mediaId);
                mMediaBrowser.subscribe(mediaId, mBrowserSubscriptionCallback);
                MediaController mMediaController = new MediaController(getMyApplicationContext(), mMediaBrowser.getSessionToken());
                mMediaController.registerCallback(mMediaControllerCompatCallback);
                if (mMediaController.getMetadata() != null) {
                }
            }
        }

        @Override
        public void onConnectionSuspended() {
            MMLog.d(TAG, "MediaBrowser.onConnectionSuspended");
        }

        @Override
        public void onConnectionFailed() {
            MMLog.d(TAG, "MediaBrowser.onConnectionFailed");
        }
    };

    private static final MediaBrowser.SubscriptionCallback mBrowserSubscriptionCallback = new MediaBrowser.SubscriptionCallback() {
        @Override
        public void onChildrenLoaded(@NonNull String parentId, @NonNull List<MediaBrowser.MediaItem> children) {
            super.onChildrenLoaded(parentId, children);
        }

        @Override
        public void onChildrenLoaded(@NonNull String parentId, @NonNull List<MediaBrowser.MediaItem> children, @NonNull Bundle options) {
            super.onChildrenLoaded(parentId, children, options);
        }

        @Override
        public void onError(@NonNull String parentId) {
            super.onError(parentId);
        }

        @Override
        public void onError(@NonNull String parentId, @NonNull Bundle options) {
            super.onError(parentId, options);
        }
    };
    private static final MediaController.Callback mMediaControllerCompatCallback = new MediaController.Callback() {
        //蓝牙音乐信息变化之后在这里进行回调
        @Override
        public void onPlaybackStateChanged(PlaybackState state) {
        }

        @Override
        public void onSessionDestroyed() {
            super.onSessionDestroyed();
        }

        @Override
        public void onSessionEvent(@NonNull String event, @Nullable Bundle extras) {
            super.onSessionEvent(event, extras);
        }

        @Override
        public void onQueueChanged(@Nullable List<MediaSession.QueueItem> queue) {
            super.onQueueChanged(queue);
        }

        @Override
        public void onQueueTitleChanged(@Nullable CharSequence title) {
            super.onQueueTitleChanged(title);
        }

        @Override
        public void onExtrasChanged(@Nullable Bundle extras) {
            super.onExtrasChanged(extras);
        }

        @Override
        public void onAudioInfoChanged(MediaController.PlaybackInfo info) {
            super.onAudioInfoChanged(info);
        }

        @Override
        public void onMetadataChanged(MediaMetadata metadata) {

        }
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //public method
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    public static void openActivity(Context context, String targetPackageName, String targetActivityClassName) {
        Intent intent = new Intent();
        ComponentName cn = new ComponentName(targetPackageName, targetActivityClassName);
        intent.setComponent(cn);
        try {
            context.startActivity(intent);
        } catch (ActivityNotFoundException e) {
            MMLog.e(TAG, String.valueOf(e));
        }
    }

    public static void SendMessage(Context context, String action, String packageName, Bundle bundle) {
        Intent intent = new Intent();
        intent.setAction(action);
        if (bundle != null) intent.putExtras(bundle);
        if (FileUtils.NotEmptyString(packageName)) intent.setPackage(packageName);
        context.sendBroadcast(intent);
    }
}
