package com.zhuchao.android.utils;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.SoundPool;

import com.zhuchao.android.fbase.MMLog;

public class RadarBeepManager {
    private static final String TAG = "RadarBeepManager";

    private SoundPool soundPool;
    private int beepSoundId = -1;
    private int streamId = -1;
    private boolean isLoaded = false;
    private final Context context;

    public RadarBeepManager(Context context) {
        this.context = context;
    }

    // 加载雷达音频文件（仅在需要时调用）
    public void loadBeepSound(int resId) {
        if (soundPool == null) {
            soundPool = new SoundPool.Builder()
                    .setMaxStreams(1)
                    .setAudioAttributes(new AudioAttributes.Builder()
                            .setUsage(AudioAttributes.USAGE_ASSISTANCE_SONIFICATION) // ✅ 关键点：保证与蓝牙通话混音
                            .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                            .build())
                    .build();
        }

        // 释放旧的音频
        if (beepSoundId != -1) {
            soundPool.unload(beepSoundId);
        }

        // 加载新的音频
        beepSoundId = soundPool.load(context, resId, 1);
        isLoaded = false;

        soundPool.setOnLoadCompleteListener((sp, sampleId, status) -> {
            if (status == 0 && sampleId == beepSoundId) {
                isLoaded = true;
                MMLog.d(TAG, "Beep sound loaded successfully.");
            } else {
                MMLog.e(TAG, "Failed to load beep sound.");
            }
        });
    }

    // 播放倒车雷达声音
    public void startBeep() {
        if (isLoaded) {
            streamId = soundPool.play(beepSoundId, 1.0f, 1.0f, 1, -1, 1.0f);
            MMLog.d(TAG, "Beep sound started.");
        } else {
            MMLog.e(TAG, "Beep sound not loaded yet!");
        }
    }

    // 停止播放倒车雷达声音
    public void stopBeep() {
        if (streamId != -1) {
            soundPool.stop(streamId);
            MMLog.d(TAG, "Beep sound stopped.");
        }
    }

    // 释放资源
    public void release() {
        if (soundPool != null) {
            soundPool.release();
            soundPool = null;
            MMLog.d(TAG, "SoundPool released.");
        }
    }
}

