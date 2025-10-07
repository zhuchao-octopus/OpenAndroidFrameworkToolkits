package com.zhuchao.android.player;

import static com.zhuchao.android.fbase.FileUtils.EmptyString;

import static org.videolan.libvlc.interfaces.IMedia.Slave.Type.Audio;
import static org.videolan.libvlc.interfaces.IMedia.Track.Type.Text;
import static org.videolan.libvlc.interfaces.IMedia.Track.Type.Video;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.net.Uri;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;

import androidx.annotation.NonNull;

import com.zhuchao.android.fbase.MMLog;
import com.zhuchao.android.fbase.PlaybackEvent;
import com.zhuchao.android.fbase.eventinterface.PlayerCallback;

import org.videolan.libvlc.LibVLC;
import org.videolan.libvlc.Media;
import org.videolan.libvlc.MediaPlayer;
import org.videolan.libvlc.interfaces.IVLCVout;

import java.io.FileDescriptor;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/* media 类的 播放状态，与vlc 事件不同
 public static class State {
        public static final int NothingSpecial = 0;
        public static final int Opening = 1;
        public static final int Playing = 3;
        public static final int Paused = 4;
        public static final int Stopped = 5;
        public static final int Ended = 6;
        public static final int Error = 7;
        public static final int MAX = 8;

        public State() {
        }
    }
 */

public class OPlayer extends PlayControl implements View.OnLayoutChangeListener {///implements VLCPlayer.VLCPlayerCallback
    private final String TAG = "OPlayer";
    private MediaPlayer mMediaPlayer = null;
    private Media media = null;
    private LibVLC FLibVLC = null;
    private IVLCVout vlcVout;
    private IVLCVoutCallBack mIVLCVoutCallBack;

    //private long progressTick = 0;
    //private boolean firstPlaying = false;
    @Override
    public String getTAG() {
        return TAG;
    }

    private final MediaPlayer.EventListener mEventListener = new MediaPlayer.EventListener() {
        @Override
        public void onEvent(MediaPlayer.Event event) {
            ///if ((System.currentTimeMillis() - progressTick < 1000) && (event.type == MediaPlayer.Event.Playing)) {
            ///    if(progressTick > 0)
            ///       return;//一秒回调一次
            ///}
            ///if ((System.currentTimeMillis() - progressTick < 1000) && (status == PlaybackEvent.Status_Playing)) {
            ///    if (progressTick > 0) {
            ///        MMLog.e(TAG,"DAFDFDFDF TEST ");
            ///        return;//一秒回调一次
            ///    }
            ///}
            if (mMediaPlayer == null) return;
            playerStatusInfo.setEventCode(event.type);
            int status = mMediaPlayer.getPlayerState();

            if (status == Media.State.Ended) playerStatusInfo.setEventType(PlaybackEvent.Status_Ended);
            else if (status == Media.State.Error) playerStatusInfo.setEventType(PlaybackEvent.Status_Error);
            else playerStatusInfo.setEventType(status);

            playerStatusInfo.setPosition(mMediaPlayer.getPosition());
            playerStatusInfo.setTimeChanged(event.getTimeChanged());
            playerStatusInfo.setPositionChanged(event.getPositionChanged());
            playerStatusInfo.setLengthChanged(event.getLengthChanged());//getLength()
            playerStatusInfo.setChangedType(event.getEsChangedType());

            playerStatusInfo.setOutCount(event.getVoutCount());
            playerStatusInfo.setBuffering(event.getBuffering());
            playerStatusInfo.setPlayRate(mMediaPlayer.getRate());
            playerStatusInfo.setLength(mMediaPlayer.getLength());
            ////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////
            if (playerEventCallBack != null) playerEventCallBack.onEventPlayerStatus(playerStatusInfo);

            ///if (status == PlaybackEvent.Status_Playing)
            ///    progressTick = System.currentTimeMillis();

            switch (event.type) {
                case MediaPlayer.Event.Playing:
                    ///progressTick = System.currentTimeMillis();
                    break;
                case MediaPlayer.Event.EndReached:
                case MediaPlayer.Event.Stopped:
                case MediaPlayer.Event.MediaChanged:
                    ///progressTick = 0;
                    break;
            }
        }
    };

