package com.glitchaka.horrorme;

import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import java.util.concurrent.atomic.AtomicInteger;

/** Synthesized chiptune music and effects. No packaged audio assets. */
final class AudioSynth {
    private static final int RATE=22050;
    private final AtomicInteger pending=new AtomicInteger();
    private volatile float tension=0;
    private volatile boolean muted=false;
    private volatile boolean running;
    private Thread thread;
    private long frame;
    private double phase;
    private int activeEffect=-1,effectFrames;
    private int noise=0x167b34;

    void queue(int eventMask) {
        if(eventMask!=0) pending.getAndAccumulate(eventMask,(a,b)->a|b);
    }
    void setTension(float pressure) {tension=Math.max(0,Math.min(1,pressure));}
    void toggleMute() {muted=!muted;}

    synchronized void start() {
        if(running)return;
        running=true;
        thread=new Thread(this::synthesize,"HorrorMeAudio");
        thread.start();
    }
    synchronized void stop() {
        running=false;
        Thread t=thread;
        thread=null;
        if(t!=null&&t!=Thread.currentThread()) {
            try {t.join(800);}catch(InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }
    private void synthesize() {
        AudioTrack track=null;
        try {
            int buffer=Math.max(4096,AudioTrack.getMinBufferSize(RATE,
                    AudioFormat.CHANNEL_OUT_MONO,AudioFormat.ENCODING_PCM_16BIT));
            track=new AudioTrack(
                    new AudioAttributes.Builder()
                        .setUsage(AudioAttributes.USAGE_GAME)
                        .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC).build(),
                    new AudioFormat.Builder()
                        .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                        .setSampleRate(RATE)
                        .setChannelMask(AudioFormat.CHANNEL_OUT_MONO).build(),
                    buffer,AudioTrack.MODE_STREAM,AudioManager.AUDIO_SESSION_ID_GENERATE);
            if(track.getState()!=AudioTrack.STATE_INITIALIZED)return;
            track.play();
            short[] samples=new short[512];
            final double[] notes={110.0,116.54,130.81,103.83,110.0,98.0,87.31,103.83};
            while(running) {
                int bits=pending.getAndSet(0);
                if(bits!=0){
                    activeEffect=31-Integer.numberOfLeadingZeros(bits);
                    effectFrames=RATE/3;
                    phase=0;
                }
                float stress=tension;
                for(int j=0;j<samples.length;j++,frame++) {
                    double time=frame/(double)RATE;
                    int step=(int)(time*4.8);
                    double base=notes[(step/4)%notes.length];
                    double bass=.30*Math.sin(2*Math.PI*time*base*.5);
                    double drone=.10*Math.sin(2*Math.PI*time*base*1.006)
                                  +.055*Math.sin(2*Math.PI*time*base*1.503);
                    double beatPosition=(time*2.4)%1.0;
                    double beat=beatPosition<.14?
                            .24*Math.exp(-beatPosition*30)*
                            Math.sin(2*Math.PI*time*(68+55*(1-beatPosition))):0;
                    double notePosition=(time*4.8)%1.0;
                    double pulse=notePosition<.3?
                            .055*Math.signum(Math.sin(2*Math.PI*time*base*2))*
                            Math.exp(-notePosition*9):0;
                    noise^=noise<<13;
                    noise^=noise>>>17;
                    noise^=noise<<5;
                    double hiss=((noise&65535)/32768.0-1)*(.016+stress*.06);
                    double value=(bass+drone+beat+pulse+hiss)*(.22+stress*.11);

                    if(effectFrames>0) {
                        double progress=1-effectFrames/(RATE/3.0);
                        double freq;
                        switch(activeEffect) {
                            case 0: freq=620-410*progress;break; // Ancla
                            case 1: freq=420+900*progress;break; // Escaner
                            case 2: freq=700+190*Math.sin(progress*23);break;
                            case 3: freq=180-145*progress;break;
                            case 4: freq=85+230*progress;break;
                            case 5: freq=75-50*progress;break;
                            default:freq=440+660*progress;break;
                        }
                        phase+=2*Math.PI*freq/RATE;
                        double effect=Math.sin(phase);
                        if(activeEffect!=3&&activeEffect!=5)
                            effect=effect>0?.8:-.8;
                        value+=effect*Math.pow(1-progress,1.5)*.34;
                        effectFrames--;
                    }
                    samples[j]=(short)(muted?0:Math.max(-.90,Math.min(.90,value))*32700);
                }
                int written=track.write(samples,0,samples.length,AudioTrack.WRITE_BLOCKING);
                if(written<0)break;
            }
        } catch(Exception ignored) {
            // Missing or busy audio device must not prevent gameplay.
        } finally {
            if(track!=null) {
                try {track.pause();track.flush();track.release();}
                catch(Exception ignored){}
            }
        }
    }
}
