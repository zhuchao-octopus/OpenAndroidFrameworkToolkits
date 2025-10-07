package com.zhuchao.android.player;

import static android.media.MediaPlayer.SEEK_CLOSEST;
import static android.media.MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_AUDIO;

import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.AssetFileDescriptor;
import android.media.AudioAttributes;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Build;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

import androidx.annotation.NonNull;
import androidx.constraintlayout.widget.ConstraintLayout;

import com.zhuchao.android.fbase.MMLog;
import com.zhuchao.android.fbase.PlaybackEvent;
import com.zhuchao.android.fbase.ThreadUtils;
import com.zhuchao.android.fbase.eventinterface.PlayerCallback;

import org.jetbrains.annotations.NotNull;

import java.io.FileDescriptor;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class MPlayer extends PlayControl implements MediaPlayer.OnCompletionListener, MediaPlayer.OnSeekCompleteListener, MediaPlayer.OnErrorListener, MediaPlayer.OnBufferingUpdateListener, MediaPlayer.OnVideoSizeChangedListener, SurfaceHolder.Callback {

    private final String TAG = "MPlayer";
    private MediaPlayer mediaPlayer = null;
    private ProgressThread progressThread = null;

    public MPlayer(Context context, PlayerCallback callback) {
        super(context, callback);
        freeSurfaceView();
        mediaPlayer = new MediaPlayer();
    }

    @Override
    public String getTAG() {
        return TAG;
    }

    @Override
    public void setSource(String filePath) {
        if (mediaPlayer == null) {
            MMLog.e(TAG, "Wrong!!!! player is missed!!!!!!!!!!");
            return;
        }
        try {
            PreparePlayerComponent();
            playerStatusInfo.setEventType(PlaybackEvent.Status_Opening);
            //MMLog.i(TAG, "CallbackProgress()");
            CallbackProgress();
            MMLog.d(TAG, "SetSource to " + filePath);
            mediaPlayer.setDataSource(filePath);
            ///MMLog.i(TAG, "setSource end");
        } catch (IllegalArgumentException e) {
            MMLog.e(TAG, "setSource() " + e.toString() + " filePath = " + filePath);
            playerStatusInfo.setEventType(PlaybackEvent.Status_Error);
        } catch (IOException | IllegalStateException e) {
            MMLog.e(TAG, "setSource() " + e.toString());
            playerStatusInfo.setEventType(PlaybackEvent.Status_Error);
        }

    }

    @Override
    public void setSource(Uri uri) {
        try {
            PreparePlayerComponent();
            playerStatusInfo.setEventType(PlaybackEvent.Status_Opening);
            CallbackProgress();
            mediaPlayer.setDataSource(mContext, uri);
        } catch (IllegalArgumentException | IOException | IllegalStateException e) {
            MMLog.e(TAG, "setSource() " + e.toString());
            playerStatusInfo.setEventType(PlaybackEvent.Status_Error);
        }
    }

    @Override
    public void setSource(AssetFileDescriptor afd) {
        try {
            PreparePlayerComponent();
            playerStatusInfo.setEventType(PlaybackEvent.Status_Opening);
            CallbackProgress();
            mediaPlayer.setDataSource(afd);
        } catch (IllegalArgumentException | IOException | IllegalStateException e) {
            MMLog.e(TAG, "setSource() " + e.toString());
            playerStatusInfo.setEventType(PlaybackEvent.Status_Error);
        }
    }

    @Override
    public void setSource(FileDescriptor fd) {
        try {
            PreparePlayerComponent();
            playerStatusInfo.setEventType(PlaybackEvent.Status_Opening);
            CallbackProgress();
            mediaPlayer.setDataSource(fd);
        } catch (IllegalArgumentException | IllegalStateException | IOException e) {
            MMLog.e(TAG, "setSource() " + e.toString());
            playerStatusInfo.setEventType(PlaybackEvent.Status_Error);
        } //catch (Exception e) {
        //    MMLog.e(TAG, "setSource() " + e.toString());
        //    playerStatusInfo.setEventType(PlaybackEvent.Status_Error);
        //}

    }

    @Override
    public void setSurfaceView(@NotNull SurfaceView surfaceView) {
        mSurfaceView = surfaceView;
        mTextureView = null;
        if (mediaPlayer == null) mediaPlayer = new MediaPlayer();
        if (mSurfaceView == null || mSurfaceView.getHolder() == null) {
            MMLog.log(TAG, "setSurfaceView() mSurfaceView / Holder = null");
            return;
        }
        try {
            mSurfaceView.getHolder().addCallback(this);
            MMLog.d(TAG, "setSurfaceView().addCallback to " + surfaceView.toString());
        } catch (Exception e) {
            //e.printStackTrace();
            MMLog.e(TAG, "setSurfaceView().addCallback " + e.toString());
        }

        if (mediaPlayer != null) {
            try {
                if (!mSurfaceView.getHolder().isCreating()) {
                    if (mSurfaceView.getHolder().getSurface().isValid()) {
                        mediaPlayer.setDisplay(mSurfaceView.getHolder());
                        playerStatusInfo.setSurfacePrepared(true);
                        MMLog.d(TAG, "setSurfaceView().setDisplay  to " + surfaceView.toString());
                    } else {
                        MMLog.d(TAG, "setSurfaceView().setDisplay failed,is invalid");
                    }
                } else MMLog.d(TAG, "setSurfaceView().getHolder is not created ,is invalid");
            } catch (Exception e) {
                //e.printStackTrace();
                MMLog.d(TAG, "setSurfaceView().setDisplay " + e.toString());
            }
        } else MMLog.d(TAG, "setSurfaceView().mediaPlayer is null");
    }

    @Override
    public void setTextureView(TextureView textureView) {
        mSurfaceView = null;
        mTextureView = textureView;
    }

    @Override//播放中变换
    public void reAttachSurfaceView(@NonNull SurfaceView surfaceView) {
        if (mediaPlayer == null) mediaPlayer = new MediaPlayer();
        try {
            surfaceView.getHolder().addCallback(this);
        } catch (Exception e) {
            MMLog.e(TAG, "reAttachSurfaceView().addCallback " + e.toString());
        }

        try {
            mediaPlayer.setDisplay(surfaceView.getHolder());
            MMLog.d(TAG, "reAttachSurfaceView() to  " + surfaceView.toString());
            mSurfaceView = surfaceView;
            mTextureView = null;
            playerStatusInfo.setSurfacePrepared(true);
        } catch (Exception e) {
            MMLog.e(TAG, "reAttachSurfaceView() setDisplay " + e.toString());
        }
    }

    @Override
    public void reAttachTextureView(TextureView textureView) {

    }

    @Override
    public void fastForward(float x) {

    }

    @Override
    public void fastBack(float x) {

    }

    @Override
    public void fastForward(int x) {

    }

    @Override
    public void fastBack(int x) {

    }

    @Override
    public void fastForward(long x) {

    }

    @Override
    public void fastBack(long x) {

    }

    //播放循序
    //setSource(url);
    //.play();
    @Override
    public void play() {
        if (mediaPlayer == null) {
            MMLog.log(TAG, "mediaPlayer == null");
            return;
        }
        ///if (mSurfaceView == null) {
        ///    MMLog.log(TAG, "mSurfaceView == null");
        ///    return;
        ///}
        try {
            AsyncPlayProcess(playerStatusInfo.getEventType());
        } catch (Exception e) {
            MMLog.e(TAG, "play() " + "playStatus = " + playerStatusInfo.getEventType() + " " + e.getMessage());
        }
    }

    @Override
    public void pause() {
        if (mediaPlayer == null) return;
        mediaPlayer.pause();
        playerStatusInfo.setEventType(PlaybackEvent.Status_Paused);
        //playStatus = PlaybackEvent.Status_Paused;
    }

    @Override
    public void playPause() {
        if (mediaPlayer == null) return;
        if (playerStatusInfo.getEventType() == PlaybackEvent.Status_Playing) {
            pause();
            //MMLog.log(TAG, "playPause.pause playStatus = " + playStatus);
        } else {
            AsyncPlayProcess(playerStatusInfo.getEventType());
        }
    }

    @Override
    public void stop() {
        if (mediaPlayer == null) {
            playerStatusInfo.setEventType(PlaybackEvent.Status_NothingIdle);
            return;
        }
        if (!mediaPlayer.isPlaying()) {
            playerStatusInfo.setEventType(PlaybackEvent.Status_Stopped);
            return;
        }

        if (playerStatusInfo.getEventType() == PlaybackEvent.Status_Stopped) return;

        try {
            mediaPlayer.stop();
        } catch (IllegalStateException e) {
            MMLog.e(TAG, "stop() " + e.toString());
        }
        playerStatusInfo.setEventType(PlaybackEvent.Status_Stopped);
    }

    @Override
    public void stopFree() {
        stop();
    }

    @Override
    public void resume() {

    }

    @Override
    public Boolean isPlaying() {
        if (mediaPlayer != null) {
            if (playerStatusInfo.getEventType() == PlaybackEvent.Status_Playing) return true;
            else return mediaPlayer.isPlaying();
        } else {
            return false;
        }
    }

    @Override
    public void setPlayTime(long l) {
        if (mediaPlayer == null) return;
        try {
            MMLog.log(TAG, "seek position to = " + l);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                mediaPlayer.seekTo((int) l, SEEK_CLOSEST);
            } else {
                mediaPlayer.seekTo((int) l);
            }
        } catch (IllegalStateException e) {
            MMLog.e(TAG, "seek to " + e.toString());
        }
    }

    @Override
    public void setVolume(int var1) {
        if (var1 < 0) return;
        DefaultVolumeValue = var1;
        try {
            if ((mediaPlayer != null) && (mediaPlayer.isPlaying()))
                mediaPlayer.setVolume(playerStatusInfo.getVolume() / 100, playerStatusInfo.getVolume() / 100);
        } catch (Exception e) {
            MMLog.e(TAG, "setVolume() " + e.toString());
        }
    }

    @Override
    public int getVolume() {
        return playerStatusInfo.getVolume();
    }

    @Override
    public Long getTime() {
        try {
            if ((mediaPlayer != null) && mediaPlayer.isPlaying()) {
                playerStatusInfo.setTimeChanged((long) mediaPlayer.getCurrentPosition());
                playerStatusInfo.setPlayRate(mediaPlayer.getPlaybackParams().getSpeed());
            }
        } catch (Exception e) {
            MMLog.e(TAG, "getTime() " + e.toString());
        }
        return playerStatusInfo.getTimeChanged();
    }

    @Override
    public float getPosition() {
        try {
            if ((mediaPlayer != null) && (mediaPlayer.isPlaying())) playerStatusInfo.setPositionChanged(mediaPlayer.getCurrentPosition());
        } catch (Exception e) {
            MMLog.e(TAG, "getPosition() " + e.toString());
        }
        return playerStatusInfo.getPositionChanged();
    }

    @Override
    public void setPosition(float f) {

    }

    @Override
    public long getLength() {
        return playerStatusInfo.getLengthChanged();
    }

    @Override
    public int getPlayerStatus() {
        ///if ((mediaPlayer != null) && (mediaPlayer.isPlaying()))
        ///    playStatus = PlaybackEvent.Status_Playing;
        ///else if (mediaPlayer == null)
        ///    playStatus = PlaybackEvent.Status_NothingIdle;
        return playerStatusInfo.getEventType();
    }

    @Override
    public void setRate(float v) {
        try {
            if ((mediaPlayer != null))//&& (mediaPlayer.isPlaying())
                mediaPlayer.setPlaybackParams(mediaPlayer.getPlaybackParams().setSpeed(v));
        } catch (Exception e) {
            MMLog.e(TAG, "setRate() " + e.toString() + " v = " + v);
        }
    }

    @Override
    public void setWindowSize(int width, int height) {
        if (mSurfaceView != null && playerStatusInfo.isSurfacePrepared()) {
            mSurfaceView.getHolder().setFixedSize(width, height);
            MMLog.d(TAG, "setWindowSize to " + width + " " + height);
        } else {
            MMLog.d(TAG, "setWindowSize to " + width + " " + height + " failed " + playerStatusInfo.isSurfacePrepared());
        }
    }

    @Override
    public void setAspectRatio(String aspect) {

    }

    @Override
    public void setScale(float scale) {
        try {
            if (mediaPlayer != null && mediaPlayer.isPlaying()) {
                mediaPlayer.setVideoScalingMode((int) scale);
                MMLog.d(TAG, "setScale() " + (int) scale);
            }
        } catch (Exception e) {
            MMLog.e(TAG, "setScale() " + (int) scale + "," + e.toString());
        }
    }

    @Override
    public void setHWDecoderEnabled(Boolean HWDecoderEnabled) {

    }

    @Override
    public void setNoAudio() {
        mediaPlayer.setVolume(0, 0);
    }

    @Override
    public void setOption(String option) {

    }

    @Override
    public int getAudioTracksCount() {
        MediaPlayer.TrackInfo[] trackInfo = null;
        if (mediaPlayer != null) {
            try {
                trackInfo = mediaPlayer.getTrackInfo();
            } catch (IllegalStateException e) {
                MMLog.e(TAG, "getAudioTracksCount() " + e.toString());
            }
            assert trackInfo != null;
            return trackInfo.length;
        } else return 0;
    }

    @Override
    public Map<Integer, String> getAudioTracks() {
        MediaPlayer.TrackInfo[] trackInfo = null;
        Map<Integer, String> mtd = new HashMap<Integer, String>();
        if (mediaPlayer == null) return mtd;

        try {
            trackInfo = mediaPlayer.getTrackInfo();
        } catch (IllegalStateException e) {
            MMLog.e(TAG, "getAudioTracks() " + e.toString());
        }

        for (int i = 0; i < Objects.requireNonNull(trackInfo).length; i++) {
            //if (trackInfo[i].getTrackType() == MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_AUDIO)
            mtd.put(i, trackInfo[i].getLanguage());
            MMLog.d(TAG, "getAudioTracks() " + trackInfo[i].toString());
        }
        return mtd;
    }

    @Override
    public int getAudioTrack() {
        try {
            if (mediaPlayer != null) return mediaPlayer.getSelectedTrack(MEDIA_TRACK_TYPE_AUDIO);
        } catch (IllegalStateException e) {
            MMLog.e(TAG, "getAudioTrack() " + e.toString());
        }
        return -1;
    }

    @Override
    public void setAudioTrack(int index) {
        if (mediaPlayer == null) return;
        try {
            MediaPlayer.TrackInfo[] trackInfo = mediaPlayer.getTrackInfo();
            if (index <= 0) {
                mediaPlayer.deselectTrack(getAudioTrack());
                return;
            }
            for (int i = 0; i < trackInfo.length; i++) {
                MediaPlayer.TrackInfo Info = trackInfo[i];
                //if (Info.getTrackType() == MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_AUDIO)
                if (index == i) {
                    mediaPlayer.selectTrack(i);
                    MMLog.d(TAG, "setAudioTrack() track info = " + Info.toString());
                    break;
                }
            }
        } catch (IllegalStateException e) {
            MMLog.e(TAG, "setAudioTrack() " + e.toString());
        }
    }

    @Override
    public void deselectTrack(int index) {
        if (mediaPlayer == null) return;
        try {
            MediaPlayer.TrackInfo[] trackInfo = mediaPlayer.getTrackInfo();
            for (int i = 0; i < trackInfo.length; i++) {
                MediaPlayer.TrackInfo Info = trackInfo[i];
                //if (Info.getTrackType() == MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_AUDIO)
                if (index == i) {
                    mediaPlayer.deselectTrack(i);
                    MMLog.d(TAG, "deselectTrack() track info = " + Info.toString());
                    break;
                }
            }
        } catch (IllegalStateException e) {
            MMLog.e(TAG, "setAudioTrack() " + e.toString());
        }
    }

    @Override
    public int getVideoWidth() {
        if (mediaPlayer == null) return 0;
        return mediaPlayer.getVideoWidth();
    }

    @Override
    public int getVideoHeight() {
        if (mediaPlayer == null) return 0;
        return mediaPlayer.getVideoHeight();
    }

    @Override
    public void startRecording(String filePath) {

    }

    @Override
    public void stopRecording(String filePath) {

    }

    @Override
    public void pushTo(String filePath, boolean duplicated) {

    }

    @Override
    public void setCallback(PlayerCallback callback) {
        playerEventCallBack = callback;
    }

    @Override
    public boolean onError(MediaPlayer mediaPlayer, int i, int i1) {
        playerStatusInfo.setEventType(PlaybackEvent.Status_Error);
        MMLog.e(TAG, "onError:" + i + "," + i1 + " playStatus = " + PlaybackEvent.Status_Error);
        playerStatusInfo.setLastError(PlaybackEvent.Status_Error);
        ///return true;//
        ///CallbackProgress(PlaybackEvent.Status_Ended);//给前端处理错误的机会
        return false;//// to call OnCompletionListener.onCompletion()方法。/自身结束错误
    }

    @Override
    public void onCompletion(MediaPlayer mediaPlayer) {
        ///playStatus = PlaybackEvent.Status_Ended;
        playerStatusInfo.setEventType(PlaybackEvent.Status_Ended);
        ///MMLog.log(TAG, "onCompletion playing ended playStatus = " + PlaybackEvent.Status_Ended);
    }

    @Override
    public void onSeekComplete(MediaPlayer mediaPlayer) {
        MMLog.log(TAG, "onSeekComplete:" + mediaPlayer.toString());
        AsyncPlayProcess(PlaybackEvent.Status_SEEKING);
    }

    @Override
    public void onBufferingUpdate(MediaPlayer mediaPlayer, int i) {
        //MLog.log(TAG, "onBufferingUpdate:" + i);
        playerStatusInfo.setBuffering(i);
        //CallbackProgress(i);
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        MMLog.log(TAG, "surfaceCreated," + surfaceHolder.toString());
        playerStatusInfo.setSurfacePrepared(true);
        if (mediaPlayer == null) mediaPlayer = new MediaPlayer();
        mediaPlayer.setDisplay(surfaceHolder);
        ///surfaceHolder.setSizeFromLayout();
        ///surfaceHolder.setFixedSize(w,h);
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {
        MMLog.log(TAG, "surfaceChanged,format=" + i + " width=" + i1 + " height=" + i2);
        playerStatusInfo.setSurfaceW(i1);
        playerStatusInfo.setSurfaceH(i2);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        MMLog.log(TAG, "surfaceDestroyed," + surfaceHolder.toString());
        playerStatusInfo.setSurfacePrepared(false);
    }

    @Override
    public void onVideoSizeChanged(MediaPlayer mediaPlayer, int i, int i1) {
        playerStatusInfo.setVideoW(i);
        playerStatusInfo.setVideoH(i1);
        MMLog.log(TAG, "onVideoSizeChanged width=" + i + " height=" + i1 + ", SurfaceView.getWidth()=" + mSurfaceView.getWidth() + " SurfaceView.getHeight()=" + mSurfaceView.getHeight());
        ///autoFitVideoSize(mSurfaceView, i, i1);
        ///MMLog.log(TAG, "onVideoSizeChanged width=" + i + " height=" + i1 + ", SurfaceView.getWidth()=" + mSurfaceView.getWidth() + " SurfaceView.getHeight()=" + mSurfaceView.getHeight());
        if (mSurfaceView != null) {
            ThreadUtils.runOnMainUiThread(new Runnable() {
                @Override
                public void run() {
                    autoFitVideoSize(mSurfaceView, i, i1);
                    ///playerStatusInfo.setSurfaceW( mSurfaceView.getWidth());
                    ///playerStatusInfo.setSurfaceH( mSurfaceView.getHeight());
                    MMLog.log(TAG, "onVideoSizeChanged width=" + i + " height=" + i1 + ", SurfaceView.getWidth()=" + mSurfaceView.getWidth() + " SurfaceView.getHeight()=" + mSurfaceView.getHeight());
                }
            });
        }
    }

    @Override
    public void free() {
        if (mediaPlayer == null) return;

        try {
            if (playerStatusInfo != null) {
                playerStatusInfo.setEventType(PlaybackEvent.Status_FreeDoNothing);
                playerStatusInfo.setLastError(0);
            }

            CallbackProgress();//通知doNothing

            if (progressThread != null) progressThread.finish();
            progressThread = null;
            if (mediaPlayer.isPlaying()) mediaPlayer.stop();
            mediaPlayer.reset();
            mediaPlayer.setOnCompletionListener(null);
            mediaPlayer.setOnSeekCompleteListener(null);
            mediaPlayer.setOnErrorListener(null);
            mediaPlayer.setOnBufferingUpdateListener(null);
            mediaPlayer.release();
            mediaPlayer = null;

            if (playerStatusInfo != null) playerStatusInfo.setEventType(PlaybackEvent.Status_NothingIdle);

            freeSurfaceView();
            MMLog.d(TAG, "call free() to release all player context");
        } catch (Exception e) {
            MMLog.e(TAG, "freed() " + e.toString());
        }
    }

    private void freeSurfaceView() {
        playerStatusInfo.setSurfacePrepared(false);
        playerStatusInfo.setSurfaceW(0);
        playerStatusInfo.setSurfaceW(0);
        if (mSurfaceView != null) {
            mSurfaceView.getHolder().removeCallback(this);
            mSurfaceView = null;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    private void PreparePlayerComponent() {
        try {
            if (mediaPlayer == null) mediaPlayer = new MediaPlayer();
            ///MMLog.d(TAG, "prepare player component,playStatus = " + playerStatusInfo.getEventType());
            mediaPlayer.reset();//必须调用mediaPlayer.prepareAsync();
        } catch (Exception e) {
            MMLog.e(TAG, "prepare player failed " + e.toString());
            mediaPlayer = null;
            playerStatusInfo.setEventType(PlaybackEvent.Status_Error);
            return;
        }
        playerStatusInfo.setSourcePrepared(false);
        //mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
        mediaPlayer.setAudioAttributes(new AudioAttributes.Builder().setContentType(AudioAttributes.CONTENT_TYPE_MOVIE).build());
        mediaPlayer.setOnCompletionListener(this);
        mediaPlayer.setOnSeekCompleteListener(this);
        mediaPlayer.setOnErrorListener(this);
        mediaPlayer.setOnBufferingUpdateListener(this);
        mediaPlayer.setOnVideoSizeChangedListener(this);
        playerStatusInfo.setEventType(PlaybackEvent.Status_NothingIdle);
        playerStatusInfo.setLastError(PlaybackEvent.Status_NothingIdle);
        StartPlayProgressThread(); //开启进度 pooling 线程
    }

    //错误处理，播放状态异常内部自动复位
    private synchronized void AsyncPlayProcess(int Status) {
        ///MMLog.log(TAG, "Start asyncPlayProcess,status = " + Status);
        try {
            switch (Status) {
                case PlaybackEvent.Status_NothingIdle:
                    MMLog.log(TAG, "Sync processor player is idle, nothing to do status = " + Status);
                    break;
                case PlaybackEvent.Status_Stopped:
                case PlaybackEvent.Status_Opening:
                    playerStatusInfo.setEventType(PlaybackEvent.Status_Buffering);//开始准备
                    mediaPlayer.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
                        @Override
                        public void onPrepared(MediaPlayer mp) {
                            try {
                                playerStatusInfo.setLengthChanged(mp.getDuration());
                                playerStatusInfo.setLength(mp.getDuration());
                                playerStatusInfo.setEventType(PlaybackEvent.Status_HasPrepared);
                                playerStatusInfo.setSourcePrepared(true);
                                CallbackProgress();//给前端seek to 的机会

                                if (!mp.isPlaying()) {
                                    mp.start();
                                    mp.setVideoScalingMode(MediaPlayer.VIDEO_SCALING_MODE_SCALE_TO_FIT);
                                    playerStatusInfo.setEventType(PlaybackEvent.Status_Playing);
                                    MMLog.d(TAG, "Start playing... status = " + playerStatusInfo.getEventType());
                                }
                            } catch (IllegalStateException e) {
                                MMLog.e(TAG, "AsyncPlayProcess().getDuration() " + "status = " + playerStatusInfo.getEventType() + " " + e.toString());
                            }
                        }
                    });//mediaPlayer.setOnPreparedListener...
                    ///////////////////////////////////////////////////////////////////////////
                    try {
                        mediaPlayer.prepareAsync();
                    } catch (IllegalStateException e) {//不做任何处理交给onError()处理
                        MMLog.e(TAG, "AsyncPlayProcess().prepareAsync() " + "status = " + playerStatusInfo.getEventType() + " " + e.toString());
                        if (playerStatusInfo.getEventType() == PlaybackEvent.Status_Buffering)//没有激活onError()
                            playerStatusInfo.setEventType(PlaybackEvent.Status_Error);//激活错误处理onError() 复位后进入end模式跳到下一曲
                    }
                    break;
                case PlaybackEvent.Status_Buffering:
                case PlaybackEvent.Status_Playing:
                    MMLog.log(TAG, "AsyncPlayProcess is playing status = " + Status);
                    break;
                case PlaybackEvent.Status_Paused:
                case PlaybackEvent.Status_SEEKING:
                    MMLog.log(TAG, "AsyncPlayProcess start to play directly status = " + Status);
                    if (!playerStatusInfo.isSourcePrepared()) {
                        MMLog.log(TAG, "source is not prepared can not start to play status = " + Status);
                        break;
                    }
                    try {
                        mediaPlayer.start();
                    } catch (IllegalStateException e) {
                        e.printStackTrace();
                    }
                    playerStatusInfo.setEventType(PlaybackEvent.Status_Playing);
                    break;
                case PlaybackEvent.Status_Ended:
                case PlaybackEvent.Status_Error:
                default:
                    MMLog.log(TAG, "AsyncPlayProcess do nothing end/error,status = " + Status);
                    break;
            }
        } catch (Exception e) {
            MMLog.e(TAG, "AsyncPlayProcess() " + "status = " + playerStatusInfo.getEventType() + " " + e.toString());
        }
    }

    private void CallbackProgress() {
        switch (playerStatusInfo.getEventType()) {
            case PlaybackEvent.Status_Opening:
                playerStatusInfo.setLengthChanged(0);
                playerStatusInfo.setTimeChanged(0);
                playerStatusInfo.setPositionChanged(0);
                break;
            case PlaybackEvent.Status_Buffering:
            case PlaybackEvent.Status_HasPrepared:
            case PlaybackEvent.Status_Playing:
            case PlaybackEvent.Status_Paused:
                getTime();
                break;
            case PlaybackEvent.Status_Stopped:
            case PlaybackEvent.Status_Ended:
            case PlaybackEvent.Status_Error:
            default:
                break;
        }

        if (playerEventCallBack != null && playerStatusInfo != null) playerEventCallBack.onEventPlayerStatus(playerStatusInfo);
    }

    private void StartPlayProgressThread() {   //创建进度状态线程
        if (progressThread == null) {
            MMLog.log(TAG, "Create and start progress task");
            progressThread = new ProgressThread();
            progressThread.start();
        } else {
            if (!progressThread.isActive()) {
                MMLog.log(TAG, "Recreate and start progress task");
                progressThread = new ProgressThread();
                progressThread.start();
            }
        }
    }

    private class ProgressThread extends Thread {
        private boolean isActive = true;

        @Override
        public void run() {
            super.run();
            while (isActive) {
                //内部状态不向外传递
                if (playerStatusInfo.getEventType() >= PlaybackEvent.Status_INTERNAL) continue;
                //缓冲状态已经在上面调用过了，不再常规调用
                //if (playerStatusInfo.getEventType() == PlaybackEvent.Status_Buffering) continue;

                try {
                    CallbackProgress();
                } catch (Exception e) {
                    MMLog.e(TAG, "Progress polling " + e.toString() + " playStatus = " + playerStatusInfo.getEventType());
                }
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    MMLog.e(TAG, "Progress polling " + e.toString() + " playStatus = " + playerStatusInfo.getEventType());
                }
            }
        }

        void finish() {
            isActive = false;
        }

        boolean isActive() {
            return isActive;
        }
    }

    public void autoFitVideoSize(SurfaceView surfaceView, int videoWidth, int videoHeight) {
        View view = (View) surfaceView.getParent();
        int deviceWidth = view.getWidth();//mContext.getResources().getDisplayMetrics().widthPixels;
        int deviceHeight = view.getHeight();//surfaceView.getHeight();// wgetResources().getDisplayMetrics().heightPixels;
        float devicePercent = 0;
        ///MMLog.d(TAG, "autoFitVideoSize: deviceViewWidth=" + deviceWidth + " deviceViewHeight=" + deviceHeight);

        //下面进行求屏幕比例,因为横竖屏会改变屏幕宽度值,所以为了保持更小的值除更大的值.
        if (mContext.getResources().getConfiguration().orientation == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT) { //竖屏
            devicePercent = (float) deviceWidth / (float) deviceHeight; //竖屏状态下宽度小与高度,求比
        } else { //横屏
            devicePercent = (float) deviceHeight / (float) deviceWidth; //横屏状态下高度小与宽度,求比
        }

        if (videoWidth > videoHeight) { //判断视频的宽大于高,那么我们就优先满足视频的宽度铺满屏幕的宽度,然后在按比例求出合适比例的高度
            videoWidth = deviceWidth;//将视频宽度等于设备宽度,让视频的宽铺满屏幕
            videoHeight = (int) (deviceWidth * devicePercent);//设置了视频宽度后,在按比例算出视频高度

        } else {  //判断视频的高大于宽,那么我们就优先满足视频的高度铺满屏幕的高度,然后在按比例求出合适比例的宽度
            if (mContext.getResources().getConfiguration().orientation == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT) {//竖屏
                videoHeight = deviceHeight;
                //接受在宽度的轻微拉伸来满足视频铺满屏幕的优化
                float videoPercent = (float) videoWidth / (float) videoHeight;//求视频比例 注意是宽除高 与 上面的devicePercent 保持一致
                float differenceValue = Math.abs(videoPercent - devicePercent);//相减求绝对值
                ///MMLog.d(TAG, "devicePercent=" + devicePercent);
                ///MMLog.d(TAG, "videoPercent=" + videoPercent);
                ///MMLog.d(TAG, "differenceValue=" + differenceValue);
                if (differenceValue < 0.3) { //如果小于0.3比例,那么就放弃按比例计算宽度直接使用屏幕宽度
                    videoWidth = deviceWidth;
                } else {
                    videoWidth = (int) (videoWidth / devicePercent);//注意这里是用视频宽度来除
                }
            } else { //横屏
                videoHeight = deviceHeight;
                videoWidth = (int) (deviceHeight * devicePercent);
            }
        }

        ViewGroup.LayoutParams layoutParams = surfaceView.getLayoutParams();
        if (layoutParams instanceof ConstraintLayout.LayoutParams) {
            ConstraintLayout.LayoutParams constLayoutParams = (ConstraintLayout.LayoutParams) surfaceView.getLayoutParams();
            constLayoutParams.width = videoWidth;
            constLayoutParams.height = videoHeight;
            constLayoutParams.verticalBias = 0.5f;
            constLayoutParams.horizontalBias = 0.5f;
            surfaceView.setLayoutParams(constLayoutParams);
        } else if (layoutParams instanceof FrameLayout.LayoutParams) {
            FrameLayout.LayoutParams frameLayoutParams = (FrameLayout.LayoutParams) surfaceView.getLayoutParams();
            frameLayoutParams.width = videoWidth;
            frameLayoutParams.height = videoHeight;
            surfaceView.setLayoutParams(layoutParams);
        }
        else if (layoutParams instanceof LinearLayout.LayoutParams) {
            LinearLayout.LayoutParams linearLayoutParams = (LinearLayout.LayoutParams) surfaceView.getLayoutParams();
            linearLayoutParams.width = videoWidth;
            linearLayoutParams.height = videoHeight;
            surfaceView.setLayoutParams(layoutParams);
        }
    }
}