    public OPlayer(Context context, PlayerCallback callback) {
        super(context, callback);
        ///ArrayList<String> Options = new ArrayList<String>();
        ///Options.add("--cr-average=10000");
        mIVLCVoutCallBack = new IVLCVoutCallBack();
        try {
            FLibVLC = new LibVLC(mContext);
            mMediaPlayer = new MediaPlayer(FLibVLC);
            mMediaPlayer.setScale(0);
            mMediaPlayer.setEventListener(mEventListener);
            vlcVout = mMediaPlayer.getVLCVout();
            vlcVout.addCallback(mIVLCVoutCallBack);
            mMediaPlayer.setVolume(DefaultVolumeValue);
            mMediaPlayer.setScale(0);
            mMediaPlayer.setAspectRatio(null);
            ///MLog.log(TAG,"OPlayer=========>");
        } catch (Exception e) {
            ///e.printStackTrace();
            MMLog.e(TAG, e.toString());
        }
    }

    public OPlayer(Context context, ArrayList<String> options, PlayerCallback callback) {
        super(context, callback);
        try {
            mIVLCVoutCallBack = new IVLCVoutCallBack();
            ///options.add("--cr-average=10000");
            ///Options.add("--file-caching=300");//文件缓存
            ///Options.add("--network-caching=10000");//网络缓存
            ///Options.add("--live-caching=10000");//直播缓存
            ///Options.add("--sout-mux-caching=1500");//输出缓存

            FLibVLC = new LibVLC(mContext, options);
            mMediaPlayer = new MediaPlayer(FLibVLC);
            mMediaPlayer.setEventListener(mEventListener);
            vlcVout = mMediaPlayer.getVLCVout();
            vlcVout.addCallback(mIVLCVoutCallBack);
            mMediaPlayer.setVolume(DefaultVolumeValue);
            mMediaPlayer.setScale(0);
            mMediaPlayer.setAspectRatio(null);
            ///MLog.log(TAG, "OPlayer=========>");
        } catch (IllegalStateException e) {
            MMLog.e(TAG, e.toString());
        }
    }

    private void freeMedia() {
        if (media != null && !media.isReleased()) {
            media.release();
        }
        media = null;
    }

    private void prepareMediaPlayer() {
        if (mMediaPlayer == null) {
            mMediaPlayer = new MediaPlayer(FLibVLC);
            mMediaPlayer.setEventListener(mEventListener);
            vlcVout = mMediaPlayer.getVLCVout();
            vlcVout.addCallback(mIVLCVoutCallBack);
            //MMLog.log(TAG, "prepare MediaPlayer successfully");
        }
    }

    public void setSource(@NonNull String filePath) {
        try {
            MMLog.log(TAG, "setSource source = " + filePath);

            if (EmptyString(filePath)) return;
            if (filePath.startsWith("http") || filePath.startsWith("rtsp") || filePath.startsWith("ftp") || filePath.startsWith("file")) {
                Uri uri = Uri.parse(filePath);
                MMLog.log(TAG, "setSource protocol source = " + filePath);
                setSource(uri);
                return;
            }
            freeMedia();
            if (FLibVLC == null) {
                MMLog.log(TAG, "FLibVLC = null ");
                return;
            }
            prepareMediaPlayer();
            media = new Media(FLibVLC, filePath);
            ///media.addOption(":no-audio");
            media.addOption(":fullscreen");
            ///media.addOption(":no-autoscale");
            ///media.addOption(":file-caching=10000");//文件缓存
            ///media.addOption(":network-caching=10000");//网络缓存
            ///media.addOption(":live-caching=10000");//直播缓存
            ///media.addOption(":sout-mux-caching=10000");//输出缓存
            setHWDecoderEnabled(true);
            mMediaPlayer.setMedia(media);
            playerStatusInfo.setSourcePrepared(true);
            ///mMediaPlayer.setAspectRatio(null);
            ///mMediaPlayer.setScale(0);
            mMediaPlayer.setVolume(DefaultVolumeValue);
            MMLog.log(TAG, "setSource successfully ");
        } catch (Exception e) {
            //e.printStackTrace();
            MMLog.e(TAG, e.toString());
        }
    }

