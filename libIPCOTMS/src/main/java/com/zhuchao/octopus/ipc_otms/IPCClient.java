package com.zhuchao.octopus.ipc_otms;

import android.os.Handler;
import android.os.Looper;

import androidx.annotation.NonNull;

import java.util.Arrays;

public class IPCClient {
    static {
        System.loadLibrary("OAPPC");
    }

    private native void nativeIpcInitClient();

    private native void nativeIpcRegisterCallback(IPCClient client);

    private native void nativeIpcUnregisterCallback(IPCClient client);

    private native void nativeIpcSendMessage(byte msgGroup, byte msgId, byte[] data);

    private native void nativeIpcSendMessageDelay(byte msgGroup, byte msgId, byte[] data, int delay_ms);

    public interface IPCMessageResponseCallback {
        void onResponse(IPCMessage message);
    }

    private IPCMessageResponseCallback ipcMessageResponseCallback;
    private final Handler mainHandler = new Handler(Looper.getMainLooper());

    public void setMessageCallback(IPCMessageResponseCallback cb) {
        this.ipcMessageResponseCallback = cb;
        nativeIpcRegisterCallback(this);
    }

    public void removeMessageCallback() {
        nativeIpcUnregisterCallback(this);
        this.ipcMessageResponseCallback = null;
    }

    public void initIpcClient() {
        nativeIpcInitClient();
    }

    public void sendIpcMessage(byte group, byte id, byte[] data) {
        nativeIpcSendMessage(group, id, data);
    }

    public void sendIpcMessage(byte group, byte id, byte[] data, int delay_ms) {
        nativeIpcSendMessageDelay(group, id, data, delay_ms);
    }

    private void onIpcNativeResponse(byte msgGroup, byte msgId, byte[] data) {
        final IPCMessage msg = new IPCMessage(msgGroup, msgId, data);
        if (ipcMessageResponseCallback != null) {
            mainHandler.post(() -> ipcMessageResponseCallback.onResponse(msg));
        }
    }

    public static class IPCMessage {
        public byte msgGroup;
        public byte msgId;
        public byte[] data;

        public IPCMessage(byte group, byte id, byte[] data) {
            this.msgGroup = group;
            this.msgId = id;
            this.data = data;
        }

        @NonNull
        @Override
        public String toString() {
            return "IPCMessage{" + "group=" + msgGroup + ", id=" + msgId + ", data=" + Arrays.toString(data) + '}';
        }
    }
}
