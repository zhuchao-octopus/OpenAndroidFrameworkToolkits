package com.zhuchao.android.utils;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

public class RadarBeepGenerator {
    private static final int SAMPLE_RATE = 44100;  // 采样率
    private AudioTrack audioTrack;
    private boolean isPlaying = false;

    public RadarBeepGenerator() {
        int bufferSize = AudioTrack.getMinBufferSize(SAMPLE_RATE,
                AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT);

        audioTrack = new AudioTrack(AudioManager.STREAM_VOICE_CALL, SAMPLE_RATE,
                AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT,
                bufferSize, AudioTrack.MODE_STREAM);
    }

    public void playBeep() {
        if (isPlaying) return;
        isPlaying = true;

        new Thread(() -> {
            short[] buffer = generateBeep(1000, 0.5);  // 生成 1000Hz 哔哔声
            audioTrack.play();
            while (isPlaying) {
                audioTrack.write(buffer, 0, buffer.length);
            }
        }).start();
    }

    public void stopBeep() {
        isPlaying = false;
        audioTrack.stop();
    }

    public void release() {
        audioTrack.release();
    }

    private short[] generateBeep(int frequency, double duration) {
        int samples = (int) (SAMPLE_RATE * duration);
        short[] buffer = new short[samples];

        for (int i = 0; i < samples; i++) {
            buffer[i] = (short) (Math.sin(2 * Math.PI * i * frequency / SAMPLE_RATE) * Short.MAX_VALUE);
        }
        return buffer;
    }

}