    public void setSource(@NonNull Uri uri) {
        if (FLibVLC == null) {
            MMLog.log(TAG, "FLibVLC = null ");
            return;
        }
        freeMedia();
        prepareMediaPlayer();
        media = new Media(FLibVLC, uri);
        ///media.addOption(":no-audio");
        media.addOption(":fullscreen");
        ///media.addOption(":no-autoscale");
        ///media.addOption(":file-caching=10000");//文件缓存
        ///media.addOption(":network-caching=10000");//网络缓存
        ///media.addOption(":live-caching=10000");//直播缓存
        ///media.addOption(":sout-mux-caching=10000");//输出缓存
        setHWDecoderEnabled(true);
        mMediaPlayer.setMedia(media);
        playerStatusInfo.setSourcePrepared(true);
        ///mMediaPlayer.setAspectRatio(null);
        ///mMediaPlayer.setScale(0);
        mMediaPlayer.setVolume(DefaultVolumeValue);
    }

    public void setSource(@NonNull AssetFileDescriptor afd) {
        if (FLibVLC == null) {
            MMLog.log(TAG, "FLibVLC = null ");
            return;
        }
        freeMedia();
        prepareMediaPlayer();
        media = new Media(FLibVLC, afd);
        ///media.addOption(":no-audio");
        media.addOption(":fullscreen");
        ///media.addOption(":no-autoscale");
        ///media.addOption(":file-caching=10000");//文件缓存
        ///media.addOption(":network-caching=10000");//网络缓存
        ///media.addOption(":live-caching=10000");//直播缓存
        ///media.addOption(":sout-mux-caching=10000");//输出缓存
        setHWDecoderEnabled(true);
        mMediaPlayer.setMedia(media);
        playerStatusInfo.setSourcePrepared(true);
        ///mMediaPlayer.setAspectRatio(null);
        ///mMediaPlayer.setScale(0);
        mMediaPlayer.setVolume(DefaultVolumeValue);
    }

    public void setSource(@NonNull FileDescriptor fd) {
        if (FLibVLC == null) {
            MMLog.log(TAG, "FLibVLC = null ");
            return;
        }
        freeMedia();
        prepareMediaPlayer();
        media = new Media(FLibVLC, fd);
        ///media.addOption(":no-audio");
        media.addOption(":fullscreen");
        ///media.addOption(":no-autoscale");
        ///media.addOption(":file-caching=10000");//文件缓存
        ///media.addOption(":network-caching=10000");//网络缓存
        ///media.addOption(":live-caching=10000");//直播缓存
        ///media.addOption(":sout-mux-caching=10000");//输出缓存
        setHWDecoderEnabled(true);
        mMediaPlayer.setMedia(media);
        playerStatusInfo.setSourcePrepared(true);
        ///mMediaPlayer.setAspectRatio(null);
        ///mMediaPlayer.setScale(0);
        mMediaPlayer.setVolume(DefaultVolumeValue);
    }

    public void setSurfaceView(SurfaceView surfaceView) {
        if (surfaceView == null) {
            MMLog.log(TAG, "setSurfaceView surfaceView is null");
            return;
        }
        if (surfaceView == mSurfaceView) {
            MMLog.log(TAG, "setSurfaceView surfaceView is same");
            //return;
        }
        if (surfaceView.getHolder() != null) {
            surfaceView.getHolder().setKeepScreenOn(true);
        } else {
            MMLog.log(TAG, "setSurfaceView surfaceView.getHolder() is null");
        }
        prepareMediaPlayer();
        try {
            vlcVout = mMediaPlayer.getVLCVout();
            if (vlcVout.areViewsAttached()) vlcVout.detachViews();

            vlcVout.setVideoView(surfaceView);
            vlcVout.attachViews();

            mTextureView = null;
            mSurfaceView = surfaceView;
            mSurfaceView.addOnLayoutChangeListener(this);
            MMLog.log(TAG, "setSurfaceView successfully");
        } catch (Exception e) {
            //e.printStackTrace();
            MMLog.e(TAG, "setSurfaceView failed " + e.toString());
        }
    }

