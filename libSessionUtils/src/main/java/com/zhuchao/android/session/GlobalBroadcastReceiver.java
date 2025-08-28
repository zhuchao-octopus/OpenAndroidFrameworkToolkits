package com.zhuchao.android.session;

import static com.zhuchao.android.fbase.MessageEvent.MESSAGE_EVENT_AIDL_CANBOX_CLASS_NAME;
import static com.zhuchao.android.fbase.MessageEvent.MESSAGE_EVENT_AIDL_MUSIC_CLASS_NAME;
import static com.zhuchao.android.fbase.MessageEvent.MESSAGE_EVENT_AIDL_NOTICE_CENTER_CLASS_NAME;
import static com.zhuchao.android.fbase.MessageEvent.MESSAGE_EVENT_AIDL_PACKAGE_NAME;
import static com.zhuchao.android.fbase.MessageEvent.MESSAGE_EVENT_AIDL_RECORDER_CLASS_NAME;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothA2dp;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.media.AudioManager;
import android.net.Uri;

import com.zhuchao.android.fbase.EventCourier;
import com.zhuchao.android.fbase.MMLog;
import com.zhuchao.android.fbase.MessageEvent;
import com.zhuchao.android.fbase.TAppProcessUtils;

import java.io.File;
import java.util.Objects;

