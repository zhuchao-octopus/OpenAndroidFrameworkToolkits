package com.zhuchao.android.player;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.net.Uri;
import android.view.SurfaceView;
import android.view.TextureView;

import com.zhuchao.android.fbase.PlayerStatusInfo;
import com.zhuchao.android.fbase.eventinterface.PlayerCallback;

import java.io.FileDescriptor;
import java.util.Map;

public abstract class PlayControl {
    //private String TAG = "PlayControl>>>";
    protected Context mContext;
    protected PlayerCallback playerEventCallBack = null;
    protected TextureView mTextureView = null;
    protected SurfaceView mSurfaceView = null;
    protected Boolean hwDecoderEnabled = true;
    protected int DefaultVolumeValue = 50;
    protected PlayerStatusInfo playerStatusInfo;

    public PlayControl(Context context, PlayerCallback callback) {
        mContext = context;
        playerEventCallBack = callback;
        playerStatusInfo = new PlayerStatusInfo();
        playerStatusInfo.setVolume(DefaultVolumeValue);
        playerStatusInfo.setSurfacePrepared(false);
        playerStatusInfo.setSourcePrepared(false);
    }

    public abstract String getTAG();

    public abstract void setSource(String filePath);

    public abstract void setSource(Uri uri);

    public abstract void setSource(AssetFileDescriptor afd);

    public abstract void setSource(FileDescriptor fd);

    public abstract void pushTo(String filePath, boolean duplicated);

    public abstract void setSurfaceView(SurfaceView surfaceView);

    public abstract void setTextureView(TextureView textureView);

    public abstract void reAttachSurfaceView(SurfaceView surfaceView);

    public abstract void reAttachTextureView(TextureView textureView);

    public TextureView getTextureView() {
        return mTextureView;
    }

    public SurfaceView getSurfaceView() {
        return mSurfaceView;
    }

    public abstract void fastForward(float x);

    public abstract void fastBack(float x);

    public abstract void fastForward(int x);

    public abstract void fastBack(int x);

    public abstract void fastForward(long x);

    public abstract void fastBack(long x);

    public abstract void play();

    public abstract void pause();

    public abstract void playPause();

    public abstract void stop();

    public abstract void stopFree();

    public abstract void resume();

    public abstract Boolean isPlaying();

    public abstract void setPlayTime(long l);

    public abstract void setVolume(int var1);

    public abstract int getVolume();

    public abstract Long getTime();

    public abstract float getPosition();

    public abstract void setPosition(float f);

    public abstract long getLength();

    public abstract int getPlayerStatus();

    public abstract void setRate(float v);

    public abstract void setWindowSize(int width, int height);

    public abstract void setAspectRatio(String aspect);

    public abstract void setScale(float scale);

    public abstract void setHWDecoderEnabled(Boolean HWDecoderEnabled);

    public abstract void setCallback(PlayerCallback callback);

    public abstract void setNoAudio();

    public abstract void setOption(String option);

    public abstract int getAudioTracksCount();

    public abstract Map<Integer, String> getAudioTracks();

    public abstract int getAudioTrack();

    public abstract void setAudioTrack(int index);

    public abstract void deselectTrack(int index);

    public abstract int getVideoWidth();

    public abstract int getVideoHeight();

    public abstract void startRecording(String filePath);

    public abstract void stopRecording(String filePath);

    public PlayerStatusInfo getPlayerStatusInfo() {
        return playerStatusInfo;
    }

    public abstract void free();

}