    public void setTextureView(@NonNull TextureView textureView) {
        if (textureView.equals(mTextureView)) return;
        if (vlcVout.areViewsAttached()) vlcVout.detachViews();

        vlcVout.setVideoView(textureView);
        vlcVout.attachViews();
        mSurfaceView = null;
        mTextureView = textureView;
    }

    public void reAttachSurfaceView(SurfaceView surfaceView) {

        if (surfaceView == null && mMediaPlayer != null) {
            vlcVout = mMediaPlayer.getVLCVout();
            if (vlcVout.areViewsAttached()) vlcVout.detachViews();
            MMLog.log(TAG, "reAttachSurfaceView surfaceView = null");
            return;
        }
        setSurfaceView(surfaceView);
        MMLog.log(TAG, "reattached surface view successfully");
    }

    public void reAttachTextureView(TextureView textureView) {
        if (textureView == null) return;
        mTextureView = textureView;
        if (vlcVout.areViewsAttached()) vlcVout.detachViews();

        vlcVout.setVideoView(mTextureView);
        vlcVout.attachViews();

        mTextureView = textureView;
        mSurfaceView = null;
    }

    @Override
    public void pushTo(String filePath, boolean duplicated) {
        freeMedia();
        media = new Media(FLibVLC, filePath);
        //media.addOption(":no-audio");
        media.addOption(":fullscreen");
        ///media.addOption(":no-autoscale");
        ///media.addOption(":file-caching=10000");//文件缓存
        ///media.addOption(":network-caching=10000");//网络缓存
        ///media.addOption(":live-caching=10000");//直播缓存
        ///media.addOption(":sout-mux-caching=10000");//输出缓存
        if (duplicated) media.addOption(new String(":sout=#duplicate{dst=rtp{sdp=rtsp://:8554/0},dst=display}"));//边推流边播放
        else media.addOption(new String(":sout=#rtp{sdp=rtsp://:8554/0}"));//只推流

        setHWDecoderEnabled(true);
        mMediaPlayer.setMedia(media);
        playerStatusInfo.setSourcePrepared(true);
        ///mMediaPlayer.setAspectRatio(null);
        ///mMediaPlayer.setScale(0);
        mMediaPlayer.setVolume(DefaultVolumeValue);

    }

    public void play() {
        if (media == null) {
            MMLog.log(TAG, "Calling play() media = null");
            return;
        }
        if (mMediaPlayer == null) {
            MMLog.log(TAG, "Calling play() mMediaPlayer = null");
            return;
        }
        MMLog.log(TAG, "Calling native play()");
        mMediaPlayer.play();
    }

    public void pause() {
        if (media == null) return;
        mMediaPlayer.pause();
    }

    public void fastForward(float x) {
        mMediaPlayer.setRate(x);
    }

    public void fastBack(float x) {
        mMediaPlayer.setRate(x);
    }

    public void fastForward(int x) {
        mMediaPlayer.setPosition(mMediaPlayer.getPosition() + x);
    }

    public void fastBack(int x) {
        mMediaPlayer.setPosition(mMediaPlayer.getPosition() - x);
    }

    public void fastForward(long x) {
        mMediaPlayer.setTime(mMediaPlayer.getTime() + x);
    }

    public void fastBack(long x) {
        mMediaPlayer.setTime(mMediaPlayer.getTime() - x);
    }

    public void playPause() {
        if (mMediaPlayer == null) return;
        if (isPlaying()) {
            pause();
        } else {
            play();
        }
    }

    public void stop() {
        if (mMediaPlayer == null) return;
        try {
            if (mMediaPlayer.getPlayerState() != Media.State.Stopped) mMediaPlayer.stop();
        } catch (Exception e) {
            MMLog.log(TAG, "Call stop() " + e.toString());
        }
    }

    @Override
    public void stopFree() {
        try {
            freeMediaPlayer();
            freeMedia();
            MMLog.log(TAG, "call stopFree() complete");
        } catch (Exception e) {
            //e.printStackTrace();
            MMLog.e(TAG, e.toString());
        }
    }

    public void resume() {
        if (mMediaPlayer == null) return;
        mMediaPlayer.play();
    }