public class GlobalBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = "GlobalReceiver";
    //public static final String Action_OCTOPUS_permission = "com.octopus.android.action.OCTOPUS.PERMISSION";
    public static final String ACTION_BOOT_COMPLETED = "android.intent.action.BOOT_COMPLETED";
    public static final String ACTION_PAIRING_CANCEL = "android.bluetooth.device.action.PAIRING_CANCEL";
    public static GlobalBroadcastReceiver mGlobalBroadcastReceiver = null;
    public static final String ACTION_MEDIA_SCANNER_SCAN_DIR = "android.intent.action.MEDIA_SCANNER_SCAN_DIR";

    @SuppressLint("UnspecifiedRegisterReceiverFlag")
    public synchronized static void registerGlobalBroadcastReceiver(Context context) {
        if (mGlobalBroadcastReceiver == null) {
            mGlobalBroadcastReceiver = new GlobalBroadcastReceiver();
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(Intent.ACTION_MEDIA_SHARED); //如果SDCard未安装,并通过USB大容量存储共享返回
            intentFilter.addAction(Intent.ACTION_MEDIA_MOUNTED); //表明sd对象是存在并具有读/写权限
            intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED); //SDCard已卸掉,如果SDCard是存在但没有被安装
            intentFilter.addAction(Intent.ACTION_MEDIA_CHECKING); //表明对象正在磁盘检查
            intentFilter.addAction(Intent.ACTION_MEDIA_EJECT); //物理的拔出 SDCARD
            intentFilter.addAction(Intent.ACTION_MEDIA_REMOVED); //完全拔出
            intentFilter.addDataScheme("file");
            intentFilter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
            intentFilter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
            //intentFilter.addAction(UsbManager.ACTION_USB_STATE);
            intentFilter.addAction(UsbManager.ACTION_USB_ACCESSORY_ATTACHED);
            intentFilter.addAction(UsbManager.ACTION_USB_ACCESSORY_DETACHED);

            intentFilter.addAction(Intent.ACTION_LOCALE_CHANGED);
            intentFilter.addAction(Intent.ACTION_CONFIGURATION_CHANGED);
            intentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);

            intentFilter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
            intentFilter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
            intentFilter.addAction(BluetoothAdapter.ACTION_SCAN_MODE_CHANGED);
            intentFilter.addAction(BluetoothAdapter.ACTION_CONNECTION_STATE_CHANGED);

            intentFilter.addAction(BluetoothDevice.ACTION_ACL_CONNECTED);
            intentFilter.addAction(BluetoothDevice.ACTION_ACL_DISCONNECTED);
            intentFilter.addAction(BluetoothDevice.ACTION_ACL_DISCONNECT_REQUESTED);
            intentFilter.addAction(BluetoothDevice.ACTION_PAIRING_REQUEST);
            intentFilter.addAction(BluetoothDevice.ACTION_FOUND);

            intentFilter.addAction(BluetoothA2dp.ACTION_CONNECTION_STATE_CHANGED);
            intentFilter.addAction(BluetoothA2dp.ACTION_PLAYING_STATE_CHANGED);


            intentFilter.addAction(MessageEvent.MESSAGE_EVENT_OCTOPUS_ACTION_HELLO);
            intentFilter.addAction(MessageEvent.MESSAGE_EVENT_OCTOPUS_ACTION_TEST);
            context.registerReceiver(mGlobalBroadcastReceiver, intentFilter);
        }
    }

    public synchronized static void unregisterGlobalBroadcastReceiver(Context context) {
        try {
            if (mGlobalBroadcastReceiver != null) context.unregisterReceiver(mGlobalBroadcastReceiver);
            mGlobalBroadcastReceiver = null;
        } catch (Exception e) {
            //throw new RuntimeException(e);
        }
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        MMLog.d(TAG, TAG + " action=" + intent.getAction() + " " + context.getPackageName());
        switch (Objects.requireNonNull(intent.getAction())) {
            case ACTION_BOOT_COMPLETED:
                Intent intent1 = new Intent();
                intent1.setComponent(new ComponentName(MESSAGE_EVENT_AIDL_PACKAGE_NAME, MESSAGE_EVENT_AIDL_CANBOX_CLASS_NAME));
                context.startService(intent1);

                ///Intent intent2 = new Intent();
                ///intent2.setComponent(new ComponentName(MESSAGE_EVENT_AIDL_PACKAGE_NAME, MESSAGE_EVENT_AIDL_MUSIC_CLASS_NAME));
                ///context.startService(intent2);

                ///Intent intent3 = new Intent();
                ///intent3.setComponent(new ComponentName(MESSAGE_EVENT_AIDL_PACKAGE_NAME, MESSAGE_EVENT_AIDL_RECORDER_CLASS_NAME));
                ///context.startService(intent3);

                ///Intent intent4 = new Intent();
                ///intent4.setComponent(new ComponentName(MESSAGE_EVENT_AIDL_PACKAGE_NAME, MESSAGE_EVENT_AIDL_NOTICE_CENTER_CLASS_NAME));
                ///context.startService(intent4);
                break;
            case MessageEvent.MESSAGE_EVENT_OCTOPUS_ACTION_HELLO:
                MMLog.add(null);
                MMLog.add("*Hello " + TAppProcessUtils.getCurrentProcessNameAndId(context));
                //Cabinet.getEventBus().printAllEventListener();
                MMLog.add(Cabinet.getEventBus().getEventListeners());
                MMLog.print(TAG);
                break;
            case Intent.ACTION_MEDIA_MOUNTED:
                EventCourier eventCourier1 = new EventCourier(mGlobalBroadcastReceiver.getClass(), MessageEvent.MESSAGE_EVENT_USB_MOUNTED);
                eventCourier1.setObj(intent);
                Cabinet.getEventBus().post(eventCourier1);
                break;
            case Intent.ACTION_MEDIA_UNMOUNTED:
                EventCourier eventCourier2 = new EventCourier(mGlobalBroadcastReceiver.getClass(), MessageEvent.MESSAGE_EVENT_USB_UNMOUNT);
                eventCourier2.setObj(intent);
                Cabinet.getEventBus().post(eventCourier2);
                break;
            case Intent.ACTION_MEDIA_EJECT:
                ///Uri data = intent.getData();
                ///if (bundle != null) {
                ///    for (String key : bundle.keySet())
                ///        MMLog.log(TAG, "USB device eject," + key + ":" + bundle.toString() + " url="+data.toString());
                ///}
                EventCourier eventCourier3 = new EventCourier(mGlobalBroadcastReceiver.getClass(), MessageEvent.MESSAGE_EVENT_USB_EJECT);
                eventCourier3.setObj(intent);
                Cabinet.getEventBus().post(eventCourier3);
                break;
            case UsbManager.ACTION_USB_DEVICE_ATTACHED:
            case UsbManager.ACTION_USB_DEVICE_DETACHED:
                UsbDevice usbDevice = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                if (usbDevice != null) {
                    MMLog.i(TAG, "getProductId=" + usbDevice.getProductId());
                    MMLog.i(TAG, "getVendorId=" + usbDevice.getVendorId());
                    MMLog.i(TAG, "getSerialNumber=" + usbDevice.getSerialNumber());
                }
                break;
            case Intent.ACTION_MEDIA_SCANNER_FINISHED:
                EventCourier eventCourier4 = new EventCourier(mGlobalBroadcastReceiver.getClass(), MessageEvent.MESSAGE_EVENT_USB_SCANNING_FINISHED);
                eventCourier4.setObj(intent);
                Cabinet.getEventBus().post(eventCourier4);
                break;
            //BluetoothAdapter 相关广播
            case BluetoothAdapter.ACTION_STATE_CHANGED: //蓝牙开关
                //("Bluetooth Switch state :STATE_OFF --> 10, STATE_TURNING_ON --> 11, STATE_ON --> 12 ,STATE_TURNING_OFF --> 13 ");
                int switchState = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, -1);
                MMLog.d(TAG, "Bluetooth Switch change to= " + switchState);
                break;
            case BluetoothAdapter.ACTION_SCAN_MODE_CHANGED: //蓝牙扫描状态修改
                //("Bluetooth scan mode :SCAN_MODE_NONE --> 20, SCAN_MODE_CONNECTABLE --> 21, SCAN_MODE_CONNECTABLE_DISCOVERABLE --> 23");
                int scanMode = intent.getIntExtra(BluetoothAdapter.EXTRA_SCAN_MODE, -1);
                MMLog.d(TAG, "Bluetooth scanMode change to = " + scanMode);
                break;
            case BluetoothAdapter.ACTION_CONNECTION_STATE_CHANGED: //蓝牙连接，和ACL连接的区别 ?
                //("bluetooth ConnectState :STATE_DISCONNECTED=0, STATE_CONNECTING=1, STATE_CONNECTED=2, STATE_DISCONNECTING=3");
                int connectPreviousState = intent.getIntExtra(BluetoothAdapter.EXTRA_PREVIOUS_CONNECTION_STATE, -1);
                MMLog.d(TAG, "Bluetooth connectPreviousState = " + connectPreviousState);
                int connectState = intent.getIntExtra(BluetoothAdapter.EXTRA_CONNECTION_STATE, -1);
                MMLog.d(TAG, "Bluetooth state = " + connectState);
                break;

            //BluetoothDevice 相关广播
            case BluetoothDevice.ACTION_ACL_CONNECTED: //蓝牙连接
                //判断当前连接的蓝牙数量
                MMLog.debug(TAG, "ACTION_ACL_CONNECTED() ！");
                break;
            case BluetoothDevice.ACTION_ACL_DISCONNECTED: //蓝牙断开
                MMLog.debug(TAG, "ACTION_ACL_DISCONNECTED() ！");
                break;
            case BluetoothDevice.ACTION_BOND_STATE_CHANGED: //蓝牙绑定状态改变,绑定前后蓝牙变化广播
                //("Bluetooth bond state :BOND_NONE = 10, BOND_BONDING = 11, BOND_BONDED = 12 ");
                int bondPreviousState = intent.getIntExtra(BluetoothDevice.EXTRA_PREVIOUS_BOND_STATE, -1);
                MMLog.debug(TAG, "Bluetooth bondPreviousState = " + bondPreviousState);
                int bondState = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE, -1);
                MMLog.debug(TAG, "Bluetooth bondState = " + bondState);
                break;
            case BluetoothDevice.ACTION_PAIRING_REQUEST: //蓝牙配对
                MMLog.debug(TAG, "ACTION_PAIRING_REQUEST ！");
                int pinNumber = intent.getIntExtra("android.bluetooth.device.extra.PAIRING_KEY", 1234);
                MMLog.debug(TAG, "ACTION_PAIRING_REQUEST  pinNumber = " + pinNumber);
                break;
            case ACTION_PAIRING_CANCEL://BluetoothDevice.ACTION_PAIRING_CANCEL: //蓝牙取消配对
                MMLog.debug(TAG, "ACTION_PAIRING_CANCEL!");
                break;
        }
    }

    public static void startMediaScanning(Context context, String scanDir) {
        Intent scanIntent = new Intent(ACTION_MEDIA_SCANNER_SCAN_DIR);
        scanIntent.setData(Uri.fromFile(new File(scanDir)));
        context.sendBroadcast(scanIntent);
    }

    public static void unMute(Context context) {
        AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        int currVolume = audioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
        //if (CABINET.properties.getInt("protocol") == 1570)
        {
            int maxVolume = audioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
            MMLog.i(TAG, "GlobalBroadcastReceiver maxVolume = " + maxVolume + " currVolume = " + currVolume);
            //audioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, AudioManager.ADJUST_UNMUTE, AudioManager.FLAG_SHOW_UI); //取消系统静音
            if (currVolume < maxVolume) {
                audioManager.setStreamVolume(AudioManager.STREAM_MUSIC, maxVolume, AudioManager.FLAG_PLAY_SOUND);
                MMLog.i(TAG, "GlobalBroadcastReceiver currVolume = " + currVolume);
            }
        }
    }

}
