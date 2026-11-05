package com.glitchaka.horrorme;

import android.app.Activity;
import android.os.Bundle;
import android.os.SystemClock;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import java.util.HashMap;
import java.util.Map;

public final class MainActivity extends Activity {
    private static final int PIXEL_WIDTH = 384;
    private static final int PIXEL_HEIGHT = 216;
    private static final int FORWARD=1<<0, BACK=1<<1, LEFT=1<<2, RIGHT=1<<3;
    private static final int TURN_LEFT=1<<4, TURN_RIGHT=1<<5, SPRINT=1<<6;
    private static final int INTERACT=1<<7, SCAN=1<<8, ANCHOR=1<<9;
    private static final int MAP=1<<10, REJECT=1<<11, ENTER=1<<12;
    private static final int PAUSE=1<<13, MUTE=1<<14;

    static {
        System.loadLibrary("horrorme");
    }
    private static native void nativeFrame(int buttons,float lookDelta,float deltaSeconds,int[] pixels);
    private static native int nativePhase();
    private static native int nativeSoundEvents();
    private static native float nativeTension();
    private static native void nativePause();

    private GameView gameView;

    private void immersive() {
        getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
                View.SYSTEM_UI_FLAG_FULLSCREEN |
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
    }

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        immersive();
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        gameView=new GameView();
        setContentView(gameView);
    }

    @Override public void onWindowFocusChanged(boolean focused) {
        super.onWindowFocusChanged(focused);
        if(focused) immersive();
    }

    @Override protected void onResume() {
        super.onResume();
        immersive();
        if(gameView!=null)gameView.resume();
    }
    @Override protected void onPause() {
        if(gameView!=null)gameView.pause();
        nativePause();
        super.onPause();
    }

    @Override public void onBackPressed() {
        if(gameView!=null)gameView.pulse(PAUSE);
    }

    private static int bitForKey(int code) {
        switch(code) {
        case KeyEvent.KEYCODE_W: return FORWARD;
        case KeyEvent.KEYCODE_S: return BACK;
        case KeyEvent.KEYCODE_A: return LEFT;
        case KeyEvent.KEYCODE_D: return RIGHT;
        case KeyEvent.KEYCODE_DPAD_UP: return FORWARD;
        case KeyEvent.KEYCODE_DPAD_DOWN: return BACK;
        case KeyEvent.KEYCODE_DPAD_LEFT: return TURN_LEFT;
        case KeyEvent.KEYCODE_DPAD_RIGHT: return TURN_RIGHT;
        case KeyEvent.KEYCODE_SHIFT_LEFT:
        case KeyEvent.KEYCODE_SHIFT_RIGHT: return SPRINT;
        case KeyEvent.KEYCODE_E: return INTERACT;
        case KeyEvent.KEYCODE_F: return SCAN;
        case KeyEvent.KEYCODE_Q: return ANCHOR;
        case KeyEvent.KEYCODE_TAB: return MAP;
        case KeyEvent.KEYCODE_R: return REJECT;
        case KeyEvent.KEYCODE_ENTER: return ENTER;
        case KeyEvent.KEYCODE_ESCAPE: return PAUSE;
        case KeyEvent.KEYCODE_M: return MUTE;
        default: return 0;
        }
    }
    @Override public boolean onKeyDown(int keyCode,KeyEvent event) {
        int bit=bitForKey(keyCode);
        if(bit==0)return super.onKeyDown(keyCode,event);
        if(gameView!=null)gameView.setKeyboard(bit,true);
        return true;
    }
    @Override public boolean onKeyUp(int keyCode,KeyEvent event) {
        int bit=bitForKey(keyCode);
        if(bit==0)return super.onKeyUp(keyCode,event);
        if(gameView!=null)gameView.setKeyboard(bit,false);
        return true;
    }

    private final class GameView extends SurfaceView implements SurfaceHolder.Callback,Runnable {
        private final SurfaceHolder surface;
        private final Paint paint=new Paint();
        private final Paint textPaint=new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Bitmap bitmap=Bitmap.createBitmap(PIXEL_WIDTH,PIXEL_HEIGHT,Bitmap.Config.ARGB_8888);
        private final int[] pixels=new int[PIXEL_WIDTH*PIXEL_HEIGHT];
        private final Rect sourceRect=new Rect(0,0,PIXEL_WIDTH,PIXEL_HEIGHT);
        private final AudioSynth sound=new AudioSynth();

        private final Object lock=new Object();
        private final Map<Integer,TouchPointer> pointers=new HashMap<>();
        private int keyboard=0,held=0,impulse=0;
        private float dragDelta=0;
        private volatile boolean resumed=true;
        private volatile boolean running=false;
        private volatile int phase=0;
        private boolean previousMute=false;
        private Thread thread;

        private static final int ROLE_MOVE=0;
        private static final int ROLE_LOOK=1;
        private static final int ROLE_BUTTON=2;

        private final class TouchPointer {
            int role;
            int bit;
            float x,y,previousX;
            TouchPointer(int kind,int mask,float px,float py) {
                role=kind; bit=mask; x=px;y=py;previousX=px;
            }
        }

        GameView() {
            super(MainActivity.this);
            surface=getHolder();
            surface.addCallback(this);
            setFocusable(true);
            setFocusableInTouchMode(true);
            paint.setFilterBitmap(false);
            textPaint.setTypeface(android.graphics.Typeface.create(
                    android.graphics.Typeface.MONOSPACE,android.graphics.Typeface.BOLD));
        }

        @Override public void surfaceCreated(SurfaceHolder holder) { startIfReady(); }
        @Override public void surfaceChanged(SurfaceHolder holder,int format,int width,int height) {}
        @Override public void surfaceDestroyed(SurfaceHolder holder) { stopRendering(); }

        synchronized void resume() {
            resumed=true;
            startIfReady();
        }
        synchronized void pause() {
            resumed=false;
            stopRendering();
        }
        private synchronized void startIfReady() {
            if(!resumed||running||!surface.getSurface().isValid())return;
            running=true;
            sound.start();
            thread=new Thread(this,"HorrorMeRender");
            thread.start();
        }
        private synchronized void stopRendering() {
            running=false;
            Thread copy=thread;
            thread=null;
            if(copy!=null&&copy!=Thread.currentThread()) {
                try {copy.join(1200);}catch(InterruptedException ignored) {
                    Thread.currentThread().interrupt();
                }
            }
            sound.stop();
            synchronized(lock) {
                pointers.clear();
                held=keyboard=impulse=0;
                dragDelta=0;
            }
        }

        void pulse(int bit) {
            synchronized(lock){impulse|=bit;}
        }
        void setKeyboard(int bit,boolean down) {
            synchronized(lock) {
                if(down){keyboard|=bit;impulse|=bit;}
                else keyboard&=~bit;
            }
        }

        @Override public void run() {
            long previous=System.nanoTime();
            while(running&&surface.getSurface().isValid()) {
                final long started=System.nanoTime();
                float dt=Math.max(0.001f,Math.min(.05f,(started-previous)/1_000_000_000f));
                previous=started;
                int keys;
                float look;
                synchronized(lock) {
                    keys=held|keyboard|impulse;
                    look=dragDelta;
                    dragDelta=0;
                    impulse=0;
                }
                boolean muteDown=(keys&MUTE)!=0;
                if(muteDown&&!previousMute)sound.toggleMute();
                previousMute=muteDown;
                nativeFrame(keys,look,dt,pixels);
                phase=nativePhase();
                sound.queue(nativeSoundEvents());
                sound.setTension(nativeTension());

                bitmap.setPixels(pixels,0,PIXEL_WIDTH,0,0,PIXEL_WIDTH,PIXEL_HEIGHT);
                Canvas screen=null;
                try {
                    screen=surface.lockCanvas();
                    if(screen!=null)drawScreen(screen);
                }catch(Exception ignored) {
                    // Surface may be detached while switching activities.
                }finally {
                    if(screen!=null) {
                        try {surface.unlockCanvasAndPost(screen);}
                        catch(Exception ignored){}
                    }
                }
                long workMillis=(System.nanoTime()-started)/1_000_000L;
                if(workMillis<20)SystemClock.sleep(20-workMillis);
            }
        }

        private float virtualX(float x) {return x*960f/Math.max(1,getWidth());}
        private float virtualY(float y) {return y*540f/Math.max(1,getHeight());}

        private boolean within(float x,float y,float cx,float cy,float radius) {
            float dx=x-cx,dy=y-cy;
            return dx*dx+dy*dy<radius*radius;
        }
        private int buttonAt(float x,float y) {
            if(phase==0||phase==3||phase==4)return ENTER;
            if(phase==2)return PAUSE;
            if(within(x,y,920,66,52))return PAUSE;
            if(within(x,y,920,134,42))return MUTE;
            if(within(x,y,893,327,77))return INTERACT;
            if(within(x,y,731,451,57))return SCAN;
            if(within(x,y,833,451,57))return ANCHOR;
            if(within(x,y,629,451,53))return MAP;
            if(within(x,y,929,469,55))return REJECT;
            if(within(x,y,284,473,58))return SPRINT;
            return 0;
        }

        private void recalculateHeld() {
            int bits=0;
            for(TouchPointer p:pointers.values()) {
                if(p.role==ROLE_BUTTON){bits|=p.bit;continue;}
                if(p.role==ROLE_MOVE) {
                    float dx=p.x-146,dy=p.y-408;
                    if(dy < -25)bits|=FORWARD;
                    if(dy > 25)bits|=BACK;
                    if(dx < -25)bits|=LEFT;
                    if(dx > 25)bits|=RIGHT;
                }
            }
            held=bits;
        }

        @Override public boolean onTouchEvent(MotionEvent event) {
            final int action=event.getActionMasked();
            final int actionIndex=event.getActionIndex();
            synchronized(lock) {
                if(action==MotionEvent.ACTION_CANCEL) {
                    pointers.clear();
                    held=0;
                    return true;
                }
                if(action==MotionEvent.ACTION_DOWN||action==MotionEvent.ACTION_POINTER_DOWN) {
                    int id=event.getPointerId(actionIndex);
                    float x=virtualX(event.getX(actionIndex));
                    float y=virtualY(event.getY(actionIndex));
                    int bit=buttonAt(x,y);
                    TouchPointer pointer;
                    if(bit!=0) {
                        pointer=new TouchPointer(ROLE_BUTTON,bit,x,y);
                        impulse|=bit;
                    } else if(x<340&&y>200) {
                        pointer=new TouchPointer(ROLE_MOVE,0,x,y);
                    } else {
                        pointer=new TouchPointer(ROLE_LOOK,0,x,y);
                    }
                    pointers.put(id,pointer);
                }
                for(int i=0;i<event.getPointerCount();++i) {
                    int id=event.getPointerId(i);
                    TouchPointer p=pointers.get(id);
                    if(p==null)continue;
                    final float x=virtualX(event.getX(i));
                    final float y=virtualY(event.getY(i));
                    if(p.role==ROLE_LOOK)
                        dragDelta+=Math.max(-40,Math.min(40,x-p.previousX))*.8f;
                    p.previousX=x;
                    p.x=x;p.y=y;
                }
                if(action==MotionEvent.ACTION_UP||action==MotionEvent.ACTION_POINTER_UP) {
                    pointers.remove(event.getPointerId(actionIndex));
                }
                recalculateHeld();
            }
            return true;
        }

        private void circle(Canvas c,float x,float y,float r,int fill,int stroke) {
            paint.setStyle(Paint.Style.FILL);
            paint.setColor(fill);
            c.drawCircle(x,y,r,paint);
            paint.setStyle(Paint.Style.STROKE);
            paint.setStrokeWidth(2f);
            paint.setColor(stroke);
            c.drawCircle(x,y,r,paint);
            paint.setStyle(Paint.Style.FILL);
        }

        private void control(Canvas c,float x,float y,float r,String label,int bit,int base) {
            final boolean pressed;
            synchronized(lock){pressed=(held&bit)!=0||(keyboard&bit)!=0;}
            circle(c,x,y,r,pressed?0x9983B9BF:0x66324753,
                    pressed?0xffd8ffff:0x99b4ccd3);
            textPaint.setColor(pressed?Color.WHITE:0xffd5e4e6);
            textPaint.setTextSize(label.length()>6?17:20);
            textPaint.setTextAlign(Paint.Align.CENTER);
            c.drawText(label,x,y+7,textPaint);
        }

        private void drawScreen(Canvas c) {
            c.drawColor(Color.BLACK);
            float w=c.getWidth(),h=c.getHeight();
            float scale=Math.min(w/PIXEL_WIDTH,h/PIXEL_HEIGHT);
            int displayW=Math.round(PIXEL_WIDTH*scale);
            int displayH=Math.round(PIXEL_HEIGHT*scale);
            int x0=Math.round((w-displayW)*.5f);
            int y0=Math.round((h-displayH)*.5f);
            c.drawBitmap(bitmap,sourceRect,
                    new Rect(x0,y0,x0+displayW,y0+displayH),paint);

            // UI is a native Android Canvas overlay, independent of the raycaster.
            c.save();
            c.scale(w/960f,h/540f);
            if(phase==0||phase==3||phase==4) {
                circle(c,480,445,83,0xaa374e50,0xffb6dfdf);
                textPaint.setTextSize(24);
                textPaint.setTextAlign(Paint.Align.CENTER);
                textPaint.setColor(0xfff3f7e9);
                c.drawText(phase==0?"ENTRAR":"REINICIAR",480,453,textPaint);
            } else {
                circle(c,146,408,100,0x443a6678,0x99b9d6dd);
                float stickX=146,stickY=408;
                synchronized(lock) {
                    for(TouchPointer p:pointers.values())if(p.role==ROLE_MOVE) {
                        stickX=146+Math.max(-65,Math.min(65,p.x-146));
                        stickY=408+Math.max(-65,Math.min(65,p.y-408));
                        break;
                    }
                }
                circle(c,stickX,stickY,35,0x779fcad1,0xffd7f5f4);
                textPaint.setTextSize(18);
                textPaint.setColor(0xffbbd2db);
                textPaint.setTextAlign(Paint.Align.CENTER);
                c.drawText("MOVER",146,291,textPaint);
                c.drawText("DESLIZA PARA MIRAR",470,385,textPaint);
                control(c,284,473,44,"CORRER",SPRINT,0);
                control(c,629,451,44,"PLANO",MAP,0);
                control(c,731,451,46,"ESCAN",SCAN,0);
                control(c,833,451,46,"ANCLA",ANCHOR,0);
                control(c,893,327,65,"SELLAR",INTERACT,0);
                control(c,929,469,44,"NO",REJECT,0);
                control(c,920,66,35,"PAUSA",PAUSE,0);
                control(c,920,134,32,"AUDIO",MUTE,0);
                if(phase==2) {
                    textPaint.setTextSize(28);
                    textPaint.setTextAlign(Paint.Align.CENTER);
                    textPaint.setColor(Color.WHITE);
                    c.drawText("TOCA PARA REANUDAR",480,270,textPaint);
                }
            }
            c.restore();
        }
    }
}