    private void freeMediaPlayer() {
        if (vlcVout != null) {
            vlcVout.removeCallback(mIVLCVoutCallBack);
            vlcVout.detachViews();
            mSurfaceView = null;
        }
        if (mMediaPlayer != null) {
            mMediaPlayer.setEventListener(null);
            mMediaPlayer.stop();
            mMediaPlayer.release();
            mMediaPlayer = null;
        }
    }

    public void free() {
        try {
            if (vlcVout != null) {
                vlcVout.removeCallback(mIVLCVoutCallBack);
                vlcVout.detachViews();
                mSurfaceView = null;
            }
            if (mMediaPlayer != null) {
                mMediaPlayer.setEventListener(null);
                mMediaPlayer.stop();
                mMediaPlayer.release();
                mMediaPlayer = null;
            }
            if (media != null) {
                media.release();
                media = null;
            }
            if (FLibVLC != null) FLibVLC.release();
            FLibVLC = null;
        } catch (Exception e) {
            e.printStackTrace();
        }
        MMLog.log(TAG, "free()");
    }

    public Boolean isPlaying() {
        if (mMediaPlayer == null) return false;
        return mMediaPlayer.isPlaying();
    }

    public boolean isSeekable() {
        if (mMediaPlayer == null) return false;
        return mMediaPlayer.isSeekable();
    }

    public void setPlayTime(long l) {
        if (mMediaPlayer == null) return;
        mMediaPlayer.setTime(l);
    }

    public void setVolume(int var1) {
        if (mMediaPlayer == null) return;
        DefaultVolumeValue = var1;
        mMediaPlayer.setVolume(DefaultVolumeValue);
    }

    public int getVolume() {
        if (mMediaPlayer == null) return DefaultVolumeValue;
        DefaultVolumeValue = mMediaPlayer.getVolume();
        return DefaultVolumeValue;
    }

    public Long getTime() {
        if (mMediaPlayer == null) return 0L;
        return mMediaPlayer.getTime();
    }

    public float getPosition() {
        if (mMediaPlayer == null) return 0;
        return mMediaPlayer.getPosition();
    }

    public void setPosition(float f) {
        if (mMediaPlayer == null) return;
        mMediaPlayer.setPosition(f);
    }

    public long getLength() {
        if (mMediaPlayer == null) return 0;
        return mMediaPlayer.getLength();
    }

    public int getPlayerStatus() {
        if (mMediaPlayer != null) return mMediaPlayer.getPlayerState();
        else return PlaybackEvent.Status_Ended;
    }

    public void setRate(float v) {
        if (mMediaPlayer == null) return;
        mMediaPlayer.setRate(v);
    }

    public void setWindowSize(int width, int height) {
        if (mMediaPlayer == null) return;
        mMediaPlayer.getVLCVout().setWindowSize(width, height);
    }

    public void setAspectRatio(String aspect) {
        if (mMediaPlayer == null) return;
        mMediaPlayer.setAspectRatio(aspect);
    }

    public void setScale(float scale) {
        if (mMediaPlayer == null) return;
        mMediaPlayer.setScale(scale);
    }

    public void setHWDecoderEnabled(Boolean HWDecoderEnabled) {
        this.hwDecoderEnabled = HWDecoderEnabled;
        if (media != null) {
            try {
                media.setHWDecoderEnabled(HWDecoderEnabled, HWDecoderEnabled);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public void setNoAudio() {
        media.addOption(":no-audio");
    }

    public void setOption(String option) {
        this.media.addOption(option);
        mMediaPlayer.setMedia(media);
    }

    public int getAudioTracksCount() {
        ///if (mMediaPlayer == null)
        ///    return 0;
        return 0;//mMediaPlayer.getAudioTracksCount();
    }

    public Map<Integer, String> getAudioTracks() {
        Map<Integer, String> mtd = new HashMap<Integer, String>();
        if (mMediaPlayer == null) return mtd;
        /*
        MediaPlayer.TrackDescription[] TrackDescriptions = mMediaPlayer.getAudioTracks();
        if (TrackDescriptions == null) return mtd;

        for (MediaPlayer.TrackDescription td : TrackDescriptions) {
            mtd.put(td.id, td.name);
        }
        */

        int i = 0;
        Media.Track[] TrackDescriptions = mMediaPlayer.getTracks(Audio);
        for (Media.Track tracks : TrackDescriptions) {
            mtd.put(i, tracks.id + " " + tracks.name + " " + tracks.type);
            i++;
        }

        TrackDescriptions = mMediaPlayer.getTracks(Video);
        for (Media.Track tracks : TrackDescriptions) {
            mtd.put(i, tracks.id + " " + tracks.name + " " + tracks.type);
            i++;
        }

        TrackDescriptions = mMediaPlayer.getTracks(Text);
        for (Media.Track tracks : TrackDescriptions) {
            mtd.put(i, tracks.id + " " + tracks.name + " " + tracks.type);
            i++;
        }

        return mtd;
    }

    public int getAudioTrack() {
        ///if (mMediaPlayer == null)
        ///    return 0;
        return 0;//mMediaPlayer.getAudioTrack();
    }

    public void setAudioTrack(int index) {
        if (mMediaPlayer == null) return;
        ///mMediaPlayer.setAudioTrack(index);
        mMediaPlayer.selectTrack(String.valueOf(index));//.setAudioTrack(index);
    }

    @Override
    public void deselectTrack(int index) {

    }

    @Override
    public int getVideoWidth() {
        return 0;
    }

    @Override
    public int getVideoHeight() {
        return 0;
    }

    @Override
    public void startRecording(String filePath) {
        if(mMediaPlayer != null)
          mMediaPlayer.record(filePath,true);
    }

    @Override
    public void stopRecording(String filePath) {
        if (mMediaPlayer == null)
            return;
        mMediaPlayer.record(filePath,false);
    }

    public void setCallback(PlayerCallback callback) {
        this.playerEventCallBack = callback;
    }

    @Override
    public void onLayoutChange(View v, int left, int top, int right, int bottom, int oldLeft, int oldTop, int oldRight, int oldBottom) {
        int newWidth = right - left;
        int newHeight = bottom - top;
        mMediaPlayer.getVLCVout().setWindowSize(newWidth, newHeight);
    }

    class IVLCVoutCallBack implements IVLCVout.Callback, IVLCVout.OnNewVideoLayoutListener {
        @Override
        public void onNewVideoLayout(IVLCVout vlcVout, int width, int height, int visibleWidth, int visibleHeight, int sarNum, int sarDen) {
            MMLog.log(TAG, "IVLCVout.width=" + width + ",height=" + height + ",visibleWidth=" + visibleWidth + ",visibleHeight=" + visibleHeight + ",sarNum=" + sarNum + ",sarDen=" + sarDen);
            playerStatusInfo.setVideoH(height);
            playerStatusInfo.setVideoW(width);
            playerStatusInfo.setSurfaceH(visibleHeight);
            playerStatusInfo.setSurfaceW(visibleWidth);
        }

        @Override
        public void onSurfacesCreated(IVLCVout ivlcVout) {
            MMLog.log(TAG, "onSurfacesCreated.IVLCVout");
            try {
                if (mSurfaceView != null) {
                    MMLog.log(TAG, "mSurfaceView.getWidth() = " + mSurfaceView.getWidth() + ",getHeight() = " + mSurfaceView.getHeight());
                    vlcVout.setWindowSize(mSurfaceView.getWidth(), mSurfaceView.getHeight());
                    playerStatusInfo.setSurfaceH(mSurfaceView.getHeight());
                    playerStatusInfo.setSurfaceW(mSurfaceView.getWidth());
                } else if (mTextureView != null) {
                    vlcVout.setWindowSize(mTextureView.getWidth(), mTextureView.getHeight());
                }
                if (mMediaPlayer != null) {
                    mMediaPlayer.setAspectRatio(null);
                    mMediaPlayer.setScale(0);
                    playerStatusInfo.setSurfacePrepared(true);
                }
            } catch (Exception e) {
                //e.printStackTrace();
                MMLog.e(TAG, "onSurfacesCreated.IVLCVout, " + e.toString());
            }
        }

        @Override
        public void onSurfacesDestroyed(IVLCVout ivlcVout) {
            MMLog.log(TAG, "onSurfacesDestroyed.IVLCVout");
            playerStatusInfo.setSurfacePrepared(false);
        }
    }

}

